#include "Serial.h"
#include "../GLCD/GLCD.h"
#include <LPC17xx.H>                              /* LPC17xx definitions    */

char buffer_out[10+1];	

void UART0_IRQHandler(){	
	static int i = 0, j = 0;
	static char buffer_in[10];
	volatile uint8_t sel_IIR = LPC_UART0->IIR;
	
	if((sel_IIR & 0xFE) == 0x04)	/* received data */
	{
			if ((buffer_in[i] = LPC_UART0->RBR)!=13){
					SER_putFirstChar(0,buffer_in[i]);		/* terminal echo */
					i++;
			}
			else{
					buffer_in[i] = '\0';
					i=0;									
					GUI_Text(100,300,(uint8_t*)buffer_in,Yellow,Blue);
					SER_putFirstChar(0,10);
					SER_putFirstChar(0,13);
			}
	}
	else if((sel_IIR & 0xFE) == 0x02){ /* transmitted data */
			if(buffer_out[j]!='\0'){ 
					j++;
					LPC_UART0->THR = buffer_out[j];
			}
			else{
					j=0;
					LPC_UART0->IER		&= ~(0x02);													 /* THRE interrupt enabled 						*/
			}				
	}
}
