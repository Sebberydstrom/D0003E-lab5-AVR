#ifndef TRAFFICHANDLER_H_
#define TRAFFICHANDLER_H_

/* Controller logic
The core task of the controller is to control the status of the four lamps on basis
of just sensor inputs and the passage of time. The controller logic is subject to
the following requirements:
1. It must be safe; i.e., cars going in opposite directions must never be allowed on
   the bridge at the same time.
 
2. It may not stop making progress. This means that the situation where the bridge is
   empty but queued cars are not allowed to enter the bridge must never arise.
3. It may not cause starvation. That is, the controller must make sure that northbound
   cars are not indefinitely delayed by a steady stream of southbound cars, and vice versa.
4. If there is a queue of cars from a particular direction, better bridge utilization
   will be achieved by letting more than one car pass from the same direction before
   allowing cars from the other direction. Delay between cars from the same direction
   should be 1 second.
5. For our particular scenario, we will assume that a reasonable time for passing the
   bridge once it is entered is 5 seconds.
6. It is, however, not required to deal with cars that stop on the bridge, make u-turns,
   ignore green lights, ignore red lights, ignore the one-car-per-green restriction, drive
   at extreme speeds (high and low), etc. These are not unrealistic assumptions ? few real
   traffic light systems are designed to detect (say) cars that stop in the middle of a
   crossing due to engine failure.
*/

#include <stdbool.h>
#include <stdint.h>
#include "TinyTimber.h"

struct Communicator;

struct Lane {
   int16_t in_queue;
   uint8_t light;
};

struct Traffichandler {
	Object super;

   // NORTHBOUND or SOUTHBOUND lane.
   struct Lane lane[2];

   // How many cars are currently on the bridge.
   uint16_t on_bridge;

   // How many cars have passed the bridge since the last time
   // the other side got a green light.
   uint16_t passed_before_change;

   // If an update to the lights is pending, i.e. do not make
   // any decisions about changing them.
   bool light_update_pending;

   uint8_t last_green_direction;

   // Pointer to the serial object as we have to write the light
   // data to it.
   struct Communicator* com;
};

#define initTraffichandler(com) { initObject(), {{0,0}, {0,0}}, 0, 0, false, 0, com }

// Sensor activation for when a car enters the queue.
int traffichandler_queue(struct Traffichandler* self, int direction);

// Sensor activation for when a car enters the bridge.
int traffichandler_bridge(struct Traffichandler* self, int direction);

// Handler for when a car *should* have left bridge.
int traffichandler_leave_bridge(struct Traffichandler* self, int direction);

int traffichandler_check_lights(struct Traffichandler* self, int arg);

// Set the light in `direction` to green, automatically sets the other directions light to red.
int traffichandler_set_light(struct Traffichandler* self, int direction);

// Set light to red in the specified direction.
int traffichandler_set_red_light(struct Traffichandler* self, int direction);

// Write the status of the lights to the serial port.
int traffichandler_write_lights(struct Traffichandler* self, int arg);

// Display number of cars in each queue and cars on the bridge on the LCD.
int traffichandler_print(struct Traffichandler* self, int arg);

int traffichandler_init(struct Traffichandler* self, int arg);




#endif /* TRAFFICHANDLER_H_ */