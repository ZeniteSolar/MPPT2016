/**
 * @file handlers.c
 * @brief
 *
 * @author joaoantoniocardoso
 *
 * @date 8/26/2016
 *
 */

#include "handlers.h"
#include "lib/globalDefines.h"
#include "lib/ATmega328.h"
#include <avr/io.h>

// Global Variables
extern volatile systemFlags_t systemFlags;
extern volatile uint8 d;        // variavel do pwm -> 0 a 159
extern volatile int32 pi;		// variavel da potência de entrada
extern volatile int32 pi_old;	// variavel da potencia de entrada antiga
extern volatile int8 updown;	// variavel que define o estado atual do algoritimo P&B
extern volatile int32 ii_med;	// variavel da media da corrente de entrada
extern volatile int32 vi_med;	// variavel da media da tensão de entrada

ISR(ADC_vect)
{
	switch (systemFlags.channel){
		case 0:
			EMA(vi_med, ADC*vi_angle +vi_offset, EMA_GRADE);
			adcSelectChannel(ADC_CHANNEL_1);
			systemFlags.channel = 1;
			break;
		case 1:
			EMA(ii_med, ADC*ii_angle +ii_offset, EMA_GRADE);
			adcSelectChannel(ADC_CHANNEL_0);
			systemFlags.channel = 0;
			systemFlags.conversionReady = 1;
			break;
	}
}

ISR(TIMER0_COMPA_vect)
{
	if(systemFlags.conversionReady){
		printf("v: %6u, i: %6u, p: %6u, d: %6u\n", (uint16) vi_med, (uint16) ii_med, (uint16) pi, d);
		systemFlags.conversionReady = 0;
	}
}

ISR(TIMER0_COMPB_vect)
{
	adcStartConversion();
}

ISR(TIMER1_COMPA_vect)
{
	pi = (uint32)( ((uint16)vi_med)*((uint16)ii_med) );

	// Aplica limites a perturbação
	if( (d > D_MAX) || (d < D_MIN) || (pi < pi_old) )	updown ^=1;
	if(!updown) d -= D_STEP;
	else d += D_STEP;

    // Proteção para a condição de operação sem carga, que leva a
    // tensão da entrada do circuito (do regulador lm317) ao infinito,
    // estourando o capacitor da entrada e possivelmente danificando o
    // regulador.
    // Em tal condição, a potência é baixa pois a corrente de entrada
    // é baixa, porém esta característica também pode ocorrer em uma
    // operação comum na qual se tem pouca radiação solar incidente.
    // Ao invés de tentar uma distinção específica para este caso,
    // pode-se, a princípio, limitar a tensão máxima na saída, relativo
    // ao ciclo tarefa aplicado à tensão de entrada atual. Para tal:
    // vo = (d%)*vi/((d%)-1).
    // Para limitar, definimos uma tensão máxima na saída VO_MAX, logo,
    // podemos equacionar a seguinte comparação:
    // d*vi <= VO_MAX*(160-d)
    // Podemos simplesmente baixar o ciclo-tarefa até que a tensão
    // de saída abaixe de VO_MAX.
    //while( d*vi_med >= VO_MAX*(1023/48)*(160-d) ) d -= D_STEP;
    //while( d*((uint64)vi_med) -(VO_MAX*21)*(160-d) >= 0 ) d -= D_STEP;
	while( ((d*( (VO_MAX*21) +((uint64)vi_med) ) >= VO_MAX*3360)) && (d > D_MIN) && (d < D_MAX) ) d -= D_STEP;

    // Uma possível limitação para a potência de entrada pode ser
    // computada como a seguir:
    // vi_real = vi_med*48/1023;
    // ii_real = ii_med*25/1023;
    // pi_real = vi_med*ii_med*25*48/(1023*1023)
    // PI_MAX >= pi_real
    // PI_MAX*872 >= vi_med*ii_med = pi
    // TODO: Ainda assim é será necessário pensar na equação do painel
    // para garantir uma ação que altere o ciclo-tarefa a fim de limitar
    // a operação do conversor à uma potência do painel menor que PI_MAX.
	//if( pi >= PI_MAX*872 ) d -= D_STEP;

	// Aplica o PWM
	timer2SetCompareBValue(d);

	// recicla
	pi_old = pi;
}

