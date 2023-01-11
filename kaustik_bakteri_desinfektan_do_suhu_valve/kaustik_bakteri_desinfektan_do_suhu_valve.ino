#include <esp_now.h>
#include <WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include <EEPROM.h>
#include <SPIFFS.h>
#include <FS.h>
#include <OneWire.h>
#include <DallasTemperature.h>
// GPIO where the DS18B20 is connected to

#define oneWireBus 5
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);
#define ledindikator 2
//#define phkaustik 34
#define flowpin2 34  //custom
#define pheffluen 35
#define flowpin 13
#define doeffluen 32
#define relayPin_kaustik 33
#define relayPin_bakteri 27
#define relayPin_defsinfektan 23
#define relayPin_good 19
#define relayPin_bad 18
#define VREF 5000            //VREF (mv)
#define ADC_RES 1024         //ADC Resolution
#define pinlvbakteri 4       //d5
#define pinlvdesinfektan 14  //d6
#define pinlvkaustik 22      //d7
#define pinlvkoagulan 16
//Single-point calibration Mode=0
//Two-point calibration Mode=1
#define TWO_POINT_CALIBRATION 0

#define READ_TEMP (25)  //Current water temperature ℃, Or temperature sensor function

//Single point calibration needs to be filled CAL1_V and CAL1_T
#define CAL1_V (1600)  //mv
#define CAL1_T (25)    //℃
//Two-point calibration needs to be filled CAL2_V and CAL2_T
//CAL1 High temperature point, CAL2 Low temperature point
#define CAL2_V (1300)  //mv
#define CAL2_T (15)    //℃

const uint16_t DO_Table[41] = {
  14460, 14220, 13820, 13440, 13090, 12740, 12420, 12110, 11810, 11530,
  11260, 11010, 10770, 10530, 10300, 10080, 9860, 9660, 9460, 9270,
  9080, 8900, 8730, 8570, 8410, 8250, 8110, 7960, 7820, 7690,
  7560, 7430, 7300, 7180, 7070, 6950, 6840, 6730, 6630, 6530, 6410
};
uint8_t Temperaturet;
uint16_t ADC_Raw;
uint16_t ADC_Voltage;
uint16_t DO;

int16_t readDO(uint32_t voltage_mv, uint8_t temperature_c) {
#if TWO_POINT_CALIBRATION == 0
  uint16_t V_saturation = (uint32_t)CAL1_V + (uint32_t)35 * temperature_c - (uint32_t)CAL1_T * 35;
  return (voltage_mv * DO_Table[temperature_c] / V_saturation);
#else
  uint16_t V_saturation = (int16_t)((int8_t)temperature_c - CAL2_T) * ((uint16_t)CAL1_V - CAL2_V) / ((uint8_t)CAL1_T - CAL2_T) + CAL2_V;
  return (voltage_mv * DO_Table[temperature_c] / V_saturation);
#endif
}
float calibrationFactor = 4.5;
volatile int pulseCount;
volatile int pulseCount2;  //custom
//byte sensorInterrupt = 13; // GPIO13
byte pulse1Sec = 0;
byte pulse1Sec2 = 0;  //custom
float flowRate;
float flowRate2;
unsigned long flowMilliLitres;
unsigned int totalMilliLitres;
float flowLitres;
float totalLitres;
float totalflowfloat;
String totalflow;

unsigned long flowMilliLitres2;
unsigned int totalMilliLitres2;
float flowLitres2;
float totalLitres2;
float totalflowfloat2;
String totalflow2;

float doValue;
int read_value = 0;
float regresi = 0, a = 0, b = 0, c = 0;
float voltage_effluen, sensorValue_effluen, pH_effluen, phValue_effluen, phRe_effluen, temperature_effluen = 25, phb1_effluen = 7.00, phb2_effluen = 10.01, phv1_effluen = 496, phv2_effluen = 333;
float voltage_kaustik, sensorValue_kaustik, pH_kaustik, phValue_kaustik, phRe_kaustik, temperature_kaustik = 25, phb1_kaustik = 7.00, phb2_kaustik = 10.01, phv1_kaustik = 496, phv2_kaustik = 333;
float offsite1back = 1, offsite2back = 1, offsite3back = 1, offsite4back = 1, offsite5back = 1, offsite6back = 1, offsite7back = 1;
uint8_t MACmaster[] = { 0x94, 0xb5, 0x55, 0x18, 0x53, 0x74 };  //94:B5:55:18:53:74
unsigned int readingId = 0;
int BOARD_ID = 1;
int h_kaustik, f_kaustik, h_effluen, f_effluen, h_defsinfektan, h_bakteri;
int Liquid_levelbak = 0;
int Liquid_levelde = 0;
int Liquid_levelko = 0;
int Liquid_levelka = 0;

