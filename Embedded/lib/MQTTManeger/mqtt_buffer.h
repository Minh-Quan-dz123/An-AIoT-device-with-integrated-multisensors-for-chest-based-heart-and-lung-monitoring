#pragma once

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#define MAX_PAYLOAD_SIZE 128

// 1 enum kiểu dữ liệu gửi đi
enum PayloadType
{
    PAYLOAD_CHAR,
    PAYLOAD_INT,
    PAYLOAD_UINT16,
    PAYLOAD_FLOAT
};

// 2 cấu trúc 1 message
struct MQTTMessage
{
    char topic[32];
    PayloadType type;
    size_t size;

    union {
        char char_data[MAX_PAYLOAD_SIZE];
        int int_data[MAX_PAYLOAD_SIZE];
        uint16_t uint16_data[MAX_PAYLOAD_SIZE];
        float float_data[MAX_PAYLOAD_SIZE];
    } data;
};

class MQTTBuffer
{
    private:
        QueueHandle_t queue;// hàng đợi trong esp32

    public:
        // 1 constructor
        MQTTBuffer();

        // hàm begin
        bool begin(int maxSize = 10);// tối đa 10 phần tử, nếu mặc định

        bool push(const MQTTMessage &msg);
        bool pop(MQTTMessage &msg);

        bool isEmpty();
};