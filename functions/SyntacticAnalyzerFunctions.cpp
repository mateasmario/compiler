#include "../structs/Token.h"
#include "../enums/TokenCodes.h"
#include "../functions/ErrorFunctions.h"
#include "../functions/TokenFunctions.h"
#include "../functions/SymbolFunctions.h"

Token* consumedTk;
int crtDepth;
Symbol *crtStruct;
Symbol *crtFunc;

Token* assignToken;
Token* lastFuncToken;
Token* lastArgToken;

Type* outType;

Symbols symbols;

int typeBase(Type &type);
int stmCompound();
int funcArg();
int exprAssign(RetVal& rv);
int expr(RetVal& rv);
int declVar();
int declStruct();
int declFunc();
int arrayDecl(Type& type);
int exprUnary(RetVal& rv);
int exprRel1(RetVal& rv);
int exprRel(RetVal& rv);
int exprPrimary(RetVal& rv);
int exprPostfix1(RetVal& rv);
int exprPostfix(RetVal& rv);
int exprOr1(RetVal& rv);
int exprMul1();
int exprMul(RetVal& rv);
int exprEq1(RetVal& rv);
int exprEq(RetVal& rv);
int exprMul1(RetVal& rv);
int exprCast(RetVal& rv);
int exprAnd1(RetVal& rv);
int exprAnd(RetVal& rv);
int exprAdd1(RetVal& rv);
int exprAdd(RetVal& rv);
int exprOr(RetVal& rv);

int consume(int code)
{
	if (tokens->code == code) {
		consumedTk = tokens;
		tokens = tokens->next;
		return 1;
	}
	return 0;
}

int unit() {
	Token* startTk = tokens;

	while (1) {
		Token* currPos = tokens;

		if (int result = declStruct()) {
			if (result == -1) {
				tokens = currPos;
				return 0;
			}
		}
		else if (declFunc()) {

		}
		else if (declVar()) {

		}
		else if (stmCompound()) {

		}
		else {
			break;
		}
	}

	if (consume(END)) {
		return 1;
	}


	return 0;
}

int declStruct() {
	Token* startTk = tokens;

	if (consume(STRUCT)) {
		if (consume(ID)) {
			Token* idToken = consumedTk;
			if (consume(LACC)) {

				// Symbolic Table
				if (findSymbol(symbols, idToken->text, crtDepth)) {
					tkerr(idToken, "Symbol redefinition: %s.", idToken->text);
				}
				crtStruct = addSymbol(symbols, idToken->text, CLS_STRUCT, crtDepth);

				while (1) {
					if (declVar()) {

					}
					else {
						break;
					}
				}

				if (consume(RACC)) {
					if (consume(SEMICOLON)) {
						crtStruct = NULL;
						return 1;
					}
					else {
						tkerr(tokens, "Missing ; at the end of struct body.");
					}
				}
				else {
					tkerr(tokens, "Missing } when ending struct body.");
				}
			}
			else if (consume(ID)) { // if typebase for variable declaration, not struct declaration
				Type type;

				Symbol* s = findSymbol(symbols, idToken->text, crtDepth);
				if (s == NULL)tkerr(startTk, "undefined symbol: %s", idToken->text);
				if (s->cls != CLS_STRUCT)tkerr(startTk, "%s is not a struct", idToken->text);

				type.typeBase = TB_STRUCT;
				type.s = s;

				idToken = consumedTk;

				arrayDecl(type);
				addVar(symbols, startTk, idToken, &type, crtStruct, crtFunc, crtDepth);
				while (1) {
					if (consume(COMMA)) {
						if (consume(ID)) {
							idToken = consumedTk;
							arrayDecl(type);
							addVar(symbols, startTk, idToken, &type, crtStruct, crtFunc, crtDepth);
						}
					}
					else {
						break;
					}
				}

				return 1;
			}
			else {
				tkerr(tokens, "Missing { at the beggining of the struct body.");
			}
		}
		else {
			tkerr(tokens, "Missing struct identifier.");
		}
	}
		
	tokens = startTk;
	return 0;
}

int declVar() {
	Token* startTk = tokens;

	Type type;

	if (typeBase(type)) {
		if (consume(ID)) {
			Token* idToken = consumedTk;
			arrayDecl(type);
			addVar(symbols, startTk, idToken, &type, crtStruct, crtFunc, crtDepth);
			while (1) {
				if (consume(COMMA)) {
					if (consume(ID)) {
						idToken = consumedTk;
						arrayDecl(type);
						addVar(symbols, startTk, idToken, &type, crtStruct, crtFunc, crtDepth);
					}
				}
				else {
					break;
				}
			}
			if (consume(SEMICOLON)) {
				return 1;
			}
			else {
				tkerr(tokens, "Missing ; or syntax error in type base declaration.");
			}
		}
		else {
			tkerr(tokens, "Missing type base identifier.");
		}
	}

	tokens = startTk;
	return 0;
}

