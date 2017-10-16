#include "Misc.h"

volatile uint8_t Clk500msFlag = 0;
volatile uint8_t Clk2msCnt = 0;


void Misc_Init( void )
{
    //Ð´FLASHÊ¹ÄÜ
    FLASH_DeInit();
}

void Delay(__IO uint16_t nCount)
{
    /* Decrement nCount value */
    while ( nCount != 0 )
    {
        nCount--;
    }
}

void DelayMs( __IO uint16_t nCount )
{
    while(nCount--)
    {
        Delay( 60000 );
    }
}

void SysClk2msTick( void )
{
    Clk2msCnt ++;
    if( Clk2msCnt >= 250 )
    {
        Clk2msCnt = 0;
        Clk500msFlag = 1;
    }
}

void FLASH_WriteData( uint32_t Address, uint8_t* Buf, uint16_t Len )
{
    uint16_t i;
    
    FLASH_Unlock( FLASH_MemType_Data );
    FLASH_EraseByte( Address ); 
    for( i=0; i<Len; i++ )
    {
        FLASH_ProgramByte( Address, Buf[i] );
    }
    FLASH_Lock( FLASH_MemType_Data );
}

void FLASH_ReadData( uint32_t Address, uint8_t* Buf, uint16_t Len )
{
    uint16_t i;
    
    for( i=0; i<Len; i++ )
    {
        Buf[i] = FLASH_ReadByte( Address );
    }
}
