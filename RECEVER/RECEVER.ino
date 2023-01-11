#include <esp_now.h>
#include <WiFi.h>
#include <ArduinoJson.h>
//#include <HardwareSerial.h>
// Structure example to receive data
// Must match the sender structure
#define ledindikator 4
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


epal board1;
epal board2;
epal board3;
epal board4;
epal board5;
epal board6;
epal board7;
epal board8;
epal board9;

epal boardsStruct[11] = {board1, board2, board3, board4, board5, board6, board7, board8, board9};

String recv_jsondata;
String recv_str_jsondata;
uint8_t broadcastAddress[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
StaticJsonDocument<500> doc_send;
StaticJsonDocument<500> doc_recv;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {
  //Initialize Serial Monitor
  Serial.begin(115200);
  Serial1.begin(115200);
  pinMode(ledindikator, OUTPUT);
  //Set device as a Wi-Fi Station
  WiFi.mode(WIFI_AP_STA);
  Serial.print("Mac Address: ");
  Serial.print(WiFi.macAddress());
  Serial.println("ESP-Now Receiver");
  //Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_recv_cb([](const uint8_t *mac_addr, const uint8_t *data_epal, int len) {
    char macStr[18];
    Serial.print("Packet received from: ");
    digitalWrite(ledindikator, HIGH);
    delay(200);
    digitalWrite(ledindikator, LOW);
    snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
             mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    Serial.println(macStr);
    memcpy(&myepal, data_epal, sizeof(myepal));
    Serial.printf("Board ID %u: %u bytes\n", myepal.id, len);
    // Update the structures with the new incoming data
    boardsStruct[myepal.id - 1].ph1 = myepal.ph1;
    boardsStruct[myepal.id - 1].valve1 = myepal.valve1;
    boardsStruct[myepal.id - 1].valve1a = myepal.valve1a;
    boardsStruct[myepal.id - 1].flow1 = myepal.flow1;
    boardsStruct[myepal.id - 1].do2 = myepal.do2;
    boardsStruct[myepal.id - 1].suhu2 = myepal.suhu2;
    boardsStruct[myepal.id - 1].dosing3 = myepal.dosing3;
    boardsStruct[myepal.id - 1].ph3 = myepal.ph3;
    boardsStruct[myepal.id - 1].flow3 = myepal.flow3;
    boardsStruct[myepal.id - 1].dosing4 = myepal.dosing4;
    boardsStruct[myepal.id - 1].ph4 = myepal.ph4;
    boardsStruct[myepal.id - 1].dosing5 = myepal.dosing5;
    boardsStruct[myepal.id - 1].dosing5a = myepal.dosing5a;
    boardsStruct[myepal.id - 1].level6 = myepal.level6;
    boardsStruct[myepal.id - 1].level7 = myepal.level7;
    boardsStruct[myepal.id - 1].level8 = myepal.level8;
    boardsStruct[myepal.id - 1].level9 = myepal.level9;
    Serial.printf("Ph1 value: %4.2f \n", boardsStruct[myepal.id - 1].ph1);
    Serial.printf("Valve1 value: %d \n", boardsStruct[myepal.id - 1].valve1);
    Serial.printf("Valve1a value: %d \n", boardsStruct[myepal.id - 1].valve1a);
    Serial.printf("Flow1 value: %4.2f \n", boardsStruct[myepal.id - 1].flow1);

    Serial.printf("do2 value: %4.2f \n", boardsStruct[myepal.id - 1].do2);
    Serial.printf("suhu2 value: %4.2f \n", boardsStruct[myepal.id - 1].suhu2);

    Serial.printf("Dosing3 value: %d \n", boardsStruct[myepal.id - 1].dosing3);
    Serial.printf("Ph3 value: %4.2f \n", boardsStruct[myepal.id - 1].ph3);
    Serial.printf("Flow3 value: %4.2f \n", boardsStruct[myepal.id - 1].flow3);

    Serial.printf("Dosing4 value: %d \n", boardsStruct[myepal.id - 1].dosing4);
    Serial.printf("Ph4 value: %4.2f \n", boardsStruct[myepal.id - 1].ph4);

    Serial.printf("Dosing5 value: %d \n", boardsStruct[myepal.id - 1].dosing5);
    Serial.printf("Dosing5a value: %d \n", boardsStruct[myepal.id - 1].dosing5a);

    Serial.printf("Level6 value: %d \n", boardsStruct[myepal.id - 1].level6);
    Serial.printf("Level7 value: %d \n", boardsStruct[myepal.id - 1].level7);
    Serial.printf("Level8 value: %d \n", boardsStruct[myepal.id - 1].level8);
    Serial.printf("Level9 value: %d \n", boardsStruct[myepal.id - 1].level9);

    Serial.println();
  });
  esp_now_register_send_cb(OnDataSent);
  // Adding Peer Devices
  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));
  for (int ii = 0; ii < 6; ++ii )
  {
    peerInfo.peer_addr[ii] = (uint8_t) broadcastAddress[ii];
  }
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
}

