/*
********************************************************************************
*                                        rsd257.p_bust CODE
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
*                                    the ISR code theisrs.c
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



#include "../lib.BSP/Inc/bsp_usart1host.h"
#include "../lib.BSP/Inc/bsp_can1wheels.h"
///,#include "../li.DRIVER/drv_canopen.h"

#include "../lib.BSP/Inc/bsp_timer.h"
#include "../lib.BSP/Inc/bsp_key.h"

#include "../lib.BSP/Inc/bsp_pwrCtrl.h"
#include "../lib.BSP/Inc/bsp_miscCtrl.h"


#include "../lib.BSP/Inc/bsp_usart3ble.h"
#include "../lib.BSP/Inc/bsp_rs485Signals.h"

#include "../li.DRIVER/liy_util_mnLog.h"
#include "../lib.BSP/Inc/bsp_led.h"

/*
********************************************************************************
*                                    LOCAL DEFINES
********************************************************************************
*/

uint16_t  usart1host_lenrxd;
uint16_t  usart2rs485_lenrxd;
uint16_t  usart3host_lenrxd;

/*
********************************************************************************
*                               FUNCTION PROTOTYPES
********************************************************************************
*/

////////////////////////////////////////////////49 USART1 interrupt
// usart1 interrupt handler

/////////9 void  USART1_IRQHandler( void )
void USART1HOST_IRQHandler( void )
{
  	
	//uint8_t rlen;
	if( (USART1HOST->SR & 0x00000010) == 0x00000010 )   // bit_4, idle line detected flag
	//if( USART_GetITStatus( USART1HOST, USART_IT_IDLE ) != RESET )  // 0x0424 ?
	{
		//USART_ClearITPendingBit( USART1HOST, USART_IT_IDLE ); 
		//USART_ReceiveData( USART1HOST );   // !!! must read DR, so that to clear the IDLE Flag
		(USART1HOST->SR);      ////..** first read this register for clear the flag
		(USART1HOST->DR);      ////..** second read this register for clear the flag
		
		//lenrxd = USART1HOST_RXBUFF_SIZE - DMA_GetCurrDataCounter( DMA2_Stream5 ); 
		usart1host_lenrxd = USART1HOST_RXBUFF_SIZE - USART1HOST_RXDMA_STREAM->NDTR;     // the data length received.......
		
		//DMA_Cmd( USART1HOST_RXDMA_STREAM, DISABLE );  //
		USART1HOST_RXDMA_STREAM->CR &= ~0x00000001;    // bit_0, close DMA stream for re-config DMA
		
		//DMA_SetCurrDataCounter( USART1HOST_RXDMA_STREAM, USART1HOST_RXBUFF_SIZE );  //
		USART1HOST_RXDMA_STREAM->NDTR = USART1HOST_RXBUFF_SIZE;    // re-config RxBuf size
		
		//DMA_ClearFlag( USART1HOST_RXDMA_STREAM, DMA_FLAG_TCIF5 );
		USART1HOST_DMA->HIFCR |= 0x00000800;     // clear TCIF5 flag (transfer complete flag?)
		
		//DMA_Cmd( USART1HOST_RXDMA_STREAM, ENABLE );   //
		USART1HOST_RXDMA_STREAM->CR |= 0x00000001;    // bit_0, open DMA stream again for next receive
		
		// for test only
		//uart1host_SendString( usart1host_Rxbuf, rlen );
		
		if( usart1host_lenrxd >3 )    // if less than 4-byte, that is not my frame
	  {
		// send Q to task task_Usart, this is a frame from host 

	  }
	}
	
	if( (USART1HOST->SR & 0x00000020) == 0x00000020 )   // bit_5, rx register no empty flag
  //if( USART_GetITStatus( USART1HOST, USART_IT_RXNE ) != RESET ) 
	{
     //USART_ReceiveData( USART1HOST );   ////..** for clear the flag
     (USART1HOST->DR);      ////..** read this register for clear the flag
		
  }
	
	
/*   HAL mode
	if( __HAL_UART_GET_FLAG(&uart_Handle, UART_FLAG_IDLE) != RESET )   
	{
		// now usart line in idle status, so I get received data
		
    // count of received data byte... 
		lenrxd = USART2HOST_RXBUFF_SIZE - (uart_Handle.hdmarx->Instance->NDTR);
		
		// stop DMA and call HAL_UART_AbortReceiveCpltCallback() to get received data
		HAL_UART_AbortReceive_IT( &uart_Handle );     // should .... see line.1928 in _hal_uart.c
		
		
		// clear interrupt flag, <idle> 
		__HAL_UART_CLEAR_IDLEFLAG( &uart_Handle );
	}     */
		

}


