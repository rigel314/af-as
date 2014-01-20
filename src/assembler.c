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
 * These opcodes are 6bits because the other 10 are split 5 each to describe each argument.
 */
const struct instructionSetEntry instructionSet[NUM_INSTRUCTIONS] =
{
	{"nop",  3, 0, target_None, target_None,			0b000000},
	{"dd",   2, 2, target_Any, target_RegisterOrDeref,	0b000001},
	{"add",  3, 2, target_RegisterOrDeref, target_Any,	0b000010},
	{"sub",  3, 2, target_RegisterOrDeref, target_Any,	0b000011},
	{"mul",  3, 2, target_RegisterOrDeref, target_Any,	0b000100},
	{"div",  3, 2, target_RegisterOrDeref, target_Any,	0b000101},
	{"and",  3, 2, target_RegisterOrDeref, target_Any,	0b000110},
	{"or",   2, 2, target_RegisterOrDeref, target_Any,	0b000111},
	{"xor",  3, 2, target_RegisterOrDeref, target_Any,	0b001000},
	{"inc",  3, 1, target_RegisterOrDeref, target_None,	0b001001},
	{"dec",  3, 1, target_RegisterOrDeref, target_None,	0b001010},
	{"pop",  3, 1, target_RegisterOrDeref, target_None,	0b001011},
	{"push", 4, 1, target_Any, target_None,				0b001100},
	{"shr",  3, 1, target_Any, target_None,				0b001101},
	{"shl",  3, 1, target_Any, target_None,				0b001110},
	{"shcr", 4, 1, target_Any, target_None,				0b001111},
	{"shcl", 4, 1, target_Any, target_None,				0b010000},
	{"ror",  3, 1, target_Any, target_None,				0b010001},
	{"rol",  3, 1, target_Any, target_None,				0b010010},
	{"rocr", 4, 1, target_Any, target_None,				0b010011},
	{"rocl", 4, 1, target_Any, target_None,				0b010100},
	{"halt", 4, 0, target_None, target_None,			0b010101},
};

/**
 * This lists the registers that this assembler understands.
 * To add a new one:
 * 	1. Add a new entry below.
 * 	2. Add a new entry in the same place in the enumeration (registers) in assembler.h.
 * The fields mean:
 * 	{"regName", strlen(regName), Opcode}
 * These opcodes are 5bits because the high bit signifies that it should be dereferenced.
 */
