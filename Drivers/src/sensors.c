/*
 * File:   sensors.c
 * Author: jdgrant
 *
 * This file will contain helper functions for driving the sensors and interfacing with them.
 * There is also an init function for initializing the various sensors.
 * 
 */

#include "BOARD.h"
#include "IO_Ports.h"
#include "LED.h"
#include "pwm.h"
#include "AD.h"
#include "motor.h"
#include <stdio.h>

#define TRACK_WIRE_OUTPUT PIN6

uint8_t readTrackWire()
{
    return (IO_PortsReadPort(PORTY) & TRACK_WIRE_OUTPUT) >> 6;
}

void sensorsInit()
{
    IO_PortsSetPortOutputs(PORTY,TRACK_WIRE_OUTPUT);
}