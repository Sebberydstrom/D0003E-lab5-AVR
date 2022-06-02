#include "Communicator.h"
#include <avr/io.h>
#include "Traffichandler.h"
#include "common.h"

int com_receive_ready(struct Communicator* self, __attribute__((unused)) int arg) {
	uint8_t data = UDR0;
	
	// Send off data to Traffic controller.
	if (data & (1 << NB_CAR_ARRIVAL)) {
		ASYNC(self->ctrl, traffichandler_queue, NORTHBOUND);
	}
	if (data & (1 << SB_CAR_ARRIVAL)) {
		ASYNC(self->ctrl, traffichandler_queue, SOUTHBOUND);
	}
	if (data & (1 << NB_BRIDGE_ENTRY)) {
		ASYNC(self->ctrl, traffichandler_bridge, NORTHBOUND);
	}
	if (data & (1 << SB_BRIDGE_ENTRY)) {
		ASYNC(self->ctrl, traffichandler_bridge, SOUTHBOUND);
	}
	return 0;
}

int com_data_register_ready(struct Communicator* self, __attribute__((unused)) int arg) {
	if (self->data != 0) {
		UDR0 = self->data;
		self->data = 0;
	}
	
	// Disable data register ready interrupt.
	UCSR0B = UCSR0B & ~(1 << UDRIE0);
	return 0;
}

int com_write_data(struct Communicator* self, int data) {
	// Check if data register is ready to be written to.
	if (UCSR0A & (1 << UDRE0)) {
		UDR0 = data;
		self->data = 0;
		} else {
		// Otherwise, wait until the data register is empty
		// and write on that interrupt.
		
		// Enable data register ready interrupt.
		UCSR0B = UCSR0B | (1 << UDRIE0);
		self->data = data;
	}
	return 0;
}