#ifndef JUNCTION_CONCURRENTMAP_LEAPFROG_H
#define JUNCTION_CONCURRENTMAP_LEAPFROG_H

#include <Core.h>
#include <Leapfrog.h>
#include <QSBR.h>

#include <QAtomicPointer>

template <typename K, typename V, class KT = DefaultKeyTraits<K>, class VT = DefaultValueTraits<V> >
class ConcurrentMap_Leapfrog
{
public:
    typedef K Key;
    typedef V Value;
    typedef KT KeyTraits;
    typedef VT ValueTraits;
    typedef typename BestFit<Key>::Unsigned Hash;
    typedef Leapfrog<ConcurrentMap_Leapfrog> Details;

private:
    QAtomicPointer<typename Details::Table> m_root;

public:
    ConcurrentMap_Leapfrog(quint64 capacity = Details::InitialSize) : m_root(Details::Table::create(capacity))
    {
    }

    ~ConcurrentMap_Leapfrog()
    {
        typename Details::Table* table = m_root.load();
        table->destroy();
    }

    // publishTableMigration() is called by exactly one thread from Details::TableMigration::run()
    // after all the threads participating in the migration have completed their work.
    void publishTableMigration(typename Details::TableMigration* migration)
    {
        // There are no racing calls to this function.
        typename Details::Table* oldRoot = m_root.load();
        m_root.storeRelease(migration->m_destination);
        TURF_ASSERT(oldRoot == migration->getSources()[0].table);
        // Caller will GC the TableMigration and the source table.
    }

    // A Mutator represents a known cell in the hash table.
    // It's meant for manipulations within a temporary function scope.
    // Obviously you must not call QSBR::Update while holding a Mutator.
    // Any operation that modifies the table (exchangeValue, eraseValue)
    // may be forced to follow a redirected cell, which changes the Mutator itself.
    // Note that even if the Mutator was constructed from an existing cell,
    // exchangeValue() can still trigger a resize if the existing cell was previously marked deleted,
    // or if another thread deletes the key between the two steps.
    class Mutator
    {
    private:
        friend class ConcurrentMap_Leapfrog;

        ConcurrentMap_Leapfrog& m_map;
        typename Details::Table* m_table;
        typename Details::Cell* m_cell;
        Value m_value;

        // Constructor: Find existing cell
        Mutator(ConcurrentMap_Leapfrog& map, Key key, bool) : m_map(map), m_value(Value(ValueTraits::NullValue))
        {
            TURF_TRACE(ConcurrentMap_Leapfrog, 0, "[Mutator] find constructor called", quint64(0), quint64(key));
            Hash hash = KeyTraits::hash(key);
            for (;;) {
                m_table = m_map.m_root.loadAcquire();
                m_cell = Details::find(hash, m_table);
                if (!m_cell)
                    return;
                Value value = m_cell->value.load(Consume);
                if (value != Value(ValueTraits::Redirect)) {
                    // Found an existing value
                    m_value = value;
                    return;
                }
                // We've encountered a Redirect value. Help finish the migration.
                TURF_TRACE(ConcurrentMap_Leapfrog, 1, "[Mutator] find was redirected", quint64(m_table), 0);
                m_table->jobCoordinator.participate();
                // Try again using the latest root.
            }
        }

        // Constructor: Insert or find cell
        Mutator(ConcurrentMap_Leapfrog& map, Key key) : m_map(map), m_value(Value(ValueTraits::NullValue))
        {
            TURF_TRACE(ConcurrentMap_Leapfrog, 2, "[Mutator] insertOrFind constructor called", quint64(0), quint64(key));
            Hash hash = KeyTraits::hash(key);
            for (;;) {
                m_table = m_map.m_root.loadAcquire();
                quint64 overflowIdx;
                switch (Details::insertOrFind(hash, m_table, m_cell, overflowIdx)) { // Modifies m_cell
                case Details::InsertResult_InsertedNew: {
                    // We've inserted a new cell. Don't load m_cell->value.
                    return;
                }
                case Details::InsertResult_AlreadyFound: {
                    // The hash was already found in the table.
                    Value value = m_cell->value.load(Consume);
                    if (value == Value(ValueTraits::Redirect)) {
                        // We've encountered a Redirect value.
                        TURF_TRACE(ConcurrentMap_Leapfrog, 3, "[Mutator] insertOrFind was redirected", quint64(m_table), quint64(m_value));
                        break; // Help finish the migration.
                    }
                    // Found an existing value
                    m_value = value;
                    return;
                }
                case Details::InsertResult_Overflow: {
                    // Unlike ConcurrentMap_Linear, we don't need to keep track of & pass a "mustDouble" flag.
                    // Passing overflowIdx is sufficient to prevent an infinite loop here.
                    // It defines the start of the range of cells to check while estimating total cells in use.
                    // After the first migration, deleted keys are purged, so if we hit this line during the
                    // second loop iteration, every cell in the range will be in use, thus the estimate will be 100%.
                    // (Concurrent deletes could result in further iterations, but it will eventually settle.)
                    Details::beginTableMigration(m_map, m_table, overflowIdx);
                    break;
                }
                }
                // A migration has been started (either by us, or another thread). Participate until it's complete.
                m_table->jobCoordinator.participate();
                // Try again using the latest root.
            }
        }

