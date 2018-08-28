
MVTOOL_PREFIX := /opt/mv_pro_5.0/montavista/pro/devkit/arm/v5t_le/bin/arm_v5t_le-

CC 	= $(MVTOOL_PREFIX)gcc
CFLAGS 	=  -O3 -g -Wall
INCLUDE = -I../ -I../../libriff/
LIBDIR 	= -L../ -L../../libriff/
LIBS	= -lavi -lriff 

all : # main.c ../libavi.a ../../libriff/libriff.a
	-rm avt *.o
	cd ../ ; make 
	cd ../../libriff ; make
	$(CC) $(CFLAGS) $(INCLUDE) $(LIBDIR) main.c $(LIBS) -o avt

../libavi.a :
	cd ../ ; make

../../libriff/libriff.a :
	cd ../../libriff ; make

clean : 
	rm *.o avt
