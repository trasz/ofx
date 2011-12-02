CFLAGS=		-ggdb

default:	all

all:		ofx

ofx:		ofx.o packet.o array.o
	$(CC) -o ofx ofx.o packet.o array.o

clean:
	rm -f ofx *.o
