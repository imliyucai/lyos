/**
  ******************************************************************************
  * @file      sys_abc.c
  * @author    liyc
  * @version   V1.0
  * @date      2024-4-28
  * @brief     RSD79 board code
  *            the basic function and decleration of mcu
  ******************************************************************************
  **/ 

#include "stm32f4xx.h"

#include "./sys_abc.h"

/* To correctly read data from FLASH memory, the number of wait states (LATENCY)
    must be correctly programmed according to the frequency of the CPU clock
    (HCLK) and the supply voltage of the device. */
		
static void sys_SetupPll( void );
static void sys_SetClk( void );
static void sys_SetotherClk( void );

////////////////////////////////////37
static void Error_Handler( void )
{
  // USER CODE BEGIN Error_Handler_Debug 
  // User can add his own implementation to report the HAL error return state 
  __disable_irq();
  while(1)
  {      }
  // USER CODE END Error_Handler_Debug 
}

void system_ConfigMisc( uint32_t addrstart ) 
{  
  //***NVIC_SetVectorTable( NVIC_VectTab_FLASH, 0x0000 );
	#define NVIC_VectTab_RAM            ((uint32_t)0x20000000)
  #define NVIC_VectTab_FLASH           ((uint32_t)0x08000000)
	SCB->VTOR = NVIC_VectTab_FLASH | (addrstart & (uint32_t)0x1FFFFF80);
	
	//***NVIC_PriorityGroupConfig( NVIC_PriorityGroup_2 );   
	/* Set the PRIGROUP[10:8] bits according to NVIC_PriorityGroup value */
  #define AIRCR_VECTKEY_MASK       ((uint32_t)0x05FA0000)
	#define NVIC_PriorityGroup_2         ((uint32_t)0x500)
	SCB->AIRCR = AIRCR_VECTKEY_MASK | NVIC_PriorityGroup_2;
	
}

void system_ConfigClockX( void )    // 16MHz HSE -> 168MHz sysClk
{
// adapted from SetSysClock() in system_stm32f4xx.c version 2016 (V1.8.1)

	//__IO 
	uint32_t StartUpCounter =0, HSEStatus =0;
	
	RCC->APB1ENR |= (uint32_t)0x10000000;     // bit_28, power for interface clock
	while( (RCC->APB1ENR & ((uint32_t)0x10000000)) != (uint32_t)0x10000000 )  
  {;}    // wait write bit ok
	
	/// Enable HSE 
  RCC->CR |= ((uint32_t)RCC_CR_HSEON);
 
  /// Wait till HSE is ready and if Time out is reached exit 
  do
  {
    HSEStatus = RCC->CR & RCC_CR_HSERDY;
    StartUpCounter++;
  } while( (HSEStatus == 0) && (StartUpCounter != HSE_STARTUP_TIMEOUT) );
	
	if( (RCC->CR & RCC_CR_HSERDY) != RESET )
  {    HSEStatus = (uint32_t)0x01;    }   // HSE ready
  else
  {    HSEStatus = (uint32_t)0x00;    }   // HSE wrong maybe, how to do ??
	
	if( HSEStatus == (uint32_t)0x01 )
  {
		/// Select regulator voltage output Scale 1 mode 
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
    PWR->CR |= PWR_CR_VOS;

    /// HCLK = SYSCLK /1 
    RCC->CFGR |= RCC_CFGR_HPRE_DIV1;
		/// PCLK2 = HCLK /2 
    RCC->CFGR |= RCC_CFGR_PPRE2_DIV2;
    /// PCLK1 = HCLK /4 
    RCC->CFGR |= RCC_CFGR_PPRE1_DIV4;
		
	#define PLL_M    16
	#define PLL_N    336
	#define PLL_P     2
	#define PLL_Q    7
		
		/// Configure the main PLL 
    RCC->PLLCFGR = PLL_M |(PLL_N <<6) |(((PLL_P >>1) -1) <<16) 
                   |(RCC_PLLCFGR_PLLSRC_HSE) |(PLL_Q <<24);
		
		/// Enable the main PLL 
    RCC->CR |= RCC_CR_PLLON;

    /// Wait till the main PLL is ready 
    while( (RCC->CR & RCC_CR_PLLRDY) == 0 )
		{ ; 	}
		
		////** ?? Configure Flash prefetch, Instruction cache, Data cache and wait state 
    FLASH->ACR = FLASH_ACR_PRFTEN |FLASH_ACR_ICEN |FLASH_ACR_DCEN |FLASH_ACR_LATENCY_5WS;

		/// Select the main PLL as system clock source 
    RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));
    RCC->CFGR |= RCC_CFGR_SW_PLL;

    /// Wait till the main PLL is used as system clock source 
    while( (RCC->CFGR & (uint32_t)RCC_CFGR_SWS ) != RCC_CFGR_SWS_PLL )
    {  ;  }
		
		
	}	
	else
	{ Error_Handler();   }
	
