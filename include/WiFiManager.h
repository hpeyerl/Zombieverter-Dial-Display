#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "CANData.h"

class WiFiManager {
public:
    WiFiManager();
    
    bool init(CANDataManager* canMgr);
    void update();
    
    bool isAPMode() { return apMode; }
    bool isConnected() { return WiFi.status() == WL_CONNECTED; }
    String getIPAddress();
    
    void startAP();
    void stopAP();
    
private:
    WebServer* server;
    CANDataManager* canManager;
    bool apMode;
    bool serverRunning;
    
    // Web server handlers
    void handleRoot();
    void handleUpload();
    void handleFileUpload();
    void handleGetParams();
    void handleNotFound();
    
    // Static callbacks for web server
    static void handleRootStatic();
    static void handleUploadStatic();
    static void handleFileUploadStatic();
    static void handleGetParamsStatic();
    static void handleNotFoundStatic();
    
    static WiFiManager* instance;
    
    // HTML pages
    String generateIndexPage();
    String generateUploadPage();
};

#endif // WIFI_MANAGER_H
