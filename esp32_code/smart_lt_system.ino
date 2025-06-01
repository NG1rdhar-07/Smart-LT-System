#define BLYNK_TEMPLATE_ID "Add yours :)"
#define BLYNK_TEMPLATE_NAME "SmartLT"
#define BLYNK_AUTH_TOKEN "Add yours :)"

#include <WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <Stepper.h>
#include <HTTPClient.h>
#include <time.h>
#include <BlynkSimpleEsp32.h>
#include <WebServer.h>

char ssid[] = "Add yours :)";
char pass[] = "Add yours :)";
char auth[] = BLYNK_AUTH_TOKEN;

const String thingSpeakApiKey = "Add yours :)";
const String thingSpeakServer = "http://api.thingspeak.com/update";

const long gmtOffset_sec = 19800;
const int daylightOffset_sec = 0;

#define DHTPIN 2
#define DHTTYPE DHT22
#define PIR_PIN 3
#define LED_TEMP 4
#define LED_LDR 5
#define LED_MOTION 6
#define LDR_PIN 0

Stepper myStepper(2048, 7, 8, 9, 10);
LiquidCrystal_I2C lcd(0x27, 16, 2);

#define TEMP_THRESHOLD 21.0
#define LDR_THRESHOLD 500

DHT dht(DHTPIN, DHTTYPE);
WebServer server(80);

unsigned long previousMillis = 0;
unsigned long lastTempCheck = 0;
unsigned long lastThingSpeakUpdate = 0;
const unsigned long lcdInterval = 3000;
const unsigned long tempInterval = 2000;
const unsigned long thingSpeakInterval = 15000;

bool motionDetected = false;
int displayState = 0;
bool displayCustomText = false;
String customLcdText = "";

float temp, hum;
int ldr;
int ldrInverted;
bool motion;

char lcdLine1[17] = "";
char lcdLine2[17] = "";
String latestRollNumber = "";

unsigned long customTextStartMillis = 0;
const unsigned long customTextDuration = 5000;  // 5 seconds

void setup() {
  Serial.begin(115200);
  Wire.begin(19, 18);
  lcd.init();
  lcd.backlight();

  dht.begin();
  myStepper.setSpeed(10);

  pinMode(LED_TEMP, OUTPUT);
  pinMode(LED_LDR, OUTPUT);
  pinMode(LED_MOTION, OUTPUT);
  pinMode(PIR_PIN, INPUT);

  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    lcd.print(".");
  }

  Blynk.begin(auth, ssid, pass);

  configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org", "time.nist.gov");
  while (time(nullptr) < 100000) {
    delay(500);
  }

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Server started");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP().toString());
  delay(3000);
}

void loop() {
  unsigned long currentMillis = millis();
  Blynk.run();
  server.handleClient();
  readSensors();

  if (displayCustomText) {
    displayCustomMessageOnLCD();
    if (millis() - customTextStartMillis > customTextDuration) {
      displayCustomText = false;
      lcd.clear();  // Clear the custom message after timeout
    }
  } else {
    updateLCD(currentMillis);
  }

  handleTempActions(currentMillis);
  sendToThingSpeak(currentMillis);
  updateBlynk();
}

void handleRoot() {
  String html = "<!DOCTYPE HTML><html><head><title>IoT Monitoring</title>"
                "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
                "<style>html{font-family:Arial;text-align:center;}h2{font-size:2rem;}"
                ".card{background:white;border-radius:10px;box-shadow:0 2px 10px rgba(0,0,0,0.2);"
                "margin:20px;padding:15px;}</style></head><body><h2>IoT Monitoring</h2>"
                "<div class='card'><p>Temperature: " + String(temp,1) + " °C</p></div>"
                "<div class='card'><p>Humidity: " + String(hum,1) + " %</p></div>"
                "<div class='card'><p>Light Level: " + String(ldrInverted) + "</p></div>"
                "<div class='card'><p>Motion: " + (motion ? "Detected" : "Not Detected") + "</p></div>"
                "</body></html>";
  server.send(200, "text/html", html);
}

void handleData() {
  String json = "{";
  json += "\"temperature\":" + String(temp, 1) + ","; 
  json += "\"humidity\":" + String(hum, 1) + ","; 
  json += "\"light\":" + String(ldrInverted) + ","; 
  json += "\"motion\":" + String(motion ? "true" : "false"); 
  json += "}";
  server.send(200, "application/json", json);
}  
void readSensors() {
  temp = dht.readTemperature();
  hum = dht.readHumidity();
  ldr = analogRead(LDR_PIN);
  ldrInverted = 4095 - ldr;
  motion = digitalRead(PIR_PIN);

  digitalWrite(LED_LDR, ldr > LDR_THRESHOLD ? HIGH : LOW);
  motionDetected = (motion == HIGH);

  if (motionDetected) {
    for (int i = 0; i < 3; i++) {
      digitalWrite(LED_MOTION, HIGH); delay(200);
      digitalWrite(LED_MOTION, LOW); delay(200);
    }
  }
}

