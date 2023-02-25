#ifndef _LEXICALANALYZERFUNCTIONS_H_
#define _LEXICALANALYZERFUNCTIONS_H_

#include "../structs/Token.h"

int processKeyword(char* currentId, int line, Token** tokens, Token** lastToken);

#endif