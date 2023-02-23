#include <stdio.h>
#include <cstdarg>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define SAFEALLOC(var,Type) if((var=(Type*)malloc(sizeof(Type)))==NULL)err("not enough memory");

enum { ID, CT_INT, END, BREAK, CHAR, DOUBLE, ELSE, FOR, IF, INT, RETURN, STRUCT, VOID, WHILE }; // token codes

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

int line = 1;
Token* lastToken;
Token* tokens;

void err(const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	fprintf(stderr, "Error on line %d: ", line);
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

void showAtoms() {
	Token* curr = tokens;
	while (curr != NULL) {
		if (curr->code == ID) {
			printf("[ID] %s\n", curr->text);
		}
		else if (curr->code == CT_INT) {
			printf("[CT_INT] %d\n", curr->i);
		}
		else if (curr->code == END) {
			printf("[END] No value\n");
		}
		else if (curr->code == BREAK) {
			printf("[BREAK] No value\n");
		}
		else if (curr->code == CHAR) {
			printf("[CHAR] No value\n");
		}
		else if (curr->code == DOUBLE) {
			printf("[DOUBLE] No value\n");
		}
		else if (curr->code == ELSE) {
			printf("[ELSE] No value\n");
		}
		else if (curr->code == FOR) {
			printf("[FOR] No value\n");
		}
		else if (curr->code == IF) {
			printf("[IF] No value\n");
		}
		else if (curr->code == INT) {
			printf("[INT] No value\n");
		}
		else if (curr->code == RETURN) {
			printf("[RETURN] No value\n");
		}
		else if (curr->code == STRUCT) {
			printf("[STRUCT] No value\n");
		}
		else if (curr->code == VOID) {
			printf("[VOID] No value\n");
		}
		else if (curr->code == WHILE) {
			printf("[WHILE] No value\n");
		}
		curr = curr->next;
	}
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
			{
				ch = fgetc(file);

				if (ch == '\n') {
					line++;
				}
				else if (isalpha(ch) || ch == '_') {
					state = 1;
					currentId = (char*)realloc(currentId, (currentIdLength + 1) * sizeof(char));
					currentId[currentIdLength++] = ch;
				}
				else if (ch == EOF) {
					addToken(END);
					return END;
				}
				else if (ch >= '1' && ch <= '9') {
					state = 3;
					currentId = (char*)realloc(currentId, (currentIdLength + 1) * sizeof(char));
					currentId[currentIdLength++] = ch;
				}
				else if (ch == '0') {
					state = 4;
					currentId = (char*)realloc(currentId, (currentIdLength + 1) * sizeof(char));
					currentId[currentIdLength++] = ch;
				}
			}
				break;
			case 1:
			{
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
			}
				break;
			case 2:
			{
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
			}
				break; 
			case 3:
			{
				ch = fgetc(file);
				if (isdigit(ch)) {
					currentId = (char*)realloc(currentId, (currentIdLength + 1) * sizeof(char));
					currentId[currentIdLength++] = ch;
				}
				else if (ch == '\n') {
					line++;
					state = 6;
				}
				else if (isspace(ch) || ch == EOF) {
					state = 6;
				}
				else {
					err("Invalid character");
				}
			}
				break;
			case 4:
			{
				ch = fgetc(file);
				if (ch == 'x' || ch == 'X') {
					state = 5;
					currentId = (char*)realloc(currentId, (currentIdLength + 1) * sizeof(char));
					currentId[currentIdLength++] = ch;
				}
				else if (ch >= '0' && ch <= '7') {
					currentId = (char*)realloc(currentId, (currentIdLength + 1) * sizeof(char));
					currentId[currentIdLength++] = ch;
				}
				else if (ch == '\n') {
					line++;
					state = 6;
				}
				else if (isspace(ch) || ch == EOF) {
					state = 6;
				}
				else {
					err("Invalid character");
				}
			}
				break;
			case 5:
			{
				ch = fgetc(file);
				if (isalnum(ch)) {
					state = 6;
					currentId = (char*)realloc(currentId, (currentIdLength + 1) * sizeof(char));
					currentId[currentIdLength++] = ch;
				}
				else {
					err("Invalid character");
				}
			}
				break;
			case 6:
				ch = fgetc(file);
				if (isalnum(ch)) {
					currentId = (char*)realloc(currentId, (currentIdLength + 1) * sizeof(char));
					currentId[currentIdLength++] = ch;
				}
				else if (ch == '\n') {
					line++;
					state = 7;
				}
				else if (isspace(ch) || ch == EOF) {
					state = 7;
				}
				else {
					err("Invalid character");
				}
				break;
			case 7:
			{
				currentId[currentIdLength++] = 0; // add string terminator

				tk = addToken(CT_INT);
				tk->i = strtol(currentId, NULL, 0);
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

	showAtoms();	

	return 0;
}