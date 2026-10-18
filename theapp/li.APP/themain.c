
/*
********************************************************************************
*
*                      the main code file themain.c
*
*                                       
*                  on STM32F407vet6 in the pxb3430 board
*
* Filename      : themain.c
* Version       : V1.00
* Programmer(s) : liyc
*                 
********************************************************************************
*/

/*
********************************************************************************
*                            INCLUDE FILES
********************************************************************************
*/


#include "./sys_abc.h"
#include "./util_misc.h"
#include "./util_time.h"
#include "./sys_iap.h"

#include "../lib.BSP/Inc/bsp_iowhich.h"
#include "../lib.BSP/Inc/bsp_color2812Ctrl.h"
#include "../lib.BSP/Inc/bsp_can1wheels.h"
#include "../lib.BSP/Inc/bsp_pwrCtrl.h"
#include "../lib.BSP/Inc/bsp_miscCtrl.h"
#include "../lib.BSP/Inc/bsp_adc.h"
#include "../lib.BSP/Inc/bsp_iic.h"


#include "../lib.BSP/Inc/bsp_timer.h"
#include "../lib.BSP/Inc/bsp_usart1host.h"


#include "../lz.test/lz_test.h"



/*
********************************************************************************
*                        LOCAL DEFINES
********************************************************************************
*/


/*
********************************************************************************
   2	  3	   5	  7	  11	 13	  17	 19	 
  23	 29   31	 37	  41	 43	  47	 53	  59	 
  61	 67	  71   73	  79	 83	  89	 97	
	
	101	 103	107	 109	113  127	131	 137	139	 149
	151	 157	163	 167	173  179	181	 191	193	 197	199	

  211	 223	227	 229  233	 239	241	 251	257	 	
  263	 269  271	 277	281  283	293	

  307	 311	313	 317	331	 337	347	 349
  353	 359	367	 373	379	 383	389	 397	

  401	 409  419	 421	431	 433	439	 443	449	 
	457  461	463  467	479	 487	491	 499	

  503	 509	521	 523	541  547	557
	563	 569	571	 577	587	 593	599	
	
	601  607	613	 617	619	 631	641	 643	647
	653	 659  661	 673	677	 683	691
               
	701	 709	719	 727	733  739	743	 751
	757	 761	769	 773	787	 797	
	809  811	821	 823	827	 829	839	 853	 	
  857  859	863  877	881	 883	887	
  907	 911	919	 929	937	 941  947	
  953	 967	971	 977	983	 991	997							 
********************************************************************************
*/
  


 
/*
********************************************************************************
*                                                
********************************************************************************
*/


/*
********************************************************************************
*                        FUNCTION PROTOTYPES
********************************************************************************
*/

//extern void system_ConfigClock( void );


/*
*********************************************************************************

*********************************************************************************
*/ 

///, must set default tempe, otherwise will be zero
uint8_t tempeT_drive =103;
uint8_t tempeT_chgx =107;
uint8_t toflashtempeT =0x00;

uint8_t  flag_mchargeing =0x00;
uint32_t  time_mcharge =0;

#define  RATIO_VOLTADC  (15.21f)

#define TOFLASHTEMPET  (0x37)
#define TORESETSELF  (0x74)

extern uint8_t  togetnewcode;  ///, =0xff;
static void resetmyself( void )
{
  __disable_irq();
	util_waitms( 479 );
  NVIC_SystemReset();   ///, look, reset now...
}

