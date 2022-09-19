/*
 * bootloader.h
 *
 *  Created on: Sep 11, 2022
 *      Author: suraj
 */

#ifndef INC_BOOTLOADER_H_
#define INC_BOOTLOADER_H_

#include "stm32f4xx_hal.h"

typedef enum {
	BL_OK,
	BL_ERROR
}BL_StatusTypedef;


//macros
#define FLASH_DOWNLOAD_AREA 0x08008000U

#define BL_UART huart5
#define BL_VERSION "1.0"
#define CMD_SIZE 1

//command code
#define BL_JMP_TO_USER_CODE 		'0'
#define BL_WRITE_BIN_TO_MEMORY 		'1'
#define BL_GET_VERSION 				'2'


/*user code section*/
#define USER_CODE_SECTION_ADDRESS 0x08008000U
#define RESET_HANDLER_ADDRESS 0x08008004U

//function prototypes
void bootloader_mode(void);

//bootloader command functions
BL_StatusTypedef bootloader_jump_to_user_code(UART_HandleTypeDef*);
BL_StatusTypedef bootloader_get_version(UART_HandleTypeDef*);
BL_StatusTypedef bootloader_write_bin_to_memory(uint8_t *,int );
BL_StatusTypedef bootloader_read_memory(UART_HandleTypeDef*);
BL_StatusTypedef bootloader_flash_erase_all(UART_HandleTypeDef*);
BL_StatusTypedef bootloader_flash_erase_download_area();
BL_StatusTypedef bootloader_flash_varify(UART_HandleTypeDef*);


#endif /* INC_BOOTLOADER_H_ */
