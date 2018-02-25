
#include "Emitter.h"
#include "IRProtocols.h"

#define START_IR_TIMER() (TCCR1B =  _BV(CS11)) // 16MHz / 8 = 0.5us ticks
#define STOP_IR_TIMER()  (TCCR1B = 0)

const IR_Protocol_t* IR_CurProtocol = &IRProt_3DVision;
SyncMode_t IR_SyncMode = SYNCMODE_NONE;

static volatile bool emitterActive = false;
static bool emitterActive_last = false;
static volatile bool synced = false;
static volatile uint32_t lastFrame = 0;

static uint8_t swapEyes = 0;
static volatile uint8_t curEye = 0;
static uint8_t nextEye = 0;

static uint8_t lastPulse;
static uint8_t curPulse;
static uint8_t curToken;

static void SendToken(uint8_t token);

void IR_Init(void)
{
	/* GPIO */
	bitSet(DDR_LED_EYE,    LED_EYE);
	bitSet(DDR_LED_IR,     LED_IR);
	bitSet(DDRB, 5);
	bitSet(PORTD, 5); // Force sync/output

	/* TIMER1 - IR token and pulse timing*/
	TCCR1B = 0; // Timer stopped, normal mode
	TCCR1A = 0;
	TIMSK1 = 0; // All interrupts disabled
	TIFR1 = 0xFF; // Clear pending interrupt flags if any

	IR_SetSyncMode(SYNCMODE_COMBINED);
}

void IR_Update(uint32_t curTime)
{
	if (IR_SyncMode == SYNCMODE_FREERUN)
	{
		// Send without any sync source - good for testing glasses
		uint16_t delta = curTime-lastFrame;
		if (delta >= 9) // 111.(1)Hz
		{
			IR_SetEye(!curEye);
			IR_StartFrame();
		}
	}

	/* Update activity led */
	if (emitterActive)
	{
		if (!emitterActive_last) // just started
		{
			bitSet(PORT_LED_ACTIVE, LED_ACTIVE);
			emitterActive_last = true;
		}
		else
		{
			if ((curTime-lastFrame) >= 200)
			{
				// Sync timeout
				bitClear(PORT_LED_ACTIVE, LED_ACTIVE);
				bitSet(PORT_LED_EYE, LED_EYE); // Active low
				emitterActive = false;
				emitterActive_last = false;
			}
		}
	}
}

void IR_SetSyncMode(SyncMode_t mode)
{
	if (mode & SYNCMODE_EXTERNAL)
	{
		EICRA |= (0 << ISC11) | (1 << ISC10); // any edge
		bitSet(EIMSK, INT1); // enable external interrupt
	}
	else
	{
		EICRA &= ~((0 << ISC11) | (1 << ISC10)); // any edge
		bitClear(EIMSK, INT1);
	}
	synced = false;
	emitterActive = false;
	IR_SyncMode = mode;
}

void IR_SwapEyes(uint8_t swap)
{
	swapEyes = swap != 0;
}

void IR_SetEye(uint8_t eye)
{
	nextEye = eye ^ swapEyes;
	synced = true;
}
void IR_StartFrame(void)
{
	emitterActive = true;
	curEye = nextEye;
	lastFrame = millis();
	synced = false;
	SendToken(curEye * 2);
}
//void IR_EndFrame(void) {}


static void SendToken(uint8_t token)
{
	if (IR_CurProtocol->sizes[token] == 0) // Check if token exists
		return;
	curToken = token;

	curPulse = IR_CurProtocol->indices[token]; // Get timing array start index
	lastPulse = curPulse + IR_CurProtocol->sizes[token];

	bitClear(PORT_LED_IR, LED_IR);
	TCNT1 = 0;
	OCR1A = FRAME_PAN; // Token pan/delay
	//OCR1B = 0x00FF;
	bitSet(TIMSK1, OCIE1A); // Enable rising edge interrupt
	//TIFR1 = 0xFF; // Clear pending interrupts if any
	START_IR_TIMER();

	// Light up only between frames - invisible(ideally) with glasses
	bitSet(PORT_LED_EYE, LED_EYE); // Active low
}

ISR(TIMER1_COMPA_vect) // IR pulse rising edge
{
	bitSet(PORT_LED_IR, LED_IR);
	
	OCR1B = OCR1A + (IR_CurProtocol->timings[curPulse++] * 2); // Pulse duration
	bitSet(TIMSK1, OCIE1B); // Enable falling edge interrupt
	bitClear(TIMSK1, OCIE1A); // Disable this interrupt
}
ISR(TIMER1_COMPB_vect) // IR pulse falling edge
{
	bitClear(PORT_LED_IR, LED_IR);

	if (curPulse == lastPulse) // Token finished
	{
		uint8_t nextsize;
		bool hasnext = (curToken & 1) == 0; // end of shutter opening token
		if (hasnext)
		{
			nextsize = IR_CurProtocol->sizes[curToken+1];
			hasnext = nextsize > 0; // closing token exists
		}
		if (hasnext)
		{
			OCR1A = OCR1B + FRAME_DURATION;
			bitSet(TIMSK1, OCIE1A); // Enable rising edge interrupt
			curToken++;
			curPulse = IR_CurProtocol->indices[curToken]; // Get timing array start index
			lastPulse = curPulse + nextsize;
			//bitSet(PORTB, 5); // Frame start debug
		}
		else
		{
			//bitClear(PORTB, 5); // Frame end debug
			STOP_IR_TIMER();
			bitClear(PORT_LED_EYE, LED_EYE); // Active low
		}
	}
	else
	{
		OCR1A = OCR1B + (IR_CurProtocol->timings[curPulse++] * 2);  // Time until next pulse
		bitSet(TIMSK1, OCIE1A); // Enable rising edge interrupt
	}
	bitClear(TIMSK1, OCIE1B); // Disable this interrupt
}

// Frame sync edge
ISR (INT1_vect)
{
	if (IR_SyncMode & SYNCMODE_EXTERNAL)
	{
		if ((IR_SyncMode == SYNCMODE_EXTERNAL) || ((PIND & _BV(4)) == 0))
		{
			// VESA 3D sync: high = left eye, low = right eye
			IR_SetEye((PIN_SYNCIN & _BV(SYNCIN)) != 0);
		}
		
		if (synced) // When using USB sync
			IR_StartFrame();
	}
}
