#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

// Wi-Fi credentials
const char *ssid = "";
const char *password = "";

// Server URL
const char *serverUrl = "http://SERVER-IP/location"; // HTTP API URL or IP address

// Phone number
#define PHONE_NUMBER "+91xxxxxxxxx"

// Pin definitions for GPS and GSM
#define rxGPS 5 // D1
#define txGPS 4 // D2
SoftwareSerial neogps(rxGPS, txGPS);
TinyGPSPlus gps;

#define redl 15
#define greenl 13
// #define rxGSM 1 // TX
#define rxGSM 0 // D3
#define txGSM 2 // D4
SoftwareSerial sim800(rxGSM, txGSM);

// Push button pin definitions
#define BUTTON_SEND 14 // Send location button
#define BUTTON_CALL 12 // Make call button

WiFiClient wifiClient; // Simple client for HTTP communication

// Global variables
String smsStatus;
String senderNumber;
String receivedDate;
String msg;
boolean DEBUG_MODE = 1;

void setup()
{
    // Setup for GPS and GSM
    Serial.begin(115200);
    Serial.println("NodeMCU USB serial initialize");

    pinMode(redl, OUTPUT);
    pinMode(greenl, OUTPUT);
    digitalWrite(greenl, LOW);
    digitalWrite(redl, HIGH);

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
    digitalWrite(redl, LOW); // Reset red LED
    digitalWrite(greenl, HIGH);
    delay(500);
    digitalWrite(greenl, LOW);

    // Wi-Fi setup
    WiFi.begin(ssid, password);

    bool connected = false;
    for (int attempts = 0; attempts < 15; attempts++)
    {
        digitalWrite(redl, HIGH);
        if (WiFi.status() == WL_CONNECTED)
        {
            connected = true;
            digitalWrite(redl, LOW);
            break;
        }
        delay(500);
        Serial.println("Connecting...");
        digitalWrite(redl, LOW);
        delay(500);
    }

    if (connected)
    {
        Serial.println("Connected to Wi-Fi!");
        digitalWrite(greenl, HIGH); // Turn on green LED to indicate connection
        delay(2000);
        digitalWrite(greenl, LOW);   // Turn off red LED
        delay(500);
    }
    else
    {
        Serial.println("Failed to connect to Wi-Fi.");
        digitalWrite(redl, HIGH); // Keep red LED on to indicate failure
        delay(2000);
        digitalWrite(greenl, LOW);
        delay(500); // Turn off green LED
    }

    // Button setup
    pinMode(BUTTON_SEND, INPUT);
    pinMode(BUTTON_CALL, INPUT);
}

void loop()
{
    // GPS data processing
    while (neogps.available() > 0)
    {
        gps.encode(neogps.read());
    }

    if (gps.location.isUpdated())
    {
        ESP.wdtDisable();
        double latitude = gps.location.lat();
        double longitude = gps.location.lng();
        double altitude = gps.altitude.meters();
        double speed = gps.speed.kmph();
        if (gps.time.isValid())
        {
            int hour = gps.time.hour();
            int minute = gps.time.minute();
            int second = gps.time.second();

            // Add 5 hours and 30 minutes to GMT time
            hour += 5;
            minute += 30;

            // Handle overflow for minutes and hours
            if (minute >= 60)
            {
                minute -= 60;
                hour += 1;
            }
            if (hour >= 24)
            {
                hour -= 24;
            }

            char timeIST[9];
            snprintf(timeIST, sizeof(timeIST), "%02d:%02d:%02d", hour, minute, second);

            Serial.print("Latitude: ");
            Serial.println(latitude, 6);
            Serial.print("Longitude: ");
            Serial.println(longitude, 6);
            Serial.print("Altitude: ");
            Serial.println(altitude);
            Serial.print("Speed: ");
            Serial.println(speed);
            Serial.print("Time: ");
            Serial.println(timeIST);

            if (WiFi.status() == WL_CONNECTED)
            {
                HTTPClient http;
                http.begin(wifiClient, serverUrl); // Use the WiFiClient object here
                http.addHeader("Content-Type", "application/json");

                String jsonData = "{";
                jsonData += "\"latitude\": \"" + String(latitude, 6) + "\", ";
                jsonData += "\"longitude\": \"" + String(longitude, 6) + "\", ";
                jsonData += "\"altitude\": \"" + String(altitude, 2) + "\", ";
                jsonData += "\"speed\": \"" + String(speed, 2) + "\", ";
                jsonData += "\"time\": \"" + String(timeIST) + "\"";
                jsonData += "}";

                int httpResponseCode = http.POST(jsonData);

                if (httpResponseCode > 0)
                {
                    String response = http.getString();
                    Serial.println(httpResponseCode);
                    Serial.println(response);
                }
                else
                {
                    Serial.print("Error in HTTP request: ");
                    Serial.println(httpResponseCode);
                }
                http.end();
            }
            else
            {
                Serial.println("Wi-Fi disconnected!");
            }
        }
    }

    // Check button states
    if (digitalRead(BUTTON_SEND) == HIGH) // Button pressed (active LOW)
    {
        hw_wdt_disable();
        sendLocation(PHONE_NUMBER);
        delay(1000); // Debounce delay
    }

    if (digitalRead(BUTTON_CALL) == HIGH) // Button pressed (active LOW)
    {
        hw_wdt_disable();
        makeCall(PHONE_NUMBER);
        delay(1000); // Debounce delay
    }
}

// The sendLocation and Reply functions remain unchanged
void sendLocation(String phoneNumber)
{
    digitalWrite(redl,HIGH);
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

void Reply(String text, String Phone)
{
    sim800.print("AT+CMGF=1\r");
    delay(1000);
    sim800.print("AT+CMGS=\"" + Phone + "\"\r");
    delay(1000);
    sim800.print(text);
    delay(100);
    sim800.write(0x1A);
    delay(1000);
    digitalWrite(redl,LOW);
    digitalWrite(greenl,HIGH);
    Serial.println("SMS Sent Successfully.");
    delay(2000);
    digitalWrite(greenl,LOW);
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

void makeCall(String phoneNumber) {
    sim800.print("ATD");
    sim800.print(phoneNumber);
    sim800.println(";");
    unsigned long startTime = millis();
    // digitalWrite(greenl,HIGH);
    // delay(7000);
    // digitalWrite(greenl,LOW);

    while (millis() - startTime < 16000) { // Keep the call active for 16 seconds
        digitalWrite(greenl, HIGH);
        delay(50); // Short delay to allow background tasks
        digitalWrite(greenl, LOW);
        delay(50);
        yield(); // Reset the WDT and handle background tasks
    }

    sim800.println("ATH"); // Hang up the call
    delay(1000);
    Serial.println("Call made .");
}
void hw_wdt_disable(){
  *((volatile uint32_t*) 0x60000900) &= ~(1); // Hardware WDT OFF
}

void hw_wdt_enable(){
  *((volatile uint32_t*) 0x60000900) |= 1; // Hardware WDT ON
}