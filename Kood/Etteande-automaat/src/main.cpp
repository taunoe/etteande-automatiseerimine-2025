/*
 * Projekt:  Etteande automatiseerimine
 * Autor:    Tauno Erik
 * Algus:    2025.06.26
 * Muudetud: 2025.07.30
 */
#include <Arduino.h>

/*************************************************
 Samm-mootorid
**************************************************/
// Mootori suund
#define FORWARD 1
#define BACK    0
// Delays
#define EDASI_AEG  350  // ms
#define TAGASI_AEG  50  // ms
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
Lükkamise relee
**************************************************/
// Delays
#define LYKKAMISE_AEG    3100  // ms
#define TAGASITULEKU_AEG 5000  // ms
// Pins
const int PUSH_RELEE_PIN = 9;  // D9
// Function prototypes
void init_push_relee();
void push_relee_ON();
void push_relee_OFF();

/*************************************************
 Küsi robotilt
**************************************************/
#define ROBOTI_SIGNAALI_AEG 100  // ms
#define ROBOTI_DETAILIDE_TOOMISE_AEG 5000  // ms
// Pins
const int ROBOT_PIN = 10; // D10
// Function prototypes
void init_ask_from_robot();
bool ask_from_robot();


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
// Function prototypes
void init_switches(int left_pin, int right_pin);
bool is_details();


/*************************************************
 Olekud
**************************************************/
enum State {
  IDLE,             // 0 Ootab
  ASK_NEW_DETAILS,  // 1 Ütle robotile
  MOVE_FORWARD,     // 2 Mootorid liiguvad
  PUSH,             // 3 Lükkamise relee lülitatud
  VIGA,             // 4 Viga
  IS_DETAILS        // 5 Kas on uusi detail?
};

// Init state
static State current_state = IDLE;

static unsigned int counter = 0;
static bool status = true;

void setup() {
  Serial.begin(115200);

  // Stepper motor pins
  init_motor();
  // Lülitid
  init_switches(LEFT_BTN_PIN, RIGHT_BTN_PIN);
  // Lükkamise relee
  init_push_relee();
  // Roboti relee
  init_ask_from_robot();
}

void loop() {
  // Olekumasin ////////////////////////////////////
  switch (current_state) {
    case IDLE: //0
      // Oota
      Serial.println("masina olek: OOTA");
      delay(1000);
      // Järgmine samm:
      current_state = IS_DETAILS;
      break;

    case ASK_NEW_DETAILS: //1
      Serial.println("masina olek: KÜSI ROBERTALT");
      // TODO:
      status = ask_from_robot();
      //delay(2000);
      // Järgmine samm:
      if (status == true) {
        current_state = IS_DETAILS;
      }
      break;

    case MOVE_FORWARD: //2
      Serial.println("masina olek: LIIGUTA EDASI");
      // Liiguta edasi mootoreid
      run_step_motor(FORWARD, EDASI_AEG, M1_SPEED, M1_PULSE_PIN, M1_DIRECTION_PIN);
      delay(100);
      // Liiguta mootoreid nõks tagasi
      run_step_motor(BACK, TAGASI_AEG, M1_SPEED, M1_PULSE_PIN, M1_DIRECTION_PIN);
      delay(100);
      // Järgmine samm:
      current_state = PUSH;
      break;

    case PUSH: //3
      Serial.println("masina olek: LÜKKA");
      push_relee_ON();
      delay(LYKKAMISE_AEG);
      push_relee_OFF();
      delay(TAGASITULEKU_AEG);
      counter++;
      Serial.print("--------Loendur: ");
      Serial.println(counter);
      // Järgmine olek:
      current_state = IS_DETAILS;
      break;
    
    case VIGA: //4
      Serial.println("masina olek: VIGA");
      // Järgmine olek:
      current_state = IDLE;
      break;

    case IS_DETAILS: //5
      status = is_details();
      // Järgmine olek:
      if(status == true) {
        current_state = MOVE_FORWARD;
        Serial.println("masina olek: KAS ON DETAILE: JAH");
      }
      else {
        current_state = ASK_NEW_DETAILS;
        Serial.println("masina olek: KAS ON DETAILE: EI");
      }
      break;
  }  // switch end

}  // loop end


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

void init_push_relee() {
  pinMode(PUSH_RELEE_PIN, OUTPUT);
  digitalWrite(PUSH_RELEE_PIN, LOW);
  Serial.println("Lükkamise relee seadistatud!");
}

void push_relee_ON() {
  digitalWrite(PUSH_RELEE_PIN, HIGH);
  Serial.println("Lükkamise relee ON");
}

void push_relee_OFF() {
  digitalWrite(PUSH_RELEE_PIN, LOW);
  Serial.println("Lükkamise relee OFF");
}


void init_ask_from_robot() {
  pinMode(ROBOT_PIN, OUTPUT);
  digitalWrite(ROBOT_PIN, LOW);
  Serial.println("Roboti pin seadistatud!");
}

/*
*/
bool ask_from_robot() {
  Serial.println("Küsin ROBERTALT");
  digitalWrite(ROBOT_PIN, HIGH);
  delay(ROBOTI_SIGNAALI_AEG);
  digitalWrite(ROBOT_PIN, LOW);
  delay(ROBOTI_DETAILIDE_TOOMISE_AEG);
  return true;
}

/*
return: false - Kui üks nuppudest on alla vajutamata
return: true  - Mõlemad nuppud allvajutatud
*/
bool is_details() {
  // Lülitid ////////////////////////////////////////////////
  bool left_btn_state = digitalRead(LEFT_BTN_PIN);
  bool right_btn_state = digitalRead(RIGHT_BTN_PIN);

  // Detect left button release
  if (left_btn_state == LOW) {
    Serial.println("Vasak nupp vabastatud");
    return false;
  }

  // Detect right button release
  if (right_btn_state == LOW) {
    Serial.println("Parem nupp vabastatud");
    return false;
  }

  return true;
}

/*
bool is_details() {
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


    // Reset release times to prevent retriggering
    left_release_time = 0;
    right_release_time = 0;
    // Detaili ei ole
    return false;
  }
  // TODO: kui üks vajutatud ja teine mitte siis on viga

  return true;
}
*/