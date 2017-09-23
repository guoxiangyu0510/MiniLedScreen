#include "stm8l15x.h"//STM8L051/151��ϵ�й��ÿ⺯��
#include "Picture.h"
#include "stdlib.h"

#define GOTO_RET(LB,RV,RC) do{(RV)=(RC);goto LB;}while(0);

//����LED�˿�
#define LED_PORT                GPIOD
#define LED_PINS                GPIO_Pin_0
#define KEY_PORT                GPIOB
#define KEY_PINS                GPIO_Pin_1

//74HC595��ض���
#define HC595_SET_STCP(VAL)     GPIO_WriteBit(GPIOB,GPIO_Pin_3,VAL)       //�������������ʱ��
#define HC595_SET_OE(VAL)       GPIO_WriteBit(GPIOB,GPIO_Pin_2,VAL)       //�������ʹ�ܵ�ƽ
#define HC595_SET_RST(VAL)      GPIO_WriteBit(GPIOA,GPIO_Pin_1,VAL)       //���ø�λʹ�ܵ�ƽ
#define HC595_SET_DATA(DAT)     SPI1->DR = DAT                            //������λ�Ĵ�������

//EV1527��ض���
#define EV1527_BIT_0            0
#define EV1527_BIT_1            1
#define EV1527_BIT_SYN          2

#define EV1527_INT_PORT         GPIOC
#define EV1527_INT_PIN          GPIO_Pin_1

//LED��ʾ����ض���
#define LEDSCRN_ON              RESET
#define LEDSCRN_OFF             SET
#define LEDSCRN_SET_LUM(LUM)    TIM3_SetCompare2( LUM );                 //������Ļ����
#define LEDSCRN_SET_STAT(STAT)  HC595_SET_OE(STAT)                       //������Ļ״̬

typedef struct 
{
    GPIO_TypeDef*       Port;
    GPIO_Pin_TypeDef    Pin;
}Hang_t;

typedef enum
{
    Normal,
    Reversal,
}LedScrnDispMode;

//��ѡ
const Hang_t Hang[] = {
{ GPIOC, GPIO_Pin_5 },
{ GPIOC, GPIO_Pin_6 },
{ GPIOA, GPIO_Pin_2 },
{ GPIOA, GPIO_Pin_3 },
{ GPIOD, GPIO_Pin_0 },
{ GPIOB, GPIO_Pin_0 },
{ GPIOB, GPIO_Pin_1 },
{ GPIOB, GPIO_Pin_7 }};

volatile uint8_t Ev1527RecvOkFlag = 0;
uint16_t Pwm = 50;
uint8_t  DispBuf[ 3*3*8 ];

void Delay(__IO uint16_t nCount)
{
    /* Decrement nCount value */
    while (nCount != 0)
    {
        nCount--;
    }
}

uint32_t abc(uint32_t sysclk)
{
    return sysclk+0;
}

void BoardInit( void )
{
    volatile uint32_t SysClk;
    
    /*��ʾɨ��ʱ��*/
    SysClk = CLK_GetClockFreq();
    
    CLK_PeripheralClockConfig( CLK_Peripheral_TIM4, ENABLE );
    
    TIM4_DeInit();
    
    TIM4_TimeBaseInit( TIM4_Prescaler_32768, (SysClk>>TIM4_Prescaler_32768)-1 );
    
    TIM4_ITConfig( TIM4_IT_Update, ENABLE );
    
    TIM4_ARRPreloadConfig( ENABLE );
    
    TIM4_Cmd( ENABLE );
    
    
    /*�ⲿ�ж�*/
    GPIO_Init( KEY_PORT, KEY_PINS, GPIO_Mode_In_PU_IT );
    //GPIO_Init( GPIOC, GPIO_Pin_1, GPIO_Mode_In_PU_IT );
    EXTI_DeInit();                                             //�ָ��жϵ��������� 
    EXTI_SetPinSensitivity( EXTI_Pin_1, EXTI_Trigger_Falling );   //�ⲿ�ж�1���½��ش�����������9
    
    
    GPIO_Init( LED_PORT, LED_PINS, GPIO_Mode_Out_PP_High_Fast ); //��ʼ��LED�˿�
    
    
    /*PWM����*/  
    CLK_PeripheralClockConfig( CLK_Peripheral_TIM3, ENABLE );
    
    TIM3_DeInit();
    
    TIM3_TimeBaseInit( TIM3_Prescaler_32, TIM3_CounterMode_Up, 255 );
    
    TIM3_ARRPreloadConfig( ENABLE );
    
    TIM3_OC2Init( TIM3_OCMode_PWM1, TIM3_OutputState_Enable, Pwm, TIM3_OCPolarity_High, TIM3_OCIdleState_Set );
    
    TIM3_CtrlPWMOutputs( ENABLE );
    
    TIM3_Cmd( ENABLE );
    
    //дFLASHʹ��
    FLASH_DeInit();
    
    FLASH_Unlock( FLASH_MemType_Data );
    
    //FLASH_EraseByte(0x1000);
    
    //FLASH_ProgramByte(0x1000, 0x44);
    
    SysClk = FLASH_ReadByte( 0x1000 );
    Delay(SysClk);
    
    FLASH_Lock( FLASH_MemType_Data );
    
    
    enableInterrupts();
}
#if 0
void main(void)
{
    int16_t jsq=0;
    
    BoardInit();
    while (1)
    {

        Pwm = (uint16_t)abs(jsq-255);
        
        TIM3_SetCompare2( Pwm );
        jsq++;
        if(jsq>=510)
        {
            jsq = 0;
        }
        Delay( 500 );
    }
}
#endif

void main(void)
{
    BoardInit();
    
    while(1)
    {
        if( Ev1527RecvOkFlag )
        {
            
            Ev1527RecvOkFlag = 0;
        } 
    }
}

void LedScreen_ShowImage( uint8_t ImgBuf[72], LedScrnDispMode DispMode )
{
    uint8_t Cnt;
    
    for( Cnt=0; Cnt<72; Cnt++ )
    {
        DispBuf[Cnt] = (DispMode == Normal)? ImgBuf[Cnt]:~ImgBuf[Cnt];
    }
}

void LedScreen_Scan( void )
{
    static uint8_t H_Count=0;
    uint8_t i;
    
    //����������
    HC595_SET_STCP( RESET );
    for( i=0; i<9; i++ )
    {
        SPI1->SR = (uint8_t)(~SPI_FLAG_TXE);
        SPI1->DR = DispBuf[ i + H_Count ];
        while( (SPI1->SR & (uint8_t)SPI_FLAG_TXE) != SET );
    }
    
    //����
    HC595_SET_OE( SET );

    HC595_SET_STCP( SET );
    (Hang[H_Count].Port)->ODR |= Hang[H_Count].Pin;
    
    HC595_SET_OE( RESET );

    H_Count += 9;
    if( H_Count >= 72 )
    {
        H_Count = 0;
    }
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




#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
    /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

    /* Infinite loop */
    while (1)
    {
    }
}
#endif

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
