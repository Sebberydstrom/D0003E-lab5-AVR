#include <avr/io.h>
#include "inititation.h"

int main() {

	// initiates necessary objects, display & USART serial port.
	initiate();

	INSTALL(&com, com_receive_ready, IRQ_USART0_RX);
	INSTALL(&com, com_data_register_ready, IRQ_USART0_UDRE);

	return TINYTIMBER(&ctrl, traffichandler_init, 0);
}