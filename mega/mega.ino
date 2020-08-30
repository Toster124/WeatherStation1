//------БИБЛИОТЕКИ------
#include <Servo.h>
#include <LowPower.h>
#include <JsonParser.h>
#include <dht.h>
#include "GyverTimer.h"
#include <microDS3231.h>
#include "SD.h"
#include "MHZ19.h"
#include <GyverBME280.h>
#include <iarduino_RF433_Transmitter.h>
//------ПИНЫ------
#define DHT_PIN A0
#define SERVO_PIN 8
#define WIFI_PIN 10
#define SD_PIN 11
#define mhz19_PIN 25
#define RADIO_PIN 24
#define SERVO_SLEEP 5 //rtc.getHours() >
#define SERVO_SLEEP2 23 //rtc.getHours() <
#define MIN_VOLTAGE 3050
//------ОТЛАДКА------
#define DEBUG 0
//------ИНИЦИАЛИЗАЦИЯ------
dht DHT;
GyverBME280 bme;
MicroDS3231 rtc;
iarduino_RF433_Transmitter radioTX(27);
MHZ19 myMHZ19;
JsonParser<28> parser;
JsonParser<10> parser1;
//------ТАЙМЕРЫ------
GTimer_ms myTimer(20000);
GTimer_ms myTimer2(3000);
//------ПЕРЕМЕННЫЕ------
float mytemp[11];
boolean wake_flag, move_arrow, esp_servo_wake_flag = 0, err, mhz_work_now = 0;
byte sleep_count, icon[11], esp_wake_flag = 3, humesp1, angle[8], last_angle[8], weather_predict_percent;
float vcc, k = 0.8, a, b;
String description;
int delta, co2ppm = 0;
unsigned long pressure, aver_pressure, pressure_array[6], time_array[6];
unsigned long sumX, sumY, sumX2, sumXY;
const char com1[] PROGMEM = {"AT+CIPMODE=1"};
const char com2[] PROGMEM = {"AT+CIPSTART=\"""TCP""\",\"""api.openweathermap.org""\",80"};
const char com3[] PROGMEM = {"AT+CIPSEND"};
Servo servo[8];
/*
  Куда несет дым? Куда же бегут все эти люди!
  К чему эти дни? Где нет нас двоих и не будет.
  Куда несет дым? Куда же бегут все эти люди!
  К чему эти дни? Где нет нас двоих и не будет.
*/
void setup() {
  //------BEGIN------
  if (DEBUG == 1) {
    Serial.begin(9600);
  }
  Serial2.begin(9600);
  Serial1.begin(9600);
  bme.setMode(FORCED_MODE);
  bme.begin();
  //------ТАЙМЕРЫ------
  myTimer.setMode(MANUAL);
  myTimer2.setMode(MANUAL);
  //------ПИНЫ------
  pinMode(SERVO_PIN, OUTPUT);
  pinMode(RADIO_PIN, OUTPUT);
  pinMode(WIFI_PIN, OUTPUT);
  pinMode(SD_PIN, OUTPUT);
  pinMode(mhz19_PIN, OUTPUT);
  digitalWrite(mhz19_PIN, 0);
  //------SERVO------
  digitalWrite(SERVO_PIN, 1);
  delay(200);
  servo[0].attach(2);
  servo[1].attach(3);
  servo[2].attach(4);
  servo[3].attach(5);
  servo[4].attach(6);
  servo[5].attach(7);
  servo[6].attach(14);
  servo[7].attach(15);
  delay(200);
  angle[6] = 180;
  angle[7] = 180;
  if (DEBUG == 1) {
    Serial.println(F("Все сервы на 0"));
  }
  //------все сервы на 0------
  for (int i = 0; i <= 7; i++) {
    servo[i].write(0);
    delay(100);
  }
  delay(3000);

  int voltage = readVcc();
  if (DEBUG == 1) {
    Serial.println(F("Все сервы на заряд"));
    Serial.println(voltage);
  }
  //
  voltage = map(voltage, 3000, 4200, 180, 0);
  voltage = constrain(voltage, 0, 180);
  //------все сервы на voltage------
  for (int i = 0; i <= 7; i++) {
    servo[i].write(voltage);
    delay(100);
  }
  delay(3000);

  //------все сервы на 90------
  if (DEBUG == 1) {
    Serial.println(F("Все сервы на 90"));
  }
  for (int i = 0; i <= 7; i++) {
    servo[i].write(90);
    delay(100);
  }
  delay(1000);
  digitalWrite(SERVO_PIN, 0);
  wake_flag = 1;
}

