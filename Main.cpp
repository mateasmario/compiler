#include <stdio.h>
#include <cstdarg>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "structs/Token.h"
#include "enums/TokenCodes.h"
#include "functions/LexicalAnalyzerFunctions.h"
#include "functions/ErrorFunctions.h"
#include "functions/TokenFunctions.h"

Token* lastToken;
Token* tokens;

int line = 1;

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
		getNextTokenResult = getNextToken(&line, file, &tokens, &lastToken);
	} while (getNextTokenResult != END);

	showAtoms(tokens);	

	return 0;
}