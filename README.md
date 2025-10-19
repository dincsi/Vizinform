# Bragotron - Vizinform kijelző
Ardiuino // MAX7219 8x8 LED mártix alapú kijelző.

## Hardware

### Kijelző
Tetszölege számú kijelző összeköthető, a modulok darabszámát a `MAX_DEVICES` konstans tartalmazza.

### Bluetooth
HC-05 modul használata

![Arduino nano / HC-05](doc/images/hc05.png)

Mivel a HC-05 modul írás tüskéjét 3.3V vezérli, egy 1k és 2k ellenállásból álló feszültségosztóra van szükség. Az RX  és TX láb bekötésének helyét az `RX_PIN` és a `TX_PIN` értékei mutatják. AT módba kapcsoláshoz az `EN` lábat HI-ra kell kötni.

## Arduino kezelőszoftver
### Használat
Csatlakoztasd a kijelzőt egy számítógéphez és küld rá soros porton keresztül a megjeleníteni kívánt szöveget latin2 kódolással. 

Használhatod PlatformIO monitor funkcióját, majd:
- CTRL+T CTRL+A és írd be, hogy latin2
- gépeld be a szöveget és üss entert.

### Bluetooth
Használhatsz tetszőleges bluetooth terminal programot, ami képes latin2 kódoással küldeni adatot vagy a szöveget előbb konvertáld HEX formátumba.

### Magyar betűtípus
- hun.h
- [szerkesztés vagy új készítése](https://pjrp.github.io/MDParolaFontEditor)

## Új funkciók

### Óra mód
A kijelző mostantól képes az aktuális időt HH:MM formátumban megjeleníteni. Az időt a következő parancs segítségével állíthatod be Bluetooth-on keresztül:

```
@set clock hh:mm:ss
```

Példa:
```
@set clock 14:30:00
```
Ezután a kijelzőn "14:30" jelenik meg, és percenként frissül.

### Szöveg mód
A szöveg mód lehetővé teszi egyedi szöveg megjelenítését. Használja a következő parancsot:

```
@set text szoveg
```

Példa:
```
@set text Hello, világ!
```

### EEPROM használata
- Az utoljára beállított szöveg vagy óra mód mentésre kerül az EEPROM-ba, így újraindítás után automatikusan betöltődik.
- Az EEPROM címzése és maximális szöveghossza konfigurálható a `EEPROM_ADDR_START` és `MAX_TEXT_LENGTH` konstansokkal.

### Betűtípusok
- Az óra mód fix szélességű betűtípust használ a pontos megjelenítés érdekében.
- A szöveg mód a `hun.h` magyar ékezetes betűtípust használja.

### Parancsok összefoglalása
- `@set clock hh:mm:ss` - Óra mód beállítása.
- `@set text szoveg` - Szöveg mód beállítása.