void loop() {
  if (wake_flag) {
    delay(500);
    //------ДАТЧИК DH22------
    DHT.read22(DHT_PIN);
    if (DEBUG == 1) {
      Serial.println(F("DHT"));
      Serial.println(DHT.temperature);
      Serial.println(DHT.humidity);
    }
    delay(500);


    if (esp_wake_flag == 3) {
      wifibegin();
      digitalWrite(RADIO_PIN, 1);
      delay(2000);
        radioTX.begin(1000);
  radioTX.openWritingPipe(5);
    radioTX.write(icon, sizeof(icon));
    delay(200);
    digitalWrite(RADIO_PIN, 0);
    }

    //------ДАТЧИК BME280------
    bme.oneMeasurement();                          // Если используем принудительный мод - необходимо будить датчик для проведения измерения
    while (bme.isMeasuring());
    if (DEBUG == 1) {
      Serial.println(F("Давление"));
      Serial.println(pressureToMmHg(bme.readPressure()));
    }
    weather_predict(); // предсказание погоды
    if (mhz_work_now == 1) {
      mhz19_read(); // датчик mhz19
    }



    //------СРЕДНЯЯ ТЕМПЕРАТУРА И ВЛАЖНОСТЬ------
    angle[4] = map(aver_temp(), 0, 40, 180, 0);
    angle[4] = constrain(angle[4], 0, 180);
    angle[5] = map(aver_hum(), 0, 100, 180, 0);
    angle[5] = constrain(angle[5], 0, 180);

    // дальше такая фишка: если угол несильно изменился с прошлого раза, то нет смысла лишний раз включать серву
    // и тратить энергию/жужжать. Так что находим разницу, и если изменение существенное - то поворачиваем стрелку
    for (int i = 0; i <= 7; i++) {
      if (abs(angle[i] -  last_angle[i]) > 3) move_arrow = 1;
    }
    //------ДВИГАЕМ СЕРВЫ------
    if ((move_arrow) && rtc.getHours() > SERVO_SLEEP && rtc.getHours() < SERVO_SLEEP2) {
      if (DEBUG == 1) {
        Serial.println(F("Крутим сервы"));
      }
      digitalWrite(SERVO_PIN, 1);
      for (int i = 0; i <= 7; i++) {
        servo[i].write(angle[i]);
        if (DEBUG == 1) {
          Serial.println(angle[i]);
        }
        delay(100);
      }
      delay(1000);
      digitalWrite(SERVO_PIN, 0);
    }
    move_arrow = 0;
    esp_wake_flag++;
    vcc = readVcc();
    sdwrite(); //запись на sd карту
    

    for (int i = 0; i <= 7; i++) { //записываем старые углы
      last_angle[i] = angle[i];
    }

    if (readVcc() < MIN_VOLTAGE) {
      digitalWrite(SERVO_PIN, 1);
      delay(1000);
      //------все сервы на 178------
      for (int i = 0; i <= 7; i++) {
        servo[i].write(178);
        delay(100);
      }
      delay(1000);
      digitalWrite(SERVO_PIN, 0);
      delay(100);
      LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);     // вечный сон если акум сел
    }
    if (DEBUG == 1) {
      Serial.println(F("Сладких снов"));
    }
    delay(100);
    wake_flag = 0;
    delay(10);                       // задержка для стабильности
  }


  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);      // спать 8 сек. mode POWER_OFF, АЦП выкл
  sleep_count++;            // +1 к счетчику просыпаний
  if (sleep_count == 48) {
    digitalWrite(mhz19_PIN, 1);
    delay(200);
    digitalWrite(mhz19_PIN, 0);
    delay(200);
    digitalWrite(mhz19_PIN, 1);
    mhz_work_now = 1;
    Serial.println(F("подрубил дачтик"));
    delay(50);
  }
  if (sleep_count >= 70) {  // если время сна превысило 10 минут (75 раз по 8 секунд - подгон = 70)
    wake_flag = 1;          // рарешить выполнение расчета
    sleep_count = 0;        // обнулить счетчик
    delay(2);               // задержка для стабильности
  }
}

