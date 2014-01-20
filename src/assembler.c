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
	{"",   0, 0b01111}
};

void resolveLabels(struct lineinfo* lines, int len)
{
	for(int i=0; i<len; i++)
	{
		if(lines[i].type != lineType_Instruction)
			continue;
		
		if(lines[i].line.inst.type1 == argType_Label || lines[i].line.inst.type1 == argType_DerefLabel)
		{
			char* arglbl = lines[i].line.inst.arg1.name;
			
			lines[i].line.inst.type1 = argType_Immediate | (0x01 & lines[i].line.inst.type1);
			lines[i].line.inst.arg1.val = findLabel(arglbl, lines, len);
			
			free(arglbl);
			
			if(lines[i].line.inst.arg1.val == -1)
				lines[i].type = lineType_Error;
		}
		if(lines[i].line.inst.type2 == argType_Label || lines[i].line.inst.type2 == argType_DerefLabel)
		{
			char* arglbl = lines[i].line.inst.arg1.name;
			
			lines[i].line.inst.type2 = argType_Immediate | (0x01 & lines[i].line.inst.type2);
			lines[i].line.inst.arg2.val = findLabel(lines[i].line.inst.arg2.name, lines, len);
			
			free(arglbl);
			
			if(lines[i].line.inst.arg2.val == -1)
				lines[i].type = lineType_Error;
		}
	}
}
int findLabel(char* name, struct lineinfo* lines, int len)
{
	for(int i=0; i<len; i++)
	{
		if(lines[i].type != lineType_Label)
			continue;
		if(!strcmp(lines[i].line.lbl.name, name))
			return lines[i].address;
	}
		
	return -1;
}

