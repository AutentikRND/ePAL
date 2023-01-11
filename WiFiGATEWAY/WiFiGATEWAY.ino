#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <strings_en.h>
#include <DNSServer.h>
#include <WiFiUdp.h>
#include "Nextion.h"
#include <HardwareSerial.h>
#include <SD.h>
#include "FS.h"
#define led 4
String recv_str_jsondata;
StaticJsonDocument<500> doc_send;
StaticJsonDocument<500> doc_recv;
WiFiManager wifiManager;
///SPI H
File myFile;
SPIClass SPISD(HSPI);
#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif
#define SD_MOSI 13
#define SD_MISO 32
#define SD_SCK 14
#define SD_CS 15
#define ss 5
#define rst 12
#define dio0 2
#define FILE_LINE_LENGTH 81  // a line has 80 chars
char txtLine[FILE_LINE_LENGTH];
String txtstring;
char postdata[FILE_LINE_LENGTH];
bool readCondition = true;  // Has to be defined somewhere to trigger SD read
WiFiClient client;
//User ID
String userId = "c8879e6e-db31-44e4-905e-ee87f238076a";
//ID Device
String idDevice = "7367a54e-6cf7-4b6b-b818-3f71454c909e";


//Email Account
String email = "talpha.autentik@gmail.com";
//Email Password
String pass = "tEHTmN";





String dayStamp;
String timeStamp;
String outgoing, last;
String serverName = "http://diawan.io/api/get_url";
String formattedDate;
String linkdiawan;
String name;
String namahari;
String nomortanggal;
String nilaiwaktu;
String lasttime;
String dataMessage;
String timestamprtc;
String waktu, jam, hari;
String level6, level7, level8, level9;
//FLOAT>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> Semua Variabel Float
float offsite1 = 1, offsite2 = 1, offsite3 = 1, offsite4 = 1, offsite5 = 1, offsite6 = 1, offsite7 = 1;
float PHboard1, flowboard1, DOboard2, suhuboard2, PHboard3, flowboard3, PHboard4;
float koreksi1 = 0;
float koreksi2 = 0;
float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0, g = 0, h = 0, i = 0, j = 0, k = 0, l = 0, o = 0, p = 0, q = 0, r = 0, s = 0, t = 0, u = 0, v = 0, w = 0;
//INTEGER>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> Semua Variabel Integer
int httpResponseCode = 200;
int statusvalveboard1, statusvalveboard1a, statusdosingboard3, statusdosingboard4, statusdosingboard5, statusdosingboard5a, levelboard6, levelboard7, levelboard8, levelboard9;
int state = 0;
int qualwifi = 0;
int intervalwaktu = 60;
int persen;
int val = 0;
int mati = 0;
int x = 1;
float data1, data1kal, data2, data2kal, data3, data3kal, data4, data4kal, data5, data5kal, data6, data6kal, data7, data7kal, data8, data8kal, data9, data9kal, data10, data10kal, data11, data11kal, data12, data12kal, data13, data13kal;
const int buz = 33;
//MIXED VARIABLES>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
unsigned int timeout = 120;
unsigned long previousMillis1 = 0;
unsigned long previousMillis2 = 0;
char dataHari[7][12] = { "SUNDAY", "MONDAY", "TUESDAY", "WEDNESDAY", "THURSDAY", "FRIDAY", "SATURDAY" };
bool portalRunning = false;
bool startAP = false;
unsigned int startTime = millis();
//Nextion>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
NexDSButton bt0 = NexDSButton(0, 1, "bt0");
NexDSButton bt1 = NexDSButton(0, 2, "bt1");
NexDSButton bt2 = NexDSButton(0, 3, "bt2");
NexDSButton bt3 = NexDSButton(0, 4, "bt3");
NexDSButton bt4 = NexDSButton(0, 5, "bt4");
NexDSButton bt5 = NexDSButton(0, 6, "bt5");
NexDSButton bt6 = NexDSButton(0, 7, "bt6");
NexDSButton bt7 = NexDSButton(0, 8, "bt7");
NexDSButton bt8 = NexDSButton(0, 9, "bt8");
NexDSButton bt9 = NexDSButton(0, 10, "bt9");
NexText t0 = NexText(0, 11, "t0");
NexText t1 = NexText(0, 12, "t1");
NexText t2 = NexText(0, 13, "t2");
NexText t3 = NexText(0, 14, "t3");
NexText t4 = NexText(0, 15, "t4");
NexText t5 = NexText(0, 16, "t5");
NexText t6 = NexText(0, 17, "t6");
NexText t7 = NexText(0, 18, "t7");
NexText t8 = NexText(0, 19, "t8");
NexText t9 = NexText(0, 20, "t9");
NexText t10 = NexText(0, 21, "t10");
NexText t11 = NexText(0, 22, "t11");
//RTOS>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TaskHandle_t Task1;
TaskHandle_t Task2;
//Nama WiFi Device>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
char node_ID[] = "PR000105-01";

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial1.begin(115200);
  pinMode(led, OUTPUT);
  nexInit();
  SD_Init();
  wifiManager.setConfigPortalTimeout(300);
  if (wifiManager.autoConnect(node_ID)) {
    String wifi = WiFi.SSID();
    Serial.println(WiFi.localIP());
  }
  if (WiFi.status() != WL_CONNECTED) {
  } else {
    geturl();
    //sendfile();
    //txtsend();
    //deleteFile(SD, "/datalog.txt");

    digitalWrite(led, HIGH);
    delay(200);
    digitalWrite(led, LOW);
    delay(200);
    digitalWrite(led, HIGH);
    delay(200);
    digitalWrite(led, LOW);
    delay(200);
  }
  xTaskCreatePinnedToCore(
    Task1code, /* Task function. */
    "Task1",   /* name of task. */
    10000,     /* Stack size of task */
    NULL,      /* parameter of the task */
    1,         /* priority of the task */
    &Task1,    /* Task handle to keep track of created task */
    0);        /* pin task to core 0 */
  delay(500);
  //create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
    Task2code, /* Task function. */
    "Task2",   /* name of task. */
    10000,     /* Stack size of task */
    NULL,      /* parameter of the task */
    1,         /* priority of the task */
    &Task2,    /* Task handle to keep track of created task */
    1);        /* pin task to core 1 */
  delay(500);
  digitalWrite(buz, HIGH);
  delay(50);
  digitalWrite(buz, LOW);
  delay(50);
  digitalWrite(buz, HIGH);
  delay(50);
  digitalWrite(buz, LOW);
}

