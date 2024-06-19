#include <WiFi.h>
#include <TelnetClient.h>

//////////////////////////////////////////////////////////////////////
// Use Putty as serial terminal for better output with this example //
//////////////////////////////////////////////////////////////////////


//please enter your sensitive data in the Secret tab
char ssid[] = "xxxxxxxxx";               // your network SSID (name)
char pass[] = "xxxxxxxxx";               // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;             // the Wi-Fi radio's status


// Put here your telnet's ip address or hostname
const char* telnetHost = "towel.blinkenlights.nl" ;
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

  // no user login needed for this server
  if (!telnet.connect()) {
    Serial.println("No telnet connection");
    while (1);
  }

}

void loop () {
  telnet.listen();
}
