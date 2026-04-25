#include "flex_sensor.h"

// 1. Define giá trị mặc định
#define FLEX_SENSOR_PIN 32
#define FLEX_SAMPLING_PERIOD_MS 50     // 20Hz
#define FLEX_BLOCK_DURATION_MS 3000    // gom 3 giây
#define FLEX_MAX_SAMPLES 64            // 3 giây / 50ms = 60 mẫu, lấy dư 64

// 2. Biến cấu hình toàn cục
static FlexSensorConfig flex_config = {
    .adc_pin = FLEX_SENSOR_PIN,
    .sampling_period_ms = FLEX_SAMPLING_PERIOD_MS,
    .block_duration_ms = FLEX_BLOCK_DURATION_MS,
    .max_samples_per_block = FLEX_MAX_SAMPLES,
    .ema_alpha = 0.9f
};

// 3. Biến trạng thái
static float filtered_flex_value = 0.0f;
static bool ema_initialized = false;

static FlexDataBlock current_block = {0};
static bool block_initialized = false;

// 3.1 Task chính
void TaskFlexSensor(void *pvParameters)
{
    // --------thêm--------
    PublishTask* publisher = (PublishTask*)pvParameters;
    //----------------------------

    uint32_t previous_wake_time = xTaskGetTickCount();

    // thêm để check bộ nhớ
    static uint32_t timer = millis();
    while (1)
    {
        // 1. Đọc ADC thô
        int raw_flex_value = readRawFlexValue();

        // 2. Lọc EMA nhẹ
        float filtered_value = applyEMALowPassFilter(raw_flex_value);

        // 3. Thêm mẫu vào block
        addSampleToFlexBlock(raw_flex_value, filtered_value);

        // 4. Nếu đủ block thì gửi
        if (isFlexBlockReadyToSend())
        {
            sendFlexDataBlock(publisher);
            resetFlexDataBlock();
        }

        if(millis() - timer > 2000)
        {
          timer = millis();
          Serial.print("--->[FLEX SENSOR] Stack con du: ");
          Serial.println(uxTaskGetStackHighWaterMark(NULL));
        }

        // 5. Giữ sampling ổn định
        vTaskDelayUntil(&previous_wake_time, pdMS_TO_TICKS(flex_config.sampling_period_ms));
    }
}

// 3.2 Khởi tạo task
void initFlexSensorTask(PublishTask* publisher)
{
    xTaskCreate(
        TaskFlexSensor,
        "Flex Sensor Task",
        4092,
        publisher, //------------sửa----------
        2,
        NULL
    );
}

// 3.3 Cấu hình
void setupFlexSensorConfiguration(const FlexSensorConfig &config)
{
    flex_config.adc_pin = config.adc_pin;
    flex_config.sampling_period_ms = config.sampling_period_ms;
    flex_config.block_duration_ms = config.block_duration_ms;
    flex_config.max_samples_per_block = config.max_samples_per_block;
    flex_config.ema_alpha = config.ema_alpha;
}

// 3.4 Đọc ADC thô
int readRawFlexValue()
{
    return analogRead(flex_config.adc_pin);
}

// 3.5 Lọc EMA
float applyEMALowPassFilter(int raw_value)
{
    if (!ema_initialized)
    {
        filtered_flex_value = (float)raw_value;
        ema_initialized = true;
    }

    filtered_flex_value =
        (flex_config.ema_alpha * filtered_flex_value) +
        ((1.0f - flex_config.ema_alpha) * (float)raw_value);

    return filtered_flex_value;
}

// 3.6 Thêm mẫu vào block
void addSampleToFlexBlock(int raw_value, float filtered_value)
{
    uint32_t current_time_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;

    if (!block_initialized)
    {
        current_block.block_start_time_ms = current_time_ms;
        current_block.sampling_period_ms = flex_config.sampling_period_ms;
        current_block.sample_count = 0;
        block_initialized = true;
    }

    if (current_block.sample_count < flex_config.max_samples_per_block)
    {
        uint16_t index = current_block.sample_count;

        current_block.samples[index].timestamp_ms = current_time_ms;
        current_block.samples[index].raw_adc_value = raw_value;
        current_block.samples[index].filtered_value = filtered_value;

        current_block.sample_count++;
    }
}

// 3.7 Kiểm tra block sẵn sàng gửi
bool isFlexBlockReadyToSend()
{
    if (!block_initialized)
    {
        return false;
    }

    uint32_t current_time_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
    uint32_t elapsed_time_ms = current_time_ms - current_block.block_start_time_ms;

    // Gửi khi đủ thời gian hoặc đầy buffer
    if (elapsed_time_ms >= flex_config.block_duration_ms)
    {
        return true;
    }

    if (current_block.sample_count >= flex_config.max_samples_per_block)
    {
        return true;
    }

    return false;
}

// 3.8 Gửi / in block dữ liệu
void sendFlexDataBlock(PublishTask* publisher)
{
    // ----------thêm----------
    if (publisher == nullptr) return;

    MQTTMessage msg;

    // topic
    strncpy(msg.topic, "esp32/flex", sizeof(msg.topic) - 1);
    msg.topic[sizeof(msg.topic) - 1] = '\0';

    msg.type = PAYLOAD_FLOAT;

    //  giới hạn do MQTTMessage chỉ có 32 phần tử
    uint16_t count = current_block.sample_count;
    if (count > 32) count = 32;

    msg.size = count;

    for (uint16_t i = 0; i < count; i++)
    {
        msg.data.float_data[i] = current_block.samples[i].filtered_value;
    }

    publisher->send(msg);

    Serial.print("--->[FLEX SENSOR] Sent block size: ");
    Serial.println(count);

    //---------------------------------------------

    // Hiện tại debug bằng Serial
    // Sau này có thể thay bằng MQTT publish / HTTP POST

    Serial.println("===== FLEX DATA BLOCK START =====");
    Serial.print("block_start_time_ms: ");
    Serial.println(current_block.block_start_time_ms);

    Serial.print("sampling_period_ms: ");
    Serial.println(current_block.sampling_period_ms);

    Serial.print("sample_count: ");
    Serial.println(current_block.sample_count);

    for (uint16_t i = 0; i < current_block.sample_count; i++)
    {
        Serial.print(current_block.samples[i].timestamp_ms);
        Serial.print(" | ");
        Serial.print(current_block.samples[i].raw_adc_value);
        Serial.print(" | ");
        Serial.println(current_block.samples[i].filtered_value, 2);
    }

    Serial.println("--->[FLEX DATA BLOCK END]: ");
}

// 3.9 Reset block
void resetFlexDataBlock()
{
    current_block.block_start_time_ms = 0;
    current_block.sampling_period_ms = flex_config.sampling_period_ms;
    current_block.sample_count = 0;
    block_initialized = false;
}