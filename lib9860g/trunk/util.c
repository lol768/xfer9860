/**
 * @file util.c
 * @author Andreas Bertheussen
 * @brief Common functionality for the different parts of the library.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

/** 
* @brief Convert the given value in ASCII-hexadecimal format into an integer.
* 
* @param string Pointer to where the string is stored.
* @param length The length of the string.
*/
int aschexToInt(char *string, int length) {
	int value = 0;
	char *buffer = (char*)malloc((length+1) * sizeof(char));
	memcpy(buffer, string, length);
	buffer[length] = 0;	/* NULL-terminate the extra char */

	value = strtol(buffer, NULL, 16);	/* get the value in an int */
	free(buffer);
	return value;
}

void strreverse(char* begin, char* end) {
	char aux;
	while(end>begin)
		aux=*end, *end--=*begin, *begin++=aux;
}
	
int intToAschex(unsigned int value, char* str, int digits) {
	if (value > 0x7FFFFFFF) {
		printf("Too big argument for intToAscHex(). Aborting.\n");
		return -1;
	}
	static char num[] = "0123456789ABCDEF";
	char* wstr=str;
	int count = 0;
	div_t res;

	do {
		res = div(value,16);
		*wstr++ = num[res.rem];
		value=res.quot;
		count++;
	} while (wstr < str+digits);
	
	strreverse(str,wstr-1);
	return 0;
}
