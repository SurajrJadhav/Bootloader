/*
 ============================================================================
 Name        : xmodem.c
 Author      : Suraj_Jadhav
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */


#include "xmodem.h"



#define BL_JMP_TO_USER_CODE 		'0'
#define BL_WRITE_MEMORY 		'1'
#define BL_GET_VERSION 			'2'
#define BL_READ_MEMORY 			'3'

extern int serial_port;

//function prototypes
void command_menu(){
	printf("0	BL_JMP_TO_USER_CODE\n");
	printf("1	BL_WRITE_MEMORY\n");
	printf("2	BL_GET_VERSION\n");
	printf("3	BL_READ_MEMORY\n");
	printf("Enter : ");
}/*command_menu()*/


int main() {


	serial_port = open("/dev/ttyUSB2", O_RDWR);
	  // Create new termios struct, we call it 'tty' for convention
	struct termios tty;

	  // Read in existing settings, and handle any error
	  if(tcgetattr(serial_port, &tty) != 0) {
	      printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
	      return 1;
	  }

	  tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
	  tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
	  tty.c_cflag &= ~CSIZE; // Clear all bits that set the data size
	  tty.c_cflag |= CS8; // 8 bits per byte (most common)
	  tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
	  tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

	  tty.c_lflag &= ~ICANON;
	  tty.c_lflag &= ~ECHO; // Disable echo
	  tty.c_lflag &= ~ECHOE; // Disable erasure
	  tty.c_lflag &= ~ECHONL; // Disable new-line echo
	  tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
	  tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
	  tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes

	  tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
	  tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
	  // tty.c_oflag &= ~OXTABS; // Prevent conversion of tabs to spaces (NOT PRESENT ON LINUX)
	  // tty.c_oflag &= ~ONOEOT; // Prevent removal of C-d chars (0x004) in output (NOT PRESENT ON LINUX)

	  tty.c_cc[VTIME] = 10;    // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
	  tty.c_cc[VMIN] = 0;

	  // Set in/out baud rate to be 115200
	  cfsetispeed(&tty, B115200);
	  cfsetospeed(&tty, B115200);

	  // Save tty settings, also checking for error
	  if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
	      printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
	      return 1;
	  }



	char input;
	command_menu();

	scanf("%c",&input);

	switch(input)
	{

		case BL_JMP_TO_USER_CODE	:write(serial_port,"0",O_RDWR);	break;
		case BL_WRITE_MEMORY		:xmodem_send_packet();	break;
		case BL_GET_VERSION			:write(serial_port,"2",O_RDWR);	break;
		case BL_READ_MEMORY			:write(serial_port,"3",O_RDWR);	break;
	}
	close(serial_port);
	return 0; // success

}/*main()*/




