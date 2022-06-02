#include "lcd.h"
#include <avr/io.h>

#define MAX_CHARS 6

#define IS_UPPERCASE(x) ((x) >= 'A' && (x) <= 'Z')
#define IS_LOWERCASE(x) ((x) >= 'a' && (x) <= 'z')
#define IS_DIGIT(x) ((x) >= '0' && (x) <= '9')

#define DECODE_MASK_EVEN 0x0f
#define DECODE_MASK_ODD 0xf0

/* Segment Control Code
         Nibbles       Segments         Char 'A'
SCC =  { nibble0 } = {{K, -, -, A}, = {{0, 0, 0, 1},
	   { nibble1 } =  {J, F, H, B},    {0, 1, 0, 1},
	   { nibble2 } =  {L, E, G, C},    {1, 1, 1, 1},
       { nibble3 } =  {M, P, N, D}},   {0, 0, 0, 0}}
Segments:
    AAAAAAA
  F H  J  K B
  F  H J K  B
    GGG LLL 
  E  P N M  C
  E P  N  M C
    DDDDDDD
A:	Segments: A, B, C, E, F, G, L
	SCC = {{0,0,0,1}, {0,1,0,1}, {1,1,1,1}, {0,0,0,0}}
	SCC_A = { 0x0, 0xF, 0x5, 0x1 } = 0x0F51
		
B:	Segments: A, B, C, D, J, L, N
C:	Segments: A, D, E, F
D:	Segments: A, B, C, D, J, N
E:	Segments: A, D, E, F, G
F:	Segments: A, E, F, G
G:	Segments: A, C, D, E, F, L
H:	Segments: B, C, E, F, G, L
I:	Segments: A, D, J, N
J:	Segments: B, C, D, E
K:	Segments: E, F, G, K, M
L:	Segments: D, E, F
M:	Segments: B, C, E, F, H, K
N:	Segments: B, C, E, F, H, M
O:	Segments: A, B, C, D, E, F
P:	Segments: A, B, E, F, G, L
Q:	Segments: A, B, C, D, E, F, M
R:	Segments: A, B, E, F, G, L, M
S:	Segments: A, C, D, F, G, L
T:	Segments: A, J, N
U:	Segments: B, C, D, E, F
V:	Segments: E, F, K, P
W:	Segments: B, C, E, F, M, P
X:	Segments: H, K, M, P
Y:	Segments: B, C, D, F, G, L
Z:	Segments: A, D, K, P
0:	Segments: A, B, C, D, E, F, K, P
1:	Segments: B, C, K
2:	Segments: A, B, D, E, G, L
3:	Segments: A, B, C, D, L
4:	Segments: B, C, F, G, L
5:	Segments: A, D, F, G, M
6:	Segments: A, C, D, E, F, G, L
7:	Segments: A, B, C
8:	Segments: A, B, C, D, E, F, G, L
9:	Segments: A, B, C, D, F, G, L
*/

#define SCC_NUM(x) (NUMBERS[(x) - '0'])
#define SCC_CHAR(x) (CHARACTERS[(x) - 'a'])

// Segment Control Characters: a-z.
static uint16_t CHARACTERS[] = {
	0xf51,
	0x3991,
	0x1441,
	0x3191,
	0x1641,
	0x641,
	0x1d41,
	0xf50,
	0x3081,
	0x1510,
	0x8648,
	0x1440,
	0x578,
	0x8570,
	0x1551,
	0xe51,
	0x9551,
	0x8e51,
	0x9241,
	0x2081,
	0x1550,
	0x4448,
	0xc550,
	0xc028,
	0x1b50,
	0x5009,
};

// Segment Control Characters: 0-9.
static uint16_t NUMBERS[] = {
	0x5559,
	0x118,
	0x1e11,
	0x1911,
	0xb50,
	0x1b41,
	0x1f41,
	0x111,
	0x1f51,
	0x1b51,
};

