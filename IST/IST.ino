#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

// Wi-Fi credentials
const char* ssid = "wifi_name";
const char* password = "wifi_password";

// GPS module configuration
TinyGPSPlus gps;
SoftwareSerial ss(5, 4); // RX, TX pins (change according to your setup)

WiFiClient wifiClient;  // Declare WiFiClient object

void setup() {
  Serial.begin(115200);
  ss.begin(9600); // GPS module baud rate
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
  }
  Serial.println("Connected!");
}

void loop() {
  // Read GPS data
  while (ss.available() > 0) {
    gps.encode(ss.read());
  }

  // Only send data if valid GPS data is available
  if (gps.location.isUpdated()) {
    double latitude = gps.location.lat();
    double longitude = gps.location.lng();
    double altitude = gps.altitude.meters(); // Altitude in meters
    double speed = gps.speed.kmph();         // Speed in km/h
    
    // Convert GMT time to IST
    if (gps.time.isValid()) {
      int hour = gps.time.hour();   // GMT hour
      int minute = gps.time.minute();
      int second = gps.time.second();

      // Add 5 hours and 30 minutes to GMT time
      hour += 5;
      minute += 30;

      // Handle overflow for minutes and hours
      if (minute >= 60) {
        minute -= 60;
        hour += 1;
      }
      if (hour >= 24) {
        hour -= 24; // Wrap around to the next day if hour exceeds 24
      }

      char timeIST[9];
      snprintf(timeIST, sizeof(timeIST), "%02d:%02d:%02d", hour, minute, second);

      // Print the GPS data on the Serial Monitor
      Serial.print("Latitude: ");
      Serial.println(latitude, 6);
      Serial.print("Longitude: ");
      Serial.println(longitude, 6);
      Serial.print("Altitude: ");
      Serial.println(altitude);
      Serial.print("Speed: ");
      Serial.println(speed);
      Serial.print("Time (IST): ");
      Serial.println(timeIST);

      // If Wi-Fi is connected, send GPS data to server
      if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(wifiClient, "http://your_server_location:port/location");  // Your API URL
        http.addHeader("Content-Type", "application/json");

        // Create the JSON payload with the GPS data
        String jsonData = "{";
        jsonData += "\"latitude\": \"" + String(latitude, 6) + "\", ";
        jsonData += "\"longitude\": \"" + String(longitude, 6) + "\", ";
        jsonData += "\"altitude\": \"" + String(altitude, 2) + "\", ";
        jsonData += "\"speed\": \"" + String(speed, 2) + "\", ";
        jsonData += "\"time\": \"" + String(timeIST) + "\"";
        jsonData += "}";

        int httpResponseCode = http.POST(jsonData);

        if (httpResponseCode > 0) {
          String response = http.getString();
          Serial.println(httpResponseCode);
          Serial.println(response);
        } else {
          Serial.println("Error in HTTP request");
        }
        http.end();
      }
    } else {
      Serial.println("Invalid GPS time data");
    }

    delay(10000); // Send data every 10 seconds
  }
}