unsigned long previousMillis1 = 0;
unsigned long previousMillis2 = 0;
unsigned long previousMillis3 = 0;
unsigned long previousMillis4 = 0;

typedef struct epal {  // data isinya PH debit dan status dosing
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

typedef struct feedbackepal {  // data isinya PH debit dan status dosing
  float offsite1;
  float offsite2;
  float offsite3;
  float offsite4;
  float offsite5;
  float offsite6;
  float offsite7;
} feedbackepal;
feedbackepal myfeedbackepal;
//RTOS>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TaskHandle_t Task1;
TaskHandle_t Task2;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  if (status == ESP_NOW_SEND_SUCCESS) {
    digitalWrite(ledindikator, HIGH);
  } else {
    digitalWrite(ledindikator, LOW);
  }
}

void OnDataRecv(const uint8_t *mac, const uint8_t *data_feedbackepal, int len) {
  char macStr[18];
  Serial.print("Packet received from: ");
  digitalWrite(ledindikator, HIGH);
  delay(200);
  digitalWrite(ledindikator, LOW);
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.println(macStr);
  memcpy(&myfeedbackepal, data_feedbackepal, sizeof(myfeedbackepal));
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
  Serial.println();
}

void IRAM_ATTR pulseCounter() {
  pulseCount++;
}
void IRAM_ATTR pulseCounter2() {
  pulseCount2++;
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(ledindikator, OUTPUT);
  pinMode(relayPin_bakteri, OUTPUT);
  pinMode(relayPin_defsinfektan, OUTPUT);
  pinMode(relayPin_kaustik, OUTPUT);
  pinMode(relayPin_bad, OUTPUT);
  pinMode(relayPin_good, OUTPUT);
  pinMode(ledindikator, OUTPUT);
  // pinMode(phkaustik, INPUT);
  // pinMode(pheffluen, INPUT);
  pinMode(flowpin, INPUT_PULLUP);
  pinMode(doeffluen, INPUT);
  pinMode(pinlvbakteri, INPUT);
  pinMode(pinlvdesinfektan, INPUT);
  pinMode(pinlvkaustik, INPUT);
  pinMode(pinlvkoagulan, INPUT);
  //Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  Serial.print("Mac Address: ");
  Serial.print(WiFi.macAddress());
  Serial.println("ESP-Now Receiver");
  //Init ESP-NOW
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(OnDataRecv);
  esp_now_register_send_cb(OnDataSent);
  // Adding Peer Devices
  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));
  for (int ii = 0; ii < 6; ++ii) {
    peerInfo.peer_addr[ii] = (uint8_t)MACmaster[ii];
  }
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
  xTaskCreatePinnedToCore(
    Task1code, /* Fungsi RTOS */
    "Task1",   /* Nama RTOS */
    10000,     /* Ukuran RTOS */
    NULL,      /* Parameter RTOS */
    1,         /* Prioritas Tugas */
    &Task1,    /* Task handle to keep track of created task */
    0);        /* Pilihan Core RTOS Core 0*/
  delay(500);
  //create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
    Task2code, /* Fungsi RTOS */
    "Task2",   /* Nama RTOS */
    10000,     /* Ukuran RTOS */
    NULL,      /* Parameter RTOS */
    1,         /* Prioritas Tugas */
    &Task2,    /* Task handle to keep track of created task */
    1);        /* Pilihan Core RTOS Core 1 */
  pulseCount = 0;
  flowRate = 0.0;
  flowLitres = 0;
  //totalflowfloat = 0;
  totalflowfloat = readFile(SPIFFS, "/nilaiflow.txt").toFloat();
  delay(10);
  Serial.print("TOTAL FLOW =");
  Serial.println(totalflow);
  attachInterrupt(digitalPinToInterrupt(flowpin), pulseCounter, FALLING);
  pulseCount2 = 0;
  flowRate2 = 0.0;
  flowLitres2 = 0;
  //totalflowfloat = 0;
  totalflowfloat2 = readFile(SPIFFS, "/nilaiflow2.txt").toFloat();
  delay(10);
  Serial.print("TOTAL FLOW =");
  Serial.println(totalflow);
  attachInterrupt(digitalPinToInterrupt(flowpin2), pulseCounter2, FALLING);
}

