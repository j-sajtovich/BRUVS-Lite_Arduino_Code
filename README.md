# BRUVS-Lite_Arduino_Code
This repository contains the Arduino Nano sketch used to operate the BRUVS-Lite system described in Sajtovich et al. (2025). The code controls the recording and light settings of the system.

# Hardware
- Arduino nano microcontroller

# How to use 
1. Download the 'Stereo_BRUVS_4.7.0.04.ino' file and open in Arduino IDE software
2. Configure BRUVS-Lite settings by altering numerical values under 'Settings' heading only (lines 92-95)
     - Line 93: 'RecordTime =' Sets the total recording duration of the BRUVS-Lite system. Value is in milliseconds (e.g., `30000` = 30 seconds).
     - Line 94: 'LightLevel =' Adjusts the intensity of the integrated lights. Smaller values result in decreased light.
     - Line 95: 'Interval =' Specifies the delay between recording intervals (standby period). Value is in milliseconds (e.g., `300000` = 5 minutes).
3. Upload configured code to the Arduino nano microcontrollers in BOTH BRUVS-Lite stereo-camera's prior to operation.

> Only modify the numerical values in the Settings section to avoid errors in functionality.

# Citation
If you use this code or BRUVS-Lite system, please cite:
> Sajtovich, J. et al. (2025). Development and Application of BRUVS-Lite: A Stereo-BRUV System with Integrated Lighting for Benthic Marine Monitoring in Northern Latitudes. Methods in Ecology and Evolution. 

# Questions?
  - Feel free to contact me at [jessica.sajtovich@dal.ca](mailto:jessica.sajtovich@dal.ca)
