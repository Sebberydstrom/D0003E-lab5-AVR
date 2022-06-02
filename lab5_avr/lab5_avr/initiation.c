#include "initiation.h"


// Setup asynchronous normal mode (U2X = 0)
// Baud rate is the calculated as: BAUD = Clock / (16*UBRR + 1),
// and this gives UBRR = Clock / (16 * BAUD) - 1
#define FOSC 8000000 // Clock rate.
#define BAUD 9600

void init_usart() {
	// Set PRUSART0 bit to one to enable.
	uint8_t mask = (1 << PRUSART0);
	PRR = PRR & ~mask;

	// Setup asynchronous normal mode (U2X = 0)
	// Baud rate is the calculated as: BAUD = Clock / (16*UBRR + 1),
	// and this gives UBRR = Clock / (16 * BAUD) - 1
	uint32_t fosc = FOSC;
	uint32_t baud = BAUD;
	uint32_t ubrr = fosc / (16 * baud) - 1;

	// Set baud rate.
	UBRR0H = ubrr >> 8;
	UBRR0L = ubrr;

	
	// Enable receiver and transmitter. Enable receiver interrupts and
	// interrupts for when the data register is ready to be written to.
	UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0); // | (1 << UDRIE0);

	// Frame format: 9600N81
	// 8 data, 1 stop bit, no parity bit.
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void initiate() {
	
	init_usart();
	init_lcd();
	clear();
	
	// Serial port object.
	com = initCommunicator(NULL);
	// Logic handling object for the traffic lights.
	ctrl = initTraffichandler(&com);
	// setting reference and ...
	com.ctrl = &ctrl;
	// Why make this call? Can have something to do with the reading. // Check out USART.
	com_receive_ready(NULL, 0);
}

