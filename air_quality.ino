#define BLYNK_TEMPLATE_ID "TMPL2EFX1sppg"
#define BLYNK_TEMPLATE_NAME "Air Quality Monitoring"
#define BLYNK_AUTH_TOKEN "snZ4B3hrwUPDPT79FFO0lLgAvT_mprRg"

#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Pixel 7a";
char pass[] = "Vedant369";

BlynkTimer timer;
int gas = A0;
#define DHTPIN 2  
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ThingSpeak
WiFiClient client;
String apiKey = "LVVMPZUGTS82J2IM";
const char *server = "api.thingspeak.com";

byte degree_symbol[8] = {
  0b00111,
  0b00101,
  0b00111,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};

void sendToThingSpeak(float temperature, float humidity, int airQuality) {
  if (client.connect(server, 80)) {
    String postStr = "api_key=" + apiKey;
    postStr += "&field1=" + String(temperature);
    postStr += "&field2=" + String(humidity);
    postStr += "&field3=" + String(airQuality);
    postStr += "\r\n\r\n";

    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);

    Serial.println("✅ Sent to ThingSpeak");
  }
  client.stop();
}

void sendSensor() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  int analogSensor = analogRead(gas);

  if (isnan(h) || isnan(t)) {
    Serial.println("❌ Failed to read from DHT sensor!");
  } else {
    Serial.print("🌡 Temp: "); Serial.print(t); Serial.print(" °C  |  💧 Humidity: ");
    Serial.print(h); Serial.print(" %  |  Gas: "); Serial.println(analogSensor);
  }

  Blynk.virtualWrite(V0, t);
  Blynk.virtualWrite(V1, h);
  Blynk.virtualWrite(V2, analogSensor);
  sendToThingSpeak(t, h, analogSensor);
}

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("Connecting to Wi-Fi...");

  pinMode(gas, INPUT);
  Blynk.begin(auth, ssid, pass);
  dht.begin();
  timer.setInterval(30000L, sendSensor);

  Wire.begin();
  lcd.begin(16, 2);
  lcd.createChar(1, degree_symbol);
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Air Quality");
  lcd.setCursor(3, 1);
  lcd.print("Monitoring");
  delay(2000);
  lcd.clear();
}

void loop() {
  Blynk.run();
  timer.run();

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  int gasValue = analogRead(gas);

  lcd.setCursor(0, 0); lcd.print("Temperature ");
  lcd.setCursor(0, 1); lcd.print(t); lcd.setCursor(6, 1); lcd.write(1);
  lcd.setCursor(7, 1); lcd.print("C");
  delay(4000); lcd.clear();

  lcd.setCursor(0, 0); lcd.print("Humidity "); lcd.print(h); lcd.print("%");
  delay(4000); lcd.clear();

  lcd.setCursor(0, 0); lcd.print("Gas Value: "); lcd.print(gasValue);
  lcd.setCursor(0, 1);
  if (gasValue < 500) {
    lcd.print("Fresh Air");
    Serial.println("Fresh Air");
  } else {
    lcd.print("Bad Air");
    Serial.println("Bad Air");
    Blynk.logEvent("pollution_alert", "Bad Air");
  }
  delay(4000);
  lcd.clear();
}