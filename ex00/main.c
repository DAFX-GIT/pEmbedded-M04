#include <avr/io.h>

#define	my_sei()  __asm__ __volatile__ ("sei" ::: "memory")
#define __INTR_ATTRS __used__, __externally_visible__
#define my_ISR(vector) void vector (void) __attribute__ ((__signal__,__INTR_ATTRS)) ; void vector (void)

my_ISR(INT0_vect) { //set a timer debounce

	EIMSK &= ~(1 << INT0);              // ignore further edges (bounces)
	TCNT1 = 0;                          // restart count from 0
	TIFR1 = (1 << OCF1A);               // clear any stale compare flag
	TCCR1B |= ((1 << CS12) | (1 << CS10));    // start Timer1 with prescaler 1024

	
}

my_ISR(TIMER1_COMPA_vect) { //once debounce finsihed check state and toggle light
	TCCR1B &= ~((1 << CS12) | (1 << CS11) | (1 << CS10));       // stop Timer1
	if (!(EICRA & (1 << ISC00)))        // we were waiting for a falling edge → it was a press
		PORTB ^= (1 << PB0);            // toggle D1
	EICRA ^= (1 << ISC00);              // falling <-> rising: now wait for the opposite edge
	EIFR = (1 << INTF0);                // drop edges latched while masked
	EIMSK |= (1 << INT0);               // listen to the button again
}

int main () {
	DDRB |= (1 << PB0);     // PB0 (D1) en sortie → LED

	DDRD &= ~(1 << PD2);    // PD2 (SW1) en entrée → Bouton
    PORTD |= (1 << PD2);    // Activer pull-up sur PD2

	EICRA |=  (1 << ISC01);                 // isc01 1 and isc00 0 set the falling edge mode = press (active-low button)
    EICRA &= ~(1 << ISC00);
    EIMSK |=  (1 << INT0); 					// on press trigger INT0_vect

	// Timer1: CTC mode, stopped (no clock source yet)
	TCCR1A = 0; //clears/reset any leftover
	TCCR1B = (1 << WGM12);              // CTC, TOP = OCR1A
	OCR1A  = 468;                       // 16MHz/1024 = 64µs/tick → 469 ticks ≈ 30ms
	TIMSK1 = (1 << OCIE1A);             // interrupt on compare match A

	my_sei();
	while (1) {

	}
}