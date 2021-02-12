/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           IRQ_adc.c
** Last modified Date:  20184-12-30
** Last Version:        V1.00
** Descriptions:        functions to manage A/D interrupts
** Correlated files:    adc.h
**--------------------------------------------------------------------------------------------------------       
*********************************************************************************************************/

#include "lpc17xx.h"
#include "adc.h"
#include "../led/led.h"
#include "../timer/timer.h"
#include <string.h>
#include "../GLCD/GLCD.h" 
#include "../TouchPanel/TouchPanel.h"
/*----------------------------------------------------------------------------
  A/D IRQ: Executed when A/D Conversion is ready (signal from ADC peripheral)
 *----------------------------------------------------------------------------*/

unsigned short AD_current;   
unsigned short AD_last = 0xFF;     /* Last converted value               */

const int freqs[15]={1062,1125,1263,1417,1592,1684,1890,2120,2249,2525,2834,3175,3367,3779,4240};
//vettore di stringhe da 15 elementi dove scrivo i corrispondendti "xxxHz - nome delle note"
char note[15][20]={"523 Hz - C5","494 Hz - B4","440 Hz - A4","392 Hz - G4","349 Hz - F4","330 Hz - E4","294 Hz - D4","262 Hz - C4","247 Hz - B3","220 Hz - A3","196 Hz - G3","175 Hz - F3","165 Hz - E3","147 Hz - D3","131 Hz - C3"};
int tono[2]={1263,1263};
extern int t; //variabile t che in base a se ho selezionato primo o secondo tono sullo schermo vale 0 o 1
//extern char message[2][15];
extern int tn[];
char msg[2][20];
extern int firstTouch[];
int last=15;
int current;

void updateTone(void){
	static int confirm=0;
	static int x0=0;
	static int x1=0;

		current= AD_current*14/0xFFF;
		if(current!=last){
			last=current;
			confirm=0;
		}
		else 
		{
			confirm++;
			
			if(confirm==3){ //per sfarfallio, 3 è veloce a cambiare durante la rotazione, più alto sarebbe più affidabile
				tn[t]=freqs[AD_current*14/0xFFF]; //potrei usare [current]
				strcpy(msg[t],note[AD_current*14/0xFFF]); //potrei usare [current]
					
				if(!t){//nota1 sta cambiando
					
					if(x0>strlen(msg[0])){
						LCD_ClearSection(60,60,240,100,Blue);
						x0=strlen(msg[0]);
					}
					else if(x0<strlen(msg[0])){
						x0=strlen(msg[0]);
					}
				
					GUI_Text(60, 60, (uint8_t *) " Select Note 1 ", Blue2, White);
					GUI_Text(60, 80, (uint8_t *) msg[0], Blue2, White);
					LED_Out(0);
				}
				else if (t==1){//nota2 sta cambiando
					
					if(x1>strlen(msg[1])){
						LCD_ClearSection(60,140,240,280,Blue);
						x1=strlen(msg[1]);
					}
					else if(x1<strlen(msg[1])){
						x1=strlen(msg[1]);
					}
					
					GUI_Text(60, 140, (uint8_t *) " Select Note 2 ", Blue2, White);
					GUI_Text(60, 160, (uint8_t *) msg[1], Blue2, White);
					LED_Out(0);
					
				}
			}
		}
}

void ADC_IRQHandler(void) {
	
	static int first=0;
	
  AD_current = ((LPC_ADC->ADGDR>>4) & 0xFFF); /* Read Conversion Result             */



 //senza sfarfallio e firstTouch
	if(firstTouch[t]){ //sistema anti cambiamento nota prima che si muove il potenziometro (appena tocchi la nota)
			current= AD_current*14/0xFFF;
			first=current;
			firstTouch[t]=0;
		
	} else {
		current= AD_current*14/0xFFF;
		if(current!=first){
			updateTone();
			first=15;
		}
	}

}
