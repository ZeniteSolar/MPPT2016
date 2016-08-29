/**
 * @file main.h
 * @brief this is the main
 *
 * @author joaoantoniocardoso
 *
 * @date 8/26/2016
 */

#ifndef __HANDLERS_H
#define __HANDLERS_H

#define F_CPU 16000000UL

#include "lib/globalDefines.h"
#include "lib/ATmega328.h"
#include <avr/io.h>
#include "handlers.h"

// Configurations
#define N_MAX   1       //!< Numero de amostras para a média leitura de corrente e tensão
#define D_STEP  1       //!< (1/4096)
#define D_MAX   135     //!< Maximum Duty Cycle
#define D_MIN   70      //!< Minimum Duty Cycle //70
#define VO_MAX  47l     //!< Maximum Output Voltage allowed (battery's voltage)
#define PI_MAX  260l    //!< Maximum Input Power (in Watts) allowed

// AD Calibration
#define vi_offset 0//-183
#define vi_angle 1
#define ii_offset 0
#define ii_angle 1

// Exponential Moving Average
#define EMA_1(ma, a, n) ma += ((a) >> (n)) - ((ma) >> (n))  //1.5us for n=4
#define EMA_2(ma, a, n) ma += ((a) -(ma)) >> (n)  //1.5us for n=4
#define EMA_3(ma, a, n)   ma += ((a) -(ma)) / pow(2, (n)) //96us for n=4
#define EMA_GRADE 4     // 2^n for EMA filter
#define EMA EMA_2

typedef union systemFlags_t{
    struct{
        uint8 channel           :   1;
        uint8 mean              :   1;
        uint8 conversionReady   :   1;
        uint8 error             :   1;
    };
    uint8 allFlags;
} systemFlags_t;

// Retorna a média simples de 0 a n itens do vetor v
uint16 mean(uint16 *v, uint8 n);

// Configure the system
void systemConfig(void);

#endif /* ifndef __HANDLERS_H */
