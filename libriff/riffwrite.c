/*
 * File for basic riff I/O routines 
 *
 * SOME ENDIAN CONVERSIONS ARE DONE HERE..
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

#define _INSIDE_RIFF

#include "riffwrite.h"
#include "sanity.h"
#include <stdlib.h>
#include <string.h>


EXPORT uint32_t 
_RFWs2fcc(char i0, char i1, char i2, char i3)
{
  return(i0+
	 (i1<<8) +
	 (i2<<16)+
	 (i3<<24));

}

EXPORT char *
_RFWfcc2s(uint32_t input)
{
  static char output[8];

  output[0]=input&0xff;
  output[1]=(input>>8)&0xff;
  output[2]=(input>>16)&0xff;
  output[3]=(input>>24)&0xff;
  output[4]=0;

  return(output);
}


/*******************************************************************/
/* Returns number of bytes written, ie length+8 */
/* normally NOT LENGTH*/
EXPORT int
RFWwrite_chunk(riff_handle *handle, uint32_t type, uint32_t length, unsigned char *data)
{
  int result;
  int little_endian_length;

  SCHK((handle->file_position&0x1)==0, ACT_WARN);

  /* Align */
  if (ftell(handle->output)&0x1) {
    fputc(0x0,handle->output);
    handle->file_position++;
  }

  little_endian_length=INT_TO_LE_INT(length);    

  result=fwrite((void *)&type, 4, 1, handle->output)<<2;
  result+=fwrite((void *)&little_endian_length, 4, 1, handle->output)<<2;
  result+=(fwrite((void *)data, length, 1, 
		  handle->output)==1?length:0);

  
#ifdef CHECKS
  if (result!=length+8) {
    fprintf(stderr,"write_chunk() : could not write data. \n");
    fprintf(stderr,"write_chunk() : only wrote %d bytes.\n",result);
  }
#endif
  
  handle->file_position+=result;

  /* Align once more. */
  if (handle->file_position&0x1){
    fputc(0x0,handle->output);
    handle->file_position++;
    result++;
  }

  return(result);

}

/*******************************************************************/
/* Aligns the file by adding a junk chunk of the appropriate size. */
/* This is particularly useful when the file is going to be read */
/* off a cd, where choosing a chunk alignment of 2048 (=sector size) */
/* can be quite beneficial. This is of course a JUNK chunk. */
/* Input is a riff_handle and the desired boundary, output is
   the number of bytes written. */
EXPORT int
RFWalign_riff(riff_handle *handle, int boundary)
{
  int result;
  int chunk_size;
  int little_endian_length;
  int modulo;
  int i;

  SCHK(handle!=0, ACT_FAIL);
  SCHK(boundary>2,ACT_FAIL);
  SCHK(((boundary&0x1)==0), ACT_WARN);

  /* Align */
  if (ftell(handle->output)&0x1) {
    fputc(0x0,handle->output);
    handle->file_position++;
  }

  modulo=handle->file_position%boundary;

  XCHK(handle->file_position==ftell(handle->output), ACT_ERROR);

  chunk_size=modulo-8;
  SCHK(chunk_size<0, ACT_FAIL);
  
  little_endian_length=INT_TO_LE_INT(chunk_size);    

  result=fwrite((void *)&opJUNK, 4, 1, handle->output)<<2;
  result+=fwrite((void *)&little_endian_length, 4, 1, handle->output)<<2;
  
  /* Write some junk... */
  for (i=0; i<(chunk_size>>2); i++)
    result+=(fwrite((void *)&boundary, 4, 1, handle->output))<<2;

  for (i=0; i<(chunk_size&0x3); i++){
    if (fputc(0x0,handle->output)!=EOF)
      result++;
  }

  
#ifdef CHECKS
  if (result!=modulo) {
    fprintf(stderr,"write_chunk() : could not write data. \n");
    fprintf(stderr,"write_chunk() : only wrote %d bytes.\n",result);
  }
#endif
  
  handle->file_position+=result;

  XCHK(handle->file_position==ftell(handle->output), ACT_ERROR);
  SCHK(handle->file_position%boundary==0, ACT_FAIL);

  return(result);

}  


