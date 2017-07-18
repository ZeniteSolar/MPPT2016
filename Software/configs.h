
#ifndef _CONFIGS_H
#define _CONFIGS_H

#define VERBOSE				1					//! sends system status via serial

#define D_STEP				((uint8)	1)		//! Duty Cycle Step
#define D_MAX				((uint8)	135)	//!1000 Maximum Duty Cycle = 135/159 = 84.9[%]
#define D_INITIAL			((uint8)	100)	//! Initial Duty Cycle = 100/159 = 62[%]
#define D_MIN				((uint8)	64)		//! Minimum Duty Cycle = 64/159 = 40.0[%]

#define VI_MAX				((uint16)	65000)	//!
#define VI_MIN				((uint16)	20)		//!
#define vi_offset 			((uint8)	0)		//!
#define vi_angle 			((uint8)	1)		//!
#define vi_ema_grade 		((uint8)	4)		//!

#define II_MAX				((uint16)	65000)	//!
#define II_MIN				((uint16)	20)		//!
#define ii_offset 			((uint8)	0)		//!
#define ii_angle 			((uint8)	1)		//!
#define ii_ema_grade 		((uint8)	4)		//!

#define VO_MAX				((uint16)	800)	//! Maximum Output Voltage allowed
#define VO_MIN  			((uint16)	20)		//! Minimum Output Voltage allowed
#define vo_offset 			((uint8)	0)		//!
#define vo_angle 			((uint8)	1)		//!
#define vo_ema_grade 		((uint8) 	4)		//!

#define PI_MAX  			((uint64)	65000)	//! Maximum Input Power allowed
#define PI_MIN				((uint64)	2000)	//! Minimum acceptable input power
#define pi_offset 			((uint8)	0)		//!
#define pi_angle 			((uint8)	1)		//!
#define pi_ema_grade 		((uint8) 	2)		//!

#define mppt_freq_divider 	((uint16)	500)	//!	10 Hz
#define safety_freq_divider ((uint16) 	50000)	//! 0.1 Hz

#define low_pi_counter_limit ((uint16)	10)		//! 0.01Hz = 100 seconds

#define PWM_DDR 			(DDRD)				//! only used to set pwm pin as output
#define PWM_BIT 			(PD3)				//! only used to set pwm pin as output

typedef enum{	STATUS_TURNING_ON = 0,
				STATUS_ON,
				STATUS_RUNNING,
				STATUS_TURNING_OFF,
				STATUS_OFF,
			} status_t;

typedef enum{	ERROR_NONE = 0,
				ERROR_NO_BATTERY,
				ERROR_BATTERY_OVERVOLTAGE,
				ERROR_BATTERY_UNDERVOLTAGE,
				ERROR_NO_PANEL,
				ERROR_PANEL_OVERVOLTAGE,
			} error_t;

typedef union systemFlags_t{
	struct{
		uint8 channel			:	2;
		uint8 conversionReady	:	1;
	};
	uint8 allFlags;
} systemFlags_t;

#endif /* _CONFIGS_H */