void Task1code(void* pvParameters) {
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());

  for (;;) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis1 >= 3000) {
      previousMillis1 = currentMillis;
      Serial.println("task1");
      if (WiFi.status() != WL_CONNECTED) {
        WiFi.disconnect();
        WiFi.reconnect();
        Save_Data();
        Serial.println(mati);
      } else {
        x = 0;
        if (w == 0) {
          //sendfile();
          //txtsend();
          //deleteFile(SD, "/datalog.txt");
          x = 1;
        }
        long rssi = WiFi.RSSI();
        qualwifi = 2 * (rssi + 100);
        sendTofirebase(PHboard1, statusvalveboard1, statusvalveboard1a, flowboard1, DOboard2, suhuboard2, statusdosingboard3, PHboard3, flowboard3, statusdosingboard4, PHboard4, statusdosingboard5, statusdosingboard5a, levelboard6, levelboard7, levelboard8, levelboard9);
        waktusekarang();
      }
    }

    if (currentMillis - previousMillis2 >= 1000) {
      previousMillis2 = currentMillis;
      //BUTTON NEXTION
      Serial.println("task2");
      bt0.setValue(statusvalveboard1a);
      Serial.write(0xff);
      Serial.write(0xff);
      Serial.write(0xff);
      bt1.setValue(statusvalveboard1);
      Serial.write(0xff);
      Serial.write(0xff);
      Serial.write(0xff);
      bt2.setValue(statusdosingboard5);
      Serial.write(0xff);
      Serial.write(0xff);
      Serial.write(0xff);
      bt3.setValue(statusdosingboard4);
      Serial.write(0xff);
      Serial.write(0xff);
      Serial.write(0xff);
      bt4.setValue(statusdosingboard3);
      Serial.write(0xff);
      Serial.write(0xff);
      Serial.write(0xff);
      bt5.setValue(statusdosingboard5);
      Serial.write(0xff);
      Serial.write(0xff);
      Serial.write(0xff);
      bt6.setValue(levelboard6);
      Serial.write(0xff);
      Serial.write(0xff);
      Serial.write(0xff);
      bt7.setValue(levelboard7);
      Serial.write(0xff);
      Serial.write(0xff);
      Serial.write(0xff);
      bt8.setValue(levelboard8);
      Serial.write(0xff);
      Serial.write(0xff);
      Serial.write(0xff);
      bt9.setValue(levelboard9);
      Serial.write(0xff);
      Serial.write(0xff);
      Serial.write(0xff);
      //STRING NEXTION
      String stringPHBoard1 = String(PHboard1, 1);
      t0.setText(stringPHBoard1.c_str());
      Serial.write(0xff);
      Serial.write(0xff);
      Serial.write(0xff);
      String stringDOBoard2 = String(DOboard2, 1);
      t1.setText(stringDOBoard2.c_str());
      Serial.write(0xff);
      Serial.write(0xff);
      Serial.write(0xff);
      String stringsuhuBoard2 = String(suhuboard2, 1);
      t2.setText(stringsuhuBoard2.c_str());
      Serial.write(0xff);
      Serial.write(0xff);
      Serial.write(0xff);
      String stringflowBoard1 = String(flowboard1, 4);
      t3.setText(stringflowBoard1.c_str());
      Serial.write(0xff);
      Serial.write(0xff);
      Serial.write(0xff);
      String stringPHBoard4 = String(PHboard4, 1);
      t4.setText(stringPHBoard4.c_str());
      Serial.write(0xff);
      Serial.write(0xff);
      Serial.write(0xff);
      String stringPHBoard3 = String(PHboard3, 1);
      t5.setText(stringPHBoard3.c_str());
      Serial.write(0xff);
      Serial.write(0xff);
      Serial.write(0xff);
      String stringflowBoard3 = String(flowboard3, 4);
      t6.setText(stringflowBoard3.c_str());
      Serial.write(0xff);
      Serial.write(0xff);
      Serial.write(0xff);
      t7.setText(level6.c_str());
      Serial.write(0xff);
      Serial.write(0xff);
      Serial.write(0xff);
      t8.setText(level7.c_str());
      Serial.write(0xff);
      Serial.write(0xff);
      Serial.write(0xff);
      t9.setText(level8.c_str());
      Serial.write(0xff);
      Serial.write(0xff);
      Serial.write(0xff);
      t10.setText(level9.c_str());
      Serial.write(0xff);
      Serial.write(0xff);
      Serial.write(0xff);
      IPAddress myIP = WiFi.localIP();
      char buf[20];
      sprintf(buf, "%d.%d.%d.%d", myIP[0], myIP[1], myIP[2], myIP[3]);
      t11.setText(buf);
      Serial.write(0xff);
      Serial.write(0xff);
      Serial.write(0xff);
    }
    vTaskDelay(10);
  }
}

