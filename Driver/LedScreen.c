#include "LedScreen.h"
#include "Misc.h"
#include "string.h"
#include "Picture.h"

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

const uint8_t ScanTab[9] = { 48,49,50,24,25,26,0,1,2 };
LedScrnPwr_t LedScrnPwrState = LedScrn_Init;
uint16_t Pwm = 0x20;

void LedScreen_Init( void )
{

    Delay( 60000 );
    GPIO_Init( GPIOB, GPIO_Pin_4, GPIO_Mode_Out_PP_Low_Slow );  //电源引脚
    GPIO_Init( GPIOB, GPIO_Pin_3, GPIO_Mode_Out_PP_Low_Slow );  //STCP引脚   
    GPIO_Init( GPIOB, GPIO_Pin_2, GPIO_Mode_Out_PP_Low_Slow );  //OE引脚
    GPIO_Init(GPIOB, GPIO_Pin_5, GPIO_Mode_Out_PP_High_Fast);   //DS和SHCP引脚对应SPI的DS和SCLK
    GPIO_Init(GPIOB, GPIO_Pin_6, GPIO_Mode_Out_PP_High_Fast);
    
    LedScreen_SetPower( LedScrn_Off ); 
    LedScreen_Clear();
    
    /*SPI控制器使能*/
    CLK_PeripheralClockConfig( CLK_Peripheral_SPI1, ENABLE );  
    SPI_DeInit( SPI1 );
    SPI_Init( SPI1, SPI_FirstBit_MSB, SPI_BaudRatePrescaler_2, SPI_Mode_Master, SPI_CPOL_Low, SPI_CPHA_1Edge, SPI_Direction_1Line_Tx, SPI_NSS_Soft, 0x00 );
       
    /*显示扫描时钟 500HZ (2ms)*/
    CLK_PeripheralClockConfig( CLK_Peripheral_TIM4, ENABLE );   
    TIM4_DeInit();   
    TIM4_TimeBaseInit( TIM4_Prescaler_16, 250-1 ); 
    TIM4_ARRPreloadConfig( ENABLE );
     
    /*显示亮度PWM控制*/  
    CLK_PeripheralClockConfig( CLK_Peripheral_TIM2, ENABLE );
    TIM2_DeInit();
    TIM2_TimeBaseInit( TIM2_Prescaler_8, TIM2_CounterMode_Up, 255 );
    TIM2_ARRPreloadConfig( ENABLE );
    TIM2_OC2Init( TIM2_OCMode_PWM1, TIM2_OutputState_Enable, Pwm, TIM2_OCPolarity_High, TIM2_OCIdleState_Set );   
    
    /*环境亮度采集ADC*/
    /*CLK_PeripheralClockConfig( CLK_Peripheral_ADC1, ENABLE );
    ADC_DeInit( ADC1 );
    ADC_Init( ADC1, ADC_ConversionMode_Single, ADC_Resolution_8Bit, ADC_Prescaler_2 );
    ADC_ChannelCmd( ADC1, ADC_Channel_4, ENABLE );
    ADC_SamplingTimeConfig( ADC1, ADC_Group_SlowChannels, ADC_SamplingTime_24Cycles );
    */
    
    Delay( 40000 );
}

void LedScreen_ShowImage( uint8_t ImgBuf[72], LedScrnDispMode DispMode )
{
    uint8_t Cnt;
    
    for( Cnt=0; Cnt<72; Cnt++ )
    {
        DispBuf[Cnt] = (DispMode == Normal)? ImgBuf[Cnt]:~ImgBuf[Cnt];
    }
}

void LedScreen_DrawPixel( uint8_t x, uint8_t y, uint8_t color )
{
    if( x > 24 || y > 24 )
    {
        return;
    }
    if( color )
    {
        DispBuf[y+y+y+(x>>3)] |= (0x80>>(x&0x07));
    }
    else
    {
        DispBuf[y+y+y+(x>>3)] &= ~(0x80>>(x&0x07));
    }
}

void LedScreen_PrintString( uint8_t x, uint8_t y, char *str )
{
    uint8_t *Pos;
    uint8_t xx,yy;
        
    if( x > 24 || y > 24 )
    {
        return;
    }
    while( *str != '\0' )
    {
        if( *str >= '0' && *str <= '9' )
        {
            Pos = (uint8_t*)(Ascii+(((*str)-'0')<<3));
        }
        else if( *str >= 'A' && *str <= 'Z' )
        {
            Pos = (uint8_t*)(Ascii+80+(((*str)-'A')<<3));
        }
        else if( *str >= 'a' && *str <= 'z' )
        {
            Pos = (uint8_t*)(Ascii+288+(((*str)-'a')<<3));
        }
        else
        {
            str++;
            continue;
        }
        
        if( x+5 >= 24 )
        {
            if( y+8 < 24)
            {
                x = 0;
                y +=8;
            }
            else
            {
                return;
            }
        }       
        for( yy=0; yy<8; yy++ )
        {
            for( xx=0; xx<5; xx++ )
            {
                LedScreen_DrawPixel( x+xx, y+yy, (*(Pos+yy))&(0x80>>xx) );
            }
        }
        x += 6;
        str++;
    }
}

