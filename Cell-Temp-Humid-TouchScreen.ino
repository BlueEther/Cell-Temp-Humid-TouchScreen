#include <Arduino.h>
#include <TimeLib.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <OneWire.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <Array.h>
#include <dht.h>
#include <Wire.h>
#include <DallasTemperature.h>

#include <SoftwareSerial.h>
#include <Nextion.h>
//#define nextion Serial
SoftwareSerial nextion(D2, D1);// Nextion TX / RX  blue / yellow

Nextion myNextion(nextion, 9600); //create a Nextion object named myNextion using the nextion serial port @ 9600bps


#define min(X, Y) (((X)<(Y))?(X):(Y))
#define max(X, Y) (((X)>(Y))?(X):(Y))

#define HOME 0        //log to blueether if true else kbwaikato
#define USEWIFI 1     //establish wifi or not
#define HOMEWIFI 0    //blueether tech wifi if true else kbcorp
#define DEBUG 1
#define TEST 0        //test postfix in feeds
#define USELED 1      //light leds on read temp ot post to mqtt
#define SENSEARCH 0   // SEARCH FOR SENSORS
#define INPUT_SIZE 18 // size of serial string from nextion
#define LOCATION 1 // Depot = 0 // QYard = 1

#include "wifi.conf"
// /************************* WiFi Access Point *********************************/
// #if (HOMEWIFI)
  // #define WLAN_SSID       "AP_1 Name"
  // #define WLAN_PASS       "iwillnotgiveyoumykey"
// #else
  // #define WLAN_SSID       "AP_2 Name"
  // #define WLAN_PASS       "iwillnotgiveyoumykeyyoubastard"
// #endif
// /************************* Adafruit.io Setup *********************************/

// #define AIO_SERVER      "io.adafruit.com"
// #define AIO_SERVERPORT  1883
// #if (HOME)
  // #define AIO_USERNAME    "USER1"
  // #define AIO_KEY         "1234567890"
// #else
  // #define AIO_USERNAME    "USER2"
  // #define AIO_KEY         "0987654321"  
// #endif

/************ Global State (you don't need to change this!) ******************/
// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
//UDP for ntps
WiFiUDP Udp;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/
// Setup a feed called 'photocell' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>

#if (TEST)
  Adafruit_MQTT_Publish tempOW0    = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/tempOW0Test");
  Adafruit_MQTT_Publish tempOW1    = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/tempOW1Test");
  Adafruit_MQTT_Publish tempHAvg0  = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/tempHAvg0Test");
  Adafruit_MQTT_Publish tempHAvg1  = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/tempHAvg1Test");
  Adafruit_MQTT_Publish warningS  = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/warningStringTest");
  Adafruit_MQTT_Publish warning   = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/warningTest");
  Adafruit_MQTT_Publish battery   = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/batteryTest");
  Adafruit_MQTT_Publish humidity  = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humidityTest");
#else
  #if (LOCATION)
    Adafruit_MQTT_Publish tempOW0    = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/QY_Temp_Top");
    Adafruit_MQTT_Publish tempOW1    = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/QY_Temp_Bottom");
    Adafruit_MQTT_Publish warningS  = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/QY_WarningString");
    Adafruit_MQTT_Publish warning   = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/QY_Warning");
    Adafruit_MQTT_Publish humidity  = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/QY_Humidity");  
  #else
    Adafruit_MQTT_Publish tempOW0    = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Depot_Temp_Top");
    Adafruit_MQTT_Publish tempOW1    = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Depot_Temp_Bottom");
    Adafruit_MQTT_Publish warningS  = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Depot_WarningString");
    Adafruit_MQTT_Publish warning   = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Depot_Warning");
    Adafruit_MQTT_Publish humidity  = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Depot_Humidity");
  #endif
#endif

/*************************** Sketch Code ************************************/
// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();

// NTP Servers:
void sendNTPpacket(IPAddress &address);

static const char ntpServerName[] = "nz.pool.ntp.org";
const int timeZone = 12;     // Central European Time
unsigned int localPort = 8888;  // local port to listen for UDP packets
time_t getNtpTime();
// Pin sensors is connected to on sheild
const int LEDPin = D4; 
//int LEDPin = LED_BUILTIN;
const int RELAYPin = D5; //on d5 in inc 1
const int OWPin = D6; //on D6 in incubator 1
const int DHTPin = D4; //on d3 in inc 1
OneWire  ds(OWPin);  // on pin Digital 2 on sheild
DallasTemperature sensors(&ds);

 
#define OverTemp 35
#define UnderTemp 32
#define CritTemp 36
dht DHT; //create DHT object

