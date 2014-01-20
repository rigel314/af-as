/*
 * main.c
 *
 *  Created on: Jan 17, 2014
 *      Author: cody
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
	
	if(argc != 2)
	{
		printf("Invalid Arguments!\n");
		return 1;
	}
	
	fp = fopen(argv[1], "r");
	if(!fp)
	{
		printf("Invalid File!\n");
		return 2;
	}
	fclose(fp);
	
	preprocess = preprocessFile(argv[1]);
	
	numLines = structify(preprocess, &lines);
	resolveLabels(lines, numLines);
	assemble(NULL, lines, numLines);
	
	freeLineinfos(lines, numLines);
	free(preprocess);
	return 0;
}