int typeBase(Type& type) {
	Token* startTk = tokens;

	if (consume(INT)) {
		type.typeBase = TB_INT;
		return 1;
	}
	else if (consume(DOUBLE)) {
		type.typeBase = TB_DOUBLE;
		return 1;
	}
	else if (consume(CHAR)) {
		type.typeBase = TB_CHAR;
		return 1;
	}
	else if (consume(STRUCT)) {
		if (consume(ID)) {
			Token* idToken = consumedTk;

			Symbol* s = findSymbol(symbols, idToken->text, crtDepth);
			if (s == NULL)tkerr(startTk, "undefined symbol: %s", idToken->text);
			if (s->cls != CLS_STRUCT)tkerr(startTk, "%s is not a struct", idToken->text);
			
			type.typeBase = TB_STRUCT;
			type.s = s;
			
			return 1;
		}
		else {
			tkerr(tokens, "Missing struct identifier.");
		}
	}
		
	tokens = startTk;
	return 0;
}

int arrayDecl(Type& type) {
	Token* startTk = tokens;

	// For now, do not return the real number of elements
	type.nElements = -1;

	Token* arrayId = consumedTk;

	if (consume(LBRACKET)) {
		RetVal rv;
		if (expr(rv)) {
			if (!rv.isCtVal) {
				tkerr(tokens, "The array size is not a constant.");
			}
			if (rv.type.typeBase != TB_INT) {
				tkerr(tokens, "The array size is not an integer.");
			}
			type.nElements = rv.ctVal.i;
		}
		else {
			type.nElements = 0;
		}

		if (consume(RBRACKET)) {
			return 1;
		}
		else {
			tkerr(tokens, "Missing ] at the end of the array declaration.");
		}
	}
		
	tokens = startTk;
	return 0;
}

int typeName(Type &type) {
	Token* startTk = tokens;

	if (typeBase(type)) {
		arrayDecl(type);
		return 1;
	}
		
	tokens = startTk;
	return 0;
}

int declFunc() {
	Token* startTk = tokens;
	Type type;

	if (typeBase(type)) {
		if (consume(MUL)) {
			type.nElements = 0;
		}
		else {
			type.nElements = -1;
		}
		if (consume(ID)) {
			Token* idToken = consumedTk;
			if (consume(LPAR)) {
				if (findSymbol(symbols, idToken->text, crtDepth))
					tkerr(startTk, "Symbol redefinition: %s.", idToken->text);
				crtFunc = addSymbol(symbols, idToken->text, CLS_FUNC, crtDepth);
				crtFunc->type = type;
				crtDepth++;

				if (funcArg()) {
					while (1) {
						if (consume(COMMA)) {
							if (funcArg()) {

							}
							else {
								tkerr(tokens, "Expected function argument after ,");
							}
						}
						else {
							break;
						}
					}
				}
				if (consume(RPAR)) {
					crtDepth--;
					if (stmCompound()) {
						return 1;
					}
					else {
						tkerr(tokens, "Missing statement compound after function declaration.");
					}
					deleteSymbolsAfter(symbols, crtFunc);
					crtFunc = NULL;
				}
				else {
					tkerr(tokens, "Expected ) at the end of the function declaration.");
				}
			}
			else {
				Token* idToken = consumedTk;
				arrayDecl(type);
				addVar(symbols, startTk, idToken, &type, crtStruct, crtFunc, crtDepth);
				arrayDecl(type);
				while (1) {
					if (consume(COMMA)) {
						if (consume(ID)) {
							idToken = consumedTk;
							arrayDecl(type);
							addVar(symbols, startTk, idToken, &type, crtStruct, crtFunc, crtDepth);
							arrayDecl(type);
						}
					}
					else {
						break;
					}
				}
				if (consume(SEMICOLON)) {
					return 1;
				}
				else {
					tkerr(tokens, "Missing ; or syntax error in type base declaration.");
				}
			}
		}
		else {
			tkerr(tokens, "Expected identifier after type base.");
		}
	}
	if (consume(VOID)) {
		type.typeBase = TB_VOID;
		if (consume(ID)) {
			Token* idToken = consumedTk;
			if (consume(LPAR)) {
				if (findSymbol(symbols, idToken->text, crtDepth))
					tkerr(startTk, "Symbol redefinition: %s.", idToken->text);
				crtFunc = addSymbol(symbols, idToken->text, CLS_FUNC, crtDepth);
				crtFunc->type = type;
				crtDepth++;

				if (funcArg()) {
					while (1) {
						if (consume(COMMA)) {
							if (funcArg()) {

							}
							else {
								tkerr(tokens, "Expected function argument after ,");
							}
						}
						else {
							break;
						}
					}
				}
				if (consume(RPAR)) {
					crtDepth--;
					if (stmCompound()) {
						return 1;
					}
					else {
						tkerr(tokens, "Missing statement compound after function declaration.");
					}
					deleteSymbolsAfter(symbols, crtFunc);
					crtFunc = NULL;
				}
				else {
					tkerr(tokens, "Expected ) at the end of the function declaration.");
				}
			}
			else {
				tkerr(tokens, "Expected ( after the function identifier.");
			}
		}
		else {
			tkerr(tokens, "Expected identifier after type base.");
		}
	}
		
	tokens = startTk;
	return 0;
}

