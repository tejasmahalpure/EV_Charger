#include <ZMPT101B.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <BluetoothSerial.h>
#include <EEPROM.h>
#include <PubSubClient.h>
#include <Preferences.h>
#include <String.h>
#include <Update.h>

// URL to the firmware binary file
const char* firmwareURL = "http://ev.falconides.com/OTA.bin";
// Current firmware version
const char* currentVersion = "1.0.0";

// Constants for Voltage and Current Sensor
#define SENSITIVITY 500.0f
ZMPT101B voltageSensor(35, 50.0);
const int ACS_ADC = 34;

//Constants for calculating Power
const float sensitivity = 0.011, offset = 2.0;

float instantPower, consumedPower = 0, sum = 0.0;
float voltage, Irms;
long t = 0;

char unit[5];
float unitCon = 0;

hw_timer_t* timer = NULL;

char unitConsumed[5], Voltage[7], Current[7];  //Json variables

Preferences pref;

#define IDLE 0
#define CHARGING 1
#define FULL 2
#define OPEN 3
#define CLOSE 4

int Flag = 0, tokenFlag = 4;

char Token[11] = "0000";
String rxDevID;
char DevID[5];
char* Status;
String macAddress;

BluetoothSerial serialBT;

// MCU pins
const int relay = 4;
int BTSwitch = 27;

//Serial BT variable
char cmd = '0';
String receivedString;
int i;

// wifi variables
//Falcon Systems
//Ykk17371
char newSSID[32];
char newPass[32];
char SSIDbuffer[25], PassBuffer[25];


// site variables
const char* host = "http://ev.falconides.com/api/Get_Device_ID";

// mqtt connection variables
const char* mqtt_broker = "108.181.184.135";  //mqtt broker url
char topic[4] = "F";                          //topic name
const char* mqtt_username = "mqtt";           //mqtt user name
const char* mqtt_password = "falcon@123";     //mqtt password
const int mqtt_port = 1883;                   //mqtt comm port

WiFiClient espClient;            //wifi instance
PubSubClient client(espClient);  //mqtt instance

const int networkStatusInd = 16;  // blue led
const int chargingProcessInd = 18; // green led

const int R = 19, G = 21, B = 22;

char Data[160];
//char check[10] = "INet?";
//int INetcnt = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  //Configure IO pins
  pinMode(BTSwitch, INPUT);
  pinMode(networkStatusInd, OUTPUT);
  pinMode(chargingProcessInd, OUTPUT);
  pinMode(R, OUTPUT);
  pinMode(G, OUTPUT);
  pinMode(B, OUTPUT);
  pinMode(relay, OUTPUT);

  timer = timerBegin(0, 80, true);

  voltageSensor.setSensitivity(SENSITIVITY);

  pref.begin("my-app", false);
  String StoredSSID = pref.getString("SSID", "defalutValue");
  String StoredPass = pref.getString("PassWord", "defaultValue");

  StoredSSID.toCharArray(SSIDbuffer, sizeof(SSIDbuffer));
  StoredPass.toCharArray(PassBuffer, sizeof(PassBuffer));

  pref.end();

  Serial.println(SSIDbuffer);
  Serial.println(PassBuffer);
  // wifi connection
  WiFi.begin(SSIDbuffer, PassBuffer);

  int L = 0;
  do {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(500);
    }
    L++;
  } while (L < 43);

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wifi ByPassed");
    digitalWrite(networkStatusInd, LOW);
  }
  else {
    Serial.print("Connected to: ");
    Serial.println(SSIDbuffer);
    digitalWrite(networkStatusInd, HIGH);
  }


  getDeviceID();
  rxDevID.toCharArray(DevID, 5);
  strcat(topic, DevID);
  connectMQTTBroker();
}

void loop() {
  // put your main code here, to run repeatedly:

  if (digitalRead(BTSwitch) == 1) {
    serialBT.begin("Esp32_BT");
    Serial.println("Bluetooth ON");
    while (digitalRead(BTSwitch) == 1) {
      BTFunctions();
      //i = 3;
      digitalWrite(networkStatusInd, LOW);
      delay(1000);
      digitalWrite(networkStatusInd, HIGH);
      delay(1000);
    }
  }

  client.loop();
  delay(100);

  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(networkStatusInd, HIGH);
  } else if (WiFi.status() != WL_CONNECTED) {
    digitalWrite(networkStatusInd, LOW);
    int L = 0;
    do {
      if (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
      }
      L++;
    } while (L < 43);
  }

  calculatePower();
  unitCon = atof(unit);

  if (consumedPower > unitCon) {
    charging(0);
    calculatePower();

    char conunit[5];
    char v[5];
    char i[5];

    dtostrf(voltage, 6, 2, v);
    json_edit(Data, "Voltage", v);
    dtostrf(Irms, 6, 2, i);
    json_edit(Data, "Current", i);
    dtostrf(consumedPower, 6, 2, conunit);
    json_edit(Data, "unitConsumed", conunit);

    digitalWrite(chargingProcessInd, LOW);

    client.publish(topic, Data);
    timerRestart(timer);
    timerStop(timer);
  }

  digitalWrite(G, HIGH);
}

// OTA use flag 
// MQTT closes 
// null for GetStat
// auto-connect 
// token ID error
// save data
// current drop 

//idle cond  //charging -> app stop, interuppted, power off, internet off   //charging complete
