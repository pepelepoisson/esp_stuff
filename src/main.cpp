/*------------------------------------------------------------------------------
  07/01/2018
  Author: Makerbro
  Platforms: ESP8266
  Language: C++/Arduino
  File: webserver_html.ino
  ------------------------------------------------------------------------------

*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>

#include <wifi_credentials.h>  // wifi ssid and password
#include <HC-SR04_sensor.h>  // All functions related to HC-SR04 ultrasonic sensor processing

ESP8266WebServer server;
WebSocketsServer webSocket = WebSocketsServer(81);

#define PIN_LED 2

char webpage[] PROGMEM = R"=====(
<html>
<head>
  <script>
    var Socket;
    var gvalue;
    function init() {
      Socket = new WebSocket('ws://' + window.location.hostname + ':81/');
      Socket.onmessage = function(event){
        if (gvalue!=event.data){
          var obj=JSON.parse(event.data);
          gvalue=obj.passages;
          document.getElementById("rxConsole1").value = gvalue;
          gvalue=obj.distance;
          document.getElementById("rxConsole2").value = gvalue;
        }
      }
    }
  </script>
</head>
<body onload="javascript:init()">
  <div>
  <p><strong>Nombre de passages: </strong></p>
  <textarea id="rxConsole1"></textarea>
  </div>
  <div>
  <p><strong>Distance: </strong></p>
  <textarea id="rxConsole2"></textarea>
  </div>
  <hr/>
</body>
</html>
)=====";


void setup()
{
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(PIN_LED, OUTPUT);
  WiFi.begin(ssid,password);
  Serial.begin(115200);
  while(WiFi.status()!=WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/",[](){
    server.send_P(200, "text/html", webpage);
  });
  server.begin();
  webSocket.begin();
  current_distance=0;
  previous_distance=0;
for (int i=0; i<=20; i++){GetDistance();}  // Get distance at start (assuming no obstacles visible)
}

void loop()
{
  GetDistance();
  CheckForObstacles();
  webSocket.loop();
  server.handleClient();

  Serial.print(distance_short_sample.getAverage()); Serial.print(",");
  Serial.print(distance_long_sample.getAverage()); Serial.print(",");
  Serial.print(obstacle_detected); Serial.print(",");
  Serial.print(no_obstacle_detected); Serial.print(",");
  Serial.print(confirmed_passages); Serial.println(",");


DynamicJsonBuffer jBuffer;
JsonObject& root = jBuffer.createObject();

root["distance"]=distance_long_sample.getAverage();
root["passages"]=confirmed_passages;

//root.prettyPrintTo(Serial);
String output_message;
root.printTo(output_message);
webSocket.broadcastTXT(output_message);
//serializeJson(root, output_message);

//root.prettyPrintTo(webSocket);
//webSocket.broadcastTXT(root);


  /*if(distance<10){
    char c[] = "y";
    webSocket.broadcastTXT(c, sizeof(c));
  }*/

  if(Serial.available() > 0){
    char c[] = {(char)Serial.read()};
    webSocket.broadcastTXT(c, sizeof(c));
  }
}
