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


#ifndef _AVIOPS_H
#define _AVIOPS_H

/* flags for use in <dwFlags> in AVIFileHdr */
#define AVIF_HASINDEX           0x00000010      /*  Index at end of file? */
#define AVIF_MUSTUSEINDEX       0x00000020
#define AVIF_ISINTERLEAVED      0x00000100
#define AVIF_TRUSTCKTYPE        0x00000800      /* Use CKType to find key frames? */
#define AVIF_WASCAPTUREFILE     0x00010000
#define AVIF_COPYRIGHTED        0x00020000

#include "swap_endian.h"
#include "riffcodes.h"

#ifdef _INSIDE_AVIWRITE

const uint32_t opmovi=FOURCC('m','o','v','i');
const uint32_t opvids=FOURCC('v','i','d','s');
const uint32_t opauds=FOURCC('a','u','d','s');
const uint32_t optxts=FOURCC('t','x','t','s');  

const uint32_t opAVI=FOURCC('A','V','I',' ');
const uint32_t opavih=FOURCC('a','v','i','h');

const uint32_t oprec=FOURCC('r','e','c',' ');
const uint32_t opstrh=FOURCC('s','t','r','h');
const uint32_t opstrf=FOURCC('s','t','r','f');
const uint32_t opstrl=FOURCC('s','t','r','l');

const uint32_t ophdrl=FOURCC('h','d','r','l');
const uint32_t opstrd=FOURCC('s','t','r','d');
const uint32_t opstrn=FOURCC('s','t','r','n');
const uint32_t opidx1=FOURCC('i','d','x','1');


const uint16_t opdb=TWOCC('d','b');
const uint16_t opdc=TWOCC('d','c');
const uint16_t oppc=TWOCC('p','c');
const uint16_t opwb=TWOCC('w','b');

#else /* outside of aviwrite */

extern const uint32_t opRIFF;
extern const uint32_t opmovi;
extern const uint32_t opvids;
extern const uint32_t opauds;

extern const uint32_t optxts;
extern const uint32_t opAVI;
extern const uint32_t opLIST;
extern const uint32_t opavih;

extern const uint32_t oprec;
extern const uint32_t opstrh;
extern const uint32_t opstrf;
extern const uint32_t opstrl;

extern const uint32_t ophdrl;
extern const uint32_t opJUNK;
extern const uint32_t opstrd;
extern const uint32_t opstrn;

extern const uint32_t opidx1;

extern const uint16_t opdb;
extern const uint16_t opdc;
extern const uint16_t oppc;
extern const uint16_t opwb;

#endif /* _INSIDE_AVIWRITE */

#endif
