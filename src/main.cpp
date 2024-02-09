#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-websocket-server-arduino/
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/

// Replace with your network credentials
const char* ssid = "E308";
const char* password = "98806829";

bool ledState = 0;
const int ledPin = 2;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
#define _TIMERINTERRUPT_LOGLEVEL_   4
#include <ESP32TimerInterrupt.h>

#define TIMER0_INTERVAL_MS 1

// put function declarations here:

int analogPin1 = GPIO_NUM_32; // potentiometer wiper (middle terminal) connected to analog pin 32
int analogPin2 = GPIO_NUM_34;
int buttonPin = GPIO_NUM_4; // push button connected to digital pin 4

int potVal1 = 0; //AMP
int potVal2 = 0; //FEQ
int pVal = 0;
unsigned long timeDelay = 37;
unsigned long bpm = 0;

enum monState {AR, DEAD, HB};
enum monState state = HB;

                    // outside leads to ground and +5V
//int val = 0;  // variable to store the value read
  static uint8_t points[] = {
65,
65,
65,
65,
70,
76,
74,
70,
65,
63,
65,
65,
65,
65,
48,
230,
15,
65,
65,
65,
74,
90,
100,
102,
100,
95,
80,
70,
65,
65,
65,
65
};
#include <vector>

std::vector<uint8_t> points2(points + 12, points + 16);
ESP32Timer ITimer0(0);
bool IRAM_ATTR TimerHandler0(void * timerNo){
  static int index = 0;
  static int counter = timeDelay;
  static monState lastState = state;
  if(lastState != state){
    index = 0;
    counter = timeDelay;
    lastState = state;
  }

  if(counter <= 0){
    if(state == HB){ 
      pVal = points[index];
      if(index >= sizeof(points)){
      index = 0;
      }
    }else if(state == AR){
      pVal = points2[index];
      if(index >= sizeof(points2)){
      index = 0;
      }
    }else if (state == DEAD){
      pVal = 65;
      index = 0;
    }
    index++;
    counter = timeDelay;
  }else{
    pVal=65;
  }
    counter--;
  return true;
  }

// const char index_html[] PROGMEM = R"rawliteral(
// <!DOCTYPE HTML><html>
// <head>
//   <title>ESP Web Server</title>
//   <meta name="viewport" content="width=device-width, initial-scale=1">
//   <link rel="icon" href="data:,">
//   <style>
//   html {
//     font-family: Arial, Helvetica, sans-serif;
//     text-align: center;
//   }
//   h1 {
//     font-size: 1.8rem;
//     color: white;
//   }
//   h2{
//     font-size: 1.5rem;
//     font-weight: bold;
//     color: #143642;
//   }
//   .topnav {
//     overflow: hidden;
//     background-color: #143642;
//   }
//   body {
//     margin: 0;
//   }
//   .content {
//     padding: 30px;
//     max-width: 600px;
//     margin: 0 auto;
//   }
//   .card {
//     background-color: #F8F7F9;;
//     box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5);
//     padding-top:10px;
//     padding-bottom:20px;
//   }
//   .button {
//     padding: 15px 50px;
//     font-size: 24px;
//     text-align: center;
//     outline: none;
//     color: #fff;
//     background-color: #0f8b8d;
//     border: none;
//     border-radius: 5px;
//     -webkit-touch-callout: none;
//     -webkit-user-select: none;
//     -khtml-user-select: none;
//     -moz-user-select: none;
//     -ms-user-select: none;
//     user-select: none;
//     -webkit-tap-highlight-color: rgba(0,0,0,0);
//    }
//    /*.button:hover {background-color: #0f8b8d}*/
//    .button:active {
//      background-color: #0f8b8d;
//      box-shadow: 2 2px #CDCDCD;
//      transform: translateY(2px);
//    }
//    .state {
//      font-size: 1.5rem;
//      color:#8c8c8c;
//      font-weight: bold;
//    }
//   </style>
// <title>ESP Web Server</title>
// <meta name="viewport" content="width=device-width, initial-scale=1">
// <link rel="icon" href="data:,">
// </head>
// <body>
//   <div class="topnav">
//     <h1>ESP WebSocket Server</h1>
//   </div>
//   <div class="content">
//     <div class="card">
//       <h2>Output - GPIO 2</h2>
//       <p class="state">state: <span id="state">%STATE%</span></p>
//       <p><button id="button" class="button">Toggle</button></p>
//     </div>
//   </div>
// <script>
//   var gateway = `ws://${window.location.hostname}/ws`;
//   var websocket;
//   window.addEventListener('load', onLoad);
//   function initWebSocket() {
//     console.log('Trying to open a WebSocket connection...');
//     websocket = new WebSocket(gateway);
//     websocket.onopen    = onOpen;
//     websocket.onclose   = onClose;
//     websocket.onmessage = onMessage; // <-- add this line
//   }
//   function onOpen(event) {
//     console.log('Connection opened');
//   }
//   function onClose(event) {
//     console.log('Connection closed');
//     setTimeout(initWebSocket, 2000);
//   }
//   function onMessage(event) {
//     var state;
//     if (event.data == "1"){
//       state = "ON";
//     }
//     else{
//       state = "OFF";
//     }
//     document.getElementById('state').innerHTML = state;
//   }
//   function onLoad(event) {
//     initWebSocket();
//     initButton();
//   }
//   function initButton() {
//     document.getElementById('button').addEventListener('click', toggle);
//   }
//   function toggle(){
//     websocket.send('toggle');
//   }
// </script>
// </body>
// </html>
// )rawliteral";
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP Web Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <style>
  html {
    font-family: Arial, Helvetica, sans-serif;
    text-align: center;
  }
  h1 {
    font-size: 1.8rem;
    color: white;
  }
  h2{
    font-size: 1.5rem;
  }
  </style>
  <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
