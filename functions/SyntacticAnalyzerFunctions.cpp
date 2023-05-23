#include "../structs/Token.h"
#include "../enums/TokenCodes.h"
#include "../functions/ErrorFunctions.h"
#include "../functions/TokenFunctions.h"
#include "../functions/SymbolFunctions.h"
#include "../structs/Vm.h"
#include "../functions/VmFunctions.h"
#include "../enums/VmEnums.h"
#include "../functions/CodeGenerationFunctions.h"

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

	Instr* labelMain = addInstr(O_CALL);
	addInstr(O_HALT);

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

	labelMain->args[0].addr = requireSymbol(symbols, "main", crtDepth)->addr;

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

				offset = 0;

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
	Instr* instrBeforeExpr;

	// For now, do not return the real number of elements
	type.nElements = -1;

	Token* arrayId = consumedTk;

	if (consume(LBRACKET)) {
		RetVal rv;
		instrBeforeExpr = lastInstruction;
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

		deleteInstructionsAfter(instrBeforeExpr);

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
	Symbol** ps;

	if (typeBase(type)) {
		if (consume(MUL)) {
			type.nElements = 0;
		}
		else {
			type.nElements = -1;
		}
		if (consume(ID)) {
			sizeArgs = offset = 0;
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

					crtFunc->addr = addInstr(O_ENTER);
					sizeArgs = offset;

					//update args offsets for correct FP indexing
					for (Symbol* ps : symbols) {
						if (ps->mem == MEM_ARG) {
							//2*sizeof(void*) == sizeof(retAddr)+sizeof(FP)
							ps->offset -= sizeArgs + 2 * sizeof(void*);
						}
					}
					offset = 0;

					if (stmCompound()) {
						deleteSymbolsAfter(symbols, crtFunc);

						((Instr*)crtFunc->addr)->args[0].i = offset;  // setup the ENTER argument 
						if (crtFunc->type.typeBase == TB_VOID) {
							addInstrII(O_RET, sizeArgs, 0);
						}

						crtFunc = NULL;

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
			sizeArgs = offset = 0;
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

					crtFunc->addr = addInstr(O_ENTER);
					sizeArgs = offset;

					//update args offsets for correct FP indexing
					for (Symbol* ps : symbols) {
						if (ps->mem == MEM_ARG) {
							//2*sizeof(void*) == sizeof(retAddr)+sizeof(FP)
							ps->offset -= sizeArgs + 2 * sizeof(void*);
						}
					}
					offset = 0;

					if (stmCompound()) {
						deleteSymbolsAfter(symbols, crtFunc);

						((Instr*)crtFunc->addr)->args[0].i = offset;  // setup the ENTER argument 
						if (crtFunc->type.typeBase == TB_VOID) {
							addInstrII(O_RET, sizeArgs, 0);
						}

						crtFunc = NULL;

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

	Type type;

	if (typeBase(type)) {
		if (consume(ID)) {
			Token* tokenId = consumedTk;
			arrayDecl(type);

			Symbol* s = addSymbol(symbols, tokenId->text, CLS_VAR, crtDepth);
			s->mem = MEM_ARG;
			s->type = type;
			s->offset = offset;
			s = addSymbol(crtFunc->args, tokenId->text, CLS_VAR, crtDepth);
			s->mem = MEM_ARG;
			s->type = type;
			s->offset = offset;

			offset += typeArgSize(&s->type);            

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
	Instr* i, * i1, * i2, * i3, * i4, * is, * ib3, * ibs;

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
					i1 = createCondJmp(&rv);

					if (stm()) {
						if (consume(ELSE)) {
							i2 = addInstr(O_JMP);

							if (stm()) {
								i1->args[0].addr = i2->next;
								i1 = i2;

								return 1;
							}
							else {
								tkerr(tokens, "Missing statement after ELSE keyword.");
							}
						}

						i1->args[0].addr = addInstr(O_NOP);

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
		Instr* oldLoopEnd = crtLoopEnd;	
		crtLoopEnd = createInstr(O_NOP);
		i1 = lastInstruction;

		if (consume(LPAR)) {
			lastFuncToken = NULL;
			if (expr(rv)) {
				if (rv.type.typeBase == TB_STRUCT) {
					tkerr(tokens, "A structure cannot be logically tested.");
				}
				if (consume(RPAR)) {
					i2 = createCondJmp(&rv);
					if (stm()) {
						addInstrA(O_JMP, i1->next);
						appendInstr(crtLoopEnd);
						i2->args[0].addr = crtLoopEnd;
						crtLoopEnd = oldLoopEnd;

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
		Instr* oldLoopEnd = crtLoopEnd;
		crtLoopEnd = createInstr(O_NOP);

		if (consume(LPAR)) {
			lastFuncToken = NULL;
			expr(rv1);

			if (typeArgSize(&rv1.type))
				addInstrI(O_DROP, typeArgSize(&rv1.type));

			if (consume(SEMICOLON)) {
				i2 = lastInstruction; /* i2 is before rv2 */

				int isExpr = expr(rv2);

				if (isExpr == 0) { // infinite loop
					i4 = NULL;
				}
				else {
					i4 = createCondJmp(&rv2);
				}

				if (rv2.type.typeBase == TB_STRUCT) {
					tkerr(tokens, "A structure cannot be logically tested.");
				}
				if (consume(SEMICOLON)) {
					ib3 = lastInstruction; /* ib3 is before rv3 */

					expr(rv);

					if (typeArgSize(&rv.type))
						addInstrI(O_DROP, typeArgSize(&rv.type));

					if (consume(RPAR)) {
						ibs = lastInstruction; /* ibs is before stm */

						if (stm()) {
							// if rv3 exists, exchange rv3 code with stm code: rv3 stm -> stm rv3
							if (ib3 != ibs) {
								i3 = ib3->next;
								is = ibs->next;
								ib3->next = is;
								is->last = ib3;
								lastInstruction->next = i3;
								i3->last = lastInstruction;
								ibs->next = NULL;
								lastInstruction = ibs;
							}
							addInstrA(O_JMP, i2->next);
							appendInstr(crtLoopEnd);
							if (i4)i4->args[0].addr = crtLoopEnd;
							crtLoopEnd = oldLoopEnd;

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
			if (!crtLoopEnd) {
				tkerr(tokens, "break without for or while");
			}
			addInstrA(O_JMP, crtLoopEnd);
		
			return 1;
		}
		else {
			tkerr(tokens, "Missing ; after BREAK keyword.");
		}
	}
	if (consume(RETURN)) {
		if (expr(rv)) {
			i = getRVal(&rv);
			addCastInstr(i, &rv.type, &crtFunc->type);

			if (crtFunc->type.typeBase == TB_VOID) {
				tkerr(tokens, "A void function cannot return a value.");
			}
			cast(&crtFunc->type, &rv.type, tokens);
		}
		if (consume(SEMICOLON)) {
			if (crtFunc->type.typeBase == TB_VOID) {
				addInstrII(O_RET, sizeArgs, 0);
			}
			else {
				addInstrII(O_RET, sizeArgs, typeArgSize(&crtFunc->type));
			}

			return 1;
		}
		else {
			tkerr(tokens, "Missing ; after RETURN statement.");
		}
	}
	else if (expr(rv)) {
		if (typeArgSize(&rv.type))addInstrI(O_DROP, typeArgSize(&rv.type));

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
	Instr* i, * oldLastInstr = lastInstruction;
	
	if (exprOr(rv)) {
		if (consume(ASSIGN)) {
			if (exprAssign(rve)) {
				// before "rv->isCtVal=rv->isLVal=0;"
				i = getRVal(&rve);
				addCastInstr(i, &rve.type, &rv.type);
				//duplicate the value on top before the dst addr
				addInstrII(O_INSERT,
					sizeof(void*) + typeArgSize(&rv.type),
					typeArgSize(&rv.type));
				addInstrI(O_STORE, typeArgSize(&rv.type));

				if (!rv.isLVal) {
					tkerr(tokens, "Cannot assign to a non-lval.");
				}
				if (rv.type.nElements > -1 || rve.type.nElements > -1) {
					tkerr(tokens, "The arrays cannot be assigned.");
				}
				cast(&rv.type, &rve.type, tokens);
				rv.isCtVal = rv.isLVal = 0;
			}
			// NEWLY ADDED ELSE
			else {
				tkerr(tokens, "Missing expression on right side of ASSIGN operator (=).");
			}
			
		}
		return 1;
	}

	deleteInstructionsAfter(oldLastInstr);
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
	Instr* i1, * i2;Type t, t1, t2;

	if (consume(OR)) {
		i1 = rv.type.nElements < 0 ? getRVal(&rv) : lastInstruction;
		t1 = rv.type;

		if (exprAnd(rve)) {
			/* use (rv, rve); return rv */
			if (rv.type.typeBase == TB_STRUCT || rve.type.typeBase == TB_STRUCT) {
				tkerr(tokens, "A structure cannot be logically tested.");

				if (rv.type.nElements >= 0) {      // vectors
					addInstr(O_OR_A);
				}
				else {  // non-vectors
					i2 = getRVal(&rve);t2 = rve.type;
					t = getArithType(&t1, &t2);
					addCastInstr(i1, &t1, &t);
					addCastInstr(i2, &t2, &t);
					switch (t.typeBase) {
					case TB_INT:addInstr(O_OR_I);break;
					case TB_DOUBLE:addInstr(O_OR_D);break;
					case TB_CHAR:addInstr(O_OR_C);break;
					}
				}

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
	Instr* i1, * i2;Type t, t1, t2;

	if (consume(AND)) {
		i1 = rv.type.nElements < 0 ? getRVal(&rv) : lastInstruction;
		t1 = rv.type;

		if (exprEq(rve)) {
			/* use (rv, rve); return rv */
			if (rv.type.typeBase == TB_STRUCT || rve.type.typeBase == TB_STRUCT) {
				tkerr(tokens, "A structure cannot be logically tested.");

				if (rv.type.nElements >= 0) {      // vectors
					addInstr(O_AND_A);
				}
				else {  // non-vectors
					i2 = getRVal(&rve);t2 = rve.type;
					t = getArithType(&t1, &t2);
					addCastInstr(i1, &t1, &t);
					addCastInstr(i2, &t2, &t);
					switch (t.typeBase) {
					case TB_INT:addInstr(O_AND_I);break;
					case TB_DOUBLE:addInstr(O_AND_D);break;
					case TB_CHAR:addInstr(O_AND_C);break;
					}
				}

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
	Instr* i1, * i2;Type t, t1, t2;
	Token* consumedSymbolTk;

	if (consume(EQUAL)) {
		consumedSymbolTk = consumedTk;

		i1 = rv.type.nElements < 0 ? getRVal(&rv) : lastInstruction;
		t1 = rv.type;

		if (exprRel(rve)) {
			/* use (rv, rve); return rv */
			if (rv.type.typeBase == TB_STRUCT || rve.type.typeBase == TB_STRUCT) {
				tkerr(tokens, "A structure cannot be compared.");
			}

			if (rv.type.nElements >= 0) {      // vectors
				addInstr(consumedSymbolTk->code == EQUAL ? O_EQ_A : O_NOTEQ_A);
			}
			else {  // non-vectors
				i2 = getRVal(&rve);t2 = rve.type;
				t = getArithType(&t1, &t2);
				addCastInstr(i1, &t1, &t);
				addCastInstr(i2, &t2, &t);
				if (consumedSymbolTk->code == EQUAL) {
					switch (t.typeBase) {
					case TB_INT:addInstr(O_EQ_I);break;
					case TB_DOUBLE:addInstr(O_EQ_D);break;
					case TB_CHAR:addInstr(O_EQ_C);break;
					}
				}
				else {
					switch (t.typeBase) {
					case TB_INT:addInstr(O_NOTEQ_I);break;
					case TB_DOUBLE:addInstr(O_NOTEQ_D);break;
					case TB_CHAR:addInstr(O_NOTEQ_C);break;
					}
				}
			}

			rv.type = createType(TB_INT, -1);
			rv.isCtVal = rv.isLVal = 0;

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
		consumedSymbolTk = consumedTk;

		i1 = rv.type.nElements < 0 ? getRVal(&rv) : lastInstruction;
		t1 = rv.type;

		if (exprRel(rve)) {
			/* use (rv, rve); return rv */
			if (rv.type.typeBase == TB_STRUCT || rve.type.typeBase == TB_STRUCT) {
				tkerr(tokens, "A structure cannot be compared.");
			}

			if (rv.type.nElements >= 0) {      // vectors
				addInstr(consumedSymbolTk->code == EQUAL ? O_EQ_A : O_NOTEQ_A);
			}
			else {  // non-vectors
				i2 = getRVal(&rve);t2 = rve.type;
				t = getArithType(&t1, &t2);
				addCastInstr(i1, &t1, &t);
				addCastInstr(i2, &t2, &t);
				if (consumedSymbolTk->code == EQUAL) {
					switch (t.typeBase) {
					case TB_INT:addInstr(O_EQ_I);break;
					case TB_DOUBLE:addInstr(O_EQ_D);break;
					case TB_CHAR:addInstr(O_EQ_C);break;
					}
				}
				else {
					switch (t.typeBase) {
					case TB_INT:addInstr(O_NOTEQ_I);break;
					case TB_DOUBLE:addInstr(O_NOTEQ_D);break;
					case TB_CHAR:addInstr(O_NOTEQ_C);break;
					}
				}
			}

			rv.type = createType(TB_INT, -1);
			rv.isCtVal = rv.isLVal = 0;

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
	Instr* i1, * i2;Type t, t1, t2;
	Token* consumedSymbolTk;

	if (consume(LESS)) {
		consumedSymbolTk = consumedTk;

		i1 = getRVal(&rv);
		t1 = rv.type;

		if (exprAdd(rve)) {
			/* use (rv, rve); return rv */
			if (rv.type.nElements > -1 || rve.type.nElements > -1) {
				tkerr(tokens, "An array cannot be compared.");
			}
			if (rv.type.typeBase == TB_STRUCT || rve.type.typeBase == TB_STRUCT) {
				tkerr(tokens, "A structure cannot be compared.");
			}

			i2 = getRVal(&rve);t2 = rve.type;
			t = getArithType(&t1, &t2);
			addCastInstr(i1, &t1, &t);
			addCastInstr(i2, &t2, &t);

			switch (consumedSymbolTk->code) {
			case LESS:
				switch (t.typeBase) {
				case TB_INT:addInstr(O_LESS_I);break;
				case TB_DOUBLE:addInstr(O_LESS_D);break;
				case TB_CHAR:addInstr(O_LESS_C);break;
				}
				break;
			case LESSEQ:
				switch (t.typeBase) {
				case TB_INT:addInstr(O_LESSEQ_I);break;
				case TB_DOUBLE:addInstr(O_LESSEQ_D);break;
				case TB_CHAR:addInstr(O_LESSEQ_C);break;
				}
				break;
			case GREATER:
				switch (t.typeBase) {
				case TB_INT:addInstr(O_GREATER_I);break;
				case TB_DOUBLE:addInstr(O_GREATER_D);break;
				case TB_CHAR:addInstr(O_GREATER_C);break;
				}
				break;
			case GREATEREQ:
				switch (t.typeBase) {
				case TB_INT:addInstr(O_GREATEREQ_I);break;
				case TB_DOUBLE:addInstr(O_GREATEREQ_D);break;
				case TB_CHAR:addInstr(O_GREATEREQ_C);break;
				}
				break;
			}

			rv.type = createType(TB_INT, -1);
			rv.isCtVal = rv.isLVal = 0;

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
		i1 = getRVal(&rv);
		t1 = rv.type;

		consumedSymbolTk = consumedTk;

		if (exprAdd(rve)) {
			/* use (rv, rve); return rv */
			if (rv.type.nElements > -1 || rve.type.nElements > -1) {
				tkerr(tokens, "An array cannot be compared.");
			}
			if (rv.type.typeBase == TB_STRUCT || rve.type.typeBase == TB_STRUCT) {
				tkerr(tokens, "A structure cannot be compared.");
			}

			i2 = getRVal(&rve);t2 = rve.type;
			t = getArithType(&t1, &t2);
			addCastInstr(i1, &t1, &t);
			addCastInstr(i2, &t2, &t);

			switch (consumedSymbolTk->code) {
			case LESS:
				switch (t.typeBase) {
				case TB_INT:addInstr(O_LESS_I);break;
				case TB_DOUBLE:addInstr(O_LESS_D);break;
				case TB_CHAR:addInstr(O_LESS_C);break;
				}
				break;
			case LESSEQ:
				switch (t.typeBase) {
				case TB_INT:addInstr(O_LESSEQ_I);break;
				case TB_DOUBLE:addInstr(O_LESSEQ_D);break;
				case TB_CHAR:addInstr(O_LESSEQ_C);break;
				}
				break;
			case GREATER:
				switch (t.typeBase) {
				case TB_INT:addInstr(O_GREATER_I);break;
				case TB_DOUBLE:addInstr(O_GREATER_D);break;
				case TB_CHAR:addInstr(O_GREATER_C);break;
				}
				break;
			case GREATEREQ:
				switch (t.typeBase) {
				case TB_INT:addInstr(O_GREATEREQ_I);break;
				case TB_DOUBLE:addInstr(O_GREATEREQ_D);break;
				case TB_CHAR:addInstr(O_GREATEREQ_C);break;
				}
				break;
			}

			rv.type = createType(TB_INT, -1);
			rv.isCtVal = rv.isLVal = 0;
			
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
		i1 = getRVal(&rv);
		t1 = rv.type;

		consumedSymbolTk = consumedTk;

		if (exprAdd(rve)) {
			/* use (rv, rve); return rv */
			if (rv.type.nElements > -1 || rve.type.nElements > -1) {
				tkerr(tokens, "An array cannot be compared.");
			}
			if (rv.type.typeBase == TB_STRUCT || rve.type.typeBase == TB_STRUCT) {
				tkerr(tokens, "A structure cannot be compared.");
			}

			i2 = getRVal(&rve);t2 = rve.type;
			t = getArithType(&t1, &t2);
			addCastInstr(i1, &t1, &t);
			addCastInstr(i2, &t2, &t);

			i2 = getRVal(&rve);t2 = rve.type;
			t = getArithType(&t1, &t2);
			addCastInstr(i1, &t1, &t);
			addCastInstr(i2, &t2, &t);

			switch (consumedSymbolTk->code) {
			case LESS:
				switch (t.typeBase) {
				case TB_INT:addInstr(O_LESS_I);break;
				case TB_DOUBLE:addInstr(O_LESS_D);break;
				case TB_CHAR:addInstr(O_LESS_C);break;
				}
				break;
			case LESSEQ:
				switch (t.typeBase) {
				case TB_INT:addInstr(O_LESSEQ_I);break;
				case TB_DOUBLE:addInstr(O_LESSEQ_D);break;
				case TB_CHAR:addInstr(O_LESSEQ_C);break;
				}
				break;
			case GREATER:
				switch (t.typeBase) {
				case TB_INT:addInstr(O_GREATER_I);break;
				case TB_DOUBLE:addInstr(O_GREATER_D);break;
				case TB_CHAR:addInstr(O_GREATER_C);break;
				}
				break;
			case GREATEREQ:
				switch (t.typeBase) {
				case TB_INT:addInstr(O_GREATEREQ_I);break;
				case TB_DOUBLE:addInstr(O_GREATEREQ_D);break;
				case TB_CHAR:addInstr(O_GREATEREQ_C);break;
				}
				break;
			}
			

			rv.type = createType(TB_INT, -1);
			rv.isCtVal = rv.isLVal = 0;

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
		i1 = getRVal(&rv);
		t1 = rv.type;

		consumedSymbolTk = consumedTk;

		if (exprAdd(rve)) {
			/* use (rv, rve); return rv */
			if (rv.type.nElements > -1 || rve.type.nElements > -1) {
				tkerr(tokens, "An array cannot be compared.");
			}
			if (rv.type.typeBase == TB_STRUCT || rve.type.typeBase == TB_STRUCT) {
				tkerr(tokens, "A structure cannot be compared.");
			}

			i2 = getRVal(&rve);t2 = rve.type;
			t = getArithType(&t1, &t2);
			addCastInstr(i1, &t1, &t);
			addCastInstr(i2, &t2, &t);
			i2 = getRVal(&rve);t2 = rve.type;
			t = getArithType(&t1, &t2);
			addCastInstr(i1, &t1, &t);
			addCastInstr(i2, &t2, &t);

			switch (consumedSymbolTk->code) {
			case LESS:
				switch (t.typeBase) {
				case TB_INT:addInstr(O_LESS_I);break;
				case TB_DOUBLE:addInstr(O_LESS_D);break;
				case TB_CHAR:addInstr(O_LESS_C);break;
				}
				break;
			case LESSEQ:
				switch (t.typeBase) {
				case TB_INT:addInstr(O_LESSEQ_I);break;
				case TB_DOUBLE:addInstr(O_LESSEQ_D);break;
				case TB_CHAR:addInstr(O_LESSEQ_C);break;
				}
				break;
			case GREATER:
				switch (t.typeBase) {
				case TB_INT:addInstr(O_GREATER_I);break;
				case TB_DOUBLE:addInstr(O_GREATER_D);break;
				case TB_CHAR:addInstr(O_GREATER_C);break;
				}
				break;
			case GREATEREQ:
				switch (t.typeBase) {
				case TB_INT:addInstr(O_GREATEREQ_I);break;
				case TB_DOUBLE:addInstr(O_GREATEREQ_D);break;
				case TB_CHAR:addInstr(O_GREATEREQ_C);break;
				}
				break;
			}

			rv.type = createType(TB_INT, -1);
			rv.isCtVal = rv.isLVal = 0;
		
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
	Instr* i1, * i2;
	Type t1, t2;

	if (consume(ADD)) {
		Token* consumedSymbolTk = consumedTk;
		i1 = getRVal(&rv);
		t1 = rv.type;

		if (exprMul(rve)) {
			/* use (rv, rve); return rv */
			if (rv.type.nElements > -1 || rve.type.nElements > -1) {
				tkerr(tokens, "An array cannot be added or subtracted.");
			}
			if (rv.type.typeBase == TB_STRUCT || rve.type.typeBase == TB_STRUCT) {
				tkerr(tokens, "A structure cannot be added or subtracted.");
			}
			rv.type = getArithType(&rv.type, &rve.type);

			i2 = getRVal(&rve);t2 = rve.type;
			addCastInstr(i1, &t1, &rv.type);
			addCastInstr(i2, &t2, &rv.type);
			if (consumedSymbolTk->code == ADD) {
				switch (rv.type.typeBase) {
				case TB_INT:addInstr(O_ADD_I);break;
				case TB_DOUBLE:addInstr(O_ADD_D);break;
				case TB_CHAR:addInstr(O_ADD_C);break;
				}
			}
			else {
				switch (rv.type.typeBase) {
				case TB_INT:addInstr(O_SUB_I);break;
				case TB_DOUBLE:addInstr(O_SUB_D);break;
				case TB_CHAR:addInstr(O_SUB_C);break;
				}
			}

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
		i1 = getRVal(&rv);
		t1 = rv.type;

		if (exprMul(rve)) {
			/* use (rv, rve); return rv */
			if (rv.type.nElements > -1 || rve.type.nElements > -1) {
				tkerr(tokens, "An array cannot be added or subtracted.");
			}
			if (rv.type.typeBase == TB_STRUCT || rve.type.typeBase == TB_STRUCT) {
				tkerr(tokens, "A structure cannot be added or subtracted.");
			}
			rv.type = getArithType(&rv.type, &rve.type);

			i2 = getRVal(&rve);t2 = rve.type;
			addCastInstr(i1, &t1, &rv.type);
			addCastInstr(i2, &t2, &rv.type);
			if (consumedTk->code == ADD) {
				switch (rv.type.typeBase) {
				case TB_INT:addInstr(O_ADD_I);break;
				case TB_DOUBLE:addInstr(O_ADD_D);break;
				case TB_CHAR:addInstr(O_ADD_C);break;
				}
			}
			else {
				switch (rv.type.typeBase) {
				case TB_INT:addInstr(O_SUB_I);break;
				case TB_DOUBLE:addInstr(O_SUB_D);break;
				case TB_CHAR:addInstr(O_SUB_C);break;
				}
			}

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
	Instr *i1, *i2;
	Type t1, t2;

	if (consume(MUL)) {
		Token* consumedSymbolTk = consumedTk;
		i1 = getRVal(&rv);
		t1 = rv.type;

		if (exprCast(rve)) {
			/* use (rv, rve); return rv */
			if (rv.type.nElements > -1 || rve.type.nElements > -1) {
				tkerr(tokens, "An array cannot be multiplied or divided.");
			}
			if (rv.type.typeBase == TB_STRUCT || rve.type.typeBase == TB_STRUCT) {
				tkerr(tokens, "A structure cannot be multiplied or divided.");
			}
			rv.type = getArithType(&rv.type, &rve.type);

			i2 = getRVal(&rve);t2 = rve.type;
			addCastInstr(i1, &t1, &rv.type);
			addCastInstr(i2, &t2, &rv.type);
			if (consumedSymbolTk->code == MUL) {
				switch (rv.type.typeBase) {
				case TB_INT:addInstr(O_MUL_I);break;
				case TB_DOUBLE:addInstr(O_MUL_D);break;
				case TB_CHAR:addInstr(O_MUL_C);break;
				}
			}
			else {
				switch (rv.type.typeBase) {
				case TB_INT:addInstr(O_DIV_I);break;
				case TB_DOUBLE:addInstr(O_DIV_D);break;
				case TB_CHAR:addInstr(O_DIV_C);break;
				}
			}

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
		i1 = getRVal(&rv);
		t1 = rv.type;

		if (exprCast(rve)) {
			/* use (rv, rve); return rv */
			if (rv.type.nElements > -1 || rve.type.nElements > -1) {
				tkerr(tokens, "An array cannot be multiplied or divided.");
			}
			if (rv.type.typeBase == TB_STRUCT || rve.type.typeBase == TB_STRUCT) {
				tkerr(tokens, "A structure cannot be multiplied or divided.");
			}
			rv.type = getArithType(&rv.type, &rve.type);

			i2 = getRVal(&rve);t2 = rve.type;
			addCastInstr(i1, &t1, &rv.type);
			addCastInstr(i2, &t2, &rv.type);
			if (consumedTk->code == MUL) {
				switch (rv.type.typeBase) {
				case TB_INT:addInstr(O_MUL_I);break;
				case TB_DOUBLE:addInstr(O_MUL_D);break;
				case TB_CHAR:addInstr(O_MUL_C);break;
				}
			}
			else {
				switch (rv.type.typeBase) {
				case TB_INT:addInstr(O_DIV_I);break;
				case TB_DOUBLE:addInstr(O_DIV_D);break;
				case TB_CHAR:addInstr(O_DIV_C);break;
				}
			}

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
	Instr* oldLastInstr = lastInstruction;

	Type type;

	if (consume(LPAR)) {
		if (typeName(type)) {
			if (consume(RPAR)) {
				if (exprCast(rve)) {
					cast(&type, &rve.type, tokens);

					if (rv.type.nElements < 0 && rv.type.typeBase != TB_STRUCT) {
						switch (rve.type.typeBase) {
						case TB_CHAR:
							switch (type.typeBase) {
							case TB_INT:addInstr(O_CAST_C_I);break;
							case TB_DOUBLE:addInstr(O_CAST_C_D);break;
							}
							break;
						case TB_DOUBLE:
							switch (type.typeBase) {
							case TB_CHAR:addInstr(O_CAST_D_C);break;
							case TB_INT:addInstr(O_CAST_D_I);break;
							}
							break;
						case TB_INT:
							switch (type.typeBase) {
							case TB_CHAR:addInstr(O_CAST_I_C);break;
							case TB_DOUBLE:addInstr(O_CAST_I_D);break;
							}
							break;
						}
					}

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
	else {
		deleteInstructionsAfter(oldLastInstr);

		if (exprUnary(rv)) {
			return 1;
		}
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

			getRVal(&rv);
			switch (rv.type.typeBase) {
			case TB_CHAR:addInstr(O_NEG_C);break;
			case TB_INT:addInstr(O_NEG_I);break;
			case TB_DOUBLE:addInstr(O_NEG_D);break;
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

			if (rv.type.nElements < 0) {
				getRVal(&rv);
				switch (rv.type.typeBase) {
				case TB_CHAR:addInstr(O_NOT_C);break;
				case TB_INT:addInstr(O_NOT_I);break;
				case TB_DOUBLE:addInstr(O_NOT_D);break;
				}
			}
			else {
				addInstr(O_NOT_A);
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
				addCastInstr(lastInstruction, &rve.type, &typeInt);
				getRVal(&rve);
				if (typeBaseSize(&rv.type) != 1) {
					addInstrI(O_PUSHCT_I, typeBaseSize(&rv.type));
					addInstr(O_MUL_I);
				}
				addInstr(O_OFFSET);

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
			// sMember->offset was replaced with offset
			if (offset) {
				addInstrI(O_PUSHCT_I, offset);
				addInstr(O_OFFSET);
			}

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
	Instr* i;

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
				Symbols::iterator crtDefArg = s->args.begin();
				
				Symbol* crtSymbol = *crtDefArg;
				cast(&crtSymbol->type, &arg.type, tokens);

				if ((*crtDefArg)->type.nElements < 0) {  //only arrays are passed by addr
					i = getRVal(&arg);
				}
				else {
					i = lastInstruction;
				}
				addCastInstr(i, &arg.type, &(*crtDefArg)->type);

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

							if ((*crtDefArg)->type.nElements < 0) {
								i = getRVal(&arg);
							}
							else {
								i = lastInstruction;
							}
							addCastInstr(i, &arg.type, &(*crtDefArg)->type);

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

					i = addInstr(s->cls == CLS_FUNC ? O_CALL : O_CALLEXT);
					i->args[0].addr = s->addr;

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

				i = addInstr(s->cls == CLS_FUNC ? O_CALL : O_CALLEXT);
				i->args[0].addr = s->addr;

				return 1;
			}
			else {
				tkerr(tokens, "Missing ) after expression.");
			}
		}
		else {
			if (s->cls == CLS_FUNC || s->cls == CLS_EXTFUNC)
				tkerr(tokens, "Missing call for function %s.", idToken->text);

			if (s->depth) {
				addInstrI(O_PUSHFPADDR, s->offset);
			}
			else {
				addInstrA(O_PUSHCT_A, s->addr);
			}

			return 1;
		}
	}
	else if (consume(CT_INT)) {
		rv.type = createType(TB_INT, -1);
		rv.ctVal.i = consumedTk->i;
		rv.isCtVal = 1;
		rv.isLVal = 0;

		addInstrI(O_PUSHCT_I, consumedTk->i);

		return 1;
	}
	else if (consume(CT_REAL)) {
		rv.type = createType(TB_DOUBLE, -1);
		rv.ctVal.d = consumedTk->r;
		rv.isCtVal = 1;
		rv.isLVal = 0;

		i = addInstr(O_PUSHCT_D);
		i->args[0].d = consumedTk->r;

		return 1;
	}
	else if (consume(CT_CHAR)) {
		rv.type = createType(TB_CHAR, -1);
		rv.ctVal.i = consumedTk->i;
		rv.isCtVal = 1;
		rv.isLVal = 0;

		addInstrI(O_PUSHCT_C, consumedTk->i);

		return 1;
	}
	else if (consume(CT_STRING)) {
		rv.type = createType(TB_CHAR, 0);
		rv.ctVal.str = consumedTk->text;
		rv.isCtVal = 1;
		rv.isLVal = 0;

		addInstrA(O_PUSHCT_A, consumedTk->text);

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

	//vmTest(symbols, 0);
}