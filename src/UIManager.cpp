#include "UIManager.h"
#include "Config.h"

UIManager::UIManager() 
    : canManager(nullptr), currentScreen(SCREEN_SPLASH),
      editing(false), editingParamIndex(0), editingValue(0),
      lastUpdateTime(0), bmsPage(0), bmsMaxPages(1) {
}

bool UIManager::init(CANDataManager* canMgr) {
    canManager = canMgr;
    currentScreen = SCREEN_SPLASH;
    showSplash();
    return true;
}

void UIManager::update() {
    if (millis() - lastUpdateTime < 100) return;
    lastUpdateTime = millis();
    
    switch (currentScreen) {
        case SCREEN_SPLASH:
            // Splash is static, drawn once
            break;
        case SCREEN_DASHBOARD:
            drawDashboard();
            break;
        case SCREEN_POWER:
            drawPower();
            break;
        case SCREEN_TEMPERATURE:
            drawTemperature();
            break;
        case SCREEN_BATTERY:
            drawBattery();
            break;
        case SCREEN_BMS:
            drawBMS();
            break;
        case SCREEN_GEAR:
            drawGear();
            break;
        case SCREEN_MOTOR:
            drawMotor();
            break;
        case SCREEN_REGEN:
            drawRegen();
            break;
        case SCREEN_WIFI:
            drawWiFi();
            break;
        case SCREEN_SETTINGS:
            drawSettings();
            break;
        default:
            break;
    }
}

void UIManager::setScreen(ScreenID screen) {
    if (screen >= SCREEN_COUNT) return;
    
    // Reset BMS page when leaving BMS screen
    if (currentScreen == SCREEN_BMS && screen != SCREEN_BMS) {
        bmsPage = 0;
    }
    
    currentScreen = screen;
    editing = false;
    clearScreen();
}

void UIManager::nextScreen() {
    ScreenID next = (ScreenID)((currentScreen + 1) % SCREEN_COUNT);
    if (next == SCREEN_SPLASH) next = SCREEN_DASHBOARD;
    setScreen(next);
}

void UIManager::previousScreen() {
    ScreenID prev = (ScreenID)(currentScreen - 1);
    if (prev < SCREEN_DASHBOARD) prev = (ScreenID)(SCREEN_COUNT - 1);
    setScreen(prev);
}

void UIManager::incrementValue() {
    if (editing) {
        editingValue += 1;
    }
}

void UIManager::decrementValue() {
    if (editing) {
        editingValue -= 1;
    }
}

void UIManager::selectValue() {
    if (editing) {
        // Save the value
        CANParameter* param = canManager->getParameterByIndex(editingParamIndex);
        if (param) {
            canManager->setParameter(param->id, editingValue);
        }
        editing = false;
    }
}

void UIManager::nextBMSPage() {
    if (currentScreen == SCREEN_BMS) {
        bmsPage++;
        if (bmsPage >= bmsMaxPages) {
            bmsPage = 0;
        }
        clearScreen();
    }
}

void UIManager::previousBMSPage() {
    if (currentScreen == SCREEN_BMS) {
        if (bmsPage == 0) {
            bmsPage = bmsMaxPages - 1;
        } else {
            bmsPage--;
        }
        clearScreen();
    }
}

void UIManager::showSplash() {
    clearScreen();
    
    M5.Display.setTextDatum(middle_center);
    
    // Title - larger and bolder
    M5.Display.setTextColor(TFT_GREEN, TFT_BLACK);
    M5.Display.setTextSize(3);
    M5.Display.drawString("ZombieVerter", SCREEN_CENTER_X, SCREEN_CENTER_Y - 40);
    
    // Subtitle
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.setTextSize(2);
    M5.Display.drawString("Display", SCREEN_CENTER_X, SCREEN_CENTER_Y);
    
    // Version/Edition - smaller
    M5.Display.setTextColor(TFT_CYAN, TFT_BLACK);
    M5.Display.setTextSize(1);
    M5.Display.drawString("M5Stack Dial Edition", SCREEN_CENTER_X, SCREEN_CENTER_Y + 40);
    M5.Display.drawString("v1.0", SCREEN_CENTER_X, SCREEN_CENTER_Y + 60);
}

void UIManager::showConnectionStatus(bool connected) {
    int16_t y = SCREEN_HEIGHT - 20;
    M5.Display.fillRect(0, y, SCREEN_WIDTH, 20, TFT_BLACK);
    
    M5.Display.setTextColor(connected ? TFT_GREEN : TFT_RED);
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(bottom_center);
    M5.Display.drawString(connected ? "CAN Connected" : "CAN Disconnected", 
                         SCREEN_CENTER_X, SCREEN_HEIGHT - 5);
}

