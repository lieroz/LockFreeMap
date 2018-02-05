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

// Include the implementation:
#include "ConditionVariable_CPP11.h"

// Alias it:
namespace turf
{
typedef turf::ConditionVariable_CPP11 ConditionVariable;
}

#endif // TURF_CONDITIONVARIABLE_H
