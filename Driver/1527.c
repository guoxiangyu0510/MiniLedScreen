#include <string.h>
#include "1527.h"
#include "Misc.h"

//定义LED端口
#define LED_PORT                GPIOD
#define LED_PINS                GPIO_Pin_0
#define KEY_PORT                GPIOB
#define KEY_PINS                GPIO_Pin_1

volatile uint8_t Ev1527RecvOkFlag = 0;
uint8_t  Ev1527RecvBuf[12];
uint8_t  DevRfAddr[3];

void EV1527_Init( void )
{
    /*外部中断*/
    GPIO_Init( KEY_PORT, KEY_PINS, GPIO_Mode_In_PU_IT );
    //GPIO_Init( GPIOC, GPIO_Pin_1, GPIO_Mode_In_PU_IT );
    EXTI_DeInit();                                             //恢复中断的所有设置 
    EXTI_SetPinSensitivity( EXTI_Pin_1, EXTI_Trigger_Falling );   //外部中断1，下降沿触发，向量号9 
    FLASH_ReadData( EV1527_CFG_SAVE_ADDR, DevRfAddr, 3 );
    GPIO_Init( LED_PORT, LED_PINS, GPIO_Mode_Out_PP_High_Fast ); //初始化LED端口
}

void EV1527_SaveSelfAddr( uint8_t NewAddr[3] )
{
    FLASH_WriteData( EV1527_CFG_SAVE_ADDR, NewAddr, 3 );
    memcpy( DevRfAddr, NewAddr, 3 );
}

int8_t EV1527_RecvBit( void )
{
    uint16_t T;        //电平时间
    
    //每个码位应该都是以高电平开始，如果为低电平则过滤
    while( GPIO_ReadInputDataBit( EV1527_INT_PORT, EV1527_INT_PIN ) == RESET );
    
    //接收高电平，统计时间
    TIM2_SetCounter( 0x0000 );
    TIM2_Cmd( ENABLE );
    while( GPIO_ReadInputDataBit( EV1527_INT_PORT, EV1527_INT_PIN ) == SET );
    TIM2_Cmd( DISABLE );
    T = TIM2_GetCounter();
    if( T > 12 )       //超时，非正常高电平时间
    {
        return -1;
    }
    if( T > 4 )        //高电平大于4T为码位1
    {
        return EV1527_BIT_1;
    }
    
    //接收低电平进一步判断是否为同步码
    TIM2_SetCounter( 0x0000 );
    TIM2_Cmd( ENABLE );
    while( GPIO_ReadInputDataBit( EV1527_INT_PORT, EV1527_INT_PIN ) == RESET );
    TIM2_Cmd( DISABLE );
    T = TIM2_GetCounter();
    if( T > 124 )
    {
        return -1;
    }
    if( T > 12 )     //同步码
    {
        return EV1527_BIT_SYN;
    }
    return EV1527_BIT_0;   
}

int8_t EV1527_Recv( uint8_t* RecvBuf )
{
    int8_t   ThisBit;
    int8_t   Ret;
    uint8_t  i;
    
    //接收地址码(前10个码位)
    for( i=0; i<20; i++)
    {
        ThisBit = EV1527_RecvBit();
        if( ThisBit < 0 )
        {
            GOTO_RET( _lRet, Ret, -1 );
        }
    }

    //接收数据码(后2个码位)
    for( i=0; i<4; i++)
    {
        TIM2_SetCounter( 0x0000 );
    } 
_lRet:
    
    return Ret;   
}

int8_t EV1257_IsReceived( void )
{
    if( memcmp( Ev1527RecvBuf, DevRfAddr, 3 ) == 0 )
    {
        //匹配键码
        switch( Ev1527RecvBuf[3] )
        {
            //开显示,设置显示参数
        case EV1527_KEY0:
            break;
        case EV1527_KEY1:
            break;
        case EV1527_KEY2:
            break;
        case EV1527_KEY3:
            break;
        default:
            break;
        }
        if( 1 )
        {
            //LedScrnAutoSetLum();
        }
    }
    return 0;
}

uint8_t EV1257_GetRecvData( void )
{
    return Ev1527RecvBuf[3];
}

void EV1257_ClearRecvBuf( void )
{
    memset( Ev1527RecvBuf, 0x00, sizeof(Ev1527RecvBuf) );
    Ev1527RecvOkFlag = 0;
}