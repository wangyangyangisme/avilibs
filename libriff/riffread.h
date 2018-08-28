/*
 * Header file for basic riff reading routines 
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


/*
  Please note that moving backwards in a RIFF file is inherently
  rather hard to do because the blocks (chunks/lists) are linked
  in a forward direction only, meaning that there is no way to
  seek to a previous block without reading the file from the 
  beggining. 
  
  This library does not implement a way of seeking backwards
  quickly because a) That would require an index structure
  b) It may not be necessary c) In avi files an index structure
  already exists for the movi list, so if you use avi files you
  do NOT need an extra mechanism for bacwkard seeking.

  However, due to the fact that this library saves the start 
  offset of all lists you CAN use that to seek to the 
  higher level list immediately (exit_list()). Backwards
  seeking of chunks is not supported, unless of course you
  use RFRseek_to_position() and you know what you are doing.

  Another mechanism that can be useful is RFRsave_position
  and RFRrestore_position that can help you do some back and
  forth in the file. 
*/

#ifndef _RIFFREAD_H
#define _RIFFREAD_H

#include "defines.h"
#include "riffwrite.h"

/* NOTE: RFR prefix stands for "riff read" functions. This is 
   here to remind you that you should not use an RFR function
   when writing. (RFW prefix) */ 

/* Open a RIFF file for reading and create the necessary handle. */
/* Accepts a filename, returns a riff_handle *, NULL on failure! */
EXPORT riff_handle *
RFRopen_riff_file(char *filename);


/* Close a RIFF file and FREE MEMORY. */
EXPORT int
RFRclose_riff_file(riff_handle *handle);

/* Skip the next chunk. Please note that this WILL NOT skip
   a list. If a list is found, it will return 1. If it
   skips a chunk succesfully it returns 0 and finally, <0
   on various failures. */
EXPORT int
RFRskip_chunk(riff_handle *input);

/* Skip the next list. Please note that this WILL NOT skip 
   a chunk, if that is the next item. You should know whether
   the next item is a chunk or a list by using RFRget_block_info.
   Returns 1 if a chunk is found, 0 on success, <0 on various 
   failures.
*/
EXPORT int
RFRskip_list(riff_handle *input);

/* Returns the next item's (chunk or LIST) size by default, <0 on various
   failures. 

   NOTE: RFRget_next_info advances the file_position.

   This function fills internal data structures (namely block_type,
   block_size and block_subtype)
 */
EXPORT int
_RFRget_next_info(riff_handle *input);

/* Returns the next item's (chunk or LIST) size by default, <0 on various
   failures. If type, size and list_type are null it will only return the
   next item's size without failing. */
EXPORT int
RFRget_block_info(riff_handle *input, 
	      uint32_t *type, 
	      uint32_t *size,
	      uint32_t *list_type);  /* list_type is ONLY USED IF TYPE==LIST */

/* Get the chunk data. 
   CAUTION: the buffer MUST BE ABLE TO CONTAIN AN AMOUNT OF DATA EQUAL TO
   THE CHUNK SIZE. NO CHECKING IS DONE. IF IT CRASHES, YOU ARE THE ONE TO
   BLAME! */
EXPORT int
RFRget_chunk(riff_handle *input, unsigned char *buffer);

/* Enter the list, that is go to the first item in that list. */
/* Returns 0 on success, -1 on various failures. */
EXPORT int
RFRenter_list(riff_handle *input);

/**************************************************************/
/* Exit the list, that is go to the START OF THE LIST. Not 
   AFTER the list. (not inside the list, however)
*/
EXPORT int
RFRgoto_list_start(riff_handle *handle);

/**************************************************************/
/* Exit the list, that is go to the END OF THE LIST, positioning
   the file pointer at the first item AFTER the list.
*/
EXPORT int
RFRgoto_list_end(riff_handle *handle);

/***************************************************************************/
/* Exit the list. Stay where you are. This is potentially dangerous. 
   It should be used when you have reached the end of the list. It does
   not fseek() and therefore it is faster than RFRgoto_list_end() 

   PLEASE NOTE: Do NOT use unless you are at the end of a list or
   at least at a block start. */
EXPORT int
RFRexit_list(riff_handle *handle);

/*****************************************************************************/
/* Instructs the library to skip all JUNK chunks by default,
 ignoring them when skipping. An exception is RFRseek_forward_to_chunk_type(JUNK)
 that will seek until the next JUNK chunk.
 1 means skip JUNK chunks, 0 means stop at them. */
EXPORT int
RFRskip_junk(riff_handle *input, int skip_junk);  

/* Will seek forward, entering and exiting lists until
   this type of chunk is found. */
EXPORT int
RFRseek_forward_to_chunk_type(riff_handle *input, uint32_t type);

/* Will seek forward, entering and exiting lists until
   this type of list is found. */
/* Returns 0 on success, <0 on various failures. */
EXPORT int
RFRseek_forward_to_list_type(riff_handle *input, uint32_t type);

/* 
   Will search >ONLY< INSIDE THE CURRENT LIST for the requested
   chunk. This function will not enter or exit lists and is therefore
   safer than the previous functions.

   Returns 1 if found, 0 if not found and <0 on various failures. */
EXPORT int
RFRfind_chunk(riff_handle *handle, uint32_t type);

/* Will simply fseek() to the specified position. This can be
   used to utilize an external indexing mechanism.

   Returns 0 on success, <0 on various failures. */
EXPORT int
RFRseek_to_position(riff_handle *input, long file_position);

/******************************************************************/
/* This function will take you at the >exact< specified point
   immediately. Use only if you know what you are doing. Make sure
   you don't cross list boundaries (unless if you are using
   "linear" view).

   NOTE: you should move to a word aligned position for 
   _RFRget_next_info() to work. _RFRraw_seek() does not enforce
   word alignment but most functions expect the file to be
   word aligned and may fail or warn you.
   
*/
EXPORT int
_RFRraw_seek(riff_handle *input, long file_position);

/* Will save current position in a "temporary holder" so that
   we can return to it immediately. The holder variable will only
   keep the last saved position. */

EXPORT int
RFRsave_position(riff_handle *input);


/* Return to the saved position. */
EXPORT int
RFRrestore_position(riff_handle *input);


/* A wrapper for input->file_position. */
EXPORT long
RFRget_position(riff_handle *input);
  
/* Returns to position 0 and clears the list stack */
EXPORT int
RFRrewind(riff_handle *handle);

/**********************************************************/
/* A simple wrapper function that tells us when we have reached
   the end of the file. */
EXPORT int
RFReof(riff_handle *input);

#endif /* _RIFFREAD_H */


