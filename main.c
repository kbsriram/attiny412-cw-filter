#define F_CPU 16000000UL // 16 MHz system clock

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>
#include <avr/sleep.h>
#include <util/atomic.h>
#include <stdint.h>
#include <math.h>
#include "dsp.h"
#include "rotary.h"

// (1) Vdd
// (2) PA6 - DAC
// (3) PA7 - ROT_SWITCH
// (4) PA1 - ADC0
// (5) PA3 - ROT_A
// (6) PA0 - UPDI
// (7) PA2 - ROT_B
// (8) GND


// Timer B period for triggering ADC.
// TCB0 runs on CLK_PER
// Our desired period is CLK_PER / SAMPLE_RATE - 1
#define SAMPLE_TIMER_TICKS (F_CPU / SAMPLE_RATE - 1)

#define FC_MIN 50
#define FC_MAX 4000
static volatile uint16_t fc = 600;

static volatile uint16_t bw = 40;

static volatile filter_state_t filter_state = {0, 0, 0, 0};

static volatile coefficient_t coeff = {0, 0, 0};

static rotary_state_t rotary_state;

void setup_clock(void) {
  // Fuse set to OSC16, disabling prescalar to get
  // a clock of 16Mhz
  _PROTECTED_WRITE(CLKCTRL.MCLKCTRLB, 0);
}

void setup_rotary(void) {
  // Input with pullup on PA2, PA3, PA7
  PORTA.DIRCLR = PIN2_bm | PIN3_bm | PIN7_bm;
  PORTA.PIN2CTRL = PORT_PULLUPEN_bm;
  PORTA.PIN3CTRL = PORT_PULLUPEN_bm;
  PORTA.PIN7CTRL = PORT_PULLUPEN_bm;
  rotary_init(&rotary_state);
}

void setup_adc(void) {
  // Input on PA1/AIN1
  PORTA.DIRCLR = PIN1_bm;
  // Disable digital buffer for lower noise.
  PORTA.PIN1CTRL = PORT_ISC_INPUT_DISABLE_gc;

  // Configure ADC0

  // Accumulate 1 samples
  ADC0.CTRLB = ADC_SAMPNUM_ACC1_gc;

  // Sample for an extra 14 cycles (2 + 14 = 16 clock cycles overall)
  ADC0.SAMPCTRL = 14;

  // Use VDD as reference, set prescaler for ADC clock (CLK_ADC)
  // CLK_ADC should be between 50kHz and 1.5MHz for 10-bit
  // CLK_PER = 16 MHz. Prescaler DIV16 -> CLK_ADC = 1Mhz
  ADC0.CTRLC = ADC_REFSEL_VDDREF_gc | ADC_PRESC_DIV16_gc;

  // Select AIN1 (PA1)
  ADC0.MUXPOS = ADC_MUXPOS_AIN1_gc;

  // Enable start conversion triggered on event
  ADC0.EVCTRL = ADC_STARTEI_bm;          

  // Enable Result Ready Interrupt
  ADC0.INTCTRL = ADC_RESRDY_bm;        

  // Enable ADC, 10-bit resolution (default)
  ADC0.CTRLA = ADC_ENABLE_bm | ADC_RESSEL_10BIT_gc;
}

void setup_dac(void) {
  // DAC output (PA6): input buffer and pullup disabled
  PORTA.PIN6CTRL = PORT_ISC_INPUT_DISABLE_gc;

  // enable dac with output buffered.
  DAC0.CTRLA = DAC_ENABLE_bm | DAC_OUTEN_bm;
}

void setup_adc_timer(void) {
  // Setup TCB0 to periodically generate events at SAMPLE_RATE
  // This will trigger an ADC sample, as it is set up to listen
  // for this event.

  // Set Periodic Interrupt mode (generates event/interrupt on TOP)
  TCB0.CTRLB = TCB_CNTMODE_INT_gc;

  // Set the period (compare) value calculated earlier
  TCB0.CCMP = SAMPLE_TIMER_TICKS;

  // Enable TCB0 Event output. OVF triggers event in INT mode.
  TCB0.EVCTRL = TCB_CAPTEI_bm;

  // Enable timer clock at CLK_PER
  TCB0.CTRLA = TCB_CLKSEL_CLKDIV1_gc | TCB_ENABLE_bm;
}

void setup_events(void) {
  // Connect TCB0 Overflow (Event Generator) to ADC0 Start Conversion (Event User)
  EVSYS.SYNCCH0 = EVSYS_SYNCCH0_TCB0_gc; // TCB0 Overflow event

  // ADC0 listens on ASYNCUSER1. So route SYNC channel 0 to ASYNCUSER1
  // EVSYS.ASYNCUSER1 = EVSYS_ASYNCUSER1_SYNCCH0_gc;
  EVSYS.ASYNCUSER1 = EVSYS_SYNCUSER1_SYNCCH0_gc;
}

ISR(ADC0_RESRDY_vect) {
  // ADC conversion complete

  // Read ADC result. It's unsigned, but we're (theoretically)
  // careful to not not exceed 2^10
  int16_t adc_result = (int16_t) ADC0.RES;
  int16_t filtered = filter(adc_result - 512, &coeff, &filter_state);
  // int16_t filtered = adc_result;

  filtered >>= 2;
  if (filtered < -128) {
    filtered = -128;
  } else if (filtered > 127) {
    filtered = 127;
  }

  DAC0.DATA = (uint8_t)(filtered + 128);
}


void setup(void) {
  setup_clock();
  setup_rotary();
  setup_adc();
  setup_dac();
  setup_adc_timer();
  setup_events();
  update_iir(fc, bw, &coeff);
}

int main(void) {
  setup();

  // Enable global interrupts
  sei();

  while (1) {
    set_sleep_mode(SLEEP_MODE_IDLE);
    sleep_mode();

    // Woken up on timer interrupt
    uint8_t pins = PORTA.IN;
    // Conveniently, PA3/PA2 are next to each other and can be read
    // in one shift|mask operation.
    switch(rotary_update(&rotary_state, (pins >> 2) & 0x3)) {
      default:
        // do nothing
        break;

      case DIR_CW:
        ATOMIC_BLOCK(ATOMIC_FORCEON) {
          fc += 25;
          if (fc >= FC_MAX) {
            fc = FC_MAX;
          }
          update_iir(fc, bw, &coeff);
        }
        break;

      case DIR_CCW:
        ATOMIC_BLOCK(ATOMIC_FORCEON) {
          fc -= 25;
          if (fc <= FC_MIN) {
            fc = FC_MIN;
          }
          update_iir(fc, bw, &coeff);
        }
        break;
    }
  }
}
