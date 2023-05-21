#include "../structs/Token.h"

#ifndef _ERRORFUNCTIONS_H_
#define _ERRORFUNCTIONS_H_

void err(const char* fmt);
void lineErr(const char* fmt, int line);
void vmErr(int opcode);
void tbErr(int typebase);

#endif