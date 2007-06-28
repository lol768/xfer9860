/*******************************************************************************
	xfer9860 - fx-9860G (SD) communication utility
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

#include "sendfile.h"

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
				displayHelp();
				return 0;
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
	printf(	"--- Send file to an fx-9860G (SD) by USB.\n"
		"Usage: xfer9860 <action> filename [-t throttle]\n"
		"Actions:\n"
		" -u file\tUpload `file' from PC to `filename' on device.\n"
		//" -d file\tDownload `file' from device to`filename' on PC.\n"
		"\nParameters:\n"
		" -t\tThrottle setting. The value specified in ms will be used to slow down\n"
	      	"\t transfers. Default is 0. Try increasing this in case of problems.\n"
	      	"");
	printf("\n");


}
