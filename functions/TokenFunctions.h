#include <stdio.h>
#include "../structs/Token.h"

#ifndef _TOKENFUNCTIONS_H_
#define _TOKENFUNCTIONS_H_

int getNextToken(int line, FILE* file, Token** tokens, Token** lastToken);
void tkerr(const Token * tk, const char* fmt, ...);
Token* addToken(int code, int line, Token ** tokens, Token ** lastToken);
void showAtoms(Token* tokens);

#endif