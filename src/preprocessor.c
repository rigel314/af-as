/*
 * preprocessor.c
 *
 *  Created on: Jan 17, 2014
 *      Author: cody
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "preprocessor.h"
#include "common.h"

char* preprocessFile(char* name)
{
	FILE* fp;
	long fSize;
	char* buf;
	
	fp = fopen(name, "r");
	if(!fp)
		return NULL;
	
	fseek(fp, 0, SEEK_END);
	fSize = ftell(fp);
	rewind(fp);
	
	buf = malloc(fSize+1);
	if(!buf)
	{
		fclose(fp);
		return NULL;
	}
	
	if(fread(buf, fSize, 1, fp) != 1)
	{
		fclose(fp);
		free(buf);
		return NULL;
	}
	
	fclose(fp);
	
	buf[fSize] = '\0';
	
	// Remove comments.
	for(int i = 0; i < strlen(buf); i++)
	{
		if(buf[i] == '#')
			removeComment(buf, i);
	}
	
	// Remove all leading whitespace.
	for(int i = 0; i < strlen(buf)-1; i++)
	{
		bool changeFlag = true;
		while(changeFlag)
		{
			changeFlag = false;
			if(buf[i] == '\n' && (buf[i+1] == ' ' || buf[i+1] == '\t'))
			{
				changeFlag = true;
				strShiftLeft(buf + i + 1);
			}
		}
	}
	
	return buf;
}

void removeComment(char* buf, int i)
{
	int len;
	
	for(len = 0; buf[i+len] != '\n' && buf[i+len]; len++);
	
	for(int j = i; j < strlen(buf); j++)
	{
		buf[j] = buf[j+len];
	}
//	buf[i+len] = '\0';
}
