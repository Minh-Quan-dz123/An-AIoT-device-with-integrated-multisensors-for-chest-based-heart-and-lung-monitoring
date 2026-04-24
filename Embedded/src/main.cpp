#include <Arduino.h>
#include "globals.h"
#include <wifi_Manager.h>
#include <connect_mqtt.h>
#include <publish_task.h>
#include <flex_sensor.h>
#include <ecg_ad8232.h>

// 1 wifi task
WiFiManagerTask wifiManager;

// 2 task giám sát wifi
void systemMonitorTask(void* param)
{
  uint32_t lastPrint = 0;
  uint32_t checkWiFi = 0;
  bool reconnecting = false;

  while (1)
  {
    // 2.1 in trạng thái mỗi 3s
    if (millis() - lastPrint >= 3000)
    {
      lastPrint = millis();

      switch (g_wifiState)
      {
        case WIFI_INIT: Serial.println("--->[WIFI STATE] INIT"); break;
        case WIFI_CONNECTING: Serial.println("--->[WIFI STATE] CONNECTING"); break;
        case WIFI_CONNECTED: Serial.println("--->[WIFI STATE] CONNECTED"); break;
        case WIFI_AP_MODE: Serial.println("--->[WIFI STATE] AP_MODE"); break;
        case WIFI_FAIL: Serial.println("--->[WIFI STATE] FAIL"); break;
      }
    }

    // 2.2 check wifi
    if(millis() - checkWiFi >= 1000)
    {
      checkWiFi = millis();
      
      /// check stack
      //Serial.print("--->[WIFI Monitor] Stack con du: ");
      //Serial.println(uxTaskGetStackHighWaterMark(NULL));
      
      if (WiFi.status() != WL_CONNECTED)
      {
        Serial.println("--->[WIFI STATE] WiFi lost connection!");
        g_wifiState = WIFI_CONNECTING;
        // 2.2.1 tránh gọi begin liên tục
        if (!reconnecting)
        {
          reconnecting = true;
          Serial.println("--->[WIFI SYSTEM] Reconnecting WiFi...");
          wifiManager.begin(); // restart toàn bộ flow
        }
      }
      else
      {
        g_wifiState = WIFI_CONNECTED;
        reconnecting = false; // reset cờ khi đã ổn
      }
      
    }
        
    vTaskDelay(200 / portTICK_PERIOD_MS);
  }
}

// 3 tạo object MQTT
MQTTTask mqttClient(
    "60294ba1a7534e358c2dc4bc7b7cc9f9.s1.eu.hivemq.cloud",
    8883,
    "MinhQuan225386",
    "12348765@Mq"
);

// 4 tạo object publish mqtt
MQTTBuffer mqttBuffer;
PublishTask publisher(&mqttClient, &mqttBuffer);

// setup
void setup() 
{
  Serial.begin(115200);
  delay(200);

  // Nếu dùng chân LO+/LO- của AD8232, cấu hình cả hai chân vào đây
  // ECGConfig ecg_config = {1000, 5, 34, 35, 36};
  ECGConfig ecg_config;
    ecg_config.buffer_size = 1000;
    ecg_config.sampling_period_ms = 5;    // 5ms = 200Hz
    ecg_config.adc_pin = 34;              // GPIO34
    ecg_config.lead_off_pin_pos = 35;     // GPIO35 (LO+)
    ecg_config.lead_off_pin_neg = 36;     // GPIO36 (LO-)
  setupECGConfiguration(ecg_config);


  // 1 khởi động wifi task
  //wifiManager.begin(); // khởi động hệ thống WiFi manager, ko cần gọi, task dưới tự gọi
  // 1.1 tạo task theo dõi trạng thái wifi
  xTaskCreate(systemMonitorTask,"PrintWiFiState",2048,NULL,2,NULL);

  // 2 khởi động mqtt client
  mqttClient.begin();

  // 3 khởi động publisher mqtt
  mqttBuffer.begin(100);
  publisher.begin();


  // 4 chạy các sensor task
  initFlexSensorTask(&publisher);
  initEcgTask(&publisher);
}

void loop() 
{

}

