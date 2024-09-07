# ESP32-CAM Guage Reader


## Usage

1. Do git clone `git clone --recurse-submodules https://github.com/surajmandalcell/aoa_esp32_camera.git`

2. Use `main.cpp` as esp32-cam firmware. The code is written in Arduino IDE.

3. Use main.py as the python script to read the gauge.


## Update libraries

To update the libraries, use the following command:

```bash
git submodule update --remote libraries/esp32cam
```


# Thanks to Bartek Szymanski for the code to read the gauge. The improvements are made on top of his code.

https://github.com/bartek-szymanski-szyba/ESP32-projects.git