int funcArg() {
	Token* startTk = tokens;

	Type type;

	if (typeBase(type)) {
		if (consume(ID)) {
			Token* tokenId = consumedTk;
			arrayDecl(type);

			Symbol* s = addSymbol(symbols, tokenId->text, CLS_VAR, crtDepth);
			s->mem = MEM_ARG;
			s->type = type;
			s = addSymbol(crtFunc->args, tokenId->text, CLS_VAR, crtDepth);
			s->mem = MEM_ARG;
			s->type = type;

			return 1;
		}
		else {
			tkerr(tokens, "Missing identifier in function arguments specification.");
		}
	}
		
	tokens = startTk;
	return 0;
}

int stm() {
	Token* startTk = tokens;
	RetVal rv, rv1, rv2;

	if (stmCompound()) {
		return 1;
	}
	if (consume(IF)) {
		if (consume(LPAR)) {
			lastFuncToken = NULL;
			if (expr(rv)) {
				if (rv.type.typeBase == TB_STRUCT) {
					tkerr(tokens, "A structure cannot be logically tested.");
				}
				if (consume(RPAR)) {
					if (stm()) {
						if (consume(ELSE)) {
							if (stm()) {
								return 1;
							}
							else {
								tkerr(tokens, "Missing statement after ELSE keyword.");
							}
						}
						return 1;
					}
					else {
						tkerr(tokens, "Missing statement after condition in IF.");
					}
				}
				else {
					tkerr(tokens, "Missing ) after condition in IF.");
				}
			}
			else {
				tkerr(tokens, "Missing IF condition.");
			}
		}
		else {
			tkerr(tokens, "Missing ( after IF keyword.");
		}
	}
	if (consume(WHILE)) {
		if (consume(LPAR)) {
			lastFuncToken = NULL;
			if (expr(rv)) {
				if (rv.type.typeBase == TB_STRUCT) {
					tkerr(tokens, "A structure cannot be logically tested.");
				}
				if (consume(RPAR)) {
					if (stm()) {
						return 1;
					}
					else {
						tkerr(tokens, "Missing statement after condition in WHILE.");
					}
				}
				else {
					tkerr(tokens, "Missing ) after condition in WHILE.");
				}
			}
			else {
				tkerr(tokens, "Missing WHILE condition.");
			}
		}
		else {
			tkerr(tokens, "Missing ( after WHILE keyword.");
		}
	}
	if (consume(FOR)) {
		if (consume(LPAR)) {
			lastFuncToken = NULL;
			expr(rv1);

			if (consume(SEMICOLON)) {
				expr(rv2);
				if (rv2.type.typeBase == TB_STRUCT) {
					tkerr(tokens, "A structure cannot be logically tested.");
				}
				if (consume(SEMICOLON)) {
					expr(rv);

					if (consume(RPAR)) {
						if (stm()) {
							return 1;
						}
					}
					else {
						tkerr(tokens, "Missing ) after conditions in FOR.");
					}
				}
				else {
					tkerr(tokens, "Missing ; after expression in FOR.");
				}
			}
			else {
				tkerr(tokens, "Missing ; after expression in FOR.");
			}
		}
		else {
			tkerr(tokens, "Missing ( after FOR keyword.");
		}
	}
	if (consume(BREAK)) {
		if (consume(SEMICOLON)) {
			return 1;
		}
		else {
			tkerr(tokens, "Missing ; after BREAK keyword.");
		}
	}
	if (consume(RETURN)) {
		if (expr(rv)) {
			if (crtFunc->type.typeBase == TB_VOID) {
				tkerr(tokens, "A void function cannot return a value.");
			}
			cast(&crtFunc->type, &rv.type, tokens);
		}
		if (consume(SEMICOLON)) {
			return 1;
		}
		else {
			tkerr(tokens, "Missing ; after RETURN statement.");
		}
	}
	else if (expr(rv)) {
		if (consume(SEMICOLON)) {
			return 1;
		}
		else {
			tkerr(tokens, "Missing ; or syntax error in statement.");
		}
	}
	else {
		if (consume(SEMICOLON)) {
			return 1;
		}
	}

	tokens = startTk;
	return 0;
}

