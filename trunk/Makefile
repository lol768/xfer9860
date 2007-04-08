OBJECTS=xfer9860.o Casio9860.o

xfer9860: Casio9860.o xfer9860.o
	cc ${OBJECTS} -o xfer9860 -lusb
Casio9860.o: Casio9860.c Casio9860.h
	cc -c Casio9860.c -o Casio9860.o
xfer9860.o: xfer9860.c Casio9860.h
	cc -c xfer9860.c -o xfer9860.o