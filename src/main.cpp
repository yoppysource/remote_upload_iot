#include <Arduino.h>
#include <ArduinoHttpClient.h>
#include <WiFiNINA.h>
#include <ArduinoJson.h>
#include <arduino_secrets.h>
#include <endpoint_generator.h>
#include <Time.h>
#include <TimeAlarms.h>
#include <constants.h>

WiFiServer server(80); 
WiFiClient wifiClient = server.available();
HttpClient httpClient = HttpClient(wifiClient, SECRET_HOST_NAME, SECRET_HTTP_PORT);
EndpointGenerator endpointGenerator;

int status = WL_IDLE_STATUS;

// initialize setting value
// assign value if there was an error for initializing
// TODO: 더 나은 이니셜라이즈 에러 보여줄 방법 찾아내기
String planterId = SECRET_PLANTER_ID;
double ec = 1.0;
char* turnOn = "08:00";
char* turnOff = "23:00";
char* _id;

void connectToServer() {
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(SECRET_SSID);
    status = WiFi.begin(SECRET_SSID, SECRET_PASSWORD);
    delay(10000);
  }
  // WiFi.config(IPAddress(192,168,0,1));
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  IPAddress ip = WiFi.localIP();
  IPAddress gateway = WiFi.gatewayIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
}

void initializeArduinoFromDatabase() {
  StaticJsonDocument<200> jsonDoc;
  Serial.println("making Get request");
  httpClient.get(endpointGenerator.getInitPath());

  int statusCode = httpClient.responseStatusCode();
  String body = httpClient.responseBody();
  Serial.print("Status code: ");
  Serial.println(statusCode);
  Serial.print("Response: ");
  Serial.println(body);

  DeserializationError error = deserializeJson(jsonDoc, body);

  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());

    return;
  }
  ec = jsonDoc["setting"]["ec"];
  turnOn = jsonDoc["setting"]["turnOn"];
  turnOff = jsonDoc["setting"]["turnOff"];
  _id = jsonDoc["_id"];
}


void handleRequestFromServer() { 
    StaticJsonDocument<200> jsonDoc;                           
    Serial.println("new wifiClient");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the wifiClient
    while (wifiClient.connected()) {            // loop while the wifiClient's connected
      if (wifiClient.available()) {             // if there's bytes to read from the wifiClient,
        char c = wifiClient.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character
          if (currentLine.length() == 0) {
            // HTTP request는 /n으로 끝난다 따라서, 만약 c가 newline인데 currentLine이 존재하지 않는다면,
            // 모든 리퀘스트 문자를 다 받았다고 전제하고 반복문에서 빠져나간다.
            break;
          }
          else {
            // 새로운 /n으로 끝났으면 새로운 줄이라고 인식해서 currentLine을 비워준다
            currentLine = "";
          }
        }
        else if (c != '\r') {
          // carriage return character가 아니라면, 더해준다
          currentLine += c;
        }
        if (currentLine.endsWith("GET /monitor")) {
          //TODO: insert monior value
          jsonDoc["ec"] = ec;
          jsonDoc["planter"] = _id;
          jsonDoc["planterId"] = planterId;
          String payload;
          serializeJson(jsonDoc, payload);
          Serial.println("Sending response");
          // send a standard http response header
          wifiClient.println("HTTP/1.1 200 OK");
          wifiClient.println("Content-type:application/json");
          wifiClient.println(); 
          wifiClient.print(payload);      
          wifiClient.println();
        }
        if (currentLine.endsWith("POST /control")) {
          String body = "";
          bool isBody = false;
          int cnt = 0;
          while(wifiClient.connected() && cnt < 10000){
          cnt++;
          char c = wifiClient.read();
          if(c == '{') isBody = true;
          if(isBody && c != '/n' && c != '/r') body += c;
          if(c == '}') break;
          }
          Serial.println(body);
          DeserializationError error = deserializeJson(jsonDoc, body);

          // Test if parsing succeeds.
          if (error) {
          Serial.print(F("deserializeJson() failed: "));
          Serial.println(error.c_str());
          wifiClient.println("HTTP/1.1 401 invaild request");
          wifiClient.println();
          break;
          } else {
          if(jsonDoc.containsKey("ec")) ec = jsonDoc["ec"];
          if(jsonDoc.containsKey("turnOn"))turnOn = jsonDoc["turnOn"];
          if(jsonDoc.containsKey("turnOff")) turnOff = jsonDoc["turnOff"];
          Serial.println(ec);
          Serial.println(turnOn);
          Serial.println(turnOff);

          String payload;
          jsonDoc["ec"] = ec;
          jsonDoc["turnOn"] = turnOn;
          jsonDoc["turnOff"] =  turnOff;  
          serializeJson(jsonDoc, payload);
          Serial.println("Sending response");
          wifiClient.println("HTTP/1.1 200 OK");
          wifiClient.println("Content-type:application/json");
          wifiClient.println(); 
          wifiClient.print(payload);
          wifiClient.println();
          break;
          }
        }
      }
    }
    // close the connection:
    wifiClient.stop();
    Serial.println("client disconnected");
  }
  void helloFriday() {
    Serial.println("hello this is friday!");
  }

void setup() {
  Serial.begin(9600);
  while(!Serial);
  Serial1.begin(9600);
  while(!Serial1);

  connectToServer();
  server.begin();
  initializeArduinoFromDatabase();
  Serial1.print("init from wifi!!\n");
  String docToUno;
  StaticJsonDocument<200> jsonDoc;
  jsonDoc["ec"] = ec;
  // jsonDoc["turnOn"] = turnOn;
  // jsonDoc["turnOff"] =  turnOff;  
  serializeJson(jsonDoc, docToUno);
  Serial1.print(docToUno + "\n");
  delay(1000);
  Serial.println(WiFi.getTime());
  time_t time = WiFi.getTime() + TIME_DIFFERENCE_FROM_UTC;
 setTime(time);

  Serial.println(hour());
  Serial.println(minute());
  Serial.println(second());
    Serial.println(day());
  Serial.println(weekday());
  Serial.println(month());
  Serial.println(year());
  Alarm.alarmRepeat(hour(),minute()+1,0,helloFriday);  // 5:45pm every day




  //TODO: ec value를 슬레이브에 보내서 저장 시키기
}

void loop() {
  // 반드시 알람 라이브러리 쓸때는 써야된다... 얘가 프로그램 시작한 후 시간세서 알아서 던져주는 그런 역할임.
  Alarm.delay(0);

  Serial1.print("monitor\n");
  while(Serial1.available() >0) {
  String fromUno = Serial1.readStringUntil('\n');
  if(fromUno.endsWith("}")){
    Serial.println(fromUno);
    fromUno = "";
    }
  }
  
  delay(1000);

   wifiClient = server.available();
  if(wifiClient){
    handleRequestFromServer();
  }
  
  // put your main code here, to run repeatedly:
}