

CC 	= gcc
CFLAGS 	= -march=pentium -O3 -g -Wall
INCLUDE = -I../ -I../../libriff/
LIBDIR 	= -L../ -L../../libriff/
LIBS	= -lavi -lriff 


examples: example1 example2

all: tutorial example1 example2 # example3...

example1 : example1.c # main.c ../libavi.a ../../libriff/libriff.a
	-rm example1
	cd ../ ; make 
	cd ../../libriff ; make
	$(CC) $(CFLAGS) $(INCLUDE) $(LIBDIR) example1.c $(LIBS) -o example1

example2 : example2.c # main.c ../libavi.a ../../libriff/libriff.a
	-rm example2
	cd ../ ; make 
	cd ../../libriff ; make
	$(CC) $(CFLAGS) $(INCLUDE) $(LIBDIR) example2.c $(LIBS) -o example2

example3 : example3.c # main.c ../libavi.a ../../libriff/libriff.a
	-rm example3
	cd ../ ; make 
	cd ../../libriff ; make
	$(CC) $(CFLAGS) $(INCLUDE) $(LIBDIR) example3.c $(LIBS) -o example3

tutorial: tutorial.tex example1.c example2.c # example3.c
	latex tutorial.tex
	latex tutorial.tex # Rebuild to get the index right
	dvips tutorial.dvi
	pdflatex tutorial.tex
	pdflatex tutorial.tex

../libavi.a :
	cd ../ ; make

../../libriff/libriff.a :
	cd ../../libriff ; make

clean : 
	rm *.o example1
