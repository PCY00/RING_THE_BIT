#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"

#define BT_RXD 4
#define BT_TXD 5
#define LED 13
#define BUTTON_PIN 2 // 버튼을 연결한 핀 번호

SoftwareSerial HM10(BT_RXD, BT_TXD);  // RX핀(4번)은 HM10의 TX에 연결
                                      // TX핀(5번)은 HM10의 RX에 연결  
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
  pinMode(LED, OUTPUT);    // LED를 출력으로 설정
  pinMode(BUTTON_PIN, INPUT_PULLUP); // 내장 풀업 저항을 사용하여 버튼 핀을 입력으로 설정
  HM10.begin(9600);

  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
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
      //Serial.println( beatsPerMinute);
    }
  }
  
  // Bluetooth 모듈에서 데이터를 읽어옴
  if (HM10.available()){       
    char h = (char)HM10.read();
    Serial.println(h);
    if(h == 'o'){                   // 알파벳 소문자 'o'를 입력하면
      digitalWrite(LED, HIGH);     // LED가 점등됨
    }
    if(h == 'f'){                   // 알파벳 소문자 'f'를 입력하면
      digitalWrite(LED, LOW);       // LED가 소등됨
    }
  }
  
  // 버튼을 눌렀을 때 central로 "1" 전송
  if(digitalRead(BUTTON_PIN) == LOW){ // 버튼이 눌렸는지 확인
    //HM10.println("1"); // central로 "1"을 전송
    //Serial.println("눌렸다");

    beatAvg /= RATE_SIZE;
    
    data_json(true, (int)beatAvg);

    beatAvg = 0.0;
    RATE_SIZE = 0;
    
    delay(1000); // 중복 전송 방지를 위해 1초의 딜레이 추가
  }
  
  // 시리얼 모니터를 통해 데이터를 읽어서 Bluetooth 모듈로 전송함
  if(Serial.available()){
    char h = (char)Serial.read();
    HM10.println(h);
  }
}

void data_json(bool status, int bpm){
  buffer["power_status"] = status;
  buffer["bpm"] = bpm;

  char output[256];

  serializeJson(buffer, output);
  Serial.println(output);
  HM10.println(output);
}
