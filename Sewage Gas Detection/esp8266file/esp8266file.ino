#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

// Replace with your network credentials
const char *ssid = "MelodyJumper";
const char *password = "noobmaster";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create an Event Source on /events
AsyncEventSource events("/events");

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 30000;

long currentTime;

float lgp_ppm_float;
float co_ppm_float;
float nh4_ppm_float;

void getSensorReadings()
{
  currentTime = millis();
}

void initWiFi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

String processor(const String &var)
{
  getSensorReadings();
  Serial.println(var);
  if (var == "TIME")
  {
    return String(currentTime);
  }
  return String();
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP Web Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <link rel="icon" href="data:,">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    p { font-size: 1.2rem;}
    body {  margin: 0;}
    .topnav { overflow: hidden; background-color: #50B8B4; color: white; font-size: 1rem; }
    .content { padding: 20px; }
    .card { background-color: white; box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5); }
    .cards { max-width: 800px; margin: 0 auto; display: grid; grid-gap: 2rem; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); }
    .reading { font-size: 1.4rem; }
  </style>
</head>
<body>
  <div class="topnav">
    <h1> WEB SERVER (SSE)</h1>
  </div>
  <div class="content">
    <div class="cards">
     <!--  <div class="card">
       <p><i class="fas fa-thermometer-half" style="color:#059e8a;"></i> TEMPERATURE</p><p><span class="reading"><span id="temp">%TEMPERATURE%</span> &deg;C</span></p>
     </div> -->
      <div class="card">
        <p><i class="fas fa-angle-double-down" style="color:#e1e437;"></i>LPG ppm</p><p><span class="reading"><span id="lpg">%PRESSURE%</span> ppm</span></p>
      </div>
      <div class="card">
        <p><i class="fas fa-angle-double-down" style="color:#e1e437;"></i> CO ppm</p><p><span class="reading"><span id="co">%TIME%</span> ppm</span></p>
      </div>
       <div class="card">
        <p><i class="fas fa-tint" style="color:#00add6;"></i> NH4 ppm</p><p><span class="reading"><span id="nh4">%HUMIDITY%</span> ppm</span></p>
      </div>
      <p id="warning"> </p>
    </div>
  </div>
<script>
if (!!window.EventSource) {
 var source = new EventSource('/events');
 
 source.addEventListener('open', function(e) {
  console.log("Events Connected");
 }, false);
 source.addEventListener('error', function(e) {
  if (e.target.readyState != EventSource.OPEN) {
    console.log("Events Disconnected");
  }
 }, false);
 
 source.addEventListener('message', function(e) {
  console.log("message", e.data);
 }, false);
 
 source.addEventListener('temperature', function(e) {
  console.log("temperature", e.data);
  document.getElementById("temp").innerHTML = e.data;
 }, false);

 source.addEventListener('time', function(e) {
  console.log("temperature", e.data);
  document.getElementById("temp").innerHTML = e.data;
 }, false);
 
 source.addEventListener('lpg', function(e) {
  console.log("lpg", e.data);
  document.getElementById("lpg").innerHTML = e.data;
 }, false);
 
 source.addEventListener('co', function(e) {
  console.log("co", e.data);
  document.getElementById("co").innerHTML = e.data;
 }, false);

 source.addEventListener('nh4', function(e) {
  console.log("nh4", e.data);
  document.getElementById("nh4").innerHTML = e.data;
 }, false);

 source.addEventListener('warning', function(e) {
  console.log("warning", e.data);
  document.getElementById("warning").innerHTML = e.data;
 }, false);
}
</script>
</body>
</html>)rawliteral";

void setup()
{
  Serial.begin(115200);
  initWiFi();

  // Handle Web Server
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/html", index_html, processor); });

  // Handle Web Server Events
  events.onConnect([](AsyncEventSourceClient *client)
                   {
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 10000); });
  server.addHandler(&events);
  server.begin();
}

void loop()
{
  if (Serial.available())
  {
    // Serial.print(Serial.read());
    // events.send(Serial.readStringUntil(10).c_str(), "time", millis());
    String ReadString;
    ReadString = Serial.readStringUntil(10);
    ReadString.trim();
    Serial.println(ReadString.c_str());
    if (ReadString.startsWith("$"))
    {
      Serial.print("DATA: ");
      ReadString.remove(0, 1);
      int pos = ReadString.indexOf(',');
      char lgp_ppm[pos + 5];
      int i;
      for (i = 0; i < pos; i++)
      {
        lgp_ppm[i] = ReadString.charAt(i);
      }
      lgp_ppm[i] = '\0';
      Serial.print(lgp_ppm);
      Serial.print(", ");
      ReadString.remove(0, pos + 1);
      events.send(lgp_ppm, "lpg", millis());

      // co ppm
      pos = ReadString.indexOf(',');
      char co_ppm[pos + 2]; // declare character array UTC_time with a size of the dbit of data
      for (i = 0; i < pos; i++)
      {
        co_ppm[i] = ReadString.charAt(i);
      }
      co_ppm[i] = '\0';
      Serial.print(co_ppm);
      Serial.print(", ");
      ReadString.remove(0, pos + 1);
      events.send(co_ppm, "co", millis());
      
      // nh4 ppm
      pos = ReadString.indexOf(',');
      char nh4_ppm[pos + 2]; // declare character array UTC_time with a size of the dbit of data
      for (i = 0; i < pos; i++)
      {
        nh4_ppm[i] = ReadString.charAt(i);
      }
      nh4_ppm[i] = '\0';
      Serial.print(nh4_ppm);
      ReadString.remove(0, pos + 1);
      events.send(nh4_ppm, "nh4", millis());

    //   // nh4
    //   pos = ReadString.indexOf(',');
    //   char nh4_ppm[pos + 2]; // declare character array UTC_time with a size of the dbit of data
    //   for (i = 0; i < pos; i++)
    //   {
    //     nh4_ppm[i] = ReadString.charAt(i);
    //   }
    //   nh4_ppm[i] = '\0';
    //   Serial.print(nh4_ppm);
    } else if (ReadString.startsWith('w')) {
          events.send("detected","warning",millis());

    }
  }
  if ((millis() - lastTime) > timerDelay)
  {
    getSensorReadings();
    // Serial.printf("Temperature = %.2f ºC \n", temperature);
    // Serial.printf("Humidity = %.2f \n", humidity);
    Serial.print("Time = ");
    Serial.print(currentTime);
    Serial.println();

    // Send Events to the Web Server with the Sensor Readings
    // events.send("ping", NULL, millis());
    // events.send(String(temperature).c_str(),"temperature",millis());
    // events.send(String(humidity).c_str(),"humidity",millis());
    // events.send(String(pressure).c_str(),"pressure",millis());
    // events.send(String(currentTime).c_str(), "time", millis());

    lastTime = millis();
  }
}


