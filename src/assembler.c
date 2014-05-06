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
#include <stdbool.h>
#include <stdint.h>

#include "assembler.h"
#include "common.h"

/**
 * This is the instruction set that this assembler understands.
 * To add a new one:
 * 	1. Add a new entry below.
 * 	2. Add a new entry in the same place in the enumeration (instructions) in assembler.h.
 * The fields mean:
 * 	{"mnemonic", strlen(mnemonic), number of args, constraint for arg1, constraint for arg2, Opcode}
 * These opcodes are 6bits.
 */
const struct instructionSetEntry instructionSet[NUM_INSTRUCTIONS] =
{
	{"nop",  3, 0, 0, {0, 0, 0, 0},																0b00000000000000000000000000000000, 0b11111111111111111111111111111111},
	{"halt", 4, 0, 0, {0, 0, 0, 0},																0b00000000000000000000000000000001, 0b11111111111111111111111111111111},
	{"wai",  3, 0, 0, {0, 0, 0, 0},																0b00000000000000000000000000000010, 0b11111111111111111111111111111111},
	{"dd",   2, 2, 0, {target_Any, target_RegisterOrDeref,0 ,0},								0b01000000000000000000000001000000, 0b11111100000000110000111111000000},
	{"add",  3, 2, 1, {target_RegisterOrDeref, target_Any, target_Any, 0},						0b10100000000000000000000000000000, 0b11111000000000110000000000000000},
	{"sub",  3, 2, 1, {target_RegisterOrDeref, target_Any, target_Any, 0},						0b10100000000000010000000000000000, 0b11111000000000110000000000000000},
	{"adc",  3, 2, 1, {target_RegisterOrDeref, target_Any, target_Any, 0},						0b10100000000000100000000000000000, 0b11111000000000110000000000000000},
	{"sbc",  3, 2, 1, {target_RegisterOrDeref, target_Any, target_Any, 0},						0b10100000000000110000000000000000, 0b11111000000000110000000000000000},
	{"mul",  3, 2, 2, {target_RegisterOrDeref, target_RegisterOrDeref, target_Any, target_Any},	0b10000000000000000000000000000000, 0b11111000000000000000000000000000},
	{"imul", 4, 2, 2, {target_RegisterOrDeref, target_RegisterOrDeref, target_Any, target_Any},	0b10001000000000000000000000000000, 0b11111000000000000000000000000000},
	{"div",  3, 2, 2, {target_RegisterOrDeref, target_RegisterOrDeref, target_Any, target_Any},	0b10010000000000000000000000000000, 0b11111000000000000000000000000000},
	{"idiv", 4, 2, 2, {target_RegisterOrDeref, target_RegisterOrDeref, target_Any, target_Any},	0b10011000000000000000000000000000, 0b11111000000000000000000000000000},
	{"and",  3, 2, 1, {target_RegisterOrDeref, target_Any, target_Any, 0},						0b10101000000000000000000000000000, 0b11111000000000110000000000000000},
	{"or",   2, 2, 1, {target_RegisterOrDeref, target_Any, target_Any, 0},						0b10101000000000010000000000000000, 0b11111000000000110000000000000000},
	{"xor",  3, 2, 1, {target_RegisterOrDeref, target_Any, target_Any, 0},						0b10101000000000100000000000000000, 0b11111000000000110000000000000000},
	{"inc",  3, 1, 0, {target_RegisterOrDeref, 0, 0, 0},										0b10111000000000110000000000000000, 0b11111000000000110000111111111111},
	{"dec",  3, 1, 0, {target_RegisterOrDeref, 0, 0, 0},										0b10111000000000110000000000000001, 0b11111000000000110000111111111111},
	{"pop",  3, 1, 1, {target_RegisterOrDeref, target_Any, 0, 0},								0b01000000000000000000000010000000, 0b11111100000000110000111111000000},
	{"push", 4, 1, 1, {target_RegisterOrDeref, target_Any, 0, 0},								0b01000000000000000000000011000000, 0b11111100000000110000111111000000},
	{"shr",  3, 1, 1, {target_RegisterOrDeref, target_Any, 0, 0},								0b10101000000000110000000000000000, 0b11111000000000110000111111000000},
	{"shl",  3, 1, 1, {target_RegisterOrDeref, target_Any, 0, 0},								0b10101000000000110000000001000000, 0b11111000000000110000111111000000},
	{"ishr", 4, 1, 1, {target_RegisterOrDeref, target_Any, 0, 0},								0b10101000000000110000000010000000, 0b11111000000000110000111111000000},
	{"ishl", 4, 1, 1, {target_RegisterOrDeref, target_Any, 0, 0},								0b10101000000000110000000011000000, 0b11111000000000110000111111000000},
	{"ror",  3, 1, 1, {target_RegisterOrDeref, target_Any, 0, 0},								0b10101000000000110000000100000000, 0b11111000000000110000111111000000},
	{"rol",  3, 1, 1, {target_RegisterOrDeref, target_Any, 0, 0},								0b10101000000000110000000101000000, 0b11111000000000110000111111000000},
	{"rrc",  3, 1, 1, {target_RegisterOrDeref, target_Any, 0, 0},								0b10101000000000110000000110000000, 0b11111000000000110000111111000000},
	{"rlc",  3, 1, 1, {target_RegisterOrDeref, target_Any, 0, 0},								0b10101000000000110000000111000000, 0b11111000000000110000111111000000},
	{"call", 4, 1, 1, {target_RegisterOrDeref, target_Any, 0, 0},								0b00100000000000000000000000000000, 0b11111100000000110000111111000000},
	{"bcda", 4, 1, 1, {target_RegisterOrDeref, target_Any, 0, 0},								0b10101000000000110000001000000000, 0b11111000000000110000111111000000},
};

