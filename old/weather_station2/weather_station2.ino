//-----------------------НАСТРОЙКИ---------------------
#define servo_Vcc 2
#define esp_Vcc 9
#define DHTPIN A0
#define DHTTYPE DHT22
#define debug 1
//------БИБЛИОТЕКИ------
#include <Servo.h>
#include <LowPower.h>
#include <SoftwareSerial.h>
#include <JsonParser.h> 
#include "DHT.h"
#include "GyverTimer.h"
//#include <SPI.h>
//#include <SD.h>
//------ИНИЦИАЛИЗАЦИЯ------
DHT dht(DHTPIN, DHTTYPE);
JsonParser<32> parser;
//File myFile;
//Sd2Card card;
SoftwareSerial esp8266 (10, 11);
GTimer_ms myTimer(20000);
GTimer_ms myTimer1(30000);
boolean wake_flag, move_arrow;
boolean humflag = 1;
boolean esp_servo_wake_flag = 0;
boolean card_ye = 0;
int sleep_count, angle, delta, last_angle = 90;
int mytemp[10];
int flagl = 0;
int tempesp;
int temp2;
int tempday;
int humesp;
int tempesp1;
int temp21;
int tempday1;
int humesp1;
int esp_wake_flag = 3;
int hum;
int temp;
int last_angle_temp;
int last_angle_hum;
const int chipSelect = 3;
float my_vcc_const = 1.080;    // константа вольтметра
byte v = 0;
float vcc;
char c;
String lol = "TCP";
String lol2 = "api.openweathermap.org";
String nameOfCity = "Kaluga"; 
String apiKey = "6a4ba421859c9f4166697758b68d889b"; 
String esp;
String cmd = "GET /data/2.5/forecast?q=" + nameOfCity + "&appid=" + apiKey + "&mode=json&units=metric&cnt=10";
String cmddate = "nginx";
String cipstart = "AT+CIPSTART=\""+lol+"\",\""+lol2+"\",80";
String time1;

Servo servotempesp;
Servo servotemp2;
Servo servotempday;
Servo servohumesp;
Servo servohum;
Servo servotemp;


void setup() {
Serial.begin(9600);
esp8266.begin(9600); 
dht.begin();
//SD.begin(chipSelect);
//if (!card.init(SPI_HALF_SPEED, chipSelect)) {
// if (debug) {
//   Serial.println("sd карта не инициализирована");
// }
// } else {
//   Serial.println("sd карта инициализирована");
//   card_ye = 1;
// }
  
myTimer.setMode(MANUAL);
myTimer1.setMode(MANUAL);
myTimer.stop();
myTimer.reset();
myTimer1.stop();
myTimer1.reset();

pinMode(servo_Vcc, OUTPUT);
pinMode(esp_Vcc, OUTPUT);
digitalWrite(servo_Vcc, 1);

delay(200);

servotempesp.attach(5);
servotemp2.attach(4);
servotempday.attach(3);
servohumesp.attach(8);
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
if (debug) {
  Serial.println(readVcc());
}
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
digitalWrite(servo_Vcc, 0);
Serial.println("off servo");
wake_flag = 1;
}

