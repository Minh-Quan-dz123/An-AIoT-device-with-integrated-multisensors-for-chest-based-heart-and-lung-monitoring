#include "wifi_Manager.h"

// 0 constructor
WiFiManagerTask::WiFiManagerTask() : server(80)
{
    state = WIFI_INIT; // trạng thái ban đầu
    g_wifiState = state;
}

// 1 bắt đầu luồng
void WiFiManagerTask::begin()
{
    // 1.1 lấy thông tin từ bộ nhớ flash ra trước
    preferences.begin("wifi", true);
    ssid = preferences.getString("ssid", "");
    password = preferences.getString("pass", "");
    //ssid = ""; password = "";
    preferences.end();
    //

    // 1.2 tạo task xử lý
    xTaskCreate(
        taskMain,
        "WiFiMain",
        4096,
        this,
        2,// độ ưu tiên số 2
        &mainTaskHandle
    );
}

// 2 wrapper của task wifi
void WiFiManagerTask::taskMain(void* param)
{
    WiFiManagerTask* self = (WiFiManagerTask*)param;
    self->loop();
}

// 2 vòng loop chính của task
void WiFiManagerTask::loop()
{
    // 2.1 kiểm tra có ssid trước đó ko
    bool hasSSID = (ssid.length() > 0);
    if (!hasSSID)
    {
        // ko thì bắt đầu luồng AP và Server
        printlnWiFiNotConnected();
        state = WIFI_AP_MODE;
        g_wifiState = state;
        startAP();
        startServer();
        
    }
    else // 2.2. nếu đã có rồi thì task này tự động kết nối wifi
    {
        state = WIFI_CONNECTING;
        g_wifiState = state;
        bool resConnectWifi = connectWiFi();
        if(resConnectWifi) // nếu oke thì xóa task này
        {
            state = WIFI_CONNECTED;
            g_wifiState = state;

            Serial.print("--->[WIFI Manager] Main Stack con du: ");
            Serial.println(uxTaskGetStackHighWaterMark(NULL));

            vTaskDelete(NULL);
        }
        else // nếu ko thì bật AP và web
        {
            state = WIFI_AP_MODE;
            g_wifiState = state;
            startAP();
            startServer();
        }
    }

    // cứ đợi server hoạt động thôi và đợi task con gửi signal
    uint32_t val;

    uint32_t runTime = millis();
    while (1)
    {
        
        if(millis() - runTime >= 1000)
        {
            runTime = millis();
            Serial.print("--->[WIFI Manager] Main Stack con du: ");
            Serial.println(uxTaskGetStackHighWaterMark(NULL));
        }

        if(xTaskNotifyWait(0, 0, &val, 0) == pdTRUE)
        {
            if(val == 1)
            {
                state = WIFI_CONNECTED;
                g_wifiState = state;
                // tắt AP và server luôn
                stopAP();
                server.stop();
                vTaskDelete(NULL);
            }
        }
        server.handleClient();
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}


// 3 hàm thực hiện kết nối wifi
bool WiFiManagerTask::connectWiFi()
{
    printlnWiFiConnecting();
    state = WIFI_CONNECTING;
    g_wifiState = state;

    // 3.1 chuyển sang chế độ STA
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());

    // 3.2 block luồng hiện tại tối đa 10 giây
    int timeout = 10;
    while (WiFi.status() != WL_CONNECTED && timeout--)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    // 3.3 nếu kết nối thành công
    if (WiFi.status() == WL_CONNECTED)
    {
        printlnWiFiConnected();
        state = WIFI_CONNECTED;
        g_wifiState = state;
        Serial.println(WiFi.localIP());
        return true;
    }

    // nếu xịt
    printlnWiFiFailed();
    state = WIFI_FAIL;
    g_wifiState = state;
    return false;
}


// 4 hàm bật AP
void WiFiManagerTask::startAP()
{
    // bật AP mode với ko có password
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ap_ssid);

    Serial.println("--->[WIFI MAnager] AP MODE STARTED");
    Serial.println(WiFi.softAPIP());
}

// 4.5 tắt chế độ AP mode
void WiFiManagerTask::stopAP()
{
    WiFi.softAPdisconnect(true);
}

// 5 đăng ký và chạy server
void WiFiManagerTask::startServer()
{
    server.on("/", [this]() { handleRoot(); });
    server.on("/save", HTTP_POST, [this]() { handleSave(); });
    server.on("/status", [this]() { handleStatus(); });

    server.begin();
}