void assemble(char* file, struct lineinfo* lines, int len)
{
	int addr = 0;
	FILE* fp = fopen(file, "w");
	if(!fp)
		fp = stdout;
	
	for(int i=0; i<len; i++)
	{
		int inst;
		uint16_t word = 0;
		char target1 = -1;
		char target2 = -1;
		
		if(lines[i].type == lineType_Error)
			fprintf(stderr, "Line %d - Error!\n", lines[i].lineNum);
		if(lines[i].type != lineType_Instruction)
			continue;
		
		if(!addr)
			addr = lines[i].address;
		if(addr != lines[i].address)
		{
			for(; addr<lines[i].address; addr++)
				writeInt8(fp, 0);
		}
		addr += lines[i].width;
		
		inst = lines[i].line.inst.instruction;
		
		word = instructionSet[inst].bitField << 10;
		if(lines[i].line.inst.type1 == argType_DerefRegister || lines[i].line.inst.type1 == argType_Register)
		{
			target1 = regStrings[lines[i].line.inst.arg1.val].bitField;
			if(lines[i].line.inst.type1 == argType_DerefRegister)
				target1 |= TargetDerefBit;
			word |= (target1 << 5);
		}
		else if(lines[i].line.inst.type1 == argType_DerefImmediate || lines[i].line.inst.type1 == argType_Immediate)
		{
			target1 = regStrings[reg_immediate].bitField;
			if(lines[i].line.inst.type1 == argType_DerefImmediate)
				target1 |= TargetDerefBit;
			word |= (target1 << 5);
		}
		if(lines[i].line.inst.type2 == argType_DerefRegister || lines[i].line.inst.type2 == argType_Register)
		{
			target2 = regStrings[lines[i].line.inst.arg2.val].bitField;
			if(lines[i].line.inst.type2 == argType_DerefRegister)
				target2 |= TargetDerefBit;
			word |= target2;
		}
		else if(lines[i].line.inst.type2 == argType_DerefImmediate || lines[i].line.inst.type2 == argType_Immediate)
		{
			target2 = regStrings[reg_immediate].bitField;
			if(lines[i].line.inst.type2 == argType_DerefImmediate)
				target2 |= TargetDerefBit;
			word |= target2;
		}
		
		writeInt16(fp, word);
		
		if((target1 | TargetDerefBit) == (regStrings[reg_immediate].bitField | TargetDerefBit))
			writeInt16(fp, lines[i].line.inst.arg1.val);
		if((target2 | TargetDerefBit) == (regStrings[reg_immediate].bitField | TargetDerefBit))
			writeInt16(fp, lines[i].line.inst.arg1.val);
	}
	
	if(fp != stdout)
		fclose(fp);
}

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
			switch(getDirective(&source[pos], len))
			{
				int val;
				
				case directive_org:
					if(sscanf(&source[pos], ".org %i", &val) == 1)
						addr = val;
					else
						line[i].type = lineType_Error;
					break;
			}
		}
		else // Must be an instruction.
		{
			int cmdLen;
			
			line[i].type = lineType_Instruction;
			
			line[i].line.inst.instruction = getInstruction(&source[pos], len);
			if(line[i].line.inst.instruction == NUM_INSTRUCTIONS)
			{
				line[i].width = 0;
				line[i].type = lineType_Error;
			}
			else
			{
				cmdLen = instructionSet[(int) line[i].line.inst.instruction].mnumonicLen;
				
				switch(instructionSet[(int) line[i].line.inst.instruction].numArgs)
				{
					char* endOfFirstArg;
					
					case 0:
						line[i].width = 2;
						line[i].line.inst.type1 = argType_Unused;
						line[i].line.inst.type2 = argType_Unused;
						break;
					
					case 1:
						line[i].width = 2;
						
						line[i].line.inst.type2 = argType_Unused;
						
						if(getArgAndType(&source[pos+cmdLen], len-cmdLen-1, &line[i].line.inst.arg1, &line[i].line.inst.type1) == argType_Bad)
							line[i].type = lineType_Error;
						
						if(!isValidType(line[i].line.inst.type1, instructionSet[(int) line[i].line.inst.instruction].flags1))
							line[i].type = lineType_Error;
						
						if(line[i].line.inst.type1 == argType_DerefImmediate || line[i].line.inst.type1 == argType_Immediate)
							line[i].width += 2;
						break;
					
					case 2:
						line[i].width = 2;
						
						endOfFirstArg = strnchr(&source[pos], ',', len);
						if(endOfFirstArg == NULL)
							line[i].type = lineType_Error;
						
						if(getArgAndType(&source[pos+cmdLen], endOfFirstArg-&source[pos+cmdLen], &line[i].line.inst.arg1, &line[i].line.inst.type1) == argType_Bad)
							line[i].type = lineType_Error;
						
						if(!isValidType(line[i].line.inst.type1, instructionSet[(int) line[i].line.inst.instruction].flags1))
							line[i].type = lineType_Error;
						
						if( line[i].line.inst.type1 == argType_DerefImmediate || line[i].line.inst.type1 == argType_Immediate ||
							line[i].line.inst.type1 == argType_DerefLabel || line[i].line.inst.type1 == argType_Label)
							line[i].width += 2;
						
						if(getArgAndType(endOfFirstArg+1, len-(int)(endOfFirstArg-&source[pos])-2, &line[i].line.inst.arg2, &line[i].line.inst.type2) == argType_Bad)
							line[i].type = lineType_Error;
						
						if(!isValidType(line[i].line.inst.type2, instructionSet[(int) line[i].line.inst.instruction].flags2))
							line[i].type = lineType_Error;
						
						if( line[i].line.inst.type2 == argType_DerefImmediate || line[i].line.inst.type2 == argType_Immediate ||
							line[i].line.inst.type2 == argType_DerefLabel || line[i].line.inst.type2 == argType_Label)
							line[i].width += 2;
						break;
				}
			}
			addr += line[i].width;
		}
		
		pos += len;
	}
	
	return numLines;
}

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
		
		if(valFlag && str[i] != ')')
			return argType_Bad;
		
		if(str[i] == '(')
			parenFlag++;
		if(parenFlag && str[i] == ')')
			parenFlag++;
		
		if(!valFlag && (((str[i] | ASCIIshiftBit) >= 'a' && (str[i] | ASCIIshiftBit) <= 'z') || str[i] == '_'))
		{
			int j = 0;
			int toklen = 0;
			
			for(int k=i; k<len && str[k]!=' ' && str[k]!='\t' && str[k]!=')'; k++, toklen++);
			
			while(strncasecmp(&str[i], regStrings[j].name, toklen) && ++j < NUM_ALLREGISTERS);
			
			if(j == NUM_REGISTERS)
				return argType_Bad;
			
			if(j == NUM_ALLREGISTERS)
			{
				valFlag = true;
				retval = argType_Label;
				
				val->name = calloc(len+1, 1);
				
				for(int j=0; ((str[i] | ASCIIshiftBit) >= 'a' && (str[i] | ASCIIshiftBit) <= 'z') ||
							 (str[i] >= '0' && str[i] <= '9' && i>0) ||
							 (str[i] == '_')
							 ; i++)
					val->name[j++] = str[i];
				i--;
			}
		}
		
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
		
		if(!valFlag && str[i] == 'r') // Found a register.
		{
			retval = argType_Register;
			
			if(sscanf(&str[i], "r%u", &val->val) == 1) // Sets val to register number. reg_r0 will have value zero from enum, reg_r1 will have 1, and so on.
				valFlag = true;
			else
				return argType_Bad;
			
			if(val->val >= NUM_REGISTERS)
				return argType_Bad;
			
			for(i++; i<len && str[i] >= '0' && str[i] <= '9'; i++);
			i--;
		}
		
		if(!valFlag && str[i] >= '0' && str[i] <= '9')
		{
			retval = argType_Immediate;
			
			if(sscanf(&str[i], "%u", &val->val) == 1) // Sets val to the immediate value found.
				valFlag = true;
			else
				return argType_Bad;
			
			if(val->val > 0xFFFF)
				return argType_Bad;
			
			for(i++; i<len && str[i] >= '0' && str[i] <= '9'; i++);
			i--;
		}
	}
	
	if(!valFlag || parenFlag==1)
		return argType_Bad;
	
	if(parenFlag == 2)
		retval++;
	
	*type = retval;
	
	return retval;
}

int getInstruction(char* str, int len)
{
	int i = 0;
	
	while(strncasecmp(str, instructionSet[i].mnumonic, instructionSet[i].mnumonicLen) && ++i < NUM_INSTRUCTIONS);
	return i;
}

int getDirective(char* str, int len)
{
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

void freeLineinfos(struct lineinfo* lines, int len)
{
	for(int i=0; i<len; i++)
	{
		if(lines[i].type == lineType_Label)
			free(lines[i].line.lbl.name);
		if(lines[i].type == lineType_Byte)
			free(lines[i].line.byte.vals);
	}
	free(lines);
}
