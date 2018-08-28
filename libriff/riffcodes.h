/*
 * Copyright (C) 2001 Petros Tsantoulis <ptsant@otenet.gr>
 *
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 *
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with this program; if not, write to the Free Software
 *     Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111, USA.
 */

#ifndef _RIFFCODES_H
#define _RIFFCODES_H

#include <stdint.h>
#include "fourcc.h"
#include "swap_endian.h"

#ifdef _INSIDE_RIFF
EXPORT const uint32_t opRIFF=FOURCC('R','I','F','F');
EXPORT const uint32_t opLIST=FOURCC('L','I','S','T');
EXPORT const uint32_t opJUNK=FOURCC('J','U','N','K');

#else

extern const uint32_t opRIFF;
extern const uint32_t opLIST;
extern const uint32_t opJUNK;

#endif

#endif
