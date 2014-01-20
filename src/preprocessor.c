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

/**
 * char* preprocessFile(char* name)
 * 	name is a filename to open and preprocess.
 * Returns a string that is a preprocessed version of the file name.
 */
char* preprocessFile(char* name)
{
	FILE* fp;
	long fSize;
	char* buf;
	
	fp = fopen(name, "r"); // Opening the file.
	if(!fp)
		return NULL;
	
	// Reading the entire file into memory.
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
	// Done reading file.
	
	buf[fSize] = '\0';// Null terminate the new string.
	
	// Remove comments.
	for(int i = 0; i < strlen(buf); i++)
	{
		if(buf[i] == '#') // Comments start with # and go until \n.
			removeComment(buf, i);
	}
	
	// Remove all leading whitespace.
	for(int i = 0; i < strlen(buf)-1; i++)
	{
		bool changeFlag = true;
		while(changeFlag) // Keep removing whitespace following \n until there is no change.
		{
			changeFlag = false;
			if(buf[i] == '\n' && (buf[i+1] == ' ' || buf[i+1] == '\t'))
			{
				changeFlag = true;
				strShiftLeft(buf + i + 1); // Remove a character.
			}
		}
	}
	
	return buf;
}

/**
 * void removeComment(char* buf, int i)
 * 	buf is a string to remove a comment from.
 * 	i is the index in buf at the beginning of the comment.
 * Removes all characters from buf[i] to the next newline or the first '\0'.
 */
void removeComment(char* buf, int i)
{
	int len;
	
	for(len = 0; buf[i+len] != '\n' && buf[i+len]; len++); // Find the length of the comment.
	
	for(int j = i; j < strlen(buf); j++)
	{
		buf[j] = buf[j+len]; // Shift the string left one byte.
	}
//	buf[i+len] = '\0';
}