void UIManager::drawDashboard() {
    clearScreen();
    
    M5.Display.setTextDatum(middle_center);
    
    // Title
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(TFT_CYAN, TFT_BLACK);
    M5.Display.drawString("CAN Status", SCREEN_CENTER_X, 20);
    
    int y = 60;
    
    // Connection status
    M5.Display.setTextSize(3);
    if (canManager->isConnected()) {
        M5.Display.setTextColor(TFT_GREEN, TFT_BLACK);
        M5.Display.drawString("RECEIVING", SCREEN_CENTER_X, y);
        y += 40;
        
        M5.Display.setTextSize(2);
        M5.Display.setTextColor(TFT_YELLOW, TFT_BLACK);
        M5.Display.drawString("CAN Messages!", SCREEN_CENTER_X, y);
    } else {
        M5.Display.setTextColor(TFT_RED, TFT_BLACK);
        M5.Display.drawString("NO CAN", SCREEN_CENTER_X, y);
    }
    
    y = 140;
    
    // Check for parameter updates
    int updatedCount = 0;
    for (int i = 0; i < canManager->getParameterCount(); i++) {
        CANParameter* param = canManager->getParameterByIndex(i);
        if (param && (millis() - param->lastUpdateTime) < 5000) {
            updatedCount++;
        }
    }
    
    M5.Display.setTextSize(2);
    if (updatedCount > 0) {
        M5.Display.setTextColor(TFT_GREEN, TFT_BLACK);
        char buf[32];
        snprintf(buf, sizeof(buf), "%d params OK", updatedCount);
        M5.Display.drawString(buf, SCREEN_CENTER_X, y);
    } else {
        M5.Display.setTextColor(TFT_RED, TFT_BLACK);
        M5.Display.drawString("No param data", SCREEN_CENTER_X, y);
        y += 30;
        
        M5.Display.setTextSize(1);
        M5.Display.setTextColor(TFT_YELLOW, TFT_BLACK);
        M5.Display.drawString("Need: 0x183 or 0x283", SCREEN_CENTER_X, y);
        y += 15;
        M5.Display.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
        M5.Display.drawString("(Node 3 PDO messages)", SCREEN_CENTER_X, y);
    }
}

void UIManager::updateLastCANID(uint32_t canID) {
    // Not used in this simplified version
}

void UIManager::drawPower() {
    clearScreen();
    
    CANParameter* power = canManager->getParameter(2);
    CANParameter* current = canManager->getParameter(4);
    
    // Title
    M5.Display.setTextDatum(middle_center);
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_DARKGREY, TFT_BLACK);
    M5.Display.drawString("Power", SCREEN_CENTER_X, 15);
    
    if (power) {
        float percent = (float)(power->getValueAsInt() - power->minValue) / 
                       (power->maxValue - power->minValue);
        
        // Extract just the number
        char valueStr[32];
        power->toString(valueStr, sizeof(valueStr));
        char numStr[16] = "";
        int i = 0, j = 0;
        while (valueStr[i] && j < 15) {
            if ((valueStr[i] >= '0' && valueStr[i] <= '9') || valueStr[i] == '.' || valueStr[i] == '-') {
                numStr[j++] = valueStr[i];
            } else if (valueStr[i] == ' ') {
                break;
            }
            i++;
        }
        numStr[j] = '\0';
        
        // Large value in center
        M5.Display.setTextSize(5);
        M5.Display.setTextColor(TFT_ORANGE, TFT_BLACK);
        M5.Display.drawString(numStr, SCREEN_CENTER_X, SCREEN_CENTER_Y - 20);
        
        // Unit below
        M5.Display.setTextSize(3);
        M5.Display.setTextColor(TFT_YELLOW, TFT_BLACK);
        M5.Display.drawString(power->unit, SCREEN_CENTER_X, SCREEN_CENTER_Y + 25);
        
        // Draw ring gauge around edge
        drawRingGaugeSimple(percent, TFT_ORANGE);
    }
    
    // Current at bottom
    if (current) {
        char valueStr[32];
        current->toString(valueStr, sizeof(valueStr));
        M5.Display.setTextSize(2);
        M5.Display.setTextColor(TFT_CYAN, TFT_BLACK);
        M5.Display.setTextDatum(bottom_center);
        M5.Display.drawString(valueStr, SCREEN_CENTER_X, SCREEN_HEIGHT - 10);
    }
    
    // Connection status
    M5.Display.setTextDatum(top_right);
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(canManager->isConnected() ? TFT_GREEN : TFT_RED, TFT_BLACK);
    M5.Display.drawString(canManager->isConnected() ? "CAN" : "---", SCREEN_WIDTH - 5, 5);
}