//  ADC_MODE(ADC_VCC);


bool ParPower = false;
#if (LOCATION)
  DeviceAddress addr0 = {0x28, 0xFF, 0xAF, 0x26, 0x64, 0x16, 0x4, 0x71};
  DeviceAddress addr1 = {0x28, 0xFF, 0xD6, 0xC9, 0x43, 0x16, 0x4, 0xD2};
#else
  DeviceAddress addr0 = {0x28, 0xFF, 0x9E, 0xC7, 0x43, 0x16, 0x4, 0x10};
  DeviceAddress addr1 = {0x28, 0xFF, 0x70, 0xC9, 0x85, 0x16, 0x4, 0xFF};
#endif


static const char arrySize = 60;
static float hourAvg[2][arrySize];
//static float hourMin[arrySize]; 
//static float hourMax[arrySize];

static float minMinMax[2][6];

//Array<float> hourAvgArray = Array<float>(hourAvg,arrySize);
//Array<float> hourMinArray = Array<float>(hourMin,arrySize);
//Array<float> hourMaxArray = Array<float>(hourMax,arrySize);
Array<float> minMinMaxArray0 = Array<float>(minMinMax[0],6);
Array<float> minMinMaxArray1 = Array<float>(minMinMax[1],6);

bool   sendH = false;
bool   restart = false;
float  minAvg = 0;
int    minC = 0; // count of no. of temps in min Average
int    hourC = 0;
float  tmpOW0 = 0;
float  tmpOW1 = 0;
float  humid = 0;
char   nekMin = 99;
char   SensorFail = 0;
bool   indicatorState = true;
unsigned long mills = 0;
char nexPage = 0;
char nexButton = 0;
bool nexChange = false;
bool WIFIFAIL = false;
  
void setup() {
  
  pinMode(RELAYPin, OUTPUT);
  pinMode(LEDPin, OUTPUT);
  pinMode(DHTPin, INPUT);
  digitalWrite(LEDPin, LOW);
  digitalWrite(RELAYPin, LOW);
  //Serial.begin(115200);
  Serial.begin(9600);
  myNextion.init();
  myNextion.sendCommand("page page0");   //make sure we are at main menu on reboot
  delay(10);
  
  //DHT.read22(DHTPin);
  DHT.read11(DHTPin);
   
  delay(500);
  
  Serial.println(F("Cell Temp"));
  
  // Connect to WiFi access point.
  if(USEWIFI){
    int c=0;
    Serial.println(); Serial.println();
    Serial.print(F("Connecting to "));
    Serial.println(WLAN_SSID);
    
    WiFi.begin(WLAN_SSID, WLAN_PASS);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(c);
      Serial.print(",");
      if (c > 50) { 
        Serial.println("");
        Serial.print("WIFIFAIL: ");
        Serial.println(WIFIFAIL);
        WIFIFAIL = true;
        break;
      }   
      c++;
    }

        Serial.print("WIFIFAIL: ");
        Serial.println(WIFIFAIL);
        
    if(!WIFIFAIL){
    Serial.println();
    Serial.println(F("WiFi connected"));
    Serial.println(F("IP address: ")); 
    Serial.println(WiFi.localIP());
    
    Serial.println(F("Starting UDP"));
    Udp.begin(localPort);
    Serial.print(F("Local port: "));
    Serial.println(Udp.localPort());
    Serial.println(F("waiting for sync"));
    setSyncProvider(getNtpTime);
    setSyncInterval(3600);
    }
  }
  sensors.setResolution(addr0, 12);
  sensors.setResolution(addr1, 12);
  sensors.requestTemperatures(); 
  
  tmpOW0 = sensors.getTempC(addr0);
  tmpOW1 = sensors.getTempC(addr1);
  delay(2000);
  sensors.requestTemperatures(); 
  tmpOW0 = sensors.getTempC(addr0);
  tmpOW1 = sensors.getTempC(addr1);
