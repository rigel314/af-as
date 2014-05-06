/*
 * main.c
 *
 *  Created on: Jan 17, 2014
 *      Author: cody
 *
 *	TODO:
 *		Check for garbage and invalid characters on label lines.
 *		Detect .org less than current address.
 *		Add way more directives.
 *		Add error messages.
 *		Add listing file.
 *		Add command line switches.
 *			output file
 *			listing file
 *			output type
 *				hex
 *				srec
 *				bin
 *				ELF?
 *		Handle multiple files?
 */

#include <stdio.h>
#include <stdlib.h>

#include "preprocessor.h"
#include "assembler.h"

int main(int argc, char* argv[])
{
	FILE* fp;
	struct lineinfo* lines;
	char* preprocess;
	int numLines;
	
	if(argc != 2) // Simple check for an input filename.
	{
		printf("Invalid Arguments!\n");
		return 1;
	}
	
	fp = fopen(argv[1], "r"); // Try to open the file.
	if(!fp)
	{
		printf("Invalid File!\n"); // If it didn't work, complain about no input.
		return 2;
	}
	fclose(fp);
	
	preprocess = preprocessFile(argv[1]); // Run the preprocessor stage.
	
	numLines = structify(preprocess, &lines); // Run the first pass.  This calculates addresses and instruction widths.
	resolveLabels(lines, numLines); // Run the second pass.  This converts label names to their calculated address from the first pass.
	assemble(NULL, lines, numLines); // Actually output binary code.
	
	// Free everything.
	freeLineinfos(lines, numLines);
	free(preprocess);
	return 0;
}
