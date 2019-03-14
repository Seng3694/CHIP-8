#ifndef C8_REGISTERS_H
#define C8_REGISTERS_H

#include "config.h"

typedef struct c8_registers {
	union {
		struct {
			uint8 v0;
			uint8 v1;
			uint8 v2;
			uint8 v3;
			uint8 v4;
			uint8 v5;
			uint8 v6;
			uint8 v7;
			uint8 v8;
			uint8 v9;
			uint8 va;
			uint8 vb;
			uint8 vc;
			uint8 vd;
			uint8 ve;
			uint8 vf;
		};

		uint8 v[16];
	};
} c8_registers_t;

#endif