//  for (int i=0; i < arrySize; i++){
//    hourAvg[i] = tmpOW;
//    hourMin[i] = tmpOW;
//    hourMax[i] = tmpOW;
//  }
  for (int i=0; i < 6; i++){
    minMinMax[0][i] = tmpOW0;
    minMinMax[1][i] = tmpOW1;
  }
  mills= millis();
  
  myNextion.sendCommand("click 1,0");
  nexPage = 1;
  nexButton = 3;
  nexChange = TRUE;
}


void loop() {
  //String message = myNextion.listen(); //check for message
  parseNexMsg(myNextion.listen());

  if(nexChange){  
    //Serial.println(int(nexPage));
    //Serial.println(int(nexButton));
    updateNexData();
    updateNexPage(nexPage, nexButton);
    nexChange = FALSE;
  }


  //Start main temp log 
  if (USELED) digitalWrite(LEDPin, HIGH);
  if(millis() - mills > 10000){
  mills= millis();
// Main code...   
  if(SensorFail > 50) ESP.restart(); // restart the esp8266 to see if that helps with the 10 consecutive failed reads
  
  if(!WIFIFAIL){MQTT_connect();}
  
  if (USELED) digitalWrite(LEDPin, LOW);
  sensors.requestTemperatures(); 
  tmpOW0 = sensors.getTempC(addr0);
  tmpOW1 = sensors.getTempC(addr1);
  
  if(!ckError(tmpOW0)) return; //start loop() again if temp reading is shit
  if(!ckError(tmpOW1)) return; //start loop() again if temp reading is shit
  
  SensorFail = 0;
  DHT.read11(DHTPin); //seem to need 2nd read for first time (first read moved to setup)
  if (USELED) digitalWrite(LEDPin, HIGH);

  if(DEBUG) printDHT(); //DHT11 debug messages

  humid = DHT.humidity;
  humidity.publish(humid,0);

  updateNexData();
  if(nexPage== 1 || nexPage == 3){
    updateNexPage(nexPage, nexButton);
  }

  //if (USELED) digitalWrite(LEDPin, LOW);
  tempOW0.publish(tmpOW0,1);
  tempOW1.publish(tmpOW1,1);
  //if (USELED) digitalWrite(LEDPin, HIGH);
  
  digitalWrite(RELAYPin, LOW);

  if(DEBUG){
    Serial.print(F("Digital temp: "));
    Serial.print(F("Top = "));
    Serial.print(tmpOW0);
    Serial.print(F("c. Bottom = "));
    Serial.print(tmpOW1);
    Serial.println(F("c."));
  }
  char thisMin = minute();
  minMinMax[0][second()/10] = tmpOW0;
  minMinMax[1][second()/10] = tmpOW1;
  
  if(DEBUG) printMinuteTemps(); //debug messages 
  minC = second()/10;
  if(minC == 0){
      
    saveTemps();
    if(DEBUG) printMinMaxAvg(); //debug messages
    //writeMQTTTemps(); //no space on io.adafruit for motr feeds
    
    if(DEBUG) printRunningAvg();
    
    ckAVgTemp(hourAvg[0][minute()]);
    ckAVgTemp(hourAvg[1][minute()]);
    nekMin = thisMin + 1;
    minAvg = 0;
    minC =  0;
//    Serial.print(F("free mem = "));
//    Serial.println(ESP.getFreeHeap());
  }
 
  if(DEBUG){
    if(hourC >= 60){ //60
      hourC = 0;
    }
  }


  if (minute() > 0) sendH = true;
  if (minute() == 0 && sendH == true){
//    tempHAvgH.publish(hourAvgArray.getAverage(),1);
//    Serial.print(F("Average over 60 min: "));
//    Serial.println(hourAvgArray.getAverage(),1);
//    tempHMinH.publish(hourMinArray.getMin(),1);
//    Serial.print(F("Min over 60 min: "));
//    Serial.println(hourMinArray.getMin(),1);
//    tempHMaxH.publish(hourMaxArray.getMax(),1);
//    Serial.print(F("Max over 60 min: "));
//    Serial.println(hourMaxArray.getMax(),1);
//    sendH = false;
//    if (hour() > 0) restart = true;
//    if (hour() == 0 && restart == true){
//      while(1)// truger the WD and restart
//    }
  }  

  }//delay(10000 - millis() + mills);
}

