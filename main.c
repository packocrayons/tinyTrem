#define ADMUX_PB5 0b0
#define ADMUX_PB2 0b1
#define ADMUX_PB4 0b10
#define ADMUX_PB3 0b11
#define ADMUX_SETTINGS 0

#define ADC_DEPTH 	ADMUX_PB5
#define ADC_SENS 	ADMUX_PB4
#define ADC_UP		ADMUX_PB2
#define ADC_DOWN	ADMUX_PB3

#define TICKS_PER_SECOND 4902

uint16_t ticks = 0, seconds = 0;

#define RMODE_DEPTH 	1
#define RMOD_SENS	2
#define RMODE_UP	4
#define RMODE_DOWN	8

uint8_t reading_mode;

void setup_timers(){
	PLLCSR = (1 << PCKE) | (1 << PLLE);
	while (!(PLLCSR & PLOCK)); //data sheet is vague here - does hardware set this? Will know if it hangs here
	DDRB |= (1 << PORTB1);
	TCCR1 = (1 << CTC1) | (1 << PWM1A) | (1 << COM1A1) | (1 << CS10);
	TCCR0A = 0;
	TCCR0B = (1 << CS01) | (1 << CS00);
	TIMSK = (1 << TOIE0);
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
	uint16_t depth, sensitivity, up_time, down_time, t;
	setup_timers();
	setup_adc();
	while (1){
		t = read_adc(ADC_DEPTH);
		if (in_reading_mode(RMODE_DEPTH) || tdepth - depth > RMODE_THRESH){
			enter_reading_mode(RMODE_DEPTH);
			depth = tdepth;
		}
		t = read_adc(ADC_SENS);
		if (in_reading_mode(RMODE_SENS) || t - sensitivity > RMODE_THRESH){
			enter_reading_mode(RMODE_SENS);
			sensitivity = t;
		}
		t = read_adc(ADC_UP)
		if (in_reading_mode(RMODE_UP) || t - up_time > RMODE_THRESH){
			enter_reading_mode(RMODE_UP);
			up_time = t;
		}
		t = read_adc(ADC_DEPTH)
		if (in_reading_mode(RMODE_DEPTH) || tdepth - depth > RMODE_THRESH){
			enter_reading_mode(RMODE_DEPTH);
			depth = tdepth;
		}

	}
}

ISR(TIMER0_OVF_vect){
	ticks++;
	if (ticks >= TICKS_PER_SECOND){
		ticks = 0;
		seconds++;
	}
}