    public:
        Value getValue() const
        {
            // Return previously loaded value. Don't load it again.
            return Value(m_value);
        }

        Value exchangeValue(Value desired)
        {
            TURF_ASSERT(desired != Value(ValueTraits::NullValue));
            TURF_ASSERT(desired != Value(ValueTraits::Redirect));
            TURF_ASSERT(m_cell); // Cell must have been found or inserted
            TURF_TRACE(ConcurrentMap_Leapfrog, 4, "[Mutator::exchangeValue] called", quint64(m_table), quint64(m_value));
            for (;;) {
                Value oldValue = m_value;
                if (m_cell->value.compareExchangeStrong(m_value, desired, ConsumeRelease)) {
                    // Exchange was successful. Return previous value.
                    TURF_TRACE(ConcurrentMap_Leapfrog, 5, "[Mutator::exchangeValue] exchanged Value", quint64(m_value), quint64(desired));
                    Value result = m_value;
                    m_value = desired; // Leave the mutator in a valid state
                    return result;
                }
                // The CAS failed and m_value has been updated with the latest value.
                if (m_value != Value(ValueTraits::Redirect)) {
                    TURF_TRACE(ConcurrentMap_Leapfrog, 6, "[Mutator::exchangeValue] detected race to write value", quint64(m_table), quint64(m_value));
                    if (oldValue == Value(ValueTraits::NullValue) && m_value != Value(ValueTraits::NullValue)) {
                        TURF_TRACE(ConcurrentMap_Leapfrog, 7, "[Mutator::exchangeValue] racing write inserted new value", quint64(m_table), quint64(m_value));
                    }
                    // There was a racing write (or erase) to this cell.
                    // Pretend we exchanged with ourselves, and just let the racing write win.
                    return desired;
                }
                // We've encountered a Redirect value. Help finish the migration.
                TURF_TRACE(ConcurrentMap_Leapfrog, 8, "[Mutator::exchangeValue] was redirected", quint64(m_table), quint64(m_value));
                Hash hash = m_cell->hash.load();
                for (;;) {
                    // Help complete the migration.
                    m_table->jobCoordinator.participate();
                    // Try again in the new table.
                    m_table = m_map.m_root.loadAcquire();
                    m_value = Value(ValueTraits::NullValue);
                    quint64 overflowIdx;
                    switch (Details::insertOrFind(hash, m_table, m_cell, overflowIdx)) { // Modifies m_cell
                    case Details::InsertResult_AlreadyFound:
                        m_value = m_cell->value.load(Consume);
                        if (m_value == Value(ValueTraits::Redirect)) {
                            TURF_TRACE(ConcurrentMap_Leapfrog, 9, "[Mutator::exchangeValue] was re-redirected", quint64(m_table), quint64(m_value));
                            break;
                        }
                        goto breakOuter;
                    case Details::InsertResult_InsertedNew:
                        goto breakOuter;
                    case Details::InsertResult_Overflow:
                        TURF_TRACE(ConcurrentMap_Leapfrog, 10, "[Mutator::exchangeValue] overflow after redirect", quint64(m_table), overflowIdx);
                        Details::beginTableMigration(m_map, m_table, overflowIdx);
                        break;
                    }
                    // We were redirected... again
                }
            breakOuter:;
                // Try again in the new table.
            }
        }

        void assignValue(Value desired)
        {
            exchangeValue(desired);
        }

