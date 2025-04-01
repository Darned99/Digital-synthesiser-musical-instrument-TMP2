/*-----------------------------------------------------------------------------------------------------
					Technika Mikroprocesorowa 2 - projekt
					
					Projekt temat 16 - Instrument muzyczny
					
					Rozwiązania i funkcjonalności projektu:
						- 13 klawiszy dźwiękowych
						- 2 klawisze zmiany oktawy (góra-dół)
						- 1 klawisz - zmiana kształtu przebiegu wyjściowego (sekwencyjnie: sinus, piła, trójkąt)
						- zmiana głośności polem dotykowym
						- wyświetlana nazwa nuty
						- synteza DDS za pomocą DAC0
						
					Komponenty wykorzystane w projekcie:
						- FRDM-KL05Z wraz z polem dotykowym
						- wyświetlacz LCD wraz z I2C
						- głośnik 
						- klawiatura 4x4
						
					autor: Barłomiej Kisielewski
					wersja: 27.01.2025r.
------------------------------------------------------------------------------------------------------*/

#include "MKL05Z4.h"
#include "DAC.h"
#include "tsi.h"
#include "klaw.h"
#include "frdm_bsp.h"
#include "lcd1602.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>

#define DIV_CORE    8192	// Przerwanie co 0.120ms - fclk = 8192Hz, df = 8Hz
#define MASKA_10BIT 0x03FF

#define A4_FREQ 440.0 // Częstotliwość na podstawie której stosuje się strojenie w muzyce 
#define NOTE_COUNT 61	// Ilość dźwięków w projekcie 
#define TABLE_SIZE 1024	// Wielkość tablicy próbek

#define SIN_WAVE 0	
#define TRI_WAVE 1
#define SAW_WAVE 2

#define MIN_OCTAVE 0 
#define MAX_OCTAVE 4

volatile int16_t WaveTable[TABLE_SIZE];

volatile uint8_t on_off = 1;
volatile uint16_t slider = 100;
volatile uint16_t faza;
volatile uint16_t mod; // Modulator fazy
volatile float freq;

volatile int8_t currentOctave = 2;	// Obecna oktawa, 4 jest początkową 
volatile char current_key = 0;
volatile uint8_t key_pressed = 0; // Flaga dla naciśniętego klawisza
volatile char key;

// Dodane flagi i zmienne globalne dla komunikacji między przerwaniem a pętlą główną
volatile bool display_update_needed = false;
volatile bool key_state_changed = false;
volatile char last_key_change = 0;

volatile uint8_t wave_type = SIN_WAVE;  // Domyślnie sinus
volatile uint16_t modValues[NOTE_COUNT]; // Wielkość tablicy na podstawie ilości nut

