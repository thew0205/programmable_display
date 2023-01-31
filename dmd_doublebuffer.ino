///*
//  Use the Marquee function to make an LED-sign type display
//
//  This marquee function scrolls in all directions, random distances for each
//  direction. If you edit the code in the "switch" statement below then you can
//  make a simpler marquee that only scrolls in one direction.
//*/
//
#include "switches_screen.h"
#include "Arial14.h"
#include "SystemFont5x7.h"
#include "Droid_Sans_12.h"
// Set Width to the number of displays wide you have
const int WIDTH = 1;

// You can change to a smaller font (two lines) by commenting this line,
// and uncommenting the line after it:
//const uint8_t *FONT = Arial14;
//const uint8_t *FONT = SystemFont5x7;
const uint8_t *FONT = Droid_Sans_12;
#include "DMD2_double_buffer.h"


SwitchesScreen screen(2, 1);
//
//
//SoftDMD dmd(2, 1); // DMD controls the entire display
//DMD_TextBox box(dmd);
//const int LED_pin = 13;
//volatile byte count;
//ISR(TIMER2_OVF_vect)
//{
//  flash();
//}
// the setup routine runs once when you press reset:
void setup() {


  Serial.begin(9600);
  //  dmd.setBrightness(255);
  //  dmd.selectFont(FONT);
  //  dmd.drawBox(5, 5, 11, 11);
  //  dmd.swapBuffers();
  //  dmd.begin();
  //  delay(2000);
  //  dmd.swapBuffers();
  //  Serial.println("tried");

//pinMode(LED_pin, OUTPUT);
//  digitalWrite(LED_pin, LOW);
//  
//  cli();
//  TCCR2A = 0; TCCR2B = 0;
//  //OCR2A = reload;
//  //TCCR2A = 1<<WGM21;
//  TCCR2B = (1 << CS22) | (1 << CS21) | (1 << CS20);
//  TIMSK2 = (1 << TOIE2);
//  sei();

  screen.init();
//  Serial.println("tried");
//  Serial.print("OCR2A: ");
//  Serial.println(OCR2A, BIN);
//  Serial.print("TCCR2A: ");
//  Serial.println(TCCR2A, BIN);
//  Serial.print("TCCR2B: ");
//  Serial.println(TCCR2B, BIN);
//  Serial.print("TIMSK2: ");
//  Serial.println(TIMSK2, BIN);
//  Serial.println("TIMER2 Setup Finished.");
//      screen. appendWord("Welcome");
//      screen. render('g');
  screen.mainLoop();


}

// the loop routine runs over and over again forever:
void loop() {
  //  const char *next = MESSAGE;
  //  while (*next) {
  //    //    Serial.print(*next);
  //    box.print(*next);
  //    delay(2000);
  //    next++;
  //    dmd.swapBuffers();
  //  }

}


//
//void flash()
//{
//
//  static boolean output = HIGH;
//  digitalWrite(LED_pin, output);
//  output = !output;
//}
