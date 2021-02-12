/*----------------------------------------------------------------------------
 * Name:    sample.c
 * Purpose: to control led through EINT and joystick buttons
 * Note(s):
 *----------------------------------------------------------------------------
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2017 Politecnico di Torino. All rights reserved.
 *----------------------------------------------------------------------------*/
                  
#include <stdio.h>
#include "LPC17xx.H"                    /* LPC17xx definitions                */
#include "led/led.h"
#include "button_EXINT/button.h"
#include "timer/timer.h"
#include "RIT/RIT.h"
#include "joystick/joystick.h"
#include "adc/adc.h"
#include "dac/dac.h"
#include "GLCD/GLCD.h" 
#include "TouchPanel/TouchPanel.h"

/*----------------------------------------------------------------------------
  Main Program
 *----------------------------------------------------------------------------*/
int main (void) {
  	
	SystemInit();  												/* System Initialization (i.e., PLL)  */
  
	LCD_Initialization();
	TP_Init();
	LCD_Clear(Black);
	TouchPanel_Calibrate();
	draw_power();
	
	LED_init();                           /* LED Initialization                 */
  BUTTON_init();												/* BUTTON Initialization              */
	joystick_init();											/* Joystick Initialization            */
	init_RIT(0x004C4B40);									/* RIT Initialization 50 msec       	*/
	enable_RIT();													/* RIT enabled												*/
	init_timer(0, 0x005F5E10);  					/*timer 0 initialized on 250 msec     */
	init_timer(1, 0x002625A0);  					/*timer 1 initialized on 100 msec     */
	init_timer(3, 0x30D4 ); 						  /* 500us * 25MHz = 1.25*10^3 = 0x30D4 */ //da testare 50us
	ADC_init();														/* ADC Initialization									*/
	DAC_init();														/* DAC Initialization	  							*/
	
	LPC_SC->PCON |= 0x1;									/* power-down	mode										*/
	LPC_SC->PCON &= ~(0x2);				

	NVIC_DisableIRQ(EINT0_IRQn);		/* disable Button interrupts			 */
	LPC_PINCON->PINSEL4    &= ~(1 << 20);     /* GPIO pin selection */	
		
  while (1) {                           /* Loop forever                       */	
  		__ASM("wfi");
  }

}
