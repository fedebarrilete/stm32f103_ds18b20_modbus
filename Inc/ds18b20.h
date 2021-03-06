
// https://cnnblike.com/post/stm32-OneWire/

#ifndef __DS18B20_H__
#define __DS18B20_H__

#include "main.h"

void OneWire_Init(UART_HandleTypeDef *huart);
USART_TypeDef *OneWire_UART_Instance(void);
uint8_t OneWire_complete(void);
//float OWBuffer_get_temp(uint8_t *buff, uint8_t *valid);
void OW_adq(uint8_t *rom, uint8_t *buff);
//int OW_buff_proc(THER18B20DATA *th, uint8_t *buff);

//void HAL_UART_TxCpltCallback(UART_HandleTypeDef *uarth);
//void HAL_UART_RxCpltCallback(UART_HandleTypeDef *uarth);
void OneWire_Execute(uint8_t ROM_Command,uint8_t* ROM_Buffer,
                     uint8_t Function_Command,uint8_t* Function_buffer);
void OneWire_SetCallback(void(*OnComplete)(void), void(*OnErr)(void));

void OneWire_TxCpltCallback(void);
void OneWire_RxCpltCallback(void);


#endif

