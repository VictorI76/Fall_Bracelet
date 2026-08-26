#include <Arduino.h>

// Constants
#define MAIN_CORE 1
#define GENERAL_DELAY 1005000
#define SERIAL_QUEUE_LENGTH 20
#define WORD_SIZE 20

// Pins
static uint8_t pinHeartBeat = 34;
static uint8_t pinShock = 35;
static uint8_t pinTouch = 36;
static uint8_t ledBuiltIn = 2;

// Variables

// Heart Sensor
volatile static uint16_t heartRate = 0;
volatile static uint8_t avgHeartRateCount = 0;
volatile static uint8_t heartRateAvgCountTop = 3;
volatile static uint16_t avgHeartRate = 0;
volatile static uint16_t lastAvgHeartRate = 0;
volatile static uint8_t avgHeartRateError = 50;
volatile static uint8_t readHreatBeatCount = 0;
volatile static uint8_t heartBeatCountTop = 20;
volatile static uint8_t BPM = 0;
static uint32_t avgBPM = 0;
static uint32_t avgBPMSum = 0;
static uint32_t countRegBPM = 0;

// Shock Sensor
volatile static uint16_t shocksCounter = 0;


// Led built in
uint8_t status = 0;

// Timer 0
static const uint16_t timer_divider0 = 40;
static const uint64_t timer_frequency0 = 1000000; // => 1MHz
static const uint64_t timer_max_count0 = 1000000; // => 1s
static hw_timer_t *timer_hw0 = NULL;

// Spinlock
portMUX_TYPE spinlock = portMUX_INITIALIZER_UNLOCKED;

// Semaphore
volatile static SemaphoreHandle_t semHeartBeat_ISR = NULL;
static SemaphoreHandle_t semHeartBeat_Mutex = NULL;
volatile static SemaphoreHandle_t semShockSensor_ISR = NULL;
volatile static SemaphoreHandle_t semTouchSensor_ISR = NULL;

// Queue
static QueueHandle_t serialQueue;

// Functions
void blinkLed(uint8_t times, uint8_t pin, uint16_t blinkTime);

// Task
void taskHeartBeat(void *parameter);
void taskShockSensor(void *parameter);
void taskTouchSensor(void *parameter);
void taskBuzzerMusic(void *parameter);
void taskLedLight(void *parameter);
void taskWriteToSerial(void *parameter);

// ISR
void IRAM_ATTR onTimer0(void);
void IRAM_ATTR onShock();
void IRAM_ATTR onTouch();

void setup() {

    // Serial
    Serial.begin(115200);

    // Pins
    pinMode(pinHeartBeat, INPUT);
    pinMode(pinShock, INPUT);
    pinMode(pinTouch, INPUT);
    pinMode(ledBuiltIn, OUTPUT);

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
    xTaskCreatePinnedToCore (
        taskHeartBeat,
        "Read the heart rate",
        2048,
        NULL,
        2,
        NULL,
        MAIN_CORE
    );

    xTaskCreatePinnedToCore (
        taskWriteToSerial,
        "Write to serial",
        1024,
        NULL,
        1,
        NULL,
        MAIN_CORE
    );

    xTaskCreatePinnedToCore (
        taskShockSensor,
        "Shock sensor triggered",
        1024,
        NULL,
        3,
        NULL,
        MAIN_CORE
    );

    xTaskCreatePinnedToCore (
        taskTouchSensor,
        "Touch sensor pressed",
        1024,
        NULL,
        3,
        NULL,
        MAIN_CORE
    );

    Serial.println("Start scanning!");
}

void loop() {
    vTaskDelete(NULL);
}


