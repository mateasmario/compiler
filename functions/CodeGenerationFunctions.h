#ifndef _CODEGENERATIONFUNCTIONS_H_
#define _CODEGENERATIONFUNCTIONS_H_

extern int sizeArgs, offset;

int typeFullSize(Type* type);
int typeBaseSize(Type* type);
int typeFullSize(Type* type);
int typeArgSize(Type* type);
Instr* getRVal(RetVal* rv);
void addCastInstr(Instr* after, Type* actualType, Type* neededType);
Instr* createCondJmp(RetVal* rv);

#endif