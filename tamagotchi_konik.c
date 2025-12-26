#include "unihiker_k10.h"
#include "AIRecognition.h"
#include "asr.h"

// Deklaracje funkcji
void drawHorse();
void drawStats();
void drawMenu();
void updateNeeds();
void checkSensors();
void feedHorse();
void playWithHorse();
void cleanHorse();
void checkSleep();
void updateMood();
void onButtonAPressed();
void onButtonBPressed();

// Deklaracje obiektów
UNIHIKER_K10    k10;
AIRecognition   ai;
Music           music;
ASR             asr;
AHT20           aht20;

// ============================================
// SYSTEM POTRZEB KONIKA (0-100)
// ============================================
uint8_t hunger = 80;        // Głód (100 = pełny, 0 = bardzo głodny)
uint8_t happiness = 80;     // Szczęście
uint8_t cleanliness = 80;   // Czystość
uint8_t energy = 80;        // Energia

// Nastrój konika (0=smutny, 1=normalny, 2=szczęśliwy)
uint8_t mood = 1;

// ============================================
// ZMIENNE SENSORÓW
// ============================================
float light_level = 0;      // Natężenie światła
float temperature = 0;      // Temperatura
float humidity = 0;         // Wilgotność
float accel_x = 0;          // Akcelerometr X
float accel_y = 0;          // Akcelerometr Y
float accel_z = 0;          // Akcelerometr Z
float accel_magnitude = 0;  // Siła ruchu

// ============================================
// ZMIENNE GRY
// ============================================
bool is_sleeping = false;   // Czy konik śpi
uint16_t step_count = 0;    // Licznik kroków
uint32_t last_update = 0;   // Ostatnia aktualizacja potrzeb
uint32_t last_step = 0;     // Ostatni krok
bool menu_active = false;   // Czy menu jest aktywne
uint8_t menu_selection = 0; // Wybrana opcja menu (0=Nakarm, 1=Stats, 2=Wyjdź)

// Progi akcelerometru
const float STEP_THRESHOLD = 1.5;      // Próg wykrycia kroku
const float SHAKE_THRESHOLD = 2.5;     // Próg wykrycia mycia/potrząsania
const float PET_THRESHOLD = 0.3;       // Próg wykrycia głaskania

// ============================================
// FUNKCJA: Rysowanie konika
// ============================================
void drawHorse() {
  // Wyczyść obszar konika
  k10.canvas->canvasRectangle(60, 80, 120, 100, 0xFFFFFF, 0xFFFFFF, true);

  // Rysuj konika w zależności od nastroju
  uint32_t color = 0x8B4513; // Brązowy

  if (is_sleeping) {
    // Konik śpiący (leży)
    k10.canvas->canvasText("  Z z z", 100, 70, 0x666666, k10.canvas->eCNAndENFont16, 50, 0);
    // Ciało
    k10.canvas->canvasRectangle(80, 140, 80, 30, color, color, true);
    // Głowa
    k10.canvas->canvasCircle(75, 145, 15, color, color, true);
    // Oczy zamknięte
    k10.canvas->canvasLine(70, 145, 75, 145, 0x000000);

  } else if (mood == 2) {
    // Konik szczęśliwy (skacze)
    // Ciało
    k10.canvas->canvasRectangle(90, 120, 60, 40, color, color, true);
    // Głowa
    k10.canvas->canvasCircle(85, 125, 20, color, color, true);
    // Oczy
    k10.canvas->canvasCircle(80, 120, 3, 0x000000, 0x000000, true);
    k10.canvas->canvasCircle(90, 120, 3, 0x000000, 0x000000, true);
    // Uśmiech
    k10.canvas->canvasLine(75, 130, 85, 135, 0xFF0000);
    k10.canvas->canvasLine(85, 135, 95, 130, 0xFF0000);
    // Nogi (w ruchu)
    k10.canvas->canvasLine(100, 160, 95, 175, color);
    k10.canvas->canvasLine(120, 160, 125, 175, color);
    k10.canvas->canvasLine(130, 160, 125, 175, color);
    k10.canvas->canvasLine(140, 160, 145, 175, color);

  } else if (mood == 0) {
    // Konik smutny
    // Ciało
    k10.canvas->canvasRectangle(90, 130, 60, 40, color, color, true);
    // Głowa opuszczona
    k10.canvas->canvasCircle(85, 145, 20, color, color, true);
    // Oczy
    k10.canvas->canvasCircle(80, 140, 3, 0x000000, 0x000000, true);
    k10.canvas->canvasCircle(90, 140, 3, 0x000000, 0x000000, true);
    // Smutek
    k10.canvas->canvasLine(75, 150, 85, 145, 0x0000FF);
    k10.canvas->canvasLine(85, 145, 95, 150, 0x0000FF);
    // Nogi
    k10.canvas->canvasLine(100, 170, 100, 185, color);
    k10.canvas->canvasLine(120, 170, 120, 185, color);
    k10.canvas->canvasLine(130, 170, 130, 185, color);
    k10.canvas->canvasLine(140, 170, 140, 185, color);

  } else {
    // Konik normalny
    // Ciało
    k10.canvas->canvasRectangle(90, 130, 60, 40, color, color, true);
    // Głowa
    k10.canvas->canvasCircle(85, 135, 20, color, color, true);
    // Oczy
    k10.canvas->canvasCircle(80, 130, 3, 0x000000, 0x000000, true);
    k10.canvas->canvasCircle(90, 130, 3, 0x000000, 0x000000, true);
    // Pysk
    k10.canvas->canvasLine(75, 140, 95, 140, 0x000000);
    // Nogi
    k10.canvas->canvasLine(100, 170, 100, 185, color);
    k10.canvas->canvasLine(120, 170, 120, 185, color);
    k10.canvas->canvasLine(130, 170, 130, 185, color);
    k10.canvas->canvasLine(140, 170, 140, 185, color);
  }

  // Ogon
  k10.canvas->canvasLine(150, 140, 170, 130, color);
  k10.canvas->canvasLine(150, 145, 170, 135, color);
}

