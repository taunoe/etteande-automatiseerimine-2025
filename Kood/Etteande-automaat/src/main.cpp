/*
 * Projekt:  Etteande automatiseerimine
 * Autor:    Tauno Erik
 * Algus:    2025.06.26
 * Muudetud: 2025.07.20
 */
#include <Arduino.h>


/*************************************************
 Samm-mootorid
**************************************************/
// Mootori suund
#define CW      0  // clockwise
#define FORWARD 0
#define CCW     1  // counterclockwise
#define BACK    1
// Motor 1 pins
const int M1_PULSE_PIN = 2;      // D2
const int M1_DIRECTION_PIN = 3;  // D3
// Speed of the motor
const int M1_SPEED = 500;
// Function prototypes
void init_motor();
void run_step_motor(int dir, int steps, int speed, int pulse_pin, int direction_pin);
//void run(int speed, int pulse_pin);



/*************************************************
 Lülitid
**************************************************/
// Pins
const int LEFT_BTN_PIN = 7;   // D7
const int RIGHT_BTN_PIN = 8;  // D8
// Timing
const unsigned long RELEASE_WINDOW = 200;  // milliseconds
unsigned long left_release_time = 0;
unsigned long right_release_time = 0;

void init_switches(int left_pin, int right_pin);

/*************************************************
 Relee
**************************************************/
// Pins
const int RELEE_PIN = 9;  // D9
// Function prototypes
void init_relee();
void relee_ON();
void relee_OFF();

/*************************************************
 Küsi robotilt
**************************************************/
void ask_from_robot();

/*************************************************
// Olekud
**************************************************/
enum State {
  IDLE,             // Ootab
  ASK_NEW_DETAILS,  // Ütle robotile
  MOVE_FORWARD,     // Mootorid liiguvad
  PUSH,             // Relee lülitatud
  ERROR             // Viga
};

// Init state
State current_state = IDLE;



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
  static bool last_left_btn_state = LOW;
  static bool last_right_btn_state = LOW;

  bool left_btn_state = digitalRead(LEFT_BTN_PIN);
  bool right_btn_state = digitalRead(RIGHT_BTN_PIN);

  // Detect left button release
  if (last_left_btn_state == HIGH && left_btn_state == LOW) {
    left_release_time = millis();
    Serial.println("Vasak nupp vabastatud");
  }

  // Detect right button release
  if (last_right_btn_state == HIGH && right_btn_state == LOW) {
    right_release_time = millis();
    Serial.println("Parem nupp vabastatud");
  }

  last_left_btn_state = left_btn_state;
  last_right_btn_state = right_btn_state;

  // Check if both buttons were released within the allowed window
  if (left_release_time != 0 && right_release_time != 0 &&
      abs((unsigned long)(left_release_time - right_release_time)) <= RELEASE_WINDOW) {
      Serial.println("Mölemad nupud vabastatud");
    // Detaili ei ole
    // Küsi robotilt
    current_state = ASK_NEW_DETAILS;

    // Reset release times to prevent retriggering
    left_release_time = 0;
    right_release_time = 0;
  }

  // Olekumasin ////////////////////////////////////
  switch (current_state) {
    case IDLE:
      // Oota
      Serial.println("switch: state: IDLE");
      delay(1000);
      // Järgmine samm:
      current_state = MOVE_FORWARD;
      break;

    case ASK_NEW_DETAILS:
      Serial.println("switch: state: ASK_NEW_DETAILS");
      // TODO:
      ask_from_robot();
      delay(1000);
      // Järgmine samm:
      current_state = MOVE_FORWARD;
      break;

    case MOVE_FORWARD:
      Serial.println("switch: state: MOVE_FORWARD");
      // Liiguta edasi mootoreid
      run_step_motor(FORWARD, 500, M1_SPEED, M1_PULSE_PIN, M1_DIRECTION_PIN);
      //delay(10);
      // Liiguta mootoreid nõks tagasi
      run_step_motor(BACK, 500, M1_SPEED, M1_PULSE_PIN, M1_DIRECTION_PIN);
      //delay(10);
      // Järgmine samm:
      current_state = PUSH;
      break;

    case PUSH:
      Serial.println("switch: state: PUSH");
      relee_ON();
      delay(1000);
      relee_OFF();
      // Järgmine samm:
      current_state = MOVE_FORWARD;
      break;

    case ERROR:
      // Viga
      Serial.println("switch: state: ERROR");
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

void run_step_motor(int dir, int steps, int speed, int pulse_pin, int direction_pin)
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
 /*
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
*/

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

void ask_from_robot() {
  println("Küsin ROBERTALT");
}