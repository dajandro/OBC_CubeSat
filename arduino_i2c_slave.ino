#include <Wire.h>

#define SLAVE_ADDRESS 0x13
int i = 0;
int n = 0;
char c;
String data_read = "";
char data_write[17]; // 2 bytes

String c1 = "00000000"; // state_of_charge
String c2 = "00000001"; // voltaje_data_bus
String c3 = "00000010"; // temperatura_baterias

String c1b = "10000000"; // state_of_charge_backup
String c2b = "10000001"; // voltaje_data_bus_backup
String c3b = "10000010"; // temperatura_baterias_backup

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

  Serial.println("Ready Slave 19!");
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
    if (i<8){
      data_read += c;
    }
    else
    {
      Serial.println("Data exceed bounds");
    }
    i++;
    n++;
  }
  // ACK
  if (n == 1){
    Serial.println("ACK received");
  }
  // Data
  else{
    if (data_read == c1 || data_read == c2 || data_read == c3){
      Serial.print("Data received from RP1: ");
    }
    if (data_read == c1b || data_read == c2b || data_read == c3b){
      Serial.print("Data received  from RP2: ");
    }
    //Serial.print("Data received: ");
    Serial.println(data_read);
  }
}

// callback for sending data
void sendData(){
  data_write[0] = ' ';
  data_write[1] = ' ';
  if (data_read == c1 || data_read == c1b){
    for(int i=0; i<16; i++){
      int rnd = random(0,2);
      char b[2];
      String str;
      str = String(rnd);
      str.toCharArray(b,2);
      data_write[i] = b[0];
    }
  }
  else if (data_read == c2 || data_read == c2b){
    for(int i=0; i<13; i++){
      int rnd = random(0,2);
      char b[2];
      String str;
      str = String(rnd);
      str.toCharArray(b,2);
      data_write[i] = b[0];
    }
    char b[2];
    String str;
    str = String(0);
    str.toCharArray(b,2);
    data_write[13] = b[0];
    data_write[14] = b[0];
    data_write[15] = b[0];
  }
  else if (data_read == c3 || data_read == c3b){
    for(int i=0; i<12; i++){
      int rnd = random(0,2);
      char b[2];
      String str;
      str = String(rnd);
      str.toCharArray(b,2);
      data_write[i] = b[0];
    }
    char b[2];
    String str;
    str = String(0);
    str.toCharArray(b,2);
    data_write[12] = b[0];
    data_write[13] = b[0];
    data_write[14] = b[0];
    data_write[15] = b[0];
  }
  Serial.print("Data sended: ");
  Serial.println(data_write);
  Wire.write(data_write);
}