void UIManager::drawTemperature() {
    clearScreen();
    
    CANParameter* motorTemp = canManager->getParameter(5);
    CANParameter* inverterTemp = canManager->getParameter(6);
    
    // Title
    M5.Display.setTextDatum(middle_center);
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_DARKGREY, TFT_BLACK);
    M5.Display.drawString("Temperature", SCREEN_CENTER_X, 15);
    
    // Motor temp - top half
    if (motorTemp) {
        char valueStr[32];
        motorTemp->toString(valueStr, sizeof(valueStr));
        
        // Extract number
        char numStr[16] = "";
        int i = 0, j = 0;
        while (valueStr[i] && j < 15) {
            if ((valueStr[i] >= '0' && valueStr[i] <= '9') || valueStr[i] == '-') {
                numStr[j++] = valueStr[i];
            } else if (valueStr[i] == ' ') {
                break;
            }
            i++;
        }
        numStr[j] = '\0';
        
        M5.Display.setTextSize(1);
        M5.Display.setTextColor(TFT_YELLOW, TFT_BLACK);
        M5.Display.drawString("Motor", SCREEN_CENTER_X, SCREEN_CENTER_Y - 60);
        
        M5.Display.setTextSize(5);
        M5.Display.setTextColor(TFT_ORANGE, TFT_BLACK);
        M5.Display.drawString(numStr, SCREEN_CENTER_X, SCREEN_CENTER_Y - 30);
        
        M5.Display.setTextSize(2);
        M5.Display.setTextColor(TFT_CYAN, TFT_BLACK);
        M5.Display.drawString(motorTemp->unit, SCREEN_CENTER_X, SCREEN_CENTER_Y + 5);
    }
    
    // Inverter temp - bottom
    if (inverterTemp) {
        char valueStr[32];
        inverterTemp->toString(valueStr, sizeof(valueStr));
        
        M5.Display.setTextDatum(bottom_center);
        M5.Display.setTextSize(1);
        M5.Display.setTextColor(TFT_CYAN, TFT_BLACK);
        M5.Display.drawString("Inverter:", SCREEN_CENTER_X - 30, SCREEN_HEIGHT - 10);
        
        M5.Display.setTextSize(2);
        M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
        M5.Display.drawString(valueStr, SCREEN_CENTER_X + 30, SCREEN_HEIGHT - 10);
    }
    
    // Connection status
    M5.Display.setTextDatum(top_right);
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(canManager->isConnected() ? TFT_GREEN : TFT_RED, TFT_BLACK);
    M5.Display.drawString(canManager->isConnected() ? "CAN" : "---", SCREEN_WIDTH - 5, 5);
}

void UIManager::drawBattery() {
    clearScreen();
    
    CANParameter* soc = canManager->getParameter(7);
    CANParameter* voltage = canManager->getParameter(3);
    
    // Title
    M5.Display.setTextDatum(middle_center);
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_DARKGREY, TFT_BLACK);
    M5.Display.drawString("Battery", SCREEN_CENTER_X, 15);
    
    if (soc) {
        float percent = (float)soc->getValueAsInt() / 100.0f;
        
        // Extract number
        char valueStr[32];
        soc->toString(valueStr, sizeof(valueStr));
        char numStr[16] = "";
        int i = 0, j = 0;
        while (valueStr[i] && j < 15) {
            if ((valueStr[i] >= '0' && valueStr[i] <= '9') || valueStr[i] == '.') {
                numStr[j++] = valueStr[i];
            } else if (valueStr[i] == ' ') {
                break;
            }
            i++;
        }
        numStr[j] = '\0';
        
        // SOC label
        M5.Display.setTextSize(1);
        M5.Display.setTextColor(TFT_YELLOW, TFT_BLACK);
        M5.Display.drawString("State of Charge", SCREEN_CENTER_X, SCREEN_CENTER_Y - 60);
        
        // Large percentage
        M5.Display.setTextSize(6);
        M5.Display.setTextColor(TFT_GREEN, TFT_BLACK);
        M5.Display.drawString(numStr, SCREEN_CENTER_X, SCREEN_CENTER_Y - 20);
        
        // % symbol
        M5.Display.setTextSize(3);
        M5.Display.setTextColor(TFT_CYAN, TFT_BLACK);
        M5.Display.drawString("%", SCREEN_CENTER_X, SCREEN_CENTER_Y + 25);
        
        // Draw ring gauge
        drawRingGaugeSimple(percent, TFT_GREEN);
    }
    
    // Voltage at bottom
    if (voltage) {
        char valueStr[32];
        voltage->toString(valueStr, sizeof(valueStr));
        M5.Display.setTextSize(2);
        M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
        M5.Display.setTextDatum(bottom_center);
        M5.Display.drawString(valueStr, SCREEN_CENTER_X, SCREEN_HEIGHT - 10);
    }
    
    // Connection status
    M5.Display.setTextDatum(top_right);
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(canManager->isConnected() ? TFT_GREEN : TFT_RED, TFT_BLACK);
    M5.Display.drawString(canManager->isConnected() ? "CAN" : "---", SCREEN_WIDTH - 5, 5);
}

