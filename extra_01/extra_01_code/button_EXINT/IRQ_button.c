#include "button.h"
#include "lpc17xx.h"
#include "../led/led.h"
#include "../timer/timer.h"

extern volatile int busy;
extern volatile float pos;
//non è stato ritenuto necessario implementare il debouncing in quanto in caso di bouncing la richiesta sarebbe la medesima e comunque viene subito diabilitato l'interrupt appena viene gestito il primo click
void EINT0_IRQHandler (void)	  	/* INT0														 */
{		
	
	LPC_SC->EXTINT &= (1 << 0);     /* clear pending interrupt         */
}


void EINT1_IRQHandler (void)	  	/* KEY1														 */
{
	NVIC_DisableIRQ(EINT1_IRQn);		/* disable Button1 interrupts			 */
	LPC_PINCON->PINSEL4    &= ~(1 << 22);     /* GPIO pin selection */
	NVIC_DisableIRQ(EINT2_IRQn);		/* disable Button2 interrupts			 */
	LPC_PINCON->PINSEL4    &= ~(1 << 24);     /* GPIO pin selection */
	
	//busy=1;
	LED_On(0);
	LED_On(2);
	
	if(pos==0){ //ascensore al piano 0
		busy=1;
	} else { //ascinsore al piano 1
		//aspetto 7,27 secondi  (tempo che ci mette l'ascensore a salire) e faccio blinkare nel frattempo (quindi contatore di 29)
		enable_timer(0); //fin quando il timer non entra nel caso che si disablita da solo e setta busy=1 (vedi timer interrupt handler) il busy sarà a 0 e non sarà possibile fare nulla
		//busy=1; //lo faccio nell'interrupt del timer
		pos=0; //controllare se và bene qua, perchè in questo caso mi serve per come ho impostato l'if netl timer0 interrupt handler  
	}
	
	
	
	LPC_SC->EXTINT &= (1 << 1);     /* clear pending interrupt         */
}

void EINT2_IRQHandler (void)	  	/* KEY2														 */
{
	NVIC_DisableIRQ(EINT1_IRQn);		/* disable Button1 interrupts			 */
	LPC_PINCON->PINSEL4    &= ~(1 << 22);     /* GPIO pin selection */
	NVIC_DisableIRQ(EINT2_IRQn);		/* disable Button2 interrupts			 */
	LPC_PINCON->PINSEL4    &= ~(1 << 24);     /* GPIO pin selection */
	
	//busy=1;
	LED_On(0);
	LED_On(2);
	
	if(pos==8){ //ascensore al piano 1
		busy=1;
	} else { //ascinsore al piano 0
		//aspetto 7,27 secondi e faccio blinkare nel frattempo (quindi contatore di 29)
		pos=8;
		enable_timer(0);
		//busy=1; //lo faccio nell'interrupt del timer
	}
	
	
  LPC_SC->EXTINT &= (1 << 2);     /* clear pending interrupt         */    
}


