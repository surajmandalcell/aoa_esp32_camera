# ESP32-CAM Gauge Reader

This project utilizes an ESP32-CAM to capture images of analog gauges and process them to obtain digital readings. It combines embedded systems programming with computer vision techniques to create an IoT solution for gauge monitoring.

## Features

- Captures high-quality images using ESP32-CAM
- Processes images to extract gauge readings with high accuracy
- Supports Wi-Fi connectivity for real-time data transmission
- Includes both ESP32-CAM firmware and Python processing script
- Configurable image capture intervals and resolution
- Support for multiple gauge types (circular and linear)
- Data logging and export capabilities
- Web interface for remote monitoring and configuration
- Video recording functionality with automatic storage management
- Configurable storage usage limit to prevent SD card overflow

## Prerequisites

- ESP32-CAM board
- Arduino IDE with ESP32 board support
- Python 3.x
- Git
- SD card for storage (if using video recording feature)

## Installation

1. Clone the repository with submodules:
   ```bash
   git clone --recurse-submodules https://github.com/surajmandalcell/aoa_esp32_camera.git
   cd aoa_esp32_camera
   ```

2. Set up Wi-Fi credentials and other configurations:
   - Rename `.env.h.example` to `.env.h`
   - Edit `.env.h` with your Wi-Fi SSID, password, and other settings:
     ```cpp
     #ifndef ENV_H
     #define ENV_H

     #define WIFI_SSID "your_wifi_ssid"
     #define WIFI_PASS "your_wifi_password"

     #define RECORD_VIDEO true
     #define VIDEO_DURATION 60000
     #define MAX_STORAGE_USAGE 80

     #endif
     ```

3. Open `aoa_esp32_camera.ino` in Arduino IDE.(The esp32cam library is already included as a submodule in the project)
4. Compile and upload the firmware to your ESP32-CAM.
5. Install Python dependencies:
   ```bash
   pip install -r requirements.txt
   ```
6. Run the Python script:
   ```bash
   python aoa_esp32_camera.py
   ```
7. Access the ESP32-CAM endpoints to view images or video streams.

## Usage

### ESP32-CAM Firmware

1. Open `aoa_esp32_camera.ino` in Arduino IDE.
2. Select the appropriate board and port.
3. Compile and upload the firmware to your ESP32-CAM.

### Python Script

1. Install required Python dependencies:
   ```bash
   pip install -r requirements.txt
   ```
2. Run the Python script:
   ```bash
   python aoa_esp32_camera.py
   ```

## Configuration

### Camera Settings

You can adjust camera settings in the `initializeCamera()` function in `aoa_esp32_camera.ino`:

```cpp
esp32cam::Config cfg;
cfg.setPins(esp32cam::pins::AiThinker);
cfg.setResolution(hiRes);
cfg.setBufferCount(2);
cfg.setJpeg(80);
```

### Image Processing Parameters

Modify image processing parameters in the Python script `aoa_esp32_camera.py`:

```python
threshold_img = 120
threshold_ln = 150
minLineLength = 40
maxLineGap = 8
diff1LowerBound = 0.15
diff1UpperBound = 0.25
diff2LowerBound = 0.5
diff2UpperBound = 1.0
```

### Video Recording Settings

Adjust video recording settings in the `.env.h` file:

```cpp
#define RECORD_VIDEO true
#define VIDEO_DURATION 60000
#define MAX_STORAGE_USAGE 80
```

## API Endpoints

The ESP32-CAM firmware provides the following HTTP endpoints:

### `/cam.bmp`
- **Method**: GET
- **Description**: Captures and returns a low-resolution BMP image.
- **Response**: BMP image file

### `/cam-lo.jpg`
- **Method**: GET
- **Description**: Captures and returns a low-resolution JPEG image.
- **Response**: JPEG image file

### `/cam-hi.jpg`
- **Method**: GET
- **Description**: Captures and returns a high-resolution JPEG image.
- **Response**: JPEG image file

### `/cam.jpg`
- **Method**: GET
- **Description**: Redirects to `/cam-hi.jpg` for backward compatibility.
- **Response**: HTTP 302 redirect

### `/cam.mjpeg`
- **Method**: GET
- **Description**: Starts an MJPEG stream for real-time video.
- **Response**: MJPEG stream

### `/video/start`
- **Method**: GET
- **Description**: Starts video recording.
- **Response**: Text confirmation

### `/video/stop`
- **Method**: GET
- **Description**: Stops video recording.
- **Response**: Text confirmation

