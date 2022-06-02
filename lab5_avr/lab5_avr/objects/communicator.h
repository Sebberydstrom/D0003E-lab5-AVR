#ifndef COMMUNICATOR_H_
#define COMMUNICATOR_H_


#include <stdint.h>
#include "TinyTimber.h"

// Forward declare, as the Traffichandler also calls us.
struct Traffichandler;

typedef struct Communicator {
	Object super;

	// Buffered data, won't write anything if data is set to 0.
	uint8_t data;

	// Controller to handle received data.
	struct Traffichandler* ctrl;
} Communicator;

#define initCommunicator(ctrl) { initObject(), 0, ctrl }

// Interrupt handler for when data is ready to be read from the serial port register.
int com_receive_ready(struct Communicator* self, int arg);

// Interrupt handler for when data is ready to be written to the serial port register.
// This writes the desired traffic light data if it is nonzero.
int com_data_register_ready(struct Communicator* self, int arg);

// Writes data if possible, if the data register is not ready it will keep the data
// passed and send when it can. Note that it wont send anything if the data register
// is not ready to be written to and data is 0.
int com_write_data(struct Communicator* self, int data);



#endif /* COMMUNICATOR_H_ */