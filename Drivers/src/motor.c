/*
 * File:   motor.c
 * Author: jdgrant
 *
 * Motor file for helper functions which will be used to perform basic movement
 * and configure motor speed.
 * 
 */

#include "BOARD.h"
#include "IO_Ports.h"
#include "LED.h"
#include "pwm.h"
#include "AD.h"
#include "motor.h"
#include <stdio.h>

//#define DEBUG
#define PWM_LEFT_MOTOR  PWM_PORTY10
#define PWM_RIGHT_MOTOR PWM_PORTY12
#define DIR_LEFT_MOTOR  PIN11 
#define DIR_RIGHT_MOTOR PIN12

void motorInit()
{
    IO_PortsSetPortOutputs(PORTZ, DIR_LEFT_MOTOR|DIR_RIGHT_MOTOR);
    PWM_AddPins(PWM_LEFT_MOTOR|PWM_RIGHT_MOTOR);
}

void tankTurnRight()
{
	motorForward(1);
	motorForward(0);
}
void tankTurnLeft()
{
	motorBackward(0);
	motorBackward(1);
}

void moveBackward()
{
	motorForward(0);
	motorBackward(1);
}

void moveForward()
{
	motorForward(1);
	motorBackward(0);
}

void stopMoving()
{
	setMoveSpeed(0);
}

/*
* lr = 1 makes right wheel go forward
* lr = 0 makes left wheel go forward
*/
void motorForward(int lr)
{
	// sets IN0 = 0, IN1 = 1
	if (lr) 
	{
        IO_PortsWritePort(PORTZ,IO_PortsReadPort(PORTZ) | DIR_RIGHT_MOTOR); // Set Dir right motor = high (forward)
	}
	else
	{   
        IO_PortsWritePort(PORTZ,IO_PortsReadPort(PORTZ) | DIR_LEFT_MOTOR); // Set Dir left motor = high (forward)
	}
	return;
}

/*
* lr = 1 makes right wheel go backward
* lr = 0 makes left wheel go backward
*/
void motorBackward(int lr)
{
	// sets IN0 = 0, IN1 = 1
	if (lr) 
	{
		IO_PortsWritePort(PORTZ,IO_PortsReadPort(PORTZ) & ~DIR_RIGHT_MOTOR); // Set Dir right motor = low (backward)
	}
	else
	{
		IO_PortsWritePort(PORTZ,IO_PortsReadPort(PORTZ) & ~DIR_LEFT_MOTOR); // Set Dir left motor = low (backward)
	}
	return;
}

/*
 * Right now speed should be duty cycle 0-100
 * This should be updated to take input of speed in cm/s
 * and calculate pwm required to supply enough voltage,
 * that or we use feedback from our quad encoders.
 */
void setMoveSpeed(int speed)
{
    if(speed > 100)
    {
        speed = 100;
    }
    else if(speed < 0)
    {
        speed = 0;
    }
    PWM_SetDutyCycle(PWM_LEFT_MOTOR,speed*10);
    PWM_SetDutyCycle(PWM_RIGHT_MOTOR,speed*10);
    
    return;
}

