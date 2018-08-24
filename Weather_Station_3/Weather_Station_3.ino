#include <ESP8266WiFi.h> //Выводит 3 состояния погоды, температуру или ошибку
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

char key = '3';
const char* ssid     = "golovinka";
const char* password = "11111111";
const String server  = "api.openweathermap.org";
const String lon     = "36.80";
const String lat     = "55.19";
const String appid   = "23dfdb6c46a746011182b406b75dc521";
const String url     = "http://" + server + "/data/2.5/weather?lat=" + lat + "&lon=" + lon + "&units=metric&appid=" + appid;

unsigned long lastConnectionTime = 0;
unsigned long postingInterval = 0;
String httpData;
struct weather_structure {
  const char* main;
  int temp;
};
weather_structure weather;

void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(2000);
  }
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    if (millis() < lastConnectionTime) lastConnectionTime = 0;
    if ((millis() - lastConnectionTime) > postingInterval || lastConnectionTime == 0) {
      if (httpRequest() && parseData()) {
        PrintNumber(weather.main);
        Serial.print("PRINT");
        Serial.print(key);
        Serial.print(weather.temp);
      }
    }
  }
}

bool httpRequest() {
  HTTPClient client;
  bool find = false;
  client.begin(url);
  int httpCode = client.GET();

  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
      httpData = client.getString();
      if (httpData.indexOf(F("\"main\":{\"temp\":")) > -1) {// Если есть main и temp, то идем дальше
        lastConnectionTime = millis();
        find = true;
      }
      //Если ничего не нашли, то выводим 9 в монитор порта
    }
  }

  postingInterval = find ? 600L * 1000L : 60L * 1000L;
  client.end();

  return find;
}

bool parseData() {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(httpData);

  if (!root.success()) {
    return false;
  }
  weather.main = root["weather"][0]["main"];
  weather.temp = root["main"]["temp"];
  httpData = "";
  return true;
}

void PrintNumber(const char* character) {
  Serial.println(character);
  if (character[0] == 'T' || character[0] == 'R' || character[0] == 'D' || character[0] == 'S'){
    key = '0';
    return;
  }
  if (character[0] == 'C' && character[2] == 'e') {
    key = '1';
    return;
  }
  if (character[0] == 'A' || character[0] == 'C') {
    key = '2';
    return;
  }
}
