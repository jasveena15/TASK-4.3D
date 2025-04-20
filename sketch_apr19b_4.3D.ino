#include <Wire.h>
#include <BH1750.h>
#define USING_TIMER_TC3 true          // Use Timer TC3 (to avoid conflict with USB/millis)
#include <SAMDTimerInterrupt.h>

// ----------- Pin Assignments -----------
#define BUTTON_PIN       2
#define LED_BUTTON       4

#define LIGHT_OUT_PIN    8     // Output pin to simulate light interrupt
#define LIGHT_INT_PIN    3     // Input pin for simulated light interrupt
#define LED_LIGHT        5

#define DIST_OUT_PIN    12    // Output pin to simulate distance interrupt
#define DIST_INT_PIN    13     // Input pin for simulated distance interrupt


#define LED_TIMER        9     // Timer activity LED

#define TRIG_PIN        10
#define ECHO_PIN        11

#define LIGHT_THRESHOLD   1000.0
#define DISTANCE_THRESHOLD 20.0 // cm


BH1750 lightSensor;
SAMDTimer ITimer(TIMER_TC3);

volatile bool ledButtonState = false;
volatile bool ledLightState = false;
volatile bool ledDistanceState = false;
volatile bool ledTimerState = false;

// ---------- ISR: Button Press ----------
void ButtonPress() {
  ledButtonState = !ledButtonState;
  digitalWrite(LED_BUTTON, ledButtonState);
  Serial.println("Button Interrupt: LED_BUTTON toggled");
}

// ---------- ISR: Light Trigger ----------
void LightInterrupt() {
  ledLightState = !ledLightState;
  digitalWrite(LED_LIGHT, ledLightState);
  Serial.println("Light Interrupt: LED_LIGHT toggled");
}

// ---------- ISR: Distance Trigger ----------
void DistanceInterrupt() {
  ledDistanceState = !ledDistanceState;
  Serial.println("Distance Interrupt: object is less than 15 cm close");
}

// ---------- Timer Callback ----------
void pollSensors() {
  // Toggle LED_TIMER to show heartbeat
  ledTimerState = !ledTimerState;
  digitalWrite(LED_TIMER, ledTimerState);

  // ---- Light Sensor Polling ----
  float lux = lightSensor.readLightLevel();
  // Serial.print("Lux: ");
  // Serial.println(lux);
  digitalWrite(LIGHT_OUT_PIN, (lux > LIGHT_THRESHOLD) ? HIGH : LOW);

  // ---- Distance Polling ----
   long duration;
digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH, 25000); // 25ms timeout (~4m max)
  if (duration > 0) {
    float distance = duration * 0.034 / 2;
    Serial.print("Ultrasonic Distance: ");
    Serial.print(distance);
    Serial.println(" cm");

    // If object detected within 0–20cm, simulate an interrupt using DISTANCE_OUT_PIN
    if (distance > 0 && distance <= 15) {

      digitalWrite(DIST_OUT_PIN, HIGH);  // RISING edge to trigger interrupt
      delay(10);                              // Allow time for detection
      digitalWrite(DIST_OUT_PIN, LOW);   // Reset signal
    } else {
      ledDistanceState = false; // Reset flag so next detection can trigger again
    }
  } else {
      ledDistanceState = false; // Reset if nothing is detected
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin();

  // --- LED Outputs ---
  pinMode(LED_BUTTON, OUTPUT);
  pinMode(LED_LIGHT, OUTPUT);
  pinMode(LED_TIMER, OUTPUT);
   

  // --- Simulated Interrupt Pins ---
  pinMode(LIGHT_OUT_PIN, OUTPUT);
  pinMode(LIGHT_INT_PIN, INPUT);
  pinMode(DIST_OUT_PIN, OUTPUT);
  pinMode(DIST_INT_PIN, INPUT);

  // --- Button Pin Setup ---
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), ButtonPress, CHANGE);

  // --- Simulated Sensor Interrupts ---
  attachInterrupt(digitalPinToInterrupt(LIGHT_INT_PIN), LightInterrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(DIST_INT_PIN), DistanceInterrupt, RISING);

  // --- Ultrasonic Sensor Pins ---*
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // --- BH1750 Setup ---
  if (!lightSensor.begin()) {
    Serial.println("BH1750 failed to initialize.");
  }

  // --- Timer Setup ---
  if (!ITimer.attachInterruptInterval(1000000, pollSensors)) {
    Serial.println("Failed to start timer interrupt.");
  } else {
    Serial.println("Timer interrupt active (1Hz).");
  }
}

 

void loop() {
  // Nothing here — all logic handled by interrupts
}
