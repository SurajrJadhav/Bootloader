/*
 * xmodem.c
 *
 *  Created on: 18-Sep-2022
 *      Author: suraj
 */
#include"xmodem.h"


int serial_port;


/*
 * 		Packet Info
*      +-----+-------+-------+------+-----+------+
*      | SOH | PKT_num | Data_Size | data | SUM  |
*      +-----+-------+-------+------+-----+------+
*      	  1		 1			 2		  x		  1
*/
__uint8_t xmodem_calcrc(unsigned char *ptr, int count)
{
	__uint8_t  crc;
    char i;

    crc = 0;
    while (--count >= 0)
    {
        crc = crc ^ (*ptr);
        ptr++;
        i = 8;
        do
        {
            if (crc & 0x80)
                crc = crc << 1 ^ 0x97;
            else
                crc = crc << 1;
        } while(--i);
    }
    return crc;
}

void xmodem_wait_for_send(){
	unsigned char ch=0;
	while(ch!=NAK)
	{
		read(serial_port,&ch,1);
	}

}

void xmodem_send_packet(){
	int fd = open("/home/suraj/STM_CubeIDE_Workspace/user_test/Debug/user_test.bin",O_RDWR);
	if(fd<=0)
		perror("open");
	{
	}

	__uint8_t rxbuf[1050]={0},response;

	/*write 1 to send signal of data read*/
	write(serial_port,"1",O_RDWR);
	int EOT_Flag,i,j,packet_num=0;
	__uint16_t size;
	do{
		/*start txn*/
		size=PKT_SIZE_1K;
		rxbuf[0]=STX;
		rxbuf[1]=packet_num++;

		/*read the data equal to packet size*/
		for(int k=0;k<PKT_SIZE_1K;k++){
			j=read(fd,rxbuf+k+4,1);
			if(j==0){
				EOT_Flag=1;
				size=k;
				break;
			}
		}

		/*update size*/
		rxbuf[2]=(size & (0x00ff));
		rxbuf[3]=((size & (0xff00))>>8);
		printf("size = %d\n",*(__uint16_t*)&rxbuf[2]);

		if(packet_num==1)
			xmodem_wait_for_send();
		else
			sleep(1);
		//send header
		write(serial_port,rxbuf,1);
		//send packet number
		write(serial_port,rxbuf+1,1);
		//send size
		write(serial_port,rxbuf+2,2);

		//crc addition
		__uint8_t crc=xmodem_calcrc(rxbuf+4, size);
		rxbuf[size+4]=crc;

		/*debug*/
		for(int k=0;k<size+5;k++){
			printf("0x%.2x ",rxbuf[k]);
		}
		printf("\n");

		//send data
		write(serial_port,rxbuf+4,size);
		//send crc
		write(serial_port,&crc,1);
		//read ack
		read(serial_port,&response,1);
		if(response!=ACK){
			printf("NAK received!!!\n");
			return;
		}
		else
		{
			printf("ACK received = %p!!!\n",response);
		}

	}while(EOT_Flag==0);

	/*EOT of file*/
	rxbuf[0]=EOT;
	printf("0x%.2x ",rxbuf[0]);
	write(serial_port,rxbuf,1);

	//write(stdout,rxbuf,10);


}


