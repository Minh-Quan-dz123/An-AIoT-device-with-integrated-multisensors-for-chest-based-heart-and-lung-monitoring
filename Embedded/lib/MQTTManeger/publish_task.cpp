#include "publish_task.h"

// 2 khởi tạo constructor
PublishTask::PublishTask(MQTTTask* mqttTask, MQTTBuffer* buf)
{
    mqttConnector = mqttTask;
    mqttBuffer = buf;
}

// 3 begin
void PublishTask::begin()
{   
    // 3.1 tạo task
    xTaskCreate(taskWrapper,"PublishTask",6144,this,2,&taskHandle);
}
void PublishTask::taskWrapper(void* param)
{
    PublishTask* self = (PublishTask*)param;
    self->taskLoop();
}

// 4 hàm send
bool PublishTask::send(MQTTMessage msg)
{
    if (!mqttBuffer) 
    {
        Serial.println("--->[PUBLISHER TASK] loi truyen message");
        return false;
    }
    return mqttBuffer->push(msg);
}


// 5 hàm chạy loop chính
void PublishTask::taskLoop()
{
    // cứ X giây thì kiểm tra gửi dữ liệu lên server
    const uint16_t deltalTime = 1000; // 1s
    MQTTMessage msg;

    runTime = millis();
    while (true)
    {
        if(millis() - runTime >= deltalTime)
        {
            // 5.1 gán time
            runTime = millis();
            Serial.print("--->[PUBLISHER TASK] checking queue ");

            //  --------------------TODO: duyệt tất cả topic và gửi dữ liệu lên hivemq
            if (mqttConnector != nullptr && mqttConnector->isConnected())
            {
               // 5.2 pop message từ queue
                while (mqttBuffer->pop(msg))
                {
                    Serial.print("Publish topic: ");
                    Serial.println(msg.topic);

                    // 5.3 xử lý theo type
                    switch (msg.type)
                    {
                        case PAYLOAD_CHAR:
                        {
                            mqttConnector->publish(msg.topic, msg.data.char_data);
                            break;
                        }

                        case PAYLOAD_INT:
                        // chuyển sang string tạm thời
                        {
                            char payload[128];
                            int offset = 0;

                            for (size_t i = 0; i < msg.size; i++)
                            {
                                offset += sprintf(payload + offset, "%d", msg.data.int_data[i]);

                                if (i < msg.size - 1) payload[offset++] = ',';
                            }

                            payload[offset] = '\0';
                            mqttConnector->publish(msg.topic, payload);
                            break;
                        }
                        

                        case PAYLOAD_UINT16:
                        {
                            char payload[128];
                            int offset = 0;

                            for (size_t i = 0; i < msg.size; i++)
                            {
                                offset += sprintf(payload + offset, "%u", msg.data.uint16_data[i]);

                                if (i < msg.size - 1) payload[offset++] = ',';
                            }

                            payload[offset] = '\0';
                            mqttConnector->publish(msg.topic, payload);
                            break;
                        }

                        case PAYLOAD_FLOAT:
                        {
                            char payload[128];
                            int offset = 0;

                            for (size_t i = 0; i < msg.size; i++)
                            {
                                offset += sprintf(payload + offset, "%.2f", msg.data.float_data[i]);

                                if (i < msg.size - 1) payload[offset++] = ',';
                            }

                            payload[offset] = '\0';
                            mqttConnector->publish(msg.topic, payload);
                            break;
                        }
                    }
                    vTaskDelay(10 / portTICK_PERIOD_MS);
                }
            }
            Serial.print("--->[Publisher Task] Stack còn dư: ");
            Serial.println(uxTaskGetStackHighWaterMark(NULL));
        }
        // block 0.2s
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}