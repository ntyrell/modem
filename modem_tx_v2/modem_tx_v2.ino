#include "IntervalTimer.h"
#include <avr/interrupt.h>
#include <FrequencyTimer2.h>

const int N = 31; // size of data buffer as well as number of FIR taps (length of gold code = 2^n-1)
const int M = 33; // total number of gold codes (2^n+1)
const int fs = 200; // sample rate [Hz]

const int sig_out = 13; // pin that the (digital) signal is written out on
const int tone_out = 14; // pin that the modulated signal is output on
const unsigned long Th = 43; // period of high frequency [us] has to be integer!! :/
const unsigned long Tl = 53; // period of low frequency [us]

int (*signal_coeff)[N]; // this is a pointer to a length N 1D array that will point towards the code that we want to send
// this big array stores M pseudo-random noise sequences
int signal_coeff_MN[M][N] = {{1,1,1,1,1,0,0,1,1,0,1,0,0,1,0,0,0,0,1,0,1,0,1,1,1,0,1,1,0,0,0},
                             {1,1,1,1,1,0,0,1,0,0,1,1,0,0,0,0,1,0,1,1,0,1,0,1,0,0,0,1,1,1,0},
                             {0,0,0,0,0,0,0,0,1,0,0,1,0,1,0,0,1,0,0,1,1,1,1,0,1,0,1,0,1,1,0},
                             {0,0,0,0,1,0,1,1,1,1,0,0,0,1,0,1,0,1,0,0,0,0,0,1,1,0,0,0,1,0,1},
                             {0,0,0,1,1,1,0,1,0,1,1,0,0,1,1,0,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1},
                             {0,0,1,1,0,0,0,0,0,0,1,0,0,0,0,1,1,0,0,0,0,0,1,1,0,1,0,1,1,1,1},
                             {0,1,1,0,1,0,1,0,1,0,1,0,1,1,1,1,0,1,1,1,1,0,1,0,0,1,1,0,1,1,1},
                             {1,1,0,1,1,1,1,1,1,0,1,1,0,0,1,0,1,0,0,0,1,0,0,0,0,0,0,0,1,1,1},
                             {1,0,1,1,0,1,0,1,1,0,0,0,1,0,0,1,0,1,1,0,1,1,0,0,1,1,0,0,1,1,0},
                             {0,1,1,0,0,0,0,1,1,1,1,1,1,1,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,0},
                             {1,1,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,1,0,1,1,0,0,1,0,0,0,0,1},
                             {1,0,0,1,1,0,0,0,1,1,0,0,1,1,1,0,0,0,0,1,0,0,0,0,0,1,0,1,0,1,0},
                             {0,0,1,1,1,0,1,1,0,1,1,1,0,0,0,0,0,1,0,1,1,1,0,0,0,1,1,1,1,0,0},
                             {0,1,1,1,1,1,0,0,0,0,0,0,1,1,0,0,1,1,0,0,0,1,0,0,0,0,1,0,0,0,1},
                             {1,1,1,1,0,0,1,0,1,1,1,1,0,1,0,1,1,1,1,1,0,1,0,0,1,0,0,1,0,1,1},
                             {1,1,1,0,1,1,1,1,0,0,0,0,0,1,1,1,1,0,0,1,0,1,0,1,1,1,1,1,1,1,0},
                             {1,1,0,1,0,1,0,0,1,1,1,0,0,0,1,1,0,1,0,1,0,1,1,1,0,0,1,0,1,0,0},
                             {1,0,1,0,0,0,1,1,0,0,1,0,1,0,1,0,1,1,0,1,0,0,1,0,1,0,0,0,0,0,0},
                             {0,1,0,0,1,1,0,0,1,0,1,1,1,0,0,1,1,1,0,1,1,0,0,1,1,1,0,1,0,0,0},
                             {1,0,0,1,0,0,1,1,1,0,0,1,1,1,1,1,1,1,0,0,1,1,1,1,0,1,1,1,0,0,1},
                             {0,0,1,0,1,1,0,1,1,1,0,1,0,0,1,1,1,1,1,0,0,0,1,0,0,0,1,1,0,1,0},
                             {0,1,0,1,0,0,0,1,0,1,0,0,1,0,1,1,1,0,1,1,1,0,0,0,1,0,1,1,1,0,1},
                             {1,0,1,0,1,0,0,0,0,1,1,1,1,0,1,1,0,0,0,0,1,1,0,1,1,0,1,0,0,1,1},
                             {0,1,0,1,1,0,1,0,0,0,0,1,1,0,1,0,0,1,1,0,0,1,1,1,1,0,0,1,1,1,0},
                             {1,0,1,1,1,1,1,0,1,1,0,1,1,0,0,0,1,0,1,1,0,0,1,1,1,1,1,0,1,0,1},
                             {0,1,1,1,0,1,1,1,0,1,0,1,1,1,0,1,0,0,0,1,1,0,1,1,0,0,0,0,0,1,0},
                             {1,1,1,0,0,1,0,0,0,1,0,1,0,1,1,0,0,1,0,0,1,0,1,0,1,1,0,1,1,0,1},
                             {1,1,0,0,0,0,1,0,0,1,0,0,0,0,0,0,1,1,1,0,1,0,0,1,0,1,1,0,0,1,0},
                             {1,0,0,0,1,1,1,0,0,1,1,0,1,1,0,1,1,0,1,0,1,1,1,0,0,0,0,1,1,0,0},
                             {0,0,0,1,0,1,1,0,0,0,1,1,0,1,1,1,0,0,1,0,0,0,0,0,1,1,1,0,0,0,0},
                             {0,0,1,0,0,1,1,0,1,0,0,0,0,0,1,0,0,0,1,1,1,1,0,1,0,0,0,1,0,0,1},
                             {0,1,0,0,0,1,1,1,1,1,1,0,1,0,0,0,0,0,0,0,0,1,1,0,1,1,1,1,0,1,1},
                             {1,0,0,0,0,1,0,1,0,0,1,1,1,1,0,0,0,1,1,1,0,0,0,1,0,0,1,1,1,1,1}};
                           
