/*
 * Projekt:  Mootorid
 * Autor:    Tauno Erik
 * Algus:    2025.06.26
 * Muudetud: 2026.02.18
 */
#include <Arduino.h>

/*************************************************
 Seaded
**************************************************/
// Aja seaded: 1000ms == 1 second
#define MOOTORI_EDASI_AEG      5 //950  // ms
#define MOOTORI_TAGASI_AEG    80  // ms

// Pinnid:
const int M1_PULSE_PIN     =  2;  // Mootor
const int M1_DIRECTION_PIN =  3;  // Mootor
const int INPUT_PIN        =  7;  // Input

#define FORWARD 1                 // Mootori suund
#define BACK    0                 // Mootori suund
const int M1_SPEED = 500;         // Mootori kiirus

int input_state = 0;        // current state of the button
int last_input_state = 0;    // previous state of the button

/*************************************************
 Function prototypes
**************************************************/
void init_motor();
void run_step_motor(int dir, int steps, int speed, int pulse_pin, int direction_pin);
void motor_edasi();
void init_input(int input_pin);


/*************************************************
 Setup
**************************************************/
void setup() {
  //Serial.begin(115200);
  init_motor();
  init_input(INPUT_PIN);
}

/*************************************************
 Loop
**************************************************/
void loop() {
  input_state = digitalRead(INPUT_PIN);

  if (input_state == HIGH) {
    //Serial.println("ON");
    motor_edasi();
  }
  else {
    //Serial.print("_");
  }
}  // loop end


/*
Seadista mootori pinnid
*/
void init_motor() {
  pinMode(M1_PULSE_PIN, OUTPUT);
  pinMode(M1_DIRECTION_PIN, OUTPUT);
  Serial.println("Mootor seadistatud!");
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


/*
Rattad lükkavad detaile edasi teatud aja
*/
void motor_edasi() {
  // Liiguta edasi mootoreid
  run_step_motor(FORWARD, MOOTORI_EDASI_AEG, M1_SPEED, M1_PULSE_PIN, M1_DIRECTION_PIN);
  //delay(100);
  // Liiguta mootoreid nõks tagasi
  //run_step_motor(BACK, MOOTORI_TAGASI_AEG, M1_SPEED, M1_PULSE_PIN, M1_DIRECTION_PIN);
  //delay(100);
}


/*
Seadista input pin
*/
void init_input(int input_pin) {
  pinMode(input_pin, INPUT);   // Pulled down externally
  Serial.println("Input seadistatud!");
}
