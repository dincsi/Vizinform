// Including the required Arduino libraries
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include "hun.h"            // Hungarian font library
#include <EEPROM.h>         // EEPROM könyvtár hozzáadása
#include <SoftwareSerial.h> // SoftwareSerial könyvtár hozzáadása

// Uncomment according to your hardware type
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 12

// Lábkiosztás ARDUINO_AVR_UNO
// DIN - Data In / MOSI (11)
// CLK - Clock / SCL (13)
// CS - Chip Select / PMW (3)
// Softwareserial TX (4) RX (5)

#define CS_PIN 3 // Chip Select láb (PMW)
#define RX_PIN 8 // Softwareserial RX láb
#define TX_PIN 9 // Softwareserial TX láb

SoftwareSerial bluetoothSerial(RX_PIN, TX_PIN); // Softwareserial objektum létrehozása

// EEPROM cím és maximális szöveghossz definíciója
#define EEPROM_ADDR_START 0
#define MAX_TEXT_LENGTH 128 // Maximális szöveghossz az EEPROM-ban

#include "EEPROMHelper.h"

// EEPROM helper példány
EEPROMHelper eeprom(EEPROM_ADDR_START, MAX_TEXT_LENGTH);

#include "Clock.h"

// Simple software clock instance
Clock clock;
bool clockMode = false;
unsigned long lastClockDisplayMillis = 0;
String clockText = "00:00:00"; // persistent clock text shown on the display

MD_Parola myDisplay = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
String textToDisplay = "- - - Bragotron - Vizinform - - -"; // Alapértelmezett szöveg - ékezetes betűket \xHH alakban kell megadni

// EEPROM kezelést most az EEPROMHelper osztály végzi

