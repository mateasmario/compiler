#include "../structs/Symbol.h"

#ifndef _SYNTACTICANALYZERFUNCTIONS_H_
#define _SYNTACTICANALYZERFUNCTIONS_H_

int consume(int code);
int unit();
int declStruct();
int declVar();
int typeBase();
int arrayDecl(Type& type);
int typeName();
int declFunc();
int funcArg();
int stm();
int stmCompound();
int expr();
int exprAssign();
void analyzeSyntax();

#endif