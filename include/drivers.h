/******************************************************************************
 *  MODULE NAME  : Tiva-C Driver Layer Header
 *  FILE         : drivers.h
 *  DESCRIPTION  : Header file for UART, GPIO, LED control functions
 *                 and logging for Tiva-C platform.
 *  AUTHOR       : Omar Ashraf
 *  Date         : December 2025
 ******************************************************************************/

#ifndef DRIVERS_H
#define DRIVERS_H

/******************************************************************************
 *  INCLUDES
 ******************************************************************************/
#include "scheduler.h"  /* Include scheduler definitions for MLFQ_QueueLevel_t */

/******************************************************************************
 *  FUNCTION PROTOTYPES
 ******************************************************************************/

/* Description : Initializes UART0 peripheral with 115200 baud, 8N1 settings */
void initUART(void);

/* Description : Initializes GPIO Port F pins as output and turns LEDs off */
void initGPIO(void);

/* Description : Sends a null-terminated string over UART0 for logging */
void sendLog(const char *message);

/* Description : Sets RGB LED color based on MLFQ queue level */
void setLEDColor(MLFQ_QueueLevel_t queueLevel);

#endif /* DRIVERS_H */

/******************************************************************************
 *  END OF FILE
 ******************************************************************************/
