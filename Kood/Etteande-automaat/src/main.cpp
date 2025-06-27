/*
 * Projekt:  Etteande automatiseerimine
 * Autor:    Tauno Erik
 * Algus:    2025.06.26
 * Muudetud: 2025.06.27
 */
#include <Arduino.h>


/*************************************************
 Samm-mootorid
**************************************************/
// Mootori suund
#define CW  0  // clockwise
#define CCW 1  // counterclockwise
// Motor 1 pins
const int M1_PULSE_PIN = 2;
const int M1_DIRECTION_PIN = 3;
// Speed of the motor
const int M1_SPEED = 500;
// Function prototypes
void run_steps(int dir, int steps, int speed, int pulse_pin, int direction_pin);
void run(int speed, int pulse_pin);
void init_motor();


/*************************************************
 Lülitid
**************************************************/
// Pins
const int LEFT_BTN_PIN = 7;
const int RIGHT_BTN_PIN = 8;
// Timing
const unsigned long RELEASE_WINDOW = 200;  // milliseconds
unsigned long left_release_time = 0;
unsigned long right_release_time = 0;

void init_switches(int left_pin, int right_pin);

/*************************************************
 Relee
**************************************************/
// Pins
const int RELEE_PIN = 5;
// Function prototypes
void init_relee();
void relee_ON();
void relee_OFF()

/*************************************************
// Olekud
**************************************************/
enum State {
  IDLE,             // Tühikäik
  WAIT_NEW_DETAILS,
  ASK_NEW_DETAILS,  // Ütle robotile
  MOVE_FORWARD,     // Mootorid
  PUSH,             //Relee
  ERROR
};

State current_state = WAIT_NEW_DETAILS;



void setup() {
  Serial.begin(115200);

  // Stepper motor pins
  init_motor();
  // Lülitid
  init_switches(LEFT_BTN_PIN, RIGHT_BTN_PIN);
  // Relee
  init_relee();
}

void loop() {
  // Lülitid ////////////////////////////////////////////////
  static bool last_left_state = LOW;
  static bool last_right_state = LOW;

  bool left_state = digitalRead(LEFT_BTN_PIN);
  bool right_state = digitalRead(RIGHT_BTN_PIN);

  // Detect left button release
  if (last_left_state == HIGH && left_state == LOW) {
    left_release_time = millis();
    Serial.println("Left button released");
  }

  // Detect right button release
  if (last_right_state == HIGH && right_state == LOW) {
    right_release_time = millis();
    Serial.println("Right button released");
  }

  last_left_state = left_state;
  last_right_state = right_state;

  // Check if both buttons were released within the allowed window
  if (left_release_time != 0 && right_release_time != 0 &&
      abs((long)(left_release_time - right_release_time)) <= RELEASE_WINDOW) {
    
    // Turn LED on for 1 second
    digitalWrite(LED_PIN, HIGH);
    delay(1500);
    digitalWrite(LED_PIN, LOW);

    // Reset release times to prevent retriggering
    left_release_time = 0;
    right_release_time = 0;
  }

  // Olekumasin ////////////////////////////////////
  switch (current_state) {
    case IDLE:
      // Oota
      Serial.println("State: IDLE");
      
      if (digitalRead(2) == HIGH) {
        currentState = RUNNING;
      }

      break;

    case ERROR:
      // Viga
      Serial.println("State: Error!");
      current_state = IDLE; // Näiteks muudame tagasi ootama
      break;
  }
}


/**********************************************************************
 * Run the stepper motor for a number of steps
 * @param dir   Direction (CW or CCW)
 * @param steps Number of steps
 * @param speed Speed of the motor
 **********************************************************************/

void run_steps(int dir, int steps, int speed, int pulse_pin, int direction_pin)
{
  digitalWrite(direction_pin, dir);  // Set direction

  for (int i = 0; i < steps; i++)
  {
    digitalWrite(pulse_pin, HIGH);
    delayMicroseconds(speed);  // Pulse width (adjust for speed)
    digitalWrite(pulse_pin, LOW);
    delayMicroseconds(speed);  // Delay between steps (adjust for speed)
  }
}


/**********************************************************************
 * Run the stepper motor forever until both buttons are unpressed
 * @param speed Speed of the motor
 **********************************************************************/
void run(int speed, int pulse_pin)
{
  while (true)
  {
    digitalWrite(pulse_pin, HIGH);
    delayMicroseconds(speed);  // Pulse width (adjust for speed)
    digitalWrite(pulse_pin, LOW);
    delayMicroseconds(speed);  // Delay between steps (adjust for speed)

    // Exit the loop if neither button is unpressed
    if (digitalRead(BTN_LEFT_PIN) == HIGH && digitalRead(BTN_RIGHT_PIN) == HIGH)
    {
      break;
    }
  }
}

void init_switches(int left_pin, int right_pin) {
  pinMode(left_pin, INPUT);   // Pulled down externally
  pinMode(right_pin, INPUT);  // Pulled down externally
  Serial.println("Lülitid seadistatud!");
}

void init_motor() {
  pinMode(M1_PULSE_PIN, OUTPUT);
  pinMode(M1_DIRECTION_PIN, OUTPUT);
  Serial.println("Mootor seadistatud!");
}

void init_relee() {
  pinMode(RELEE_PIN, OUTPUT);
  digitalWrite(RELEE_PIN, LOW);
  Serial.println("Relee seadistatud!");
}

void relee_ON() {
  digitalWrite(RELEE_PIN, HIGH);
  Serial.println("Relee ON");
}

void relee_OFF() {
  digitalWrite(RELEE_PIN, LOW);
  Serial.println("Relee OFF");
}
