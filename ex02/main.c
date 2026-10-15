#include <avr/io.h>

#define	my_sei()  __asm__ __volatile__ ("sei" ::: "memory")
#define __INTR_ATTRS __used__, __externally_visible__
#define my_ISR(vector) void vector (void) __attribute__ ((__signal__,__INTR_ATTRS)) ; void vector (void)

#define BIN_MAX 15

const uint8_t ledBits[4] = {PB0, PB1, PB2, PB4}; // store all led in an array

volatile uint8_t value = 0;

void showNumber(uint8_t n) {
	for (uint8_t i = 0; i < 4; i++) {
		if ((n >> i) & 1) // shift binaire et mask pour afficher que le bit de droite qui au depart est le premier, puis deuxieme avec le shift etc etc...
			PORTB |=  (1 << ledBits[i]);    // LED on
		else
			PORTB &= ~(1 << ledBits[i]);    // LED off
	}
}

static void startDebounce(void) {
	TCNT1 = 0;
	TIFR1 = (1 << OCF1A);
	TCCR1B |= (1 << CS12) | (1 << CS10);
}

my_ISR(INT0_vect) {                  // SW1
	EIMSK &= ~(1 << INT0);
	startDebounce();
}

my_ISR(PCINT2_vect) {                // SW2
	PCMSK2 &= ~(1 << PCINT20);
	startDebounce();
}

my_ISR(TIMER1_COMPA_vect) { //once debounce finished check state and update the value
	static uint8_t prev = (1 << PD2) | (1 << PD4);   // both released (pull-ups)

	TCCR1B &= ~((1 << CS12) | (1 << CS11) | (1 << CS10));       // stop Timer1
	uint8_t now = PIND & ((1 << PD2) | (1 << PD4));
	uint8_t pressed = prev & ~now;   // was high (released), now low (pressed)

	if ((pressed & (1 << PD2)) && value < BIN_MAX) value++;
	if ((pressed & (1 << PD4)) && value > 0)       value--;
	prev = now;

	EIFR  = (1 << INTF0);  EIMSK  |= (1 << INT0);      // re-arm both
	PCIFR = (1 << PCIF2);  PCMSK2 |= (1 << PCINT20);

	showNumber(value);
}

int main () {
	// Configuration
	DDRB |= (1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB4); //D1 D2 D3 D4 en sortie → LED
	DDRD &= ~((1 << PD4) | (1 << PD2));    // pd2 pd4 -> in
	PORTD |= (1 << PD4) | (1 << PD2);      // Activer pull-up sur PD2 PD4

	EICRA |=  (1 << ISC00);                 // any logical change
	EICRA &= ~(1 << ISC01);
	EIMSK |=  (1 << INT0);                  // on press trigger INT0_vect

	PCICR  |= (1 << PCIE2);     // enable pin-change group 2 (PCINT16..23 = PORTD)
	PCMSK2 |= (1 << PCINT20);   // only PD4 in that group

	// Timer1: CTC mode, stopped (no clock source yet)
	TCCR1A = 0; //clears/reset any leftover
	TCCR1B = (1 << WGM12);              // CTC, TOP = OCR1A
	OCR1A  = 468;                       // 16MHz/1024 = 64µs/tick → 469 ticks ≈ 30ms
	TIMSK1 = (1 << OCIE1A);             // interrupt on compare match A

	my_sei();
	while (1) {}
}
