/*
 * Projekt:  Etteande automatiseerimine
 * Autor:    Tauno Erik
 * Algus:    2025.06.26
 * Muudetud: 2025.09.09
 */
#include <Arduino.h>

/*************************************************
 Seaded
**************************************************/
// Aja seaded: 1000ms == 1 second
#define MOOTORI_EDASI_AEG    950  // ms
#define MOOTORI_TAGASI_AEG    80  // ms

#define LYKKAMISE_AEG       3100  // ms

#define ROBOTI_SIGNAALI_AEG  100  // ms
#define ROBOTI_TOOMISE_AEG  5000  // ms ?
// Ultraheli anduri:
#define MIN_DETAILI_KAUGUS  15.0  // cm ultraheli andurist
#define MAX_DETAILI_KAUGUS  21.0  // cm ultraheli andurist

// Mitu korda mõõdetakse, et arvutda keskmist kaugust:
#define MITUKORDA             10

// Minu iimased detaili masinas:
#define VIIMASED_TK            3

// Pinnid:
const int M1_PULSE_PIN     =  2;  // Mootor
const int M1_DIRECTION_PIN =  3;  // Mootor
const int KOLVI_LIMIT_PIN  =  8;  // Kolvi Limit Switch
const int TEINE_LIMIT_PIN  =  7;  // Lüli peale Kolvi
const int PUSH_RELEE_PIN   =  9;  // 
const int ROBOT_PIN        = 10;  // 
const int US1_TRIG_PIN     = 11;  // Ultraheli
const int US1_ECHO_PIN     = 12;  // Ultraheli

#define FORWARD 1                 // Mootori suund
#define BACK    0                 // Mootori suund
const int M1_SPEED = 500;         // Mootori kiirus

static unsigned int loendur_viimased = VIIMASED_TK;

static bool status = true;
// Masin loendab lükkatud detaile
static unsigned int loendur_kokku = 0;

//const unsigned long RELEASE_WINDOW = 200;  // Lülitid
//unsigned long left_release_time = 0;  // Lülitid
//unsigned long right_release_time = 0;  // Lülitid

/*************************************************
 Function prototypes
**************************************************/
void init_motor();
void init_push_relee();
void init_ask_from_robot();
void init_switches(int left_pin, int right_pin);
void init_ultrasound(int trig_pin, int echo_pin);

void oota(int aeg);
void lykka();
void oota_kolbi_tagasi();
void oota_laua_vabastamist();
void run_step_motor(int dir, int steps, int speed, int pulse_pin, int direction_pin);
void liiguta_edasi();
void push_relee_ON();
void push_relee_OFF();
void ask_from_robot();
bool is_details();
double measure_distance(int trig_pin, int echo_pin);


/*************************************************
 Olekud
**************************************************/
enum State {
  OOTA,           // 0 - Ootab
  KYSI_ROBOTILT,  // 1 - Ütle robotile
  RATTAD_EDASI,   // 2 - Mootorid liiguvad //EDASI
  KOLB_LYKKAB,    // 3 - Lükkamise relee lülitatud //LYKKA
  VIGA,           // 4 - Viga
  KAS_ON_LAUDU,   // 5 - Kas on uusi detaile?
  VIIMASED        // 6 - Viimased detailid masinas
};

// Alg olek
static State next_step = OOTA;

void setup() {
  Serial.begin(115200);

  // Stepper motor pins
  init_motor();
  // Lülitid
  init_switches(KOLVI_LIMIT_PIN, TEINE_LIMIT_PIN);
  // Lükkamise relee
  init_push_relee();
  // Roboti relee
  init_ask_from_robot();
  // Ultraheli kaugusandur
  init_ultrasound(US1_TRIG_PIN, US1_ECHO_PIN);
}

