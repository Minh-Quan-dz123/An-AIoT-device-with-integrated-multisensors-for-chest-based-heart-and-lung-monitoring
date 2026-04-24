#ifndef FLEX_SENSOR_TASKS_H
#define FLEX_SENSOR_TASKS_H

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


// ------------------- thêm
#include "publish_task.h"

// 1 mẫu dữ liệu Flex
struct FlexSample {
    uint32_t timestamp_ms;   // thời gian lấy mẫu
    int raw_adc_value;       // giá trị ADC thô
    float filtered_value;    // giá trị sau EMA
};

// Cấu hình Flex sensor
struct FlexSensorConfig {
    uint8_t adc_pin;               // chân ADC
    uint16_t sampling_period_ms;   // chu kỳ lấy mẫu
    uint16_t block_duration_ms;    // thời gian gom 1 block
    uint16_t max_samples_per_block;// số mẫu tối đa trong block
    float ema_alpha;               // hệ số EMA
};

// Khối dữ liệu gửi đi
struct FlexDataBlock {
    uint32_t block_start_time_ms;  // thời điểm bắt đầu block
    uint16_t sampling_period_ms;   // chu kỳ lấy mẫu
    uint16_t sample_count;         // số mẫu hiện có
    FlexSample samples[64];        // bộ đệm mẫu
};

// 1 Hàm task chính
void TaskFlexSensor(void *pvParameters);

// 2 Hàm khởi tạo task
void initFlexSensorTask(PublishTask* publisher);// --------sửa

// 3 Hàm cấu hình
void setupFlexSensorConfiguration(const FlexSensorConfig &config);

// 4 Hàm đọc ADC thô
int readRawFlexValue();

// 5 Hàm lọc EMA
float applyEMALowPassFilter(int raw_value);

// 6 Hàm thêm mẫu vào block
void addSampleToFlexBlock(int raw_value, float filtered_value);

// 7 Hàm kiểm tra block đã đủ để gửi chưa
bool isFlexBlockReadyToSend();

// ---------------8 sửa: truyền publisher vào
void sendFlexDataBlock(PublishTask* publisher);

// 9 Hàm reset block
void resetFlexDataBlock();

#endif