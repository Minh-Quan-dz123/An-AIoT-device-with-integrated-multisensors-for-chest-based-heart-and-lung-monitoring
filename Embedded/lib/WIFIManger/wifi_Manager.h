#ifndef WIFI_MANAGER_TASK_H
#define WIFI_MANAGER_TASK_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "globals.h"


class WiFiManagerTask
{
    private:
        // 1 trạng thái
        WiFiState state;
        // và cập nhật cả ở globals

    public:
        // 2 constructor
        WiFiManagerTask();

        // 3 hàm bắt đầu quá trình
        void begin();

        WiFiState getState();

    private:
        
        // 4 ssid và password
        String ssid;
        String password;

        const char* ap_ssid = "ESP32_SETUP_HEARD_LUNG_HEHE";// tên wifi của esp

        // 5 object server và flash
        WebServer server;
        Preferences preferences;

        // 6 con trỏ tới 2 task sẽ làm việc
        TaskHandle_t mainTaskHandle = NULL; // task chính
        TaskHandle_t connectTaskHandle = NULL; // task thực hiện connect

        // 7 hàm wrapper của task chính
        static void taskMain(void* param);
        void loop(); // hàm loop chính của logic

        // 8 hàm wrapper của task phụ
        static void taskConnect(void* param);
        void createConnectTask();// 9 hàm tạo task kết nối wifi

        // 10 các hàm phục vụ kết nối
        bool connectWiFi();// thực hiện kết nối wifi
        void startAP();
        void stopAP();

        // 11 hàm phục vụ cho server
        void startServer();
        void handleRoot();
        void handleSave();
        void handleStatus();
};

void printlnWiFiConnecting();
void printlnWiFiConnected();
void printlnWiFiFailed();
void printlnWiFiNotConnected();

#endif