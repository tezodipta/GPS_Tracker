# GPS_Tracker
# ESP8266 GPS Tracker with Web Interface and SMS Functionality

## Overview
This project is a GPS tracker that uses the **ESP8266 Wi-Fi module**, **NEO-6M GPS module**, and **SIM800L GSM module** to gather and transmit location data like **latitude, longitude, speed, altitude**, and timestamps. The data is sent to a backend server using HTTP, displayed on a web interface, and can also be sent via **SMS** or **missed call** to a specified phone number by triggering a button.

---

## Required Components
1. **ESP8266 Wi-Fi Module**  
2. **NEO-6M GPS Module**  
3. **SIM800L GSM Module**  
4. **Connecting Wires**  
5. **Power Supply** (3.3V for ESP8266 and 5V for GPS and GSM modules)

---

## Circuit Diagram
### Connections:
| ESP8266 Pin | NEO-6M GPS Pin | SIM800L Pin  |
|-------------|----------------|--------------|
| D1          | D1 (RX)        |              |
| D2          | D2 (TX)        |              |
| D3          |                | D3 (TX)      |   
| D4          |                | D4 (RX)      | 
| VCC         | 3V             | 3.7v Battery |
| GND         | GND            | GND          |

>create a common ground for all the modules.

> **Note**: Ensure proper power supply to all modules. Use a voltage regulator or level shifter as needed for compatibility.  

---

## Project Structure

```
Main Folder
│
├── Circuit_pic/
│   └── Circuit.png                            # Circuit diagram for the project. 
|
├── Final/
│   └── Final_code.ino                         # Code for sending location data to Website and sending location Via sms by push button and Emergency calling function.
|
├── GMT/
│   └── GMT.ino                                # Code for sending GMT time, lat, long, etc to the server.
│
├── IST/
│   └── IST.ino                                # Code for sending IST time, lat, long, etc to the server.
│
├── sendLocation/
│   └── sendLocation.ino                       # Code for sending GMT time, lat, long, via sms ony.
│
├── Webpage/
│   └── frontend/                              # Frontend code for the web interface
│   └── backend/                               # Backend code for the web server
```

---

## Installing Dependencies

### Prerequisites
- Install **Node.js** from [nodejs.org](https://nodejs.org).  
- Verify the installation:
  ```bash
  node -v
  npm -v

- Install cors
    ```bash
    npm install cors
    ```
- To start the server, run:
    ```bash
    node server.js
    ```
    or
    ```bash
    nodemon server.js
    ```
> For ESP8266
- Install the **Arduino IDE** from [arduino.cc](https://www.arduino.cc/en/software).
- 1. ESP8266 Library (ESP8266WiFi, ESP8266HTTPClient, WiFiClientSecure)
    Open Arduino IDE.
    Go to File > Preferences.
    Add the following URL to the Additional Board Manager URLs field:
    ```
    http://arduino.esp8266.com/stable/package_esp8266com_index.json
    ```
    Go to Tools > Board > Boards Manager.
    Search for ESP8266 and click Install.
    This will install the ESP8266 core along with the required libraries (ESP8266WiFi.h, ESP8266HTTPClient.h, WiFiClientSecure.h).
- 2. TinyGPS++ Library
    Go to Sketch > Include Library > Manage Libraries.
    In the Library Manager, search for TinyGPS++.
    Install the library by Mikal Hart.

### Circuit Diagram
![Circuit image](https://github.com/tezodipta/GPS_Tracker/blob/main/Circuit_Pic/Circuit.png?raw=true)

### output sample
{
  "latitude": "33.225897",
  "longitude": "17.546809",
  "altitude": "0.00",
  "speed": "0.48",
  "time": "19:42:35"
}

