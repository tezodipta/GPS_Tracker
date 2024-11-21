#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

// Wi-Fi credentials
const char *ssid = "wifi-name";
const char *password = "password";

// Server URL
const char *serverUrl = "https://gps-tracker-esp8266.onrender.com/location"; // HTTPS API URL or IP address

// Phone number
#define PHONE_NUMBER "+913001234567"

// Pin definitions (from the first code)
#define rxGPS 5 // D1
#define txGPS 4 // D2
SoftwareSerial neogps(rxGPS, txGPS);
TinyGPSPlus gps;

#define rxGSM 0 // D3
#define txGSM 2 // D4
SoftwareSerial sim800(rxGSM, txGSM);

WiFiClientSecure wifiClientSecure; // Secure client for HTTPS communication

// Global variables
String smsStatus;
String senderNumber;
String receivedDate;
String msg;
boolean DEBUG_MODE = 1;

void setup()
{
    // Setup from the first code
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

    // Setup from the second code
    // Connect to Wi-Fi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting...");
    }
    Serial.println("Connected to Wi-Fi!");

    // Disable SSL certificate validation (for development purposes only)
    wifiClientSecure.setInsecure();
}

void loop()
{
    // Logic from the second code
    while (neogps.available() > 0)
    {
        gps.encode(neogps.read());
    }

    if (gps.location.isUpdated())
    {
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
                http.begin(wifiClientSecure, serverUrl);
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

            delay(10000); // Send data every 10 seconds
        }
    }

    // Call the function from the first code to send an SMS
    sendLocation(PHONE_NUMBER);
    makeCall(PHONE_NUMBER);
}

void sendLocation(String phoneNumber)
{
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
    Serial.println("SMS Sent Successfully.");
}

void debugPrint(String text)
{
    if (DEBUG_MODE == 1)
        Serial.println(text);
}
void makeCall(String phoneNumber)
{
    sim800.print("ATD");
    sim800.print(phoneNumber);
    sim800.println(";");
    delay(10000); // Keep the call active for 10 seconds
    sim800.println("ATH"); // Hang up the call
    delay(1000);
    Serial.println("Call made and ended.");
}