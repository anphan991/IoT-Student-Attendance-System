#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <HTTPClient.h>   // Thêm để gọi API
#include <ArduinoJson.h>  // Thêm để xử lý JSON
#include "time.h"

#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""

// --- CẤU HÌNH API SERVER ---
#define SERVER_URL "https://danganhle0623-iot.hf.space" 

// --- CẤU HÌNH THIẾT BỊ ---
#define DEVICE_ID "ESP_PHONG_B202" 

#define BUZZER_PIN   14
#define GREEN_LED    26
#define RED_LED      25
#define RELAY_PIN    27

static const int RFID_RX_PIN = 16;
static const int RFID_TX_PIN = 17;

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 25200; // UTC+7
const int   daylightOffset_sec = 0;

HardwareSerial RFIDSerial(2);
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Danh sách sinh viên tải về từ Database
struct Student {
  String uid;
  String mssv;
  String name;
};

const int MAX_STUDENTS = 100; // Giới hạn số lượng SV để không tràn RAM ESP32
Student students[MAX_STUDENTS];
int currentStudentCount = 0;

QueueHandle_t uploadQueue;

// Cấu trúc dữ liệu gửi vào Queue
struct UploadData {
  char uid[20];
  unsigned long timestamp;
};

// Lấy Epoch Time để đồng bộ chính xác với Backend
unsigned long getEpochTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return 0;
  }
  time(&now);
  return now;
}

// --- HÀM 1: LẤY DANH SÁCH SINH VIÊN TỪ SUPABASE (Quy API) ---
void fetchStudentList() {
  lcd.clear();
  lcd.print("Tai du lieu...");
  
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String api_url = String(SERVER_URL) + "/api/students/" + String(DEVICE_ID);
    
    Serial.println("Goi API: " + api_url);
    http.begin(api_url);
    int httpCode = http.GET();
    
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      
      // Parse JSON
      DynamicJsonDocument doc(4096); 
      DeserializationError error = deserializeJson(doc, payload);

      if (error) {
        Serial.print("Loi JSON: "); Serial.println(error.c_str());
        lcd.setCursor(0, 1); lcd.print("Loi du lieu!");
        delay(2000);
        return;
      }

      currentStudentCount = 0;
      JsonArray array = doc.as<JsonArray>();
      
      for (JsonObject repo : array) {
        if (currentStudentCount >= MAX_STUDENTS) break;
        students[currentStudentCount].uid = repo["uid"].as<String>();
        students[currentStudentCount].mssv = repo["mssv"].as<String>();
        students[currentStudentCount].name = repo["name"].as<String>();
        currentStudentCount++;
      }
      
      Serial.print("Da tai: "); Serial.print(currentStudentCount); Serial.println(" SV.");
      lcd.setCursor(0, 1); lcd.print("Da tai: " + String(currentStudentCount) + " SV");
      delay(2000);
      
    } else {
      Serial.print("Loi HTTP: "); Serial.println(httpCode);
      lcd.setCursor(0, 1); lcd.print("Loi may chu!");
      delay(2000);
    }
    http.end();
  }
}

