CFLAGS=		-ggdb -Wall

default:	all

all:		ofx

ofx:		ofx.o packet.o array.o ofproto.o matlab.o monitoring.o ofswitch.o ofport.o topology.o
	$(CC) -o ofx ofx.o packet.o array.o ofproto.o matlab.o monitoring.o ofswitch.o ofport.o topology.o

clean:
	rm -f ofx *.o *.core
