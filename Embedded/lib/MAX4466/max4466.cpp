#include "max4466.h"


// 1 task đọc microphone
void micTask(void *parameter) 
{
    while (1) 
    {
        uint32_t signalMax = 0;
        uint32_t signalMin = 4095;

        unsigned long start = millis();

        // lấy mẫu trong 50ms
        int val = analogRead(MAX4466_PIN);

        Serial.print("--->[MAX4466] ");
        Serial.println(val);


        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// start task
void startMicTask() 
{
    // đọc với giá trị 12 bit
    analogReadResolution(12);

    xTaskCreate(
        micTask,
        "Mic Task",
        2048,
        NULL,
        2,
        NULL
    );
}
