/*
 * ESP32 WiFi Extender / Repeater
 * Professional Control Panel with Terminal-Style UI
 *
 * Features:
 * - WiFi Repeater (STA + AP mode)
 * - Connected device management
 * - Password management
 * - Time-based access control
 * - Real-time monitoring dashboard
 */

#include <WiFi.h>
#include <WebServer.h>
#include <EEPROM.h>
#include <ArduinoJson.h>
#include <esp_wifi.h>
#include <DNSServer.h>

// ============== CONFIGURATION ==============
#define EEPROM_SIZE 512
#define MAX_CLIENTS 10
#define DNS_PORT 53

// Default credentials
String source_ssid = "The Maobveras";       // WiFi to extend (set via UI)
String source_password = "bigFeather09";    // Source WiFi password
String ap_ssid = "ESP32_Extender";
String ap_password = "admin1234";
String admin_user = "admin";
String admin_pass = "admin";

// Server
WebServer server(80);
DNSServer dnsServer;

// Connected clients tracking
struct ClientInfo {
  String mac;
  String ip;
  String hostname;
  unsigned long connectedTime;
  unsigned long timeLimit;  // 0 = unlimited, else minutes
  bool blocked;
};

ClientInfo clients[MAX_CLIENTS];
int clientCount = 0;

// Status
bool isConnectedToSource = false;
unsigned long uptime = 0;
int signalStrength = 0;
bool captiveDnsEnabled = false;

// Forward declarations
void handleRoot();
void handleApi();
void handleScan();
void handleConnect();
void handleSettings();
void handleClients();
void handleBlock();
void handleTimeLimit();
void handleNotFound();
void loadSettings();
void saveSettings();
void updateClients();
String getEncryptionTypeName(wifi_auth_mode_t authMode);
void sendJsonResponse(int code, const String& payload);
void handleOptions();

void setup() {
  Serial.begin(115200);
  Serial.println("\n[BOOT] ESP32 WiFi Extender Starting...");

  EEPROM.begin(EEPROM_SIZE);
  loadSettings();

  // Setup AP + STA mode
  WiFi.mode(WIFI_AP_STA);

  // Configure AP
  WiFi.softAP(ap_ssid.c_str(), ap_password.c_str(), 6, 0, MAX_CLIENTS);
  Serial.printf("[AP] SSID: %s | IP: %s\n", ap_ssid.c_str(), WiFi.softAPIP().toString().c_str());

  // Try connecting to source WiFi
  if (source_ssid.length() > 0) {
    Serial.printf("[STA] Connecting to: %s\n", source_ssid.c_str());
    WiFi.begin(source_ssid.c_str(), source_password.c_str());

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      delay(500);
      Serial.print(".");
      attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
      isConnectedToSource = true;
      signalStrength = WiFi.RSSI();
      Serial.printf("\n[STA] Connected! IP: %s | RSSI: %d\n", WiFi.localIP().toString().c_str(), signalStrength);

      // Keep radio active for best bridge behavior.
      esp_wifi_set_ps(WIFI_PS_NONE);
    } else {
      Serial.println("\n[STA] Connection failed. Running in AP-only mode.");
    }
  }

  // Captive DNS should run only in AP-only mode.
  if (!isConnectedToSource) {
    dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
    captiveDnsEnabled = true;
    Serial.println("[DNS] Captive portal DNS enabled (AP-only mode)");
  } else {
    Serial.println("[DNS] Captive portal DNS disabled (internet passthrough mode)");
  }

  // Web server routes
  server.on("/", handleRoot);
  server.on("/api/status", handleApi);
  server.on("/api/scan", handleScan);
  server.on("/api/connect", HTTP_POST, handleConnect);
  server.on("/api/settings", HTTP_POST, handleSettings);
  server.on("/api/clients", handleClients);
  server.on("/api/block", HTTP_POST, handleBlock);
  server.on("/api/timelimit", HTTP_POST, handleTimeLimit);

  // CORS preflight routes for local preview app.
  server.on("/api/status", HTTP_OPTIONS, handleOptions);
  server.on("/api/scan", HTTP_OPTIONS, handleOptions);
  server.on("/api/connect", HTTP_OPTIONS, handleOptions);
  server.on("/api/settings", HTTP_OPTIONS, handleOptions);
  server.on("/api/clients", HTTP_OPTIONS, handleOptions);
  server.on("/api/block", HTTP_OPTIONS, handleOptions);
  server.on("/api/timelimit", HTTP_OPTIONS, handleOptions);
  server.on("/api/reboot", HTTP_OPTIONS, handleOptions);

  server.on("/api/reboot", []() {
    sendJsonResponse(200, "{\"status\":\"rebooting\"}");
    delay(1000);
    ESP.restart();
  });

  server.onNotFound(handleRoot);

  server.begin();
  Serial.println("[WEB] Server started on port 80");
}

