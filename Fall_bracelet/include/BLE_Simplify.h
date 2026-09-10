#pragma once
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEDescriptor.h>
#include <atomic>



// Varibles
// -> Server
extern BLEServer *pServer;

// -> Service
extern BLEService *pAlarmService;
extern BLEService *pDataService;

// -> Characteristic
extern BLECharacteristic *pAlarmCharacteristic;
extern BLECharacteristic *pHeartRateCharacteristic;

extern std::atomic<unsigned int> BPM;

extern std::atomic<unsigned int> alarmCode;

void initBLEServer();

void ServiceAlarm();

void ServiceData();

void StartAdvertising();


