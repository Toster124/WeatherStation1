//------БИБЛИОТЕКИ------
#include <Servo.h>
#include <JsonParser.h> 
#include "DHT.h"
#include "GyverTimer.h"
#include <STM32SD.h>
#include <libmaple/pwr.h>
#include <libmaple/scb.h>
#include <RTClock.h>

//------ИНИЦИАЛИЗАЦИЯ------
DHT dht(PB1, DHT22);
RTClock rtclock(RTCSEL_LSE);
JsonParser<28> parser;
GTimer_ms myTimer(20000);
GTimer_ms myTimer1(30000);
GTimer_ms myTimer2(3000);
boolean wake_flag, move_arrow, humflag = 1, esp_servo_wake_flag = 0, err;
byte sleep_count, flagl = 0, tempesp, temp2, tempday, humesp, humesp1, esp_wake_flag = 3, hum, temp, last_angle_temp, last_angle_hum, v = 0;
int mytemp[10];
float vcc;   
String esp;
long sleepp;
long readVcc = 3800;


Servo servotempesp;
Servo servotemp2;
Servo servotempday;
Servo servohumesp;
Servo servohum;
Servo servotemp;

void disableClocks() 
{
    rcc_clk_disable(RCC_ADC1);
    rcc_clk_disable(RCC_ADC2);
    rcc_clk_disable(RCC_ADC3);
    rcc_clk_disable(RCC_AFIO);
    rcc_clk_disable(RCC_BKP);
    rcc_clk_disable(RCC_CRC);
    rcc_clk_disable(RCC_DAC);
    rcc_clk_disable(RCC_DMA1);
    rcc_clk_disable(RCC_DMA2);
    rcc_clk_disable(RCC_FLITF);
    rcc_clk_disable(RCC_FSMC);
    //rcc_clk_disable(RCC_GPIOA); // needed by N5110
    rcc_clk_disable(RCC_GPIOB);
    rcc_clk_disable(RCC_GPIOC);
    rcc_clk_disable(RCC_GPIOD);
    rcc_clk_disable(RCC_GPIOE);
    rcc_clk_disable(RCC_GPIOF);
    rcc_clk_disable(RCC_GPIOG);
    rcc_clk_disable(RCC_I2C1);
    rcc_clk_disable(RCC_I2C2);
    //rcc_clk_disable(RCC_PWR); // needed by standby
    rcc_clk_disable(RCC_SDIO);
    //rcc_clk_disable(RCC_SPI1); // needed by N5110
    rcc_clk_disable(RCC_SPI2);
    rcc_clk_disable(RCC_SPI3);
    rcc_clk_disable(RCC_SRAM);
    rcc_clk_disable(RCC_TIMER1);
    rcc_clk_disable(RCC_TIMER2);
    rcc_clk_disable(RCC_TIMER3);
    rcc_clk_disable(RCC_TIMER4);
    rcc_clk_disable(RCC_TIMER5);
    rcc_clk_disable(RCC_TIMER6);
    rcc_clk_disable(RCC_TIMER7);
    rcc_clk_disable(RCC_TIMER8);
    rcc_clk_disable(RCC_TIMER9);
    rcc_clk_disable(RCC_TIMER10);
    rcc_clk_disable(RCC_TIMER11);
    rcc_clk_disable(RCC_TIMER12);
    rcc_clk_disable(RCC_TIMER13);
    rcc_clk_disable(RCC_TIMER14);
    rcc_clk_disable(RCC_USART1);
    rcc_clk_disable(RCC_USART2);
    rcc_clk_disable(RCC_USART3);
    rcc_clk_disable(RCC_UART4);
    rcc_clk_disable(RCC_UART5);
    rcc_clk_disable(RCC_USB);
}




static void int_fun() {};

// standby=true for deep sleep
void sleepMode(bool standby, uint8_t seconds)
{ 
  rtclock.createAlarm(&int_fun, rtclock.getTime()+seconds);  // wakeup int
  PWR_BASE->CR &= PWR_CR_LPDS | PWR_CR_PDDS | PWR_CR_CWUF;
  PWR_BASE->CR |= PWR_CR_CWUF;
  PWR_BASE->CR |= PWR_CSR_EWUP;
  SCB_BASE->SCR |= SCB_SCR_SLEEPDEEP;
  if(standby) {
    PWR_BASE->CR |= PWR_CR_PDDS;
    PWR_BASE->CR &= ~PWR_CR_LPDS;
  } else {
    adc_disable(ADC1);
    adc_disable(ADC2);
    PWR_BASE->CR &= ~PWR_CR_PDDS;
    PWR_BASE->CR |= PWR_CR_LPDS;
  }
  asm("    wfi");
  SCB_BASE->SCR &= ~SCB_SCR_SLEEPDEEP;
}