////////////////////////////////////////////////49 USART DMA interrupt
/////////9 void  DMA2_Stream5_IRQHandler( void )
void USART1HOST_DMA_IRQHandler( void )	
{
  /* HAL mode
	HAL_DMA_IRQHandler( uart_Handle.hdmarx );     // should ... see line.748 in _hal_dma.c
	 */  
	
	// DMA2_Stream5_Channel4 for USART1 RxDMA
	if(	(USART1HOST_DMA->HISR & 0x00000800) == 0x00000800 )  // bit_11, Stream5 transfer complete interrupt flag
	//if( DMA_GetITStatus( USART1HOST_RXDMA_STREAM, DMA_IT_TCIF5 ) == SET ) // Stream5 transfer complete interrupt
	{                                          // interrupt flag On...
		//DMA_ClearITPendingBit( USART1HOST_RXDMA_STREAM, DMA_IT_TCIF5 );   // clear interrupt falg
		USART1HOST_DMA->HIFCR |= 0x00000800;     // clear TCIF5 flag
		
		//DMA_Cmd( USART1HOST_RXDMA_STREAM, DISABLE );  // close DMA stream for re-config DMA
    USART1HOST_RXDMA_STREAM->CR &= ~0x00000001;    // bit_0, close DMA stream for re-config DMA
		
		//DMA_SetCurrDataCounter( USART1HOST_RXDMA_STREAM, USART1HOST_RXBUFF_SIZE );   // for next receive
    USART1HOST_RXDMA_STREAM->NDTR = USART1HOST_RXBUFF_SIZE;    // re-config RxBuf size
		
		//DMA_ClearFlag( USART1HOST_RXDMA_STREAM, DMA_FLAG_TCIF5 );
    USART1HOST_DMA->HIFCR |= 0x00000800;     // clear TCIF5 flag
		
		//DMA_Cmd( USART1HOST_RXDMA_STREAM, ENABLE );   // open DMA stream again for next receive
    USART1HOST_RXDMA_STREAM->CR |= 0x00000001;    // bit_0, open DMA stream again for next receive
    
	}
	

}


/*   HAL mode
void HAL_UART_AbortReceiveCpltCallback( UART_HandleTypeDef *huart )
{
	OS_ERR err;
	if( huart == &uart_Handle )
	{
		// send Q to task task_Usart, this is a frame from host 
		OSTaskQPost(  (OS_TCB    *)&taskUsartTCB, 
		              (void      *) usart2host_Rxbuf, 
		              (OS_MSG_SIZE) lenrxd,   
		         			(OS_OPT     ) OS_OPT_POST_FIFO,
		              (OS_ERR    *)&err    );  
		
		// restart next DMA receive
		HAL_UART_Receive_DMA( &uart_Handle, usart2host_Rxbuf, USART2HOST_RXBUFF_SIZE );
	}
}  */

uint8_t  togetnewcode =0xff;
extern uint8_t tempeT_drive;
extern uint8_t tempeT_chgx;
////////////////////////////////////////////////49 CAN interrupt
  uint32_t rdlr, rdhr; 
	uint32_t len;
	uint32_t  idx;
  uint8_t  dat[15];
	uint8_t  temp[15];
  uint32_t  tmp32;
