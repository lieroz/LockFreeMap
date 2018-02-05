/*------------------------------------------------------------------------
  Junction: Concurrent data structures in C++
  Copyright (c) 2016 Jeff Preshing

  Distributed under the Simplified BSD License.
  Original location: https://github.com/preshing/junction

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the LICENSE file for more information.
------------------------------------------------------------------------*/

#ifndef JUNCTION_SIMPLEJOBCOORDINATOR_H
#define JUNCTION_SIMPLEJOBCOORDINATOR_H

#include <QMutex>
#include <QWaitCondition>
#include <QMutexLocker>

namespace junction
{

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
    SimpleJobCoordinator() : m_job(uptr(NULL))
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
        uptr prevJob = uptr(NULL);
        for (;;) {
            uptr job = m_job.loadAcquire();
            if (job == prevJob) {
                QMutexLocker guard(&mutex);
                for (;;) {
                    job = m_job.load(); // No concurrent writes inside lock
                    if (job != prevJob)
                        break;
                    condVar.wait(&mutex);
                }
            }
            if (job == 1)
                return;
            reinterpret_cast<Job*>(job)->run();
            prevJob = job;
        }
    }

    void runOne(Job* job)
    {
//        TURF_ASSERT(job != (Job*) m_job.load(turf::Relaxed));
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

} // namespace junction

#endif // JUNCTION_SIMPLEJOBCOORDINATOR_H
