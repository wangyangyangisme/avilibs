/*  
 * Set of functions that change data endiannes.
 *  
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



/********************************************************************/
/* NOTE : I DO NOT OWN A BIG-ENDIAN MACHINE. THIS CODE MIGHT BE
   TERRIBLY WRONG. IF YOU CARE ABOUT IT, EMAIL ME WITH DETAILS
   OR FIX IT YOURSELF. 
*/

#ifndef _SWAP_H
#define _SWAP_H

/*This is an adjustment to make libriff compile w/ MingGW or other Windows Gcc tools*/
#if defined UNIX
#include <endian.h>
#endif

#include <stdint.h>

#include "defines.h"

#if __BYTE_ORDER == __LITTLE_ENDIAN

#ifndef INT_TO_LE_INT
#define INT_TO_LE_INT(x) (x)
#endif

#ifndef WORD_TO_LE_WORD
#define WORD_TO_LE_WORD(x) (x)
#endif

#else


#ifndef INT_TO_LE_INT
/* Please note: this is not fast. */
#define INT_TO_LE_INT(x) ((x&0xff)<<24+((x>>8)&0xff)<<16+((x>>16)&0xff)<<8+((x>>24)&0xff)))
#endif

#ifndef WORD_TO_LE_WORD
#define WORD_TO_LE_WORD(x) ((x&0xff)<<8 + ((x>>8)&0xff))
#endif

#endif /* __BYTE_ORDER == __LITTLE_ENDIAN */

#endif



