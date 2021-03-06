# Robot, Movement Sensor and three.js 3D model

Adding a backpack containing an ESP32, 9-axis movement sensor, battery and switch to a toy robot and controlling a 3D model in the browser with three.js. Full instructions on the blog: https://robotzero.one/esp32-mpu9250-three-js/

![Toy Robot and 3D Model](https://robotzero.one/github-assets/3d-robot-mpu9250-threejs.jpg)

This repository contains the Arduino IDE sketch robotRotationMPU9250.ino and the three folders described below:

### web-examples
If you just want to have a quick look and play with this project, this folder contains two examples that don't require the ESP32 and MPU9250.

### sd
This folder contains the files to be copied to the SD card.

### parcel
This folder contains the original HTML and JavaScript files that are used by Parcel to create the dist folder. The dist folder contains the HTML that is served by the ESP32 and the parcel.xxx.js that the ESP32 serves from the SD card. I've added this in case anyone wants to see where the files used in the project come from.
