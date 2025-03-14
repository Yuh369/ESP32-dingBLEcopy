// 代码来源：https://github.com/CBHZQRP/ESP32-dingBLEcopy/blob/main/BLE%20Server.txt 
// 文件说明：原有的基础上支持其他型号ESP32设备

#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <HTTPClient.h>

#if !defined(ESP32_S2)
// 只有不为 ESP32-S2 的平台才包含 BLE 相关库
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <esp_sleep.h>
#endif

// AP 模式相关
#define AP_SSID "ESP32BLEServer"
#define AP_PASSWORD "12345678"
#define WEB_PORT 80

// LED 引脚：如果目标板有 LED_BUILTIN 定义，则优先使用
#ifndef LED_PIN
  #ifdef LED_BUILTIN
    #define LED_PIN LED_BUILTIN
  #else
    #define LED_PIN 2  // 默认备用引脚
  #endif
#endif

char ssid[32] = "";
char password[32] = "";

// LeanCloud 服务器配置
const char* api_url = "api/1.1/classes/AdvertisingData?order=-createdAt&limit=1";    //例"https://abcdefb.lc-cn-n1-shared.com/1.1/classes/AdvertisingData?order=-createdAt&limit=1"
const char* appkey = "key";  //改为你的appkey
const char* appid = "id";   改为你的appid

#if !defined(ESP32_S2)
// 只有支持 BLE 的平台编译以下代码

uint8_t bleMac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};   //改为你的mac

boolean rawMoreThan31 = true;

BLEAdvertising *pAdvertising = nullptr;

uint8_t bleRaw[31];
uint8_t bleRaw32[31] = {0};

void updateBLEData(const String& data) {
    int dataLength = data.length() / 2;
    for (int i = 0; i < dataLength; ++i) {
        if (i < 31) {
            bleRaw[i] = strtoul(data.substring(i * 2, i * 2 + 2).c_str(), nullptr, 16);
        } else {
            bleRaw32[i - 31] = strtoul(data.substring(i * 2, i * 2 + 2).c_str(), nullptr, 16);
        }
    }
    
    esp_err_t errRc = ::esp_ble_gap_config_adv_data_raw(bleRaw, 31);
    if (errRc != ESP_OK) {
        Serial.printf("esp_ble_gap_config_adv_data_raw error: %d\n", errRc);
    }

    if (rawMoreThan31) {
        errRc = ::esp_ble_gap_config_scan_rsp_data_raw(bleRaw32, sizeof(bleRaw32));
        if (errRc != ESP_OK) {
            Serial.printf("esp_ble_gap_config_scan_rsp_data_raw error: %d\n", errRc);
        }
    }
    
    if (pAdvertising) {
        pAdvertising->start();
    }
}
#endif  // !ESP32_S2

// WebServer 对象
WebServer server(WEB_PORT);

String lastObjectId = "";

// 加载和保存 WiFi 配置
void loadCredentials() {
    Preferences preferences;
    preferences.begin("wifi", false);
    preferences.getString("ssid", ssid, sizeof(ssid));
    preferences.getString("password", password, sizeof(password));
    preferences.end();
}

void saveCredentials() {
    Preferences preferences;
    preferences.begin("wifi", false);
    preferences.putString("ssid", ssid);
    preferences.putString("password", password);
    preferences.end();
}

// 处理根目录请求，返回 WiFi 配置页面
void handleRoot() {
    String html = "<html><head><meta charset='UTF-8'><title>ESP32 配置页面</title></head>";
    html += "<body><h1>ESP32 配置页面</h1>";
    html += "<form method='post' action='/config'>";
    html += "SSID:<br><input type='text' name='ssid'><br>";
    html += "Password:<br><input type='password' name='password'><br><br>";
    html += "<input type='submit' value='提交'>";
    html += "</form></body></html>";
    server.send(200, "text/html; charset=utf-8", html);
}

// 处理配置请求，保存 WiFi 配置后重启
void handleConfig() {
    if (server.hasArg("ssid") && server.hasArg("password")) {
        strncpy(ssid, server.arg("ssid").c_str(), sizeof(ssid) - 1);
        strncpy(password, server.arg("password").c_str(), sizeof(password) - 1);
        saveCredentials();
        server.send(200, "text/plain", "配置已保存，正在重启...");
        ESP.restart();
    } else {
        server.send(400, "text/plain", "无效请求");
    }
}

