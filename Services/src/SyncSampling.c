// Joseph Grant
// Midterm Problem 8
// 11/1/16

/*******************************************************************************
 * MODULE #INCLUDE                                                             *
 ******************************************************************************/

#include "BOARD.h"
#include "LED.h"
#include "AD.h"
#include "IO_Ports.h"
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "TimerService.h"
#include <stdio.h>

/*******************************************************************************
 * MODULE #DEFINES                                                             *
 ******************************************************************************/
//#define DEBUG
#define TIMER_0_TICKS 500 // 2 ticks = 2 ms
#define ON 1
#define HI_THRESHOLD 150
#define LO_THRESHOLD 100
#define NUM_LEDS 4

/*******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES                                                 *
 ******************************************************************************/
/* Prototypes for private functions for this machine. They should be functions
   relevant to the behavior of this state machine */

/*******************************************************************************
 * PRIVATE MODULE VARIABLES                                                    *
 ******************************************************************************/

static const char *eventName;
static ES_Event storedEvent;
static uint8_t MyPriority;
static unsigned int adPins[]={AD_PORTV5,AD_PORTV6,AD_PORTV7,AD_PORTV8};
static uint16_t ledPins[]={PIN3,PIN4,PIN5,PIN7};
static uint16_t ledBanks[]={LED_BANK1,LED_BANK1,LED_BANK2,LED_BANK2};

/*******************************************************************************
 * PUBLIC FUNCTIONS                                                            *
 ******************************************************************************/

/**
 * @Function InitSyncSamplingService(uint8_t Priority)
 * @param Priority - internal variable to track which event queue to use
 * @return TRUE or FALSE
 * @brief This will get called by the framework at the beginning of the code
 *        execution. It will post an ES_INIT event to the appropriate event
 *        queue, which will be handled inside RunSyncSamplingService function.
 *
 *        Returns TRUE if successful, FALSE otherwise
 */
uint8_t InitSyncSamplingService(uint8_t Priority) {
    ES_Event ThisEvent;

    MyPriority = Priority;

    // I am assuming BOARD_Init(); is called in some beautiful main function
    // AD_Init(); should go there too but I am leaving it here anyways because
    // it is not essential to the main function like BOARD_Init();
    AD_Init();
    LED_Init();
    LED_AddBanks(LED_BANK1|LED_BANK2|LED_BANK3);
    LED_SetBank(LED_BANK1,0x0);
    LED_SetBank(LED_BANK2,0x0);
    LED_SetBank(LED_BANK3,0x0);
    IO_PortsSetPortOutputs(PORTZ, PIN3|PIN4|PIN5|PIN7);
    AD_AddPins(AD_PORTV5|AD_PORTV6|AD_PORTV7|AD_PORTV8);

    // post the initial transition event
    IO_PortsWritePort(PORTZ,IO_PortsReadPort(PORTZ) | PIN3); // This Sets PIN7 (the LED) high for the first sample
    IO_PortsWritePort(PORTZ,IO_PortsReadPort(PORTZ) | PIN4); // This Sets PIN7 (the LED) high for the first sample
    IO_PortsWritePort(PORTZ,IO_PortsReadPort(PORTZ) | PIN5); // This Sets PIN7 (the LED) high for the first sample
    ES_Timer_InitTimer(SYNC_SAMPLE_TIMER, TIMER_0_TICKS);
    ThisEvent.EventType = ES_INIT;
    if (ES_PostToService(MyPriority, ThisEvent) == TRUE) {
        return TRUE;
    } else {
        return FALSE;
    }
}

/**
 * @Function PostSyncSamplingService(ES_Event ThisEvent)
 * @param ThisEvent - the event (type and param) to be posted to queue
 * @return TRUE or FALSE
 * @brief This function is a wrapper to the queue posting function, and its name
 *        will be used inside ES_Configure to point to which queue events should
 *        be posted to.
 *
 *        Returns TRUE if successful, FALSE otherwise
 */
uint8_t PostSyncSamplingService(ES_Event ThisEvent) {
    return ES_PostToService(MyPriority, ThisEvent);
}

/**
 * @Function RunSyncSamplingService(ES_Event ThisEvent)
 * @param ThisEvent - the event (type and param) to be responded.
 * @return Event - return event (type and param), in general should be ES_NO_EVENT
 * @brief This service is responsible for synchronous sampling of a VISHAY TCRT5000.
 *        A sample is taken while the LED is on and while it is OFF and the difference
 *        is compared to a threshold to see if there was tape. 
 * 
 *       Returns ES_NO_EVENT if the event have been "consumed." 
 */
