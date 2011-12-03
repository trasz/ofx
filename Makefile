CFLAGS=		-ggdb -Wall

default:	all

all:		ofx

ofx:		ofx.o packet.o array.o ofproto.o matlab.o
	$(CC) -o ofx ofx.o packet.o array.o ofproto.o matlab.o

clean:
	rm -f ofx *.o *.core
