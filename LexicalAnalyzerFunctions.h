#ifndef _LEXICALANALYZERFUNCTIONS_H_
#define _LEXICALANALYZERFUNCTIONS_H_

#include "Token.h"

int processKeyword(char* currentId, int line, Token** tokens, Token** lastToken);

#endif