/*******************************************************************/
/* Rewrites a chunk. This function's only difference with */
/* write_chunk() is that it does not increase handle->file_position */
/* (also it does not do any alignment) */
/* Returns number of bytes written, ie length+8 */
/* normally NOT LENGTH*/
EXPORT int
RFWrewrite_chunk(riff_handle *handle, 
	      uint32_t type, 
	      uint32_t length, 
	      unsigned char *data,
	      long file_pos)
{
  int result;
  uint32_t little_endian_length;
  long cur_file_pos;

  /* We won't write unaligned data. */

#ifdef CHECKS
  if (file_pos&0x1){
#ifdef VERBOSE
    fprintf(stderr,"rewrite_chunk(,,,,%ld) : refusing to write on a non-aligned boundary!\n",
	    file_pos);
#endif
    return(-1);
  }
#endif

  cur_file_pos=ftell(handle->output);

#ifdef EXTENDED_CHECKS
  if (cur_file_pos==-1){
    fprintf(stderr,"rewrite_chunk() : cannot obtain current file position!\n");
    return(-2);
  }
#endif
  
  if (fseek(handle->output, file_pos, SEEK_SET)!=0){
#ifdef VERBOSE
    fprintf(stderr,"rewrite_chunk(%s,,,,%ld) : cannot seek to start of header!\n",
	    handle->filename,file_pos);
#endif
    return(-3);
  }

  little_endian_length=INT_TO_LE_INT(length);    

  result=fwrite((void *)&type, 4, 1, handle->output)<<2;
  result+=fwrite((void *)&little_endian_length, 4, 1, handle->output)<<2;
  result+=(fwrite((void *)data, length, 1, 
		  handle->output)==1?length:0);


  SCHK((result==length+8),ACT_ERROR);
  
#ifdef CHECKS
  if (result!=length+8) {
    fprintf(stderr,"rewrite_chunk() : could not write data. \n");
    fprintf(stderr,"rewrite_chunk() : only wrote %d bytes.\n",result);
  }
#endif
  
  /* rewrite_chunk() does not increase file size */
  /* handle->file_position+=result; */ 

  if (fseek(handle->output, cur_file_pos, SEEK_SET)!=0){
#ifdef VERBOSE
    fprintf(stderr,"update_avi_header(%s) : cannot return to previous file position!\n",
	    handle->filename);
#endif   
    return(-2);
  }

  return(result);

}
    
  

/*******************************************************************/
/* Returns 12 on success, number of bytes written (<12) on failure */
EXPORT int
RFWcreate_list(riff_handle *handle, uint32_t type)
{
  uint32_t tmp=0x0;    

#ifdef EXTENDED_CHECKS
  /* Really not very propable */
  if (handle->list_count==1023) {
    fprintf(stderr,"create_list(%s) : cannot handle more than 1024 open lists!\n",
	    handle->filename);
    return(0);
  }  
#endif

  handle->list_start[handle->list_count]=ftell(handle->output);
  
#ifdef CHECKS
  if (handle->list_start[handle->list_count]&0x1) {
    fprintf(stderr,"create_list() : create_list(%s) invoked on a non-aligned boundary.\n",
	    _RFWfcc2s(type));
    fprintf(stderr,
	    "create_list() : close_list()/write_chunk() should have done proper alignment. \n");
    fprintf(stderr,"create_list() : Forcing 16-bit alignment.\n");
    
    fputc(0x0,handle->output);
    handle->file_position++; /* Added a byte! */
    handle->list_start[handle->list_count]++;
  }
#endif

  handle->list_count++;  
  tmp=fwrite((void *)&opLIST, 4, 1, handle->output);
  tmp+=fwrite((void *)&type, 4, 1, handle->output); /* Make room for the list length */ 
  tmp+=fwrite((void *)&type, 4, 1, handle->output);

  tmp<<=2;
  
  handle->file_position+=tmp;

#ifdef CHECKS
  if (tmp!=12) {
    fprintf(stderr,"Could not write 12 bytes of data. Only wrote %d.\n",tmp);
    fprintf(stderr,"Trying to continue. \n");
  }
#endif

  return(tmp);
}