void Task2code(void* pvParameters) {
  Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());

  for (;;) {
    doWiFiManager();
    dapatSerial();
    vTaskDelay(10);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
}

void dapatSerial() {
  if (Serial1.available()) {
    recv_str_jsondata = Serial1.readStringUntil('\n');
    Serial.println(recv_str_jsondata);
    DeserializationError error = deserializeJson(doc_recv, recv_str_jsondata);

    if (!error) {  // if not error in deserialization
      PHboard1 = doc_recv["v1"];
      statusvalveboard1 = doc_recv["v2"];
      statusvalveboard1a = doc_recv["v3"];

      flowboard1 = doc_recv["v4"];

      float DOboard2raw = doc_recv["v5"];
      DOboard2 = DOboard2raw * offsite3;

      float suhuboard2raw = doc_recv["v6"];
      suhuboard2 = suhuboard2raw * offsite4;

      statusdosingboard3 = doc_recv["v7"];

      PHboard3 = doc_recv["v8"];

      flowboard3 = doc_recv["v9"];


      statusdosingboard4 = doc_recv["v10"];

      PHboard4 = doc_recv["v11"];

      statusdosingboard5 = doc_recv["v12"];
      statusdosingboard5a = doc_recv["v13"];
      levelboard6 = doc_recv["v14"];
      levelboard7 = doc_recv["v15"];
      levelboard8 = doc_recv["v16"];
      levelboard9 = doc_recv["v17"];

      if (levelboard6 == 1) {
        level6 = "F";
      } else {
        level6 = "E";
      }

      if (levelboard7 == 1) {
        level7 = "F";
      } else {
        level7 = "E";
      }

      if (levelboard8 == 1) {
        level8 = "F";
      } else {
        level8 = "E";
      }

      if (levelboard9 == 1) {
        level9 = "F";
      } else {
        level9 = "E";
      }

      Serial.printf("Ph1 value: %4.2f \n", PHboard1);
      Serial.printf("Valve1 value: %d \n", statusvalveboard1);
      Serial.printf("Valve1a value: %d \n", statusvalveboard1a);
      Serial.printf("Flow1 value: %4.2f \n", flowboard1);

      Serial.printf("do2 value: %4.2f \n", DOboard2);
      Serial.printf("suhu2 value: %4.2f \n", suhuboard2);

      Serial.printf("Dosing3 value: %d \n", statusdosingboard3);
      Serial.printf("Ph3 value: %4.2f \n", PHboard3);
      Serial.printf("Flow3 value: %4.2f \n", flowboard3);

      Serial.printf("Dosing4 value: %d \n", statusdosingboard4);
      Serial.printf("Ph4 value: %4.2f \n", PHboard4);

      Serial.printf("Dosing5 value: %d \n", statusdosingboard5);
      Serial.printf("Dosing5a value: %d \n", statusdosingboard5a);

      Serial.printf("Level6 value: %d \n", levelboard6);
      Serial.printf("Level7 value: %d \n", levelboard7);
      Serial.printf("Level8 value: %d \n", levelboard8);
      Serial.printf("Level9 value: %d \n", levelboard9);

      Serial.println();
      delay(1000);
      kirimSerial();
    }

    else {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }
    recv_str_jsondata = "";
  }
}

