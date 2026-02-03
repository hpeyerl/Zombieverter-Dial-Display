#include "WiFiManager.h"
#include "Config.h"
#include <SPIFFS.h>

WiFiManager* WiFiManager::instance = nullptr;

WiFiManager::WiFiManager() 
    : server(nullptr), canManager(nullptr), apMode(false), serverRunning(false) {
    instance = this;
}

bool WiFiManager::init(CANDataManager* canMgr) {
    canManager = canMgr;
    
    // Make sure WiFi is completely off initially (like the working test)
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    delay(500);
    
    #if DEBUG_SERIAL
    Serial.println("WiFi Manager initialized (WiFi off)");
    #endif
    
    // Initialize SPIFFS with format on failure
    if (!SPIFFS.begin(false)) {
        #if DEBUG_SERIAL
        Serial.println("SPIFFS mount failed, formatting...");
        #endif
        
        // Try formatting
        if (!SPIFFS.format()) {
            #if DEBUG_SERIAL
            Serial.println("SPIFFS format failed!");
            #endif
            return false;
        }
        
        // Try mounting again
        if (!SPIFFS.begin(false)) {
            #if DEBUG_SERIAL
            Serial.println("SPIFFS mount failed after format");
            #endif
            return false;
        }
    }
    
    #if DEBUG_SERIAL
    Serial.println("SPIFFS mounted");
    Serial.printf("Total: %d bytes\n", SPIFFS.totalBytes());
    Serial.printf("Used: %d bytes\n", SPIFFS.usedBytes());
    #endif
    
    return true;
}

void WiFiManager::startAP() {
    #if DEBUG_SERIAL
    Serial.println("\n=== WiFiManager::startAP() called ===");
    #endif
    
    if (apMode) {
        #if DEBUG_SERIAL
        Serial.println("WiFi AP already running - skipping");
        #endif
        return;
    }
    
    #if DEBUG_SERIAL
    Serial.println("Step 1: Stopping any existing WiFi...");
    #endif
    
    // Complete shutdown
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    delay(500);
    
    #if DEBUG_SERIAL
    Serial.println("Step 2: Setting WiFi mode to AP...");
    #endif
    
    // Set to AP mode
    WiFi.mode(WIFI_AP);
    delay(500);
    
    #if DEBUG_SERIAL
    Serial.println("Step 3: Starting Access Point...");
    Serial.print("  SSID: ");
    Serial.println(WIFI_AP_SSID);
    Serial.print("  Password: ");
    Serial.println(WIFI_AP_PASSWORD);
    Serial.print("  Channel: ");
    Serial.println(WIFI_AP_CHANNEL);
    #endif
    
    // Start AP with all parameters explicit
    bool apStarted = WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD, WIFI_AP_CHANNEL, false, 4);
    
    #if DEBUG_SERIAL
    Serial.print("Step 4: softAP() returned: ");
    Serial.println(apStarted ? "TRUE" : "FALSE");
    #endif
    
    if (!apStarted) {
        #if DEBUG_SERIAL
        Serial.println("✗✗✗ FAILED to start AP! ✗✗✗");
        Serial.println("Possible causes:");
        Serial.println("  - WiFi hardware issue");
        Serial.println("  - Power supply insufficient");
        Serial.println("  - ESP32 WiFi not initialized");
        #endif
        return;
    }
    
    #if DEBUG_SERIAL
    Serial.println("Step 5: Waiting for AP to fully start...");
    #endif
    delay(1000);
    
    IPAddress IP = WiFi.softAPIP();
    
    #if DEBUG_SERIAL
    Serial.println("Step 6: AP Started Successfully!");
    Serial.println("✓✓✓✓✓✓✓✓✓✓✓✓✓✓✓✓✓✓✓✓✓✓✓✓✓✓✓✓✓✓");
    Serial.print("IP Address: ");
    Serial.println(IP);
    Serial.print("WiFi Mode: ");
    Serial.println(WiFi.getMode());
    Serial.print("MAC Address: ");
    Serial.println(WiFi.softAPmacAddress());
    Serial.println("✓✓✓✓✓✓✓✓✓✓✓✓✓✓✓✓✓✓✓✓✓✓✓✓✓✓✓✓✓✓");
    #endif
    
    apMode = true;
    
    // Start web server
    if (!server) {
        #if DEBUG_SERIAL
        Serial.println("Step 7: Starting web server...");
        #endif
        
        server = new WebServer(WEB_SERVER_PORT);
        
        server->on("/", HTTP_GET, handleRootStatic);
        server->on("/upload", HTTP_GET, handleUploadStatic);
        server->on("/upload", HTTP_POST, []() {
            instance->server->send(200);
        }, handleFileUploadStatic);
        server->on("/params", HTTP_GET, handleGetParamsStatic);
        server->onNotFound(handleNotFoundStatic);
        
        server->begin();
        serverRunning = true;
        
        #if DEBUG_SERIAL
        Serial.print("✓ Web server started on port ");
        Serial.println(WEB_SERVER_PORT);
        Serial.println("=== WiFi AP FULLY READY ===\n");
        #endif
    }
}