void setup()
{
  Serial.begin(9600);           // Fontos a megfelelő baud rate beállítása
  bluetoothSerial.begin(38400); // Bluetooth baud rate beállítása

  Serial.println("=== HC-05 Bluetooth inicializalasa ===");

  // Várakozás a modul inicializálására
  delay(2000);

  // HC-05 név beállítása AT parancsokkal
  Serial.println("HC-05 nev beallitasa...");

  // 1. AT mód ellenőrzése
  Serial.println("AT mod ellenorzese...");
  bluetoothSerial.print("AT");
  delay(1000);

  if (bluetoothSerial.available())
  {
    String response = bluetoothSerial.readString();
    response.trim();
    Serial.println("AT valasz: " + response);

    if (response == "OK")
    {
      Serial.println("AT mod OK - nev beallitasa...");

      // 2. Név beállítása
      bluetoothSerial.print("AT+NAME=Bragotron");
      delay(1000);

      if (bluetoothSerial.available())
      {
        String nameResponse = bluetoothSerial.readString();
        nameResponse.trim();
        Serial.println("Nev beallitas valasz: " + nameResponse);

        if (nameResponse == "OK")
        {
          Serial.println("SIKER: Bluetooth nev beallitva - Bragotron");
        }
      }

      // 3. Ellenőrzés - név lekérdezése
      bluetoothSerial.print("AT+NAME?");
      delay(1000);
      if (bluetoothSerial.available())
      {
        String currentName = bluetoothSerial.readString();
        Serial.println("Jelenlegi nev: " + currentName);
      }
    }
    else
    {
      Serial.println("HIBA: AT mod nem elerheto!");
      Serial.println("Ellenorizd:");
      Serial.println("1. EN/KEY pin csatlakoztatva van a VCC-hez?");
      Serial.println("2. HC-05 LED lassan villog (2mp-enkent)?");
      Serial.println("3. Modul nincs parosítva mas eszkozzel?");
    }
  }
  else
  {
    Serial.println("HIBA: Nincs valasz a HC-05 modultol!");
    Serial.println("Ellenorizd a kabelezest es az AT modot!");
  }

  Serial.println("Bluetooth inicializalva.");

  // EEPROM inicializálása (ESP32/ESP8266 esetén szükséges lehet a méret megadásával)
  // EEPROM.begin(MAX_TEXT_LENGTH + 2); // Méret = max hossz + hossz bájt + null terminátor

  // Szöveg betöltése az EEPROM-ból
  String savedText = eeprom.readString(EEPROM_ADDR_START);
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
  myDisplay.setIntensity(0); // increase from 0 (very dim) to visible
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
  if (bluetoothSerial.available() > 0)
  {
    // String inputText = Serial.readStringUntil('\n');

    String inputText = bluetoothSerial.readString(); // Bluetooth bemenet olvasása
    inputText.trim();                                // Eltávolítjuk a felesleges szóközöket/sortöréseket

    // ha a szöveg paranccsal (@set) kezdődik
    if (inputText.startsWith("@set"))
    {
      // megnézzük az utána következő részt és ha
      // - clock akkor beállítjuk az órát a megadott időre és óra üzemmódba kapcsolunk
      // - text akkor normál szöveg módba kapcsolunk és a megadott szöveget jelenítjük meg
      if (inputText.startsWith("@set clock"))
      {
        // Expected format: "@set clock hh:mm:ss"
        int prefixLen = 11; // length of "@set clock"
        String rest = "";
        if (inputText.length() > prefixLen)
        {
          rest = inputText.substring(prefixLen);
        }
        rest.trim();
        if (rest.length() >= 8)
        {
          int hh = rest.substring(0, 2).toInt();
          int mm = rest.substring(3, 5).toInt();
          int ss = rest.substring(6, 8).toInt();
          clock.setTime((uint8_t)hh, (uint8_t)mm, (uint8_t)ss);
          clockMode = true;
          Serial.print("Ora beallitva: ");
          Serial.println(clock.getTime());
          // Update persistent text and show on display
          clockText = clock.getTime();
          myDisplay.displayClear();
          myDisplay.displayReset();
          myDisplay.displayText(clockText.c_str(), PA_CENTER, 100, 0, PA_PRINT);
        }
        else
        {
          Serial.println("Hiba: id formatuma: hh:mm:ss (pl. @set clock 14:30:00)");
        }
      }
      else if (inputText.startsWith("@set text"))
      {
        // Szöveg mód beállítása
        String newText = inputText.substring(9); // "@set text " hosszúságú rész levágása
        newText.trim();
        if (newText.length() > 0 && inputText != textToDisplay && newText.length() <= MAX_TEXT_LENGTH)
        {
          textToDisplay = newText;
          Serial.println("Szoveg mod beallitva: " + textToDisplay);

          // Új szöveg mentése az EEPROM-ba
          eeprom.writeString(EEPROM_ADDR_START, textToDisplay);
          Serial.println("Szoveg elmentve az EEPROM-ba.");

          // Kijelző frissítése az új szöveggel
          // myDisplay.displayText(textToDisplay.c_str(), PA_CENTER, 100, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
          clockMode = false; // Kilépünk az óra módból, ha szöveget állítunk be
          myDisplay.displayClear();
          myDisplay.displayReset();
          myDisplay.displayText(textToDisplay.c_str(), PA_CENTER, 100, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
        }
        else
        {
          Serial.println("Hiba: A szoveg tul hosszu vagy ures az EEPROM menteshez!");
        }
      }
      else
      {
        Serial.println("Hiba: Ismeretlen @set parancs!");
      }
    }
  }

  // Kijelző animáció futtatása
  if (myDisplay.displayAnimate())
  {
    myDisplay.displayReset(); // Animáció végén reset (ha szükséges)
  }

  // Clock update and display (if enabled)
  clock.update();
  if (clockMode)
  {
    // Update display once per second
    if (millis() - lastClockDisplayMillis >= 1000UL)
    {
      lastClockDisplayMillis = millis();
      clock.update();
      String t = clock.getTime();
      Serial.println(t);
      if (t != clockText)
      {
        clockText = t;
        myDisplay.displayText(clockText.c_str(), PA_CENTER, 100, 0, PA_PRINT);
      }
    }
  }
}
