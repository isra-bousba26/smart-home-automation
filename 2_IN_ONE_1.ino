#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

// --- RESTART POINTER ---
void(* resetFunc) (void) = 0; 

// Pins
#define DHTPIN 2
#define DHTTYPE DHT11
#define PIR_PIN 3
#define FLAME_PIN 4
#define BUZZER 5
#define FAN_PIN 6
#define PUMP_PIN 7
#define GAS_DIGITAL 8
#define LED_PIN 9    
#define LDR_PIN A0

// Logic Constants
#define ON LOW       
#define OFF HIGH
#define TEMP_THRESHOLD 30.0 

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Timing Variables
unsigned long previousMillisLCD = 0;
unsigned long fireEndMillis = 0; 
const long intervalLCD = 500;
const long restartGracePeriod = 3000; // 1 second clean-up time

bool fireActive = false;
bool needsRestart = false;

void setup() {
  pinMode(PIR_PIN, INPUT);
  pinMode(FLAME_PIN, INPUT);
  pinMode(GAS_DIGITAL, INPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  // Force safe state on start
  digitalWrite(FAN_PIN, OFF);
  digitalWrite(PUMP_PIN, OFF);
  noTone(BUZZER);

  dht.begin();
  lcd.init();
  lcd.backlight();
}

void loop() {
  unsigned long currentMillis = millis();

  // 1. Read Sensors
  bool fire = (digitalRead(FLAME_PIN) == LOW);
  bool gas = (digitalRead(GAS_DIGITAL) == LOW);
  float t = dht.readTemperature();
  int ldrValue = analogRead(LDR_PIN);
  bool motion = digitalRead(PIR_PIN);

  // 2. LDR Logic
  digitalWrite(LED_PIN, (ldrValue < 400) ? HIGH : LOW);

  // 3. Fan Logic
  digitalWrite(FAN_PIN, (t > TEMP_THRESHOLD || gas) ? ON : OFF);

  // 4. Fire & Clean Restart Logic
  if (fire) {
    digitalWrite(PUMP_PIN, ON);
    tone(BUZZER, 1000);
    fireActive = true;
    needsRestart = true; 
  } else {
    if (fireActive) {
      // Fire just stopped
      digitalWrite(PUMP_PIN, OFF);
      noTone(BUZZER);
      fireActive = false;
      fireEndMillis = currentMillis; // Start the timer for restart
    }
  }

  // Check if it's time for the clean restart
  if (needsRestart && !fireActive) {
    if (currentMillis - fireEndMillis >= restartGracePeriod) {
      // Explicitly clear hardware one last time
      digitalWrite(PUMP_PIN, OFF);
      digitalWrite(FAN_PIN, OFF);
      noTone(BUZZER);
      resetFunc(); 
    }
  }

  // 5. Gas Alarm (Only if no fire)
  if (gas && !fire) {
    if ((currentMillis / 200) % 2) tone(BUZZER, 500);
    else noTone(BUZZER);
  } else if (!fire) {
    noTone(BUZZER);
  }

  // 6. Timed LCD Update
  if (currentMillis - previousMillisLCD >= intervalLCD) {
    previousMillisLCD = currentMillis;
    updateDisplay(t, ldrValue, motion, fire, gas);
  }
}

void updateDisplay(float t, int ldr, bool m, bool f, bool g) {
  lcd.setCursor(4, 0);
  if (f)      lcd.print("ALARM: FIRE      ");
  else if (g) lcd.print("ALARM: GAS       ");
  else        lcd.print("SYSTEM SECURE    ");

  lcd.setCursor(0, 1);
  lcd.print("GAS:"); lcd.print(g ? "Y " : "N ");
  lcd.print("FIRE:"); lcd.print(f ? "Y " : "N ");
  lcd.print("PIR:"); lcd.print(m ? "Y" : "N");

  lcd.setCursor(0, 2);
  lcd.print("TEMP:"); lcd.print(t, 1);
  lcd.print("C LDR:"); lcd.print(ldr);
  lcd.print("%    ");

  lcd.setCursor(0, 3);
  lcd.print("FAN:"); lcd.print(digitalRead(FAN_PIN) == ON ? "ON " : "OFF");
  lcd.print(" PMP:"); lcd.print(digitalRead(PUMP_PIN) == ON ? "ON " : "OFF");
}