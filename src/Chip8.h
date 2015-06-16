
#ifndef CHIP8_HEADER
#define CHIP8_HEADER

#define MEM_SIZE 4096
#define GFX_WIDTH 64
#define GFX_HEIGHT 32
#define GFX_SIZE (GFX_WIDTH*GFX_HEIGHT)


struct Chip8{

	//Current Opcode
	unsigned short opcode;

	//4K memory
	unsigned char memory[MEM_SIZE];

	//15 8bit general purpose registers and one for the carry flag
	unsigned char V[16];

	//Index Register 0x000 to 0xFFF
	unsigned short I;

	//Program Counter 0x000 to 0xFFF
	unsigned short pc;

	//Pixels
	unsigned char gfx[GFX_SIZE];

	//Timers
	unsigned char delayTimer;
	unsigned char soundTimer;

	//Stack
	unsigned short stack[16];
	//Stack Pointer
	unsigned short sp;

	//Keys
	unsigned char key[16];

	//Fontset
	static const unsigned char fontset[80];


	//Emulation vars
	bool drawFlag;
	unsigned char prevKey[16];


	//Clear memory
	void reset();

	//Load
	void loadGame(const char* game);

	//Emulate one cycle
	virtual unsigned short update();

	bool executeOpcode();

	//Set Keys
	void setKeys(unsigned char keys[16]);

};


#endif