// W programie zostały użyte zmienne dla półtonów -> C# / cis,des = c, D# / dis,es = d, F# / fis,ges = f, G# / gis,as = g, A# / ais, B = a
const char* noteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
static const char NOTE_KEYS[] = "CcDdEFfGgAaBZ"; // Klawisze odpowiadające dźwiękom


	void generateWaveTable(uint8_t wave_type) {
  
    if (wave_type == SIN_WAVE){
			
         for (uint16_t faza = 0; faza < TABLE_SIZE; faza++) {
            WaveTable[faza] = (sin(faza * 0.0061359231515) * 2047.0); // Ładowanie 1024 sztuk, 12-bitowych próbek sinus do tablicy
					}
    }
    else if (wave_type == TRI_WAVE) {
			
        for (uint16_t faza = 0; faza < TABLE_SIZE; faza++) {
            if (faza < TABLE_SIZE / 2) {
                WaveTable[faza] = (4.0 * 2047.0 * faza / TABLE_SIZE - 2047.0);
            } 
						else {
                WaveTable[faza] = (-4.0 * 2047.0 * (faza - TABLE_SIZE / 2) / TABLE_SIZE + 2047.0);
							}
				}
		}
     else if(wave_type == SAW_WAVE){
        
			 for (uint16_t faza = 0; faza < TABLE_SIZE; faza++) {
            WaveTable[faza] = (2.0 * 2047.0 * faza / TABLE_SIZE - 2047.0);
        }
			}
	}

	
	void calculateModValues(float df) {	// Funkcja obliczająca wartości mod dla każdej z nut 
			
		for (int n = 0; n < NOTE_COUNT; n++) {
					float freq = A4_FREQ * pow(2.0, (float)(n-33) / 12.0); // Wzór na częstotliwośc A4 * 2^(n/12), odpowiednie przesunięcie by zaczynać od 2 oktawy i mieć zakres do 6
					modValues[n] = (uint16_t)(freq / df);
			}
	}
	
	
	uint16_t getModFromKeyboard(char key) { // Funkcja, która po wciśnięciu klawisza przypisuje daną wartość mod, w zależności od konkretnego klawisza oraz jego obecnej oktawy
    uint16_t mod = 0;
    
    for (uint8_t i = 0; i < 13; i++) {
        if (key == NOTE_KEYS[i]) {
            mod = modValues[i + (currentOctave * 12)];
            key_pressed = 1;
            break;
        }
    }
		
    return mod;
	}

	
	// Funkcja do wyświetlenia informacji 
	void update_display(char key) { 
    char display[17];
    char waveName[4];
    int8_t indexInOctave = 0;
		bool isOctaveChange = (key == 'v' || key == 's' || key == 'x');
    
    
    switch (wave_type) {
        case SIN_WAVE:
            strcpy(waveName, "Sin");
            break;
        case TRI_WAVE:
            strcpy(waveName, "Tri");
            break;
        case SAW_WAVE:
            strcpy(waveName, "Saw");
            break;
        default:
            strcpy(waveName, "???");
            break;
    }
    

    for(uint8_t i = 0; i < 13; i++) {
        if(key == NOTE_KEYS[i]) {
            indexInOctave = i;
            break;
        }
    }
    
    int8_t displayOctave = currentOctave;
    if(key == 'Z') {
        displayOctave = currentOctave + 1; //Wyświetlenie obecnej oktawy jako większa, gdy wciśniemy S13 odpowiedzialny za C z następnej oktawy
    }
    
		
		 
    int16_t n = indexInOctave + (currentOctave * 12);
    
		if(n < NOTE_COUNT && key != 0 && !isOctaveChange) {
				freq = A4_FREQ * pow(2.0, (float)(n - 33) / 12.0);
			sprintf(display, "N:%-2s F:%.2fHz", noteNames[indexInOctave%12], freq); // Modulo dla C z wyższej oktawy
    } 
		else {
        sprintf(display, "N:   F:         ");
    }
    LCD1602_SetCursor(0, 0);
    LCD1602_Print(display);

    
    sprintf(display, "O:%d W:%s A:%d%% ", displayOctave + 2, waveName, slider);
    LCD1602_SetCursor(0, 1);
    LCD1602_Print(display);
	}

	
	void SysTick_Handler(void) { // Podprogram obsługi przerwania od SysTick'a
			
			if (on_off && key_pressed) { // Generuj dźwięk tylko, gdy klawisz jest wciśnięty
					uint16_t dac;
					int16_t sample;

					sample = WaveTable[faza];
					
					dac = (sample / 100)* slider + 0x0800; // Dany przebieg, odpowiednio sinusoidalny, trójkątny, piłokształtny
					DAC_Load_Trig(dac);

					faza += mod; // faza - generator cyfrowej fazy
					faza &= MASKA_10BIT; // rejestr sterujacy przetwornikiem, liczący modulo 1024 (N=10 bitów)
			}

				
					// Zwraca znak odpowiadącemu wciśniętemu klawiszowi, w przeciwnym wypadku 0
					char key = klaw_read();
					
					if (key != current_key) {
						key_state_changed = true;  // Ustaw flagę zmiany stanu
						last_key_change = key;     // Zapisz który klawisz się zmienił
        
						if (key == 0) {
								key_pressed = 0;
						} else {
								if (key == 'v') {
										if (currentOctave < MAX_OCTAVE) {
												currentOctave++;
										}
								} else if (key == 's') {
										if (currentOctave > MIN_OCTAVE) {
												currentOctave--;
										}
								} else if (key == 'x') {
										wave_type = (wave_type + 1) % 3;
										generateWaveTable(wave_type);
								} else {
										mod = getModFromKeyboard(key);
								}
						}
        
						display_update_needed = true;  // Ustawienie flagi na potrzeby wyświetlacza
						current_key = key;
				}
	}
	
	
	int main(void) {
			float df = (float)DIV_CORE / MASKA_10BIT;		// Rozdzielczność generatora delta fs

			klaw_Init(); // Inicjalizacja klawiatury matrycowej 4x4
			TSI_Init(); // Inicjalizacja pola dotykowego - slider
			LCD1602_Init(); // Inicjalizacja wyświetlacza LCD
			LCD1602_Backlight(TRUE); // Włączenie podświetalnia

			calculateModValues(df); // Przypisanie wartości mod dla każdego z dźwięków
			DAC_Init();	
			generateWaveTable(SIN_WAVE); // Tworzenie tablicy próbek dla danego przebiegu, bazowo dla sinusa
			
			faza = 0; // Ustawienie wartości początkowych generatora cyfrowej fazy i modulatora fazy
			//mod = 64;
			update_display(0); // Wyświetlenie informacji już od samego początku

			NVIC_SetPriority(SysTick_IRQn, 0);
			SysTick_Config(SystemCoreClock / DIV_CORE);

			uint8_t prevSlider = 0;

			while (1) {
					uint8_t w = TSI_ReadSlider(); // Odczytaj pozycję panelu dotykowego
					if (w != 0 && w != prevSlider) {
							slider = w; // Ustawienie amplitudy dźwięku
							update_display(current_key);
							prevSlider = w;
					}
					
						if (display_update_needed) {
								if (key_state_changed) {
										update_display(last_key_change);
										key_state_changed = false;  
								} else {
										update_display(current_key);
								}
								display_update_needed = false;  
						}
			}
	}