void UIManager::drawBMS() {
    clearScreen();
    
    // Parse Victron/REC BMS data from 0x373
    // This message contains min/max cell voltages and temps
    // We'll store this data when we receive it
    
    // Title
    M5.Display.setTextDatum(middle_center);
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(TFT_CYAN, TFT_BLACK);
    M5.Display.drawString("BMS", SCREEN_CENTER_X, 20);
    
    int yPos = 55;
    int lineHeight = 45;
    
    // Get BMS summary parameters
    CANParameter* cellVoltMax = canManager->getParameter(20);
    CANParameter* cellVoltMin = canManager->getParameter(21);
    CANParameter* cellTempMax = canManager->getParameter(24);
    
    // Max Cell Voltage
    M5.Display.setTextDatum(middle_left);
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(TFT_GREEN, TFT_BLACK);
    M5.Display.drawString("Max", 10, yPos);
    
    M5.Display.setTextDatum(middle_right);
    M5.Display.setTextSize(3);
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    if (cellVoltMax && cellVoltMax->lastUpdateTime > 0) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%.3f V", cellVoltMax->getValueAsInt() / 1000.0);
        M5.Display.drawString(buf, SCREEN_WIDTH - 10, yPos);
    } else {
        M5.Display.drawString("--- V", SCREEN_WIDTH - 10, yPos);
    }
    yPos += lineHeight;
    
    // Min Cell Voltage
    M5.Display.setTextDatum(middle_left);
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(TFT_YELLOW, TFT_BLACK);
    M5.Display.drawString("Min", 10, yPos);
    
    M5.Display.setTextDatum(middle_right);
    M5.Display.setTextSize(3);
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    if (cellVoltMin && cellVoltMin->lastUpdateTime > 0) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%.3f V", cellVoltMin->getValueAsInt() / 1000.0);
        M5.Display.drawString(buf, SCREEN_WIDTH - 10, yPos);
    } else {
        M5.Display.drawString("--- V", SCREEN_WIDTH - 10, yPos);
    }
    yPos += lineHeight;
    
    // Delta (calculated)
    M5.Display.setTextDatum(middle_left);
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(TFT_ORANGE, TFT_BLACK);
    M5.Display.drawString("Delta", 10, yPos);
    
    M5.Display.setTextDatum(middle_right);
    M5.Display.setTextSize(3);
    if (cellVoltMax && cellVoltMin && 
        cellVoltMax->lastUpdateTime > 0 && cellVoltMin->lastUpdateTime > 0) {
        int32_t delta = cellVoltMax->getValueAsInt() - cellVoltMin->getValueAsInt();
        char buf[16];
        snprintf(buf, sizeof(buf), "%d mV", delta);
        
        // Color code: red if >100mV
        M5.Display.setTextColor(delta > 100 ? TFT_RED : TFT_WHITE, TFT_BLACK);
        M5.Display.drawString(buf, SCREEN_WIDTH - 10, yPos);
    } else {
        M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
        M5.Display.drawString("--- mV", SCREEN_WIDTH - 10, yPos);
    }
    yPos += lineHeight;
    
    // Max Temperature
    M5.Display.setTextDatum(middle_left);
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(TFT_MAGENTA, TFT_BLACK);
    M5.Display.drawString("Temp", 10, yPos);
    
    M5.Display.setTextDatum(middle_right);
    M5.Display.setTextSize(3);
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    if (cellTempMax && cellTempMax->lastUpdateTime > 0) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%d C", cellTempMax->getValueAsInt());
        M5.Display.drawString(buf, SCREEN_WIDTH - 10, yPos);
    } else {
        M5.Display.drawString("--- C", SCREEN_WIDTH - 10, yPos);
    }
}

void UIManager::drawBMSSummary() {
    // Not used - keeping for compatibility
    drawBMS();
}

void UIManager::drawBMSModule(uint8_t moduleIndex) {
    // Not used - keeping for compatibility
    drawBMS();
}


