#include <stdio.h>
#include <cstdarg>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define SAFEALLOC(var,Type) if((var=(Type*)malloc(sizeof(Type)))==NULL)err("not enough memory");

enum { ID, END, BREAK, CHAR, DOUBLE, ELSE, FOR, IF, INT, RETURN, STRUCT, VOID, WHILE }; // token codes

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

int line;
Token* lastToken;
Token* tokens;

void err(const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	fprintf(stderr, "error: ");
	vfprintf(stderr, fmt, va);
	fputc('\n', stderr);
	va_end(va);
	exit(-1);
}

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

Token* addToken(int code)
{
	Token* tk;
	SAFEALLOC(tk, Token)
		tk->code = code;
	tk->line = line;
	tk->next = NULL;
	if (lastToken) {
		lastToken->next = tk;
	}
	else {
		tokens = tk;
	}
	lastToken = tk;
	return tk;
}

int processKeyword(char* currentId) {
	// Lower string
	for (int i = 0; i < strlen(currentId); i++) {
		currentId[i] = tolower(currentId[i]);
	}

	if (strcmp(currentId, "break") == 0) {
		addToken(BREAK);
		return BREAK;
	}
	else if (strcmp(currentId, "char") == 0) {
		addToken(CHAR);
		return CHAR;
	}
	else if (strcmp(currentId, "double") == 0) {
		addToken(DOUBLE);
		return DOUBLE;
	}
	else if (strcmp(currentId, "else") == 0) {
		addToken(ELSE);
		return ELSE;
	}
	else if (strcmp(currentId, "for") == 0) {
		addToken(FOR);
		return FOR;
	}
	else if (strcmp(currentId, "if") == 0) {
		addToken(IF);
		return IF;
	}
	else if (strcmp(currentId, "int") == 0) {
		addToken(INT);
		return INT;
	}
	else if (strcmp(currentId, "return") == 0) {
		addToken(RETURN);
		return RETURN;
	}
	else if (strcmp(currentId, "struct") == 0) {
		addToken(STRUCT);
		return STRUCT;
	}
	else if (strcmp(currentId, "void") == 0) {
		addToken(VOID);
		return VOID;
	}
	else if (strcmp(currentId, "while") == 0) {
		addToken(WHILE);
		return WHILE;
	}
	else {
		return -1;
	}
}

int getNextToken(FILE* file)
{
	int state = 0;
	char ch;
	Token* tk;

	char* currentId = (char*)malloc(sizeof(char));
	int currentIdLength = 0;

	while (1) {
		switch (state) {
			case 0:
				ch = fgetc(file);

				if (isalpha(ch) || ch == '_') {
					state = 1;
					currentId = (char*)realloc(currentId, (currentIdLength + 1) * sizeof(char));
					currentId[currentIdLength++] = ch;
				}
				else if (ch == EOF) {
					addToken(END);
					return END;
				}
				break;
			case 1:
				ch = fgetc(file);

				if (isalnum(ch) || ch == '_') {
					// state = 1 not needed. It stays in the same state
					currentId = (char*)realloc(currentId, (currentIdLength + 1) * sizeof(char));
					currentId[currentIdLength++] = ch;
				}
				else {
					state = 2;
					ungetc(ch, file);
				}
				break;
			case 2:
				currentId[currentIdLength++] = 0; // add string terminator

				int processKeywordResult = processKeyword(currentId);

				if (processKeywordResult != -1) {
					return processKeywordResult;
				}
				else {
					tk = addToken(ID);
					tk->text = currentId;
					return tk->code;
				}
				break;
		}
	}
}

int main(int argc, char** argv) {
	// Check if a file name has been specified as a command-line argument
	if (argc != 2) {
		err("Please specify a file to compile.");
		return -1;
	}

	// Check if the specified file exists
	FILE* file = fopen(argv[1], "r");
	if (file == NULL) {
		err("Invalid file name.");
		return -2;
	}

	// Lexical analyzer
	int getNextTokenResult;

	do {
		getNextTokenResult = getNextToken(file);
	} while (getNextTokenResult != END);

	return 0;
}