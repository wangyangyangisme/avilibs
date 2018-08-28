/*
 * Implementation of the functions that parse the avi header
 * structure.
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

#include <string.h>
#include <stdlib.h>

#include "avicodes.h"
#include "avi_structures.h"
#include "riffread.h"
#include "sanity.h"
#include "aviread.h"
#include "avi_handle.h"
#include "defines.h"


/* Some useful functions, similar to the ones in fourcc.h */

/* Build the two-character code for the stream N */
/* NOTE: from number N (with two digits d1d2) it
   builds 'd1','d2',0,0  NOT 'd1','d2','0','0'.
   You can then "OR" (|) this with 0,0,'d','c' etc
   to get the fcc. (again, NOT with '0','0','d','c') */   
uint32_t 
_AVRn2fcc(int num)
{
  /* Note: I have not actually used this one. It may be
     seriously wrong. */
  return(((num/10+'0'))|((num%10+'0')<<8));
}

/* Get the number from a four character code */
int
_AVRfcc2n(uint32_t fourcc)
{
  int d1, d2;

  d1=((fourcc&0xff)-(int)'0')*10;
  d2=((fourcc>>8)&0xff)-(int)'0';
  
  return(d1+d2);
}


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
AVRopen_avi_file(char *filename)
{
  int retv;
  int i;
  avi_handle *avi_file;

  /* We use PSCHK and PXCHK in functions returning a pointer */
  PSCHK(filename!=NULL, ACT_ERROR);
  if (filename==NULL) return(NULL);

  avi_file=_AVRcreate_handle(filename);  
  /* Everything has been set to zero by _AVRcreate_handle() */

  PSCHK(avi_file!=NULL, ACT_ERROR);
  if (avi_file==NULL) return(NULL);  /* We need the if so that lint won't complain. */

  /* Get the MAIN avi header */
  retv=_AVRget_avi_header(avi_file);

  if (retv!=0) {
#ifdef VERBOSE
    fprintf(stderr,"%s:%d %s(): Corrupt avi header? _AVRget_avi_header() failed!\n",
	    __FILE__,__LINE__,__FUNCTION__);
#endif
    /* Deallocate data, we failed. */
    (void)RFRclose_riff_file(avi_file->riff_file);
    free(avi_file);
    return(0);
  }
  
  /* Get all stream headers */
  retv=_AVRget_stream_info(avi_file);

  if (retv!=0) {
#ifdef VERBOSE
    fprintf(stderr,"%s:%d %s(): Corrupt avi header? Could not get stream info!\n",
	    __FILE__,__LINE__,__FUNCTION__);
#endif
    /* Deallocate data, we failed. */
    (void)RFRclose_riff_file(avi_file->riff_file);
    free(avi_file);
    return(0);
  }

  /* Get the idx1 chunk, if it exists */  
  if ((avi_file->avi_header.ulFlags & AVIF_HASINDEX)!=0){
    retv=_AVRget_idx(avi_file);

    /* Do we NEED an index chunk? */
    if ((avi_file->avi_header.ulFlags & AVIF_MUSTUSEINDEX)!=0) {
      /* Did we actually get it? */
      if (retv!=0) {
#ifdef VERBOSE
	fprintf(stderr,"%s:%d %s(): FAILURE! The header says we MUST use the index, but no index is found.\n",
		__FILE__, __LINE__, __FUNCTION__);
	fprintf(stderr,"Maybe you should build an index?\n");
#endif
	/* HERE WE BUILD THE INDEX */
      }
    }

    /* If we have an idx chunk, switch stream support to 
       linear view so that we can work faster */
    if (retv==0) 
      for (i=0; i<avi_file->total_streams; i++)
	avi_file->stream_file[i]->linear_view=1;

#ifdef VERY_VERBOSE
    fprintf(stderr, "%s:%d %s(): Linear view for stream support has been enabled.\n",
	    __FILE__, __LINE__, __FUNCTION__);
#endif
  }

  _AVRinit_stream_position(avi_file);

  /* Everything is now complete? */
    
  return(avi_file);
  
}


