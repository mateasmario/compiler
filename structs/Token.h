#ifndef _TOKEN_H_
#define _TOKEN_H_

typedef struct _Token {
	int code; // code (name)
	union {
		char* text; // used for ID, CT_STRING (dynamically allocated)
		long int i; // used for CT_INT, CT_CHAR
		double r; // used for CT_REAL
	};
	int line; // the input file line
	struct _Token* next; // link to the next token
} Token;

#endif