///,	float convalue = 439.823/NUMONECYCLE;
float convalue = 439.823/36;
// CAN1 RX_FIFO0 interrupt handler
///,extern  OS_TCB  taskCan1wheelsTCB; 
/////////9 void CAN1_RX0_IRQHandler( void )
void	CAN1WHEEL_RX_IRQHandler( void )
{
///,  OS_ERR err;
  //if( CAN_GetITStatus( CAN1,CAN_IT_FMP0 )==SET ) {
///,  OSIntEnter();
   
  ///// use basic macro in stm32f4xx.h to operate registers of peripheral
	idx = READ_REG( CAN1->sFIFOMailBox[0].RIR );   // id  // rid = rir >>21;
	len = READ_REG( CAN1->sFIFOMailBox[0].RDTR );    // len  //	*len = rdtr &0x0000000f;
	
	rdlr = READ_REG( CAN1->sFIFOMailBox[0].RDLR );    // data
	rdhr = READ_REG( CAN1->sFIFOMailBox[0].RDHR );    // data
		dat[0] = rdlr &0x000000ff;        dat[1] = (rdlr &0x0000ff00)>>8;
		dat[2] = (rdlr &0x00ff0000)>>16;  dat[3] = (rdlr &0xff000000)>>24;
		dat[4] = rdhr &0x000000ff;        dat[5] = (rdhr &0x0000ff00)>>8;
		dat[6] = (rdhr &0x00ff0000)>>16;  dat[7] = (rdhr &0xff000000)>>24;
	
	
	SET_BIT( CAN1->RF0R, 0x00000020 );       // RFOM0 clear this message and rxflag
	///, store input frame in specified struct, which id as index, len as flag...
	
	///, for test only...has received can1 frame, now send back 
//,	can1wheels_SendMessage( dat, (uint8_t)(len &0x0000000f), ((uint16_t)(idx>>21))+3 );
	
	len &= 0x0000000f;
	idx >>= 21;  idx &= 0x00000fff;
	
	if( len ==8 )
	{
	  if( idx ==0x500 )
	  {
	    cand500.kon.byte0 =dat[0]; 
		  cand500.htout.byte2 =dat[2];
		  cand500.lout.byte4 =dat[4];
		  cand500.byte1 =dat[1]; cand500.byte3 =dat[3]; cand500.byte5 =dat[5];
		  cand500.byte6 =dat[6]; cand500.byte7 =dat[7];
	  }
	  if( idx ==0x540 )
	  {
	    tempeT_chgx =dat[0]; 
		  tempeT_drive =dat[1];

#define TOFLASHTEMPET  (0x37)			
extern uint8_t toflashtempeT;			
			toflashtempeT =TOFLASHTEMPET;
		}
  }
	
	if( idx ==0x550 )  ///, this newcode command
	{
	  if( dat[1]==0x69 && dat[0]==0xa5 )
		{
		  togetnewcode = 0x35;
		}
	}
	
	
	
/*	if( cand500.kon.byte0bit.ctlchgm ==1 )
	{ ctlinm_toPowerOn(); }
	else { ctlinm_toPowerOff(); }
	
	if( cand500.kon.byte0bit.ctlchga ==1 )
	{ ctlina_toPowerOn(); }
	else { ctlina_toPowerOff(); }
	
	if( cand500.kon.byte0bit.ctlfan ==1 )
	{ ctlfan_toPowerOn(); }
	else { ctlfan_toPowerOff(); }
	
	if( cand500.kon.byte0bit.ctldriveleft ==1 )
	{ mpwr_toPowerOn(); }
	else { mpwr_toPowerOff(); }
	
///	if( cand500.kon.byte0bit.ctldriveright ==1 )
///	{ mpwr_toPowerOn(); }     ///, one same signal control left and right drive
///	else { mpwr_toPowerOff(); }  
	
	if( cand500.kon.byte0bit.reset ==1 )
	{ ; }   ///, resetmyself();
	
	/////////////////////////////,
	if( cand500.htout.byte2bit.ctlhtout ==1 )
	{ htout_toPowerOn(); }
	else { htout_toPowerOff(); }
	
	if( cand500.htout.byte2bit.ctlhtout1 ==1 )
	{ htout1_toPowerOn(); }
	else { htout1_toPowerOff(); }
	
	if( cand500.lout.byte4bit.ctllout1 ==1 )
	{ lout1_toPowerOn(); }
	else { lout1_toPowerOff(); }
	
	if( cand500.lout.byte4bit.ctllout2 ==1 )
	{ lout2_toPowerOn(); }
	else { lout2_toPowerOff(); }
	
	if( cand500.lout.byte4bit.ctllout3 ==1 )
	{ lout3_toPowerOn(); }
	else { lout3_toPowerOff(); }
	
	if( cand500.lout.byte4bit.ctllout4 ==1 )
	{ lout4_toPowerOn(); }  
	else { lout4_toPowerOff(); }   */
	
/*	switch( dat[5] )
	{
	  case 0xaa:   mpwr_toPowerOn();  break;
		case 0x55:   mpwr_toPowerOff();  break;
		
		case 0xa0:   htout_toPowerOn();  break;
		case 0x50:   htout_toPowerOff();  break;
		
		case 0xa1:   htout1_toPowerOn();  break;
		case 0x51:   htout1_toPowerOff();  break;
		
		
		case 0xb1:   lout1_toPowerOn();  break;
		case 0x61:   lout1_toPowerOff();  break;
		
		case 0xb2:   lout2_toPowerOn();  break;
		case 0x62:   lout2_toPowerOff();  break;
		
		case 0xb3:   lout3_toPowerOn();  break;
		case 0x63:   lout3_toPowerOff();  break;
		
		case 0xb4:   lout4_toPowerOn();  break;
		case 0x64:   lout4_toPowerOff();  break;
		
		case 0xc1:   ctlina_toPowerOn();  break;
		case 0x71:   ctlina_toPowerOff();  break;
		
		case 0xc2:   ctlinm_toPowerOn();  break;
		case 0x72:   ctlinm_toPowerOff();  break;
		
		case 0xc3:   ctlfan_toPowerOn();  break;
		case 0x73:   ctlfan_toPowerOff();  break;
		
		
		default:  break;
	}
	*/
	
	
	
	if( len ==6 )
	{
		if( idx ==0x181 )
	  {
	    tmp32 = (uint32_t)( (float)rdlr *convalue );
			//tmp32 = (uint32_t)((float)rdlr /NUMONECYCLE );  tmp32 = tmp32 *(WHEEL_RADIUSMM *PIDOUBLE); // /NUMONECYCLE
			temp[3] =(tmp32 &0xff000000)>>24; temp[4] =(tmp32 &0x00ff0000)>>16;
			temp[5] =(tmp32 &0x0000ff00)>>8; temp[6] =(tmp32 &0x000000ff);
	  }
	  else if( idx ==0x182 )
		{
		  rdlr = 0xffffffff -rdlr;
			//tmp32 = (uint32_t)((float)rdlr /NUMONECYCLE );  tmp32 = tmp32 *(WHEEL_RADIUSMM *PIDOUBLE); // /NUMONECYCLE
			tmp32 = (uint32_t)( (float)rdlr *convalue );
			temp[7] =(tmp32 &0xff000000)>>24; temp[8] =(tmp32 &0x00ff0000)>>16;
			temp[9] =(tmp32 &0x0000ff00)>>8; temp[10] =(tmp32 &0x000000ff);
	/*		temp[0] =MSG_BACK_FLAGA;  temp[1] =12;  temp[2] =HOSTCMD_READRUNCOUNTER; 
			temp[11] =MSG_BACK_FLAGY; temp[12] =MSG_BACK_FLAGZ;
	    OSTaskQPost(  (OS_TCB    *)&taskUsartTCB, 
		                (void      *)temp, 
		                (OS_MSG_SIZE) (12+1), 
		                (OS_OPT     ) OS_OPT_POST_FIFO,
		                (OS_ERR    *)&err  );    */
			
		}
  }
	switch( idx )
	{  // Emergency Message
	///,	case 0x81: mnLog_WriteLog( LOGCODE_DRVL_ERROR, (dat[0]<<8) |dat[1] );   break;
	///,	case 0x82: mnLog_WriteLog( LOGCODE_DRVR_ERROR, (dat[0]<<8) |dat[1] );   break;
		 // any ack Message
		case 0x581:  break;
		case 0x582:  break;
		
	}
	
	
					
									
///,	OSIntExit();
	// } CAN_ClearITPendingBit( CAN1,CAN_IT_FMP0 );								
}     


