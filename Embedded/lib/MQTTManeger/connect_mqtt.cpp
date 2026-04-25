#include "connect_mqtt.h"

// 1 constructor
MQTTTask::MQTTTask(String server,uint16_t port,String user,String pass): client(espClient)
{
    mqttServer = server;
    mqttPort = port;
    mqttUser = user;
    mqttPass = pass;

    // bật TLS bỏ kiểm tra cert
    espClient.setInsecure();

    // setting vào PubSubClient client;
    client.setServer(mqttServer.c_str(), mqttPort);

    // clientId của esp này
    clientId = "ESP32-Heard-Lung-" + String((uint32_t)ESP.getEfuseMac(), HEX);
}

// 2 hàm begin bắt đầu luồng
void MQTTTask::begin()
{
    xTaskCreate(taskWrapper,"MQTT_Task",4096,this,2,&mqttTaskHandle);
}
void MQTTTask::taskWrapper(void* param)
{
    MQTTTask* self = (MQTTTask*)param;
    self->taskLoop();
}

// 3 loop chính
void MQTTTask::taskLoop()
{
  static uint32_t lastReconnect = millis()-3000;
  static uint32_t lastCheck = millis();
  while(1)
  {
    // 1. check WiFi
    if (WiFi.status() != WL_CONNECTED)
    { // nếu chưa kết nối thì đợi 
      vTaskDelay(500 / portTICK_PERIOD_MS);
      continue;
    }

    // 2. nếu wifi kết nối rồi thì check kết nối MQTT
    if (!client.connected())
    {
      // nếu chưa kết nối thì gọi kết nối lại
      if (millis() - lastReconnect > 5000) // 5 giây mới thử lại
      {
        lastReconnect = millis();
        reconnect();
      }
    }
    else 
    {
      if (millis() - lastCheck > 2000) // 5 giây mới thử lại
      {
        lastCheck = millis();
        Serial.println("--->[MQTT Connector Task] Connected to HiveMQ");
      }
    }
    // 3. keep alive MQTT
    client.loop();
    vTaskDelay(20 / portTICK_PERIOD_MS);
  }
}

// 4 hàm reconnect
void MQTTTask::reconnect()
{
  uint32_t start = millis();// kiểm soát timeout
  uint32_t timer = millis();

  // liên tục check connect chưa
  static uint8_t counter = 0;
  while (!client.connected())
  {
    if((millis()- timer) >= 2000)
    {
      timer = millis();
      Serial.print("--->[MQTT Connector Task] stask con du:");
      Serial.println(uxTaskGetStackHighWaterMark(NULL));
    }
    // 4.0 kiểm soát timeout
    if (millis() - start > 10000) 
    {
      //start = millis();
      Serial.println("--->[MQTT Connector Task] reconnect timeout");
      return;
    }
    // nếu chưa thì bắt đầu kết nối
    Serial.println("--->[MQTT Connector Task] connecting...");

    // 4.1 kết nối tới server
    if (client.connect(clientId.c_str(),mqttUser.c_str(),mqttPass.c_str()))
    {
      Serial.println("--->[MQTT Connector Task] connected");

      // ----------------------------subscribe topic-----------------------------
      client.subscribe("esp32/test");
      // ------------------------------------------------------------------------
    }
    else// 4.2 nếu ko kết nối được thì in ra lỗi
    {
        Serial.print("--->[MQTT Connector Task] failed, rc=");
        Serial.println(client.state());

        // block 2 giây để đợi
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

// 5 wrapper publish
void MQTTTask::publish(const char* topic, const char* message)
{
  if(client.connected()) client.publish(topic, message);
}

// 6 wrapper publish
void MQTTTask::subscribe(const char* topic)
{
  if(client.connected()) client.subscribe(topic);
}


// 8 is connect
bool MQTTTask::isConnected()
{
  return client.connected();
}