/**
 * This lists the registers that this assembler understands.
 * To add a new one:
 * 	1. Add a new entry below.
 * 	2. Add a new entry in the same place in the enumeration (registers) in assembler.h.
 * The fields mean:
 * 	{"regName", strlen(regName), Opcode}
 * These opcodes are 6bits because the high bit signifies that it should be dereferenced.
 */
const struct validRegs regStrings[NUM_ALLREGISTERS+2] =
{
	{"r0", 2, 0b000001},
	{"r1", 2, 0b000010},
	{"r2", 2, 0b000011},
	{"r3", 2, 0b000100},
	{"r4", 2, 0b000101},
	{"r5", 2, 0b000110},
	{"r6", 2, 0b000111},
	{"r7", 2, 0b001000},
	{"r8", 2, 0b001001},
	{"r9", 2, 0b001010},
	{"", 0, 0},
	{"pc", 2, 0b011100},
	{"sp", 2, 0b011101},
	{"fl", 2, 0b011110},
	{"", 0, 0},
	{"",   0, 0b011111} // Signifies immediate value.
};

/**
 * This lists the conditions that this assembler understands.
 * To add a new one:
 * 	1. Add a new entry below.
 * 	2. Add a new entry in the same place in the enumeration (registers) in assembler.h.
 * The fields mean:
 * 	{"condName", strlen(condName), Opcode}
 * These opcodes are 4bits.
 */
const struct validRegs condStrings[NUM_CONDITIONS] =
{
	{"al", 2, 0b00000},
	{"nv", 2, 0b00001},
	{"z",  1, 0b00010},
	{"nz", 2, 0b00011},
	{"c",  1, 0b00100},
	{"nc", 2, 0b00101},
	{"v",  1, 0b00110},
	{"nv", 2, 0b00111},
	{"n",  1, 0b01000},
	{"nn", 2, 0b01001},
	{"ge", 2, 0b01010},
	{"gt", 2, 0b01011},
	{"le", 2, 0b01100},
	{"lt", 2, 0b01101},
	{"hs", 2, 0b01110},
	{"hi", 2, 0b01111},
	{"ls", 2, 0b00000}, // TODO: Look this up.
	{"lo", 2, 0b00000}, // TODO: Look this up.
	{"eq", 2, 0b00010},
	{"ne", 2, 0b00011},
};

