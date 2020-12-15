#include <stdio.h> // Standard and File IO
#include <stdlib.h> // Memory Allocation and Exit Codes
#include <stdbool.h> // Boolean Types
#include <string.h> // String Comparison
#include <SDL.h> // Graphics and Keyboard Input

const int MS_SIZE = 32; // The number of words in the main store
const int MAX_OP = 5; // The maximum operation mnemonic length
const char* const PROGRAM_NAME = "PATP"; // The window's title
const int BACKGROUND[] = {0x7f, 0x7f, 0x7f}; // Grey
const int ROW_COUNT = 17; // There are 17 potential y coordinates for a memory cell
const int COLUMN_COUNT = 18; // There are 18 potential x coordinates for a memory cell
const double CELL_SIZE = 0.9; // The cell will be 0.9 times the maximum length / width
const int WORD_LENGTH = 8; // Number of cells per word
const int PLAY = SDL_SCANCODE_SPACE; // Space is used to pause and play
const int STEP = SDL_SCANCODE_RETURN; // Enter (return) is used to go step-by-step
const int QUIT = SDL_SCANCODE_ESCAPE; // Escape quits the program
const int DELAY = 250; // The number of milliseconds between each step in play mode

/* Returns the correct opcode for the operation mnemonic
   Different operations may have multiple valid mnemonics
   Mnemonics must be fully capitalised
 */
unsigned char get_opcode(char operation[]) {
	if (!strcmp(operation, "INC1") || !strcmp(operation, "INC")) {
		return 1;
	}
	
	if (!strcmp(operation, "ADD")) {
		return 2;
	}
	
	if (!strcmp(operation, "DEC1") || !strcmp(operation, "DEC")) {
		return 3;
	}
	
	if (!strcmp(operation, "JMP")) {
		return 4;
	}
	
	if (
		!strcmp(operation, "BUZ")
		|| !strcmp(operation, "BNZ")
		|| !strcmp(operation, "BZC")
		|| !strcmp(operation, "BNE")
	) {
		return 5;
	}
	
	if (!strcmp(operation, "LOAD")) {
		return 6;
	}
	
	if (!strcmp(operation, "STORE")) {
		return 7;
	}
	
	// Clear and undefine mnemonics default to clear
	return 0;
}

/* Returns true if the operation requires an operand
 */
bool needs_op(unsigned char opcode) {
	switch (opcode) {
		// CLEAR, INC, and DEC do not require an operand
		case 0:
		case 1:
		case 3:
			return false;
			
		// ADD, JMP, BNZ, LOAD, and STORE require an operand
		default:
			return true;
	}
}

/* Runs the program according to the data in the main store
 */
