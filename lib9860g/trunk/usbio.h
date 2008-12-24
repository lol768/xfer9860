#ifndef USBIO_H
#define USBIO_H

struct usb_device *fx_findDevice(void);
struct usb_dev_handle *fx_claimDevice(void);
int fx_releaseDevice(struct usb_dev_handle *, int);
int fx_printData(char *, unsigned int, unsigned int, int);
int fx_write(struct usb_dev_handle *, char *, int);
int fx_read(struct usb_dev_handle *, char *, int);
#endif /* USBIO_H */