void sendTofirebase(float PHboard1, int statusvalveboard1, int statusvalveboard1a, float flowboard1, float DOboard2, float suhuboard2, int statusdosingboard3, float PHboard3, float flowboard3, int statusdosingboard4, float PHboard4, int statusdosingboard5, int statusdosingboard5a, int levelboard6, int levelboard7, int levelboard8, int levelboard9) {
  WiFiClient client;
  HTTPClient http;
  String load = "{}";
  http.begin(client, linkdiawan);
  Serial.println(linkdiawan);
  String jsonStr = "";
  http.addHeader("Content-Type", "application/json");
  //uncomment this line if creating epal semi auto
  //String isi = " {\"email\":\"" + email + "\",\"password\":\"" + pass + "\", \"userId\":\"" + userId + "\",\"idDevice\":\"" + idDevice + "\",\"value\":{\"data1\":" + PHboard1 + ",\"data2\":" + DOboard2 + ",\"data3\":" + suhuboard2 + ",\"data4\":" + PHboard3 + ",\"data5\":" + statusdosingboard3 + ",\"data6\":" + PHboard4 + ",\"data7\":" + statusdosingboard4 + ",\"wifi\":" + qualwifi + "}}";
  //uncomment this line if creating epal full auto
  //String isi = "{\"email\":\"" + email + "\",\"password\":\"" + pass + "\", \"userId\":\"" + userId + "\",\"idDevice\":\"" + idDevice + "\",\"value\":{\"data1\":" + PHboard1 + ",\"data2\":" + statusvalveboard1 + ",\"data3\":" + statusvalveboard1a + ",\"data4\":" + flowboard1 + ",\"data5\":" + DOboard2 + ",\"data6\":" + suhuboard2 + ",\"data7\":" + statusdosingboard3 + ",\"data8\":" + PHboard3 + ",\"data9\":" + flowboard3 + ",\"data10\":" + statusdosingboard4 + ",\"data11\":" + PHboard4 + ",\"data12\":" + statusdosingboard5 + ",\"data13\":" + statusdosingboard5a + ",\"data14\":" + levelboard6 + ",\"data15\":" + levelboard7 + ",\"data16\":" + levelboard8 + ",\"data17\":" + levelboard9 + ",\"wifi\":" + qualwifi + "}}";
  //uncomment untuk kendal rawat jalan
  //String isi = "{\"email\":\"" + email + "\",\"password\":\"" + pass + "\", \"userId\":\"" + userId + "\",\"idDevice\":\"" + idDevice + "\",\"value\":{\"data1\":" + PHboard1 + ",\"data2\":" + DOboard2 + ",\"data3\":" + suhuboard2 + ",\"data4\":" + statusdosingboard5a + ",\"data5\":" + statusdosingboard5a + ",\"wifi\":" + qualwifi + "}}";
  //uncomment untuk kendal rawat inap
  String isi = "{\"email\":\"" + email + "\",\"password\":\"" + pass + "\", \"userId\":\"" + userId + "\",\"idDevice\":\"" + idDevice + "\",\"value\":{\"data1\":" + PHboard1 + ",\"data2\":" + statusvalveboard1 + ",\"data3\":" + statusvalveboard1a + ",\"data4\":" + flowboard1 + ",\"data5\":" + DOboard2 + ",\"data6\":" + suhuboard2 + ",\"data7\":" + flowboard3 + ",\"data8\":" + statusdosingboard5 + ",\"data9\":" + statusdosingboard5a + ",\"data10\":" + levelboard6 + ",\"data11\":" + levelboard9 + ",\"wifi\":" + qualwifi + "}}";

  Serial.println(isi);
  httpResponseCode = http.POST(isi);
  Serial.print("HTTP Response code:");
  Serial.println(httpResponseCode);

  if (httpResponseCode != 200) {
    delay(500);
  } else if (httpResponseCode == 500) {
  }

  else if (httpResponseCode == -1 || httpResponseCode == -11) {
  }

  else {
    digitalWrite(led, HIGH);
    delay(100);
    digitalWrite(led, LOW);
    delay(100);
    load = http.getString();
    Serial.print(load);
    DynamicJsonDocument doc(1024);
    String input = load;
    deserializeJson(doc, input);
    JsonObject obj = doc.as<JsonObject>();
    offsite1 = obj["result"]["offsite"]["offsite_value_data1"].as<float>();
    offsite2 = obj["result"]["offsite"]["offsite_value_data4"].as<float>();
    offsite3 = obj["result"]["offsite"]["offsite_value_data2"].as<float>();
    offsite4 = obj["result"]["offsite"]["offsite_value_data3"].as<float>();
    offsite5 = obj["result"]["offsite"]["offsite_value_data11"].as<float>();
    offsite6 = obj["result"]["offsite"]["offsite_value_data9"].as<float>();
    offsite7 = obj["result"]["offsite"]["offsite_value_data8"].as<float>();
    Serial.println(offsite1);
    Serial.println(offsite2);
    Serial.println(offsite3);
    Serial.println(offsite4);
    Serial.println(offsite4);
    Serial.println(offsite5);
    Serial.println(offsite7);
    last = obj["result"]["newresult"]["timestamp"].as<String>();
    name = obj["result"]["name"].as<String>();
    if (obj["result"]["restart"].as<int>() == 1) {
      ESP.restart();
    }
    if (obj["result"]["reset"].as<int>() == 1) {
      wifiManager.resetSettings();
      ESP.restart();
    }
    http.end();
  }
}

