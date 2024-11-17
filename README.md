# GPS_Tracker
# ESP8266 GPS Tracker with Web Interface

## Overview
This project is a GPS tracker that uses the **ESP8266 Wi-Fi module** and the **NEO-6M GPS module** to gather location data like **latitude, longitude, speed, altitude**, and timestamps. The data is sent to a backend server using HTTP and displayed on a web interface.  

---

## Required Components
1. **ESP8266 Wi-Fi Module**  
2. **NEO-6M GPS Module**  
3. **Connecting Wires**  
4. **Power Supply** (3.3V for ESP8266 and 5V for GPS module)(optional)

---

## Circuit Diagram
### Connections:
| ESP8266 Pin | NEO-6M GPS Pin |
|-------------|----------------|
| GPIO 5      | RX             |
| GPIO 4      | TX             |
| VCC         | 5V             |
| GND         | GND            |

> **Note**: Ensure the ESP8266 and GPS module are properly powered. Use a voltage regulator or level shifter if needed.  
- Project Structure
```
Main Folder
│
├── GMT/
│   └── gmt_code.ino           # Code for sending GMT time, lat, long, etc.
│
├── IST/
│   └── ist_code.ino           # Code for sending IST time, lat, long, etc.
│
├── Webpage/
│   └── frontend/               # Frontend code for the web interface
│   └── backend/                # Backend code for the web server
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
### output sample
{
  "latitude": "33.225897",
  "longitude": "17.546809",
  "altitude": "0.00",
  "speed": "0.48",
  "time": "19:42:35"
}

