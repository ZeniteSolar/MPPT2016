/**
 * @file main.c
 * @brief this is the main
 *
 * @author joaoantoniocardoso
 *
 * @date 8/26/2016
 *
 */

#include "main.h"

// Global Variables
volatile systemFlags_t systemFlags;
volatile uint8 d = 0;               //!< variavel do pwm -> 0 a 159
volatile int32 pi = 0;              //!< variavel da potência de entrada
volatile int32 pi_old = 0;          //!< variavel da potencia de entrada antiga
volatile int8 updown = 0;           //!< variavel que define o estado atual do algoritimo P&B
volatile int32 ii_med = 0;          //!< variavel da media da corrente de entrada
volatile int32 vi_med = 0;          //!< variavel da media da tensão de entrada

int main(void)
{
    
    systemConfig();

    sei();

    // Initial Duty Cycle Configuration
    d = D_MIN +10;

    for(;;){
        //if(systemFlags.error) break;
    }

    // Caso os limites sejam excedidos
    {
        timer2SetCompareBValue(0);                                  // duty cycle = 0 %
        timer2SetCompareAValue(0);                                  // clock = 0 Hz
        timer2Config(TIMER_A_MODE_NO_CHANGE, TIMER_B_CLOCK_DISABLE);    // clock disable
        printf("Sistema desligado por conta de valores de tensão na saída excedidos! Ultimos valores foram:\n");
        printf("v: %6u, i: %6u, p: %6u, d: %6u\n", (uint16) vi_med, (uint16) ii_med, (uint16) pi, d);
    }

	for(;;);

}

void systemConfig(void)
{
    // Variable Initialization
    systemFlags.allFlags = 0;

    // ADC Configuration
    adcConfig(ADC_MODE_SINGLE_CONVERSION, ADC_REFERENCE_POWER_SUPPLY, ADC_PRESCALER_128);
    adcSelectChannel(ADC_CHANNEL_0);    //
    adcClearInterruptRequest();         // clear adc interrupts
    adcActivateInterrupt();             // activate adc interrupts
    adcEnable();                        //
    adcStartConversion();               // first conversion takes 20 cycles

    // TIMER0 Configuration -> ADC
    // on clear timer on compare mode: f_timer = f_clk/(N*( ocr + 1))
    // ocra = f_clk/(N*f_timer) -1 = 16000000/(8*10000) -1 = 199
    timer0SetCompareAValue(199);            // setup adc frequency at 10KHz
    timer0SetCompareBValue(1);              // setup de phase angle
    timer0ActivateCompareBInterrupt();      // data aquisition
    timer0ActivateCompareAInterrupt();

    // TIMER2 Configuration -> PWM
    // fast pwm with top at ocra: f_timer = f_clk/(N*( ocra + 1)) and ocrb set the duty cycle
    // ocra = f_clk/(N*f_timer) -1 = 16000000/(1*100000) -1 = 159
    timer2SetCompareAValue(159);            // setup PWM frequency at 100KHz
    timer2SetCompareBValue(111);            // setup initial duty cycle to ~70%
    setBit(DDRD, PD3);                      // PWM output

// TIMERs Enable with delay
    timer2OutputConfig(TIMER_PORT_NON_INVERTING_MODE, TIMER_PORT_NON_INVERTING_MODE);   // --> PD3
    timer0OutputConfig(TIMER_PORT_NON_INVERTING_MODE, TIMER_PORT_NON_INVERTING_MODE);   // --> PD3
    timer2Config(TIMER_A_MODE_FAST_PWM_OCRA, TIMER_B_PRESCALER_OFF);
    timer0Config(TIMER_A_MODE_FAST_PWM_OCRA, TIMER_A_PRESCALER_8);

    // TIMER1 Configuration -> MPPT algorithm calculations
    // fcompA = fclk/(2*N*(OCRnA +1))
    // OCRnA = 2*N*fclk/fcompA -1
    // https://et06.dunweber.com/atmega_timers/
    timer1Config(TIMER_B_MODE_CTC, TIMER_A_PRESCALER_256);
    timer1SetCompareAValue(6250);           // setup interrupt to 10Hz
    timer1ClearCompareAInterruptRequest();
    timer1ActivateCompareAInterrupt();

    // UART Configuration
    usartConfig(USART_MODE_ASYNCHRONOUS, USART_BAUD_9600, USART_DATA_BITS_8, USART_PARITY_NONE, USART_STOP_BIT_SINGLE);
    usartEnableTransmitter();
    usartEnableReceiver();
    usartStdio();
     
}
