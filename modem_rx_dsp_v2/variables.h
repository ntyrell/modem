#define FS                  100000.0 // sample frequency
#define TARGET_FREQUENCY_1   23256.0 // frequency that we want to detect
#define TARGET_FREQUENCY_0   18868.0 
#define Nfreq 2 // number of frequencies
#define L 199 // Block size

int tone_detected[Nfreq] = {0}; // is tone i currently being detected?

const float TARGET_FREQUENCY[Nfreq] = {TARGET_FREQUENCY_0, TARGET_FREQUENCY_1};

float coeff[Nfreq];
float Q1[Nfreq];
float Q2[Nfreq];
float sine[Nfreq];
float cosine[Nfreq];

int sample_buffer[2][L]; // we will flip flop between the two rows in order to buffer the read data
int (*samples_to_read)[L]; // this is a pointer to the sample buffer row to be currently processed
int (*samples_to_process)[L]; 
float w[L]; // a window for windowing the sampled data
float filter_correction=1.0;

IntervalTimer sample_timer;
int sig_in = 23; // define signal input pin
int buffer_index = 0;

int sig_temp;
int count = 0;

float magnitudeSquared[Nfreq] = {0}; // output of the Goertzel filter
float mag_norm[Nfreq] = {0};
int goertzelFlag = 0; // this indicates whether or not the runGoertzel routine is currently running...
float det_thresh[Nfreq] = {20, 20}; // detection threshold
