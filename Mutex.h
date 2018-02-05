/*------------------------------------------------------------------------
  Turf: Configurable C++ platform adapter
  Copyright (c) 2016 Jeff Preshing

  Distributed under the Simplified BSD License.
  Original location: https://github.com/preshing/turf

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the LICENSE file for more information.
------------------------------------------------------------------------*/

#ifndef TURF_MUTEX_H
#define TURF_MUTEX_H

namespace turf
{
template <typename LockType>
class LockGuard;
} // namespace turf

#include "Mutex_CPP11.h"

// Alias it:
namespace turf
{

typedef turf::Mutex_CPP11 Mutex;

//---------------------------------------------------------
// Generic LockGuard
//---------------------------------------------------------
template <typename LockType> class LockGuard
{
private:
    LockType& m_lock;

public:
    LockGuard(LockType& lock) : m_lock(lock)
    {
        m_lock.lock();
    }
    ~LockGuard()
    {
        m_lock.unlock();
    }
    LockType& getMutex()
    {
        return m_lock;
    }
};

} // namespace turf

#endif // TURF_MUTEX_H
