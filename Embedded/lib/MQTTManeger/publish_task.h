#ifndef PUBLISH_TASK_H
#define PUBLISH_TASK_H

#include <Arduino.h>
#include <connect_mqtt.h>
#include <mqtt_buffer.h>



class PublishTask
{
    private:
        // 1 con trỏ task
        TaskHandle_t taskHandle;

        // 2 task
        static void taskWrapper(void* param);
        void taskLoop();

        // 3 điều khiển mqtt connection
        MQTTTask* mqttConnector;

        // 4 thuộc tính buffer
        MQTTBuffer* mqttBuffer;

        // 5 runtime
        uint32_t runTime=0;

    public:
        // constructor
        PublishTask(MQTTTask* mqttConnector, MQTTBuffer* buf);

        // hàm bắt đầu
        void begin();

        // 3 API cho main / sensor dùng
        bool send(MQTTMessage x);

};

#endif