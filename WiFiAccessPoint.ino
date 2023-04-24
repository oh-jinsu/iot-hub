#include <ESP8266WiFi.h>

#ifndef APSSID
#define APSSID "ESPap"
#define APPSK "thereisnospoon"
#endif

const char *apssid = APSSID;
const char *appassword = APPSK;

String wifissid = "";
String wifipassword = "";

WiFiServer server(80);

String serverip = "";
int serverport = 80;

WiFiClient client;

struct HttpRequest {
  WiFiClient client;
  String method;
  String url;
};

void setup()
{
  Serial.begin(115200);

  pinMode(D3, OUTPUT);

  pinMode(D4, OUTPUT);

  Serial.println();

  Serial.println("Arduino set up");
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

      String ip = getQueryStringParameter(queryString, "serverip");

      String port = getQueryStringParameter(queryString, "serverport");

      String ssid = getQueryStringParameter(queryString, "ssid");

      String password = getQueryStringParameter(queryString, "password");

      if (ip.isEmpty()) {
        request.client.print(render((
          "<body>"
            "<h1>You should provide Server IP</h1>"
          "</body>"
        )));

        continue;
      }

      if (ssid.isEmpty()) {
        request.client.print(render((
          "<body>"
            "<h1>You should provide SSID</h1>"
          "</body>"
        )));

        continue;
      }

      serverip = ip;

      serverport = port.toInt();

      wifissid = ssid;

      wifipassword = password;

      request.client.print(render(
        "<body>"
          "<h1>WiFi credential has been set</h1>"
          "<p>SSID: " + wifissid + "</p>"
        "</body>"
      ));

      request.client.flush();

      return;
    }

    request.client.print(render(
      "<body>"
        "<form action=\"/submit\">"
          "<h1>WiFi 설정</h1>"
          "<label>Server IP<br/><input id=\"serverip\" name=\"serverip\" type=\"text\" value=\"127.0.0.1\" required /></label><br />"
          "<label>Server Port<br/><input id=\"serverport\" name=\"serverport\" type=\"text\" /></label><br />"
          "<label>SSID<br/><input id=\"ssid\" name=\"ssid\" type=\"text\" required/></label><br />"
          "<label>Password<br/><input id=\"password\" name=\"password\" type=\"password\" /></label><br />"
          "<input type=\"submit\" />"
        "</form>"
      "</body>"
    ));
  }
}

void loopModeWiFi() {
  Serial.println("Switch to the WiFi Mode");

  WiFi.mode(WIFI_STA);

  WiFi.begin(wifissid, wifipassword);

  int count = 0;

  while (WiFi.status() != WL_CONNECTED) {
    if (count == 20) {
      return;
    }

    delay(1000);

    Serial.print(".");

    count++;
  }

  Serial.println();

  Serial.println("WiFi connected");

  Serial.println("Connecting to server");

  if(!client.connect(serverip, serverport)) {
    Serial.println("Failed to connect");

    return;
  }

  Serial.println("Server connected");

  while (true) {
    int count = 0;

    while (WiFi.status() != WL_CONNECTED) {
      if (count == 5) {
        Serial.println("WiFi disconnected");

        return;
      }

      delay(1000);

      count++;
    }

    if (!client.connected()) {
      Serial.println("Server disconnected");

      return;
    }

    if (!client.available()) {
      continue;
    }

    String response = client.readStringUntil('\r');

    Serial.print("Server response: ");

    Serial.println(response);

    if (response.startsWith("/left")) {
      digitalWrite(D3, HIGH);
      digitalWrite(D4, LOW);
    }

    if (response.startsWith("/right")) {
      digitalWrite(D3, LOW);
      digitalWrite(D4, HIGH);
    }
  }
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

HttpRequest getHttpRequest() {
  WiFiClient client = server.accept();

  if (!client) {
    return (HttpRequest){ client, "", "" };
  }

  client.setTimeout(5000);

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

  String url = urldecode(leftOver.substring(0, urlIndex));

  return (HttpRequest){ client, method, url };
}

String urldecode(String str)
{
  String encodedString="";
  char c;
  char code0;
  char code1;
  for (int i =0; i < str.length(); i++){
      c=str.charAt(i);
    if (c == '+'){
      encodedString+=' ';  
    }else if (c == '%') {
      i++;
      code0=str.charAt(i);
      i++;
      code1=str.charAt(i);
      c = (h2int(code0) << 4) | h2int(code1);
      encodedString+=c;
    } else{
      
      encodedString+=c;  
    }
    
    yield();
  }
  
  return encodedString;
}

unsigned char h2int(char c)
{
  if (c >= '0' && c <='9'){
      return((unsigned char)c - '0');
  }
  if (c >= 'a' && c <='f'){
      return((unsigned char)c - 'a' + 10);
  }
  if (c >= 'A' && c <='F'){
      return((unsigned char)c - 'A' + 10);
  }
  return(0);
}