void geturl() {
  WiFiClient client;
  HTTPClient http;
  String serverPath = serverName + "/" + idDevice;
  http.begin(client, serverPath.c_str());
  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    String payload = http.getString();
    Serial.println(payload);
    DynamicJsonDocument doc(1024);
    String input = payload;
    deserializeJson(doc, input);
    JsonObject obj = doc.as<JsonObject>();
    linkdiawan = obj["url"]["push"].as<String>();
    offsite1 = obj["offsite"]["offsite_data1"].as<float>();  //ph effluent
    offsite2 = obj["offsite"]["offsite_data4"].as<float>();  //flow out
    offsite3 = obj["offsite"]["offsite_data2"].as<float>();
    offsite4 = obj["offsite"]["offsite_data3"].as<float>();
    offsite5 = obj["offsite"]["offsite_data8"].as<float>();   //ph kaustik
    offsite6 = obj["offsite"]["offsite_data9"].as<float>();   //flow in
    offsite7 = obj["offsite"]["offsite_data11"].as<float>();  //ph koagulan
    //unceomment for epal semi auto
    //    offsite1 = obj["offsite"]["offsite_data1"].as<float>();  //ph board 1
    //    offsite2 = obj["offsite"]["offsite_data7"].as<float>();  //flow board 1
    //    offsite3 = obj["offsite"]["offsite_data2"].as<float>();
    //    offsite4 = obj["offsite"]["offsite_data3"].as<float>();
    //    offsite5 = obj["offsite"]["offsite_data8"].as<float>();   //ph board 3
    //    offsite6 = obj["offsite"]["offsite_data9"].as<float>();   //flow board 3
    //    offsite7 = obj["offsite"]["offsite_data11"].as<float>();   //ph board 4

    a = obj["regression"]["data1"]["a"].as<float>();
    b = obj["regression"]["data1"]["b"].as<float>();
    c = obj["regression"]["data1"]["c"].as<float>();
    d = obj["regression"]["data4"]["a"].as<float>();
    e = obj["regression"]["data4"]["b"].as<float>();
    f = obj["regression"]["data4"]["c"].as<float>();
    g = obj["regression"]["data5"]["a"].as<float>();
    h = obj["regression"]["data5"]["b"].as<float>();
    i = obj["regression"]["data5"]["c"].as<float>();
    j = obj["regression"]["data6"]["a"].as<float>();
    k = obj["regression"]["data6"]["b"].as<float>();
    l = obj["regression"]["data6"]["c"].as<float>();
    o = obj["regression"]["data8"]["a"].as<float>();
    p = obj["regression"]["data8"]["b"].as<float>();
    q = obj["regression"]["data8"]["c"].as<float>();
    r = obj["regression"]["data9"]["a"].as<float>();
    s = obj["regression"]["data9"]["b"].as<float>();
    t = obj["regression"]["data9"]["c"].as<float>();
    u = obj["regression"]["data11"]["a"].as<float>();
    v = obj["regression"]["data11"]["b"].as<float>();
    w = obj["regression"]["data11"]["c"].as<float>();
    name = obj["name"].as<String>();
    Serial.print("urlGet ");
    Serial.println(linkdiawan);
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}
void SD_Init() {
  //  SPIClass SPI2(HSPI);
  SPISD.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  if (!SD.begin(SD_CS, SPISD)) {  //SD_CS_PIN this pin is just the dummy pin since the SD need the input
    Serial.println(F("failed!"));
    digitalWrite(led, HIGH);
    delay(1000);
    digitalWrite(led, LOW);
    delay(1000);
    digitalWrite(led, HIGH);
    delay(1000);
    digitalWrite(led, LOW);
    delay(1000);
    return;
  } else {
    Serial.println(F("SD read!"));
  }
  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");

    Serial.println(" ");
    return;
  }
  File file = SD.open("/datalog.txt");
  if (!file) {
    Serial.println("File doens't exist");
    Serial.println("Creating file...");
    writeFile(SD, "/datalog.txt", "");
  } else {
    Serial.println("File already exists");
    Serial.println(" ");
  }
  file.close();
}
void Save_Data() {
  dataMessage = String(PHboard1) + "," + String(statusvalveboard1) + "," + String(statusvalveboard1a) + "," + String(flowboard1) + "," + String(DOboard2) + "," + String(suhuboard2) + "," + String(statusdosingboard3) + "," + String(PHboard3) + "," + String(flowboard3) + "," + String(statusdosingboard4) + "," + String(PHboard4) + "," + String(statusdosingboard5) + "," + String(statusdosingboard5a) + "," + String(levelboard6) + "," + String(levelboard7) + "," + String(levelboard8) + "," + String(levelboard9) + "#";
  Serial.print("Save data: ");
  Serial.println(dataMessage);
  appendFile(SD, "/datalog.txt", dataMessage.c_str());
}
// Write to the SD card (DON'T MODIFY THIS FUNCTION)
void writeFile(fs::FS& fs, const char* path, const char* message) {
  Serial.printf("Writing file: %s\n", path);
  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    Serial.println(" ");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
    Serial.println(" ");
  } else {
    Serial.println("Write failed");
    Serial.println(" ");
  }
  file.close();
}

