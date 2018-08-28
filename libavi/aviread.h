/*
 * Header file for basic AVI I/O reading routines.
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

#ifndef _AVIREAD_H
#define _AVIREAD_H

#include "fourcc.h"
#include "wformat.h"
#include "avicodes.h"
#include "riff_handle.h"
#include "riffwrite.h"
#include "defines.h"
#include "avi_structures.h"
#include "queue.h"
#include "avi_handle.h"


/* Some useful functions, similar to the ones in fourcc.h */

/* Build the two-character code for the stream N*/
uint32_t 
_AVRn2fcc(int num);

/* Get the number from a four character code */
int
_AVRfcc2n(uint32_t fourcc); 



/*************************************************************/
/* This function calls RFRopen_riff_file() and proceeds to
   parse the basic blocks of the avi structure and copy them
   to the avi_handle data structure where they can be easily
   referenced.

   Input : the filename
   Output : a pointer to a newly created handle, -1
            if it fails
   Side effects : - Advances the file pointer to the start of the
                  movi list. Reads the avi header, the stream
		  headers and the index chunk, if it exists.
		  - Allocates memory for the index chunk, sets up
		  its representation in memory and also allocates
		  memory for the avi_handle.

   NOTE: this function automagically reads all important structures
   and stores them in memory. The calling program may access them
   from the appropriate structures inside avi_handle. 

   If for some reason you wish to have absolute control over the
   parsing of the hdrl list you may wish to use the individual
   internal functions below, instead of the automagical 
   AVRopen_avi_file(). (AVRopen_avi_file essentially provides
   the "glue" between the internal functions)

*/
/*@null@*/
avi_handle *
AVRopen_avi_file(char *filename);


/*************************************************************/
/* This function frees the memory we allocated an properly closes
   all the file streams. Take care not to introduce memory leaks.
   
   Input: the avi_handle
   Output: 0 on success, <0 on failure
   Side effects: the avi_handle structure gets freed, it will
                 no longer be accessible, ALL files are closed.

*/
int
AVRclose_avi_file(avi_handle *input);



/*******************************************************************/
/* Initialize every stream to the appropriate position (its first
   frame). 
*/
int
_AVRinit_stream_position(avi_handle *input);


/*************************************************************/
/* This function does everything, except parse the header.
   It opens the file (with RFRopen_riff_file() and sets
   data structures to zero.

   This is actually called from AVRopen_avi_file() but
   stops before the header is parsed. 

   I made this for clarity and in order to allow the calling
   application to avoid the "automatic" behaviour of 
   AVRopen_avi_file().

   Input : the filename
   Output : avi_handle * or NULL if it fails
   Side effects :  avi_handle * gets allocated, riff_file gets written.

*/
/*@null@*/
avi_handle *
_AVRcreate_handle(char *filename);
   

/*************************************************************/
/* This function enters hdrl and reads the avi header. 

   Input : the avi_handle that has been created.
   Output : 0 on success, -1 on various failures
   Side effects : - The file pointer is moved inside hdrl and
                    after avih (normally before the first strl)
                  - The avi_header structure gets written.

   Prerequisites : - The file pointer must be positioned at the
       beggining of hdrl AND the first chunk must be avih (avi
       header). The avi_handle must have been correctly initialized.

 */
int
_AVRget_avi_header(avi_handle *input);


/*************************************************************/
/* This function is VERY important because it initializes 
   STREAM SUPPORT structures inside the avi_handle (have 
   a look at avi_handle.h).

   Input : the avi_handle
   Output : 0 on success, <0 on various failures
   Side effects : 
   - Advances the file pointer.
   - increments total_streams
   - Allocates memory for stream_header[i], stream_format[i]
     and stream_strd[i] and copies from the file to memory
   - Also sets stream_format_size[i] and stream_strd_size[i]
   - Zeroes the timers.

   Prerequisites : The file pointer must be positioned at the
   beggining of an strl list. Otherwise, the function fails.
   
   Expected chunks are :
   - strh
   - strf
   - strd
   - strn
   - JUNK
   
   Expected lists are :
   - strl
   - movi

   Unexpected chunks/lists will cause failure.

*/
int
_AVRget_stream_info(avi_handle *input);


/************************************************************/
/* This function reads the .idx structure from the avi file.
   and copies it to the avi_handle->idx_array, also setting
   avi_handle->idx_array_size (the number of entries)

   Input : avi_handle
   Output : 0 on success, -1 on various errors
   Side effects : memory is allocated for idx_array and
                  idx_array gets written, together with
		  idx_array_size. The file pointer is
		  RESTORED to its original position.
*/
int
_AVRget_idx(avi_handle *input);


/************************************************************/
/* This function builds an index structure (in MEMORY ONLY)
   from the avi file itself. This function can be called if:
   a) an index structure is necessary for the calling application
   b) the index structure is corrupt or cannot be trusted (?!)
   
   NOTE: this is time consuming because it scans through the
   whole file.

   Input : avi_handle
   Output : 0 on success, -1 on various errors
   Side effects : - The file pointer is not modified.
                  - Memory is allocated for the queue structure
		  AND the idx_array. The idx chunk is initially 
		  created in the queue structure and then copied
		  in the idx_array. NOTE that the queue structure
		  is NOT de-allocated automatically.
		  		  
 */
int
_AVRbuild_idx(avi_handle *input);


/************************************************************/
/* A frame is a chunk of recognizable type (dc, wb, db) inside 
   the movi list (or inside a rec list, in the movi list).
   Unknown chunks are silently IGNORED by the following functions.
   Use the libriff functions to read them. */