void start(unsigned char main_store[]) {
	// The PC and CCR are not intialised
	//     The PC can be represented with a for loop as i
	//     The CCR is just !data_register
	unsigned char data_register = 0; // The data register (starts at zero)
	
	// The window for the graphical display is initialised
	SDL_DisplayMode display_mode;
	SDL_GetDesktopDisplayMode(0, &display_mode);
	
	SDL_Window* window = SDL_CreateWindow(
		PROGRAM_NAME, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		display_mode.w, display_mode.h, SDL_WINDOW_FULLSCREEN
	);
	
	SDL_Surface* window_surface = SDL_GetWindowSurface(window);
	
	bool play = false;
	
	for (int i = 0; i < MS_SIZE; i++) {
		// The operation is stored in the first 3 bits
		unsigned char operation = (main_store[i] & 0xe0) >> 5;
		// The operand is stored in the last 5 bits
		unsigned char operand = main_store[i] & 0x1f;
		
		if (i < 10) {
			printf(" ");
		}
		
		printf("%d: ", i);
		
		switch (operation) {
			// INC
			case 1:
				data_register++;
				printf("Inc\n");
				break;
				
			// ADD
			case 2:
				data_register += operand;
				printf("Add %d\n", operand);
				break;
			
			// DEC
			case 3:
				data_register--;
				printf("Dec\n");
				break;
				
			// JMP
			case 4:
				i = operand - 1;
				printf("Jump %d\n", operand);
				break;
				
			// BNZ
			case 5:
				i = data_register ? operand - 1 : i;
				printf("Branch %d\n", operand);
				break;
				
			// LOAD
			case 6:
				data_register = main_store[operand];
				printf("Load %d\n", operand);
				break;
			
			// STORE
			case 7:
				main_store[operand] = data_register;
				printf("Store %d\n", operand);
				break;
				
			// CLEAR
			default:
				data_register = 0;
				printf("Clear\n");
		}
		
		// The current state is displayed
		// The screen is filled grey for the background
		SDL_FillRect(
			window_surface,
			NULL,
			SDL_MapRGB(window_surface->format, BACKGROUND[0], BACKGROUND[1], BACKGROUND[2])
		);
		
		// The states of the main store are blitted to the display
		for (int j = 0; j < 16; j++) {
			// The size and vertical position of the cell are determined
			SDL_Rect rect = {
				.y =
					j * window_surface->h / ROW_COUNT
					+ (1 - CELL_SIZE) * window_surface->h / (2 * ROW_COUNT)
				,
				.w = CELL_SIZE * window_surface->w / COLUMN_COUNT,
				.h = CELL_SIZE * window_surface->h / ROW_COUNT
			};
			
			for (int k = 0; k < WORD_LENGTH; k++) {
				// The horizontal position of the cell is determined
				rect.x =
					(k + 1) * window_surface->w / COLUMN_COUNT
					+ (1 - CELL_SIZE) * window_surface->w / (2 * COLUMN_COUNT)
				;
				
				// The cell is drawn
				SDL_FillRect(
					window_surface,
					&rect,
					SDL_MapRGB(
						window_surface->format,
						!!(main_store[j] & (1 << (WORD_LENGTH - k - 1))) * 0xff,
						!!(main_store[j] & (1 << (WORD_LENGTH - k - 1))) * 0xff,
						!!(main_store[j] & (1 << (WORD_LENGTH - k - 1))) * 0xff
					)
				);
			}
		}
		
		for (int j = 0; j < 16; j++) {
			// The size and vertical position of the cell are determined
			SDL_Rect rect = {
				.y =
					j * window_surface->h / ROW_COUNT
					+ (1 - CELL_SIZE) * window_surface->h / (2 * ROW_COUNT)
				,
				.w = CELL_SIZE * window_surface->w / COLUMN_COUNT,
				.h = CELL_SIZE * window_surface->h / ROW_COUNT
			};
			
			for (int k = 0; k < WORD_LENGTH; k++) {
				// The horizontal position of the cell is determined
				rect.x =
					(k + 1) * window_surface->w / COLUMN_COUNT
					+ (1 - CELL_SIZE) * window_surface->w / (2 * COLUMN_COUNT)
					+ window_surface->w / 2
				;
				
				// The cell is drawn
				SDL_FillRect(
					window_surface,
					&rect,
					SDL_MapRGB(
						window_surface->format,
						!!(main_store[j + MS_SIZE / 2] & (1 << (WORD_LENGTH - k - 1))) * 0xff,
						!!(main_store[j + MS_SIZE / 2] & (1 << (WORD_LENGTH - k - 1))) * 0xff,
						!!(main_store[j + MS_SIZE / 2] & (1 << (WORD_LENGTH - k - 1))) * 0xff
					)
				);
			}
		}
		
		// The current line display is drawn
		if (i < MS_SIZE / 2) {
			SDL_Rect rect = {
				.x = (1 - CELL_SIZE) * window_surface->w / (2 * COLUMN_COUNT),
				.y =
					i * window_surface->h / ROW_COUNT
					+ (1 - CELL_SIZE) * window_surface->h / (2 * ROW_COUNT)
				,
				.w = CELL_SIZE * window_surface->w / COLUMN_COUNT,
				.h = CELL_SIZE * window_surface->h / ROW_COUNT
			};
			
			SDL_FillRect(
				window_surface,
				&rect,
				SDL_MapRGB(
					window_surface->format, 0, 0xff, 0
				)
			);
		}
		
		else {
			SDL_Rect rect = {
				.x =
					(1 - CELL_SIZE) * window_surface->w / (2 * COLUMN_COUNT)
					+ window_surface->w / 2
				,
				.y =
					(i - 16) * window_surface->h / ROW_COUNT
					+ (1 - CELL_SIZE) * window_surface->h / (2 * ROW_COUNT)
				,
				.w = CELL_SIZE * window_surface->w / COLUMN_COUNT,
				.h = CELL_SIZE * window_surface->h / ROW_COUNT
			};
			
			SDL_FillRect(
				window_surface,
				&rect,
				SDL_MapRGB(
					window_surface->format, 0, 0xff, 0
				)
			);
		}
		
		// The size and vertical position of the cell are determined
		SDL_Rect rect = {
			.y =
				16 * window_surface->h / ROW_COUNT
				+ (1 - CELL_SIZE) * window_surface->h / (2 * ROW_COUNT)
			,
			.w = CELL_SIZE * window_surface->w / COLUMN_COUNT,
			.h = CELL_SIZE * window_surface->h / ROW_COUNT
		};
		
		// Data register blitting
		for (int k = 0; k < WORD_LENGTH; k++) {
			// The horizontal position of the cell is determined
			rect.x =
				(k + 1) * window_surface->w / COLUMN_COUNT
				+ (1 - CELL_SIZE) * window_surface->w / (2 * COLUMN_COUNT)
			;
			
			// The cell is drawn
			SDL_FillRect(
				window_surface,
				&rect,
				SDL_MapRGB(
					window_surface->format,
					0,
					0,
					!!(data_register & (1 << (WORD_LENGTH - k - 1))) * 0xbf + 0x40
				)
			);
		}
		
		// Condition code register blitting
		// The horizontal position of the cell is determined
		rect.x =
			10 * window_surface->w / COLUMN_COUNT
			+ (1 - CELL_SIZE) * window_surface->w / (2 * COLUMN_COUNT)
		;
		
		// The cell is drawn
		SDL_FillRect(
			window_surface,
			&rect,
			SDL_MapRGB(
				window_surface->format,
				!data_register * 0xbf + 0x40,
				0,
				0
			)
		);
		
		// Program counter blitting
		for (int k = 0; k < 5; k++) {
			// The horizontal position of the cell is determined
			rect.x =
				(k + 13) * window_surface->w / COLUMN_COUNT
				+ (1 - CELL_SIZE) * window_surface->w / (2 * COLUMN_COUNT)
			;
			
			// The cell is drawn
			SDL_FillRect(
				window_surface,
				&rect,
				SDL_MapRGB(
					window_surface->format,
					0,
					!!(i & (1 << (5 - k - 1))) * 0xbf + 0x40,
					0
				)
			);
		}
		
		SDL_UpdateWindowSurface(window);
		
		// Keyboard input is checked
		const Uint8* keyboard = SDL_GetKeyboardState(NULL);
		
		// The old input needs to end before new input is checked
		while (keyboard[PLAY] || keyboard[STEP]) {
			SDL_PumpEvents();
			keyboard = SDL_GetKeyboardState(NULL);
		}
		
		// If the play mode is active, the next step will be displayed unless stopped
		if (play) {
			SDL_Delay(DELAY);
			SDL_PumpEvents();
			keyboard = SDL_GetKeyboardState(NULL);
			
			if (keyboard[QUIT]) {
				break;
			}
			
			if (keyboard[PLAY]) {
				play = false;
			}
		}
		
		// Else, the system waits for the user to request the next step or enter play mode
		else {
			bool quit = false;
			
			while (true) {
				SDL_PumpEvents();
				keyboard = SDL_GetKeyboardState(NULL);
				
				if (keyboard[QUIT]) {
					quit = true;
					break;
				}
				
				if (keyboard[STEP]) {
					break;
				}
				
				if (keyboard[PLAY]) {
					play = true;
					break;
				}
			}
			
			if (quit) {
				break;
			}
		}
		
		// Waits for further input before shutdown to allow for the final state to be inspected
		if (play && i == 31) {
			while (keyboard[PLAY] || keyboard[STEP]) {
				SDL_PumpEvents();
				keyboard = SDL_GetKeyboardState(NULL);
			}
			
			while (!(keyboard[PLAY] || keyboard[STEP] || keyboard[QUIT])) {
				SDL_PumpEvents();
				keyboard = SDL_GetKeyboardState(NULL);
			}
		}
	}
	
	printf("\n");
	
	// The final state of the main store is displayed
	for (int i = 0; i < MS_SIZE; i++) {
		if (i < 10) {
			printf(" ");
		}
		
		printf("%d: ", i);
		
		for (int j = 7; j > -1; j--) {
			printf("%d", main_store[i] & 1 << j ? 1 : 0);
		}
		
		printf("\n", i);
	}
	
	// The window is destroyed at the program's end
	SDL_DestroyWindow(window);
}

