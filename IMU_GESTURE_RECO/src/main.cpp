/*
  IMU Classifier
  This example uses the on-board IMU to start reading acceleration and gyroscope
  data from on-board IMU, once enough samples are read, it then uses a
  TensorFlow Lite (Micro) model to try to classify the movement as a known gesture.
  Note: The direct use of C/C++ pointers, namespaces, and dynamic memory is generally
        discouraged in Arduino examples, and in the future the TensorFlowLite library
        might change to make the sketch simpler.
  The circuit:
  - Arduino Nano 33 BLE or Arduino Nano 33 BLE Sense board.
  Created by Don Coleman, Sandeep Mistry
  Modified by Dominic Pajak, Sandeep Mistry
  This example code is in the public domain.
*/

#include <Arduino.h>
#include <arduinoFFT.h>

#ifdef COLLECT_DATA
  #warning "This code is for collecting data to train the model"
  #define SERIAL_PRINTLN(...) 
#else 
  #define SERIAL_PRINTLN(...) Serial.println(__VA_ARGS__)
#endif

#ifdef NANO33BLE_SENSE_REV2
  #include <Arduino_BMI270_BMM150.h>
#elif defined(NANO33BLE_SENSE)
  #include <Arduino_LSM9DS1.h>
#endif


#include <TensorFlowLite.h>
#include <tensorflow/lite/micro/all_ops_resolver.h>
#include <tensorflow/lite/micro/micro_interpreter.h>
#include <tensorflow/lite/schema/schema_generated.h>

#include "model.h"
// we changed the threshold of the acceleration because it was too high to detect up-down movement and rest 
const float accelerationThreshold = 2.0; // threshold of significant in G's
const uint16_t numSamples = 128;

//buffer of datas 
uint64_t timestamp[numSamples];
float ax[numSamples];
float ay[numSamples];
float az[numSamples];
float gx[numSamples];
float gy[numSamples];
float gz[numSamples];

int samplesRead = numSamples;

// global variables used for TensorFlow Lite (Micro)
//tflite::MicroErrorReporter tflErrorReporter;

// pull in all the TFLM ops, you can remove this line and
// only pull in the TFLM ops you need, if would like to reduce
// the compiled size of the sketch.
tflite::AllOpsResolver tflOpsResolver;

const tflite::Model* tflModel = nullptr;
tflite::MicroInterpreter* tflInterpreter = nullptr;
TfLiteTensor* tflInputTensor = nullptr;
TfLiteTensor* tflOutputTensor = nullptr;

// Create a static memory buffer for TFLM, the size may need to
// be adjusted based on the model you are using
constexpr int tensorArenaSize = 8 * 1024;
byte tensorArena[tensorArenaSize] __attribute__((aligned(16)));

// FFT setup
const float signalFrequency = 1000;
const float samplingFrequency = 5000;
const uint8_t amplitude = 100;

float vImag[numSamples];


// array to map gesture index to a name
// const char* GESTURES[] = {
//     "circle-1",
//     "circle-2",
//     "rest-1",
//     "shake-1",
//     "up-down-1"
// };
// const char* GESTURES[] = {
//     "circle-1",
//     "circle-2",
//     "shake-1",
//     "up-down-1"
// };

// #define NUM_GESTURES (sizeof(GESTURES) / sizeof(GESTURES[0]))

void printFileHeader() {
  Serial.print("aX_mean,aX_stddev,aX_rms,aX_min,aX_max,aX_psdMean,aX_psdMax,");
  Serial.print("aY_mean,aY_stddev,aY_rms,aY_min,aY_max,aY_psdMean,aY_psdMax,");
  Serial.print("aZ_mean,aZ_stddev,aZ_rms,aZ_min,aZ_max,aZ_psdMean,aZ_psdMax,");
  Serial.print("gX_mean,gX_stddev,gX_rms,gX_min,gX_max,gX_psdMean,gX_psdMax,");
  Serial.print("gY_mean,gY_stddev,gY_rms,gY_min,gY_max,gY_psdMean,gY_psdMax,");
  Serial.print("gZ_mean,gZ_stddev,gZ_rms,gZ_min,gZ_max,gZ_psdMean,gZ_psdMax");
  Serial.println();
}

#define RED_LED_PIN     22
#define GREEN_LED_PIN   23
#define BLUE_LED_PIN    24

