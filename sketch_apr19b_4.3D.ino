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
// #include <Wire.h>
// #include <BH1750.h>                   // Library for the BH1750 light sensor
// #define USING_TIMER_TC3 true          // Use Timer TC3 (to avoid conflict with USB/millis)
// #include <SAMDTimerInterrupt.h>       // Hardware timer interrupt library

// //Pin Definitions
// #define SWITCH_PIN         2          // Digital input for slider switch
// #define LED1_PIN           4          // LED toggled by switch interrupt
// #define LED2_PIN           5         // LED toggled by simulated light sensor interrupt
// #define LED3_PIN           9          // LED toggled every 1 second (hardware timer)

// #define LIGHT_OUT_PIN      8          // Output pin to simulate light sensor interrupt signal
// #define LIGHT_INT_PIN      3          // Input pin to receive light sensor "interrupt" via jumper from LIGHT_OUT_PIN

// #define DISTANCE_OUT_PIN  12          // Output pin to simulate ultrasonic detection interrupt
// #define DISTANCE_INT_PIN  13         // Input pin to receive ultrasonic "interrupt" via jumper from DISTANCE_OUT_PIN

// #define TRIG_PIN          10          // Ultrasonic trigger pin
// #define ECHO_PIN          11          // Ultrasonic echo pin (read using pulseIn)

// #define LIGHT_THRESHOLD  1000         // Lux threshold to simulate light interrupt

// //Sensor and Timer Setup 
// BH1750 lightMeter;                    // Create BH1750 light sensor object
// SAMDTimer ITimer(TIMER_TC3);         // Use Timer TC3 for 1s periodic ISR

// //Global States 
// volatile bool led1State = false;     // State of LED1 (switch-based)
// volatile bool led2State = false;     // State of LED2 (light interrupt-based)
// volatile bool led3State = false;     // State of LED3 (timer-based)
// volatile bool objectLogged = false;  // Tracks whether ultrasonic object detection has been logged

// // Interrupt Service Routine: Slider switch toggle
// void handleSwitchToggle() {
//   led1State = !led1State;
//   digitalWrite(LED1_PIN, led1State);
//   Serial.println("Slider Interrupt: LED1 toggled");
// }

// // Interrupt Service Routine: Simulated light interrupt 
// void handleLightInterrupt() {
//   led2State = !led2State;
//   digitalWrite(LED2_PIN, led2State);
//   Serial.println("Light Interrupt Triggered: LED2 toggled");
// }

// // Interrupt Service Routine: Simulated ultrasonic detection interrupt 
// void handleDistanceInterrupt() {
//   if (!objectLogged) {
//     objectLogged = true; // Avoid duplicate triggers
//     Serial.println("Interrupt Triggered: Object detected within 0-20 cm!");
//   }
// }

// // Timer ISR: Fires every 1 second to handle sensing logic 
// void TIMER_ISR_INTERRUPT() {
//   // 1. Toggle LED3
//   led3State = !led3State;
//   digitalWrite(LED3_PIN, led3State);
//   // Serial.print("Timer: LED3 toggled to ");
//   // Serial.println(led3State ? "ON" : "OFF");

//   // 2. Read ambient light using BH1750 sensor
//   float lux = lightMeter.readLightLevel();
//     Serial.print("Lux: ");
//     Serial.println(lux);



//   // If lux exceeds threshold, simulate a light sensor interrupt
//   digitalWrite(LIGHT_OUT_PIN, (lux > LIGHT_THRESHOLD) ? HIGH : LOW);

//   // 3. Trigger ultrasonic sensor
//   digitalWrite(TRIG_PIN, LOW);
//   delayMicroseconds(2);
//   digitalWrite(TRIG_PIN, HIGH);
//   delayMicroseconds(10);
//   digitalWrite(TRIG_PIN, LOW);

//   // 4. Read echo pulse and convert to distance (cm)
//   long duration = pulseIn(ECHO_PIN, HIGH, 30000); // Timeout after 30ms

//   if (duration > 0) {
//     float distance = duration * 0.034 / 2;
//     Serial.print("Ultrasonic Distance: ");
//     Serial.print(distance);
//     Serial.println(" cm");

//     // If object detected within 0–20cm, simulate an interrupt using DISTANCE_OUT_PIN
//     if (distance > 0 && distance <= 20) {

//       digitalWrite(DISTANCE_OUT_PIN, HIGH);  // RISING edge to trigger interrupt
//       delay(10);                              // Allow time for detection
//       digitalWrite(DISTANCE_OUT_PIN, LOW);   // Reset signal
//     } else {
//       objectLogged = false; // Reset flag so next detection can trigger again
//     }
//   } else {
//       objectLogged = false; // Reset if nothing is detected
//   }
// }

// // Setup Function 
// void setup() {
//   Serial.begin(115200);        // Begin serial communication for debugging
//   Wire.begin();                // Start I2C communication
//   lightMeter.begin();          // Initialize BH1750 light sensor

//   // Configure pin modes
//   pinMode(SWITCH_PIN, INPUT_PULLUP);
//   pinMode(LED1_PIN, OUTPUT);
//   pinMode(LED2_PIN, OUTPUT);
//   pinMode(LED3_PIN, OUTPUT);
//   pinMode(LIGHT_OUT_PIN, OUTPUT);
//   pinMode(LIGHT_INT_PIN, INPUT_PULLUP);
//   pinMode(DISTANCE_OUT_PIN, OUTPUT);
//   pinMode(DISTANCE_INT_PIN, INPUT_PULLUP);
//   pinMode(TRIG_PIN, OUTPUT);
//   pinMode(ECHO_PIN, INPUT);

//   // Attach external interrupts
//   attachInterrupt(digitalPinToInterrupt(SWITCH_PIN), handleSwitchToggle, CHANGE);          // Switch interrupt
//   attachInterrupt(digitalPinToInterrupt(LIGHT_INT_PIN), handleLightInterrupt, RISING);     // Simulated light interrupt
//   attachInterrupt(digitalPinToInterrupt(DISTANCE_INT_PIN), handleDistanceInterrupt, RISING); // Simulated distance interrupt

//   // Start 1-second timer
//   if (ITimer.attachInterruptInterval(1000000, TIMER_ISR_INTERRUPT)) {
//     Serial.println("Timer (TC3) interrupt started at 1s interval.");
//   }

//   Serial.println("System Ready.");
// }

// // Main loop is unused as everything is interrupt-driven 
// void loop() {
//   // All actions handled by timer and hardware interrupts
// }