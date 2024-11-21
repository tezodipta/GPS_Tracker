#include <SoftwareSerial.h>
#include <TinyGPS++.h>

#define PHONE_NUMBER "+913001234567"


#define rxGPS 5 // D1
#define txGPS 4 // D2
SoftwareSerial neogps(rxGPS, txGPS);
TinyGPSPlus gps;

#define rxGSM 0 // D3
#define txGSM 2 // D4
SoftwareSerial sim800(rxGSM, txGSM);

String smsStatus;
String senderNumber;
String receivedDate;
String msg;
boolean DEBUG_MODE = 1;


void setup()
{
    Serial.begin(115200);
    Serial.println("NodeMCU USB serial initialize");

    sim800.begin(9600);
    Serial.println("SIM800L serial initialize");

    neogps.begin(9600);
    Serial.println("NEO6M serial initialize");

    sim800.listen();
    neogps.listen();
  

    smsStatus = "";
    senderNumber = "";
    receivedDate = "";
    msg = "";

    delay(15000);
    sim800.println("AT+CMGF=1"); // SMS text mode
    delay(1000);
    sim800.println("AT+CLIP=1"); // Enable Caller ID
    delay(500);
}

void loop()
{
    delay(30000);
    sendLocation(PHONE_NUMBER);
}


void Reply(String text, String Phone)
{
    // return;
    sim800.print("AT+CMGF=1\r");
    delay(1000);
    sim800.print("AT+CMGS=\"" + Phone + "\"\r");
    delay(1000);
    sim800.print(text);
    delay(100);
    sim800.write(0x1A); // ascii code for ctrl-26 //sim800.println((char)26); //ascii code for ctrl-26
    delay(1000);
    Serial.println("SMS Sent Successfully.");


    smsStatus = "";
    senderNumber = "";
    receivedDate = "";
    msg = "";
}

void debugPrint(String text)
{
    if (DEBUG_MODE == 1)
        Serial.println(text);
}

void sendLocation(String phoneNumber)
{
    //-----------------------------------------------------------------
    // Can take up to 60 seconds
    boolean newData = false;
    for (unsigned long start = millis(); millis() - start < 10000;)
    {
        while (neogps.available())
        {
            if (gps.encode(neogps.read()))
            {
                newData = true;
                break;
            }
        }
    }

   if (newData)
    {
        newData = false;
        String latitude = String(gps.location.lat(), 6);
        String longitude = String(gps.location.lng(), 6);

        String text = "Latitude= " + latitude +
                      "\nLongitude= " + longitude +
                      "\nSpeed= " + String(gps.speed.kmph()) + " km/h" +
                      "\nAltitude= " + String(gps.altitude.meters()) + " meters" +
                      "\nhttp://maps.google.com/maps?q=loc:" + latitude + "," + longitude;

        debugPrint(text);
        Reply(text, phoneNumber);
    }
}