void sdwrite() {
  String sendm;
  digitalWrite(SD_PIN, 1);
  /*
    Да простит мне Бог все мои грехи
    Меня не любят дома, для них я еретик
    Меня не любят многие, потому я что впереди
    Всегда на несколько шагов, цел и невредим
  */
  delay(500);
  if (DEBUG == 1) {
    Serial.println(F("запись на sd"));
  }
  sendm = rtc.getHours();
  sendm += ":";
  sendm += rtc.getMinutes();
  sendm += ":";
  sendm += rtc.getSeconds();

  myTimer2.reset();
  myTimer2.start();

  while (!SD.begin(34) && !myTimer2.isReady()) {}
  delay(100);
  File myFile = SD.open("weather3.csv", FILE_WRITE); // постирония
  myFile.print("\"");
  myFile.print(sendm);
  sendm = rtc.getYear();
  sendm += ".";
  sendm += rtc.getMonth();
  sendm += ".";
  sendm += rtc.getDate();
  myFile.print("\",\"");
  myFile.print(sendm);
  myFile.print("\",\"");
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
  myFile.print(bme.readTemperature());
  myFile.print("\",\"");
  myFile.print(DHT.humidity);
  myFile.print("\",\"");
  myFile.print(pressureToMmHg(bme.readPressure()));
  myFile.print("\",\"");
  myFile.print(bme.readHumidity());
  myFile.print("\",\"");
  myFile.print(co2ppm);
  myFile.print("\",\"");
  myFile.print(weather_predict_percent);
  myFile.print("\",\"");
  myFile.print(vcc / 1000);
  myFile.print("\",\"");
  if (!err) {
    myFile.print(icon[2]);
  }
  myFile.print("\",\"");
  if (!err) {
    myFile.print(description);
  }
  else {
    myFile.print("Error");
  }
  myFile.println("\"");

  myFile.close();
  delay(300);
  digitalWrite(SD_PIN, 0);
  if (DEBUG == 1) {
    Serial.println(F("запись на sd окончена"));
  }
  delay(100);
}
void mhz19_read() {
   myMHZ19.begin(Serial1);
    myMHZ19.autoCalibration(false);
  co2ppm = myMHZ19.getCO2();
  if (DEBUG == 1) {
    Serial.println(F("CO2"));
    Serial.println(co2ppm);
  }
  digitalWrite(mhz19_PIN, 0);
  delay(200);
  digitalWrite(mhz19_PIN, 1);
  angle[7] = map(co2ppm, 400, 2000, 180, 0);
  angle[7] = constrain(angle[7], 0, 180);
}

void weather_predict() {
  delay(1000);
  pressure = bme.readPressure();                          // найти текущее давление по среднему арифметическому
  for (byte i = 0; i < 5; i++) {                   // счётчик от 0 до 5 (да, до 5. Так как 4 меньше 5)
    pressure_array[i] = pressure_array[i + 1];     // сдвинуть массив давлений КРОМЕ ПОСЛЕДНЕЙ ЯЧЕЙКИ на шаг назад
  }
  pressure_array[5] = pressure;                    // последний элемент массива теперь - новое давление

  sumX = 0;
  sumY = 0;
  sumX2 = 0;
  sumXY = 0;
  for (int i = 0; i < 6; i++) {                    // для всех элементов массива
    sumX += time_array[i];
    sumY += (long)pressure_array[i];
    sumX2 += time_array[i] * time_array[i];
    sumXY += (long)time_array[i] * pressure_array[i];
  }
  a = 0;
  a = (long)6 * sumXY;             // расчёт коэффициента наклона приямой
  a = a - (long)sumX * sumY;
  a = (float)a / (6 * sumX2 - sumX * sumX);
  // Вопрос: зачем столько раз пересчитывать всё отдельными формулами? Почему нельзя считать одной большой?
  // Ответ: а затем, что ардуинка не хочет считать такие большие числа сразу, и обязательно где-то наё*бывается,
  // выдавая огромное число, от которого всё идёт по пи*зде.
  delta = a * 6;                   // расчёт изменения давления
  delta = constrain(delta, -250, 0);
  angle[6] = map(delta, 0, -250, 180, 0);
  angle[6] = constrain(angle[6], 0, 180);
  weather_predict_percent = map(delta, 0, -250, 0, 100);  // пересчитать в 100%
  weather_predict_percent = constrain(weather_predict_percent, 0, 100);                   // ограничить диапазон
  if (DEBUG == 1) {
    Serial.println(F("Предсказание: "));
    Serial.println(weather_predict_percent);
    Serial.println(F("%"));
  }


}

