/* 
 *  Based off of https://www.embedded.com/design/configurable-systems/4024443/The-Goertzel-Algorithm
 *  Implements the Goertzel Algorithm to detect a specific tone in a deterministically sampled input signal
 */

#include "WProgram.h"

void resetGoertzel()
{
  for (int i = 0; i < Nfreq; i++){
    Q2[i] = 0;
    Q1[i] = 0;
  }
}

void initGoertzel()
{
  int k;
  float floatL;
  float w;
  floatL = (float) L;
  for (int i = 0; i < Nfreq; i++){
    k = (int) (0.5 + ((floatL * TARGET_FREQUENCY[i]) / FS));
    w = (2.0 * PI * k) / floatL;
    sine[i] = sin(w);
    cosine[i] = cos(w);
    coeff[i] = 2.0 * cosine[i];
  }
  resetGoertzel();
}

void processSample(unsigned int sample)
{
  float Q0;
  for (int i = 0; i < Nfreq; i++){
    Q0 = coeff[i] * Q1[i] - Q2[i] + (float) sample;
    Q2[i] = Q1[i];
    Q1[i] = Q0;
  }
}

float getMagnitudeSquared(int i)
{
  float result;
  result = Q1[i] * Q1[i] + Q2[i] * Q2[i] * coeff[i];
  return result; // this is a copout :/ need to figure out what's going on. seems like there's some weird sign stuff that goes on when cos(w)<0
}

void readSignal()
{
  sig_temp = analogRead(sig_in);
  (*samples_to_read)[count] = w[count]*sig_temp; // store a windowed version of the data
  count++;
  if (count == L) {
    // reset count to 0
    count = 0; 
    // flip the pointer to the other sample buffer...
    samples_to_process = &sample_buffer[buffer_index];
    samples_to_read = &sample_buffer[!buffer_index];
    buffer_index = !buffer_index;
    // flag the runGoertzel subroutine which will process samples_to_process...
    digitalWrite(15, LOW);
    digitalWrite(15, HIGH);
    // ERROR ERROR ERROR, this means that we've flipped buffers and we haven't finished processing the old one
    if (goertzelFlag == 1) {
      Serial.println("runGoertzel may have taken too long :(");
    }
  }
}

void runGoertzel()
{
  goertzelFlag = 1; 
  for (int i = 0; i < L; i++){
    processSample((*samples_to_process)[i]);
  }
  for (int i = 0; i < Nfreq; i++){
    magnitudeSquared[i] = abs(getMagnitudeSquared(i)); // don't forget the abs!
    mag_norm[i] = filter_correction/L*sqrt(magnitudeSquared[i]);
    if (mag_norm[i] > det_thresh[i]) {
      tone_detected[i]=1;
      //Serial.print("tone ");
      //Serial.print(i);
      //Serial.print(" detected! ");
      //Serial.println(mag_norm[i]);
      digitalWrite(13, HIGH);
      digitalWrite(i, HIGH);
    } else {
      tone_detected[i]=0;
      digitalWrite(13, LOW);
      digitalWrite(i, LOW);
    }
  }
  resetGoertzel();
  goertzelFlag = 0;
}

