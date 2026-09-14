# IMU GESTURE RECO 
This PlatformIO project is part of the 'Laboratory of IoT' course project. 
This PIO project will contains the code for running the sampling of the IMU sensor and extracting the features from the collected data using the Power Spectral Density (PSD) method, and based to the build configurations, will perform the inference of the trained Neural Network model for gesture recognition or will simply stream the collected data through the serial port to be saved in a CSV file for the training of the model.
The whole flow is divided into three main parts:
1. Collecting data from the IMU sensor;
2. Apply PSD;
3. Feed the trained Neural Network model for gesture recognition using the collected data or stream the data to the PC;
4. If the inference is performed, the recognized gesture is sent to the Keyboard HID interface, so it can be used to control a game or any other application that uses keyboard input.

So this project is focused to be the first and the third part, while the second part is performed using a Jupyter Notebook that can be runned locally or on Google Colab.

# Compiling the project

## Compiling the project (using VSCode and PlatformIO)
1. Install VSCode on you PC;
2. Install PlatformIO from the extension menu on VSCode;
3. Open this directory using VSCode;
4. After PlatformIO loads, select the Environment for you own board from the down bar;
5. Click on Upload on the down bar;
6. Open the Console.
> NOTE: PlatformIO needs access to USB ports, maybe it's needed to update udev rules under linux or installing some driver for different OSes 

## Compiling the project (using PlatformIO CLI)
1. Install PlatformIO CLI on your PC, following the instructions on the official [website](https://docs.platformio.org/en/latest/core/installation/methods/index.html);
2. Allow PlatformIO to access USB ports, maybe it's needed to update udev rules under linux or installing some driver for different OSes;
3. Be sure to have acces to PlatformIO Core CLI binaries, see [here](https://docs.platformio.org/en/latest/core/installation/shell-commands.html) for more details;
5. Open a terminal and navigate to the project folder;
6. Run the command `pio run -e <environment_name> -t upload` to compile and upload the code to your board, replacing `<environment_name>` with the name of the environment you want to use (see down below for more details);
7. Open the Console or use `pio device monitor -e <environment_name>` to monitor the serial output.

## Environments description
Here you can find a description of the different environments available in the `platformio.ini` file:
- `nano33ble_Sense` & `nano33ble_SenseRev2`: these environments are used to compile the gesture recognition code for the Arduino Nano 33 BLE Sense and the Arduino Nano 33 BLE Sense Rev2, respectively.
- `nano33ble_Sense_CollectData` & `nano33ble_SenseRev2_CollectData`: these environments are used to compile the data collection code for the Arduino Nano 33 BLE Sense and the Arduino Nano 33 BLE Sense Rev2, respectively.

> Note: The data collection code performs also the feature extraction, so the collected data are already in the format needed for the training of the Neural Network model.

## USB HID output
When built with one of the inference environments (`nano33ble_Sense` / `nano33ble_SenseRev2`), the board also enumerates as a USB HID keyboard: every recognized gesture whose confidence is above `gestureConfidenceThreshold` emits the corresponding key tap (WASD mapping defined by `gestureKeys` in `src/main.cpp`). The `unknown` class and low-confidence predictions do not generate any key press.
