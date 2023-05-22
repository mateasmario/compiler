#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../definitions/VmDefinitions.h"
#include "../functions/ErrorFunctions.h"
#include "../structs/Vm.h"
#include "../functions/EssentialFunctions.h"
#include "../enums/VmEnums.h"
#include "../structs/Symbol.h"
#include "../functions/VmFunctions.h"

Instr* instructions, * lastInstruction;
Instr* crtLoopEnd;

void pushd(double d)
{
	if (SP + sizeof(double) > stackAfter)err("out of stack");
	*(double*)SP = d;
	SP += sizeof(double);
}
double popd()
{
	SP -= sizeof(double);
	if (SP < stack)err("not enough stack bytes for popd");
	return *(double*)SP;
}
void pusha(void* a)
{
	if (SP + sizeof(void*) > stackAfter)err("out of stack");
	*(void**)SP = a;
	SP += sizeof(void*);
}
void* popa()
{
	SP -= sizeof(void*);
	if (SP < stack)err("not enough stack bytes for popa");
	return *(void**)SP;
}
void pushc(char c)
{
	if (SP + sizeof(char) > stackAfter)err("out of stack");
	*(char*)SP = c;
	SP += sizeof(char);
}
char popc()
{
	SP -= sizeof(char);
	if (SP < stack)err("not enough stack bytes for popc");
	return *(char*)SP;
}
void pushi(int i)
{
	if (SP + sizeof(int) > stackAfter)err("out of stack");
	*(int*)SP = i;
	SP += sizeof(int);
}
int popi()
{
	SP -= sizeof(int);
	if (SP < stack)err("not enough stack bytes for popi");
	return *(int*)SP;
}

Instr* createInstr(int opcode)
{
	Instr* i;
	SAFEALLOC(i, Instr)
		i->opcode = opcode;
	return i;
}
void insertInstrAfter(Instr* after, Instr* i)
{
	i->next = after->next;
	i->last = after;
	after->next = i;
	if (i->next == NULL)lastInstruction = i;
}
Instr* addInstr(int opcode)
{
	Instr* i = createInstr(opcode);
	i->next = NULL;
	i->last = lastInstruction;
	if (lastInstruction) {
		lastInstruction->next = i;
	}
	else {
		instructions = i;
	}
	lastInstruction = i;
	return i;
}
Instr* addInstrAfter(Instr* after, int opcode)
{
	Instr* i = createInstr(opcode);
	insertInstrAfter(after, i);
	return i;
}

Instr* addInstrA(int opcode, void* addr) {
	Instr* instr = addInstr(opcode);
	instr->args[0].addr = addr;
	return instr;
}

Instr* addInstrI(int opcode, long int val) {
	Instr* instr = addInstr(opcode);
	instr->args[0].i = val;
	return instr;
}

Instr* addInstrII(int opcode, long int val1, long int val2) {
	Instr* instr = addInstr(opcode);
	instr->args[0].i = val1;
	instr->args[1].i = val2;
	return instr;
}

Instr* appendInstr(Instr* i) {
	insertInstrAfter(lastInstruction, i);
	return i;
}

void deleteInstructionsAfter(Instr* start) {
	Instr* curr = NULL;
	curr = start->next;

	while (curr != NULL) {
		Instr* temp = curr;
		curr = curr->next;
	}

	start->next = NULL;
	lastInstruction = start;
}

void* allocGlobal(int size)
{
	void* p = globals + nGlobals;
	if (nGlobals + size > GLOBAL_SIZE)err("insufficient globals space");
	nGlobals += size;
	return p;
}

