
#include <string.h>
#include <stdio.h>
#include "ds18b20.h"
//#include "my_config.h"

/* La calibracion de CALIB debe dar 786us */ /* 1200 * n, 85<=n<=105 */
//#define BAUDRATE 105600 // 103448 // 115200  (15.52ms GOOD).
//#define BAUDRATE 103448 // 
//#define BAUDRATE 115200    
//#define BAUDRATE 124800
#define BAUDRATE 134400

//#define BAUDRATE 135600
//#define BAUDRATE 136800 // good
//#define BAUDRATE 138000

//12000 -> 416us
// 9600 -> 520us
// 7400 -> 676us
#define BR_RESET 9600//10800

/*
 * USART2(Tx=D5 Rx=D6)
 */


typedef struct {
    __IO uint8_t Reset;              //Communication Phase 1: Reset
    __IO uint8_t ROM_Command;        //Communication Phase 2: Rom command
    __IO uint8_t Function_Command;   //Communication Phase 3: DS18B20 function command
    __IO uint8_t *ROM_TxBuffer;
    __IO uint8_t *ROM_RxBuffer;
    __IO uint8_t ROM_TxCount;
    __IO uint8_t ROM_RxCount;
    __IO uint8_t *Function_TxBuffer;
    __IO uint8_t *Function_RxBuffer;
    __IO uint8_t Function_TxCount;
    __IO uint8_t Function_RxCount;
    __IO uint8_t ROM;
    __IO uint8_t Function;
} State;

typedef struct {
		void(*OnComplete)(void);
		void(*OnErr)(void);
} OneWire_Callback;


static void OneWire_UARTInit(uint32_t baudRate);
static void StateMachine(void);
static uint8_t ROMStateMachine(void);
static uint8_t FunctionStateMachine(void);

static State _state;
static uint8_t internal_Buffer[73]; /* a mi me da 72 */


volatile static uint8_t  complete_on_wire = 0;

static UART_HandleTypeDef *_ow_huart = NULL;

__IO OneWire_Callback onewire_callback = {
	.OnComplete = NULL,
	.OnErr = NULL,
};


uint8_t OneWire_complete(void)
{
	return complete_on_wire;
}


void OneWire_SetCallback(void(*OnComplete)(void), void(*OnErr)(void))
{
	onewire_callback.OnErr = OnErr;
	onewire_callback.OnComplete = OnComplete;
}


void OneWire_Init(UART_HandleTypeDef *huart)
{
	_ow_huart = huart;
	OneWire_UARTInit(BR_RESET);
}

USART_TypeDef *OneWire_UART_Instance(void)
{
	return _ow_huart->Instance;
}

// Declare a USART_HandleTypeDef handle structure. 
static void OneWire_UARTInit(uint32_t baudRate)
{
//    _ow_huart->Instance=USART2;
      _ow_huart->Init.BaudRate = baudRate;
//    _ow_huart->Init.WordLength = UART_WORDLENGTH_8B;
//    _ow_huart->Init.StopBits = UART_STOPBITS_1;
//    _ow_huart->Init.Parity = UART_PARITY_NONE;
//    _ow_huart->Init.Mode = UART_MODE_TX_RX;
//    _ow_huart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
//    _ow_huart->Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(_ow_huart);
    return ;
}


void OneWire_TxCpltCallback(){
	return;
}


void OneWire_RxCpltCallback(){
        StateMachine();
	
	return;
}

 /* OneWire_SendBytes & OneWire_ReadBytes */
static void StateMachine()
{
	switch (_state.Reset){
        case 0: // start the reset produce;
		OneWire_UARTInit(BR_RESET);
		internal_Buffer[0]=0xf0;
		HAL_UART_Transmit_DMA(_ow_huart, internal_Buffer, 1);
		HAL_UART_Receive_DMA(_ow_huart, internal_Buffer, 1);
		_state.Reset++;
		break;
        case 1: // to check if the device exist or not.
		if (internal_Buffer[0]==0xf0)
		{
			if ( onewire_callback.OnErr != NULL )
				onewire_callback.OnErr();
			break;
		}
		_state.Reset++;
        case 2:
		if (ROMStateMachine() == 0 )
			_state.Reset++;
		else break;
        case 3:
		if (FunctionStateMachine() == 0 )
			_state.Reset++;
		else break;
        case 4:
		complete_on_wire = 1;
		if ( onewire_callback.OnComplete != NULL )
			onewire_callback.OnComplete();
		break;
	}
	return ;
}



static uint8_t ROMStateMachine(void)
{
	switch (_state.ROM) {
        case 0: // start the ROM command by sending the ROM_Command
		OneWire_UARTInit(BAUDRATE);
		for (uint8_t i=0;i<8;i++)
			internal_Buffer[i]=((_state.ROM_Command >> i) & 0x01)?0xff:0x00;
		HAL_UART_Transmit_DMA(_ow_huart,internal_Buffer,8);
		HAL_UART_Receive_DMA(_ow_huart,internal_Buffer,8);
		_state.ROM++;
		break;
        case 1: // continue by sending necessary Tx buffer
		if ( _state.ROM_TxCount != 0 ){
			for (uint8_t i=0;i<_state.ROM_TxCount;i++)
				for (uint8_t j=0;j<8;j++)
					internal_Buffer[i*8+j]=((_state.ROM_TxBuffer[i]>>j)&0x01)?0xff:0x00;
			HAL_UART_Transmit_DMA(_ow_huart,internal_Buffer,_state.ROM_TxCount*8);
			HAL_UART_Receive_DMA(_ow_huart,internal_Buffer,_state.ROM_TxCount*8);
			_state.ROM++;
			break;							
		}
		if ( _state.ROM_RxCount !=0 ){
			for (uint8_t i=0;i<=_state.ROM_RxCount*8;i++)
				internal_Buffer[i]=0xff;
			HAL_UART_Transmit_DMA(_ow_huart,internal_Buffer,_state.ROM_RxCount*8);
			HAL_UART_Receive_DMA(_ow_huart,internal_Buffer,_state.ROM_RxCount*8);
			_state.ROM++;
			break;
		} 
		_state.ROM++;
        case 2:
		if ( _state.ROM_RxCount != 0 ){
			for (uint8_t i=0;i<_state.ROM_RxCount;i++) {
				_state.ROM_RxBuffer[i] = 0;
				for (uint8_t j=0;j<8;j++) {
					//_state.ROM_RxBuffer[i]=(_state.ROM_RxBuffer[i])+(((internal_Buffer[i*8+j]==0xff)?0x01:0x00)<<j);
					if ( internal_Buffer[(i<<3)+j] == 0xff ) {
						_state.ROM_RxBuffer[i] |= 1 << j;
					}
				}
			}
		}
		_state.ROM = 0;
		break;
        }
	
	return _state.ROM;
}


