#include <stdio.h>
#include <stdlib.h>

#include "../structs/Symbol.h"

#ifndef _SYMBOLFUNCTIONS_H_
#define _SYMBOLFUNCTIONS_H_

void initSymbols(Symbols* symbols);
Symbol* addSymbol(Symbols* symbols, const char* name, int cls);

#endif