// can_Handle.Instance  = CAN1
/*if( CAN_GetITStatus( CAN1,CAN_IT_FMP0 )==SET )
  {
    CAN_ClearITPendingBit( CAN1,CAN_IT_FMP0 );
    CAN_Receive( CAN1, CAN_FIFO0, &CAN1_RecvMessage );
  }  */

/*	inten = READ_REG( can_Handle.Instance->IER );    // read interrupt enable register
	dat[0] = (inten &0xff000000)>>24;  dat[1] = (inten &0x00ff0000)>>16;
	dat[2] = (inten &0x0000ff00)>>8;  dat[3] = (inten &0x000000ff);
	
	intflag = READ_REG( can_Handle.Instance->RF0R );  // read fifo0 data
	dat[0] = (intflag &0xff000000)>>24;  dat[1] = (intflag &0x00ff0000)>>16;
	dat[2] = (intflag &0x0000ff00)>>8;  dat[3] = (intflag &0x000000ff);
*/	




/////////9 void  USART2_IRQHandler( void )
void USART2RS485_IRQHandler( void )
{
	

	if( (UART2RS485->SR & 0x00000010) == 0x00000010 )   // bit_4, idle line detected flag
	{
		//USART_ClearITPendingBit( USART1HOST, USART_IT_IDLE ); 
		//USART_ReceiveData( USART1HOST );   // !!! must read DR, so that to clear the IDLE Flag
		(UART2RS485->SR);      ////..** first read this register for clear the flag
		(UART2RS485->DR);      ////..** second read this register for clear the flag
		
		usart2rs485_lenrxd = USART2RS485_RXBUFF_SIZE - USART2RS485_RXDMA_STREAM->NDTR;     // the data length received.......
		
		USART2RS485_RXDMA_STREAM->CR &= ~0x00000001;    // bit_0, close DMA stream for re-config DMA
		
		USART2RS485_RXDMA_STREAM->NDTR = USART2RS485_RXBUFF_SIZE;    // re-config RxBuf size
		
		USART2RS485_DMA->HIFCR |= 0x00000800;     // clear TCIF5 flag (transfer complete flag?)
		
		USART2RS485_RXDMA_STREAM->CR |= 0x00000001;    // bit_0, open DMA stream again for next receive
		
		// for test only
		//rs485_SendFrame( usart2rs485_Rxbuf, 7 );
		//usart2rs485_Rxbuf[0]=0x77; usart2rs485_Rxbuf[5]=0x88;
		
		// send Q to task task_dowork, this is a frame from rs-485 

	}
		
	if( (UART2RS485->SR & 0x00000020) == 0x00000020 )   // bit_5, rx register no empty flag
  {
     (UART2RS485->DR);      ////..** read this register for clear the flag
		
  }     
	
}

