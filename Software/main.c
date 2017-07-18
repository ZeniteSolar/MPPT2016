/**
 * @file main.c
 *
 * @author João Antônio Cardoso
 *
 * @date 16/11/2016
 *
 * @version 3.0
 */

#include <avr/interrupt.h>
#include <stdio.h>

#define F_CPU 16000000UL

#include "globalDefines.h"
#include "ema.h"
#include "ATmega328.h"
#include <avr/io.h>
#include "configs.h"

#define timer1_disable() timer1Config(TIMER_B_MODE_NORMAL, TIMER_B_CLOCK_DISABLE);
#define timer1_enable(prescaler) timer1SetCounterValue(0); timer1Config(TIMER_B_MODE_NORMAL, prescaler);

volatile status_t status = 	STATUS_TURNING_ON;
volatile error_t error = 	ERROR_NONE;

volatile systemFlags_t systemFlags;
volatile uint8	d = 0;				//! variavel do pwm -> 0 a 159
volatile int8	updown = 0;			//! variavel que define o estado atual do algoritimo P&B. (1=up)

//volatile int64	pi = 0;				//! variavel da potencia instantanea de entrada
volatile int64	pi_med = 0;			//! variavel da media da potencia de entrada
volatile int64	pi_med_old = 0;		//! variavel da media da potencia de entrada antiga

volatile int32	ii_med = 0;			//! variavel da media da corrente de entrada
volatile int32	vi_med = 0;			//! variavel da media da tensao de entrada
volatile int32	vo_med = 0;			//! variavel da media da tensao de saida

volatile uint16	mppt_freq_divider_counter = 0;
volatile uint16	safety_freq_divider_counter = 0;
volatile uint16	timer1_counter = 0;

volatile uint16 low_pi_counter = 0;

/**
 * @brief setup interrupts and IOs
 */
void ioSetup(void)
{
	//clrBit(PORT_opto, BIT_opto);		// to set as input
	//setBit(PORT_opto, BIT_opto);		// to enable pull-up

	//setBit(DDRD, PD4); 				// debug pin

	// INT_VO as input pull-up with an enabled interrupt for low level
	//pcint7_0ClearInterruptRequest();
	//pcint7_0Enable();
	//pcint4ActivateInterrupt(PORT_INPUT_PULL_UP);

	// PWM output
	setBit(PWM_DDR, PWM_BIT);
}

/**
 * @brief USART setup
 */
void usartSetup(void)
{
	usartConfig(USART_MODE_ASYNCHRONOUS,
			USART_BAUD_9600,
			USART_DATA_BITS_8,
			USART_PARITY_NONE,
			USART_STOP_BIT_SINGLE);
	usartEnableTransmitter();
	usartEnableReceiver();
	usartClearReceptionBuffer();
	usartActivateReceptionCompleteInterrupt();
	usartStdio();
}

/**
 * @brief timers setup
 * @ref https://et06.dunweber.com/atmega_timers/
 */
void timerSetup()
{
	// TIMER0 Configuration -> ADC & CONTROL LOOPS
	// on clear timer on compare mode: f_timer = f_clk/(N*( ocr + 1))
	// ocra = f_clk/(N*f_timer) -1 = 16000000/(8*10000) -1 = 199 //10kHz
	timer0SetCompareAValue(199);			// setup adc frequency at 10KHz
	timer0ActivateCompareAInterrupt();
	timer0Config(TIMER_A_MODE_FAST_PWM_OCRA, TIMER_A_PRESCALER_8);


	// TIMER2 Configuration -> PWM
	// fast pwm with top at ocra: f_timer = f_clk/(N*( ocra + 1)) and ocrb set the duty cycle
	// ocra = f_clk/(N*f_timer) -1 = 16000000/(1*100000) -1 = 159
	timer2SetCompareAValue(159);			// setup PWM frequency at 100KHz
	timer2SetCompareBValue(0);				// setup initial duty cycle to 0%
	timer2OutputConfig(TIMER_PORT_NON_INVERTING_MODE, TIMER_PORT_NON_INVERTING_MODE);	// pwm
	timer2Config(TIMER_A_MODE_FAST_PWM_OCRA, TIMER_B_PRESCALER_OFF);


	// TIMER1 Configuration -> delays
	// fcompA = fclk/(2*N*(OCRnA +1))
	// OCRnA = 2*N*fclk/fcompA -1
	timer1Config(TIMER_B_MODE_NORMAL, TIMER_A_CLOCK_DISABLE);
}

/*
 * @brief ADC setup
 */
void adcSetup()
{
	adcConfig(	ADC_MODE_SINGLE_CONVERSION,
				ADC_REFERENCE_POWER_SUPPLY,
				ADC_PRESCALER_128	);
	adcSelectChannel(ADC_CHANNEL_0);	//
	adcClearInterruptRequest();			// clear adc interrupts
	adcActivateInterrupt();				// activate adc interrupts
	adcEnable();						//
	adcStartConversion();				// this first conversion takes 20 cycles
}

