#ifndef DRIVERS_H
#define DRIVERS_H

/* Initializes UART0 for debug logging */
void initUART(void);

/* Initializes GPIO pins for LED output */
void initGPIO(void);

/* Sends a log message over UART */
void sendLog(const char *message);

/* Sets LED color based on MLFQ queue level */
void setLEDColor(int queueLevel);

#endif /* DRIVERS_H */
