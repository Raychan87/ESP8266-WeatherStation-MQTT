#include <ESP8266WiFi.h> 		  //Wlan-Client
#include <PubSubClient.h>		  //MQTT
#include <Wire.h>             //BME280 & INA219
#include <Adafruit_Sensor.h>  //BME280
#include <Adafruit_BME280.h>  //BME280
#include <Adafruit_INA219.h>  //INA219
#include "Y:/Accounts.h"			//Accounts

//---Inhalt von Accounts.h---//
//const char* SSID =          //Wlan SSID
//const char* PSK =           //Wlan Passwort
//const char* MQTT_BROKER =   //MQTT Server IP
//#define MQTT_PORT           //MQTT Port
//const char* MQTT_User =     //MQTT Account
//const char* MQTT_PW =       //MQTT Passwort

//---GPIO-PINs---//
//D2 = SDA, D1 = SCL
#define I2C_BME280 0x76 //BME280
#define I2C_INA219 0x40 //INA219

//
#define LOOP_TIME 5000  //ms
#define SLEEP_TIME 10   //sec
#define SLEEP_MODE 1    //0 = OFF, 1 = ON
#define BAUDRATE 115200
#define TEMP_CORR -3.0  //K

//MQTT Topic fürs empfangen von Daten
#define MQTT_SUB_LOOPTIME "ESP8266-Wetter/wachtime_ms" 
#define MQTT_SUB_SLEEPTIME "ESP8266-Wetter/sleeptime_s"
#define MQTT_SUB_SLEEPMODE "ESP8266-Wetter/sleepmode"
#define MQTT_SUB_TEMPCORR "ESP8266-Wetter/tempkorrektur"

//MQTT Topic fürs senden von Daten
#define MQTT_TX_PRES "ESP8266-Wetter/luftdruck"
#define MQTT_TX_TEMP "ESP8266-Wetter/temperatur"
#define MQTT_TX_HUMI "ESP8266-Wetter/feuchtigkeit"
#define MQTT_TX_AMP "ESP8266-Wetter/strom_ma"
#define MQTT_TX_PWR "ESP8266-Wetter/leistung_mw"
#define MQTT_TX_BUS_V "ESP8266-Wetter/verbraucher_v"
#define MQTT_TX_SHU_V "ESP8266-Wetter/shunt_mv"
#define MQTT_TX_LOAD_V "ESP8266-Wetter/gesamt_v"

//________________________________________________________
//Var
int dutyCycle, DutyCounter;
float Temperatur, Luftdruck, Luftfeuchtigkeit;
struct config {
  int LoopTime;
  int SleepTime;
  int SleepMode;
  float TempCorr;
unsigned long previousMillis = 0;  
const char* MQTT_Clint_Name = "ESP-Wetter";
//INA219
//    Shunt    Verbraucher
//--+-|||||-------||||||-----GND
// |--VShunt--|  |--VBus--|
//|----VLoad(VShunt+VBus)------|
float shuntVoltage_mV = 0.0;
float loadVoltage_V = 0.0;
float busVoltage_V = 0.0;
float current_mA = 0.0;
float power_mW = 0.0; 

//________________________________________________________
//Init
WiFiClient espClient;           		        //WLan
PubSubClient client(espClient); 		        //MQTT
Adafruit_BME280 bme;                        //BME280, I2C
Adafruit_INA219 ina219(I2C_INA219);         //INA290, I2C

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
	Serial.println("");
  Serial.println("WiFi connected");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());
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
      Serial.println("MQTT connected");
      client.subscribe(MQTT_SUB_LOOPTIME); //Eine Subscribe abonieren.
      client.subscribe(MQTT_SUB_SLEEPTIME); //Eine Subscribe abonieren.
      client.subscribe(MQTT_SUB_SLEEPMODE); //Eine Subscribe abonieren.
      client.subscribe(MQTT_SUB_TEMPCORR); //Eine Subscribe abonieren.
    }
	}
}

//________________________________________________________
//MQTT Empfangen der Topics
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
    config.LoopTime = msg_wert;
  }

  //SleepTime
  if (srtTopic == MQTT_SUB_SLEEPTIME){
    config.SleepTime = msg_wert;
  }

  //SleepMode
  if (srtTopic == MQTT_SUB_SLEEPMODE){
    config.SleepMode = msg_wert;
  }

  //Temperatur Korrektur
  if (srtTopic == MQTT_SUB_TEMPCORR){
    config.TempCorr = msg_wert;
  }

  //Schreiben der Daten in den EEPROM
  EEPROM.begin(256);      //Puffergröße die verwendet werden soll
  EEPROM.put(0, config);  //Schreiben einer Structur ab Adresse 0
  EEPROM.commit();        //Übernahme in den Flash
  EEPROM.end();           //Schließt die EEPROM Operation
}

//________________________________________________________
//Init
void setup() {
	Serial.begin(BAUDRATE); //Baudrate
  bme.begin(I2C_BME280);  //BME280 Messung starten
  ina219.begin();         //INA219 Messung starten
	setup_wifi();           //Wlan verbinden
	client.setServer(MQTT_BROKER, MQTT_PORT); //MQTT Broker
  client.setCallback(callback); //MQTT Empfangen von Nachrichten aktivieren
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
  if (currentMillis - previousMillis >= config.LoopTime) {
    previousMillis = currentMillis;

    //********************************************//
    //___________________BME280___________________//

    //Temperatur Messung
    Temperatur = 0;
    for (int i=1; i <= 3; i++){
      Temperatur = Temperatur + bme.readTemperature();
    }
    Temperatur = Temperatur / 3;
    Temperatur = Temperatur + config.TempCorr;
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

    //********************************************//
    //___________________IMA290___________________//

    //Shunt Spannung
    shuntVoltage_mV = ina219.getShuntVoltage_mV();
    client.publish(MQTT_TX_SHU_V,String(shuntVoltage_mV).c_str(),true); //MQTT

    //Verbraucher Spannung
    busVoltage_V = ina219.getBusVoltage_V();
    client.publish(MQTT_TX_BUS_V,String(busVoltage_V).c_str(),true); //MQTT

    //Gesamtspannung
    loadVoltage_V  = busVoltage_V + (shuntVoltage_mV/1000);
    client.publish(MQTT_TX_LOAD_V,String(loadVoltage_V).c_str(),true); //MQTT

    //Strom
    current_mA = ina219.getCurrent_mA();
    client.publish(MQTT_TX_AMP,String(current_mA).c_str(),true); //MQTT

    //Leistung
    power_mW = ina219.getPower_mW();
    client.publish(MQTT_TX_PWR,String(power_mW).c_str(),true); //MQTT
  
    //Energiesparmodus
    if (config.SleepMode){

      delay(250);

      // Die Zeit ist in Mikrosekunden angegeben
      // 1 Sekunde = 1.000.000 Mikrosekunden
      ESP.deepSleep(1e6 * config.SleepTime); // ESP8266 in den Deep-Sleep-Modus versetzen
    }    
  }
}




