void UIManager::drawWiFi() {
    clearScreen();
    
    M5.Display.setTextDatum(middle_center);
    
    // Title - larger
    M5.Display.setTextSize(3);
    M5.Display.setTextColor(TFT_CYAN, TFT_BLACK);
    M5.Display.drawString("WiFi", SCREEN_CENTER_X, 30);
    
    // Status indicator
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(TFT_GREEN, TFT_BLACK);
    M5.Display.drawString("ACTIVE", SCREEN_CENTER_X, 60);
    
    // SSID - very clear and large
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_YELLOW, TFT_BLACK);
    M5.Display.drawString("Connect to:", SCREEN_CENTER_X, 90);
    
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.drawString("ZombieVerter", SCREEN_CENTER_X, 110);
    M5.Display.drawString("-Display", SCREEN_CENTER_X, 130);
    
    // Password - clear
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_ORANGE, TFT_BLACK);
    M5.Display.drawString("Password:", SCREEN_CENTER_X, 155);
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.drawString("zombieverter", SCREEN_CENTER_X, 175);
    
    // IP Address - largest and most important
    M5.Display.setTextSize(3);
    M5.Display.setTextColor(TFT_GREEN, TFT_BLACK);
    M5.Display.drawString("192.168.4.1", SCREEN_CENTER_X, 210);
    
    // Exit instruction - small at bottom
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_RED, TFT_BLACK);
    M5.Display.setTextDatum(bottom_center);
    M5.Display.drawString("Rotate to exit", SCREEN_CENTER_X, SCREEN_HEIGHT - 3);
}

void UIManager::drawSettings() {
    clearScreen();
    
    M5.Display.setTextDatum(middle_center);
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(TFT_CYAN, TFT_BLACK);
    M5.Display.drawString("CAN Debug", SCREEN_CENTER_X, 15);
    
    M5.Display.setTextSize(1);
    
    int y = 40;
    
    // Show connection status
    if (canManager->isConnected()) {
        M5.Display.setTextColor(TFT_GREEN, TFT_BLACK);
        M5.Display.drawString("CONNECTED", SCREEN_CENTER_X, y);
    } else {
        M5.Display.setTextColor(TFT_RED, TFT_BLACK);
        M5.Display.drawString("DISCONNECTED", SCREEN_CENTER_X, y);
    }
    y += 20;
    
    // Show parameter count
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    char buf[64];
    snprintf(buf, sizeof(buf), "%d params loaded", canManager->getParameterCount());
    M5.Display.drawString(buf, SCREEN_CENTER_X, y);
    y += 20;
    
    // Check if ANY parameters have recent updates
    bool anyUpdated = false;
    int updateCount = 0;
    unsigned long oldestUpdate = 0;
    
    for (int i = 0; i < canManager->getParameterCount(); i++) {
        CANParameter* param = canManager->getParameterByIndex(i);
        if (param) {
            unsigned long age = millis() - param->lastUpdateTime;
            if (age < 5000) {
                anyUpdated = true;
                updateCount++;
            }
            if (param->lastUpdateTime > 0 && (oldestUpdate == 0 || param->lastUpdateTime > oldestUpdate)) {
                oldestUpdate = param->lastUpdateTime;
            }
        }
    }
    
    // Show update status - LARGE and CLEAR
    if (anyUpdated) {
        M5.Display.setTextColor(TFT_GREEN, TFT_BLACK);
        M5.Display.setTextSize(2);
        snprintf(buf, sizeof(buf), "%d UPDATING!", updateCount);
        M5.Display.drawString(buf, SCREEN_CENTER_X, y);
        y += 25;
    } else {
        M5.Display.setTextColor(TFT_RED, TFT_BLACK);
        M5.Display.setTextSize(2);
        M5.Display.drawString("NO UPDATES", SCREEN_CENTER_X, y);
        y += 25;
    }
    
    // Show sample of what's updating - show MORE parameters
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_CYAN, TFT_BLACK);
    M5.Display.drawString("Param Values:", SCREEN_CENTER_X, y);
    y += 15;
    
    // Show parameters 1-8 specifically
    for (int id = 1; id <= 8 && y < 210; id++) {
        CANParameter* param = canManager->getParameter(id);
        if (param) {
            unsigned long age = millis() - param->lastUpdateTime;
            if (age < 5000) {
                M5.Display.setTextColor(TFT_GREEN, TFT_BLACK);
            } else {
                M5.Display.setTextColor(TFT_DARKGREY, TFT_BLACK);
            }
            snprintf(buf, sizeof(buf), "ID%d=%d %s", param->id, param->getValueAsInt(), param->unit);
            M5.Display.drawString(buf, SCREEN_CENTER_X, y);
            y += 11;
        }
    }
    
    if (!anyUpdated && oldestUpdate > 0) {
        M5.Display.setTextColor(TFT_YELLOW, TFT_BLACK);
        unsigned long age = (millis() - oldestUpdate) / 1000;
        snprintf(buf, sizeof(buf), "Last: %lus ago", age);
        M5.Display.drawString(buf, SCREEN_CENTER_X, y);
    }
    
    // Show what we're looking for
    y = 180;
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    M5.Display.drawString("Listening for:", SCREEN_CENTER_X, y);
    y += 12;
    M5.Display.drawString("0x183, 0x283", SCREEN_CENTER_X, y);
    y += 12;
    M5.Display.drawString("0x5XX, any 8-byte", SCREEN_CENTER_X, y);
}