/*******************************************************************/
/* Initialize every stream to the appropriate position. 
   This works as a global "rewind" for all streams. 
   You should call it if you build an index from scratch with build_idx(). 
*/
int
_AVRinit_stream_position(avi_handle *input)
{

  register int i;
  int retv;
  int stream_code;

  SCHK(input!=NULL, ACT_ERROR);

  /* Now we will move every stream pointer inside the movi list */
  for (i=0; i<input->total_streams; i++){      
    /* Move to the beggining (it should already be there! */
    RFRrewind(input->stream_file[i]);
    retv=RFRseek_forward_to_list_type(input->stream_file[i], opmovi);
    if (retv==0){
#ifdef VERBOSE
      fprintf(stderr, "%s:%d %s(): Could not find a movi list in file %s. Is it really an avi file?\n",
	      __FILE__, __LINE__, __FUNCTION__, input->stream_file[i]->filename);
#endif
      exit(EXIT_FAILURE);
      }
    
#ifdef VERBOSE
    fprintf(stderr,"%s:%d %s(): Stream %d positioned at block type %s, size %d, file pos. %lu.\n",
	    __FILE__, __LINE__, __FUNCTION__, i, _RFWfcc2s(input->stream_file[i]->block_type),
	    input->stream_file[i]->block_size, input->stream_file[i]->file_position);
#endif

    /* Enter the movi list */
    RFRenter_list(input->stream_file[i]);

  }

  /* Now we will move every stream pointer to the first frame
     inside the movi list */
  for (i=0; i<input->total_streams; i++){
    stream_code=_AVRn2fcc(i);
    
    /* Is this the first frame? */
    if (((input->stream_file[i]->block_type&0xffff)==
	 stream_code) && (input->stream_file[i]->block_type!=opLIST))
      continue;
    /* If not, then find the first frame of that stream */
    AVRnext_frame(input, i);
  }
  
  if (input->idx_array)
    /* Now we need to move every index pointer to the first
       frame of the corresponding stream. We know that the
       actual file is positioned correctly so we use that
       as reference. */
    for (i=0; i<input->total_streams; i++){
      input->stream_idxp[i]=0;

      /* Compare the real position with the index */
      while (input->stream_file[i]->file_position !=
	     input->movi_offset + 4 + 
	     input->idx_array[input->stream_idxp[i]].lChunkOffset){
	/* Advance the index pointer until we are there */
	input->stream_idxp[i]++;
	/* This should not normally happen. Either the index is
	   corrupt or no frames exist for this stream. The function
	   will not fail. */       
	if (i==input->stream_idxp[i]==input->idx_array_size){
	  fprintf(stderr,
		  "%s:%d %s(): Could not find the index position for the first frame of stream %d.\n",
		  __FILE__, __LINE__, __FUNCTION__, i);
	  fprintf(stderr,"\tFile position is %ld. I will rewind the index.\n",
		  input->stream_file[i]->file_position);
	  /* Rewind the index. We did not find any frames! */
	  input->stream_idxp[i]=0;
	  break;
	}
      }
    }
      
  return(0);
      
}
  

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
_AVRcreate_handle(char *filename)
{
  riff_handle *riff_file;
  avi_handle *avi_file;
  
  PSCHK(filename!=NULL, ACT_ERROR);

  /* Open as riff file. */
  riff_file=RFRopen_riff_file(filename);
  PSCHK(riff_file!=NULL, ACT_ERROR);

  avi_file=(avi_handle *)malloc(sizeof(avi_handle));
  PSCHK(avi_file!=NULL, ACT_FAIL);

  /* Make sure everything is set to zero */
  memset((void *)avi_file, 0, sizeof(avi_handle));

  avi_file->riff_file=riff_file;

  /* Initialize the data structures in avi_handle */
  avi_file->read_mode=1; /* Only for reading. */

  return(avi_file);

}  

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
_AVRget_avi_header(avi_handle *input)
{
  uint32_t block_type;
  uint32_t block_size;
  uint32_t list_type;
  int retv;

  SCHK(input!=NULL, ACT_ERROR);
  SCHK(input->riff_file!=NULL, ACT_FAIL);
  
  RFRskip_chunk(input->riff_file);
  (void)RFRget_block_info(input->riff_file, &block_type, &block_size, &list_type);

  /* NOTE: because _RFWfcc2s() overwrites the same data space you CANNOT use it
     twice in fprintf() because the second time it gets called it will overwrite 
     the first. */
#ifdef VERY_VERBOSE
  fprintf(stderr, "Get avi header at block %s, size:%d, subtype:",
	  _RFWfcc2s(block_type),
	  block_size);
  fprintf(stderr,"%s\n",
	  _RFWfcc2s(list_type));
#endif

  SCHK(block_type==opLIST, ACT_FAIL);
  SCHK(list_type==ophdrl, ACT_FAIL);
  
  RFRenter_list(input->riff_file);

  (void)RFRget_block_info(input->riff_file, &block_type, &block_size, &list_type);
  
  if (block_type!=opavih) {
    fprintf(stderr, "AVRopen_avi_file.c: _AVRget_avi_header(): FAILURE! Expected the avi header.\n");
    fprintf(stderr, "\tGot a chunk of type %s instead. \n", _RFWfcc2s(block_type));
    exit(EXIT_FAILURE);
  }
  
  retv=RFRget_chunk(input->riff_file, (unsigned char *)&input->avi_header);
  
  XCHK(retv==sizeof(MainAVIHeader), ACT_WARN);
  
  return(0);

}

      
  
  

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

   Prerequisites : The file pointer MUST BE at the beggining
                   of the movi list, so that we can skip this
		   and find the idx1 chunk. 
