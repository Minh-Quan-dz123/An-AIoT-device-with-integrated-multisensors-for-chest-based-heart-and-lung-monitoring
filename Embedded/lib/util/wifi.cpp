#include <Arduino.h>
#include "wifi.h"

ESP8266WebServer server(80);

String wifi_ssid = "";
String wifi_pass = "";
String device_secret = "";

bool wifiConfigured = false;
const char* espId = deviceIdentity.deviceId.c_str();

struct WifiConfig {
  String ssid;
  String password;
};

#define MAX_WIFI_CONFIGS 4
WifiConfig wifiConfigs[MAX_WIFI_CONFIGS];
int wifiConfigCount = 0;


void setWifi()
{
  Serial.println("\n--- chưa có wifi thì bật chế độ AP ---");

  WiFi.mode(WIFI_AP_STA);

  // 1. Phát WiFi cho điện thoại kết nối
  WiFi.softAP("ESP8266_Config_AP_"+String(espId));
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  // 2. Trang nhập WiFi
  server.on("/", HTTP_GET, []() {
     server.send(200, "text/html", R"rawliteral(
      <!DOCTYPE html>
      <html>
      <head>
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <style>
      body{
        font-family:Arial;
        background:#f2f2f2;
        text-align:center;
        margin:0;
      }
      .box{
        background:white;
        margin:40px auto;
        padding:20px;
        width:260px;
        border-radius:6px;
        box-shadow:0 2px 6px rgba(0,0,0,0.15);
      }
      h2{
        margin-top:0;
        color:#2f7ed8;
      }
      input{
        width:100%;
        padding:8px;
        margin:6px 0;
        border:1px solid #ccc;
        border-radius:4px;
      }
      button{
        width:100%;
        padding:10px;
        background:#2f7ed8;
        color:white;
        border:none;
        border-radius:4px;
        font-size:15px;
      }
      button:active{
        background:#1f5fa8;
      }
      </style>
      </head>

      <body>
      <div class="box">
      <h2>ESP8266 Setup</h2>

      <form action="/save" method="POST">

        <input name="ssid" placeholder="WiFi SSID" required>

        <input name="pass" type="password" placeholder="Password">

        <input name="device_secret" placeholder="Device Secret">

        <button type="submit">Connect</button>
      </form>

      </div>

      </body>
      </html>
      )rawliteral");

      });

  // 3. Nhận ssid và password
  server.on("/save", HTTP_POST, []() {
    wifi_ssid = server.arg("ssid");
    wifi_pass = server.arg("pass");
    device_secret = server.arg("device_secret");

    server.send(200, "text/html",
      "<h3>Saved! Connecting...</h3>");

    wifiConfigured = true;// đã được cấu hình
  });

  server.begin();
  Serial.println("Web server started");

  // 4. Chờ user nhập WiFi trong tối đa 2 phút
  uint32_t start = millis();
  while (!wifiConfigured && (millis() - start < 120000)) {
    server.handleClient();
    delay(10);
  }

  server.stop();// qua vòng while thì dùng server

  // nếu chưa được cấu hình thì bỏ qua, cho esp làm việc khác sau đó tự loop quay lại
  if(!wifiConfigured)
  {
    Serial.println("Khong duoc cau hinh wifi trong 2 phut, chuyen sang che do AP de cau hinh lai");
    return;
  }

  // 5. Kết nối WiFi nhà mà user đã nhập
  WiFi.softAPdisconnect(true);
  WiFi.begin(wifi_ssid.c_str(), wifi_pass.c_str());

  Serial.println("Dang ket noi WiFi nha...");

  uint32_t countConnect = 16; // kết nối trong countCount/2 giây
  while (WiFi.status() != WL_CONNECTED && countConnect > 0) {
    delay(500); // 8 giây
    Serial.print(".");
    countConnect--;
  }
  if (WiFi.status() != WL_CONNECTED) 
  {// nếu kết nối thất bại thì bỏ cuộc, cho esp làm việc khác sau đó tự loop quay lại
    Serial.println("\n--- KET NOI THAT BAI ---");
    wifiConfigured = false; // cấu hình lại từ đầu
    WiFi.disconnect(true);
    return;
  }

  Serial.println("\n--- KET NOI THANH CONG ---");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  // lưu wifi và device secret vào flash
  saveWifiConfig();

}


// lưu tối đa 4 ssid cùng lúc
/*định dạng file wifi_config.txt
ssid1|password1
ssid2|password2
ssid3|password3
ssid4|password4*/
void loadWifiConfig() // gọi ở config.cpp, bước đầu tiên trong setup() để load vào RAM, sau đó mới thử kết nối wifi trong list
{
  if(!LittleFS.begin())
  {
    Serial.println("ko begin duoc littleFS de load wifi_config.txt");
    return;
  }
  // 1. Load file cũ nếu có
  if(LittleFS.exists("/wifi_config.txt"))
  {
    File file = LittleFS.open("/wifi_config.txt", "r");
    if(file)
    {
      String line;
      int index = 0;
      while(file.available() && index < MAX_WIFI_CONFIGS)
      {
        line = file.readStringUntil('\n');
        line.trim();

        int sep = line.indexOf('|');// sep là vị trí phân cách giữa ssid và password (vị trí của "|")

        if(sep > 0)
        {
          wifiConfigs[index].ssid = line.substring(0, sep); // cắt lấy ssid
          wifiConfigs[index].password = line.substring(sep + 1); // cắt lấy password
          index++;
        }
        
      }
      wifiConfigCount = index;
      file.close();

      Serial.println("Loaded wifi:");
      for (int i = 0; i < wifiConfigCount; i++)
      {
        Serial.println(wifiConfigs[i].ssid);
      }
    }
  }
}

