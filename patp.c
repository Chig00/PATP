#include <stdio.h> // Standard and File IO
#include <stdlib.h> // Memory Allocation and Exit Codes
#include <stdbool.h> // Boolean Types
#include <string.h> // String Comparison

const int MS_SIZE = 32; // The number of words in the main store
const int MAX_OP = 5; // The maximum operation mnemonic length

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
	
	// The program is run
	start(main_store);
	
	return EXIT_SUCCESS;
}