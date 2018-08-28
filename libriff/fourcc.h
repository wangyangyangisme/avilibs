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


#ifndef _FOURCC_H
#define _FOURCC_H

/* We need these to ensure proper file structure regardless of endian-ness */
#ifndef FOURCC
#define FOURCC( ch0, ch1, ch2, ch3 )                                \
        ( (long)(unsigned char)(ch0) | ( (long)(unsigned char)(ch1) << 8 ) |            \
        ( (long)(unsigned char)(ch2) << 16 ) | ( (long)(unsigned char)(ch3) << 24 ) )
#endif

#ifndef TWOCC
#define TWOCC( ch0, ch1 ) \
((long)(unsigned char)(ch0) | ( (long)(unsigned char)(ch1)<<8))
#endif

#ifndef COMBINETWO2CC
#define COMBINETWO2CC( uint16_1, uint16_2 ) \
((long)(uint16_t)(uint16_1) | ((long)(uint16_t)(uint16_2)<<16))
#endif

/*      divx codecs     */
#define fccDIVX FOURCC('D','I','V','X')
#define fccDIV3 FOURCC('D', 'I', 'V', '3')
#define fccDIV4 FOURCC('D', 'I', 'V', '4')
#define fccdiv3 FOURCC('d', 'i', 'v', '3')
#define fccdiv4 FOURCC('d', 'i', 'v', '4')
#define fccMP41 FOURCC('M', 'P', '4', '1')
#define fccMP43 FOURCC('M', 'P', '4', '3')
/*      old ms mpeg-4 codecs */
#define fccMP42 FOURCC('M', 'P', '4', '2')
#define fccmp42 FOURCC('m', 'p', '4', '2')
#define fccmp43 FOURCC('m', 'p', '4', '3')
#define fccmpg4 FOURCC('m', 'p', 'g', '4')
#define fccMPG4 FOURCC('M', 'P', 'G', '4')
/*      windows media codecs */
#define fccWMV1 FOURCC('W', 'M', 'V', '1')
#define fccwmv1 FOURCC('w', 'm', 'v', '1')
#define fccWMV2 FOURCC('W', 'M', 'V', '2')
#define fccwmv2 FOURCC('w', 'm', 'v', '2')
/*      other codecs    */
#define fccIV32 FOURCC('I', 'V', '3', '2')
#define fccIV41 FOURCC('I', 'V', '4', '1')
#define fccIV50 FOURCC('I', 'V', '5', '0')
#define fccI263 FOURCC('I', '2', '6', '3')
#define fccCVID FOURCC('c', 'v', 'i', 'd')
#define fccVCR2 FOURCC('V', 'C', 'R', '2')
#define fccMJPG FOURCC('M', 'J', 'P', 'G')
#define fccYUV  FOURCC('Y', 'U', 'V', ' ')
#define fccYUY2 FOURCC('Y', 'U', 'Y', '2')
#define fccYV12 FOURCC('Y', 'V', '1', '2')/* Planar mode: Y + V + U  (3 planes) */
#define fccIYUV FOURCC('I', 'Y', 'U', 'V')/* Planar mode: Y + U + V  (3 planes) */
#define fccUYVY FOURCC('U', 'Y', 'V', 'Y')/* Packed mode: U0+Y0+V0+Y1 (1 plane) */
#define fccYVYU FOURCC('Y', 'V', 'Y', 'U')/* Packed mode: Y0+V0+Y1+U0 (1 plane) */


#endif
