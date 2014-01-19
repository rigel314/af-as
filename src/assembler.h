/*
 * assembler.h
 *
 *  Created on: Jan 17, 2014
 *      Author: cody
 */

#ifndef ASSEMBLER_H_
#define ASSEMBLER_H_

enum instArgType { argType_Unused, argType_Immediate, argType_DerefImmediate, argType_Register, argType_DerefRegister, argType_Label, argType_DerefLabel, argType_Bad };

enum registers { reg_r0, reg_r1, reg_r2, reg_r3, NUM_REGISTERS, reg_pc, reg_sp, NUM_ALLREGISTERS, reg_immediate };

enum lineType { lineType_None, lineType_Instruction, lineType_Label, lineType_Byte, lineType_Error };

enum directive { directive_org, directive_equ, directive_db, directive_dw, directive_fill, directive_text, NUM_DIRECTIVES };

enum instructions { inst_nop, inst_dd, inst_add, inst_sub, inst_mul, inst_div, inst_and, inst_or, inst_xor, inst_inc, inst_dec, inst_pop, inst_push, inst_shr, inst_shl, inst_shcr, inst_shcl, inst_ror, inst_rol, inst_rocr, inst_rocl, inst_halt, NUM_INSTRUCTIONS };

enum targetFlags { target_None, target_Register_Only, target_Immediate_Only, target_RegisterOrDeref, target_Any, target_Special };

struct label
{
	char* name;
};

struct instruction
{
	int arg1;
	int arg2;
	char instruction;
	char type1;
	char type2;
};

struct byte
{
	char* vals;
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
	int width;
	char type;
};

struct instructionSetEntry
{
	char mnumonic[5];
	char mnumonicLen;
	char numArgs;
	char flags1;
	char flags2;
	char bitField;
};

struct validRegs
{
	char name[3];
	char len;
	char bitField;
};

extern const struct instructionSetEntry instructionSet[NUM_INSTRUCTIONS];
extern const struct validRegs regStrings[NUM_ALLREGISTERS+2];

void resolveLabels(struct lineinfo* lines, int len);
int findLabel(char* name, struct lineinfo* lines, int len);
void assemble(struct lineinfo* lines, int len);
int structify(char* source, struct lineinfo** lines);
int getArgAndType(char* str, int len, int* val, char* type);
int getInstruction(char* str, int len);
int getDirective(char* str, int len);
void freeLineinfos(struct lineinfo* lines, int len);

#endif /* ASSEMBLER_H_ */
