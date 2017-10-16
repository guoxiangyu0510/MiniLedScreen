#include <string.h>
#include "1527.h"
#include "Misc.h"



volatile uint8_t Ev1527RecvOkFlag = 0;
volatile uint8_t Ev1527EventKey = EV1527_KEY_NONE;

uint8_t  Ev1527RecvBuf[3];
uint8_t  RfSelfAddr[3];

void EV1527_Init( void )
{
    /*外部中断*/
    GPIO_Init( EV1527_INT_PORT, EV1527_INT_PIN, GPIO_Mode_In_PU_IT );   //上拉输入
    EXTI_DeInit();                                                      //恢复中断的所有设置 
    EXTI_SetPinSensitivity( EXTI_Pin_1, EXTI_Trigger_Falling );         //外部中断1，下降沿触发，向量号9
    
    CLK_PeripheralClockConfig( CLK_Peripheral_TIM3, ENABLE );
    TIM3_DeInit();
    TIM3_TimeBaseInit( TIM3_Prescaler_16, TIM3_CounterMode_Up, 0xFFFF );
    TIM3_ARRPreloadConfig( ENABLE );
    TIM3_ClearFlag( TIM3_FLAG_Update );
    FLASH_ReadData( EV1527_CFG_SAVE_ADDR, RfSelfAddr, 3 );
}

void EV1527_SetPower( uint8_t Pwr )
{
    if( Pwr )
    {
        GPIO_Init( EV1527_INT_PORT, EV1527_INT_PIN, GPIO_Mode_In_PU_IT );
    }
    else
    {
        GPIO_Init( EV1527_INT_PORT, EV1527_INT_PIN, GPIO_Mode_In_PU_No_IT );
    }
}

void EV1527_SaveSelfAddr( uint8_t NewAddr[3] )
{
    FLASH_WriteData( EV1527_CFG_SAVE_ADDR, NewAddr, 3 );
    memcpy( RfSelfAddr, NewAddr, 3 );
}

//统计当前电平状态时间
static uint16_t EV1527_BitTime( BitStatus Bit )
{ 
    int8_t  Time = 0;
        
    if( ((EV1527_INT_PORT->IDR & (uint8_t)EV1527_INT_PIN)?SET:RESET) != Bit )
    {
        return 0;
    } 
    
    TIM3_SetCounter( 0x0000 ); 
    TIM3_Cmd( ENABLE );  
    while( ((EV1527_INT_PORT->IDR & (uint8_t)EV1527_INT_PIN)?SET:RESET) == Bit )
    {
        if( TIM3_GetFlagStatus( TIM3_FLAG_Update) == SET )
        {
            TIM3_ClearFlag( TIM3_FLAG_Update );
            goto _lRet;
        }
    }
    Time = TIM3_GetCounter();
_lRet:
    TIM3_Cmd( DISABLE );
    return Time;
}

#define EV1527_HIGH     RESET
#define EV1527_LOW      SET

static int8_t EV1527_RecvBit( void )
{
    uint16_t TH,TL;        //电平时间
    
    //每个码位应该都是以高电平开始 
    TH = EV1527_BitTime( EV1527_HIGH );
    if( TH == 0 )
    {
        return -1;
    }
    
    TL = EV1527_BitTime( EV1527_LOW );
    if( TL == 0 )
    {
        return -1;
    }
    
    if( TH <= 15 || TL <= 15 || TH >= 120|| TL >= 120 )
    {
        return -1;
    }
    
    if( TH < 37 )
    {
        if( TL > 80 )
        {
            return EV1527_BIT_0;
        }
    }
    else if( TH > 80 )
    {
        if( TL < 37 )
        {
            return EV1527_BIT_1;
        }
    }
    else
    {
    }
    return -1;
}

int8_t EV1527_RecvData( void )
{
    int8_t   BitCnt;
    uint8_t  ByteCnt;
    
    ByteCnt = 0;
    BitCnt = 0;
    while( ByteCnt < 3 )
    {
        switch( EV1527_RecvBit() )
        {
            //同步码
            case EV1527_BIT_SYN:
                BitCnt = 0;
                ByteCnt = 0;
                break;
            case EV1527_BIT_1:
                Ev1527RecvBuf[ByteCnt] <<= 1;
                Ev1527RecvBuf[ByteCnt] |= 1;
                BitCnt ++;
                break;
            case EV1527_BIT_0:
                Ev1527RecvBuf[ByteCnt] <<= 1;
                BitCnt ++;
                break;
            default:
                return -1;
        }
        if( BitCnt >= 8 )
        {
            BitCnt = 0;
            ByteCnt ++;
        }
    }
    //if( Ev1527RecvBuf[0] == RfSelfAddr[0] && 
    //    Ev1527RecvBuf[1] == RfSelfAddr[1] &&
    //    Ev1527RecvBuf[2]&0xF0 == RfSelfAddr[2]&0xF0 )
    //{
        Ev1527EventKey = Ev1527RecvBuf[2] & 0x0F;
        Ev1527RecvOkFlag = 1;
    //}
    return 0;   
}

uint8_t* EV1257_GetRecvBuf( void )
{
    return Ev1527RecvBuf;
}

uint8_t EV1257_GetEventKey( void )
{
    if( Ev1527RecvOkFlag )
    {
        return Ev1527EventKey;
    }
    return EV1527_KEY_NONE;
}

uint8_t EV1257_IsReceived( void )
{
    return Ev1527RecvOkFlag;
}

void EV1257_RecvNext( void )
{
    memset( Ev1527RecvBuf, 0x00, sizeof(Ev1527RecvBuf) );
    Ev1527RecvOkFlag = 0;
    Ev1527EventKey = EV1527_KEY_NONE;
}