To access these endpoints, use the IP address of your ESP32-CAM. For example:
```
http://<ESP32-CAM_IP_ADDRESS>/cam-hi.jpg
```

## API Documentation

### ESP32-CAM Firmware API

#### `CameraManager::handleBmp()`
Handles BMP image requests.

#### `CameraManager::handleJpgLo()`
Handles low-resolution JPEG image requests.

#### `CameraManager::handleJpgHi()`
Handles high-resolution JPEG image requests.

#### `CameraManager::handleJpg()`
Redirects to high-resolution JPEG image.

#### `CameraManager::handleMjpeg()`
Handles MJPEG stream requests.

#### `CameraManager::initializeCamera()`
Initializes the camera.

Returns:
- `bool`: True if successful, false otherwise.

#### `CameraManager::handleStartRecording()`
Starts video recording.

#### `CameraManager::handleStopRecording()`
Stops video recording.

#### `CameraManager::update()`
Updates the camera manager, capturing images if recording is active.

#### `NetworkManager::connectToWiFi()`
Connects to WiFi network.

Returns:
- `bool`: True if successful, false otherwise.

#### `StorageManager::initialize()`
Initializes the SD card storage.

Returns:
- `bool`: True if successful, false otherwise.

#### `StorageManager::cleanupStorage()`
Removes oldest recordings if storage usage exceeds the defined limit.

#### `StorageManager::createFolder(const String& folderName)`
Creates a new folder for storing recordings.

Parameters:
- `folderName` (String): Name of the folder to create.

Returns:
- `String`: Path of the created folder, or empty string if failed.

### Python Script API

#### `avg_circles(circles, b)`
Calculates the average center coordinates and radius of detected circles.

Parameters:
- `circles` (numpy.ndarray): Detected circles.
- `b` (int): Number of circles to average.

Returns:
- `tuple`: Average x, y coordinates and radius.

#### `dist_2_pts(x1, y1, x2, y2)`
Calculates the Euclidean distance between two points.

Parameters:
- `x1`, `y1`, `x2`, `y2` (float): Coordinates of two points.

Returns:
- `float`: Distance between the points.

#### `take_measure(threshold_img, threshold_ln, minLineLength, maxLineGap, diff1LowerBound, diff1UpperBound, diff2LowerBound, diff2UpperBound)`
Processes the captured image to extract the gauge reading.

Parameters:
- Various thresholds and bounds for image processing.

Returns:
- `tuple`: Processed images (original and with detected features).

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

- ESP32-CAM library by [yoursunny](https://github.com/yoursunny/esp32cam)

## Support

For issues, feature requests, or questions, please open an issue in the GitHub repository.

---

[AI PROMPT - INCLUDE IN PUBLIC README AND FINAL OUTPUT FILE, USE IT TO EFFORTLESSLY UPDATE README IN FUTURE AND MAINTAIN CONSISTENCY]

You are an AI assistant helping to maintain and update the documentation for the ESP32-CAM Gauge Reader project. When asked to update or modify the README, please consider the following:

1. Maintain the current structure of the README, including sections for Features, Prerequisites, Installation, Usage, Configuration, API Documentation, Contributing, Updating Dependencies, License, Acknowledgements, Support and AI PROMPT.

2. If new features are added, update the Features section accordingly. Ensure that each feature is concisely described and highlights its benefit to the user.

3. When updating the API Documentation, follow this format for each function:
   ```
   #### `function_name(parameters)`
   Brief description of the function's purpose.

   Parameters:
   - `param_name` (type): Description of the parameter.

   Returns:
   - `return_type`: Description of the return value.
   ```

4. If configuration options are changed or added, update the Configuration section with clear examples of how to modify these settings.

5. Keep the Installation and Usage instructions up-to-date with any changes in the setup or running process.

6. If new dependencies are added, update the Prerequisites section and ensure that the installation instructions (including the `requirements.txt` file) are accurate.

7. Maintain a professional and clear tone throughout the document. Use concise language and avoid unnecessary technical jargon.

8. If substantial changes are made to the project structure or functionality, consider updating the project description at the beginning of the README.

9. Ensure that all links (e.g., to the license file or external resources) remain valid and up-to-date.

10. If asked about specific code implementations or technical details not covered in the README, indicate that you would need to review the actual code files to provide accurate information.

By following these guidelines, you'll help maintain a comprehensive and user-friendly README that accurately reflects the current state of the project.