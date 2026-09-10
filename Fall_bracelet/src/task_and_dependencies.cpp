#include "task_and_dependencies.h"

// Pins
uint8_t pinTouch = 36;
uint8_t pinShock = 35;


// Heart sensor
std::atomic<unsigned int> BPM{0};
volatile unsigned long int avgBPM = 0;
volatile uint16_t avgHeartRate = 0;
volatile uint16_t lastAvgHeartRate = 0;
uint32_t avgBPMSum = 0;
uint32_t countRegBPM = 0;

// Serial
QueueHandle_t serialQueue;

// Led/Light
TaskHandle_t lightTaskHandle;

// Shock + Touch
std::atomic<unsigned int> alarmCode;

// Semaphores
volatile SemaphoreHandle_t semHeartBeat_ISR = NULL;
volatile SemaphoreHandle_t semHeartBeat_Mutex = NULL;
volatile SemaphoreHandle_t semShockSensor_ISR = NULL;
volatile SemaphoreHandle_t semTouchSensor_ISR = NULL;


void taskHeartBeat(void *parameter) {
    Serial.println("Reading heart beat!");

    unsigned int BPMSerial;
    unsigned long int avgBPMSerial;
    char auxToSerial[WORD_SIZE * 3];
    memset(auxToSerial, 0, WORD_SIZE * 3);
    memset(&avgBPMSerial, 0, sizeof(uint32_t));
    memset(&BPMSerial, 0, sizeof(uint8_t));

    while (1) {
        if (xSemaphoreTake(semHeartBeat_ISR, GENERAL_DELAY) == pdTRUE) {

            BPMSerial = 0;
            avgBPMSerial = 0;

            if (xSemaphoreTake(semHeartBeat_Mutex, GENERAL_DELAY) == pdTRUE) {
                BPM = BPM * 6;
                
                pHeartRateCharacteristic->notify();
                
                avgBPMSum += BPM;
                countRegBPM++;
                if (countRegBPM == 10) {
                    avgBPM = avgBPMSum / 10;
                    countRegBPM = 0;
                    avgBPMSum = 0;
                }

                BPMSerial = BPM;
                avgBPMSerial = avgBPM;
                
                BPM = 0;
                avgHeartRate = 0;
                lastAvgHeartRate = 0;
                xSemaphoreGive(semHeartBeat_Mutex);
            }

            memset(auxToSerial, 0, WORD_SIZE * 3);

            sprintf(auxToSerial, "BPM: %u AVG BPM: %lu", BPMSerial, avgBPMSerial);

            for (uint8_t i = 0;i < WORD_SIZE * 3;i += WORD_SIZE) {
                if (xQueueSend(serialQueue, auxToSerial + i, GENERAL_DELAY) != pdTRUE) {
                    // i -= WORD_SIZE;
                }
            }

        } else  {
            Serial.println("Cound't take the BPM semaphore!");
        }
    }
}

void taskWriteToSerial(void *parameter) {
    char msg[WORD_SIZE] = "";
    while (1) {
        if (Serial.available()) {
            memset(msg, 0, WORD_SIZE);
            if (xQueueReceive(serialQueue, msg, GENERAL_DELAY)) {
                Serial.println(msg);
            }   
        }
    }
}

void taskShockSensor(void *parameter) {
    char msg[] = "Shock detected!\n";

    while(1) {
        if (xSemaphoreTake(semShockSensor_ISR, GENERAL_DELAY) == pdTRUE) {
            gpio_intr_disable((gpio_num_t)pinShock);
            xQueueSend(serialQueue, msg, GENERAL_DELAY);
            alarmCode = 1;
            pAlarmCharacteristic->notify();
            vTaskResume(lightTaskHandle);
            gpio_intr_enable((gpio_num_t)pinShock);
        }
    }
}

void taskTouchSensor(void *parameter) {
    char msg[] = "Touch sensor t:)\n";

    while (1) {
        if (xSemaphoreTake(semTouchSensor_ISR, GENERAL_DELAY) == pdTRUE) {
            gpio_intr_disable((gpio_num_t)pinTouch);
            xQueueSend(serialQueue, msg, GENERAL_DELAY);
            alarmCode = 0;
            pAlarmCharacteristic->notify();
            vTaskSuspend(lightTaskHandle);
            digitalWrite(LED_BUILT_IN, LOW);
            gpio_intr_enable((gpio_num_t)pinTouch);
        }
    }
}

void taskLedLight(void *parameter) {
    uint16_t blinkTime = 200;
    while (1) {
        digitalWrite(LED_BUILT_IN, HIGH);
        vTaskDelay(pdMS_TO_TICKS(blinkTime));
        digitalWrite(LED_BUILT_IN, LOW);
        vTaskDelay(pdMS_TO_TICKS(blinkTime));
    }
}





