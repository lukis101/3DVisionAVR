/*
             LUFA Library
     Copyright (C) Dean Camera, 2018.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2018  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaims all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

/** \file
 *
 *  Header file for Emitter.c.
 */

#ifndef _EMITTER_H_
#define _EMITTER_H_

/* Includes: */
	#include <avr/io.h>
	#include <avr/wdt.h>
	#include <avr/power.h>
	#include <avr/interrupt.h>
	#include <avr/sfr_defs.h>

	#include "Descriptors.h"
	#include "LUFA/Drivers/USB/USB.h"
	#include "LUFA/Platform/Platform.h"
	#include "IREmitter.h"

/* Pin defines */
	#define LED_STBY        6
	#define DDR_LED_STBY    DDRB
	#define PORT_LED_STBY   PORTB
	#define LED_ACTIVE      2
	#define DDR_LED_ACTIVE  DDRB
	#define PORT_LED_ACTIVE PORTB

	#define FORCEIN         4
	#define PIN_FORCEIN     PINB
	#define PORT_FORCEIN    PORTB

	volatile uint32_t millisPassed;

/* Util macros */
	#define bitSet(addr,bit) (addr |= (1<<bit))
	#define bitClear(addr,bit) (addr &= ~(1<<bit))
	#define millis() (millisPassed)

/* Function Prototypes: */
	void SetupUSBHardware(void);

	void EVENT_USB_Device_Connect(void);
	void EVENT_USB_Device_Disconnect(void);
	void EVENT_USB_Device_ConfigurationChanged(void);
	void EVENT_USB_Device_ControlRequest(void);

	void returnData(void);
	
	void UART_Write(uint8_t* data, uint8_t amount);

#endif /* _EMITTER_H_ */
