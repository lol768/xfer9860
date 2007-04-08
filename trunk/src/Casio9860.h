/***************************************************************************
 *   Copyright (C) 2007													   *
 *		Andreas Bertheussen <andreasmarcel@gmail.com>					   *
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

#define C9860_VENDOR_ID   0x07CF
#define C9860_PRODUCT_ID  0x6101

/*	Function declarations	*/
int init_9860(usb_dev_handle* usb_handle);
struct usb_device *device_init(void);


#endif