////////////////////////////////////////////////49 USART2RS485 DMA interrupt
/////////9 void  DMA1_Stream5_IRQHandler( void )
void USART2RS485_DMA_IRQHandler( void )	
{
   
	
	if(	(USART2RS485_DMA->HISR & 0x00000800) == 0x00000800 )  // bit_11, Stream5 transfer complete interrupt flag
	{                                          // interrupt flag On...
		
		USART2RS485_DMA->HIFCR |= 0x00000800;     // clear TCIF5 flag
		
		USART2RS485_RXDMA_STREAM->CR &= ~0x00000001;    // bit_0, close DMA stream for re-config DMA
		
		USART2RS485_RXDMA_STREAM->NDTR = USART2RS485_RXBUFF_SIZE;    // re-config RxBuf size
		
		USART2RS485_DMA->HIFCR |= 0x00000800;     // clear TCIF5 flag
		
		USART2RS485_RXDMA_STREAM->CR |= 0x00000001;    // bit_0, open DMA stream again for next receive
    
	}
	
}


//////////////////////////////////////////////..usart3 to host interrupt
/////////9 void  USART3_IRQHandler( void )
void USART3HOST_IRQHandler( void )
{
 	
                            ///. USART_SR_IDLE = 0x0010
	if( (USART3HOST->SR & USART_SR_IDLE) == USART_SR_IDLE )   // bit_4, idle line detected flag
	{
		//USART_ClearITPendingBit( USART1HOST, USART_IT_IDLE ); 
		//USART_ReceiveData( USART1HOST );   // !!! must read DR, so that to clear the IDLE Flag
		(USART3HOST->SR);      ////..** first read this register for clear the flag
		(USART3HOST->DR);      ////..** second read this register for clear the flag
		
		usart3host_lenrxd = USART3HOST_RXBUFF_SIZE - USART3HOST_RXDMA_STREAM->NDTR;     // the data length received.......
		
		USART3HOST_RXDMA_STREAM->CR &= ~0x00000001;    // bit_0, close DMA stream for re-config DMA
		
		USART3HOST_RXDMA_STREAM->NDTR = USART3HOST_RXBUFF_SIZE;    // re-config RxBuf size
		
		///.USART3HOST_DMA->HIFCR |= 0x00000800;     // clear TCIF5 flag (transfer complete flag?)
		USART3HOST_DMA->LIFCR |= DMA_LIFCR_CTCIF1;
		
		USART3HOST_RXDMA_STREAM->CR |= 0x00000001;    // bit_0, open DMA stream again for next receive
		
		///. for test only
		//rs485_SendFrame( usart2rs485_Rxbuf, 7 );
		uart3ble_SendString( usart3host_Rxbuf, 7 );
		//usart2rs485_Rxbuf[0]=0x77; usart2rs485_Rxbuf[5]=0x88;
		
/*		// send Q to task task_dowork, this is a frame from rs-485 
		OSTaskQPost(  (OS_TCB    *)&taskDoworkTCB, 
		              (void      *) usart2rs485_Rxbuf,   //..
		              (OS_MSG_SIZE) usart2rs485_lenrxd,   
		         			(OS_OPT     ) OS_OPT_POST_FIFO,
		              (OS_ERR    *)&err    );    */
	}
		                    ///. USART_SR_RXNE = 0x0020
	if( (USART3HOST->SR & USART_SR_RXNE) == USART_SR_RXNE )   // bit_5, rx register no empty flag
  {
     (USART3HOST->DR);      ////..** read this register for clear the flag
		
  }     
	

}

