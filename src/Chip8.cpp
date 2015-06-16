#include "Chip8.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <time.h>

#define VX V[(opcode & 0x0F00) >> 8]
#define VY V[(opcode & 0x00F0) >> 4]

const unsigned char Chip8::fontset[80] =
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void Chip8::reset(){
	pc = 0x200;
	opcode = 0;
	I = 0;
	sp = 0;

	for (int i = 0; i < GFX_SIZE; i++){
		gfx[i] = 0;
	}

	for (int i = 0; i < 16; i++){
		stack[i] = 0;
		V[i] = 0;
	}

	for (int i = 0; i < MEM_SIZE; i++){
		memory[i] = 0;
	}

	for (int i = 0; i < 80; i++){
		memory[i] = fontset[i];
	}

	delayTimer = 0;
	soundTimer = 0;

	drawFlag = false;

	srand(time(NULL));
}

void Chip8::loadGame(const char* game){
	std::ifstream file;
	file.open(game, std::ios::in | std::ios::binary);

	if (file.is_open()){

		file.seekg(0, file.end);
		int length = file.tellg();
		file.seekg(0, file.beg);

		char* buffer = new char[length];

		file.read(buffer, length);
		file.close();

		for (int i = 0; i < length; i++){
			memory[0x200 + i] = buffer[i];
		}
		printf("Loaded file: %s\n", game);
	} else {
		printf("Failed to load file: %s\n", game);
	}
}

void Chip8::setKeys(unsigned char keys[16]){
	for (int i = 0; i < 16;i++){
		key[i] = keys[i];
	}
}

unsigned short Chip8::update(){
	opcode = memory[pc] << 8 | memory[pc + 1];

	
	if (executeOpcode()){
		//printf("Executed opcode: 0x%4X at 0x%4X\n", opcode, pc);
	}else{
		//printf("Unknown opcode: 0x%4X at 0x%4X\n", opcode, pc);
	}


	for (int i = 0; i < 16; i++){
		prevKey[i] = key[i];
	}
	

	if (delayTimer > 0){
		delayTimer--;
	}

	if (soundTimer > 0){
		if (soundTimer == 1){
			printf("\a");
		}
		soundTimer--;
	}

	return opcode;
}

