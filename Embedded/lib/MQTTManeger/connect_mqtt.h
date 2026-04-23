#ifndef MQTT_TASK_H
#define MQTT_TASK_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>


class MQTTTask
{
    private:
        // 1 wifi client và mqtt client
        WiFiClientSecure espClient;
        PubSubClient client;

        // 2 con trỏ tới task
        TaskHandle_t mqttTaskHandle;

        // 3 thông tin server HiveMQ
        String mqttServer;
        uint16_t mqttPort;
        String mqttUser;
        String mqttPass;

        // 4 id của client
        String clientId;

        // 5 wrapper của task này
        static void taskWrapper(void* param);
        void taskLoop();// hàm loop chính
        void reconnect();// kết nối lại

    public:
        // 6 hàm khởi tạo
        MQTTTask(String server,uint16_t port,String user,String pass);

        // 7 hàm bắt đầu luồng
        void begin();

        // 8 hàm wrapper publish, subcribe
        void publish(const char* topic, const char* message);
        void subscribe(const char* topic);

        // 9 trả về trạng thái connect chưa
        bool isConnected();

};

#endif