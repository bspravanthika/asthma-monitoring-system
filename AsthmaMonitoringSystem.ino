#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// ===== WIFI =====
const char* ssid = "Firmware";
const char* password = "Solutions@12345";

// ===== SERVER =====
const char* serverName = "http://esskay-012024.live/pollution12345/save_data.php";

// ===== PINS =====
#define MQ135_PIN D6
#define MQ2_PIN   D7
#define ONE_WIRE_BUS D5
#define BUZZER    D0

// ===== OBJECTS =====
LiquidCrystal_I2C lcd(0x27, 16, 2);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(115200);

  pinMode(MQ135_PIN, INPUT);
  pinMode(MQ2_PIN, INPUT);
  pinMode(BUZZER, OUTPUT);

  digitalWrite(BUZZER, LOW);

  sensors.begin();

  lcd.init();
  lcd.backlight();

  // ===== WIFI CONNECT =====
  WiFi.begin(ssid, password);
  lcd.setCursor(0,0);
  lcd.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  lcd.clear();
  lcd.print("WiFi Connected");
  delay(2000);
  lcd.clear();
}

void loop() {

  int airQuality = digitalRead(MQ135_PIN);
  int gasState   = digitalRead(MQ2_PIN);

  sensors.requestTemperatures();
  float temp = sensors.getTempCByIndex(0);

  lcd.clear();

  if (temp == DEVICE_DISCONNECTED_C) {
    lcd.print("Temp Error!");
    delay(2000);
    return;
  }

  // ===== ALERT =====
  if (gasState == LOW || airQuality == LOW) {
    digitalWrite(BUZZER, HIGH);

    lcd.setCursor(0,0);
    lcd.print("!!! WARNING !!!");

    if (gasState == LOW) {
      lcd.setCursor(0,1);
      lcd.print("Gas Detected!");
    } else {
      lcd.setCursor(0,1);
      lcd.print("Air Pollution!");
    }
  } else {
    digitalWrite(BUZZER, LOW);

    lcd.setCursor(0,0);
    lcd.print("Temp:");
    lcd.print(temp);
    lcd.print("C");

    lcd.setCursor(0,1);
    lcd.print("Air: NORMAL");
  }

  // ===== SEND DATA =====
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    String url = serverName;
    url += "?temp=" + String(temp);
    url += "&mq2=" + String(gasState);
    url += "&mq135=" + String(airQuality);

    http.begin(url);
    int httpCode = http.GET();

    Serial.print("HTTP Response: ");
    Serial.println(httpCode);

    http.end();
  }

  delay(5000); // send every 5 sec
}