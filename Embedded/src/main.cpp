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
  ECGConfig ecg_config = { 34, 5, 1000, 35, 36 };
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
