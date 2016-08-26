/* -----------------------------------------------------------------------------
 * Project:			GPDSE AVR8 Integrated Library (Zenite Solar Mod Version)
 * File:			basicDefines.h
 * Module:			Global definitions file for the GAIL Project
 * Author:			Leandro Schwarz
 * Version:			12.3 (modified by Zenite Solar)
 * Last edition:	2016-01-15
 * ---------------------------------------------------------------------------*/

#ifndef __BASICDEFINES_H
#define __BASICDEFINES_H

// -----------------------------------------------------------------------------
// Basic definitions -----------------------------------------------------------

#ifndef F_CPU
	#define F_CPU 16000000UL
#endif

// -----------------------------------------------------------------------------
// Header files ----------------------------------------------------------------

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

// -----------------------------------------------------------------------------
// Bit handling macro functions ------------------------------------------------

#ifndef setBit
	#define setBit(reg, bit)					((reg) |= (1 << (bit)))
#endif
#ifndef clrBit
	#define clrBit(reg, bit)					((reg) &= ~(1 << (bit)))
#endif
#ifndef cplBit
	#define cplBit(reg, bit)					((reg) ^= (1 << (bit)))
#endif
#ifndef isBitSet
	#define isBitSet(reg, bit)					(((reg) >> (bit)) & 1)
#endif
#ifndef isBitClr
	#define isBitClr(reg, bit)					(!(((reg) >> (bit)) & 1))
#endif
#ifndef waitUntilBitIsSet
	#define waitUntilBitIsSet(reg, bit)			do{}while(isBitClr((reg), (bit)))
#endif
#ifndef waitUntilBitIsClear
	#define waitUntilBitIsClear(reg, bit)		do{}while(isBitSet((reg), (bit)))
#endif
#ifndef setMask
	#define setMask(reg, mask, offset)			((reg) |= ((mask) << (offset)))
#endif
#ifndef clrMask
	#define clrMask(reg, mask, offset)			((reg) &= ~((mask) << (offset)))
#endif
#ifndef cplMask
	#define cplMask(reg, mask, offset)			((reg) ^= ((mask) << (offset)))
#endif

// -----------------------------------------------------------------------------
// New data types --------------------------------------------------------------

typedef char				int8;
typedef int					int16;
typedef long int			int32;
typedef long long			int64;
typedef unsigned char		uint8;
typedef unsigned int		uint16;
typedef unsigned long int	uint32;
typedef unsigned long long	uint64;

#endif