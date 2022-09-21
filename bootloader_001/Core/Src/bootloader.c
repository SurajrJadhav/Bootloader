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

	if(bootloader_signature_get_reset_flag(BL_UART))
	{
		uint8_t input;
		HAL_UART_Transmit(huart5, "Update available press 'y' for update\n\r", 30, HAL_MAX_DELAY);
		HAL_UART_Receive(huart5, &input, 1, HAL_MAX_DELAY);

		if(input=='y' || input=='Y')
		{
			/*update application area*/
			if(bootloader_app_update(BL_UART)==BL_OK){
				/*change flag in signature area*/
				bootloader_update_signature_reset_flag(huart5);
				HAL_UART_Transmit(huart5, "Updated", strlen("Updated"), HAL_MAX_DELAY);
			}
			else{
				HAL_UART_Transmit(huart5, "Update Failed",strlen("Update Failed"), HAL_MAX_DELAY);
				return BL_ERROR;
			}
		}
	}

//	bootloader_unlock_flash();

	//set msp
	uint32_t msp_value=*(uint32_t*)FLASH_APPLICATION_AREA;
	__set_MSP(msp_value);

	//relocate vector table
	SCB->VTOR=(__IOM uint32_t)FLASH_APPLICATION_AREA;

	//create function pointer for reset handler
	void(*reset_handler)(void);

	reset_handler=(void(*)(void))(*(uint32_t *)(FLASH_APPLICATION_AREA+4U));

	reset_handler();

	return BL_ERROR;
}

/*********Bootloader Command Functions*****************************/

BL_StatusTypedef bootloader_get_bl_version(UART_HandleTypeDef*BL_UART)
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
BL_StatusTypedef bootloader_flash_erase_signature_area(){
	uint32_t SectorError;
	FLASH_EraseInitTypeDef EraseSectorInfo;

	/*Initialise flash parameters*/
	EraseSectorInfo.Sector=FLASH_SECTOR_3;
	EraseSectorInfo.NbSectors=1;
	EraseSectorInfo.VoltageRange=FLASH_VOLTAGE_RANGE_3;
	EraseSectorInfo.TypeErase=FLASH_TYPEERASE_SECTORS;

	/*erase flash*/
	HAL_FLASH_Unlock();
	if(HAL_FLASHEx_Erase(&EraseSectorInfo,&SectorError)!=HAL_OK){
		return BL_ERROR;
	}
	HAL_FLASH_Lock();

	return BL_OK;

}

BL_StatusTypedef bootloader_flash_erase_download_area(){

	uint32_t SectorError;
	FLASH_EraseInitTypeDef EraseSectorInfo;

	/*Initialise flash parameters*/
	EraseSectorInfo.Sector=FLASH_SECTOR_8;
	EraseSectorInfo.NbSectors=4;
	EraseSectorInfo.VoltageRange=FLASH_VOLTAGE_RANGE_3;
	EraseSectorInfo.TypeErase=FLASH_TYPEERASE_SECTORS;

	/*erase flash*/
	HAL_FLASH_Unlock();
	if(HAL_FLASHEx_Erase(&EraseSectorInfo,&SectorError)!=HAL_OK){
		return BL_ERROR;
	}
	HAL_FLASH_Lock();

	return BL_OK;

}
BL_StatusTypedef bootloader_flash_erase_application_area(){

	uint32_t SectorError;
	FLASH_EraseInitTypeDef EraseSectorInfo;

	/*Initialize flash parameters*/
	EraseSectorInfo.Sector=FLASH_SECTOR_4;
	EraseSectorInfo.NbSectors=4;
	EraseSectorInfo.VoltageRange=FLASH_VOLTAGE_RANGE_3;
	EraseSectorInfo.TypeErase=FLASH_TYPEERASE_SECTORS;

	/*erase flash*/
	HAL_FLASH_Unlock();
	if(HAL_FLASHEx_Erase(&EraseSectorInfo,&SectorError)!=HAL_OK){
		return BL_ERROR;
	}
	HAL_FLASH_Lock();

	return BL_OK;
}

