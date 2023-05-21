#ifndef _VM_H_
#define _VM_H_

typedef struct _Instr {
	int opcode; // O_*
	union {
		long int i; // int, char
		double d;
		void* addr;
	}args[2];
	struct _Instr* last, * next; // links to last, next instructions
}Instr;

#endif