/**
 * void resolveLabels(struct lineinfo* lines, int len)
 * 	lines is the struct returned by the first pass. (structify())
 * 	len is the length of the lines array.
 * Second pass.  Converts labels used as arguments to addresses.
 */
void resolveLabels(struct lineinfo* lines, int len)
{
	for(int i=0; i<len; i++)
	{
		if(lines[i].type != lineType_Instruction) // The only thing that matters is instructions at this point.
			continue;
		
		for(int j=0; j<4; j++)
			if(lines[i].line.inst.types[j] == argType_Label || lines[i].line.inst.types[j] == argType_DerefLabel)
			{ // If the first argument is a label
				char* arglbl = lines[i].line.inst.args[j].name; // Backup the name so it can be free()ed later.
				
				// Set the type correct.  Deref types are always (non-deref type ORed with 0x01).  argType_Deref(.*) == (argType_\1 | 1)
				lines[i].line.inst.types[j] = argType_Immediate | (0x01 & lines[i].line.inst.types[j]); // enum order: instArgType
				lines[i].line.inst.args[j].val = findLabel(arglbl, lines, len);
				
				free(arglbl);
				
				if(lines[i].line.inst.args[j].val == -1) // If findLabel didn't find anything, signal an error.
					lines[i].type = lineType_Error;
			}
	}
}

/**
 * int findLabel(char* name, struct lineinfo* lines, int len)
 * 	name is the label to look for.
 * 	lines is the struct returned by the first pass. (structify())
 * 	len is the length of the lines array.
 * Returns the address of the label name, or -1 if it is not found.
 */
int findLabel(char* name, struct lineinfo* lines, int len)
{
	for(int i=0; i<len; i++)
	{
		if(lines[i].type != lineType_Label) // Only look for labels.
			continue;
		if(!strcmp(lines[i].line.lbl.name, name)) // If found, give the address.
			return lines[i].address;
	}
		
	return -1; // Not found.
}

/**
 * void assemble(char* file, struct lineinfo* lines, int len)
 * 	file is the file name of the output file.
 * 	lines is the struct returned by the first pass. (structify())
 * 	len  is the length of the lines array.
 * Assembles then writes assembled binary code to file.  If file can't be opened, it uses stdout.  Also prints errors.
 */