void setup() {
Serial1.begin(9600); 

myTimer.setMode(MANUAL);
myTimer1.setMode(MANUAL);
myTimer2.setMode(MANUAL);
myTimer.stop();
myTimer.reset();
myTimer1.stop();
myTimer1.reset();


pinMode(PB14, OUTPUT);
pinMode(PB8, OUTPUT);
digitalWrite(PB14, 1);

delay(200);

servotempesp.attach(PB15);
servotemp2.attach(PA8);
servotempday.attach(PA8);
servohumesp.attach(PA2);
servohum.attach(PB0);
servotemp.attach(PA3);

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

int voltage = readVcc;
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
digitalWrite(PB14, 0);
wake_flag = 1;
}

void loop() {
if (wake_flag) {
      delay(500);
 //------ДАТЧИК DH22------
 
 delay(500);
  temp = map(dht.readTemperature(), 0, 40, 180, 0);
  temp = constrain(temp, 0, 180);
  hum = map(dht.readHumidity(), 0, 100, 180, 0);
  hum = constrain(hum, 0, 180);
  delay(500);
   if (esp_wake_flag == 3) {
    digitalWrite(PB8, 1);
    delay(10); 
    myTimer1.reset();
        myTimer1.start();
    while(!Serial1.find("WIFI GOT IP") && !myTimer1.isReady()) {}
    if (!myTimer1.isReady()) {
      myTimer.reset();
        myTimer.start();
    }
    delay(10); 
      Serial1.println("AT+CIPMODE=1");
      while(!Serial1.find("OK") && !myTimer.isReady()) {}
      if (!myTimer.isReady()) {
      myTimer.reset();
        myTimer.start();
    }
    delay(10); 
        Serial1.println("AT+CIPSTART=\"""TCP""\",\"""api.openweathermap.org""\",80");
        while(!Serial1.find("OK") && !myTimer.isReady()) {}
      if (!myTimer.isReady()) {
      myTimer.reset();
        myTimer.start();
    }
    delay(10); 
        Serial1.println("AT+CIPSEND");
        while(!Serial1.find(">") && !myTimer.isReady()) {}
      if (!myTimer.isReady()) {
      myTimer.reset();
        myTimer.start();
    }
    delay(10); 
        Serial1.println("GET /data/2.5/forecast?id=553915&appid=6a4ba421859c9f4166697758b68d889b&mode=json&units=metric&cnt=10");
       
      while (flagl < 10 && !myTimer.isReady()) {
          unsigned int i = 0; //счетчик времени
      int n = 1; // счетчик символов
        char json[150]="{";
        while (!Serial1.find("\"main\":{") && !myTimer.isReady()){}
        while (i<60000 && !myTimer.isReady()) {
        if(Serial1.available() && !myTimer.isReady()) {
          char c = Serial1.read();
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

  digitalWrite(PB8, 0);
  
   
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
  digitalWrite(PB14, 1);
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
  digitalWrite(PB14, 0);
  move_arrow = 0;
  esp_servo_wake_flag = 0;
}
  v = 0;
  flagl = 0;
  esp_wake_flag++;
    humflag = 1;
    vcc = readVcc;
    
 delay(500);
    myTimer2.reset();
    myTimer2.start();
    while(!SD.begin(PA0) && !myTimer2.isReady()) {}
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
        myFile.print(dht.readTemperature());
        myFile.print("\",\"");
        myFile.print(dht.readHumidity());
        myFile.print("\",\"");
        myFile.print(vcc/1000);
        myFile.println("\"");
       
    while(!myTimer2.isReady()) {} 
   
    myFile.close();
    delay(100);
    
    
    delay(100);
    sleepp = millis();
    wake_flag = 0;
    delay(10);                       // задержка для стабильности
  }

  

if (millis() - sleepp >= 601000) {
  wake_flag = 1;
}



}

