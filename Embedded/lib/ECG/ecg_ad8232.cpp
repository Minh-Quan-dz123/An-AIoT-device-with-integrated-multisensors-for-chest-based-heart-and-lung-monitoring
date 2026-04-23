//1 include thư viện/header
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ecg_ad8232.h"

// 2 define các giá trị cần thiết, setup giá trị cho các biến ban đầu
#define ECG_PIN 34                          // GPIO34 (D34) - ADC pin cho tín hiệu ECG từ AD8232
#define ECG_SAMPLING_PERIOD_MS 5            // Chu kỳ lấy mẫu 5ms (tần số ~200Hz)
#define ADC_MAX_VALUE 4095                  // Giá trị ADC tối đa (12-bit)
#define REFERENCE_VOLTAGE 3300              // Điện áp tham chiếu (mV)
#define LEAD_OFF_PIN_DISABLED 0xFF          // Giá trị sentinel khi không dùng lead-off

// Biến toàn cục lưu cấu hình ECG
static ECGConfig ecg_config = {
    .buffer_size = 1000,
    .sampling_period_ms = ECG_SAMPLING_PERIOD_MS,
    .adc_pin = ECG_PIN,
    .lead_off_pin_pos = LEAD_OFF_PIN_DISABLED,
    .lead_off_pin_neg = LEAD_OFF_PIN_DISABLED
};

// 3 code các hàm

// 3.1. Hàm xử lý tín hiệu cho ECG
// Đây là hàm chính được hệ điều hành FreeRTOS quản lý
void TaskECG(void *pvParameters) 
{
    //------------------- thêm------------
    PublishTask* publisher = (PublishTask*)pvParameters;
    //-----------------------------------------

    // Lấy thời điểm hiện tại làm mốc thời gian gốc cho vòng lặp
    uint32_t previous_wake_time = xTaskGetTickCount();
    
    // ---------thêm counter để giảm số lần print dữ liệu---------
    static uint32_t timer = millis();
    //--------------------------------------------------

    while (1) 
    {
        // 1. Đọc giá trị ADC từ cảm biến AD8232
        uint16_t raw_ecg_value = readRawECGValue();
        
        // 2. Chuyển đổi giá trị số sang điện áp thực tế (mV)
        float ecg_voltage_mv = convertADCToVoltage(raw_ecg_value);
        
        // 3. Kiểm tra các điện cực có bị bong ra không (Lead-off detection)
        uint8_t lead_off = checkLeadOffStatus();
        
        // 4. Đóng gói dữ liệu vào struct kèm timestamp
        ECGData ecg_data = {
            .timestamp_ms = xTaskGetTickCount() * portTICK_PERIOD_MS,
            .voltage_mv = ecg_voltage_mv,
            .raw_adc_value = raw_ecg_value,
            .lead_off_status = lead_off
        };

        // --------------4.5. MQTT send từng số 1-----------
        if (publisher != nullptr)
        {
            MQTTMessage msg;

            strcpy(msg.topic, "esp32/ecg");
            msg.type = PAYLOAD_FLOAT;
            msg.size = 3;

            // dùng static để tránh lỗi bộ nhớ
            static float payload[3];

            payload[0] = ecg_data.voltage_mv;
            payload[1] = ecg_data.raw_adc_value;
            payload[2] = ecg_data.lead_off_status;

            publisher->send(msg);
        }
        //----------------------------------
        
        //----------------- 5. Debug dữ liệu qua Serial (Plotter hoặc Monitor)
        if ( millis() - timer >= 2000) 
        {
            //printECGData(ecg_data);
            timer = millis();
            Serial.print("--->[ECG AD8232 Task] Stack con du: ");
            Serial.println(uxTaskGetStackHighWaterMark(NULL));
        }
        //------------------------------------------------
        
        // Duy trì tần số lấy mẫu ổn định tuyệt đối bằng cách bù trừ thời gian thực thi code
        vTaskDelayUntil(&previous_wake_time, pdMS_TO_TICKS(ecg_config.sampling_period_ms));
    }
}

