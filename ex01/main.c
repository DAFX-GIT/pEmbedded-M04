#include <avr/io.h>

#define	my_sei()  __asm__ __volatile__ ("sei" ::: "memory")
#define __INTR_ATTRS __used__, __externally_visible__
#define my_ISR(vector) void vector (void) __attribute__ ((__signal__,__INTR_ATTRS)) ; void vector (void)


my_ISR(TIMER0_COMPA_vect) {         // every ~5ms: one step of the ramp // once run a hundred times, we get ~500ms
	static uint8_t duty = 0;
	static int8_t  step = 1;

	duty += step;
	if (duty == 100 || duty == 0)
		step = -step;                   // flip direction at the ends
	OCR1A = duty;                       // TOP = 100 → duty is directly a %
	if (duty == 0)                      // OCR1A = 0 still outputs a 1-tick spike each period
		TCCR1A &= ~(1 << COM1A1);       // → disconnect OC1A, PB1 falls back to PORTB (low) = real 0%
	else
		TCCR1A |= (1 << COM1A1);        // reconnect OC1A to the PWM
}

int main () {
	DDRB |= (1 << PB1);                 // PB1 (D2) en sortie → LED

	// Timer1: fast PWM mode 14, TOP = ICR1 = 100, prescaler 1 → 16MHz/101 ≈ 158kHz
	ICR1   = 100;
	OCR1A  = 0;                         // boot clear, this is set in the interrupt
	TCCR1A = (1 << WGM11);              // OC1A disconnected at boot (duty 0), the ISR connects it
	TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS10);

	// Timer0: CTC, prescaler 1024 → 64µs/tick, 78 ticks ≈ 4.99ms
	TCCR0A = (1 << WGM01);
	OCR0A  = 77;
	TCCR0B = (1 << CS02) | (1 << CS00);
	TIMSK0 = (1 << OCIE0A);             // trigger the interrupt on compare match A

	my_sei();
	while (1) {}
}
