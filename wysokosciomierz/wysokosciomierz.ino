#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BMP3XX.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>

#define BMP_SCK 13
#define BMP_MISO 12
#define BMP_MOSI 11
#define BMP_CS 10

Adafruit_BMP3XX bmp;


Adafruit_SSD1306 display(4);

const int buttonPin1 = 4;  // Pin przycisku 1
const int buttonPin2 = 7;  // Pin przycisku 2

int digits[4] = {0, 0, 0, 0};  // Tablica do przechowywania cyfr
int currentDigit = 0;  // Aktualny indeks tablicy
float Height = 0;
float SEALEVELPRESSURE_HPA = 0;
bool buttonState1Last = HIGH;  // Poprzedni stan przycisku 1
bool buttonState2Last = HIGH;  // Poprzedni stan przycisku 2
bool powerSavingMode = false;

void setup() {
  pinMode(buttonPin1, INPUT_PULLUP);
  pinMode(buttonPin2, INPUT_PULLUP);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextColor(SSD1306_WHITE);

  bmp.begin_SPI(BMP_CS, BMP_SCK, BMP_MISO, BMP_MOSI);

  // Set up oversampling and filter initialization
  bmp.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
  bmp.setPressureOversampling(BMP3_OVERSAMPLING_4X);
  bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
  bmp.setOutputDataRate(BMP3_ODR_50_HZ);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.drawCircle(64, 16, 15, SSD1306_WHITE);
  display.setCursor(64-20, 16-6);
  display.print("  ...");
  display.display(); 
}

void loop() {
  bmp.performReading();
  int buttonState1 = digitalRead(buttonPin1);
  int buttonState2 = digitalRead(buttonPin2);

  // Zmiana cyfry w tablicy
  if (buttonState1 == LOW && buttonState1Last == HIGH && buttonState2 == HIGH) {
    digits[currentDigit] = (digits[currentDigit] + 1) % 10;
    Mydisplay(digits);
  }

  // Zmiana indeksu cyfry w tablicy
  if (buttonState2 == LOW && buttonState2Last == HIGH && buttonState1 == HIGH) {
    currentDigit = (currentDigit + 1) % 4;
  }

  // Sprawdź, czy wprowadzenie cyfr zostało zakończone
  if (buttonState1 == LOW && buttonState1Last == HIGH && buttonState2 == LOW && buttonState2Last == HIGH)
  {
    // Zakończono wprowadzanie cyfr, zamień tablicę na liczbę i wyświetl
    Height = digits[0] * 1000 + digits[1] * 100 + digits[2] * 10 + digits[3];
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.print("entered height:");
    display.setTextSize(2);
    display.setCursor(0, 12);
    display.print(Height);
    display.print(" m");
    display.display();
    delay(1000);

    SEALEVELPRESSURE_HPA = (bmp.pressure / 100.0 )* exp(Height / 7990.0);
    
    while(1)
    {
      buttonState1 = digitalRead(buttonPin1);

      if (buttonState1 == LOW && buttonState1Last == HIGH && buttonState2 == HIGH) {
        powerSavingMode = !powerSavingMode;

        if (powerSavingMode) {
          display.clearDisplay();
          display.display();
        }
        delay(1000);
      }

      if (!powerSavingMode) {
        bmp.performReading();
        display.clearDisplay();
        display.setCursor(0, 0);
        display.setTextSize(3);
        display.println(String(MyreadAltitude(SEALEVELPRESSURE_HPA),1) + "m");
        display.setTextSize(1);
        display.setCursor(0, 23);
        display.println("  " + String(bmp.temperature) + "C   " + String(bmp.pressure / 100.0) + "hPa");
        display.display();
        delay(3000);
      }
      buttonState1Last = buttonState1;
    }
    
  }

  // Zapisz obecny stan przycisków jako poprzedni
  buttonState1Last = buttonState1;
  buttonState2Last = buttonState2;

  delay(50);  // Opóźnienie dla stabilności odczytu
}


void Mydisplay(int (&digits)[4])
{
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.print("height:");
  display.setTextSize(2);
  display.setCursor(0, 12);
  for (int i = 0; i < 4; ++i) {
      display.print(digits[i]);
      display.print(" ");
    }
  display.display();
}

float MyreadAltitude(float SeaLevel)
{
  float atmospheric = bmp.pressure / 100.0F;
  return (-7990.0*log(atmospheric/SeaLevel));
}