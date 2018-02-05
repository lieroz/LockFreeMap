/*------------------------------------------------------------------------
  Turf: Configurable C++ platform adapter
  Copyright (c) 2016 Jeff Preshing

  Distributed under the Simplified BSD License.
  Original location: https://github.com/preshing/turf

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the LICENSE file for more information.
------------------------------------------------------------------------*/

#ifndef TURF_ATOMIC_H
#define TURF_ATOMIC_H

#include "Atomic_CPP11.h"

// Alias it:
namespace turf
{

template <typename T> class Atomic : public Atomic_CPP11<T>
{
public:
    Atomic()
    {
    }
    Atomic(T value) : Atomic_CPP11<T>(value)
    {
    }
    ~Atomic()
    {
    }
};

} // namespace turf

#endif // TURF_ATOMIC_H