// ============================================
// FUNKCJA: Rysowanie pasków statusu
// ============================================
void drawStats() {
  int y_offset = 20;
  int bar_height = 12;
  int bar_width = 200;

  // Wyczyść obszar statystyk
  k10.canvas->canvasRectangle(10, 10, 220, 60, 0xFFFFFF, 0xFFFFFF, true);

  // Głód
  k10.canvas->canvasText("Glod:", 15, y_offset, 0x000000, k10.canvas->eCNAndENFont16, 50, 0);
  k10.canvas->canvasRectangle(70, y_offset, bar_width, bar_height, 0xCCCCCC, 0xCCCCCC, true);
  uint32_t hunger_color = (hunger > 50) ? 0x00FF00 : (hunger > 20) ? 0xFFFF00 : 0xFF0000;
  k10.canvas->canvasRectangle(70, y_offset, (hunger * bar_width) / 100, bar_height, hunger_color, hunger_color, true);

  // Szczęście
  y_offset += 15;
  k10.canvas->canvasText("Radosc:", 15, y_offset, 0x000000, k10.canvas->eCNAndENFont16, 50, 0);
  k10.canvas->canvasRectangle(70, y_offset, bar_width, bar_height, 0xCCCCCC, 0xCCCCCC, true);
  uint32_t happy_color = (happiness > 50) ? 0x00FF00 : (happiness > 20) ? 0xFFFF00 : 0xFF0000;
  k10.canvas->canvasRectangle(70, y_offset, (happiness * bar_width) / 100, bar_height, happy_color, happy_color, true);

  // Czystość
  y_offset += 15;
  k10.canvas->canvasText("Czystosc:", 15, y_offset, 0x000000, k10.canvas->eCNAndENFont16, 50, 0);
  k10.canvas->canvasRectangle(70, y_offset, bar_width, bar_height, 0xCCCCCC, 0xCCCCCC, true);
  uint32_t clean_color = (cleanliness > 50) ? 0x00FF00 : (cleanliness > 20) ? 0xFFFF00 : 0xFF0000;
  k10.canvas->canvasRectangle(70, y_offset, (cleanliness * bar_width) / 100, bar_height, clean_color, clean_color, true);

  // Energia
  y_offset += 15;
  k10.canvas->canvasText("Energia:", 15, y_offset, 0x000000, k10.canvas->eCNAndENFont16, 50, 0);
  k10.canvas->canvasRectangle(70, y_offset, bar_width, bar_height, 0xCCCCCC, 0xCCCCCC, true);
  uint32_t energy_color = (energy > 50) ? 0x00FF00 : (energy > 20) ? 0xFFFF00 : 0xFF0000;
  k10.canvas->canvasRectangle(70, y_offset, (energy * bar_width) / 100, bar_height, energy_color, energy_color, true);
}

