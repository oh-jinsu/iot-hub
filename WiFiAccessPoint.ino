#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#ifndef APSSID
#define APSSID "ESPap"
#define APPSK "thereisnospoon"
#endif

const char *apssid = APSSID;
const char *appassword = APPSK;

String wifissid = "";
String wifipassword = "";

WiFiServer server(3000);

struct HttpRequest {
  WiFiClient client;
  String method;
  String url;
};

void setup()
{
  delay(1000);

  Serial.begin(115200);

  Serial.println();
}

void loop()
{
  loopModeAP();

  loopModeWiFi();
}

void loopModeAP()
{
  Serial.println("Switch to the AP mode");

  WiFi.softAP(apssid, appassword);

  Serial.print("AP IP address: ");

  Serial.println(WiFi.softAPIP());

  server.begin();

  Serial.println("Server is listening");

  while(true) {
    HttpRequest request = getHttpRequest();

    if (!request.client) {
      continue;
    }

    if (request.url.startsWith("/submit")) {
      int seperatorIndex = request.url.indexOf("?");

      String queryString = request.url.substring(seperatorIndex + 1);

      String ssid = getQueryStringParameter(queryString, "ssid");

      String password = getQueryStringParameter(queryString, "password");

      if (ssid.isEmpty()) {
        request.client.print(render((
          "<body>"
            "<h1>You should provide SSID</h1>"
          "</body>"
        )));

        continue;
      }

      wifissid = ssid;

      wifipassword = password;

      request.client.print(render(
        "<body>"
          "<h1>WiFi credential has been set</h1>"
        "</body>"
      ));

      return;
    }

    request.client.print(render(
      "<body>"
        "<form action=\"/submit\">"
          "<h1>WiFi 설정</h1>"
          "<label>SSID<br/><input id=\"ssid\" name=\"ssid\" type=\"text\" required/></label><br />"
          "<label>Password<br/><input id=\"password\" name=\"password\" type=\"password\" /></label><br />"
          "<input type=\"submit\" />"
        "</form>"
      "</body>"
    ));
  }
}

void loopModeWiFi() {

}

String getQueryStringParameter(String from, String key) {
  int keyIndex = from.indexOf(key + "=");

  if (keyIndex == -1) {
    return "";
  }

  String leftOver = from.substring(keyIndex + key.length() + 1);

  int seperatorIndex = leftOver.indexOf("&");

  if (seperatorIndex == -1) {
    return leftOver;
  }

  return leftOver.substring(0, seperatorIndex);
}

String render(String inner) {
  String header =
    "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html lang=\"en\">"
      "<head>"
        "<meta charset=\"UTF-8\">"
        "<meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">"
        "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
        "<title>Document</title>"
      "</head>";

  String tail = "</html>";

  return header + inner + tail;
}

HttpRequest getHttpRequest() {
  WiFiClient client = server.accept();

  if (!client) {
    return (HttpRequest){ client, "", "" };
  }

  client.setTimeout(5000);  // default is 1000

  String req = client.readStringUntil('\r');

  Serial.print("Request: ");

  Serial.println(req);

  while (client.available()) {
    client.read();
  }

  int methodIndex = req.indexOf(" ");

  String method = req.substring(0, methodIndex);

  String leftOver = req.substring(methodIndex + 1);

  int urlIndex = leftOver.indexOf(" ");

  String url = leftOver.substring(0, urlIndex);

  return (HttpRequest){ client, method, url };
}


