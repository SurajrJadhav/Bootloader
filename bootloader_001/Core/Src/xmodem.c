/*
 * xmodem.c
 *
 *  Created on: 15-Sep-2022
 *      Author: suraj
 */

#include "xmodem.h"
#include "bootloader.h"

#define DEBUG_XMODEM
/*
 * 		Packet Info
*      +-----+-------+-------+------+-----+------+
*      | SOH | PKT_num | Data_Size | data | SUM  |
*      +-----+-------+-------+------+-----+------+
*/

uint8_t xmodem_calcrc(uint8_t *ptr, int count)
{
	uint8_t  crc;
    char i;

    crc = 0;
    while (--count >= 0)
    {
        crc = crc ^ *ptr++;
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



XMODEM_StatusTypedef xmodem_receive(UART_HandleTypeDef *BL_UART){

	uint8_t rxbuf[1050]={0};
	uint8_t header,packet_number=0,size[2],received_crc,response;
	uint16_t actual_size;

	/*Receive & Check Header**/
	HAL_UART_Receive(BL_UART, &header, 1, HAL_MAX_DELAY);

	switch(header)
	 {
	 case SOH:
	 case STX:
		 	 /*receive PKT_num & size*/
		 	 HAL_UART_Receive(BL_UART, &packet_number, 1, HAL_MAX_DELAY);
		 	 /*size is of 2 byte*/
		 	 HAL_UART_Receive(BL_UART, size, 2, HAL_MAX_DELAY);
		 	 /*use size from 1 byte to 2 byte*/
		 	 actual_size=*(uint16_t*)size;
		 	 /*receive DATA*/
		 	 HAL_UART_Receive(BL_UART, rxbuf, actual_size, HAL_MAX_DELAY);
		 	 /*receive CRC*/
		 	 HAL_UART_Receive(huart5, &received_crc, 1, HAL_MAX_DELAY);

		 	uint8_t calculated_crc = xmodem_calcrc(rxbuf, actual_size);
		 	 if(calculated_crc != received_crc){
		 		 //send NAK
		 		 response=NAK;
		 		 HAL_UART_Transmit(huart5, &response, 1, HAL_MAX_DELAY);
		 		 return XMODEM_ERROR;
		 	 }
		 	 else
		 	 {
		 		//send NAK
		 		response=ACK;
		 		HAL_UART_Transmit(huart5, &response, 1, HAL_MAX_DELAY);
		 	 }
		 	 /*flash memory*/
		 	 bootloader_write_bin_to_memory(rxbuf,actual_size);

#ifdef DEBUG_XMODEM
		 	/*debug*/
		 	char temp[12];
		 	sprintf(temp,"send=%d\n\r",packet_number);
		 	HAL_UART_Transmit(BL_UART, temp, strlen(temp), HAL_MAX_DELAY);
	 		/*debug end*/
#endif
		 	break;

	 case EOT:
		 /*Receive of EOT will jump to user app*/
#ifdef DEBUG_XMODEM
		 HAL_UART_Transmit(BL_UART, "Jump to user", 15, HAL_MAX_DELAY);
#endif
		 if(bootloader_jump_to_user_code(BL_UART) == BL_ERROR){
			 HAL_UART_Transmit(BL_UART, "ERROR", strlen("ERROR"), HAL_MAX_DELAY);
			 while(1);
		 }

	 }
}

