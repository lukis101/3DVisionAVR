
#ifndef _IREMITTER_H_
#define _IREMITTER_H_

// Duration between close to open eye frames - in half-microseconds (@16MHz)
#define FRAME_DURATION  (2*1000)
// Time between sync trigger and start of IR token (same units)
#define FRAME_PAN       (2*3000) // for 120Hz display (use 2*3400 for 100Hz)

// INT1, pin 2 on "Arduino Pro Micro"
#define SYNCIN          1
#define PIN_SYNCIN      PIND

#define LED_IR          0
#define DDR_LED_IR      DDRD
#define PORT_LED_IR     PORTD
#define LED_EYE         0
#define DDR_LED_EYE     DDRB
#define PORT_LED_EYE    PORTB

#define EYE_RIGHT 0
#define EYE_LEFT  1

typedef enum {
	SYNCMODE_NONE     = 0,
	SYNCMODE_DRIVER   = 1, // }
	SYNCMODE_EXTERNAL = 2, // } Can be used as masks
	SYNCMODE_COMBINED = 3,
	SYNCMODE_FREERUN  = 4
} SyncMode_t;

SyncMode_t IR_SyncMode;

void IR_Init(void);
void IR_Update(uint32_t curTime);
void IR_SetSyncMode(SyncMode_t mode);
void IR_SwapEyes(uint8_t swap);

void IR_SetEye(uint8_t eye);
void IR_StartFrame(void);
//void IR_EndFrame(void);

#endif /* _IREMITTER_H_ */