void updateLCD(unsigned long now) {
  if (now - previousMillis >= lcdInterval) {
    previousMillis = now;
    lcd.clear();

    switch (displayState) {
      case 0: displayTime(); break;
      case 1: displayTemperature(); break;
      case 2: displayHumidity(); break;
    }

    displayState = (displayState + 1) % 3;
  }
}

void displayCustomMessageOnLCD() {
  lcd.clear();
  int lineBreakPos = customLcdText.indexOf('\n');

  if (lineBreakPos >= 0) {
    String firstLine = customLcdText.substring(0, lineBreakPos);
    String secondLine = customLcdText.substring(lineBreakPos + 1);

    lcd.setCursor(0, 0); lcd.print(firstLine.substring(0, 16));
    lcd.setCursor(0, 1); lcd.print(secondLine.substring(0, 16));
  } else {
    lcd.setCursor(0, 0); lcd.print(customLcdText.substring(0, 16));
  }
}

void displayTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    strcpy(lcdLine1, "Time Error");
    lcd.setCursor(0, 0); lcd.print(lcdLine1);
    return;
  }
  char timeStr[9];
  strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
  lcd.setCursor(0, 0); lcd.print("Current Time:");
  lcd.setCursor(0, 1); lcd.print(timeStr);
}

void displayTemperature() {
  lcd.setCursor(0, 0); lcd.print("Temp:");
  lcd.setCursor(0, 1); lcd.print(isnan(temp) ? "Read Error" : String(temp, 1) + " C");
}

void displayHumidity() {
  lcd.setCursor(0, 0); lcd.print("Humidity:");
  lcd.setCursor(0, 1); lcd.print(isnan(hum) ? "Read Error" : String(hum, 1) + " %");
}

void handleTempActions(unsigned long now) {
  if (now - lastTempCheck >= tempInterval) {
    lastTempCheck = now;
    if (!isnan(temp)) {
      digitalWrite(LED_TEMP, (temp > TEMP_THRESHOLD && motionDetected) ? HIGH : LOW);
    }
  }
}

void rotateStepperForward() {
  myStepper.step(2048);
}

void rotateStepperReverse() {
  myStepper.step(-2048);
}

void sendToThingSpeak(unsigned long now) {
  if (now - lastThingSpeakUpdate >= thingSpeakInterval && WiFi.status() == WL_CONNECTED) {
    lastThingSpeakUpdate = now;
    HTTPClient http;
    String url = thingSpeakServer + "?api_key=" + thingSpeakApiKey +
                 "&field1=" + String(temp, 2) +
                 "&field2=" + String(hum, 2) +
                 "&field3=" + String(ldrInverted) +
                 "&field4=" + String(motion);
    http.begin(url);
    http.GET();
    http.end();
  }
}

void updateBlynk() {
  Blynk.virtualWrite(V4, temp);
  Blynk.virtualWrite(V8, hum);
  Blynk.virtualWrite(V1, ldrInverted);
}

// Blynk input handlers

// Optional message display
BLYNK_WRITE(V10) {
  String textInput = param.asStr();
  if (textInput.length() > 0) {
    customLcdText = textInput;
    displayCustomText = true;
    customTextStartMillis = millis();  // start timer
  } else {
    displayCustomText = false;
  }
}

// Direct Roll Number Entry: marks attendance
BLYNK_WRITE(V9) {
  latestRollNumber = param.asStr();

  if (WiFi.status() == WL_CONNECTED && latestRollNumber.length() > 0) {
    HTTPClient http;
    http.begin("http://192.168.87.67/:5000/store");  // Flask server URL
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String postData = "roll=" + latestRollNumber;
    int httpResponseCode = http.POST(postData);

    if (httpResponseCode > 0) {
      Serial.println("Roll number sent to Pi");
    } else {
      Serial.println("Error sending to Pi");
    }

    http.end();
  }
}

// Curtain control
BLYNK_WRITE(V5) {
  int curtainCommand = param.asInt();
  if (curtainCommand == 1) rotateStepperForward();
  else if (curtainCommand == 2) rotateStepperReverse();
  previousMillis = millis();
} 

void displayAttendanceOnLCD(String rollNumber) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(rollNumber);
  lcd.setCursor(0, 1);
  lcd.print(": Marked");

  displayCustomText = true;
  customLcdText = rollNumber + "\n: Marked";
  previousMillis = millis();
}
