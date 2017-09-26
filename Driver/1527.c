#include <string.h>
#include "1527.h"
#include "Misc.h"

//����LED�˿�
#define LED_PORT                GPIOD
#define LED_PINS                GPIO_Pin_0
#define KEY_PORT                GPIOB
#define KEY_PINS                GPIO_Pin_1

volatile uint8_t Ev1527RecvOkFlag = 0;
uint8_t  Ev1527RecvBuf[12];
uint8_t  DevRfAddr[3];

void EV1527_Init( void )
{
    /*�ⲿ�ж�*/
    GPIO_Init( KEY_PORT, KEY_PINS, GPIO_Mode_In_PU_IT );
    //GPIO_Init( GPIOC, GPIO_Pin_1, GPIO_Mode_In_PU_IT );
    EXTI_DeInit();                                             //�ָ��жϵ��������� 
    EXTI_SetPinSensitivity( EXTI_Pin_1, EXTI_Trigger_Falling );   //�ⲿ�ж�1���½��ش�����������9 
    FLASH_ReadData( EV1527_CFG_SAVE_ADDR, DevRfAddr, 3 );
    GPIO_Init( LED_PORT, LED_PINS, GPIO_Mode_Out_PP_High_Fast ); //��ʼ��LED�˿�
}

void EV1527_SaveSelfAddr( uint8_t NewAddr[3] )
{
    FLASH_WriteData( EV1527_CFG_SAVE_ADDR, NewAddr, 3 );
    memcpy( DevRfAddr, NewAddr, 3 );
}

int8_t EV1527_RecvBit( void )
{
    uint16_t T;        //��ƽʱ��
    
    //ÿ����λӦ�ö����Ըߵ�ƽ��ʼ�����Ϊ�͵�ƽ�����
    while( GPIO_ReadInputDataBit( EV1527_INT_PORT, EV1527_INT_PIN ) == RESET );
    
    //���ոߵ�ƽ��ͳ��ʱ��
    TIM2_SetCounter( 0x0000 );
    TIM2_Cmd( ENABLE );
    while( GPIO_ReadInputDataBit( EV1527_INT_PORT, EV1527_INT_PIN ) == SET );
    TIM2_Cmd( DISABLE );
    T = TIM2_GetCounter();
    if( T > 12 )       //��ʱ���������ߵ�ƽʱ��
    {
        return -1;
    }
    if( T > 4 )        //�ߵ�ƽ����4TΪ��λ1
    {
        return EV1527_BIT_1;
    }
    
    //���յ͵�ƽ��һ���ж��Ƿ�Ϊͬ����
    TIM2_SetCounter( 0x0000 );
    TIM2_Cmd( ENABLE );
    while( GPIO_ReadInputDataBit( EV1527_INT_PORT, EV1527_INT_PIN ) == RESET );
    TIM2_Cmd( DISABLE );
    T = TIM2_GetCounter();
    if( T > 124 )
    {
        return -1;
    }
    if( T > 12 )     //ͬ����
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
    
    //���յ�ַ��(ǰ10����λ)
    for( i=0; i<20; i++)
    {
        ThisBit = EV1527_RecvBit();
        if( ThisBit < 0 )
        {
            GOTO_RET( _lRet, Ret, -1 );
        }
    }

    //����������(��2����λ)
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
        //ƥ�����
        switch( Ev1527RecvBuf[3] )
        {
            //����ʾ,������ʾ����
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