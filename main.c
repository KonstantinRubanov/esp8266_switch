#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/wdt.h>
#include <stdbool.h>

//define debug print enable or disable on uart
#define DEBUGENABLED 1


#ifndef UPE
#define UPE 2
#endif

#ifndef DOR
#define DOR 3
#endif

#ifndef FE
#define FE 4
#endif

#ifndef UDRE
#define UDRE 5
#endif

#ifndef RXC
#define RXC 7
#endif

#ifndef TXC
#define TXC 6
#endif

#define FRAMING_ERROR (1<<FE)
#define PARITY_ERROR (1<<UPE)
#define DATA_OVERRUN (1<<DOR)
#define DATA_REGISTER_EMPTY (1<<UDRE)
#define RX_COMPLETE (1<<RXC)


#include "USART_RS232_C_file.c"		
#include "cmd.c"				


uint8_t packet_detect;
uint8_t init;


void SetDEtect(uint8_t detect){
  packet_detect = detect;
}

void SetInit(uint8_t init_state){
  init = init_state;
}

ISR(USART_RXC_vect)
{
  char status,data;
  status=UCSRA;
  data=UDR;
  if ((status & (FRAMING_ERROR | PARITY_ERROR | DATA_OVERRUN))==0){
	RESPONSE_BUFFER[Counter] = data;				/* Copy data to buffer & increment counter */
	if(Counter == DEFAULT_BUFFER_SIZE)
	  Counter = 0;
	else
	  Counter++;
	if(data == (char)13){
	  if(strstr(RESPONSE_BUFFER, "+IPD") != NULL){
	     SetInit(0);
	     if (strstr(RESPONSE_BUFFER, "/on") > 0) {
		    SetInit(1);
			PORTC|=(1<<0);
		}
		if (strstr(RESPONSE_BUFFER, "/off") > 0) {
		    SetInit(1);
			PORTC&=~(1<<0);
		}
         SetDEtect(1);
	  }
	}
  }	    
}

void blink(){
 	PORTC|=(1<<1);
	_delay_ms(1000);	
	PORTC&=~(1<<1);
	_delay_ms(1000);
}

int main(void)
{
   SetInit(0); 
   SetDEtect(0);  
   cli();  
      
   PortMcuInit();
   
   USART_Init();	 
   _delay_ms(1000);						        
    sei();
	//blink();
    while(!Start());  	
	SendATandExpectResponse("AT+RST","Ready"); 
	//blink();
    SendATandExpectResponse("AT+CIPMODE=0","OK");
     //blink();
	SendATandExpectResponse("AT+CIPMUX=1","OK");
     //blink();
	SendATandExpectResponse("AT+CIPSERVER=1,88","OK");
     //blink();
	SendATandExpectResponse("AT+CIPSTO=5","OK");	
	
	PORTC|=(1<<1);
		
    while(1){
      if(!packet_detect){ _delay_ms(1); }		
	  else{	
	    if(!init){ 
		  if(SendATandExpectResponse("AT+CIPSEND=0,330\r\n",">")){
            USART_SendString("HTTP/1.1 200 OK\r\n"); 
            USART_SendString("Content-Type: text/html; charset=UTF-8\r\n\r\n"); 
            USART_SendString("<html><head><title>ESP8266</title></head><body><button onclick=\"Check(1)\">on</button> <button onclick=\"Check(0)\">off</button><script> function Check(state){ var xhttp=new XMLHttpRequest();xhttp.open(\"GET\",(state)?\"/on\":\"/off\",true);xhttp.send();}</script></body></html>\r\n");	
		    //WaitForExpectedResponse("OK");
		  }
		}
		Buffer_Flush();	
        SetDEtect(0);	    
	  }
	  
  }
  return 0;  	
}



