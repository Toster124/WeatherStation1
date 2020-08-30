//------БИБЛИОТЕКИ------
#include <ServoSmooth.h>
#include <LowPower.h>
#include <SoftwareSerial.h>
#include <JsonParser.h> 
#include <dht.h>
#include "GyverTimer.h"
#include <Wire.h>
#include "RTClib.h"
//------ИНИЦИАЛИЗАЦИЯ------
dht DHT;
#define AMOUNT 6  // кол-во серво
ServoSmooth servos[AMOUNT];
RTC_DS3231 rtc;
JsonParser<28> parser;
JsonParser<10> parser1;
SoftwareSerial esp8266 (8, 9);
SoftwareSerial ard (11, 12);
GTimer_ms myTimer(20000);
GTimer_ms myTimer1(30000);
GTimer_ms myTimer2(3000);
boolean wake_flag, move_arrow, humflag = 1, esp_servo_wake_flag = 0, err;
byte sleep_count, flagl = 0, tempesp, temp2, tempday, humesp, humesp1, esp_wake_flag = 3, hum, temp, last_angle_temp, last_angle_hum, v = 0;
float mytemp[10];
float vcc;   
String esp, sendm, description;
int icon[10];
uint32_t servoTimer;
byte m;

void setup() {
//Serial.begin(9600);
esp8266.begin(9600); 
ard.begin(9600);
rtc.begin();

DHT.read22(A0);

myTimer.setMode(MANUAL);
myTimer1.setMode(MANUAL);
myTimer2.setMode(MANUAL);
myTimer.stop();
myTimer.reset();
myTimer1.stop();
myTimer1.reset();


pinMode(2, OUTPUT);
pinMode(4, OUTPUT);
pinMode(10, OUTPUT);
pinMode(12, OUTPUT);
pinMode(A3, OUTPUT);
digitalWrite(2, 1);

delay(200);

  servos[0].attach(5); //tempesp
  servos[1].attach(A2);//temp2
  servos[2].attach(3);//tempday
  servos[3].attach(A1);//humesp
  servos[4].attach(7);//hum
  servos[5].attach(6);//temp
  
  servos[0].setSpeed(20);
  servos[0].setAccel(0.05);
  servos[1].setAccel(0.05);
  servos[1].setSpeed(20);
  servos[2].setSpeed(20);
  servos[2].setAccel(0.05);
  servos[3].setSpeed(20);
  servos[3].setAccel(0.05);
  servos[4].setSpeed(20);
  servos[4].setAccel(0.05);
  servos[5].setSpeed(20);
  servos[5].setAccel(0.05);
  
delay(200);
for (byte m = 0; m < AMOUNT; m++) {
     servos[m].setTargetDeg(0);
    }
    myTimer2.reset();
myTimer2.start();  
while(!myTimer2.isReady()) {
    if (millis() - servoTimer >= 20) {
    servoTimer = millis();
    for (byte i = 0; i < AMOUNT; i++) {
      servos[i].tickManual();   // двигаем все сервы. Такой вариант эффективнее отдельных тиков
    }
  }
}

delay(300);

int voltage = readVcc();
voltage = map(voltage, 3000, 4200, 180, 0);
voltage = constrain(voltage, 0, 180);
for (byte m = 0; m < AMOUNT; m++) {
     servos[m].setTargetDeg(voltage);
    }
        myTimer2.reset();
myTimer2.start();
    while(!myTimer2.isReady()) {
    if (millis() - servoTimer >= 20) {
    servoTimer = millis();
    for (byte i = 0; i < AMOUNT; i++) {
      servos[i].tickManual();   // двигаем все сервы. Такой вариант эффективнее отдельных тиков
    }
  }
}
   
voltage = 0;
delay(300);

for (byte m = 0; m < AMOUNT; m++) {
     servos[m].setTargetDeg(90);
    }
    myTimer2.reset();
myTimer2.start();  
while(!myTimer2.isReady()) {
    if (millis() - servoTimer >= 20) {
    servoTimer = millis();
    for (byte i = 0; i < AMOUNT; i++) {
      servos[i].tickManual();   // двигаем все сервы. Такой вариант эффективнее отдельных тиков
    }
  }
}
delay(1000);
digitalWrite(2, 0);
wake_flag = 1;
}

