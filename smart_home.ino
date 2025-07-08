#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h> // stawia serwer HTTP
#include <ArduinoJson.h>
#include <DHT.h> 

// konfiugracja Wi-Fi
const char* ssid = "SiecGoscia_2.4G";
const char* password = "niebojsienic69";

// konfiguracja czujnika temperatury i wilgotności DHT11
const int lightSensorPin = 34;
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// konfiguracja RGB
#define RED_PIN   12
#define GREEN_PIN 13
#define BLUE_PIN  14
// wartosci poczatkowe po uruchomieniu
int redValue = 120;
int greenValue = 100;
int blueValue = 230;

// http port 80
WebServer server(80);

// generowanie strony
void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="pl">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>SmartHome ESP32</title>
  <style>
    :root {
      --bg-color: #0f0f0f; --card-color: #1f1f1f; --accent: #00e5ff;
      --text-color: #f0f0f0; --secondary-text: #aaa; --success: #4caf50;
      --error: #f44336; --button: #00bcd4; --button-hover: #00acc1;
      --font-main: 'Segoe UI', sans-serif; --shadow-color: rgba(0, 229, 255, 0.08);
      --transition: all 0.3s ease;
    }
    * { box-sizing: border-box; margin: 0; padding: 0; }
    body {
      margin: 0; padding: 20px; font-family: var(--font-main);
      background: var(--bg-color); color: var(--text-color);
      display: flex; justify-content: center; align-items: center;
      min-height: 100vh;
      transition: var(--transition);
    }
    .container {
      width: 100%; max-width: 500px; padding: 25px 20px;
      background: var(--card-color); border-radius: 16px;
      box-shadow: 0 0 20px var(--shadow-color); text-align: center;
      transition: var(--transition);
    }
    .connection-status {
      border-radius: 12px; padding: 15px; margin: 0 0 20px 0;
      position: relative; font-weight: bold; text-align: center;
      background: linear-gradient(145deg, #1f1f1f, #2a2a2a);
      box-shadow: 0 0 10px rgba(76, 175, 80, 0.5);
      transition: var(--transition);
    }
    .connection-status.error {
      box-shadow: 0 0 10px rgba(244, 67, 54, 0.5);
    }
    .sensor-data, .rgb-data {
      background: linear-gradient(145deg, var(--card-color), #2a2a2a);
      border-radius: 12px; padding: 20px; margin: 0 0 16px 0;
      position: relative; transition: var(--transition);
    }
    .sensor-data::before, .rgb-data::before {
      content: ''; position: absolute; top: 0; left: 0;
      width: 4px; height: 100%; background: var(--accent);
      transition: var(--transition);
    }
    h1 { margin-bottom: 15px; }
    h2 { margin-bottom: 10px; font-size: 1.2rem; }
    .sensor-value { 
      font-size: 32px; font-weight: bold; 
      color: var(--success); margin: 5px 0;
    }
    .info { margin-top: 20px; color: var(--secondary-text); font-size: 0.9rem; }
    .rgb-values { 
      display: flex; justify-content: center; gap: 15px;
      margin: 10px 0; font-size: 0.95rem;
    }
    .color-preview { 
      width: 60px; height: 60px; border-radius: 50%; 
      margin: 10px auto; border: 2px solid var(--accent);
      transition: var(--transition);
      box-shadow: 0 0 10px rgba(0,0,0,0.3);
    }
    .slider-container { margin-top: 15px; }
    .slider-row {
      display: flex; align-items: center; 
      gap: 10px; margin: 8px 0;
    }
    .slider-row label { 
      width: 25px; text-align: right; 
      font-weight: bold; color: var(--accent);
    }
    .slider-row input[type="range"] { 
      flex: 1; height: 8px;
      -webkit-appearance: none;
      background: #333;
      border-radius: 4px;
      outline: none;
    }
    .slider-row input[type="range"]::-webkit-slider-thumb {
      -webkit-appearance: none;
      width: 18px; height: 18px;
      border-radius: 50%;
      background: var(--accent);
      cursor: pointer;
    }
    .slider-row .value-display {
      width: 35px; text-align: left;
      font-family: monospace;
    }
    .btn {
      background: var(--button); color: white;
      border: none; padding: 8px 16px;
      border-radius: 5px; cursor: pointer;
      margin-top: 10px; font-weight: bold;
      transition: var(--transition);
    }
    .btn:hover { background: var(--button-hover); }
    .btn:active { transform: scale(0.98); }
    @media (max-width: 480px) {
      body { padding: 15px; }
      .container { padding: 20px 15px; }
      .rgb-values { font-size: 0.85rem; gap: 10px; }
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>SmartHome ESP32</h1>
    <div id="connection-status" class="connection-status">
      <div>Status: <span id="status-text">Połączono</span></div>
      <div>IP: <span id="ip-display">)rawliteral";
  html += WiFi.localIP().toString();
  html += R"rawliteral(</span></div>
    </div>

    <div class="sensor-data">
      <h2>Natężenie światła</h2>
      <div class="sensor-value" id="light">--</div>
      <div class="sensor-unit">(0-4095)</div>
    </div>

    <div class="sensor-data">
      <h2>Temperatura</h2>
      <div class="sensor-value" id="temp">--</div>
      <div class="sensor-unit">°C</div>
    </div>

    <div class="sensor-data">
      <h2>Wilgotność</h2>
      <div class="sensor-value" id="humidity">--</div>
      <div class="sensor-unit">%</div>
    </div>

    <div class="rgb-data">
      <h2>Sterowanie RGB</h2>
      <div class="color-preview" id="colorPreview"></div>
      <div class="rgb-values">
        <div>R: <span id="redValue">--</span></div>
        <div>G: <span id="greenValue">--</span></div>
        <div>B: <span id="blueValue">--</span></div>
      </div>
      
      <div class="slider-container">
        <div class="slider-row">
          <label for="redSlider">R</label>
          <input type="range" id="redSlider" min="0" max="255" value="120">
          <span class="value-display" id="redOut">120</span>
        </div>
        <div class="slider-row">
          <label for="greenSlider">G</label>
          <input type="range" id="greenSlider" min="0" max="255" value="80">
          <span class="value-display" id="greenOut">80</span>
        </div>
        <div class="slider-row">
          <label for="blueSlider">B</label>
          <input type="range" id="blueSlider" min="0" max="255" value="200">
          <span class="value-display" id="blueOut">200</span>
        </div>
      </div>
      
      <button class="btn" id="randomColorBtn">Losowy kolor</button>
      <button class="btn" id="turnOffBtn">Wyłącz</button>
    </div>

    <div class="info">
      <p>Ostatnia aktualizacja: <span id="last-update">--</span></p>
      <p>Wersja: 1.2.0 | ESP32-WROOM</p>
    </div>
  </div>

  <script>
    document.addEventListener('DOMContentLoaded', () => {
      // Elementy DOM
      const lightElement = document.getElementById('light');
      const tempElement = document.getElementById('temp');
      const humidityElement = document.getElementById('humidity');
      const redValueEl = document.getElementById('redValue');
      const greenValueEl = document.getElementById('greenValue');
      const blueValueEl = document.getElementById('blueValue');
      const colorPreview = document.getElementById('colorPreview');
      const connectionStatus = document.getElementById('connection-status');
      const statusText = document.getElementById('status-text');
      const ipDisplay = document.getElementById('ip-display');
      
      // Suwaki RGB
      const redSlider = document.getElementById('redSlider');
      const greenSlider = document.getElementById('greenSlider');
      const blueSlider = document.getElementById('blueSlider');
      const redOut = document.getElementById('redOut');
      const greenOut = document.getElementById('greenOut');
      const blueOut = document.getElementById('blueOut');
      
      // Przyciski
      const randomColorBtn = document.getElementById('randomColorBtn');
      const turnOffBtn = document.getElementById('turnOffBtn');
      
      // Zmienne stanu
      let isConnected = true;
      let lastUpdateTime = '--';
      
      // Funkcja wysyłająca kolor do ESP32
      function sendRGB(r, g, b) {
        if (!isConnected) return;
        
        fetch(`/setRGB?r=${r}&g=${g}&b=${b}`)
          .then(response => {
            if (!response.ok) throw new Error('Błąd wysyłania');
            updateUI(r, g, b);
          })
          .catch(error => {
            console.error('Błąd:', error);
            setConnectionStatus(false);
          });
      }
      
      // Aktualizacja interfejsu
      function updateUI(r, g, b) {
        redValueEl.textContent = r;
        greenValueEl.textContent = g;
        blueValueEl.textContent = b;
        colorPreview.style.background = `rgb(${r},${g},${b})`;
        redSlider.value = r; redOut.textContent = r;
        greenSlider.value = g; greenOut.textContent = g;
        blueSlider.value = b; blueOut.textContent = b;
      }
      
      // Ustawianie statusu połączenia
      function setConnectionStatus(connected) {
        isConnected = connected;
        if (connected) {
          statusText.textContent = "Połączono";
          connectionStatus.classList.remove("error");
        } else {
          statusText.textContent = "Brak połączenia";
          connectionStatus.classList.add("error");
        }
      }
      
      // Obsługa suwaków
      function handleSliderChange() {
        const r = parseInt(redSlider.value);
        const g = parseInt(greenSlider.value);
        const b = parseInt(blueSlider.value);
        
        redOut.textContent = r;
        greenOut.textContent = g;
        blueOut.textContent = b;
        
        sendRGB(r, g, b);
      }
      
      // Losowy kolor
      function setRandomColor() {
        const r = Math.floor(Math.random() * 256);
        const g = Math.floor(Math.random() * 256);
        const b = Math.floor(Math.random() * 256);
        
        redSlider.value = r; redOut.textContent = r;
        greenSlider.value = g; greenOut.textContent = g;
        blueSlider.value = b; blueOut.textContent = b;
        
        sendRGB(r, g, b);
      }
      
      // Wyłącz diodę
      function turnOffLED() {
        redSlider.value = 0; redOut.textContent = 0;
        greenSlider.value = 0; greenOut.textContent = 0;
        blueSlider.value = 0; blueOut.textContent = 0;
        
        sendRGB(0, 0, 0);
      }
      
      // Pobieranie danych z czujników
      async function fetchSensorData() {
        try {
          const response = await fetch('/sensors');
          if (!response.ok) throw new Error('Błąd odpowiedzi');
          
          const data = await response.json();
          
          // Aktualizacja danych czujników
          lightElement.textContent = data.light ?? '--';
          tempElement.textContent = data.temperature ?? '--';
          humidityElement.textContent = data.humidity ?? '--';
          
          // Aktualizacja czasu
          lastUpdateTime = new Date().toLocaleTimeString();
          document.getElementById('last-update').textContent = lastUpdateTime;
          
          setConnectionStatus(true);
          return true;
        } catch (error) {
          console.error('Błąd pobierania danych:', error);
          setConnectionStatus(false);
          return false;
        }
      }
      
      // Inicjalizacja
      function init() {
        // Nasłuchiwanie zdarzeń
        redSlider.addEventListener('input', handleSliderChange);
        greenSlider.addEventListener('input', handleSliderChange);
        blueSlider.addEventListener('input', handleSliderChange);
        randomColorBtn.addEventListener('click', setRandomColor);
        turnOffBtn.addEventListener('click', turnOffLED);
        
        // Pierwsze pobranie danych
        fetchSensorData();
        
        // Cykliczne odświeżanie
        setInterval(fetchSensorData, 2000);
      }
      
      init();
    });
  </script>
</body>
</html>
)rawliteral";

  server.send(200, "text/html", html);
}

