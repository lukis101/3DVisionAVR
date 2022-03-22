
#ifndef _IRPROTOCOLS_H_
#define _IRPROTOCOLS_H_

// Definitions for IR protocols, indexed in this order:
// [0]: Open right eye [1]: Close right eye
// [2]: Open left eye  [3]: Close left eye

typedef struct
{
	uint8_t sizes[4];
	uint8_t indices[4];
	uint16_t timings[];
} IR_Protocol_t;

const IR_Protocol_t IRProt_Samsung07 = {
	.sizes   = { 5,0, 0,0 },
	.indices = { 0,0, 0,0 },
	.timings = { 14,12,14,12,14 }
};
const IR_Protocol_t IRProt_Xpand = {
	.sizes   = { 5,0, 3,0 },
	.indices = { 0,0, 5,0 },
	.timings = { 18,20,18,20,18,  18,60,18 }
};

// For 3D Vision only, indexed in this order:
// [0]: Close left eye  [1] Open right eye
// [2]: Close right eye [3]: Open left eye
const IR_Protocol_t IRProt_3DVision = {
	.sizes   = { 3,3, 3,1 },
	.indices = { 0,3, 6,9 },
	.timings = { 23,21,24,  23,46,31,
	             23,78,40,  43 }
};
const IR_Protocol_t IRProt_Sharp = {
	.sizes   = { 15,0, 15,0 },
	.indices = { 0,0, 15,0 },
	.timings = { 20,20,20,20,20,80,20,140,20,20,20,80,20,20,20,
	             20,20,20,20,20,60,20, 60,20,20,20,80,20,20,20 }
};
const IR_Protocol_t IRProt_Sony = {
	.sizes   = { 9,9, 9,9 },
	.indices = { 27, 0,9, 18 },
	.timings = { 20,20,20,20,20,300,20,20,20,  20,20,20,20,20,220,20,20,20,
	             20,20,20,20,20,140,20,20,20,  20,20,20,20,20,380,20,20,20 }
};
const IR_Protocol_t IRProt_Panasonic = {
	.sizes   = { 7,7, 7,7 },
	.indices = { 0,7, 14,21 },
	.timings = { 20,20,20,100,20,20,20,  20,60,20,20,20,60,20,
	             20,60,20, 60,20,20,20,  20,20,20,60,20,60,20 }
};

#endif /* _IRPROTOCOLS_H_ */