IntervalTimer sample_timer; // this is a timer object that can run a routine periodically
IntervalTimer tone_timer_h;
IntervalTimer tone_timer_l;

int potpin_y = A9; // joystick pot y is read on this pin
int potpin_x = A8; // joystick pot x is read on this pin

void setup()
{
  // Open serial communications in case you need to debug
  Serial.begin(9600);

  pinMode(7, OUTPUT);
  digitalWrite(7, LOW); // just so the potentiometer bridge works...
  
  pinMode(sig_out, OUTPUT);
  digitalWrite(sig_out, LOW);
  pinMode(FREQUENCYTIMER2_PIN, OUTPUT);
}

int count = 0; // keeps track of where in the signal we are

void write_signal()
{
  digitalWrite(sig_out,(*signal_coeff)[count]);
  if(count==0) {
    if((*signal_coeff)[count]==1) FrequencyTimer2::setPeriod(Th);
    else FrequencyTimer2::setPeriod(Tl);
    FrequencyTimer2::enable();
  }
  else {
  if((*signal_coeff)[count]==1&&(*signal_coeff)[count-1]==0) {
    FrequencyTimer2::setPeriod(Th);
  } else if ((*signal_coeff)[count]==0&&(*signal_coeff)[count-1]==1) {
    FrequencyTimer2::setPeriod(Tl);   
  }
  }
  count+=1;
}

int potpos_x;
int potpos_y;

void loop()
{
  Serial.println(Th);
Serial.println(Tl);
  if(count==N) {
    count=0;
    sample_timer.end();
    digitalWrite(sig_out, LOW);
    FrequencyTimer2::disable();
    delay(1000*N/fs); // add a delay between signals
  }
  potpos_x=analogRead(potpin_x);
  potpos_y=analogRead(potpin_y);
  if (potpos_x>900 && count==0) {
    signal_coeff = &signal_coeff_MN[0];
    //FrequencyTimer2::setPeriod(20);
    //FrequencyTimer2::enable();
    //delay(100*N/fs); // give a startup tone..
    sample_timer.begin(write_signal, 1e6/fs);
    delay(1000*N/fs); // wait for the signal to send
  }
  if (potpos_x<100 && count==0) {
    signal_coeff = &signal_coeff_MN[1];
    //FrequencyTimer2::setPeriod(20);
    //FrequencyTimer2::enable();
    //delay(100*N/fs); // give a startup tone..
    sample_timer.begin(write_signal, 1e6/fs);
    delay(1000*N/fs); // wait for the signal to send
  }
  if (potpos_y>900 && count==0) {
    signal_coeff = &signal_coeff_MN[2];
    //FrequencyTimer2::setPeriod(20);
    //FrequencyTimer2::enable();
    //delay(100*N/fs); // give a startup tone..
    sample_timer.begin(write_signal, 1e6/fs);
    delay(1000*N/fs); // wait for the signal to send
  }
  if (potpos_y<100 && count==0) {
    signal_coeff = &signal_coeff_MN[3];
    //FrequencyTimer2::setPeriod(20);
    //FrequencyTimer2::enable();
    //delay(100*N/fs); // give a startup tone..
    sample_timer.begin(write_signal, 1e6/fs);
    delay(1000*N/fs); // wait for the signal to send
  }
}