void run(Instr* IP)
{
	long int iVal1, iVal2;
	double dVal1, dVal2;
	char* aVal1, *aVal2;
	char* FP = 0, * oldSP;
	SP = stack;
	stackAfter = stack + STACK_SIZE;
	while (1) {
		printf("%p/%d\t", IP, SP - stack);

		if (IP == NULL)
			break;

		switch (IP->opcode) {
		case O_ADD_C:
			iVal1 = popc();
			iVal2 = popc();
			printf("ADD_C\t(%c-%c -> %c)\n", iVal2, iVal1, iVal2 + iVal1);
			pushc(iVal2 + iVal1);
			IP = IP->next;
			break;
		case O_ADD_D:
			dVal1 = popd();
			dVal2 = popd();
			printf("ADD_D\t(%g-%g -> %g)\n", dVal2, dVal1, dVal2 + dVal1);
			pushd(dVal2 + dVal1);
			IP = IP->next;
			break;
		case O_ADD_I:
			iVal1 = popi();
			iVal2 = popi();
			printf("ADD_I\t(%d-%d -> %d)\n", iVal2, iVal1, iVal2 + iVal1);
			pushi(iVal2 + iVal1);
			IP = IP->next;
			break;
		case O_AND_A:
			aVal1 = (char*)popc();
			aVal2 = (char*)popc();
			printf("AND_A\t(%d-%d -> %ld)\n", aVal2, aVal1, aVal2 && aVal1);
			pushc(aVal2 && aVal1);
			IP = IP->next;
			break;
		case O_AND_C:
			iVal1 = popc();
			iVal2 = popc();
			printf("AND_C\t(%c-%c -> %ld)\n", iVal2, iVal1, iVal2 && iVal1);
			pushc(iVal2 && iVal1);
			IP = IP->next;
			break;
		case O_AND_D:
			dVal1 = popd();
			dVal2 = popd();
			printf("AND_D\t(%g-%g -> %ld)\n", dVal2, dVal1, dVal2 && dVal1);
			pushd(dVal2 && dVal1);
			IP = IP->next;
			break;
		case O_AND_I:
			iVal1 = popi();
			iVal2 = popi();
			printf("AND_I\t(%d-%d -> %ld)\n", iVal2, iVal1, iVal2 && iVal1);
			pushi(iVal2 && iVal1);
			IP = IP->next;
			break;
		case O_CALL:
			// void* cast error, did a cast to (char*)
			aVal1 = (char*)IP->args[0].addr;
			printf("CALL\t%p\n", aVal1);
			pusha(IP->next);
			IP = (Instr*)aVal1;
			break;
		case O_CALLEXT:
			printf("CALLEXT\t%p\n", IP->args[0].addr);
			(*(void(*)())IP->args[0].addr)();
			IP = IP->next;
			break;
		case O_CAST_C_D:
			iVal1 = popc();
			dVal1 = (double)iVal1;
			printf("CAST_C_D\t(%c -> %g)\n", iVal1, dVal1);
			pushd(dVal1);
			IP = IP->next;
			break;
		case O_CAST_C_I:
			iVal1 = popc();
			iVal2 = (int)iVal1;
			printf("CAST_C_I\t(%c -> %ld)\n", iVal1, iVal2);
			pushi(iVal2);
			IP = IP->next;
			break;
		case O_CAST_D_C:
			dVal1 = popd();
			iVal1 = (char)dVal1;
			printf("CAST_D_C\t(%g -> %c)\n", dVal1, iVal1);
			pushc(iVal1);
			IP = IP->next;
			break;
		case O_CAST_D_I:
			dVal1 = popd();
			iVal1 = (int)dVal1;
			printf("CAST_D_I\t(%g -> %ld)\n", dVal1, iVal1);
			pushi(iVal1);
			IP = IP->next;
			break;
		case O_CAST_I_C:
			iVal1 = popi();
			iVal2 = (char)iVal1;
			printf("CAST_I_C\t(%ld -> %c)\n", iVal1, iVal2);
			pushc(iVal2);
			IP = IP->next;
			break;
		case O_DIV_C:
			iVal1 = popc();
			iVal2 = popc();
			printf("DIV_C\t(%c-%c -> %c)\n", iVal2, iVal1, iVal2 / iVal1);
			pushc(iVal1/iVal2);
			IP = IP->next;
			break;
		case O_DIV_D:
			dVal1 = popd();
			dVal2 = popd();
			printf("DIV_D\t(%g-%g -> %g)\n", dVal2, dVal1, dVal2 / dVal1);
			pushd(dVal1/dVal2);
			IP = IP->next;
			break;
		case O_DIV_I:
			iVal1 = popi();
			iVal2 = popi();
			printf("DIV_I\t(%ld-%ld -> %ld)\n", iVal2, iVal1, iVal2 / iVal1);
			pushi(iVal1 / iVal2);
			IP = IP->next;
			break;
		case O_CAST_I_D:
			iVal1 = popi();
			dVal1 = (double)iVal1;
			printf("CAST_I_D\t(%ld -> %g)\n", iVal1, dVal1);
			pushd(dVal1);
			IP = IP->next;
			break;
		case O_DROP:
			iVal1 = IP->args[0].i;
			printf("DROP\t%ld\n", iVal1);
			if (SP - iVal1 < stack)err("not enough stack bytes");
			SP -= iVal1;
			IP = IP->next;
			break;
		case O_ENTER:
			iVal1 = IP->args[0].i;
			printf("ENTER\t%ld\n", iVal1);
			pusha(FP);
			FP = SP;
			SP += iVal1;
			IP = IP->next;
			break;
		case O_EQ_A:
			aVal1 = (char*)popc();
			aVal2 = (char*)popc();
			printf("EQ_A\t(%d==%d -> %ld)\n", aVal2, aVal1, aVal2 == aVal1);
			pushi(aVal2 == aVal1);
			IP = IP->next;
			break;
		case O_EQ_C:
			iVal1 = popc();
			iVal2 = popc();
			printf("EQ_C\t(%c==%c -> %ld)\n", iVal2, iVal1, iVal2 == iVal1);
			pushi(iVal2 == iVal1);
			IP = IP->next;
			break;
		case O_EQ_D:
			dVal1 = popd();
			dVal2 = popd();
			printf("EQ_D\t(%g==%g -> %ld)\n", dVal2, dVal1, dVal2 == dVal1);
			pushi(dVal2 == dVal1);
			IP = IP->next;
			break;
		case O_EQ_I:
			iVal1 = popi();
			iVal2 = popi();
			printf("EQ_I\t(%d==%d -> %ld)\n", iVal2, iVal1, iVal2 == iVal1);
			pushi(iVal2 == iVal1);
			IP = IP->next;
			break;
		case O_GREATER_C:
			iVal1 = popc();
			iVal2 = popc();
			printf("GREATER_C\t(%c==%c -> %ld)\n", iVal2, iVal1, iVal1 > iVal2);
			pushi(iVal1 > iVal2);
			IP = IP->next;
			break;
		case O_GREATER_D:
			dVal1 = popc();
			dVal2 = popc();
			printf("GREATER_D\t(%g==%g -> %ld)\n", dVal2, dVal1, dVal1 > dVal2);
			pushi(dVal1 > dVal2);
			IP = IP->next;
			break;
		case O_GREATER_I:
			iVal1 = popi();
			iVal2 = popi();
			printf("GREATER_I\t(%d==%d -> %ld)\n", iVal2, iVal1, iVal1 > iVal2);
			pushi(iVal1 > iVal2);
			IP = IP->next;
			break;
		case O_GREATEREQ_C:
			iVal1 = popc();
			iVal2 = popc();
			printf("GREATER_C\t(%c==%c -> %ld)\n", iVal2, iVal1, iVal1 >= iVal2);
			pushi(iVal1 >= iVal2);
			IP = IP->next;
			break;
		case O_GREATEREQ_D:
			dVal1 = popc();
			dVal2 = popc();
			printf("GREATER_D\t(%g==%g -> %ld)\n", dVal2, dVal1, dVal1 >= dVal2);
			pushi(dVal1 >= dVal2);
			IP = IP->next;
			break;
		case O_GREATEREQ_I:
			iVal1 = popi();
			iVal2 = popi();
			printf("GREATER_I\t(%d==%d -> %ld)\n", iVal2, iVal1, iVal1 >= iVal2);
			pushi(iVal1 >= iVal2);
			IP = IP->next;
			break;
		case O_HALT:
			printf("HALT\n");
			return;
		case O_INSERT:
			iVal1 = IP->args[0].i; // iDst
			iVal2 = IP->args[1].i; // nBytes
			printf("INSERT\t%ld,%ld\n", iVal1, iVal2);
			if (SP + iVal2 > stackAfter)err("out of stack");
			memmove(SP - iVal1 + iVal2, SP - iVal1, iVal1); //make room
			memmove(SP - iVal1, SP + iVal2, iVal2); //dup
			SP += iVal2;
			IP = IP->next;
			break;
		case O_JF_A:
			iVal1 = popc();
			printf("JF\t%p\t(%ld)\n", IP->args[0].addr, iVal1);
			// void* cast error, did a cast to (Instr*)
			IP = (iVal1 == 0) ? (Instr*)IP->args[0].addr : IP->next;
			break;
		case O_JF_C:
			iVal1 = popc();
			printf("JF\t%p\t(%ld)\n", IP->args[0].addr, iVal1);
			// void* cast error, did a cast to (Instr*)
			IP = (iVal1 == 0) ? (Instr*)IP->args[0].addr : IP->next;
			break;
		case O_JF_D:
			dVal1 = popd();
			printf("JF\t%p\t(%ld)\n", IP->args[0].addr, dVal1);
			// void* cast error, did a cast to (Instr*)
			IP = (dVal1 == 0) ? (Instr*)IP->args[0].addr : IP->next;
			break;
		case O_JF_I:
			iVal1 = popi();
			printf("JF\t%p\t(%ld)\n", IP->args[0].addr, iVal1);
			// void* cast error, did a cast to (Instr*)
			IP = (iVal1 == 0) ? (Instr*)IP->args[0].addr : IP->next;
			break;
		case O_JMP:
			aVal1 = (char*)IP->args[0].addr;
			IP = (Instr*)aVal1;
			break;
		case O_JT_A:
			iVal1 = popc();
			printf("JT\t%p\t(%ld)\n", IP->args[0].addr, iVal1);
			// void* cast error, did a cast to (Instr*)
			IP = iVal1 ? (Instr*)IP->args[0].addr : IP->next;
			break;
		case O_JT_C:
			iVal1 = popc();
			printf("JT\t%p\t(%ld)\n", IP->args[0].addr, iVal1);
			// void* cast error, did a cast to (Instr*)
			IP = iVal1 ? (Instr*)IP->args[0].addr : IP->next;
			break;
		case O_JT_D:
			dVal1 = popd();
			printf("JT\t%p\t(%ld)\n", IP->args[0].addr, dVal1);
			// void* cast error, did a cast to (Instr*)
			IP = dVal1 ? (Instr*)IP->args[0].addr : IP->next;
			break;
		case O_JT_I:
			iVal1 = popi();
			printf("JT\t%p\t(%ld)\n", IP->args[0].addr, iVal1);
			// void* cast error, did a cast to (Instr*)
			IP = iVal1 ? (Instr*)IP->args[0].addr : IP->next;
			break;
		case O_LESS_C:
			iVal1 = popc();
			iVal2 = popc();
			printf("LESS_C\t(%c==%c -> %ld)\n", iVal2, iVal1, iVal1 < iVal2);
			pushi(iVal1 < iVal2);
			IP = IP->next;
			break;
		case O_LESS_D:
			dVal1 = popc();
			dVal2 = popc();
			printf("LESS_D\t(%g==%g -> %ld)\n", dVal2, dVal1, dVal1 < dVal2);
			pushi(dVal1 < dVal2);
			IP = IP->next;
			break;
		case O_LESS_I:
			iVal1 = popi();
			iVal2 = popi();
			printf("LESS_I\t(%d==%d -> %ld)\n", iVal2, iVal1, iVal1 < iVal2);
			pushi(iVal1 < iVal2);
			IP = IP->next;
			break;
		case O_LESSEQ_C:
			iVal1 = popc();
			iVal2 = popc();
			printf("LESSEQ_C\t(%c==%c -> %ld)\n", iVal2, iVal1, iVal1 <= iVal2);
			pushi(iVal1 <= iVal2);
			IP = IP->next;
			break;
		case O_LESSEQ_D:
			dVal1 = popc();
			dVal2 = popc();
			printf("LESSEQ_D\t(%g==%g -> %ld)\n", dVal2, dVal1, dVal1 <= dVal2);
			pushi(dVal1 <= dVal2);
			IP = IP->next;
			break;
		case O_LESSEQ_I:
			iVal1 = popi();
			iVal2 = popi();
			printf("LESSEQ_I\t(%c==%c -> %ld)\n", iVal2, iVal1, iVal1 <= iVal2);
			pushi(iVal1 <= iVal2);
			IP = IP->next;
			break;
		case O_LOAD:
			iVal1 = IP->args[0].i;
			// void* cast error, did a cast to (char*)
			aVal1 = (char*)popa();
			printf("LOAD\t%ld\t(%p)\n", iVal1, aVal1);
			if (SP + iVal1 > stackAfter)err("out of stack");
			memcpy(SP, aVal1, iVal1);
			SP += iVal1;
			IP = IP->next;
			break;
		case O_MUL_C:
			iVal1 = popc();
			iVal2 = popc();
			printf("MUL_C\t(%c-%c -> %c)\n", iVal2, iVal1, iVal2 * iVal1);
			pushc(iVal2 * iVal1);
			IP = IP->next;
			break;
		case O_MUL_D:
			dVal1 = popd();
			dVal2 = popd();
			printf("MUL_D\t(%g-%g -> %g)\n", dVal2, dVal1, dVal2 * dVal1);
			pushd(dVal2 * dVal1);
			IP = IP->next;
			break;
		case O_MUL_I:
			iVal1 = popi();
			iVal2 = popi();
			printf("MUL_I\t(%d-%d -> %d)\n", iVal2, iVal1, iVal2 * iVal1);
			pushi(iVal2 * iVal1);
			IP = IP->next;
			break;
		case O_NEG_C:
			iVal1 = popc();
			printf("NEG_C\t(%c -> %c)\n", iVal1, -1*iVal1);
			pushc(-1*iVal1);
			IP = IP->next;
			break;
		case O_NEG_D:
			dVal1 = popd();
			printf("NEG_D\t(%g -> %g)\n", dVal1, -1 * dVal1);
			pushc(-1 * dVal1);
			IP = IP->next;
			break;
		case O_NEG_I:
			dVal1 = popi();
			printf("NEG_I\t(%d -> %d)\n", dVal1, -1 * dVal1);
			pushc(-1 * dVal1);
			IP = IP->next;
			break;
		case O_NOP:
			IP = IP->next;
			break;
		case O_NOTEQ_A:
			aVal1 = (char*)popc();
			aVal2 = (char*)popc();
			printf("NOTEQ_A\t(%d==%d -> %ld)\n", aVal1, aVal2, aVal1 != aVal2);
			pushi(aVal1 != aVal2);
			IP = IP->next;
			break;
		case O_NOTEQ_C:
			iVal1 = popc();
			iVal2 = popc();
			printf("NOTEQ_C\t(%c==%c -> %ld)\n", iVal1, iVal2, iVal1 != iVal2);
			pushi(iVal1 != iVal2);
			IP = IP->next;
			break;
		case O_NOTEQ_D:
			dVal1 = popd();
			dVal2 = popd();
			printf("NOTEQ_D\t(%g==%g -> %ld)\n", dVal1, dVal2, dVal1 != dVal2);
			pushi(dVal1 != dVal2);
			IP = IP->next;
			break;
		case O_NOTEQ_I:
			iVal1 = popi();
			iVal2 = popi();
			printf("NOTEQ_I\t(%d==%d -> %ld)\n", iVal1, iVal2, iVal1 != iVal2);
			pushi(iVal1 != iVal2);
			IP = IP->next;
			break;
		case O_NOT_A:
			aVal1 = (char*)popc();
			printf("NOT_A\t(%d -> %d)\n", aVal1, !aVal1);
			pushc(!aVal1);
			IP = IP->next;
			break;
		case O_NOT_C:
			iVal1 = popc();
			printf("NOT_C\t(%c -> %c)\n", iVal1, !iVal1);
			pushc(!iVal1);
			IP = IP->next;
			break;
		case O_NOT_D:
			dVal1 = popd();
			printf("NOT_D\t(%g -> %g)\n", dVal1, !dVal1);
			pushc(!dVal1);
			IP = IP->next;
			break;
		case O_NOT_I:
			iVal1 = popi();
			printf("NOT_I\t(%d -> %d)\n", iVal1, !iVal1);
			pushc(!iVal1);
			IP = IP->next;
			break;
		case O_OFFSET:
			iVal1 = popi();
			// void* cast error, did a cast to (char*)
			aVal1 = (char*)popa();
			printf("OFFSET\t(%p+%ld -> %p)\n", aVal1, iVal1, aVal1 + iVal1);
			pusha(aVal1 + iVal1);
			IP = IP->next;
			break;
		case O_OR_A:
			aVal1 = (char*)popc();
			aVal2 = (char*)popc();
			printf("OR_A\t(%d-%d -> %ld)\n", aVal2, aVal1, aVal2 || aVal1);
			pushc(aVal2 || aVal1);
			IP = IP->next;
			break;
		case O_OR_C:
			iVal1 = popc();
			iVal2 = popc();
			printf("OR_C\t(%c-%c -> %ld)\n", iVal2, iVal1, iVal2 || iVal1);
			pushc(iVal2 || iVal1);
			IP = IP->next;
			break;
		case O_OR_D:
			dVal1 = popd();
			dVal2 = popd();
			printf("OR_D\t(%g-%g -> %ld)\n", dVal2, dVal1, dVal2 || dVal1);
			pushd(dVal2 || dVal1);
			IP = IP->next;
			break;
		case O_OR_I:
			iVal1 = popi();
			iVal2 = popi();
			printf("OR_I\t(%d-%d -> %ld)\n", iVal2, iVal1, iVal2 || iVal1);
			pushi(iVal2 || iVal1);
			IP = IP->next;
			break;
		case O_PUSHFPADDR:
			iVal1 = IP->args[0].i;
			printf("PUSHFPADDR\t%ld\t(%p)\n", iVal1, FP + iVal1);
			pusha(FP + iVal1);
			IP = IP->next;
			break;
		case O_PUSHCT_A:
			// void* cast error, did a cast to (char*)
			aVal1 = (char*)IP->args[0].addr;
			printf("PUSHCT_A\t%p\n", aVal1);
			pusha(aVal1);
			IP = IP->next;
			break;
		case O_PUSHCT_C:
			iVal1 = IP->args[0].i;
			printf("PUSHCT_C\t%p\n", iVal1);
			pushc(iVal1);
			IP = IP->next;
			break;
		case O_PUSHCT_D:
			dVal1 = IP->args[0].d;
			printf("PUSHCT_D\t%p\n", dVal1);
			pushd(dVal1);
			IP = IP->next;
			break;
		case O_PUSHCT_I:
			iVal1 = IP->args[0].i;
			printf("PUSHCT_I\t%p\n", iVal1);
			pushi(iVal1);
			IP = IP->next;
			break;
		case O_RET:
			iVal1 = IP->args[0].i; // sizeArgs
			iVal2 = IP->args[1].i; // sizeof(retType)
			printf("RET\t%ld,%ld\n", iVal1, iVal2);
			oldSP = SP;
			SP = FP;
			// void* cast error, did a cast to (char*)
			FP = (char*)popa();
			// void* cast error, did a cast to (Instr*)
			IP = (Instr*)popa();
			if (SP - iVal1 < stack)err("not enough stack bytes");
			SP -= iVal1;
			memmove(SP, oldSP - iVal2, iVal2);
			SP += iVal2;
			break;
		case O_STORE:
			iVal1 = IP->args[0].i;
			if (SP - (sizeof(void*) + iVal1) < stack)err("not enough stack bytes for SET");
			// void* cast error, did a cast to (char*)
			aVal1 = (char*) ( * (void**)(SP - ((sizeof(void*) + iVal1))));
			printf("STORE\t%ld\t(%p)\n", iVal1, aVal1);
			memcpy(aVal1, SP - iVal1, iVal1);
			SP -= sizeof(void*) + iVal1;
			IP = IP->next;
			break;
		case O_SUB_C:
			iVal1 = popc();
			iVal2 = popc();
			printf("SUB_C\t(%g-%g -> %g)\n", iVal2, iVal1, iVal2 - iVal1);
			pushd(iVal2 - iVal1);
			IP = IP->next;
			break;
		case O_SUB_D:
			dVal1 = popd();
			dVal2 = popd();
			printf("SUB_D\t(%g-%g -> %g)\n", dVal2, dVal1, dVal2 - dVal1);
			pushc(dVal2 - dVal1);
			IP = IP->next;
			break;
		case O_SUB_I:
			iVal1 = popi();
			iVal2 = popi();
			printf("SUB_I\t(%g-%g -> %g)\n", iVal2, iVal1, iVal2 - iVal1);
			pushi(iVal2 - iVal1);
			IP = IP->next;
			break;
		default:
			vmErr(IP->opcode);
		}
	}
}