BL_StatusTypedef bootloader_read_memory(UART_HandleTypeDef*BL_UART){

}

BL_StatusTypedef bootloader_update_signature_set_flag(UART_HandleTypeDef*BL_UART){

	bootloader_unlock_flash();

	bl_sig_t temp;
	memcpy(&temp,FLASH_SIGNATURE_AREA,sizeof(bl_sig_t));
	temp.update_flag=1;
	/*update signature*/
	bootloader_signature_update(&temp);

	bootloader_lock_flash();
}
BL_StatusTypedef bootloader_update_signature_reset_flag(UART_HandleTypeDef*BL_UART){

	bootloader_unlock_flash();

	bl_sig_t temp;
	memcpy(&temp,FLASH_SIGNATURE_AREA,sizeof(bl_sig_t));
	temp.update_flag=0;
	/*update signature*/
	bootloader_signature_update(&temp);

	bootloader_lock_flash();
}
BL_StatusTypedef bootloader_update_signature_app_version(UART_HandleTypeDef*BL_UART){

	bootloader_unlock_flash();

	bl_sig_t temp;
	memcpy(&temp,FLASH_SIGNATURE_AREA,sizeof(bl_sig_t));
	temp.app_version+=0.1;

	/*update signature*/
	bootloader_signature_update(&temp);

	bootloader_lock_flash();
}
BL_StatusTypedef bootloader_update_signature_bl_version(UART_HandleTypeDef*BL_UART){

	bootloader_unlock_flash();

	bl_sig_t temp;
	memcpy(&temp,FLASH_SIGNATURE_AREA,sizeof(bl_sig_t));
	temp.bl_version+=0.1;

	/*update signature*/
	bootloader_signature_update(&temp);

	bootloader_lock_flash();
}

int bootloader_signature_get_reset_flag(UART_HandleTypeDef*BL_UART){

	/*open flash for operation*/
	bootloader_unlock_flash();

	uint8_t flag=((bl_sig_t *)FLASH_SIGNATURE_AREA)->update_flag;

	/*lock flash operation*/
	bootloader_lock_flash();
	return flag;
}


BL_StatusTypedef bootloader_app_update(UART_HandleTypeDef*BL_UART){

	/*erase memory in order to write*/
	bootloader_flash_erase_application_area();

	HAL_FLASH_Unlock();
	uint32_t address=FLASH_APPLICATION_AREA;
	uint32_t *data =FLASH_DOWNLOAD_AREA;

	while(*data != 0xffffffff){

		if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, *data)==HAL_ERROR)
		{
			HAL_FLASH_Lock();
			return BL_ERROR;
		}
		address=address+4;
		data++;
	}
	HAL_FLASH_Lock();
	return BL_OK;
}

BL_StatusTypedef  bootloader_signature_update(bl_sig_t *sigdata){

	/*UNLOCK flash for operation*/
	bootloader_unlock_flash();

	uint32_t address=FLASH_SIGNATURE_AREA;
	uint8_t *data = (uint8_t*)sigdata;
	/*erase sector to update*/
	bootloader_flash_erase_signature_area();

	/*copy content to signature area*/
	memcpy(FLASH_SIGNATURE_AREA,sigdata,sizeof(bl_sig_t));

	/*LOCK flash operation*/
	bootloader_lock_flash();

}


BL_StatusTypedef bootloader_unlock_flash(){
	HAL_FLASH_Unlock();
	CLEAR_BIT(FLASH->CR, FLASH_CR_PSIZE);
	FLASH->CR |= FLASH_PSIZE_WORD;
	FLASH->CR |= FLASH_CR_PG;
}
BL_StatusTypedef bootloader_lock_flash(){
	/*disable programming enable bit*/
	FLASH->CR &= (~FLASH_CR_PG);
	HAL_FLASH_Lock();
}