*/
int
_AVRget_idx(avi_handle *input)
{
  long file_position;
  unsigned int oblock_type;
  unsigned int oblock_size;
  unsigned int oblock_subtype;
  int retv;
  
  SCHK(input!=NULL, ACT_ERROR);
  SCHK(input->riff_file!=NULL, ACT_FAIL);

  if (input->riff_file->block_type!=opLIST) {
    fprintf(stderr, 
	    "AVRopen_avi_file.c: _AVRget_idx(): FAILURE! The file is not positioned at the");
    fprintf(stderr,"\t beggining of a list. Corrupt avi header (?)\n");
    exit(EXIT_FAILURE);
  }

  if (input->riff_file->block_subtype!=opmovi) {
    fprintf(stderr, "%s:%d %s(): FAILURE! The current list is not a movi list.\n", 
	    __FILE__, __LINE__, __FUNCTION__);
    fprintf(stderr, "%s:%d %s(): Got type %s instead. \n",
	    __FILE__, __LINE__, __FUNCTION__, _RFWfcc2s(input->riff_file->block_subtype));
    exit(EXIT_FAILURE);
  }
  

  /* Preserve current block information to ensure
     that we get back to the same position (start of movi
     list) */
  oblock_type=input->riff_file->block_type;
  oblock_size=input->riff_file->block_size;
  oblock_subtype=input->riff_file->block_subtype;

  file_position=input->riff_file->file_position;

  /* Save the position of the movi list for use with the
     offsets in the index chunk. */
  input->movi_offset=input->riff_file->file_position;
  
  retv=RFRskip_list(input->riff_file);
  
  if (retv<0){
    fprintf(stderr,"%s:%d:%s(): Failed to skip the movi list. Truncated file?\n",
	    __FILE__, __LINE__, __FUNCTION__);
    return(-1);
  } 

  /* Skip any junk chunks that may be there for alignment. Warn if we encounter
   unknown lists.*/
  while (!RFReof(input->riff_file)){
    if (input->riff_file->block_type==opLIST)       
      fprintf(stderr,"%s:%d %s(): WARNING! Expected the idx chunk. Got list %s instead.\n",
	      __FILE__, __LINE__, __FUNCTION__, _RFWfcc2s(input->riff_file->block_subtype));    
    else { 
      /* Success */
      if (input->riff_file->block_type==opidx1) break;
      
      /* Unexpected chunk! */
      if (input->riff_file->block_type!=opJUNK) 
	fprintf(stderr,"%s:%d %s(): WARNING! Expected the idx chunk. Got list %s instead.\n",
		__FILE__, __LINE__, __FUNCTION__, _RFWfcc2s(input->riff_file->block_subtype));      
    }
  }

  /* If we did not find the idx chunk by now, the function returns
     an error. We do not fail, because the file may still be playable. */
  if(input->riff_file->block_type!=opidx1) {
#ifdef VERBOSE
    fprintf(stderr,"%s:%d %s(): WARNING! Did not find the idx chunk. Trying to continue...\n",
	    __FILE__, __LINE__, __FUNCTION__);
#endif
    return(-1);
  }

  /*  --> MALLOC <-- */
  input->idx_array=malloc(input->riff_file->block_size+32);
  
  SCHK(input!=0, ACT_FAIL);
  
  (void)RFRget_chunk(input->riff_file, (unsigned char *)input->idx_array);

  input->idx_array_size=input->riff_file->block_size/sizeof(AVIINDEXENTRY);
  
  XCHK(input->riff_file->block_size%sizeof(AVIINDEXENTRY)==0, ACT_WARN);
  
  if (RFRseek_to_position(input->riff_file, file_position)!=0){
    fprintf(stderr,"%s:%d %s: Seeking failed. \n",
	    __FILE__, __LINE__, __FUNCTION__);
  }
  
  SCHK(input->riff_file->block_type==oblock_type, ACT_FAIL);
  SCHK(input->riff_file->block_size==oblock_size, ACT_FAIL);
  SCHK(input->riff_file->block_subtype==oblock_subtype, ACT_FAIL);

#ifdef VERBOSE
  fprintf(stderr,"%s:%d %s(): Got the idx chunk. Size = %d entries.\n",
	  __FILE__, __LINE__, __FUNCTION__, input->idx_array_size);
#endif

  return(0);

}


