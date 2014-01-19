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
//	printf("%s\n\n", preprocess);
	
	numLines = structify(preprocess, &lines);
	resolveLabels(lines, numLines);
	assemble(NULL, lines, numLines);
	
//	printf("\nStuff:%d\n", numLines);
//	
//	for(int i=0; i<numLines; i++)
//	{
//		if(lines[i].type != lineType_None)
//		{
//			printf("%d", lines[i].lineNum);
//			if(lines[i].type == lineType_Label)
//			{
//				printf(" %s: %#.4x", lines[i].line.lbl.name, lines[i].address);
//			}
//			else if(lines[i].type == lineType_Instruction)
//			{
//				switch(instructionSet[(int) lines[i].line.inst.instruction].numArgs)
//				{
//					case 0:
//						printf(" %s:", instructionSet[(int) lines[i].line.inst.instruction].mnumonic);
//						break;
//						
//					case 1:
//						if(lines[i].line.inst.type1 == argType_Immediate || lines[i].line.inst.type1 == argType_DerefImmediate)
//							printf(" %s: %#.4x", instructionSet[(int) lines[i].line.inst.instruction].mnumonic, lines[i].line.inst.arg1.val);
//						else
//							printf(" %s: %s", instructionSet[(int) lines[i].line.inst.instruction].mnumonic, regStrings[ lines[i].line.inst.arg1.val].name);
//						break;
//						
//					case 2:
//						printf(" %s: ", instructionSet[(int) lines[i].line.inst.instruction].mnumonic);
//						
//						if(lines[i].line.inst.type1 == argType_Immediate || lines[i].line.inst.type1 == argType_DerefImmediate)
//							printf("%#.4x, ", lines[i].line.inst.arg1.val);
//						else
//							printf("%s, ", regStrings[lines[i].line.inst.arg1.val].name);
//						
//						if(lines[i].line.inst.type2 == argType_Immediate || lines[i].line.inst.type2 == argType_DerefImmediate)
//							printf("%#.4x", lines[i].line.inst.arg2.val);
//						else
//							printf("%s", regStrings[lines[i].line.inst.arg2.val].name);
//						break;
//				}
//			}
//			else
//			{
//				printf(" Error!");
//			}
//			printf("\n");
//		}
//	}
	
	freeLineinfos(lines, numLines);
	free(preprocess);
	return 0;
}