void loop() {
  if (captiveDnsEnabled) {
    dnsServer.processNextRequest();
  }
  server.handleClient();

  // Heartbeat for Serial Monitor visibility.
  static unsigned long lastHeartbeat = 0;
  if (millis() - lastHeartbeat > 15000) {
    lastHeartbeat = millis();
    Serial.printf("[SYS] Uptime: %lus | AP clients: %d | STA: %s\n",
                  millis() / 1000,
                  WiFi.softAPgetStationNum(),
                  isConnectedToSource ? "Connected" : "Disconnected");
  }

  // Update every 5 seconds
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 5000) {
    lastUpdate = millis();
    updateClients();

    if (isConnectedToSource) {
      signalStrength = WiFi.RSSI();
      if (captiveDnsEnabled) {
        dnsServer.stop();
        captiveDnsEnabled = false;
        Serial.println("[DNS] Captive portal DNS stopped after STA connection");
      }
    } else {
      if (!captiveDnsEnabled) {
        dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
        captiveDnsEnabled = true;
        Serial.println("[DNS] Captive portal DNS re-enabled (STA disconnected)");
      }
    }

    // Check time limits
    for (int i = 0; i < clientCount; i++) {
      if (clients[i].timeLimit > 0) {
        unsigned long elapsed = (millis() - clients[i].connectedTime) / 60000;
        if (elapsed >= clients[i].timeLimit) {
          clients[i].blocked = true;
        }
      }
    }

    uptime = millis() / 1000;
  }
}

void updateClients() {
  wifi_sta_list_t stationList;
  tcpip_adapter_sta_list_t adapterList;

  esp_wifi_ap_get_sta_list(&stationList);
  tcpip_adapter_get_sta_list(&stationList, &adapterList);

  // Track new clients
  for (int i = 0; i < adapterList.num; i++) {
    tcpip_adapter_sta_info_t station = adapterList.sta[i];
    char macStr[18];
    sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X",
            station.mac[0], station.mac[1], station.mac[2],
            station.mac[3], station.mac[4], station.mac[5]);

    String mac = String(macStr);
    bool found = false;

    for (int j = 0; j < clientCount; j++) {
      if (clients[j].mac == mac) {
        clients[j].ip = ip4addr_ntoa((const ip4_addr_t*)&station.ip);
        found = true;
        break;
      }
    }

    if (!found && clientCount < MAX_CLIENTS) {
      clients[clientCount].mac = mac;
      clients[clientCount].ip = ip4addr_ntoa((const ip4_addr_t*)&station.ip);
      clients[clientCount].hostname = "Device-" + String(clientCount + 1);
      clients[clientCount].connectedTime = millis();
      clients[clientCount].timeLimit = 0;
      clients[clientCount].blocked = false;
      clientCount++;
    }
  }
}