// 3.2. Hàm khởi tạo task ECG
void initEcgTask(PublishTask* publisher) 
{
    xTaskCreate(
        TaskECG,                // Hàm task sẽ chạy
        "ECG Task",             // Tên task (để debug)
        4092,                   // Kích thước stack (byte)
        publisher,              // Tham số truyền vào task (truyền publisher)
        2,                      // Độ ưu tiên (priority) - 2 là mức trung bình
        NULL                    // Handle của task
    );
}

// 3.3. Hàm cấu hình ECG
void setupECGConfiguration(const ECGConfig &config)
{
    ecg_config.adc_pin = config.adc_pin;
    ecg_config.sampling_period_ms = config.sampling_period_ms;
    ecg_config.buffer_size = config.buffer_size;
    ecg_config.lead_off_pin_pos = config.lead_off_pin_pos;
    ecg_config.lead_off_pin_neg = config.lead_off_pin_neg;
}

// 3.4. Hàm đọc giá trị ADC thô
uint16_t readRawECGValue()
{
    // Đọc giá trị từ GPIO34 (chân ADC cho OUTPUT của AD8232)
    // analogRead() tự động chuyển đổi ADC và trả về giá trị 0-4095
    uint16_t adc_value = analogRead(ecg_config.adc_pin);
    return adc_value;
}

// 3.5. Hàm chuyển đổi ADC sang điện áp (mV)
float convertADCToVoltage(uint16_t adc_value)
{
    // Công thức: Voltage (mV) = (ADC_value / ADC_MAX) * REFERENCE_VOLTAGE
    float voltage_mv = ((float)adc_value / (float)ADC_MAX_VALUE) * REFERENCE_VOLTAGE;
    return voltage_mv;
}

// 3.6. Hàm kiểm tra trạng thái lead-off
// Lead-off xảy ra khi electrode mất contact với da bệnh nhân
uint8_t checkLeadOffStatus()
{
    static bool lead_off_pins_initialized = false;
    bool lead_off = false;

    if (ecg_config.lead_off_pin_pos == LEAD_OFF_PIN_DISABLED && ecg_config.lead_off_pin_neg == LEAD_OFF_PIN_DISABLED) {
        // Chưa cấu hình chân lead-off: không kiểm tra được, giả định tín hiệu OK
        return 0;
    }

    if (!lead_off_pins_initialized) {
        if (ecg_config.lead_off_pin_pos != LEAD_OFF_PIN_DISABLED) {
            pinMode(ecg_config.lead_off_pin_pos, INPUT);
        }
        if (ecg_config.lead_off_pin_neg != LEAD_OFF_PIN_DISABLED) {
            pinMode(ecg_config.lead_off_pin_neg, INPUT);
        }
        lead_off_pins_initialized = true;
    }

    if (ecg_config.lead_off_pin_pos != LEAD_OFF_PIN_DISABLED) {
        lead_off = lead_off || (digitalRead(ecg_config.lead_off_pin_pos) == HIGH);
    }
    if (ecg_config.lead_off_pin_neg != LEAD_OFF_PIN_DISABLED) {
        lead_off = lead_off || (digitalRead(ecg_config.lead_off_pin_neg) == HIGH);
    }

    // Với module AD8232: chân LO+/LO- thường active HIGH khi electrode bị rời.
    return lead_off ? 1 : 0;
}

// 3.7. Hàm in dữ liệu ECG ra Serial Monitor
void printECGData(const ECGData &data)
{
    // Format: Timestamp | ADC_Value | Voltage_mV | LeadOff_Status
    Serial.print("--->[ECG AD8232]: ");
    Serial.print(data.timestamp_ms);
    Serial.print(" | ");
    Serial.print(data.raw_adc_value);
    Serial.print(" | ");
    Serial.print(data.voltage_mv, 2);  // In 2 chữ số thập phân
    Serial.print(" mV | ");
    Serial.println(data.lead_off_status ? "Lead OFF" : "Lead OK");
}