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
//#include "DFRobot_PH.h"

#define BUILTIN_LED 2
#define SENSOR  13
#define pinlvkaustik 5
#define sensorPin A0
#define RelayPin 14
int read_value = 0;
float regresi = 0, a = 0, b = 0, c = 0;
float voltage, sensorValue, pH, phValue, phRe, temperature = 25, phb1 = 7.00, phb2 = 10.01, phv1 = 496, phv2 = 333;
float offsite1back, offsite2back, offsite3back, offsite4back, offsite5back = 1, offsite6back = 1, offsite7back = 1;

uint8_t MACmaster[] = { 0x30, 0xc6, 0xf7, 0x2f, 0xa6, 0x30 };
unsigned int readingId = 0;
int BOARD_ID = 3;//BOARD ID>>>>>>>>>>>>
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
int Liquid_levelka;
boolean ledState = LOW;
float calibrationFactor = 4.5;
volatile int pulseCount;
//byte sensorInterrupt = 13; // GPIO13
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
    digitalWrite(BUILTIN_LED, LOW);
    i = 0;
  }
  else {
    Serial.println("Delivery fail");
    digitalWrite(BUILTIN_LED, HIGH);
  }
}
void OnRecv(uint8_t * mac, uint8_t *data_feedbackepal, uint8_t len) {
  memcpy(&myfeedbackepal, data_feedbackepal , sizeof(myfeedbackepal));
  Serial.print("Bytes received: ");
  Serial.println(len);
  offsite1back = myfeedbackepal.offsite1;//ph1
  offsite2back = myfeedbackepal.offsite2;//flow1
  offsite3back = myfeedbackepal.offsite3;//do2
  offsite4back = myfeedbackepal.offsite4;//suhu2
  offsite5back = myfeedbackepal.offsite5;//ph3
  offsite6back = myfeedbackepal.offsite6;//flow3
  offsite7back = myfeedbackepal.offsite7;//ph4
  Serial.println(offsite1back);
  Serial.println(offsite2back);
  Serial.println(offsite3back);
  Serial.println(offsite4back);
  Serial.println(offsite5back);
  Serial.println(offsite6back);
  Serial.println(offsite7back);
}

// void IRAM_ATTR pulseCounter()
// {
//   pulseCount++;
// }

void setup() {
  Serial.begin(115200);
  // if (!SPIFFS.begin()) {
  //   Serial.println("An Error has occurred while mounting SPIFFS");
  //   return;
  // } else {
  //   Serial.println("SPIFFS oke");
  // }
  WiFi.mode(WIFI_STA);
  // Get Mac Add
  Serial.print("Mac Address: ");
  Serial.print(WiFi.macAddress());
  Serial.println("ESP-Now Sender");

  // Initializing the ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Problem during ESP-NOW init");
    return;
  }
  delay(1); // This delay seems to make it work more reliably???
  pinMode(BUILTIN_LED, OUTPUT);
  pinMode(SENSOR, INPUT);
  pinMode(RelayPin, OUTPUT);
  pulseCount = 0;
  flowRate = 0.0;
  flowLitres = 0;
  //totalflowfloat = 0;
  //totalflowfloat = readFile(SPIFFS, "/nilaiflow.txt").toFloat();;
  delay(10);
  Serial.print("TOTAL FLOW =");
  Serial.println(totalflow);
  previousMillis = 0;
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  Serial.println("Registering a peer");
  esp_now_add_peer(MACmaster, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
  Serial.println("Registering send callback function");
  esp_now_register_send_cb(onSent);
  esp_now_register_recv_cb(OnRecv);
  //attachInterrupt(digitalPinToInterrupt(SENSOR), pulseCounter, FALLING);
}

void loop() {
  currentMillis = millis();
  if (currentMillis - previousMillis > 1000) {
    pulse1Sec = pulseCount;
    pulseCount = 0;
    flowRate = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) / calibrationFactor;
    previousMillis = millis();
    flowLitres = (flowRate / 60);
    totalflowfloat += flowLitres;
    totalflow = String(totalflowfloat);
    totalLitres = totalflowfloat / offsite6back;
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

  if (currentMillis - previousMillis2 >= 60000) {
    //writeFile(SPIFFS, "/nilaiflow.txt", totalflow.c_str());
    previousMillis2 = currentMillis;
    delay(500);
  }
  if (currentMillis - previousMillis3 >= 3000) {
    previousMillis3 = currentMillis;
    //Liquid_levelka = digitalRead(pinlvkaustik);
    Get_pH();
    myepal.id = BOARD_ID;
    myepal.ph1 = 0;
    myepal.valve1 = 0;
    myepal.valve1a = 0;
    myepal.flow1 = 0;
    myepal.do2 = 0;
    myepal.suhu2 = 0;
    myepal.dosing3 = h;
    myepal.ph3 = pH;
    myepal.flow3 = totalLitres;
    myepal.dosing4 = 0;
    myepal.ph4 = 0;
    myepal.dosing5 = 0;
    myepal.dosing5a = 0;
    myepal.level6 = 0;
    //myepal.level7 = Liquid_levelka;
    myepal.level7 = 0;
    myepal.level8 = 0;
    myepal.level9 = 0;
    Serial.println("Send a new message");
    esp_now_send(MACmaster, (uint8_t *) &myepal, sizeof(myepal));
    delay(500);
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

  if (pH > 10) {
    digitalWrite(RelayPin, LOW);
    Serial.println("lebih dari 10");
    h = 0;
  }
  else {
    digitalWrite(RelayPin, HIGH);
    Serial.println("kurang dari 10");
    h = 1;
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