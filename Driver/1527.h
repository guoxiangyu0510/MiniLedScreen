#ifndef _1527_H_
#define _1527_H_

#include "stm8l15x.h"

#define GOTO_RET(LB,RV,RC) do{(RV)=(RC);goto LB;}while(0);

//EV1527相关定义
#define EV1527_BIT_0            0
#define EV1527_BIT_1            1
#define EV1527_BIT_SYN          2

#define EV1527_KEY0             0
#define EV1527_KEY1             1
#define EV1527_KEY2             2
#define EV1527_KEY3             3

#define EV1527_INT_PORT         GPIOC
#define EV1527_INT_PIN          GPIO_Pin_1

#define EV1527_CFG_SAVE_ADDR    0x1000

extern volatile uint8_t Ev1527RecvOkFlag;
extern void EV1527_Init( void );
extern int8_t EV1257_IsReceived( void );
extern uint8_t EV1257_GetRecvData( void );
extern void EV1257_ClearRecvBuf( void );


#endif