void loop() {
  // put your main code here, to run repeatedly:
}

void Task1code(void *pvParameters) {
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());

  for (;;) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis1 > 1000) {
      pulse1Sec = pulseCount;
      pulseCount = 0;
      flowRate = ((1000.0 / (millis() - previousMillis1)) * pulse1Sec) / calibrationFactor;
      previousMillis1 = currentMillis;
      flowLitres = (flowRate / 60);
      totalflowfloat += flowLitres;
      totalflow = String(totalflowfloat);
      totalLitres = totalflowfloat / offsite6back;
      // Print the flow rate for this second in litres / minute
      Serial.print("Flow rate: ");
      Serial.print(float(flowRate));  // Print the integer part of the variable
      Serial.print("L/min");
      Serial.print("\t");  // Print tab space
      // Print the cumulative total of litres flowed since starting
      Serial.print("Output Liquid Quantity: ");
      Serial.print(totalflow);
      Serial.println("L");
    }
    if (currentMillis - previousMillis4 > 1300) {
      pulse1Sec2 = pulseCount2;
      pulseCount2 = 0;
      flowRate2 = ((1000.0 / (millis() - previousMillis4)) * pulse1Sec2) / calibrationFactor;
      previousMillis4 = currentMillis;
      flowLitres2 = (flowRate2 / 60);
      totalflowfloat2 += flowLitres2;
      totalflow2 = String(totalflowfloat2);
      totalLitres2 = totalflowfloat2 / offsite6back;
      // Print the flow rate for this second in litres / minute
      Serial.print("Flow rate2: ");
      Serial.print(float(flowRate2));  // Print the integer part of the variable
      Serial.print("L/min");
      Serial.print("\t");  // Print tab space
      // Print the cumulative total of litres flowed since starting
      Serial.print("Output Liquid Quantity2: ");
      Serial.print(totalflow2);
      Serial.println("L");
    }

    if (currentMillis - previousMillis2 >= 2000) {
      previousMillis2 = currentMillis;
      //Get_pH_kaustik();
      Get_pH_effluen();
      sensorlain();
    }
    if (currentMillis - previousMillis3 >= 3000) {
      previousMillis3 = currentMillis;
      writeFile(SPIFFS, "/nilaiflow.txt", totalflow.c_str());
      writeFile(SPIFFS, "/nilaiflow2.txt", totalflow.c_str());
      myepal.id = BOARD_ID;
      myepal.ph1 = pH_effluen;
      myepal.valve1 = h_effluen;
      myepal.valve1a = f_effluen;
      myepal.flow1 = totalLitres;
      myepal.do2 = doValue;
      myepal.suhu2 = Temperaturet;
      myepal.dosing3 = h_kaustik;
      myepal.ph3 = pH_kaustik;
      myepal.flow3 = totalLitres2;
      myepal.dosing4 = 0;
      myepal.ph4 = 0;
      myepal.dosing5 = h_defsinfektan;
      myepal.dosing5a = h_bakteri;
      myepal.level6 = Liquid_levelbak;
      myepal.level7 = Liquid_levelka;
      myepal.level8 = Liquid_levelko;
      myepal.level9 = Liquid_levelde;
      Serial.println("Send a new message");
      // Broadcasting data (JSON) via ESP-NOW
      esp_err_t result = esp_now_send(MACmaster, (uint8_t *)&myepal, sizeof(myepal));
      delay(100);
      if (result == ESP_OK) {
        Serial.println("Sent with success");
      } else {
        Serial.println(result);
      }
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void Task2code(void *pvParameters) {
  Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());
  for (;;) {
    h_bakteri = 1;
    h_defsinfektan = 1;
    digitalWrite(relayPin_bakteri, HIGH);
    delay(3947);
    digitalWrite(relayPin_bakteri, LOW);
    digitalWrite(relayPin_defsinfektan, HIGH);
    delay(23682);
    h_bakteri = 0;
    h_defsinfektan = 0;
    digitalWrite(relayPin_defsinfektan, LOW);
    vTaskDelay(3600000 / portTICK_PERIOD_MS);
  }
}
void Get_pH_effluen() {
  int currentValue_effluen = 0;
  for (int i = 0; i < 10; i++) {
    currentValue_effluen += analogRead(pheffluen);
    delay(10);
  }
  sensorValue_effluen = (currentValue_effluen / 10);
  float m_effluen = (phb2_effluen - phb1_effluen) / (phv2_effluen - phv1_effluen);
  Serial.println(m_effluen);
  float c_effluen = phb2_effluen - (m_effluen * phv2_effluen);
  Serial.println(c_effluen);
  pH_effluen = ((m_effluen * sensorValue_effluen) + c_effluen) * offsite1back;
  Serial.print("V EFFLUEN:");
  Serial.println(sensorValue_effluen);
  Serial.print("PH EFFLUEN:");
  Serial.println(pH_effluen);
  if (6 < pH_effluen && pH_effluen < 9) {
    digitalWrite(relayPin_good, HIGH);
    digitalWrite(relayPin_bad, LOW);
    Serial.println("GOOD");
    h_effluen = 1;
    f_effluen = 0;
  } else {
    digitalWrite(relayPin_good, LOW);
    digitalWrite(relayPin_bad, HIGH);
    Serial.println("BAD");
    h_effluen = 0;
    f_effluen = 1;
  }
}
// void Get_pH_kaustik() {
//   int currentValue_kaustik = 0;
//   for (int i = 0; i < 10; i++) {
//     currentValue_kaustik += analogRead(phkaustik);
//     delay(10);
//   }
//   sensorValue_kaustik = (currentValue_kaustik / 10);
//   float m_kaustik = (phb2_kaustik - phb1_kaustik) / (phv2_kaustik - phv1_kaustik);
//   Serial.println(m_kaustik);
//   float c_kaustik = phb2_kaustik - (m_kaustik * phv2_kaustik);
//   Serial.println(c_kaustik);
//   Serial.print("offsite1 = ");
//   Serial.println(offsite1back);
//   pH_kaustik = ((m_kaustik * sensorValue_kaustik) + c_kaustik) * offsite5back;
//   Serial.print("v KAUSTIK:");
//   Serial.println(sensorValue_kaustik);
//   Serial.print("ph KAUSTIK:");
//   Serial.println(pH_kaustik);
//   if (pH_kaustik > 10) {
//     digitalWrite(relayPin_kaustik, LOW);
//     Serial.println("lebih dari 10");
//     h_kaustik = 0;
//   } else {
//     digitalWrite(relayPin_kaustik, HIGH);
//     Serial.println("kurang dari 10");
//     h_kaustik = 1;
//   }
// }
void sensorlain() {
  sensors.requestTemperatures();
  Temperaturet = sensors.getTempCByIndex(0);
  ADC_Raw = analogRead(doeffluen);
  ADC_Voltage = uint32_t(VREF) * ADC_Raw / ADC_RES;
  Serial.print("Temperaturet:\t" + String(Temperaturet) + "\t");
  Serial.print("ADC RAW:\t" + String(ADC_Raw) + "\t");
  Serial.print("ADC Voltage:\t" + String(ADC_Voltage) + "\t");
  Serial.println("DO:\t" + String(readDO(ADC_Voltage, Temperaturet)) + "\t");
  doValue = (0.0001 * readDO(ADC_Voltage, Temperaturet));
  Liquid_levelbak = digitalRead(pinlvbakteri);
  Liquid_levelde = digitalRead(pinlvdesinfektan);
  Liquid_levelko = digitalRead(pinlvkaustik);
  Liquid_levelka = digitalRead(pinlvkoagulan);
  Serial.print("Liquid_levelbak= ");
  Serial.println(Liquid_levelbak, DEC);
  Serial.print("Liquid_levelde= ");
  Serial.println(Liquid_levelde, DEC);
  Serial.print("Liquid_levelko= ");
  Serial.println(Liquid_levelko, DEC);
  Serial.print("Liquid_levelka= ");
  Serial.println(Liquid_levelka, DEC);
}
String readFile(fs::FS &fs, const char *path) {
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

void writeFile(fs::FS &fs, const char *path, const char *message) {
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