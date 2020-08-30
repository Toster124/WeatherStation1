#include "SD.h"
#include <SoftwareSerial.h>
#include "GyverTimer.h"
#include <JsonParser.h> 
#include "MHZ19.h"  
SoftwareSerial ard (8, 9);
SoftwareSerial mySerial(3, 2);
GTimer_ms myTimer(3000);
GTimer_ms myTimer1(120000);
GTimer_ms myTimer2(500);
JsonParser<28> parser;
MHZ19 myMHZ19;
float pars[5];
String msg;
boolean err;
char var[200];
int n = 0;
byte humesp;
void setup() {
ard.begin(9600);
//Serial.begin(9600);
mySerial.begin(9600);
myMHZ19.begin(mySerial);                                // *Important, Pass your Stream reference 
myMHZ19.autoCalibration(false);
ard.println("готов");
myTimer1.reset();
        myTimer1.start();
ard.listen();
 while (!ard.available() && !myTimer1.isReady()) {};
 //char json[150]="{";
myTimer2.reset();
        myTimer2.start();
// int n = 1; // счетчик символов

while (!myTimer2.isReady()) {
  if (ard.available()) {
var[n] = (ard.read());
//Serial.print(var[n]);
n++;


  }
    /*      char c = ard.read();
          Serial.print(c);
            json[n]=c;
            if(c=='}') break;
            n++;
            */
}
//Serial.println(var);
JsonHashTable hashTable = parser.parseHashTable(var);

 //    pars[0] = hashTable.getLong("tempesp");
  //   pars[1] = hashTable.getLong("temph");
     
     pars[2] = hashTable.getDouble("tempday");
     humesp = hashTable.getLong("humesp");
     pars[3] = hashTable.getDouble("temp");
     pars[4] = hashTable.getDouble("hum");
     pars[5] = hashTable.getDouble("vcc");
     err = hashTable.getLong("err");
  pars[0] = hashTable.getDouble("tempesp");
     pars[1] = hashTable.getDouble("temph");  
     /*
Serial.println(pars[0]);
Serial.println(pars[1]);
Serial.println(pars[2]);
Serial.println(humesp);
Serial.println(pars[3]);
Serial.println(pars[4]);
Serial.println(pars[5]);
*/
//Serial.println(hashTable.getLong("tempesp"));

String date = hashTable.getString("date");
String times = String(hashTable.getLong("hour"), DEC);;
times += ":";
times += String(hashTable.getLong("min"), DEC);
times += ":";
times += String(hashTable.getLong("sec"), DEC);
//byte icon = hashTable.getLong("icon");
String description = hashTable.getString("description");
 
//Serial.println(date);
//Serial.println(times);
  //  Serial.println(description);
        int CO2 = myMHZ19.getCO2();
myTimer.reset();
    myTimer.start();
    while(!SD.begin(6) && !myTimer.isReady()) {}
    delay(100);
    File myFile = SD.open("weather2.csv", FILE_WRITE); // постирония
    myTimer.reset();
    myTimer.start();  
        myFile.print("\"");
        myFile.print(times);
        myFile.print("\",\"");
        myFile.print(date);
        myFile.print("\",\"");
        if (!err) {
        myFile.print(pars[0]);
        }
        myFile.print("\",\"");
         if (!err) {
        myFile.print(pars[1]);
         }
        myFile.print("\",\"");
         if (!err) {
        myFile.print(pars[2]);
         }
         myFile.print("\",\"");
        if (!err) {
        myFile.print(humesp);
        }
        myFile.print("\",\"");
        myFile.print(pars[3]);
        myFile.print("\",\"");
        myFile.print(pars[4]);
        myFile.print("\",\"");
        myFile.print(pars[5]);
        myFile.print("\",\"");
        myFile.print(description);
        myFile.print("\",\"");
        myFile.print(CO2);
        myFile.println("\"");
       
    while(!myTimer.isReady()) {} 
   
    myFile.close();
    delay(300);

    ard.println("1");

}

void loop() {
}
