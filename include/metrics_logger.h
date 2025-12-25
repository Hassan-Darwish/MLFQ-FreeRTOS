/******************************************************************************
 * MODULE NAME  : Metrics Logger
 * FILE         : metrics_logger.h
 * DESCRIPTION  : Implementation of logging and reporting logic for the MLFQ
 * scheduler project.
 * AUTHOR       : Yousef Tantawy
 * DATE         : December 2025
 ******************************************************************************/

#ifndef METRICS_LOGGER_H_
#define METRICS_LOGGER_H_

#include "FreeRTOS.h"
#include "scheduler.h"
#include <stdint.h>

/******************************************************************************
 * MACRO DEFINITIONS AND CONFIGURATIONS
 ******************************************************************************/

#define LOG_BUFFER_SIZE 128

/******************************************************************************
 * FUNCTION PROTOTYPES
 ******************************************************************************/

/*
 * Description : Computes latency given arrival and start times.
 */
float calculateLatency(uint32_t arrivalTick, uint32_t startTick);

/*
 * Description : Formats a TaskStats struct into a readable string.
 * stats: The task statistics to format
 */
char *formatStatsLog(MLFQ_Task_Profiler_t stats);

/*
 * Description : Iterates through all tasks in the scheduler, retrieves their
 * stats, and prints a formatted report over UART.
 */
void printQueueReport(void);

#endif /* METRICS_LOGGER_H_ */
