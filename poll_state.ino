#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
const char* ssid = "aang1";  // Enter SSID here
const char* password = "air12345";  //Enter Password here

const int LED1 = D0;
const int LED2 = D5;
String current_state = "2";
String required_state;
void setup () {
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  digitalWrite(LED2, HIGH);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print("Connecting..\n");
  }
}


void loop() {
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
