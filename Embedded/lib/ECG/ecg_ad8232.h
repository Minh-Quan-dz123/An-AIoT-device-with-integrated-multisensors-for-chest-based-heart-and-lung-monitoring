#ifndef AD8232_TASKS_H
#define AD8232_TASKS_H

//1. khai báo các thư viện cần để sử dụng
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

//2. Khai báo các biến, struct của header này
// Struct đại diện cho dữ liệu ECG
struct ECGData {
    uint32_t timestamp_ms;     // thời gian lấy mẫu (ms)
    int raw_adc_value;         // giá trị ADC thô
    float voltage_mv;          // điện áp (mV)
    uint8_t lead_off_status;   // trạng thái lead-off (1: mất tín hiệu, 0: có tín hiệu)
};

// Cấu hình ECG
struct ECGConfig {
    uint8_t adc_pin;           // chân ADC để đọc
    uint16_t sampling_period_ms; // chu kỳ lấy mẫu (ms)
    uint32_t buffer_size;      // kích thước bộ đệm (nếu cần)
    uint8_t lead_off_pin_pos;  // chân LEAD-OFF LO+ (nếu có)
    uint8_t lead_off_pin_neg;  // chân LEAD-OFF LO- (nếu có)
};

//3. khai báo (nhớ là chỉ khai báo thôi) các hàm tự định nghĩa
void TaskECG(void *pvParameters);              // hàm xử lý logic chính
void initEcgTask(void *pvParameters);          // hàm khởi tạo tasks
void setupECGConfiguration(const ECGConfig &config); // cấu hình ECG
void printECGData(const ECGData &data);        // in dữ liệu ECG ra Serial
int readRawECGValue();                         // đọc giá trị ADC thô
float convertADCToVoltage(int adc_value);      // chuyển đổi ADC sang điện áp
uint8_t checkLeadOffStatus();                  // kiểm tra trạng thái lead-off

#endif
