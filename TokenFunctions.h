#include "Token.h"

#ifndef _TOKENFUNCTIONS_H_
#define _TOKENFUNCTIONS_H_

void tkerr(const Token * tk, const char* fmt, ...);

Token* addToken(int code, int line, Token ** tokens, Token ** lastToken);

#endif