void assemble(char* file, struct lineinfo* lines, int len)
{
	int addr = 0;
	FILE* fp = fopen(file, "w"); // Try to open file.
	if(!fp)
		fp = stdout; // Use stdout as a fallback.
	
	for(int i=0; i<len; i++)
	{
		int inst;
		uint32_t word = 0;
		
		if(lines[i].type == lineType_Error) // If there was an error, print it.
			fprintf(stderr, "Line %d - Error!\n", lines[i].lineNum);
		if(lines[i].type != lineType_Instruction) // Instructions are the only other important line type.
			continue;							  // Byte lines are important too, but there's no way to generate them. '.db' and '.dw' are not implemented.
		
//		if(!addr) // Don't write extra bytes for first address.
//			addr = lines[i].address;
		if(addr != lines[i].address)
		{ // If there was a .org directive that changed the address, write a bunch of zeroes.
			for(; addr<lines[i].address; addr++)
				writeInt8(fp, 0);
		}
		addr += lines[i].width; // Always increment addr by the width of this instruction.
		
		inst = lines[i].line.inst.instruction; // Simplify some array indexing.
		
		word = instructionSet[inst].bitField;
		
		// Or the targets into place.
		for(int j=0; j < lines[i].line.inst.numArgsUsed; j++)
		{
			int shift = 0;
			switch(j)
			{
				case 0:
					if(lines[i].line.inst.numArgsUsed == 2)
						shift = 6;
					if(lines[i].line.inst.numArgsUsed > 2)
						shift = 18;
					break;
				
				case 1:
					if(lines[i].line.inst.numArgsUsed < 4)
						shift = 6;
					if(lines[i].line.inst.numArgsUsed == 4)
						shift = 12;
					break;
				
				case 2:
					if(lines[i].line.inst.numArgsUsed == 4)
						shift = 6;
					break;
			}
			
			if(lines[i].line.inst.types[j] == argType_DerefRegister || lines[i].line.inst.types[j] == argType_Register)
			{
				int extrabit = 0;
				
				if(lines[i].line.inst.types[j] == argType_DerefRegister)
					extrabit = TargetDerefBit;
				word |= (condStrings[lines[i].line.inst.args[j].val].bitField | extrabit) << shift;
			}
			else if(lines[i].line.inst.types[j] == argType_DerefImmediate || lines[i].line.inst.types[j] == argType_Immediate)
			{
				int extrabit = 0;
				
				if(lines[i].line.inst.types[j] == argType_DerefImmediate)
					extrabit = TargetDerefBit;
				word |= (condStrings[reg_immediate].bitField | extrabit) << shift;
			}
		}
		
		// Or the 'mutate flags' flag into place;
		if(!(instructionSet[inst].bitMask & 0b00000100000000000000000000000000) && !lines[i].line.inst.mutateFlags)
			word |= 0b00000100000000000000000000000000;
		
		// Or the conditions into place.
		if(!(instructionSet[inst].bitMask & 0b00000000000000001111000000000000) && inst < inst_mul && inst > inst_idiv)
			word |= condStrings[lines[i].line.inst.condition].bitField << 12;
		
		// TODO: implement relative call and bitwise arg notting.
		
		writeInt32(fp, word); // Write the instruction.
		
		// Write bytes for immediate values.
		for(int j=0; j < lines[i].line.inst.numArgsUsed; j++)
		{
			if(lines[i].line.inst.types[j] == argType_Immediate)
				switch(lines[i].line.inst.width)
				{
					case instWidth_8:
						writeInt8(fp, lines[i].line.inst.args[j].val & 0xFF);
						break;
						
					case instWidth_16:
						writeInt16(fp, lines[i].line.inst.args[j].val & 0xFFFF);
						break;
						
					case instWidth_32:
						writeInt32(fp, lines[i].line.inst.args[j].val & 0xFFFFFFFF);
						break;
				}
			if(lines[i].line.inst.types[j] == argType_DerefImmediate)
				writeInt32(fp, lines[i].line.inst.args[j].val);
		}
		
//		if(lines[i].line.inst.type1 == argType_DerefRegister || lines[i].line.inst.type1 == argType_Register)
//		{ // If there's a register, shift the register opcode to the right places.
//			target1 = regStrings[lines[i].line.inst.arg1.val].bitField;
//			if(lines[i].line.inst.type1 == argType_DerefRegister)
//				target1 |= TargetDerefBit; // Get the deref bit set properly.
//			word |= (target1 << 5);
//		}
//		else if(lines[i].line.inst.type1 == argType_DerefImmediate || lines[i].line.inst.type1 == argType_Immediate)
//		{ // Do the same for immediate values.
//			target1 = regStrings[reg_immediate].bitField;
//			if(lines[i].line.inst.type1 == argType_DerefImmediate)
//				target1 |= TargetDerefBit;
//			word |= (target1 << 5);
//		}
//		if(lines[i].line.inst.type2 == argType_DerefRegister || lines[i].line.inst.type2 == argType_Register)
//		{ // Same for the second arg.
//			target2 = regStrings[lines[i].line.inst.arg2.val].bitField;
//			if(lines[i].line.inst.type2 == argType_DerefRegister)
//				target2 |= TargetDerefBit;
//			word |= target2;
//		}
//		else if(lines[i].line.inst.type2 == argType_DerefImmediate || lines[i].line.inst.type2 == argType_Immediate)
//		{ // Same for the second arg.
//			target2 = regStrings[reg_immediate].bitField;
//			if(lines[i].line.inst.type2 == argType_DerefImmediate)
//				target2 |= TargetDerefBit;
//			word |= target2;
//		}
//		
//		writeInt16(fp, word); // Write the instruction.
//		
//		// Write the bytes for immediate values.
//		if((target1 | TargetDerefBit) == (regStrings[reg_immediate].bitField | TargetDerefBit))
//			writeInt16(fp, lines[i].line.inst.arg1.val);
//		if((target2 | TargetDerefBit) == (regStrings[reg_immediate].bitField | TargetDerefBit))
//			writeInt16(fp, lines[i].line.inst.arg1.val);
	}
	
	if(fp != stdout) // Keep everything clean.
		fclose(fp);
}