/*
 * @brief it just call the setup funcs
 */
void setup(void)
{
	// Setup ADC
	adcSetup();

	// Setup USART
	usartSetup();

	// Setup inputs and outputs
	ioSetup();

	// Setup timers
	timerSetup();

	// Enable global interrupts
	sei();

	// starts a timer to waits the electrical transitory of the dc/dc converter
	timer1_enable(TIMER_A_PRESCALER_1024);
}

/**
 * @brief send system status via usart
 */
void sendStatus_USART(void)
{
	// To print float add C Linker Options: -Wl,-u,vfprintf -lprintf_flt -lm
	printf("vo: %0.2f, vi: %0.2f, ii: %0.2f, pi: %0.2f, d: %0.2f, s: %u, e: %u\n",
			vo_med*1.f,
			vi_med*1.f,
			ii_med*1.f,
			pi_med*1.f,
			d*100./159,
			status,
			error);
}

/*
 * @brief main
 */
int main(void)
{
	// Variable Initialization
	systemFlags.allFlags = 0;

	// Setup all the system
	setup();

	// Initial Duty Cycle Configuration
	d = 0;

	for(;;){
		#ifdef VERBOSE
		if(systemFlags.conversionReady){

			sendStatus_USART();

			systemFlags.conversionReady = 0;
		}
		#endif
	}

}

/*
 * @brief interrupt for int_vo -> protects against overvoltage in vo (detected as low level in PB4)
 */
ISR(PCINT0_vect)
{
	//if(isBitClr(PINB, PB4))
		//timer2SetCompareBValue(0);		// set Duty Cycle to 0
}

/*
 * @brief adc isr
 * @frequency	called at 10kHz
 * @duration 	5us
 */
ISR(ADC_vect)
{
	switch (systemFlags.channel){
		case ADC_CHANNEL_0:	// it takes about 4us
			EMA(vi_med, (ADC +(vi_offset))*(vi_angle), vi_ema_grade);
			adcSelectChannel(ADC_CHANNEL_1);
			systemFlags.channel = ADC_CHANNEL_1;
			break;
		case ADC_CHANNEL_1: // it takes about 4us
			EMA(ii_med, (ADC +(ii_offset))*(ii_angle), ii_ema_grade);
			adcSelectChannel(ADC_CHANNEL_2);
			systemFlags.channel = ADC_CHANNEL_2;
			break;
		case ADC_CHANNEL_2: // it takes about 4us
			EMA(vo_med, (ADC +(vo_offset))*(vo_angle), vo_ema_grade);
			adcSelectChannel(ADC_CHANNEL_0);
			systemFlags.channel = ADC_CHANNEL_0;
			systemFlags.conversionReady = 1;
			break;
	}
}

/**
 * @brief P&O algorithm
 */
inline void mppt_task(void)
{
	// Computes power input
	//pi = (uint32)( ((uint16)vi_med) * ((uint16)ii_med) );
	EMA(pi_med, (vi_med*ii_med +(pi_offset))*(pi_angle), vi_ema_grade);

	// Respects limits for duty Cycle
	if( (d > D_MAX) || (d < D_MIN) || (pi_med < pi_med_old) )	updown ^=1;

	// Apply a perturbation
	if(!updown) d -= D_STEP;
	else d += D_STEP;

	// recycles
	pi_med_old = pi_med;
}

/**
 * @brief safety task
 */
inline void safety_task(void)
{


		/* Prote��o para a condi��o de opera��o sem carga, que leva a
		 tens�o da entrada do circuito (do regulador lm317) ao infinito,
		 estourando o capacitor da entrada e possivelmente danificando o
		 regulador.
		 Em tal condi��o, a pot�ncia � baixa pois a corrente de entrada
		 � baixa, por�m esta caracter�stica tamb�m pode ocorrer em uma
		 opera��o comum na qual se tem pouca radia��o solar incidente.
		 Ao inv�s de tentar uma distin��o espec�fica para este caso,
		 pode-se, a princ�pio, limitar a tens�o m�xima na sa�da, relativo
		 ao ciclo tarefa aplicado � tens�o de entrada atual. Para tal:
		 vo = (d%)*vi/((d%)-1).
		 Para limitar, definimos uma tens�o m�xima na sa�da VO_MAX, logo,
		 podemos equacionar a seguinte compara��o:
		 d*vi <= VO_MAX*(160-d)
		 Podemos simplesmente baixar o ciclo-tarefa at� que a tens�o
		 de sa�da abaixe de VO_MAX.
		 */
			//while( d*vi_med >= VO_MAX*(1023/48)*(160-d) ) d -= D_STEP;
			//while( d*((uint64)vi_med) -(VO_MAX*21)*(160-d) >= 0 ) d -= D_STEP;
			//while( ((d*( (VO_MAX*21) +((uint64)vi_med) ) >= VO_MAX*3360)) && (d > D_MIN) && (d < D_MAX) ) d -= D_STEP;

		/* Uma poss�vel limita��o para a pot�ncia de entrada pode ser
		 computada como a seguir:
		 vi_real = vi_med*48/1023;
		 ii_real = ii_med*25/1023;
		 pi_real = vi_med*ii_med*25*48/(1023*1023)
		 PI_MAX >= pi_real
		 PI_MAX*872 >= vi_med*ii_med = pi
		 TODO: Ainda assim � ser� necess�rio pensar na equa��o do painel
		 para garantir uma a��o que altere o ciclo-tarefa a fim de limitar
		 a opera��o do conversor � uma pot�ncia do painel menor que PI_MAX.
		 */
			//if( pi >= PI_MAX*872 ) d -= D_STEP;

	if((pi_med < PI_MIN) || (ii_med < II_MIN)){
		if(low_pi_counter++ > low_pi_counter_limit){
			status = STATUS_TURNING_ON;
			low_pi_counter = 0;
		}
	}else{
		low_pi_counter--;// = 0;
	}


}