void loop() {
  float PHboard1 = boardsStruct[0].ph1;
  int statusvalveboard1 = boardsStruct[0].valve1;
  int statusvalveboard1a = boardsStruct[0].valve1a;
  float flowboard1 = boardsStruct[0].flow1;

  float DOboard2 = boardsStruct[0].do2;
  float suhuboard2 = boardsStruct[0].suhu2;

  int statusdosingboard3 = boardsStruct[2].dosing3;
  float PHboard3 = boardsStruct[2].ph3;
  float flowboard3 = boardsStruct[2].flow3;

  int statusdosingboard4 = boardsStruct[0].dosing4;
  float PHboard4 = boardsStruct[0].ph4;

  int statusdosingboard5 = boardsStruct[0].dosing5;
  int statusdosingboard5a = boardsStruct[0].dosing5a;

  int levelboard6 = boardsStruct[0].level6;
  int levelboard7 = boardsStruct[2].level7;
  int levelboard8 = boardsStruct[0].level8;
  int levelboard9 = boardsStruct[0].level9;

  StaticJsonDocument<500> jsonDocEspnow; // JSON Doc for Receiving data from ESPNOW Devices
  jsonDocEspnow["v1"] = PHboard1;
  jsonDocEspnow["v2"] = statusvalveboard1;
  jsonDocEspnow["v3"] = statusvalveboard1a;
  jsonDocEspnow["v4"] = flowboard1;
  jsonDocEspnow["v5"] = DOboard2;
  jsonDocEspnow["v6"] = suhuboard2;
  jsonDocEspnow["v7"] = statusdosingboard3;
  jsonDocEspnow["v8"] = PHboard3;
  jsonDocEspnow["v9"] = flowboard3;
  jsonDocEspnow["v10"] = statusdosingboard4;
  jsonDocEspnow["v11"] = PHboard4;
  jsonDocEspnow["v12"] = statusdosingboard5;
  jsonDocEspnow["v13"] = statusdosingboard5a;
  jsonDocEspnow["v14"] = levelboard6;
  jsonDocEspnow["v15"] = levelboard7;
  jsonDocEspnow["v16"] = levelboard8;
  jsonDocEspnow["v17"] = levelboard9;
  char buffer[256];
  serializeJson(jsonDocEspnow, buffer);            // Writing Data to Serial2
  Serial1.write(buffer);
  Serial.println("Serilising to Serial1 ");
  Serial.println(buffer);
  delay(1000);
  if (Serial1.available())
  {
    // Recieving data (JSON) from BLYNK ESP
    recv_str_jsondata = Serial1.readStringUntil('\n');
    //Serializing JSON
    Serial.println(recv_str_jsondata);
    DeserializationError error = deserializeJson(doc_recv, recv_str_jsondata);
    if (!error) {                                           // if not error in deserialization
      myfeedbackepal.offsite1 = doc_recv["v1"];
      myfeedbackepal.offsite2 = doc_recv["v2"];
      myfeedbackepal.offsite3 = doc_recv["v3"];
      myfeedbackepal.offsite4 = doc_recv["v4"];
      myfeedbackepal.offsite5 = doc_recv["v5"];
      myfeedbackepal.offsite6 = doc_recv["v6"];
      myfeedbackepal.offsite7 = doc_recv["v7"];
      // Broadcasting data (JSON) via ESP-NOW
      esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myfeedbackepal, sizeof(myfeedbackepal));

      if (result == ESP_OK)
      {
        Serial.println("Sent with success");
      }
      else
      {
        Serial.println(result);
      }
      delay(20);
    }
    else {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }
    recv_str_jsondata = "";
  }
}
