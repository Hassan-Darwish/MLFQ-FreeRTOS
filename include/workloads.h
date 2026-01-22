/******************************************************************************
 *  MODULE NAME  : Workload Tasks
 *  FILE         : workloads.h
 *  DESCRIPTION  : Declares workload task interfaces and configuration
 *                 constants used to simulate different execution behaviors.
 *  AUTHOR       : Ahmed Alaa
 *  Date         : December 2025
 ******************************************************************************/

#ifndef WORKLOADS_H_
#define WORKLOADS_H_

/******************************************************************************
 *  FUNCTION PROTOTYPES
 ******************************************************************************/
/*
 * Description : Simulates a blocking operation by forcing the
 *               calling task to yield execution.
 */
void simulateBlocking(void);

/*
 * Description : Entry function for an interactive workload task
 *               that performs short computations and blocks frequently.
 */
void runInteractiveTask(void *pvParameters);

/*
 * Description : Entry function for a CPU-intensive workload task
 *               that performs long computations before blocking.
 */
void runCPUHeavyTask(void *pvParameters);

#endif /* WORKLOADS_H_ */

/******************************************************************************
 *  END OF FILE
 ******************************************************************************/
