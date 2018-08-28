
#define _INSIDE_AVIWRITE

#include <stdio.h>
#include <time.h>
#include "riffread.h"
#include "avicodes.h"



int
main(int argc, char *argv[])
{
  int iters;
  int i,j;
  riff_handle *handle;
  riff_handle *output;
  unsigned char *big_buffer;

  uint32_t btype;
  uint32_t bsize;
  uint32_t btype2;
  
  int strhcount;

  long seek_targets[1024];
  int seekt;

  clock_t start, end;

  for (i=0; i<argc; i++)
    fprintf(stderr,"argument %d : %s \n",i, argv[i]);

  iters=atoi(argv[2]);
  handle=RFRopen_riff_file(argv[1]);

  big_buffer=malloc(1048576);

  /* Does this work? It seems so... */
  handle->linear_view=1;

  fprintf(stderr,"Testing RFRget_block_info() and RFRskip_chunk()\n");

  for (i=0; i<iters; i++){
    RFRget_block_info(handle, &btype, &bsize, &btype2);

    if (btype!=opLIST) {
      fprintf(stderr,"\t [%s]: ", _RFWfcc2s(btype2));
      fprintf(stderr,"Block %d is type %s, size %d \n", i, _RFWfcc2s(btype), bsize);
    } else {
      fprintf(stderr,"Block %d is type %s, size %d,", i, _RFWfcc2s(btype), bsize);
      fprintf(stderr," container list_type is [%s]\n", _RFWfcc2s(btype2));
    }
    
    if (btype==opLIST) {
      if (btype2==opLIST)
	fprintf(stderr,"\tList of type %s closes.\n",_RFWfcc2s(bsize));
      else
	fprintf(stderr,"\tEntering list of type is %s.\n", _RFWfcc2s(btype2));
      RFRenter_list(handle);
    } else RFRskip_chunk(handle);
    if ((i&1023)==0) fprintf(stderr,"1024 blocks read... \n");
    if (RFReof(handle)==1) break;
  }

  fprintf(stderr,"----------\n");

  fprintf(stderr,"RFRrewind() : ");
  RFRrewind(handle);
  fprintf(stderr,"OK\n");

  fprintf(stderr,"RFRskip_junk(1) : ");
  RFRskip_junk(handle,1);
  fprintf(stderr,"OK\n");

  fprintf(stderr,"Testing RFRget_block_info() and RFRskip_chunk() without JUNK\n");

  for (i=0; i<iters; i++){
    RFRget_block_info(handle, &btype, &bsize, &btype2);

    if (btype!=opLIST) {
      fprintf(stderr,"\t [%s]: ", _RFWfcc2s(btype2));
      fprintf(stderr,"Block %d is type %s, size %d \n", i, _RFWfcc2s(btype), bsize);
    } else {
      fprintf(stderr,"Block %d is type %s, size %d,", i, _RFWfcc2s(btype), bsize);
      fprintf(stderr," container list_type is [%s]\n", _RFWfcc2s(btype2));
    }

    if (btype==opLIST) {
      if (btype2==opLIST)
	fprintf(stderr,"\tList from position %d closes.\n",bsize);
      else
	fprintf(stderr,"\tEntering list of type is %s.\n", _RFWfcc2s(btype2));
      RFRenter_list(handle);
    } else RFRskip_chunk(handle);
    if ((i&1023)==0) fprintf(stderr,"1024 blocks read... \n");
    if (RFReof(handle)==1) break;
  }

  fprintf(stderr,"------------\n");
  fprintf(stderr,"RFRrewind() : ");
  RFRrewind(handle);
  fprintf(stderr,"OK\n");

  fprintf(stderr,"RFRskip_junk(1) : ");
  RFRskip_junk(handle,1);
  fprintf(stderr,"OK\n");

  fprintf(stderr,"Testing RFRget_block_info() and RFRget_chunk() without JUNK\n");

  strhcount=0;

  for (i=0; i<iters; i++){
    RFRget_block_info(handle, &btype, &bsize, &btype2);
    fprintf(stderr,"Block %d is type %s, size %d \n", i, _RFWfcc2s(btype), bsize);
    
    if (btype==opLIST) {

      if (btype2==opLIST) {
	fprintf(stderr,"\tList from position %d closes.\n",bsize);
	RFRenter_list(handle);
	/* This is an STRL list */
      } else if (btype2==opstrl) {      
	strhcount++;
	fprintf(stderr, "Now we found the stream header list number %i . Initialize codec.\n", 	
		strhcount);
	fprintf(stderr, 
		"Read the next chunks in structures AVIStreamHeader and BITMAPINFOHEADER.\n");	
	RFRenter_list(handle);
	/* Enter code that reads AVIStreamHeader and BITMAPINFOHEADER - if it is video! */

      
	/* This is the MOVI list */
      } else if (btype2==opmovi) {      
	fprintf(stderr, "Now we are insider the actual movie. Grab the ##dc chunks.\n");
	RFRenter_list(handle);
	/* This is the HDRL list */
      } else if (btype2==ophdrl) {     
	fprintf(stderr,"Now we found the avi header list that contains strl's.\n");
	RFRenter_list(handle);
      } else {      	
	/* This is another list */
	fprintf(stderr,"\tEntering list of type is %s.\n", _RFWfcc2s(btype2));
	RFRenter_list(handle);	
      }

    }
    
    /* This is a chunk, not a list */
    else {
      /* Note that the audio stream may be 00wb or 02wb or whatever... I use
	 01wb as an example. You have to read the strh/strf chunks to know. */
      if (btype==_RFWs2fcc('0','1','w','b')) {
	fprintf(stderr, "Skipping audio chunk...\n");
	RFRskip_chunk(handle);
      } else if (btype==_RFWs2fcc('0','0','d','c')) {
	fprintf(stderr, "Reading video chunk...\n");
	RFRget_chunk(handle,big_buffer);      
	/* Now call the decoding function */
      } else 
	/* We skip this chunk */
	RFRskip_chunk(handle);
    }
    if ((i&1023)==0) fprintf(stderr,"1024 blocks read... \n");
    if (RFReof(handle)==1) break;
  }


  fprintf(stderr,"------------\n");
  fprintf(stderr,"RFRrewind() : ");
  RFRrewind(handle);
  fprintf(stderr,"OK\n");

  fprintf(stderr,"RFRskip_junk(1) : ");
  RFRskip_junk(handle,1);
  fprintf(stderr,"OK\n");

  fprintf(stderr,"Testing RFRseek_forward_to_chunk_type()\n");

  fprintf(stderr,"Trying an unsuccesful seek operation. This should take some time.\n");
  fprintf(stderr,"RFRseek_forward_to_chunk_type(,'pppp')\n");
  RFRseek_forward_to_chunk_type(handle, _RFWs2fcc('p','p','p','p'));

  fprintf(stderr,"RFRrewind() : \n");
  RFRrewind(handle);

  seekt=0;

  for (i=0; i<iters; i++){
    if (RFRseek_forward_to_chunk_type(handle, _RFWs2fcc('0','0','d','c'))==1){
      RFRget_block_info(handle, &btype, &bsize, &btype2);
      //fprintf(stderr,"Block %d is type %s, size %d \n", i, _RFWfcc2s(btype), bsize);
      RFRskip_chunk(handle);
    } else fprintf(stderr,"seeking was not succesful!\n");

    if (seekt<1024) seek_targets[seekt++]=RFRget_position(handle);

    if ((i&1023)==0) fprintf(stderr,"1024 blocks read... \n");
    if (RFReof(handle)==1) break;
  }


  fprintf(stderr,"------------\n");
  fprintf(stderr,"RFRrewind() : ");
  RFRrewind(handle);
  fprintf(stderr,"OK\n");

  fprintf(stderr,"RFRskip_junk(1) : ");
  RFRskip_junk(handle,1);
  fprintf(stderr,"OK\n");

  fprintf(stderr,"Testing RFRseek_forward_to_list_type()\n");

  fprintf(stderr,"Trying an unsuccesful seek operation. This should take some time.\n");
  fprintf(stderr,"RFRseek_forward_to_list_type(,'pppp')\n");
  RFRseek_forward_to_list_type(handle, _RFWs2fcc('p','p','p','p'));

  fprintf(stderr,"RFRrewind() : \n");
  RFRrewind(handle);

  
  fprintf(stderr,"Trying random targets for RFRseek_to_position()\n");

  start=clock();

  for (i=0; i<iters; i++){
    j=rand()%seekt;
    fprintf(stderr,"Seeking to position %ld : ", seek_targets[j]);
    RFRseek_to_position(handle, seek_targets[j]);
    fprintf(stderr," now at %ld. \n", handle->file_position);
    
    if ((i&1023)==0) fprintf(stderr,"1024 blocks read... \n");
    if (RFReof(handle)==1) break;
  }

  end=clock();
  end-=start;

  fprintf(stderr,"Done %d iterations in %f seconds. %f sec/seek. \n",
	  iters,
	  (double)end/CLOCKS_PER_SEC,
	  (double)end/CLOCKS_PER_SEC/iters);

  /********************************/

  RFRrewind(handle);
  RFRskip_junk(handle, 1);

  fprintf(stderr,"------------\n");

  fprintf(stderr,"Now let's try something real. Stream extraction. \n");
  fprintf(stderr,"We will keep all chunks except 00dc ones.\n");
  fprintf(stderr,"Stream headers are copied as-is\n");  
  fprintf(stderr,"------------\n");

  output=RFWcreate_riff_handle("./kopria.avi", opAVI);


  for (;RFReof(handle)==0;){
    RFRget_block_info(handle, &btype, &bsize, &btype2);
    if (btype==opLIST) {
      if (btype2!=opLIST){
	RFRenter_list(handle); 
	RFWcreate_list(output,btype2);
	fprintf(stderr,"Created list of type %s.\n",_RFWfcc2s(btype2));      
      } else {
	RFRskip_list(handle);
	RFWclose_list(output);
	fprintf(stderr,"Closed list. \n");
      }
    } else {
      /* Skip every 00dc chunk. */
      if (btype==_RFWs2fcc('0','0','d','c')) RFRskip_chunk(handle);
      else {
	RFRget_chunk(handle, big_buffer);
	RFWwrite_chunk(output, btype, bsize, big_buffer);
	fprintf(stderr,"Wrote chunk of type %s\n", _RFWfcc2s(btype));
      }
    }
  }
   
    
  RFRclose_riff_file(handle);
  RFWclose_riff(output);

}    
  
