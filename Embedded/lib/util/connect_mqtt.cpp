#include "ConnectHiveMQ.h"

#include "wifiControl.h"

// controller
#include "DHT11Control.h"
#include "soil.h"

#include "scheduleControl.h"
#include "cycleControl.h"

#include "DS3231Control.h"

#include "PumpControl.h"

#include "WateringService.h"

#include "device_identity.h"



const char* mqtt_server ="60294ba1a7534e358c2dc4bc7b7cc9f9.s1.eu.hivemq.cloud";//"60294ba1a7534e358c2dc4bc7b7cc9f9.s1.eu.hivemq.cloud";
const uint16_t mqtt_port = 8883; // tên port
const char* mqtt_user = "MinhQuan225386";
const char* mqtt_pass = "12348765@Mq";
const char* esp_id = deviceIdentity.deviceId.c_str(); // id của esp

bool emerTurnOff = false;


char topic1[64]; // device/{device_id}/connection/request/{"is_connected"}:                                               subcribe: 
char topic2[64]; // device/{device_id}/connection/response/{"ok"}:                                                        publish: gửi trạng thái kết nối và điều kiện tưới cây
char topic3[64]; // device/{device_id}/realtime/request/{"get"}:                                                          publish: esp yêu cầu gửi thời gian hiện tại tới HiveMQ
char topic4[64]; // device/{device_id}/realtime/response/{"year":2024,"month":6,"day":1,"hour":12,"minute":0,"second":0}: subcribe:server gửi thời gian hiện tại tới HiveMQ
char topic5[64]; // device/{device_id}/telemetry/{"temp":0, "humi":0, "soli":0}:                                          publish: gửi điều kiện hiện tại tới HiveMQ
char topic6[64]; // device/{device_id}/logs/{"mode": 1, "duration": 3, "minute": 30, "second":0}:                         publish: gửi log
char topic7[64]; // device/{device_id}/mode/request/{mode}:                                                           subcribe: yêu cầu thay đổi chế độ tưới cây                                 
char topic8[64]; // device/{device_id}/mode/response/{mode}:                                                          publish: gửi trạng thái chế độ tưới cây
char topic9[64]; // device/{device_id}/schedules/add/request/{"repeat":0, "duration":0,"dayOfWeek":0,"hour":15,"minute":20,"second":30}: subcribe: yêu cầu thêm 1 lịch tưới cây mới
char topic10[64];// device/{device_id}/schedules/add/response/{"ok"}:                                                                     publish: phản hồi đã thêm lịch tưới cây mới thành công hay chưa                  
char topic11[64];// device/{device_id}/schedules/delete/request/{"repeat":0,"hour":15,"minute":20,"second":30}:           subcribe: yêu cầu xóa 1 lịch tưới cây 
char topic12[64];// device/{device_id}/schedules/delete/response/{"ok"}:                                                  publish: phản hồi đã xóa lịch tưới cây thành công hay chưa
char topic13[64];// device/{device_id}/ebw_mode/request/{"lower_soil":0,"duration":0}:                                    subcribe: yêu cầu thay đổi chế độ tưới cây theo điều kiện sinh trưởng của cây trồng
char topic14[64];// device/{device_id}/ebw_mode/response/{"ok"}:                                                          publish: phản hồi đã thay đổi chế độ tưới cây theo điều kiện sinh trưởng của cây trồng thành công hay chưa
char topic15[64];// device/{device_id}/on_demand_watering/request/{"duration":0}:                                         subcribe: yêu cầu tưới cây ngay với thời gian tưới là duration 
char topic16[64];// device/{device_id}/on_demand_watering/request/{"turn_off"}:   hoặc duration <= 0                                        subcribe: yêu cầu tắt ngay chế độ tưới cây đang hoạt động
char topic17[64];// device/{device_id}/on_demand_watering/response/{"ok"}:                                                publish: phản hồi đã tưới cây ngay với thời gian tưới là duration thành công hay chưa

