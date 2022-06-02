#ifndef LCD_H_
#define LCD_H_

#include <stdint.h>
#include <stdbool.h>

// Mask for the special characters in LCDDR0/1/2.
#define SPECIAL_MASK ((1 << 1) | (1 << 2) | (1 << 5) | (1 << 6))

/* Special Character Map:
LCDDR0:  Bit 1: Caret  (1)
         Bit 2: Number (1)
	     Bit 5: Caret  (2)
		 Bit 6: Number (2)
LCDDR1:  Bit 1: Number (4)
         Bit 2: Caret  (3)
		 Bit 5: Number (5)
		 Bit 6: Caret  (4)
LCDDR2:  Bit 1: Caret  (9)
         Bit 2: Number (9)
		 Bit 5: Caret  (10)
		 Bit 6: Number (10)
LCDDR3:  Bit 1: Caret  (5) + Number (3)
LCDDR8:  Bit 1: Colons
LCDDR13: Bit 1: Caret  (7) + Caret  (8)
LCDDR18: Bit 1: Number (7) + Number (8)
*/

// Initialize the LCD screen.
void init_lcd();

// Clear everything on the screen except special characters.
void clear();

// Writes a single alphanumerical character to the screen at position 0 to 5. 
int writeChar(char ch, int pos);

// Writes a long the to the screen, the lowest six digits are shown.
int writeLong(long i);

int printAt(long num, int pos);

#endif