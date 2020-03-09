#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>

/* Put your SSID & Password */
const char* ssid = "NodeMCU";  // Enter SSID here
const char* password = "12345678";  //Enter Password here

/* Put IP Address details */
IPAddress local_ip(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

ESP8266WebServer server(80);

const int LED1 = D0;
const int LED2 = D5;

String current_state = "2";
String required_state;

void setup() {
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);
  server.on("/", handle_OnConnect);
  server.on("/configure_inputs", configure_inputs);
  server.begin();
  Serial.println("Local HTTP server started @ " + local_ip);
}

void handle_OnConnect() {
  Serial.println("Inside OnConnect");
  String f = "<form action='/configure_inputs'>";
  f += "<label for='ssid'>SSID:</label>";
  f += "<input type='text' id='ssid' name='ssid'><br><br>";
  f += "<label for='password'>Password:</label>";
  f += "<input type='text' id='password' name='password'><br><br>";
  f += "<input type='submit' value='Submit'>";
  f += "</form>";
  server.send(200, "text/html", f);
}

void configure_inputs() {
  Serial.println("Inside Configure Inputs");
  String ssid = server.arg("ssid");
  String password = server.arg("password");
  Serial.println("ssid and password are .... ");
  Serial.println(ssid);
  Serial.println(password);

  String f = "Connecting to SSID - ";
  f += ssid;
  //server.send(200, "text/html", f);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  server.send(200, "text/html", "Successfully Connected to SSID - " + ssid);
}

void loop() {
  server.handleClient();
  delay(5000);    //Send a request every 10 seconds
  Serial.println("Looping...");
  if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status
    HTTPClient http;  //Declare an object of class HTTPClient
    Serial.println("Hitting Get Request...");
    http.begin("http://192.168.1.7:8000/check_state/");
    int httpCode = http.GET();                                                                  //Send the request
    if (httpCode > 0) { //Check the returning code
      String required_state = http.getString();   //Get the request response payload
      Serial.println("Required State - ");
      Serial.print(required_state);                     //Print the response payload
      if (required_state != current_state) {
        Serial.println("Desired State is Not Equal to Required State...");
        if (required_state.equals("1")) {
          Serial.println("Setting State 1");
          http.begin("curl -XGET http://192.168.1.7:8000/set_state/on/");
        }
        else if (required_state.equals("0")) {
          Serial.println("Setting State 0");
          http.begin("curl -XGET http://192.168.1.7:8000/set_state/off/");
        }
        else {
          Serial.println("Check Code. BUG FOUND - Required State should be 1 or 0 \n");
          Serial.print("But Required State is - ");
          Serial.print(required_state);
          return;
        }
        int httpCode = http.GET();
        if (httpCode > 0) { //Check the returning code
          if (required_state.equals("1")) {
            Serial.println("Required state set to HIGH success");
            digitalWrite(LED2, HIGH);
          }
          else if (required_state.equals("0")) {
            Serial.println("Required state set to LOW success");
            digitalWrite(LED2, LOW);
          }
          else{
            Serial.println("Error..");
            return;
            }
          current_state = required_state;
          Serial.print("State Changed Successfully...\n");
        }
       else{
        Serial.println("Failed to hit get request to set the state...");
        }
      }
      else {
        Serial.println("Required State is Same as Current State");
      }
    }
    else {
      http.end();
      Serial.print("Unable to Fetch Required State");
      return;
    }
    http.end();   //Close connection
  }
  else {
    Serial.print("Not Connected.....");
  }
  Serial.println("==================");

}
