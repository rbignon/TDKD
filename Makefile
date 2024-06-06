all: build

BINDIR=./

build:
	cd src; make;

clean:
	rm -f *.o *~ *.~* *.save *.bak \#* .\#*
	cd src; rm -f *.o *~ *.~* *.save *.bak \#* .\#*
	cd include; rm -f *~ *.~* *.save *.bak \#* .\#*
