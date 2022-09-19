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


int calcrc(char *ptr, int count);

int main() {


	serial_port = open("/dev/ttyUSB2", O_RDWR);
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




