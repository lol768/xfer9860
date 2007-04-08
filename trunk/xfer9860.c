/***************************************************************************
 *   Copyright (C) 2007													   *
 *		Manuel Naranjo <naranjo.manuel@gmail.com>						   *
 *		Andreas Bertheussen <andreasmarcel@gmail.com>					   *
 *   Copyright (C) 2004													   *
 *		Greg Kroah-Hartman <greg@kroah.com>								   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <usb.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "Casio9860.h"

int main(int argc, char *argv[]) {
	int retval = 0;
	char *buffer;
	struct stat file_info;

	if (argc < 2) {
		printf("Upload a file to the fx-9860 by USB\nUsage: %s <filename>\n", argv[0]);
		return 0;
	}
	FILE *source = fopen(argv[1], "r");
	if (source == NULL) {
		printf("Unable to open file: %s\n", argv[1]);
		goto exit;
	}

	struct usb_device *usb_dev;
	struct usb_dev_handle *usb_handle;

	usb_dev = (struct usb_device*)device_init();
	if (usb_dev == NULL) {
		fprintf(stderr,
				"The calculator does not seem to be connected,\n"
				"- make sure it is set to receive; press [menu], [D], [F2]\n");
		goto exit;
	}

	usb_handle = (struct usb_dev_handle*)usb_open(usb_dev);
	if (usb_handle == NULL) {
		fprintf(stderr, "Unable to open the USB device\n");
		goto exit_close;
	}

	retval = usb_claim_interface(usb_handle, 0);
	if (retval < 0) {
		fprintf(stderr, "Unable to claim USB Interface\n");
		goto exit_unclaim;
	}

	retval = init_9860(usb_handle);
	if (retval < 0){
		fprintf(stderr, "Couldn't initilizate connection\n");
		goto exit_unclaim;
	}

	stat(argv[1], &file_info);
	printf("Filesize: %i\n", file_info.st_size);

	exit_unclaim:
		usb_release_interface(usb_handle, 0);
	exit_close:
    	usb_close(usb_handle);
		free(usb_dev);
	exit:
		free(source);
	return EXIT_SUCCESS;
}