</head>
<body>
  <h1>ESP Web Server</h1>
  <canvas id="myChart"></canvas>
  <script>
    var ctx = document.getElementById('myChart').getContext('2d');
    var chart = new Chart(ctx, {
      type: 'line',
      data: {
        labels: [],
        datasets: [{
          label: 'ECG',
          data: [],
          borderColor: 'rgb(255, 99, 132)',
          fill: false
        }, {
          label: 'BPM',
          data: [],
          borderColor: 'rgb(75, 192, 192)',
          fill: false
        }]
      }
    });

  var gateway = `ws://${window.location.hostname}/ws`;
  var websocket;
  window.addEventListener('load', onLoad);
  function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.binaryType = 'arraybuffer';
    websocket.onmessage = onMessage;
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
  }
  function onOpen(event) {
    console.log('Connection opened');
  }
  function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
  }
  function onMessage(evt) {
      var dataView = new DataView(evt.data);
      var ecg = dataView.getFloat32(0, true);
      var bpm = dataView.getFloat32(4, true);
      chart.data.labels.push('');
      chart.data.datasets[0].data.push(evt.data.ECG);
      chart.data.datasets[1].data.push(evt.data.BPM);
      chart.update();
  }
  function onLoad(event) {
    initWebSocket();
  }
  </script>
</body>
</html>
)rawliteral";

void notifyClients() {
  ws.textAll(String(ledState));
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    if (strcmp((char*)data, "toggle") == 0) {
      ledState = !ledState;
      notifyClients();
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

String processor(const String& var){
  Serial.println(var);
  if(var == "STATE"){
    if (ledState){
      return "ON";
    }
    else{
      return "OFF";
    }
  }
  return String();
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);
  delay(1000);
  	if (ITimer0.attachInterruptInterval(TIMER0_INTERVAL_MS * 1000, TimerHandler0))
	{
		Serial.print(F("Starting  ITimer0 OK, millis() = "));
		Serial.println(millis());
	}

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());

  initWebSocket();

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  // Start server
  server.begin();
}
unsigned long lastButtonPressTime = 0;
const unsigned long debounceDelay = 150;
// Create a buffer to hold the binary data
uint8_t buffer[8];
void loop() {
  ws.cleanupClients();
  static unsigned long lastTimer = 0;
  potVal1 = analogRead(analogPin1);
  long mappedVal1 = map(potVal1,0,4095,0,100);
  float ecg = pVal * (mappedVal1/100.0)+65*(1-(mappedVal1/100.0));
  
  potVal2 = analogRead(analogPin2);
  timeDelay = map(potVal2,0,4095,9,48);
  bpm = map(potVal2,0,4095,220,40);
    if(pVal != 0){
    // Print to Serial for debugging
    Serial.print(ecg);
    Serial.print(",");
    Serial.println(bpm);
    memcpy(buffer, &ecg, sizeof(ecg));
    memcpy(buffer + sizeof(ecg), &bpm, sizeof(bpm));

    // Send the buffer to all connected WebSocket clients
    ws.binaryAll(buffer, sizeof(buffer));

    pVal = 0; 
}
  
  // Check for button press with debounce
  if (millis() - lastButtonPressTime >= debounceDelay) {
    if(digitalRead(buttonPin) == LOW){
      if(state == HB){
        state = AR;
      }
      else if(state == AR){
        state = DEAD;
      }
      else if(state == DEAD){
        state = HB;
      }
      lastButtonPressTime = millis();
    }
  }
}


