/*
 * ema.h
 *
 *  Created on: Nov 16, 2016
 *      Author: joaoantoniocardoso
 */

#ifndef EMA_H_
#define EMA_H_

// Exponential Moving Average
#define EMA_1(ma, a, n)	ma += ((a) >> (n)) - ((ma) >> (n))  //1.5us for n=4
#define EMA_2(ma, a, n)	ma += ((a) -(ma)) >> (n)  //1.5us for n=4
#define EMA_3(ma, a, n)	ma += ((a) -(ma)) / pow(2, (n))	//96us for n=4
#define EMA_GRADE 4		// 2^n for EMA filter
#define EMA EMA_2

#endif /* EMA_H_ */
