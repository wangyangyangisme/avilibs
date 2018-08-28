
  fprintf(stderr,"---> Seek without using idx... \n");

  for(i=0; i<32; i++) 
    stream_size[i]=frames[i]=0;  
  
  for (j=0; j<20;)
    for (i=0; i<avifile->total_streams; i++){
      frame_size=AVRget_frame(avifile, i,(void *)big_buffer, &frame_type);
      if (frame_size>0) {
	fprintf(stdout, "Stream %d:read frame %d: size %d, type %s.\n",
		i, j, frame_size, _RFWfcc2s(frame_type));
	stream_size[i]+=frame_size;
	j++;
      } else AVRnext_frame(avifile, i);	
    }

  for (i=0; i<avifile->total_streams; i++)
    fprintf(stdout,"%s: %8d bytes.\n", 
	    avifile->stream_name[i], stream_size[i]);
    
