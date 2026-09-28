/*
 * Projekt:  Juhtmoodul
 * Autor:    Tauno Erik
 * Algus:    2026.03.03
 * Muudetud: 2026.09.28
 */
#include <Arduino.h>
#include <pico/mutex.h>        // Race Condition Protection

auto_init_mutex(my_mutex);  // Race Condition Protection

/*************************************************
 Seaded
**************************************************/
#define JAH LOW
#define EI  HIGH
#define PUNANE   LOW
#define ROHELINE HIGH


// Input pins:
constexpr int SILINER_SWITCH_PIN = 14;
constexpr int ANDUR_1_PIN = 26;
constexpr int ANDUR_2_PIN = 27;
constexpr int ANDUR_3_PIN = 28;

// Output pins:
constexpr int OPTOCOUPLER_SILINDER_PIN = 22;
constexpr int OPTOCOUPLER_MOOTOR_PIN = 15;


unsigned long startTime = 0;
bool timerRunning = false;
int motor_state = 0; // 0-off


enum State {
  ALGUS,           // 0
  KAS_A3_VABA,     // 1 - Kas andur 3 on vaba
  KAS_A1_A2_VABA,  // 2 - Kas andur 1 ja 2 on vaba
  MOOTORID,        // 3 - Mootorid liiguvad
  KOLB_LYKKAB,     // 4 - Lükkamise relee lülitatud
  KOLB_TAGASI,     // 5 - Lükkamise relee lülitatud
  OOTA             // 6
};

// Alg olek
static State masin_olek = ALGUS;

/*************************************************
 Function prototypes
**************************************************/
int readSensorMajority(int sensorPin);
State oota(int wait_time, State next_state);


/*******************************************************************
 SETUP Core 0
 *******************************************************************/
void setup() {
  Serial.begin(115200);

  // Input pins
  pinMode(SILINER_SWITCH_PIN, INPUT);
  pinMode(ANDUR_1_PIN, INPUT);
  pinMode(ANDUR_2_PIN, INPUT);
  pinMode(ANDUR_3_PIN, INPUT);

  // Silinder pin
  pinMode(OPTOCOUPLER_SILINDER_PIN, OUTPUT);
  digitalWrite(OPTOCOUPLER_SILINDER_PIN, LOW);

  // Motor pin
  pinMode(OPTOCOUPLER_MOOTOR_PIN, OUTPUT);
  digitalWrite(OPTOCOUPLER_MOOTOR_PIN, LOW);
}

/*******************************************************************
 SETUP Core 1
 *******************************************************************/
void setup1() {
  

}



/*******************************************************************
 Core 0 loop
 *******************************************************************/
void loop() {
  // mutex_enter_blocking(&my_mutex);
  // mutex_exit(&my_mutex);
  static unsigned long time_now = millis();

  switch (masin_olek)
  {
  case ALGUS:
    Serial.println("[ ALGUS ]");
    masin_olek = oota(1000, KAS_A3_VABA);
    break;

  case KAS_A3_VABA:
    Serial.println("[ KAS_A3_VABA ]");
    // LOW = Punane = jah on detail
    if (readSensorMajority(ANDUR_3_PIN) == HIGH)
    {
      Serial.println("Ei ole detaili");
      masin_olek = oota(1000, KAS_A3_VABA);
    } 
    else
    {
      Serial.println("On detail");
      masin_olek = KAS_A1_A2_VABA;
    }
    break;

  case KAS_A1_A2_VABA:
    Serial.println("[ KAS_A1_A2_VABA ]");

    if (readSensorMajority(ANDUR_1_PIN) == HIGH 
    && readSensorMajority(ANDUR_2_PIN) == HIGH) 
    {
      Serial.println("A1 ja A2 on vabad");
      masin_olek = MOOTORID;
    }
    else if (readSensorMajority(ANDUR_1_PIN) == LOW 
    && readSensorMajority(ANDUR_2_PIN) == LOW) 
    {
      Serial.println("A1 ja A2 on detail");
      digitalWrite(OPTOCOUPLER_MOOTOR_PIN, LOW);
      masin_olek = KOLB_LYKKAB;
    }
    else
    {
      Serial.println("A1 ja A2 on erinevad");
      masin_olek = oota(1000, KAS_A1_A2_VABA);
    }
    break;

  case MOOTORID:
    Serial.println("[ MOOTORID ]");
    digitalWrite(OPTOCOUPLER_MOOTOR_PIN, HIGH);
    masin_olek = oota(1000, KAS_A1_A2_VABA);
    break;
  
  case KOLB_LYKKAB:
    Serial.println("[ KOLB_LYKKAB ]");
    digitalWrite(OPTOCOUPLER_SILINDER_PIN, HIGH);
    masin_olek = oota(3000, KOLB_TAGASI);
    break;
  
  case KOLB_TAGASI:
    Serial.println("[ KOLB_TAGASI ]");
    if (digitalRead(SILINER_SWITCH_PIN) == HIGH) {
      Serial.println("Silinder tagasi");
      masin_olek = KAS_A3_VABA;
    } else {
      Serial.print("-");
      masin_olek = oota(1000, KOLB_TAGASI);
    }
    break;
  }

}  // loop end


/*******************************************************************
 Core 1 loop
 *******************************************************************/
void loop1() {
}


/**
 * Reads a sensor and returns the majority value over 50 samples
 * @param sensorPin The pin to read
 * @return HIGH or LOW based on the majority of samples
 */
int readSensorMajority(int sensorPin) {
  int highCount = 0;

  for (int sample = 0; sample < 50; sample++) {
    if (digitalRead(sensorPin) == HIGH) {
      highCount++;
    }
    delay(5);
  }

  return highCount > 25 ? HIGH : LOW;
}

/**
 * Waits for a specified time and then returns the next state
 * @param next_state The state to return after waiting
 * @param wait_time The time to wait (in milliseconds)
 * @return The next state
 */
State oota(int wait_time, State next_state) {
  Serial.println("Ootan!");
  delay(wait_time);
  return next_state;
}