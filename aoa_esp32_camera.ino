#include <esp32cam.h>
#include <WebServer.h>
#include <WiFi.h>
#include <FS.h>
#include <SD_MMC.h>
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
const int HTTP_BAD_REQUEST = 400;
const int HTTP_SERVICE_UNAVAILABLE = 503;

class StorageManager {
public:
    static bool initialize() {
        if (!SD_MMC.begin()) {
            Serial.println("SD Card Mount Failed");
            return false;
        }
        return true;
    }

    static void cleanupStorage() {
        if (getStorageUsage() > MAX_STORAGE_USAGE) {
            removeOldestFolder();
        }
    }

    static String createFolder(const String& folderName) {
        String path = "/" + folderName;
        if (SD_MMC.mkdir(path)) {
            return path;
        }
        return "";
    }

private:
    static float getStorageUsage() {
        uint64_t totalBytes = SD_MMC.totalBytes();
        uint64_t usedBytes = SD_MMC.usedBytes();
        return (usedBytes * 100.0) / totalBytes;
    }

    static void removeOldestFolder() {
        File root = SD_MMC.open("/");
        if (!root || !root.isDirectory()) {
            Serial.println("Failed to open root directory");
            return;
        }

        String oldestFolder;
        unsigned long oldestTime = ULONG_MAX;

        File file = root.openNextFile();
        while (file) {
            if (file.isDirectory()) {
                String folderName = String(file.name());
                unsigned long startTime = extractStartTime(folderName);
                if (startTime < oldestTime) {
                    oldestTime = startTime;
                    oldestFolder = folderName;
                }
            }
            file = root.openNextFile();
        }

        if (oldestFolder.length() > 0) {
            removeDirectory("/" + oldestFolder);
            Serial.println("Removed oldest folder: " + oldestFolder);
        }
    }

    static unsigned long extractStartTime(const String& folderName) {
        int underscoreIndex = folderName.indexOf('_');
        if (underscoreIndex != -1) {
            return folderName.substring(0, underscoreIndex).toInt();
        }
        return 0;
    }

    static void removeDirectory(const String& path) {
        File dir = SD_MMC.open(path);
        if (!dir.isDirectory()) {
            return;
        }

        File file = dir.openNextFile();
        while (file) {
            if (file.isDirectory()) {
                removeDirectory(path + "/" + String(file.name()));
            } else {
                SD_MMC.remove(path + "/" + String(file.name()));
            }
            file = dir.openNextFile();
        }
        SD_MMC.rmdir(path);
    }
};

class ImageRecorder {
private:
    String currentFolder;
    bool isRecording;
    unsigned long startTimestamp;

public:
    ImageRecorder() : isRecording(false) {}

    bool startRecording() {
        if (isRecording) return false;

        startTimestamp = millis();
        currentFolder = generateFolderName(startTimestamp);
        String path = StorageManager::createFolder(currentFolder);
        if (path.isEmpty()) {
            Serial.println("Failed to create folder");
            return false;
        }

        isRecording = true;
        Serial.println("Started recording in folder: " + currentFolder);
        return true;
    }

    bool stopRecording() {
        if (!isRecording) return false;

        unsigned long stopTimestamp = millis();
        String newFolderName = generateFolderName(startTimestamp, stopTimestamp);
        if (SD_MMC.rename("/" + currentFolder, "/" + newFolderName)) {
            Serial.println("Stopped recording. Folder renamed to: " + newFolderName);
        } else {
            Serial.println("Failed to rename folder");
        }

        isRecording = false;
        StorageManager::cleanupStorage();
        return true;
    }

    bool captureImage(esp32cam::Frame* frame) {
        if (!isRecording || !frame) return false;

        String fileName = "/" + currentFolder + "/" + String(millis()) + ".jpg";
        File file = SD_MMC.open(fileName, FILE_WRITE);
        if (!file) {
            Serial.println("Failed to create image file");
            return false;
        }

        if (file.write(frame->data(), frame->size()) != frame->size()) {
            Serial.println("Failed to write image data");
            file.close();
            return false;
        }

        file.close();
        return true;
    }

    bool isCurrentlyRecording() const {
        return isRecording;
    }

private:
    String generateFolderName(unsigned long start, unsigned long stop = 0) {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%lu", start);
        String folderName = String(buffer);
        if (stop > 0) {
            snprintf(buffer, sizeof(buffer), "_%lu", stop);
            folderName += String(buffer);
        }
        return folderName;
    }
};

class CameraManager {
private:
    WebServer& server;
    esp32cam::Resolution loRes;
    esp32cam::Resolution hiRes;
    ImageRecorder imageRecorder;

    void serveJpg() {
        auto frame = esp32cam::capture();
        if (!frame) {
            Serial.println("Capture failed");
            server.send(HTTP_SERVICE_UNAVAILABLE, "", "");
            return;
        }
        Serial.printf("JPEG: %dx%d %db\n", frame->getWidth(), frame->getHeight(),
                      static_cast<int>(frame->size()));

        server.setContentLength(frame->size());
        server.send(HTTP_OK, MIME_TYPE_JPEG);
        WiFiClient client = server.client();
        frame->writeTo(client);
    }

public:
    CameraManager(WebServer& srv) : server(srv) {
        loRes = esp32cam::Resolution::find(LOW_RES_WIDTH, LOW_RES_HEIGHT);
        hiRes = esp32cam::Resolution::find(HIGH_RES_WIDTH, HIGH_RES_HEIGHT);
    }

