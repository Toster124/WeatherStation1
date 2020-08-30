//------БИБЛИОТЕКИ------
#include <Servo.h>
#include <LowPower.h>
#include <SoftwareSerial.h>
#include <JsonParser.h> 
#include <dht.h>
#include "GyverTimer.h"
#include <Wire.h>
#include "RTClib.h"
//------ИНИЦИАЛИЗАЦИЯ------
dht DHT;
RTC_DS3231 rtc;
JsonParser<28> parser;
JsonParser<10> parser1;
SoftwareSerial esp8266 (8, 9);
SoftwareSerial ard (11, 12);
GTimer_ms myTimer(100000);
GTimer_ms myTimer1(70000);
GTimer_ms myTimer2(3000);
boolean wake_flag, move_arrow, humflag = 1, esp_servo_wake_flag = 0, err;
byte sleep_count, flagl = 0, tempesp, temp2, tempday, humesp, humesp1, esp_wake_flag = 3, hum, temp, last_angle_temp, last_angle_hum, v = 0;
float mytemp[10];
float vcc;   
String esp, sendm, description;
uint32_t sendTimer;
int icon[10];
Servo servotempesp;
Servo servotemp2;
Servo servotempday;
Servo servohumesp;
Servo servohum;
Servo servotemp;

void setup() {
//Serial.begin(9600);
//uart.begin(9600);
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

servotempesp.attach(5);
servotemp2.attach(A2);
servotempday.attach(3);
servohumesp.attach(A1);
servohum.attach(7);
servotemp.attach(6);

delay(200);
servotempesp.write(0);
delay(100);
servotemp2.write(0);
delay(100);
servotempday.write(0);
delay(100);
servohumesp.write(0);
delay(100);
servohum.write(0);
delay(100);
servotemp.write(0);

delay(3000);

int voltage = readVcc();
voltage = map(voltage, 3000, 4200, 180, 0);
voltage = constrain(voltage, 0, 180);
servotempesp.write(voltage);
delay(100);
servotemp2.write(voltage);
delay(100);
servotempday.write(voltage);
delay(100);
servohumesp.write(voltage);
delay(100);
servohum.write(voltage);
delay(100);
servotemp.write(voltage);
voltage = 0;
delay(3000);

servotempesp.write(90);
delay(100);
servotemp2.write(90);
delay(100);
servotempday.write(90);
delay(100);
servohumesp.write(90);
delay(100);
servohum.write(90);
delay(100);
servotemp.write(90);
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
    delay(1500); 
    myTimer1.reset();
        myTimer1.start();
        esp8266.listen();
        esp8266.write("AT\r\n");
 while(!esp8266.find("OK") && !myTimer1.isReady()){
   if (millis() - sendTimer >= 500) {
    sendTimer = millis();
    esp8266.write("AT\r\n");
   // uart.println("1");
   }
  }
if (!myTimer1.isReady()) {
      myTimer.reset();
        myTimer.start();
    }


  // set the data rate for the SoftwareSerial port
  esp8266.write("AT+CREG?\r\n");
  while(!esp8266.find("1") && !myTimer.isReady()){
   if (millis() - sendTimer >= 500) {
    sendTimer = millis();
    esp8266.write("AT+CREG?\r\n");
   // uart.println("2");
   }

  }
if (!myTimer.isReady()) {
      myTimer.reset();
        myTimer.start();
    }


  esp8266.write("AT+CSTT=\"internet.beeline.ru\",\"beeline\",\"beeline\"\r\n");
  while(!esp8266.find("OK") && !myTimer.isReady()){}
  if (!myTimer.isReady()) {
      myTimer.reset();
        myTimer.start();
    }
  esp8266.write("AT+CIICR\r\n");
  while(!esp8266.find("OK") && !myTimer.isReady()){}
  if (!myTimer.isReady()) {
      myTimer.reset();
        myTimer.start();
    }
  esp8266.write("AT+CIFSR\r\n");
   
    while(!esp8266.available() && !myTimer.isReady()){}

  delay(2000);
  if (!myTimer.isReady()) {
      myTimer.reset();
        myTimer.start();
    }
  esp8266.write("ATE1V1+CMEE=2; &W\r\n");
  while(!esp8266.find("OK") && !myTimer.isReady()){}
 if (!myTimer.isReady()) {
      myTimer.reset();
        myTimer.start();
    }
  esp8266.write("AT+CIPSTART=\"TCP\",\"api.openweathermap.org\",80\r\n");
  while(!esp8266.find("CONNECT OK") && !myTimer.isReady()){}
 if (!myTimer.isReady()) {
      myTimer.reset();
        myTimer.start();
    }
  esp8266.write("AT+CIPSEND=105\r\n");
 
  delay(1000);
  esp8266.write("GET /data/2.5/forecast?q=Kaluga,ru&appid=6a4ba421859c9f4166697758b68d889b&mode=json&units=metric&cnt=10\r\n");
  while(!esp8266.find("SEND OK") && !myTimer.isReady()){}
  myTimer.reset();
  myTimer.start();
      while (flagl < 10 && !myTimer.isReady()) {
      int n = 1; // счетчик символов
        char json[150]="{";
        while (!esp8266.find("\"main\":{")){}
        while(true){
        if(esp8266.available()) {
          char c = esp8266.read();
            json[n]=c;
            if(c=='}') break;
              n++;
            }}
          //  uart.println(json);
            char jsona[100]="{";
            int p = 1; // счетчик символов
             while (!esp8266.find("\"weather\":[{")){}
           while (true) {
        if(esp8266.available()) {
          char c = esp8266.read();
            jsona[p]=c;
            if(c=='}') break;
              p++;
            
            }
            }
          //  uart.println(jsona);
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

if (myTimer.isReady() || humesp1 < 10) {
  err = 1;
  }
  else {
    err = 0;
  }
  vcc = readVcc();
//  delay(1000);
//  sendSMS();

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
    if (!err) {
  esp_servo_wake_flag = 1;
    }
 }
 
    // дальше такая фишка: если угол несильно изменился с прошлого раза, то нет смысла лишний раз включать серву
    // и тратить энергию/жужжать. Так что находим разницу, и если изменение существенное - то поворачиваем стрелку    
    if (abs(temp - last_angle_temp) > 3 || abs(hum - last_angle_hum) > 3) move_arrow = 1;
    DateTime now = rtc.now();
if (move_arrow || esp_servo_wake_flag) {
  last_angle_temp = temp;
  last_angle_hum = hum;
  digitalWrite(2, 1);
myTimer2.reset();
myTimer2.start();  
while(!myTimer2.isReady()) {
  servotempesp.write(tempesp);
  delay(100);
  servotemp2.write(temp2);
  delay(100);
  servotempday.write(tempday);
  delay(100);
  servohumesp.write(humesp);
  delay(100);
  servohum.write(hum);
  delay(100);
  servotemp.write(temp);
  
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
servotempesp.write(178);
delay(100);
servotemp2.write(178);
delay(100);
servotempday.write(178);
delay(100);
servohumesp.write(178);
delay(100);
servohum.write(178);
delay(100);
servotemp.write(178);
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
void sendSMS() {
  float tempdht = DHT.temperature;
  float humdht = DHT.humidity;
      myTimer.reset();
        myTimer.start();

  esp8266.write("AT+CMGF=1\r\n");
  while (!esp8266.find("OK") && !myTimer.isReady()){}
  if (!myTimer.isReady()) {
      myTimer.reset();
        myTimer.start();
    }
  esp8266.write("AT+CMGS=\"+79621713511\"\r\n");
  while(!esp8266.find(">") && !myTimer.isReady()) {};
 // esp = "";
 // esp+=String(int(temp[0]))+ "."+String(getDecimal(temp[0])); //combining both whole and decimal part in string with a fullstop between them
  esp8266.print(int(mytemp[0]));
  esp8266.print(".");
  esp8266.print(getDecimal(mytemp[0]));
  esp8266.print(" ");
  esp8266.print(int(mytemp[2]));
  esp8266.print(".");
  esp8266.print(getDecimal(mytemp[2]));
  esp8266.print(" ");
  esp8266.print(int(mytemp[9]));
  esp8266.print(".");
  esp8266.print(getDecimal(mytemp[9]));
  esp8266.print(" ");
  esp8266.print(humesp1);
  esp8266.print(" ");
  esp8266.print(int(tempdht));
  esp8266.print(".");
  esp8266.print(getDecimal(tempdht));
  esp8266.print(" ");
  esp8266.print(int(humdht));
  esp8266.print(".");
  esp8266.print(getDecimal(humdht));
  esp8266.print(" ");
  esp8266.print(int(vcc/1000));
  esp8266.print(".");
  esp8266.print(getDecimal(vcc/1000));
  delay(500);
  esp8266.write((char)26);
  delay(9000);
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
long getDecimal(float val)
{
  int intPart = int(val);
  long decPart = 100*(val-intPart); //I am multiplying by 1000 assuming that the foat values will have a maximum of 3 decimal places. 
                                    //Change to match the number of decimal places you need
  if(decPart>0)return(decPart);           //return the decimal part of float number if it is available 
  else if(decPart<0)return((-1)*decPart); //if negative, multiply by -1
  else if(decPart=0)return(0);           //return 0 if decimal part of float number is not available
}
