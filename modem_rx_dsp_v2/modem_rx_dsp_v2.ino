#include "IntervalTimer.h"
#include <avr/interrupt.h>
#include "variables.h"
#include "variables_codes.h"
#include "windows.h"
#include "goertzel.h"
#include "codes.h"

void setup() {
  // try and speed up the ADC a little
  analogReadAveraging(0); // no averaging, the default is 4x I think

  Serial.begin(9600);
  initBlackman();
  initGoertzel();
  init_filter_coeff();

  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

  pinMode(0, OUTPUT);
  digitalWrite(0, LOW);

  pinMode(1, OUTPUT);
  digitalWrite(1, LOW);
    
  // you need to jumper pins 14 and 15 together; writing 15 to low will thus trigger an interrupt on pin 14; this seems like a clunky workaround, but interrupts need to be triggered by hardware
  pinMode(14, INPUT);
  pinMode(15, OUTPUT);
  digitalWrite(15, HIGH);

  samples_to_read = &sample_buffer[buffer_index]; // the first N samples will be stored in the 0th row of the sample buffer
  attachInterrupt(14, runGoertzel, LOW); // writing pin 15 low will trigger an interrupt on pin 14 that runs this runGoertzel routine
  sample_timer.begin(readSignal, 1e6/FS); 
  
  code_timer.begin(matched_filter, 1e6/(n*fs)); // runs the matched_filter subroutine every 1/fs seconds
}

void loop() {
}
