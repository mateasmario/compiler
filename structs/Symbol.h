#include <stdio.h>
#include <vector>

#include "../enums/SymbolCodes.h"

#ifndef _SYMBOL_H
#define _SYMBOL_H_

typedef struct _Type Type;
typedef struct _Symbol Symbol;
typedef std::vector<Symbol*> Symbols;

struct _Type {
	int typeBase; // TB_*
	Symbol* s; // struct definition for TB_STRUCT
	int nElements; // >0 array of given size, 0=array without size, <0 non array
};

struct _Symbol {
	const char* name; // a reference to the name stored in a token
	int cls; // CLS_*
	int mem; // MEM_*
	_Type type;
	int depth; // 0-global, 1-in function, 2... - nested blocks in function
	union {
		std::vector<Symbol*> args; // used only of functions
		std::vector<Symbol*> members; // used only for structs
	};
	union {
		void* addr; // vm: the memory address for global symbols
		int offset; // vm: the stack offset for local symbols
	};
};

typedef union {
	long int i; // int, char
	double d; // double
	const char* str; // char[]
}CtVal;

typedef struct {
	Type type; // type of the result
	int isLVal; // if it is a LVal
	int isCtVal; // if it is a constant value (int, real, char, char[])
	CtVal ctVal; // the constat value
}RetVal;


#endif