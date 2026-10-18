/**
  ******************************************************************************
  * @file      sys_abc.h
  * @author    liyc
  * @version   V1.0
  * @date      2024-4-28
  * @brief     RSD79 board code
  *            the basic function and decleration of mcu
  ******************************************************************************
  **/ 


#ifndef __SYS_ABC_H
#define	__SYS_ABC_H



#include <stdint.h>


void system_ConfigMisc( uint32_t addrstart );

void system_ConfigClock( void );
void system_ConfigClockX( void );


#endif   /* __SYS_ABC_H */


/////////////////////////////////////////////////////////////////66 end of file




