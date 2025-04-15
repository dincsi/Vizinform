// Including the required Arduino libraries
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include "hun.h"
// Uncomment according to your hardware type
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 8
#define CS_PIN 3

MD_Parola myDisplay = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
String textToDisplay = "FOK-GYEM Vizinform ;)"; // A megjelenítendő szöveg

void setup()
{
  Serial.begin(9600); // Fontos a megfelelő baud rate beállítása
  // Initialize the object
  myDisplay.begin();
  // Set the brightness of the display (0-15)
  myDisplay.setIntensity(0);
  myDisplay.setFont(hun);
  // Clear the display
  myDisplay.displayClear();
  myDisplay.displayText(textToDisplay.c_str(), PA_CENTER, 100, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  Serial.write("Kijelző inicializálva\n");
}

void loop()
{
  // Belovasunk a soros poortról egy sort \n-ig és kcseréljük a szöveget
  if ((int)Serial.available() > 0)
  {
    String inputText = Serial.readStringUntil('\n');
    textToDisplay = inputText;
    myDisplay.displayText(textToDisplay.c_str(), PA_CENTER, 100, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
    Serial.write("Szöveg frissítve: ");
    Serial.println(textToDisplay);
  }
  if (myDisplay.displayAnimate())
  {
    myDisplay.displayReset();
  }
}