WiFiClientSecure espClient;
MqttClient client(espClient);

// 0 setup MQTT topics
void setupMQTT()
{
  snprintf(topic1, sizeof(topic1), "device/%s/connection/request", esp_id);
  snprintf(topic2, sizeof(topic2), "device/%s/connection/response", esp_id);
  snprintf(topic3, sizeof(topic3), "device/%s/realtime/request", esp_id);
  snprintf(topic4, sizeof(topic4), "device/%s/realtime/response", esp_id);
  snprintf(topic5, sizeof(topic5), "device/%s/telemetry", esp_id);
  snprintf(topic6, sizeof(topic6), "device/%s/logs", esp_id);
  snprintf(topic7, sizeof(topic7), "device/%s/mode/request", esp_id);
  snprintf(topic8, sizeof(topic8), "device/%s/mode/response", esp_id);
  snprintf(topic9, sizeof(topic9), "device/%s/schedules/add/request", esp_id);
  snprintf(topic10, sizeof(topic10), "device/%s/schedules/add/response", esp_id);
  snprintf(topic11, sizeof(topic11), "device/%s/schedules/delete/request", esp_id);
  snprintf(topic12, sizeof(topic12), "device/%s/schedules/delete/response", esp_id);
  snprintf(topic13, sizeof(topic13), "device/%s/ebw_mode/request", esp_id);
  snprintf(topic14, sizeof(topic14), "device/%s/ebw_mode/response", esp_id);
  snprintf(topic16, sizeof(topic16), "device/%s/on_demand_watering/request", esp_id);// 15-16 là 1, cách nhau nội dung
  snprintf(topic17, sizeof(topic17), "device/%s/on_demand_watering/response", esp_id);

  // in ra các topic để kiểm tra
  Serial.println("cac topic da duoc setup: ");
  Serial.println(topic1);
  Serial.println(topic2);
  Serial.println(topic3);
  Serial.println(topic4);
  Serial.println(topic5);
  Serial.println(topic6);
  Serial.println(topic7);
  Serial.println(topic8);
  Serial.println(topic9);
  Serial.println(topic10);
  Serial.println(topic11);
  Serial.println(topic12);
  Serial.println(topic13);
  Serial.println(topic14);
  Serial.println(topic16);
  Serial.println(topic17);


  reconnectMQTT();// gọi để đảm bảo kết nối với HiveMQ ngay sau khi setupMQTT được gọi
}

// 1 gửi conditions --- > topic5
void sendDataToHiveMQ(float temp, float hum, float soil)
{
  // tạo json 
  char condition[60];
  sprintf(condition, "{\"temp\": %.2f, \"humi\": %.2f, \"soil\": %.2f}", temp, hum, soil);

  client.beginMessage(topic5, false, 0);  // bắt đầu gửi
  client.print(condition);      // gửi payload
  client.endMessage();          // kết thúc gửi
}

// 2 gửi log ---> topic6
void writeLog(uint8_t mode, uint16_t duration, uint8_t minute, uint8_t second)
{
  // tạo json
  char log[128];
  sprintf(log,
          "{\"mode\":%u,\"duration\":%u,\"minute\":%u,\"second\":%u}",
          mode, duration, minute, second);
  
  client.beginMessage(topic6,false,2);  // bắt đầu gửi
  client.print(log);                    // gửi payload
  client.endMessage();                  // kết thúc gửi

}