bool Chip8::executeOpcode(){

	switch (opcode & 0xF000){
		case 0x0000:
			switch (opcode & 0x000F){
			case 0x0000: // 00E0: Clears the screen
				for (int i = 0; i < GFX_SIZE; i++){
					gfx[i] = 0;
				}
				pc += 2;
				drawFlag = true;
				return true;
			case 0x000E: // 00EE: Returns from a subroutine
				sp--;
				pc = stack[sp];
				pc += 2;
				return true;

			default:
				return false;
			}
			break;

		case 0x1000: // 1NNN: Jumps to address NNN
			pc = (opcode & 0x0FFF);
			return true;

		case 0x2000: // 2NNN: Calls subroutine at NNN
			stack[sp] = pc;
			sp++;
			pc = opcode & 0x0FFF;
			return true;

		case 0x3000: // 3XNN: Skips the next instruction if VX equals NN
			if (VX == (opcode & 0x00FF)){
				pc += 4;
			} else {
				pc += 2;
			}
			return true;

		case 0x4000: // 4XNN: Skips the next instruction if VX does not equal NN
			if (VX != (opcode & 0x00FF)){
				pc += 4;
			} else {
				pc += 2;
			}
			return true;

		case 0x5000: // 5XY0: Skips the next instruction if VX equals VY
			if (VX == VY){
				pc += 4;
			} else {
				pc += 2;
			}
			return true;

		case 0x6000: // 6XNN: Sets VX to NN
			VX = (opcode & 0x00FF);
			pc += 2;
			return true;

		case 0x7000: // 7XNN: Adds NN to VX
			VX += (opcode & 0x00FF);
			pc += 2;
			return true;

		case 0x8000:
			switch (opcode & 0x000F){
			case 0x0000: // 8XY0: Sets VX to the value of VY
				VX = VY;
				pc += 2;
				return true;

			case 0x0001: // 8XY1: Sets VX to VX or VY
				VX |= VY;
				pc += 2;
				return true;

			case 0x0002: // 8XY2: Sets VX to VX and VY
				VX &= VY;
				pc += 2;
				return true;

			case 0x0003: // 8XY3: Sets VX to VX xor VY
				VX ^= VY;
				pc += 2;
				return true;

			case 0x0004: // 8XY4: Adds VY to VX     VF is set to 1 if there is a carry value and to 0 if there is not
				//V[0xF] = (VY > (0xFF - VX));
				if (VY > (0xFF - VX)){
					V[0xF] = 1;
				} else {
					V[0xF] = 0;
				}
				VX += VY;
				pc += 2;
				return true;

			case 0x0005: // 8XY5: VY is subtracted from VX     VF is set to 0 if there is a borrow, and 1 if there is not
				//V[0xF] = (VY <= VX);
				if (VY > VX){
					V[0xF] = 0;
				} else {
					V[0xF] = 1;
				}
				VX -= VY;
				pc += 2;
				return true;

			case 0x0006: // 8XY6: Shifts VX right by 1    VF is set to the value of the least significant bit of VX before the shift
				V[0xF] = VX & 0x1;
				VX >>= 1;
				pc += 2;
				return true;

			case 0x0007: // 8XY7: Sets VX to VY minus VX    VF is set to 0 when there is a borrow and 1 when there is not
				//V[0xF] = (VX <= VY);
				if (VX > VY){
					V[0xF] = 0;
				} else {
					V[0xF] = 1;
				}
				VX = VY - VX;
				pc += 2;
				return true;

			case 0x000E: // 8XYE: Shifts VX left by 1    VF is set to the value of the most significant bit of VX before the shift
				V[0xF] = VX >> 7;
				VX <<= 1;
				pc += 2;
				return true;

			default:
				return false;

			}
			break;

		case 0x9000: // 9XY0: Skips the next instruction if VX doesn't equal VY
			if (VX != VY){
				pc += 4;
			} else {
				pc += 2;
			}
			return true;

		case 0xA000: // ANNN: Sets I to the address NNN
			I = opcode & 0x0FFF;
			pc += 2;
			return true;

		case 0xB000: // BNNN: Jumps to the address NNN plus V0
			pc = (opcode & 0x0FFF) + V[0];
			return true;

		case 0xC000: // CXNN: Sets VX to a random number, masked by NN
			VX = (rand() % 0xFF) & (opcode & 0x00FF);
			pc += 2;
			return true;

		case 0xD000: // CXYN: Sprites stored in memory at location in index register (I), maximum 8bits wide. Wraps around the screen. If when drawn, clears a pixel, register VF is set to 1 otherwise it is zero. All drawing is XOR drawing (i.e. it toggles the screen pixels)
		{
			unsigned short x = VX;
			unsigned short y = VY;
			unsigned short height = opcode & 0x000F;
			unsigned short pixel;

			V[0xF] = 0;
			for (int yline = 0; yline < height; yline++)
			{
				pixel = memory[I + yline];
				for (int xline = 0; xline < 8; xline++)
				{
					if ((pixel & (0x80 >> xline)) != 0)
					{
						if (gfx[(x + xline + ((y + yline) * 64))] == 1)
						{
							V[0xF] = 1;
						}
						gfx[x + xline + ((y + yline) * 64)] ^= 1;
					}
				}
			}

			drawFlag = true;
			pc += 2;
			return true;
		}

		case 0xE000:
			switch (opcode & 0x00FF){
			case 0x009E: // EX9E: Skips the next instruction if the key stored in VX is pressed
				if (key[VX] != 0){
					pc += 4;
				}
				else{
					pc += 2;
				}
				return true;
			case 0x00A1: // EXA1: Skips the next instruction if the key stored in VX is not pressed
				if (key[VX] == 0){
					pc += 4;
				}
				else{
					pc += 2;
				}
				return true;

			default:
				return false;

			}
			break;

		case 0xF000:
			switch (opcode & 0x00FF){
			case 0x0007: // FX07: Sets VX to the value of the delay timer
				VX = delayTimer;
				pc += 2;
				return true;
			case 0x000A: // FX0A: A key press is awaited and then stored in VX
				for (int i = 0; i < 16; i++){
					if (key[i] != 0 && prevKey[i] == 0){
						VX = i;
						pc += 2;
						break;
					}
				}
				return true;
			case 0x0015: // FX15: Sets the delay timer to VX
				delayTimer = VX;
				pc += 2;
				return true;
			case 0x0018: // FX18: Sets the sound timer to VX
				soundTimer = VX;
				pc += 2;
				return true;
			case 0x001E: // FX1E: Adds VX to I
				//V[0xF] = I + VX > 0xFFF;
				if (I + VX > 0xFFF){
					V[0xF] = 1;
				} else {
					V[0xF] = 0;
				}
				I += VX;
				pc += 2;
				return true;
			case 0x0029: // FX29: Sets I to the location of the sprite for the character in VX Characters 0-F are represented by a 4x5 font
				I = VX * 0x5;
				pc += 2;
				return true;
			case 0x0033: // FX33: Stores the Binary-coded decimal representation of VX, with the most significant of three digits at the address in I, the middle digit at I plus 1, and the least significant digit at I plus 2. (In other words, take the decimal representation of VX, place the hundreds digit in memory at location in I, the tens digit at location I+1, and the ones digit at location I+2.)
				memory[I] = VX / 100;
				memory[I + 1] = (VX / 10) % 10;
				memory[I + 2] = (VX % 100) % 10;
				pc += 2;
				return true;
			case 0x0055: // FX55: Stores V0 to VX in memory starting at address I
				for (int i = 0; i <= ((opcode & 0x0F00) >> 8); i++){
					memory[i + I] = V[i];
				}
				I += ((opcode & 0x0F00) >> 8) + 1;
				pc += 2;
				return true;
			case 0x0065: // FX55: Fills V0 to VX with values from memory starting at address I
				for (int i = 0; i <= ((opcode & 0x0F00) >> 8); i++){
					V[i] = memory[i + I];
				}
				I += ((opcode & 0x0F00) >> 8) + 1;
				pc += 2;
				return true;

			default:
				return false;

			}
			break;

		default:
			return false;
	}

}