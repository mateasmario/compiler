#include <stdio.h>
#include <stdlib.h>

#include "../structs/Symbol.h"
#include "../functions/EssentialFunctions.h"
#include "../functions/ErrorFunctions.h"

int crtDepth;

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