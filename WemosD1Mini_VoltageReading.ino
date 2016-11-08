/*LED_Breathing.ino Arduining.com  20 AUG 2015
Using NodeMCU Development Kit V1.0
Going beyond Blink sketch to see the blue LED breathing.
A PWM modulation is made in software because GPIO16 can't
be used with analogWrite().
*/

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>

// start telnet server (do NOT put in setup())
const uint16_t aport = 23; // standard port
WiFiServer TelnetServer(aport);
WiFiClient TelnetClient;

// replace with your channel’s thingspeak API key and your SSID and password
String apiKey = "M9DXXB9MJRCW0OJ8";
const char* ssid = "AZ_ZAHRA_PELNI"; //"My ASUS"; "AZ_ZAHRA_PELNI";
const char* password = "azzahra4579";
const char* server = "api.thingspeak.com";

WiFiClient client;

#define LED       D8      // Led in NodeMCU at pin GPIO16 (D0).
 
#define BRIGHT    350     //max led intensity (1-500)
#define INHALE    1250    //Inhalation time in milliseconds.
#define PULSE     INHALE*1000/BRIGHT
#define REST      300000    //Rest Between Inhalations.

#define APin      A0

float AVal = 0;
float RVolt = 3.4; //3.3 <= 5V | 3.35 <= 7V | 3.4 <= 15V
float RBit = 1023;
float Vi, Vo = 0;
float R1 = 10080;
float R2 = 987;
int sample_max = 8192; //10bit=2^10=1024, 13bit=2^13=8192
float sample_sum = 0;


//----- Setup function. ------------------------
void setup() {      
  Serial.begin(9600);        
  TelnetServer.begin();
  TelnetServer.setNoDelay(true);
  Serial.println();
  Serial.println();
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

//----- Loop routine. --------------------------
void loop() {
  for(int i=0; i<sample_max; i++){
    sample_sum += analogRead(APin);
    delay(0);
  }
  AVal = sample_sum/sample_max;
  Vo = (AVal*RVolt)/RBit;
  Vi = Vo/(R2/(R1+R2));
  sample_sum = 0; 
  Serial.print("AVG :  ");
  Serial.print(AVal);
  Serial.print("  ");
  Serial.print(Vo);
  Serial.print("  ");
  Serial.print(Vi);
  Serial.println(" V");
  Serial.println();

  update_cloud(Vi);
  
  delay(REST);

//  Serial.println("ON");
//  digitalWrite(LED, HIGH);
//  delay(1000);
//  Serial.println("OFF");
//  digitalWrite(LED, LOW);
//  delay(5000);

  //ramp increasing intensity, Inhalation: 
//  Serial.println("OFF");
//  for (int i=1;i<BRIGHT;i++){
//    digitalWrite(LED, LOW);          // turn the LED on.
//    delayMicroseconds(i*10);         // wait
//    digitalWrite(LED, HIGH);         // turn the LED off.
//    delayMicroseconds(PULSE-i*10);   // wait
//    delay(0);                        //to prevent watchdog firing.
//  }
  
  //ramp decreasing intensity, Exhalation (half time):
//  Serial.println("ON");
//  for (int i=BRIGHT-1;i>0;i--){
//    digitalWrite(LED, LOW);          // turn the LED on.
//    delayMicroseconds(i*10);          // wait
//    digitalWrite(LED, HIGH);         // turn the LED off.
//    delayMicroseconds(PULSE-i*10);  // wait
//    i--;
//    delay(0);                        //to prevent watchdog firing.
//  }
//  delay(REST);                       //take a rest...
}

void Debug(String output) {
  if (!TelnetClient)  // otherwise it works only once
        TelnetClient = TelnetServer.available();
  if (TelnetClient.connected()) {
    TelnetClient.println(output);
  }  
}

void update_cloud(int val){
  Serial.print("Update to Cloud => voltage: " + String(val));
  Debug("Update to Cloud => voltage: " + String(val));
  if (client.connect(server,80)) {
    String postStr = apiKey;
    postStr +="&field1="; postStr += String(val);
    postStr += "\r\n\r\n";
    
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);

    Serial.println(" (Success)");
    Debug(" (Success)");
  } else {
    Serial.println(" (Failed)");
    Debug(" (Failed)");
  }
  client.stop();
}
