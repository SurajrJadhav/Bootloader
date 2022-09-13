/*
 * bootloader.c
 *
 *  Created on: Sep 11, 2022
 *      Author: suraj
 */
#include "bootloader.h"
#include "main.h"

void bootloader_jump_to_user_code(UART_HandleTypeDef*BL_UART){

	//set msp
	uint32_t msp_value=*(uint32_t*)USER_CODE_SECTION_ADDRESS;
	__set_MSP(msp_value);

	//relocate vector table
	SCB->VTOR=(__IOM uint32_t)USER_CODE_SECTION_ADDRESS;

	//create function pointer for reset handler
	void(*reset_handler)(void);

	reset_handler=(void(*)(void))(*(uint32_t *)RESET_HANDLER_ADDRESS);
	reset_handler();
}

/*********Bootloader Command Functions*****************************/

void bootloader_get_version(UART_HandleTypeDef*BL_UART)
{

}
void bootloader_write_bin_to_memory(UART_HandleTypeDef *BL_UART)
{
//	HAL_UART_Transmit(huart5, "Writing", strlen("Writing"), HAL_MAX_DELAY);
//	uint32_t address=0x0800C000 ;
//	HAL_FLASH_Unlock();
//	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, 0x22222222);
//	HAL_FLASH_Lock();
}
void bootloader_read_memory(UART_HandleTypeDef*BL_UART){

}