void WiFiManager::stopAP() {
    if (!apMode) return;
    
    if (server && serverRunning) {
        server->stop();
        delete server;
        server = nullptr;
        serverRunning = false;
    }
    
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_OFF);
    apMode = false;
    
    #if DEBUG_SERIAL
    Serial.println("WiFi AP stopped");
    #endif
}

void WiFiManager::update() {
    if (server && serverRunning) {
        server->handleClient();
    }
}

String WiFiManager::getIPAddress() {
    if (apMode) {
        return WiFi.softAPIP().toString();
    } else if (isConnected()) {
        return WiFi.localIP().toString();
    }
    return "Not connected";
}

// Web server handlers
void WiFiManager::handleRoot() {
    server->send(200, "text/html", generateIndexPage());
}

void WiFiManager::handleUpload() {
    server->send(200, "text/html", generateUploadPage());
}

void WiFiManager::handleFileUpload() {
    HTTPUpload& upload = server->upload();
    static File uploadFile;
    static size_t uploadSize = 0;
    
    if (upload.status == UPLOAD_FILE_START) {
        #if DEBUG_SERIAL
        Serial.printf("Upload start: %s\n", upload.filename.c_str());
        #endif
        
        uploadSize = 0;
        
        // Delete old file if exists
        if (SPIFFS.exists("/params.json")) {
            SPIFFS.remove("/params.json");
        }
        
        // Open file for writing
        uploadFile = SPIFFS.open("/params.json", "w");
        if (!uploadFile) {
            #if DEBUG_SERIAL
            Serial.println("Failed to open file for writing");
            #endif
        }
        
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        // Check size limit
        if (uploadSize + upload.currentSize > MAX_JSON_SIZE) {
            #if DEBUG_SERIAL
            Serial.println("File too large!");
            #endif
            if (uploadFile) {
                uploadFile.close();
                SPIFFS.remove("/params.json");
            }
            server->sendHeader("Location", "/?error=2"); // Size error
            server->send(303);
            return;
        }
        
        // Write chunk to file
        if (uploadFile) {
            uploadFile.write(upload.buf, upload.currentSize);
            uploadSize += upload.currentSize;
        }
        
    } else if (upload.status == UPLOAD_FILE_END) {
        if (uploadFile) {
            uploadFile.close();
        }
        
        #if DEBUG_SERIAL
        Serial.printf("Upload complete: %d bytes\n", uploadSize);
        #endif
        
        // Validate and load parameters
        File file = SPIFFS.open("/params.json", "r");
        if (file) {
            String jsonContent = file.readString();
            file.close();
            
            // Basic JSON validation
            if (jsonContent.length() == 0 || jsonContent.length() > MAX_JSON_SIZE) {
                #if DEBUG_SERIAL
                Serial.println("Invalid file size");
                #endif
                SPIFFS.remove("/params.json");
                server->sendHeader("Location", "/?error=2");
                server->send(303);
                return;
            }
            
            // Try to parse
            if (canManager->loadParametersFromJSON(jsonContent.c_str())) {
                #if DEBUG_SERIAL
                Serial.println("Parameters loaded successfully");
                #endif
                
                server->sendHeader("Location", "/?success=1");
                server->send(303);
            } else {
                #if DEBUG_SERIAL
                Serial.println("Failed to parse parameters");
                #endif
                
                SPIFFS.remove("/params.json");
                server->sendHeader("Location", "/?error=1");
                server->send(303);
            }
        } else {
            server->sendHeader("Location", "/?error=3");
            server->send(303);
        }
    } else if (upload.status == UPLOAD_FILE_ABORTED) {
        #if DEBUG_SERIAL
        Serial.println("Upload aborted");
        #endif
        if (uploadFile) {
            uploadFile.close();
        }
        SPIFFS.remove("/params.json");
    }
}

void WiFiManager::handleGetParams() {
    File file = SPIFFS.open("/params.json", "r");
    if (file) {
        server->streamFile(file, "application/json");
        file.close();
    } else {
        server->send(404, "text/plain", "No parameters file found");
    }
}

void WiFiManager::handleNotFound() {
    server->send(404, "text/plain", "404: Not found");
}

// Static callbacks
void WiFiManager::handleRootStatic() {
    if (instance) instance->handleRoot();
}

void WiFiManager::handleUploadStatic() {
    if (instance) instance->handleUpload();
}