/********************************************************************/
/* 0 on success, -1 on failure */
EXPORT int
RFWclose_list(riff_handle *handle)
{
  long file_position=0;
  long list_size=0;
  uint32_t little_endian_size;
  int result;

#ifdef CHECKS
  if (handle->list_count==0) {
    fprintf(stderr,"close_list() : No open lists. Cannot continue. \n");
    return(-1);
  }
#endif


#ifdef EXTENDED_CHECKS
  file_position=ftell(handle->output);

  if (file_position!=handle->file_position){
    fprintf(stderr,"close_list() : Bug found! File size is not correctly reported. \n");
    fprintf(stderr,"close_list() : handle->file_position=%ld, file_position=%ld\n",
	    handle->file_position, file_position);
    file_position=handle->file_position;
    while (!feof(handle->output)) fgetc(handle->output);
    file_position=ftell(handle->output);
    if (file_position!=handle->file_position) {
      fprintf(stderr,"close_list() : handle->file_position is incorrect. \n");
      handle->file_position=file_position;
    }
  }
#else
  file_position=handle->file_position;
#endif
  
  if (handle->file_position&0x1) { /* File needs alignment */
    fputc((char)0x0,handle->output);
    handle->file_position++;
    file_position++;
  }

  /* compare current file_position with file_position when the list opened */
  list_size=handle->file_position-handle->list_start[handle->list_count-1]-8;
    
  /* LIST size is 4 bytes after the list start */
  if (fseek(handle->output, handle->list_start[handle->list_count-1]+4 , SEEK_SET)!=0){
#ifdef CHECKS
    fprintf(stderr,"close_list() : fseek() to start of list failed! \n");
#endif
    return(-1);
  }

#ifdef VERY_VERBOSE    
  fprintf(stderr,"close_list(%d) : List size is %ld.\n",handle->list_count,
	  list_size);
#endif 

  little_endian_size=INT_TO_LE_INT(list_size);
  result=fwrite(&little_endian_size, 4, 1, handle->output);

  handle->list_count--;

  /* Return to the previous position, plus alignment, if applicable */
  if (fseek(handle->output, handle->file_position, SEEK_SET)!=0) {
#ifdef CHECKS
    fprintf(stderr,"close_list() : fseek() to original position failed!\n");
#endif
    return(-1);
  }
  
  if (result==1)
    return(0);
  
  return(-1);

}

/*******************************************************************/
/* This writes a fake header with whatever information we'd like.
   It is used in conjunction with write_raw_chunk to make any
   sort of chunk.
   
   Returns the number of bytes written, <0 on failures. 
*/

EXPORT int
RFWwrite_chunk_header(riff_handle *handle, uint32_t type, uint32_t length)
{
  int result;
  int little_endian_length;

  /* Align */
  if (ftell(handle->output)&0x1) {
    fputc(0x0,handle->output);
    handle->file_position++;
  }

  little_endian_length=INT_TO_LE_INT(length);    

  result=fwrite((void *)&type, 4, 1, handle->output)<<2;
  result+=fwrite((void *)&little_endian_length, 4, 1, handle->output)<<2;

#ifdef CHECKS
  if (result!=8) {
    fprintf(stderr,"%s:%d: %s : could not write data. \n", 
	    __FILE__, __LINE__, __FUNCTION__);
    fprintf(stderr,"%s:%d: %s : only wrote %d bytes.\n",
	    __FILE__, __LINE__, __FUNCTION__, result);
  }
#endif
    
  
  handle->file_position+=result;

  return(result);

}  

