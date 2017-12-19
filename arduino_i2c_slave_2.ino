#include <Wire.h>

#define SLAVE_ADDRESS 0x08
int i = 0;
int n = 0;
char c;
String data_read = "";
char data_write[9];

String picture = "00000000";

void setup() {
  // put your setup code here, to run once:
  pinMode(13, OUTPUT);
  // start serial for output
  Serial.begin(9600);
  // initialize i2c as slave
  Wire.begin(SLAVE_ADDRESS);

  // define callbacks for i2c communication
  Wire.onReceive(receiveData);
  Wire.onRequest(sendData);

  Serial.println("Ready Slave 08!");
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(100);
}

// callback for received data
void receiveData(int byteCount){
  i = 0;
  n = 0;
  data_read = "";
  while(Wire.available()) {
    c = Wire.read();
    data_read += c;
    i++;
    n++;
  }
  // ACK
  if (n == 1){
    Serial.println("ACK received");
  }
  // Data
  else{
    Serial.print("Data received: ");
    Serial.println(data_read);
  }
}

// callback for sending data
void sendData(){
  String ready_to_receive = "00000001";
  ready_to_receive.toCharArray(data_write,9);
  Serial.print("Data sended: ");
  Serial.println(data_write);
  Wire.write(data_write);
}
