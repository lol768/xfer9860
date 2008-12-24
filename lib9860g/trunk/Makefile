LIBSRCS=lib9860g.c packetio.c usbio.c log.c util.c
LIBOBJS=$(LIBSRCS:.c=.o)

PROGSRCS=main.c
PROGOBJS=$(PROGSRCS:.c=.o)

AR=ar
RM=rm
CC=gcc
CFLAGS=-Wall
INCLUDES=-I"C:\Program files\libusb-win32\include"
LIBDIR=-L"C:\Program Files\LibUSB-Win32\lib\gcc" -L.
LIBS= -l9860g -lusb

LIBOUT=lib9860g.a

PROGOUT=output.exe

.PHONY: clean


$(PROGOUT): $(PROGOBJS) $(LIBOUT)
	$(CC) $(CFLAGS) $(LIBDIR) $(PROGOBJS) -o $(PROGOUT) $(LIBS)

$(LIBOUT): $(LIBOBJS)
	$(AR) ruvs $(LIBOUT) $(LIBOBJS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $(<:.c=.o)

clean:
	$(RM) -rf $(PROGOUT) $(LIBOUT) *.o