// 3 callback: được gọi khi nó nhận được 1 tin từ HiveMQ
void callback(int messageSize)
{
  // 1 đọc mess
  String topic = client.messageTopic();
  String msg = "";
  while(client.available()) {msg += (char)client.read();}
  msg.trim(); // cắt đoạn trắng đầu  cuối của string
  Serial.print("nhan duoc topic "); 
  Serial.print(topic); 
  Serial.print(": "); 
  Serial.println(msg);

  //2 tiến hành kiểm tra
  //2.1 nếu là topic 1,2: connection request
  if(topic.equals(topic1))
  {
    if(msg.equalsIgnoreCase("is_connected"))
    {
      initSoil(); // 1 bật soil
      loadEEPROM(); // 2 load để đặt flag lại cho eeprom
      Serial.println("da khoi tao esp");
    }
    else if(msg.equalsIgnoreCase("reset"))
    {
      // tắt dht11, soil, xóa LittleFS
      deleteSchedule(0,-1);// 1 xóa littleFS
      clearEEPROM();// 2 xóa eeprom
      status = 0; // 3 tắt tất cả trạng thái
      endSoil(); // 4 tắt soil
      Serial.println("da xoa du lieu");
    }

    // ---publish topic 2 để trả lời kết nối---
    client.beginMessage(topic2, false, 2); // trả lời lại ok đã nhận được yêu cầu kết nối
    client.print("ok");
    client.endMessage();
  }
  //----------------------------------------------------------

  //2.2 topic 4 nhận realtime từ server để đồng bộ thời gian
  else if(topic.equals(topic4))
  {

    Serial.println(" => nhan duoc topic4: ");
    JsonDocument doc; // tạo object json dài 150 byte để đọc dữ liệu
    DeserializationError error = deserializeJson(doc, msg);
    if(error) // kiểm tra có phải dưới JSON không
    {
      Serial.print("loi json tai topic3: ");
      Serial.println(error.c_str());
      return;
    }
    uint8_t second = (uint8_t)doc["second"];
    uint8_t minute = (uint8_t)doc["minute"];
    uint8_t hour = (uint8_t)doc["hour"];
    uint8_t day = (uint8_t)doc["day"];
    if(second >= 59)
    {
      second = 0;
      if(minute >= 59)
      {
        minute = 0;
        if(hour >= 23)
        {
          hour = 0;
          day++;
        }
        else hour++;
      }
      else minute++;
    }
    else second++;
    setRealTime((uint16_t)doc["year"], (uint8_t)doc["month"], day, hour, minute,second );
  }
  //-----------------------------------------------------------------

  //2.3 topic 7,8 - thay đổi mode tưới cây
  else if(topic.equals(topic7))
  {
    uint8_t newStatus = msg.toInt();
    if(newStatus == 3) {status = 0; return;}

    managerStatus(newStatus);
    Serial.println(" => nhan duoc topic7: ");

    // ---publish topic 8 để trả lời đã thay đổi chế độ tưới cây---
    client.beginMessage(topic8, false, 2); // trả lời lại ok đã nhận được yêu cầu thay đổi chế độ tưới cây
    client.print(String("{\"mode\":") + String(status) + String("}"));
    client.endMessage();
  }

  //--------------------------------------------------------------

  //2.4 topic 9,10 schedule add
  else if(topic.equals(topic9))
  {
    /*{"repeat":0,"dayOfWeek":0,"hour":15,"minute":20,"second":30,"time":3}*/
    Serial.println(" => nhan duoc topic9: ");
    JsonDocument doc; // tạo object json dài 150 byte để đọc dữ liệu
    DeserializationError error = deserializeJson(doc, msg);
    if(error) // kiểm tra có phải dưới JSON không
    {
      Serial.print("loi json tai topic9: ");
      Serial.println(error.c_str());
      return;
    }
    // đầu vào là thứ giờ phút giây => cần lưu => ngày giờ phút giây
    Schedule x;
    nowTime = rtc.now();

    // chuyển đồi từ thứ -> ngày
    if (doc["repeat"].is<int>()) 
    {
      // Server gửi số
      x.repeat = doc["repeat"].as<int>();
    }
    else if (doc["repeat"].is<const char*>()) 
    {
      // Server gửi chuỗi
      const char* repeatStr = doc["repeat"];

      if (strcmp(repeatStr, "once") == 0) {
        x.repeat = 0;
      } 
      else if (strcmp(repeatStr, "daily") == 0) {
        x.repeat = 1;
      } 
      else if (strcmp(repeatStr, "weekly") == 0) {
        x.repeat = 2;
      } 
      else {
        Serial.println("repeat khong hop le");
        x.repeat = 0; // mặc định an toàn
      }
    }
    else {
      Serial.println("repeat sai kieu du lieu");
      x.repeat = 0;
    }

    x.year = nowTime.year();
    x.month = nowTime.month();

    // chuyển đồi từ thứ -> ngày
    x.day = (uint8_t)doc["dayOfWeek"]; // 
    // (i,xi), (j,xj), cho i,xi,xj. tìm j
    // if(xj <= xi) => xi - xj =  i -j => j = i - xi + xj;
    // else => xj - xi = j - i => j = i - xi + xj
    // => x.day = nowTime.day() - nowTime.dayOfTheWeek() + x.day;
    x.day = nowTime.day() - nowTime.dayOfTheWeek() + x.day;
    // end

    x.hour = doc["hour"];
    x.minute = doc["minute"];
    x.second = doc["second"];
    x.wateringTime = doc["time"];
    

    Serial.print("repeat: "); Serial.print(x.repeat);
    Serial.print(" , year: "); Serial.print(x.year);
    Serial.print(" , month: "); Serial.print(x.month);
    Serial.print(" , day: "); Serial.print(x.day);
    Serial.print(" , hour: "); Serial.print(x.hour);
    Serial.print(" , minute: "); Serial.print(x.minute);
    Serial.print(" , second: "); Serial.print(x.second);
    Serial.print(" , time: "); Serial.println(x.wateringTime);
    addSchedule(x);

    // ---publish topic 10 để trả lời đã thêm lịch tưới cây mới thành công hay chưa---
    client.beginMessage(topic10, false, 2); // trả lời lại ok đã nhận được yêu cầu thêm lịch tưới cây mới
    client.print("ok");
    client.endMessage();
  }
  // ------------------------------------------------------------

  //2.5 topic11,12 schedue delete
  else if(topic.equals(topic11))
  {
    Serial.println(" => nhan duoc topic11: ");

    if(msg == "-1") // xóa hết
    {
      deleteSchedule(-1);
      return;
    }

    JsonDocument doc; // tạo object json dài 150 byte để đọc dữ liệu
    DeserializationError error = deserializeJson(doc, msg);
    if(error) // kiểm tra có phải dưới JSON không
    {
      Serial.print("loi json tai topic11: ");
      Serial.println(error.c_str());
      return;
    }
    // đầu vào là thứ giờ phút giây => cần lưu => ngày giờ phút giây
    nowTime = rtc.now();
    uint8_t x = doc["dayOfWeek"];
    x = (uint8_t)nowTime.day() - (uint8_t)nowTime.dayOfTheWeek() + x;

    uint8_t re=0;
    if (doc["repeat"].is<int>()) 
    {
      // Server gửi số
      re = doc["repeat"].as<int>();
    }
    else if (doc["repeat"].is<const char*>()) 
    {
      // Server gửi chuỗi
      const char* repeatStr = doc["repeat"];

      if (strcmp(repeatStr, "once") == 0) {
        re = 0;
      } 
      else if (strcmp(repeatStr, "daily") == 0) {
        re = 1;
      } 
      else if (strcmp(repeatStr, "weekly") == 0) {
        re = 2;
      } 
      else {
        Serial.println("repeat khong hop le");
        re = 0; // mặc định an toàn
      }
    }
    else {
      Serial.println("repeat sai kieu du lieu");
      re = 0;
    }

    deleteSchedule(re ,(uint8_t)doc["hour"], (uint8_t)doc["minute"], (uint8_t)doc["second"]);
  }

  //--------------------------------------------------------------------

  //2.6 nếu là topic 13, 14: yêu cầu thay đổi chế độ tưới cây theo điều kiện sinh trưởng
  else if(topic.equals(topic13))
  {
    Serial.println(" => nhan duoc topic13: ");
    // chuỗi gửi về là chuỗi JSON dạng: {temp,humi, lower_soil, duration: }
    JsonDocument doc; // tạo object json dài 50 byte để đọc dữ liệu
    DeserializationError error = deserializeJson(doc, msg);
    if(error) // kiểm tra có phải dưới JSON không
    {
      Serial.print("loi json tai topic13: ");
      Serial.println(error.c_str());
      return;
    }

    setBioCycle(0,0,(uint8_t)doc["lower_soil"], (uint16_t)doc["duration"]);

    // ---publish topic 14 để trả lời đã thay đổi chế độ tưới cây theo điều kiện sinh trưởng của cây trồng thành công hay chưa---
    client.beginMessage(topic14, false, 2); // trả lời lại ok đã nhận được yêu cầu thay đổi chế độ tưới cây theo điều kiện sinh trưởng của cây trồng
    client.print("ok");
    client.endMessage();
  }

  //2.7 nếu là topic 16: bật tắt máy bơm ngay lập tức
  else if(topic.equals(topic16))
  {
    // gán thời gian tưới
    // nếu msg là string "turn_off" thì tắt máy bơm ngay, nếu không thì gán thời gian tưới
    if(msg.equalsIgnoreCase("turn_off")|| msg.toInt() <= 0)// tắt máy bơm ngay khi nó ở mode bất ki
    {
      emerTurnOff = true;
    }
    else if(msg.toInt() > 0)// bật máy bơm ngay lập tức
    {
      wateringDuration = msg.toInt();
      status = 3;
    }
    // ---publish topic 17 để trả lời đã bật/tắt tưới cây ngay lập tức thành công hay chưa---
    client.beginMessage(topic17, false, 2); // trả lời
    client.print("ok");
    client.endMessage();

  }

  
}

