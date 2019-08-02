/*
 * USART_RS232_C_file.c
 * http://www.electronicwings.com
 *
 */ 

#include "USART_RS232_H_file.h"						/* Include USART header file */

void USART_Init()				/* USART initialize function */
{ 
// USART initialization
// Communication Parameters: 8 Data, 1 Stop, No Parity
// USART Receiver: On
// USART Transmitter: On
// USART Mode: Asynchronous
// USART Baud Rate: 115200 (Double Speed Mode)
UCSRA=0x02;
UCSRB=0x98;
UCSRC=0x86;
UBRRH=0x00;
UBRRL=0x0C;

}


void USART_TxChar(char data)						/* Data transmitting function */
{
	while(!(UCSRA & (1 << UDRE)));  
    UDR = data; 
}

void USART_SendString(char *str)					/* Send string of USART data function */ 
{
	int i=0;																	
	while (str[i]!=0)
	{
		USART_TxChar(str[i]);						/* Send each char of string till the NULL */
		i++;
	}
}