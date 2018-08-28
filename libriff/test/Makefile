

rft : rft.c ../libriff.a ../riffread.h
	gcc -O3 -march=athlon -g -L../ -I../ rft.c -lriff -o rft

clean : 
	rm *.o kopria.avi rft	

distclean :
	rm *.c~ Makefile~ *.avi rft
