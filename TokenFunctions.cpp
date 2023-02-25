#include <stdio.h>
#include <cstdarg>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "Token.h"
#include "ErrorFunctions.h"

#define SAFEALLOC(var,Type) if((var=(Type*)malloc(sizeof(Type)))==NULL)err("not enough memory");

void tkerr(const Token* tk, const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	fprintf(stderr, "error in line %d: ", tk->line);
	vfprintf(stderr, fmt, va);
	fputc('\n', stderr);
	va_end(va);
	exit(-1);
}

Token* addToken(int code, int line, Token** tokens, Token** lastToken)
{
	Token* tk;
	SAFEALLOC(tk, Token)
		tk->code = code;
	tk->line = line;
	tk->next = NULL;
	if (*lastToken) {
		(* lastToken)->next = tk;
	}
	else {
		*tokens = tk;
	}
	*lastToken = tk;
	return tk;
}