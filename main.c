/*
* Ibus2.c
*
* Created: 1-5-2024 10:38:40
* Author : bertv
*/

#include <avr/io.h>
#include <asf.h>
#define F_CPU           16000000UL

#include <stdio.h>	/* for "puts" */
#include <stdlib.h>
#include <string.h>  /*for string manupulations */
#include "global.h"
#include <util/delay.h>


#define FOSC 16000000 // Clock Speed
typedef unsigned int UINT;
typedef unsigned char UCHAR;
typedef unsigned long ULONG;
typedef unsigned char byte;
typedef unsigned int  word;
typedef unsigned long lword;
typedef unsigned char   u8;
typedef unsigned int    u16;
typedef unsigned long   u32;
typedef signed char     s8;
typedef signed int      s16;
typedef signed long     s32;

#define MYUBRR FOSC/16/BAUD-1
#define BAUD115000 FOSC/8/(115000-1)

#define maxlength 2300
#define IBUS_DEFAULT_VALUE (uint16_t) 1500
#define IBUS_MIN_VALUE (uint16_t) 900
#define IBUS_FRAME_LENGTH 0x20  // iBus packet size (2 byte header, 14 channels x 2 bytes, 2 byte checksum)
#define IBUS_COMMAND40 0x40     // Command is always 0x40
#define IBUS_MAXCHANNELS 14     // iBus has a maximum of 14 channels
#define maxppmchannels 10

#define Led		B,5
#define PPM		B,0
#define TEST	B,1

void USART_Init(UINT);
void usart_putc(char data);
void usart_pstr(char const *s);
void CopyContent(void);
u8 pulsecounter=0;
u16 pulsarray[maxppmchannels+3];
u16 pulsmidcontent[maxppmchannels+3];
u16 ppm_channel_data[IBUS_MAXCHANNELS+5];

void InitTimer1(void);
u16 ibus_checksum=0;
u8 buffer_index=0;
u8 dataready=false;
//char temp[50];

void CopyContent(void)
{
	SET(Led);
	u8 i=0;
	if (dataready) {return;}
	for (i=0;i<maxppmchannels;i++)
	{
		if (dataready==false) {pulsmidcontent[i]=pulsarray[i];}
		
	}
	dataready=true;
}

ISR(PCINT0_vect)
{
	static u16 OldTnct=0;
	u16 Pulse;

	
	if (IS_SET(PPM)) {
		SET(TEST);
	
	}
	else {
			Pulse=TCNT1-OldTnct;
			OldTnct=TCNT1;
			if ((Pulse>5000) && (pulsecounter<maxppmchannels)) {RESET(TEST);pulsecounter=0;CopyContent();return;}

			pulsarray[pulsecounter]=Pulse>>=1;
			pulsecounter++;
			if (pulsecounter>maxppmchannels) {pulsecounter=0;}
		
	}

}
void InitTimer1(void)
{
	TCCR1A=0;
	TCCR1B=(1<<CS11 ); //prescaler 128 1<<CS11;
	TIFR1 = 1<<TOV1; //Clear TOV1 / clear pending interrupts  timer0
	TCNT1=0;
}
int main(void)
{
	u8 i=0;
	u8 d=0;
	u8 b=0;
	USART_Init(BAUD115000);
	PCICR=0;
	PCMSK1=0;
	PCICR=(1<<PCIE0);
	PCMSK0=(1<<PCINT0);
	SET_OUTPUT(Led);
	SET_OUTPUT(TEST);
	SET_INPUT(PPM);
	InitTimer1();

	//usart_pstr("test 1234567890 abcdefgh");
	
	for (i=0; i<IBUS_MAXCHANNELS ;i++) {
		ppm_channel_data[i]=IBUS_MIN_VALUE;
	};
	ppm_channel_data[0]=IBUS_DEFAULT_VALUE;
	ppm_channel_data[1]=IBUS_DEFAULT_VALUE;
	ppm_channel_data[3]=IBUS_DEFAULT_VALUE;
	ppm_channel_data[IBUS_MAXCHANNELS-1]=IBUS_DEFAULT_VALUE; //for angle mode permanent on
	for (i=0; i<maxppmchannels ;i++) {
		pulsarray[i]=IBUS_MIN_VALUE;
	};
	sei();

	
	while(1)
	{
		
		
		usart_putc(IBUS_FRAME_LENGTH);
		usart_putc(IBUS_COMMAND40);
		
		buffer_index=0;
		ibus_checksum=0xffff -IBUS_FRAME_LENGTH-IBUS_COMMAND40;
		// Write the IBus channel data to the buffer
		for (i = 0; i < IBUS_MAXCHANNELS; i++) {
			d=ppm_channel_data[i] & 0xFF;
			ibus_checksum-=d;
			usart_putc(d);
			d=(ppm_channel_data[i] >> 8) & 0xFF;
			ibus_checksum-=d;
			usart_putc(d);
		}
		
		usart_putc(ibus_checksum & 0xFF);
		usart_putc((ibus_checksum >> 8) & 0xFF);
		// Write the IBus checksum to the buffer
		RESET(Led);
		
		for (i=0;i<255;i++){
			if (dataready) {
				for (b=0;b<maxppmchannels;b++) {ppm_channel_data[b]=pulsmidcontent[b];
					//printf(temp,"%d,",ppm_channel_data[b]);
					//usart_pstr(temp);
				}
				dataready=false;
				RESET(Led);
				//printf(temp,"\r\n");
				//usart_pstr(temp);
				
			}
			_delay_us(90);
		}
		
		
	}
}
void USART_Init( UINT ubrr)
{
	/*Set baud rate */
	UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char)ubrr;
	/*Enable receiver and transmitter */
	UCSR0B = (1<<RXEN0)|(1<<TXEN0)| (1<<RXCIE0);
	/* Set frame format: 8data, 2stop bit */
	UCSR0C = (1<<USBS0)|(3<<UCSZ00);
	UCSR0A=(1<<U2X0); //double speed
}

void usart_putc(char data) {
	// Wait for empty transmit buffer
	while ( !(UCSR0A & (1<<UDRE0)) );
	// Start transmission
	UDR0 = data;

}

void usart_pstr(char const *s)
{
	

	// loop through entire string
	while (*s) {
		
		usart_putc(*s);
		s++;
	}

}
//betaflight checksum calculation
/*
static uint16_t calculateChecksum(const uint8_t *ibusPacket)
{
	uint16_t checksum = 0xFFFF;
	uint8_t dataSize = ibusPacket[0] - IBUS_CHECKSUM_SIZE;
	for (unsigned i = 0; i < dataSize; i++) {
		checksum -= ibusPacket[i];
	}

	return checksum;
}

bool isChecksumOkIa6b(const uint8_t *ibusPacket, const uint8_t length)
{
	uint16_t calculatedChecksum = calculateChecksum(ibusPacket);

	// Note that there's a byte order swap to little endian here
	return (calculatedChecksum >> 8) == ibusPacket[length - 1]
	&& (calculatedChecksum & 0xFF) == ibusPacket[length - 2];
}
*/
