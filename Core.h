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

typedef char s8;
typedef unsigned char u8;
typedef short s16;
typedef unsigned short u16;
typedef int s32;
typedef unsigned int u32;
typedef long long sptr;
typedef unsigned long long uptr;
typedef long long sreg;
typedef unsigned long long ureg;
typedef long long s64;
typedef unsigned long long u64;

#define TURF_CALL_MEMBER(obj, pmf) ((obj).*(pmf))

#define TURF_TRACE(arg1, arg2, arg3, arg4, arg5)
#define TURF_ASSERT(arg1)
#define TURF_STATIC_ASSERT(arg1)

#endif // TURF_CORE_H