void wifibegin() {
  byte request_flag = 0, tempesp, temp2, tempday, humesp, v = 0;
  digitalWrite(WIFI_PIN, 1);
  delay(10);
  myTimer.reset();
  myTimer.start();
  while (!Serial2.find("WIFI GOT IP") && !myTimer.isReady()) {
    Serial.println(F("."));
  }
  if (!myTimer.isReady()) {
    myTimer.reset();
    myTimer.start();
  }
  if (DEBUG == 1) {
    Serial.println(F("WIFI GOT IP"));
  }
  delay(10);
  String com;
  for (byte i = 0; i < strlen_P(com1); i++) {
    com += (char)pgm_read_byte(&com1[i]);
  }
  Serial2.println(com);
  while (!Serial2.find("OK") && !myTimer.isReady()) {}
  if (!myTimer.isReady()) {
    myTimer.reset();
    myTimer.start();
  }
  if (DEBUG == 1) {
    Serial.println(F("OK"));
  }
  delay(10);
  com = "";
  for (byte i = 0; i < strlen_P(com2); i++) {
    com += (char)pgm_read_byte(&com2[i]);
  }
  Serial2.println(com);
  while (!Serial2.find("OK") && !myTimer.isReady()) {
    if (Serial2.find("CLOSED")) {
      Serial2.println(com);
    }
    }
  if (!myTimer.isReady()) {
    myTimer.reset();
    myTimer.start();
  }
  if (DEBUG == 1) {
    Serial.println(F("CIPSTART OK"));
  }
  delay(10);
  com = "";
  for (byte i = 0; i < strlen_P(com3); i++) {
    com += (char)pgm_read_byte(&com3[i]);
  }
  Serial2.println(com);
  while (!Serial2.find(">") && !myTimer.isReady()) {}
  if (!myTimer.isReady()) {
    myTimer.reset();
    myTimer.start();
  }

  delay(10);

  Serial2.println("GET /data/2.5/forecast?id=553915&appid=6a4ba421859c9f4166697758b68d889b&mode=json&units=metric&cnt=10");

  while (request_flag < 10 && !myTimer.isReady()) {
    int n = 1; // счетчик символов
    char json12[165] = "{";
    while (!Serial2.find("\"main\":{") && !myTimer.isReady()) {}
    while (!myTimer.isReady()) {
      if (Serial2.available() && !myTimer.isReady()) {
        char c = Serial2.read();
        json12[n] = c;
        if (c == '}') break;
        n++;
      }
    }
    char jsona[85] = "{";
    int p = 1; // счетчик символов
    while (!Serial2.find("\"weather\":[{") && !myTimer.isReady()) {}
    while (!myTimer.isReady()) {
      if (Serial2.available() && !myTimer.isReady()) {
        char c = Serial2.read();
        jsona[p] = c;
        if (c == '}') break;
        p++;

      }
    }
    JsonHashTable hashTable = parser.parseHashTable(json12);
    JsonHashTable hashTable1 = parser1.parseHashTable(jsona);
    mytemp[v] = hashTable.getDouble("temp");
    String icons = hashTable1.getString("icon");
    icon[v] = icons.toInt();
    if (v == 0) {
      humesp1 = hashTable.getLong("humidity");
      description = hashTable1.getString("description");
    }
    v++;
    request_flag++;

  }
  if (myTimer.isReady() || humesp1 < 10) {
    err = 1;
  }
  else {
    err = 0;
  }
  myTimer.stop();
  myTimer.reset();

  digitalWrite(WIFI_PIN, 0);
  if (DEBUG == 1) {
    Serial.println(F("Wifi: "));
    Serial.println(mytemp[0]);
    Serial.println(mytemp[2]);
    Serial.println(mytemp[9]);
    Serial.println(humesp1);
  }
  //-------ПЕРЕВОД В ГРАДУСЫ--------
  angle[0] = map(mytemp[0], -30, 30, 180, 0);
  if (mytemp[0] > 30) {
    angle[0] = 0;
  }
  if (mytemp[0] < -30) {
    angle[0] = 180;
  }

  angle[1] = map(mytemp[2], -30, 30, 180, 0);
  if (mytemp[2] > 30) {
    angle[1] = 0;
  }
  if (mytemp[2] < -30) {
    angle[1] = 180;
  }

  angle[2] = map(mytemp[9], -30, 30, 180, 0);
  if (mytemp[9] > 30) {
    angle[2] = 0;
  }
  if (mytemp[9] < -30) {
    angle[2] = 180;
  }

  angle[3] = map(humesp1, 0, 100, 180, 0);
  angle[3] = constrain(angle[3], 0, 180);

  v = 0;
  request_flag = 0;
  esp_wake_flag = 0;
}
float aver_temp() {
  float temp_lol = DHT.temperature;
  if (DEBUG == 1) {
    Serial.println(F("Температура: "));
    Serial.println(temp_lol);
    Serial.println(bme.readTemperature());
  }
  temp_lol += bme.readTemperature();
  temp_lol /= 2;
  if (DEBUG == 1) {
    Serial.println(temp_lol);
  }
  return temp_lol;
}
float aver_hum() {
  float hum_lol = DHT.humidity;
  if (DEBUG == 1) {
    Serial.println(F("Влажность: "));
    Serial.println(hum_lol);
    Serial.println(bme.readHumidity());
  }
  hum_lol += bme.readHumidity();
  hum_lol /= 2;
  if (DEBUG == 1) {
    Serial.println(hum_lol);
  }
  return hum_lol;
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
