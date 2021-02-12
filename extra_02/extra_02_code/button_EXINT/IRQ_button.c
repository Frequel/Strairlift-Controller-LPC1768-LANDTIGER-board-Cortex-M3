#include "button.h"
#include "lpc17xx.h"
#include "../led/led.h"
#include "../timer/timer.h"
#include "../RIT/RIT.h"

extern volatile int busy;
extern volatile float pos;
extern int down;
extern int em_mode;
volatile int up=0;
volatile int dw=0;
extern int t_en;
extern int c0;

void EINT0_IRQHandler (void)	  	/* INT0														 */
{		
	NVIC_DisableIRQ(EINT0_IRQn);		/* disable Button0 interrupts			 */
	LPC_PINCON->PINSEL4    &= ~(1 << 20);     /* GPIO pin selection */
	down=1;
	LPC_SC->EXTINT &= (1 << 0);     /* clear pending interrupt         */
}


void EINT1_IRQHandler (void)	  	/* KEY1														 */
{
	NVIC_DisableIRQ(EINT1_IRQn);		/* disable Button1 interrupts			 */
	LPC_PINCON->PINSEL4    &= ~(1 << 22);     /* GPIO pin selection */
	NVIC_DisableIRQ(EINT2_IRQn);		/* disable Button2 interrupts			 */
	LPC_PINCON->PINSEL4    &= ~(1 << 24);     /* GPIO pin selection */

	LED_On(0);
	LED_On(2);
	
	disable_timer(3);
	t_en=0;
	c0=0;
	
	if(em_mode){
		up=1;
		dw=0;
		disable_timer(2);
	}
	else if(pos==8){ //ascensore al piano 1
		busy=1;
	} else { //ascinsore al piano 0
		//aspetto 7,27 secondi e faccio blinkare nel frattempo (quindi contatore di 29)
		pos=8; 
		busy=1;
		up=1;
		dw=0;
		c0=0;
		enable_timer(0);
	}
	
	LPC_SC->EXTINT &= (1 << 1);     /* clear pending interrupt         */
}

void EINT2_IRQHandler (void)	  	/* KEY2														 */
{
	NVIC_DisableIRQ(EINT1_IRQn);		/* disable Button1 interrupts			 */
	LPC_PINCON->PINSEL4    &= ~(1 << 22);     /* GPIO pin selection */
	NVIC_DisableIRQ(EINT2_IRQn);		/* disable Button2 interrupts			 */
	LPC_PINCON->PINSEL4    &= ~(1 << 24);     /* GPIO pin selection */

	LED_On(0);
	LED_On(2);	
	
	disable_timer(3);
	t_en=0;
	c0=0;
	
	if(em_mode){
		up=0;
		dw=1;
		//disable_timer(0);
		//init_timer(0,0x005F5E10);
		disable_timer(2);
	}
	else if(pos==0){ //ascensore al piano 0
		busy=1;
	}
	else { //ascinsore al piano 1
		//aspetto 7,27 secondi  (tempo che ci mette l'ascensore a salire) e faccio blinkare nel frattempo (quindi contatore di 29)
		busy=1;
		up=0;
		dw=1;
		c0=0;
		enable_timer(0); 
		pos=0;
	}
	
	
  LPC_SC->EXTINT &= (1 << 2);     /* clear pending interrupt         */    
}

