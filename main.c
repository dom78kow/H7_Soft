/*
 * main.c
 *
 *  Created on: 23-06-2018 _ update 29.10.2019
 *       Autor: DK
 *       ATtiny13A SS7 controller
 *       H7 light driver
 */

#include <avr/io.h>
#include <util/delay.h>
//#include <avr/interrupt.h>
//#include <stdlib.h>


uint8_t Machine_State = 0;
uint8_t Over_Current_counter = 0;
uint8_t Under_Current_counter = 0;

//uint16_t ADC_resistor = 0;
//uint16_t ADC_circuit = 0;

// definicje wejsc i wyjsc
#define ADC2_res (1<<PB4)					// signal from 2mOhm resistor
#define ADC3_cir (1<<PB3)					// signal from internal circuit
#define OUT_mos  (1<<PB0)					// output signal for MOS  transistor
#define OUT_led  (1<<PB2)					// output signal for OUT1, drive for 2N7002 - NEW!!!

#define MOS_on PORTB |= OUT_mos				// MOS On
#define MOS_off PORTB &= ~OUT_mos			// MOS Off

#define OUT1_on PORTB |= OUT_led			// OUT1 On		NEW!!!
#define OUT1_off PORTB &= ~OUT_led			// OUT1 Off		NEW!!!


// FUNCTION DECLARATION
void Start_Up(void);
void Warm_Up (void);
void Fully_Open(void);
void Watch_Current(void);
void Fully_Close(void);
void End_State(void);

void ADC_config(void);
uint16_t ADC2_resistor (void);
uint16_t ADC3_circuit (void);

// MAIN ****************************************************
int main(void)
{

// DO ONCE
	DDRB |= OUT_mos;				// out direction port 1 to DDR
	DDRB |= OUT_led;				// out direction port 1 to DDR NEW!!!
	ADC_config();					// ADC configuration
    OUT1_off;						// turn off OUT1 NEW!!!

// DO WHILE ***********************************************
	while(1)
	{

		switch (Machine_State)
		{
		  case 0:
			Start_Up();
		  break;

		  case 10:
			Warm_Up();
		  break;

		  case 20:
			Fully_Open();
		  break;

		  case 30:
			Watch_Current();
		  break;

		  case 40:
		  	Fully_Close();
		  break;

		  case 50:
			End_State();
		  break;

		  default:
			Machine_State = 0;

		}

	}
// WHILE END **********************************************

}
// MAIN END ************************************************

// STATE MACHIN FUNCTIONS
//0
void Start_Up(void) {

	for(int i=0; i<5; i++){

		MOS_on;
		_delay_us(120);
		MOS_off;
		_delay_us(80);
	}
	Machine_State = 10;
	_delay_us(100);

}
//10
void Warm_Up (void) {

	MOS_on;
	//OUT1_on;
	_delay_us(10);

	if(ADC2_resistor() < 56) {		// 30A on 2mOhm  = 60mV, 60mV * 1024 / 1100mV ---> 56 digits
		Over_Current_counter++;		// ADC conversion time = c.a 110us
	} else {
		Over_Current_counter = 0;
		Machine_State = 10;
	}

	if(Over_Current_counter >= 10) {
		Machine_State = 20;
	}

	MOS_off;
	//OUT1_off;
	_delay_us(80);

}
//20
void Fully_Open(void) {
	MOS_on;
	Over_Current_counter = 0;
	Machine_State = 30;
}
//30
void Watch_Current(void) {

	if(ADC2_resistor() > 56) {		// 30A on 2mOhm  = 60mV, 60mV * 1024 / 1100mV ---> 56 digits (62mV w realu)
		Over_Current_counter++;		// ADC conversion time = c.a 110us
	} else {
		Over_Current_counter = 0;
		Machine_State = 30;
	}

	if(ADC2_resistor() < 1) {		// 1A on 2mOhm  = 2mV, 2mV * 1024 / 1100mV ---> 2 digits
	   Under_Current_counter++;
	} else {
	   Under_Current_counter = 0;	// szumy na poziomie 5 mV - 2,5A !!!
	   Machine_State = 30;
	}

	if(Over_Current_counter >= 5) {
	   Machine_State = 40;
	}

	if(Under_Current_counter >= 5) {
	   Machine_State = 40;
	}
	_delay_us(10);

}
//40
void Fully_Close(void) {
	MOS_off;
	OUT1_on;
	Machine_State = 50;
}
//50
void End_State(void) {
	Machine_State = 50;
	_delay_us(100);
}

// CONFIG FUNCTIONS IMPLEMENTATION
uint16_t ADC2_resistor (void) {

	ADMUX |= (1<<MUX1);				// select ADC2
	ADCSRA |= (1<<ADSC);			// start conversion
	while(ADCSRA & (1<<ADSC));  	// waiting for end of conversion
	return ADCW;
}

uint16_t ADC3_circuit (void) {

	ADMUX |= (1<<MUX1)|(1<<MUX0);	// select ADC3
	ADCSRA |= (1<<ADSC);			// start conversion
	while (ADCSRA & (1<<ADSC));		// waiting for end of conversion
	return ADCW;
}

void ADC_config (void) {

	ADMUX |= (1<<REFS0);  //Internal voltage reference 1.1V
	ADCSRA |= (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1); //Enable ADC and set prescaler 64 (9600000/64=150kHz)
}