int stmCompound() {
	Token* startTk = tokens;
	Symbol* start = symbols.end()[-1];

	if (consume(LACC)) {
		crtDepth++;
		while (1) {
			if (declVar()) {

			}
			else if (stm()) {

			}
			else {
				break;
			}
		}

		if (consume(RACC)) {
			crtDepth--;
			deleteSymbolsAfter(symbols, start);
			return 1;
		}
		else {
			tkerr(tokens, "Missing } at the end of the statement.");
		}
	}

	tokens = startTk;
	return 0;
}

int expr(RetVal &rv) {
	Token* startTk = tokens;

	if (exprAssign(rv)) {
		return 1;
	}

	tokens = startTk;
	return 0;
}

int exprAssign(RetVal &rv) {
	Token* startTk = tokens;
	RetVal rve;
	
	if (exprOr(rv)) {
		if (consume(ASSIGN)) {
			if (exprAssign(rve)) {
				if (!rv.isLVal) {
					tkerr(tokens, "Cannot assign to a non-lval.");
				}
				if (rv.type.nElements > -1 || rve.type.nElements > -1) {
					tkerr(tokens, "The arrays cannot be assigned.");
				}
				cast(&rv.type, &rve.type, tokens);
				rv.isCtVal = rv.isLVal = 0;
			}
			return 1;
		}
		return 1;
	}

	tokens = startTk;
	return 0;
}

// exprOr ::= exprOr OR exprAnd | exprAnd
// [Recursivitate Stanga]
// exprOr ::= exprAnd exprOr1
// exprOr1 ::= OR exprAnd exprOr1 | EPS
int exprOr(RetVal &rv) {
	Token* startTk = tokens;

	if (exprAnd(rv)) {
		if (exprOr1(rv)) {
			return 1;
		}
		else {
			tkerr(tokens, "Missing OR expression.");
		}
	}

	tokens = startTk;
	return 0;
}

int exprOr1(RetVal &rv) {
	Token* startTk = tokens;
	RetVal rve;

	if (consume(OR)) {
		if (exprAnd(rve)) {
			/* use (rv, rve); return rv */
			if (rv.type.typeBase == TB_STRUCT || rve.type.typeBase == TB_STRUCT) {
				tkerr(tokens, "A structure cannot be logically tested.");
				rv.type = createType(TB_INT, -1);
				rv.isCtVal = rv.isLVal = 0;
			}

			if (exprOr1(rv)) {
				return 1;
			}
			else {
				tkerr(tokens, "Missing OR expression.");

				Token* startTk = tokens;
				return 0;
			}
		}
		else {
			tkerr(tokens, "Missing AND expression.");

			tokens = startTk;
			return 0;
		}
	}
	else
		return 1;
}

// exprAnd ::= exprAnd AND exprEq | exprEq
// [Recursivitate Stanga]
// exprAnd ::= exprEq exprAnd1
// exprAnd1 ::= AND exprEq exprAnd1 | EPS
int exprAnd(RetVal &rv) {
	Token* startTk = tokens;

	if (exprEq(rv)) {
		if (exprAnd1(rv)) {
			return 1;
		}
		else {
			tkerr(tokens, "Missing AND expression.");
		}
	}

	tokens = startTk;
	return 0;
}

int exprAnd1(RetVal &rv) {
	Token* startTk = tokens;
	RetVal rve;

	if (consume(AND)) {
		if (exprEq(rve)) {
			/* use (rv, rve); return rv */
			if (rv.type.typeBase == TB_STRUCT || rve.type.typeBase == TB_STRUCT) {
				tkerr(tokens, "A structure cannot be logically tested.");
				rv.type = createType(TB_INT, -1);
				rv.isCtVal = rv.isLVal = 0;
			}
			if (exprAnd1(rv)) {
				return 1;
			}
			else {
				tkerr(tokens, "Missing AND expression.");

				tokens = startTk;
				return 0;
			}
		}
		else {
			tkerr(tokens, "Missing EQUAL/NOT-EQUAL expression.");

			tokens = startTk;
			return 0;
		}
	}
	else {
		return 1;
	}
}

// exprEq ::= exprEq (EQUAL|NOTEQ) exprRel | exprRel
// [Recursivitate Stanga]
// exprEq ::= exprRel exprEq1
// exprEq1 ::= (EQUAL|NOTEQ) exprRel exprEq1 | EPS
int exprEq(RetVal &rv) {
	Token* startTk = tokens;

	if (exprRel(rv)) {
		if (exprEq1(rv)) {
			return 1;
		}
		else {
			tkerr(tokens, "Missing EQUAL/NOT-EQUAL expression.");
		}
	}

	tokens = startTk;
	return 0;
}

