/*
 * Header file for the avi_handle structure.
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


#ifndef _AVI_HANDLE
#define _AVI_HANDLE

#include <stdio.h>

#include "riff_handle.h"
#include "avi_structures.h"

#define MAX_STREAMS 128

/* This is the structure we use to keep track of multiple avi files */
typedef struct _avi_handle {
  
  int read_mode; /* 1 means reading only, 0 means writing only */

  /* We use 64-bit data to support future +4GB files */ /* No we don't */
  riff_handle  *riff_file;  
  MainAVIHeader avi_header; /* Just in case we need this information elsewhere */
  
  long movi_offset;


  /********* STREAM SUPPORT *****************************************/

  /* Number of streams allocated */
  uint32_t vids_streams;
  uint32_t auds_streams;
  uint32_t txts_streams;
  uint32_t total_streams;

  AVIStreamHeader *stream_header[MAX_STREAMS]; /* Support up to MAX_STREAMS streams. This is more than
						  the possible number of streams (=100)
						  */

  /* Note that since the strf chunk can have a variable size,
     (especially with WAVEFORMATEX chunks that can have surplus
     "tail" information for the decoder) we use a void pointer.
     Use caution when "casting" the void pointer. */
  void *stream_format[MAX_STREAMS]; 
  int  stream_format_size[MAX_STREAMS];       /* These two describe the contents of the strf
						 chunk. Cast them to the appropriate data
						 structure (BITMAPINFO, WAVEFORMATEX). */

  /* The avi file format allows the codecs to write extra initialization information in 
     the form of a "strd" chunk inside the stream list. */
  void *stream_strd[MAX_STREAMS];
  int  stream_strd_size[MAX_STREAMS];  
  
  riff_handle *stream_file[MAX_STREAMS];   /* We open the file once for each stream. We then
					  use different file pointers to access the streams
					  in order to minimize the calls to fseek(). Note
					  that the position of the streams in the file may
					  differ. E.g. an avi file with all the audio
					  followed by all the video chunks is VALID (even
					  though it is REALLY BAD) and should be read
					  properly.. */

  char *stream_name[MAX_STREAMS];   /* An optional "strn" (stream name) chunk may be
				       provided. It is placed in a NULL terminated string. */

  /* I have to say that I never encountered an avi file that provided a stream name!! :-) */


  double stream_time[MAX_STREAMS];     /* PRE-ALPHA stage, timing is not yet a planned feature */
  int    stream_chunk[MAX_STREAMS];   /* The current chunk, counting only the ones that
				       belong to this stream. This is a marker of position,
				      similar to stream_time (more accurate, propably) */
  
  int stream_idxp[MAX_STREAMS];      /* This is our position in the index. */
  /********* INDEX SUPPORT **********/

  /* Make an index structure, something like a linked list */
  queue_handle *idx_list;

  /* This is the index structure in array form, only created when reading. 
   This is more economical than a list representation. The list representation 
   is used when writing (building the index).*/
  AVIINDEXENTRY *idx_array;
  uint32_t idx_array_size;  /* In idx structures, not bytes... */
  

} avi_handle;

#endif