/**
 * bool isValidType(char type, char constraint)
 * 	type is the argType from the return of getArgAndType().
 * 	constraint is an argument flag from the instruction set struct.
 * Returns true only if type is allowed by constraint.
 */
bool isValidType(char type, char constraint)
{
	if(constraint == target_None && type != argType_Unused)
		return false;
	if(constraint == target_Register_Only && type != argType_Register)
		return false;
	if(constraint == target_Immediate_Only && type != argType_Immediate && type != argType_Label)
		return false;
	if(constraint == target_RegisterOrDeref && type != argType_DerefImmediate && type != argType_DerefLabel && type != argType_DerefRegister && type != argType_Register)
		return false;
	
	return true;
}

/**
 * int structify(char* source, struct lineinfo** lines)
 * 	source is the preprocessed source.
 * 	lines is a pointer to an array of structs that will hold information about each line in the source.
 * Reads preprocessed source and parses it into a nice struct.
 * Returns number of lines.
 */
int structify(char* source, struct lineinfo** lines)
{
	int numLines;
	int pos = 0; // Will be the beginning of each line.
	int addr = 0;
	struct lineinfo* line;
	
	numLines = strchrCount(source, '\n') + 1; // Number of newlines is the number of lines.
	
	*lines = line = calloc(numLines, sizeof(struct lineinfo)); // Allocate the memory.
	if(!*lines)
		return 0;
	
	for(int i=0; i<numLines; i++)
	{
		int len;
		
		for(len = 0; source[pos+len] != '\n' && source[pos+len]; len++); // Calculate the length of each line.
		len++;
		
		// Store everything we know and any defaults.
		line[i].address = addr;
		line[i].lineNum = i+1;
		line[i].type = lineType_None;
		
		if(len == 1)
		{ // Empty line.
			line[i].type = lineType_None;
			pos++;
			continue;
		}
		
		if(strnchr(&source[pos], ':', len)) // Label on this line.
		{
			line[i].type = lineType_Label;
			line[i].line.lbl.name = calloc(len, 1); // Store the label.
			if(line[i].line.lbl.name)
			{
				for(int j=0; ((source[pos+j] | ASCIIshiftBit) >= 'a' && (source[pos+j] | ASCIIshiftBit) <= 'z') || // [a-zA-Z]
							 (source[pos+j] >= '0' && source[pos+j] <= '9' && j>0) || // [0-9]
							 (source[pos+j] == '_') // _ is valid in a label name.
							 ; j++)
					line[i].line.lbl.name[j] = source[pos + j]; // Actually copy the label.
				
				if(strlen(line[i].line.lbl.name) == 0)
				{ // Invalid label detected.
					line[i].type = lineType_Error;
					free(line[i].line.lbl.name);
				}
			}
		}
		else if(source[pos] == '.') // Assembler directive on this line.
		{
			switch(getDirective(&source[pos], len))
			{
				int val;
				
				case directive_org:  // Change the address.
					if(sscanf(&source[pos], ".org %i", &val) == 1) // Grab the value.
						addr = val;
					else
						line[i].type = lineType_Error; // sscanf() failed.
					break;
			}
		}
		else // Must be an instruction.
		{
			line[i].type = lineType_Instruction;
			line[i].line.inst.mutateFlags = true;
			line[i].line.inst.condition = cond_always;
			line[i].line.inst.width = instWidth_32;
			for(int j=0; j<4; j++)
				line[i].line.inst.types[j] = argType_Unused;
			
			line[i].line.inst.instruction = getInstruction(&source[pos], len);
			
			if(line[i].line.inst.instruction == NUM_INSTRUCTIONS)
			{ // Instruction not found.
				line[i].width = 0;
				line[i].type = lineType_Error;
			}
			else
			{
				int cmdLen;
				int j = 0;
				char* endOfPrevArg = NULL;
				char* endOfThisArg = strnchr(&source[pos], ',', len);
				int argLength = 0;
				
				
				// Getting flags, conditions, and width.
				cmdLen = instructionSet[(int) line[i].line.inst.instruction].mnemonicLen;
				
				if(source[pos] == '!')
				{
					cmdLen++;
					line[i].line.inst.mutateFlags = false;
				}
				if(source[pos+cmdLen] == '{')
				{
					int cond = getCondition(&source[pos+cmdLen+1], len-cmdLen-1);
					if(cond == NUM_CONDITIONS)
					{
						line[i].type = lineType_Error;
					}
					else
					{
						line[i].line.inst.condition = cond;
						cmdLen += condStrings[cond].len + 1;
					}
					
					if(source[pos+cmdLen] != '}')
					{
						line[i].type = lineType_Error;
						cmdLen -= 1;
					}
					else
					{
						cmdLen++;
					}
				}
				if(source[pos+cmdLen] == '.')
				{
					int wid = getDataWidth(&source[pos+cmdLen+1], len-cmdLen-1);
					if(wid != instWidth_8 && wid != instWidth_16 && wid != instWidth_32)
					{
						line[i].type = lineType_Error;
					}
					else
					{
						line[i].line.inst.width = wid;
						cmdLen += ((wid==instWidth_8)? 2 : 3);
					}
				}
				
				// Getting instruction arguments.
				endOfPrevArg = &source[pos+cmdLen];
				
				if(endOfThisArg == NULL)
					argLength = len-cmdLen-1;
				else
					argLength = endOfThisArg - endOfPrevArg;
				
				// Some complicated magic to get an input file parsed properly.
				while(getArgAndType(endOfPrevArg+1, argLength-1, &line[i].line.inst.args[j], &line[i].line.inst.types[j]) != argType_Bad && ++j<4 && endOfThisArg != NULL)
				{
					endOfPrevArg = endOfThisArg;
					
					endOfThisArg = strnchr(endOfPrevArg+1, ',', argLength-1);
					if(endOfThisArg == NULL)
						argLength = len - (endOfPrevArg-&source[pos]) - 1;
					else
						argLength = endOfThisArg - endOfPrevArg;
				}
				
				if(j < instructionSet[(int) line[i].line.inst.instruction].numArgs)
					line[i].type = lineType_Error;
				if(j > instructionSet[(int) line[i].line.inst.instruction].numArgs + instructionSet[(int) line[i].line.inst.instruction].numOptArgs)
					line[i].type = lineType_Error;
				
				line[i].line.inst.numArgsUsed = j;
				
				// Argument type checking.
				for(int k=0; k<j; k++)
					if(!isValidType(line[i].line.inst.types[k], instructionSet[(int) line[i].line.inst.instruction].argFlags[k]))
						line[i].type = lineType_Error;
				
				line[i].width = 4;
				
				for(int j=0; j<4; j++)
				{
					if((line[i].line.inst.types[j]) == argType_Immediate || (line[i].line.inst.types[j]) == argType_Label)
						switch(line[i].line.inst.width)
						{
							case instWidth_8:
								line[i].width += 1;
								break;
							case instWidth_16:
								line[i].width += 2;
								break;
							case instWidth_32:
								line[i].width += 4;
								break;
						}
					if((line[i].line.inst.types[j]) == argType_DerefImmediate || (line[i].line.inst.types[j]) == argType_DerefLabel)
						line[i].width += 4;
				}
			}
			addr += line[i].width; // Always increment addr by instruction width.
		}
		
		pos += len; // Always increment pos by line length to keep &source[pos] pointed at the beginning of a line.
	}
	
	return numLines;
}