int exprEq1(RetVal &rv) {
	Token* startTk = tokens;
	RetVal rve;

	if (consume(EQUAL)) {
		if (exprRel(rve)) {
			/* use (rv, rve); return rv */
			if (rv.type.typeBase == TB_STRUCT || rve.type.typeBase == TB_STRUCT) {
				tkerr(tokens, "A structure cannot be compared.");
			rv.type = createType(TB_INT, -1);
			rv.isCtVal = rv.isLVal = 0;
			}

			if (exprEq1(rv)) {
				return 1;
			}
			else {
				tkerr(tokens, "Missing EQUAL/NOT-EQUAL expression.");

				tokens = startTk;
				return 0;
			}
		}
		else {
			tkerr(tokens, "Missing relational expression.");

			tokens = startTk;
			return 0;
		}
	}
	else if (consume(NOTEQ)) {
		if (exprRel(rv)) {
			if (exprEq1(rv)) {
				return 1;
			}
			else {
				tkerr(tokens, "Missing EQUAL/NOT-EQUAL expression.");

				tokens = startTk;
				return 0;
			}
		}
		else {
			tkerr(tokens, "Missing relational expression.");

			tokens = startTk;
			return 0;
		}
	}
	else {
		return 1;
	}
}


// exprRel ::= exprRel (LESS|LESSEQ|GREATER|GREATEREQ) exprAdd | exprAdd
// [Recursivitate Stanga]
// exprRel ::= exprAdd exprRel1
// exprRel1 ::= (LESS|LESSEQ|GREATER|GREATEREQ) exprAdd exprRel1 | EPS
int exprRel(RetVal &rv) {
	Token* startTk = tokens;

	if (exprAdd(rv)) {
		if (exprRel1(rv)) {
			return 1;
		}
		else {
			tkerr(tokens, "Missing relational expression.");
		}
	}

	tokens = startTk;
	return 0;
}

int exprRel1(RetVal &rv) {
	Token* startTk = tokens;
	RetVal rve;

	if (consume(LESS)) {
		if (exprAdd(rve)) {
			/* use (rv, rve); return rv */
			if (rv.type.nElements > -1 || rve.type.nElements > -1) {
				tkerr(tokens, "An array cannot be compared.");
			}
			if (rv.type.typeBase == TB_STRUCT || rve.type.typeBase == TB_STRUCT) {
				tkerr(tokens, "A structure cannot be compared.");
				rv.type = createType(TB_INT, -1);
				rv.isCtVal = rv.isLVal = 0;
			}

			if (exprRel1(rv)) {
				return 1;
			}
			else {
				tkerr(tokens, "Missing relational expression.");

				tokens = startTk;
				return 0;
			}
		}
		else {
			tkerr(tokens, "Missing addition/subtraction expression.");

			tokens = startTk;
			return 0;
		}
	}
	else if (consume(LESSEQ)) {
		if (exprAdd(rve)) {
			/* use (rv, rve); return rv */
			if (rv.type.nElements > -1 || rve.type.nElements > -1) {
				tkerr(tokens, "An array cannot be compared.");
			}
			if (rv.type.typeBase == TB_STRUCT || rve.type.typeBase == TB_STRUCT) {
				tkerr(tokens, "A structure cannot be compared.");
				rv.type = createType(TB_INT, -1);
				rv.isCtVal = rv.isLVal = 0;
			}
			if (exprRel1(rv)) {
				return 1;
			}
			else {
				tkerr(tokens, "Missing relational expression.");

				tokens = startTk;
				return 0;
			}
		}
		else {
			tkerr(tokens, "Missing addition/subtraction expression.");

			tokens = startTk;
			return 0;
		}
	}
	else if (consume(GREATER)) {
		if (exprAdd(rve)) {
			/* use (rv, rve); return rv */
			if (rv.type.nElements > -1 || rve.type.nElements > -1) {
				tkerr(tokens, "An array cannot be compared.");
			}
			if (rv.type.typeBase == TB_STRUCT || rve.type.typeBase == TB_STRUCT) {
				tkerr(tokens, "A structure cannot be compared.");
				rv.type = createType(TB_INT, -1);
				rv.isCtVal = rv.isLVal = 0;
			}
			if (exprRel1(rv)) {
				return 1;
			}
			else {
				tkerr(tokens, "Missing relational expression.");

				tokens = startTk;
				return 0;
			}
		}
		else {
			tkerr(tokens, "Missing addition/subtraction expression.");

			tokens = startTk;
			return 0;
		}
	}
	else if (consume(GREATEREQ)) {
		if (exprAdd(rve)) {
			/* use (rv, rve); return rv */
			if (rv.type.nElements > -1 || rve.type.nElements > -1) {
				tkerr(tokens, "An array cannot be compared.");
			}
			if (rv.type.typeBase == TB_STRUCT || rve.type.typeBase == TB_STRUCT) {
				tkerr(tokens, "A structure cannot be compared.");
				rv.type = createType(TB_INT, -1);
				rv.isCtVal = rv.isLVal = 0;
			}
			if (exprRel1(rv)) {
				return 1;
			}
			else {
				tkerr(tokens, "Missing relational expression.");

				tokens = startTk;
				return 0;
			}
		}
		else {
			tkerr(tokens, "Missing addition/subtraction expression.");

			tokens = startTk;
			return 0;
		}
	}
	else {
		return 1;
	}
}

