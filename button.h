#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef uint8_t button_state_t;

void button_init(button_state_t* state);
// Returns true on a (debounced) button click.
bool button_update(button_state_t* state, bool is_pressed);
