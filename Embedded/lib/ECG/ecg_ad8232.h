#ifndef AD8232_TASKS_H
#define AD8232_TASKS_H

//1. khai báo các thư viện cần để sử dụng
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// thêm
#include <publish_task.h>
#include <mqtt_buffer.h>

//2. Khai báo các biến, struct của header này
// Struct đại diện cho dữ liệu ECG
struct ECGData {
    uint32_t timestamp_ms;     // thời gian lấy mẫu (ms)
    float voltage_mv;          // điện áp (mV)
    uint16_t raw_adc_value;         // giá trị ADC thô
    uint8_t lead_off_status;   // trạng thái lead-off (1: mất tín hiệu, 0: có tín hiệu)
};

// Cấu hình ECG
struct ECGConfig {
    uint32_t buffer_size;      // kích thước bộ đệm (nếu cần)
    uint16_t sampling_period_ms; // chu kỳ lấy mẫu (ms)
    uint8_t adc_pin;           // chân ADC để đọc
    uint8_t lead_off_pin_pos;  // chân LEAD-OFF LO+ (nếu có)
    uint8_t lead_off_pin_neg;  // chân LEAD-OFF LO- (nếu có)
};

//3. khai báo (nhớ là chỉ khai báo thôi) các hàm tự định nghĩa
void TaskECG(void *pvParameters);              // hàm xử lý logic chính
void initEcgTask(PublishTask* publisher);          // hàm khởi tạo tasks, đã sửa
void setupECGConfiguration(const ECGConfig &config); // cấu hình ECG
void printECGData(const ECGData &data);        // in dữ liệu ECG ra Serial
uint16_t readRawECGValue();                         // đọc giá trị ADC thô
float convertADCToVoltage(uint16_t adc_value);      // chuyển đổi ADC sang điện áp
uint8_t checkLeadOffStatus();                  // kiểm tra trạng thái lead-off

#endif