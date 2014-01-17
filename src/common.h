/*
 * common.h
 *
 *  Created on: Jan 17, 2014
 *      Author: cody
 */

#ifndef COMMON_H_
#define COMMON_H_

// ASCII shiftBit for ignoring case.
#define ASCIIshiftBit 0x20

int strchrCount(char* s, char c);
char* strnchr(char* s, char c, int len);
void strShiftLeft(char* s);

#endif /* COMMON_H_ */
