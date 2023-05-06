#include <stdio.h>
#include <stdlib.h>

#include "../structs/Symbol.h"
#include "../structs/Token.h"

#ifndef _SYMBOLFUNCTIONS_H_
#define _SYMBOLFUNCTIONS_H_

void initSymbols(Symbols* symbols);
Symbol* addSymbol(Symbols* symbols, const char* name, int cls, int crtDepth);
Symbol* findSymbol(Symbols* symbols, const char* name, int crtDepth);
void addVar(Symbols *symbols, Token* crtTk, Token* tkName, Type* t, Symbol* crtStruct, Symbol* crtFunc, int crtDepth);
void deleteSymbolsAfter(Symbols* symbols, Symbol* start);
Type createType(int typeBase, int nElements);
void cast(Type* dst, Type* src, Token* crtTk);
Symbol* addExtFunc(const char* name, Type type, Symbols* symbols, int crtDepth);
Symbol* addFuncArg(Symbol* func, const char* name, Type type, int crtDepth);
void addExtFuncs(Symbols* symbols, int crtDepth);
Type getArithType(Type* s1, Type* s2);
#endif