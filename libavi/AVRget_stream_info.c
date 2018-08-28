/*
 * Implementation of the _AVIget_stream_info function that
 * prepars stream information and support.
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
#include "defines.h"

/*************************************************************/
/* This function is VERY important because it initializes 
   STREAM SUPPORT structures inside the avi_handle (have 
   a look at avi_handle.h).

   Input : the avi_handle
   Output : 0 on success, >0 on various failures
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
_AVRget_stream_info(avi_handle *input)
{
  int junk_status;
  int inside_strl;

  SCHK(input!=NULL, ACT_ERROR);
  XCHK(input->riff_file!=NULL, ACT_FAIL);

  /* Temporarily ignore all junk chunks (libriff ability) */
  junk_status=input->riff_file->skip_junk;
  input->riff_file->skip_junk=1;

  
  /* If we are at a JUNK chunk, skip it. 
     Normally this should NOT happen, however, since JUNK chunks should
     be ignored, we try to skip them. strl is expected afterwards. */
  if (input->riff_file->block_type==opJUNK)
    RFRskip_chunk(input->riff_file);

  /* Make some rather verbose warnings for the end user... */
  if (input->riff_file->block_type!=opLIST) {
#ifdef VERBOSE
    fprintf(stderr,"%s:%d %s(): FAILURE! Expecting the strl list, found chunk of type %s instead.\n",
	    __FILE__, __LINE__, __FUNCTION__, _RFWfcc2s(input->riff_file->block_type));
    fprintf(stderr,"Is the file corrupt?\n");
#endif
    return(-2);
  }    
	    
  
  if (input->riff_file->block_subtype!=opstrl) {
#ifdef VERBOSE
        fprintf(stderr,"%s:%d %s(): FAILURE! Expecting the strl list, found list of type %s instead.\n",
	    __FILE__, __LINE__, __FUNCTION__, _RFWfcc2s(input->riff_file->block_subtype));
    fprintf(stderr,"Is the file corrupt?\n");
#endif
    return(-3);
  }

  /* Now we know that the environment is sane. We are at the beggining of
     an strl list. */

  /* Loop until we find the movi list. */
  while (1) {

    /* This is the movi list. We have to stop here. */
    if (input->riff_file->block_type==opLIST && input->riff_file->block_subtype==opmovi)
      break; 

    /* This is a list end block, from a list we did not recognize. Ignore it. */
    if (input->riff_file->block_type==opLIST && input->riff_file->block_subtype==opLIST) {
      RFRskip_list(input->riff_file);
      continue;
    }

    /* This is a list we do not recognize. Ignore it. */
    if (input->riff_file->block_type==opLIST && input->riff_file->block_subtype!=opstrl) {
#ifdef VERBOSE
      fprintf(stderr,"%s:%d %s(): Unknown list of type %s inside hdrl (header list), skipping it... \n",
	      __FILE__, __LINE__, __FUNCTION__,_RFWfcc2s(input->riff_file->block_subtype));
      if (input->riff_file->block_subtype==_RFWs2fcc('o','d','m','l')) {
	fprintf(stderr,"%s:%d %s(): This appears to be an OpenDML AVI file. It should be readable.\n",
		__FILE__, __LINE__, __FUNCTION__);
      }
#endif
      RFRskip_list(input->riff_file);
      continue;
    }
						 

    if (RFRenter_list(input->riff_file)<0) {
      fprintf(stderr,"%s:%d %s(): libriff failed... Could not RFRenter_list().\n",
	      __FILE__, __LINE__, __FUNCTION__);
      return(-4);
    } else {

      inside_strl=1;

      /* Loop inside the strl list to get strh, strf, strd (?), and strn */
      while (inside_strl) {
	
	/* We expect total_streams to be between 0 and 100 (or MAX_STREAMS-1, whichever
	 is smaller */
	SCHK(input->total_streams>=0, ACT_FAIL);
	SCHK(input->total_streams<MAX_STREAMS, ACT_FAIL); 
	
	if (input->riff_file->block_type==opLIST) {
	  fprintf(stderr,"--> Found list of type %s.\n", 
		  _RFWfcc2s(input->riff_file->block_subtype));
	  RFRenter_list(input->riff_file);
	}
	
	/* Is this the stream header ? */
	if (input->riff_file->block_type==opstrh) {
	  SCHK(input->riff_file->block_size==sizeof(AVIStreamHeader), ACT_FAIL);
	  /*  --> MALLOC <-- */
	  input->stream_header[input->total_streams]=(AVIStreamHeader *)malloc(sizeof(AVIStreamHeader));
	  SCHK(input->stream_header[input->total_streams]!=NULL, ACT_FAIL);     
	  
	  /* Grab the strh structure... */
	  (void)RFRget_chunk(input->riff_file, (unsigned char *)input->stream_header[input->total_streams]);
	} else {
	  fprintf(stderr,"%s:%d %s(): WARNING! We expected the strh chunk. Got %s instead.\n",
		  __FILE__, __LINE__, __FUNCTION__, _RFWfcc2s(input->riff_file->block_type));
	  fprintf(stderr,"Corrupt avi file? Trying to continue...\n");
	}

	/* Is this the stream format? */
	if (input->riff_file->block_type==opstrf){
	  input->stream_format_size[input->total_streams]=input->riff_file->block_size;

	  /*  --> MALLOC <-- */
	  input->stream_format[input->total_streams]=malloc(input->riff_file->block_size);
	  SCHK(input->stream_format[input->total_streams]!=NULL, ACT_FAIL);
	  
	  /* Grab the strf structure... */
	  (void)RFRget_chunk(input->riff_file, (unsigned char *)input->stream_format[input->total_streams]);
	} else {
	  fprintf(stderr,"%s:%d %s(): WARNING! We expected the strf chunk. Got %s instead.\n",
		  __FILE__, __LINE__, __FUNCTION__, _RFWfcc2s(input->riff_file->block_type));
	  fprintf(stderr,"Corrupt avi file? Trying to continue...\n");
	}

	/* Does an strd chunk exist? (not required... ) */
	if (input->riff_file->block_type==opstrd) {
	  input->stream_strd_size[input->total_streams]=input->riff_file->block_size;

	  /* --> MALLOC <-- */
	  input->stream_strd[input->total_streams]=malloc(input->riff_file->block_size);
	  SCHK(input->stream_strd[input->total_streams]!=NULL, ACT_FAIL);

	  (void)RFRget_chunk(input->riff_file,(unsigned char *)input->stream_format[input->total_streams]);
	}
	 
	/* Does an strn chunk exist? (not required...) */
	/* We treat this like a text string. (it is in fact null terminated) */
	if (input->riff_file->block_type==opstrn) {
	  /* allocate n for text + null termination */
	  input->stream_name[input->total_streams]=(char *)malloc(input->riff_file->block_size);
	  SCHK(input->stream_name!=NULL, ACT_FAIL);

	  (void)RFRget_chunk(input->riff_file, (unsigned char *)input->stream_name[input->total_streams]);
	}	
      
	/* This is either a new list (=unexpected!) or the list_end block. We stop
	   here */
	if (input->riff_file->block_type==opLIST) {
	  if (input->riff_file->block_subtype==opLIST) {
	    
	    /* This is where the description of the stream ends. */	  

	    /* Skip the list end block.. */
	    RFRenter_list(input->riff_file);       
	    /* Stop looping inside strl. */
	    inside_strl=0;

	    /* Ensure the stream description is complete... (FAIL) */
	    SCHK(input->stream_header[input->total_streams]!=NULL, ACT_FAIL);
	    SCHK(input->stream_format[input->total_streams]!=NULL, ACT_FAIL);
	    
	    /* Give an arbitrary name... Nifty feature... :-) */
	    if (input->stream_name[input->total_streams]==NULL){	      
	      input->stream_name[input->total_streams]=(char *)malloc(128);
	      XCHK(input->stream_name[input->total_streams]!=NULL, ACT_FAIL);
	      if (input->stream_header[input->total_streams]->fccType==opvids){
		sprintf(input->stream_name[input->total_streams],"Stream %d (video, %s)",
			input->total_streams, 
			_RFWfcc2s(input->stream_header[input->total_streams]->fccHandler));
		input->vids_streams++;
	      }
	      else if (input->stream_header[input->total_streams]->fccType==opauds) {
		sprintf(input->stream_name[input->total_streams],"Stream %d (audio, %s)",
			input->total_streams,
			input->stream_header[input->total_streams]->fccHandler==0?"----":
			_RFWfcc2s(input->stream_header[input->total_streams]->fccHandler));
	
		input->auds_streams++;
	      }
	      else if (input->stream_header[input->total_streams]->fccType==optxts) {
		sprintf(input->stream_name[input->total_streams],"Stream %d (text)",
			input->total_streams);
		input->txts_streams++;
	      } else 
		sprintf(input->stream_name[input->total_streams],"Stream %d (UNKNOWN)",
			input->total_streams);
	    }
	     
	    /* Create file handle. CAUTION: Some operating systems have a hard limit on the
	       number of file descriptors that can be open. Linux allows 1024, which is
	       plenty. Normally, most avi files contain 2-3 streams, so opening 3 more
	       files would not be a problem even for MS-DOS 3.3. */

	    /*	    input->stream_file[input->total_streams]=fopen(input->riff_file->filename,"rb");  */

	    /* NOTE: switch to linear view if we get an index! */
	    input->stream_file[input->total_streams]=RFRopen_riff_file(input->riff_file->filename);
	    SCHK(input->stream_file[input->total_streams]!=NULL, ACT_ERROR); 

	    
	    /* Stream time is 0. */
	    input->stream_time[input->total_streams]=0.0;
	   
	    /* Position in idx is 0 */
	    input->stream_idxp[input->total_streams]=0;

	    /* Increase total number of streams. */
	    input->total_streams++;

#ifdef VERY_VERBOSE
	    AVRprint_stream_info(input, stderr, input->total_streams-1);
#endif

	  } else {
#ifdef VERBOSE
	  fprintf(stderr,"%s:%d %s(): Found an unexpected list of type %s inside the strl list. \n",
		  __FILE__, __LINE__, __FUNCTION__, _RFWfcc2s(input->riff_file->block_subtype));
	  fprintf(stderr,"Corrupt avi header? I will try to ignore this list and skip it. \n");
#endif
	  }
	} else RFRskip_chunk(input->riff_file); /* Ends stream finalization */

      } /* Ends loop inside strl */

    } 

  } /* Ends loop inside hdrl */


  /* Did we get them all (according to the main header) */
  if (input->total_streams!=input->avi_header.ulStreams) {
#ifdef VERBOSE
    fprintf(stderr,"%s:%d %s(): Expected %u streams, got %u. Corrupt AVI header?\n",
	    __FILE__, __LINE__, __FUNCTION__, input->avi_header.ulStreams, 
	    input->total_streams);
#endif
  }

  /* Restore JUNK chunk behaviour */
  input->riff_file->skip_junk=junk_status;

  return(0);
}
	