void LedScreen_PrintHex( uint8_t x, uint8_t y, uint8_t *HexVector, uint8_t Len )
{
    char StrBuf[6];
    uint8_t Cnt;
    
    if( Len > 3 )
    {
        return;
    }
    for( Cnt=0; Cnt<(Len<<1); Cnt+=2 )
    {
        StrBuf[Cnt] = (HexVector[Cnt>>1]>>4) + '0';
        StrBuf[Cnt+1] = (HexVector[Cnt>>1]&0x0F) + '0';
    } 
    LedScreen_PrintString( x, y, StrBuf );
}

//只能画垂直线或者水平线
void LedScreen_DrawLine( uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t color )
{
    if( x1 != x2 && y1 != y2 )
    {
        return;
    }
    if( x1 == x2 )
    {
        if( y1 > y2 )
        {
            for( ; y1>=y2; y1-- )
            {
                LedScreen_DrawPixel( x1, y1, color );
                Delay( 4000 );
            }
        }
        else
        {
            for( ; y1<=y2; y1++ )
            {
                LedScreen_DrawPixel( x1, y1, color );
                Delay( 4000 );
            }
        }
    }
    else
    {
        if( x1 > x2 )
        {
            for( ; x1>=x2; x1-- )
            {
                LedScreen_DrawPixel( x1, y1, color );
                Delay( 4000 );
            }
        }
        else
        {
            for( ; x1<=x2; x1++ )
            {
                LedScreen_DrawPixel( x1, y1, color );
                Delay( 4000 );
            }
        }
    }
}

const uint8_t xy[4][2]={{7,7},{16,7},{16,16},{7,16}};
void LedScreen_PlayInitMovie()
{
    uint8_t i;
    uint8_t j;
    
    LedScreen_Clear();
    for( i=0; i<4; i++ )
    {
        j = (i+1)&0x03;
        LedScreen_DrawLine( xy[i][0], xy[i][1], xy[j][0], xy[j][1], 1);
    }
    DelayMs( 4 );
    LedScreen_Clear();
}
void LedScreen_Clear( void )
{
    memset( DispBuf, 0x00, sizeof(DispBuf) );
}

void LedScreen_SetPower( LedScrnPwr_t Power )
{
    uint8_t Index;
    
    if( Power == LedScrnPwrState )
    {
        return;
    }
    if( Power == LedScrn_On )
    {   
        for( Index=0; Index<sizeof(Hang)/sizeof(Hang_t); Index++ )
        {
            GPIO_Init( Hang[Index].Port, Hang[Index].Pin, GPIO_Mode_Out_PP_Low_Slow );
            GPIO_WriteBit( Hang[Index].Port, Hang[Index].Pin, SET );
        }  
        HC595_SET_STCP( RESET );
        HC595_SET_OE( SET );
        GPIO_WriteBit( GPIOB, GPIO_Pin_4, RESET );              //打开电源
        SPI_Cmd( SPI1, ENABLE );                                //打开数据端口
        TIM4_ITConfig( TIM4_IT_Update, ENABLE );                //打开扫描时钟
        TIM4_Cmd( ENABLE );                                     
        TIM2_Cmd( ENABLE );                                     //打开亮度PWM定时器
        //ADC_Cmd( ADC1, ENABLE );
        LedScrnPwrState = LedScrn_On;
    }
    else
    {
        TIM4_ITConfig( TIM4_IT_Update, DISABLE );
        TIM4_Cmd( DISABLE );                                    //关闭扫描时钟
        HC595_SET_STCP( RESET );                                //HC595填充0XFF，防止残影现象
        for( Index=0; Index<9; Index++ )
        {
            while( !(SPI1->SR & (uint8_t)SPI_FLAG_TXE) );
            HC595_SET_DATA( 0xFF );      
        }
        HC595_SET_STCP( SET );
        TIM2_Cmd( DISABLE );                                    //关闭亮度PWM定时器
        SPI_Cmd( SPI1, DISABLE );                               //关闭数据端口 
        GPIO_WriteBit( GPIOB, GPIO_Pin_5, RESET );              //数据端口置地防止漏电流
        GPIO_WriteBit( GPIOB, GPIO_Pin_6, RESET );
        //ADC_Cmd( ADC1, DISABLE );
        GPIO_WriteBit( GPIOB, GPIO_Pin_4, SET );                //关闭电源
        HC595_SET_STCP( RESET );
        HC595_SET_OE( RESET );
        for( Index=0; Index<sizeof(Hang)/sizeof(Hang_t); Index++ )
        {
            GPIO_Init( Hang[Index].Port, Hang[Index].Pin, GPIO_Mode_Out_PP_Low_Slow );
            GPIO_WriteBit( Hang[Index].Port, Hang[Index].Pin, RESET );
        } 
        LedScrnPwrState = LedScrn_Off;
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
        HC595_SET_DATA( ~DispBuf[ ScanTab[i]+Pos ] );      
    }
    
    //消隐
    TIM2_CtrlPWMOutputs( DISABLE );
    HC595_SET_STCP( SET );
    
    //行选
    (Hang[(H_Count-1)&0x07].Port)->ODR |= Hang[(H_Count-1)&0x07].Pin;
    (Hang[H_Count].Port)->ODR &= ~Hang[H_Count].Pin;
    
    //HC595_SET_OE( RESET );
    TIM2_CtrlPWMOutputs( ENABLE );

    H_Count ++;
    Pos += 3;
    if( H_Count >= 8 )
    {
        H_Count = 0;
        Pos = 0;
    }
}