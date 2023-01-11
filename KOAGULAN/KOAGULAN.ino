/**
  ESP-NOW
  Sender developed by hanif
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <espnow.h>
#include <SPI.h>
#include <Wire.h>
#include <EEPROM.h>
#include <FS.h>
#include "DFRobot_PH.h"
int LED = 2;      // Assign LED1 to pin GPIO2
#define flowpin 13
#define sensorPin A0
#define RelayPin 14
int address = 50;
DFRobot_PH ph;
int read_value = 0;
float regresi = 0, a = 0, b = 0, c = 0;
float offsite1back = 1, offsite2back = 1, offsite3back = 1, offsite4back = 1, offsite5back = 1, offsite6back = 1, offsite7back = 1;
float voltage, sensorValue, pH, phValue, phRe, temperature = 25, phb1 = 7.00, phb2 = 10.01, phv1 = 496, phv2 = 333;
uint8_t MACmaster[] = {0x94, 0xb5, 0x55, 0x18, 0x53, 0x74};//94:B5:55:18:53:74
unsigned int readingId = 0;
int BOARD_ID = 4;//BOARD ID>>>>>>>>>>>>
typedef struct epal { // data isinya PH debit dan status dosing
  int id;
  float ph1;
  int valve1;
  int valve1a;
  float flow1;

  float do2;
  float suhu2;
  int dosing3;
  float ph3;
  float flow3;

  int dosing4;
  float ph4;

  int dosing5;
  int dosing5a;

  int level6;
  int level7;
  int level8;
  int level9;
} epal;
epal myepal;

typedef struct feedbackepal { // data isinya PH debit dan status dosing
  float offsite1;
  float offsite2;
  float offsite3;
  float offsite4;
  float offsite5;
  float offsite6;
  float offsite7;
} feedbackepal;
feedbackepal myfeedbackepal;

long currentMillis = 0;
long previousMillis = 0;
long previousMillis2 = 0;
long previousMillis3 = 0;
int interval = 1000;
boolean ledState = LOW;
float calibrationFactor = 4.5;
volatile byte pulseCount;
byte pulse1Sec = 0;
float flowRate;
unsigned long flowMilliLitres;
unsigned int totalMilliLitres;
float flowLitres;
float totalLitres;
float totalflowfloat;
String totalflow;
int h;

void onSent(uint8_t *mac1, uint8_t sendStatus) {
  int i;
  if (sendStatus == 0) {
    Serial.println("Delivery success");
    digitalWrite(LED, LOW);
    i = 0;
  }
  else {
    Serial.println("Delivery fail");
    digitalWrite(LED, HIGH);
  }
} void OnRecv(uint8_t * mac, uint8_t *data_feedbackepal, uint8_t len) {
  memcpy(&myfeedbackepal, data_feedbackepal , sizeof(myfeedbackepal));
  Serial.print("Bytes received: ");
  Serial.println(len);
  offsite1back = myfeedbackepal.offsite1;
  offsite2back = myfeedbackepal.offsite2;
  offsite3back = myfeedbackepal.offsite3;
  offsite4back = myfeedbackepal.offsite4;
  offsite5back = myfeedbackepal.offsite5;
  offsite6back = myfeedbackepal.offsite6;
  offsite7back = myfeedbackepal.offsite7;
  Serial.println(offsite1back);
  Serial.println(offsite2back);
  Serial.println(offsite3back);
  Serial.println(offsite4back);
  Serial.println(offsite5back);
  Serial.println(offsite6back);
  Serial.println(offsite7back);
}

void IRAM_ATTR pulseCounter()
{
  pulseCount++;
}

void setup() {
  Serial.begin(115200);
  if (!SPIFFS.begin()) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  } else {
    Serial.println("SPIFFS oke");
  }
  pinMode(LED, OUTPUT);
  WiFi.mode(WIFI_STA);
  // Get Mac Add
  Serial.print("Mac Address: ");
  Serial.print(WiFi.macAddress());
  Serial.println("ESP-Now Sender");
  pinMode(flowpin, INPUT_PULLUP);
  pinMode(RelayPin, OUTPUT);
  pulseCount = 0;
  flowRate = 0.0;
  flowLitres = 0;
  //totalflowfloat = 1;

  totalflowfloat = readFile(SPIFFS, "/nilaiflow.txt").toFloat();
  // Initializing the ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Problem during ESP-NOW init");
    return;
  }
  delay(1); // This delay seems to make it work more reliably???
  delay(100);
  Serial.print("TOTAL FLOW =");
  Serial.println(totalflowfloat);
  previousMillis = 0;
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  Serial.println("Registering a peer");
  esp_now_add_peer(MACmaster, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
  Serial.println("Registering send callback function");
  esp_now_register_send_cb(onSent);
  esp_now_register_recv_cb(OnRecv);
  attachInterrupt(digitalPinToInterrupt(flowpin), pulseCounter, FALLING);
}

void loop() {
  currentMillis = millis();
  if (currentMillis - previousMillis > 1000)
  {
    pulse1Sec = pulseCount;
    pulseCount = 0;
    flowRate = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) / calibrationFactor;
    previousMillis = millis();
    flowLitres = (flowRate / 60);
    totalflowfloat += flowLitres;
    totalflow = String(totalflowfloat);
    totalLitres = totalflowfloat / offsite2back;

    // Print the flow rate for this second in litres / minute
    Serial.print("Flow rate: ");
    Serial.print(float(flowRate));  // Print the integer part of the variable
    Serial.print("L/min");
    Serial.print("\t");       // Print tab space
    // Print the cumulative total of litres flowed since starting
    Serial.print("Output Liquid Quantity: ");
    Serial.print(totalflow);
    Serial.println("L");
  }

  if (currentMillis - previousMillis3 >= 3000) {
    previousMillis3 = currentMillis;
    Get_pH();
    writeFile(SPIFFS, "/nilaiflow.txt", totalflow.c_str());
    myepal.id = BOARD_ID;
    myepal.ph1 = 0;
    myepal.valve1 = 0;
    myepal.valve1a = 0;
    myepal.flow1 = 0;
    myepal.do2 = 0;
    myepal.suhu2 = 0;
    myepal.dosing3 = 0;
    myepal.ph3 = 0;
    myepal.flow3 = totalLitres ;
    myepal.dosing4 = h;
    myepal.ph4 = pH;
    myepal.dosing5 = 0;
    myepal.dosing5a = 0;
    myepal.level6 = 0;
    myepal.level7 = 0;
    myepal.level8 = 0;
    myepal.level9 = 0;
    Serial.println("Send a new message");
    esp_now_send(MACmaster, (uint8_t *) &myepal, sizeof(myepal));
  }
}

void Get_pH()
{
  int currentValue = 0;
  for (int i = 0; i < 10; i++)
  {
    currentValue += analogRead(sensorPin);
    delay(100);
  }
  sensorValue = (currentValue / 10);
  float m = (phb2 - phb1) / (phv2 - phv1);
  Serial.println(m);
  float c = phb2 - (m * phv2);
  Serial.println(c);
  pH = ((m * sensorValue) + c) * offsite7back;
  Serial.print("v:");
  Serial.println(sensorValue);
  Serial.println(pH);

  if (pH > 8) {
    digitalWrite(RelayPin, HIGH);
    Serial.println("lebih dari 8");
    h = 1;
  }
  else {
    digitalWrite(RelayPin, LOW);
    Serial.println("kurang dari 8");
    h = 0;
  }
}
String readFile(fs::FS & fs, const char * path) {
  Serial.printf("Reading file: %s\r\n", path);
  File file = fs.open(path, "r");
  if (!file || file.isDirectory()) {
    Serial.println("- empty file or failed to open file");
    return String();
  }
  Serial.println("- read from file:");
  String fileContent;
  while (file.available()) {
    fileContent += String((char)file.read());
  }
  Serial.println(fileContent);
  return fileContent;
  file.close();
}

void writeFile(fs::FS & fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\r\n", path);
  File file = fs.open(path, "w");
  if (!file) {
    Serial.println("- failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }
  file.close();
}
