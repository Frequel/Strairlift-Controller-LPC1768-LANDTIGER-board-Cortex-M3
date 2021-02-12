/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           IRQ_timer.c
** Last modified Date:  2014-09-25
** Last Version:        V1.00
** Descriptions:        functions to manage T0 and T1 interrupts
** Correlated files:    timer.h
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
#include "lpc17xx.h"
#include "timer.h"
#include "../led/led.h"

/******************************************************************************
** Function name:		Timer0_IRQHandler
**
** Descriptions:		Timer/Counter 0 interrupt handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/
int c0=0;
int c1=0;
extern volatile int busy;
extern volatile float pos;
extern volatile int select;

void TIMER0_IRQHandler (void)
{
	//da modificare e aggiungere alla condizione di c0==29 che sia a pos 0 o 8 (è sufficiente? DA VERIFICARE)
	if(c0==29 && (pos ==0 || pos==8)){
		c0=0;
		disable_timer(0);
		enable_timer(1);
		select=0;
		busy=1;
	} else {
		if(c0%2==0){
			LED_On(7);
		} else {
			LED_Off(7);
		}
		c0++;
	}
	
  LPC_TIM0->IR = 1;			/* clear interrupt flag */
  return;
}


/******************************************************************************
** Function name:		Timer1_IRQHandler
**
** Descriptions:		Timer/Counter 1 interrupt handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/
void TIMER1_IRQHandler (void)
{       
	if(c1==30){
		c1=0;
		disable_timer(1);
		select=1;
		/*enable_RIT(); //non più necessario */
	
	} else {
		if(c1%2==0){
			LED_On(7);
		} else {
			LED_Off(7);
		}
		c1++;
	}
	
  LPC_TIM1->IR = 1;			/* clear interrupt flag */
  return;
}

/******************************************************************************
**                            End Of File
******************************************************************************/