//////////////////////////////////////////////////51	
/*
	RCC->APB1ENR |= (uint32_t)0x10000000;     // bit_28, power for interface clock
	while( (RCC->APB1ENR & ((uint32_t)0x10000000)) != (uint32_t)0x10000000 )  
  {;}    // wait write bit ok
		
	PWR->CR |= (uint32_t)0x00004000;     // bit_14, internal regulator voltage scale 1
	
	RCC->CR = (uint32_t)0x00000000;
	
	#define RCC_PLLSource_HSE       ((uint32_t)0x00400000)
  //#define RCC_PLLSource    RCC_PLLSource_HSE
	#define PLL_M    16
	#define PLL_N    336
	#define PLL_P     2
	#define PLL_Q    7
	RCC->PLLCFGR = PLL_M |(PLL_N <<6) |(((PLL_P >>1) -1) <<16) 
	                |(RCC_PLLSource_HSE) |(PLL_Q <<24);
	
	RCC->CR |= (uint32_t)0x00010000;   // bit_16, HSE enable
	while( (RCC->CR & ((uint32_t)0x00020000)) != (uint32_t)0x00020000 )  
  {;}    // wait HSE ready
		
	RCC->CR |= (uint32_t)0x01000000;    // bit_24, PLL enable
	while( (RCC->CR & ((uint32_t)0x02000000)) != (uint32_t)0x02000000 )  
  {;}    // wait PLL locked, i.e. stable
	
	 RCC->CFGR = (uint32_t)0x00000000;
	RCC->CFGR &= ~((uint32_t)0x00000010);    // bit_7..4, AHB=SYSCLK/1(0xxx)
	RCC->CFGR |= (uint32_t)0x00008000;    // bit_15..13, APB2=AHB/2(100)
	RCC->CFGR |= (uint32_t)0x00001400;    // bit_12..10, APB1=AHB/4(101)	
		RCC->CFGR |= (uint32_t)0x00000002;    // bit_1..0, PLL as SYSCLK(10)
*/
}

void system_ConfigClock( void )    // 16MHz HSE -> 168MHz sysClk
{
  // Configure the main internal regulator output voltage
  //__HAL_RCC_PWR_CLK_ENABLE();
	RCC->APB1ENR |= (uint32_t)0x10000000;     // bit_28, power for interface clock
	while( (RCC->APB1ENR & ((uint32_t)0x10000000)) != (uint32_t)0x10000000 )  
  {;}    // wait write bit ok
																					
  //__HAL_PWR_VOLTAGESCALING_CONFIG( PWR_REGULATOR_VOLTAGE_SCALE1 );
	PWR->CR |= (uint32_t)0x00004000;     // bit_14, internal regulator voltage scale 1
	while( (PWR->CR & ((uint32_t)0x00004000)) != (uint32_t)0x00004000 )  
  {;}    // wait write bit ok

																													
  // Initializes the RCC Oscillators according to the specified parameters
	sys_SetupPll();
	
  // Initializes the CPU, AHB and APB buses clocks
	sys_SetClk();
}