/**
 * int getArgAndType(char* str, int len, union arg* val, char* type)
 * 	str is the string to parse.
 * 	len is the farthest to index in str.
 * 	val is a place to store the detected value.
 * 	type is a place to store the detected type.
 * Parses a substring for a value and type. Returns type.
 */
int getArgAndType(char* str, int len, union arg* val, char* type)
{
	int parenFlag = 0;
	bool valFlag = false;
	int retval = argType_Bad;
	
	*type = argType_Bad;
	
	for(int i=0; i<len; i++)
	{
		if((str[i] == ' ' || str[i] == '\t')) // Any meaningless whitespace.
			continue;
		if(parenFlag==2) // Any non-whitespace after ')'.
			return argType_Bad;
		if(parenFlag && str[i] == '(') // More than one '('.
			return argType_Bad;
		
		if(valFlag && str[i] != ')') // The only thing you can have after finding a value is whitespace or ')'.
			return argType_Bad;
		
		// Handle parentheses under normal circumstances.
		if(str[i] == '(')
			parenFlag++;
		if(parenFlag && str[i] == ')')
			parenFlag++;
		
		// Possibly a label.
		if(!valFlag && (((str[i] | ASCIIshiftBit) >= 'a' && (str[i] | ASCIIshiftBit) <= 'z') || str[i] == '_'))
		{
			int j = 0;
			int toklen = 0;
			
			for(int k=i; k<len && str[k]!=' ' && str[k]!='\t' && str[k]!=')'; k++, toklen++); // Calculate the length of this token.
			
			// Check if the token found is a reserved word (i.e. a register).
			while(strncasecmp(&str[i], regStrings[j].name, toklen) && ++j < NUM_ALLREGISTERS);
			
			if(j == NUM_REGISTERS)
				return argType_Bad; // Error, token was an empty string. Can't happen.
			
			if(j == NUM_ALLREGISTERS) // Good, wasn't a register. It will be treated as a label.
			{
				valFlag = true;
				retval = argType_Label;
				
				val->name = calloc(len+1, 1); // Allocate space for copying the label.
				
				// Copy the label again.
				for(int j=0; ((str[i] | ASCIIshiftBit) >= 'a' && (str[i] | ASCIIshiftBit) <= 'z') ||
							 (str[i] >= '0' && str[i] <= '9' && i>0) ||
							 (str[i] == '_')
							 ; i++)
					val->name[j++] = str[i];  // skip to end of label, copying as we go.
				i--; // Always i-- to not skip bytes.
			}
		}
		
		// Special registers.
		if(!valFlag && str[i] == 'p' && str[i+1] == 'c') // Found pc.
		{
			valFlag = true;
			retval = argType_Register;
			val->val = reg_pc;
			i++;
		}
		if(!valFlag && str[i] == 's' && str[i+1] == 'p') // Found sp.
		{
			valFlag = true;
			retval = argType_Register;
			val->val = reg_sp;
			i++;
		}
		if(!valFlag && str[i] == 'f' && str[i+1] == 'l') // Found fl.
		{
			valFlag = true;
			retval = argType_Register;
			val->val = reg_fl;
			i++;
		}
		
		// Numbered registers.
		if(!valFlag && str[i] == 'r') // Found a register.
		{
			retval = argType_Register;
			
			if(sscanf(&str[i], "r%u", &val->val) == 1) // Sets val to register number. reg_r0 will have value zero from enum, reg_r1 will have 1, and so on.
				valFlag = true;
			else
				return argType_Bad; // sscanf() failed.
			
			if(val->val >= NUM_REGISTERS)
				return argType_Bad;
			
			for(i++; i<len && str[i] >= '0' && str[i] <= '9'; i++); // skip to end of register.
			i--; // Always i-- to not skip bytes.
		}
		
		// Immediate values.
		if(!valFlag && str[i] >= '0' && str[i] <= '9')
		{
			retval = argType_Immediate;
			
			if(sscanf(&str[i], "%u", &val->val) == 1) // Sets val to the immediate value found.
				valFlag = true;
			else
				return argType_Bad; // sscanf() failed.
			
			if(val->val > 0xFFFF || val->val < 0) // Out of range.
				return argType_Bad;
			
			for(i++; i<len && str[i] >= '0' && str[i] <= '9'; i++); // skip to end of number.
			i--; // Always i-- to not skip bytes.
		}
	}
	
	if(!valFlag || parenFlag==1)
		return argType_Bad; // No value found or incomplete parentheses.
	
	if(parenFlag == 2) // Deref types are always 1 + non-deref types.
		retval++; // enum order: instArgType
	
	*type = retval; // Finally set detected type.
	
	return retval;
}