// exprAdd ::= exprAdd (ADD | SUB) exprMul | exprMul
// [Recursivitate Stanga]
// exprAdd ::= exprMul exprAdd1
// exprAdd1 ::= (ADD | SUB) exprMul exprAdd1 | EPS
int exprAdd(RetVal &rv) {
	Token* startTk = tokens;

	if (exprMul(rv)) {
		if (exprAdd1(rv)) {
			return 1;
		}
		else {
			tkerr(tokens, "Missing addition/subtraction expression.");
		}
	}

	tokens = startTk;
	return 0;
}

int exprAdd1(RetVal &rv) {
	Token* startTk = tokens;
	RetVal rve;

	if (consume(ADD)) {
		if (exprMul(rve)) {
			/* use (rv, rve); return rv */
			if (rv.type.nElements > -1 || rve.type.nElements > -1) {
				tkerr(tokens, "An array cannot be added or subtracted.");
			}
			if (rv.type.typeBase == TB_STRUCT || rve.type.typeBase == TB_STRUCT) {
				tkerr(tokens, "A structure cannot be added or subtracted.");
			}
			rv.type = getArithType(&rv.type, &rve.type);
			rv.isCtVal = rv.isLVal = 0;

			if (exprAdd1(rv)) {
				return 1;
			}
			else {
				tkerr(tokens, "Missing addition/subtraction expression.");

				tokens = startTk;
				return 0;
			}
		}
		else {
			tkerr(tokens, "Missing multiplication expression.");

			tokens = startTk;
			return 0;
		}
	}
	else if (consume(SUB)) {
		if (exprMul(rve)) {
			/* use (rv, rve); return rv */
			if (rv.type.nElements > -1 || rve.type.nElements > -1) {
				tkerr(tokens, "An array cannot be added or subtracted.");
			}
			if (rv.type.typeBase == TB_STRUCT || rve.type.typeBase == TB_STRUCT) {
				tkerr(tokens, "A structure cannot be added or subtracted.");
			}
			rv.type = getArithType(&rv.type, &rve.type);
			rv.isCtVal = rv.isLVal = 0;

			if (exprAdd1(rv)) {
				return 1;
			}
			else {
				tkerr(tokens, "Missing addition/subtraction expression.");

				tokens = startTk;
				return 0;
			}
		}
		else {
			tkerr(tokens, "Missing multiplication expression.");

			tokens = startTk;
			return 0;
		}
	}
	else {
		return 1;
	}
}

// exprMul ::= exprMul (MUL | DIV) exprCast | exprCast
// [Recursivitate Stanga]
// exprMul ::= exprCast exprMul1
// exprMul1 ::= (MUL | DIV) exprCast exprMul1 | EPS
int exprMul(RetVal &rv) {
	Token* startTk = tokens;

	if (exprCast(rv)) {
		if (exprMul1(rv)) {
			return 1;
		}
		else {
			tkerr(tokens, "Missing multiplication expression.");
		}
	}

	tokens = startTk;
	return 0;
}

int exprMul1(RetVal &rv) {
	Token* startTk = tokens;
	RetVal rve;

	if (consume(MUL)) {
		if (exprCast(rve)) {
			/* use (rv, rve); return rv */
			if (rv.type.nElements > -1 || rve.type.nElements > -1) {
				tkerr(tokens, "An array cannot be multiplied or divided.");
			}
			if (rv.type.typeBase == TB_STRUCT || rve.type.typeBase == TB_STRUCT) {
				tkerr(tokens, "A structure cannot be multiplied or divided.");
			}
			rv.type = getArithType(&rv.type, &rve.type);
			rv.isCtVal = rv.isLVal = 0;
		
			if (exprMul1(rv)) {
				return 1;
			}
			else {
				tkerr(tokens, "Missing multiplication expression.");

				tokens = startTk;
				return 0;
			}
		}
		else {
			tkerr(tokens, "Missing cast expression.");

			tokens = startTk;
			return 0;
		}
	}
	else if (consume(DIV)) {
		if (exprCast(rve)) {
			/* use (rv, rve); return rv */
			if (rv.type.nElements > -1 || rve.type.nElements > -1) {
				tkerr(tokens, "An array cannot be multiplied or divided.");
			}
			if (rv.type.typeBase == TB_STRUCT || rve.type.typeBase == TB_STRUCT) {
				tkerr(tokens, "A structure cannot be multiplied or divided.");
			}
			rv.type = getArithType(&rv.type, &rve.type);
			rv.isCtVal = rv.isLVal = 0;

			if (exprMul1(rv)) {
				return 1;
			}
			else {
				tkerr(tokens, "Missing multiplication expression.");

				tokens = startTk;
				return 0;
			}
		}
		else {
			tkerr(tokens, "Missing cast expression.");

			tokens = startTk;
			return 0;
		}
	}
	else {
		return 1;
	}
}