// 6 hàm chứa nội dung html
void WiFiManagerTask::handleRoot()
{
    String html = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
        <meta charset="UTF-8">
        <title>ESP32 WiFi Setup</title>
    </head>
    <body>
        <h2>WiFi Setup</h2>

        <form action="/save" method="POST">
            <input name="ssid" placeholder="SSID"><br>
            <input name="pass" type="password" placeholder="Password"><br>
            <button type="submit">Connect</button>
        </form>

        <p id="status">Status: ...</p>

        <script>
            setInterval(() => {
                fetch('/status')
                .then(r => r.text())
                .then(t => document.getElementById('status').innerText = "Status: " + t);
            }, 2000);
        </script>
    </body>
    </html>
    )rawliteral";

    server.send(200, "text/html", html);
}

// 7 hàm xử lý lưu wifi khi user bấm save
void WiFiManagerTask::handleSave()
{
    // 7.1 lấy thông tin
    ssid = server.arg("ssid");
    password = server.arg("pass");

    // 7.2 lưu
    preferences.begin("wifi", false);
    preferences.putString("ssid", ssid);
    preferences.putString("pass", password);
    preferences.end();

    // 7.3 cập nhật trạng thái
    server.send(200, "text/plain", "Saved. Connecting...");

    // 7.4 gọi task con để thực hiện kết nối
    state = WIFI_CONNECTING;
    g_wifiState = state;
    createConnectTask();
}

// 8 hàm lưu trạng thái phục vụ việc hiển thị
void WiFiManagerTask::handleStatus()
{
    String s;
    switch (state)
    {
        case WIFI_INIT: s = "WIFI_INIT"; break;
        case WIFI_AP_MODE: s = "WIFI_AP_MODE"; break;
        case WIFI_CONNECTING: s = "WIFI_CONNECTING"; break;
        case WIFI_CONNECTED: s = "WIFI_CONNECTED"; break;
        case WIFI_FAIL: s = "WIFI_FAIL"; break;
    }
    server.send(200, "text/plain", s);
}

// 9 hàm tạo task con
void WiFiManagerTask::createConnectTask()
{
    // phải kiểm tra cái này nếu user bấm nhiều lần save trên UI
    if(connectTaskHandle == NULL)
    {
        xTaskCreate(taskConnect, "WiFiConnectTask", 4096, this, 2, &connectTaskHandle);
    }
}


// 10 task con thực hiện kết nối wifi
void WiFiManagerTask::taskConnect(void* param)
{
    WiFiManagerTask* self = (WiFiManagerTask*)param;
    // 10.1 nếu kết nối thành công
    bool res = self->connectWiFi(); //(blocking task con)
    if(res)
    {
        printlnWiFiConnected();
        xTaskNotify(self->mainTaskHandle, 1, eSetValueWithOverwrite);
    }
    else
    {   // 10.2 nếu kết nối xịt thì dọn bộ nhớ
        self->state = WIFI_FAIL;
        g_wifiState = WIFI_FAIL;

        self->ssid = "";
        self->password = "";

        self->preferences.begin("wifi", false);
        self->preferences.clear();
        self->preferences.end();

        // bật lại AP, server thì vẫn vậy
        self->startAP();
        self->state = WIFI_AP_MODE;
        g_wifiState = WIFI_AP_MODE;
        printlnWiFiFailed();
    }

    // kết nối thành công hay thất bại đều delete task này
    self->connectTaskHandle = NULL;

    // kiểm tra số byte
    Serial.print("--->[WIFI Connector] Connect Stack remaining: ");
    Serial.println(uxTaskGetStackHighWaterMark(NULL));

    vTaskDelete(NULL);
}

WiFiState WiFiManagerTask::getState()
{
    return state;
}

// các hàm println
void printlnWiFiConnecting() {
    Serial.println("--->[WIFI Manager] Dang ket noi WiFi...");
}

void printlnWiFiConnected() {
    Serial.println("--->[WIFI Manager] Da ket noi WiFi thanh cong");
}

void printlnWiFiFailed() {
    Serial.println("--->[WIFI Manager] Ket noi WiFi that bai");
}

void printlnWiFiNotConnected() {
    Serial.println("--->[WIFI Manager] Chua ket noi WiFi");
}