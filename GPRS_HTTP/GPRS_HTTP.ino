/*
Sketch: GPRS HTTP Test

*/
#include <gprs.h>
#include <SoftwareSerial.h>

char http_cmd[] = "GET /data/2.5/forecast?q=Kaluga,ru&appid=6a4ba421859c9f4166697758b68d889b&mode=json&units=metric&cnt=10";
char buffer[512];

GPRS gprs;

void setup() {
  Serial.begin(9600);
  while(!Serial);
  Serial.println("GPRS - HTTP Connection Test...");  
  gprs.preInit();
  while(0 != gprs.init()) {
     delay(1000);
     Serial.println("init error");
  }  
  while(!gprs.join("internet", "beeline", "beeline")) {  //change "cmnet" to your own APN
      Serial.println("gprs join network error");
      delay(2000);
  }
  // successful DHCP
  Serial.print("IP Address is ");
  Serial.println(gprs.getIPAddress());
  
  Serial.println("Init success, start to connect api.openweathermap.org...");
  
  if(0 == gprs.connectTCP("api.openweathermap.org", 80)) {
      Serial.println("connect api.openweathermap.org success");
  }else{
      Serial.println("connect error");
      while(1);
  }

  Serial.println("waiting to fetch...");
  if(0 == gprs.sendTCPData(http_cmd))
  {      
    gprs.serialDebug();
  }
  
//  gprs.closeTCP();
//  gprs.shutTCP();
//  Serial.println("close");  
}

void loop() {    
    
}
