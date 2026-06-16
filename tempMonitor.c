/**
 * =====================================================================================
 * @file        tempMonitor.c
 * @brief       EPICS Real-Time Temperature Monitor Simulator
 * @description A multithreaded EPICS application demonstrating the Producer-Consumer 
 * pattern, strict mutex synchronization, and event-driven emergency alarms.
 * Designed to simulate hardware polling and time-critical interlocks.
 * =====================================================================================
 */

#include <stdio.h>
#include <iocsh.h>
#include <epicsExport.h>
#include "epicsThread.h"
#include "epicsMutex.h"
#include "epicsEvent.h"
#include "epicsMessageQueue.h"

/* --- Global Resources (Shared Memory & Sync Tools) --- */
epicsMutexId        dataLock;
epicsEventId        alarmEvent;
epicsMessageQueueId tempQueue;

/* Structure representing the shared hardware state */
typedef struct {
    double currentTemp;
    int isCritical;
} SystemState;

// Initial state: 20.0 °C, no critical alarm
SystemState state = {20.0, 0}; 

/* * =========================================================================
 * 1. ALARM THREAD (Priority 91: Highest)
 * This thread simulates a critical hardware interlock. It sleeps almost
 * 100% of the time to save CPU cycles and ONLY wakes up if an emergency occurs.
 * ========================================================================= 
 */
static void alarmTask(void *arg) {
    printf("[ALARM-THREAD] Started. Sleeping silently until a critical event occurs...\n");

    while(1) {
        /* * Suspend the thread entirely. It waits indefinitely and consumes 0% CPU 
         * until another thread explicitly calls epicsEventSignal().
         */
        epicsEventWait(alarmEvent);
        
        /* If the thread reaches this line, an emergency signal was received! */
        printf("\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
        printf("[ALARM-THREAD] CRITICAL TEMPERATURE REACHED! SHUTTING DOWN MOTOR!\n");
        printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
        
        break; // Exit loop and terminate the thread gracefully
    }
}

/* * =========================================================================
 * 2. PROCESSING THREAD (Priority 50: Medium)
 * The 'Consumer' thread. It waits for incoming data in the Message Queue,
 * processes it, and logs the results. Slower than the hardware polling thread.
 * ========================================================================= 
 */
static void processingTask(void *arg) {
    double receivedTemp;
    printf("[PROC-THREAD] Started. Waiting for data in the Queue...\n");
    
    while(1) {
        /*
         * Blocks until data arrives. Once data is available, it pops the oldest 
         * message into 'receivedTemp' and frees up space in the queue automatically.
         */
        epicsMessageQueueReceive(tempQueue, &receivedTemp, sizeof(double));
        
        printf("[PROC-THREAD] Consumed Data: Temperature is now %.2f °C\n", receivedTemp);
        
        // Simulate data processing time
        epicsThreadSleep(0.05);
    }
}

/* * =========================================================================
 * 3. SENSOR THREAD (Priority 90: High)
 * The 'Producer' thread. Interacts directly with the hardware interface.
 * Requires high priority to ensure consistent polling intervals.
 * ========================================================================= 
 */
static void sensorTask(void *arg) {
    double temp = 20.0;
    printf("[SENSOR-THREAD] Started. Polling hardware every 0.2 seconds...\n");
    
    while(1) {
        // Simulate hardware reading (temperature rising)
        temp += 12.5;

        // Push new reading to the queue for the processing thread
        epicsMessageQueueSend(tempQueue, &temp, sizeof(double));

        // Protect shared memory before updating the global state
        epicsMutexLock(dataLock);
        state.currentTemp = temp;

        // Check for emergency threshold
        if (state.currentTemp >= 60.0 && state.isCritical == 0) {
            state.isCritical = 1;
            
            // CRITICAL: Always unlock the mutex BEFORE triggering the wake-up signal
            epicsMutexUnlock(dataLock);
            epicsEventSignal(alarmEvent);
        } else {
            // Normal operation: just unlock
            epicsMutexUnlock(dataLock);
        }
        
        // Prevent CPU starvation for lower priority threads
        epicsThreadSleep(0.2);
    }
}

/* * =========================================================================
 * 4. INITIALIZATION & EPICS REGISTRATION
 * Sets up the OSI abstraction tools, spawns threads, and registers the 
 * function with the EPICS IOC shell.
 * ========================================================================= 
 */
void startTempMonitor(void) {
    printf("\n--- Initializing EPICS Real-Time Temperature Monitor ---\n");
    
    // Initialize synchronization primitives
    dataLock   = epicsMutexCreate();
    alarmEvent = epicsEventCreate(epicsEventEmpty);
    tempQueue  = epicsMessageQueueCreate(10, sizeof(double));

    // Spawn Threads with explicitly defined priorities and stack sizes
    epicsThreadCreate("AlarmTask",
                      91,
                      epicsThreadGetStackSize(epicsThreadStackSmall),
                      (EPICSTHREADFUNC)alarmTask,
                      NULL);

    epicsThreadCreate("SensorTask",
                      90,
                      epicsThreadGetStackSize(epicsThreadStackMedium),
                      (EPICSTHREADFUNC)sensorTask,
                      NULL);

    epicsThreadCreate("ProcTask",
                      50,
                      epicsThreadGetStackSize(epicsThreadStackMedium),
                      (EPICSTHREADFUNC)processingTask,
                      NULL);
}

/* --- IOC Shell Registration --- */
static const iocshFuncDef startTempMonitorFuncDef = {"startTempMonitor", 0, NULL};

static void startTempMonitorCallFunc(const iocshArgBuf *args) {
    startTempMonitor();
}

static void tempMonitorRegister(void) {
    iocshRegister(&startTempMonitorFuncDef, startTempMonitorCallFunc);
}

epicsExportRegistrar(tempMonitorRegister);
