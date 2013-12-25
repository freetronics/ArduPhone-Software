int GsmOnKey    = 19;
int GsmDsrCts   = 21;

void setup() {
  Serial.begin(38400);
  Serial1.begin(115200);
  //Serial1.begin(9600);
  
  pinMode(GsmOnKey, OUTPUT);
  pinMode(GsmDsrCts, OUTPUT);

  // Turn on GSM module
  Serial.print( "Beginning GSM power on: " );
  PowerOnGsmModule();
  Serial.println( "READY" );

  Serial.print( "Waiting for GSM module to settle: " );
  delay(5000);
  Serial.println( "READY" );

  //Serial1.print("AT\r");
  Serial.print( "Switching GSM UART to 9600: " );
  Serial1.print("at+ipr=9600\r"); // Tell the ADH8066 to change to 9600bps
  Serial1.begin(9600);
  Serial.println( "READY" );

  Serial.println( "Serial proxy now running..." );
}

void loop() {
  // read from port 1, send to port 0:
  if (Serial1.available()) {
    int inByte = Serial1.read();
    Serial.write(inByte); 
  }
  
  // read from port 0, echo to port 0, send to port 1:
  if (Serial.available()) {
    int inByte = Serial.read();
    Serial1.write(inByte); 
    Serial.write(inByte);
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
