#include "../structs/Vm.h"

#ifndef _VMFUNCTIONS_H_
#define _VMFUNCTIONS_H_

void pushd(double d);
double popd();
void pusha(void* a);
void* popa();
void pushc(char c);
char popc();
void pushi(int i);
int popi();

Instr* createInstr(int opcode);
void insertInstrAfter(Instr* after, Instr* i);
Instr* addInstr(int opcode);
Instr* addInstrAfter(Instr* after, int opcode);
Instr* addInstrA(int opcode, void* addr);
Instr* addInstrI(int opcode, long int val);
Instr* addInstrII(int opcode, long int val1, long int val2);
void deleteInstructionsAfter(Instr* start);
void* allocGlobal(int size);
void run(Instr* IP);

#endif