# PrincessPiTracker
**here be dragons**  
btle tracking via esp32 but with more support for chips, more options, radical power shaving and nun of da suck  
  
i hvae become one with the build system, the toolchain, the sdk :Scuffled_Flustered: the ephmeral metaphysicaL Cconcept of esp-idf has adopted me as one of its own i shall soon know the true name of compuilauion
  
![this is the face of pleasure](assets/CHAMELEON.jpg)
# Original reaDME below
# Find My Device ESP32 Firmware
This code enables you to use an ESP32-device as a custom Google Find My Device tracker. Note that the firmware is very experimental. 

The firmware works differently to regular Find My Device trackers. It is made to be as simple as possible. It has no Fast Pair support, MAC rotation, advertisement rotation, etc.

Currently known working devices include the ESP32 (Dev Module V1), the ESP32-CAM, ESP32-C3, ESP32-C6, and ESP32-S3. If you use a different board and it works/doesn't work, feel free to message me, I'll update this README then. 

Tested on ESP-IDF v5.5.1

## How to use
- Run the Python Script [`main.py`](../main.py) in the parent folder. Follow the instructions of the [README of the parent folder](../README.md).
- When the device list is displayed, press 'r' to register a new ESP32/Zephyr device in your account. Copy the displayed advertisement key.
- Install Visual Studio Code [here](https://code.visualstudio.com/download)
- Go to Visual Studio Code Extensions, search, install and open 'ESP-IDF' by Espressif
- Open the folder containing this README file in Visual Studio Code
- Run ESP-IDF set target from the command pallete (Ctrl+Shift+P) and add your model of esp32 (esp32, esp32s3, esp32c3, and esp32c6 currently supported)
- Run ESP-IDF menuconfig from the command pallete in VS Code (Ctrl+Shift+P)
- Under "GoogleFindMyTools Config" add your advertisement key
- Connect your ESP32 to your system with USB
- On the bottom left of Visual Studio Code, click the 'plug' icon and select your ESP32, it should be named '/dev/tty.usbserial-0001' (Linux), COM5 (Windows) or similar
- Click the 'Fire' icon to build and flash the firmware
- If asked, use UART as flash method
- After flashing, the ESP32 will restart and start advertising as the Find My Device tracker previously registered

## Known Issues
- You need to run [`main.py`](../main.py) every 4 days to keep receiving location reports from the server. This is because the advertisements have to be "announced" to Google. 
- Might not work with 'fresh' Google accounts: "Your encryption data is locked on your device" is shown if you have never paired a Find My Device tracker with an Android device. Solution: See [README of the parent folder](../README.md).
- You cannot view locations for the ESP32 in the Google Find My Device app. You will need to use the Python script to do so.
- No privacy features such as rotating MAC addresses are implemented
- The firmware was built to receive as many network reports as possible. Therefore, it might consume more power than necessary. To fix this, you can tweak the parameters (TX Power and advertising interval) in [`main.c`](main/main.c)
