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
#include "../adc/adc.h"
#include "../dac/dac.h"
#include "../GLCD/GLCD.h" 
#include "../TouchPanel/TouchPanel.h"

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
extern int tono[];
int acc=1;
char message[2][15] = {"440 Hz - A", "440 Hz - A"};
int t=0;//-1;
extern volatile int maintenance;
extern int up;
extern int dw;
extern int t_en;
int tn[2];
extern int counter;
extern char msg[2][20];
int firstTouch[2]={0,0};
//extern int mov_en;

uint16_t SinTable[45] =                                       
{
    410, 467, 523, 576, 627, 673, 714, 749, 778,
    799, 813, 819, 817, 807, 789, 764, 732, 694, 
    650, 602, 550, 495, 438, 381, 324, 270, 217,
    169, 125, 87 , 55 , 30 , 12 , 2  , 0  , 6  ,   
    20 , 41 , 70 , 105, 146, 193, 243, 297, 353
};

void TIMER0_IRQHandler (void)
{       
	int f = LPC_TIM0 -> MR0;
	
	if(c0==29 && (pos == 0 || pos == 8) && f==0x005F5E10 && (up || dw)){
		c0=0;
		disable_timer(0);
		enable_timer(1);
		select=0;
		counter=0;
		up=0;
		dw=0;
		//busy=1;
	} else {
		if(c0%2==0){
			LED_On(7);
			if(f==0x002FAF08){
				init_timer(2,tono[0]);
				reset_timer(2);
				enable_timer(2);
			}
		} else {
			LED_Off(7);
			if(f==0x002FAF08){
				init_timer(2,tono[1]);
				reset_timer(2);
				enable_timer(2);
			}
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

void TIMER2_IRQHandler (void)
{       
	static int ticks=0;
	/* DAC management */	
	int value = SinTable[ticks] * 0.375; //ho riproporzionatoe su 1023 e moltiplicato per 0.3
	DAC_convert (value<<6);
	ticks++;
	if(ticks==45) ticks=0;
	
  LPC_TIM2->IR = 1;			/* clear interrupt flag */
  return;
}

void TIMER3_IRQHandler (void)
{      
  getDisplayPoint(&display, Read_Ads7846(), &matrix ) ; 
	if(display.x <= 220 && display.x > 0){ 
			if((display.x <= 215 && display.x > 185) && (display.y <= 75 && display.y > 45) && acc){ 
			
			NVIC_DisableIRQ(EINT0_IRQn);		/* disable Button0 interrupts			 */
			LPC_PINCON->PINSEL4    &= ~(1 << 20);     /* GPIO pin selection */
			NVIC_DisableIRQ(EINT1_IRQn);		/* disable Button1 interrupts			 */
			LPC_PINCON->PINSEL4    &= ~(1 << 22);     /* GPIO pin selection */
			NVIC_DisableIRQ(EINT2_IRQn);		/* disable Button2 interrupts			 */
			LPC_PINCON->PINSEL4    &= ~(1 << 24);     /* GPIO pin selection */
			
			LCD_Clear(Blue);
			GUI_Text(70, 10, (uint8_t *) " Maintenance ", Yellow, Blue);
			
			GUI_Text(60, 60, (uint8_t *) " Select Note 1 ", White, Blue);
			GUI_Text(60, 80, (uint8_t *) message[0], White, Blue);
			
			GUI_Text(60, 140, (uint8_t *) " Select Note 2 ", White, Blue);
			GUI_Text(60, 160, (uint8_t *) message[1], White, Blue);
			
			GUI_Text(20, 280, (uint8_t *) " SAVE ", White, Green);
			GUI_Text(180, 280, (uint8_t *) " QUIT ", White, Red);
				
			LED_Out(0);
				
			acc=0; 
			tn[0]=tono[0];
			tn[1]= tono[1];
			strcpy(msg[0],message[0]);
			strcpy(msg[1],message[1]);
			
		} 
		else if((display.y <= 120 && display.y > 15) && !acc){
			GUI_Text(60, 60, (uint8_t *) " Select Note 1 ", Blue2, White);
			GUI_Text(60, 80, (uint8_t *) msg[0], Blue2, White);
			
			GUI_Text(60, 140, (uint8_t *) " Select Note 2 ", White, Blue);
			GUI_Text(60, 160, (uint8_t *) msg[1], White,Blue);
			
			GUI_Text(20, 280, (uint8_t *) " SAVE ", White, Green);
			
			LED_Out(0);
			
			t=0;
			maintenance=1;
			firstTouch[0]=1;
			firstTouch[1]=0;
		}
		else if((display.y <= 200 && display.y > 135) && !acc){
			GUI_Text(60, 60, (uint8_t *) " Select Note 1 ", White, Blue);
			GUI_Text(60, 80, (uint8_t *) msg[0], White, Blue);
			
			GUI_Text(60, 140, (uint8_t *) " Select Note 2 ", Blue2, White);
			GUI_Text(60, 160, (uint8_t *) msg[1], Blue2, White);
			
			GUI_Text(20, 280, (uint8_t *) " SAVE ", White, Green);
			
			LED_Out(0);
			
			t=1;
			//conversion=1;
			maintenance=1;
			firstTouch[0]=0;
			firstTouch[1]=1;
		}
		else if((display.x <= 60 && display.x > 10) && (display.y <= 300 && display.y > 270) && !acc){
			//save
			
			t=2;
			GUI_Text(70, 10, (uint8_t *) " Maintenance ", Yellow, Blue);
			
			GUI_Text(60, 60, (uint8_t *) " Select Note 1 ", White, Blue);
			GUI_Text(60, 80, (uint8_t *) message[0], White, Blue);
			
			GUI_Text(60, 140, (uint8_t *) " Select Note 2 ", White, Blue);
			GUI_Text(60, 160, (uint8_t *) message[1], White, Blue);
			
			GUI_Text(20, 280, (uint8_t *) " SAVE ", Green, White);
			
			LED_Out(0);
			
			tono[0]=tn[0];
			tono[1]=tn[1];
			strcpy(message[0],msg[0]);
			strcpy(message[1],msg[1]);
			
		}
		else if((display.x <= 220 && display.x > 170) && (display.y <= 300 && display.y > 270) && !acc){
		//quit
			acc=1; 
			maintenance=0;
			LCD_Clear(Black);
			disable_timer(3);
			t_en=0;
			draw_power();
			LPC_GPIO2->FIOCLR    = 0x000000FF;  //all LEDs off
			
			NVIC_EnableIRQ(EINT1_IRQn);							 /* enable Button interrupts			*/
			LPC_PINCON->PINSEL4    |= (1 << 22);     /* External interrupt 0 pin selection */
			NVIC_EnableIRQ(EINT2_IRQn);							 /* enable Button interrupts			*/
			LPC_PINCON->PINSEL4    |= (1 << 24);     /* External interrupt 0 pin selection */
		}
	}
	else{
		//do nothing if touch returns values out of bounds
	}	
	
	
	
  LPC_TIM3->IR = 1;			/* clear interrupt flag */
  return;
}



/******************************************************************************
**                            End Of File
******************************************************************************/