// API z odczytem czujników przez handleSensors()
// Odczytuje z czujnika światła, temperatury i wilgotności.
// Odczytuje aktualne wartości RGB.
// Wysyła wszystko w formacie JSON – strona internetowa to odbiera i wyświetla.
void handleSensors() {
  int lightValue = analogRead(lightSensorPin);
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  StaticJsonDocument<200> doc;
  doc["light"] = lightValue;
  if (!isnan(temperature)) doc["temperature"] = round(temperature * 10) / 10.0;
  if (!isnan(humidity)) doc["humidity"] = round(humidity * 10) / 10.0;
  doc["red"] = redValue;
  doc["green"] = greenValue;
  doc["blue"] = blueValue;

  String json;
  serializeJson(doc, json);

  server.send(200, "application/json", json);
}


// obsługa zmiany koloru LED przez przeglądarkę handleSetRGB
void handleSetRGB() {
  if (server.hasArg("r") && server.hasArg("g") && server.hasArg("b")) {
    redValue = constrain(server.arg("r").toInt(), 0, 255);
    greenValue = constrain(server.arg("g").toInt(), 0, 255);
    blueValue = constrain(server.arg("b").toInt(), 0, 255);

    ledcWrite(0, redValue);
    ledcWrite(1, greenValue);
    ledcWrite(2, blueValue);

    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Missing parameters");
  }
}

