#ifndef _MISC_H_
#define _MISC_H_

#include "stm8l15x.h"

extern volatile uint8_t Clk500msFlag;
extern volatile uint8_t Clk2msCnt;

extern void Misc_Init( void );
extern void Delay(__IO uint16_t nCount);
extern void DelayMs( __IO uint16_t nCount );
extern void SysClk2msTick( void );
extern void FLASH_WriteData( uint32_t Address, uint8_t* Buf, uint16_t Len );
extern void FLASH_ReadData( uint32_t Address, uint8_t* Buf, uint16_t Len );

#endif