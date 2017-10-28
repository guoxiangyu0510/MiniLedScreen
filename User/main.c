#include "stm8l15x.h"//STM8L051/151等系列共用库函数
#include "stdlib.h"
#include "string.h"
#include "Picture.h"
#include "LedScreen.h"
#include "Misc.h"
#include "1527.h"





void BoardInit( void )
{   
    LedScreen_Init();
    
    EV1527_Init();
       
    Misc_Init();
    
    enableInterrupts();
}

void main(void)
{
    uint8_t ScrnOnDispFlag=0;
    uint8_t ScrnFlash=0;
        
    BoardInit();
    
    while(1)
    {
        if( EV1257_IsReceived() )
        {
            
            switch( EV1257_GetRecvData() )
            {
            case EV1527_KEY0:
                    break;
            case EV1527_KEY1:
                break;
            case EV1527_KEY2:
                break;
            }
            
            ScrnOnDispFlag = 1;
            
            //等到Int引脚平静后进入睡眠模式
            EV1257_ClearRecvBuf();
        }
        if( Clk500msFlag )
        {
            Clk500msFlag = 0;
            if( ScrnOnDispFlag )   
            {
                ScrnFlash = !ScrnFlash;
            }
            if( ScrnFlash )
            {
                LEDSCRN_SET_STAT( LEDSCRN_ON );
            }
            else
            {
                LEDSCRN_SET_STAT( LEDSCRN_OFF );
            }
        }
    }
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