int exprCast(RetVal &rv) {
	Token* startTk = tokens;
	RetVal rve;

	Type type;

	if (consume(LPAR)) {
		if (typeName(type)) {
			if (consume(RPAR)) {
				if (exprCast(rve)) {
					cast(&type, &rve.type, tokens);
					rv.type = type;
					rv.isCtVal = rv.isLVal = 0;

					return 1;
				}
				else {
					tkerr(tokens, "Missing cast expression.");
				}
			}
			else {
				tkerr(tokens, "Missing ) after type name.");
			}
		}
		else if (expr(rv)) {
			while (1) {
				if (consume(COMMA)) {
					if (expr(rv)) {

					}
					else {
						tkerr(tokens, "Missing expression after ,");
					}
				}
				else {
					break;
				}
			}
			if (consume(RPAR)) {
				return 1;
			}
		}
		else {
			tkerr(tokens, "Missing type name or expression after (.");
		}
	}
	else if (exprUnary(rv)) {
		return 1;
	}

	tokens = startTk;
	return 0;
}

int exprUnary(RetVal &rv) {
	Token* startTk = tokens;

	if (consume(SUB)) {
		if (exprUnary(rv)) {
			if (rv.type.nElements >= 0) {
				tkerr(tokens, "Unary '-' cannot be applied to an array.");
			}
			if (rv.type.typeBase == TB_STRUCT) {
				tkerr(tokens, "Unary '-' cannot be applied to a struct.");
			}
			rv.isCtVal = rv.isLVal = 0;

			return 1;
		}
		else {
			tkerr(tokens, "Missing unary expression.");
		}
	}
	else if (consume(NOT)) {
		if (exprUnary(rv)) {
			if (rv.type.typeBase == TB_STRUCT) {
				tkerr(tokens, "'!' cannot be applied to a struct.");
			}
			rv.type = createType(TB_INT, -1);
			rv.isCtVal = rv.isLVal = 0;

			return 1;
		}
		else {
			tkerr(tokens, "Missing unary expression.");
		}
	}
	else if (exprPostfix(rv)) {
		return 1;
	}

	tokens = startTk;
	return 0;
}

// exprPostfix ::= 
// [Recursivitate Stanga]
// exprPostfix ::= exprPrimary exprPostfix1
// exprPostfix1 ::= LBRACKET expr RBRACKET exprPostfix1 | DOT ID exprPostfix1 | EPS
int exprPostfix(RetVal &rv) {
	Token* startTk = tokens;

	if (exprPrimary(rv)) {
		if (exprPostfix1(rv)) {
			return 1;
		}
		else {
			tkerr(tokens, "Missing postfix expression.");
		}
	}

	tokens = startTk;
	return 0;
}

int exprPostfix1(RetVal &rv) {
	Token* startTk = tokens;
	RetVal rve;

	if (consume(LBRACKET)) {
		if (expr(rve)) {
			/* use (rv, rve); return rv */
			if (rv.type.nElements < 0) {
				tkerr(tokens, "Only an array can be indexed.");
			}
			Type typeInt = createType(TB_INT, -1);
			cast(&typeInt, &rve.type, tokens);
			rv.type = rv.type;
			rv.type.nElements = -1;
			rv.isLVal = 1;
			rv.isCtVal = 0;

			if (consume(RBRACKET)) {
				if (exprPostfix1(rv)) {
					return 1;
				}
				else {
					tkerr(tokens, "Missing postfix expression.");

					tokens = startTk;
					return 0;
				}
			}
			else {
				tkerr(tokens, "Missing ] after expression.");

				tokens = startTk;
				return 0;
			}
		}
		else {
			tkerr(tokens, "Missing expression.");

			tokens = startTk;
			return 0;
		}
	}
	else if (consume(DOT)) {
		if (consume(ID)) {
			Token* tokenId = consumedTk;

			Symbol* sStruct = rv.type.s;
			Symbol* sMember = findSymbol(sStruct->members, tokenId->text, crtDepth);
			if (!sMember)
				tkerr(tokens, "Struct %s does not have a member %s.", sStruct->name, tokenId->text);
			rv.type = sMember->type;
			rv.isLVal = 1;
			rv.isCtVal = 0;

			if (exprPostfix1(rv)) {
				return 1;
			}
			else {
				tkerr(tokens, "Missing postfix expression.");

				tokens = startTk;
				return 0;
			}
		}
		else {
			tkerr(tokens, "Missing identifier.");

			tokens = startTk;
			return 0;
		}
	}
	else {
		return 1;
	}
}