void loop() {
if (wake_flag) {
      delay(500);
 //------ДАТЧИК DH22------
 DHT.read22(A0);
 delay(500);
  temp = map(DHT.temperature, 0, 40, 180, 0);
  temp = constrain(temp, 0, 180);
  hum = map(DHT.humidity, 0, 100, 180, 0);
  hum = constrain(hum, 0, 180);
  delay(500);
   if (esp_wake_flag == 3) {
    digitalWrite(A3, 1);
    delay(10); 
    myTimer1.reset();
        myTimer1.start();
        esp8266.listen();
    while(!esp8266.find("WIFI GOT IP") && !myTimer1.isReady()) {}
    if (!myTimer1.isReady()) {
      myTimer.reset();
        myTimer.start();
    }
    delay(10); 
      esp8266.println("AT+CIPMODE=1");
      while(!esp8266.find("OK") && !myTimer.isReady()) {}
      if (!myTimer.isReady()) {
      myTimer.reset();
        myTimer.start();
    }
    delay(10); 
        esp8266.println("AT+CIPSTART=\"""TCP""\",\"""api.openweathermap.org""\",80");
        while(!esp8266.find("OK") && !myTimer.isReady()) {}
      if (!myTimer.isReady()) {
      myTimer.reset();
        myTimer.start();
    }
    delay(10); 
        esp8266.println("AT+CIPSEND");
        while(!esp8266.find(">") && !myTimer.isReady()) {}
      if (!myTimer.isReady()) {
      myTimer.reset();
        myTimer.start();
    }
    delay(10); 
        esp8266.println("GET /data/2.5/forecast?id=553915&appid=6a4ba421859c9f4166697758b68d889b&mode=json&units=metric&cnt=10");
       
      while (flagl < 10 && !myTimer.isReady()) {
      int n = 1; // счетчик символов
        char json[150]="{";
        while (!esp8266.find("\"main\":{") && !myTimer.isReady()){}
        while (!myTimer.isReady()) {
        if(esp8266.available() && !myTimer.isReady()) {
          char c = esp8266.read();
            json[n]=c;
            if(c=='}') break;
              n++;
            }
            }
            char jsona[100]="{";
            int p = 1; // счетчик символов
             while (!esp8266.find("\"weather\":[{") && !myTimer.isReady()){}
           while (!myTimer.isReady()) {
        if(esp8266.available() && !myTimer.isReady()) {
          char c = esp8266.read();
            jsona[p]=c;
            if(c=='}') break;
              p++;
            
            }
            }
 JsonHashTable hashTable = parser.parseHashTable(json);
  JsonHashTable hashTable1 = parser1.parseHashTable(jsona);
     float tempars = hashTable.getDouble("temp");
      String icons = hashTable1.getString("icon");
      icon[v] = icons.toInt();
      
if (humflag == 1) {
  humesp1 = hashTable.getLong("humidity");
 description = hashTable1.getString("description");
  humflag = 0;
} 
 mytemp[v] = tempars;
  v++;
  flagl++;
  
}

if (myTimer.isReady()) {
  err = 1;
  }
  else {
    err = 0;
  }
  myTimer.stop();
  myTimer.reset();
  myTimer1.stop();
  myTimer1.reset();

  digitalWrite(A3, 0);
 
   
    tempesp = map(mytemp[0], -30, 30, 180, 0);
    if (mytemp[0] > 30) {tempesp = 0;}
    if (mytemp[0] < -30) {tempesp = 180;}

    temp2 = map(mytemp[2], -30, 30, 180, 0);
    if (mytemp[2] > 30) {temp2 = 0;}
  if (mytemp[2] < -30) {temp2 = 180;}
  
    tempday = map(mytemp[9], -30, 30, 180, 0);
   if (mytemp[9] > 30) {tempday = 0;}
  if (mytemp[9] < -30) {tempday = 180;}

    humesp = map(humesp1, 0, 100, 180, 0);
    humesp = constrain(humesp, 0, 180);
    esp_wake_flag = 0;
  esp_servo_wake_flag = 1;
 }
 
    // дальше такая фишка: если угол несильно изменился с прошлого раза, то нет смысла лишний раз включать серву
    // и тратить энергию/жужжать. Так что находим разницу, и если изменение существенное - то поворачиваем стрелку    
    if (abs(temp - last_angle_temp) > 3 || abs(hum - last_angle_hum) > 3) move_arrow = 1;
    DateTime now = rtc.now();
if ((move_arrow || esp_servo_wake_flag) && now.hour() > 7) {
  last_angle_temp = temp;
  last_angle_hum = hum;
  digitalWrite(2, 1);
  delay(500);
    servos[0].setTargetDeg(tempesp);
    servos[1].setTargetDeg(temp2);
    servos[2].setTargetDeg(tempday);
    servos[3].setTargetDeg(humesp);
    servos[4].setTargetDeg(hum);
    servos[5].setTargetDeg(temp);
    myTimer2.reset();
myTimer2.start();  
while(!myTimer2.isReady()) {
    if (millis() - servoTimer >= 20) {
    servoTimer = millis();
    for (byte i = 0; i < AMOUNT; i++) {
      servos[i].tickManual();   // двигаем все сервы. Такой вариант эффективнее отдельных тиков
    }
  }
}
delay(500);
  digitalWrite(2, 0);
  move_arrow = 0;
  esp_servo_wake_flag = 0;
}
  v = 0;
  flagl = 0;
  esp_wake_flag++;
    humflag = 1;
    vcc = readVcc();
   
   
digitalWrite(4, 1);
ard.listen();
 delay(500);
 sendm = "{\"date\":";
  sendm +=now.year();
  sendm +=".";
  sendm +=now.month();
sendm +=".";
 sendm +=now.day();
sendm +=",\"hour\":";
sendm +=now.hour();
  sendm +=",\"min\":";
  sendm +=now.minute();
sendm +=",\"sec\":";
 sendm +=now.second();
 sendm += ",\"tempesp\":";
 sendm +=mytemp[0];
  sendm +=",\"temph\":";
  sendm +=mytemp[2];
  sendm +=",\"tempday\":";
  sendm +=mytemp[9];
  sendm +=",\"humesp\":";
  sendm +=humesp1;
  sendm +=",\"temp\":";
  sendm +=DHT.temperature;
  sendm +=",\"hum\":";
  sendm +=DHT.humidity;
 sendm += ",\"vcc\":";
 sendm +=vcc/1000;
 sendm += ",\"description\":";
 sendm +=description;
 sendm += ",\"err\":";
 sendm +=err;
  sendm +="}";
  while (!ard.available()) {}
  delay(2000);
    ard.println(sendm);
// Serial.println(sendm);
    delay(100);
myTimer1.reset();
        myTimer1.start();
    while(!myTimer1.isReady() && !ard.find("1")) {};
    digitalWrite(4, 0);

    
    if (readVcc() < 3250) {
  digitalWrite(2, 1);
delay(1000);
for (byte m = 0; m < AMOUNT; m++) {
     servos[m].setTargetDeg(178);
    }
    myTimer2.reset();
myTimer2.start();  
while(!myTimer2.isReady()) {
    if (millis() - servoTimer >= 20) {
    servoTimer = millis();
    for (byte i = 0; i < AMOUNT; i++) {
      servos[i].tickManual();   // двигаем все сервы. Такой вариант эффективнее отдельных тиков
    }
  }
}
delay(2000);
  digitalWrite(2, 0);
  delay(100);
LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);     // вечный сон если акум сел
    }
    
    delay(100);
    wake_flag = 0;
    delay(10);                       // задержка для стабильности
  }

LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);      // спать 8 сек. mode POWER_OFF, АЦП выкл
 sleep_count++;            // +1 к счетчику просыпаний
  if (sleep_count >= 72) {  // если время сна превысило 10 минут (75 раз по 8 секунд - подгон = 70)
    wake_flag = 1;          // рарешить выполнение расчета
    sleep_count = 0;        // обнулить счетчик
    delay(2);               // задержка для стабильности
  }



}


long readVcc() { //функция чтения внутреннего опорного напряжения, универсальная (для всех ардуин)
#if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
  ADMUX = _BV(MUX5) | _BV(MUX0);
#elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
  ADMUX = _BV(MUX3) | _BV(MUX2);
#else
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#endif
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA, ADSC)); // measuring
  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH
  uint8_t high = ADCH; // unlocks both
  long result = (high << 8) | low;

  result = 1.080 * 1023 * 1000 / result; // расчёт реального VCC
  return result; // возвращает VCC
}
