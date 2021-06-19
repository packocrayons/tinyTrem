
void setup_timers(){
	PLLCSR = (1 << PCKE) | (1 << PLLE);
	while (!(PLLCSR & PLOCK)); //data sheet is vague here - does hardware set this? Will know if it hangs here
	DDRB |= (1 << PORTB1);
	TCCR1 = (1 << CTC1) | (1 << PWM1A) | (1 << COM1A1) | (1 << CS10);
}



void main(){
	setup_timers();
	setup_adc();