const struct validRegs regStrings[NUM_ALLREGISTERS+2] =
{
	{"r0", 2, 0b00000},
	{"r1", 2, 0b00001},
	{"r2", 2, 0b00010},
	{"r3", 2, 0b00011},
	{"", 0, 0},
	{"pc", 2, 0b01101},
	{"sp", 2, 0b01110},
	{"", 0, 0},
	{"",   0, 0b01111} // Signifies immediate value.
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
		
		if(lines[i].line.inst.type1 == argType_Label || lines[i].line.inst.type1 == argType_DerefLabel)
		{ // If the first argument is a label
			char* arglbl = lines[i].line.inst.arg1.name; // Backup the name so it can be free()ed later.
			
			// Set the type correct.  Deref types are always (non-deref type ORed with 0x01).  argType_Deref(.*) == (argType_\1 | 1)
			lines[i].line.inst.type1 = argType_Immediate | (0x01 & lines[i].line.inst.type1); // enum order: instArgType
			lines[i].line.inst.arg1.val = findLabel(arglbl, lines, len);
			
			free(arglbl);
			
			if(lines[i].line.inst.arg1.val == -1) // If findLabel didn't find anything, signal an error.
				lines[i].type = lineType_Error;
		}
		if(lines[i].line.inst.type2 == argType_Label || lines[i].line.inst.type2 == argType_DerefLabel)
		{ // If the second argument is a label
			char* arglbl = lines[i].line.inst.arg1.name;
			
			lines[i].line.inst.type2 = argType_Immediate | (0x01 & lines[i].line.inst.type2); // enum order: instArgType
			lines[i].line.inst.arg2.val = findLabel(lines[i].line.inst.arg2.name, lines, len);
			
			free(arglbl);
			
			if(lines[i].line.inst.arg2.val == -1)
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
		uint16_t word = 0;
		char target1 = -1;
		char target2 = -1;
		
		if(lines[i].type == lineType_Error) // If there was an error, print it.
			fprintf(stderr, "Line %d - Error!\n", lines[i].lineNum);
		if(lines[i].type != lineType_Instruction) // Instructions are the only other important line type.
			continue;
		
		if(!addr) // Don't write extra bytes for first address.
			addr = lines[i].address;
		if(addr != lines[i].address)
		{ // If there was a .org directive that changed the address, write a bunch of zeroes.
			for(; addr<lines[i].address; addr++)
				writeInt8(fp, 0);
		}
		addr += lines[i].width; // Always increment addr by the width of this instruction.
		
		inst = lines[i].line.inst.instruction; // Simplify some array indexing.
		
		word = instructionSet[inst].bitField << 10; // Shift the opcode for the mnemonic to the right place.
		if(lines[i].line.inst.type1 == argType_DerefRegister || lines[i].line.inst.type1 == argType_Register)
		{ // If there's a register, shift the register opcode to the right places.
			target1 = regStrings[lines[i].line.inst.arg1.val].bitField;
			if(lines[i].line.inst.type1 == argType_DerefRegister)
				target1 |= TargetDerefBit; // Get the deref bit set properly.
			word |= (target1 << 5);
		}
		else if(lines[i].line.inst.type1 == argType_DerefImmediate || lines[i].line.inst.type1 == argType_Immediate)
		{ // Do the same for immediate values.
			target1 = regStrings[reg_immediate].bitField;
			if(lines[i].line.inst.type1 == argType_DerefImmediate)
				target1 |= TargetDerefBit;
			word |= (target1 << 5);
		}
		if(lines[i].line.inst.type2 == argType_DerefRegister || lines[i].line.inst.type2 == argType_Register)
		{ // Same for the second arg.
			target2 = regStrings[lines[i].line.inst.arg2.val].bitField;
			if(lines[i].line.inst.type2 == argType_DerefRegister)
				target2 |= TargetDerefBit;
			word |= target2;
		}
		else if(lines[i].line.inst.type2 == argType_DerefImmediate || lines[i].line.inst.type2 == argType_Immediate)
		{ // Same for the second arg.
			target2 = regStrings[reg_immediate].bitField;
			if(lines[i].line.inst.type2 == argType_DerefImmediate)
				target2 |= TargetDerefBit;
			word |= target2;
		}
		
		writeInt16(fp, word); // Write the instruction.
		
		// Write the bytes for immediate values.
		if((target1 | TargetDerefBit) == (regStrings[reg_immediate].bitField | TargetDerefBit))
			writeInt16(fp, lines[i].line.inst.arg1.val);
		if((target2 | TargetDerefBit) == (regStrings[reg_immediate].bitField | TargetDerefBit))
			writeInt16(fp, lines[i].line.inst.arg1.val);
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
			int cmdLen;
			
			line[i].type = lineType_Instruction;
			
			line[i].line.inst.instruction = getInstruction(&source[pos], len);
			if(line[i].line.inst.instruction == NUM_INSTRUCTIONS)
			{ // Instruction not found.
				line[i].width = 0;
				line[i].type = lineType_Error;
			}
			else
			{
				cmdLen = instructionSet[(int) line[i].line.inst.instruction].mnemonicLen;
				
				switch(instructionSet[(int) line[i].line.inst.instruction].numArgs)
				{
					char* endOfFirstArg;
					
					case 0: // No args, always 2 bytes wide.
						line[i].width = 2;
						line[i].line.inst.type1 = argType_Unused;
						line[i].line.inst.type2 = argType_Unused;
						break;
					
					case 1: // 1 arg, 2 bytes wide plus optional extra 2 bytes for an immediate value.
						line[i].width = 2;
						
						line[i].line.inst.type2 = argType_Unused;
						
						// Bad args are errors.
						if(getArgAndType(&source[pos+cmdLen], len-cmdLen-1, &line[i].line.inst.arg1, &line[i].line.inst.type1) == argType_Bad)
							line[i].type = lineType_Error;
						
						// Unsatified constraints are errors.
						if(!isValidType(line[i].line.inst.type1, instructionSet[(int) line[i].line.inst.instruction].flags1))
							line[i].type = lineType_Error;
						
						if( line[i].line.inst.type1 == argType_DerefImmediate || line[i].line.inst.type1 == argType_Immediate ||
							line[i].line.inst.type1 == argType_DerefLabel || line[i].line.inst.type1 == argType_Label)
							line[i].width += 2;
						break;
					
					case 2: // 2 args, 2 bytes wide plus optional extra 2 bytes for immediate values.
						line[i].width = 2;
						
						endOfFirstArg = strnchr(&source[pos], ',', len);
						if(endOfFirstArg == NULL)
							line[i].type = lineType_Error;
						
						// Bad args are errors.
						if(getArgAndType(&source[pos+cmdLen], endOfFirstArg-&source[pos+cmdLen], &line[i].line.inst.arg1, &line[i].line.inst.type1) == argType_Bad)
							line[i].type = lineType_Error;
						
						// Unsatified constraints are errors.
						if(!isValidType(line[i].line.inst.type1, instructionSet[(int) line[i].line.inst.instruction].flags1))
							line[i].type = lineType_Error;
						
						if( line[i].line.inst.type1 == argType_DerefImmediate || line[i].line.inst.type1 == argType_Immediate ||
							line[i].line.inst.type1 == argType_DerefLabel || line[i].line.inst.type1 == argType_Label)
							line[i].width += 2;
						
						// Bad args are errors.
						if(getArgAndType(endOfFirstArg+1, len-(int)(endOfFirstArg-&source[pos])-2, &line[i].line.inst.arg2, &line[i].line.inst.type2) == argType_Bad)
							line[i].type = lineType_Error;
						
						// Unsatified constraints are errors.
						if(!isValidType(line[i].line.inst.type2, instructionSet[(int) line[i].line.inst.instruction].flags2))
							line[i].type = lineType_Error;
						
						if( line[i].line.inst.type2 == argType_DerefImmediate || line[i].line.inst.type2 == argType_Immediate ||
							line[i].line.inst.type2 == argType_DerefLabel || line[i].line.inst.type2 == argType_Label)
							line[i].width += 2;
						break;
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
	
	// Iterate through the instruction set looking for matches.
	while(strncasecmp(str, instructionSet[i].mnemonic, instructionSet[i].mnemonicLen) && ++i < NUM_INSTRUCTIONS);
	
	return i;
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