// inicjalizacja 
void setup() {
  Serial.begin(115200); // uruchamia port szeregowy do debugowania
  delay(1000);

  pinMode(lightSensorPin, INPUT); //wejście analogowe
  dht.begin(); // inicjalizacja DHT11 - start 

  // PWM dla RGB (piny 12, 13, 14, kanały 0,1,2)
  ledcSetup(0, 5000, 8);
  ledcSetup(1, 5000, 8);
  ledcSetup(2, 5000, 8);
  ledcAttachPin(RED_PIN, 0);
  ledcAttachPin(GREEN_PIN, 1);
  ledcAttachPin(BLUE_PIN, 2);

  ledcWrite(0, redValue);
  ledcWrite(1, greenValue);
  ledcWrite(2, blueValue);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Łączenie z WiFi ");
  int attempt = 0;
  while (WiFi.status() != WL_CONNECTED && attempt < 20) {
    delay(500);
    Serial.print(".");
    attempt++;
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Połączono! IP: ");
    Serial.println(WiFi.localIP());
    server.on("/", handleRoot);
    server.on("/sensors", handleSensors);
    server.on("/setRGB", handleSetRGB);   // obsługa RGB
    server.begin();
    Serial.println("Serwer HTTP uruchomiony");
  } else {
    Serial.println("Nie udało się połączyć.");
  }
}

void loop() {
  server.handleClient(); // obsługa żądań HTTP od przeglądarki 

  static unsigned long lastPrint = 0;
  if (millis() - lastPrint >= 2000) {
    lastPrint = millis();
    // printy
    Serial.print("Light: ");
    Serial.print(analogRead(lightSensorPin));
    Serial.print(" | Temp: ");
    Serial.print(dht.readTemperature());
    Serial.print("°C | Wilgotność: ");
    Serial.print(dht.readHumidity());
    Serial.print("% | RGB: ");
    Serial.print(redValue); Serial.print(", ");
    Serial.print(greenValue); Serial.print(", ");
    Serial.println(blueValue);
  }
}