/*************************************************************/
/* This function frees the memory we allocated an properly closes
   all the file streams. Take care not to introduce memory leaks.
   
   Input: the avi_handle
   Output: 0 on success, <0 on failure
   Side effects: the avi_handle structure gets freed, it will
                 no longer be accessible, ALL files are closed.

*/
int
AVRclose_avi_file(avi_handle *input)
{
  register int i;

  SCHK(input!=NULL, ACT_ERROR);
  
  /* Free stream support structures... */
  for (i=0; i<input->total_streams; i++){
    if (input->stream_header[i]!=NULL)
      free(input->stream_header[i]);
    if (input->stream_format[i]!=NULL) 
      free(input->stream_format[i]);
    input->stream_format_size[i]=0;
    if (input->stream_strd[i]!=NULL) 
      free(input->stream_strd[i]);
    input->stream_strd_size[i]=0;
    if (input->stream_name[i]!=NULL) 
      free(input->stream_name[i]);

    if (input->stream_file[i]!=NULL){
      (void)RFRclose_riff_file(input->stream_file[i]);
      input->stream_file[i]=NULL;
    }
    
  }

  /* Free idx chunk */
  if (input->idx_array!=NULL)
    free(input->idx_array);
    
  /* Free linked ilst, if it exists (from AVRbuild_idx())  */
  /* CODE NOT YET WRITTEN!!!!!!!! WARNING!! */

  /* Close the riff file */
  i=RFRclose_riff_file(input->riff_file);

  /* Free the avi handle */
  free(input);

  if (i!=0) {
#ifdef VERBOSE
    fprintf(stderr,"%s:%d %s(): Failed to close riff file!?! A slight memory leak may exist. \n",
	    __FILE__, __LINE__, __FUNCTION__);
#endif
    return(-1);
  }

  return(0);

}


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
AVRprint_stream_info(avi_handle *input, FILE *where, unsigned int stream)
{

  /* We ignore this and print to stderr */
  if (where==NULL) where=stderr;

  if (stream>input->total_streams) {
    fprintf(where,"Requested stream %u. Valid range is 0-%u.\n",
	    stream, input->total_streams);
    return(-1);
  }
  
  if (stream<0) {
    fprintf(where,"Requested stream %u. Range is 0-%u.\n",
	    stream, input->total_streams);
    return(-2);
  }

  SCHK(input!=NULL, ACT_ERROR);

  fprintf(where,"__________ Stream %d description _________ \n", stream);
  
  if (input->stream_name[stream]!=NULL)
    fprintf(where,"Stream Name  : %s\n", input->stream_name[stream]);

  if (input->stream_file[stream]!=NULL){
    fprintf(where,"File pointer : %p\n", (void *)input->stream_file[stream]);
    fprintf(where,"File position: %ld\n", input->stream_file[stream]->file_position);
  } else 
    fprintf(where,"File pointer :  NULL! (serious error!)n");

  if (input->stream_header[stream]!=NULL) {
    fprintf(where,"Stream header follows : \n");
    /* Print the structure */
    fprintf(where,"\tfccType   : %s\n", _RFWfcc2s(input->stream_header[stream]->fccType));
    fprintf(where,"\tfccHandler: %s\n", _RFWfcc2s(input->stream_header[stream]->fccHandler));

    /* Type the flags in human readable form... */
    fprintf(where,"\tFlags     : %x ", input->stream_header[stream]->ulFlags);
    if (input->stream_header[stream]->ulFlags&AVISF_DISABLED)
      fprintf(where,"(AVISF_DISABLED)");
    if (input->stream_header[stream]->ulFlags&AVISF_VIDEO_PALCHANGES)
      fprintf(where,"(AVISF_VIDEO_PALCHANGES)");
    fprintf(where,"\n");

    fprintf(where,"\tPriority  : %u\n", input->stream_header[stream]->ulPriority);
    fprintf(where,"\tInitialFrames : %u\n", input->stream_header[stream]->ulInitialFrames);
    fprintf(where,"\tScale     : %u\n", input->stream_header[stream]->ulScale);
    fprintf(where,"\tRate      : %u\n", input->stream_header[stream]->ulRate);
    fprintf(where,"\tLength    : %u\n", input->stream_header[stream]->ulLength);
    fprintf(where,"\tBufferSize: %u\n", input->stream_header[stream]->ulSuggestedBufferSize);
    if (input->stream_header[stream]->ulQuality==-1)
      fprintf(where,"\tQuality   : default (-1)\n");
    else
      fprintf(where,"\tQuality   : %d\n", input->stream_header[stream]->ulQuality);
    
    if (input->stream_header[stream]->ulSampleSize==0)
      fprintf(where,"\tSampleSize: 0 (VBR)\n");
    else
      fprintf(where,"\tSampleSize: %u\n", input->stream_header[stream]->ulSampleSize);
    fprintf(where,"\tRight corner (x,y) : %d, %d\n", 
	    input->stream_header[stream]->ulrcFrame[0],
	    input->stream_header[stream]->ulrcFrame[1]);	    
  }

  /* Add code to display the stream format... */

  fprintf(where,"Stream format header (%s: size %d bytes) follows : \n",
	  input->stream_header[stream]->fccType==opvids?"BTIMAPINFO":"WAVEFORMATEX",
	  input->stream_format_size[stream]);

#define GET_WVFX(x) (((WAVEFORMATEX *)input->stream_format[stream])->x)
#define GET_BMI(x)  (((BITMAPINFO *)input->stream_format[stream])->x)
		      
  if (input->stream_header[stream]->fccType==opauds){
    fprintf(where,"\twFormatTag     : %d\n", GET_WVFX(wFormatTag));
    fprintf(where,"\twChannels      : %d\n", GET_WVFX(wChannels));
    fprintf(where,"\tlSamplesPerSec : %d\n", GET_WVFX(lSamplesPerSec));
    fprintf(where,"\tlAvgBytesPerSec: %d\n", GET_WVFX(lAvgBytesPerSec));
    fprintf(where,"\twBlockAlign    : %d\n", GET_WVFX(wBlockAlign));
    fprintf(where,"\twBitsPerSample : %d\n", GET_WVFX(wBitsPerSample));
    
    /* It appears that wSize is equal to the extra bytes we have in our
       disposal (including wSize, which is 2 bytes). Therefore, if 
       we have 0 real extra bytes, wSize is 2. I'm not sure about that,
       but the difference between wSize and real-sizeof(WAVEFORMATEX) is
       always 2. */
    fprintf(where,"\twSize          : %d (real is %d, WAVEFORMATEX is %d)\n",
	    GET_WVFX(wSize), input->stream_format_size[stream], sizeof(WAVEFORMATEX));
    
  }

#undef GET_WVFX
#undef GET_BMI

  if (input->stream_strd[stream]!=NULL) 
    fprintf(where, "The stream has extra data in a strd chunk of size %d.\n",
	    input->stream_strd_size[stream]);
  
  fprintf(where,"---> Current stream position is : %f sec. \n", input->stream_time[stream]); 

  return(0);

}


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
AVRprint_header_info(avi_handle *input, FILE *where)
{

  /* We ignore this and print to stderr */
  if (where==NULL) where=stderr;

  SCHK(input!=NULL, ACT_ERROR);

  fprintf(where,"______ AVI Header and Handle information ______\n");

  fprintf(where,"MainAVIHeader : \n");

#define REPORT(name, what) fprintf(where,"\t%s : %u\n", name, what)

  REPORT("MicroSecPerFrame   ",input->avi_header.ulMicroSecPerFrame);
  REPORT("MaxBytesPerSec     ",input->avi_header.ulMaxBytesPerSec);
  REPORT("PaddingGranularity ",input->avi_header.ulPaddingGranularity);
  REPORT("Flags              ",input->avi_header.ulFlags);
  REPORT("TotalFrames        ",input->avi_header.ulTotalFrames);
  REPORT("InitialFrames      ",input->avi_header.ulInitialFrames);
  REPORT("Streams            ",input->avi_header.ulStreams);
  REPORT("SuggestedBufferSize",input->avi_header.ulSuggestedBufferSize);
  REPORT("Width              ",input->avi_header.ulWidth);
  REPORT("Height             ",input->avi_header.ulHeight);
  REPORT("Reserved[0]        ",input->avi_header.ulReserved[0]);
  REPORT("Reserved[1]        ",input->avi_header.ulReserved[1]);
  REPORT("Reserved[2]        ",input->avi_header.ulReserved[2]);
  REPORT("Reserved[3]        ",input->avi_header.ulReserved[3]);

#undef REPORT

  fprintf(where, "--\n");
  fprintf(where, "Frames per second : %f\n", (double)1000000.0/input->avi_header.ulMicroSecPerFrame);
  fprintf(where, "Streams found     : %d (%d video, %d audio, %d other)\n", input->total_streams,
	  input->vids_streams, input->auds_streams, input->txts_streams);
  fprintf(where, "-- Flags\n");

  if ((input->avi_header.ulFlags & AVIF_HASINDEX) !=0)
    fprintf(where, "HAS INDEX (at %p, %u entries)\n", 
	    (void *)input->idx_array,
	    input->idx_array_size);
  if ((input->avi_header.ulFlags & AVIF_MUSTUSEINDEX) !=0)
    fprintf(where, "MUST USE INDEX\n");
  if ((input->avi_header.ulFlags & AVIF_ISINTERLEAVED) !=0)
    fprintf(where, "IS INTERLEAVED\n");
  if ((input->avi_header.ulFlags & AVIF_TRUSTCKTYPE) !=0)
    fprintf(where, "TRUST CK TYPE (the index can be trusted)\n");
  if ((input->avi_header.ulFlags & AVIF_WASCAPTUREFILE) !=0)
    fprintf(where, "WAS CAPTURE FILE\n");
  if ((input->avi_header.ulFlags & AVIF_COPYRIGHTED) !=0)
    fprintf(where, "IS COPYRIGHTED\n");

  
  return(0);
}
