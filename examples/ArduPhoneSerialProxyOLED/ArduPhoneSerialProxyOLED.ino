#include <SPI.h>
#include <SD.h>
#include <FTOLED.h>
#include <fonts/SystemFont5x7.h>

const byte pin_cs = 4;
const byte pin_dc = 7;
const byte pin_reset = 5;

OLED oled(pin_cs, pin_dc, pin_reset);
OLED_TextBox box(oled);

int GsmReady    = 18;
int GsmOnKey    = 19;
int GsmNetOk    = 20;
int GsmDsrCts   = 21;
int GsmSense1v8 = 22;

void setup() {
  pinMode(GsmReady, INPUT);
  pinMode(GsmOnKey, OUTPUT);
  pinMode(GsmNetOk, INPUT);
  pinMode(GsmDsrCts, OUTPUT);
  pinMode(GsmSense1v8, INPUT);
  
  pinMode(GsmSense1v8, OUTPUT);
  digitalWrite(GsmSense1v8, HIGH); // turn on internal pull-up resistor
  
  oled.begin();
  oled.setOrientation(ROTATE_180);
  oled.selectFont(SystemFont5x7);
  Serial.begin(9600);
  //Serial1.begin(115200);
  Serial1.begin(9600);
  Serial.print("Ready: ");
  //Serial.println(digitalRead(GsmReady));
  
  Serial.println("Beginning GSM power on");
  box.println("Beginning GSM power on");
  PowerOnGsmModule();
  Serial.println("READY");
  box.println("READY");
  
  //Serial.println("Waiting 20");
  //box.println("Waiting 20");
  //delay(20000);
  //Serial.println("Sending");
  //SendSms();
  //Serial.println("Sent");
  
  /*
  Serial.println("Changing baud rate to 9600");
  Serial1.println("AT+IPR=9600");
  Serial.println("Done");
  Serial1.begin(9600);
  
  Serial1.print("AT\r");
  Serial.print("Ready");
  box.println("Ready");
  */
  //Serial.println(digitalRead(GsmReady));
}

void loop() {
  //Serial.print("Ready: ");
  //Serial.println(digitalRead(GsmReady));

  // read from port 1, send to port 0:
  if (Serial1.available()) {
    int inByte = Serial1.read();
    Serial.write(inByte);
    box.setForegroundColour(GREEN);
    if(inByte == 13)
    {
      box.println("");
    } else {
      box.write(inByte);
    }
  }
  
  // read from port 0, echo to port 0, send to port 1:
  if (Serial.available()) {
    int inByte = Serial.read();
    Serial1.write(inByte); 
    Serial.write(inByte);
    box.setForegroundColour(YELLOW);
    if(inByte == 13)
    {
      box.println("");
    } else {
      box.write(inByte);
    }
  }
}


void PowerOnGsmModule ()
{
  // Begin t1
  digitalWrite( GsmOnKey, HIGH );
  digitalWrite( GsmDsrCts, HIGH );
  delay(200);
  // Begin t2
  digitalWrite( GsmOnKey, LOW );
  digitalWrite( GsmDsrCts, HIGH );
  delay(2000);
  // Begin t3
  digitalWrite( GsmOnKey, HIGH );
  digitalWrite( GsmDsrCts, HIGH );
  delay(2500);
  // Begin t4
  digitalWrite( GsmOnKey, LOW );
  digitalWrite( GsmDsrCts, LOW );
  delay(100);
}



void OldPowerOnGsmModule()
{
  delay(1000);
  digitalWrite(GsmOnKey, LOW);
  pinMode(GsmOnKey, OUTPUT);
  delay(12000);
  pinMode(GsmOnKey, INPUT);
  delay(3000);
}

void SendSms()
{
  Serial1.println("AT+CMGS=\"0410413665\"");
  delay(1000);
  Serial1.print("Test from sketch\x1A");
 //Serial1.print( TERMINATION );
}
