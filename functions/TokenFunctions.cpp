#include <stdio.h>
#include <cstdarg>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "../structs/Token.h"
#include "ErrorFunctions.h"
#include "LexicalAnalyzerFunctions.h"
#include "../enums/TokenCodes.h"

#define SAFEALLOC(var,Type) if((var=(Type*)malloc(sizeof(Type)))==NULL)err("not enough memory");

Token* lastToken;
Token* tokens;

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

int isAllowedSymbol(char ch) {
	if (ch == ',' || ch == ';' || ch == '(' || ch == ')' || ch == '[' || ch == ']' || ch == '{' || ch == '}' || ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '.' || ch == '&' || ch == '|' || ch == '!' || ch == '=' || ch == '<' || ch == '>')
		return true;
	return false;
}

int getNextToken(int *line, FILE* file, Token** tokens, Token** lastToken)
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
				*line = *line + 1;;
			}
			else if (isalpha(ch) || ch == '_') {
				state = 1;
				currentId = (char*)realloc(currentId, (currentIdLength + 1) * sizeof(char));
				currentId[currentIdLength++] = ch;
			}
			else if (ch == '\'') {
				state = 14;
			}
			else if (ch == '"') {
				state = 19;
			}
			else if (ch == ',') {
				state = 22;
			}
			else if (ch == ';') {
				state = 23;
			}
			else if (ch == '(') {
				state = 24;
			}
			else if (ch == ')') {
				state = 25;
			}
			else if (ch == '[') {
				state = 26;
			}
			else if (ch == ']') {
				state = 27;
			}
			else if (ch == '{') {
				state = 28;
			}
			else if (ch == '}') {
				state = 29;
			}
			else if (ch == '+') {
				state = 30;
			}
			else if (ch == '-') {
				state = 31;
			}
			else if (ch == '*') {
				state = 32;
			}
			else if (ch == '/') {
				state = 33;
			}
			else if (ch == '.') {
				state = 34;
			}
			else if (ch == '&') {
				state = 35;
			}
			else if (ch == '|') {
				state = 36;
			}
			else if (ch == '!') {
				state = 37;
			}
			else if (ch == '=') {
				state = 38;
			}
			else if (ch == '<') {
				state = 39;
			}
			else if (ch == '>') {
				state = 40;
			}
			else if (ch == EOF) {
				addToken(END, *line, tokens, lastToken);
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
			else if (!isspace(ch)) {
				lineErr("Invalid character", *line);
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

			int processKeywordResult = processKeyword(currentId, *line, tokens, lastToken);

			if (processKeywordResult != -1) {
				return processKeywordResult;
			}
			else {
				tk = addToken(ID, *line, tokens, lastToken);
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
			else if (ch == '.') {
				state = 8;
				currentId = (char*)realloc(currentId, (currentIdLength + 1) * sizeof(char));
				currentId[currentIdLength++] = ch;
			}
			else if (ch == 'e' || ch == 'E') {
				state = 10;
				currentId = (char*)realloc(currentId, (currentIdLength + 1) * sizeof(char));
				currentId[currentIdLength++] = ch;
			}
			else if (ch == '\n') {
				*line = *line + 1;;
				state = 7;
			}
			else if (isspace(ch) || ch == EOF) {
				state = 7;
			}
			else if (isAllowedSymbol(ch)) {
				ungetc(ch, file);
				state = 7;
			}
			else {
				lineErr("Invalid character", *line);
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
			else if (ch == '.') {
				state = 8;
				currentId = (char*)realloc(currentId, (currentIdLength + 1) * sizeof(char));
				currentId[currentIdLength++] = ch;
			}
			else if (ch == 'e' || ch == 'E') {
				state = 8;
			}
			else if (ch == '\n') {
				*line = *line + 1;;
				state = 7;
			}
			else if (isspace(ch) || ch == EOF) {
				state = 7;
			}
			else if (isAllowedSymbol(ch)) {
				ungetc(ch, file);
				state = 7;
			}
			else {
				lineErr("Invalid character", *line);
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
				lineErr("Invalid character", *line);
			}
		}
		break;
		case 6:
		{
			ch = fgetc(file);
			if (isalnum(ch)) {
				currentId = (char*)realloc(currentId, (currentIdLength + 1) * sizeof(char));
				currentId[currentIdLength++] = ch;
			}
			else if (ch == '\n') {
				*line = *line + 1;;
				state = 7;
			}
			else if (isspace(ch) || ch == EOF) {
				state = 7;
			}
			else if (isAllowedSymbol(ch)) {
				state = 7;
				ungetc(ch, file);
			}
			else {
				lineErr("Invalid character", *line);
			}
		}
		break;
		case 7:
		{
			currentId[currentIdLength++] = 0; // add string terminator

			tk = addToken(CT_INT, *line, tokens, lastToken);
			tk->i = strtol(currentId, NULL, 0);
			return tk->code;
		}
		break;
		case 8:
		{
			ch = fgetc(file);
			if (isdigit(ch)) {
				state = 9;
				currentId = (char*)realloc(currentId, (currentIdLength + 1) * sizeof(char));
				currentId[currentIdLength++] = ch;
			}
			else {
				lineErr("Invalid character", *line);
			}
		}
		break;
		case 9:
		{
			ch = fgetc(file);
			if (isdigit(ch)) {
				currentId = (char*)realloc(currentId, (currentIdLength + 1) * sizeof(char));
				currentId[currentIdLength++] = ch;
			}
			else if (ch == 'e' || ch == 'E') {
				state = 10;
				currentId = (char*)realloc(currentId, (currentIdLength + 1) * sizeof(char));
				currentId[currentIdLength++] = ch;
			}
			else if (ch == '\n') {
				*line = *line + 1;;
				state = 13;
			}
			else if (isspace(ch) || ch == EOF) {
				state = 13;
			}
		}
		break;
		case 10:
		{
			ch = fgetc(file);
			if (ch == '+' || ch == '-') {
				state = 11;
				currentId = (char*)realloc(currentId, (currentIdLength + 1) * sizeof(char));
				currentId[currentIdLength++] = ch;
			}
			else if (isdigit(ch)) {
				state = 12;
				currentId = (char*)realloc(currentId, (currentIdLength + 1) * sizeof(char));
				currentId[currentIdLength++] = ch;
			}
			else if (ch == '\n') {
				*line = *line + 1;;
				state = 11;
			}
			else if (isspace(ch) || ch == EOF) {
				state = 11;
			}
		}
		break;
		case 11:
		{
			ch = fgetc(file);
			if (isdigit(ch)) {
				state = 12;
				currentId = (char*)realloc(currentId, (currentIdLength + 1) * sizeof(char));
				currentId[currentIdLength++] = ch;
			}
			else {
				lineErr("Invalid character", *line);
			}
		}
		break;
		case 12:
		{
			ch = fgetc(file);
			if (isdigit(ch)) {
				currentId = (char*)realloc(currentId, (currentIdLength + 1) * sizeof(char));
				currentId[currentIdLength++] = ch;
			}
			else if (ch == '\n') {
				*line = *line + 1;;
				state = 13;
			}
			else if (isspace(ch) || ch == EOF) {
				state = 13;
			}
			else if (isAllowedSymbol(ch)) {
				ungetc(ch, file);
				state = 13;
			}
			else {
				lineErr("Invalid character", *line);
			}
		}
		break;
		case 13:
		{
			currentId[currentIdLength++] = 0; // add string terminator

			tk = addToken(CT_REAL, *line, tokens, lastToken);
			tk->r = strtod(currentId, NULL);
			return tk->code;
		}
		break;
		case 14:
		{
			ch = fgetc(file);
			if (ch == '\\') {
				state = 15;
				currentId = (char*)realloc(currentId, (currentIdLength + 1) * sizeof(char));
				currentId[currentIdLength++] = ch;
			}
			else if (ch != '\'') {
				state = 17;
				currentId = (char*)realloc(currentId, (currentIdLength + 1) * sizeof(char));
				currentId[currentIdLength++] = ch;
			}
			else {
				lineErr("Invalid character", *line);
			}
		}
		break;
		case 15:
		{
			ch = fgetc(file);
			if (ch == 'a' || ch == 'b' || ch == 'f' || ch == 'n' || ch == 'r' || ch == 'n' || ch == 'r' || ch == 't' || ch == 'v' || ch == '\'' || ch == '?' || ch == '"' || ch == '\\' || ch == '0') {
				state = 16;
				currentId = (char*)realloc(currentId, (currentIdLength + 1) * sizeof(char));
				currentId[currentIdLength++] = ch;
			}
			else if (ch == EOF) {
				lineErr("EOF encountered before escape character", *line);
			}
			else {
				lineErr("Invalid escape character", *line);
			}
		}
		break;
		case 16:
		{
			ch = fgetc(file);
			if (ch == '\'') {
				state = 18;
			}
			else {
				lineErr("Invalid character", *line);
			}
		}
		break;
		case 17:
		{
			ch = fgetc(file);
			if (ch == '\'') {
				state = 18;
				currentId = (char*)realloc(currentId, (currentIdLength + 1) * sizeof(char));
				currentId[currentIdLength++] = ch;
			}
			else {
				lineErr("Invalid character", *line);
			}
		}
		break;
		case 18:
		{
			currentId[currentIdLength++] = 0; // add string terminator

			tk = addToken(CT_CHAR, *line, tokens, lastToken);
			tk->i = currentId[0];
			return tk->code;
		}
		break;
		case 19:
		{
			ch = fgetc(file);
			if (ch == '"') {
				state = 21;
			}
			else if (ch == '\\') {
				state = 20;
				currentId = (char*)realloc(currentId, (currentIdLength + 1) * sizeof(char));
				currentId[currentIdLength++] = ch;
			}
			else if (ch == EOF) {
				lineErr("EOF encountered before escape character", *line);
			}
			else {
				currentId = (char*)realloc(currentId, (currentIdLength + 1) * sizeof(char));
				currentId[currentIdLength++] = ch;
			}
		}
		break;
		case 20:
		{
			ch = fgetc(file);
			if (ch == 'a' || ch == 'b' || ch == 'f' || ch == 'n' || ch == 'r' || ch == 'n' || ch == 'r' || ch == 't' || ch == 'v' || ch == '\'' || ch == '?' || ch == '"' || ch == '\\' || ch == '0') {
				state = 19;
				currentId = (char*)realloc(currentId, (currentIdLength + 1) * sizeof(char));
				currentId[currentIdLength++] = ch;
			}
			else {
				lineErr("Invalid escape character", *line);
			}
		}
		break;
		case 21:
		{
			currentId[currentIdLength++] = 0; // add string terminator

			tk = addToken(CT_STRING, *line, tokens, lastToken);
			tk->text = currentId;
			return tk->code;
		}
		break;
		case 22:
		{
			tk = addToken(COMMA, *line, tokens, lastToken);
			return tk->code;
		}
		break;
		case 23:
		{
			tk = addToken(SEMICOLON, *line, tokens, lastToken);
			return tk->code;
		}
		break;
		case 24:
		{
			tk = addToken(LPAR, *line, tokens, lastToken);
			return tk->code;
		}
		break;
		case 25:
		{
			tk = addToken(RPAR, *line, tokens, lastToken);
			return tk->code;
		}
		break;
		case 26:
		{
			tk = addToken(LBRACKET, *line, tokens, lastToken);
			return tk->code;
		}
		break;
		case 27:
		{
			tk = addToken(RBRACKET, *line, tokens, lastToken);
			return tk->code;
		}
		break;
		case 28:
		{
			tk = addToken(LACC, *line, tokens, lastToken);
			return tk->code;
		}
		break;
		case 29:
		{
			tk = addToken(RACC, *line, tokens, lastToken);
			return tk->code;
		}
		break;
		case 30: 
		{
			tk = addToken(ADD, *line, tokens, lastToken);
			return tk->code;
		}
		break;
		case 31:
		{
			tk = addToken(SUB, *line, tokens, lastToken);
			return tk->code;
		}
		break;
		case 32:
		{
			tk = addToken(MUL, *line, tokens, lastToken);
			return tk->code;
		}
		break;
		case 33:
		{
			ch = fgetc(file);
			if (ch == '/') {
				state = 47;
			}
			else if (ch == '*') {
				state = 48;
			}
			else {
				ungetc(ch, file);
				tk = addToken(DIV, *line, tokens, lastToken);
				return tk->code;
			}
		}
		break;
		case 34:
		{
			tk = addToken(DOT, *line, tokens, lastToken);
			return tk->code;
		}
		break;
		case 35:
		{
			ch = fgetc(file);
			if (ch == '&') {
				state = 41;
			}
			else {
				err("Invalid character");
			}
		}
		break;
		case 36:
		{
			ch = fgetc(file);
			if (ch == '|') {
				state = 42;
			}
			else {
				err("Invalid character");
			}
		}
		break;
		case 37:
		{
			ch = fgetc(file);
			if (ch == '=') {
				state = 43;
			}
			else {
				ungetc(ch, file);
				tk = addToken(NOT, *line, tokens, lastToken);
				return tk->code;
			}
		}
		break;
		case 38:
		{
			ch = fgetc(file);
			if (ch == '=') {
				state = 44;
			}
			else {
				ungetc(ch, file);
				tk = addToken(ASSIGN, *line, tokens, lastToken);
				return tk->code;
			}
		}
		break;
		case 39:
		{
			ch = fgetc(file);
			if (ch == '=') {
				state = 45;
			}
			else {
				ungetc(ch, file);
				tk = addToken(LESS, *line, tokens, lastToken);
				return tk->code;
			}
		}
		break;
		case 40:
		{
			ch = fgetc(file);
			if (ch == '=') {
				state = 46;
			}
			else {
				ungetc(ch, file);
				tk = addToken(GREATER, *line, tokens, lastToken);
				return tk->code;
			}
		}
		break;
		case 41:
		{
			tk = addToken(AND, *line, tokens, lastToken);
			return tk->code;
		}
		break;
		case 42:
		{
			tk = addToken(OR, *line, tokens, lastToken);
			return tk->code;
		}
		break;
		case 43:
		{
			tk = addToken(NOTEQ, *line, tokens, lastToken);
			return tk->code;
		}
		break;
		case 44:
		{
			tk = addToken(EQUAL, *line, tokens, lastToken);
			return tk->code;
		}
		break;
		case 45:
		{
			tk = addToken(LESSEQ, *line, tokens, lastToken);
			return tk->code;
		}
		break;
		case 46:
		{
			tk = addToken(GREATEREQ, *line, tokens, lastToken);
			return tk->code;
		}
		break;
		case 47:
		{
			ch = fgetc(file);
			if (ch == '\n' || ch == EOF) {
				return -10;
			}
		}
		break;
		case 48:
		{
			ch = fgetc(file);
			if (ch == '*') {
				state = 49;
			}
			else if (ch == EOF) {
				err("Comment not closed before EOF");
			}
		}
		break;
		case 49:
		{
			ch = fgetc(file);
			if (ch == '/') {
				state = 50;
			}
			else if (ch != '*') {
				state = 48;
			}
		}
		break;
		case 50:
		{
			return -10;
		}
		break;
		}
		}
}

