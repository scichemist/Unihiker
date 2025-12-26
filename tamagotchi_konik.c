#include "unihiker_k10.h"
#include "AIRecognition.h"
#include "asr.h"

// Deklaracje funkcji
void drawHorse();
void drawStats();
void drawMenu();
void drawMainScreen();
void updateNeeds();
void checkSensors();
void feedHorse();
void checkSleep();
void updateMood();
void updateBrightness();
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
uint8_t prev_mood = 1;      // Poprzedni nastrój dla wykrycia zmiany

// ============================================
// ZMIENNE SENSORÓW
// ============================================
float light_level = 0;      // Natężenie światła
float prev_light = 0;       // Poprzednia wartość światła
float temperature = 0;      // Temperatura
float humidity = 0;         // Wilgotność
float accel_magnitude = 0;  // Siła ruchu

// ============================================
// ZMIENNE GRY
// ============================================
bool is_sleeping = false;   // Czy konik śpi
bool prev_sleeping = false; // Poprzedni stan snu
uint32_t step_count = 0;    // Licznik kroków
uint32_t last_update = 0;   // Ostatnia aktualizacja potrzeb
uint32_t last_step = 0;     // Ostatni krok
uint32_t last_sensor_check = 0; // Ostatnie sprawdzenie sensorów
bool menu_active = false;   // Czy menu jest aktywne
uint8_t menu_selection = 0; // Wybrana opcja menu
bool needs_redraw = true;   // Czy ekran wymaga odświeżenia
uint32_t last_action_time = 0; // Czas ostatniej akcji (do wygaszania komunikatów)
String action_message = ""; // Komunikat akcji

// Menu settings
uint8_t screen_brightness = 100; // Jasność ekranu (0-100)
bool auto_brightness = true;     // Auto-jasność włączona

// Progi akcelerometru - ZWIĘKSZONE dla mniejszej czułości
const float STEP_THRESHOLD = 3.0;      // Próg wykrycia kroku (było 1.5)
const float SHAKE_THRESHOLD = 4.5;     // Próg wykrycia mycia/potrząsania (było 2.5)
const float PET_THRESHOLD = 1.0;       // Próg wykrycia głaskania (było 0.3)

// Czasy aktualizacji (w milisekundach)
const uint32_t NEEDS_UPDATE_INTERVAL = 600000;  // 10 minut = 600000 ms
const uint32_t SENSOR_CHECK_INTERVAL = 1000;    // Co sekundę sprawdzaj sensory
const uint32_t ACTION_MESSAGE_DURATION = 150;   // Komunikaty przez 150ms (było 2000)

// ============================================
// FUNKCJA: Aktualizacja jasności ekranu
// ============================================
void updateBrightness() {
  if (auto_brightness) {
    if (light_level > 200) {
      // Jasno - pełna jasność
      k10.rgb->brightness(100);
    } else if (light_level > 50) {
      // Średnio - 50% jasności
      k10.rgb->brightness(50);
    } else {
      // Ciemno - minimum 15% żeby nie zgasło całkowicie
      k10.rgb->brightness(15);
    }
  } else {
    // Ręczna jasność
    k10.rgb->brightness(screen_brightness);
  }
}

