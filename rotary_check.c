#include <stdio.h>
#include "rotary.h"

#define DIRECTION(v) ((v) & 0x30)

int main(void) {
  rotary_state_t rotary_state;
  rotary_init(&rotary_state);

  // mimic clockwise, then ccw turn with bounce
  uint8_t inputs[] = {
    // clockwise
    0x3, 0x3, 0x3, 0x1, 0x0, 0x1, 0x3, 0x1, 0x0, 0x2, 0x0, 0x2, 0x3,
    // ccw
    0x3, 0x3, 0x3, 0x2, 0x0, 0x2, 0x3, 0x2, 0x0, 0x1, 0x0, 0x1, 0x3
  };

  for (uint8_t i = 0; i < sizeof(inputs); i++) {
    uint8_t inp = inputs[i];
    rotary_state_t before = rotary_state;
    rotary_update(&rotary_state, inp);
    printf("%hhu: %hhu -> %hhu: %hhu\n", inp, before, rotary_state, DIRECTION(rotary_state));
  }           
}
