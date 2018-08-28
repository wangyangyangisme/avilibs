/*
 * Header file for basic riff handle structure
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


#ifndef _RIFF_HANDLE
#define _RIFF_HANDLE

#define MAX_LISTS 1024

/* This structure should allow us to both read and write
   riff files. Some portions are not necessary for reading
   only or for writing only. */
typedef struct _riff_handle {

  uint32_t list_start[MAX_LISTS];
  uint32_t list_end[MAX_LISTS];  /* Used when >READING< a file */
  uint32_t list_type[MAX_LISTS];

  uint32_t list_count;

  /********************** IMPORTANT ***********************/
  /* linear_view constantly keeps list_count=0 so that
     the library will not care about the riff file lists.
     This means that you can use raw seeking safely.
     This makes the file look "linear" and not "hierarchical"
     to the READING functions. linear_view will NOT work with
     the writing functions. Set linear_view=1 to switch to 
     linear_view. You may NOT return to hierarchical view
     afterwards. Don't mess with this if you don't know what
     you are doing. */
  int linear_view; 

  /* We use different FILE * so that the
     inappropriate functions will fail if 
     they do not check read_mode first. */
  FILE *output;
  FILE *input;

  char *filename;
  
  /* This is the same as file_size when writing. */
  /* We only use file_position when writing. file_size
     is ignored. */
  long file_position;
  long file_size; /* We use long because this is what glibc ftell() gives us */

  /* Reading/Writing */
  int read_mode; 
  long temporary_position;
  
  /* Skipping JUNK when reading? */
  int skip_junk;

  /* File type as an uint32_t four character code (e.g. 'a','v','i',' ') */
  uint32_t file_fcc;

  uint32_t block_type;
  uint32_t block_size;
  uint32_t block_subtype; /* ONLY if block_type==LIST */

} riff_handle;

#endif