void WiFiManager::handleFileUploadStatic() {
    if (instance) instance->handleFileUpload();
}

void WiFiManager::handleGetParamsStatic() {
    if (instance) instance->handleGetParams();
}

void WiFiManager::handleNotFoundStatic() {
    if (instance) instance->handleNotFound();
}

// HTML pages
String WiFiManager::generateIndexPage() {
    String html = R"(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ZombieVerter Display</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            max-width: 600px;
            margin: 50px auto;
            padding: 20px;
            background: #1a1a1a;
            color: #fff;
        }
        h1 {
            color: #4CAF50;
            text-align: center;
        }
        .card {
            background: #2a2a2a;
            border-radius: 10px;
            padding: 20px;
            margin: 20px 0;
            box-shadow: 0 4px 6px rgba(0,0,0,0.3);
        }
        .button {
            display: inline-block;
            background: #4CAF50;
            color: white;
            padding: 12px 24px;
            border-radius: 5px;
            text-decoration: none;
            margin: 5px;
            text-align: center;
        }
        .button:hover {
            background: #45a049;
        }
        .info {
            background: #333;
            padding: 10px;
            border-radius: 5px;
            margin: 10px 0;
        }
        .success {
            background: #4CAF50;
            padding: 10px;
            border-radius: 5px;
            margin: 10px 0;
        }
        .error {
            background: #f44336;
            padding: 10px;
            border-radius: 5px;
            margin: 10px 0;
        }
    </style>
</head>
<body>
    <h1>⚡ ZombieVerter Display</h1>
    <div class="card">
        <h2>Configuration Portal</h2>
        <div class="info">
            <strong>Device:</strong> M5Stack Dial<br>
            <strong>IP Address:</strong> )";
    
    html += getIPAddress();
    html += R"(<br>
            <strong>Status:</strong> WiFi AP Mode
        </div>
    </div>
)";

    // Show success/error messages
    if (server->hasArg("success")) {
        html += R"(
    <div class="success">
        ✓ Parameters uploaded and loaded successfully!
    </div>
)";
    }
    
    if (server->hasArg("error")) {
        html += R"(
    <div class="error">
        ✗ Failed to parse parameters file. Please check JSON format.
    </div>
)";
    }
    
    html += R"(
    <div class="card">
        <h2>Upload Parameters</h2>
        <p>Upload your ZombieVerter parameters JSON file:</p>
        <a href="/upload" class="button">📤 Upload params.json</a>
    </div>
    
    <div class="card">
        <h2>Download Current Parameters</h2>
        <p>Download the currently loaded parameters:</p>
        <a href="/params" class="button" download="params.json">📥 Download params.json</a>
    </div>
    
    <div class="card">
        <h2>Instructions</h2>
        <ol>
            <li>Create or obtain your ZombieVerter parameters JSON file</li>
            <li>Click "Upload params.json" above</li>
            <li>Select your file and upload</li>
            <li>The display will reload with your parameters</li>
            <li>Turn off WiFi mode by long-pressing the button</li>
        </ol>
    </div>
</body>
</html>
)";
    
    return html;
}

String WiFiManager::generateUploadPage() {
    String html = R"(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Upload Parameters</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            max-width: 600px;
            margin: 50px auto;
            padding: 20px;
            background: #1a1a1a;
            color: #fff;
        }
        h1 {
            color: #4CAF50;
            text-align: center;
        }
        .card {
            background: #2a2a2a;
            border-radius: 10px;
            padding: 20px;
            margin: 20px 0;
            box-shadow: 0 4px 6px rgba(0,0,0,0.3);
        }
        input[type="file"] {
            display: block;
            width: 100%;
            padding: 10px;
            margin: 10px 0;
            background: #333;
            border: 2px solid #4CAF50;
            border-radius: 5px;
            color: #fff;
        }
        input[type="submit"] {
            background: #4CAF50;
            color: white;
            padding: 12px 24px;
            border: none;
            border-radius: 5px;
            cursor: pointer;
            font-size: 16px;
            width: 100%;
        }
        input[type="submit"]:hover {
            background: #45a049;
        }
        .button {
            display: inline-block;
            background: #666;
            color: white;
            padding: 12px 24px;
            border-radius: 5px;
            text-decoration: none;
            margin: 5px 0;
        }
        .button:hover {
            background: #555;
        }
    </style>
</head>
<body>
    <h1>📤 Upload Parameters</h1>
    <div class="card">
        <h2>Select params.json File</h2>
        <form method="POST" action="/upload" enctype="multipart/form-data">
            <input type="file" name="file" accept=".json" required>
            <input type="submit" value="Upload">
        </form>
    </div>
    <div class="card">
        <a href="/" class="button">← Back to Home</a>
    </div>
</body>
</html>
)";
    
    return html;
}

