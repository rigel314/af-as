/*
 * common.h
 *
 *  Created on: Jan 17, 2014
 *      Author: cody
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <stdint.h>

// ASCII shiftBit for ignoring case.
#define ASCIIshiftBit 0x20

// Usual min and max macros.
#define min(x, y) ((x)<(y)?(x):(y))
#define max(x, y) ((x)>(y)?(x):(y))

void writeInt8(FILE* fp, uint8_t val);
void writeInt16(FILE* fp, uint16_t val);
void writeInt32(FILE* fp, uint32_t val);
void strnprint(char* str, int len);
int strchrCount(char* s, char c);
char* strnchr(char* s, char c, int len);
void strShiftLeft(char* s);

#endif /* COMMON_H_ */
