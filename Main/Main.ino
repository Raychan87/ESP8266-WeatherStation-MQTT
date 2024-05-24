#include <ESP8266WiFi.h> 		  //Wlan-Client
#include <PubSubClient.h>		  //MQTT
#include <Wire.h>             //BME280
#include <Adafruit_Sensor.h>  //BME280
#include <Adafruit_BME280.h>  //BME280
#include "Y:/Accounts.h"			//Accounts

//---Inhalt von Accounts.h---//
//const char* SSID =          //Wlan SSID
//const char* PSK =           //Wlan Passwort
//const char* MQTT_BROKER =   //MQTT Server IP
//#define MQTT_PORT           //MQTT Port
//const char* MQTT_User =     //MQTT Account
//const char* MQTT_PW =       //MQTT Passwort

//---GPIO-PINs---//
#define I2C_ADRESS 0x76  //D2 = SDA, D1 = SCL

//
#define LOOP_TIME 250  //ms
#define SLEEP_TIME 10   //sec
#define BAUDRATE 115200

//MQTT Topic fürs empfangen von Daten
//#define MQTT_SUB_LOOPTIME "ESP32-Wetter/looptime" 
//#define MQTT_SUB_SLEEPTIME "ESP32-Wetter/sleeptime"

//MQTT Topic fürs senden von Daten
#define MQTT_TX_PRES "ESP32-Wetter/luftdruck"
#define MQTT_TX_TEMP "ESP32-Wetter/temperatur"
#define MQTT_TX_HUMI "ESP32-Wetter/luftfeuchtigkeit "

//________________________________________________________
//Var
int dutyCycle, DutyCounter;
float Temperatur, Luftdruck, Luftfeuchtigkeit;
int LoopTime = LOOP_TIME;
int SleepTime = SLEEP_TIME;
unsigned long previousMillis = 0;  
const char* MQTT_Clint_Name = "ESP-Wetter";

//________________________________________________________
//Init
WiFiClient espClient;           		//WLan
PubSubClient client(espClient); 		//MQTT
Adafruit_BME280 bme;                //BME280, I2C

//________________________________________________________
//Aufbau der WLan Verbindung
void setup_wifi() {
	delay(10);
	Serial.println();
	Serial.print("Connecting to ");
	Serial.println(SSID);

  //Anmelden ins WLan
	WiFi.begin(SSID, PSK);

  //Warten bis die Verbindung aufgebaut wurde
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	//Serial.println("");
  //Serial.println("WiFi connected");
	//Serial.println("IP address: ");
	//Serial.println(WiFi.localIP());
}

//________________________________________________________
//MQTT Verbindung und Reconnect
void reconnect() {
	while (!client.connected()) {
		Serial.print("Reconnecting...");
		if (!client.connect(MQTT_Clint_Name)) {
			Serial.print("failed, rc=");
			Serial.print(client.state());
			Serial.println(" retrying in 5 seconds");
			delay(5000);
		}else{
      Serial.println("connected");
//      client.subscribe(MQTT_SUB_LOOPTIME); //Eine Subscribe abonieren.
//      client.subscribe(MQTT_SUB_SLEEPTIME); //Eine Subscribe abonieren.
    }
	}
}

//________________________________________________________
//MQTT Empfangen der Topics
/*
void callback(char* topic, byte* payload, unsigned int length) {

  char msg[10];
  String srtTopic = topic; //Umwandeln des Topics in ein String

  //Payload der Nachricht auslesen
  for (int i = 0; i < length; i++) {
    msg[i] = (char)payload[i];
  }
  //String wird in Int gewandelt
  String stringa(msg);
  int msg_wert = stringa.toInt(); 

  //LoopTime
  if (srtTopic == MQTT_SUB_LOOPTIME){
    LoopTime = msg_wert;
  }

  //SleepTime
  if (srtTopic == MQTT_SUB_SLEEPTIME){
    SleepTime = msg_wert;
  }
}*/

//________________________________________________________
//Init
void setup() {
	Serial.begin(BAUDRATE); //Baudrate
  bme.begin(I2C_ADRESS); //BME280
	setup_wifi();	//Wlan verbinden
	client.setServer(MQTT_BROKER, MQTT_PORT); //MQTT Broker
 // client.setCallback(callback); //MQTT Empfangen von Nachrichten aktivieren
}

//________________________________________________________
//Main
void loop() {

  unsigned long currentMillis = millis();

	//Verbinden zum MQTT Server
	if (!client.connected()) {
	  reconnect();
	}
	client.loop();

  //Loop mit Delay
  if (currentMillis - previousMillis >= LoopTime) {
    previousMillis = currentMillis;

    //Temperatur Messung
    Temperatur = 0;
    for (int i=1; i <= 3; i++){
      Temperatur = Temperatur + bme.readTemperature();
    }
    Temperatur = Temperatur / 3;
    client.publish(MQTT_TX_TEMP,String(Temperatur).c_str(),true); //MQTT

    //Luftfeuchtigkeit Messung
    Luftfeuchtigkeit = 0;
    for (int i=1; i <= 3; i++){
      Luftfeuchtigkeit = Luftfeuchtigkeit + bme.readHumidity();
    }
    Luftfeuchtigkeit = Luftfeuchtigkeit / 3;
    client.publish(MQTT_TX_HUMI,String(Luftfeuchtigkeit).c_str(),true); //MQTT

    //Luftdruck Messung
    Luftdruck = 0;
    for (int i=1; i <= 3; i++){
      Luftdruck = Luftdruck + (bme.readPressure() / 100.0F);
    }
    Luftdruck = Luftdruck / 3;
    client.publish(MQTT_TX_PRES,String(Luftdruck).c_str(),true); //MQTT

    delay(100);

    // Die Zeit ist in Mikrosekunden angegeben
    // 1 Sekunde = 1.000.000 Mikrosekunden
    ESP.deepSleep(1e6 * SleepTime); // ESP8266 in den Deep-Sleep-Modus versetzen
  }
}




















