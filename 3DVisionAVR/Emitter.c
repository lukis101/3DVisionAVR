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

#include "Emitter.h"

/* Driver-specific vars */
static uint8_t command = 0;
static uint8_t offset = 0;
static uint8_t amount = 0;
static uint8_t dataBuff[EMITTER_EPSIZE];
static uint8_t ramx22[2];
static uint8_t ramx18[3];

/* Time keeping */
volatile uint32_t millisPassed = 0;

/* Serial out */
volatile uint8_t serBuff[256];
volatile uint8_t serBuffTail = 0;
volatile uint8_t serBuffHead = 0;
volatile bool serTxActive = false;

/** Main program entry point. This routine configures the hardware required by the application, then
 *  enters a loop to run the application tasks in sequence.
 */
int main(void)
{
	memset(dataBuff, 0, sizeof(dataBuff));

	SetupUSBHardware();
	IR_Init();
	GlobalInterruptEnable();

	for (;;)
	{
		USB_USBTask();

		uint32_t curtime = millisPassed;
		IR_Update(curtime);		
		
		// TODO handle unconfigured state properly: LEDs, reduced power mode etc.
		if (USB_DeviceState != DEVICE_STATE_Configured)
			continue;

		/* Control / setup */
		Endpoint_SelectEndpoint(EMITTER_EP_CONTROL_OUT); // Commands to emitter
		if (Endpoint_IsOUTReceived() && Endpoint_IsConfigured() && Endpoint_IsReadWriteAllowed())
		{
			Endpoint_Read_Stream_LE(dataBuff, Endpoint_BytesInEndpoint(), NULL);
			Endpoint_ClearOUT();
			
			command = dataBuff[0];
			offset = dataBuff[1];
			amount = dataBuff[2];
			if (command & 0x01) // Write
			{
				if (offset == 0x22)
					memcpy(ramx22, dataBuff+4, amount);
				else if (offset == 0x18)
					memcpy(ramx18, dataBuff+4, amount);
			}
			else if (command & 0x02) // Read
			{				
				Endpoint_SelectEndpoint(EMITTER_EP_CONTROL_IN); // To emitter
				Endpoint_WaitUntilReady();
				returnData();
			}
			if (command & 0x40) // Clear
			{
				if (offset == 0x22)
					memset(ramx22, 0, amount);
				else if (offset == 0x18)
					memset(ramx18, 0, amount);
			}
		}
		//Endpoint_SelectEndpoint(EMITTER_CONTROLEP_IN); // Back to PC
		//if (Endpoint_IsConfigured() && Endpoint_IsINReady() && Endpoint_IsReadWriteAllowed())
		//{
		//	bitSet(PORTB, 6);
		//	returnData();
		//	Endpoint_ClearIN();
		//}
		
		Endpoint_SelectEndpoint(EMITTER_EP_BUTTON_IN);
		if (Endpoint_IsINReady())
		{
			//Endpoint_Write_Stream_LE(dataBuff, 8, NULL);
			//Endpoint_ClearIN();
		}
		
		/* Eye swap controls */
		Endpoint_SelectEndpoint(EMITTER_EP_SWAP_OUT);
		if (Endpoint_IsOUTReceived())
		{			
			Endpoint_Read_Stream_LE(dataBuff, 8, NULL);
			Endpoint_ClearOUT();

			if (IR_SyncMode & SYNCMODE_DRIVER)
			{
				// Eye sync packet
				if ((dataBuff[0] == 0xAA) && ((dataBuff[1] & 0xFE) == 0xFE))
				{
					// 0xFE = left, 0xFF = right
					uint8_t eye = dataBuff[1] & 1; // Flipped, too late for current frame
					IR_SetEye(eye);

					if (IR_SyncMode == SYNCMODE_DRIVER)
					{
						IR_StartFrame();
					}
				}
			}
		}
	}
}

/** Configures the board hardware and chip peripherals */
void SetupUSBHardware(void)
{
#if (ARCH == ARCH_AVR8)
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);
#elif (ARCH == ARCH_XMEGA)
	/* Start the PLL to multiply the 2MHz RC oscillator to 32MHz and switch the CPU core to run from it */
	XMEGACLK_StartPLL(CLOCK_SRC_INT_RC2MHZ, 2000000, F_CPU);
	XMEGACLK_SetCPUClockSource(CLOCK_SRC_PLL);

	/* Start the 32MHz internal RC oscillator and start the DFLL to increase it to 48MHz using the USB SOF as a reference */
	XMEGACLK_StartInternalOscillator(CLOCK_SRC_INT_RC32MHZ);
	XMEGACLK_StartDFLL(CLOCK_SRC_INT_RC32MHZ, DFLL_REF_INT_USBSOF, F_USB);

	PMIC.CTRL = PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_HILVLEN_bm;
#endif

	/* GPIO */
	bitSet(DDR_LED_STBY,   LED_STBY);
	bitSet(DDR_LED_ACTIVE, LED_ACTIVE);
	//bitSet(PORT_FORCEIN, FORCEIN); // Pullup, pull low to force freerun mode

	/* TIMER0 - 1khz time tracking ticks*/
	OCR0A = 250;
	TCCR0A = _BV(WGM01) | _BV(WGM00); // CTC mode with OCR0A top
	TCCR0B = _BV(WGM02) | _BV(CS01) | _BV(CS00); // clk / 64
	TIMSK0 = _BV(TOIE0); // overflow interrupt enable
	
	/* UART */
	UBRR1 = ((F_CPU / 8) / 115200) - 1;
	UCSR1A = _BV(U2X1); // double speed mode
	UCSR1C = _BV(UCSZ11) | _BV(UCSZ10);
	
	UCSR1B = 0;
	//bitSet(UCSR1B, RXEN0);
	//bitSet(UCSR1B, RXCIE1);
	bitSet(UCSR1B, TXEN1);
	bitSet(UCSR1B, TXCIE1);
	//bitSet(UCSR1B, UDRIE1);
	
	/* Hardware Initialization */
	USB_Init();
}

