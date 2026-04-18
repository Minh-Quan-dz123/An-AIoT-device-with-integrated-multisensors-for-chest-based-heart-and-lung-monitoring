# 📘 HƯỚNG DẪN ESP32 + FreeRTOS + Flex Sensor

## 1. 🎯 Mục tiêu 

Hoàn hiện folder FlexSensor/ chứa "các thư viện tự định nghĩa"

- Đọc tín hiệu ECG từ cảm biến AD8232
- Xử lý tín hiệu theo thời gian thực
- Chạy bằng FreeRTOS (đa nhiệm)

---

## 2. 🧠 Kiến trúc hệ thống

Hệ thống gồm 3 phần chính:

### 🟢 Hardware
- Cảm biến Flex Sensor
- ESP32 (ADC input) 

### 🟡 Software (FreeRTOS)
- TaskFlexSersor: đọc và xử lý tín hiệu Flex Sersor

### 🔵 Application
- Hiển thị Serial Monitor (tạm thời )
- Gửi dữ liệu ra ngoài (nếu cần)

---

## 3. 📁 Các file cần làm (PlatformIO)
- tất cả các file ở folder  lib/FlexSensor/ có thể tạo thêm các file .h hay .cpp để chia project/tổ chức code cho dễ hiểu (nếu cần)

---

## 4. ⚙️ Nguyên tắc lập trình

- `.h` → chỉ khai báo (KHÔNG code logic)
- `.cpp` → viết toàn bộ logic
- `main.cpp` → chỉ gọi hàm khởi tạo task
- FreeRTOS → mọi xử lý nằm trong task

---

## 5. 🔌 Kết nối AD8232 với ESP32

| Flex Sensor | ESP32 |
|-------------|------|
|   OUTPUT    | GPIO32 (D32) |
|     GND     | GND |
|    3.3V     | 3.3V|


---

## 6. 🧵 FreeRTOS Task ECG

### 📌 Nguyên tắc:
- Task chạy liên tục trong `while(1)`
- Không dùng `delay()` → dùng `vTaskDelay()`
- Sampling ổn định ~100–500Hz

---

## 7. 🔄 Luồng hoạt động
main.cpp
->
initFlexTask()
->
xTaskCreate(TaskFlexSenSor)
->
TaskSenSor chạy liên tục:
- đọc ADC
- xử lý tín hiệu
- gửi Serial / buffer (tạm thời là vậy, sau này sẽ gửi tín hiệu lên server)


---

## 8. 💻 Code TaskFlexSensor (cốt lõi)

```cpp (ví dụ)
void TaskFlexSensor(void *pvParameters) 
{
    const int ecgPin = 34;

    while (1) 
    {
        // 1. Đọc tín hiệu điện áp
        int ecgValue = analogRead(flexPin);

        // 2. Xuất dữ liệu debug
        Serial.println(flexValue);

        // 3. Delay để giữ tần số lấy mẫu
        vTaskDelay(5 / portTICK_PERIOD_MS); 
    }
}

void initEcgTask(void *pvParameters) 
{
    xTaskCreate(
        TaskFlexSensor,
        "Flex Sensor Task",
        4096,
        NULL,
        2,
        NULL
    );
}

#include "ecg_ad8232.h"

void setup() 
{
    Serial.begin(115200);

    // Khởi tạo task ECG
    initEcgTask(NULL);
}

void loop() 
{
    // Không dùng loop vì FreeRTOS đã xử lý
}