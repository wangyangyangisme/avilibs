

# Just make everythng...
all : 
	cd libriff ; make 
	cd libavi ; make 

install:
	cd libriff ; make install
	cd libavi : make install
