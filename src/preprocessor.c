/*
 * preprocessor.c
 *
 *  Created on: Jan 17, 2014
 *      Author: cody
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "preprocessor.h"

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
	
	for(int i = 0; i < strlen(buf); i++)
	{
		if(buf[i] == '#')
			removeComment(buf, i);
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
