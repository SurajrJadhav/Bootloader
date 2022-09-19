/*
 * bootloader.c
 *
 *  Created on: Sep 11, 2022
 *      Author: Suraj
 */
#include "bootloader.h"
#include "main.h"
#include "xmodem.h"

BL_StatusTypedef bootloader_jump_to_user_code(UART_HandleTypeDef*BL_UART){

	//set msp
	uint32_t msp_value=*(uint32_t*)USER_CODE_SECTION_ADDRESS;
	__set_MSP(msp_value);

	//relocate vector table
	SCB->VTOR=(__IOM uint32_t)USER_CODE_SECTION_ADDRESS;

	//create function pointer for reset handler
	void(*reset_handler)(void);

	reset_handler=(void(*)(void))(*(uint32_t *)RESET_HANDLER_ADDRESS);
	reset_handler();

	return BL_ERROR;
}

/*********Bootloader Command Functions*****************************/

BL_StatusTypedef bootloader_get_version(UART_HandleTypeDef*BL_UART)
{

}

BL_StatusTypedef bootloader_write_bin_to_memory(uint8_t *rxbuf ,int size)
{
	static uint32_t address=FLASH_DOWNLOAD_AREA;

	HAL_FLASH_Unlock();
	int i=0;
	while(i < size){

		if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE,address++, *(rxbuf+i)) == HAL_OK ){
			i++;
		}
		else{
			return BL_ERROR;
		}
	}
	HAL_FLASH_Lock();
	return BL_OK;
}
BL_StatusTypedef bootloader_flash_erase_download_area(){
	HAL_FLASH_Unlock();
	FLASH_Erase_Sector(FLASH_SECTOR_2, FLASH_VOLTAGE_RANGE_3);
	HAL_FLASH_Lock();
	return BL_OK;
}

BL_StatusTypedef bootloader_read_memory(UART_HandleTypeDef*BL_UART){

}


