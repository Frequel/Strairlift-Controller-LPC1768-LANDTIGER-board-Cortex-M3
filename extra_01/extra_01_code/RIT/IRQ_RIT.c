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
volatile int arrived=0;
volatile int curr_state=0; //gli stati sono 0 per fermo 1 per in movimento up 2 per in movimento down (se uguale a zero e arrived=1 allora è arrivato)
//sarebbe bello usare il RIT per contare un minuto, quindi 60secondi cioè un counter di 1200 (se arriva a 1200 è passato un minuto senza fare nulla, quindi o sblocchi o và in allarme, se invece muovi up o down và a zero)
volatile int counter=0;
volatile int select =1; //variabile usata per disabilitare i tasto select durante i 3 secondi del blinking a 5Hz per poi riabilitarlo appena dopo


void RIT_IRQHandler (void)
{						
	if(busy){	
			if((LPC_GPIO1->FIOPIN & (1<<25)) == 0 && !mov_en && select){	
				/* Joytick Select pressed */
				mov_en++;
				counter=0; //resetto il contatore perchè ho premuto select e quindi non sono fermo dopo aver premuto il bottone, ma sono in procinto di muovermi, quindi se non mi muovo, finirò in uno stato di alarm in caso di counter==12000
			}
			else if(mov_en){
				if((LPC_GPIO1->FIOPIN & (1<<29)) == 0 ){ //&& pos!=8){	 //perchè questo && ? perchè se volessi impedire di andare in sù se sono già al piano1
					/* Joytick up pressed */
					counter=0;
					if(curr_state==2){ //lo faccio perchè non sò se il polling è stao preciso nel captare lo stato del controller, quindi aggiungo un'ultima volta lo spazio percorso dallo stato precedente
						pos-=0.0555;
						/*siccome appunto potrei essere arrivato con l'ultimo push down del joystick e con il polling non lo avevo notato in tempo, 
						dovrei verificare se non fossi arrivato a 0 in quel caso e quindi essere arrivato ma non è richiesto/necessario, quindi ho evitato di complicare il codice*/
					} else if (curr_state==0){
						//faccio partire il timer per blikare a 2Hz
						LED_Off(1); //ho optato per la soluzione in cui se sono in stato di alarm e poi ripremo up/down, il funzionamento riprende correttamente, per cui devo assicurarmi di spegnere i led di alarm
						LED_Off(3);
						enable_timer(0);  //faccio partire il timer per blikare a 2Hz
					}
					curr_state=1;
					pos+=0.0555;
					if(pos>=8){
						//fermo il timer per i 2Hz
						disable_timer(0);
						c0=0;
						LED_Off(7); //spengo il led prima di far partire il blink a 5Hz //inutile se uno và sempre dritto senza fare avanti e indietro
						//faccio blinkare a 5Hz per 3 secondi
						enable_timer(1);
						select=0; //disabilito il select per i 3 secondi di blinking a 5Hz
						//disabilito il RIT perchè troppo più veloce del 5Hz e quindi mi và a controllare subito se non ho premuto select (1 minuto di attesa)? cosiì assicuro che per 3 secondi select non possa essere ripremuto 
						//disable_RIT(); //in pratica non necessario perchè ho utilizzato un ulteriore flag e la sua combinazione con gli altri rende impossibile entrare in uno degli if che modificano qualcosa di importante perchè in teoria bottoni sono disabilitati
						arrived=1;
						mov_en = 0;
						pos=8;
						curr_state=0;
					}	
					
				}			
				else if((LPC_GPIO1->FIOPIN & (1<<26)) == 0){	 //&& pos!=0){	 //perchè questo && ? perchè se volessi impedire di andare in giù se sono già al piano0
					/* Joytick down pressed */
					counter=0;
					if(curr_state==1){ //lo faccio perchè non sò se il polling è stao preciso nel captare lo stato del controller, quindi aggiungo un'ultima volta lo spazio percorso dallo stato precedente
						pos+=0.0555;
						//siccome appunto potrei essere arrivato con l'ultimo push down del joystick e con il polling non lo avevo notato in tempo, dovrei verificare se non fossi arrivato a 0 in quel caso e quindi essere arrivato						
					}  else if (curr_state==0){
						//faccio partire il timer per blikare a 2Hz
						LED_Off(1);
						LED_Off(3);
						enable_timer(0);
					}
					curr_state=2;
					pos-=0.0555;
					if(pos<=0){
						//fermo il timer per i 2Hz
						disable_timer(0);
						c0=0;
						LED_Off(7); //spengo il led prima di far partire il blink a 5Hz //inutile se uno và sempre dritto senza fare avanti e indietro
						//faccio blinkare a 5Hz per 3 secondi
						enable_timer(1);
						select=0; //disabilito il select per i 3 secondi di blinking a 5Hz
						//disabilito il RIT perchè troppo più veloce del 5Hz e quindi mi và a controllare subito se non ho premuto select (1 minuto di attesa)? cosicchè assicuro che per 3 secondi select non possa essere ripremuto ?
						//disable_RIT(); //in pratica non necessario perchè comunque i flag rendono impossibile entrare in uno degli if che modificano qualcosa di importante perchè in teoria bottoni sono disabilitati e  busy settato a 0 (salvo errori di algoritmo che non ho riscontrato)
						arrived=1;
						mov_en = 0;
						pos=0;
						curr_state=0;
					}	
				} else {
					//caso in cui il movimento è abilitato, quindi select è stato premuto ma non ci muoviamo (o ci siamo mossi e il movimento è ancora abilitato ma non ci muoviamo più per qualche motivo)
					curr_state=0;
					disable_timer(0); //disabilito timer dei 2Hz blinking
					LED_On(7);
					counter++;
					if(counter==1200){
						//siamo nello stato di alarm
						LED_On(1);
						LED_On(3);
						/*con il seguente codice commentato resetto tutto tutte le variabili in quanto non è specificato cosa si debba fare una volta entrato in stato di alarm, in questa maniera si blocca tutto e non si può fare più niente. 
						ho optato per un'altra soluzione in cui lo stato di alarm segnala solo inattività dopo 1 minuto piuttosto che blocare tutto */
						/* counter=0;
						mov_en=0;
						select=0; */
					}
				}
		} else if(select && !mov_en){
			//caso in  cui non si preme select una volta abilitato (e/o arrivato?)
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
					}
		}
	}
	
  LPC_RIT->RICTRL |= 0x1;	/* clear interrupt flag */
	
  return;
 }

/******************************************************************************
**                            End Of File
******************************************************************************/
