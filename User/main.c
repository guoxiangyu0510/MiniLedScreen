#include "stm8l15x.h"//STM8L051/151等系列共用库函数
#include "stdlib.h"
#include "string.h"
#include "Picture.h"
#include "LedScreen.h"
#include "Misc.h"
#include "1527.h"



void BoardInit( void )
{     
    uint8_t Cnt;
    char Ch;
    
    Misc_Init();
    
    LedScreen_Init();
    
    EV1527_Init();
    
    enableInterrupts();
    
    //短路Boot引脚进入遥控学习模式
    GPIO_Init( GPIOC, GPIO_Pin_0, GPIO_Mode_In_PU_No_IT );
    if( (GPIOC->IDR & (uint8_t)GPIO_Pin_0) == 0 )
    {
        Cnt = 0;
        while( Cnt < 4 )
        {
            EV1527_SetPower( EV1527_PWR_OFF );
            LedScreen_SetPower( LedScrn_On ); 
            
            Ch = '1' + Cnt;
            LedScreen_Clear();
            LedScreen_PrintString( 0, 0, &Ch ); 
            DelayMs( 8 );
            
            LedScreen_SetPower( LedScrn_Off );
            EV1527_SetPower( EV1527_PWR_ON );
            while( EV1257_IsReceived() == 0 );

            EV1527_SetPower( EV1527_PWR_OFF );   
            LedScreen_SetPower( LedScrn_On ); 
            LedScreen_Clear();
            LedScreen_PrintHex( 0, 0, EV1257_GetRecvBuf(), 3 );
            DelayMs( 8 );
            
            EV1257_RecvNext();
            Cnt++;
        }
        LedScreen_Clear();
        LedScreen_SetPower( LedScrn_Off );     
    }
}

void main(void)
{
    uint8_t ScrnOnDispFlag = 0;
    uint8_t ScrnFlash = 0;
    uint8_t ScrnFlashCnt = 0;
    uint8_t *DispPicture;
    
        
    BoardInit();
    
    EV1527_SetPower( EV1527_PWR_OFF );   
    LedScreen_SetPower( LedScrn_On ); 
    //LedScreen_PrintString(0,0,"1234567abDZ");
    LedScreen_PlayInitMovie();    
    LedScreen_SetPower( LedScrn_Off );
    EV1527_SetPower( EV1527_PWR_ON ); 
    
    while(1)
    {
        
        if( EV1257_IsReceived() )
        {       
            switch( EV1257_GetEventKey() )
            {
            case EV1527_KEY0:
                DispPicture = (uint8_t*)Pic0;
                break;
            case EV1527_KEY1:
                DispPicture = (uint8_t*)Pic1;
                break;
            case EV1527_KEY2:
                DispPicture = (uint8_t*)Pic2;
                break;
            case EV1527_KEY3:
                DispPicture = (uint8_t*)Pic3;
                break;
            default:
                break;
            }   
            
            EV1527_SetPower( 0 );
            LedScreen_SetPower( LedScrn_On );
            
            ScrnOnDispFlag = 1;
            ScrnFlashCnt = 0;
            
            //等到Int引脚平静后进入睡眠模式
            EV1257_RecvNext();
        }
        if( Clk500msFlag )
        {
            Clk500msFlag = 0;      
            if( ScrnOnDispFlag )   
            {
                ScrnFlash = !ScrnFlash;
                ScrnFlashCnt ++;
                if( ScrnFlashCnt >= 6 )
                {
                    ScrnOnDispFlag = 0;
                    ScrnFlashCnt = 0;
                    LedScreen_Clear();
                    LedScreen_SetPower( LedScrn_Off );
                    EV1527_SetPower( 1 );             
                }           
            }
            if( !ScrnFlash )
            {
                LedScreen_Clear();
            }
            else
            {
                LedScreen_ShowImage( DispPicture, Normal);
            }
        }
        if( !ScrnOnDispFlag )
        {
            sim();  
            halt();     //掉电模0.278ms
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
