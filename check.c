#include <stdio.h>
#include <math.h>
#include "dsp.h"

filter_state_t filter_state = {0, 0, 0, 0};

coefficient_t coeff = {0, 0, 0};

#define FREQ 600
#define AMPLITUDE 20
#define NCYCLES 50

void rows(int16_t v) {
  static uint16_t last_min = 0;
  if ((v < 0) && (-v > last_min)) {
    last_min = -v;
  }

  uint16_t n = (uint16_t) ((v + last_min) / 10);
  for (uint16_t i = 0; i < n; i++) {
    printf("*");
  }
  printf("\n");
}

int main(void) {
  uint16_t fc = 600;
  uint16_t bw = 100;
  update_iir(fc, bw, &coeff);

  printf("b0: %d\na0: %d\na1: %d\n", coeff.b0, coeff.a1, coeff.a2);

  double time_step = 1.0 / SAMPLE_RATE;
  double time = 0.0;
  int num_samples = NCYCLES * SAMPLE_RATE / fc;

  for (int i = 0; i < num_samples; i++) {
    double sine_value = AMPLITUDE * sin(2.0 * M_PI * FREQ * time);
    int16_t v = (int16_t) sine_value;
    int16_t filtered = filter(v, &coeff, &filter_state);
    rows(filtered);
    time += time_step;
  }

  printf("-----------------------------------------\n");

  num_samples = 10 * SAMPLE_RATE / fc;
  for (int i = 0; i < num_samples; i++) {
    int16_t filtered = filter(AMPLITUDE/2, &coeff, &filter_state);
    rows(filtered);
    time += time_step;
  }
  return 0;
}
