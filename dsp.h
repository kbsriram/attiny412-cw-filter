#pragma once

#include <math.h>
#include <stdint.h>

#define SAMPLE_RATE 16000
#define FIXED_POINT_SHIFT 14

typedef struct {
  int16_t x_n1;
  int16_t x_n2;
  int16_t y_n1;
  int16_t y_n2;
} filter_state_t;

typedef struct {
  int16_t b0;
  int16_t a1;
  int16_t a2;
} coefficient_t;

inline static void update_iir(uint16_t fc, uint16_t bw, volatile coefficient_t* coeff) {
  double fc_d = (double) fc;
  double w0 = 2.0 * M_PI * fc_d / SAMPLE_RATE;
  double q_factor = fc_d / bw;
  double sin_w0 = sin(w0);
  double cos_w0 = cos(w0);
  double alpha = sin_w0 / (2.0 * q_factor);

  // BPF with constant skirt gain.
  // See: https://www.w3.org/TR/audio-eq-cookbook/
  // Also, return normalized to a0 (so all formulae
  // divided by a0 = 1 + alpha
  double a0 = 1 + alpha;
  double b0 = alpha * q_factor / a0;
  double a1 = -2.0 * cos_w0 / a0;
  double a2 = (1.0 - alpha) / a0;

  coeff->b0 = (int16_t)(b0 * (1 << FIXED_POINT_SHIFT) + 0.5);
  coeff->a1 = (int16_t)(a1 * (1 << FIXED_POINT_SHIFT) + 0.5);
  coeff->a2 = (int16_t)(a2 * (1 << FIXED_POINT_SHIFT) + 0.5);
}

inline static int16_t filter(int16_t x_n, volatile coefficient_t* coeff, volatile filter_state_t* state) {
  // Calculation implemented here is:
  // y_n = b0*x_n - b0*x_n2 - a1*y_n1 - a2*y_n2
  int32_t accum = (int32_t) (x_n - state->x_n2);
  accum *= coeff->b0;
  accum -= ((int32_t) coeff->a1) * state->y_n1;
  accum -= ((int32_t) coeff->a2) * state->y_n2;

  int16_t result = (int16_t)(accum >> FIXED_POINT_SHIFT);
  state->x_n2 = state->x_n1;
  state->x_n1 = x_n;
  state->y_n2 = state->y_n1;
  state->y_n1 = result;
  return result;
}