////////////////////////??
static void sys_SetupPll( void )
{
  uint32_t tickstart;
	do{                         
      //SET_BIT(RCC->CR, RCC_CR_HSEON); 
      RCC->CR |= ((uint32_t)RCC_CR_HSEON);		
    } while(0U) ;
	
  tickstart = 1000;   /* Wait till HSE is ready */
  while( (RCC->CR & RCC_CR_HSERDY) == RESET )
	{
    tickstart--;  if( tickstart==0 ) break;
  }
			
  if( (RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL )				
  {
     /* Disable the main PLL. */
    RCC->CR &= ~RCC_CR_PLLON;
		
    tickstart = 1000;   /* Wait till PLL is ready */
    while( (RCC->CR & RCC_CR_PLLON) != RESET )
		{
  		tickstart--;  if( tickstart==0 ) break;
    }
				
			/* Configure the main PLL clock source, multiplication and division factors. */
      WRITE_REG( RCC->PLLCFGR, (RCC_PLLCFGR_PLLSRC_HSE                 \
                                 |16                                   \
                                 |(336 <<6)        \
                                 |(((2 >> 1U) - 1U) <<16)  \
                                 |(7 <<24) ) );
        /* Enable the main PLL. */
      RCC->CR |= RCC_CR_PLLON;
				
    tickstart = 1000;   /* Wait till PLL is ready */
    while( (RCC->CR & RCC_CR_PLLON) != SET )
		{
    	tickstart--;  if( tickstart==0 ) break;
    }	

	}			
}
static void sys_SetClk( void )
{
	uint32_t tickstart;
    /* Program the new number of wait states to the LATENCY bits in the FLASH_ACR register */
  (*(__IO uint8_t *)ACR_BYTE0_ADDRESS = (uint8_t)(FLASH_ACR_LATENCY_5WS)); // FLASH_ACR_LATENCY_5WS
	
	MODIFY_REG( RCC->CFGR, RCC_CFGR_PPRE1, 0x00001C00U );       // RCC_HCLK_DIV16=0x00001C00U
	MODIFY_REG( RCC->CFGR, RCC_CFGR_PPRE2, (0x00001C00U <<3) );    // RCC_HCLK_DIV16=0x00001C00U
	MODIFY_REG( RCC->CFGR, RCC_CFGR_HPRE, 0x00000000U );    // RCC_SYSCLK_DIV1=0x00000000U
	
	/* Check the HSE ready flag */
//  if( (RCC->CR & RCC_CR_HSERDY) != SET )
	//if( __HAL_RCC_GET_FLAG(RCC_FLAG_HSERDY) == RESET )
//  {        Error_Handler();   				}
	/* Check the PLL ready flag */
//  if( (RCC->CR & RCC_CR_PLLON) != SET )
	//if( __HAL_RCC_GET_FLAG(RCC_FLAG_PLLRDY) == RESET )
//  {        Error_Handler();    				}		
	
  MODIFY_REG( RCC->CFGR, RCC_CFGR_SW, (RCC_CFGR_SW_PLL) );  //
			
    
  tickstart = 1000;
  while( (RCC->CFGR & RCC_CFGR_SWS) != (RCC_CFGR_SW_PLL <<2U) )
  {
		tickstart--;  if( tickstart==0 ) break;
  }		
	
	MODIFY_REG( RCC->CFGR, RCC_CFGR_PPRE1, RCC_CFGR_PPRE1_DIV4 );
	MODIFY_REG( RCC->CFGR, RCC_CFGR_PPRE2, ((RCC_CFGR_PPRE1_DIV2) << 3U) );
		
		
	sys_SetotherClk( );    ////*** must init tick.......
}

void sys_SetotherClk( void )
{
  /* Update the SystemCoreClock global variable */
  //SystemCoreClock = HAL_RCC_GetSysClockFreq() >> AHBPrescTable[(RCC->CFGR & RCC_CFGR_HPRE)>> RCC_CFGR_HPRE_Pos];

	SystemCoreClock = 168000000U;  // set it direct ok??
	
  /* Configure the source of time base considering new system clocks settings */
  // TICK_INT_PRIORITY=0x0fU
	SysTick_Config( SystemCoreClock / (1000U /1U) ); 
	
	uint32_t prioritygroup = NVIC_GetPriorityGrouping();
  NVIC_SetPriority( SysTick_IRQn, NVIC_EncodePriority(prioritygroup, 0x0fU, 0U) );
	
//  uwTickPrio = TICK_INT_PRIORITY;
}
	
/////////////////////////////////////////////////////////////////66 end of file
/*
void system_ConfigClock( void )    // 16MHz HSE -> 168MHz sysClk
{
  RCC_OscInitTypeDef  rccOsc_InitStruct = {0};
  RCC_ClkInitTypeDef  rccClk_InitStruct = {0};

  // Configure the main internal regulator output voltage
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG( PWR_REGULATOR_VOLTAGE_SCALE1 );

  // Initializes the RCC Oscillators according to the specified parameters
  rccOsc_InitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  rccOsc_InitStruct.HSEState = RCC_HSE_ON;
  rccOsc_InitStruct.PLL.PLLState = RCC_PLL_ON;
  rccOsc_InitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  rccOsc_InitStruct.PLL.PLLM = 16;   //8;
  rccOsc_InitStruct.PLL.PLLN = 336;  //168;
  rccOsc_InitStruct.PLL.PLLP = RCC_PLLP_DIV2; 
  rccOsc_InitStruct.PLL.PLLQ = 7;    //4;
  if( HAL_RCC_OscConfig(&rccOsc_InitStruct) != HAL_OK )
  {
    Error_Handler();
  }

  // Initializes the CPU, AHB and APB buses clocks
  rccClk_InitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK |RCC_CLOCKTYPE_HCLK 
                                 |RCC_CLOCKTYPE_PCLK1 |RCC_CLOCKTYPE_PCLK2;
  rccClk_InitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  rccClk_InitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  rccClk_InitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  rccClk_InitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if( HAL_RCC_ClockConfig(&rccClk_InitStruct, FLASH_LATENCY_5) != HAL_OK )
  {
    Error_Handler();
  }
}  */