    void handleBmp() {
        if (!esp32cam::Camera.changeResolution(loRes)) {
            Serial.println("SET-LO-RES FAIL");
        }

        auto frame = esp32cam::capture();
        if (frame == nullptr) {
            Serial.println("CAPTURE FAIL");
            server.send(HTTP_SERVICE_UNAVAILABLE, "", "");
            return;
        }
        Serial.printf("BMP: %dx%d %db\n", frame->getWidth(), frame->getHeight(),
                      static_cast<int>(frame->size()));

        if (!frame->toBmp()) {
            Serial.println("CONVERT FAIL");
            server.send(HTTP_SERVICE_UNAVAILABLE, "", "");
            return;
        }
        server.setContentLength(frame->size());
        server.send(HTTP_OK, MIME_TYPE_BMP);
        WiFiClient client = server.client();
        frame->writeTo(client);
    }

    void handleJpgLo() {
        if (!esp32cam::Camera.changeResolution(loRes)) {
            Serial.println("SET-LO-RES FAIL");
        }
        serveJpg();
    }

    void handleJpgHi() {
        if (!esp32cam::Camera.changeResolution(hiRes)) {
            Serial.println("SET-HI-RES FAIL");
        }
        serveJpg();
    }

    void handleJpg() {
        server.sendHeader("Location", "/cam-hi.jpg");
        server.send(HTTP_REDIRECT, "", "");
    }

    void handleMjpeg() {
        if (!esp32cam::Camera.changeResolution(hiRes)) {
            Serial.println("SET-HI-RES FAIL");
        }

        Serial.println("STREAM BEGIN");
        WiFiClient client = server.client();
        auto startTime = millis();
        int res = esp32cam::Camera.streamMjpeg(client);
        if (res <= 0) {
            Serial.printf("STREAM ERROR %d\n", res);
            return;
        }
        auto duration = millis() - startTime;
        Serial.printf("STREAM END %dfrm %0.2ffps\n", res, 1000.0 * res / duration);
    }

    void update() {
        if (imageRecorder.isCurrentlyRecording()) {
            auto frame = esp32cam::capture();
            if (frame) {
                imageRecorder.captureImage(frame.get());
            }
        }
    }

    void handleStartRecording() {
        if (imageRecorder.startRecording()) {
            server.send(HTTP_OK, "text/plain", "Recording started");
        } else {
            server.send(HTTP_BAD_REQUEST, "text/plain", "Failed to start recording");
        }
    }

    void handleStopRecording() {
        if (imageRecorder.stopRecording()) {
            server.send(HTTP_OK, "text/plain", "Recording stopped");
        } else {
            server.send(HTTP_BAD_REQUEST, "text/plain", "Failed to stop recording");
        }
    }

    bool initializeCamera() {
        esp32cam::Config cfg;
        cfg.setPins(esp32cam::pins::AiThinker);
        cfg.setResolution(hiRes);
        cfg.setBufferCount(2);
        cfg.setJpeg(80);
        
        bool success = esp32cam::Camera.begin(cfg);
        Serial.println(success ? "Camera initialized successfully" : "Error: Camera initialization failed");

        if (success) {
            StorageManager::initialize();
        }

        return success;
    }
};

class NetworkManager {
public:
    static bool connectToWiFi() {
        WiFi.persistent(false);
        WiFi.mode(WIFI_STA);
        WiFi.begin(WIFI_SSID, WIFI_PASS);
        
        unsigned long startAttemptTime = millis();
        
        while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
            delay(WIFI_CONNECTION_RETRY_DELAY);
        }
        
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("Failed to connect to WiFi");
            return false;
        }
        
        Serial.println("WiFi connected");
        Serial.print("Camera Ready! Use 'http://");
        Serial.print(WiFi.localIP());
        Serial.println("' to connect");
        return true;
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

    server.on("/cam.bmp", HTTP_GET, std::bind(&CameraManager::handleBmp, &cameraManager));
    server.on("/cam-lo.jpg", HTTP_GET, std::bind(&CameraManager::handleJpgLo, &cameraManager));
    server.on("/cam-hi.jpg", HTTP_GET, std::bind(&CameraManager::handleJpgHi, &cameraManager));
    server.on("/cam.jpg", HTTP_GET, std::bind(&CameraManager::handleJpg, &cameraManager));
    server.on("/cam.mjpeg", HTTP_GET, std::bind(&CameraManager::handleMjpeg, &cameraManager));
    server.on("/video/start", HTTP_GET, std::bind(&CameraManager::handleStartRecording, &cameraManager));
    server.on("/video/stop", HTTP_GET, std::bind(&CameraManager::handleStopRecording, &cameraManager));

    server.begin();
    Serial.println("HTTP server started");
}

void loop() {
    server.handleClient();
    cameraManager.update();
}