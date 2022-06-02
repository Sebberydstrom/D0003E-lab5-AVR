#include "Traffichandler.h"
#include "Communicator.h"
#include "common.h"
#include "lcd.h"
#include <avr/io.h>

// What happen with this command? What's written and why while?
#define ASSERT(expr) if (!(expr)) while(1) { LCDDR0 = 0xff; }

#define NB_PRINT_POS 0
#define SB_PRINT_POS 4
#define BRIDGE_PRINT_POS 2

#define NB_GREEN_MASK (1 << NB_GREEN)
#define NB_RED_MASK (1 << NB_RED)
#define SB_GREEN_MASK (1 << SB_GREEN)
#define SB_RED_MASK (1 << SB_RED)
#define PACK_LIGHTS(N, S) (((N) == GREEN ? NB_GREEN_MASK : NB_RED_MASK) | ((S) == GREEN ? SB_GREEN_MASK : SB_RED_MASK))

#define NORTHBOUND_GREEN PACK_LIGHTS(GREEN, RED)
#define SOUTHBOUND_GREEN PACK_LIGHTS(RED, GREEN)

int traffichandler_queue(struct Traffichandler* self, int direction) {
	ASSERT(direction == SOUTHBOUND || direction == NORTHBOUND);
	
	if (self->lane[NORTHBOUND].in_queue == 0 && self->lane[SOUTHBOUND].in_queue == 0 && self->on_bridge == 0) {
		AFTER(MSEC(500), self, traffichandler_check_lights, 0);
	}
	self->lane[direction].in_queue += 1;
	ASYNC(self, traffichandler_print, 0);
	return 0;
}

int traffichandler_bridge(struct Traffichandler* self, int direction) {
	ASSERT(direction == SOUTHBOUND || direction == NORTHBOUND);
	self->lane[direction].in_queue -= 1;
	self->on_bridge += 1;
	self->passed_before_change += 1;

	AFTER(MSEC(TIME_CROSS_BRIDGE), self, traffichandler_leave_bridge, direction);
	ASYNC(self, traffichandler_print, 0);

	// When a car has begun to cross the bridge, set the lights to red and check
	// which lights to set.
	// AFTER(MSEC(DELAY_SWITCH_TO_RED), self, traffichandler_set_red_light, 0); // Don't need a delay, but makes it easier to check in GUI.
	// AFTER(MSEC(DELAY_CROSSING), self, traffichandler_check_lights, 0);
	
	// Check directly if the traffic lights need to change.
	ASYNC(self, traffichandler_check_lights, 0);
	return 0;
}

int traffichandler_leave_bridge(struct Traffichandler* self, __attribute__((unused)) int direction) {
	self->on_bridge -= 1;
	ASYNC(self, traffichandler_print, 0);
	return 0;
}

int traffichandler_check_lights(struct Traffichandler* self, __attribute__((unused)) int arg) {
	struct Lane* north = &self->lane[NORTHBOUND];
	struct Lane* south = &self->lane[SOUTHBOUND];
	
	ASSERT(self->light_update_pending == false);
	
	if (self->on_bridge == 0) {
		// No cars on the bridge. Just put a green light!
		if (north->in_queue > 0) {
			ASYNC(self, traffichandler_set_light, NORTHBOUND_GREEN);
		} else if (south->in_queue > 0) {
			ASYNC(self, traffichandler_set_light, SOUTHBOUND_GREEN);
		}
	} else {
		if (self->passed_before_change >= MAX_CARS_BEFORE_LIGHT_SWITCH) {
			// When too many cars have passed on one side, switch over the light.
			if (self->last_green_direction == NORTHBOUND && south->in_queue > 0) {
				self->light_update_pending = true;
				AFTER(MSEC(TIME_CROSS_BRIDGE + DELAY_LIGHT_SWITCH), self, traffichandler_set_light, SOUTHBOUND_GREEN);
				return 0;
			} else if (self->last_green_direction == SOUTHBOUND && north->in_queue > 0) {
				self->light_update_pending = true;
				AFTER(MSEC(TIME_CROSS_BRIDGE + DELAY_LIGHT_SWITCH), self, traffichandler_set_light, NORTHBOUND_GREEN);
				return 0;
			}
		}
		
		// Otherwise if we have more cars waiting for the currently green side, let one more through.
		// If there are no more cars left, schedule a change for the other side.

		uint8_t active_direction = self->last_green_direction;
		uint8_t other_direction = active_direction == NORTHBOUND ? SOUTHBOUND : NORTHBOUND;
		if (self->lane[active_direction].in_queue > 0) {
			uint8_t lights = active_direction == NORTHBOUND ? NORTHBOUND_GREEN : SOUTHBOUND_GREEN;
			ASYNC(self, traffichandler_set_light, lights);
		} else if (self->lane[other_direction].in_queue > 0) {
			self->light_update_pending = true;
			uint8_t other_lights = active_direction == NORTHBOUND ? SOUTHBOUND_GREEN : NORTHBOUND_GREEN;
			
			AFTER(MSEC(TIME_CROSS_BRIDGE + DELAY_LIGHT_SWITCH), self, traffichandler_set_light, other_lights);
		} else {
			// If no cars are currently queued on either side, but a car is on the bridge then wait for more cars
			// to possible join the queue before making a decision.
			AFTER(MSEC(10), self, traffichandler_check_lights, 0);
		}
	}
	return 0;
}

