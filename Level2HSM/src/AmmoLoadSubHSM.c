/*
 * File:   AmmoLoadSubHSM.h
 * Author: jdgrant
 *
 * This file includes the top level of our hierarchal state machine. At this level
 * the robot performs the basic logic of finding loading stations, loading ammo, 
 * and finding targets to deposit the ammo. This basic logic is looped.
 * 
 */

/*******************************************************************************
 * MODULE #INCLUDE                                                             *
 ******************************************************************************/

#include "ES_Configure.h"
#include "ES_Framework.h"
#include "BOARD.h"
#include "AmmoLoadSubHSM.h"
#include "sensors.h"
#include "motor.h"

/*******************************************************************************
 * PRIVATE #DEFINES                                                            *
 ******************************************************************************/

/*******************************************************************************
 * MODULE #DEFINES                                                             *
 ******************************************************************************/

typedef enum {
    InitPState,
    TankTurn,
    Backward,
} HSMState_t;

static const char *StateNames[] = {
	"InitPState",
	"TankTurn",
	"Backward",
};


/*******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES                                                 *
 ******************************************************************************/
/* Prototypes for private functions for this machine. They should be functions
   relevant to the behavior of this state machine
   Example: char RunAway(uint_8 seconds);*/
/*******************************************************************************
 * PRIVATE MODULE VARIABLES                                                            *
 ******************************************************************************/
/* You will need MyPriority and the state variable; you may need others as well.
 * The type of state variable should match that of enum in header file. */

static HSMState_t CurrentState = InitPState; // <- change enum name to match ENUM
static uint8_t MyPriority;


/*******************************************************************************
 * PUBLIC FUNCTIONS                                                            *
 ******************************************************************************/

/**
 * @Function InitAmmoLoadSubHSM(uint8_t Priority)
 * @param Priority - internal variable to track which event queue to use
 * @return TRUE or FALSE
 * @brief This will get called by the framework at the beginning of the code
 *        execution. It will post an ES_INIT event to the appropriate event
 *        queue, which will be handled inside RunAmmoLoadSubHSM function.
 */
uint8_t InitAmmoLoadSubHSM(uint8_t Priority)
{
    MyPriority = Priority;
    // put us into the Initial PseudoState
    CurrentState = InitPState;
    // post the initial transition event
    if (ES_PostToService(MyPriority, INIT_EVENT) == TRUE) {
        return TRUE;
    } else {
        return FALSE;
    }
}

/**
 * @Function PostAmmoLoadSubHSM(ES_Event ThisEvent)
 * @param ThisEvent - the event (type and param) to be posted to queue
 * @return TRUE or FALSE
 * @brief This function is a wrapper to the queue posting function.
 */
uint8_t PostAmmoLoadSubHSM(ES_Event ThisEvent)
{
    return ES_PostToService(MyPriority, ThisEvent);
}

/**
 * @Function RunAmmoLoadSubHSM(ES_Event ThisEvent)
 * @param ThisEvent - the event (type and param) to be responded.
 * @return Event - return event (type and param), in general should be ES_NO_EVENT
 * @brief This is the implementation for the top level of our state machine and 
 *        it contains the basic logic for our autonomous robot to meet the minimum
 *        specs of the competition.
 */
ES_Event RunAmmoLoadSubHSM(ES_Event ThisEvent)
{
    uint8_t makeTransition = FALSE; // use to flag transition
    static uint8_t turnParam; // use this flag to turnCW or turnCCW
    HSMState_t nextState; // <- change type to correct enum

    ES_Tattle(); // trace call stack

    switch (CurrentState) {
    case InitPState: // If current state is initial Pseudo State
        if (ThisEvent.EventType == ES_INIT)// only respond to ES_Init
        {
            // this is where you would put any actions associated with the
            // transition from the initial pseudo-state into the actual
            // initial state
            // Initialize all sub-state machines
            //InitAmmoLoadHSM();
            // now put the machine into the actual initial state
            nextState = TankTurn;
            makeTransition = TRUE;
            ThisEvent.EventType = ES_NO_EVENT;
            ;
        }
        break;

    case TankTurn:
        switch (ThisEvent.EventType) {  
            case ES_ENTRY:
                tankTurnRight();
                break;
            case TW_TRIGGERED:
                // This value assumes we are using only middle and back track wires, meaning both bits are set high
                if (ThisEvent.EventParam == 0x3)
                {
                    nextState = Backward;
                    makeTransition = TRUE;
                }
                break;
            case ES_NO_EVENT:
            default:
                break;
        }
        break;
        
    case Backward:
        switch (ThisEvent.EventType) {
            case ES_ENTRY:
                moveBackward();
                break;
            case TW_TRIGGERED:
                // This value assumes we are using only middle and back track wires, meaning both bits are set high
                if (ThisEvent.EventParam != 0x3)
                {
                    nextState = TankTurn;
                    makeTransition = TRUE;
                }
                break;
            case ES_NO_EVENT:
            default:
                break;
        }
        break;
        
    default: // all unhandled states fall into here
        break;
    } // end switch on Current State

    if (makeTransition == TRUE) { // making a state transition, send EXIT and ENTRY
        // recursively call the current state with an exit event
        RunAmmoLoadSubHSM(EXIT_EVENT); // <- rename to your own Run function
        CurrentState = nextState;
        RunAmmoLoadSubHSM(ENTRY_EVENT); // <- rename to your own Run function
    }

    ES_Tail(); // trace call stack end
    return ThisEvent;
}