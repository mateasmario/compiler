#ifndef _SYNTACTICANALYZERFUNCTIONS_H_
#define _SYNTACTICANALYZERFUNCTIONS_H_

int consume(int code);
int unit();
int declStruct();
int declVar();
int typeBase();
int arrayDecl();
int typeName();
int declFunc();
int funcArg();
int stm();
int stmCompound();
int expr();
int exprAssign();
void analyzeSyntax();

#endif