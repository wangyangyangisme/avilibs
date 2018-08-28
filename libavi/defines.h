/*  
 * Common definitions that may be used by all files...
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

/* Various defines... */
#ifndef _DEFINES_H
#define _DEFINES_H

#ifndef __LINE__
#define __LINE__ 0
#endif

#ifndef __FILE__
#define __FILE__ "FILE.c"
#endif

#ifndef __FUNCTION__
#define __FUNCTION "you are out of luck!"
#endif

/* this is for the WIN32 PORT */ 
#if defined WIN32 
#if defined BUILD_DLL
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __declspec(dllimport)
#endif
#else 
#define EXPORT
#endif

#define CHECKS  /* Sanity checks */
#define EXTENDED_CHECKS /* Paranoid checks */
#define VERBOSE  /* Information reporting */
/* #define VERY_VERBOSE *//* Lots of redundant information */

#endif

