// #include "task_and_dependencies.h"

// void taskHeartBeat(void *parameter) {
//     Serial.println("Reading heart beat!");

//     unsigned int BPMSerial;
//     unsigned long int avgBPMSerial;
//     char auxToSerial[WORD_SIZE * 3];
//     memset(auxToSerial, 0, WORD_SIZE * 3);
//     memset(&avgBPMSerial, 0, sizeof(uint32_t));
//     memset(&BPMSerial, 0, sizeof(uint8_t));

//     while (1) {
//         if (xSemaphoreTake(semHeartBeat_ISR, GENERAL_DELAY) == pdTRUE) {

//             BPMSerial = 0;
//             avgBPMSerial = 0;

//             if (xSemaphoreTake(semHeartBeat_Mutex, GENERAL_DELAY) == pdTRUE) {
//                 BPM = BPM * 6;
//                 avgBPMSum += BPM;
//                 countRegBPM++;
//                 if (countRegBPM == 10) {
//                     avgBPM = avgBPMSum / 10;
//                     countRegBPM = 0;
//                     avgBPMSum = 0;
//                 }

//                 BPMSerial = BPM;
//                 avgBPMSerial = avgBPM;
                
//                 BPM = 0;
//                 avgHeartRate = 0;
//                 lastAvgHeartRate = 0;
//                 xSemaphoreGive(semHeartBeat_Mutex);
//             }

//             memset(auxToSerial, 0, WORD_SIZE * 3);

//             sprintf(auxToSerial, "BPM: %u AVG BPM: %lu", BPMSerial, avgBPMSerial);

//             for (uint8_t i = 0;i < WORD_SIZE * 3;i += WORD_SIZE) {
//                 if (xQueueSend(serialQueue, auxToSerial + i, GENERAL_DELAY) != pdTRUE) {
//                     // i -= WORD_SIZE;
//                 }
//             }

//         } else  {
//             Serial.println("Cound't take the BPM semaphore!");
//         }
//     }
// }


// void taskWriteToSerial(void *parameter) {
//     char msg[WORD_SIZE] = "";
//     while (1) {
//         if (Serial.available()) {
//             memset(msg, 0, WORD_SIZE);
//             if (xQueueReceive(serialQueue, msg, GENERAL_DELAY)) {
//                 Serial.println(msg);
//             }   
//         }
//     }
// }


// void taskShockSensor(void *parameter) {
//     char msg[] = "Shock detected!\n";

//     while(1) {
//         if (xSemaphoreTake(semShockSensor_ISR, GENERAL_DELAY) == pdTRUE) {
//             xQueueSend(serialQueue, msg, GENERAL_DELAY);
//             vTaskResume(lightTaskHandle);
//         }
//     }
// }


// void taskTouchSensor(void *parameter) {
//     char msg[] = "Touch sensor t:)\n";

//     while (1) {
//         if (xSemaphoreTake(semTouchSensor_ISR, GENERAL_DELAY) == pdTRUE) {
//             gpio_intr_disable((gpio_num_t)pinTouch);
//             xQueueSend(serialQueue, msg, GENERAL_DELAY);
//             vTaskSuspend(lightTaskHandle);
//             gpio_intr_enable((gpio_num_t)pinTouch);
//         }
//     }
// }


// void taskLedLight(void *parameter) {
//     uint16_t blinkTime = 500;
//     while (1) {
//         digitalWrite(LED_LIGHT_PIN, HIGH);
//         vTaskDelay(pdMS_TO_TICKS(blinkTime));
//         digitalWrite(LED_LIGHT_PIN, LOW);
//         vTaskDelay(pdMS_TO_TICKS(blinkTime));
//     }
// }







