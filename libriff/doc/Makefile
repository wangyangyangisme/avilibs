
manual: manual.tex
	latex manual.tex ; latex manual.tex ; dvips manual.dvi

pdf : manual.tex
	pdflatex manual.tex

clean : 
	rm manual.dvi manual.ps manual.pdf

distclean : 
	rm manual.tex~
