/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
The Membrane (C) project
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#ifndef SM_APPLY_H
#define SM_APPLY_H

#include "sm_event.h"
#include "sm_state.h"

#define SM_FSM(S) (*(S)->fsm)

// Public methods

void sm_apply_event(sm_event *e, sm_state *s); 

#endif //SM_APPLY_H