/*******************************************************************/ 
/* Returns number of bytes written. NOTE : length normally      */
/* Does not write "legal" chunks. Only writes raw data.  */
/* NO ALIGNMENT IS DONE. */
EXPORT int
RFWwrite_raw_chunk(riff_handle *handle, uint32_t length, unsigned char *data)
{
  int result;

  /* NO ALIGNMENT !!! */
  
  result=(fwrite((void *)data, length, 1, 
		  handle->output)==1?length:0);

  
#ifdef CHECKS
  if (result!=length) {
    fprintf(stderr,"%s:%d: %s : could not write data. \n", 
	    __FILE__, __LINE__, __FUNCTION__);
    fprintf(stderr,"%s:%d: %s : only wrote %d bytes.\n",
	    __FILE__, __LINE__, __FUNCTION__, result);
  }
#endif
  
  handle->file_position+=result;

  return(result);

}


/***************************************************************/
/* Returns a riff_handle pointer. Also allocates data structures. */
/* This structure is used to keep the FILE * and also keep track */
/* of lists. DOES MEMORY ALLOCATION. */

EXPORT riff_handle *
RFWcreate_riff_handle(char *filename, uint32_t riff_type)
{
  riff_handle *result;
  int bytes;

#ifdef CHECKS
  if (!filename) {
    fprintf(stderr,"create_riff() : filename not given!\n");
    return(0);
  }
#endif

  result=(riff_handle *)malloc(sizeof(riff_handle));
  
#ifdef CHECKS
  if (result==NULL){
    fprintf(stderr, "create_riff() : cannot allocate enough memory!\n");
    return(0);
  }
#endif

  memset(result, 0x0, sizeof(riff_handle));
  result->filename=(char *)malloc(strlen(filename)+1);
  
#ifdef CHECKS
  if (result->filename==0){
    fprintf(stderr, "create_riff() : cannot allocate memory for the filename string!\n");
    return(0);
  }
#endif

  strcpy(result->filename, filename);

  result->output=fopen(filename,"w");

#ifdef CHECKS
  if (result->output==0){
    fprintf(stderr,"create_riff() : cannot create file %s! \n",filename);
    return(0);
  }
#endif

  result->read_mode=0; /* We are not reading. We are writing. */
  result->skip_junk=0; /* We don't care */

  /* Write "RIFF" and reserve 4 bytes for the file size */
  bytes=fwrite((void *)&opRIFF, 4, 2, result->output)<<2; 
  bytes+=fwrite((void *)&riff_type, 4, 1, result->output)<<2;

  result->file_position=bytes;
#ifdef CHECKS
  if (bytes!=12){
    fprintf(stderr,"create_riff() : Only wrote %d bytes to file!\n",bytes);
  }
#endif

  return(result);
  
}



/***********************************************************/
/* Closes the RIFF file. FREES MEMORY.*/
/* Must also update the avi header */
/* Returns -1 on failure, 0 if OK */
EXPORT int
RFWclose_riff(riff_handle *handle)
{
  int result;

  if (fseek(handle->output, 4L, SEEK_SET)!=0){
#ifdef CHECKS
    fprintf(stderr,"close_riff() : Cannot seek to position 4!\n");
#endif
    return(-1);
  }

  /* The "list" size is the real file size */
  result=handle->file_position-8;
  result=fwrite(&(result), 4, 1, handle->output);

#ifdef CHECKS
  if (result!=1){
    fprintf(stderr, "close_riff() : Writing failed! \n");
    fclose(handle->output);
    return(-1);
  } 
#endif

    fclose(handle->output);
    free(handle->filename);
    free(handle); /* FREE the avi_handle memory!!! */

    return(0);
}
