/*
 * Implementation of basic riff reading routines 
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "defines.h"
#include "riffcodes.h"
#include "sanity.h"
#include "riffread.h"

/* NOTE: RFR prefix stands for "riff read" functions. This is 
   here to remind you that you should not use an RFR function
   when writing. (RFW prefix) */ 

/* Open a RIFF file for reading and create the necessary handle. */
/* Accepts a filename, returns a riff_handle *, NULL on failure! */
EXPORT riff_handle *
RFRopen_riff_file(char *filename)
{
  FILE *input;
  riff_handle *result;
  int file_type=0;
  int test;

  PSCHK(filename!=0, ACT_ERROR);
 
  input=fopen(filename, "rb");

  PSCHK(input!=0, ACT_ERROR);

  result=malloc(sizeof(riff_handle));

  PSCHK(result!=0, ACT_FAIL);

  memset(result, 0x0, sizeof(riff_handle));

  result->read_mode=1;

  /* These are not really necessary... */
  result->file_position=0;
  result->input=input;
  /*   result->list_count=0;*/
 
  /* We do not expect filenames to be greater 
     than 1023 characters! */
//  PXCHK(strlen(filename+1)<1024, ACT_WARN);

//  result->filename=malloc(strlen(filename+1));  
  PXCHK((strlen(filename)+1)<1024, ACT_WARN);

  result->filename=malloc(strlen(filename)+1);  //modify intaoism 2012.10.12
  strcpy(result->filename, filename);

  test=fread(&file_type, 4, 1, input);
  PSCHK(test==1, ACT_FAIL);
  
  if (file_type!=opRIFF) {
    fprintf(stderr,"%s:%d: %s: file %s does not seem to be a riff file\n",
	    __FILE__, __LINE__, __FUNCTION__, filename);
    fprintf(stderr,"%s:%d: %s: non-fatal, will try to continue \n",
	    __FILE__, __LINE__, __FUNCTION__);
  }

  test+=fread(&(result->file_size), 4, 1, input);
  PSCHK(test==2, ACT_FAIL);
  
  test+=fread(&(result->file_fcc), 4, 1, input);
  PSCHK(test==3, ACT_FAIL);

  result->file_position=test<<2;

  return(result);
}


/**************************************************************/
/* Close a RIFF file and FREE MEMORY. */
EXPORT int
RFRclose_riff_file(riff_handle *handle)
{
  SCHK(handle!=0, ACT_ERROR);

  free(handle->filename);
  
  fclose(handle->input);

  free(handle);

  /* That's all folks... */

  return(0);

}
