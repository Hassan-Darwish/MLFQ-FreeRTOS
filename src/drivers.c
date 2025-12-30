/******************************************************************************
 *  MODULE NAME  : Tiva-C Driver Layer
 *  FILE         : drivers.c
 *  DESCRIPTION  : Provides UART initialization, GPIO initialization,
 *                 logging over UART, and LED control based on queue level.
 *  AUTHOR       : Omar Ashraf
 *  Date         : December 2025
 ******************************************************************************/

/******************************************************************************
 *  INCLUDES
 ******************************************************************************/
#include "drivers.h"
#include "TivaWare/driverlib/hw_memmap.h"
#include "TivaWare/driverlib/hw_types.h"
#include "TivaWare/driverlib/sysctl.h"
#include "TivaWare/driverlib/gpio.h"
#include "TivaWare/driverlib/uart.h"
#include "TivaWare/driverlib/pin_map.h"

/******************************************************************************
 *  FUNCTION DEFINITIONS
 ******************************************************************************/

/*
 * Description : Initializes UART0 peripheral.
 *               Configures GPIO pins for UART RX/TX,
 *               sets baud rate to 115200, 8-bit data,
 *               no parity, and one stop bit.
 */
void initUART(void)
{
    /* Enable UART0 and GPIOA peripherals */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    /* Wait until peripherals are ready */
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0));
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));

    /* Configure GPIO pins for UART functionality */
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);

    /* Set GPIO pins as UART pins */
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    /* Configure UART parameters */
    UARTConfigSetExpClk(
        UART0_BASE,
        SysCtlClockGet(),
        115200,
        UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE
    );

    /* Enable UART module */
    UARTEnable(UART0_BASE);
}

/*
 * Description : Initializes GPIO Port F pins.
 *               Configures RGB LED pins as output
 *               and turns all LEDs off initially.
 */
void initGPIO(void)
{
    /* Enable GPIO Port F */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    /* Wait until GPIO Port F is ready */
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));

    /* Configure LED pins as output */
    GPIOPinTypeGPIOOutput(
        GPIO_PORTF_BASE,
        GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3
    );

    /* Turn off all LEDs */
    GPIOPinWrite(
        GPIO_PORTF_BASE,
        GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,
        0x0
    );
}

/*
 * Description : Sends a null-terminated string over UART.
 *               Used for logging and debugging messages.
 */
void sendLog(const char *message)
{
    /* Check for null pointer */
    if (message == 0)
        return;

    /* Send each character until end of string */
    while (*message)
    {
        UARTCharPut(UART0_BASE, *message);
        message++;
    }
}

/*
 * Description : Sets LED color based on MLFQ queue level.
 *               High    -> Green LED
 *               Medium  -> Blue LED
 *               Low     -> Red LED
 *               Default -> All LEDs off
 */
void setLEDColor(MLFQ_QueueLevel_t queueLevel)
{
    /* LED output value */
    uint8_t ledValue = 0x0;

    /* Select LED based on queue level */
    switch (queueLevel)
    {
        case MLFQ_QUEUE_HIGH:
            ledValue = GPIO_PIN_3;
            break;

        case MLFQ_QUEUE_MEDIUM:
            ledValue = GPIO_PIN_2;
            break;

        case MLFQ_QUEUE_LOW:
            ledValue = GPIO_PIN_1;
            break;

        default:
            ledValue = 0x0;
            break;
    }

    /* Update LED output */
    GPIOPinWrite(
        GPIO_PORTF_BASE,
        GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,
        ledValue
    );
}

/******************************************************************************
 *  END OF FILE
 ******************************************************************************/