/** Event handler for the USB_Connect event. This indicates that the device is enumerating via the status LEDs. */
void EVENT_USB_Device_Connect(void)
{
}
void EVENT_USB_Device_Disconnect(void)
{
}

/** Event handler for the USB_ConfigurationChanged event. This is fired when the host set the current configuration
 *  of the USB device after enumeration - the device endpoints are configured.
 */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;
	
	/* Setup Endpoints */
	ConfigSuccess &= Endpoint_ConfigureEndpoint(EMITTER_EP_SWAP_OUT,  EP_TYPE_BULK, EMITTER_EPSIZE, 1);
	ConfigSuccess &= Endpoint_ConfigureEndpoint(EMITTER_EP_BUTTON_IN, EP_TYPE_INTERRUPT, EMITTER_EPSIZE, 1);
	ConfigSuccess &= Endpoint_ConfigureEndpoint(EMITTER_EP_CONTROL_OUT, EP_TYPE_BULK, EMITTER_EPSIZE, 1);
	ConfigSuccess &= Endpoint_ConfigureEndpoint(EMITTER_EP_CONTROL_IN, EP_TYPE_BULK, EMITTER_EPSIZE, 1);
	if (ConfigSuccess)
		bitSet(PORT_LED_STBY, LED_STBY);
}

/** Event handler for the USB_ControlRequest event. This is used to catch and process control requests sent to
 *  the device from the USB host before passing along unhandled control requests to the library for processing
 *  internally.
 */
void EVENT_USB_Device_ControlRequest(void)
{
	//uint8_t reqDir  = USB_ControlRequest.bmRequestType & (1<<7);
	//uint8_t reqType = USB_ControlRequest.bmRequestType & (3<<5);
	//uint8_t reqRec  = USB_ControlRequest.bmRequestType & 0x1F;
	
	if (USB_ControlRequest.bRequest == 0xA0) // Special "Firmware load" request
	{
		/// USB_ControlRequest.wValue   --> Starting RAM adress
		/// USB_ControlRequest.wLength  --> Number of bytes to write
		/// 0xE600 is the CPUCS register address. Writing 1 puts device to a reset state.

		/*  Write firmware */
		if (USB_ControlRequest.bmRequestType == 0x40) // REQTYPE_VENDOR | REQDIR_HOSTTODEVICE
		{
			Endpoint_ClearSETUP();
			
			/// If processing the firmware:
			//uint16_t buffer[ 1024 ];
			//Endpoint_Read_Stream_LE(buffer, USB_ControlRequest.wLength, NULL);
			
			Endpoint_Discard_Stream(USB_ControlRequest.wLength, NULL); // Discard the firmware
			Endpoint_ClearStatusStage();			
		}
		/* Read firmware */
		else if (USB_ControlRequest.bmRequestType == 0xC0) // REQTYPE_VENDOR | REQDIR_DEVICETOHOST
		{
			Endpoint_ClearSETUP();
			Endpoint_Null_Stream(USB_ControlRequest.wLength, NULL);
			Endpoint_ClearStatusStage();
		}
	}
	else if ((USB_ControlRequest.bRequest == 0x06) && (USB_ControlRequest.bmRequestType == 0x80) && (USB_ControlRequest.wValue==0x0600))
	{
		//bitSet(PORT_LED2, LED2);
		Endpoint_ClearSETUP();
		//Endpoint_Write_8(0x01);
		Endpoint_ClearIN();
		Endpoint_ClearStatusStage();
	}
	//else
	//bitSet(PORT_LED1, LED1);
}

void returnData(void)
{
	dataBuff[0] = offset;
	dataBuff[1] = amount;
	dataBuff[2] = 0x00;
	dataBuff[3] = 0x04;
	if (offset == 0x22)
		memcpy( dataBuff+4, ramx22, amount);
	else if (offset == 0x18)
		memcpy(dataBuff+4, ramx18, amount);
	else for (uint8_t i=0; i<amount; i++)
		dataBuff[4+i] = 0x00;

	Endpoint_Write_Stream_LE(dataBuff, 4+amount, NULL);
	Endpoint_ClearIN();
}


ISR(TIMER0_OVF_vect) // 1kHz tick
{
	millisPassed++;
}

ISR(USART1_TX_vect) // transmit complete
{
	if (serBuffTail != serBuffHead)
		UDR1 = serBuff[serBuffTail++];
	else
		serTxActive = false;
}

void UART_Write(uint8_t* data, uint8_t amount)
{
	uint8_t i = 0;
	uint8_t head = serBuffHead;
	while (i < amount)
	{
		uint8_t nexthead = head + 1;
		if (nexthead == serBuffTail)
			break;
		serBuff[head] = data[i++];
		serBuffHead = nexthead;
		head++;
	}
	if (!serTxActive && (serBuffTail != serBuffHead))
	{
		UDR1 = serBuff[serBuffTail++]; // start transfer
		serTxActive = true;
	}
}