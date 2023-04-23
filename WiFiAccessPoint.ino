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

void setup()
{
  delay(1000);

  Serial.begin(115200);

  Serial.println();
}

void loop()
{
  waitForWiFiCredentials();

  handleClient([](WiFiClient client, String method, String url) {
    client.print(render("<html><body>You came here</body></html>"));
  });
}

void waitForWiFiCredentials()
{
  if (wifissid.length() != 0) {
    return;
  }

  Serial.println("Switch to the AP mode");

  WiFi.softAP(apssid, appassword);

  Serial.print("AP IP address: ");

  Serial.println(WiFi.softAPIP());

  server.begin();

  Serial.println("Server is listening");

  while(true) {
    handleClient(responseAP);
  }
}

void responseAP(WiFiClient client, String method, String url) {
  Serial.print(method);
  Serial.print(url);

  client.print(render(
    "<body>"
      "<form action=\"/submit\">"
        "<h1>WiFi 설정</h1>"
        "<label>SSID<br/><input id=\"ssid\" name=\"ssid\" type=\"text\" /></label><br />"
        "<label>Password<br/><input id=\"password\" name=\"password\" type=\"password\" /></label><br />"
        "<input type=\"submit\" />"
      "</form>"
    "</body>"
  ));
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

void handleClient(void (*callback)(WiFiClient, String, String)) {
  WiFiClient client = server.accept();

  if (!client) {
    return;
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

  callback(client, method, url);
}


