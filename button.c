#include <stdint.h>
#include <stdbool.h>
#include "button.h"


// buttons go through this sequence for debouncing.
// IDLE -> [not-pressed] -> IDLE
//      -> [is-pressed] -> PRESSED
// PRESSED -> [is-pressed] -> PRESSED
//         -> [not-pressed] -> DEBOUNCE, counter=0
// DEBOUNCE -> [is-pressed] -> PRESSED, counter=0
//          -> [not-pressed]
//                if (counter < MAX) counter++; -> DEBOUNCE
//                else counter = 0; emit click ->  IDLE

// button state stored in first 2 bits.
#define STATE(v) ((v) & 0xc0)
#define IDLE (0 << 6)
#define PRESSED (1 << 6)
#define DEBOUNCE (2 << 6)

// counter stored in last 6 bits.
#define COUNTER(v) ((v) & 0x3f)
#define MAX_COUNTER 0x3f

void button_init(button_state_t* state) {
  *state = 0;
}

bool button_update(button_state_t* state, bool is_pressed) {
  switch (STATE(*state)) {
    default: // fall through
    case IDLE:
      if (is_pressed) {
        *state = PRESSED;
      }
      break;

    case PRESSED:
      if (!is_pressed) {
        // move to debounce, counter = 0.
        *state = DEBOUNCE;
      }
      break;

    case DEBOUNCE:
      if (is_pressed) {
        // back to pressed, counter = 0
        *state = PRESSED;
      } else {
        // Still not pressed while debouncing.
        if (COUNTER(*state) < MAX_COUNTER) {
          (*state)++;
        } else {
          // counter in debounce state maxed.
          // reset to idle, and return click.
          *state = IDLE;
          return true;
        }
      }
      break;
  }
  return false;
}
