
VERSION = 1.08
NAME    = libriff

MVTOOL_PREFIX := /opt/mv_pro_5.0/montavista/pro/devkit/arm/v5t_le/bin/arm_v5t_le-

CC	=	$(MVTOOL_PREFIX)gcc
CFLAGS	=	-Wall -O3  -g -pedantic

LD	=	$(MVTOOL_PREFIX)gcc 
LIBS	=	-L./ -L../ 
LDFLAGS	=	-shared -Wl,-soname,$(NAME).so.$(VERSION) -lc $(LIBS)

CEXCEPT	=	endian.c
CFILES	=	$(shell ls *.c|grep -vw $(CEXCEPT))
OFILES	=	$(CFILES:.c=.o)
LOFILES	=	$(CFILES:.c=.lo)

INCLUDE_PATH = 	-I./ 

all: $(NAME).so.$(VERSION) $(NAME).a

install: $(NAME).so$(VERSION) $(NAME).a
	cp $(NAME).so.$(VERSION) /home/jnzhao/tmp ; \
	cp $(NAME).a /home/jnzhao/tmp ; \
	mkdir /home/jnzhao/tmp/include/riff ; \
	cp *.h /home/jnzhao/tmp/include/riff

dist: distclean doc/manual.ps
	pushd doc ; make distclean ; make ; popd ; pushd ../ ; \
	rm -rf $(NAME)-$(VERSION) ; mkdir $(NAME)-$(VERSION); popd ; \
	cp -a * ../$(NAME)-$(VERSION) ; \
	tar -czf ../$(NAME)-$(VERSION).tgz ../$(NAME)-$(VERSION)

$(NAME).so.$(VERSION):	$(LOFILES)
		$(LD) $(LOFILES) -o $@ $(LDFLAGS)

$(NAME).a:	$(OFILES)
		ar rcs $@ $(OFILES)  

.PHONEY:	depend clean

include .dep

%.o:	%.c
	$(CC) $(CFLAGS) $(INCLUDE_PATH) -c $< -o $@

%.lo:	%.c
	$(CC) $(CFLAGS) -fPIC $(INCLUDE_PATH) -c $< -o $@

.dep depend : *.c *.h
	gcc -M $(INCLUDE_PATH) *.c > .dep

clean :
	rm -f *.o *.lo .dep

distclean : 
	rm -f *.o *.lo *.?~ Makefile~ .dep $(NAME).a $(NAME).so.$(VERSION)
