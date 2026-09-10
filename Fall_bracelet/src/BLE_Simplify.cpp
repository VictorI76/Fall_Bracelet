#include "BLE_Simplify.h"

// Defince
#define DEVICE_NAME "Fall Band"
#define SERVICE_ALARM_UUID "00c6dbf6-9253-40a9-b487-c264df632c7c"
#define SERVICE_HEARTRATE_UUID "909fd234-a88e-43a3-a036-20ef8d169366"
#define CHARACTERISTIC_ALARM_UUID "c80355be-6dd9-4af5-90f4-9e9c43bd31a9"
#define CHARACTERISTIC_HEARTRATE_UUID "8f826b1a-6e7f-4c5d-9e7f-538eb3ddd781"

// Varibles
// -> Server
BLEServer *pServer;

// -> Service
BLEService *pAlarmService;
BLEService *pDataService;

// -> Characteristic
BLECharacteristic *pAlarmCharacteristic;
BLECharacteristic *pHeartRateCharacteristic;


// Callback
class MainServerCallback : public BLEServerCallbacks {
    void onConnect(BLEServer *pServer) {
        Serial.println("Connect!");
    }

    void onDisconnect(BLEServer *pServer) {
        Serial.println("Disconnect!");
    }
};

class AlarmCharacteristicCallback : public BLECharacteristicCallbacks {
    void onNotify(BLECharacteristic * pCharacteristic) {
        uint32_t output = alarmCode;
        pCharacteristic->setValue(output);
    }

    void onRead(BLECharacteristic * pCharacteristic) {
        uint32_t output = alarmCode;
        pCharacteristic->setValue(output);
    }
};

class HeartRateCharacteristicCallback : public BLECharacteristicCallbacks {
    void onNotify(BLECharacteristic * pCharacteristic) {
        uint32_t output = BPM;
        pCharacteristic->setValue(output);
    }

    void onRead(BLECharacteristic * pCharacteristic) {
        uint32_t output = BPM;
        pCharacteristic->setValue(output);
    }
};



void initBLEServer(){
    // Initialize Device
    BLEDevice::init(DEVICE_NAME);

    // Create Server
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MainServerCallback());
}

void ServiceAlarm(){
    // Service
    pAlarmService = pServer->createService(SERVICE_ALARM_UUID);

    // Characteristic
    pAlarmCharacteristic = pAlarmService->createCharacteristic(
        CHARACTERISTIC_ALARM_UUID,
        BLECharacteristic::PROPERTY_NOTIFY |
        BLECharacteristic::PROPERTY_READ
    );

    pAlarmCharacteristic->setCallbacks(new AlarmCharacteristicCallback());

    // Descriptors
    BLEDescriptor *pDescriptor;
    
    // Name
    pDescriptor = new BLEDescriptor((uint16_t)0x2901);
    pDescriptor->setValue("Alarm");
    pAlarmCharacteristic->addDescriptor(pDescriptor);

    // Notify
    pDescriptor = new BLEDescriptor((uint16_t)0x2902);
    pAlarmCharacteristic->addDescriptor(pDescriptor);

    // Start
    pAlarmService->start();
}

void ServiceData(){
    // Service
    pDataService = pServer->createService(SERVICE_HEARTRATE_UUID);

    // Characteristic
    pHeartRateCharacteristic = pDataService->createCharacteristic(
        CHARACTERISTIC_HEARTRATE_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_NOTIFY
    );

    pHeartRateCharacteristic->setCallbacks(new HeartRateCharacteristicCallback());

    // Descriptors
    BLEDescriptor *pDescriptor;
    
    // Name
    pDescriptor = new BLEDescriptor((uint16_t)0x2901);
    pDescriptor->setValue("Heart Rate");
    pHeartRateCharacteristic->addDescriptor(pDescriptor);

    // Notify
    pDescriptor = new BLEDescriptor((uint16_t)0x2902);
    pHeartRateCharacteristic->addDescriptor(pDescriptor);

    // Start
    pDataService->start();
}

void StartAdvertising(){
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();

    BLEAdvertisementData advertisementData;
    advertisementData.setName(DEVICE_NAME);

    pAdvertising->setAdvertisementData(advertisementData);

    pAdvertising->addServiceUUID(SERVICE_ALARM_UUID);
    pAdvertising->addServiceUUID(SERVICE_HEARTRATE_UUID);

    pAdvertising->setScanResponse(true);
    
    BLEDevice::startAdvertising();
}





