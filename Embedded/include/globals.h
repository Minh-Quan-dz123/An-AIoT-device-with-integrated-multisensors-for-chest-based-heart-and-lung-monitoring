#ifndef GLOBALS_H
#define GLOBALS_H

// 1 enum trạng thái wifi dùng chung toàn projec
enum WiFiState
{
    WIFI_INIT,
    WIFI_CONNECTING,
    WIFI_CONNECTED,
    WIFI_AP_MODE,
    WIFI_FAIL
};

// 1 biến trạng thái wifi
extern WiFiState g_wifiState;

#endif