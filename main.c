#include <avr/io.h>
#include <math.h>
#include <avr/interrupt.h>

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

#define COUNT_MAX TICKS_PER_SECOND
#define MAX_DEPTH 255

#define RMODE_DEPTH 	1
#define RMODE_SENS	2
#define RMODE_UP	4
#define RMODE_DOWN	8
#define RMODE_THRESH 	2

uint8_t reading_mode;
uint8_t _ocr1c = 0;

//#define SET_OCR1C(x) _ocr1c = OCR1C = x
void SET_OCR1C(uint8_t x){
	_ocr1c = OCR1C = x;
}
uint8_t in_reading_mode(uint8_t bit){
	return (reading_mode & bit);
}

void enter_reading_mode(uint8_t bit){
	reading_mode = reading_mode | bit;
}

void exit_reading_mode(uint8_t bit){
	reading_mode = (reading_mode & ~bit);
}

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
	ADMUX = ADMUX_SETTINGS | admux_bits;
	ADCSRA |= (1 << ADSC);
	r = (ADCH << 8) | ADCL;
	return r;
}

#define FLAG_INCREASING 0b1
#define FLAG_WAVEFORM 0b110
#define SHIFT_WAVEFORM 2
	#define WAVEFORM_OFF 0b0
	#define WAVEFORM_SQUARE 0b1
	#define WAVEFORM_TRIANGLE 0b10
	//I'm not implementing this but I'm putting support for it so some guy can come along in 10 years and do it for me
	#define WAVEFORM_SINE 0b11
void do_waveform(uint16_t counter, uint8_t depth, uint8_t waveform){
	switch (waveform){
		case WAVEFORM_OFF:
			SET_OCR1C(0);
		case WAVEFORM_SQUARE:
			if (counter == COUNT_MAX){
				SET_OCR1C(depth / (round((float) MAX_DEPTH / 255)));
			}
			if (counter == 0){
				SET_OCR1C(0);
			}
		break;
		case WAVEFORM_SINE:
		case WAVEFORM_TRIANGLE:
			SET_OCR1C(counter / (round((float) MAX_DEPTH / 255)));
		break;
	}
}
void compute_pwm(int depth, int up_time, int down_time){
	static uint8_t flags = FLAG_INCREASING | (WAVEFORM_SQUARE << FLAG_WAVEFORM);
	static uint16_t oldTicks = 0;
	static uint16_t counter;
	if (oldTicks == ticks) return;
	if (flags & FLAG_INCREASING){ //counting up
		counter += up_time;
		if (counter > COUNT_MAX){ //uint16_t can hold an entire ticks_per_second so we can do it the easy way and catch overflow
			counter = COUNT_MAX;
			flags = (flags) & ~(1 << FLAG_INCREASING);
		}
	} else {
		if (down_time >= counter) { //pre-detect overflow
			counter = 0;
			flags = flags | (1 << FLAG_INCREASING);
		}
	}
	do_waveform(counter, depth, (flags & FLAG_WAVEFORM) >> (SHIFT_WAVEFORM - 1));

int main(){
	uint16_t depth, sensitivity, up_time, down_time, t;
	setup_timers();
	setup_adc();
	while (1){
		t = read_adc(ADC_DEPTH);
		if (in_reading_mode(RMODE_DEPTH) || t - depth > RMODE_THRESH){
			enter_reading_mode(RMODE_DEPTH);
			depth = t;
		}
		t = read_adc(ADC_SENS);
		if (in_reading_mode(RMODE_SENS) || t - sensitivity > RMODE_THRESH){
			enter_reading_mode(RMODE_SENS);
			sensitivity = t;
		}
		t = read_adc(ADC_UP);
		if (in_reading_mode(RMODE_UP) || t - up_time > RMODE_THRESH){
			enter_reading_mode(RMODE_UP);
			up_time = t;
		}
		t = read_adc(ADC_DOWN);
		if (in_reading_mode(RMODE_DOWN) || t - depth > RMODE_THRESH){
			enter_reading_mode(RMODE_DOWN);
			down_time = t;
		}

		compute_pwm(depth, up_time, down_time);

	}
}

ISR(TIMER0_OVF_vect){
	ticks++;
	if (ticks >= TICKS_PER_SECOND){
		ticks = 0;
		seconds++;
	}
}