void loop() {

  switch (next_step) {
    case OOTA:  // 0
      oota(1000);
      next_step = KAS_ON_LAUDU;
      break;

    case KYSI_ROBOTILT:         // 1
      ask_from_robot();         // Signaal robotile
      oota(ROBOTI_TOOMISE_AEG); // 
      next_step = KAS_ON_LAUDU; // Järgmine tegevus
      break;

    case RATTAD_EDASI:         // 2
      oota(4250);
      liiguta_edasi();         // Rattad lükkavad edasi
      next_step = KOLB_LYKKAB; // Järgmine tegevus
      break;

    case KOLB_LYKKAB:           // 3
      oota(750);                // Oota enne lükkamist
      lykka();                  // Suruõhu kolb lükkab
      oota_kolbi_tagasi();      // Suruõhu kolb on tagasi algasendis
      oota_laua_vabastamist();  // Laud on vaba uue detaili jaoks
      next_step = KAS_ON_LAUDU; // Järgmine tegevus
      break;
    
    case VIGA:  // 4
      Serial.println("--VIGA--");
      next_step = OOTA;
      break;

    case KAS_ON_LAUDU:  // 5
      status = is_details();

      if(status == true) { // Kui on uusi detaile
        next_step = RATTAD_EDASI; // Järgmine tegevus
        Serial.println("KAS ON UUSI DETAILE: JAH");
        loendur_viimased = VIIMASED_TK;
      }
      else {
        next_step = VIIMASED; // Järgmine tegevus
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
        Serial.println("Ütlen ROBOTILE"); // Mitu korda??
        digitalWrite(ROBOT_PIN, HIGH);
        delay(ROBOTI_SIGNAALI_AEG);
        digitalWrite(ROBOT_PIN, LOW);
        // Pärast seda
        next_step = RATTAD_EDASI;
        loendur_viimased--;
      } else {
        next_step = KYSI_ROBOTILT;
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

/*
Seadista pinnid
*/
void init_switches(int pin_1, int pin_2) {
  pinMode(pin_1, INPUT);   // Pulled down externally
  pinMode(pin_2, INPUT);   // Pulled down externally
  Serial.println("Lülitid seadistatud!");
}

/*
Seadista pinnid
*/
void init_motor() {
  pinMode(M1_PULSE_PIN, OUTPUT);
  pinMode(M1_DIRECTION_PIN, OUTPUT);
  Serial.println("Mootor seadistatud!");
}

/*
Seadista pinnid
*/
void init_push_relee() {
  pinMode(PUSH_RELEE_PIN, OUTPUT);
  digitalWrite(PUSH_RELEE_PIN, LOW);
  Serial.println("Lükkamise relee seadistatud!");
}

/*
Seadista pinnid
*/
void init_ask_from_robot() {
  pinMode(ROBOT_PIN, OUTPUT);
  digitalWrite(ROBOT_PIN, LOW);
  Serial.println("Roboti pin seadistatud!");
}

/*
Seadista pinnid
*/
void init_ultrasound(int trig_pin, int echo_pin) {
  pinMode(trig_pin, OUTPUT);
  pinMode(echo_pin, INPUT);
  Serial.println("Ultraheli seadistatud!");
}

/*
Lükkamise relee ON
*/
void push_relee_ON() {
  digitalWrite(PUSH_RELEE_PIN, HIGH);
  Serial.println("Lükkamise relee ON");
}

/*
Lükkamise relee OFF
*/
void push_relee_OFF() {
  digitalWrite(PUSH_RELEE_PIN, LOW);
  Serial.println("Lükkamise relee OFF");
}

/*
Lülitab sisse relee korraks, et anda signaal
*/
void ask_from_robot() {
  Serial.println("Küsin ROBOTILT");
  digitalWrite(ROBOT_PIN, HIGH);
  delay(ROBOTI_SIGNAALI_AEG);
  digitalWrite(ROBOT_PIN, LOW);
  //delay(ROBOTI_TOOMISE_AEG);
}

/*
 Ultraheli andur möödab kaugust,
 et teada saada, kas on uusi detaile.
*/
bool is_details() {
  double kaugus = measure_distance(US1_TRIG_PIN, US1_ECHO_PIN);
  Serial.print("Kaugus: ");
  Serial.print(kaugus);
  // //TODO: kui kaugus on selgelt väga vale määra veaolek

  if (kaugus < MAX_DETAILI_KAUGUS && kaugus > MIN_DETAILI_KAUGUS) {
    Serial.print(" -  Hea!\n");
    return true;
  }

  Serial.print(" -  Halb!\n");
  return false;
}


/*********************************************************
 Ultraheli kauguse mõõtmine
 returns distance cm float
**********************************************************/
double measure_distance(int trig_pin, int echo_pin) {
  // Valem
  // tmp_in_C = 21.0
  // heli_kiirus = 331.1 + (0.606 * tmp_in_C) == 343.826
  // distance_per_us = heli_kiirus / 10000.0 == 0.0343826
  double kaugused[MITUKORDA] = {0};

  for (size_t i = 0; i < MITUKORDA; i++) {
    // Clear trigger
    digitalWrite(trig_pin, LOW);
    delayMicroseconds(2);

    // Send 10µs pulse to trigger
    digitalWrite(trig_pin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig_pin, LOW);

    // Measure the echo pulse length
    long duration = pulseIn(echo_pin, HIGH);

    double distance = duration * 0.0343 / 2; // Or divide by 29.1
    Serial.print(i);
    Serial.print(" kaugus ");
    Serial.println(distance);
    kaugused[i] = distance;
    delay(25);  // Delay between measurements
  }

  double distance_cm = 0.0;
  // Arvuta keskmine kaugus
  for (int i = 0; i < MITUKORDA; i++) {
    distance_cm += kaugused[i];
  }

  distance_cm = distance_cm / double(MITUKORDA);

  return distance_cm;
}

/*
Rattad lükkavad detaile edasi teatud aja
*/
void liiguta_edasi() {
  Serial.println("LIIGUTA EDASI");
  // Liiguta edasi mootoreid
  run_step_motor(FORWARD, MOOTORI_EDASI_AEG, M1_SPEED, M1_PULSE_PIN, M1_DIRECTION_PIN);
  delay(100);
  // Liiguta mootoreid nõks tagasi
  run_step_motor(BACK, MOOTORI_TAGASI_AEG, M1_SPEED, M1_PULSE_PIN, M1_DIRECTION_PIN);
  delay(100);
}

/*
Ära tee midagi
*/
void oota(int aeg) {
  Serial.println("Ootan");
  delay(aeg);
}

/*
Suruõhuga kolb lükkab detaile edasi
Juhitud releega
*/
void lykka() {
  Serial.println("LÜKKA");

  // Lülitab suruõhu kolvi sisse
  push_relee_ON();
  delay(LYKKAMISE_AEG);

  // Lülitab suruõhu kolvi välja
  push_relee_OFF();

  loendur_kokku++;
  Serial.print("Kokku lükkatud: ");
  Serial.print(loendur_kokku);
  Serial.print(" detaili\n");
}

void oota_kolbi_tagasi() {
  // Oota kuni suruõhu kolb tuleb tagasi
  Serial.print("Ootan kolbi tagasi:\n");
  while (digitalRead(KOLVI_LIMIT_PIN) == LOW) {
    // Oota, kuni lüliti on vajutatud
    delay(1);
  }
  Serial.print("OK!\n");
}

void oota_laua_vabastamist() {
  // Oota kuni laud on vaba uue detaili jaoks
  Serial.print("Ootan laua vabastamist:\n");
  while (digitalRead(TEINE_LIMIT_PIN) == HIGH) {
    // Oota, kuni lüliti on vajutatud
    delay(1);
  }
  Serial.print("OK!\n");

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