#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../structs/Symbol.h"
#include "../functions/EssentialFunctions.h"
#include "../functions/ErrorFunctions.h"
#include "../structs/Token.h"

int crtDepth;
Symbol* crtStruct;
Symbol* crtFunc;

void initSymbols(Symbols* symbols)
{
	symbols->begin = NULL;
	symbols->end = NULL;
	symbols->after = NULL;
}

Symbol* addSymbol(Symbols* symbols, const char* name, int cls)
{
	Symbol* s;
	if (symbols->end == symbols->after) { // create more room
		int count = symbols->after - symbols->begin;
		int n = count * 2; // double the room
		if (n == 0)n = 1; // needed for the initial case
		symbols->begin = (Symbol**)realloc(symbols->begin, n * sizeof(Symbol*));
		if (symbols->begin == NULL)err("not enough memory");
		symbols->end = symbols->begin + count;
		symbols->after = symbols->begin + n;
	}
	SAFEALLOC(s, Symbol)
		* symbols->end++ = s;
	s->name = name;
	s->cls = cls;
	s->depth = crtDepth;
	return s;
}

Symbol* findSymbol(Symbols* symbols, const char* name) {
	Symbol** p;

	// Iterate from end to the beginning of the list
	for (p = symbols->end;p != symbols->begin;p--) {
		if (strcmp((*p)->name, name) == 0) {
			return *p;
		}
	}

	return NULL;
}

void addVar(Token* crtTk, Token* tkName, Type* t)
{
	Symbol* s;
	if (crtStruct) {
		if (findSymbol(&crtStruct->members, tkName->text))
			symbolErr(crtTk, "symbol redefinition: %s", tkName->text);
		s = addSymbol(&crtStruct->members, tkName->text, CLS_VAR);
	}
	else if (crtFunc) {
		s = findSymbol(&symbols, tkName->text);
		if (s && s->depth == crtDepth)
			symbolErr(crtTk, "symbol redefinition: %s", tkName->text);
		s = addSymbol(&symbols, tkName->text, CLS_VAR);
		s->mem = MEM_LOCAL;
	}
	else {
		if (findSymbol(&symbols, tkName->text))
			symbolErr(crtTk, "symbol redefinition: %s", tkName->text);
		s = addSymbol(&symbols, tkName->text, CLS_VAR);
		s->mem = MEM_GLOBAL;
	}
	s->type = *t;
}

void deleteSymbolsAfter(Symbols* symbols, Symbol** start) {
	Symbol** p;

	for (p = start; p != symbols->end; ) {
		Symbol** temp = p;
		p--;
		free(temp);
	}

	*(symbols)->end = start[-1];
}