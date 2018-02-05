#include <QSBR.h>

QSBR DefaultQSBR;

QSBR::Context QSBR::createContext()
{
    QMutexLocker guard(&m_mutex);
    m_numContexts++;
    m_remaining++;
    Q_ASSERT(m_numContexts < (1 << 14));
    qint64 context = m_freeIndex;

    if (context >= 0) {
        Q_ASSERT(context < (qint64) m_status.size());
        Q_ASSERT(!m_status[context].inUse);
        m_freeIndex = m_status[context].nextFree;
        m_status[context] = Status();
    } else {
        context = m_status.size();
        m_status.append(Status());
    }

    return context;
}

void QSBR::destroyContext(QSBR::Context context)
{
    QVector<Action> actions;
    {
        QMutexLocker guard(&m_mutex);
        Q_ASSERT(context < m_status.size());
        if (m_status[context].inUse && !m_status[context].wasIdle) {
            Q_ASSERT(m_remaining > 0);
            --m_remaining;
        }

        m_status[context].inUse = 0;
        m_status[context].nextFree = m_freeIndex;
        m_freeIndex = context;
        m_numContexts--;

        if (m_remaining == 0) {
            onAllQuiescentStatesPassed(actions);
        }
    }

    for (quint64 i = 0; i < actions.size(); i++) {
        actions[i]();
    }
}

void QSBR::onAllQuiescentStatesPassed(QVector<Action>& actions)
{
    // m_mutex must be held
    actions.swap(m_pendingActions);
    m_pendingActions.swap(m_deferredActions);
    m_remaining = m_numContexts;

    for (quint64 i = 0; i < m_status.size(); i++) {
        m_status[i].wasIdle = 0;
    }
}

void QSBR::update(QSBR::Context context)
{
    QVector<Action> actions;
    {
        QMutexLocker guard(&m_mutex);
        Q_ASSERT(context < m_status.size());
        Status& status = m_status[context];
        Q_ASSERT(status.inUse);

        if (status.wasIdle) {
            return;
        }

        status.wasIdle = 1;
        Q_ASSERT(m_remaining > 0);

        if (--m_remaining > 0) {
            return;
        }

        onAllQuiescentStatesPassed(actions);
    }

    for (quint64 i = 0; i < actions.size(); i++) {
        actions[i]();
    }
}

void QSBR::flush()
{
    // This is like saying that all contexts are quiescent,
    // so we can issue all actions at once.
    // No lock is taken.
    for (quint64 i = 0; i < m_pendingActions.size(); i++) {
        m_pendingActions[i]();
    }

    m_pendingActions.clear();

    for (quint64 i = 0; i < m_deferredActions.size(); i++) {
        m_deferredActions[i]();
    }

    m_deferredActions.clear();
    m_remaining = m_numContexts;
}
