#include <SoftwareSerial.h>

#define BT_RXD 4
#define BT_TXD 5

SoftwareSerial HM10(BT_RXD, BT_TXD);  // RX핀(7번)은 HM10의 TX에 연결
                                      // TX핀(8번)은 HM10의 RX에 연결
void setup(){
  Serial.begin(9600);
  HM10.begin(9600); 
}

void loop(){
  if (HM10.available()){       
    //char h =(char)HM10.read();
    String data = HM10.readStringUntil('\n');
    Serial.println(data); 
  }
  /*if(Serial.available()){
    char h = (char)Serial.read();
    HM10.print(h);
  }*/
}
