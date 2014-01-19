/*
 * common.c
 *
 *  Created on: Jan 17, 2014
 *      Author: cody
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

void writeInt8(FILE* fp, uint8_t val)
{
	fprintf(fp, "%c", val);
}
void writeInt16(FILE* fp, uint16_t val)
{
	fprintf(fp, "%c%c", (val & 0xFF00) >> 8, val & 0x00FF);
}

void strnprint(char* str, int len)
{
	char* dbg;
	
	dbg = calloc(len+1, 1);
	memcpy(dbg, str, len);
	for(int i=0; i<len; i++)
		if(str[i] == '\n')
			str[i] = '\\';
	printf("%s", dbg);
}

/**
 * int strchrCount(char* s, char c)
 * 	s is a string.
 * 	c is a character to look for.
 * returns number of occurrences of c in s.
 */
int strchrCount(char* s, char c)
{
	int i;
	for (i = 0; s[i]; (s[i] == c) ? (void) i++ : (void) s++); // always increment s or i.  Casts to void to avoid warnings.
	return i;
}

char* strnchr(char* s, char c, int len)
{
	for(int i=0; i<len; i++)
	{
		if(s[i] == c)
			return &s[i];
	}
	return NULL;
}

void strShiftLeft(char* s)
{
	for(int i = 0; i < strlen(s); i++)
		s[i] = s[i+1];
}
