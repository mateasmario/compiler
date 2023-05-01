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
	*symbols->end++ = s;
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

void addVar(Symbols *symbols, Token* crtTk, Token* tkName, Type* t, Symbol* crtStruct, Symbol* crtFunc, int crtDepth)
{
	Symbol* s;
	if (crtStruct) {
		if (findSymbol(&crtStruct->members, tkName->text, crtDepth))
			tkerr(crtTk, "symbol redefinition: %s", tkName->text);
		s = addSymbol(&crtStruct->members, tkName->text, CLS_VAR, crtDepth);
	}
	else if (crtFunc) {
		s = findSymbol(symbols, tkName->text, crtDepth);
		if (s && s->depth == crtDepth)
			tkerr(crtTk, "symbol redefinition: %s", tkName->text);
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
	Symbol** p = NULL;

	if (start != NULL) {
		for (p = symbols->end - 1; ; p--) {
			if (strcmp((*p)->name, start->name) == 0) {
				break;
			}
		}
		symbols->end = p;
	}
	else {
		initSymbols(symbols);
	}
}