/*
 * assembler.c
 *
 *  Created on: Jan 17, 2014
 *      Author: cody
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "assembler.h"
#include "common.h"

int structify(char* source, struct lineinfo** lines)
{
	int numLines;
	int pos = 0; // Will be the beginning of each line.
	int addr = 0;
	struct lineinfo* line;
	
	numLines = strchrCount(source, '\n') + 1;
	
	*lines = line = malloc(numLines * sizeof(struct lineinfo));
	if(!*lines)
		return 0;
	
	for(int i=0; i<numLines; i++)
	{
		int len;
		
		for(len = 0; source[pos+len] != '\n' && source[pos+len]; len++);
		len++;
		
		line[i].address = addr;
		line[i].lineNum = i+1;
		line[i].type = lineType_None;
		
		if(len == 1)
		{
			line[i].type = lineType_None;
			pos++;
			continue;
		}
		
		if(strnchr(&source[pos], ':', len)) // Label on this line.
		{
			line[i].type = lineType_Label;
			line[i].line.lbl.name = calloc(len, 1);
			if(line[i].line.lbl.name)
			{
				for(int j=0; ((source[pos+j] | ASCIIshiftBit) >= 'a' && (source[pos+j] | ASCIIshiftBit) <= 'z') ||
							 (source[pos+j] >= '0' && source[pos+j] <= '9' && j>0) ||
							 (source[pos+j] == '_')
							 ; j++)
					line[i].line.lbl.name[j] = source[pos + j];
				
				if(strlen(line[i].line.lbl.name) == 0)
				{
					line[i].type = lineType_Error;
					free(line[i].line.lbl.name);
				}
			}
		}
		else if(source[pos] == '.') // Assembler directive on this line.
		{
			;
		}
		else
		{
			;
		}
		
		pos += len;
	}
	
	return numLines;
}

void freeLineinfos(struct lineinfo* lines, int len)
{
	for(int i=0; i<len; i++)
	{
		if(lines[i].type == lineType_Label)
			free(lines[i].line.lbl.name);
	}
	free(lines);
}
