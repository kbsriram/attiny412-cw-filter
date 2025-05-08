#pragma once
#include <stdint.h>

#define DIR_NONE 0x00
#define DIR_CW 0x10
#define DIR_CCW 0x20

typedef uint8_t rotary_state_t;

void rotary_init(rotary_state_t* state);
uint8_t rotary_update(rotary_state_t* state, uint8_t input);
