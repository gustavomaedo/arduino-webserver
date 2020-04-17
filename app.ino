#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

MDNSResponder mdns;
ESP8266WebServer server(80);

const char* ssid = "Arduino_WiFi";
const char* password = "password";
String st;
String content;

int gpio13Led = 13;
int gpio12Relay = 12;

void setup() {

  pinMode(gpio13Led, OUTPUT);
  digitalWrite(gpio13Led, LOW);
  
  pinMode(gpio12Relay, OUTPUT);
  digitalWrite(gpio12Relay, HIGH);
  
  Serial.begin(115200);
  EEPROM.begin(512);
  delay(10);
  Serial.println();
  Serial.println();
  Serial.println("Startup");
  // read eeprom for ssid and pass
  Serial.println("Reading ssid...");
  String esid;
  for (int i = 0; i < 32; ++i)
    {
      esid += char(EEPROM.read(i));
    }
  Serial.print("SSID: ");
  Serial.println(esid);
  Serial.println("Reading password...");
  String epass = "";
  for (int i = 32; i < 96; ++i)
    {
      epass += char(EEPROM.read(i));
    }
  Serial.print("PASSWORD: ");
  Serial.println(epass);  
  if ( esid.length() > 1 ) {
      // test esid 
      WiFi.begin(esid.c_str(), epass.c_str());
      if (testWifi()) { 
          launchWeb(0);
          return;
      }
  }
  setupAP(); 
}

bool testWifi(void) {
  int c = 0;
  Serial.println("Waiting for Wifi to connect");  
  while ( c < 20 ) {
    if (WiFi.status() == WL_CONNECTED) { return true; } 
    delay(500);
    Serial.print(WiFi.status());    
    c++;
  }
  Serial.println("");
  Serial.println("Connect timed out, opening AP");
  return false;
} 

void launchWeb(int webtype) {
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("SoftAP IP: ");
  Serial.println(WiFi.softAPIP());
  if (!mdns.begin("esp8266", WiFi.localIP())) {
    Serial.println("Error setting up MDNS responder!");
    while(1) { 
      delay(1000);
    }
  }
  Serial.println("mDNS responder started"); 
  createWebServer(webtype);
  // Start the server
  server.begin();
  Serial.println("Server started"); 
}

void setupAP(void) {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
    Serial.println("no networks found");
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
     {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*");
      delay(10);
     }
  }
  Serial.println(""); 
  st = "<ol>";
  for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      st += "<li>";
      st += WiFi.SSID(i);
      st += " (";
      st += WiFi.RSSI(i);
      st += ")";
      st += (WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*";
      st += "</li>";
    }
  st += "</ol>";
  delay(100);
  WiFi.softAP(ssid, password);
  Serial.println("Room WiFi");
  launchWeb(1);
  Serial.println("over");
}

void createWebServer(int webtype)
{
  // Check for any mDNS queries and send responses
  mdns.update();
  
  if ( webtype == 1 ) {
    server.on("/", []() {
        IPAddress ip = WiFi.softAPIP();
        String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
        content = "<!DOCTYPE HTML>\r\n<html>Room at ";
        content += ipStr;
        content += "<p>";
        content += st;
        content += "</p><form method='get' action='setting'><label>SSID: </label><input name='ssid' length=32><input name='pass' length=64><input type='submit'></form>";
        content += "<p>";
        content += "<p>";
        content += "<p>";
        content += "</html>";
        server.send(200, "text/html", content);  
    });
    server.on("/setting", []() {
        String qsid = server.arg("ssid");
        String qpass = server.arg("pass");
        if (qsid.length() > 0 && qpass.length() > 0) {
          Serial.println("clearing settings");
          for (int i = 0; i < 96; ++i) { EEPROM.write(i, 0); }
          Serial.println(qsid);
          Serial.println("");
          Serial.println(qpass);
          Serial.println("");
            
          Serial.println("writing ssid:");
          for (int i = 0; i < qsid.length(); ++i)
            {
              EEPROM.write(i, qsid[i]);
              Serial.print("Wrote: ");
              Serial.println(qsid[i]); 
            }
          Serial.println("writing password:"); 
          for (int i = 0; i < qpass.length(); ++i)
            {
              EEPROM.write(32+i, qpass[i]);
              Serial.print("Wrote: ");
              Serial.println(qpass[i]); 
            }    
          EEPROM.commit();
          content = "<!DOCTYPE HTML>\r\n<html>";
          content += "<p>Please, reset to connect to new WiFi.</p></html>";
        } else {
          content = "Error";
          Serial.println("Sending 404");
        }
        server.send(200, "text/html", content);
    });
  } else {

    server.on("/", [](){
      content = "";
      if(digitalRead(gpio12Relay) == HIGH) {
        content += "<h1>Room</h1><p><a href=\"off\"><button>OFF</button></a></p>";
      } else {
        content += "<h1>Room</h1><p><a href=\"on\"><button>ON</button></a></p>";
      }
      server.send(200, "text/html", content);
    });
    
    server.on("/on", [](){
      content = "";
      content += "<h1>Room</h1><p><a href=\"off\"><button>OFF</button></a></p>";
      server.send(200, "text/html", content);
      digitalWrite(gpio13Led, LOW);
      digitalWrite(gpio12Relay, HIGH);
      delay(1000);
    });
    
    server.on("/off", [](){
      content = "";
      content += "<h1>Room</h1><p><a href=\"on\"><button>ON</button></a></p>";
      server.send(200, "text/html", content);
      digitalWrite(gpio13Led, HIGH);
      digitalWrite(gpio12Relay, LOW);
      delay(1000); 
    });
    
    server.on("/clear", []() {
      content = "<!DOCTYPE HTML>\r\n<html>";
      content += "<p>Clearing the settings...</p></html>";
      content += "<p>Please, reset to configure new WiFi.</p></html>";
      server.send(200, "text/html", content);
      Serial.println("clearing settings");
      for (int i = 0; i < 96; ++i) { EEPROM.write(i, 0); }
      EEPROM.commit();
    });
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  server.handleClient();
  if (WiFi.status() != WL_CONNECTED) {
      digitalWrite(gpio13Led, HIGH);
      delay(1000); 
      digitalWrite(gpio13Led, LOW);
      delay(1000); 
  }
}