// lưu vào bộ nhớ RAM, sau đó lưu vào flash sau
void saveWifiConfig()
{
  if(!LittleFS.begin())
  {
    Serial.println("LittleFS begin failed");
    return;
  }

  //1. nếu wifi này đã có ssid rồi thì update password, nếu chưa thì thêm vào danh sách
  bool found = false;
  for(int i = 0; i < wifiConfigCount; i++)
  {
    if(wifiConfigs[i].ssid == wifi_ssid)
    {
      wifiConfigs[i].password = wifi_pass;
      found = true;
      break;
    }
  }

  // 2. nếu chưa thì thêm mới
  if(!found)
  {
    if(wifiConfigCount < MAX_WIFI_CONFIGS)
    {
      // lưu vào index tiếp theo
      wifiConfigs[wifiConfigCount].ssid = wifi_ssid;
      wifiConfigs[wifiConfigCount].password = wifi_pass;
      wifiConfigCount++;
    }
    else // nếu đã đủ rồi thì ghi đè vào wifi đầu tiên
    {
      for(int i = 0; i < MAX_WIFI_CONFIGS-1; i++)// 0,1,2 -> 1,2,3
      {
        wifiConfigs[i+1] = wifiConfigs[i];
      }
      wifiConfigs[0].ssid = wifi_ssid;
      wifiConfigs[0].password = wifi_pass;
    }
  }
  
  // 3. lưu vào flash
  File file = LittleFS.open("/wifi_config.txt", "w");
  if(file)
  {
    for(int i = 0; i < wifiConfigCount; i++)
    {
      file.println(wifiConfigs[i].ssid + "|" + wifiConfigs[i].password);
    }
    file.close();
    Serial.println("da luu wifi moi vao flash");
  }
  else
  {
    Serial.println("loi mo wifi_config.txt de viet"); 
  }
}

// bước đầu kết nối wifi trong list, mỗi wifi thử 3s (3 wifi thì 9s)
// nếu ko được thì quay lại AP config
void reconnectWifi()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("WiFi da ket noi");
    return;
  }

  // 1 load wifi từ flash vào RAM trước 
  
  if (wifiConfigCount == 0) // nếu k có thì AP luôn
  {
    Serial.println("Khong co wifi nao trong flash");
    setWifi(); // quay về AP
    return;
  }

  // 2 nếu có list thì bắt đầu scan xung quanh và kết nối các wifi có trong danh sách
  Serial.println("Dang ket noi lai WiFi trong list...");
  WiFi.disconnect(true); // ngắt kết nối trước khi thử lại
  WiFi.mode(WIFI_STA); // đảm bảo ở chế độ STA
  WiFi.persistent(false); // không lưu cấu hình mới vào flash trong quá trình thử kết nối lại

  // 2.1 bắt đầu quét
  Serial.println("Dang scan WiFi xung quanh...");
  int n = WiFi.scanNetworks(); // hàm blocking cờ 1-2s, tối đa 5-6s

  if(n <= 0)
  {
    Serial.println("Khong tim thay WiFi nao");
    setWifi(); // quay về AP
    return;
  }

  // nếu n>0 <=> đã lưu các ssid scan được trong cấu trúc dữ liệu nội bộ rồi
  // duyệt tối đa 20 wifi tìm thấy
  uint8_t countLimit = min(n,20);
  for(int i=0; i<countLimit; i++)
  {
    Serial.print(i); Serial.print(": ");
    Serial.print(WiFi.SSID(i)); Serial.print(" ("); Serial.print(WiFi.RSSI(i)); Serial.println("dBm)");

    String foundSSID = WiFi.SSID(i);
    for(int j=0; j<wifiConfigCount; j++)
    {
      if(foundSSID == wifiConfigs[j].ssid)
      {
        Serial.println("Tim thay trong list, thu ket noi...");
        WiFi.begin(wifiConfigs[j].ssid.c_str(), wifiConfigs[j].password.c_str(), WiFi.channel(i), WiFi.BSSID(i));

        uint8_t countConnect = 15; // kết nối trong countCount/4 giây
        while (WiFi.status() != WL_CONNECTED && countConnect > 0)
        {
          delay(200);
          Serial.print(".");
          countConnect--;
        }
        if (WiFi.status() == WL_CONNECTED)
        {
          Serial.println("\n--- KET NOI THANH CONG ---");
          Serial.print("IP: ");
          Serial.println(WiFi.localIP());

          WiFi.scanDelete(); // xóa kết quả scan để tiết kiệm RAM
          return;
        }
        else
        {
          Serial.println("\n--- KET NOI THAT BAI VOI WIFI " + wifiConfigs[j].ssid + " ---");
        }
      }
    }
  }
  WiFi.scanDelete();

  
  // nếu thử hết mà ko được thì quay lại AP config
  Serial.println("Khong ket noi duoc wifi nao trong list, quay lai AP config");
  setWifi();
}

