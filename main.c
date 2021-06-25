#define ADMUX_PB5 0b0
#define ADMUX_PB2 0b1
#define ADMUX_PB4 0b10
#define ADMUX_PB3 0b11
#define ADMUX_SETTINGS 0

#define ADC_DEPTH 	ADMUX_PB5
#define ADC_SENS 	ADMUX_PB4
#define ADC_UP		ADMUX_PB2
#define ADC_DOWN	ADMUX_PB3

void setup_timers(){
	PLLCSR = (1 << PCKE) | (1 << PLLE);
	while (!(PLLCSR & PLOCK)); //data sheet is vague here - does hardware set this? Will know if it hangs here
	DDRB |= (1 << PORTB1);
	TCCR1 = (1 << CTC1) | (1 << PWM1A) | (1 << COM1A1) | (1 << CS10);
}

void setup_adc(){
	ADMUX = 
		ADMUX_SETTINGS |
		ADMUX_PB5;
	ADCSRA = (1 << ADEN) |
		(1 << ADPS2) |
		(1 << ADPS1) |
		(1 << ADPS0);

}

uint16_t read_adc(uint8_t admux_bits){
	uint16_t r;
	ADMUX = ADMUX_SETINGS | admux_bits;
	ADCSRA |= (1 << ADSC);
	r = (ADCH << 8) | ADCL;
	return r;
}

void main(){
	uint16_t depth, sensitivity, up_time, down_time
	setup_timers();
	setup_adc();
	while (1){