void showAtoms(Token* tokens) {
	Token* curr = tokens;
	while (curr != NULL) {
		if (curr->code == ID) {
			printf("[ID] %s\n", curr->text);
		}
		else if (curr->code == CT_INT) {
			printf("[CT_INT] %d\n", curr->i);
		}
		else if (curr->code == CT_REAL) {
			printf("[CT_REAL] %f\n", curr->r);
		}
		else if (curr->code == CT_CHAR) {
			printf("[CT_CHAR] %d\n", curr->i);
		}
		else if (curr->code == CT_STRING) {
			printf("[CT_STRING] %s\n", curr->text);
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
		else if (curr->code == COMMA) {
			printf("[COMMA] No value\n");
		}
		else if (curr->code == SEMICOLON) {
			printf("[SEMICOLON] No value\n");
		}
		else if (curr->code == LPAR) {
			printf("[LPAR] No value\n");
		}
		else if (curr->code == RPAR) {
			printf("[RPAR] No value\n");
		}
		else if (curr->code == LBRACKET) {
			printf("[LBRACKET] No value\n");
		}
		else if (curr->code == RBRACKET) {
			printf("[RBRACKET] No value\n");
		}
		else if (curr->code == LACC) {
			printf("[LACC] No value\n");
		}
		else if (curr->code == RACC) {
			printf("[RACC] No value\n");
		}
		else if (curr->code == ADD) {
			printf("[ADD] No value\n");
		}		
		else if (curr->code == SUB) {
			printf("[SUB] No value\n");
		}
		else if (curr->code == MUL) {
			printf("[MUL] No value\n");
		}
		else if (curr->code == DIV) {
			printf("[DIV] No value\n");
		}
		else if (curr->code == DOT) {
			printf("[DOT] No value\n");
		}
		else if (curr->code == AND) {
			printf("[AND] No value\n");
		}
		else if (curr->code == OR) {
			printf("[OR] No value\n");
		}
		else if (curr->code == NOT) {
			printf("[NOT] No value\n");
		}
		else if (curr->code == ASSIGN) {
			printf("[ASSIGN] No value\n");
		}
		else if (curr->code == EQUAL) {
			printf("[EQUAL] No value\n");
		}
		else if (curr->code == NOTEQ) {
			printf("[NOTEQ] No value\n");
		}
		else if (curr->code == LESS) {
			printf("[LESS] No value\n");
		}
		else if (curr->code == LESSEQ) {
			printf("[LESSEQ] No value\n");
		}
		else if (curr->code == GREATER) {
			printf("[GREATER] No value\n");
		}
		else if (curr->code == GREATEREQ) {
			printf("[GREATEREQ] No value\n");
		}
		curr = curr->next;
	}
}