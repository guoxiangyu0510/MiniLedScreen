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

//74HC595��ض���
#define HC595_SET_STCP(VAL)     GPIO_WriteBit(GPIOB,GPIO_Pin_3,VAL)       //�������������ʱ��
#define HC595_SET_OE(VAL)       GPIO_WriteBit(GPIOB,GPIO_Pin_2,VAL)       //�������ʹ�ܵ�ƽ
#define HC595_SET_RST(VAL)      GPIO_WriteBit(GPIOA,GPIO_Pin_1,VAL)       //���ø�λʹ�ܵ�ƽ
#define HC595_SET_DATA(DAT)     SPI1->DR = DAT                            //������λ�Ĵ�������

//LED��ʾ����ض���
#define LEDSCRN_ON              RESET
#define LEDSCRN_OFF             SET
#define LEDSCRN_SET_LUM(LUM)    TIM3_SetCompare2( LUM );                 //������Ļ����
#define LEDSCRN_SET_STAT(STAT)  HC595_SET_OE(STAT)                       //������Ļ״̬

extern void LedScreen_Init( void );
extern void LedScreen_Scan( void );
extern void LedScrnAutoSetLum( void );
extern void LedScreen_ShowImage( uint8_t ImgBuf[72], LedScrnDispMode DispMode );

#endif