void UIManager::clearScreen() {
    M5.Display.fillScreen(TFT_BLACK);
}

void UIManager::drawCenteredText(const char* text, int16_t y, uint8_t size) {
    M5.Display.setTextSize(size);
    M5.Display.setTextDatum(middle_center);
    M5.Display.drawString(text, SCREEN_CENTER_X, y);
}

void UIManager::drawArcGauge(int16_t centerX, int16_t centerY, int16_t radius,
                             float startAngle, float endAngle,
                             float value, float min, float max,
                             uint32_t color) {
    float percent = (value - min) / (max - min);
    if (percent < 0) percent = 0;
    if (percent > 1) percent = 1;
    
    float totalAngle = endAngle - startAngle;
    float fillAngle = startAngle + (totalAngle * percent);
    
    // Draw background arc using circles
    for (int i = 0; i < 10; i++) {
        M5.Display.drawCircle(centerX, centerY, radius - i, TFT_DARKGREY);
    }
    
    // Draw filled arc using line segments
    if (percent > 0) {
        int segments = 50;
        for (int i = 0; i < segments * percent; i++) {
            float angle = startAngle + (i * totalAngle / segments);
            float nextAngle = startAngle + ((i + 1) * totalAngle / segments);
            
            int x1 = centerX + (radius - 10) * cos(angle * DEG_TO_RAD);
            int y1 = centerY + (radius - 10) * sin(angle * DEG_TO_RAD);
            int x2 = centerX + radius * cos(angle * DEG_TO_RAD);
            int y2 = centerY + radius * sin(angle * DEG_TO_RAD);
            
            int x3 = centerX + (radius - 10) * cos(nextAngle * DEG_TO_RAD);
            int y3 = centerY + (radius - 10) * sin(nextAngle * DEG_TO_RAD);
            int x4 = centerX + radius * cos(nextAngle * DEG_TO_RAD);
            int y4 = centerY + radius * sin(nextAngle * DEG_TO_RAD);
            
            M5.Display.fillTriangle(x1, y1, x2, y2, x3, y3, color);
            M5.Display.fillTriangle(x2, y2, x3, y3, x4, y4, color);
        }
    }
}

void UIManager::drawRingGauge(const char* label, const char* value,
                             float percent, uint32_t color) {
    if (percent < 0) percent = 0;
    if (percent > 1) percent = 1;
    
    // Draw background ring
    M5.Display.drawCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, 100, TFT_DARKGREY);
    M5.Display.drawCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, 95, TFT_DARKGREY);
    
    // Draw filled arc using line segments (270 degrees total, starting from bottom)
    float startAngle = 135;
    float totalAngle = 270;
    
    int segments = 100;
    for (int i = 0; i < segments * percent; i++) {
        float angle = startAngle + (i * totalAngle / segments);
        
        int x1 = SCREEN_CENTER_X + 95 * cos(angle * DEG_TO_RAD);
        int y1 = SCREEN_CENTER_Y + 95 * sin(angle * DEG_TO_RAD);
        int x2 = SCREEN_CENTER_X + 100 * cos(angle * DEG_TO_RAD);
        int y2 = SCREEN_CENTER_Y + 100 * sin(angle * DEG_TO_RAD);
        
        M5.Display.drawLine(x1, y1, x2, y2, color);
    }
    
    // Draw label and value in center
    M5.Display.setTextDatum(middle_center);
    M5.Display.setTextColor(TFT_WHITE);
    
    M5.Display.setTextSize(1);
    M5.Display.drawString(label, SCREEN_CENTER_X, SCREEN_CENTER_Y - 20);
    
    M5.Display.setTextSize(3);
    M5.Display.drawString(value, SCREEN_CENTER_X, SCREEN_CENTER_Y + 10);
}

