/*
 * Header file for standard AVI structures
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


#ifndef _AVI_STRUCTURES
#define _AVI_STRUCTURES

#include <stdint.h>

/* avih */
typedef struct _MainAVIHeader {
    uint32_t ulMicroSecPerFrame;
    uint32_t ulMaxBytesPerSec;
    uint32_t ulPaddingGranularity;
    uint32_t ulFlags;
    uint32_t ulTotalFrames;
    uint32_t ulInitialFrames;
    uint32_t ulStreams;
    uint32_t ulSuggestedBufferSize;
    uint32_t ulWidth;
    uint32_t ulHeight;
    uint32_t ulReserved[4]; /* Set to zero */
} MainAVIHeader;


#define AVISF_DISABLED 0x0000001   /* These values are not known for sure */
#define AVISF_VIDEO_PALCHANGES 0x0001000 

/* strh */
typedef struct _AVIStreamHeader {
    uint32_t fccType;
    uint32_t fccHandler;
    uint32_t ulFlags;
    uint32_t ulPriority;
    uint32_t ulInitialFrames;
    uint32_t ulScale;
    uint32_t ulRate;
    uint32_t ulStart;
    uint32_t ulLength;
    uint32_t ulSuggestedBufferSize;
    uint32_t ulQuality; /* Between 0 - 10000, -1 means use default */
    uint32_t ulSampleSize;
    uint32_t ulrcFrame[2];  /* Coordinates x,y */
} AVIStreamHeader;
    

/* strf chunk : WAVEFORMATEX, PCMWAVERFORMAT, BITMAPINFO */

/* NOTE : documentation refers to Width, Height and X/Y Pixels/meter as LONG */
/* This is assumed to be a signed 32 bit int */

typedef struct _BITMAPINFOHEADER {
  uint32_t lSize;
  int32_t lWidth; /* Width, in pixels */
  int32_t lHeight; /* Height, in pixels */
  uint16_t wPlanes;
  uint16_t wBitCount;
  uint32_t lCompression;
  uint32_t lSizeImage; /* Size in bytes, of the image, can be 0 for uncompressed RGB */
  int16_t wXPelsPerMeter;
  int16_t wYPelsPerMeter;
  uint32_t lClrUsed;
  uint32_t lClrImportant;
} BITMAPINFOHEADER;


typedef struct _RGBQUAD {
  unsigned char rgbBlue;
  unsigned char rgbGreen;
  unsigned char rgbRed;
  unsigned char rgbReserved; /* alpha channel, propably */
} RGBQUAD;

typedef struct _BITMAPINFO {
  BITMAPINFOHEADER bmiHeader;
  RGBQUAD bmiColors[1];  /* I won't use it. */
} BITMAPINFO;

typedef struct _WAVEFORMATEX {
  uint16_t wFormatTag;
  uint16_t wChannels;
  uint32_t lSamplesPerSec;
  uint32_t lAvgBytesPerSec;
  uint16_t wBlockAlign;
  uint16_t wBitsPerSample;
  uint16_t wSize; /* Size in bytes of the surplus header,
		     depending on the format! */
} WAVEFORMATEX;


typedef struct _AVIINDEXENTRY {
  uint32_t  lckid; /* FOURCC of the chunk id */
  uint32_t  lFlags; /* Logical OR of the flags, please check docs */
  uint32_t  lChunkOffset; /* >Including< the header, i.e. before */
  uint32_t  lChunkLength; /* Not including the header */
} AVIINDEXENTRY;


#define AVIIF_LIST      0x00000001
#define AVIIF_KEYFRAME  0x00000010
#define AVIIF_FIRSTPART 0x00000020
#define AVIIF_LASTPART  0x00000040
#define AVIIF_MIDPART   (AVIIF_LASTPART | AVIFF_FIRSTPART)
#define AVIIF_NOTIME    0x00000100
#define AVIIF_COMPUSE   0x0fff0000

#define AVIIF_KNOWN_FLAGS 0x0fff0171

#endif
