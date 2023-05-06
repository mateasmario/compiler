#include <stdio.h>
#include "../enums/SymbolCodes.h"

#ifndef _SYMBOL_H
#define _SYMBOL_H_

typedef struct _Type Type;
typedef struct _Symbol Symbol;
typedef struct _Symbols Symbols;

struct _Type {
	int typeBase; // TB_*
	Symbol* s; // struct definition for TB_STRUCT
	int nElements; // >0 array of given size, 0=array without size, <0 non array
};

typedef struct _Symbols {
	Symbol** begin; // the beginning of the symbols, or NULL
	Symbol** end; // the position after the last symbol
	Symbol** after; // the position after the allocated space
};

struct _Symbol {
	const char* name; // a reference to the name stored in a token
	int cls; // CLS_*
	int mem; // MEM_*
	_Type type;
	int depth; // 0-global, 1-in function, 2... - nested blocks in function
	union {
		Symbols args; // used only of functions
		Symbols members; // used only for structs
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