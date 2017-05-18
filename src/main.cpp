#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Chip8.h"
#include "DisplaySDL.h"

#ifdef WIN32
	#include <direct.h>
	#define GetCurrentDir _getcwd
#else
	#include <unistd.h>
	#define GetCurrentDir getcwd
#endif

int main(int argc, char *argv[]){

	const char* gameToLoad = "games/PONG";
	bool debug = false;
	int smooth = 0x0;

	if (argc >= 2){
		gameToLoad = argv[1];
		for (int i = 2; i < argc; i++){
			if (strcmp(argv[i], "debug") == 0){
				debug = true;
			}else if (strcmp(argv[i], "smooth") == 0){
				smooth = 0x20;
			}
		}
	}

	char cwd[FILENAME_MAX];

	GetCurrentDir(cwd, FILENAME_MAX);

	printf("Working dir: %s\n", cwd);

	Chip8* chip = new Chip8();
	DisplaySDL* display = new DisplaySDL("Chip-8 Emulator", (GFX_WIDTH * 10) + (debug*128), GFX_HEIGHT * 10);

	chip->reset();

	chip->loadGame(gameToLoad);

	bool running = true;
	while (running){
		chip->update();
		display->update();
		
		if (chip->drawFlag){

			display->drawGridAt(chip->gfx, GFX_WIDTH, GFX_HEIGHT, 0xFF, 0, 0, GFX_WIDTH * 10, GFX_HEIGHT * 10, 0xFF, smooth);
			if (debug){
				display->drawGridAt(chip->V, 16, 1, 0x01, 64 * 10, 0, 16 * 8, 1 * 8);
				display->drawGridAt(chip->memory, 64, 64, 0x01, 64 * 10, 16, 64 * 2, 64 * 2);
			}
			display->draw();

			chip->drawFlag = false;
		}
		chip->setKeys(display->keys);

		if (display->errored){
			return 1;
		}
		if (display->quit){
			return 0;
		}
	}


	delete display;
	delete chip;
	return 0;
}