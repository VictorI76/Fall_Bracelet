#include <Arduino.h>
#include "task_and_dependencies.h"

// Constants
#define MAIN_CORE 1
#define SERIAL_QUEUE_LENGTH 20

//  Task ON/OFF
#define HEART_SWITCH false
#define WRITE_SERIAL_SWITCH true
#define SHOCK_SWITCH true
#define TOUCH_SWITCH true
#define LED_SWITCH true

// static functions
#define sgn(x, y) (((x) < (y)) ? -1 : 1)

// Pins
static uint8_t pinHeartBeat = 34;

// Variables

// Heart Sensor
volatile static uint32_t heartRate = 0;
volatile static uint8_t avgHeartRateCount = 0;
volatile static uint8_t heartRateAvgCountTop = 3;
volatile static uint8_t avgHeartRateErrorMAX = 50;
volatile static uint8_t avgHeartRateErrorMIN = 20;
volatile static uint8_t heartBeatCount = 0;
volatile static uint8_t heartBeatCountTop = 65;

volatile static uint32_t currValue;
volatile static uint32_t lastValue;
volatile static uint32_t direction;
volatile static uint16_t timeOut = 3;

// Shock Sensor
volatile static uint16_t shocksCounter = 0;
volatile static uint64_t shockTime = 0;
volatile static uint64_t lastShockTime = 0;
volatile static uint8_t shockPouse = 100;

// Touch Sensor
volatile static uint64_t pressTime = 0;
volatile static uint64_t lastPressTime = 0;
volatile static uint8_t pressPouse = 100;

// Timer 0
static const uint16_t timer_divider0 = 80;
static const uint64_t timer_frequency0 = 1000000; // => 1MHz
static const uint64_t timer_max_count0 = 100000; // => 100ms
static hw_timer_t *timer_hw0 = NULL;

// ISR
void IRAM_ATTR onTimer0(void);
void IRAM_ATTR onShock(void);
void IRAM_ATTR onTouch(void);

void setup() {

    // Serial
    Serial.begin(115200);

    // Pins
    pinMode(pinHeartBeat, INPUT);
    pinMode(pinShock, INPUT);
    pinMode(pinTouch, INPUT);
    pinMode(LED_BUILT_IN, OUTPUT);

    // Timer 0
    timer_hw0 = timerBegin(timer_frequency0);
    timerAttachInterrupt(timer_hw0, &onTimer0);
    timerAlarm(timer_hw0, timer_max_count0, true, 0);

    // Shock
    attachInterrupt(digitalPinToInterrupt(pinShock), &onShock, FALLING);

    // Touch
    attachInterrupt(digitalPinToInterrupt(pinTouch), &onTouch, RISING);

    // Semaphore
    semHeartBeat_ISR = xSemaphoreCreateBinary();
    semHeartBeat_Mutex = xSemaphoreCreateMutex();
    semShockSensor_ISR = xSemaphoreCreateBinary();
    semTouchSensor_ISR = xSemaphoreCreateBinary();

    // Queue
    serialQueue = xQueueCreate(SERIAL_QUEUE_LENGTH, sizeof(char) * WORD_SIZE);

    // Tasks
    if (HEART_SWITCH) {
        xTaskCreatePinnedToCore (
            taskHeartBeat,
            "Read the heart rate",
            2048,
            NULL,
            2,
            NULL,
            MAIN_CORE
        );
    }

    if (WRITE_SERIAL_SWITCH) {
        xTaskCreatePinnedToCore (
            taskWriteToSerial,
            "Write to serial",
            1024,
            NULL,
            1,
            NULL,
            MAIN_CORE
        );
    }

    if (SHOCK_SWITCH) {
        xTaskCreatePinnedToCore (
            taskShockSensor,
            "Shock sensor triggered",
            1024,
            NULL,
            3,
            NULL,
            MAIN_CORE
        );
    }

    if (TOUCH_SWITCH) {
        xTaskCreatePinnedToCore (
            taskTouchSensor,
            "Touch sensor pressed",
            1024,
            NULL,
            3,
            NULL,
            MAIN_CORE
        );
    }

    if (LED_SWITCH) {
        xTaskCreatePinnedToCore(
            taskLedLight,
            "A blinking routine with the leds",
            1024,
            NULL,
            1,
            &lightTaskHandle,
            MAIN_CORE
        );

        vTaskSuspend(lightTaskHandle);
    }

    Serial.println("Start scanning!");
}

void loop() {
    vTaskDelete(NULL);
}


// ISR
void IRAM_ATTR onTimer0(void) {
    BaseType_t task_woken = pdFALSE;

    heartRate = analogRead(pinHeartBeat);

    // char aux[WORD_SIZE];

    // itoa(heartRate, aux, 10);
    // strcat(aux, ",");

    // xQueueSendFromISR(serialQueue, aux, &task_woken);

    if (heartRate > 500 && timeOut <= 0) {
        
        currValue = heartRate;

        if (sgn(currValue, lastValue) == direction) {
            lastValue = currValue;
        } else {
            
            if (sgn(currValue, lastValue) == -1) {
                BPM = BPM + 1;
                timeOut = 3;
            }

            direction = direction * -1;
            lastValue = currValue;
        }

    } else if (timeOut > 0) {
        timeOut = timeOut - 1;
    }

    heartBeatCount = heartBeatCount + 1;
    if (heartBeatCount == heartBeatCountTop) {
        xSemaphoreGiveFromISR(semHeartBeat_ISR, &task_woken);
        heartBeatCount = 0;
    }
    
    if (task_woken) {
        portYIELD_FROM_ISR();
    }
}

void IRAM_ATTR onShock(void) {
    BaseType_t task_woken = pdFALSE;
    shockTime = millis();

    if (shockTime - lastShockTime > shockPouse) {
        shocksCounter = shocksCounter + 1;
        if (xSemaphoreGiveFromISR(semShockSensor_ISR, &task_woken) != pdTRUE) {
            Serial.println("Shock sensor ISR could't send the semaphore!");
        }

        if (task_woken) {
            portYIELD_FROM_ISR();
        }
        lastShockTime = shockTime;
    }
}

void IRAM_ATTR onTouch(void) {
    BaseType_t task_woken = pdFALSE;
    pressTime = millis();

    if (pressTime - lastPressTime > pressPouse) {
        if (xSemaphoreGiveFromISR(semTouchSensor_ISR, &task_woken) != pdTRUE) {
            Serial.println("Touch sensor ISR could't send the semaphore!");
        }

        if (task_woken) {
            portYIELD_FROM_ISR();
        }
        lastPressTime = pressTime;
    }
}