// 4 hàm reconnect với hivemq
void reconnectMQTT()
{
  // nếu chưa kết nối thì kiểm tra
  uint8_t countConnect = 10;
  while(!client.connected() && countConnect > 0)
  {
    Serial.println("dang ket noi toi HiveMQ...");
    reconnectWifi();

    espClient.setInsecure();// ko kiểm tra chứng chỉ SSL
    client.setId(esp_id);
    client.setUsernamePassword(mqtt_user, mqtt_pass);
  
    //xong rồi kết nối tới server a
    if(client.connect(mqtt_server, mqtt_port))
    {
      Serial.println("da ket noi voi hivemq");
      client.onMessage(callback);// gán callback
      
      // Subscribe tất cả topic
      const char* topics[] = {topic1, topic4, topic7, topic9, topic11, topic13,topic16};
      uint8_t numTopics = sizeof(topics) / sizeof(topics[0]);
      for(int i=0; i<numTopics; i++)
      {
        if(client.subscribe(topics[i], 2))
        {
          Serial.print("Subscribed: "); Serial.println(topics[i]);
        } 
        else 
        {
          Serial.print("Subscribe failed: "); Serial.println(topics[i]);
        }
      }
      // gửi response tới server bảo là đã kết nối
      client.beginMessage(topic3);  // bắt đầu gửi
      client.print("get");      // gửi payload
      client.endMessage();          // kết thúc gửi

      Serial.print("Da gui topic ");
      Serial.print(topic3);
      Serial.print(": ");
      Serial.println("get");
    }
    else
    {
      Serial.print("ket noi that bai: ");
      Serial.println(client.connectError());// lấy ra mã lỗi;
      delay(700); // chặn luồng để kết nối HiveMQ
      countConnect--;// cho thử 10 lần thôi
    }
    
  }
}











