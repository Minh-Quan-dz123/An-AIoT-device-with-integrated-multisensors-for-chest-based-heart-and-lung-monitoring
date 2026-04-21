// 1. include các header để sử dụng hàm trong các header
#include <Arduino.h>
#include <ecg_ad8232.h>


// II.setup ban đầu
void setup() 
{
  Serial.begin(115200);
  
  // Chờ Serial kết nối ổn định
  delay(2000);
  
  Serial.println("========================================");
  Serial.println("ECG AD8232 + ESP32 + FreeRTOS");
  Serial.println("========================================");
  Serial.println("Initializing ECG Task...");

  // Nếu dùng chân LO+/LO- của AD8232, cấu hình cả hai chân vào đây
  // ECGConfig ecg_config = {1000, 5, 34, 35, 36};
  ECGConfig ecg_config;
    ecg_config.buffer_size = 1000;
    ecg_config.sampling_period_ms = 5;    // 5ms = 200Hz
    ecg_config.adc_pin = 34;              // GPIO34
    ecg_config.lead_off_pin_pos = 35;     // GPIO35 (LO+)
    ecg_config.lead_off_pin_neg = 36;     // GPIO36 (LO-)
  setupECGConfiguration(ecg_config);

  // 1. Gọi hàm tạo và chạy Task ECG để đọc dữ liệu từ cảm biến AD8232
  initEcgTask(NULL);
  
  Serial.println("ECG Task initialized!");
  Serial.println("========================================\n");
}

// Dùng hệ điều hành FreeRTOS nên không cần loop nữa
void loop() 
{
  // FreeRTOS đã quản lý tất cả tasks, hàm loop không cần làm gì
}
