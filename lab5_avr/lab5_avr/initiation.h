#ifndef INITIATION_H_
#define INITIATION_H_

#include "TinyTimber.h"
#include "objects/Traffichandler.h"
#include "objects/Communicator.h"
#include "lcd.h"

// Global objects 
struct Communicator com;
struct Traffichandler ctrl;

// Objects initiation/creation & Serial COM/USART initiation & lcd initiation.
void init_usart();
void initiate();





#endif /* INITIATION_H_ */