///, #define FLASH_SECTOR3     (0x0800C000) 16kB
///, #define FLASH_SECTOR4     (0x08010000) 64kB
#define TEMPET_ADDRESS  (FLASH_SECTOR4 +16*20)
///, will use 16 words to store tempeT.
static void read_tempeT( void )
{
	uint32_t tep32;
	
  tep32 = *(uint32_t*)TEMPET_ADDRESS;
	
	if( tep32 != 0xffffffff )
	{
	  tempeT_chgx = (uint8_t)((tep32 >>24) &0x000000ff); 
		tempeT_drive = (uint8_t)((tep32 >>16) &0x000000ff); 
	}   ///, read tempeT if it is there, otherwise do nothing
}
static void write_tempeT( void )
{
  FLASH_Status status;   ///, FLASH_COMPLETE;
		
	 FLASH_Unlock( );
	FLASH_DataCacheCmd( DISABLE );
	
	uint32_t tepd32 =0x00000000;
	
	tepd32 |= (uint32_t)tempeT_chgx <<24;
	tepd32 |= (uint32_t)tempeT_drive <<16;
	
	  status = FLASH_ProgramWord( TEMPET_ADDRESS, tepd32 );
		while( status != FLASH_COMPLETE ) { ; }    
	  	
	FLASH_DataCacheCmd( ENABLE );	
	 FLASH_Lock( );
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////57 the main function
int main( void )    
{
    	
	system_ConfigMisc( 0x00020000 );    ///, vector table at 0x08020000
///.	system_ConfigClock();
	
	///.MCO1_Test_Init();
	///.MCO2_Test_Init();
	///RCC_APB2PeriphClockCmd( RCC_APB2Periph_AFIO, ENABLE );
	///GPIO_PinRemapConfig( GPIO_RMP_SW_JTAG_SW_ENABLE, ENABLE );
	
	GPIO_DeInit( GPIOA ); 
	  mpwr_signals_Config(); mpwr_toPowerOff( );
	GPIO_DeInit( GPIOB ); 
	  htout_signals_Config( ); htout_toPowerOn( );
	GPIO_DeInit( GPIOC );
	GPIO_DeInit( GPIOD ); GPIO_DeInit( GPIOE ); ///,GPIO_DeInit( GPIOF );
	
	__enable_irq();    ///, MUST re-enable irq, DDDDDDDD 
	uint8_t toreset =0x00;
///,	uint8_t toflashtempeT =0x00;
  uint8_t  xxdata[]={ 0x21,0x32,0x43,0x54,0x65,0x76,0x87,0x98, 0xa9,0xba,0xcb,0xdc,0xed,0xfe };
	uint16_t  adcvalue =0;
	
	uint16_t  v_bmv =0;
	uint16_t  vbat_adcvalue =0;
	uint16_t  vchga_adcvalue =0;
	uint16_t  vchgm_adcvalue =0;
	uint8_t  flag_chgm, flag_chga, flag_bat;
	
	int8_t tempe_chgu19 =0;
	int8_t tempe_driveu20 =0;

	
/////////////////////////////////////, configs	
	color2812_ctrl_Config();
	
	///, uint8_t color[]={ 0x77, 0x88, 0x00 };   ///, yellow
	color2812_RunLDx( COLOR_BLUE );
	/*util_waitms( 456 );
	for( int ii=0; ii<2; ii++ )
	{
	  color2812_RunLDx( COLOR_GREEN ); 	util_waitms( 756 );
		color2812_RunLDx( COLOR_RED );	util_waitms( 756 );
		color2812_RunLDx( COLOR_BLUE );  util_waitms( 756 );
		///,color2812_RunLDx( COLOR_YELLOW );  util_waitms( 1456 );
		color2812_RunLDx( COLOR_WHITE );  util_waitms( 756 );
	}  */
	
	
	
	can1wheels_Init( CAN1BAUDRATE500K ); ///, CAN1BAUDRATE250K
	can1wheels_SendMessage( xxdata, 7, 0x007 );  ///, I am ready to work...
	
	 ///, mpwr_signals_Config( );   ///, lik2677 need or not? see line179
  ///,htout_signals_Config( );  ///, if need power on at first, config 0time at line181. 
  htout1_signals_Config( );
	htout3_signals_Config( );
	htout4_signals_Config( );
	htout5_signals_Config( );
	htout6_signals_Config( );
	
	lout1_signals_Config( );
	lout2_signals_Config( );
	lout3_signals_Config( );
	lout4_signals_Config( );
	lout5_signals_Config( );
	lout6_signals_Config( );
	
	ctlina_signals_Config( );
	ctlinm_signals_Config( );
	ctlfan_signals_Config( );
	
	///,ctlinm_toPowerOff();   ///, must NO charge first
	///,ctlina_toPowerOn();
	
	adc1_Config( );
	adc1_StartConv( VBAT_ADC_CHANNEL );   ///, start forever?
	
	iic1_Config( );
	
	read_tempeT();   ///, read tempeT from Flash and set to work
	
	ctlinm_toPowerOff();   ///, chgm must be off first
///////////////////////////////////////////, the long loop 
	uint32_t  i;
	uint32_t k17,k29,k37,k41;
	///, uint32_t k11,k13,k19,k23,k31;
	uint32_t m223,m743, m977, z101;
	///, uint32_t m137,m173;
	while(1)
	{  

	/*	if( cand500.kon.byte0bit.ctlchgm ==1 )
	  { ctlinm_toPowerOn(); }
	  else { ctlinm_toPowerOff(); }   */
	
	  if( cand500.kon.byte0bit.ctlchga ==1 )
	  { ctlina_toPowerOn(); }
	  else { ctlina_toPowerOff(); }                ///, comment for test only
	
	  if( cand500.kon.byte0bit.ctlfan ==1 )
	  { ctlfan_toPowerOn(); }
	  else { ctlfan_toPowerOff(); }
	
	  if( cand500.kon.byte0bit.ctldriveleft ==1 )  ///, power wheels and its drives
	  { mpwr_toPowerOn(); }
	  else { mpwr_toPowerOff(); }
	
///	if( cand500.kon.byte0bit.ctldriveright ==1 )
///	{ mpwr_toPowerOn(); }     ///, one same signal control left and right drive
///	else { mpwr_toPowerOff(); }  
	
	  if( cand500.kon.byte0bit.reset ==1 )
	  {  toreset =TORESETSELF;  }
	
	/////////////////////////////, HTOUT
	  if( cand500.htout.byte2bit.ctlhtout ==1 )
	  { htout_toPowerOn(); }
	  else { htout_toPowerOff(); }
	
	  if( cand500.htout.byte2bit.ctlhtout1 ==1 )
	  { htout1_toPowerOn(); }
	  else { htout1_toPowerOff(); }
	
		 if( cand500.htout.byte2bit.ctlhtout3 ==1 )
	  { htout3_toPowerOn(); }
	  else { htout3_toPowerOff(); }
		 if( cand500.htout.byte2bit.ctlhtout4 ==1 )
	  { htout4_toPowerOn(); }
	  else { htout4_toPowerOff(); }
		 if( cand500.htout.byte2bit.ctlhtout5 ==1 )
	  { htout5_toPowerOn(); }
	  else { htout5_toPowerOff(); }
		 if( cand500.htout.byte2bit.ctlhtout6 ==1 )
	  { htout6_toPowerOn(); }
	  else { htout6_toPowerOff(); }
		
	/////////////////////////////, LOUT	
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
	  else { lout4_toPowerOff(); }
		
		 if( cand500.lout.byte4bit.ctllout5 ==1 )
	  { lout5_toPowerOn(); }  
	  else { lout5_toPowerOff(); }
		 if( cand500.lout.byte4bit.ctllout6 ==1 )
	  { lout6_toPowerOn(); }  
	  else { lout6_toPowerOff(); }
		
		if( toflashtempeT ==TOFLASHTEMPET )  ///, must store tempeT to Flash 
		{
		  write_tempeT();  
			toflashtempeT =0x00;
		}
		
		
/////////////////////////////////////////////////////////////////////////////////////////,91		
		util_waitms( 19 );    ///, the work time duration33/23
		i++;                ///, the counter of loop for phase control
		
////////////////////////////////////////////////////,54		
		k17 =i%17;
		switch( k17 )   //////, normal work: start adc conversion and get voltages
		{
		  case 0: adc1_StartConv( VCHGM_ADC_CHANNEL ); break;   ///, VCHGM_ADC_CHANNEL
			 case 3:   adc1_GetConvValue( &vchgm_adcvalue ); break;   ///, vchgm_adcvalue
			case 6: adc1_StartConv( VCHGA_ADC_CHANNEL ); break;
			 case 9:   adc1_GetConvValue( &vchga_adcvalue ); break;
			case 12: adc1_StartConv( VBAT_ADC_CHANNEL ); break;    ///, VBAT_ADC_CHANNEL
			 case 15:   adc1_GetConvValue( &vbat_adcvalue ); break;   ///, vbat_adcvalue
			default: break;
		}   //////. normal work: start adc conversion and get voltages		
///////////////////////////////////////////////////////////.61
 
  v_bmv =	vchgm_adcvalue / RATIO_VOLTADC;  ///,(15.21f);	
	if( v_bmv > 268  )  { flag_chgm = 0xa9; }   ///, chgm is plug in 0x109=265  vchgm > 268
	else if( v_bmv > 256 )  { flag_chgm = 0xa4; }   ///, chgm link to bat  256 < vchgm <268
  else if( v_bmv < 37  ) { flag_chgm = 0x03; }
	else { flag_chgm = 0x47; }           ///, 37 < vchgm <256
	
	if( flag_chgm ==0xa9 /*&& flag_bat != 0xa6*/ && flag_mchargeing ==0x00 )   ///, first turn on
  { ctlinm_toPowerOn();  flag_mchargeing =0x67;  time_mcharge =0;   } 
	///, lik26622 should poweron mcharge instantly when vm inserted......	
		

////////////////////////////////////////////////////,54		
		k29 =i%29;
		switch( k29 )   //////, normal work: send voltages
		{
	    case 0:    ///, send voltage in *100mV
		    v_bmv =	vbat_adcvalue / RATIO_VOLTADC;  ///,(15.21f);
			if( v_bmv > 250  )  { flag_bat = 0xa6; }   ///, battery is ok
			else if( v_bmv < 230 )  { flag_bat = 0x39; }   ///, battery need charge
			else  { flag_bat = 0x58; }    ///, battery is maybe down 
		     xxdata[1] =v_bmv>>8;  xxdata[0] =v_bmv; 
			
		    v_bmv =	vchgm_adcvalue / RATIO_VOLTADC;  ///,(15.21f);	
	///,		if( v_bmv > 260  )  { flag_chgm = 0xa9; }   ///, chgm is plug in 0x109=265
	///,		else if( v_bmv < 34 )  { flag_chgm = 0x03; }   ///, chgm is not plug in
	///,		else { flag_chgm = 0x47; }   ///, chgm is maybe in question
		     xxdata[3] =v_bmv>>8;  xxdata[2] =v_bmv;
			
		    v_bmv =	vchga_adcvalue / RATIO_VOLTADC;  ///,(15.21f);	
			if( v_bmv > 265  )  { flag_chga = 0xa9; }   ///, chga is touch at
			else if( v_bmv < 34 )  { flag_chga = 0x03; }   ///, chga is not touch at
			else { flag_chga = 0x47; }   ///, chga is maybe in question
		     xxdata[5] =v_bmv>>8;  xxdata[4] =v_bmv;
	///,	xxdata[6] =adcvalue>>8;  xxdata[7] =adcvalue; 
		    
		    can1wheels_SendMessage( xxdata, 8, 0x590 );    ///, send frame of voltage  
			break;
		
      default: break;			
		}   //////. normal work: send voltages
		
////////////////////////////////////////////////////,54		
		k37 =i%37;
		switch( k37 )   //////, normal work: read tempes
		{
			case 0:
		    tempe_driveu20 = iic1_tcn75_ReadTempe( 0 );  break;  ///, U20 drive
		  case 19: 
			  tempe_chgu19 = iic1_tcn75_ReadTempe( 4 );  break;  ///, U19 chg
		  
			case 23:
			  if( tempe_chgu19 > tempeT_chgx )   ///, stop charge because of high tempe
				{
					cand500.kon.byte0bit.ctlchga =0;   ///, reset chg cmd flag
					cand500.kon.byte0bit.ctlchgm =0;
				///,  ctlina_toPowerOff();  ctlinm_toPowerOff();
				}
				if( tempe_driveu20 > tempeT_drive )  ///, stop drive because of high tempe
				{
					cand500.kon.byte0bit.ctldriveleft =0;
				///,  mpwr_toPowerOff();   ///, should set state of drive power....
				}
			break;
				
			case 29: 
		            //////, normal work: send tempes
		    xxdata[1] =tempe_driveu20;  xxdata[0] =tempe_chgu19; 
		    
		    xxdata[3] =0;  xxdata[2] =0;
		    xxdata[5] =0;  xxdata[4] =0;
			  xxdata[6] =0;  xxdata[7] =0; 
		
			  can1wheels_SendMessage( xxdata, 8, 0x5a0 );    ///, send frame of tempe
			break;	
				
			default: break;
		}   //////. normal work: read tempes
///////////////////////////////////////////////////////////.61		
		
/*		k31 =i%31;
		switch( k31 )   //////, normal work: send tempes
		{
	    case 0: 
		    
		    xxdata[1] =tempe_driveu20;  xxdata[0] =tempe_chgu19; 
		    
		    xxdata[3] =0;  xxdata[2] =0;
		    xxdata[5] =0;  xxdata[4] =0;
			  xxdata[6] =0;  xxdata[7] =0; 
		
			  can1wheels_SendMessage( xxdata, 8, 0x5a0 );    ///, send frame of tempe
			break;
		
			default: break;			
		}   //////. normal work: send tempes   */
///////////////////////////////////////////////////////////.61

////////////////////////////////////////////////////,54		
    k41 =i%41;
		switch( k41 )   //////, normal work: send states
		{
	    case 0: 
		          ///, should get the real state from sensors lik26313...................
		    xxdata[0] =cand500.kon.byte0;     xxdata[1] =cand500.byte1; 
		    xxdata[2] =cand500.htout.byte2;		 xxdata[3] =cand500.byte3;
			  xxdata[4] =cand500.lout.byte4;    xxdata[5] =cand500.byte5;
		    xxdata[6] =cand500.byte6;  xxdata[7] =cand500.byte7; 
		
			xxdata[7] = time_mcharge &0x000000ff;  ///, test mcharge time
			
		    can1wheels_SendMessage( xxdata, 8, 0x580 );    ///, send frame of states
			break;
//////////////////////////////////////////, response to reset command 			
		  case 17:
				if( toreset ==TORESETSELF ) 
        { 
					xxdata[0] =cand500.kon.byte0;     xxdata[1] =cand500.byte1; 
		      xxdata[2] =cand500.htout.byte2;
					can1wheels_SendMessage( xxdata, 8, 0x580 );    ///, send frame of states
				  ///,resetmyself();     ///, I will reset myself...
				}
			break;
			case 19: 
        if( toreset ==TORESETSELF )
        {	
					xxdata[0] =cand500.kon.byte0;     xxdata[1] =cand500.byte1; 
		      xxdata[2] =cand500.htout.byte2;
				  can1wheels_SendMessage( xxdata, 8, 0x580 );    ///, send frame of states
				  resetmyself();    ///, now I do reset myself...
				}
			break;   
//////////////////////////////////////////. response to reset command
      default: break;			
		}   //////. normal work: send states
///////////////////////////////////////////////////////////.61		
		
////////////////////////////////////////////////////,m977				
		m977 =i%977;      ///, lik26326
		switch( m977 )    ///, will charge battery when chgm plug in 
		{
		  case 13:
		///,	  if( flag_chgm ==0xa9 && flag_mchargeing ==0x00 )     ///, logic must check....
    ///,    { ctlinm_toPowerOn();  flag_mchargeing =0x67;     }
			 break;	
			case 14:
				if( flag_mchargeing ==0x67 )
				{ time_mcharge++;  }
			 break;
			case 371:
			  ///,if( flag_chgm ==0x03 ) { ctlinm_toPowerOff(); }	
			 break;
      case 713:
        if( /*flag_mchargeing ==0x67 &&*/ flag_chgm ==0x03 )   ///, if vm lost, and is mcharging
				{	ctlinm_toPowerOff();	flag_mchargeing =0x00;	time_mcharge =0;	}
       break;		
      case 751:        ///, 15~=4.8minute @19ms*977
        if( time_mcharge >(15*6) && flag_mchargeing ==0x67 )
				{ ctlinm_toPowerOff();	flag_mchargeing =0x58;  time_mcharge =0;   
				      util_waitms( 697 );  }	 ///, off for check plugin chgm or not....................
       break; 			
			case 887:
			  ///,if( flag_chgm ==0x47 ) { ctlinm_toPowerOff(); }
			  if( flag_mchargeing ==0x58 && flag_chgm ==0xa9 )
			  { ctlinm_toPowerOn();  flag_mchargeing =0x67;  time_mcharge =0; }
			
			 break;	
      default: break;				
		}                                    ///, if vbat is not there, how to do?
///////////////////////////////////////////////////////////.m977		

////////////////////////////////////////////////////,54		
		m743 =i%743;
		switch(m743)    //////, normal work: send my version
		{
			case 0:     ///, should set myversion1 and myversion2 .......... lik26313 
		    xxdata[0] ='P';  xxdata[1] ='X';  xxdata[2] ='3';  xxdata[3] ='4';   
		    xxdata[4] ='0';  xxdata[5] ='2'; xxdata[6] ='0';  xxdata[7] ='9';
		   can1wheels_SendMessage( xxdata, 8, 0x5b0 );   ///, myversion_1 upsend
		  break;
			
			case 3:     ///, date 20260417
			  xxdata[0] ='2';  xxdata[1] ='0';  xxdata[2] ='2';  xxdata[3] ='6';  
		    xxdata[4] ='0';  xxdata[5] ='6'; xxdata[6] ='2';  xxdata[7] ='2';
		   can1wheels_SendMessage( xxdata, 8, 0x5b1 );   ///, myversion_2 upsend
		  break;
			default: break;
		}   //////. normal work: send my version
///////////////////////////////////////////////////////////.61		
		
		
////////////////////////////////////////////////////,54		
		m223 =i%223;   ///, 191/223/
		switch(m223)   //////, normal work: flash color led
		{
			case 0: color2812_RunLDx( COLOR_GREEN ); break;  ///,COLOR_GREEN
		///,	case 48+19: color2812_RunLDx( COLOR_RED ); break;
		///,	case 96+36: color2812_RunLDx( COLOR_BLUE ); break;
			case 119: color2812_RunLDx( COLOR_NULL ); break;
			default: break;
		}   //////. normal work: flash color led
///////////////////////////////////////////////////////////.61		
	
		
////////////////////////////////////////////////////,54	  
		z101 =i% 13;   ///, z101, 67, 43, 13
		switch( z101 )
		{
		  case 0: 
			  if( togetnewcode ==0x35 )	 ///, togetnewcode = 0x35;
			  {
				  xxdata[0] =0x01;  xxdata[1] =0x35; 
				  can1wheels_SendMessage( xxdata, 2, 0x558 );  
					togetnewcode =0x36;
				} 
			 break;
				
			case 2: 
			  if( togetnewcode ==0x36 )	 ///, togetnewcode =0x36;
			  {
				  xxdata[0] =0x01;  xxdata[1] =0x36;    ///, for test only
				  can1wheels_SendMessage( xxdata, 2, 0x558 );
					
	////////////////////////////////////////, flash flag ok?? 20260417
					#define  BOOT_FLAG  0x12345678
					#define  BOOT_FLAG_ADDR  0x08010000
					uint32_t BootFlag = ~BOOT_FLAG;
					///,STM_Flash_Write( BOOT_FLAG_ADDR, (uint32_t *)&BootFlag, 1 );
					Iap_ProgramFlash32( BOOT_FLAG_ADDR, (uint8_t *)&BootFlag, 4 );
					///,Iap_ProgramFlash32( pflash, data_iap, jpont );
	////////////////////////////////////////. 				
          resetmyself();					  
				} 
			 break;
			default: break;				
		}
///////////////////////////////////////////////////////////.61				
		
	}  
	
	

	
}














/////////////////////////////////////////////////////////////////.66 end of file

/////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////53 the test main function




/* 



*/




