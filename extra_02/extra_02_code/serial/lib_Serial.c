/*----------------------------------------------------------------------------
 * Name:    Serial.c
 * Purpose: MCB1700 Low level serial functions
 * Note(s):
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2008 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------*/

#include <LPC17xx.H>                              /* LPC17xx definitions    */
#include "Serial.h"

/*----------------------------------------------------------------------------
  Initialize UART pins, Baudrate
 *----------------------------------------------------------------------------*/
void SER_init (int uart) {
  LPC_UART_TypeDef *pUart;
   
  if (uart == 0) {                               /* UART0 */
    LPC_PINCON->PINSEL0 |= (1 << 4);             /* Pin P0.2 used as TXD0 (Com0) */
    LPC_PINCON->PINSEL0 |= (1 << 6);             /* Pin P0.3 used as RXD0 (Com0) */

    pUart = (LPC_UART_TypeDef *)LPC_UART0;
  } else {                                       /* UART2 */
    LPC_PINCON->PINSEL0 |= (2 << 8);             /* Pin P0.10 used as TXD1 (Com2) */
    LPC_PINCON->PINSEL0 |= (2 << 10);            /* Pin P0.11 used as RXD1 (Com2) */

    pUart = (LPC_UART_TypeDef *)LPC_UART2;
  }

  pUart->LCR    = 0x83;                          /* 8 bits, no Parity, 1 Stop bit    */  
	/* using Divisor Latch only - larger difference of the baud, but anyway working    */
	pUart->DLL    = 13;                              /* 115200 Baud Rate @ 25.0 MHZ 	 */
  pUart->DLM    = 0;                               /* High divisor latch = 0         */
	/* using Fractional Divider Register - preferred because more precise BPS */
//	pUart->DLL    = 9;                             /* 115200 Baud Rate @ 25.0 MHZ 	 */
//  pUart->DLM    = 0;                             /* High divisor latch = 0         */
//  pUart->FDR    = 0x21;                          /* FR 1,507, DIVADDVAL = 1, MULVAL = 2 */
	
	pUart->LCR    = 0x03;                            /* DLAB = 0                       */
	pUart->IER		= 0x01;													   /* RBR interrupt enabled 				 */

	NVIC_EnableIRQ(UART0_IRQn);             			   /* enable UART0 Interrupt         */
}

/*----------------------------------------------------------------------------
  Write character to Serial Port
 *----------------------------------------------------------------------------*/

void SER_putFirstChar(int uart, char c) {
  LPC_UART_TypeDef *pUart;
  pUart = (uart == 0) ? (LPC_UART_TypeDef *)LPC_UART0 : (LPC_UART_TypeDef *)LPC_UART2;
  pUart->THR = c;
}

void SER_sendString(int uart, char* str) {
  LPC_UART_TypeDef *pUart;
  pUart = (uart == 0) ? (LPC_UART_TypeDef *)LPC_UART0 : (LPC_UART_TypeDef *)LPC_UART2;
	/* enable interrupt mode THRE */
	pUart->IER		|= 0x02;													 /* THRE interrupt enabled 						*/
	SER_putFirstChar(uart, str[0]);
}