// ============================================
// FUNKCJA: Rysowanie menu
// ============================================
void drawMenu() {
  // Tło menu (półprzezroczyste)
  k10.canvas->canvasRectangle(40, 200, 160, 110, 0xEEEEEE, 0x000000, true);

  k10.canvas->canvasText("=== MENU ===", 75, 210, 0x0000FF, k10.canvas->eCNAndENFont16, 50, 0);

  // Opcje menu z podświetleniem
  if (menu_selection == 0) {
    k10.canvas->canvasRectangle(50, 235, 140, 20, 0xFFFF00, 0xFFFF00, true);
  }
  k10.canvas->canvasText("0. Nakarm", 60, 240, 0x000000, k10.canvas->eCNAndENFont16, 50, 0);

  if (menu_selection == 1) {
    k10.canvas->canvasRectangle(50, 260, 140, 20, 0xFFFF00, 0xFFFF00, true);
  }
  k10.canvas->canvasText("1. Statystyki", 60, 265, 0x000000, k10.canvas->eCNAndENFont16, 50, 0);

  if (menu_selection == 2) {
    k10.canvas->canvasRectangle(50, 285, 140, 20, 0xFFFF00, 0xFFFF00, true);
  }
  k10.canvas->canvasText("2. Zamknij", 60, 290, 0x000000, k10.canvas->eCNAndENFont16, 50, 0);
}

// ============================================
// FUNKCJA: Aktualizacja potrzeb (co ~30 sek)
// ============================================
void updateNeeds() {
  if (millis() - last_update < 30000) return; // Co 30 sekund
  last_update = millis();

  if (!is_sleeping) {
    // Potrzeby spadają z czasem
    if (hunger > 0) hunger--;
    if (cleanliness > 0) cleanliness--;
    if (energy > 2) energy -= 2;  // Energia spada szybciej

    // Szczęście zależy od innych potrzeb
    if (hunger < 30 || cleanliness < 30 || energy < 30) {
      if (happiness > 0) happiness--;
    }
  } else {
    // Podczas snu regeneracja energii
    if (energy < 100) energy += 5;
    if (energy > 100) energy = 100;
  }

  // Ogranicz wartości
  if (hunger > 100) hunger = 100;
  if (happiness > 100) happiness = 100;
  if (cleanliness > 100) cleanliness = 100;
  if (energy > 100) energy = 100;
}

// ============================================
// FUNKCJA: Sprawdzanie sensorów
// ============================================
void checkSensors() {
  // Czytaj czujniki
  light_level = k10.readALS();
  temperature = aht20.getData(AHT20::eAHT20TempC);
  humidity = aht20.getData(AHT20::eAHT20HumiRH);

  // Czytaj akcelerometr (zakładam że k10 ma metodę readAccel)
  // accel_x = k10.readAccelX(); // Musisz sprawdzić dokładną nazwę metody
  // accel_y = k10.readAccelY();
  // accel_z = k10.readAccelZ();

  // Oblicz siłę ruchu (magnitude)
  // accel_magnitude = sqrt(accel_x*accel_x + accel_y*accel_y + accel_z*accel_z);

  // TYMCZASOWO - symulacja ruchu dla testów
  accel_magnitude = random(0, 30) / 10.0;

  // Wykrywanie kroków (wzrosty przyspieszenia)
  if (accel_magnitude > STEP_THRESHOLD && millis() - last_step > 500) {
    step_count++;
    last_step = millis();

    // Spacer zwiększa szczęście i zmniejsza energię
    if (happiness < 100) happiness += 2;
    if (energy > 5) energy -= 1;

    // Wyświetl komunikat
    k10.canvas->canvasText("Spacer!", 10, 200, 0x00FF00, k10.canvas->eCNAndENFont16, 50, 0);
  }

  // Wykrywanie mycia (potrząsanie)
  if (accel_magnitude > SHAKE_THRESHOLD) {
    if (cleanliness < 100) cleanliness += 3;
    if (cleanliness > 100) cleanliness = 100;

    k10.canvas->canvasText("Mycie!", 10, 200, 0x00FFFF, k10.canvas->eCNAndENFont16, 50, 0);
  }

  // Wykrywanie głaskania (delikatny ruch)
  if (accel_magnitude > PET_THRESHOLD && accel_magnitude < STEP_THRESHOLD) {
    if (happiness < 100) happiness++;

    k10.canvas->canvasText("Glaskanie!", 10, 200, 0xFF69B4, k10.canvas->eCNAndENFont16, 50, 0);
  }

  // Sprawdź sen (ciemność)
  checkSleep();
}

// ============================================
// FUNKCJA: Sprawdzanie snu
// ============================================
void checkSleep() {
  // Jeśli ciemno (< 50 lux) i energia niska, konik zasypia
  if (light_level < 50 && energy < 40) {
    is_sleeping = true;
  } else if (light_level > 100) {
    // Jeśli jasno, budzi się
    is_sleeping = false;
  }
}