/**
 * int getInstruction(char* str, int len)
 * 	str is the string to check.
 * 	len is the farthest to index str.
 * Returns the detected instruction in str. Returns NUM_INSTRUCTIONS on error.
 */
int getInstruction(char* str, int len)
{
	int i = 0;
	
	if(str[0] == '!')
		str++;
	
	// Iterate through the instruction set looking for matches.
	while(strncasecmp(str, instructionSet[i].mnemonic, instructionSet[i].mnemonicLen) && ++i < NUM_INSTRUCTIONS);
	
	return i;
}

/**
 * int getInstruction(char* str, int len)
 * 	str is the string to check.
 * 	len is the farthest to index str.
 * Returns the detected condition in str. Returns NUM_CONDITIONS on error.
 */
int getCondition(char* str, int len)
{
	int i = 0;
	
	// Iterate through the valid conditions looking for matches.
	while(strncasecmp(str, condStrings[i].name, condStrings[i].len) && ++i < NUM_INSTRUCTIONS);
	
	return i;
}

/**
 * int getInstruction(char* str, int len)
 * 	str is the string to check.
 * 	len is the farthest to index str.
 * Returns the detected condition in str. Returns NUM_CONDITIONS on error.
 */
int getDataWidth(char* str, int len)
{
	switch(str[0])
	{
		case '8':
			return instWidth_8;
			break;
		case '1':
			if(str[1] == '6')
				return instWidth_16;
			break;
		case '3':
			if(str[1] == '2')
				return instWidth_32;
			break;
	}

	return 0;
}

