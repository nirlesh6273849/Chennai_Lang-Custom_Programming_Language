#pragma once
#include "ast.h"
#include "token.h"
#include <memory>
#include <vector>


class Parser {
public:
  explicit Parser(const std::vector<Token> &tokens);
  std::unique_ptr<Program> parseProgram();

private:
  std::vector<Token> tokens;
  size_t pos;
  // Helpers
  const Token &current() const;
  const Token &peek() const;
  const Token &advance();
  bool check(TokenType t) const;
  bool match(TokenType t);
  Token expect(TokenType t, const std::string &msg);
  [[noreturn]] void error(const std::string &msg) const;

  bool isTypeToken() const;
  std::string parseType();
  // Top level
  OperatorOverload parseOperatorOverload();
  ExprPtr parseOExpression(const std::string &op);
  std::unique_ptr<FunctionDef> parseFunction();
  // Statement
  std::unique_ptr<BlockStmt> parseBlock();
  std::unique_ptr<BlockStmt> parseIndentedBlock();
  StmtPtr parseStatement();
  StmtPtr parseDeclarationOrAssignment();
  StmtPtr parseIfStatement();
  StmtPtr parseWhileStatement();
  StmtPtr parsePrintStatement();
  // Expression (precedence climbing)
  ExprPtr parseExpression();
  ExprPtr parseAssignmentExpr();
  ExprPtr parseLogicalExpr();
  ExprPtr parseRelationalExpr();
  ExprPtr parseAdditiveExpr();
  ExprPtr parseMultiplicativeExpr();
  ExprPtr parseUnaryExpr();
  ExprPtr parsePrimary();
  ExprPtr parsePowerExpr();
  // Argument list
  std::vector<ExprPtr> parseArgumentList();
};
