#include "unihiker_k10.h"
#include "AIRecognition.h"
#include "asr.h"

// Deklaracje obiektów
UNIHIKER_K10    k10;
AIRecognition   ai;
Music           music;
ASR             asr;
AHT20           aht20;

// Zmienne globalne
bool menu_active = true;        // Czy jesteśmy w menu
uint8_t menu_selection = 1;     // Wybrana opcja w menu (1-4)
uint8_t mode_switch = 0;        // Aktualny tryb pracy
bool mode_switch_flag = 0;

// Zmienne pomocnicze
uint8_t asr_cmd3 = 0;
uint8_t asr_cmd6 = 0;
float Sensor_lux = 0;
float Sensor_temp = 0;
float Sensor_humi = 0;

// Deklaracje funkcji
void drawMenu();
void onButtonAPressed();
void onButtonBPressed();
void enterSelectedMode();
void DF_mode_switch();

// Funkcja rysująca menu z podświetleniem wybranej opcji
void drawMenu() {
  k10.canvas->canvasClear();
  k10.setScreenBackground(0xFFFFFF);

  // Tytuł
  k10.canvas->canvasText("=== MENU WYBORU ===", 30, 20, 0x0000FF, k10.canvas->eCNAndENFont16, 50, 0);

  // Opcje menu - podświetlamy wybraną
  uint32_t color1 = (menu_selection == 1) ? 0xFF0000 : 0x000000;
  uint32_t color2 = (menu_selection == 2) ? 0xFF0000 : 0x000000;
  uint32_t color3 = (menu_selection == 3) ? 0xFF0000 : 0x000000;
  uint32_t color4 = (menu_selection == 4) ? 0xFF0000 : 0x000000;

  // Rysujemy ramki wokół wybranej opcji
  if (menu_selection == 1) {
    k10.canvas->canvasRectangle(10, 60, 220, 30, 0xFF0000, 0xFFFFFF, false);
  }
  k10.canvas->canvasText("1. Camera + AI Face", 20, 70, color1, k10.canvas->eCNAndENFont16, 50, 0);

  if (menu_selection == 2) {
    k10.canvas->canvasRectangle(10, 100, 220, 30, 0xFF0000, 0xFFFFFF, false);
  }
  k10.canvas->canvasText("2. Voice Recognition", 20, 110, color2, k10.canvas->eCNAndENFont16, 50, 0);

  if (menu_selection == 3) {
    k10.canvas->canvasRectangle(10, 140, 220, 30, 0xFF0000, 0xFFFFFF, false);
  }
  k10.canvas->canvasText("3. Environment Monitor", 20, 150, color3, k10.canvas->eCNAndENFont16, 50, 0);

  if (menu_selection == 4) {
    k10.canvas->canvasRectangle(10, 180, 220, 30, 0xFF0000, 0xFFFFFF, false);
  }
  k10.canvas->canvasText("4. QR Code / Info", 20, 190, color4, k10.canvas->eCNAndENFont16, 50, 0);

  // Instrukcje
  k10.canvas->canvasText("A - Wybierz", 30, 250, 0x666666, k10.canvas->eCNAndENFont16, 50, 0);
  k10.canvas->canvasText("B - Potwierdz", 30, 270, 0x666666, k10.canvas->eCNAndENFont16, 50, 0);

  k10.canvas->updateCanvas();
}

// Przycisk A - nawigacja w menu
void onButtonAPressed() {
  if (menu_active) {
    // Nawigacja w menu - przełącz na następną opcję
    menu_selection++;
    if (menu_selection > 4) {
      menu_selection = 1;  // Zapętl do pierwszej opcji
    }
    drawMenu();  // Odśwież menu z nowym podświetleniem
  } else {
    // Jeśli jesteśmy w trybie - wróć do menu
    menu_active = true;
    menu_selection = 1;
    mode_switch = 0;
    music.stopPlayAudio();
    asr_cmd3 = 0;
    asr_cmd6 = 0;
    k10.setBgCamerImage(false);
    drawMenu();
  }
}

// Przycisk B - potwierdzenie wyboru
void onButtonBPressed() {
  if (menu_active) {
    // Potwierdź wybór i wejdź w wybrany tryb
    enterSelectedMode();
  }
}

// Funkcja uruchamiająca wybrany tryb
void enterSelectedMode() {
  menu_active = false;
  mode_switch = menu_selection;

  // Wyczyść ekran przed przejściem do trybu
  k10.canvas->canvasClear();
  k10.canvas->canvasText("Loading mode...", 60, 150, 0x0000FF, k10.canvas->eCNAndENFont16, 50, 0);
  k10.canvas->updateCanvas();
  delay(500);

  // Uruchom wybrany tryb
  DF_mode_switch();
}

