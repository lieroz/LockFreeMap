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

#include <mutex>

namespace turf
{
template <typename LockType>
class LockGuard;
} // namespace turf

// Alias it:
namespace turf
{

class Mutex : protected std::mutex
{
private:
    friend class LockGuard<Mutex>;

public:
    Mutex() : std::mutex()
    {
    }

    void lock()
    {
        std::mutex::lock();
    }

    bool tryLock()
    {
        return std::mutex::try_lock();
    }

    void unlock()
    {
        std::mutex::unlock();
    }
};

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

// Specialize LockGuard<Mutex_CPP11> so that ConditionVariable_CPP11 can use it:
template <>
class LockGuard<Mutex> : public std::unique_lock<std::mutex>
{
public:
    LockGuard(Mutex& mutex) : std::unique_lock<std::mutex>(mutex)
    {
    }
};

} // namespace turf

#endif // TURF_MUTEX_H
