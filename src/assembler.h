/*
 * assembler.h
 *
 *  Created on: Jan 17, 2014
 *      Author: cody
 */

#ifndef ASSEMBLER_H_
#define ASSEMBLER_H_

enum instArgType { argType_Unused, argType_Immediate, argType_Register };

enum lineType { lineType_None, lineType_Instruction, lineType_Label, lineType_Byte, lineType_Error };

struct label
{
	char* name;
};

struct instruction
{
	int arg1;
	int arg2;
	char instruction[5];
	char type1;
	char type2;
	char width;
};

struct byte
{
	char val;
};

union line
{
	struct label lbl;
	struct instruction inst;
	struct byte byte;
};

struct lineinfo
{
	union line line;
	int address;
	int lineNum;
	char type;
};

int structify(char* source, struct lineinfo** lines);
void freeLineinfos(struct lineinfo* lines, int len);

#endif /* ASSEMBLER_H_ */