// ============================================
// FUNKCJA: Rysowanie konika
// ============================================
void drawHorse() {
  // Rysuj konika w zależności od nastroju
  uint32_t color = 0x8B4513; // Brązowy

  if (is_sleeping) {
    // Konik śpiący (leży)
    k10.canvas->canvasText("  Z z z", 100, 70, 0xCCCCCC, k10.canvas->eCNAndENFont16, 50, 0);
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
  int y_offset = 10;
  int bar_height = 10;
  int bar_width = 150;

  // Głód
  k10.canvas->canvasText("Glod:", 10, y_offset, 0x000000, k10.canvas->eCNAndENFont16, 50, 0);
  k10.canvas->canvasRectangle(60, y_offset, bar_width, bar_height, 0xCCCCCC, 0xCCCCCC, true);
  uint32_t hunger_color = (hunger > 50) ? 0x00FF00 : (hunger > 20) ? 0xFFFF00 : 0xFF0000;
  k10.canvas->canvasRectangle(60, y_offset, (hunger * bar_width) / 100, bar_height, hunger_color, hunger_color, true);

  // Szczęście
  y_offset += 12;
  k10.canvas->canvasText("Radosc:", 10, y_offset, 0x000000, k10.canvas->eCNAndENFont16, 50, 0);
  k10.canvas->canvasRectangle(60, y_offset, bar_width, bar_height, 0xCCCCCC, 0xCCCCCC, true);
  uint32_t happy_color = (happiness > 50) ? 0x00FF00 : (happiness > 20) ? 0xFFFF00 : 0xFF0000;
  k10.canvas->canvasRectangle(60, y_offset, (happiness * bar_width) / 100, bar_height, happy_color, happy_color, true);

  // Czystość
  y_offset += 12;
  k10.canvas->canvasText("Czystosc:", 10, y_offset, 0x000000, k10.canvas->eCNAndENFont16, 50, 0);
  k10.canvas->canvasRectangle(60, y_offset, bar_width, bar_height, 0xCCCCCC, 0xCCCCCC, true);
  uint32_t clean_color = (cleanliness > 50) ? 0x00FF00 : (cleanliness > 20) ? 0xFFFF00 : 0xFF0000;
  k10.canvas->canvasRectangle(60, y_offset, (cleanliness * bar_width) / 100, bar_height, clean_color, clean_color, true);

  // Energia
  y_offset += 12;
  k10.canvas->canvasText("Energia:", 10, y_offset, 0x000000, k10.canvas->eCNAndENFont16, 50, 0);
  k10.canvas->canvasRectangle(60, y_offset, bar_width, bar_height, 0xCCCCCC, 0xCCCCCC, true);
  uint32_t energy_color = (energy > 50) ? 0x00FF00 : (energy > 20) ? 0xFFFF00 : 0xFF0000;
  k10.canvas->canvasRectangle(60, y_offset, (energy * bar_width) / 100, bar_height, energy_color, energy_color, true);

  // Licznik kroków
  y_offset += 15;
  String steps = "Kroki: " + String(step_count);
  k10.canvas->canvasText(steps.c_str(), 10, y_offset, 0x0000FF, k10.canvas->eCNAndENFont16, 50, 0);
}

// ============================================
// FUNKCJA: Rysowanie menu
// ============================================
void drawMenu() {
  // Tło menu - BIAŁY prostokąt z CZARNĄ ramką (filled=true)
  k10.canvas->canvasRectangle(20, 80, 200, 180, 0x000000, 0xFFFFFF, true);

  // Tytuł - granatowy
  k10.canvas->canvasText("=== MENU ===", 70, 95, 0x000080, k10.canvas->eCNAndENFont16, 50, 0);

  // Opcje menu z podświetleniem jako RAMKA (filled=false)
  int y = 120;

  // Nakarm
  if (menu_selection == 0) {
    k10.canvas->canvasRectangle(30, y - 3, 180, 18, 0xFF6600, 0xFFFFFF, false);
  }
  k10.canvas->canvasText("0. Nakarm", 40, y, 0x000080, k10.canvas->eCNAndENFont16, 50, 0);
  y += 22;

  // Spacer
  if (menu_selection == 1) {
    k10.canvas->canvasRectangle(30, y - 3, 180, 18, 0xFF6600, 0xFFFFFF, false);
  }
  k10.canvas->canvasText("1. Spacer", 40, y, 0x000080, k10.canvas->eCNAndENFont16, 50, 0);
  y += 22;

  // Głaskaj
  if (menu_selection == 2) {
    k10.canvas->canvasRectangle(30, y - 3, 180, 18, 0xFF6600, 0xFFFFFF, false);
  }
  k10.canvas->canvasText("2. Glaskaj", 40, y, 0x000080, k10.canvas->eCNAndENFont16, 50, 0);
  y += 22;

  // Myj
  if (menu_selection == 3) {
    k10.canvas->canvasRectangle(30, y - 3, 180, 18, 0xFF6600, 0xFFFFFF, false);
  }
  k10.canvas->canvasText("3. Myj", 40, y, 0x000080, k10.canvas->eCNAndENFont16, 50, 0);
  y += 22;

  // Statystyki
  if (menu_selection == 4) {
    k10.canvas->canvasRectangle(30, y - 3, 180, 18, 0xFF6600, 0xFFFFFF, false);
  }
  k10.canvas->canvasText("4. Statystyki", 40, y, 0x000080, k10.canvas->eCNAndENFont16, 50, 0);
  y += 22;

  // Zamknij
  if (menu_selection == 5) {
    k10.canvas->canvasRectangle(30, y - 3, 180, 18, 0xFF6600, 0xFFFFFF, false);
  }
  k10.canvas->canvasText("5. Zamknij", 40, y, 0x000080, k10.canvas->eCNAndENFont16, 50, 0);
}

// ============================================
// FUNKCJA: Rysowanie ekranu głównego
// ============================================
void drawMainScreen() {
  k10.canvas->canvasClear();

  // Ustaw tło - granatowe jeśli śpi, białe jeśli nie (ALE nie gdy menu aktywne!)
  if (is_sleeping && !menu_active) {
    k10.setScreenBackground(0x000033); // Granatowy podczas snu
  } else {
    k10.setScreenBackground(0xFFFFFF); // Biały normalnie i w menu
  }

  drawStats();
  drawHorse();

  // Wyświetl komunikat akcji jeśli aktywny
  if (action_message.length() > 0 && (millis() - last_action_time < ACTION_MESSAGE_DURATION)) {
    k10.canvas->canvasRectangle(30, 200, 180, 25, 0xFFFFCC, 0x000000, true);
    k10.canvas->canvasText(action_message.c_str(), 40, 210, 0xFF0000, k10.canvas->eCNAndENFont16, 50, 0);
  }

  // Wyświetl info o śnie
  if (is_sleeping) {
    k10.canvas->canvasText("Spi...", 10, 280, 0xCCCCFF, k10.canvas->eCNAndENFont16, 50, 0);
  }

  // Rysuj menu jeśli aktywne
  if (menu_active) {
    drawMenu();
  }

  k10.canvas->updateCanvas();
}

// ============================================
// FUNKCJA: Aktualizacja potrzeb (co 10 minut)
// ============================================
void updateNeeds() {
  if (millis() - last_update < NEEDS_UPDATE_INTERVAL) return;
  last_update = millis();

  if (is_sleeping) {
    // PODCZAS SNU - tylko regeneracja energii
    if (energy < 100) {
      energy += 10;
      if (energy > 100) energy = 100;
      needs_redraw = true;
    }
  } else {
    // PODCZAS CZUWANIA - potrzeby spadają WOLNIEJ
    if (hunger > 0) {
      hunger -= 1;  // Zmniejszono z 2
      needs_redraw = true;
    }
    if (cleanliness > 0) {
      cleanliness -= 1;  // Zmniejszono z 2
      needs_redraw = true;
    }
    if (energy > 0) {
      energy -= 1;  // Zmniejszono z 3 do 1
      needs_redraw = true;
    }

    // Szczęście zależy od innych potrzeb
    if (hunger < 30 || cleanliness < 30 || energy < 30) {
      if (happiness > 0) {
        happiness -= 1;  // Zmniejszono z 2
        needs_redraw = true;
      }
    }
  }
}

// ============================================
// FUNKCJA: Sprawdzanie sensorów
// ============================================
void checkSensors() {
  if (millis() - last_sensor_check < SENSOR_CHECK_INTERVAL) return;
  last_sensor_check = millis();

  // Czytaj czujniki
  light_level = k10.readALS();
  temperature = aht20.getData(AHT20::eAHT20TempC);
  humidity = aht20.getData(AHT20::eAHT20HumiRH);

  // Sprawdź czy światło się zmieniło znacząco
  if (abs(light_level - prev_light) > 50) {
    prev_light = light_level;
    updateBrightness();
  }

  // USUNIĘTO automatyczne wykrywanie ruchu - teraz tylko przez menu!
  // Akcelerometr nie wpływa już automatycznie na grę

  // Sprawdź sen
  checkSleep();
}

// ============================================
// FUNKCJA: Sprawdzanie snu
// ============================================
void checkSleep() {
  prev_sleeping = is_sleeping;

  // Jeśli ciemno (< 50 lux) i energia niska, konik zasypia
  if (light_level < 50 && energy < 50) {
    is_sleeping = true;
  } else if (light_level > 150 || energy > 90) {
    // Jeśli jasno lub energia wysoka, budzi się
    is_sleeping = false;
  }

  // Sprawdź czy stan się zmienił
  if (prev_sleeping != is_sleeping) {
    needs_redraw = true;
  }
}

// ============================================
// FUNKCJA: Aktualizacja nastroju
// ============================================
void updateMood() {
  prev_mood = mood;

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

  // Sprawdź czy nastrój się zmienił
  if (prev_mood != mood) {
    needs_redraw = true;
  }
}

// ============================================
// FUNKCJA: Karmienie
// ============================================
void feedHorse() {
  if (hunger < 80) {
    hunger += 30;
    if (hunger > 100) hunger = 100;

    action_message = "Mniam mniam!";
    last_action_time = millis();
    needs_redraw = true;
  } else {
    action_message = "Najedzony!";
    last_action_time = millis();
    needs_redraw = true;
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
    needs_redraw = true;
  } else {
    // Nawigacja w menu
    menu_selection++;
    if (menu_selection > 5) menu_selection = 0;  // 6 opcji (0-5)
    needs_redraw = true;
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
      // Spacer
      menu_active = false;
      if (!is_sleeping && energy > 5) {
        step_count += 50;  // Dodaj kroki
        if (happiness < 100) happiness += 10;
        if (energy > 5) energy -= 5;

        action_message = "Spacer! +50";
        last_action_time = millis();
        needs_redraw = true;
      } else {
        action_message = "Za zmeczony!";
        last_action_time = millis();
        needs_redraw = true;
      }

    } else if (menu_selection == 2) {
      // Głaskaj
      menu_active = false;
      if (!is_sleeping) {
        if (happiness < 100) happiness += 15;
        if (happiness > 100) happiness = 100;

        action_message = "Glaskanie <3";
        last_action_time = millis();
        needs_redraw = true;
      }

    } else if (menu_selection == 3) {
      // Myj
      menu_active = false;
      if (!is_sleeping) {
        if (cleanliness < 100) cleanliness += 20;
        if (cleanliness > 100) cleanliness = 100;

        action_message = "Mycie!";
        last_action_time = millis();
        needs_redraw = true;
      }

    } else if (menu_selection == 4) {
      // Statystyki
      menu_active = false;
      k10.canvas->canvasClear();
      k10.setScreenBackground(0xFFFFFF);

      k10.canvas->canvasText("=== STATYSTYKI ===", 40, 80, 0x0000FF, k10.canvas->eCNAndENFont16, 50, 0);

      String steps = "Kroki: " + String(step_count);
      k10.canvas->canvasText(steps.c_str(), 50, 110, 0x000000, k10.canvas->eCNAndENFont16, 50, 0);

      String light = "Swiatlo: " + String((int)light_level) + " lux";
      k10.canvas->canvasText(light.c_str(), 50, 130, 0x000000, k10.canvas->eCNAndENFont16, 50, 0);

      String temp = "Temp: " + String((int)temperature) + " C";
      k10.canvas->canvasText(temp.c_str(), 50, 150, 0x000000, k10.canvas->eCNAndENFont16, 50, 0);

      String hum = "Wilgotnosc: " + String((int)humidity) + " %";
      k10.canvas->canvasText(hum.c_str(), 50, 170, 0x000000, k10.canvas->eCNAndENFont16, 50, 0);

      k10.canvas->canvasText("Nacisnij A aby wrocic", 30, 250, 0x666666, k10.canvas->eCNAndENFont16, 50, 0);

      k10.canvas->updateCanvas();
      delay(5000);
      needs_redraw = true;

    } else if (menu_selection == 5) {
      // Zamknij menu
      menu_active = false;
      needs_redraw = true;
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

  // Wyświetl ekran powitalny
  k10.canvas->canvasClear();
  k10.canvas->canvasText("TAMAGOTCHI", 60, 130, 0x8B4513, k10.canvas->eCNAndENFont16, 50, 0);
  k10.canvas->canvasText("KONIK", 90, 150, 0x8B4513, k10.canvas->eCNAndENFont16, 50, 0);
  k10.canvas->updateCanvas();
  delay(2000);

  last_update = millis();
  last_sensor_check = millis();
  needs_redraw = true;
}

// ============================================
// LOOP
// ============================================
void loop() {
  // Aktualizuj potrzeby (co 10 minut)
  updateNeeds();

  // Sprawdź sensory (co sekundę)
  checkSensors();

  // Aktualizuj nastrój
  updateMood();

  // Rysuj ekran TYLKO gdy coś się zmieniło I nie ma aktywnego menu
  if (needs_redraw && !menu_active) {
    drawMainScreen();
    needs_redraw = false;
  } else if (needs_redraw && menu_active) {
    // Jeśli menu aktywne, narysuj tylko raz
    drawMainScreen();
    needs_redraw = false;
  }

  delay(150); // Opóźnienie 150ms jak użytkownik chciał
}
