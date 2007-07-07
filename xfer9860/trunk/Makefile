OBJECTS=xfer9860.o Casio9860.o usbio.o uploadfile.o downloadfile.o getinfo.o
BIN=xfer9860
OPT=

${BIN}: ${OBJECTS}
	cc ${OBJECTS} -o xfer9860 -lusb -lm ${OPT}
Casio9860.o: Casio9860.c Casio9860.h
	cc -c Casio9860.c -o Casio9860.o ${OPT}
xfer9860.o: xfer9860.c config.h
	cc -c xfer9860.c -o xfer9860.o ${OPT}
usbio.o: usbio.h usbio.c config.h
	cc -c usbio.c -o usbio.o ${OPT}
uploadfile.o: uploadfile.c uploadfile.h Casio9860.h
	cc -c uploadfile.c -o uploadfile.o ${OPT}
getinfo.o: getinfo.c getinfo.h
	cc -c getinfo.c -o getinfo.o ${OPT}
downloadfile.o: downloadfile.c downloadfile.h Casio9860.h
	cc -c downloadfile.c -o downloadfile.o ${OPT}

clean:
	-rm -f ${OBJECTS} ${BIN} *~