void setup() {
  // Configuring pins 22, 23, and 24 as outputs to power the RGB LED
  pinMode(22, OUTPUT);
  pinMode(23, OUTPUT);
  pinMode(24, OUTPUT);
  digitalWrite(22, LOW); 
  digitalWrite(23, LOW);
  digitalWrite(24, LOW);
  delay(1000);
  digitalWrite(23, HIGH);
  digitalWrite(24, HIGH);

  Serial.begin(9600);
  while (!Serial);
  digitalWrite(22, HIGH);

  // initialize the IMU
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }
  
  while (!IMU.accelerationAvailable() || !IMU.gyroscopeAvailable()) {
    SERIAL_PRINTLN("Waiting for IMU to be available...");
    delay(100);
  }
  IMU.readAcceleration(ax[0], ay[0], az[0]);
  IMU.readGyroscope(gx[0], gy[0], gz[0]);
  SERIAL_PRINTLN("IMU initialized successfully!");

  #ifndef COLLECT_DATA
    // print out the samples rates of the IMUs
    Serial.print("Accelerometer sample rate = ");
    Serial.print(IMU.accelerationSampleRate());
    Serial.println(" Hz");
    Serial.print("Gyroscope sample rate = ");
    Serial.print(IMU.gyroscopeSampleRate());
    Serial.println(" Hz");

    Serial.println();

    // get the TFL representation of the model byte array
    tflModel = tflite::GetModel(model);
    if (tflModel->version() != TFLITE_SCHEMA_VERSION) {
      Serial.println("Model schema mismatch!");
      while (1);
    }

    // Build an interpreter to run the model with.
    static tflite::MicroInterpreter static_interpreter(
        tflModel, tflOpsResolver, tensorArena, tensorArenaSize);
    tflInterpreter = &static_interpreter;

    // Create an interpreter to run the model
    //tflInterpreter = new tflite::MicroInterpreter(tflModel, tflOpsResolver, tensorArena, tensorArenaSize, &tflErrorReporter);

    // Allocate memory for the model's input and output tensors
    tflInterpreter->AllocateTensors();

    // Get pointers for the model's input and output tensors
    tflInputTensor = tflInterpreter->input(0);
    tflOutputTensor = tflInterpreter->output(0);
  #else
    printFileHeader();
  #endif
}

float mean(float *data, uint16_t size = numSamples) {
  float sum = 0;
  for (int i = 0; i < size; i++) sum += data[i];
  return sum / size;
}

float stddev(float *data, uint16_t size = numSamples) {
  float m = mean(data, size);
  float sum = 0;
  for (int i = 0; i < size; i++) {
    float d = data[i] - m;
    sum += d * d;
  }
  return sqrt(sum / size);
}

float rms(float *data, uint16_t size = numSamples) {
  float sum = 0;
  for (int i = 0; i < size; i++) {
    sum += data[i] * data[i];
  }
  return sqrt(sum / size);
}

float minVal(float *data, uint16_t size = numSamples) {
  float m = data[0];
  for (int i = 1; i < size; i++) {
    if (data[i] < m) m = data[i];
  }
  return m;
}

float maxVal(float *data, uint16_t size = numSamples) {
  float m = data[0];
  for (int i = 1; i < size; i++) {
    if (data[i] > m) m = data[i];
  }
  return m;
}

/* 
  Simple PSD approximation (no FFT)
  We approximate the signal power in the frequency domain by using 
  the squared difference between consecutive samples.
  This is similar to measuring how "fast" the signal changes,
  which correlates with high-frequency content.
*/
float psdMean(float *data, uint16_t size = numSamples/2) {
  float sum = 0;
  for (int i = 1; i < size; i++) {
    sum += data[i];
  }
  // average power over the whole window
  return sum / size;
}

float psdMax(float *data, uint16_t size = numSamples/2) {
  float maxp = 0;
  for (int i = 1; i < size; i++) {
    if (data[i] > maxp) maxp = data[i];
  }
  return maxp;
}

float complexAbs(float re, float im) {
  return sqrt(re * re + im * im);
}

void computePSD(float *dataRe, float *dataIm, float samplingFreq, uint16_t size = numSamples/2) {
  for(int i = 0; i < size; ++i){
    dataRe[i] = complexAbs(dataRe[i], dataIm[i]) * complexAbs(dataRe[i], dataIm[i]) / (size * samplingFreq);
  }
}

void computeFFT(float *data,float samplingFreq ,uint16_t size = numSamples) {
  memset(vImag, 0, sizeof(vImag)); // zero out the imaginary part
  ArduinoFFT<float> FFT = ArduinoFFT<float>(data, vImag, size, samplingFreq);
  // Compute FFT
  // FFT.windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.compute(FFT_FORWARD);
}

uint32_t t0 = 0;

void readSerialCommand() {
  String serialCommand = "";
  if (Serial.available() > 0) {
    serialCommand = Serial.readStringUntil('\n');
  }
  if (serialCommand == "printHeader") {
    // digitalWrite(GREEN_LED_PIN, LOW);
    printFileHeader();
  }else if (serialCommand != "") {
    digitalWrite(RED_LED_PIN, !digitalRead(RED_LED_PIN));
    Serial.print("Unknown command: ");
    Serial.println(serialCommand);
  }
}

