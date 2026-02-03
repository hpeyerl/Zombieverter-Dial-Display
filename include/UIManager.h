#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <Arduino.h>
#include <M5Unified.h>
#include "CANData.h"

// Screen IDs
enum ScreenID {
    SCREEN_SPLASH,
    SCREEN_DASHBOARD,
    SCREEN_POWER,
    SCREEN_TEMPERATURE,
    SCREEN_BATTERY,
    SCREEN_BMS,
    SCREEN_GEAR,
    SCREEN_MOTOR,
    SCREEN_REGEN,
    SCREEN_WIFI,
    SCREEN_SETTINGS,
    SCREEN_COUNT
};

class UIManager {
public:
    UIManager();
    
    bool init(CANDataManager* canMgr);
    void update();
    
    void setScreen(ScreenID screen);
    void nextScreen();
    void previousScreen();
    ScreenID getCurrentScreen() const { return currentScreen; }
    
    void incrementValue();
    void decrementValue();
    void selectValue();
    
    void showSplash();
    void showConnectionStatus(bool connected);
    void updateLastCANID(uint32_t canID);  // For CAN ID debugging
    
    // BMS navigation
    void nextBMSPage();
    void previousBMSPage();
    
private:
    CANDataManager* canManager;
    ScreenID currentScreen;
    bool editing;
    uint8_t editingParamIndex;
    int32_t editingValue;
    
    uint32_t lastUpdateTime;
    
    // BMS page tracking
    uint8_t bmsPage;           // 0 = summary, 1+ = module pages
    uint8_t bmsMaxPages;       // Total number of BMS pages
    
    // Drawing functions
    void drawDashboard();
    void drawPower();
    void drawTemperature();
    void drawBattery();
    void drawBMS();
    void drawBMSSummary();
    void drawBMSModule(uint8_t moduleIndex);
    void drawGear();
    void drawMotor();
    void drawRegen();
    void drawWiFi();
    void drawSettings();
    
    // Helper functions
    void drawCenteredText(const char* text, int16_t y, uint8_t size = 2);
    void drawArcGauge(int16_t centerX, int16_t centerY, int16_t radius, 
                      float startAngle, float endAngle, 
                      float value, float min, float max,
                      uint32_t color);
    void drawRingGauge(const char* label, const char* value, 
                      float percent, uint32_t color);
    void drawRingGaugeSimple(float percent, uint32_t color);
    void clearScreen();
};

#endif // UI_MANAGER_H