ES_Event RunSyncSamplingService(ES_Event ThisEvent)
{
    int i;
    static ES_EventTyp_t lastEvent = OFF_TAPE;
    //ES_Event postEvent;
    ES_EventTyp_t curEvent;
    ES_Event ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
    static uint8_t curLEDState = ON; // LED Starts as on (in init function).
    static int16_t adcValOn[NUM_LEDS], adcValOff[NUM_LEDS], adcDiff[NUM_LEDS];

    switch (ThisEvent.EventType) 
    {
        case ES_INIT:
            break;

        case ES_TIMERACTIVE:

        case ES_TIMERSTOPPED:
            break;

        case ES_TIMEOUT:
            ES_Timer_InitTimer(SYNC_SAMPLE_TIMER, TIMER_0_TICKS);

            // We always either turn the LED(s) on or off, it is simple!
            if (curLEDState == ON)
            {   
                for(i = 0;i < NUM_LEDS;i++)
                {
                    adcValOn[i] = AD_ReadADPin(adPins[i]); // Read the LED while it has been on for 2 ms 
                    #ifdef DEBUG
                    printf("\r\nadcValOn[%d]: %d",i,adcValOn[i]);
                    #endif
                    IO_PortsWritePort(PORTZ,IO_PortsReadPort(PORTZ) & ~ledPins[i]); // This Sets PINi (the LED) low  
                }
                curLEDState = FALSE;
            } else {
                for(i = 0;i < NUM_LEDS;i++)
                {
                    adcValOff[i] = AD_ReadADPin(adPins[i]); // Read the LED after it has been off for 2 ms
                    #ifdef DEBUG
                    printf("\r\nadcValOff[%d]: %d",i,adcValOff[i]);
                    #endif
                    IO_PortsWritePort(PORTZ,IO_PortsReadPort(PORTZ) | ledPins[i]); // This Sets PIN7 (the LED) high

                    curLEDState = ON;

                    adcDiff[i] = adcValOn[i] - adcValOff[i];
                    #ifdef DEBUG
                    printf("\r\nadcDiff[%d]: %d",i,adcDiff[i]);
                    #endif
                    // Do hystersis check
                    if (adcDiff[i] > HI_THRESHOLD) {
                        if ((i % 2) == 0)
                        {
                            LED_SetBank(ledBanks[i],LED_GetBank(ledBanks[i]) | 0x3);
                        }
                        if ((i % 2) != 0)
                        {
                           LED_SetBank(ledBanks[i],LED_GetBank(ledBanks[i]) | 0xC); 
                        }
                    }
                    else if (adcDiff[i] < LO_THRESHOLD)
                    {
                        if ((i % 2) == 0)
                        {
                            LED_SetBank(ledBanks[i],LED_GetBank(ledBanks[i]) & ~0x3);
                        }
                        if ((i % 2) != 0)
                        {
                           LED_SetBank(ledBanks[i],LED_GetBank(ledBanks[i]) & ~0xC); 
                        }
                    }
                }
            }
                // Check if tape was tripped
                /*if(curEvent == ON_TAPE && lastEvent == OFF_TAPE)
                {
                    printf("\r\nWe triggered!");
                    LED_SetBank(LED_BANK1,0xF);
                   // postEvent.EventType = TAPE_SENSOR_TRIPPED;
                    //postEvent.EventParam = 1;      // I am assuming we are only sampling what is known as tape sensor 1.
                                                   // If we were doing multiple tape sensor 1 might have a #define for it like
                                                   // #define CENTER_TAPE_SENSOR 1
                    //ES_PostToService(MyPriority, postEvent);
                } else if(curEvent == OFF_TAPE)
                {
                    LED_SetBank(LED_BANK1,0x0);
                }
            } */
            break;

        default:
            printf("\r\nRecieved unexpected Event: %s\twith Param: 0x%X",
                    EventNames[ThisEvent.EventType], ThisEvent.EventParam);
            break;
    }
    lastEvent = curEvent;
    return ReturnEvent;
}

// CHANGES NESSESCARY FOR ESCONFIGURE.H FILE

/*

#define TIMER0_RESP_FUNC PostSyncSamplingService
#define SYNC_SAMPLE_TIMER 0
#define NUM_SERVICES 2

These are the definitions for the SyncSampling Service 

#define SERV_1_HEADER "SyncSampling.h"
#define SERV_1_INIT InitSyncSamplingService
#define SERV_1_RUN RunSyncSamplingService
#define SERV_1_QUEUE_SIZE 3

I would also have to add ON_TAPE, OFF_TAPE, 
and TAPE_SENSOR_TRIPPED to the ES_EventTyp_t
enum list and to the EventNames string list.

*/