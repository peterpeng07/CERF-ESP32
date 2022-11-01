#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define LDR 34
#define THRESHOLD 5
int sensor;
bool status;
bool prev_status;
int count = 0;

// Wifi Credentials
const char *ssid = "36";          
const char *password = "okemos36"; 

const char *data_server = "http://cerfdataservice.pythonanywhere.com/";
WebServer server(80);   // local web server
WiFiUDP ntpUDP;   // NTP client for getting time 
NTPClient timeClient(ntpUDP);

String timeStamp;

void connect_to_Wifi()
{
  Serial.print("Connecting to ");
  Serial.print(ssid);

  // connect to your local wi-fi network
  WiFi.begin(ssid, password);

  // check wi-fi is connected to wi-fi network
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  timeClient.begin();   // initialize time server
  timeClient.setTimeOffset(-14400);   // offset timezone
}

void start_server()
{
  server.on("/", handle_Home);
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP server started!");
}
void handle_Home()
{
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println("Local Server: Accessing home page...");
  server.send(200, "text/html", SendHTML(status));
}

void handle_NotFound()
{
  Serial.println("Local Server: 404 Not found");
  server.send(404, "text/plain", "Not found");
}

void post_log()
{
  if (WiFi.status() == WL_CONNECTED) {
    timeClient.update();
    timeStamp = String(timeClient.getEpochTime());
    Serial.println(timeStamp);    

    Serial.println("Log: Posting log...");
    HTTPClient http;
    http.begin(data_server);
    http.addHeader("Content-Type", "application/json");
    String httpRequestData = "{\"time\": \"" + timeStamp + "\", ";
    if (status) {
      httpRequestData += "\"status\": \"on\"}";
    } else {
      httpRequestData += "\"status\": \"off\"}";
    }
    int httpResponseCode = http.POST(httpRequestData);
    Serial.print("Log: HTTP Response code: ");
    Serial.println(httpResponseCode);
    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }
}

void monitor()
{
  sensor = 4095 - analogRead(LDR);
  // Serial.println(sensor);

  if (sensor > 3500)
  {
    if (!status) {
      count++;
    } else {
      status = false;
      count = 0;
    }
    digitalWrite(LED_BUILTIN, LOW);
  }
  else {
    if (status) {
      count++;
    } else {
      status = true;
      count = 0;
    }
    digitalWrite(LED_BUILTIN, HIGH);
  }

  if (count > THRESHOLD) {
    if (status != prev_status) {
      if (status) {
        Serial.println("Status: light is ON");
      } else {
        Serial.println("Status: light is OFF");
      } 
      post_log();
      prev_status = status;
    }
    count = 0;
  }
}

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  delay(100);

  connect_to_Wifi();
  start_server();

  sensor = 4095 - analogRead(LDR);
  if (sensor > 3500) {
    prev_status = true;
    status = false;
  } else {
    prev_status = false;
    status = true;
  }
  post_log();
  prev_status = status;
}

void loop()
{
  server.handleClient();

  monitor();

  server.send(200, "text/html", SendHTML(status));

  delay(1000);
}

String SendHTML(bool status)
{
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr += "<title>CERF</title>\n";
  ptr += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr += "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr += ".button {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr += ".button-on {background-color: #3498db;}\n";
  ptr += ".button-on:active {background-color: #2980b9;}\n";
  ptr += ".button-off {background-color: #34495e;}\n";
  ptr += ".button-off:active {background-color: #2c3e50;}\n";
  ptr += "p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr += "</style>\n";
  ptr += "</head>\n";
  ptr += "<body>\n";
  ptr += "<h1>CERF Light Sensor</h1>\n";
  ptr += "<h3>Location: </h3>\n";
  if (status) {
    ptr += "<h3>ON</h3>\n";
  } else {
    ptr += "<h3>OFF</h3>\n";
  }
  ptr += "</body>\n";
  ptr += "</html>\n";
  return ptr;
}