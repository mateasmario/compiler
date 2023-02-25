#include <stdio.h>
#include <cstdarg>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "Token.h"
#include "TokenFunctions.h"
#include "TokenCodes.h"
#include "LexicalAnalyzerFunctions.h"
#include "ErrorFunctions.h"

Token* lastToken;
Token* tokens;

int line = 1;

void showAtoms() {
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
			else if (ch == '\'') {
				state = 14;
			}
			else if (ch == EOF) {
				addToken(END, line, &tokens, &lastToken);
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
				lineErr("Invalid character", line);
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

			int processKeywordResult = processKeyword(currentId, line, &tokens, &lastToken);

			if (processKeywordResult != -1) {
				return processKeywordResult;
			}
			else {
				tk = addToken(ID, line, &tokens, &lastToken);
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
				line++;
				state = 7;
			}
			else if (isspace(ch) || ch == EOF) {
				state = 7;
			}
			else {
				lineErr("Invalid character", line);
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
				line++;
				state = 7;
			}
			else if (isspace(ch) || ch == EOF) {
				state = 7;
			}
			else {
				lineErr("Invalid character", line);
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
				lineErr("Invalid character", line);
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
				line++;
				state = 7;
			}
			else if (isspace(ch) || ch == EOF) {
				state = 7;
			}
			else {
				lineErr("Invalid character", line);
			}
		}
		break;
		case 7:
		{
			currentId[currentIdLength++] = 0; // add string terminator

			tk = addToken(CT_INT, line, &tokens, &lastToken);
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
				lineErr("Invalid character", line);
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
				line++;
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
				line++;
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
				lineErr("Invalid character", line);
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
				line++;
				state = 13;
			}
			else if (isspace(ch) || ch == EOF) {
				state = 13;
			}
			else {
				lineErr("Invalid character", line);
			}
		}
		break;
		case 13:
		{
			currentId[currentIdLength++] = 0; // add string terminator

			tk = addToken(CT_REAL, line, &tokens, &lastToken);
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
				lineErr("Invalid character", line);
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
			else {
				lineErr("Invalid character", line);
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
				lineErr("Invalid character", line);
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
				lineErr("Invalid character", line);
			}
		}
		break;
		case 18: 
		{
			currentId[currentIdLength++] = 0; // add string terminator

			tk = addToken(CT_CHAR, line, &tokens, &lastToken);
			tk->i = currentId[0];
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