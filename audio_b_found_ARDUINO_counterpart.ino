#include <Wire.h>

#define SLAVE_ADDRESS 0x04
int n = 1;
int number = 0;
int mic1;
int mic2;
int mic3;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); // start serial for output
  // initialize i2c as slave
  Wire.begin(SLAVE_ADDRESS);

  // define callbacks for i2c communication
  Wire.onReceive(receiveData);
  Wire.onRequest(sendData);
}

void loop() {
  // put your main code here, to run repeatedly:
  mic1 = analogRead(A0)-20; // dunno why needs -20
  mic2 = analogRead(A1);
  mic3 = analogRead(A2);
  delay(1); // for stability
}

void receiveData(int byteCount) {
  while (Wire.available()) {
    number = Wire.read();
    //Serial.print("number:");
    //Serial.println((char)number);
    if( (char)number == 'A' ){
      n = 1;
    }
    else{
      Serial.write((char)number);
    }
  }
}

void sendData() {
  if (n == 1) {
    if (mic1 > 255) {
      mic1 = 255;
    }
    if (mic1 < 0) {
      mic1 = 0;
    }
    Wire.write(mic1);
    n = 2;
  }
  else if (n == 2) {
    if (mic2 > 255) {
      mic2 = 255;
    }
    if (mic2 < 0) {
      mic2 = 0;
    }
    Wire.write(mic2);
    n = 3;
  }
  else if (n == 3) {
     if (mic3 > 255) {
      mic3 = 255;
    }
    if (mic3 < 0) {
      mic3 = 0;
    }
    Wire.write(mic3);
    n = 1;
  }

}
