// Including the required Arduino libraries
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include "hun.h"
#include <EEPROM.h> // EEPROM könyvtár hozzáadása

// Uncomment according to your hardware type
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 8
#define CS_PIN 3

// EEPROM cím és maximális szöveghossz definíciója
#define EEPROM_ADDR_START 0
#define MAX_TEXT_LENGTH 50 // Maximális szöveghossz az EEPROM-ban (állítsd be szükség szerint)

MD_Parola myDisplay = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
String textToDisplay = "FOK-GYEM Vizinform ;)"; // Alapértelmezett szöveg

// --- EEPROM segédfüggvények ---
// String írása az EEPROM-ba
void writeStringToEEPROM(int addrOffset, const String &strToWrite)
{
  byte len = strToWrite.length();
  if (len > MAX_TEXT_LENGTH)
  { // Ellenőrizzük, hogy belefér-e
    len = MAX_TEXT_LENGTH;
  }
  EEPROM.write(addrOffset, len); // Elmentjük a hosszt az első bájtba
  for (int i = 0; i < len; i++)
  {
    EEPROM.write(addrOffset + 1 + i, strToWrite[i]);
  }
  // Lezáró nulla karakter (nem feltétlenül szükséges, ha a hosszt tároljuk, de jó gyakorlat)
  EEPROM.write(addrOffset + 1 + len, '\0');

  // EEPROM írás véglegesítése (ESP32/ESP8266 esetén szükséges lehet)
  // if (!EEPROM.commit()) {
  //   Serial.println("Hiba az EEPROM írásakor!");
  // }
}

// String olvasása az EEPROM-ból
String readStringFromEEPROM(int addrOffset)
{
  int newStrLen = EEPROM.read(addrOffset);
  // Ellenőrizzük, hogy a hossz érvényes-e (nem 0xFF és nem nagyobb a maximumnál)
  if (newStrLen == 0xFF || newStrLen == 0 || newStrLen > MAX_TEXT_LENGTH)
  {
    return ""; // Üres string, ha nincs érvényes adat
  }
  char data[newStrLen + 1]; // Buffer a karaktereknek + null terminátor
  for (int i = 0; i < newStrLen; i++)
  {
    data[i] = EEPROM.read(addrOffset + 1 + i);
  }
  data[newStrLen] = '\0'; // Null terminátor hozzáadása
  return String(data);
}
// --- EEPROM segédfüggvények vége ---

void setup()
{
  Serial.begin(9600); // Fontos a megfelelő baud rate beállítása

  // EEPROM inicializálása (ESP32/ESP8266 esetén szükséges lehet a méret megadásával)
  // EEPROM.begin(MAX_TEXT_LENGTH + 2); // Méret = max hossz + hossz bájt + null terminátor

  // Szöveg betöltése az EEPROM-ból
  String savedText = readStringFromEEPROM(EEPROM_ADDR_START);
  if (savedText.length() > 0)
  {
    textToDisplay = savedText;
    Serial.println("Betoltott szoveg az EEPROM-bol: " + textToDisplay);
  }
  else
  {
    Serial.println("Nincs mentett szoveg, alapertelmezett hasznalata.");
    // Opcionális: Első indításkor elmenthetjük az alapértelmezett szöveget
    // writeStringToEEPROM(EEPROM_ADDR_START, textToDisplay);
  }

  // Initialize the object
  myDisplay.begin();
  // Set the brightness of the display (0-15)
  myDisplay.setIntensity(0);
  myDisplay.setFont(hun);
  // Clear the display
  myDisplay.displayClear();
  // Megjelenítjük a betöltött vagy alapértelmezett szöveget
  myDisplay.displayText(textToDisplay.c_str(), PA_CENTER, 100, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  Serial.println("Kijelzo inicializalva."); // println használata a jobb olvashatóságért
}

void loop()
{
  // Belovasunk a soros poortról egy sort \n-ig és kcseréljük a szöveget
  if (Serial.available() > 0)
  {
    String inputText = Serial.readStringUntil('\n');
    inputText.trim(); // Eltávolítjuk a felesleges szóközöket/sortöréseket

    // Csak akkor frissítünk és mentünk, ha a szöveg változott, nem üres és belefér
    if (inputText.length() > 0 && inputText != textToDisplay && inputText.length() <= MAX_TEXT_LENGTH)
    {
      textToDisplay = inputText;
      Serial.println("Szoveg frissitve: " + textToDisplay); // println használata

      // Új szöveg mentése az EEPROM-ba
      writeStringToEEPROM(EEPROM_ADDR_START, textToDisplay);
      Serial.println("Szoveg elmentve az EEPROM-ba.");

      // Kijelző frissítése az új szöveggel
      myDisplay.displayText(textToDisplay.c_str(), PA_CENTER, 100, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
    }
    else if (inputText.length() > MAX_TEXT_LENGTH)
    {
      Serial.println("Hiba: A szoveg tul hosszu az EEPROM menteshez!");
    }
  }

  // Kijelző animáció futtatása
  if (myDisplay.displayAnimate())
  {
    myDisplay.displayReset(); // Animáció végén reset (ha szükséges)
  }
}