/**
 * int getDirective(char* str, int len)
 * 	str is the string to check.
 * 	len is the farthest to index str.
 * Returns the detected directive in str. Returns NUM_DIRECTIVES on error.
 */
int getDirective(char* str, int len)
{
	// Compare against each "known" directive.
	
	if(!strncasecmp(str, ".org", 4))
		return directive_org;
	
	if(!strncasecmp(str, ".equ", 4))
		return directive_equ;
	
	if(!strncasecmp(str, ".db", 3) || !strncmp(str, ".byte", 5))
		return directive_db;
	
	if(!strncasecmp(str, ".dw", 3) || !strncmp(str, ".word", 5))
		return directive_dw;
	
	if(!strncasecmp(str, ".fill", 5))
		return directive_fill;

	if(!strncasecmp(str, ".text", 5))
		return directive_text;

	return NUM_DIRECTIVES;
}

/**
 * void freeLineinfos(struct lineinfo* lines, int len)
 * 	lines is the struct returned by the first pass. (structify())
 * 	len is the length of the lines array.
 * free()s all malloc()ed data from lines.
 */
void freeLineinfos(struct lineinfo* lines, int len)
{
	for(int i=0; i<len; i++)
	{
		if(lines[i].type == lineType_Label)
			free(lines[i].line.lbl.name); // Only free what is necessary.
		if(lines[i].type == lineType_Byte)
			free(lines[i].line.byte.vals);
	}
	free(lines);
}