void deleteFile(fs::FS& fs, const char* path) {
  Serial.printf("Deleting file: %s\n", path);
  if (fs.remove(path)) {
    Serial.println("File deleted");
  } else {
    Serial.println("Delete failed");
  }
}

// Append data to the SD card (DON'T MODIFY THIS FUNCTION)
void appendFile(fs::FS& fs, const char* path, const char* dataMessage) {
  Serial.printf("Appending to file: %s\n", path);
  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    Serial.println(" ");
    return;
  }
  if (file.print(dataMessage)) {
    Serial.println("Message appended");
    Serial.println(" ");
  } else {
    Serial.println("Append failed");
    Serial.println(" ");
  }
  file.close();
}
void waktusekarang() {
  formattedDate = last;
  Serial.println(formattedDate);
  // Extract date
  int splitT = formattedDate.indexOf("T");
  jam = String(formattedDate.substring(splitT + 1, formattedDate.length() - 9));
  dayStamp = String(formattedDate.substring(0, splitT));
  waktu = dayStamp + "   " + jam;
  Serial.print("DATE: ");
  Serial.println(waktu);
}
String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;
  // Your Domain name with URL path or IP address with path
  http.begin(client, serverName);
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  String payload = "{}";
  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();
  return payload;
}

void sendfile() {
  postFileContent("/datalog.txt");  // Read file data_log.txt in Root directory
  delay(1);
  client.stop();
}