String getEncryptionTypeName(wifi_auth_mode_t authMode) {
  switch (authMode) {
    case WIFI_AUTH_OPEN:
      return "Open";
    case WIFI_AUTH_WEP:
      return "WEP";
    case WIFI_AUTH_WPA_PSK:
      return "WPA";
    case WIFI_AUTH_WPA2_PSK:
      return "WPA2";
    case WIFI_AUTH_WPA_WPA2_PSK:
      return "WPA/WPA2";
    case WIFI_AUTH_WPA2_ENTERPRISE:
      return "WPA2-Enterprise";
#if defined(WIFI_AUTH_WPA3_PSK)
    case WIFI_AUTH_WPA3_PSK:
      return "WPA3";
#endif
#if defined(WIFI_AUTH_WPA2_WPA3_PSK)
    case WIFI_AUTH_WPA2_WPA3_PSK:
      return "WPA2/WPA3";
#endif
#if defined(WIFI_AUTH_WAPI_PSK)
    case WIFI_AUTH_WAPI_PSK:
      return "WAPI";
#endif
    default:
      return "Unknown";
  }
}

void sendJsonResponse(int code, const String& payload) {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
  server.send(code, "application/json", payload);
}

void handleOptions() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
  server.send(204, "text/plain", "");
}

// ============== API HANDLERS ==============

void handleApi() {
  static unsigned long lastApiLog = 0;
  if (millis() - lastApiLog > 3000) {
    lastApiLog = millis();
    Serial.printf("[API] /api/status requested by UI\n");
  }

  StaticJsonDocument<512> doc;
  doc["uptime"] = uptime;
  doc["connectedToSource"] = isConnectedToSource;
  doc["sourceSSID"] = source_ssid;
  doc["apSSID"] = ap_ssid;
  doc["apIP"] = WiFi.softAPIP().toString();
  doc["staIP"] = isConnectedToSource ? WiFi.localIP().toString() : "N/A";
  doc["rssi"] = signalStrength;
  doc["clientCount"] = WiFi.softAPgetStationNum();
  doc["freeHeap"] = ESP.getFreeHeap();
  doc["chipModel"] = ESP.getChipModel();

  String output;
  serializeJson(doc, output);
  sendJsonResponse(200, output);
}

void handleScan() {
  int n = WiFi.scanNetworks();
  StaticJsonDocument<2048> doc;
  JsonArray networks = doc.createNestedArray("networks");

  Serial.printf("[SCAN] Networks found: %d\n", n);

  for (int i = 0; i < n && i < 15; i++) {
    wifi_auth_mode_t authMode = WiFi.encryptionType(i);
    String encryption = getEncryptionTypeName(authMode);

    JsonObject net = networks.createNestedObject();
    net["ssid"] = WiFi.SSID(i);
    net["rssi"] = WiFi.RSSI(i);
    net["encryption"] = encryption;
    net["channel"] = WiFi.channel(i);

    Serial.printf("[SCAN] %02d SSID: %s | RSSI: %d | ENC: %s | CH: %d\n",
                  i + 1,
                  WiFi.SSID(i).c_str(),
                  WiFi.RSSI(i),
                  encryption.c_str(),
                  WiFi.channel(i));
  }

  WiFi.scanDelete();
  String output;
  serializeJson(doc, output);
  sendJsonResponse(200, output);
}

void handleConnect() {
  if (server.hasArg("plain")) {
    StaticJsonDocument<256> doc;
    deserializeJson(doc, server.arg("plain"));

    source_ssid = doc["ssid"].as<String>();
    source_password = doc["password"].as<String>();
    Serial.printf("[STA] Connect request received for SSID: %s\n", source_ssid.c_str());
    saveSettings();

    sendJsonResponse(200, "{\"status\":\"connecting\"}");

    delay(500);
    WiFi.begin(source_ssid.c_str(), source_password.c_str());

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      delay(500);
      Serial.print(".");
      attempts++;
    }

    isConnectedToSource = (WiFi.status() == WL_CONNECTED);
    if (isConnectedToSource) {
      signalStrength = WiFi.RSSI();
      Serial.printf("\n[STA] Connected to %s | IP: %s | RSSI: %d\n",
                    source_ssid.c_str(),
                    WiFi.localIP().toString().c_str(),
                    signalStrength);
      esp_wifi_set_ps(WIFI_PS_NONE);
      if (captiveDnsEnabled) {
        dnsServer.stop();
        captiveDnsEnabled = false;
        Serial.println("[DNS] Captive portal DNS stopped (internet passthrough mode)");
      }
    } else {
      Serial.printf("\n[STA] Failed to connect to %s | status: %d\n", source_ssid.c_str(), WiFi.status());
      if (!captiveDnsEnabled) {
        dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
        captiveDnsEnabled = true;
        Serial.println("[DNS] Captive portal DNS enabled (AP-only mode)");
      }
    }
  }
}