void UIManager::drawRingGaugeSimple(float percent, uint32_t color) {
    if (percent < 0) percent = 0;
    if (percent > 1) percent = 1;
    
    // Draw background ring (thinner, at edge)
    for (int r = 115; r < 119; r++) {
        M5.Display.drawCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, r, TFT_DARKGREY);
    }
    
    // Draw filled arc using line segments (270 degrees, starting from bottom)
    float startAngle = 135;
    float totalAngle = 270;
    
    int segments = 100;
    for (int i = 0; i < segments * percent; i++) {
        float angle = startAngle + (i * totalAngle / segments);
        
        int x1 = SCREEN_CENTER_X + 115 * cos(angle * DEG_TO_RAD);
        int y1 = SCREEN_CENTER_Y + 115 * sin(angle * DEG_TO_RAD);
        int x2 = SCREEN_CENTER_X + 119 * cos(angle * DEG_TO_RAD);
        int y2 = SCREEN_CENTER_Y + 119 * sin(angle * DEG_TO_RAD);
        
        M5.Display.drawLine(x1, y1, x2, y2, color);
    }
}

void UIManager::drawGear() {
    clearScreen();
    
    CANParameter* gear = canManager->getParameter(27);
    
    // Title
    M5.Display.setTextDatum(middle_center);
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(TFT_CYAN, TFT_BLACK);
    M5.Display.drawString("GEAR", SCREEN_CENTER_X, 20);
    
    if (!gear) {
        M5.Display.setTextSize(2);
        M5.Display.setTextColor(TFT_RED, TFT_BLACK);
        M5.Display.drawString("NO DATA", SCREEN_CENTER_X, SCREEN_CENTER_Y);
        return;
    }
    
    int32_t currentGear = gear->getValueAsInt();
    
    // Display current gear - LARGE
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_YELLOW, TFT_BLACK);
    M5.Display.drawString("Current:", SCREEN_CENTER_X, 60);
    
    M5.Display.setTextSize(5);
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    
    const char* gearNames[] = {"LOW", "HIGH", "AUTO", "HI/LO"};
    M5.Display.drawString(gearNames[currentGear], SCREEN_CENTER_X, SCREEN_CENTER_Y - 10);
    
    // Show all options with indicators
    int16_t yStart = SCREEN_CENTER_Y + 50;
    M5.Display.setTextSize(2);
    
    for (int i = 0; i < 4; i++) {
        int16_t y = yStart + (i * 22);
        
        if (i == currentGear) {
            // Current selection - highlighted
            M5.Display.setTextColor(TFT_GREEN, TFT_BLACK);
            M5.Display.drawString(String("► ") + gearNames[i], SCREEN_CENTER_X, y);
        } else {
            // Other options - dimmed
            M5.Display.setTextColor(TFT_DARKGREY, TFT_BLACK);
            M5.Display.drawString(String("  ") + gearNames[i], SCREEN_CENTER_X, y);
        }
    }
    
    // Instructions
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_ORANGE, TFT_BLACK);
    M5.Display.setTextDatum(bottom_center);
    M5.Display.drawString("Rotate=adjust | Click=next", SCREEN_CENTER_X, SCREEN_HEIGHT - 3);
}

void UIManager::drawMotor() {
    clearScreen();
    
    CANParameter* motor = canManager->getParameter(129);
    
    // Title
    M5.Display.setTextDatum(middle_center);
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(TFT_CYAN, TFT_BLACK);
    M5.Display.drawString("MOTOR", SCREEN_CENTER_X, 20);
    
    if (!motor) {
        M5.Display.setTextSize(2);
        M5.Display.setTextColor(TFT_RED, TFT_BLACK);
        M5.Display.drawString("NO DATA", SCREEN_CENTER_X, SCREEN_CENTER_Y);
        return;
    }
    
    int32_t currentMotor = motor->getValueAsInt();
    
    // Display current mode - LARGE
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_YELLOW, TFT_BLACK);
    M5.Display.drawString("Active:", SCREEN_CENTER_X, 60);
    
    M5.Display.setTextSize(4);
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    
    const char* motorNames[] = {"MG1 only", "MG2 only", "MG1+MG2", "Blended"};
    M5.Display.drawString(motorNames[currentMotor], SCREEN_CENTER_X, SCREEN_CENTER_Y - 10);
    
    // Show all options with indicators
    int16_t yStart = SCREEN_CENTER_Y + 50;
    M5.Display.setTextSize(2);
    
    for (int i = 0; i < 4; i++) {
        int16_t y = yStart + (i * 22);
        
        if (i == currentMotor) {
            // Current selection - highlighted
            M5.Display.setTextColor(TFT_GREEN, TFT_BLACK);
            M5.Display.drawString(String("► ") + motorNames[i], SCREEN_CENTER_X, y);
        } else {
            // Other options - dimmed
            M5.Display.setTextColor(TFT_DARKGREY, TFT_BLACK);
            M5.Display.drawString(String("  ") + motorNames[i], SCREEN_CENTER_X, y);
        }
    }
    
    // Instructions
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_ORANGE, TFT_BLACK);
    M5.Display.setTextDatum(bottom_center);
    M5.Display.drawString("Rotate=adjust | Click=next", SCREEN_CENTER_X, SCREEN_HEIGHT - 3);
}

