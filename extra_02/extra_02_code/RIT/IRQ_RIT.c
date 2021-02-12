/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           IRQ_RIT.c
** Last modified Date:  2014-09-25
** Last Version:        V1.00
** Descriptions:        functions to manage T0 and T1 interrupts
** Correlated files:    RIT.h
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
#include "lpc17xx.h"
#include "RIT.h"
#include "../led/led.h"
#include "../timer/timer.h"
#include "../GLCD/GLCD.h" 
#include "../TouchPanel/TouchPanel.h"

/******************************************************************************
** Function name:		RIT_IRQHandler
**
** Descriptions:		REPETITIVE INTERRUPT TIMER handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/

extern int c0;
volatile int busy=0;
volatile float pos=0;
volatile int mov_en=0;
volatile int curr_state=0; //gli stati sono 0 per fermo 1 per in movimento 
volatile int counter=0; //usare il RIT per contare un minuto, quindi 60secondi cioè un counter di 1200 
volatile int select =1; //variabile usata per disabilitare i tasto select durante i 3 secondi del blinking a 5Hz per poi riabilitarlo appena dopo

volatile int em_mode = 0;
volatile int maintenance = 0;
int down=0;
extern int up;
extern int dw;
int t_en=0;

//direction =0 piano zero =1 piano1
 void move_elevator(int direction){
	counter=0;
	if (curr_state==0){
		LED_Off(1); //ho optato per la soluzione in cui se sono in stato di alarm e poi ripremo up/down, il funzionamento riprende correttamente, per cui devo assicurarmi di spegnere i led di alarm
		LED_Off(3);
		disable_timer(0);
		reset_timer(0);
		init_timer(0,0x005F5E10);
		c0=0;//aggiunto a casa da testare se lascia funzionare tutto correttamente
		enable_timer(0);  //faccio partire il timer per blikare a 2Hz
	}
	curr_state=1;
	if(direction)
		pos+=0.0555;
	else
		pos-=0.0555;
	if((pos>=8 && direction)||(pos<=0 && !direction)){
		disable_timer(0);//fermo il timer per i 2Hz
		c0=0;
		LED_Off(7); //spengo il led prima di far partire il blink a 5Hz //inutile se uno và sempre dritto senza fare avanti e indietro
		reset_timer(1);
		enable_timer(1);//faccio blinkare a 5Hz per 3 secondi
		select=0; //disabilito il select per i 3 secondi di blinking a 5Hz
		mov_en = 0;
		if(direction)
			pos=8;
		else
			pos=0;
		curr_state=0;
		up=0;
		dw=0;
		maintenance = 0;
		NVIC_DisableIRQ(EINT0_IRQn);		/* disable Button interrupts			 */
		LPC_PINCON->PINSEL4    &= ~(1 << 20);     /* GPIO pin selection */	
	}											
 }

 void startEmMode(){
	em_mode=1;
	init_timer(0,0x002FAF08);
	reset_timer(0);
	c0=0;//aggiunto a casa da testare se lascia funzionare tutto correttamente
	enable_timer(0);
	LED_On(1);
	LED_On(3);
	counter=0;
	disable_timer(3);
	t_en=0;
 }
