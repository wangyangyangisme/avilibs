

--- avilibs bundle


31/12/2001

What's this?

This is a "bundled" version of libriff and libavi (since libavi
requires libriff) so that you won't have to worry about their
dependencies. Both are made from the current tree and should
inter-operate. I suppose this makes installation simpler.

Many key features are planned for avilibs (including a third
library, named libcodec) but for the moment I just wish to finish
libavi (AVI reading should freeze by 15/01/2001). Please help
me debug it.

Petros Tsantoulis, <ptsant at otenet.gr>



--- NEW in version 0.15

- _AVRbuild_idx() now works. This function builds indexes for avi files
  without one. It may need some cleanup, but preliminary tests show 
  that it works OK. The file is thoroughly commented, have look if
  you wish.
- Minor bug fixes
- More elaborate test program (avt, in directory libavi/test)
