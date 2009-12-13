all:
	cd lib; $(MAKE) all
byte:
	cd lib; $(MAKE) byte

install:
	cd lib; $(MAKE) install

tst:
	cd test; $(MAKE)

clean:
	cd lib; $(MAKE) clean
	cd test; $(MAKE) clean
