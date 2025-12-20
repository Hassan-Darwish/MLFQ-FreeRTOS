#include "drivers.h"

/* TI DriverLib includes */
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"

/* =========================================================
 * UART INITIALIZATION
 * ========================================================= */
void initUART(void)
{
    /* Enable peripherals */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    /* Wait until peripherals are ready */
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0));
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));

    /* Configure GPIO pins for UART */
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);

    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    /* Configure UART parameters */
    UARTConfigSetExpClk(
        UART0_BASE,
        SysCtlClockGet(),
        115200,
        UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE
    );

    UARTEnable(UART0_BASE);
}

/* =========================================================
 * GPIO / LED INITIALIZATION
 * ========================================================= */
void initGPIO(void)
{
    /* Enable GPIO Port F */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));

    /* Configure RGB LED pins as output */
    GPIOPinTypeGPIOOutput(
        GPIO_PORTF_BASE,
        GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3
    );

    /* Turn off all LEDs initially */
    GPIOPinWrite(
        GPIO_PORTF_BASE,
        GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,
        0x0
    );
}

/* =========================================================
 * UART LOGGING
 * ========================================================= */
void sendLog(const char *message)
{
    if (message == 0)
        return;

    while (*message)
    {
        UARTCharPut(UART0_BASE, *message);
        message++;
    }
}

/* =========================================================
 * LED QUEUE VISUALIZATION
 * ========================================================= */
void setLEDColor(int queueLevel)
{
    uint8_t ledValue = 0x0;

    switch (queueLevel)
    {
        case 0: /* Highest priority */
            ledValue = GPIO_PIN_3; /* Green */
            break;

        case 1:
            ledValue = GPIO_PIN_2; /* Blue */
            break;

        case 2:
            ledValue = GPIO_PIN_1; /* Red */
            break;

        default:
            ledValue = 0x0;        /* All off */
            break;
    }

    GPIOPinWrite(
        GPIO_PORTF_BASE,
        GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,
        ledValue
    );
}
