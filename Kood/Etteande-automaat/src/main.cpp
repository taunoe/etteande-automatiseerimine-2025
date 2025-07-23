/*
 * Projekt:  Etteande automatiseerimine
 * Autor:    Tauno Erik
 * Algus:    2025.06.26
 * Muudetud: 2025.07.23
 */
#include <Arduino.h>


/*************************************************
 Samm-mootorid
**************************************************/
// Mootori suund
#define CW      0  // clockwise
#define FORWARD 1
#define CCW     1  // counterclockwise
#define BACK    0
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
bool ask_from_robot();

/*
Nuppud
*/
bool is_details();

/*************************************************
// Olekud
**************************************************/
enum State {
  IDLE,             // 0 Ootab
  ASK_NEW_DETAILS,  // 1 Ütle robotile
  MOVE_FORWARD,     // 2 Mootorid liiguvad
  PUSH,             // 3 Relee lülitatud
  VIGA,             // 4 Viga
  IS_DETAILS        // 5 Kas on uus detail?
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
  // Relee
  init_relee();
}

void loop() {
  //Serial.print("current_state: ");
  //Serial.println(current_state);
  //delay(1000);  // Delay for readability

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
      run_step_motor(FORWARD, 350, M1_SPEED, M1_PULSE_PIN, M1_DIRECTION_PIN);
      delay(100);
      // Liiguta mootoreid nõks tagasi
      run_step_motor(BACK, 50, M1_SPEED, M1_PULSE_PIN, M1_DIRECTION_PIN);
      delay(100);
      // Järgmine samm:
      current_state = PUSH;
      break;

    case PUSH: //3
      Serial.println("masina olek: LÜKKA");
      relee_ON();
      delay(2000);
      relee_OFF();
      counter++;
      Serial.print("Loendur: ");
      Serial.println(counter);
      // Järgmine samm:
      current_state = IS_DETAILS;
      break;
    
    case VIGA: //4
      // Viga
      Serial.println("masina olek: VIGA");
      current_state = IDLE; // Näiteks muudame tagasi ootama
      break;

    case IS_DETAILS: //5
      //Serial.print("masina: olek: KAS ON DETAILE: ");
      status = is_details();
      // Järgmine samm:
      if(status == true) {
        current_state = MOVE_FORWARD;
        Serial.println("masina olek: KAS ON DETAILE: JAH");
      }
      else {
        current_state = ASK_NEW_DETAILS;
        Serial.println("masina olek: KAS ON DETAILE: EI");
      }
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

bool ask_from_robot() {
  Serial.println("Küsin ROBERTALT");
  // TODO
  delay(2000);
  return true;
}

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