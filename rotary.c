#include <stdint.h>
#include "rotary.h"

// State machine to track rotary encoder positions.
// Note that the inputs are pulled high, so the detent
// position of the encoder produces the input 11
//
// A clockwise turn produces the sequence of inputs
// 11 -> 01 -> 00 -> 10 -> 11
// A counterclockwise turn produces
// 11 -> 10 -> 00 -> 01 -> 11
//
// There are 7 states to capture this - S represents
// the detented state, while CW1 CW2 CW3, and CCW1 CCW2 CCW3
// represent the intermediate states of each clockwise or
// counterclockwise sequence.

#define S 0
// Illegal states are mapped back to S, but tagged
// differently to document the state transitions.
#define ILLEGAL 0
#define CW1 1
#define CW2 2
#define CW3 3
#define CCW1 4
#define CCW2 5
#define CCW3 6

// Lookup table for the transition.
// The state is encoded in the last 4 bits of rotary_state_t
// The direction (DIR_NONE, DIR_CW, DIR_CCW) is encoded in
// the first 4 bits of rotary_state_t
// The table is looked up using the last 4 bits of the state
// and the input, and returns the fully encoded state.
// E.g. if the state was CW2 and the input is 10, the new
// state is available at
// TRANSITION[CW2][input] 
// Illegal transitions are just mapped to the start state, but tagged
// as ILLEGAL to document the transition.

static const uint8_t TRANSITIONS[7][4] = {
  // Inputs
  // 00,  01, 10, 11
  // State S
  {ILLEGAL, CW1, CCW1, S},
  // CW1
  {CW2, CW1, ILLEGAL, S},
  // CW2
  {CW2, CW1, CW3, ILLEGAL},
  // CW3
  {CW2, ILLEGAL, CW3, S | DIR_CW},
  // CCW1
  {CCW2, ILLEGAL, CCW2, S},
  // CCW2
  {CCW2, CCW3, CCW1, ILLEGAL},
  // CCW3
  {CCW2, CCW3, ILLEGAL, S | DIR_CCW},
};

void rotary_init(rotary_state_t* state) {
  *state = S;
}

uint8_t rotary_update(rotary_state_t* state, uint8_t input) {
  // Grab the last 4 bits as the current state
  *state = TRANSITIONS[(*state) & 0xf][input];
  return (*state) & 0x30;
}
