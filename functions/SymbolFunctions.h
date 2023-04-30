#include <stdio.h>
#include <stdlib.h>

#include "../structs/Symbol.h"
#include "../structs/Token.h"

#ifndef _SYMBOLFUNCTIONS_H_
#define _SYMBOLFUNCTIONS_H_

void initSymbols(Symbols* symbols);
Symbol* addSymbol(Symbols* symbols, const char* name, int cls);
Symbol* findSymbol(Symbols* symbols, const char* name);
void addVar(Token* crtTk, Token* tkName, Type* t);
void deleteSymbolsAfter(Symbols* symbols, Symbol** start);

#endif