void RIT_IRQHandler (void)
{	
	if(!maintenance){	
		/* button management */
		if(down!=0){ 
			if((LPC_GPIO2->FIOPIN & (1<<10)) == 0){	/* KEY0 pressed */
				down++;				
				
				if(down==2 && em_mode){
					em_mode=0;
					disable_timer(0);
					init_timer(0, 0x005F5E10);
					disable_timer(2);
					LED_Off(1);
					LED_Off(3);
					NVIC_DisableIRQ(EINT1_IRQn);		/* disable Button1 interrupts			 */
					LPC_PINCON->PINSEL4    &= ~(1 << 22);     /* GPIO pin selection */
					NVIC_DisableIRQ(EINT2_IRQn);		/* disable Button2 interrupts			 */
					LPC_PINCON->PINSEL4    &= ~(1 << 24);     /* GPIO pin selection */
				}
				else if(down==41 && !em_mode){
					startEmMode();	
				}
			}
			else {	/* button released */
				down=0;		
				NVIC_EnableIRQ(EINT0_IRQn);              /* enable irq in nvic                 */
				LPC_PINCON->PINSEL4    |= (1 << 20);     /* External interrupt 0 pin selection */			
			}
		}
		
		if(!em_mode){
			if(busy){	
					if((LPC_GPIO1->FIOPIN & (1<<25)) == 0 && !mov_en && select){	
						/* Joytick Select pressed */
						mov_en++;
						counter=0; //resetto il contatore perchè ho premuto select e quindi non sono fermo dopo aver premuto il bottone, ma sono in procinto di muovermi, quindi se non mi muovo, finirò in uno stato di alarm in caso di counter==12000
						NVIC_EnableIRQ(EINT0_IRQn);              /* enable irq in nvic                 */
						LPC_PINCON->PINSEL4    |= (1 << 20);     /* External interrupt 0 pin selection */
					}
					else if(mov_en){
						if((LPC_GPIO1->FIOPIN & (1<<29)) == 0 ){ //&& pos!=8){	 //perchè questo && ? perchè se volessi impedire di andare in sù se sono già al piano1
							/* Joytick up pressed */
							move_elevator(1);
						}			
						else if((LPC_GPIO1->FIOPIN & (1<<26)) == 0){
							/* Joytick down pressed */
							move_elevator(0);
						} else {
							// caso in cui il movimento è abilitato, quindi select è stato premuto ma non ci muoviamo 
							//(o ci siamo mossi e il movimento è ancora abilitato ma non ci muoviamo più per qualche
							// motivo)
							curr_state=0;
							disable_timer(0); //disabilito timer dei 2Hz blinking
							LED_On(7);
							counter++;
							if(counter==1200){
								//siamo nello stato di alarm
								startEmMode();
							}
						}
				} else if(select && !mov_en){
					//caso in  cui non si preme select una volta abilitato (e/o arrivato)
					counter++;
					if(counter==1200){
						//siamo nello stao in cui l'ascensore è fermo ad uno dei due piani, ma non si preme select per un minuto
						busy=0;
						LED_Out(0);
						NVIC_EnableIRQ(EINT1_IRQn);							 /* enable Button interrupts			*/
						LPC_PINCON->PINSEL4    |= (1 << 22);     /* External interrupt 0 pin selection */
						NVIC_EnableIRQ(EINT2_IRQn);							 /* enable Button interrupts			*/
						LPC_PINCON->PINSEL4    |= (1 << 24);     /* External interrupt 0 pin selection */
						counter=0;
						t_en=0;
					}
				}
			}
			else {
				//devo avviarlo una sola volta il timer e disabilitarlo quando lo finisco, 
				//resettando anche un' eventuale flag
				if(!t_en){
					reset_timer(3);
					enable_timer(3);
					t_en=1;
				}
			}
		}	else {
			/*riabilita bottoni 1 e 2 e nel caso vengono premuti, in base a chi è stato premuto,
			muovere pos fino a raggiungimento del piano*/
			NVIC_EnableIRQ(EINT1_IRQn);							 /* enable Button interrupts			*/
			LPC_PINCON->PINSEL4    |= (1 << 22);     /* External interrupt 0 pin selection */
			NVIC_EnableIRQ(EINT2_IRQn);							 /* enable Button interrupts			*/
			LPC_PINCON->PINSEL4    |= (1 << 24);     /* External interrupt 0 pin selection */
			
			if(up){
				if(curr_state==0){
					NVIC_DisableIRQ(EINT0_IRQn);		
					LPC_PINCON->PINSEL4    &= ~(1 << 20);						
				}
				
				move_elevator(1);
				
				if(pos==8){
					em_mode = 0;
				}	
			} else if(dw) {
				if(curr_state==0){
					NVIC_DisableIRQ(EINT0_IRQn);		
					LPC_PINCON->PINSEL4    &= ~(1 << 20);						
				}
				
				move_elevator(0);
				
				if(!pos){
					em_mode = 0;
				}	 
			}
		}
	} else {
		ADC_start_conversion(); 
	}
	
  LPC_RIT->RICTRL |= 0x1;	/* clear interrupt flag */
	
  return;
 }

/******************************************************************************
**                            End Of File
******************************************************************************/