/* CS132 PATP instruction set scripter
   Simulates the running of a PATP program on the architecture
   Labels and comments adjacent to operations in assembly
     source files are not currently supported
   However, due to the source considering all non-defined
     mnemonics to be CLEAR, one could include comments at
	 the end of their program and could even have short
	 comments within the program body (as long as they do
	 not use any other mnemonics that could change the program
	 operation). Program body commenting is not recommended.

   PA2P includes a GUI for PATP and shows the program state in
     real time. Shows the main store (black and white), data
	 regsiter (blue), condition code register (red), and
	 program counter (green).
   PA2P's GUI supports both play and step modes. To play or
     pause use the spacebar, to step through the program use
	 enter, and use escape to quit at any time.
 */
int main(int argc, char* argv[]) {
	// Checks if a script was given
	if (argc < 2) {
		printf("Error: No Script Given\n");
		return EXIT_FAILURE;
	}
	
	// Attempts to open the script file
	FILE* script = fopen(argv[1], "r");
	
	// Checks if the script was opened
	if (!script) {
		printf("Error: Script Not Found\n");
		return EXIT_FAILURE;
	}
	
	unsigned char main_store[MS_SIZE]; // The main store (32 words x 8 bits)
	
	// Loop to initialise the main store
	for (int i = 0; i < MS_SIZE; i++) {
		main_store[i] = 0;
	}
	
	// Loop to load the program into the main store
	for (int i = 0; i < MS_SIZE && !feof(script); i++) {
		// The operation mnemoic is stored
		char operation[MAX_OP + 1];
		fscanf(script, "%5s", operation);
		int op = get_opcode(operation);
		main_store[i] |= op << 5;
		
		if (needs_op(op)) {
			// The operand is stored in the same word
			int operand;
			fscanf(script, "%d", &operand);
			main_store[i] |= operand;
		}
	}
	
	// SDL is initialised
	SDL_Init(SDL_INIT_VIDEO);
	
	// The program is run
	start(main_store);
	
	// SDL is shutdown
	SDL_Quit();
	
	// The program is terminated
	return EXIT_SUCCESS;
}