void UIManager::drawRegen() {
    clearScreen();
    
    CANParameter* regen = canManager->getParameter(61);
    
    // Title
    M5.Display.setTextDatum(middle_center);
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(TFT_CYAN, TFT_BLACK);
    M5.Display.drawString("REGEN", SCREEN_CENTER_X, 20);
    
    if (!regen) {
        M5.Display.setTextSize(2);
        M5.Display.setTextColor(TFT_RED, TFT_BLACK);
        M5.Display.drawString("NO DATA", SCREEN_CENTER_X, SCREEN_CENTER_Y);
        return;
    }
    
    int32_t currentRegen = regen->getValueAsInt();
    
    // Display current value - LARGE
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_YELLOW, TFT_BLACK);
    M5.Display.drawString("Braking:", SCREEN_CENTER_X, 60);
    
    M5.Display.setTextSize(6);
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    char valueStr[8];
    snprintf(valueStr, sizeof(valueStr), "%d", currentRegen);
    M5.Display.drawString(valueStr, SCREEN_CENTER_X, SCREEN_CENTER_Y - 20);
    
    M5.Display.setTextSize(3);
    M5.Display.setTextColor(TFT_CYAN, TFT_BLACK);
    M5.Display.drawString("%", SCREEN_CENTER_X, SCREEN_CENTER_Y + 30);
    
    // Draw horizontal slider bar
    int16_t barWidth = 180;
    int16_t barHeight = 20;
    int16_t barX = (SCREEN_WIDTH - barWidth) / 2;
    int16_t barY = SCREEN_CENTER_Y + 70;
    
    // Draw bar background
    M5.Display.drawRect(barX, barY, barWidth, barHeight, TFT_DARKGREY);
    
    // Calculate position on bar (-35 to 0 maps to 0 to barWidth)
    float percent = (float)(currentRegen - (-35)) / (0 - (-35));
    int16_t fillWidth = barWidth * percent;
    
    // Draw filled portion
    if (fillWidth > 0) {
        M5.Display.fillRect(barX, barY, fillWidth, barHeight, TFT_GREEN);
    }
    
    // Draw current position indicator
    int16_t markerX = barX + fillWidth;
    M5.Display.fillTriangle(
        markerX - 5, barY - 5,
        markerX + 5, barY - 5,
        markerX, barY,
        TFT_YELLOW
    );
    
    // Show range labels
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(middle_left);
    M5.Display.setTextColor(TFT_DARKGREY, TFT_BLACK);
    M5.Display.drawString("-35", barX - 5, barY + barHeight / 2);
    
    M5.Display.setTextDatum(middle_right);
    M5.Display.drawString("0", barX + barWidth + 5, barY + barHeight / 2);
    
    // Show intensity description
    M5.Display.setTextDatum(middle_center);
    M5.Display.setTextSize(2);
    if (currentRegen <= -25) {
        M5.Display.setTextColor(TFT_RED, TFT_BLACK);
        M5.Display.drawString("STRONG", SCREEN_CENTER_X, barY + 35);
    } else if (currentRegen <= -15) {
        M5.Display.setTextColor(TFT_ORANGE, TFT_BLACK);
        M5.Display.drawString("MEDIUM", SCREEN_CENTER_X, barY + 35);
    } else if (currentRegen <= -5) {
        M5.Display.setTextColor(TFT_YELLOW, TFT_BLACK);
        M5.Display.drawString("LIGHT", SCREEN_CENTER_X, barY + 35);
    } else {
        M5.Display.setTextColor(TFT_DARKGREY, TFT_BLACK);
        M5.Display.drawString("MINIMAL", SCREEN_CENTER_X, barY + 35);
    }
    
    // Instructions
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_ORANGE, TFT_BLACK);
    M5.Display.setTextDatum(bottom_center);
    M5.Display.drawString("Rotate=adjust | Click=next", SCREEN_CENTER_X, SCREEN_HEIGHT - 3);
}