        Value eraseValue()
        {
            TURF_ASSERT(m_cell); // Cell must have been found or inserted
            TURF_TRACE(ConcurrentMap_Leapfrog, 11, "[Mutator::eraseValue] called", quint64(m_table), quint64(m_cell));
            for (;;) {
                if (m_value == Value(ValueTraits::NullValue))
                    return Value(m_value);
                TURF_ASSERT(m_cell); // m_value is non-NullValue, therefore cell must have been found or inserted.
                if (m_cell->value.compareExchangeStrong(m_value, Value(ValueTraits::NullValue), Consume)) {
                    // Exchange was successful and a non-NULL value was erased and returned by reference in m_value.
                    TURF_ASSERT(m_value != ValueTraits::NullValue); // Implied by the test at the start of the loop.
                    Value result = m_value;
                    m_value = Value(ValueTraits::NullValue); // Leave the mutator in a valid state
                    return result;
                }
                // The CAS failed and m_value has been updated with the latest value.
                TURF_TRACE(ConcurrentMap_Leapfrog, 12, "[Mutator::eraseValue] detected race to write value", quint64(m_table), quint64(m_cell));
                if (m_value != Value(ValueTraits::Redirect)) {
                    // There was a racing write (or erase) to this cell.
                    // Pretend we erased nothing, and just let the racing write win.
                    return Value(ValueTraits::NullValue);
                }
                // We've been redirected to a new table.
                TURF_TRACE(ConcurrentMap_Leapfrog, 13, "[Mutator::eraseValue] was redirected", quint64(m_table), quint64(m_cell));
                Hash hash = m_cell->hash.load(); // Re-fetch hash
                for (;;) {
                    // Help complete the migration.
                    m_table->jobCoordinator.participate();
                    // Try again in the new table.
                    m_table = m_map.m_root.loadAcquire();
                    m_cell = Details::find(hash, m_table);
                    if (!m_cell) {
                        m_value = Value(ValueTraits::NullValue);
                        return m_value;
                    }
                    m_value = m_cell->value.load(Relaxed);
                    if (m_value != Value(ValueTraits::Redirect))
                        break;
                    TURF_TRACE(ConcurrentMap_Leapfrog, 14, "[Mutator::eraseValue] was re-redirected", quint64(m_table), quint64(m_cell));
                }
            }
        }
    };

    Mutator insertOrFind(Key key)
    {
        return Mutator(*this, key);
    }

    Mutator find(Key key)
    {
        return Mutator(*this, key, false);
    }

    // Lookup without creating a temporary Mutator.
    Value get(Key key)
    {
        Hash hash = KeyTraits::hash(key);
        TURF_TRACE(ConcurrentMap_Leapfrog, 15, "[get] called", quint64(this), quint64(hash));
        for (;;) {
            typename Details::Table* table = m_root.loadAcquire();
            typename Details::Cell* cell = Details::find(hash, table);
            if (!cell)
                return Value(ValueTraits::NullValue);
            Value value = cell->value.load(Consume);
            if (value != Value(ValueTraits::Redirect))
                return value; // Found an existing value
            // We've been redirected to a new table. Help with the migration.
            TURF_TRACE(ConcurrentMap_Leapfrog, 16, "[get] was redirected", quint64(table), quint64(hash));
            table->jobCoordinator.participate();
            // Try again in the new table.
        }
    }

    Value assign(Key key, Value desired)
    {
        Mutator iter(*this, key);
        return iter.exchangeValue(desired);
    }

    Value exchange(Key key, Value desired)
    {
        Mutator iter(*this, key);
        return iter.exchangeValue(desired);
    }

    Value erase(Key key)
    {
        Mutator iter(*this, key, false);
        return iter.eraseValue();
    }

    // The easiest way to implement an Iterator is to prevent all Redirects.
    // The currrent Iterator does that by forbidding concurrent inserts.
    // To make it work with concurrent inserts, we'd need a way to block TableMigrations.
    class Iterator
    {
    private:
        typename Details::Table* m_table;
        quint64 m_idx;
        Key m_hash;
        Value m_value;

    public:
        Iterator(ConcurrentMap_Leapfrog& map)
        {
            // Since we've forbidden concurrent inserts (for now), nonatomic would suffice here, but let's plan ahead:
            m_table = map.m_root.loadAcquire();
            m_idx = -1;
            next();
        }

        void next()
        {
            TURF_ASSERT(m_table);
            TURF_ASSERT(isValid() || m_idx == -1); // Either the Iterator is already valid, or we've just started iterating.
            while (++m_idx <= m_table->sizeMask) {
                // Index still inside range of table.
                typename Details::CellGroup* group = m_table->getCellGroups() + (m_idx >> 2);
                typename Details::Cell* cell = group->cells + (m_idx & 3);
                m_hash = cell->hash.load();
                if (m_hash != KeyTraits::NullHash) {
                    // Cell has been reserved.
                    m_value = cell->value.load(Relaxed);
                    TURF_ASSERT(m_value != Value(ValueTraits::Redirect));
                    if (m_value != Value(ValueTraits::NullValue))
                        return; // Yield this cell.
                }
            }
            // That's the end of the map.
            m_hash = KeyTraits::NullHash;
            m_value = Value(ValueTraits::NullValue);
        }

        bool isValid() const
        {
            return m_value != Value(ValueTraits::NullValue);
        }

        Key getKey() const
        {
            TURF_ASSERT(isValid());
            // Since we've forbidden concurrent inserts (for now), nonatomic would suffice here, but let's plan ahead:
            return KeyTraits::dehash(m_hash);
        }

        Value getValue() const
        {
            TURF_ASSERT(isValid());
            return m_value;
        }
    };
};

#endif // JUNCTION_CONCURRENTMAP_LEAPFROG_H
