#include "../structs/Token.h"
#include "../enums/TokenCodes.h"
#include "../functions/ErrorFunctions.h"
#include "../functions/TokenFunctions.h"

Token* consumedTk;

int typeBase();
int stmCompound();
int funcArg();
int exprAssign();
int expr();
int declVar();
int declStruct();
int declFunc();
int arrayDecl();
int exprUnary();
int exprRel1();
int exprRel();
int exprPrimary();
int exprPostfix1();
int exprPostfix();
int exprOr1();
int exprMul1();
int exprMul();
int exprEq1();
int exprEq();
int exprCast();
int exprAnd1();
int exprAnd();
int exprAdd1();
int exprAdd();
int exprOr();

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
		if (declStruct()) {

		}
		if (declFunc()) {

		}
		if (declVar()) {

		}
		else {
			break;
		}
	}

	if (consume(END)) {
		return 1;
	}


	tokens = startTk;
	return 0;
}

int declStruct() {
	Token* startTk = tokens;

	if (consume(STRUCT)) {
		if (consume(ID)) {
			if (consume(LACC)) {
				while (1) {
					if (declVar()) {

					}
					else {
						break;
					}
				}

				if (consume(RACC)) {
					if (consume(SEMICOLON)) {
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

	if (typeBase()) {
		if (consume(ID)) {
			arrayDecl();
			while (1) {
				if (consume(COMMA)) {

				}
				else if (consume(ID)) {

				}
				else if (arrayDecl()) {

				}
				else {
					break;
				}
			}
			if (consume(SEMICOLON)) {
				return 1;
			}
			else {
				tkerr(tokens, "Missing ; at the end of the type base.");
			}
		}
		else {
			tkerr(tokens, "Missing type base identifier.");
		}
	}

	tokens = startTk;
	return 0;
}

int typeBase() {
	Token* startTk = tokens;

	if (consume(INT)) {
		return 1;
	}
	else if (consume(DOUBLE)) {
		return 1;
	}
	else if (consume(CHAR)) {
		return 1;
	}
	else if (consume(STRUCT)) {
		if (consume(ID)) {
			return 1;
		}
		else {
			tkerr(tokens, "Missing struct identifier.");
		}
	}
		
	tokens = startTk;
	return 0;
}

int arrayDecl() {
	Token* startTk = tokens;

	if (consume(LBRACKET)) {
		expr();
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

int typeName() {
	Token* startTk = tokens;

	if (typeBase()) {
		arrayDecl();
	}
		
	tokens = startTk;
	return 0;
}

int declFunc() {
	Token* startTk = tokens;

	if (typeBase()) {
		consume(MUL);
		if (consume(ID)) {
			if (consume(LPAR)) {
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
					if (stmCompound()) {
						return 1;
					}
					else {
						tkerr(tokens, "Missing statement compound after function declaration.");
					}
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
	if (consume(VOID)) {
		if (consume(ID)) {
			if (consume(LPAR)) {
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
					if (stmCompound()) {
						return 1;
					}
					else {
						tkerr(tokens, "Missing statement compound after function declaration.");
					}
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

	if (typeBase()) {
		if (consume(ID)) {
			arrayDecl();

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

	if (stmCompound()) {
		return 1;
	}
	if (consume(IF)) {
		if (consume(LPAR)) {
			if (expr()) {
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
			if (expr()) {
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
			expr();

			if (consume(SEMICOLON)) {
				expr();

				if (consume(SEMICOLON)) {
					expr();

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
		expr();
		if (consume(SEMICOLON)) {
			return 1;
		}
		else {
			tkerr(tokens, "Missing ; after RETURN statement.");
		}
	}
	else {
		expr();
		if (consume(SEMICOLON)) {
			return 1;
		}
		else {
			tkerr(tokens, "Missing ; at the end of the statement.");
		}
	}

	tokens = startTk;
	return 0;
}

int stmCompound() {
	Token* startTk = tokens;

	if (consume(LACC)) {
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
			return 1;
		}
		else {
			tkerr(tokens, "Missing } at the end of the statement.");
		}
	}

	tokens = startTk;
	return 0;
}

int expr() {
	Token* startTk = tokens;

	if (exprAssign()) {
		return 1;
	}

	tokens = startTk;
	return 0;
}

int exprAssign() {
	Token* startTk = tokens;

	if (exprUnary()) {
		if (consume(ASSIGN)) {
			if (exprAssign()) {
				return 1;
			}
			else {
				tkerr(tokens, "Missing assignment expression at right side of assignment operator.");
			}
		}
		else {
			tkerr(tokens, "Missing assignment operator.");
		}
	}
	if (exprOr()) {
		return 1;
	}

	tokens = startTk;
	return 0;
}

// exprOr ::= exprOr OR exprAnd | exprAnd
// [Recursivitate Stanga]
// exprOr ::= exprAnd exprOr1
// exprOr1 ::= OR exprAnd exprOr1 | EPS
int exprOr() {
	Token* startTk = tokens;

	if (exprAnd()) {
		if (exprOr1()) {
			return 1;
		}
		else {
			tkerr(tokens, "Missing OR expression.");
		}
	}

	tokens = startTk;
	return 0;
}

int exprOr1() {
	Token* startTk = tokens;

	if (consume(OR)) {
		if (exprAnd()) {
			if (exprOr1()) {
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
int exprAnd() {
	Token* startTk = tokens;

	if (exprEq()) {
		if (exprAnd1()) {
			return 1;
		}
		else {
			tkerr(tokens, "Missing AND expression.");
		}
	}

	tokens = startTk;
	return 0;
}

int exprAnd1() {
	Token* startTk = tokens;

	if (consume(AND)) {
		if (exprEq()) {
			if (exprAnd1()) {
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
int exprEq() {
	Token* startTk = tokens;

	if (exprRel()) {
		if (exprEq1()) {
			return 1;
		}
		else {
			tkerr(tokens, "Missing EQUAL/NOT-EQUAL expression.");
		}
	}

	tokens = startTk;
	return 0;
}

int exprEq1() {
	Token* startTk = tokens;

	if (consume(EQUAL)) {
		if (exprRel()) {
			if (exprEq1()) {
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
		if (exprRel()) {
			if (exprEq1()) {
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
int exprRel() {
	Token* startTk = tokens;

	if (exprAdd()) {
		if (exprRel1()) {
			return 1;
		}
		else {
			tkerr(tokens, "Missing relational expression.");
		}
	}

	tokens = startTk;
	return 0;
}

int exprRel1() {
	Token* startTk = tokens;

	if (consume(LESS)) {
		if (exprAdd()) {
			if (exprRel1()) {
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
		if (exprAdd()) {
			if (exprRel1()) {
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
		if (exprAdd()) {
			if (exprRel1()) {
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
		if (exprAdd()) {
			if (exprRel1()) {
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
int exprAdd() {
	Token* startTk = tokens;

	if (exprMul()) {
		if (exprAdd1()) {
			return 1;
		}
		else {
			tkerr(tokens, "Missing addition/subtraction expression.");
		}
	}

	tokens = startTk;
	return 0;
}

int exprAdd1() {
	Token* startTk = tokens;

	if (consume(ADD)) {
		if (exprMul()) {
			if (exprAdd1()) {
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
		if (exprMul()) {
			if (exprAdd1()) {
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
int exprMul() {
	Token* startTk = tokens;

	if (exprCast()) {
		if (exprMul1()) {
			return 1;
		}
		else {
			tkerr(tokens, "Missing multiplication expression.");
		}
	}

	tokens = startTk;
	return 0;
}

int exprMul1() {
	Token* startTk = tokens;

	if (consume(MUL)) {
		if (exprCast()) {
			if (exprMul1()) {
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
		if (exprCast()) {
			if (exprMul1()) {
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

int exprCast() {
	Token* startTk = tokens;

	if (consume(LPAR)) {
		if (typeName()) {
			if (consume(RPAR)) {
				if (exprCast()) {
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
			tkerr(tokens, "Missing type name after (.");
		}
	}
	else if (exprUnary()) {
		return 1;
	}

	tokens = startTk;
	return 0;
}

int exprUnary() {
	Token* startTk = tokens;

	if (consume(SUB)) {
		if (exprUnary()) {
			return 1;
		}
		else {
			tkerr(tokens, "Missing unary expression.");
		}
	}
	else if (consume(NOT)) {
		if (exprUnary()) {
			return 1;
		}
		else {
			tkerr(tokens, "Missing unary expression.");
		}
	}
	else if (exprPostfix()) {
		return 1;
	}

	tokens = startTk;
	return 0;
}

// exprPostfix ::= 
// [Recursivitate Stanga]
// exprPostfix ::= exprPrimary exprPostfix1
// exprPostfix1 ::= LBRACKET expr RBRACKET exprPostfix1 | DOT ID exprPostfix1 | EPS
int exprPostfix() {
	Token* startTk = tokens;

	if (exprPrimary()) {
		if (exprPostfix1()) {
			return 1;
		}
		else {
			tkerr(tokens, "Missing postfix expression.");
		}
	}

	tokens = startTk;
	return 0;
}

int exprPostfix1() {
	Token* startTk = tokens;

	if (consume(LBRACKET)) {
		if (expr()) {
			if (consume(RBRACKET)) {
				if (exprPostfix1()) {
					return 1;
				}
				else {
					tkerr(tokens, "Missing postfix expression.");

					tokens = startTk;
					return 0;
				}
			}
			else {
				tkerr(tokens, "Missing ) after expression.");

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
			if (exprPostfix1()) {
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

int exprPrimary() {
	Token* startTk = tokens;

	if (consume(ID)) {
		if (consume(LPAR)) {
			if (expr()) {
				while (1) {
					if (consume(COMMA)) {
						if (expr()) {

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
			if (consume(RPAR)) {
				return 1;
			}
			else {
				tkerr(tokens, "Missing ) after expression.");
			}
		}
		else {
			return 1;
		}
	}
	else if (consume(CT_INT)) {
		return 1;
	}
	else if (consume(CT_REAL)) {
		return 1;
	}
	else if (consume(CT_CHAR)) {
		return 1;
	}
	else if (consume(CT_STRING)) {
		return 1;
	}
	else if (consume(LPAR)) {
		if (expr()) {
			if (consume(RPAR)) {
				return 1;
			}
			else {
				tkerr(tokens, "Missing ) after expression.");
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
	while (tokens != NULL) {
		if (exprEq()) {
			printf("Yes!");
		}
	}
}