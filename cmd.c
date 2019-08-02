//#define _DEBUG
#define CLOSE_HTTP 3

#define ALARM_CALL 1

#define POST				1						/* Define method */
#define GET					0

#define MAX_PROCESS_BUFER 300
#define DEFAULT_BUFFER_SIZE	300						/* Define default buffer size */
#define DEFAULT_TIMEOUT		20000					/* Define default timeout */
#define DEFAULT_TIMEOUT_WDR 7000
#define DEFAULT_CRLF_COUNT	2						/* Define default CRLF count */


char Response_Status, CRLF_COUNT = 0;
unsigned int Counter = 0;
unsigned int TimeOut = 0;
char RESPONSE_BUFFER[DEFAULT_BUFFER_SIZE];

enum TERMINAL_RESPONSE_STATUS							/* Enumerate response status */
{
	TERMINAL_RESPONSE_WAITING,
	TERMINAL_RESPONSE_FINISHED,
	TERMINAL_RESPONSE_TIMEOUT,
	TERMINAL_RESPONSE_BUFFER_FULL,
	TERMINAL_RESPONSE_STARTING,
	TERMINAL_RESPONSE_ERROR
};

void PortMcuInit(){
  DDRD=0x02;
  PORTD=0xFC;
  /**/
  DDRC=0x03;
  PORTC=0xFC;
  /**/
  DDRB=0x00;
  PORTB=0xFF;
}


void Read_Response()								/* Read response */
{
	static char CRLF_BUF[2];
	static char CRLF_FOUND;
	unsigned int TimeCount = 0, TimeCountWDR = 0, ResponseBufferLength;
	while(1)
	{
	    if(TimeCountWDR >= (DEFAULT_TIMEOUT_WDR + TimeOut)){
		  TimeCountWDR = 0;
		  		  
		}
		if(TimeCount >= (DEFAULT_TIMEOUT+TimeOut))	/* Check timecount each time */
		{
			CRLF_COUNT   = 0;
			TimeOut      = 0;
			TimeCountWDR = 0;
			Response_Status = TERMINAL_RESPONSE_TIMEOUT;/* if its overflowed then make status timeout and return */
			return;
		}

		if(Response_Status == TERMINAL_RESPONSE_STARTING)
		{
			CRLF_FOUND = 0;
			memset(CRLF_BUF, 0, 2);
			Response_Status = TERMINAL_RESPONSE_WAITING;/* Make status waiting while reading */
		}
		ResponseBufferLength = strlen(RESPONSE_BUFFER);
		if (ResponseBufferLength)
		{
			_delay_ms(1);
			TimeCount++;
			TimeCountWDR++;
			if (ResponseBufferLength==strlen(RESPONSE_BUFFER))
			{   unsigned  int i = 0;
				for (;i<ResponseBufferLength;i++)
				{
					memmove(CRLF_BUF, CRLF_BUF + 1, 1);
					CRLF_BUF[1] = RESPONSE_BUFFER[i];
					if(!strncmp(CRLF_BUF, "\r\n", 2))
					{
						if(++CRLF_FOUND == (DEFAULT_CRLF_COUNT+CRLF_COUNT))
						{
							CRLF_COUNT = 0; TimeOut = 0;
							Response_Status = TERMINAL_RESPONSE_FINISHED;
							return;
						}
					}
				}
				CRLF_FOUND = 0;
			}
		}
		_delay_ms(1);
		TimeCount++;
		TimeCountWDR++;
	}
}

void Start_Read_Response()
{
	Response_Status = TERMINAL_RESPONSE_STARTING;
	do {
		Read_Response();
	} while(Response_Status == TERMINAL_RESPONSE_WAITING);/* Read response till response is waiting */
}


void Buffer_Flush()								/* Flush all variables */
{
	memset(RESPONSE_BUFFER, 0, DEFAULT_BUFFER_SIZE);
	Counter=0;
}

bool WaitForExpectedResponse(char* ExpectedResponse){
    
	Buffer_Flush();
	_delay_ms(200);
	Start_Read_Response();						/* First read response */
	if((Response_Status != TERMINAL_RESPONSE_TIMEOUT) && (strstr(RESPONSE_BUFFER, ExpectedResponse) != NULL))
	  return true;							/* Return true for success */
	else
      return false;								/* Else return false */
}

bool WaitForExpectedResponseArray(const char** ExpectedResponse, const uint8_t size_array){
    
	Buffer_Flush();
	_delay_ms(200);
	Start_Read_Response();						/* First read response */	
	for(uint8_t w=0; w < size_array; w++){	
	  if((Response_Status != TERMINAL_RESPONSE_TIMEOUT) && (strstr(RESPONSE_BUFFER, ExpectedResponse[w]) != NULL)){
		return true;							/* Return true for success */
	  }
	  else
        return false;								/* Else return false */
	 }
	 return false;
}



bool SendATandExpectResponse(char* ATCommand, char* ExpectedResponse)
{
	USART_SendString(ATCommand);				/* Send AT command to TERMINAL */
	USART_SendString("\r\n");
	return WaitForExpectedResponse(ExpectedResponse);
}

bool Start()							
{
	//for (uint8_t i=0;i<5;i++)
	//{ 
		if(SendATandExpectResponse("ATE0","OK")||SendATandExpectResponse("AT","OK"))
		{
			return true;
		}
	//}
	return false;
}