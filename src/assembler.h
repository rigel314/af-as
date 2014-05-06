/*
 * assembler.h
 *
 *  Created on: Jan 17, 2014
 *      Author: cody
 */

#ifndef ASSEMBLER_H_
#define ASSEMBLER_H_

#include <stdbool.h>
#include <stdint.h>

// Bit to set in register bitFeild to indicate derefrencing.
#define TargetDerefBit 0x20

// Argument types.
// The order of this enum is important.  lines with 'enum order' comments signify which lines rely on this order.
enum instArgType { argType_Unused, argType_Bad, argType_Immediate, argType_DerefImmediate, argType_Register, argType_DerefRegister, argType_Label, argType_DerefLabel };

// Internal values for registers.
enum registers { reg_r0, reg_r1, reg_r2, reg_r3, reg_r4, reg_r5, reg_r6, reg_r7, reg_r8, reg_r9, NUM_REGISTERS, reg_pc, reg_sp, reg_fl, NUM_ALLREGISTERS, reg_immediate };

// Line types.
enum lineType { lineType_None, lineType_Instruction, lineType_Label, lineType_Byte, lineType_Error };

// Internal values for directives.
enum directive { directive_org, directive_equ, directive_db, directive_dw, directive_fill, directive_text, NUM_DIRECTIVES };

// Internal values for instructions.
enum instructions { inst_nop, inst_halt, inst_wai, inst_dd, inst_add, inst_adc, inst_sub, inst_sbc, inst_mul, inst_imul, inst_div, inst_idiv, inst_and, inst_or, inst_xor, inst_inc, inst_dec, inst_pop, inst_push, inst_shr, inst_shl, inst_ishr, inst_ishl, inst_ror, inst_rol, inst_rrc, inst_rlc, inst_call, inst_bcda, NUM_INSTRUCTIONS };

// Restrictions on argument types.
enum targetFlags { target_None, target_Register_Only, target_Immediate_Only, target_RegisterOrDeref, target_Any, target_Special };

// Allowed conditions.
enum conditions { cond_always, cond_never, cond_zero, cond_notZero, cond_carry, cond_notCarry, cond_overflow, cond_notOverflow, cond_negative, cond_notNeg, cond_greatEq, cond_greater, cond_lessEq, cond_less, cond_highEq, cond_higher, cond_lowEq, cond_lower, cond_equal, cond_notEq, NUM_CONDITIONS };

// Allowed data bus widths for instructions.
enum allowedWidths { instWidth_32, instWidth_16, instWidth_8 };

// Labels have a name.
struct label
{
	char* name;
};

// An arg could be a value or a name.
union arg
{
	uint32_t val;
	char* name;
};

// An instruction can have up to four arguments and its value.
// It also can execute only on a specific condition.
// It also can optionally not mutate flags.
// It also can optionally be 8, 16, or 32 bits. (as far as data reads and writes)
struct instruction
{
	union arg args[4];
	int condition;
	char width;
	bool mutateFlags;
	char numArgsUsed;
	char instruction; // This is the actual instruction from the enum instructions.
	char types[4];
};

// Byte can be one or more bytes. In the future, these will be created by various assembler directives.
struct byte
{
	char* vals;
	int length;
};

// A line could be a label, an instruction, or a byte.
union line
{
	struct label lbl;
	struct instruction inst;
	struct byte byte;
};

// Every line has an address, number, width, type, and data.
struct lineinfo
{
	union line line;
	int address;
	int lineNum;
	int width;
	char type;
};

// Keeps information about the instruction set.
struct instructionSetEntry
{
	char mnemonic[5];
	char mnemonicLen;
	char numArgs;
	char numOptArgs;
	char argFlags[4];
	uint32_t bitField;
	uint32_t bitMask;
};

// Keeps information about the registers.
struct validRegs
{
	char name[4];
	char len;
	char bitField;
};

// Keeps information about the conditions.
struct validCondidions
{
	char name[3];
	char len;
	char bitField;
};

extern const struct instructionSetEntry instructionSet[NUM_INSTRUCTIONS];
extern const struct validRegs regStrings[NUM_ALLREGISTERS+2];

void resolveLabels(struct lineinfo* lines, int len);
int findLabel(char* name, struct lineinfo* lines, int len);
void assemble(char* file, struct lineinfo* lines, int len);
bool isValidType(char type, char constraint);
int structify(char* source, struct lineinfo** lines);
int getArgAndType(char* str, int len, union arg* val, char* type);
int getInstruction(char* str, int len);
int getCondition(char* str, int len);
int getDataWidth(char* str, int len);
int getDirective(char* str, int len);
void freeLineinfos(struct lineinfo* lines, int len);

#endif /* ASSEMBLER_H_ */