// ============================================
// FUNKCJA: Aktualizacja nastroju
// ============================================
void updateMood() {
  // Oblicz średnią wszystkich potrzeb
  int avg = (hunger + happiness + cleanliness + energy) / 4;

  if (avg > 70) {
    mood = 2; // Szczęśliwy
    k10.rgb->write(0, 0x00FF00); // Zielony
  } else if (avg > 40) {
    mood = 1; // Normalny
    k10.rgb->write(0, 0xFFFF00); // Żółty
  } else {
    mood = 0; // Smutny
    k10.rgb->write(0, 0xFF0000); // Czerwony
  }
}

// ============================================
// FUNKCJA: Karmienie
// ============================================
void feedHorse() {
  if (hunger < 90) {
    hunger += 20;
    if (hunger > 100) hunger = 100;

    k10.canvas->canvasText("Mniam mniam!", 70, 195, 0xFF6600, k10.canvas->eCNAndENFont16, 50, 0);
    delay(1000);
  } else {
    k10.canvas->canvasText("Jestem najedzony!", 50, 195, 0x666666, k10.canvas->eCNAndENFont16, 50, 0);
    delay(1000);
  }
}

// ============================================
// CALLBACK: Przycisk A - Menu/Nawigacja
// ============================================
void onButtonAPressed() {
  if (!menu_active) {
    // Otwórz menu
    menu_active = true;
    menu_selection = 0;
  } else {
    // Nawigacja w menu
    menu_selection++;
    if (menu_selection > 2) menu_selection = 0;
  }
}

// ============================================
// CALLBACK: Przycisk B - Akcja
// ============================================
void onButtonBPressed() {
  if (menu_active) {
    // Wykonaj akcję z menu
    if (menu_selection == 0) {
      // Nakarm
      menu_active = false;
      feedHorse();
    } else if (menu_selection == 1) {
      // Pokaż statystyki
      menu_active = false;
      k10.canvas->canvasClear();
      k10.canvas->canvasText("=== STATYSTYKI ===", 40, 100, 0x0000FF, k10.canvas->eCNAndENFont16, 50, 0);

      String steps = "Kroki: " + String(step_count);
      k10.canvas->canvasText(steps.c_str(), 60, 130, 0x000000, k10.canvas->eCNAndENFont16, 50, 0);

      String light = "Swiatlo: " + String((int)light_level) + " lux";
      k10.canvas->canvasText(light.c_str(), 60, 150, 0x000000, k10.canvas->eCNAndENFont16, 50, 0);

      String temp = "Temp: " + String((int)temperature) + " C";
      k10.canvas->canvasText(temp.c_str(), 60, 170, 0x000000, k10.canvas->eCNAndENFont16, 50, 0);

      k10.canvas->updateCanvas();
      delay(3000);
    } else if (menu_selection == 2) {
      // Zamknij menu
      menu_active = false;
    }
  }
}

// ============================================
// SETUP
// ============================================
void setup() {
  k10.begin();
  k10.initScreen(2);
  k10.creatCanvas();
  k10.setScreenBackground(0xFFFFFF);
  k10.rgb->brightness(5);

  // Przypisz przyciski
  k10.buttonA->setPressedCallback(onButtonAPressed);
  k10.buttonB->setPressedCallback(onButtonBPressed);

  // Inicjalizacja czujników
  // aht20.begin(); // Jeśli potrzebne

  // Wyświetl ekran powitalny
  k10.canvas->canvasClear();
  k10.canvas->canvasText("TAMAGOTCHI KONIK", 35, 140, 0x8B4513, k10.canvas->eCNAndENFont16, 50, 0);
  k10.canvas->canvasText("Loading...", 80, 160, 0x666666, k10.canvas->eCNAndENFont16, 50, 0);
  k10.canvas->updateCanvas();
  delay(2000);

  last_update = millis();
}

// ============================================
// LOOP
// ============================================
void loop() {
  // Aktualizuj potrzeby
  updateNeeds();

  // Sprawdź sensory
  checkSensors();

  // Aktualizuj nastrój
  updateMood();

  // Rysuj ekran
  k10.canvas->canvasClear();
  k10.setScreenBackground(0xFFFFFF);

  drawStats();
  drawHorse();

  // Wyświetl info o śnie
  if (is_sleeping) {
    k10.canvas->canvasText("Spi...", 100, 195, 0x6666FF, k10.canvas->eCNAndENFont16, 50, 0);
  }

  // Rysuj menu jeśli aktywne
  if (menu_active) {
    drawMenu();
  }

  k10.canvas->updateCanvas();

  delay(100); // Odśwież ~10 razy/s
}
