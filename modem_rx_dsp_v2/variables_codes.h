const int N = 31; // size of data buffer 
const int M = 33; // total number of gold codes (2^n+1)
const int fs = 200; // transmit chip rate [Hz]
const int n = 4; // oversampling factor to protect against aliasing?
const double thresh = 0.4; // detection threshold

int code_count0=0; // counts number of time code has been received
int code_count1=0;
int code_count2=0;
int code_count3=0;

int (*filter_coeff0)[n*N]; // this is a pointer to a length n*N 1D array that will point towards the filter that we want to use
int (*filter_coeff1)[n*N];
int (*filter_coeff2)[n*N];
int (*filter_coeff3)[n*N];
int filter_coeff_MN[M][n*N]; // this array will store the matched filter sequences
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

int filter_output0; // output of FIR filter
int filter_output1;
int filter_output2;
int filter_output3;

int data_buffer[n*N]={0}; // buffers the read signal (initialize to all zeros)

IntervalTimer code_timer; // this is a timer object that can run a routine periodically
