
String esp;
void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }



  // set the data rate for the SoftwareSerial port
  Serial2.begin(9600);
  pinMode(10, OUTPUT);
  digitalWrite(10, 1);
  //Serial2.write("AT\r\n");


}

void loop() {
  if (Serial2.available()) {
    Serial.write(Serial2.read());
  }
  if (Serial.available()) {
    Serial2.write(Serial.read());
  }


}
