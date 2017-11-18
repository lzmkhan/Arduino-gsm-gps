// TODO: implement a clearing mechanism for SIM Memory of Messages. when your sim storage is full,
// your GET will not work. 

// TODO: Change the _SS_MAX_RX_BUFF to 256 to increase RX buffer.
// in SoftwareSerial.h
// Connect GPS Tx to 0
// VCC to 3.3v
// GND to GND
// Connect GSM Rx to 10
// Tx to 9
// GND to GND
// Before uploading, disconnect the TX of GPS


#include <TinyGPS.h>
#include <SoftwareSerial.h>


// Initialize software serial to communicate with GSM module
SoftwareSerial mySerial(9, 10);

//Initialize GPS to communicate with GPS module
TinyGPS gps;

// Variable to hold latitude data to be sent to server.
float latitude;

// Variable to hold longitude data to be sent to server.
float longitude;

// Configure your Device ID here, This will be root of this Device info
int DEVICEID = 0;

// Prototype for gpsdump method
void gpsdump(TinyGPS &gps);

// Prototype for printFloat method
String printFloat(double f, int digits = 2);

/* Function name: setup

   Parameter name: None
   P.Datatype: Not Applicable
   P.Description: None

   F.Description: Executes only once. Initialization is done here in this method.

 * */

void setup() {

  mySerial.begin(9600);   // Setting the baud rate of GSM Module
  Serial.begin(9600);    // Setting the baud rate of Serial Monitor (Arduino)
  pinMode(3, OUTPUT);
  delay(10000);

  // Establish HTTP connection in GSM module.
  setupHttpInit();



}

/* Function name: loop

   Parameter name: none
   P.Datatype: Not applicable
   P.Description: None

   F.Description: Main loop of the software

 * */

void loop() {
  //clearMemorySMS();

  bool newdata = false;
  unsigned long start = millis();
  // Every 5 seconds we print an update
  while (millis() - start < 5000)
  {
    if (Serial.available())

    {
      char c = Serial.read();
      //Serial.write(c);
      //Serial.print(c);  // uncomment to see raw GPS data
      if (gps.encode(c))
      {
        newdata = true;
        break;  // uncomment to print new data immediately!
      }
    }
  }

  // Print the GPS in Hardware Serial.
  // TODO: Send this info to LCD Screen

  if (newdata)
  {
    // Serial.println("Acquired Data");
    // Serial.println("-------------");
    gpsdump(gps);
    // Serial.println("-------------");
    // Serial.println();

    // Send to Server
    sendDataToServer(DEVICEID);
    delay(500);
  }

  Serial.println("Looping");

}


/* Function name: waitUntilResponse

   Parameter name: delayMs
   P.Datatype: int
   P.Description: Delay until next event. Should be specified in milliseconds.

   F.Description: Hold Execution until the GSM Module sends reponse.

 * */

void waitUntilReponse(int delayMs)
{
  // Delay in ms
  while (mySerial.available() < 0) {
    delay(delayMs);

  }
  while (mySerial.available() > 0) {
    Serial.write(mySerial.read());
  }

}

/* Function name: setupHttpInit

   Parameter name: None
   P.Datatype : Not Applicable
   P.Description: None

   F.Description: Setting up HTTP and Bearer configuration for sending HTTP GET request.

 * */

void setupHttpInit() {

  //Serial.println("Waiting for GSM to get ready");
  //wait till gsm responds ok
  waitUntilReponse(500);

  //Serial.println("GSM is ready proceed to send commands");

  // Sending APN Settings

  mySerial.println("AT+SAPBR=3,1,\"Contype\",\"GPRS\"\r\n");
  delay(1000);
  waitUntilReponse(500);

  mySerial.println("AT+SAPBR=3,1,\"APN\",\"bsnlnet\"\r\n");
  delay(1000);
  waitUntilReponse(500);

  mySerial.println("AT+SAPBR=1,1\r\n");
  delay(3000);
  waitUntilReponse(500);

  mySerial.println("AT+HTTPINIT\r\n");
  delay(300);
  waitUntilReponse(500);

}

/* Function name: sendDataToServer

   Parameter name: deviceId
   P.Datatype: int
   P.Description: Device ID to be sent to server for whose GPS Coordinates is being Uploaded.

   F.Description: Setting up HTTP and Bearer configuration for sending HTTP GET request.

 * */

void sendDataToServer(int deviceId) {



  mySerial.println("AT+HTTPINIT=?\r\n");
  delay(300);
  waitUntilReponse(500);

  mySerial.println("AT+HTTPPARA=\"CID\",1\r\n");
  delay(300);
  waitUntilReponse(500);



  mySerial.write("AT+HTTPPARA=\"URL\",\"http://devicetracker.000webhostapp.com/devicetrackerapis/upload.php?deviceid=");
  mySerial.print(deviceId);
  mySerial.write("&lat=");
  mySerial.print(latitude,9);
  mySerial.write("&lon=");
  mySerial.print(longitude,9);
  mySerial.write(":80\"\r\n");
  delay(500);
  waitUntilReponse(500);

  mySerial.write("AT+HTTPACTION=0\r\n");
  delay(3000);
  waitUntilReponse(500);


  //mySerial.write("AT+SAPBR =0,1\r\n");
  //  delay(500);
  //waitUntilReponse(500);

  mySerial.write("AT+HTTPREAD=0,10000\r\n");
  delay(500);
  waitUntilReponse(500);

}

/* Function name: gpsdump

   Parameter name: &gps
   P.Datatype :  TinyGPS
   P.Description: Pointer to the gps global variable that contains the GPS data.

   F.Description: writes GPS data to latitude(Global variable), longitude(Global variable).

 * */

void gpsdump(TinyGPS &gps)
{
  unsigned long age;

  gps.f_get_position(&latitude, &longitude, &age);


}

void clearMemorySMS() {
  for (int i = 1; i < 31; i++) {
    mySerial.write("AT+CMGD=");
    mySerial.write(char(i));
    mySerial.write("\r\n");
    waitUntilReponse(0);
  }
}




