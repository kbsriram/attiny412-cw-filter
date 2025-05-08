#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "button.h"


int main(void) {
  button_state_t button_state;
  button_init(&button_state);

  for (int i = 0; i < 1000; i++) {
    if (button_update(&button_state, false)) {
      printf("Unexpected - 1\n");
      return 1;
    }
  }

  // One click
  if (button_update(&button_state, true)) {
    printf("Unexpected - 2\n");
  }
  // Wait for 63 more ticks
  for (int i = 0; i < 64; i++) {
    if (button_update(&button_state, false)) {
      printf("tick: %d, unexpected\n", i);
      return 1;
    }
  }

  // This one should be the debounced tick.
  if (!button_update(&button_state, false)) {
      printf("state: %0x unexpected\n", button_state);
      return 1;
  }
  // We should also be back in the idle state.
  if (button_state != 0) {
      printf("state: %0x not in expected IDLE\n", button_state);
      return 1;
  }

  // pretend we had a few bounces.
  for (int i = 0; i < 100; i++) {
    // 5 ticks pressed, 5 ticks not pressed.
    for (int j = 0; j < 5; j++) {
      if (button_update(&button_state, true)) {
        printf("Unexpected - 3\n");
        return 1;
      }
    }
    for (int j = 0; j < 5; j++) {
      if (button_update(&button_state, false)) {
        printf("Unexpected - 4\n");
        return 1;
      }
    }
  }

  // We should continue to debounce for 64 - 5 ticks.
  for (int i = 0; i < (64 - 5); i++) {
    if (button_update(&button_state, false)) {
        printf("Unexpected - 5\n");
        return 1;
    }      
  }
  // The last tick should now be a click.
  if (!button_update(&button_state, false)) {
    printf("Unexpected - 5\n");
    return 1;
  }

  printf("all checks passed.\n");
}
