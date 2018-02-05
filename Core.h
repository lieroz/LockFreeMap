/*------------------------------------------------------------------------
  Turf: Configurable C++ platform adapter
  Copyright (c) 2016 Jeff Preshing

  Distributed under the Simplified BSD License.
  Original location: https://github.com/preshing/turf

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the LICENSE file for more information.
------------------------------------------------------------------------*/

#ifndef TURF_CORE_H
#define TURF_CORE_H

#include <QtCore>

#define TURF_CALL_MEMBER(obj, pmf) ((obj).*(pmf))

#define TURF_TRACE(arg1, arg2, arg3, arg4, arg5)
#define TURF_ASSERT(arg1)
#define TURF_STATIC_ASSERT(arg1)

#endif // TURF_CORE_H
