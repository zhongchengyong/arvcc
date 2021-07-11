//
// Created by zhong on 7/4/21.
//

#ifndef ARVCC_SRC_PARSER_H_
#define ARVCC_SRC_PARSER_H_
#include <map>

#include "ast.h"

int GetTokenPrecedence();

/**
 * numberexpr ::= number
 */
std::unique_ptr<ExprAST> ParseNumberExpr();

/**
 * parenexpr ::= '(' expression ')'
 */
std::unique_ptr<ExprAST> ParseParenExpr();

/**
 * identifierexpr
 *      ::= identifier
 *      ::= identifier '(' expression* ')'
 */
std::unique_ptr<ExprAST> ParseIdentifierExpr();

/**
 * Primary
 *        ::= identifierexpr
 *        ::= numberexpr
 *        ::=parenexpr
 */
std::unique_ptr<ExprAST> ParsePrimary();

std::unique_ptr<ExprAST> ParseExpression();

/**
 * binoprhs
 *         ::= (op primary)*
 * Note: Operator-precedence parser
 */
std::unique_ptr<ExprAST> ParseBinOpRHS(int expr_prec, std::unique_ptr<ExprAST> lhs);

std::unique_ptr<PrototypeAST> ParsePrototype();

std::unique_ptr<FunctionAST> ParseDefinition();

std::unique_ptr<FunctionAST> ParseTopLevelExpr();

std::unique_ptr<PrototypeAST> ParseExtern();

void MainLoop();

#endif //ARVCC_SRC_PARSER_H_