void init_lcd() {
	// LCD Control and Status - Register B.
	// Enable external asynchronous clock source (LCDCS).
	// Set 1/3 bias and 1/4 duty cycle (LCXMUX1 and LCDMUX0 both set).
	// All 25 segments enabled (LCDPM2, LCDPM1 and LCDPM0 all set).
	LCDCRB = (1 << LCDCS) | (1 << LCDMUX1) | (1 << LCDMUX0) | (1 << LCDPM2) | (1 << LCDPM1) | (1 << LCDPM0);
	
	// LCD Frame Rate Register.
	// Prescaler N=16 (LCDDPS2, LCDPS1 and LCDPS0 all zero).
	// Clock divider D=8 (LCDCD2, LCDCD1 and LCDCD0 all set).
	LCDFRR = (1 << LCDCD2) | (1 << LCDCD1) | (1 << LCDCD0);
	
	// LCD Contrast Control.
	// Drive with 300 microseconds (LCDDC2, LCDDC1 and LCDDC0 all zero).
	// Contrast control voltage 3.35V (LCDCC3, LCDCC2, LCDCC1 and LCDCC0 all set).
	LCDCCR = (1 << LCDCC3) | (1 << LCDCC2) | (1 << LCDCC1) | (1 << LCDCC0);
	
	// LCD Control and Status - Register A.
	// Enable LCD (LCDEN) and low power waveform (LCDAB).
	// No frame interrupt and no blanking.
	LCDCRA = (1 << LCDEN) | (1 << LCDAB);
}

void clear() {
	// Mask so we don't clear the special stuff.
	LCDDR0 = LCDDR0 & SPECIAL_MASK;
	LCDDR1 = LCDDR1 & SPECIAL_MASK;
	LCDDR2 = LCDDR2 & SPECIAL_MASK;
	//LCDDR3 = 0;
	
	LCDDR5 = 0;
	LCDDR6 = 0;
	LCDDR7 = 0;
	//LCDDR8 = 0;
	
	LCDDR10 = 0;
	LCDDR11 = 0;
	LCDDR12 = 0;
	//LCDDR13 = 0;
	
	LCDDR15 = 0;
	LCDDR16 = 0;
	LCDDR17 = 0;
	//LCDDR18 = 0;
}

int writeChar(char ch, int pos) {
	// Check that we have a valid character.
	if (IS_UPPERCASE(ch)) {
		ch = 'a' + (ch - 'A');
	} else if (!IS_LOWERCASE(ch) && !IS_DIGIT(ch)) {
		return -1;
	}

	// LCD digit within range? If not return error
	if (pos < 0 || pos >= MAX_CHARS) {
		return -1;
	}

	// Lookup LCD segment control code (SCC) in LCD character table.
	uint16_t scc = 0;
	if (IS_LOWERCASE(ch)) {
		scc = SCC_CHAR(ch);
	} else {
		scc = SCC_NUM(ch);
	}
	
	// If LCD digit is even the decoding mask is 0x0F, otherwise it is 0xF0.
	bool even_lcd_digit = pos % 2 == 0 ? true : false;
	uint8_t decode_mask = even_lcd_digit ? DECODE_MASK_EVEN : DECODE_MASK_ODD;
	
	// Point the first relevant LCD Data Register.
	// Position 0/1 maps to LCDDR0 (0xEC), 2/3 maps to LCDDR2 (0xED) and 4/5 to LCDDR3 (0xEE).
	uint8_t* addr = (uint8_t*)(0xEC + pos / 2);
	for (int i = 0; i < 4; ++i) {
		uint8_t nibble = scc & 0xf;
		if (!even_lcd_digit) {
			nibble <<= 4;
		}
		uint8_t* lcddrx = (addr + i * 5);
		if (i == 0) {
			// Don't overwrite special bits in LCDDR0/1/2.
			uint8_t first_mask = decode_mask & ~SPECIAL_MASK;
			*lcddrx = (*lcddrx & ~first_mask) | (nibble & first_mask);
		} else {
			*lcddrx = (*lcddrx & ~decode_mask) | (nibble & decode_mask);
		}
		scc >>= 4;
	}

	return 0;
}

int writeLong(long i) {
	for (int j = 0; j < MAX_CHARS && i != 0; ++j) {
		char digit = i % 10;
		i /= 10;
		if (writeChar(digit + '0', MAX_CHARS - 1 - j) == -1) {
			return -1;
		}
	}
	return 0;
}

int printAt(long num, int pos) {
	if (writeChar((num % 100) / 10 + '0', pos) == -1) return -1;
	return writeChar(num % 10 + '0', pos+1);
}