////////////////////////////////////////////////49 USART3HOST rxDMA interrupt
/////////9 void  DMA1_Stream1_IRQHandler( void )
void USART3HOST_DMA_IRQHandler( void )	
{
   
	                                ///. DMA_HISR_TCIF5 = 0x00000800   DMA_LISR_TCIF1 = 0x00000800
	if(	(USART3HOST_DMA->LISR & DMA_LISR_TCIF1) == DMA_LISR_TCIF1 )  // bit_11, Stream1 transfer complete interrupt flag
	{                                          // interrupt flag On...
		
		///.USART3HOST_DMA->HIFCR |= 0x00000800;     // clear TCIF5 flag DMA_HIFCR_CTCIF5 = 0x00000800
		USART3HOST_DMA->LIFCR |= DMA_LIFCR_CTCIF1;     ///. DMA_LIFCR_CTCIF1 = 0x00000800
		
		USART3HOST_RXDMA_STREAM->CR &= ~0x00000001;    // bit_0, close DMA stream for re-config DMA
		
		USART3HOST_RXDMA_STREAM->NDTR = USART3HOST_RXBUFF_SIZE;    // re-config RxBuf size
		
		///.USART3HOST_DMA->HIFCR |= 0x00000800;     // clear TCIF5 flag
		USART3HOST_DMA->LIFCR |= DMA_LIFCR_CTCIF1;
		
		USART3HOST_RXDMA_STREAM->CR |= 0x00000001;    // bit_0, open DMA stream again for next receive
    
	}
	
}
////////////////////////////////////////////////////////////////...usart3 to host interrupt



