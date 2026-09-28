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
#define MOTOR_WORK_TIME = 3000  // ms


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
  OOTA,           // 0 - Ootab
  MOOTORID,  // 1 - Ütle robotile
  RATTAD_EDASI,   // 2 - Mootorid liiguvad //EDASI
  KOLB_LYKKAB,    // 3 - Lükkamise relee lülitatud //LYKKA
  VIGA,           // 4 - Viga
  KAS_ON_LAUDU,   // 5 - Kas on uusi detaile?
  VIIMASED        // 6 - Viimased detailid masinas
};

// Alg olek
static State next_step = OOTA;

/*************************************************
 Function prototypes
**************************************************/
int readSensorMajority(int sensorPin);



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

  // Test: Mootorid töötavad 28.09.2026
  // digitalWrite(OPTOCOUPLER_MOOTOR_PIN, HIGH);

  // Test: Andur 1
  // High = Rohelin
  // Low = Punane
  readSensorMajority(ANDUR_1_PIN) == HIGH ? Serial.println("ANDUR_1: HIGH") : Serial.println("ANDUR_1: LOW");
  readSensorMajority(ANDUR_2_PIN) == HIGH ? Serial.println("ANDUR_2: HIGH") : Serial.println("ANDUR_2: LOW");
  readSensorMajority(ANDUR_3_PIN) == HIGH ? Serial.println("ANDUR_3: HIGH") : Serial.println("ANDUR_3: LOW");
  delay(100);

  /*

  // Detect input signal
  if (digitalRead(SILINER_SWITCH_PIN) == HIGH && !timerRunning) {
    Serial.println("SILINER_SWITCH_PIN: HIGH");
    motor_state = 1;
    startTime = time_now;
    timerRunning = true;
  }

  // Check if 3 seconds have passed
  if (timerRunning && (time_now - startTime >= 3000)) {
    motor_state = 0;
    timerRunning = false;
  }

  if (motor_state == 1) {
    digitalWrite(OPTOCOUPLER_MOOTOR_PIN, HIGH);
  } else {
    digitalWrite(OPTOCOUPLER_MOOTOR_PIN, LOW);
  }*/
}  // loop end


/*******************************************************************
 Core 1 loop
 *******************************************************************/
void loop1() {
}

/**********************************************************************
 * Run the stepper motor for a number of steps
 * @param dir   Direction (CW or CCW)
 * @param steps Number of steps
 * @param speed Speed of the motor
 **********************************************************************/

 int readSensorMajority(int sensorPin) {
  int highCount = 0;

  for (int sample = 0; sample < 100; sample++) {
    if (digitalRead(sensorPin) == HIGH) {
      highCount++;
    }
    delay(1);
  }

  return highCount > 50 ? HIGH : LOW;
}