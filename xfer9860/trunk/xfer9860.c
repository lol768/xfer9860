/*******************************************************************************
	xfer9860 - a Casio fx-9860G (SD) communication utility
	Copyright (C) 2007
		Andreas Bertheussen <andreasmarcel@gmail.com>

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
	MA  02110-1301, USA.
*******************************************************************************/

#include <stdio.h>
#include <unistd.h>

#include "uploadfile.h"

const char* _version_ = "0.2";

void displayHelp();

int main(int argc, char *argv[]) {
	int opt;
	char *sourceFileName;
	int throttleSetting = 0;
	int sendFileFlag = 0;
	int throttleAble = 0;
	printf("--- xfer9860 v%s Copyright (C) 2007 Andreas Bertheussen and Manuel Naranjo.\n", _version_);

	while ((opt = getopt(argc, argv, "t:hu:")) != -1) {
		switch(opt) {
			case 't':
				throttleSetting = atoi(optarg);
				break;
			case 'h':
				break;
			case 'u':
				sourceFileName = optarg;
				sendFileFlag = 1;
				throttleAble = 1;
				break;
			default:
				return 0;
		}
	}

	if (sendFileFlag) {
		if (optind >= argc)
			printf("You must specify a destination file name.\n");
		else
			sendFile(sourceFileName, argv[optind], throttleSetting);
		return 0;
	}

	displayHelp();
	return 0;
}

void displayHelp() {
	printf(	"--- a Casio fx-9860G (SD) communication utility.\n"
		"Usage: xfer9860 <action> destname [-t throttle]\n"
		"Actions:\n"
		" -u srcname\tUpload file `srcname' from PC to `destname' on device.\n"
		//" -d srcname\tDownload file `srcname' from device to `destname' on PC.\n"
		"\nParameters:\n"
		" -t value\tThrottle setting. The value specifies the delay in ms between\n"
	      	"\t\tpackets. Default is 0. Try increasing this in case of problems.\n"
	      	"");
	printf("\n");


}
