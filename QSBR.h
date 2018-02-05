#ifndef JUNCTION_QSBR_H
#define JUNCTION_QSBR_H

#include <vector>
#include <string.h>

#include <QMutex>
#include <QMutexLocker>

#define CALL_MEMBER(obj, pmf) ((obj).*(pmf))

class QSBR
{
private:
    struct Action {
        void (*func)(void*);
        quint64 param[4]; // Size limit found experimentally. Verified by assert below.

        Action(void (*f)(void*), void* p, quint64 paramSize) : func(f)
        {
            Q_ASSERT(paramSize <= sizeof(param)); // Verify size limit.
            memcpy(&param, p, paramSize);
        }

        void operator()()
        {
            func(&param);
        }
    };

    struct Status {
        qint16 inUse : 1;
        qint16 wasIdle : 1;
        qint16 nextFree : 14;

        Status() : inUse(1), wasIdle(0), nextFree(0)
        {
        }
    };

    QMutex m_mutex;
    std::vector<Status> m_status;
    qint64 m_freeIndex;
    qint64 m_numContexts;
    qint64 m_remaining;
    std::vector<Action> m_deferredActions;
    std::vector<Action> m_pendingActions;

    void onAllQuiescentStatesPassed(std::vector<Action>& callbacks);

public:
    typedef qint16 Context;

    QSBR() : m_freeIndex(-1), m_numContexts(0), m_remaining(0)
    {
    }

    Context createContext();
    void destroyContext(Context context);

    template <class T>
    void enqueue(void (T::*pmf)(), T* target)
    {
        struct Closure {
            void (T::*pmf)();
            T* target;

            static void thunk(void* param)
            {
                Closure* self = (Closure*) param;
                CALL_MEMBER(*self->target, self->pmf)();
            }
        };

        Closure closure = {pmf, target};
        QMutexLocker guard(&m_mutex);
        m_deferredActions.push_back(Action(Closure::thunk, &closure, sizeof(closure)));
    }

    void update(Context context);
    void flush();
};

extern QSBR DefaultQSBR;

#endif // JUNCTION_QSBR_H
