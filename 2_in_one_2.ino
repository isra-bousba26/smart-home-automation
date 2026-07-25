#define BLYNK_TEMPLATE_ID "TMPL2EnFbL66t"
#define BLYNK_TEMPLATE_NAME "control"
#define BLYNK_AUTH_TOKEN "P2ph_y3LCQdKzw0wAyKweOzx6LeostCt"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Servo.h>

char auth[] = BLYNK_AUTH_TOKEN, ssid[] = "mmm", pass[] = "mmmmmmmm";
Servo win, gear;
const int led = 14, rainPin = A0; 
BlynkTimer timer;

// V1 Switch: 1 = 90 deg, 0 = 0 deg
BLYNK_WRITE(V1) { if(param.asInt()) win.write(90); else win.write(0); } 

// V2 Switch: 1 = 90 deg, 0 = 0 deg
BLYNK_WRITE(V2) { if(param.asInt()) gear.write(90); else gear.write(0); }

// V5 Switch: LED Control
BLYNK_WRITE(V5) { digitalWrite(led, param.asInt()); }

void updateRain() {
  int rawVal = analogRead(rainPin); // Get the 0-1024 value
  int rainPercent = map(rawVal, 1024, 0, 0, 100);
  
  // Print to Serial Monitor
  Serial.print("Raw Rain Value: ");
  Serial.print(rawVal);
  Serial.print(" | Percentage: ");
  Serial.println(rainPercent);

  Blynk.virtualWrite(V3, constrain(rainPercent, 0, 100)); 
}

void setup() {
  Serial.begin(115200); // Start serial communication
  pinMode(led, OUTPUT); 
  win.attach(5, 500, 2400);  // D1
  gear.attach(4, 500, 2400); // D2
  
  Blynk.begin(auth, ssid, pass);
  timer.setInterval(1000L, updateRain); // Check every 1 second
}

void loop() { 
  Blynk.run(); 
  timer.run(); 
}