/*
uint8_t  tdd[]={0xab,0xba,0x11,0x01,0xcc,0xdd};
////////////////////////////////////////////////////53 USART3 interrupt
// usart3 interrupt handler
extern  OS_TCB  taskUsart3bleTCB; 
/////////9 void  USART3_IRQHandler( void )
void USART3BLE_IRQHandler( void )
{
  OS_ERR err;
	OSIntEnter();    // now I in ISR
	
	//uint8_t tdd[4]={0xfd,0xbc,0x71,0x17};
	//rs485_SendFrame( tdd, 4 );	
	//uint8_t rlen;
	if( (USART3BLE->SR & 0x00000010) == 0x00000010 )   // bit_4, idle line detected flag
	{
		(USART3BLE->SR);      //// first read state register for clear the flag
		(USART3BLE->DR);      //// second read data register for clear the flag
		
		usart3ble_lenrxd = USART3BLE_RXBUFF_SIZE - USART3BLE_RXDMA_STREAM->NDTR;     // the data length received.......
		
		USART3BLE_RXDMA_STREAM->CR &= ~0x00000001;    // bit_0, close DMA stream for re-config DMA
		
		USART3BLE_RXDMA_STREAM->NDTR = USART3BLE_RXBUFF_SIZE;    // re-config RxBuf size
		
		USART3BLE_DMA->LIFCR |= 0x00000800;     // clear TCIF1 flag
		
		USART3BLE_RXDMA_STREAM->CR |= 0x00000001;    // bit_0, open DMA stream again for next receive
		
		///////// for test only
	//uart1host_SendString( usart3ble_Rxbuf, usart3ble_lenrxd );
  //uint8_t tdd[4]={0xf3,0xbc,0x71,0x17};
	//rs485_SendFrame( usart3ble_Rxbuf, usart3ble_lenrxd );	
		//USART3->DR = 0x37;  USART3->DR = 0xa5;
		//uart3ble_SendString( tdd, 6 );
		// send Q to task task_Usart3ble, this is a frame from ble 
		OSTaskQPost(  (OS_TCB    *)&taskUsart3bleTCB, 
		              (void      *) usart3ble_Rxbuf,   //
		              (OS_MSG_SIZE) usart3ble_lenrxd,   
		         			(OS_OPT     ) OS_OPT_POST_FIFO,
		              (OS_ERR    *)&err    );  
	}
		
	if( (USART3BLE->SR & 0x00000020) == 0x00000020 )   // bit_5, rx register no empty flag
  {
     (USART3BLE->DR);      //// read data register for clear the flag
		
  }
		
  OSIntExit();      // I leave ISR
}


////////////////////////////////////////////////////53 USART3 RX DMA interrupt
/////////9 void  DMA1_Stream1_IRQHandler( void )
void USART3BLE_DMA_IRQHandler( void )	
{
  OSIntEnter();    // now I in ISR
	
	// DMA1_Stream1_Channel4 for USART3 RxDMA
	if(	(USART3BLE_DMA->LISR & 0x00000800) == 0x00000800 )  // bit_11, Stream1 transfer complete interrupt flag
	{                                          // interrupt flag On...
		USART3BLE_DMA->LIFCR |= 0x00000800;     // clear interrupt TCIF1 flag=0x00000800 
		
		USART3BLE_RXDMA_STREAM->CR &= ~0x00000001;    // bit_0, close DMA stream for re-config DMA
		
		USART3BLE_RXDMA_STREAM->NDTR = USART3BLE_RXBUFF_SIZE;    // re-config RxBuf size
		
		USART3BLE_DMA->LIFCR |= 0x00000800;     // clear TCIF1 flag=0x00000800 
		
		USART3BLE_RXDMA_STREAM->CR |= 0x00000001;    // bit_0, open DMA stream again for next receive
    
	}
	
	OSIntExit();      // I leave ISR
}
*/

/////////////////////////////////////////////////////////////////66 end of file

/* 

*/







