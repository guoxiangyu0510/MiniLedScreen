#include "LedScreen.h"
#include "string.h"

uint8_t  DispBuf[ 3*3*8 ];

//行选
const Hang_t Hang[] = {
{ GPIOC, GPIO_Pin_5 },
{ GPIOC, GPIO_Pin_6 },
{ GPIOA, GPIO_Pin_2 },
{ GPIOA, GPIO_Pin_3 },
{ GPIOD, GPIO_Pin_0 },
{ GPIOB, GPIO_Pin_0 },
{ GPIOB, GPIO_Pin_1 },
{ GPIOB, GPIO_Pin_7 }};


void LedScreen_Init( void )
{
    //uint32_t SysClk;
    uint16_t Pwm = 0;
    
    memset( DispBuf, 0x00, sizeof(DispBuf) );
    
    /*SPI控制器使能*/
    GPIO_Init(GPIOB, GPIO_Pin_5, GPIO_Mode_Out_PP_High_Fast);
    GPIO_Init(GPIOB, GPIO_Pin_6, GPIO_Mode_Out_PP_High_Fast);
    CLK_PeripheralClockConfig( CLK_Peripheral_SPI1, ENABLE );  
    SPI_DeInit( SPI1 );
    SPI_Init( SPI1, SPI_FirstBit_MSB, SPI_BaudRatePrescaler_2, SPI_Mode_Master, SPI_CPOL_Low, SPI_CPHA_1Edge, SPI_Direction_1Line_Tx, SPI_NSS_Soft, 0x00 );
    SPI_Cmd( SPI1, ENABLE );
    
    /*显示扫描时钟 500HZ (2ms)*/
    //SysClk = CLK_GetClockFreq();
    CLK_PeripheralClockConfig( CLK_Peripheral_TIM4, ENABLE );   
    TIM4_DeInit();   
    TIM4_TimeBaseInit( TIM4_Prescaler_16, 256-1 ); 
    TIM4_ITConfig( TIM4_IT_Update, ENABLE ); 
    TIM4_ARRPreloadConfig( ENABLE );
    TIM4_Cmd( ENABLE );
    
    /*显示亮度PWM控制*/  
    CLK_PeripheralClockConfig( CLK_Peripheral_TIM3, ENABLE );
    TIM3_DeInit();
    TIM3_TimeBaseInit( TIM3_Prescaler_8, TIM3_CounterMode_Up, 255 );
    TIM3_ARRPreloadConfig( ENABLE );
    TIM3_OC2Init( TIM3_OCMode_PWM1, TIM3_OutputState_Enable, Pwm, TIM3_OCPolarity_High, TIM3_OCIdleState_Set );
    TIM3_CtrlPWMOutputs( ENABLE );
    TIM3_Cmd( ENABLE );
    
    /*环境亮度采集ADC*/
    CLK_PeripheralClockConfig( CLK_Peripheral_ADC1, ENABLE );
    ADC_DeInit( ADC1 );
    ADC_Init( ADC1, ADC_ConversionMode_Single, ADC_Resolution_8Bit, ADC_Prescaler_2 );
    ADC_ChannelCmd( ADC1, ADC_Channel_4, ENABLE );
    ADC_SamplingTimeConfig( ADC1, ADC_Group_SlowChannels, ADC_SamplingTime_24Cycles );
    ADC_Cmd( ADC1, ENABLE );
    
}

void LedScreen_ShowImage( uint8_t ImgBuf[72], LedScrnDispMode DispMode )
{
    uint8_t Cnt;
    
    for( Cnt=0; Cnt<72; Cnt++ )
    {
        DispBuf[Cnt] = (DispMode == Normal)? ImgBuf[Cnt]:~ImgBuf[Cnt];
    }
}

/*自动设置LED屏幕亮度*/
void LedScrnAutoSetLum( void )
{
    uint16_t ADC_Val;
    
    //读取环境亮度
    ADC_SoftwareStartConv( ADC1 );
    while( ADC_GetFlagStatus( ADC1, ADC_FLAG_EOC ) != SET );
    ADC_Val = ADC_GetConversionValue( ADC1 );
    if( ADC_Val > 255 )
    {
        ADC_Val = 255;
    }
    //设置目标亮度
    LEDSCRN_SET_LUM( ADC_Val );
}

void LedScreen_Scan( void )
{
    static uint8_t H_Count = 0;
    static uint8_t Pos = 0;
    uint8_t i;
    
    //先移入数据
    HC595_SET_STCP( RESET );
    for( i=0; i<9; i++ )
    {
        while( !(SPI1->SR & (uint8_t)SPI_FLAG_TXE) );
        SPI1->DR = DispBuf[ i + Pos ];      
    }
    
    //消隐
    HC595_SET_OE( SET );
    HC595_SET_STCP( SET );
    
    //行选
    (Hang[(H_Count-1)&0x07].Port)->ODR &= ~Hang[(H_Count-1)&0x07].Pin;
    (Hang[H_Count].Port)->ODR |= Hang[H_Count].Pin;
    
    HC595_SET_OE( RESET );

    H_Count ++;
    Pos += 9;
    if( H_Count >= 8 )
    {
        H_Count = 0;
        Pos = 0;
    }
}