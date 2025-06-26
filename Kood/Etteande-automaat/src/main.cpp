/*
 * Projekt:  Etteande automatiseerimine
 * Autor:    Tauno Erik
 * Algus:    2025.06.26
 * Muudetud: 2025.06.25
 */
#include <Arduino.h>

// 
int myFunction(int, int);

void setup() {
  // 
  int result = myFunction(2, 3);
}

void loop() {
  // 
}


int myFunction(int x, int y) {
  return x + y;
}