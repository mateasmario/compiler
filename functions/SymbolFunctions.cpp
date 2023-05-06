#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../structs/Symbol.h"
#include "../functions/EssentialFunctions.h"
#include "../functions/ErrorFunctions.h"
#include "../structs/Token.h"
#include "../functions/TokenFunctions.h"

void initSymbols(Symbols* symbols)
{
	symbols->begin = NULL;
	symbols->end = NULL;
	symbols->after = NULL;
}

Symbol* addSymbol(Symbols* symbols, const char* name, int cls, int crtDepth)
{
	Symbol* s;
	if ((symbols)->end == (symbols)->after) { // create more room
		int count = (symbols)->after - (symbols)->begin;
		int n = count * 2; // double the room
		if (n == 0)n = 1; // needed for the initial case
		(symbols)->begin = (Symbol**)realloc((symbols)->begin, n * sizeof(Symbol*));
		if ((symbols)->begin == NULL)err("not enough memory");
		(symbols)->end = (symbols)->begin + count;
		(symbols)->after = (symbols)->begin + n;
	}
	SAFEALLOC(s, Symbol)
		* symbols->end++ = s;
	s->name = name;
	s->cls = cls;
	s->depth = crtDepth;
	return s;
}

Symbol* findSymbol(Symbols* symbols, const char* name, int crtDepth) {
	Symbol** p;

	// If symbols list is empty
	if (symbols->begin == symbols->end) {
		if (symbols->begin != NULL && strcmp((*symbols->begin)->name, name) == 0)
			return (*symbols->begin);
		return NULL;
	}

	// Iterate from end to the beginning of the list
	for (p = symbols->end - 1;; p--) {
		if (strcmp((*p)->name, name) == 0) {
			return *p;
		}

		if (p == symbols->begin) {
			break;
		}
	}

	return NULL;
}

void addVar(Symbols* symbols, Token* crtTk, Token* tkName, Type* t, Symbol* crtStruct, Symbol* crtFunc, int crtDepth)
{
	Symbol* s;
	if (crtStruct) {
		if (findSymbol(&crtStruct->members, tkName->text, crtDepth))
			tkerr(crtTk, "Symbol redefinition: %s.", tkName->text);
		s = addSymbol(&crtStruct->members, tkName->text, CLS_VAR, crtDepth);
	}
	else if (crtFunc) {
		s = findSymbol(symbols, tkName->text, crtDepth);
		if (s && s->depth == crtDepth)
			tkerr(crtTk, "Symbol redefinition: %s.", tkName->text);
		s = addSymbol(symbols, tkName->text, CLS_VAR, crtDepth);
		s->mem = MEM_LOCAL;
	}
	else {
		if (findSymbol(symbols, tkName->text, crtDepth))
			tkerr(crtTk, "symbol redefinition: %s", tkName->text);
		s = addSymbol(symbols, tkName->text, CLS_VAR, crtDepth);
		s->mem = MEM_GLOBAL;
	}
	s->type = *t;
}

void deleteSymbolsAfter(Symbols* symbols, Symbol* start) {
	Symbol** p = symbols->end-1;


	if (start != NULL) {
		while (strcmp((*p)->name, start->name) != 0) {
			if (strcmp((*p)->name, start->name) == 0) {
				break;
			}
			
			p--;
		}
			
		symbols->end[0] = p[1];
	}
	else {
		initSymbols(symbols);
	}
}

Type createType(int typeBase, int nElements)
{
	Type t;
	t.typeBase = typeBase;
	t.nElements = nElements;
	return t;
}

void cast(Type* dst, Type* src, Token* crtTk)
{
	if (src->nElements > -1) {
		if (dst->nElements > -1) {
			if (src->typeBase != dst->typeBase)
				tkerr(crtTk, "an array cannot be converted to an array of another type");
		}
		else {
			tkerr(crtTk, "an array cannot be converted to a non-array");
		}
	}
	else {
		if (dst->nElements > -1) {
			tkerr(crtTk, "a non-array cannot be converted to an array");
		}
	}
	switch (src->typeBase) {
	case TB_CHAR:
	case TB_INT:
	case TB_DOUBLE: 
		switch (dst->typeBase) {
		case TB_CHAR:
		case TB_INT:
		case TB_DOUBLE:
			return;
		}
	case TB_STRUCT:
		if (dst->typeBase == TB_STRUCT) {
			if (src->s != dst->s)
				tkerr(crtTk, "A structure cannot be converted to another one.");
			return;
		}
	}
	tkerr(crtTk, "Incompatible types.");
}

Symbol* addExtFunc(const char* name, Type type, Symbols* symbols, int crtDepth)
{
	Symbol* s = addSymbol(symbols, name, CLS_EXTFUNC, crtDepth);
	s->type = type;
	initSymbols(&s->args);
	return s;
}
Symbol* addFuncArg(Symbol* func, const char* name, Type type, int crtDepth)
{
	Symbol* a = addSymbol(&func->args, name, CLS_VAR, crtDepth);
	a->type = type;
	return a;
}

void addExtFuncs(Symbols* symbols, int crtDepth) {
	Symbol* s;
	
	// void put_s(char s[])
	s = addExtFunc("put_s", createType(TB_VOID, -1), symbols, crtDepth);
	addFuncArg(s, "s", createType(TB_CHAR, 0), crtDepth);

	// void get_s(char s[]) 
	s = addExtFunc("get_s", createType(TB_VOID, -1), symbols, crtDepth);
	addFuncArg(s, "s", createType(TB_CHAR, 0), crtDepth);

	// void put_i(int i) 
	s = addExtFunc("put_i", createType(TB_VOID, -1), symbols, crtDepth);
	addFuncArg(s, "i", createType(TB_INT, 0), crtDepth);

	// int get_i() 
	s = addExtFunc("get_i", createType(TB_INT, 1), symbols, crtDepth);

	// void put_d(double d)
	s = addExtFunc("put_d", createType(TB_VOID, -1), symbols, crtDepth);
	addFuncArg(s, "d", createType(TB_DOUBLE, 0), crtDepth);

	// double get_d() 
	s = addExtFunc("put_d", createType(TB_DOUBLE, 1), symbols, crtDepth);

	// void put_c(char c)
	s = addExtFunc("put_c", createType(TB_VOID, -1), symbols, crtDepth);
	addFuncArg(s, "c", createType(TB_CHAR, 0), crtDepth);

	// char get_c()
	s = addExtFunc("get_c", createType(TB_CHAR, 1), symbols, crtDepth);

	// double seconds()
	s = addExtFunc("seconds", createType(TB_DOUBLE, 1), symbols, crtDepth);
}

Type getArithType(Type* s1, Type* s2) {
	if (s1->typeBase == s2->typeBase) {
		return createType(s1->typeBase, -1);
	}
	else {
		if (s1->typeBase == TB_CHAR && s2->typeBase == TB_INT
			|| s1->typeBase == TB_INT && s2->typeBase == TB_CHAR) { // CHAR + INT => INT
			return createType(TB_INT, -1);
		}
		else if (s1->typeBase == TB_INT && s2->typeBase == TB_DOUBLE
			|| s1->typeBase == TB_DOUBLE && s2->typeBase == TB_INT) { // INT + DOUBLE => DOUBLE
			return createType(TB_DOUBLE, -1);
		}
		else if (s1->typeBase == TB_CHAR && s2->typeBase == TB_DOUBLE
			|| s1->typeBase == TB_DOUBLE && s2->typeBase == TB_CHAR) { // CHAR + DOUBLE => DOUBLE
			return createType(TB_DOUBLE, -1);
		}
	}
}