unsigned long startTime = 0;
bool apModeStarted = false;
unsigned long previousMillis = 0;
const long intervalConnecting = 500;
const long intervalAPMode = 150;
unsigned long lastUpdateMillis = 0;
const long updateInterval = 10000; // 10 秒

// 启动 AP 模式，用于配置 WiFi
void startAPMode() {
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    IPAddress ip = WiFi.softAPIP();
    Serial.print("AP IP 地址: ");
    Serial.println(ip);

    server.on("/", handleRoot);
    server.on("/config", HTTP_POST, handleConfig);
    server.begin();
    Serial.println("HTTP 服务器已启动");

    startTime = millis();
    apModeStarted = true;
}

void fetchAdvertisingData() {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(api_url);
        http.addHeader("X-LC-Id", appid);
        http.addHeader("X-LC-Key", appkey);

        int httpResponseCode = http.GET();
        if (httpResponseCode > 0) {
            String payload = http.getString();
            Serial.println(payload);

            int dataIndex = payload.indexOf("\"data\":\"");
            int objectIdIndex = payload.indexOf("\"objectId\":\"");

            if (dataIndex != -1 && objectIdIndex != -1) {
                String data = payload.substring(dataIndex + 8, payload.indexOf("\"", dataIndex + 8));
                String objectId = payload.substring(objectIdIndex + 12, payload.indexOf("\"", objectIdIndex + 12));

                if (lastObjectId != objectId) {
                    lastObjectId = objectId;
#if !defined(ESP32_S2)
                    updateBLEData(data);
#endif
                }
            }
        }
        http.end();
    }
}

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW); // 初始关闭 LED

    // 加载 WiFi 配置
    loadCredentials();

    // 尝试连接 WiFi
    WiFi.begin(ssid, password);
    unsigned long connectionStartTime = millis();
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("正在连接到 WiFi...");
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >= intervalConnecting) {
            previousMillis = currentMillis;
            digitalWrite(LED_PIN, !digitalRead(LED_PIN)); // 闪烁提示
        }
        if (millis() - connectionStartTime > 30000) {
            Serial.println("连接 WiFi 超时，启动 AP 模式");
            startAPMode();
            return;
        }
    }
    Serial.println("WiFi 连接成功");
    digitalWrite(LED_PIN, LOW);

#if !defined(ESP32_S2)
    // 以下 BLE 相关代码只在支持 BLE 的平台上编译

    // 设置蓝牙 MAC 地址（注意：有些平台不允许动态设置或已内置）
    esp_err_t result = esp_base_mac_addr_set(bleMac);
    if (result != ESP_OK) {
        Serial.printf("设置 BLE 基础 MAC 地址失败，错误码: %d\n", result);
        // 如果失败，可以考虑继续或退出
    }

    // 初始化 BLE
    BLEDevice::init("");
    pAdvertising = BLEDevice::getAdvertising();

    // 设置空的扫描响应数据
    BLEAdvertisementData oScanResponseData;
    pAdvertising->setScanResponseData(oScanResponseData);

    // 设置空的广播数据
    BLEAdvertisementData oAdvertisementData;
    pAdvertising->setAdvertisementData(oAdvertisementData);

    // 启动初始广播
    updateBLEData("EEEFFF");  // 启动后将被联网后更新的数据覆盖
#endif

    // 首次获取数据
    fetchAdvertisingData();
}

void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        if (apModeStarted) {
            unsigned long currentMillis = millis();
            if (currentMillis - previousMillis >= intervalAPMode) {
                previousMillis = currentMillis;
                digitalWrite(LED_PIN, !digitalRead(LED_PIN));
            }
            server.handleClient();
            if (millis() - startTime > 90000) {
                Serial.println("AP 模式超时，重启设备");
                ESP.restart();
            }
        }
        return;
    }

    unsigned long currentMillis = millis();
    if (currentMillis - lastUpdateMillis >= updateInterval) {
        lastUpdateMillis = currentMillis;
        fetchAdvertisingData();
    }

    // 每30分钟重启一次
    static unsigned long restartMillis = 0;
    if (currentMillis - restartMillis >= 1800000) {
        restartMillis = currentMillis;
        Serial.println("设备将在30分钟后重启");
        ESP.restart();
    }
}
