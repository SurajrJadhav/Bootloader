#include <stdio.h>
#include <string.h>
#include <fcntl.h>

//function prototypes
void command_menu();

int main() {
	

	int serial_port = open("/dev/ttyUSB0", O_RDWR);
	char input;
	
	command_menu();
	
	scanf("%c",&input);
	
	switch(input)
	{
		case '0':write(serial_port,"0",O_RDWR);break;
		case '1':write(serial_port,"1",O_RDWR);break;
		case '2':write(serial_port,"2",O_RDWR);break;
		case '3':write(serial_port,"3",O_RDWR);break;
	}
	close(serial_port);
	return 0; // success
}/*main()*/

void command_menu(){
	printf("0	BL_JMP_TO_USER_CODE\n");
	printf("1	BL_WRITE_MEMORY\n");
	printf("2	BL_GET_VERSION\n");
	printf("0	BL_READ_MEMORY\n");
	printf("Enter : ");
}/*command_menu()*/