int traffichandler_set_light(struct Traffichandler* self, int packed_data) {
	self->light_update_pending = false;
	uint8_t nb_green = (packed_data >> NB_GREEN) & 0x1;
	uint8_t sb_green = (packed_data >> SB_GREEN) & 0x1;

	// Check if colors have been switched since last time.
	if ((nb_green && self->last_green_direction != NORTHBOUND) ||
	    (sb_green && self->last_green_direction != SOUTHBOUND)) {
		self->passed_before_change = 0;
	}
	
	self->lane[NORTHBOUND].light = nb_green;
	self->lane[SOUTHBOUND].light = sb_green;
	
	ASYNC(self->com, com_write_data, packed_data); // Write data to serial port.

	ASYNC(self, traffichandler_print, 0);
	return 0;
}

int traffichandler_set_red_light(struct Traffichandler* self, __attribute__((unused)) int direction) {
	ASYNC(self->com, com_write_data, PACK_LIGHTS(RED, RED));
	if (self->lane[NORTHBOUND].light == GREEN) {
		self->last_green_direction = NORTHBOUND;
	} else {
		self->last_green_direction = SOUTHBOUND;
	}
	self->lane[NORTHBOUND].light = RED;
	self->lane[SOUTHBOUND].light = RED;
	
	ASYNC(self, traffichandler_print, 0);
	return 0;
}

int traffichandler_write_lights(struct Traffichandler* self, int packed_data) {
	ASYNC(self->com, com_write_data, packed_data);
	return 0;
}

int traffichandler_print(struct Traffichandler* self, __attribute__((unused)) int arg) {
	/* Controller user interface
	The display of the AVR butterfly should print at least the following information:
	1. The length of the queue of cars waiting to enter the bridge in northbound direction.
	2. The length of the queue of cars waiting to enter the bridge in southbound direction.
	3. The number of cars currently on the bridge.
	This can be achieved by conceptually dividing the display in three parts, with two digits each.
	*/
	printAt(self->lane[NORTHBOUND].in_queue, NB_PRINT_POS);
	printAt(self->lane[SOUTHBOUND].in_queue, SB_PRINT_POS);
	printAt(self->on_bridge, BRIDGE_PRINT_POS);
	
	// Display colons.
	LCDDR8 = (LCDDR8 & ~0x1) | 0x1;
	
	if (self->lane[NORTHBOUND].light == GREEN) {
		LCDDR0 = (LCDDR0 & ~SPECIAL_MASK) | (1 << 2);
		} else {
		LCDDR0 = (LCDDR0 & ~SPECIAL_MASK) | (1 << 1);
	}
	
	if (self->lane[SOUTHBOUND].light == GREEN) {
		LCDDR1 = (LCDDR1 & ~SPECIAL_MASK) | (1 << 1);
		} else {
		LCDDR1 = (LCDDR1 & ~SPECIAL_MASK) | (1 << 6);
	}
	
	return 0;
}

int traffichandler_init(struct Traffichandler* self, int arg) {
	ASYNC(self, traffichandler_print, 0);
	ASYNC(self->com, com_write_data, PACK_LIGHTS(RED, RED));
}