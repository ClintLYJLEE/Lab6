// LCD_Sprintf.mk: this LPC824 project shows how print to the LCD
// using sprintf().

#include <stdlib.h>
#include <stdio.h>
#include "lpc824.h"
#include "serial.h"
#include "lcd.h"

#define PIN_PERIOD (GPIO_B1) // Read period from PIO0_1 pin 12
#define SYSTEM_CLK 60000000L

// LPC824 pinout:
//                             --------
//     PIO0_23/ADC_3/ACMP_I4 -|1     20|- PIO0_14/ADC_2/ACMP_I3
//             PIO0_17/ADC_9 -|2     19|- PIO0_0/ACMP_I1/TDO
//            PIO0_13/ADC_10 -|3     18|- VREFP
//                   PIO0_12 -|4     17|- VREFN
//              RESET/PIO0_5 -|5     16|- VSS
// PIO0_4/ADC_11/WAKEUP/TRST -|6     15|- VDD
//          SWCLK/PIO0_3/TCK -|7     14|- PIO0_8/XTALIN
//          SWDIO/PIO0_2/TMS -|8     13|- PIO0_9/XTALOUT
//          PIO0_11/I2C0_SDA -|9     12|- PIO0_1/ACMP_I2/CLKIN/TDI
//          PIO0_10/I2C0_SCL -|10    11|- PIO0_15
//                             --------
// WARNING pins 9 and 10 are OPEN DRAIN.  They need external pull-up resistors to VDD if used
// as outputs. 1kohm seems to work.
//
// Reserved pins:
// Pin 4:  BOOT button
// Pin 5:  Reset
// Pin 6:  TXD
// Pin 19: RXD

void ConfigPins(void)
{
	// Disable SWCLK and SWDIO on pins 7 and 8. They iare enabled by default:
	SWM_PINENABLE0 |= BIT4; // Disable SWCLK
	SWM_PINENABLE0 |= BIT5; // Disable SWDIO

	// Configure the pins connected to the LCD as outputs
	GPIO_DIR0 |= BIT17; // Used for LCD_RS  Pin 2 of TSSOP20 package.
	GPIO_DIR0 |= BIT13; // Used for LCD_E.  Pin 3 of TSSOP20 package.
	GPIO_DIR0 |= BIT3;  // Used for LCD_D4. Pin 7 of TSSOP20 package.
	GPIO_DIR0 |= BIT2;  // Used for LCD_D5. Pin 8 of TSSOP20 package.
	GPIO_DIR0 |= BIT11; // Used for LCD_D6. Pin 9 of TSSOP20 package. WARNING: NEEDS PULL-UP Resistor to VDD.
	GPIO_DIR0 |= BIT10; // Used for LCD_D7. Pin 10 of TSSOP20 package. WARNING: NEEDS PULL-UP Resistor to VDD.
	
	GPIO_DIR0 &= ~(BIT1); // Configure PIO0_1 as input.
}

long int GetPeriod (int n)
{
	int i;
	unsigned int saved_TCNT1a, saved_TCNT1b;

	SYST_RVR = 0xffffff;  // 24-bit counter set to check for signal present
	SYST_CVR = 0xffffff; // load the SysTick counter
	SYST_CSR = 0x05; // Bit 0: ENABLE, BIT 1: TICKINT, BIT 2:CLKSOURCE
	while (PIN_PERIOD!=0) // Wait for square wave to be 0
	{
		if(SYST_CSR & BIT16) return 0;
	}
	SYST_CSR = 0x00; // Disable Systick counter

	SYST_RVR = 0xffffff;  // 24-bit counter set to check for signal present
	SYST_CVR = 0xffffff; // load the SysTick counter
	SYST_CSR = 0x05; // Bit 0: ENABLE, BIT 1: TICKINT, BIT 2:CLKSOURCE
	while (PIN_PERIOD==0) // Wait for square wave to be 1
	{
		if(SYST_CSR & BIT16) return 0;
	}
	SYST_CSR = 0x00; // Disable Systick counter
	
	SYST_RVR = 0xffffff;  // 24-bit counter reset
	SYST_CVR = 0xffffff; // load the SysTick counter to initial value
	SYST_CSR = 0x05; // Bit 0: ENABLE, BIT 1: TICKINT, BIT 2:CLKSOURCE
	for(i=0; i<n; i++) // Measure the time of 'n' periods
	{
		while (PIN_PERIOD!=0) // Wait for square wave to be 0
		{
			if(SYST_CSR & BIT16) return 0;
		}
		while (PIN_PERIOD==0) // Wait for square wave to be 1
		{
			if(SYST_CSR & BIT16) return 0;
		}
	}
	SYST_CSR = 0x00; // Disable Systick counter

	return 0x1000000-SYST_CVR;
}

void main(void)
{
	long int count;
	float T, f, c, no_reading;
	char buffer[100];
	
	initUART(115200);
	enable_interrupts();
	ConfigPins();
	LCD_4BIT(); 

	waitms(500); // Wait for putty to start.
	eputs("Period measurement using the Systick free running counter.\r\n"
	      "Connect signal to PIO0_1 (pin 12).\r\n");
	      
	LCDprint("Capacitor Meter", 1, 1);
	  
	while(1)
	{
		count=GetPeriod(100);
		if(count <= 94000 && count >= 88000)
		{
			LCDprint("C=0.00 nF", 2,1);	
		}
		else if(count>0)
		{
			T=count/(SYSTEM_CLK*100.0);
			f=1/T;
			c = ((1.44*T) / (672.0 + 2*668.0))*1000000000.0 - 1.83;
			
			printf("%d\r\n", count); 
			sprintf(buffer, "C=%.2f nF", c);
			LCDprint(buffer, 2, 1);
		}
		else
		{
			eputs("NO SIGNAL                     \r");
		}
		waitms(200);
	}
}

