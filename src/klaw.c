#include "klaw.h"
#include "MKL05Z4.h"

// Mapa przycisków, tablica znaków
char map[4][4] = {
    {'x', 'v', 's', 'Z'},  
    {'B', 'a', 'A', 'g'},   
    {'G', 'f', 'F', 'E'},  
    {'d', 'D', 'c', 'C'}    
};

// Wiersze i kolumny matrycy klawiszy
char rows[4] = {R1, R2, R3, R4};
char columns[4] = {C1, C2, C3, C4};

void klaw_Init(void) {
    // Włączenie portu A
    SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK;

    // Ustawienie pinów kolumn jako wejścia z włączonym rezystorem pull-up
		// PORT_PCR_MUX(1) - opdowiedzialna za ustawienie pinu jako GPIO
		// PORT_PCR_PE_MASK - włączenie rezystora pull-up
		// PORT_PCR_PS_MASK - ustawienie rezystora pull-up jako aktywnego
		for (int i = 0; i < 4; i++) {
        PORTA->PCR[columns[i]] |= PORT_PCR_MUX(1) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK; 
        PORTA->PCR[rows[i]] |= PORT_PCR_MUX(1); // Ustawienie pinów wierszy jako GPIO
    }

    // Ustawienie wierszy jako wyjść
    for (int i = 0; i < 4; i++) {
        PTA->PDDR |= (1 << rows[i]); // Ustawienie kierunku pinów wierszy na wyjście
    }
}

char klaw_read(void) {
    char result = 0x00;	// Zmienna przechowująca wynik, początkowo 0x00 oznacza brak wciśniętego klawisza

    // Skanowanie wierszy
    for (uint8_t i = 0; i < 4; i++) {
        // Ustawienie aktualnego wiersza na niski poziom
        PTA->PDOR &= ~(1 << rows[i]);

        // Sprawdzanie, czy któryś z pinów kolumny jest niski, czyli czy jest wciśnięty
        for (uint8_t j = 0; j < 4; j++) {
            if ((PTA->PDIR & (1 << columns[j])) == 0) {
                result = map[i][j]; // Odczytanie znaku przypisanego do tego klawisza
            }
        }

        // Przywrócenie wiersza na wysoki poziom
        PTA->PDOR |= (1 << rows[i]);
    }

    return result; // Zwrócenie wyniku (klawisza, który został wciśnięty)
}
