#include "switches_screen.h"
#include "Arial14.h"
#include "SystemFont5x7.h"
#include "Droid_Sans_12.h"
#include "DMD2_double_buffer.h"

constexpr int WIDTH = 2;
const uint8_t *FONT = Droid_Sans_12;



SwitchesScreen screen(WIDTH, 1);
void setup() {
  Serial.begin(9600);
  screen.init();
}


void loop() {
  screen.mainLoop();
}