void loop() {

#ifndef COLLECT_REST
  while (1) {
    readSerialCommand();
    if (IMU.accelerationAvailable()) {
      // read the acceleration data
      IMU.readAcceleration(ax[0], ay[0], az[0]);

      // sum up the absolutes
      float aSum = fabs(ax[0]) + fabs(ay[0]) + fabs(az[0]);

      // check if it's above the threshold
      if (aSum >= accelerationThreshold) {
        // reset the sample read count
        samplesRead = 0;
        break;
      }
      
    }
  }

#else
  float aSum = 0;
  while (1) { // every 1 second
    int timeout = 1000;
    if(millis() - t0 > timeout){
      t0 = millis();
      SERIAL_PRINTLN("Timeout reached, starting new sample...");
      break;
    }

    if (IMU.accelerationAvailable()) {
      // read the acceleration data
      IMU.readAcceleration(ax[0], ay[0], az[0]);
      
      // sum up the absolutes
      aSum = fabs(ax[0]) + fabs(ay[0]) + fabs(az[0]);
      // check if it's above the threshold
      if (aSum >= accelerationThreshold && millis() - t0 > 1500) {
        // Serial.println("Movement detected!");
        t0 = millis();
        break;
      }
    }
  }
#endif
  // Sampling the data from the IMU until we have enough samples to fill our buffers
  while (samplesRead < numSamples) { 
    // check if new acceleration AND gyroscope data is available
    if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
      // read the acceleration and gyroscope data
      IMU.readAcceleration(ax[samplesRead], ay[samplesRead], az[samplesRead]);
      IMU.readGyroscope(gx[samplesRead], gy[samplesRead], gz[samplesRead]);
      // Save the timestamp of the sample
      timestamp[samplesRead] = millis();
      samplesRead++;
    }
  }
  samplesRead = 0;

  // Post processing the data by
  // normalizing the IMU data between 0 to 1 and calculating the mean sampling interval
  uint64_t interval = 0;
  for (int i = 0; i < numSamples; i++) {
    // Normalize acceleration data from -4g to +4g
    ax[i] = (ax[i] + 4.0) / 8.0;
    ay[i] = (ay[i] + 4.0) / 8.0;
    az[i] = (az[i] + 4.0) / 8.0;
    // Normalize gyroscope data from -2000 dps to +2000 dps
    gx[i] = (gx[i] + 2000.0) / 4000.0;
    gy[i] = (gy[i] + 2000.0) / 4000.0;
    gz[i] = (gz[i] + 2000.0) / 4000.0;
    // Calculate the mean sampling interval
    if (i > 0) {
      interval += timestamp[i] - timestamp[i - 1];
    }
  }
  double samplingFreq = (1.0E3 * (double)numSamples) / interval; // mean sampling interval in milliseconds
  // double samplingPeriod = (double)interval / (numSamples - 1); // mean sampling period in microseconds
  // Serial.print("Interval: ");
  // Serial.print(interval);
  // Serial.println(" ms");

  // Serial.print("Sampling frequency: ");
  // Serial.print(samplingFreq);
  // Serial.println(" Hz");

  // Serial.print("Mean sampling period: ");
  // Serial.print(samplingPeriod);
  // Serial.println(" us");

  // for (int i = 0; i < 10; ++i) {
  //   Serial.print("timestamp[");
  //   Serial.print(i);
  //   Serial.print("] = ");
  //   Serial.println(timestamp[i]);
  // }

  // Extracting features from the raw data
  float features[42];
  int k = 0;

  float* axes[6] = {ax, ay, az, gx, gy, gz};

  for(int a = 0; a < 6; a++){
    features[k++] = mean(axes[a]);
    features[k++] = stddev(axes[a]);
    features[k++] = rms(axes[a]);
    features[k++] = minVal(axes[a]);
    features[k++] = maxVal(axes[a]);
    computeFFT(axes[a], samplingFreq);
    computePSD(axes[a], vImag, samplingFreq); // Assuming a sampling frequency of 100 Hz
    features[k++] = psdMean(axes[a]);
    features[k++] = psdMax(axes[a]);
  }

#ifndef COLLECT_DATA
  // Filling the input tensor for the model
  for(int i = 0; i < 42; i++){
    tflInputTensor->data.f[i] = features[i];
  }

  // Run inference with the model on the input data
  TfLiteStatus invokeStatus = tflInterpreter->Invoke();
  if (invokeStatus != kTfLiteOk) {
    Serial.println("Invoke failed!");
    while (1);
    return;
  }

  // Loop through the output tensor values from the model
  for (uint8_t i = 0; i < NUM_GESTURES; i++) {
    Serial.print(GESTURES[i]);
    Serial.print(": ");
    Serial.println(tflOutputTensor->data.f[i], 6);
  }
  Serial.println();
  // samplesRead = numSamples; //reinitisialize (avoid some bugs)
#else
  for(int i = 0; i < 42; i++){
    Serial.print(features[i], 6);
    if(i < 41) Serial.print(",");
  }
  Serial.println();

  digitalWrite(GREEN_LED_PIN, !digitalRead(GREEN_LED_PIN));
#endif
}
