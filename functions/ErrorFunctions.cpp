#include <stdio.h>
#include <cstdarg>
#include <stdlib.h>

void err(const char* fmt) {
	fprintf(stderr, "Error: ");
	fputc('\n', stderr);
	exit(-1);
}

void lineErr(const char* fmt, int line)
{
	fprintf(stderr, "Error on line %d: ", line);
	fputc('\n', stderr);
	exit(-1);
}