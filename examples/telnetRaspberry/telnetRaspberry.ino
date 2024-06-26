#include <WiFi.h>
#include <TelnetClient.h>

//please enter your sensitive data in the Secret tab
char ssid[] = "xxxxxxxx";                // your network SSID (name)
char pass[] = "xxxxxxxx";                // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;             // the Wi-Fi radio's status


// Put here your telnet's ip address or hostname
IPAddress   telnetHost(192, 168, 1, 22);
const int   telnetPort = 23;

WiFiClient baseClient;
TelnetClient telnet(baseClient, telnetHost, telnetPort);

void setup () {

  Serial.begin(115200);
  while (!Serial);

  Serial.println("******************************************************");
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.print("\nWiFi connected, IP address: ");
  Serial.println(WiFi.localIP());

  // WICH CHARACTER SHOULD BE INTERPRETED AS "PROMPT"?
  telnet.setPromptChar('$');

  // PUT HERE YOUR USERNAME/PASSWORD
  if(telnet.login("pi", "raspberry")) {
    telnet.sendCommand("ls -l");
    telnet.sendCommand("ifconfig");
  }
  else{
    Serial.println("login failed");
  }

}

void loop () {

  // Use serial input as telnet user prompt
  while (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    telnet.sendCommand(cmd.c_str());
  }

}