void postFileContent(const char* path) {
  Serial.print(F(" -- Reading entries from file = "));
  Serial.println(path);
  if (!SD.exists(path)) {
    Serial.println("ERROR: The required file does not exist.");
    return;
  }
  File dataFile = SD.open(path, FILE_READ);  // FILE_READ is default so not realy needed but if you like to use this technique for e.g. write you need FILE_WRITE
  if (!dataFile) {
    Serial.println("Failed to open file for reading");
    return;
  }
  char c;
  uint8_t i = 0;


  WiFiClient client;

  if (client.connect("diawan.io", 80)) {
    last.replace("+", "#");
    String data = "&userId=" + String(userId) + "&idDevice=" + String(idDevice) + "&lasttimestamp=" + String(last) + "&data=";
    Serial.println("Posting sensor data...");
    client.print("POST ");
    client.print("/api/upload-json");
    client.println(" HTTP/1.1");
    client.println("Host: diawan.io");
    client.println("User-Agent: Arduino/1.0");
    client.println("Connection: close");
    client.println("Content-Type: application/x-www-form-urlencoded; charset=UTF-8");
    int length = data.length();
    client.print("Content-Length: ");
    client.println((dataFile.size() - 2) + length);
    Serial.print("length: ");
    Serial.println(String(dataFile.size() + length));
    client.println();
    client.print(data);
    while (dataFile.available()) {
      if (dataFile.position() < dataFile.size()) {
        if (dataFile.position() < dataFile.size() - 2) {
          c = dataFile.read();
          client.print(c);
        } else {
          c = dataFile.read();
          if (c != ' ') {
            Serial.println(String(c));
            if (c != '#') {
              client.print(c);
            }
          }
        }
      }
      if (dataFile.position() == dataFile.size() - 1) {
        client.println();
      }
    }
    Serial.print(data);
    Serial.println();
    String req = client.readStringUntil('\r');
    Serial.println(req);
    client.flush();
    Serial.println("Client disonnected");
    dataFile.close();
  } else {
    Serial.println("Connection failed");
  }
  Serial.println(F("DONE Reading"));
}

void doWiFiManager() {
  if (portalRunning) {
    wifiManager.process();  // do processing
  }
  // is configuration portal requested?
  if (!portalRunning) {
    if (startAP) {
      Serial.println("Button Pressed, Starting Config Portal");
      wifiManager.setConfigPortalBlocking(false);
      wifiManager.startConfigPortal(node_ID);
    } else {
      Serial.println("Button Pressed, Starting Web Portal");
      wifiManager.startWebPortal();
    }
    portalRunning = true;
    startTime = millis();
  }
}

void kirimSerial() {
  StaticJsonDocument<500> jsonDocEspnow;  // JSON Doc for Receiving data from ESPNOW Devices
  jsonDocEspnow["v1"] = offsite1;
  jsonDocEspnow["v2"] = offsite2;
  jsonDocEspnow["v3"] = offsite3;
  jsonDocEspnow["v4"] = offsite4;
  jsonDocEspnow["v5"] = offsite5;
  jsonDocEspnow["v6"] = offsite6;
  jsonDocEspnow["v7"] = offsite7;

  char buffer[256];
  serializeJson(jsonDocEspnow, buffer);  // Writing Data to Serial2
  Serial1.write(buffer);
  Serial.println("Serilising to Serial1 ");
  Serial.println(buffer);
}
