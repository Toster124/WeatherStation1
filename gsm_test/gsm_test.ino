#include <SoftwareSerial.h>
#include <JsonParser.h>
JsonParser<28> parser;
JsonParser<10> parser1;
uint32_t sendTimer;
SoftwareSerial gsm(8, 9);// 8 - RX Arduino (TX SIM800L), 9 - TX Arduino (RX SIM800L)
String esp;
byte flagl = 0, v = 0, hum[10], count = 0, num = 0;
float temp[10];
void setup() {
  // Open serial communications and wait for port to open:
pinMode(A3, OUTPUT);
digitalWrite(A3, 1);
  uart.begin(9600);
gsm.begin(9600);
delay(1000);
gsm.write("AT\r\n");
 while(!gsm.find("OK")){
   if (millis() - sendTimer >= 500) {
    sendTimer = millis();
    gsm.write("AT\r\n");
   // uart.println("1");
   }
  }



  // set the data rate for the SoftwareSerial port
  gsm.write("AT+CREG?\r\n");
  while(!gsm.find("1")){
   if (millis() - sendTimer >= 500) {
    sendTimer = millis();
    gsm.write("AT+CREG?\r\n");
    uart.println("2");
   }

  }



  gsm.write("AT+CSTT=\"internet.beeline.ru\",\"beeline\",\"beeline\"\r\n");
  while(!gsm.find("OK")){uart.println("3");}
  
  gsm.write("AT+CIICR\r\n");
  while(!gsm.find("OK")){uart.println("4");}

  gsm.write("AT+CIFSR\r\n");
   
    while(!gsm.available()){}

  delay(2000);
  gsm.write("ATE1V1+CMEE=2; &W\r\n");
  while(!gsm.find("OK")){}
 
  gsm.write("AT+CIPSTART=\"TCP\",\"api.openweathermap.org\",80\r\n");
  while(!gsm.find("CONNECT OK")){}
 
  gsm.write("AT+CIPSEND=105\r\n");
 
  delay(1000);
  gsm.write("GET /data/2.5/forecast?q=Kaluga,ru&appid=6a4ba421859c9f4166697758b68d889b&mode=json&units=metric&cnt=10\r\n");
  while(!gsm.find("SEND OK")){uart.println("send");}
  
while (flagl < 10) {
      int n = 1; // счетчик символов
        char json[150]="{";
        while (!gsm.find("\"main\":{")){}
        while(true){
        if(gsm.available()) {
          char c = gsm.read();
            json[n]=c;
            if(c=='}') break;
              n++;
            }}
         //   uart.println(json);
            
              char jsona[100]="{";
            int p = 1; // счетчик символов
             while (!gsm.find("\"weather\":[{")){}
           while (true) {
        if(gsm.available()) {
          char c = gsm.read();
            jsona[p]=c;
            if(c=='}') break;
              p++;
            
            }
            }
           uart.println(jsona);
           
 JsonHashTable hashTable = parser.parseHashTable(json);
JsonHashTable hashTable1 = parser1.parseHashTable(jsona);
     temp[v] = hashTable.getDouble("temp");

//      icon[v] = icons.toInt();
      
  hum[v] = hashTable.getLong("humidity");


  v++;
  flagl++;
  
}
gsm.write("AT+CFUN=1,1\r\n");
 drawTemp();

}

void loop() {
 
}
void sendSMS() {
  gsm.write("AT+CMGF=1\r\n");
  while (!gsm.find("OK")) {}
  
  gsm.write("AT+CMGS=\"+79621713511\"\r\n");
  while(!gsm.find(">")) {};
 // esp = "";
 // esp+=String(int(temp[0]))+ "."+String(getDecimal(temp[0])); //combining both whole and decimal part in string with a fullstop between them
  gsm.print(int(temp[0]));
  gsm.print(".");
  gsm.print(getDecimal(temp[0]));
  gsm.print(" ");
  gsm.print(int(hum[0]))+ "."+(getDecimal(hum[0]));
  delay(500);
  gsm.write((char)26);
  
  drawTemp();
  
}
void drawTemp() {
    
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
  long decPart = 1000*(val-intPart); //I am multiplying by 1000 assuming that the foat values will have a maximum of 3 decimal places. 
                                    //Change to match the number of decimal places you need
  if(decPart>0)return(decPart);           //return the decimal part of float number if it is available 
  else if(decPart<0)return((-1)*decPart); //if negative, multiply by -1
  else if(decPart=0)return(00);           //return 0 if decimal part of float number is not available
}
