/*
 * xmodem.h
 *
 *  Created on: 18-Sep-2022
 *      Author: suraj
 */

#ifndef XMODEM_H_
#define XMODEM_H_

#include <stdio.h>
#include <string.h>
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()


#define SOH		0x01
#define STX		0x02
#define ACK		0x06
#define NAK		0x15
#define EOT		0x04
#define CANCEL	0x18

#define PKT_SIZE 		128
#define PKT_SIZE_1K 	1024


__uint8_t xmodem_calcrc(unsigned char *ptr, int count);
/*function declaration*/
void xmodem_wait_for_send();
void xmodem_send_packet();

#endif /* XMODEM_H_ */