// Funkcja przełączająca tryby (oryginalna z modyfikacjami)
void DF_mode_switch() {
  if (mode_switch == 1) {
    // Tryb 1: Kamera + AI
    k10.canvas->canvasClear();
    k10.setBgCamerImage(true);
    ai.switchAiMode(ai.Face);
  }
  else if (mode_switch == 2) {
    // Tryb 2: Rozpoznawanie głosu
    mode_switch_flag = 0;
    while (!(mode_switch_flag == 1)) {delay(50);}
    k10.setBgCamerImage(false);
    k10.canvas->canvasClear();

    // Rysuj interfejs ASR
    k10.canvas->canvasText("Voice Recognition Mode", 20, 50, 0x0000FF, k10.canvas->eCNAndENFont16, 50, 0);
    k10.canvas->canvasText("Say commands:", 40, 100, 0x000000, k10.canvas->eCNAndENFont16, 50, 0);
    k10.canvas->canvasText("- Turn on/off light", 20, 130, 0x666666, k10.canvas->eCNAndENFont16, 50, 0);
    k10.canvas->canvasText("- Play animation", 20, 150, 0x666666, k10.canvas->eCNAndENFont16, 50, 0);
    k10.canvas->canvasText("- Play music", 20, 170, 0x666666, k10.canvas->eCNAndENFont16, 50, 0);
    k10.canvas->canvasText("- Play a game", 20, 190, 0x666666, k10.canvas->eCNAndENFont16, 50, 0);
    k10.canvas->updateCanvas();
  }
  else if (mode_switch == 3) {
    // Tryb 3: Monitoring środowiska
    k10.canvas->canvasClear();
    k10.canvas->canvasText("Environment Monitor", 30, 20, 0x0000FF, k10.canvas->eCNAndENFont16, 50, 0);
    k10.canvas->canvasText("Temp:", 20, 60, 0x000000, k10.canvas->eCNAndENFont16, 50, 0);
    k10.canvas->canvasText("Humi:", 20, 90, 0x000000, k10.canvas->eCNAndENFont16, 50, 0);
    k10.canvas->canvasText("Light:", 20, 120, 0x000000, k10.canvas->eCNAndENFont16, 50, 0);
    k10.canvas->updateCanvas();
  }
  else if (mode_switch == 4) {
    // Tryb 4: QR kod
    k10.setBgCamerImage(false);
    mode_switch_flag = 0;
    while (!(mode_switch_flag == 1)) {delay(50);}
    k10.canvas->canvasClear();
    k10.canvas->canvasText("For more features", 30, 190, 0x000000, k10.canvas->eCNAndENFont16, 50, 0);
    k10.canvas->canvasText("Scan QR code", 30, 210, 0x000000, k10.canvas->eCNAndENFont16, 50, 0);
    k10.canvas->updateCanvas();
  }
}

void setup() {
  k10.begin();
  k10.initScreen(2);

  // Inicjalizacja ASR
  asr.asrInit(CONTINUOUS, EN_MODE, 5000);
  while(asr._asrState == 0) {delay(100);}

  // Inicjalizacja AI
  ai.initAi();
  k10.initBgCamerImage();
  k10.setBgCamerImage(false);
  k10.creatCanvas();
  ai.switchAiMode(ai.Face);

  // Konfiguracja ekranu
  k10.setScreenBackground(0xFFFFFF);
  k10.rgb->brightness(round(2));

  // Dodaj komendy ASR
  asr.addASRCommand(1, "Turn on the light");
  asr.addASRCommand(2, "Turn off the light");
  asr.addASRCommand(3, "Play animation");
  asr.addASRCommand(5, "Play music");
  asr.addASRCommand(6, "Play a game");

  // Przypisz funkcje do przycisków
  k10.buttonA->setPressedCallback(onButtonAPressed);
  k10.buttonB->setPressedCallback(onButtonBPressed);

  // Wyświetl menu startowe
  delay(1000);
  drawMenu();
}

void loop() {
  if (menu_active) {
    // W menu - nic nie rób, czekaj na przyciski
    delay(100);
  } else {
    // Wykonuj logikę wybranego trybu
    if (mode_switch == 2) {
      // Logika trybu ASR
      if (asr.isWakeUp() && asr_cmd3 == 0 && asr_cmd6 == 0) {
        // Wake up animation
      }
      if (asr.isDetectCmdID(1)) {
        k10.rgb->write(0, 0x00FF00);
        k10.rgb->write(1, 0xFFFF00);
        k10.rgb->write(2, 0xFF0000);
      }
      if (asr.isDetectCmdID(2)) {
        k10.rgb->write(-1, 0x000000);
      }
      // ... pozostałe komendy ASR
    }
    else if (mode_switch == 3) {
      // Logika monitora środowiska
      Sensor_lux = k10.readALS();
      Sensor_temp = aht20.getData(AHT20::eAHT20TempC);
      Sensor_humi = aht20.getData(AHT20::eAHT20HumiRH);

      // Wyświetl dane
      k10.canvas->canvasRectangle(80, 60, 140, 20, 0xFFFFFF, 0xFFFFFF, true);
      k10.canvas->canvasRectangle(80, 90, 140, 20, 0xFFFFFF, 0xFFFFFF, true);
      k10.canvas->canvasRectangle(80, 120, 140, 20, 0xFFFFFF, 0xFFFFFF, true);

      String tempStr = String(Sensor_temp, 1) + " C";
      String humiStr = String(Sensor_humi, 1) + " %";
      String luxStr = String((int)Sensor_lux) + " lux";

      k10.canvas->canvasText(tempStr.c_str(), 90, 60, 0xFF0000, k10.canvas->eCNAndENFont16, 50, 0);
      k10.canvas->canvasText(humiStr.c_str(), 90, 90, 0x0000FF, k10.canvas->eCNAndENFont16, 50, 0);
      k10.canvas->canvasText(luxStr.c_str(), 90, 120, 0xFFFF00, k10.canvas->eCNAndENFont16, 50, 0);

      k10.canvas->updateCanvas();
      delay(1000);
    }
  }

  mode_switch_flag = 1;
  delay(50);
}
