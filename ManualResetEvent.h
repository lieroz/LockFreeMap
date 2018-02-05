/*------------------------------------------------------------------------
  Turf: Configurable C++ platform adapter
  Copyright (c) 2016 Jeff Preshing

  Distributed under the Simplified BSD License.
  Original location: https://github.com/preshing/turf

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the LICENSE file for more information.
------------------------------------------------------------------------*/

#ifndef TURF_MANUALRESETEVENT_H
#define TURF_MANUALRESETEVENT_H

#include <Core.h>

// Include the implementation:
#include "ManualResetEvent_CondVar.h"

// Alias it:
namespace turf {
typedef turf::ManualResetEvent_CondVar ManualResetEvent;
}

#endif // TURF_MANUALRESETEVENT_H