static uint8_t FunctionStateMachine(void)
{
	switch(_state.Function){
        case 0: 
		OneWire_UARTInit(BAUDRATE);
		for (uint8_t i=0;i<8;i++)
			internal_Buffer[i]=((_state.Function_Command>>i)&0x01)?0xff:0x00;
		HAL_UART_Transmit_DMA(_ow_huart,internal_Buffer,8);
		HAL_UART_Receive_DMA(_ow_huart,internal_Buffer,8);
		_state.Function++;
		break;
        case 1: // continue by sending necessary Tx buffer
		if ( _state.Function_TxCount != 0 ) {
			for (uint8_t i=0; i < _state.Function_TxCount; i++)
				for (uint8_t j=0; j < 8; j++)
					internal_Buffer[(i<<3)|j] = ((_state.Function_TxBuffer[i]>>j)&0x01)?0xff:0x00;
			HAL_UART_Transmit_DMA(_ow_huart, internal_Buffer, _state.Function_TxCount << 3);
			HAL_UART_Receive_DMA(_ow_huart, internal_Buffer, _state.Function_TxCount << 3);
			_state.Function++;
			break;
		}
		if ( _state.Function_RxCount != 0 ){
			for (uint8_t i=0; i <= _state.Function_RxCount << 3; i++)
				internal_Buffer[i]=0xff;
			HAL_UART_Transmit_DMA(_ow_huart, internal_Buffer, _state.Function_RxCount << 3 );
			HAL_UART_Receive_DMA(_ow_huart, internal_Buffer, _state.Function_RxCount << 3 );
			_state.Function++;
			break;
		}
		_state.Function++;
	case 2: 
		if (_state.Function_RxCount!=0){
			for (uint8_t i=0;i<_state.Function_RxCount;i++) {
				_state.Function_RxBuffer[i] = 0;
				for (uint8_t j=0;j<8;j++) {
					//_state.Function_RxBuffer[i] = _state.Function_RxBuffer[i] + (((internal_Buffer[i*8+j]==0xff)?0x01:0x00)<<j);
				    
					if ( internal_Buffer[(i<<3)+j] == 0xff ) {
						_state.Function_RxBuffer[i] |= 1<<j;
					}
				    
				}
			}
		}
		_state.Function=0;
		break;
	}
	return _state.Function;
}


void OneWire_Execute(uint8_t ROM_Command,uint8_t* ROM_Buffer,uint8_t Function_Command,uint8_t* Function_buffer)
{
	complete_on_wire = 0;
	memset(&_state, 0, sizeof(State));
	_state.ROM_Command=ROM_Command;
	_state.Function_Command=Function_Command;

	switch (ROM_Command) {
        case 0x33: /* Read ROM: */
		_state.ROM_RxBuffer=ROM_Buffer;
		_state.ROM_RxCount=8; //8 byte
		break;
        case 0x55: /* Match ROM: */
		_state.ROM_TxBuffer=ROM_Buffer;
		_state.ROM_TxCount=8; 
		break;
		
        case 0xf0: /* Search ROM it might be too hard to implement you
		    * might need to refer to Chapter "C.3. Search ROM
		    * Command" in the pdf
		    * here:http://pdfserv.maximintegrated.com/en/an/AN937.pdf */
		
		break;
		
        case 0xec: /* Alarm Search it might be too hard to implement
		    * refer to
		    * http://pdfserv.maximintegrated.com/en/an/AN937.pdf
		    * if in need. */
		break;
        case 0xcc: /* Skip Rom just send the 0xcc only since the code
		    * is implement one-slave need. */
		break;
	}
	
	switch (Function_Command) { 
        case 0x44: /* Convert T need to transmit nothing or we can
		    * read a 0 while the temperature is in progress
		    * read a 1 while the temperature is done. */
		break;
        case 0x4e: /* Write Scratchpad: */
		_state.Function_TxBuffer=Function_buffer;
		_state.Function_TxCount=3; 
		break;
        case 0x48: /* Copy Scratchpad need to transmit nothing: */
		break;
        case 0xbe: /* Read Scratchpad: */
		_state.Function_RxBuffer=Function_buffer;
		_state.Function_RxCount=9; 
		break;
        case 0xb8: /* Recall EEPROM return transmit status to master 0
		    * for in progress and 1 is for done. */
		break;
        case 0xb4: /* Read power supply only work for undetermined
		    * power supply status. so don't need to implement
		    * it.*/
		break;
	}
	
	StateMachine();
}





void OW_adq(uint8_t *rom, uint8_t *buff)
{
	memset(buff, 0, 8);
	OneWire_Execute(0x55, rom, 0xbe, buff);
	
	return;
}

