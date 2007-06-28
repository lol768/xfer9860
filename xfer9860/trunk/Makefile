OBJECTS=xfer9860.o Casio9860.o usbio.o sendfile.o
BIN=xfer9860
OPT=

${BIN}: Casio9860.o xfer9860.o usbio.o sendfile.o
	cc ${OBJECTS} -o xfer9860 -lusb -lm ${OPT}
Casio9860.o: Casio9860.c Casio9860.h
	cc -c Casio9860.c -o Casio9860.o ${OPT}
xfer9860.o: xfer9860.c Casio9860.h usbio.o usbio.h
	cc -c xfer9860.c -o xfer9860.o ${OPT}
usbio.o: usbio.h usbio.c
	cc -c usbio.c -o usbio.o ${OPT}
sendfile.o: sendfile.c sendfile.h
	cc -c sendfile.c -o sendfile.o ${OPT}

clean:
	-rm -f ${OBJECTS} ${BIN} *~
