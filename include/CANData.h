#ifndef CAN_DATA_H
#define CAN_DATA_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "Config.h"

// CAN Parameter data types
enum ParamDataType {
    PARAM_INT8,
    PARAM_UINT8,
    PARAM_INT16,
    PARAM_UINT16,
    PARAM_INT32,
    PARAM_UINT32,
    PARAM_FLOAT
};

// CAN Parameter structure
struct CANParameter {
    uint16_t id;
    char name[32];
    ParamDataType dataType;
    
    union {
        int8_t i8;
        uint8_t u8;
        int16_t i16;
        uint16_t u16;
        int32_t i32;
        uint32_t u32;
        float f32;
    } value;
    
    bool editable;
    int32_t minValue;
    int32_t maxValue;
    
    char unit[8];
    uint8_t decimalPlaces;
    
    uint32_t lastUpdateTime;
    bool dirty;
    
    // Get value as string
    void toString(char* buffer, size_t bufferSize);
    
    // Set value from int32
    void setValue(int32_t val);
    
    // Get value as int32
    int32_t getValueAsInt();
};

// CAN Message structure
struct CANMessage {
    uint32_t id;
    uint8_t data[8];
    uint8_t length;
    uint32_t timestamp;
};

// CAN Data Manager
class CANDataManager {
public:
    CANDataManager();
    
    bool init();
    void update();
    
    // Parameter management
    bool loadParametersFromJSON(const char* jsonString);
    CANParameter* getParameter(uint16_t id);
    CANParameter* getParameterByIndex(uint8_t index);
    uint16_t getParameterCount() { return parameterCount; }
    
    // CAN communication
    void requestParameter(uint16_t paramId);
    void setParameter(uint16_t paramId, int32_t value);
    bool sendMessage(uint32_t id, uint8_t* data, uint8_t length);
    
    // Connection status
    bool isConnected() { return connected; }
    uint32_t getLastMessageTime() { return lastMessageTime; }
    
    // BMS cell voltage access
    uint16_t getCellVoltage(uint8_t cellIndex);  // Returns voltage in mV
    uint8_t getCellCount() { return bmsCellCount; }
    uint32_t getCellLastUpdate(uint8_t cellIndex);
    
private:
    CANParameter parameters[MAX_PARAMETERS];
    uint16_t parameterCount;
    
    CANMessage txQueue[TX_QUEUE_SIZE];
    CANMessage rxQueue[RX_QUEUE_SIZE];
    uint8_t txHead, txTail;
    uint8_t rxHead, rxTail;
    
    bool connected;
    uint32_t lastMessageTime;
    
    // BMS cell voltage storage (up to 96 cells = 16 modules * 6 cells)
    static const uint8_t MAX_BMS_CELLS = 96;
    uint16_t bmsCellVoltages[MAX_BMS_CELLS];     // Voltages in mV
    uint32_t bmsCellUpdateTimes[MAX_BMS_CELLS];  // Last update timestamp
    uint8_t bmsCellCount;                         // Actual number of cells detected
    
    void processReceivedMessage(CANMessage& msg);
    void handleSDOResponse(CANMessage& msg);
    void handlePDOMessage(CANMessage& msg);
    void handleGenericMessage(CANMessage& msg);
    void handleBMSCellVoltage(CANMessage& msg);
    void updateParameterIfExists(uint16_t paramId, int32_t value);
    
    // Queue management
    bool enqueueTx(CANMessage& msg);
    bool dequeueTx(CANMessage& msg);
    bool enqueueRx(CANMessage& msg);
    bool dequeueRx(CANMessage& msg);
};

#endif // CAN_DATA_H
