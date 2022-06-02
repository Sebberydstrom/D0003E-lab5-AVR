#ifndef COMMON_H_
#define COMMON_H_

// How many cars are allowed to pass from one direction before the light switches.
#define MAX_CARS_BEFORE_LIGHT_SWITCH 5

// Lanes.
#define NORTHBOUND 0 // Northbound direction.
#define SOUTHBOUND 1 // Southbound direction.

// Light statuses.
#define GREEN 1
#define RED 0

// Delays for cars.
#define TIME_CROSS_BRIDGE 5000 // Time to cross the bridge in ms.
#define DELAY_CROSSING 1000 // Delay for another car to enter bridge in ms.
#define DELAY_LIGHT_SWITCH 500
#define DELAY_SWITCH_TO_RED 500

// Simulator -> AVR
#define NB_CAR_ARRIVAL  0   // Northbound car arrival sensor bit.
#define NB_BRIDGE_ENTRY 1   // Northbound bridge entry sensor bit.
#define SB_CAR_ARRIVAL  2   // Southbound car arrival sensor bit.
#define SB_BRIDGE_ENTRY 3   // Southbound bridge entry sensor bit.

// AVR -> Simualtor
#define NB_GREEN 0  // Northbound green light status bit.
#define NB_RED 1    // Northbound red light status bit.
#define SB_GREEN 2  // Southbound green light status bit.
#define SB_RED 3    // Southbound red light status bit.



#endif /* COMMON_H_ */