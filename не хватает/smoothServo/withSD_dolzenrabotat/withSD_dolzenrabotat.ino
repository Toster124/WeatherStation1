//------БИБЛИОТЕКИ------
#include <ServoSmooth.h>
#include <LowPower.h>
#include <SoftwareSerial.h>
#include <JsonParser.h> 
#include <dht.h>
#include "GyverTimer.h"
#include "SD.h"
//------ИНИЦИАЛИЗАЦИЯ------
dht DHT;
JsonParser<28> parser;
SoftwareSerial esp8266 (8, 9);
GTimer_ms myTimer(20000);
GTimer_ms myTimer1(30000);
GTimer_ms myTimer2(3000);
boolean wake_flag, move_arrow, humflag = 1, esp_servo_wake_flag = 0, err;
byte sleep_count, flagl = 0, tempesp, temp2, tempday, humesp, humesp1, esp_wake_flag = 3, hum, temp, last_angle_temp, last_angle_hum, v = 0;
int mytemp[10];
float vcc;   
String esp;
uint32_t servoTimer;

ServoSmooth servos[6];

void setup() {
esp8266.begin(9600); 
DHT.read22(A0);

myTimer.setMode(MANUAL);
myTimer1.setMode(MANUAL);
myTimer2.setMode(MANUAL);
myTimer.stop();
myTimer.reset();
myTimer1.stop();
myTimer1.reset();


pinMode(2, OUTPUT);
pinMode(A3, OUTPUT);
digitalWrite(2, 1);

delay(200);

  servos[0].attach(5);
  servos[1].attach(A2);
  servos[2].attach(A4);
  servos[3].attach(A5);
  servos[4].attach(7);
  servos[5].attach(6);
  
  servos[0].setSpeed(20);
  servos[1].setAccel(0.2);
  servos[2].setSpeed(100);
  servos[3].setAccel(0.05);
  servos[4].setSpeed(20);
  servos[5].setAccel(0.2);

delay(200);
servoTimer = millis();
for (byte i = 0; i < 6; i++) {
      servos[i].setTargetDeg(0);
    }
while (millis() - servoTimer <= 3000) {
    for (byte i = 0; i < 6; i++) {
      servos[i].tickManual();   // двигаем все сервы. Такой вариант эффективнее отдельных тиков
    }
  }
 

delay(1000);

int voltage = readVcc();
voltage = map(voltage, 3000, 4200, 180, 0);
voltage = constrain(voltage, 0, 180);
servoTimer = millis();
for (byte i = 0; i < 6; i++) {
      servos[i].setTargetDeg(voltage);
    }
while (millis() - servoTimer <= 3000) {
    for (byte i = 0; i < 6; i++) {
      servos[i].tickManual();   // двигаем все сервы. Такой вариант эффективнее отдельных тиков
    }
  }
delay(1000);

servoTimer = millis();
for (byte i = 0; i < 6; i++) {
      servos[i].setTargetDeg(90);
    }
while (millis() - servoTimer <= 3000) {
    for (byte i = 0; i < 6; i++) {
      servos[i].tickManual();   // двигаем все сервы. Такой вариант эффективнее отдельных тиков
    }
  }
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
        esp8266.println("GET /data/2.5/forecast?q=Kaluga,ru&appid=6a4ba421859c9f4166697758b68d889b&mode=json&units=metric&cnt=10");
       
      while (flagl < 10 && !myTimer.isReady()) {
          unsigned int i = 0; //счетчик времени
      int n = 1; // счетчик символов
        char json[150]="{";
        while (!esp8266.find("\"main\":{") && !myTimer.isReady()){}
        while (i<60000 && !myTimer.isReady()) {
        if(esp8266.available() && !myTimer.isReady()) {
          char c = esp8266.read();
            json[n]=c;
            if(c=='}') break;
              n++;
             i=0;
            }
            i++;
            }
           
 JsonHashTable hashTable = parser.parseHashTable(json);
     int tempars = hashTable.getLong("temp");
      
if (humflag == 1) {
  humesp1 = hashTable.getLong("humidity");
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
    tempesp = constrain(tempesp, 0, 180);

    temp2 = map(mytemp[2], -30, 30, 180, 0);
    temp2 = constrain(temp2, 0, 180);

    tempday = map(mytemp[9], -30, 30, 180, 0);
    tempday = constrain(tempday, 0, 180);

    humesp = map(humesp1, 0, 100, 180, 0);
    humesp = constrain(humesp, 0, 180);
    esp_wake_flag = 0;
  esp_servo_wake_flag = 1;
 }
 
    // дальше такая фишка: если угол несильно изменился с прошлого раза, то нет смысла лишний раз включать серву
    // и тратить энергию/жужжать. Так что находим разницу, и если изменение существенное - то поворачиваем стрелку    
    if (abs(temp - last_angle_temp) > 3 || abs(hum - last_angle_hum) > 3) move_arrow = 1;
    
if (move_arrow || esp_servo_wake_flag) {
  last_angle_temp = temp;
  last_angle_hum = hum;
  digitalWrite(2, 1);
myTimer2.reset();
myTimer2.start();  
servoTimer = millis();

  servos[0].write(tempesp);
  delay(100);
  servos[1].write(temp2);
  delay(100);
  servos[2].write(tempday);
  delay(100);
  servos[3].write(humesp);
  delay(100);
  servos[4].write(hum);
  delay(100);
  servos[5].write(temp);
  servoTimer = millis();
while (millis() - servoTimer <= 3000) {
    for (byte i = 0; i < 6; i++) {
      servos[i].tickManual();   // двигаем все сервы. Такой вариант эффективнее отдельных тиков
    }
  }
  digitalWrite(2, 0);
  move_arrow = 0;
  esp_servo_wake_flag = 0;
}
  v = 0;
  flagl = 0;
  esp_wake_flag++;
    humflag = 1;
    vcc = readVcc();
    DHT.read22(A0);
 delay(500);
    myTimer2.reset();
    myTimer2.start();
    while(!SD.begin(4) && !myTimer2.isReady()) {}
    delay(100);
    File myFile = SD.open("weather.csv", FILE_WRITE); // постирония
    myTimer2.reset();
  	myTimer2.start();  

        myFile.print("\"");
        if (!err) {
        myFile.print(mytemp[0]);
        }
       	myFile.print("\",\"");
         if (!err) {
       	myFile.print(mytemp[2]);
         }
        myFile.print("\",\"");
        if (!err) {
       	myFile.print(mytemp[9]);
        }
       	myFile.print("\",\"");
         if (!err) {
       	myFile.print(humesp1);
         }
        myFile.print("\",\"");
        myFile.print(DHT.temperature);
        myFile.print("\",\"");
        myFile.print(DHT.humidity);
        myFile.print("\",\"");
        myFile.print(vcc/1000);
        myFile.println("\"");
       
    while(!myTimer2.isReady()) {} 
   
    myFile.close();
    delay(100);
    if (readVcc() < 3250) {
  digitalWrite(2, 1);
servoTimer = millis();
for (byte i = 0; i < 6; i++) {
      servos[i].setTargetDeg(178);
    }
while (millis() - servoTimer <= 3000) {
    for (byte i = 0; i < 6; i++) {
      servos[i].tickManual();   // двигаем все сервы. Такой вариант эффективнее отдельных тиков
    }
  }
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
