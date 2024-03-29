#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <Hash.h>
#include <FS.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>

AsyncWebServer server(80);

int cutoff;
int pitlane;
int tps;
int i=0;
int j=0;
int dly;
int us=D6;
int ds=D8;
int pt=D7;
int re=D5;
int tp=D0;
float tpp;
float th;

const char* ssid_ap = "MisterShifter";
const char* password_ap = "123456789";

const char* kill = "cutoff";
const char* pit = "pitlane";
const char* TP = "tps"; 
const char* late = "delay"; 


void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}
String readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\r\n", path);
  File file = fs.open(path, "r");
  if(!file || file.isDirectory()){
    Serial.println("- empty file or failed to open file");
    return String();
  }
  Serial.println("- read from file:");
  String fileContent;
  while(file.available()){
    fileContent+=String((char)file.read());
  }
  file.close();
  Serial.println(fileContent);
  return fileContent;
}

void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\r\n", path);
  File file = fs.open(path, "w");
  if(!file){
    Serial.println("- failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }
  file.close();
}


String processor(const String& var){
  
  if(var == "cutoff"){
    return readFile(SPIFFS, "/cutoff.txt");
  }
  else if(var == "pitlane"){
    return readFile(SPIFFS, "/pitlane.txt");
  }
  else if(var == "delay"){
    return readFile(SPIFFS, "/delay.txt");
  }
    else if(var == "tps"){
    return readFile(SPIFFS, "/tps.txt");
  }
    else if(var == "wifi_sta"){
    return readFile(SPIFFS, "/wifi_sta.txt");
  }
    else if(var == "pass_sta"){
    return readFile(SPIFFS, "/pass_sta.txt");
  }
 
  return String();
}
void setup() {
 if(!SPIFFS.begin()){
     Serial.println("An Error has occurred while mounting SPIFFS");
     return;
    }

  
  //WIFI AP
  WiFi.softAP(ssid_ap, password_ap);
  //WIFI STA
  WiFi.begin(ssid_sta, password_sta);  


  
   server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
   server.on("/Tpp", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(tpp).c_str());
  });
 
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;    

  if (request->hasParam(kill)) {
      inputMessage = request->getParam(kill)->value();
      writeFile(SPIFFS, "/cutoff.txt", inputMessage.c_str());
  }
 else if (request->hasParam(pit)) {
      inputMessage = request->getParam(pit)->value();
      writeFile(SPIFFS, "/pitlane.txt", inputMessage.c_str());
  }
  else if (request->hasParam(late)) {
      inputMessage = request->getParam(late)->value();
      writeFile(SPIFFS, "/delay.txt", inputMessage.c_str());
  }
  else if (request->hasParam(TP)) {
      inputMessage = request->getParam(TP)->value();
      writeFile(SPIFFS, "/tps.txt", inputMessage.c_str());
  }          
  else {
      inputMessage = "No message sent";
  }  
  request->send(200, "text/text", inputMessage);
  });
  server.onNotFound(notFound);
  AsyncElegantOTA.begin(&server);
  server.begin();

  pinMode(us, INPUT_PULLUP);
  pinMode(ds, INPUT_PULLUP);
  pinMode(pt, INPUT_PULLUP);
  pinMode(re, OUTPUT);
  pinMode(tp, OUTPUT);
  digitalWrite(re, LOW);
}

void loop() {
  cutoff= readFile(SPIFFS, "/cutoff.txt").toInt();
  pitlane = readFile(SPIFFS, "/pitlane.txt").toInt();
  dly = readFile(SPIFFS, "/delay.txt").toInt();
  tps = readFile(SPIFFS, "/tps.txt").toInt();
  tpp = analogRead(A0);
    
int snsrstate=digitalRead(us);
  if (snsrstate==HIGH) {
    i=0;
    delay(50);
  }
  if (snsrstate==LOW && i==0) {
    
    i+=1;
    digitalWrite(re, HIGH);
    analogWrite(tp, tps);
    delay(cutoff);
    analogWrite(tp, 0);
    digitalWrite(re, LOW);
    delay(0);
  }
 
  
}