/**
 * @brief turning on task
 */
inline void turning_on_task(void)
{
	if(timer1GetCounterValue() >= ((uint16) 3*15625)){
		timer1_disable();

		error = ERROR_NONE;
		// checks the battery voltage
		if(vo_med < VO_MIN){
			error = ERROR_BATTERY_UNDERVOLTAGE;
		}else if(vo_med > VO_MAX){
			error = ERROR_BATTERY_OVERVOLTAGE;
		}
		// checks the panel voltage
		if(vi_med < VI_MIN){
			error = ERROR_NO_PANEL;
		}else if(vi_med > VI_MAX){
			error = ERROR_PANEL_OVERVOLTAGE;
		}

		// if there are no errors then waits the transitory of the
		// electronics and linearly increases the duty cycle
		if(error == ERROR_NONE){
			mppt_freq_divider_counter++;
			if(mppt_freq_divider_counter >= mppt_freq_divider*2){
				mppt_freq_divider_counter = 0;
				if(d < D_INITIAL)	d++;
				else 				status = STATUS_ON;
			}
		}
	}
}

/**
 * @brief status_on task
 */
inline void on_task(void)
{
	mppt_freq_divider_counter++;
	if(mppt_freq_divider_counter >= mppt_freq_divider){
		mppt_freq_divider_counter = 0;
		mppt_task();
	}

	safety_freq_divider_counter++;
	if(safety_freq_divider_counter >= safety_freq_divider){
		safety_freq_divider_counter = 0;
		safety_task();
	}
}

/**
 * @brief turning off task
 */
inline void turning_off_task(void)
{
	mppt_freq_divider_counter++;
	if(mppt_freq_divider_counter >= mppt_freq_divider*2){
		mppt_freq_divider_counter = 0;
		if(d > 0)	d--;
		else 		status = STATUS_OFF;
	}
}

/**
 * @brief off task
 */
inline void off_task(void)
{
	// verifica se chave continua ligada por 3 segundos,
	// entao religa o sistema

	if(timer1GetCounterValue() >= ((uint16) 3*15625)){
		timer1_disable();

		status = STATUS_TURNING_ON;
	}

}

/*
 * @brief this isr is used to read adc and for the mppt and power control loops
 * @frequency 10kHz
 * @duration min<=4us
 */
ISR(TIMER0_COMPA_vect)
{
	adcStartConversion();

	switch(status){
		case STATUS_TURNING_ON:
			//
			turning_on_task();
			break; /* end of case: STATUS_TURNING_ON */

		case STATUS_ON:
			//
			on_task();
			break; /* end of case: STATUS_ON */

		case STATUS_TURNING_OFF:
			//
			turning_off_task();
			break; /* end of case: STATUS_TURNING_OFF */

		case STATUS_OFF:
			//
			off_task();
			break; /* end of case: STATUS_OFF */

		default:
			break;
	}

	switch(error){
		case ERROR_NONE:
			break;

		case ERROR_NO_BATTERY:
			d = 0;//vi_med = ii_med = pi_med = 0;
			status = STATUS_TURNING_ON;
			break;

		case ERROR_BATTERY_OVERVOLTAGE:
			d = 0;//vi_med = ii_med = pi_med = 0;
			status = STATUS_TURNING_ON;
			break;

		case ERROR_BATTERY_UNDERVOLTAGE:
			break;

		case ERROR_NO_PANEL:
			d = 0;//vi_med = ii_med = pi_med = 0;
			status = STATUS_TURNING_ON;
			break;

		default:
			break;
	}

	// Aplica o PWM
	timer2SetCompareBValue(d);
}

/*
 * @brief
 */
//ISR(TIMER1_COMPA_vect){}