/* Linear view is enabled if we have an index. */

/* Return values are size in bytes. <0 means failure */

/************************************************************/
/* This function proceeds forward, either in linear view
   (see the libriff documentation!) or in hierarchical view
   until it meets a chunk that belongs to the given stream.

   Input: avi_handle, the stream desired.
   Output: size of the chunk in bytes. 0 is VALID!!! -1 means failure
   Side effects: - In linear view once you move forward you may not go
                   back to the start of the list. It is a "forward-only"
		   situation. Normally this is not a problem because
		   the index allows us to fseek().
		 - File position is changed. The file pointer moves
		   to the start of the next frame.
		 - This function will automatically update the index
		   pointers IF an index exists so that
		   AVRnext_frame_idx() will not be before AVRnext_frame()
		   (it may be after AVRnext_frame, this is normal)
		   
   NOTE: if MUST_USE_INDEX flag is set, this function will warn you.
*/
int
AVRnext_frame(avi_handle *input, int stream);

/*************************************************************/
/* Same as above, but get the next frame as reported from the
   index and not the next one in the file. This may skip some frames
   or may change the order of frames. (esp. note the MUST_USE_INDEX
   flag!!!).

   Input: avi_handle, the stream desired
   Output: size of the chunk in bytes, -1 means failure
   Side effects: - The file position changes, automatically
                   keeps synchronization with AVRnext_frame().
   Prerequisites: - Linear view!!!
                  - Index structure

   Note that if we don't have an index structure this will not work.
*/		  		  
int
AVRnext_frame_idx(avi_handle *input, int stream);


/*************************************************************/
/* We can only go backwards if we have an index. Get the previous
   frame, as reported by the index.

   I/O: see previous functions
   Side effects: - The file position changes
   
   As with all the _idx functions you have to enable linear
   view.
*/
int
AVRprevious_frame_idx(avi_handle *input, int stream);


/*************************************************************/
/* This is the slow version. It is only used if we DO NOT 
   have linear view+index. Try to avoid this. Intra-frame
   may not be decodable (you should seek to keyframes).
*/
int
AVRseek_to_frame(avi_handle *input, int stream, uint32_t frame);


/************************************************************/
/* Jump to frame "frame" as found in the index. Note that
   this may be different than frame "frame"  in the file 
   because the index is not always complete. In this case
   we trust the index.
*/
int
AVRseek_to_frame_idx(avi_handle *input, int stream, uint32_t frame);


/************************************************************/
/* Get the current frame!!!!!!  If the current chunk does
   NOT belong to the requested stream the function will
   NOT read it. You will have to call AVRnext_frame
   of AVRnext_frame_idx to go to the next eligible frame.

   To put it simply this function will get the current
   chunk IF it belongs to the stream we want.

   Input: avi_handle, stream number, unsigned char buffer for the
          data (MUST BE ABLE TO CONTAIN THE FULL FRAME!!!!!)
	  and pointer to variable where the frame type (fcc)
	  will be stored. If you don't want this you may use
	  NULL and frame_type will be ignored.
   Output: number of bytes read, -1 on various failures, 0 if the
           current chunk does not belong to the stream (please note
           that you may also get 0 if the size of the chunk is
	   0)
   Side effects: None (buffer gets written)


*/
int
AVRget_frame_idx(avi_handle *input, int stream, unsigned char *buffer, 
		 uint32_t *frame_type);

/************************************************************/
/* Get the current frame!!!!!!  If the current chunk does
   NOT belong to the requested stream the function will
   NOT read it. You will have to call AVRnext_frame
   of AVRnext_frame_idx to go to the next eligible frame.

   To put it simply this function will get the current
   chunk IF it belongs to the stream we want.

   THIS FUNCTION DOES NOT USE THE INDEX.

   Input: avi_handle, stream number, unsigned char buffer for the
          data (MUST BE ABLE TO CONTAIN THE FULL FRAME!!!!!)
	  and pointer to variable where the frame type (fcc)
	  will be stored. If you don't want this you may use
	  NULL and frame_type will be ignored.
   Output: number of bytes read, -1 on various failures, 0 if the
           current chunk does not belong to the stream (please note
           that you may also get 0 if the size of the chunk is
	   0)
   Side effects: File pointer advances to the next block, 
                 buffer gets written, frame_type gets written


*/
int
AVRget_frame(avi_handle *input, int stream, unsigned char *buffer, 
	     uint32_t *frame_type);

/************************************************************/
/* This function prints a detailed report of the data for
   stream number "stream" to the file "where", via fprintf.

   This is used for debugging and for user information.

   Report contains:
   - Stream header information (fccHandler, fccType etc)
   - Stream format information (is it WAVEFORMATEX etc + fields)
   - Stream data existance and size
   - Stream name
   - Stream file descriptor status (position, initialization)
  
   NOTE: this function should NOT fail with any absurd values
   (e.g. NULL pointers).   
*/
int
AVRprint_stream_info(avi_handle *input, FILE *where, unsigned int stream);


/************************************************************/
/* This function prints a detailed report of the data for
   the main header to the file "where", via fprintf.

   This is used for debugging and for user information.

   Report contains:
   - MainAVIHeader structure
   - Some avi_handle info
  
   NOTE: this function should NOT fail with any absurd values
   (e.g. NULL pointers).   
*/
int
AVRprint_header_info(avi_handle *input, FILE *where);

int
AVRget_next_frame_size(avi_handle *input, int stream);

#endif