// --- HÀM 2: GỬI KẾT QUẢ ĐIỂM DANH LÊN SERVER ---
void TaskUpload(void *pvParameters) {
  UploadData data;

  while (true) {
    if (xQueueReceive(uploadQueue, &data, portMAX_DELAY) == pdPASS) {
      if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        String api_url = String(SERVER_URL) + "/api/attendance";
        
        http.begin(api_url);
        http.addHeader("Content-Type", "application/json");

        // Tạo JSON payload bằng ArduinoJson
        StaticJsonDocument<200> doc;
        doc["uid"] = String(data.uid);
        doc["device_id"] = DEVICE_ID;
        doc["time_scan"] = data.timestamp;

        String requestBody;
        serializeJson(doc, requestBody);

        int httpResponseCode = http.POST(requestBody);
        
        if (httpResponseCode > 0) {
          Serial.print("Upload OK - HTTP Code: ");
          Serial.println(httpResponseCode);
        } else {
          Serial.print("Upload Failed - Error: ");
          Serial.println(httpResponseCode);
        }
        http.end();
      }
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

String lastUid = "";
unsigned long lastScanMs = 0;

// --- BIẾN QUẢN LÝ THỜI GIAN CẬP NHẬT TỰ ĐỘNG ---
unsigned long lastUpdateListMs = 0; 
const unsigned long UPDATE_LIST_INTERVAL = 30 * 60 * 1000; // 30 phút = 1.800.000 ms

void displayFast(String mssv, String name) {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("OK: "); lcd.print(mssv);
  lcd.setCursor(0, 1);
  if (name.length() > 16) lcd.print(name.substring(0, 16));
  else lcd.print(name);
}

void accessGranted(const String &uid, const Student &st) {
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(RELAY_PIN, HIGH);
  tone(BUZZER_PIN, 2000, 100);
  
  displayFast(st.mssv, st.name);

  UploadData pkg;
  memset(&pkg, 0, sizeof(UploadData)); // QUAN TRỌNG: Dọn sạch rác bộ nhớ C++
  strncpy(pkg.uid, uid.c_str(), sizeof(pkg.uid) - 1);
  pkg.timestamp = getEpochTime(); 
  
  // Cho phép đợi 1 giây nếu hàng đợi bận, thay vì vứt bỏ gói tin lập tức
  if (xQueueSend(uploadQueue, &pkg, 1000 / portTICK_PERIOD_MS) != pdPASS) {
    Serial.println("Loi: Hang doi day, rot goi tin Sinh Vien!");
  }

  delay(1000); 
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RELAY_PIN, LOW);
  
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Ready to Scan...");
  lcd.setCursor(0, 1); lcd.print(DEVICE_ID); 
}

void accessDenied(String uid) {
  lcd.clear(); lcd.print("UNKNOWN CARD");
  lcd.setCursor(0, 1); lcd.print(uid);
  
  digitalWrite(RED_LED, HIGH);
  tone(BUZZER_PIN, 500, 800);

  UploadData pkg;
  memset(&pkg, 0, sizeof(UploadData)); // QUAN TRỌNG: Dọn sạch rác bộ nhớ C++
  strncpy(pkg.uid, uid.c_str(), sizeof(pkg.uid) - 1);
  pkg.timestamp = getEpochTime();
  
  // Cho phép đợi 1 giây nếu hàng đợi bận
  if (xQueueSend(uploadQueue, &pkg, 1000 / portTICK_PERIOD_MS) != pdPASS) {
    Serial.println("Loi: Hang doi day, rot goi tin Khach!");
  }
  
  delay(1000);
  digitalWrite(RED_LED, LOW);
  lcd.clear(); lcd.print("Ready to Scan...");
  lcd.setCursor(0, 1); lcd.print(DEVICE_ID);
}

void setup() {
  Serial.begin(115200);
  
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  lcd.init(); lcd.backlight();
  lcd.print("Connecting WiFi");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    lcd.print(".");
  }

  // Đồng bộ thời gian
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // Khởi tạo Queue
  uploadQueue = xQueueCreate(10, sizeof(UploadData));

  // Chạy ngầm tiến trình gửi POST Request
  xTaskCreatePinnedToCore(
    TaskUpload,
    "HttpTask",
    12000,
    NULL,
    1,
    NULL,
    0
  );

  // TẢI DANH SÁCH SINH VIÊN QUA API LẦN ĐẦU KHI KHỞI ĐỘNG
  fetchStudentList();
  lastUpdateListMs = millis(); // Đánh dấu mốc thời gian tải lần đầu

  RFIDSerial.begin(115200, SERIAL_8N1, RFID_RX_PIN, RFID_TX_PIN);
  RFIDSerial.setTimeout(10); 

  Wire.begin(21, 22);
  
  lcd.clear();
  lcd.print("System Ready!");
  lcd.setCursor(0, 1); lcd.print(DEVICE_ID);
}

void loop() {
  // --- 1. TỰ ĐỘNG CẬP NHẬT DANH SÁCH THEO CHU KỲ ---
  if (millis() - lastUpdateListMs >= UPDATE_LIST_INTERVAL) {
    Serial.println("Tu dong cap nhat danh sach sau 30 phut...");
    fetchStudentList();
    lastUpdateListMs = millis(); // Reset mốc thời gian
    
    // Đảm bảo hiển thị lại màn hình chờ sau khi cập nhật xong
    lcd.clear();
    lcd.print("System Ready!");
    lcd.setCursor(0, 1); lcd.print(DEVICE_ID);
  }

  // --- 2. LOGIC ĐỌC THẺ RFID ---
  if (RFIDSerial.available()) {
    String line = RFIDSerial.readStringUntil('\n');
    
    line.trim(); line.replace("\r", ""); line.toUpperCase();
    if (line.startsWith("UID:")) line = line.substring(4);
    line.trim();

    if (line.length() < 5 || line.indexOf("NONE") >= 0) return;

    if (line == lastUid && millis() - lastScanMs < 2000) return;
    lastUid = line;
    lastScanMs = millis();

    bool found = false;
    for (int i = 0; i < currentStudentCount; i++) {
      if (line.equals(students[i].uid)) {
        accessGranted(line, students[i]);
        found = true;
        break;
      }
    }
    if (!found) accessDenied(line);
  }
}