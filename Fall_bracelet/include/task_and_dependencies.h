// #ifndef TASK_
// #define TASK_

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/timers.h"

// Defines
#define LED_BUILT_IN 2
#define WORD_SIZE 20
#define GENERAL_DELAY 1005000

// Pins
extern uint8_t pinTouch;
extern uint8_t pinShock;

// Heart sensor
extern volatile unsigned int BPM;
extern volatile unsigned long int avgBPM;
extern volatile uint16_t avgHeartRate;
extern volatile uint16_t lastAvgHeartRate;
extern uint32_t avgBPMSum;
extern uint32_t countRegBPM;

// Serial
extern QueueHandle_t serialQueue;

// Led/Light
extern TaskHandle_t lightTaskHandle;

// Semaphores
extern volatile SemaphoreHandle_t semHeartBeat_ISR;
extern volatile SemaphoreHandle_t semHeartBeat_Mutex;
extern volatile SemaphoreHandle_t semShockSensor_ISR;
extern volatile SemaphoreHandle_t semTouchSensor_ISR;


/**
 * @brief Calculate the heart beat.
 * 
 * @param parameter 
 */
void taskHeartBeat(void *parameter);

/**
 * @brief Turn on the led task on shock.
 * 
 * @param parameter 
 */
void taskShockSensor(void *parameter);

/**
 * @brief Stop the led task on touch.
 * 
 * @param parameter 
 */
void taskTouchSensor(void *parameter);

/**
 * @brief Play a secuence of sound on the buzzer.
 * 
 * @param parameter 
 */
void taskBuzzerMusic(void *parameter);

/**
 * @brief Blink the selected led at a regular interval.
 * 
 * @param parameter 
 */
void taskLedLight(void *parameter);

/**
 * @brief Write to serial the messages on queue.
 * 
 * @param parameter 
 */
void taskWriteToSerial(void *parameter);


// #endif

