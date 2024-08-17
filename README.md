# ESP8266-WeatherStation-MQTT

 
### Beschreibung
Das Programm ist für ein ESP8266. Dieser wertet ein BME280 Sensor und ein INA219 Leistungsmesser aus.
Der BME280 misst Temperatur, Luftfeuchtigkeit und Luftdruck. Der INA219 misst Strom, Spannung und Leistung des Solarpanels, welches den Betrieb des ESP8266 und deren Akku versorgt.

- WLAN Betrieb
- MQTT empfangen
     - Temperatur
     - Luftfeuchtigkeit
     - Luftdruck
     - Spannung des Solarpanels
     - Strom des Solarpanels
     - Leistung des Solarpanels
 - MQTT senden (Steuerbar)
    - Temperaturkorrekur
    - Auslesezeit
    - Deepsleep Modus (0 = Off, 1 = On)
    - Deepsleep Time

 ### Verwendete Hardware
- NodeMCU v2 ESP8266

  ![grafik](https://github.com/Raychan87/ESP8266-TempFanControl-MQTT/assets/18511462/7b1a5c8b-3612-4b7d-93e3-93347f434481)
  
- SparkFun AP63203 3,3V StepDown Regler

  ![grafik](https://github.com/Raychan87/ESP8266-TempFanControl-MQTT/assets/18511462/4370400d-bd46-4e2f-a3be-0dc437de522f)
  
- BME280
  
 ![11803_2](https://github.com/user-attachments/assets/8a8cf645-a5ef-40ab-8c48-87cdec98e024)

 - INA219
   
![arduino-INA219-esuqema](https://github.com/user-attachments/assets/f167797b-ebce-4e50-a450-ae218bfcab82)

- Waveshare Solar Power Manager
  
![710WPsjkOxL](https://github.com/user-attachments/assets/6fd5dc3d-3197-4236-8d57-37d174cf5e98)

- 6V Waveshare Solar Panel
- 3,7V 2500mAh LP785060 LiPo Akku


  
