// #ifndef TASK_
// #define TASK_

// #include <Arduino.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "freertos/semphr.h"
// #include "freertos/queue.h"
// #include "freertos/timers.h"

// // Defines
// #define LED_LIGHT_PIN 2
// #define WORD_SIZE 20
// #define GENERAL_DELAY 1005000

// // Pins
// static uint8_t pinTouch = 36;

// // Heart sensor
// volatile static unsigned int BPM = 0;
// volatile static unsigned long int avgBPM = 0;
// volatile static uint16_t avgHeartRate = 0;
// volatile static uint16_t lastAvgHeartRate = 0;
// static uint32_t avgBPMSum = 0;
// static uint32_t countRegBPM = 0;

// // Serial
// static QueueHandle_t serialQueue;

// // Led/Light
// TaskHandle_t lightTaskHandle;

// // Semaphores
// volatile static SemaphoreHandle_t semHeartBeat_ISR = NULL;
// static SemaphoreHandle_t semHeartBeat_Mutex = NULL;
// volatile static SemaphoreHandle_t semShockSensor_ISR = NULL;
// volatile static SemaphoreHandle_t semTouchSensor_ISR = NULL;

// /**
//  * @brief Calculate the heart beat.
//  * 
//  * @param parameter 
//  */
// void taskHeartBeat(void *parameter);

// /**
//  * @brief Turn on the led task on shock.
//  * 
//  * @param parameter 
//  */
// void taskShockSensor(void *parameter);

// /**
//  * @brief Stop the led task on touch.
//  * 
//  * @param parameter 
//  */
// void taskTouchSensor(void *parameter);

// /**
//  * @brief Play a secuence of sound on the buzzer.
//  * 
//  * @param parameter 
//  */
// void taskBuzzerMusic(void *parameter);

// /**
//  * @brief Blink the selected led at a regular interval.
//  * 
//  * @param parameter 
//  */
// void taskLedLight(void *parameter);

// /**
//  * @brief Write to serial the messages on queue.
//  * 
//  * @param parameter 
//  */
// void taskWriteToSerial(void *parameter);


// #endif

