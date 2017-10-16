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

typedef enum
{
    LedScrn_Off,
    LedScrn_On,
    LedScrn_Init,
}LedScrnPwr_t;

//74HC595��ض���
#define HC595_SET_STCP(VAL)     GPIO_WriteBit(GPIOB,GPIO_Pin_3,VAL)       //�������������ʱ��
#define HC595_SET_OE(VAL)       GPIO_WriteBit(GPIOB,GPIO_Pin_2,VAL)       //�������ʹ�ܵ�ƽ
#define HC595_SET_RST(VAL)      GPIO_WriteBit(GPIOA,GPIO_Pin_1,VAL)       //���ø�λʹ�ܵ�ƽ
#define HC595_SET_DATA(DAT)     SPI1->DR = DAT                            //������λ�Ĵ�������

//LED��ʾ����ض���
#define LEDSCRN_SET_LUM(LUM)    TIM3_SetCompare2( LUM );                 //������Ļ����
#define LEDSCRN_SET_STAT(STAT)  HC595_SET_OE(STAT)                       //������Ļ״̬

extern void LedScreen_Init( void );
extern void LedScreen_Scan( void );
extern void LedScrnAutoSetLum( void );
extern void LedScreen_ShowImage( uint8_t ImgBuf[72], LedScrnDispMode DispMode );
extern void LedScreen_Clear( void );
extern void LedScreen_SetPower( LedScrnPwr_t Power );
extern void LedScreen_DrawPixel( uint8_t x, uint8_t y, uint8_t color );
extern void LedScreen_PrintString( uint8_t x, uint8_t y, char *str );
extern void LedScreen_PrintHex( uint8_t x, uint8_t y, uint8_t *HexVector, uint8_t Len );
extern void LedScreen_DrawLine( uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t color );
extern void LedScreen_PlayInitMovie();

#endif
