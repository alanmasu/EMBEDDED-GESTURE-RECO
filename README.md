# Embedded Gesture Recognition
This repository contains the code for the project of the 'Laboratory of IoT' course. The project is focused on performing gesture recognition using the IMU sensor of the Arduino Nano 33 BLE Sense: four boxing punches (`jab`, `hook`, `overhand`, `uppercut`) plus an `unknown` class used to reject every other movement.

The project is divided into three main parts:
1. Collecting data from the IMU sensor;
2. Training a Neural Network model for gesture recognition using the collected data;
3. Implementing the trained model on the Arduino Nano 33 BLE Sense to perform real-time gesture recognition and exposing the recognized gestures as USB HID key presses.

# Training the Neural Network model

## The Solution
For the training of the Neural Network model, the `Model-Training-PSD.ipynb` Jupyter Notebook is used. This notebook can be run locally or on Google Colab, and it contains all the code needed to train the model using the collected data.
This notebook uses the data collected from the board but after extracting the features using FFT and PSD on the device (see [here](./Model-Training-PSD.ipynb#Extraction-of-features) to see more details) and then it trains a simple Neural Network model using TensorFlowLite and Keras. The trained model is then saved in the `scratch` directory (the model will be exported also as `model.h` and will be included by `IMU_GESTURE_RECO/include/model.h` on the Arduino code), which can be used later to implement the model on the Arduino Nano 33 BLE Sense.

## The comparison between the old model
First, I've trained a model using only the raw data collected from the IMU sensor, without extracting any features. This model was able to recognize two kinds of gestures (punch and flex) with good accuracy. So I tried to see how this model performs on the recognition of our punches (`jab`, `hook`, `overhand`, `uppercut` classes) and I found that the model wasn't able to learn nothing. 
This could be due to the fact that the raw data collected from the IMU sensor are very noisy and probably it is difficult for the model to learn the patterns of the gestures without extracting any features. So I decided to extract features from the raw data using FFT and PSD. 
Using this method, the system becomes able to capture the frequency components of the signal, which are more relevant for the recognition of the gestures start learning the differences between the gestures and achieve a sufficient good accuracy.
The data provided to train the model are stored in `data-gesures/<gesture>.csv` files.


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
You have to import the different `.csv` files of this repository (in the `data-gesures` folder in the notebook, at the same level as `sample_data`).
# Running the PlatformIO projects (Arduino code)
## Dependencies
To run the PlatformIO projects, you will need to have the PlatformIO VSCode extension installed on your system. 
This need to open the `IMU_GESTURE_RECO` folder directly with VSCode, so we prefere to use the CLI tool instead.

The following instructions provide a guide on how to use the PlatformIO CLI tool wich can be installed locally and used directly in the terminal.
You can install it using following the instructions on the [PlatformIO website](https://docs.platformio.org/en/latest/core/installation/index.html). 
A more detailed instruction is available in the [`README.md`](./IMU_GESTURE_RECO/README.md#compiling-the-project-using-platformio-cli) file of the `IMU_GESTURE_RECO` directory.

## Trained model
The trained model is included in the `model/` directory and is the last trained version of the model. This is not the directory where the model expected to be, so in order to use it please copy the `model.h` file from the `model/` directory to the `scratch/` directory and compuile the PlatformIO project as indicated in the next section.

# What are the PIO environments
In the `IMU_GESTURE_RECO/platformio.ini` file, you can find the definition of some environments like:
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
pio device monitor -d IMU_GESTURE_RECO
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
### Dependencies
To collect data using the `collect_data.py` script, you will need to have Python installed on your system, along with the `pyserial` library. You can install `pyserial` using pip:
```bash
pip install pyserial
```

### Running the data collection script
To run the data collection first of all upload the firmware selecting one of the data collection environments (`nano33ble_Sense_CollectData` or `nano33ble_SenseRev2_CollectData`), as described in the [previous section](#compiling-the-code-and-uploading-the-firmware). Then, you can run the `collect_data.py` script using:

```bash
python scripts/collect_data.py <output>
```
After running this command, the script will ask for the number of the serial port you want to use, from the list it prints.

`<output>` can be either:
- a file (e.g. `data-gesures/jab.csv`): the script logs everything coming from the serial port into that single file until you press ENTER;
- a directory (e.g. `data-gesures`): the script runs in dataset mode, collecting `SAMPLES_PER_GESTURE` samples for every gesture listed in the `GESTURES` list at the top of `scripts/collect_data.py`, saving each one to `<output>/<gesture>.csv`.

# Important notes for usage: 
> The dataset collection was performed with the Arduino on the right hand, USB port on bottom side, and the headers on the rigth side.
> If the orientation of the board is different, the model will not work correctly. 