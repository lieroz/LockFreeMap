/*------------------------------------------------------------------------
  Junction: Concurrent data structures in C++
  Copyright (c) 2016-2017 Jeff Preshing

  Distributed under the Simplified BSD License.
  Original location: https://github.com/preshing/junction

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the LICENSE file for more information.
------------------------------------------------------------------------*/

#ifndef JUNCTION_STRIPED_CONDITIONBANK_H
#define JUNCTION_STRIPED_CONDITIONBANK_H

#include <Core.h>
#include <striped/ConditionPair.h>

#include <Mutex_SpinLock.h>
#include <Util.h>

namespace junction
{
namespace striped
{

class ConditionBank
{
private:
    static const ureg SizeMask = 1023;
    turf::Mutex_SpinLock m_initSpinLock;
    turf::Atomic<ConditionPair*> m_pairs;

    ConditionPair* initialize();

public:
    ~ConditionBank();

    ConditionPair& get(void* ptr)
    {
        ConditionPair* pairs = m_pairs.load(turf::Consume);
        if (!pairs) {
            pairs = initialize();
        }
        ureg index = turf::util::avalanche(uptr(ptr)) & SizeMask;
        return pairs[index];
    }
};

extern ConditionBank DefaultConditionBank;

} // namespace striped
} // namespace junction

#define JUNCTION_STRIPED_CONDITIONBANK_DEFINE_MEMBER()
#define JUNCTION_STRIPED_CONDITIONBANK_GET(objectPtr) (junction::striped::DefaultConditionBank.get(objectPtr))

#endif // JUNCTION_STRIPED_CONDITIONBANK_H
