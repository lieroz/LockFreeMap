/*------------------------------------------------------------------------
  Turf: Configurable C++ platform adapter
  Copyright (c) 2016 Jeff Preshing

  Distributed under the Simplified BSD License.
  Original location: https://github.com/preshing/turf

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the LICENSE file for more information.
------------------------------------------------------------------------*/

#ifndef TURF_CONDITIONVARIABLE_H
#define TURF_CONDITIONVARIABLE_H

#include <Mutex.h>
#include <condition_variable>

// Alias it:
namespace turf
{
class ConditionVariable
{
private:
    std::condition_variable m_condVar;

public:
    void wait(turf::LockGuard<Mutex>& guard)
    {
        m_condVar.wait(guard);
    }

    void timedWait(turf::LockGuard<Mutex>& guard, ureg waitMillis)
    {
        if (waitMillis > 0)
            m_condVar.wait_for(guard, std::chrono::milliseconds(waitMillis));
    }

    void wakeOne()
    {
        m_condVar.notify_one();
    }

    void wakeAll()
    {
        m_condVar.notify_all();
    }
};
}

#endif // TURF_CONDITIONVARIABLE_H
