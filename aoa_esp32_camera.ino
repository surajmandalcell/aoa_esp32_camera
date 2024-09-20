#include <esp32cam.h>
#include <WebServer.h>
#include <WiFi.h>
#include ".env.h"

// Constants
const uint16_t SERVER_PORT = 80;
const uint16_t SERIAL_BAUD_RATE = 115200;
const uint16_t WIFI_CONNECTION_RETRY_DELAY = 500; // milliseconds

// Camera resolution settings
const uint16_t LOW_RES_WIDTH = 320;
const uint16_t LOW_RES_HEIGHT = 240;
const uint16_t HIGH_RES_WIDTH = 800;
const uint16_t HIGH_RES_HEIGHT = 600;

// MIME types
const char* MIME_TYPE_BMP = "image/bmp";
const char* MIME_TYPE_JPEG = "image/jpeg";

// HTTP status codes
const int HTTP_OK = 200;
const int HTTP_REDIRECT = 302;
const int HTTP_SERVICE_UNAVAILABLE = 503;

class CameraManager {
private:
    WebServer& server;
    esp32cam::Resolution loRes;
    esp32cam::Resolution hiRes;

    /**
     * @brief Captures and serves a JPEG image.
     * @return void
     */
    void serveJpg() {
        auto frame = esp32cam::capture();
        if (!frame) {
            Serial.println("Error: Capture failed");
            server.send(HTTP_SERVICE_UNAVAILABLE, "", "");
            return;
        }

        Serial.printf("Capture successful: %dx%d %db\n", frame->getWidth(), frame->getHeight(), static_cast<int>(frame->size()));
        server.setContentLength(frame->size());
        server.send(HTTP_OK, MIME_TYPE_JPEG);
        WiFiClient client = server.client();
        frame->writeTo(client);
    }

    /**
     * @brief Changes camera resolution.
     * @param resolution The desired resolution.
     * @return bool True if successful, false otherwise.
     */
    bool changeResolution(const esp32cam::Resolution& resolution) {
        if (!esp32cam::Camera.changeResolution(resolution)) {
            Serial.println("Error: Failed to change resolution");
            return false;
        }
        return true;
    }

public:
    CameraManager(WebServer& srv) : server(srv) {
        loRes = esp32cam::Resolution::find(LOW_RES_WIDTH, LOW_RES_HEIGHT);
        hiRes = esp32cam::Resolution::find(HIGH_RES_WIDTH, HIGH_RES_HEIGHT);
    }

    /**
     * @brief Handles BMP image requests.
     * @return void
     */
    void handleBmp() {
        if (!changeResolution(loRes)) {
            server.send(HTTP_SERVICE_UNAVAILABLE, "", "");
            return;
        }

        auto frame = esp32cam::capture();
        if (!frame) {
            Serial.println("Error: Capture failed");
            server.send(HTTP_SERVICE_UNAVAILABLE, "", "");
            return;
        }

        if (!frame->toBmp()) {
            Serial.println("Error: BMP conversion failed");
            server.send(HTTP_SERVICE_UNAVAILABLE, "", "");
            return;
        }

        Serial.printf("BMP conversion successful: %dx%d %db\n", frame->getWidth(), frame->getHeight(), static_cast<int>(frame->size()));
        server.setContentLength(frame->size());
        server.send(HTTP_OK, MIME_TYPE_BMP);
        WiFiClient client = server.client();
        frame->writeTo(client);
    }

    /**
     * @brief Handles low-resolution JPEG image requests.
     * @return void
     */
    void handleJpgLo() {
        if (changeResolution(loRes)) {
            serveJpg();
        } else {
            server.send(HTTP_SERVICE_UNAVAILABLE, "", "");
        }
    }

    /**
     * @brief Handles high-resolution JPEG image requests.
     * @return void
     */
    void handleJpgHi() {
        if (changeResolution(hiRes)) {
            serveJpg();
        } else {
            server.send(HTTP_SERVICE_UNAVAILABLE, "", "");
        }
    }

    /**
     * @brief Redirects to high-resolution JPEG image.
     * @return void
     */
    void handleJpg() {
        server.sendHeader("Location", "/cam-hi.jpg");
        server.send(HTTP_REDIRECT, "", "");
    }

    /**
     * @brief Handles MJPEG stream requests.
     * @return void
     */
    void handleMjpeg() {
        if (!changeResolution(hiRes)) {
            server.send(HTTP_SERVICE_UNAVAILABLE, "", "");
            return;
        }

        Serial.println("Starting MJPEG stream");
        WiFiClient client = server.client();
        auto startTime = millis();
        int res = esp32cam::Camera.streamMjpeg(client);
        if (res <= 0) {
            Serial.printf("Error: MJPEG streaming failed with code %d\n", res);
            return;
        }
        auto duration = millis() - startTime;
        Serial.printf("MJPEG stream ended: %d frames, %0.2f fps\n", res, 1000.0 * res / duration);
    }

    /**
     * @brief Initializes the camera.
     * @return bool True if successful, false otherwise.
     */
    bool initializeCamera() {
        esp32cam::Config cfg;
        cfg.setPins(esp32cam::pins::AiThinker);
        cfg.setResolution(hiRes);
        cfg.setBufferCount(2);
        cfg.setJpeg(80);
        
        bool success = esp32cam::Camera.begin(cfg);
        Serial.println(success ? "Camera initialized successfully" : "Error: Camera initialization failed");
        return success;
    }
};

class NetworkManager {
public:
    /**
     * @brief Connects to WiFi network.
     * @return bool True if successful, false otherwise.
     */
    static bool connectToWiFi() {
        WiFi.persistent(false);
        WiFi.mode(WIFI_STA);
        WiFi.begin(WIFI_SSID, WIFI_PASS);
        
        unsigned long startAttemptTime = millis();
        
        while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
            delay(WIFI_CONNECTION_RETRY_DELAY);
        }
        
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("Error: Failed to connect to WiFi");
            return false;
        }
        
        printNetworkDetails();
        return true;
    }

private:
    /**
     * @brief Prints network connection details.
     * @return void
     */
    static void printNetworkDetails() {
        Serial.println("\nWiFi connected successfully");
        Serial.print("SSID: ");
        Serial.println(WiFi.SSID());
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        Serial.print("Subnet Mask: ");
        Serial.println(WiFi.subnetMask());
        Serial.print("Gateway IP: ");
        Serial.println(WiFi.gatewayIP());
        Serial.print("DNS IP: ");
        Serial.println(WiFi.dnsIP());
        Serial.print("MAC address: ");
        Serial.println(WiFi.macAddress());
        Serial.print("http://");
        Serial.println(WiFi.localIP());
    }
};

WebServer server(SERVER_PORT);
CameraManager cameraManager(server);

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    Serial.println();

    if (!cameraManager.initializeCamera()) {
        return;
    }

    if (!NetworkManager::connectToWiFi()) {
        return;
    }

    server.on("/cam.bmp", std::bind(&CameraManager::handleBmp, &cameraManager));
    server.on("/cam-lo.jpg", std::bind(&CameraManager::handleJpgLo, &cameraManager));
    server.on("/cam-hi.jpg", std::bind(&CameraManager::handleJpgHi, &cameraManager));
    server.on("/cam.jpg", std::bind(&CameraManager::handleJpg, &cameraManager));
    server.on("/cam.mjpeg", std::bind(&CameraManager::handleMjpeg, &cameraManager));

    server.begin();
    Serial.println("HTTP server started");
}

void loop() {
    server.handleClient();
}