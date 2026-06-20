# Embedded Gesture Recognition
This repository contains the code for the 'First Assignment' of the 'Low-Power Embedded Systems' course. The project is focused on performing gesture recognition using the IMU sensor of the Arduino Nano 33 BLE Sense.

The project is divided into three main parts:
1. Collecting data from the IMU sensor;
2. Training a Neural Network model for gesture recognition using the collected data;
3. Implementing the trained model on the Arduino Nano 33 BLE Sense to perform real-time gesture recognition.

# Training the Neural Network model

## Our Solution
For the training of the Neural Network model, the `Model-Training-PSD.ipynb` Jupyter Notebook is used. This notebook can be run locally or on Google Colab, and it contains all the code needed to train the model using the collected data.
This notebook uses the data collected from the board but after extracting the features using FFT and PSD on the device (see [here](./Model-Training-PSD.ipynb#Extraction-of-features) to see more details) and then it trains a simple Neural Network model using TensorFlow and Keras. The trained model is then saved in the `scratch` directory, which can be used later to implement the model on the Arduino Nano 33 BLE Sense.

## The comparison between the old model
During classes we trained a model using only the raw data collected from the IMU sensor, without extracting any features. This model was able to recognize two kinds of gestures (punch and flex) with good accuracy. So interested on understanding why we need to extract features from the raw data, we try to se how this kind of model performs on the reconnition of the other gestures (circle, shake, up-down and rest) and we found that the accuracy of the model is very low, around 80% during the training and around 70% during the validation. This could be due to the fact that the raw data collected from the IMU sensor is very noisy and it is difficult for the model to learn the patterns of the gestures without extracting any features. On the other hand, by extracting features using FFT and PSD, we are able to capture the frequency components of the signal, which are more relevant for the recognition of the gestures.
Note that this allows the model to learn better and achieve higher accuracy also if the number of gestures is higher and the model structure is smaller, infact the "worse" model has more layers with more neurons compared to the "better" one (see the two 'Build & Train the Model' sections on the notebooks to see more details about the two models structure).
This model is implemented in the `Model-Training-Simple.ipynb` notebook, which is similar to the `Model-Training-PSD.ipynb` notebook but it uses only the raw data collected from the IMU sensor without extracting any features.
The data to provvide to this model are stored in the `data-gestures/a-<gesture>.csv` files.


# Run the training
## How to use the *jupyter-notebook* locally   

*Version of python* : 3.11 in order to use Tensorflow which is deprecated for versions > python3.11 
If you want to use the notebooks it is recommended to use a virtual environment, here is a list of instructions 

### install of the python3.11
 some examples depending on your package manager. 
MAC OS X 
```bash   
brew install python@3.11 
```

GNU/LINUX (Ubuntu) : 
```bash   
sudo apt instal python3.11 
```
### Creation of the environment in command-line 

```bash   
python3.11 -m venv .venv 
```

### Activation of the environment 
```bash   
source .venv/bin/activate
```
Normally, you will see the name of your environment shown before your prompt like bellow : 
```bash   
(.venv) streopes13@steropes13 $ ls -al 
```

### Installation of jupyter notebook (Mac)
```bash   
pip3 install jupyter-notebook 
```
other version : 
```bash   
python3 -m pip install jupyter-notebook
```
by replacing <jupyter-notebook> you can install libraries needed (_pandas, tensorflow, keras.._) 

### Installation of jupyter notebook (Linux)
```bash
pip install jupyterlab
```

### _(If needed)_ Deactivate the environment 
```bash   
deactivate
```

### _(optional)_  Verification of the environment 
Execute this command 
```bash   
which python 
which pip 
```
if it shows the *path of the venv* it works correctly 


### _(optional)_  Verification of the environment in jupyter-notebook
in a cell specified of python of notebook file (_ipynb_)  you can verify if it uses also the same environment as you have created. 
```python   
import sys 
print(sys.executable) 
```
if after executing this cell it shows you the path of your virtual environment, it works ! 


## Instructions for the Google Colab 
You have to import the diferent `.csv` files of this repository (in the `data-gestures`repository in the notebook at the same leval as `sample_data`).
### Link of the google Collab [here](https://colab.research.google.com/drive/1zVgsqpySxEAGOxTOStpjsPD_wjLUeIkG?usp=sharing)

# Running the PlatformIO projects (Arduino code)
## Dependencies
To run the PlatformIO projects, you will need to have the PlatformIO VSCode extension installed on your system. 
This need to open the `IMU_GESTURE_RECO` folder directly with VSCode, so we prefere to use the CLI tool instead.

The following instructions provide a guide on how to use the PlatformIO CLI tool wich can be installed locally and used directly in the terminal.
You can install it using following the instructions on the [PlatformIO website](https://docs.platformio.org/en/latest/core/installation/index.html). A more detailed instruction is available in the [`README.md`](./IMU_GESTURE_RECO/README.md##compiling-the-project-using-platformio-cli) file of the `IMU_GESTURE_RECO` directory.

## What are the PIO environments
In the `platformio.ini` file, you can find the definition of some environments like:
```ini
[env:nano33ble_SenseRev2]
platform = nordicnrf52
board = nano33ble_sense_rev2
framework = arduino
build_flags = -D IMU_GESTURE_RECO_SENSE_REV2
...
```
These environments are used to compile the code for different versions of the Arduino Nano 33 BLE Sense board but also with different configurations. See [here](./IMU_GESTURE_RECO/README.md#environments-description).

## Compiling the code and uploading the firmware
Using the PlatformIO CLI, you can compile the project using:
```bash
pio run -e <environment_name> -t upload -d IMU_GESTURE_RECO
```
> Make sure to replace `<environment_name>` with the name of the environment you want to use (`nano33ble_SenseRev2` or `nano33ble_Sense`).

## Monitoring the serial output
After uploading the firmware, you can monitor the serial output using:
```bash
pio device -t monitor -d IMU_GESTURE_RECO
```

## Other information
If you want to compile the project testing all the environments, you can use the command:
```bash
pio run -d IMU_GESTURE_RECO
```

and a test is written to perform validation of the model implemented on the device, to run it you can use the command:
```bash
pio test -e <environment_name> -d IMU_GESTURE_RECO
```
> Note: this capability is meant to be used with a testing framework like [Unity](https://www.throwtheswitch.org/unity) but for some reason it is not working correctly on NRF52 devices. But the test runned in this way uploads the correct firmware on the device, so you can a simple serial monitor to check the output of the test and see if it is working correctly.

# Other information
## Collecting data with the `collect_data.py` script
### Dipendencies
To collect data using the `collect_data.py` script, you will need to have Python installed on your system, along with the `pyserial` library. You can install `pyserial` using pip:
```bash
pip install pyserial
```

### Running the data collection script
To run the data collection first of all upload the firmware selecting the correct environment as described in the [previous section](). Then, you can run the `collect_data.py` script using:

```bash
python collect_data.py <file_name> 
```
After running this command, the script will ask a number to the corrisponding port you want from the list. 

> Make sure to replace `<file_name>` with the name of the file you want to save the collected data to (e.g., `data-gestures/circle-1.csv`).

## Performance of the FFT and PSD calculations on the device and correlation with Python calculations

The `scripts/fftAnalysis.py` script is used to perform correlation between the FFT and PSD values calculated by the Arduino and the ones calculated using Python. It needs the `numpy`, `pandas` and `matplotlib` libraries to work and is linked to the `data-gestures/analog.csv` file, which is the file where the we collectd data of a Triangular Wawe carried at 300Hz and sampled at 1KHz using the arduino ADC. After collecting data on the arduino, the FFT and PSD values are calculated on the device and printed out using the Serial Port, then the `fftAnalysis.py` script reads these values and compares them with the ones calculated using Python, showing the results in a plot.
Here you can find the arduino code used to collect the data and calculate the FFT and PSD values: 
```cpp
#include <arduinoFFT.h>

const uint16_t numSamples = 1024;
uint64_t timestamp[numSamples];
float samples[numSamples];
float vReal[numSamples];
float vImag[numSamples];
float psd[numSamples/2];

float complexAbs(float re, float im) {
return sqrt(re * re + im * im);
}

void computePSD(float *dataRe, float *dataIm, float* out, float samplingFreq, uint16_t size = numSamples/2) {
for(int i = 0; i < size; ++i){
    out[i] = complexAbs(dataRe[i], dataIm[i]) * complexAbs(dataRe[i], dataIm[i]) / (size * samplingFreq);
}
}

void setup() {
Serial.begin(9600);
while (!Serial);  
delay(2000); // wait for the serial monitor to open
Serial.println("Analog data collection started");
for(int i = 0; i < numSamples; i++){
    timestamp[i] = micros();
    samples[i] = analogRead(A6);
}

memcpy(vReal, samples, sizeof(samples));
memset(vImag, 0, sizeof(vImag));
ArduinoFFT<float> FFT = ArduinoFFT<float>(vReal, vImag, numSamples, 1000);
FFT.compute(FFT_FORWARD);
computePSD(vReal, vImag, psd, 1000);
FFT.complexToMagnitude();

//Print the values
Serial.println("Time (s), Sample, FFT, PSD");
for (int i = 0; i < numSamples; i++) {
    Serial.print(timestamp[i] / 1e6, 6); // time in seconds
    Serial.print(", ");
    Serial.print(samples[i], 3);
    Serial.print(", ");
    if (i < numSamples/2) {
    Serial.print(vReal[i], 3);
    Serial.print(", ");
    Serial.print(psd[i], 6);
    }else {
    Serial.print("nan, nan");
    }
    Serial.println();
}

}


void loop() {}
```