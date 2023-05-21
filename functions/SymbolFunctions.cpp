#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../structs/Symbol.h"
#include "../functions/EssentialFunctions.h"
#include "../functions/ErrorFunctions.h"
#include "../structs/Token.h"
#include "../functions/TokenFunctions.h"
#include "../functions/VmFunctions.h"
#include "../enums/VmEnums.h"
#include "../functions/CodeGenerationFunctions.h"

void put_i()
{
	printf("#%ld\n", popi());
}

void get_i() {
	int i;
	scanf("%d", &i);
	pushi(i);
}

void put_d() {
	printf("#%f\n", popd());
}

void get_d() {
	double d;
	scanf("%f", &d);
	pushd(d);
}

void put_c() {
	printf("#%c\n", popc());
}

void get_c() {
	char c;
	scanf("%d", &c);
	pushc(c);
}

void put_s() {
	char curr;
	do {
		curr = popc();
		printf("%c", curr);
	} while (curr != 0);
	printf("\n");
}

void get_s() {
	char* s;
	scanf("%s", &s);
	for (int i = strlen(s) - 1; i>=0; i--) {
		pushc(s[i]);
	}
	pushc(0);
}

void seconds() {
	time_t seconds;

	seconds = time(NULL);
	printf("%ld\n", seconds);
}


Symbol* addSymbol(Symbols &symbols, const char* name, int cls, int crtDepth)
{
	Symbol* s;
	SAFEALLOC(s, Symbol)

	s->name = name;
	s->cls = cls;
	s->depth = crtDepth;

	if (symbols.size() == 0)
		new (&(symbols)) std::vector<Symbol*>;

	symbols.push_back(s);

	return s;
}

Symbol* findSymbol(Symbols symbols, const char* name, int crtDepth) {

	for (int i = symbols.size() - 1; i >= 0; i--)
	{
		if (strcmp(symbols[i]->name, name) == 0)
			return symbols[i];
	}

	return NULL;
}

void addVar(Symbols &symbols, Token* crtTk, Token* tkName, Type* t, Symbol *crtStruct, Symbol* crtFunc, int crtDepth)
{
	Symbol* s;
	if (crtStruct) {
		if (findSymbol(crtStruct->members, tkName->text, crtDepth))
			tkerr(crtTk, "Symbol redefinition: %s.", tkName->text);
		s = addSymbol(crtStruct->members, tkName->text, CLS_VAR, crtDepth);
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
	if (crtStruct || crtFunc) {
		s->offset = offset;
	}
	else {
		s->addr = allocGlobal(typeFullSize(&s->type));
	}
	offset += typeFullSize(&s->type);
}

void deleteSymbolsAfter(Symbols &symbols, Symbol* start) {
	int currIndex = symbols.size() - 1;

	while (currIndex >= 0 && strcmp(symbols[currIndex]->name, start->name) != 0) {
		symbols.erase(symbols.begin() + currIndex);
		currIndex--;
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

Symbol* addExtFunc(const char* name, Type type, Symbols &symbols, int crtDepth, void* addr)
{
	Symbol* s = addSymbol(symbols, name, CLS_EXTFUNC, crtDepth);
	s->type = type;
	s->addr = addr;
	return s;
}
Symbol* addFuncArg(Symbol* func, const char* name, Type type, int crtDepth)
{
	Symbol* a = addSymbol(func->args, name, CLS_VAR, crtDepth);
	a->type = type;
	return a;
}

void addExtFuncs(Symbols &symbols, int crtDepth) {
	Symbol* s;
	
	// void put_s(char s[])
	s = addExtFunc("put_s", createType(TB_VOID, -1), symbols, crtDepth, put_s);
	addFuncArg(s, "s", createType(TB_CHAR, 0), crtDepth);

	// void get_s(char s[]) 
	s = addExtFunc("get_s", createType(TB_VOID, -1), symbols, crtDepth, get_s);
	addFuncArg(s, "s", createType(TB_CHAR, 0), crtDepth);

	// void put_i(int i) 
	s = addExtFunc("put_i", createType(TB_VOID, -1), symbols, crtDepth, put_i);
	addFuncArg(s, "i", createType(TB_INT, -1), crtDepth);

	// int get_i() 
	s = addExtFunc("get_i", createType(TB_INT, -1), symbols, crtDepth, get_i);

	// void put_d(double d)
	s = addExtFunc("put_d", createType(TB_VOID, -1), symbols, crtDepth, put_d);
	addFuncArg(s, "d", createType(TB_DOUBLE, -1), crtDepth);

	// double get_d() 
	s = addExtFunc("get_d", createType(TB_DOUBLE, -1), symbols, crtDepth, get_d);

	// void put_c(char c)
	s = addExtFunc("put_c", createType(TB_VOID, -1), symbols, crtDepth, put_c);
	addFuncArg(s, "c", createType(TB_CHAR, -1), crtDepth);

	// char get_c()
	s = addExtFunc("get_c", createType(TB_CHAR, -1), symbols, crtDepth, get_c);

	// double seconds()
	s = addExtFunc("seconds", createType(TB_DOUBLE, -1), symbols, crtDepth, seconds);
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

Symbol* requireSymbol(Symbols symbols, const char* name, int crtDepth) {

	Symbol* s = findSymbol(symbols, name, crtDepth);

	if (s == NULL) {
		tkerr(NULL, "Symbol not found: %s", name);
	}

	return s;
}

void vmTest(Symbols &symbols, int crtDepth)
{
	Instr* L1;
	int* v = (int*)allocGlobal(sizeof(long int));
	addInstrA(O_PUSHCT_A, v);
	addInstrI(O_PUSHCT_I, 3);
	addInstrI(O_STORE, sizeof(long int));
	L1 = addInstrA(O_PUSHCT_A, v);
	addInstrI(O_LOAD, sizeof(long int));
	addInstrA(O_CALLEXT, requireSymbol(symbols, "put_i", crtDepth)->addr);
	addInstrA(O_PUSHCT_A, v);
	addInstrA(O_PUSHCT_A, v);
	addInstrI(O_LOAD, sizeof(long int));
	addInstrI(O_PUSHCT_I, 1);
	addInstr(O_SUB_I);
	addInstrI(O_STORE, sizeof(long int));
	addInstrA(O_PUSHCT_A, v);
	addInstrI(O_LOAD, sizeof(long int));
	addInstrA(O_JT_I, L1);
	addInstr(O_HALT);
}