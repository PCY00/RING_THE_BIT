#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"

MAX30105 particleSensor;

long RATE_SIZE = 0;
long lastBeat = 0;
float beatAvg = 0.0;
float beatsPerMinute;

bool runEvery(unsigned long interval);

void setup() {
  Serial.begin(9600);

  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    while (1);
  }

  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x0A);
  particleSensor.setPulseAmplitudeGreen(0);
}

void loop() {
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
      //Serial.println( beatsPerMinute);
    }
  }

  if(runEvery(30000)){
    Serial.println((String)"All beat_Avg" + beatAvg);
    beatAvg /= RATE_SIZE;
    Serial.println((String)", Avg BPM=" + (int)beatAvg);
    Serial.println((String)"conunt:" + RATE_SIZE);
    
    beatAvg = 0.0;
    RATE_SIZE = 0;
  }
  
  if (irValue < 50000){
    Serial.println(" No finger?");
  }
}

bool runEvery(unsigned long interval){
  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    return true;
  }
  return false;
}
