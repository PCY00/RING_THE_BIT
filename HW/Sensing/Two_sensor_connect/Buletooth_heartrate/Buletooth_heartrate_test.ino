#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"

#define BT_RXD 4
#define BT_TXD 5

SoftwareSerial HM10(BT_RXD, BT_TXD);
 
StaticJsonDocument<256> buffer;
MAX30105 particleSensor;

long RATE_SIZE = 0;
long lastBeat = 0;
float beatAvg = 0.0;
float beatsPerMinute;

bool runEvery(unsigned long interval);
void data_json(bool status, int bpm);
  
void setup(){
  Serial.begin(9600);
  HM10.begin(9600);

  if (!particleSensor.begin(Wire, I2C_SPEED_FAST))
  {
    while (1);
  }
  
  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x0A);
  particleSensor.setPulseAmplitudeGreen(0);

}

void loop(){

  long irValue = particleSensor.getIR();

  if (checkForBeat(irValue) == true) {
    //We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20)
    {
      beatAvg += beatsPerMinute;
      RATE_SIZE++;
    }
  }

  if (Serial.available()){       
    String str = Serial.readStringUntil('\n');
    if(str == "ON"){
      Serial.println(str);
      beatAvg /= RATE_SIZE;
      data_json(true, (int)beatAvg);
      beatAvg = 0.0;
      RATE_SIZE = 0;
      
    }else if(str == "OFF"){
      Serial.println(str);
      beatAvg /= RATE_SIZE;
      data_json(false, (int)beatAvg);
      beatAvg = 0.0;
      RATE_SIZE = 0;
      
    }
  }
}

void data_json(bool status, int bpm){
  buffer["power_status"] = status;
  buffer["bpm"] = bpm;

  char output[256];

  serializeJson(buffer, output);

  HM10.println(output);
}
