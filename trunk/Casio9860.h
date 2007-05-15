/***************************************************************************
 *   xfer9860 - Transfer files between a Casio fx-9860G and computer	   *
 *									   *
 *   Copyright (C) 2007							   *
 *	Manuel Naranjo <naranjo.manuel@gmail.com>			   *
 *	Andreas Bertheussen <andreasmarcel@gmail.com>			   *
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

#ifndef CASIO_9860_H
#define CASIO_9860_H

#include <usb.h>

#define C9860_VENDOR_ID		0x07CF
#define C9860_PRODUCT_ID	0x6101

//Types (subtypes actually) used for specifying what packet does.
#define POSITIVE_NORMAL		'0'
#define POSITIVE_OVERWRITE	'1'
#define POSITIVE_SYSINFO	'2'

#define NEGATIVE_NORMAL		'0'
#define NEGATIVE_RETRANSMIT	'1'
#define NEGATIVE_FILEEXISTS	'2'
#define NEGATIVE_NOOVERWRITE	'3'
#define NEGATIVE_OVERWRITEERR	'4'
#define NEGATIVE_MEMFULL	'5'
#define NEGATIVE_IDENTIFY	'6'

int init_9860(usb_dev_handle*);
struct usb_device *device_init(void);
int fx_Send_Verify(struct usb_dev_handle*, char*);
int fx_Send_Terminate(struct usb_dev_handle*, char*);
int fx_Send_Positive(struct usb_dev_handle*, char*, char);
int fx_Send_Negative(struct usb_dev_handle*, char*, char);
int fx_Send_Change_Direction(struct usb_dev_handle*, char*);
int fx_Send_Flash_Capacity_Request(struct usb_dev_handle*, char*, char*);

#endif
