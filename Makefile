all:
	cd lib; $(MAKE) all

install:
	cd lib; $(MAKE) install

tst:
	cd test; $(MAKE)

clean:
	cd lib; $(MAKE) clean
	cd test; $(MAKE) clean
