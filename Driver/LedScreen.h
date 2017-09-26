#ifndef _LEDSCREEN_H_
#define _LEDSCREEN_H_

#include "stm8l15x.h" 

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

//74HC595相关定义
#define HC595_SET_STCP(VAL)     GPIO_WriteBit(GPIOB,GPIO_Pin_3,VAL)       //设置输出锁存器时钟
#define HC595_SET_OE(VAL)       GPIO_WriteBit(GPIOB,GPIO_Pin_2,VAL)       //设置输出使能电平
#define HC595_SET_RST(VAL)      GPIO_WriteBit(GPIOA,GPIO_Pin_1,VAL)       //设置复位使能电平
#define HC595_SET_DATA(DAT)     SPI1->DR = DAT                            //设置移位寄存器数据

//LED显示屏相关定义
#define LEDSCRN_ON              RESET
#define LEDSCRN_OFF             SET
#define LEDSCRN_SET_LUM(LUM)    TIM3_SetCompare2( LUM );                 //设置屏幕亮度
#define LEDSCRN_SET_STAT(STAT)  HC595_SET_OE(STAT)                       //设置屏幕状态

extern void LedScreen_Init( void );
extern void LedScreen_Scan( void );
extern void LedScrnAutoSetLum( void );
extern void LedScreen_ShowImage( uint8_t ImgBuf[72], LedScrnDispMode DispMode );

#endif