void handleSettings() {
  if (server.hasArg("plain")) {
    StaticJsonDocument<512> doc;
    deserializeJson(doc, server.arg("plain"));

    if (doc.containsKey("apSSID")) ap_ssid = doc["apSSID"].as<String>();
    if (doc.containsKey("apPassword")) ap_password = doc["apPassword"].as<String>();
    if (doc.containsKey("adminUser")) admin_user = doc["adminUser"].as<String>();
    if (doc.containsKey("adminPass")) admin_pass = doc["adminPass"].as<String>();

    saveSettings();
    sendJsonResponse(200, "{\"status\":\"saved\",\"message\":\"Settings saved. Reboot to apply.\"}");
  }
}

void handleClients() {
  StaticJsonDocument<2048> doc;
  JsonArray arr = doc.createNestedArray("clients");

  for (int i = 0; i < clientCount; i++) {
    JsonObject c = arr.createNestedObject();
    c["mac"] = clients[i].mac;
    c["ip"] = clients[i].ip;
    c["hostname"] = clients[i].hostname;
    c["connected"] = (millis() - clients[i].connectedTime) / 1000;
    c["timeLimit"] = clients[i].timeLimit;
    c["blocked"] = clients[i].blocked;
  }

  String output;
  serializeJson(doc, output);
  sendJsonResponse(200, output);
}

void handleBlock() {
  if (server.hasArg("plain")) {
    StaticJsonDocument<128> doc;
    deserializeJson(doc, server.arg("plain"));
    String mac = doc["mac"].as<String>();
    bool block = doc["block"];

    for (int i = 0; i < clientCount; i++) {
      if (clients[i].mac == mac) {
        clients[i].blocked = block;
        break;
      }
    }
    sendJsonResponse(200, "{\"status\":\"ok\"}");
  }
}

void handleTimeLimit() {
  if (server.hasArg("plain")) {
    StaticJsonDocument<128> doc;
    deserializeJson(doc, server.arg("plain"));
    String mac = doc["mac"].as<String>();
    int minutes = doc["minutes"];

    for (int i = 0; i < clientCount; i++) {
      if (clients[i].mac == mac) {
        clients[i].timeLimit = minutes;
        clients[i].connectedTime = millis();
        break;
      }
    }
    sendJsonResponse(200, "{\"status\":\"ok\"}");
  }
}

// ============== EEPROM ==============

void saveSettings() {
  StaticJsonDocument<512> doc;
  doc["srcSSID"] = source_ssid;
  doc["srcPass"] = source_password;
  doc["apSSID"] = ap_ssid;
  doc["apPass"] = ap_password;
  doc["admUser"] = admin_user;
  doc["admPass"] = admin_pass;

  String output;
  serializeJson(doc, output);

  EEPROM.writeString(0, output);
  EEPROM.commit();
  Serial.println("[EEPROM] Settings saved");
}

void loadSettings() {
  String data = EEPROM.readString(0);
  if (data.length() > 5) {
    StaticJsonDocument<512> doc;
    DeserializationError err = deserializeJson(doc, data);
    if (err == DeserializationError::Ok) {
      source_ssid = doc["srcSSID"].as<String>();
      source_password = doc["srcPass"].as<String>();
      ap_ssid = doc["apSSID"].as<String>();
      ap_password = doc["apPass"].as<String>();
      admin_user = doc["admUser"].as<String>();
      admin_pass = doc["admPass"].as<String>();
      Serial.println("[EEPROM] Settings loaded");
    }
  }
}

// ============== WEB PAGE ==============
// The HTML is served from handleRoot - defined in web_ui.ino