// ISR
void IRAM_ATTR onTimer0(void) {
    BaseType_t task_woken = pdFALSE;

    heartRate = analogRead(pinHeartBeat);
    
    avgHeartRate = avgHeartRate + heartRate;
    avgHeartRateCount = avgHeartRateCount + 1;
    if (avgHeartRateCount == heartRateAvgCountTop) {
        avgHeartRate = avgHeartRate / heartRateAvgCountTop;
        avgHeartRateCount = 0;

        if (abs(lastAvgHeartRate - avgHeartRate) > avgHeartRateError) {
            BPM = BPM + 1;
        }

        lastAvgHeartRate = avgHeartRate;
        readHreatBeatCount = readHreatBeatCount + 1;
    }

    if (heartBeatCountTop == readHreatBeatCount) {
        xSemaphoreGiveFromISR(semHeartBeat_ISR, &task_woken);
        readHreatBeatCount = 0;
    }

    if (task_woken) {
        portYIELD_FROM_ISR();
    }
}

void IRAM_ATTR onShock() {
    BaseType_t task_woken = pdFALSE;

    shocksCounter = shocksCounter + 1;
    if (xSemaphoreGiveFromISR(semShockSensor_ISR, &task_woken) != pdTRUE) {
        Serial.println("Shock sensor ISR could't send the semaphore!");
    }

    if (task_woken) {
        portYIELD_FROM_ISR();
    }
}

void IRAM_ATTR onTouch() {
    BaseType_t task_woken = pdFALSE;

    if (xSemaphoreGiveFromISR(semTouchSensor_ISR, &task_woken) != pdTRUE) {
        Serial.println("Touch sensor ISR could't send the semaphore!");
    }

    if (task_woken) {
        portYIELD_FROM_ISR();
    }
}



// Task
void taskHeartBeat(void *parameter) {
    Serial.println("Reading heart beat!");

    uint8_t BPMSerial;
    uint32_t avgBPMSerial;
    char auxToSerial[WORD_SIZE * 3];
    memset(auxToSerial, 0, WORD_SIZE * 3);
    memset(&avgBPMSerial, 0, sizeof(uint32_t));
    memset(&BPMSerial, 0, sizeof(uint8_t));

    while (1) {
        if (xSemaphoreTake(semHeartBeat_ISR, GENERAL_DELAY) == pdTRUE) {

            BPMSerial = 0;
            avgBPMSerial = 0;

            if (xSemaphoreTake(semHeartBeat_Mutex, GENERAL_DELAY) == pdTRUE) {
                BPM = BPM * 15;
                avgBPMSum += BPM;
                countRegBPM++;
                if (countRegBPM == 20) {
                    avgBPM = avgBPMSum / 20;
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

            unsigned int toPrintBPM = BPMSerial;
            unsigned long int toPrintAvgBPM = avgBPMSerial;
            memset(auxToSerial, 0, WORD_SIZE * 3);

            sprintf(auxToSerial, "BPM: %u AVG BPM: %lu \n\0", toPrintBPM, avgBPMSerial);

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
                Serial.print(msg);
            }   
        }
    }
}

void taskShockSensor(void *parameter) {
    char msg[] = "Shock detected!\n";

    while(1) {
        if (xSemaphoreTake(semShockSensor_ISR, GENERAL_DELAY) == pdTRUE) {
            xQueueSend(serialQueue, msg, GENERAL_DELAY);
            blinkLed(5, ledBuiltIn, 500);
        }
    }
}

void taskTouchSensor(void *parameter) {
    char msg[] = "Touch sensor t:)\n";

    while (1) {
        if (xSemaphoreTake(semTouchSensor_ISR, GENERAL_DELAY) == pdTRUE) {
            gpio_intr_disable((gpio_num_t)pinTouch);
            xQueueSend(serialQueue, msg, GENERAL_DELAY);
            blinkLed(5, ledBuiltIn, 200);
            gpio_intr_enable((gpio_num_t)pinTouch);
        }
    }
}


// Funcctions

void blinkLed(uint8_t times, uint8_t pin, uint16_t blinkTime){
    for (uint8_t i = 0;i < times;i++) {
        digitalWrite(pin, HIGH);
        vTaskDelay(pdMS_TO_TICKS(blinkTime));
        digitalWrite(pin, LOW);
        vTaskDelay(pdMS_TO_TICKS(blinkTime));
    }
}











