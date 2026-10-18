/*
********************************************************************************
*                                    rsd257.p_bust CODE
*
*                        (c) Copyright 2025; evertrend, Inc.; shanghai, China
*
*           All rights reserved.  Protected by international copyright laws.
*           Knowledge of the source code may not be used to write a similar
*           product.  This file may only be used in accordance with a license
*           and should not be redistributed in any way.
********************************************************************************
*/
  
/*
********************************************************************************
*
*                                    the ISR code theisrs2.c
*
*                                       
*                              on STM32F407vet6 in the rsd257.p board
*
* Filename      : theisrs.c
* Version       : V1.00
* Programmer(s) : liyc
*                 
********************************************************************************
*/

/*
********************************************************************************
*                                     INCLUDE FILES
********************************************************************************
*/

#include "stm32f4xx.h"



///,#include "../lib.BSP/Inc/bsp_usart1host.h"
///,#include "../lib.BSP/Inc/bsp_can1wheels.h"
///,#include "../li.DRIVER/drv_canopen.h"

#include "../lib.BSP/Inc/bsp_timer.h"
#include "../lib.BSP/Inc/bsp_key.h"

///,#include "../lib.BSP/Inc/bsp_usart3ble.h"
///,#include "../lib.BSP/Inc/bsp_rs485Signals.h"

#include "../li.DRIVER/liy_util_mnLog.h"
#include "../lib.BSP/Inc/bsp_led.h"

/*
********************************************************************************
*                                    LOCAL DEFINES
********************************************************************************
*/

///,uint16_t  usart1host_lenrxd;
///,uint16_t  usart2rs485_lenrxd;
///,uint16_t  usart3host_lenrxd;

/*
********************************************************************************
*                               FUNCTION PROTOTYPES
********************************************************************************
*/



////////////////////////////////////////////////////////////////...usart3 to host interrupt

////////////////////////////////////////////////49 EXTI interrupt
// keyin interrrupt handler

/////////9 void  EXTI15_10_IRQHandler( void )
void KEY012_IRQHandler( void )
{

	if( (EXTI->PR & (KEY0_INT_GPIO_PIN))  !=RESET )	
	{ 
		//AppSysStatus.Flags |= 0x00008000;   // NOTE: ??couldnot assign value to FLAG_GRP directely in ISR.

				
	  EXTI->PR = (KEY0_INT_GPIO_PIN);   // clear interrupt flag
		
///.		mnLog_WriteLog( LOGCODE_EMGBTN, 0x0505 );   // pressed=EXTI_Trigger_Falling
	}
	
	if( (EXTI->PR & (KEY1_INT_GPIO_PIN))  !=RESET )	
	{ 
	  //AppSysStatus.Flags |= 0x00004000; 

		
	  EXTI->PR = (KEY1_INT_GPIO_PIN);
		
///.		mnLog_WriteLog( LOGCODE_RELSBTN, 0x0505 );  // pressed=EXTI_Trigger_Falling
	}
	
	if( (EXTI->PR & (KEY2_INT_GPIO_PIN))  !=RESET )	
	{ 
	  //AppSysStatus.Flags |= 0x00002000; 

		
	  EXTI->PR = (KEY2_INT_GPIO_PIN);
		
		mnLog_WriteLog( 0x5c6c, 0xbb03 );
		
	}
	
  

}

/*
////////////////////////////////////////////////49 TIM6 interrupt
// TIM6 update interrrupt handler
/////////9 void  TIM6_DAC_IRQHandler( void )
void TIM6WHEELS_IRQHandler( void )
{
  
	///// TIM Update event    
  if( ((TIM6WHEELS->SR) & (TIM_SR_UIF)) == TIM_SR_UIF )  // TIM_FLAG_UPDATE
	{
    if( ((TIM6WHEELS->DIER) & (TIM_DIER_UIE)) == TIM_DIER_UIE )   // TIM_IT_UPDATE
		{
      
			TIM6WHEELS->SR = ~(TIM_DIER_UIE);
			
//      can1open_SetVelocity( 1, 0x0 );   // LSB first, left wheel
//	    can1open_SetVelocity( 2, 0x0 );

    }
  }
	
}  */


/////////////////////////////////////////////////////////////////66 end of file

/* 

*/







