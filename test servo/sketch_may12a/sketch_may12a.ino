//---------------------------------------------
//{"temp1":25,"hum1":90,"tempesp1"10,"temp21":-10,"tempday1":0,"humesp1":60}



#define servo_Vcc 2
#include <ServoSmooth.h>
ServoSmooth servotempesp;
ServoSmooth servotemp2;
ServoSmooth servotempday;
ServoSmooth servohumesp;
ServoSmooth servohum;
ServoSmooth servotemp;
boolean flag;
int tempesp;
int temp2;
int temp1;
int hum1;
int temp;
int hum;
int tempday;
int humesp;
int tempesp1;
int temp21;
int tempday1;
int humesp1;
String esp;
void setup() {
pinMode(servo_Vcc, OUTPUT);
digitalWrite(servo_Vcc, 1);
Serial.begin(9600);
servotempesp.attach(5, 500, 2400);
servotemp2.attach(4, 500, 2400);
servotempday.attach(3, 500, 2400);
servohumesp.attach(8, 500, 2400);
servohum.attach(7, 500, 2400);
servotemp.attach(6, 500, 2400);

servotempesp.setSpeed(20);   // ограничить скорость
servotempesp.setAccel(0.2);
servotemp2.setSpeed(20);   // ограничить скорость
servotemp2.setAccel(0.2);
servotempday.setSpeed(20);   // ограничить скорость
servotempday.setAccel(0.2);
servohumesp.setSpeed(20);   // ограничить скорость
servohumesp.setAccel(0.2);
servohum.setSpeed(40);   // ограничить скорость
servohum.setAccel(0.2);
servotemp.setSpeed(20);   // ограничить скорость
servotemp.setAccel(0.2);







delay(200);
  // put your setup code here, to run once:
 
}

void loop() {
 
  if (Serial.available()) {
    esp = Serial.readString();
    flag = 1;
  }
  if (flag == 1) {
    
   
   if (esp.indexOf("tempesp") != -1){

tempesp1 = getFloatFromString(esp);
tempesp = map(tempesp1, -30, 30, 180, 0);
tempesp = constrain(tempesp, 0, 180);

servotempesp.setTargetDeg(tempesp);
while (!servotempesp.tickManual()) {
  
}

  }
   if (esp.indexOf("temph") != -1){

temp21 = getFloatFromString(esp);
temp2 = map(temp21, -30, 30, 180, 0);
temp2 = constrain(temp2, 0, 180);

servotemp2.setTargetDeg(temp2);
while (!servotempesp.tickManual()) {
  
}


  }
   if (esp.indexOf("tempday") != -1){

tempday1 = getFloatFromString(esp);
tempday = map(tempday1, -30, 30, 180, 0);
tempday = constrain(tempday, 0, 180);

servotempday.setTargetDeg(tempday);
servotempday.tick();


  }
   if (esp.indexOf("humesp") != -1){

humesp1 = getFloatFromString(esp);
humesp = map(humesp1, 0, 100, 180, 0);
humesp = constrain(humesp, 0, 180);

servohumesp.setTargetDeg(humesp);
servohumesp.tick();


  }
  if (esp.indexOf("hum") != -1 && esp.indexOf("humesp") == -1){
hum1 = getFloatFromString(esp);
hum = map(hum1, 0, 100, 180, 0);
hum = constrain(hum, 0, 180);
Serial.println(hum);
servohum.setTargetDeg(hum);
while (!servohum.tickManual()) {
  
}


  }
  if (esp.indexOf("temp") != -1 && esp.indexOf("tempday") == -1 && esp.indexOf("temph") == -1 && esp.indexOf("tempesp") == -1){
temp1 = getFloatFromString(esp);
temp = map(temp1, 0, 40, 180, 0);
temp = constrain(temp, 0, 180);

servotemp.setTargetDeg(temp);
servotemp.tick();


  }
  flag = 0;
  }

}

float getFloatFromString(String str) {            // Функция извлечения цифр из сообщения - для парсинга баланса из USSD-запроса
  bool   flag     = false;
  String result   = "";
  str.replace(",", ".");                          // Если в качестве разделителя десятичных используется запятая - меняем её на точку.

  for (int i = 0; i < str.length(); i++) {
    if (isDigit(str[i]) || (str[i] == (char)46 && flag)) {        // Если начинается группа цифр (при этом, на точку без цифр не обращаем внимания),
      if (result == "" && i > 0 && (String)str[i - 1] == "-") {   // Нельзя забывать, что баланс может быть отрицательным
        result += "-";                            // Добавляем знак в начале
      }
      result += str[i];                           // начинаем собирать их вместе
      if (!flag) flag = true;                     // Выставляем флаг, который указывает на то, что сборка числа началась.
    }
    else  {                                       // Если цифры закончились и флаг говорит о том, что сборка уже была,
      if (str[i] != (char)32) {                   // Если порядок числа отделен пробелом - игнорируем его, иначе...
        if (flag) break;                          // ...считаем, что все.
      }
    }
  }

  return result.toFloat();                        // Возвращаем полученное число.
}
