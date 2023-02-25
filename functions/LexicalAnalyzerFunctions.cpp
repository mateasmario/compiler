#include <ctype.h>
#include <string.h>
#include "../enums/TokenCodes.h"
#include "TokenFunctions.h"
#include "../structs/Token.h"

int processKeyword(char* currentId, int line, Token** tokens, Token** lastToken) {
	// Lower string
	for (int i = 0; i < strlen(currentId); i++) {
		currentId[i] = tolower(currentId[i]);
	}

	if (strcmp(currentId, "break") == 0) {
		addToken(BREAK, line, tokens, lastToken);
		return BREAK;
	}
	else if (strcmp(currentId, "char") == 0) {
		addToken(CHAR, line, tokens, lastToken);
		return CHAR;
	}
	else if (strcmp(currentId, "double") == 0) {
		addToken(DOUBLE, line, tokens, lastToken);
		return DOUBLE;
	}
	else if (strcmp(currentId, "else") == 0) {
		addToken(ELSE, line, tokens, lastToken);
		return ELSE;
	}
	else if (strcmp(currentId, "for") == 0) {
		addToken(FOR, line, tokens, lastToken);
		return FOR;
	}
	else if (strcmp(currentId, "if") == 0) {
		addToken(IF, line, tokens, lastToken);
		return IF;
	}
	else if (strcmp(currentId, "int") == 0) {
		addToken(INT, line, tokens, lastToken);
		return INT;
	}
	else if (strcmp(currentId, "return") == 0) {
		addToken(RETURN, line, tokens, lastToken);
		return RETURN;
	}
	else if (strcmp(currentId, "struct") == 0) {
		addToken(STRUCT, line, tokens, lastToken);
		return STRUCT;
	}
	else if (strcmp(currentId, "void") == 0) {
		addToken(VOID, line, tokens, lastToken);
		return VOID;
	}
	else if (strcmp(currentId, "while") == 0) {
		addToken(WHILE, line, tokens, lastToken);
		return WHILE;
	}
	else {
		return -1;
	}
}