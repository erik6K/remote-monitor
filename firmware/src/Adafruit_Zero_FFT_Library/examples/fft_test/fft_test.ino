/* This example shows the most basic usage of the Adafruit ZeroFFT library.
 * it calculates the FFT and prints out the results along with their corresponding frequency
 * 
 * The signal.h file constains a 200hz sine wave mixed with a weaker 800hz sine wave.
 * The signal was generated at a sample rate of 8000hz.
 * 
 * Note that you can print only the value (coment out the other two print statements) and use
 * the serial plotter tool to see a graph.
 */

#include "Adafruit_ZeroFFT.h"
#include "signal.h"

//the signal in signal.h has 2048 samples. Set this to a value between 16 and 2048 inclusive.
//this must be a power of 2
#define DATA_SIZE 512

//the sample rate
#define FS 13392.857

// the setup routine runs once when you press reset:
void setup() {
  Serial.begin(115200);
  while(!Serial); //wait for serial to be ready

  DCRemoval(signal, DATA_SIZE);

  //run the FFT
  ZeroFFT(signal, DATA_SIZE);

  //data is only meaningful up to sample rate/2, discard the other half
  for(int i=0; i<DATA_SIZE/2; i++){
    
    //print the frequency
    Serial.print(FFT_BIN(i, FS, DATA_SIZE));
    Serial.print(" Hz: ");

    //print the corresponding FFT output
    Serial.println(signal[i]);
  }

  Serial.println("Magnitude at 50Hz:");
  Serial.println(signal[FFT_INDEX(50, FS, DATA_SIZE)]);

}

void loop() {
  //don't even do anything
}

void DCRemoval(int16_t *vData, uint16_t samples)
{
  // calculate the mean of vData
  int mean = 0;
  for (uint16_t i = 0; i < samples; i++)
  {
    mean += vData[i];
  }
  mean = mean / samples;
  // Subtract the mean from vData
  for (uint16_t i = 0; i < samples; i++)
  {
    vData[i] -= mean;
  }
}