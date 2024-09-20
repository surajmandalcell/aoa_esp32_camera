# ESP32-CAM Gauge Reader

This project utilizes an ESP32-CAM to capture images of analog gauges and process them to obtain digital readings. It combines embedded systems programming with computer vision techniques to create an IoT solution for gauge monitoring.

## Features

- Captures images using ESP32-CAM
- Processes images to extract gauge readings
- Supports Wi-Fi connectivity for data transmission
- Includes both ESP32-CAM firmware and Python processing script

## Prerequisites

- ESP32-CAM board
- Arduino IDE with ESP32 board support
- Python 3.x
- Git

## Installation

1. Clone the repository with submodules:
   ```bash
   git clone --recurse-submodules https://github.com/surajmandalcell/aoa_esp32_camera.git
   cd aoa_esp32_camera
   ```

2. Set up Wi-Fi credentials:
   - Rename `.env.h.example` to `.env.h`
   - Edit `.env.h` with your Wi-Fi SSID and password:
     ```cpp
     #ifndef ENV_H
     #define ENV_H

     #define WIFI_SSID "your_wifi_ssid"
     #define WIFI_PASS "your_wifi_password"

     #endif
     ```

## Usage

### ESP32-CAM Firmware

1. Open `main.cpp` in Arduino IDE.
2. Select the appropriate board and port.
3. Compile and upload the firmware to your ESP32-CAM.

### Python Script

1. Install required Python dependencies (list them in a `requirements.txt` file).
2. Run the Python script:
   ```bash
   python main.py
   ```

## Configuration

(Add any configuration details, such as adjusting camera settings or modifying image processing parameters)

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## Updating Dependencies

To update the ESP32-CAM library, run:
```bash
git submodule update --remote libraries/esp32cam
```

## License

This project is open-source and available under the [MIT License](LICENSE).

## Acknowledgements

- Original gauge reading code by [Bartek Szymanski](https://github.com/bartek-szymanski-szyba/ESP32-projects)
- ESP32-CAM library by [yoursunny](https://github.com/yoursunny/esp32cam)

## Support

For issues, feature requests, or questions, please open an issue in the GitHub repository.