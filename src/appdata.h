/*
 * This file is part of mpuzzle
 *
 * Copyright (C) 2005 Nokia Corporation.
 *
 *
 * This software is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this software; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#ifndef MPUZZLE_APPDATA_H
#define MPUZZLE_APPDATA_H

#include <libosso.h>

typedef struct _AppData AppData;

struct _AppData
{
    HildonProgram *program; /* handle to application */
    HildonWindow *window; /* handle to app's window */
    osso_context_t *osso; /* handle to osso */
};

#endif
