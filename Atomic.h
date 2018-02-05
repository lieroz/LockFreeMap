#ifndef ATOMIC_H
#define ATOMIC_H

#include <qatomic.h>

template <class T>
class AtomicBase
{
protected:
    QAtomicInteger<T> m_value;

public:
    T fetchAndAndRelaxed(T valueToAdd)
    {
        return this->m_value.fetchAndAndRelaxed(valueToAdd);
    }

    T fetchAndAndAcquire(T valueToAdd)
    {
        return this->m_value.fetchAndAndAcquire(valueToAdd);
    }

    T fetchAndAndRelease(T valueToAdd)
    {
        return this->m_value.fetchAndAndRelease(valueToAdd);
    }

    T fetchAndAndOrdered(T valueToAdd)
    {
        return this->m_value.fetchAndAndOrdered(valueToAdd);
    }

    T fetchAndOrRelaxed(T valueToAdd)
    {
        return this->m_value.fetchAndOrRelaxed(valueToAdd);
    }

    T fetchAndOrAcquire(T valueToAdd)
    {
        return this->m_value.fetchAndOrAcquire(valueToAdd);
    }

    T fetchAndOrRelease(T valueToAdd)
    {
        return this->m_value.fetchAndOrRelease(valueToAdd);
    }

    T fetchAndOrOrdered(T valueToAdd)
    {
        return this->m_value.fetchAndOrOrdered(valueToAdd);
    }

    T fetchAndXorRelaxed(T valueToAdd)
    {
        return this->m_value.fetchAndXorRelaxed(valueToAdd);
    }

    T fetchAndXorAcquire(T valueToAdd)
    {
        return this->m_value.fetchAndXorAcquire(valueToAdd);
    }

    T fetchAndXorRelease(T valueToAdd)
    {
        return this->m_value.fetchAndXorRelease(valueToAdd);
    }

    T fetchAndXorOrdered(T valueToAdd)
    {
        return this->m_value.fetchAndXorOrdered(valueToAdd);
    }

    T operator&=(T v)
    {
        return fetchAndAndOrdered(v) & v;
    }

    T operator|=(T v)
    {
        return fetchAndOrOrdered(v) | v;
    }

    T operator^=(T v)
    {
        return fetchAndXorOrdered(v) ^ v;
    }
};

template <class T>
class AtomicBase<T*>
{
protected:
    QAtomicPointer<T> m_value;
};

template <class T>
class Atomic : public AtomicBase<T>
{
public:
    Atomic() = default;

    Atomic(T value)
    {
        this->m_value = value;
    }

    T load() const
    {
        return this->m_value.load();
    }

    void store(T newValue)
    {
        this->m_value.store(newValue);
    }

    operator T() const
    {
        return loadAcquire();
    }

    T operator=(T newValue)
    {
        storeRelease(newValue);
        return newValue;
    }

    T loadAcquire() const
    {
        return this->m_value.loadAcquire();
    }

    void storeRelease(T newValue)
    {
        this->m_value.storeRelease(newValue);
    }

    T compareExchangeRelaxed(T expectedValue, T newValue) {
        this->m_value.testAndSetRelaxed(expectedValue, newValue);
        return expectedValue;
    }

    T compareExchangeAcquire(T expectedValue, T newValue) {
        this->m_value.testAndSetAcquire(expectedValue, newValue);
        return expectedValue;
    }

    T compareExchangeRelease(T expectedValue, T newValue) {
        this->m_value.testAndSetRelease(expectedValue, newValue);
        return expectedValue;
    }

    T compareExchangeOrdered(T expectedValue, T newValue) {
        this->m_value.testAndSetOrdered(expectedValue, newValue);
        return expectedValue;
    }

    bool testAndSetRelaxed(T expectedValue, T newValue)
    {
        return this->m_value.testAndSetRelaxed(expectedValue, newValue);
    }

    bool testAndSetAcquire(T expectedValue, T newValue)
    {
        return this->m_value.testAndSetAcquire(expectedValue, newValue);
    }

    bool testAndSetRelease(T expectedValue, T newValue)
    {
        return this->m_value.testAndSetRelease(expectedValue, newValue);
    }

    bool testAndSetOrdered(T expectedValue, T newValue)
    {
        return this->m_value.testAndSetOrdered(expectedValue, newValue);
    }

    bool testAndSetRelaxed(T expectedValue, T newValue, T &currentValue)
    {
        return this->m_value.testAndSetRelaxed(expectedValue, newValue, &currentValue);
    }

    bool testAndSetAcquire(T expectedValue, T newValue, T &currentValue)
    {
        return this->m_value.testAndSetAcquire(expectedValue, newValue, &currentValue);
    }

    bool testAndSetRelease(T expectedValue, T newValue, T &currentValue)
    {
        return this->m_value.testAndSetRelease(expectedValue, newValue, &currentValue);
    }

    bool testAndSetOrdered(T expectedValue, T newValue, T &currentValue)
    {
        return this->m_value.testAndSetOrdered(expectedValue, newValue, &currentValue);
    }

    T fetchAndStoreRelaxed(T newValue)
    {
        return this->m_value.fetchAndStoreRelaxed(newValue);
    }

    T fetchAndStoreAcquire(T newValue)
    {
        return this->m_value.fetchAndStoreAcquire(newValue);
    }

    T fetchAndStoreRelease(T newValue)
    {
        return this->m_value.fetchAndStoreRelease(newValue);
    }

    T fetchAndStoreOrdered(T newValue)
    {
        return this->m_value.fetchAndStoreOrdered(newValue);
    }

    T fetchAndAddRelaxed(qptrdiff valueToAdd)
    {
        return this->m_value.fetchAndAddRelaxed(valueToAdd);
    }

    T fetchAndAddAcquire(qptrdiff valueToAdd)
    {
        return this->m_value.fetchAndAddAcquire(valueToAdd);
    }

    T fetchAndAddRelease(qptrdiff valueToAdd)
    {
        return this->m_value.fetchAndAddRelease(valueToAdd);
    }

    T fetchAndAddOrdered(qptrdiff valueToAdd)
    {
        return this->m_value.fetchAndAddOrdered(valueToAdd);
    }

    T fetchAndSubRelaxed(qptrdiff valueToAdd)
    {
        return this->m_value.fetchAndSubRelaxed(valueToAdd);
    }

    T fetchAndSubAcquire(qptrdiff valueToAdd)
    {
        return this->m_value.fetchAndSubAcquire(valueToAdd);
    }

    T fetchAndSubRelease(qptrdiff valueToAdd)
    {
        return this->m_value.fetchAndSubRelease(valueToAdd);
    }

    T fetchAndSubOrdered(qptrdiff valueToAdd)
    {
        return this->m_value.fetchAndSubOrdered(valueToAdd);
    }

    T operator++()
    {
        return fetchAndAddOrdered(1) + 1;
    }

    T operator++(int)
    {
        return fetchAndAddOrdered(1);
    }

    T operator--()
    {
        return fetchAndSubOrdered(1) - 1;
    }

    T operator--(int)
    {
        return fetchAndSubOrdered(1);
    }

    T operator+=(qptrdiff valueToAdd)
    {
        return fetchAndAddOrdered(valueToAdd) + valueToAdd;
    }

    T operator-=(qptrdiff valueToSub)
    {
        return fetchAndSubOrdered(valueToSub) - valueToSub;
    }
};

#endif // ATOMIC_H
