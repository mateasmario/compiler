#include <stdio.h>
#include <cstdarg>
#include <stdlib.h>

#include "../structs/Token.h"

void err(const char* fmt) {
	fprintf(stderr, "Error: %s", fmt);
	fputc('\n', stderr);
	exit(-1);
}

void lineErr(const char* fmt, int line)
{
	fprintf(stderr, "Error on line %d: %s", line, fmt);
	fputc('\n', stderr);
	exit(-1);
}

void symbolErr(Token* crtTk, const char* message, char* tokenName) {
	fprintf(stderr, "Token Error: %s", tokenName);
}
