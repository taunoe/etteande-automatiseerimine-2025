/*
 * Projekt:  Etteande automatiseerimine
 * Autor:    Tauno Erik
 * Algus:    2025.06.26
 * Muudetud: 2025.08.11
 */
#include <Arduino.h>

/*************************************************
 Seaded
**************************************************/
// Aja seaded:
#define MOOTORI_EDASI_AEG    450  // ms
#define MOOTORI_TAGASI_AEG    80  // ms

#define LYKKAMISE_AEG       3100  // ms
#define TAGASITULEKU_AEG    5000  // ms

#define ROBOTI_SIGNAALI_AEG  100  // ms
#define ROBOTI_TOOMISE_AEG  5000  // ms
// Ultraheli anduri:
#define MAX_DETAILI_KAUGUS  19.0  // cm ultraheli andurist
#define MIN_DETAILI_KAUGUS  15.0  // cm ultraheli andurist

// Pinnid:
const int M1_PULSE_PIN     =  2;  // Mootor
const int M1_DIRECTION_PIN =  3;  // Mootor
const int LEFT_BTN_PIN     =  7;  // Limit Switch
const int RIGHT_BTN_PIN    =  8;  // Limit Switch
const int PUSH_RELEE_PIN   =  9;  // 
const int ROBOT_PIN        = 10;  // 
const int US1_TRIG_PIN     = 11;  // Ultraheli
const int US1_ECHO_PIN     = 12;  // Ultraheli

#define FORWARD 1  // Mootori suund
#define BACK    0  // Mootori suund
const int M1_SPEED = 500;  // Mootori kiirus

const unsigned long RELEASE_WINDOW = 200;  // Lülitid
unsigned long left_release_time = 0;  // Lülitid
unsigned long right_release_time = 0;  // Lülitid

static unsigned int loendur_kokku = 0;
static unsigned int loendur_viimased = 5; // Viimased detailid masinas
static bool status = true;

/*************************************************
 Function prototypes
**************************************************/
void oota(int aeg);
void lykka();
void kass_on();
// Mootorid
void init_motor();
void run_step_motor(int dir, int steps, int speed, int pulse_pin, int direction_pin);
void liiguta_edasi();
// Lükkamise relee
void init_push_relee();
void push_relee_ON();
void push_relee_OFF();
// Küsi robotilt
void init_ask_from_robot();
void ask_from_robot();
// Lülitid
void init_switches(int left_pin, int right_pin);
bool is_details();
// Ultraheli andur
void init_ultrasound(int trig_pin, int echo_pin);
float measure_distance(int trig_pin, int echo_pin);


/*************************************************
 Olekud
**************************************************/
enum State {
  OOTA,    // 0 Ootab
  KYSI,    // 1 Ütle robotile
  EDASI,   // 2 Mootorid liiguvad
  LYKKA,   // 3 Lükkamise relee lülitatud
  VIGA,    // 4 Viga
  KAS_ON,  // 5 Kas on uusi detaile?
  VIIMASED // 6
};

// Init state
static State next_step = OOTA;

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
  // Ultraheli kaugusandur
  init_ultrasound(US1_TRIG_PIN, US1_ECHO_PIN);
}

void loop() {
  // Olekumasin ////////////////////////////////////
  switch (next_step) {
    case OOTA: //0
      oota(1000);
      next_step = KAS_ON;
      break;

    case KYSI: //1
      ask_from_robot();
      next_step = KAS_ON;
      break;

    case EDASI: //2
      liiguta_edasi();
      next_step = LYKKA;
      break;

    case LYKKA: //3
      lykka();
      next_step = KAS_ON;
      break;
    
    case VIGA: //4
      Serial.println("VIGA");
      next_step = OOTA;
      break;

    case KAS_ON: //5
      status = is_details();
      // Järgmine olek:
      if(status == true) {
        next_step = EDASI;
        Serial.println("KAS ON UUSI DETAILE: JAH");
      }
      else {
        next_step = VIIMASED;
        Serial.println("KAS ON UUSI DETAILE: EI");
      }
      break;
    
    case VIIMASED:
       // Käivitub loendur: 5 viimast detaili
      if (loendur_viimased > 0) {
        Serial.print("Viimased ");
        Serial.print(loendur_viimased);
        Serial.print(" detaili!\n");
        // Küsi robotilt
        Serial.println("Ütken ROBOTILE");
        digitalWrite(ROBOT_PIN, HIGH);
        delay(ROBOTI_SIGNAALI_AEG);
        // Pärast seda
        next_step = LYKKA;
        loendur_viimased--;
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
void ask_from_robot() {
  Serial.println("Küsin ROBOTILT");
  digitalWrite(ROBOT_PIN, HIGH);
  delay(ROBOTI_SIGNAALI_AEG);
  digitalWrite(ROBOT_PIN, LOW);
  delay(ROBOTI_TOOMISE_AEG);
}

/*
return: false - Kui üks nuppudest on alla vajutamata
return: true  - Mõlemad nuppud allvajutatud
*/
bool is_details() {
  /*
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
  */
  float kaugus = measure_distance(US1_TRIG_PIN, US1_ECHO_PIN);
  Serial.print("Kaugus: ");
  Serial.print(kaugus);

  if (kaugus < MAX_DETAILI_KAUGUS && kaugus > MIN_DETAILI_KAUGUS) {
    Serial.print(" -  Hea!\n");
    return true;
  }

  Serial.print(" -  Halb!\n");
  return false;
}


/*
*/
void init_ultrasound(int trig_pin, int echo_pin) {
  pinMode(trig_pin, OUTPUT);
  pinMode(echo_pin, INPUT);
  Serial.println("Ultraheli seadistatud!");
}

/*********************************************************
 Ultraheli kauguse mõõtmine
 returns distance cm float
**********************************************************/
float measure_distance(int trig_pin, int echo_pin) {
  // Clear trigger
  digitalWrite(trig_pin, LOW);
  delayMicroseconds(2);

  // Send 10µs pulse to trigger
  digitalWrite(trig_pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig_pin, LOW);

  // Measure the echo pulse length
  long duration = pulseIn(echo_pin, HIGH);

  // Calculate distance in centimeters
  float distance_cm = duration * 0.0343 / 2;

  return distance_cm;
}

void liiguta_edasi() {
  Serial.println("LIIGUTA EDASI");
  // Liiguta edasi mootoreid
  run_step_motor(FORWARD, MOOTORI_EDASI_AEG, M1_SPEED, M1_PULSE_PIN, M1_DIRECTION_PIN);
  delay(100);
  // Liiguta mootoreid nõks tagasi
  run_step_motor(BACK, MOOTORI_TAGASI_AEG, M1_SPEED, M1_PULSE_PIN, M1_DIRECTION_PIN);
  delay(100);
}

void oota(int aeg) {
  Serial.println("OOTA");
  delay(aeg);
}


void lykka() {
  Serial.println("LÜKKA");

  push_relee_ON();
  delay(LYKKAMISE_AEG);

  push_relee_OFF();
  delay(TAGASITULEKU_AEG);

  loendur_kokku++;
  Serial.print("Kokku lükkatud: ");
  Serial.print(loendur_kokku);
  Serial.print(" detaili\n");
}

void kass_on() {

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