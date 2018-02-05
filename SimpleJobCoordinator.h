#ifndef JUNCTION_SIMPLEJOBCOORDINATOR_H
#define JUNCTION_SIMPLEJOBCOORDINATOR_H

#include <Core.h>
#include <QMutex>
#include <QWaitCondition>
#include <QMutexLocker>

class SimpleJobCoordinator
{
public:
    struct Job {
        virtual ~Job()
        {
        }

        virtual void run() = 0;
    };

private:
    QBasicAtomicInteger<quint64> m_job;
    QMutex mutex;
    QWaitCondition condVar;

public:
    SimpleJobCoordinator() : m_job(quint64(NULL))
    {
    }

    Job* loadConsume() const
    {
        return (Job*) m_job.loadAcquire();
    }

    void storeRelease(Job* job)
    {
        {
            QMutexLocker guard(&mutex);
            m_job.storeRelease(quint64(job));
        }

        condVar.wakeAll();
    }

    void participate()
    {
        quint64 prevJob = quint64(NULL);

        for (;;) {
            quint64 job = m_job.loadAcquire();
            if (job == prevJob) {
                QMutexLocker guard(&mutex);

                for (;;) {
                    job = m_job.load(); // No concurrent writes inside lock
                    if (job != prevJob) {
                        break;
                    }

                    condVar.wait(&mutex);
                }
            }

            if (job == 1) {
                return;
            }

            reinterpret_cast<Job*>(job)->run();
            prevJob = job;
        }
    }

    void runOne(Job* job)
    {
        Q_ASSERT(job != (Job*) m_job.load());
        storeRelease(job);
        job->run();
    }

    void end()
    {
        {
            QMutexLocker guard(&mutex);
            m_job.storeRelease(1);
        }

        condVar.wakeAll();
    }
};

#endif // JUNCTION_SIMPLEJOBCOORDINATOR_H