void loop() {
if (wake_flag) {
      delay(500);
      Serial.println("loop");
 //------ДАТЧИК DH22------
  temp = map(dht.readTemperature(), 0, 40, 180, 0);
  temp = constrain(temp, 0, 180);
  hum = map(dht.readHumidity(), 0, 100, 180, 0);
  hum = constrain(hum, 0, 180);
  if (debug) {
Serial.println(dht.readTemperature());
Serial.println(dht.readHumidity());
  }
   if (esp_wake_flag == 3) {
    digitalWrite(esp_Vcc, 1);
    myTimer1.reset();
        myTimer1.start();
    while(!esp8266.find("WIFI GOT IP") && !myTimer1.isReady()) {}
    if (!myTimer1.isReady()) {
      myTimer.reset();
        myTimer.start();
    }
    Serial.println("1");
      esp8266.println("AT+CIPMODE=1");
      while(!esp8266.find("OK") && !myTimer.isReady()) {}
      if (!myTimer.isReady()) {
      myTimer.reset();
        myTimer.start();
    }
    Serial.println("2");
        esp8266.println(cipstart);
        while(!esp8266.find("OK") && !myTimer.isReady()) {}
      if (!myTimer.isReady()) {
      myTimer.reset();
        myTimer.start();
    }
    Serial.println("3");
        esp8266.println("AT+CIPSEND");
        while(!esp8266.find(">") && !myTimer.isReady()) {}
      if (!myTimer.isReady()) {
      myTimer.reset();
        myTimer.start();
        
    }
    Serial.println("4");
        esp8266.println(cmd);
       
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
            if (debug) {
Serial.println(json);
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

  myTimer.stop();
  myTimer.reset();
  myTimer1.stop();
  myTimer1.reset();

  digitalWrite(esp_Vcc, 0);
  v = 0;
  flagl = 0;
    
    

    tempesp1 = mytemp[0];
    temp21 = mytemp[2];
    tempday1 = mytemp[9];
    if (debug) {
Serial.println(humesp1);
Serial.println(tempesp1);
Serial.println(temp21);
Serial.println(tempday1);
    }
  tempesp = map(tempesp1, -30, 30, 180, 0);
    tempesp = constrain(tempesp, 0, 180);

    temp2 = map(temp21, -30, 30, 180, 0);
    temp2 = constrain(temp2, 0, 180);

    tempday = map(tempday1, -30, 30, 180, 0);
    tempday = constrain(tempday, 0, 180);

    humesp = map(humesp1, 0, 100, 180, 0);
    humesp = constrain(humesp, 0, 180);
delay(1000);
    
    digitalWrite(esp_Vcc, 1);
    myTimer1.reset();
        myTimer1.start();
while(!esp8266.find("WIFI GOT IP") && !myTimer1.isReady()) {}
    if (!myTimer1.isReady()) {
      myTimer.reset();
        myTimer.start();
    }
      esp8266.println("AT+CIPMODE=1");
      while(!esp8266.find("OK") && !myTimer.isReady()) {}
      if (!myTimer.isReady()) {
      myTimer.reset();
        myTimer.start();
    }
        esp8266.println(cipstart);
        while(!esp8266.find("OK") && !myTimer.isReady()) {}
      if (!myTimer.isReady()) {
      myTimer.reset();
        myTimer.start();
    }
        esp8266.println("AT+CIPSEND");
        while(!esp8266.find(">") && !myTimer.isReady()) {}
      if (!myTimer.isReady()) {
      myTimer.reset();
        myTimer.start();
    }
esp8266.println(cmddate); 
unsigned int i = 0; //счетчик времени
  int n = 1; // счетчик символов
  char json[150]="Server:";
  while (!esp8266.find("openresty")){}
  while (i<60000) {
    if(esp8266.available()) {
      c = esp8266.read();
      json[n]=c;
      if(c=="GMT") break;
      n++;
      i=0;
    }
    i++;
  }
  digitalWrite(esp_Vcc, 0);
  if (debug) {
Serial.println(json);
  }
        myTimer.stop();
  myTimer.reset();
  myTimer1.stop();
  myTimer1.reset();
String str(json);
String date = str.substring(14, 25);
String hours = str.substring(26, 28);
String min_sec = str.substring(28, 34);
int hours1 = hours.toInt();
hours1 = hours1 + 3;
hours = String(hours1, DEC);
time1 = hours + min_sec;
  digitalWrite(esp_Vcc, 0);
    if (debug) {
        Serial.println(date);
Serial.println(time1);
    }
    esp_wake_flag = 0;
  esp_servo_wake_flag = 1;
 }
 
    // дальше такая фишка: если угол несильно изменился с прошлого раза, то нет смысла лишний раз включать серву
    // и тратить энергию/жужжать. Так что находим разницу, и если изменение существенное - то поворачиваем стрелку    
    if (abs(temp - last_angle_temp) > 3 || abs(hum - last_angle_hum) > 3) move_arrow = 1;
    
if (move_arrow || esp_servo_wake_flag) {
  last_angle_temp = temp;
  last_angle_hum = hum;
  digitalWrite(servo_Vcc, 1);
  delay(1000);
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
  delay(3000);
  digitalWrite(servo_Vcc, 0);
  move_arrow = 0;
  esp_servo_wake_flag = 0;
}
//vcc = readVcc();

//  if (card_ye){
// myFile = SD.open("weather.csv", FILE_WRITE);
//    if (myFile) {
//      if (debug) {
//        Serial.print("Writing to weather.csv...");
//      }
//        myFile.print("\"");
//        myFile.print(time1);
//        myFile.print("\",\"");
//        myFile.print(tempesp1);
//        myFile.print("\",\"");
//        myFile.print(temp21);
//        myFile.print("\",\"");
//        myFile.print(tempday1);
//        myFile.print("\",\"");
//        myFile.print(humesp1);
//        myFile.print("\",\"");
//        myFile.print(dht.readTemperature());
//        myFile.print("\",\"");
//        myFile.print(dht.readHumidity());
//        myFile.print("\",\"");
//        myFile.print(vcc/1000);
//        myFile.println("\"");
//        if (debug) {
//        Serial.println("done.");}
//    } else {
//        // if the file didn't open, print an error:
//        if (debug) {
//        Serial.println("error opening log.csv");}
//    }
//    myFile.close();
    
//  }
  v = 0;
  flagl = 0;
  esp_wake_flag++;
    humflag = 1;
    
    if (readVcc() < 3050) {
  digitalWrite(servo_Vcc, 1);
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
  digitalWrite(servo_Vcc, 0);
  delay(100);
LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);     // вечный сон если акум сел
    }
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

  result = my_vcc_const * 1023 * 1000 / result; // расчёт реального VCC
  return result; // возвращает VCC
}
