#include "WProgram.h"

int data_temp;
int data_prev=0;
int fh_temp;
int fl_temp;

void init_filter_coeff()
{
  int filter_coeff_temp;
  for (int i=0; i<M; i++) {
    // reverse the gold codes and set 0 to -1 to create the matched filter sequences
    for (int j=0; j<N; j++) {
      filter_coeff_temp=signal_coeff_MN[i][N-1-j];
      if (filter_coeff_temp==0) filter_coeff_temp=-1;
      for(int k=0; k<n; k++){
        // expand the filter array to work with the oversampling rate
        filter_coeff_MN[i][j*n+k]=filter_coeff_temp;
        //Serial.print(filter_coeff_temp);
        //Serial.print(" ");
      }
      //Serial.print("|");
    }
    //Serial.println();
  }
  //Serial.println();
  filter_coeff0 = &filter_coeff_MN[0]; // pick a matched filter from the big array
  filter_coeff1 = &filter_coeff_MN[1];
  filter_coeff2 = &filter_coeff_MN[2];
  filter_coeff3 = &filter_coeff_MN[3];
}

void read_code()
// reads in the recieved code and saves to an array
{
  for (int i=0;i<n*N-1;i++){
    // shift previously read array elements to the left
    data_buffer[i]=data_buffer[i+1];
  }
  // newest received data point gets saved at index N
  fh_temp=tone_detected[1];
  fl_temp=tone_detected[0];
  if (fh_temp==1&&fl_temp==0) data_temp = 1;
  else if (fh_temp==0&&fl_temp==1) data_temp = -1;
  //else if (fh_temp==0&&fl_temp==0) data_temp = data_prev; // an attempt to correct overlap errors
  else data_temp=0;
  data_buffer[n*N-1]=data_temp;
  data_prev=data_temp;
  //Serial.print("value read: ");
  //Serial.println(data_temp);
}

unsigned long detection_time=0; // keep track of the time at which detection occurrs
unsigned long prev_detection_time=0;

void fir()
// computes the FIR filter output from the data buffer
{
  filter_output0=0;
  filter_output1=0;
  filter_output2=0;
  filter_output3=0;
  for (int i=0;i<n*N;i++){
    // discrete convolution
    //Serial.println(data_buffer[i]);
    filter_output0+=(*filter_coeff0)[n*N-1-i]*data_buffer[i];
    filter_output1+=(*filter_coeff1)[n*N-1-i]*data_buffer[i];
    filter_output2+=(*filter_coeff2)[n*N-1-i]*data_buffer[i];
    filter_output3+=(*filter_coeff3)[n*N-1-i]*data_buffer[i];
  }
  //Serial.println(filter_output3);
  if(filter_output0>thresh*n*N) {
    detection_time=micros();
    if (detection_time-prev_detection_time>1e6/fs){
      code_count0++;
      Serial.print("code 0 detected! count: ");
      Serial.println(code_count0);
    }
    prev_detection_time=detection_time;
  }
  if(filter_output1>thresh*n*N) {
    detection_time=micros();
    if (detection_time-prev_detection_time>1e6/fs){
      // check to make sure this isn't a repeat detection (possible due to oversampling)
      code_count1++;
      Serial.print("code 1 detected! count: ");
      Serial.println(code_count1);
    }
    prev_detection_time=detection_time;
  }
  if(filter_output2>thresh*n*N) {
    detection_time=micros();
    if (detection_time-prev_detection_time>1e6/fs){
      code_count2++;
      Serial.print("code 2 detected! count: ");
      Serial.println(code_count2);
    }
    prev_detection_time=detection_time;
  }
  if(filter_output3>thresh*n*N) {
    detection_time=micros();
    if (detection_time-prev_detection_time>1e6/fs){
      code_count3++;
      Serial.print("code 3 detected! count: ");
      Serial.println(code_count3);
    }
    prev_detection_time=detection_time;
  }
}

void matched_filter()
{
  read_code();
  fir();
}