int exprPrimary(RetVal &rv) {
	Token* startTk = tokens;

	Type type;

	if (consume(ID)) {
		assignToken = consumedTk;
		
		Token* idToken = consumedTk;
		RetVal arg;

		Symbol* s = findSymbol(symbols, idToken->text, crtDepth);
		if (!s) {
			tkerr(tokens, "Undefined symbol %s.", idToken->text);
		}
		rv.type = s->type;
		rv.isCtVal = 0;
		rv.isLVal = 1;

		if (consume(LPAR)) {
			if (s->cls != CLS_FUNC && s->cls != CLS_EXTFUNC)
				tkerr(tokens, "Call of the non-function %s.", idToken->text);

			Token* ct = tokens;

			if (consume(ID) || consume(CT_INT) || consume(CT_CHAR) || consume(CT_REAL) || consume(CT_STRING)) {
				lastArgToken = consumedTk;
			}

			tokens = ct;

			if (expr(arg)) {
				
				if (s->args.size() == 0) {
					tkerr(tokens, "Too many arguments in call.");
				}

				Symbols::iterator crtDefArg = s->args.begin();
				
				Symbol* crtSymbol = *crtDefArg;
				cast(&crtSymbol->type, &arg.type, tokens);
				crtDefArg++;

				while (1) {
					if (consume(COMMA)) {
						if (expr(arg)) {
							lastArgToken = consumedTk;

							if (crtDefArg == s->args.end()) {
								tkerr(tokens, "Too many arguments in call.");
							}
							Symbol* crtSymbol = *crtDefArg;
							cast(&crtSymbol->type, &arg.type, tokens);
							crtDefArg++;
						}
						else {
							tkerr(tokens, "Missing expression after ,");
						}
					}
					else {
						break;
					}
				}
				if (consume(RPAR)) {
					lastFuncToken = idToken;

					if (crtDefArg != s->args.end()) {
						tkerr(tokens, "Too few arguments in call.");
					}

					rv.type = s->type;
					rv.isCtVal = rv.isLVal = 0;
				
					lastArgToken = NULL;

					return 1;
				}
			}
			if (consume(RPAR)) {
				lastFuncToken = idToken;

				// Function has more arguments, but 0 were given
				if (s->args.size() != 0) {
					tkerr(tokens, "Too few arguments in call.");
				}

				lastArgToken = NULL;

				return 1;
			}
			else {
				tkerr(tokens, "Missing ) after expression.");
			}
		}
		else {
			if (s->cls == CLS_FUNC || s->cls == CLS_EXTFUNC)
				tkerr(tokens, "Missing call for function %s.", idToken->text);
			return 1;
		}
	}
	else if (consume(CT_INT)) {
		rv.type = createType(TB_INT, -1);
		rv.ctVal.i = consumedTk->i;
		rv.isCtVal = 1;
		rv.isLVal = 0;

		return 1;
	}
	else if (consume(CT_REAL)) {
		rv.type = createType(TB_DOUBLE, -1);
		rv.ctVal.d = consumedTk->r;
		rv.isCtVal = 1;
		rv.isLVal = 0;

		return 1;
	}
	else if (consume(CT_CHAR)) {
		rv.type = createType(TB_CHAR, -1);
		rv.ctVal.i = consumedTk->i;
		rv.isCtVal = 1;
		rv.isLVal = 0;

		return 1;
	}
	else if (consume(CT_STRING)) {
		rv.type = createType(TB_CHAR, 0);
		rv.ctVal.str = consumedTk->text;
		rv.isCtVal = 1;
		rv.isLVal = 0;

		return 1;
	}
	else if (consume(LPAR)) {
		if (expr(rv)) {
			if (consume(RPAR)) {
				return 1;
			}
			else {
				tkerr(tokens, "Missing ) after expression.");
			}
		}
		else if (typeName(type)) { // if typecast
			if (consume(RPAR)) {
				if (exprCast(rv)) {
					return 1;
				}
				else {
					tkerr(tokens, "Missing cast expression.");
				}
			}
			else {
				tkerr(tokens, "Missing ) after type name.");
			}
		}
		else {
			tkerr(tokens, "Missing expression after (.");
		}
	}

	tokens = startTk;
	return 0;
}

void analyzeSyntax() {
	Type type;

	// Add predefined functions
	addExtFuncs(symbols, crtDepth);

	// Analyze Syntax
	while (tokens != NULL) {
		RetVal rv;

		if (unit()) {
		}
		else if (declStruct()) {
		}
		else if (declVar()) {
		}
		else if (arrayDecl(type)) {
		}
		else if (declFunc()) {

		}
		else if (stm()) {

		}
		else if (expr(rv)) {

		}
	}
}