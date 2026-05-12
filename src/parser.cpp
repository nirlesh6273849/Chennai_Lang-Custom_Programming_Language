#include "parser.h"
#include <iostream>
#include <stdexcept>


// ============================================================================
// Constructor
// ============================================================================

Parser::Parser(const std::vector<Token> &toks) : tokens(toks), pos(0) {}

// ============================================================================
// Helpers
// ============================================================================

const Token &Parser::current() const { return tokens[pos]; }

const Token &Parser::peek() const {
  if (pos + 1 < tokens.size())
    return tokens[pos + 1];
  return tokens.back();
}

const Token &Parser::advance() {
  const Token &tok = tokens[pos];
  if (pos < tokens.size() - 1)
    pos++;
  return tok;
}

bool Parser::check(TokenType t) const { return current().type == t; }

bool Parser::match(TokenType t) {
  if (check(t)) {
    advance();
    return true;
  }
  return false;
}

Token Parser::expect(TokenType t, const std::string &msg) {
  if (check(t))
    return advance();
  error(msg + " at line " + std::to_string(current().line) + " (got '" +
        current().lexeme + "')");
}

void Parser::error(const std::string &msg) const {
  throw std::runtime_error("Parse error: " + msg);
}

bool Parser::isTypeToken() const {
  TokenType t = current().type;
  return t == TokenType::TOK_INT || t == TokenType::TOK_FLOAT ||
         t == TokenType::TOK_CHAR || t == TokenType::TOK_STRING ||
         t == TokenType::TOK_INT_ARR || t == TokenType::TOK_FLOAT_ARR ||
         t == TokenType::TOK_CHAR_ARR;
}

std::string Parser::parseType() {
  Token tok = advance();
  return tok.lexeme;
}

// ============================================================================
// Top level:  P = polamanna FunctionOperatorList niruthuanna
// ============================================================================

std::unique_ptr<Program> Parser::parseProgram() {
  auto prog = std::make_unique<Program>();

  expect(TokenType::TOK_POLAMANNA, "Expected 'polamanna'");

  // Parse operator overloads and functions until niruthuanna
  while (!check(TokenType::TOK_NIRUTHUANNA) && !check(TokenType::TOK_EOF)) {
    if (check(TokenType::TOK_OP)) {
      prog->operators.push_back(parseOperatorOverload());
    } else {
      prog->functions.push_back(parseFunction());
    }
  }

  expect(TokenType::TOK_NIRUTHUANNA, "Expected 'niruthuanna'");
  return prog;
}

// ============================================================================
// Operator overloading:  Op ( operator ) { operator := OExpression }
// ============================================================================

OperatorOverload Parser::parseOperatorOverload() {
  OperatorOverload ov;
  expect(TokenType::TOK_OP, "Expected 'Op'");
  expect(TokenType::TOK_LPAREN, "Expected '(' after 'Op'");

  // Read the operator symbol (e.g. +, -, *, /)
  Token opTok = advance();
  ov.op = opTok.lexeme;

  expect(TokenType::TOK_RPAREN, "Expected ')' after operator");
  expect(TokenType::TOK_LBRACE, "Expected '{'");

  // Parse: operator := OExpression
  expect(TokenType::TOK_OPERATOR, "Expected 'operator'");
  expect(TokenType::TOK_COLON_ASSIGN, "Expected ':='");

  ov.body = parseOExpression(ov.op);

  // optional semicolon
  match(TokenType::TOK_SEMICOLON);

  expect(TokenType::TOK_RBRACE, "Expected '}'");
  return ov;
}

// Parse the OExpression inside operator override blocks
// LHS and RHS are turned into special identifiers __LHS__ and __RHS__
ExprPtr Parser::parseOExpression(const std::string &op) {
  // We reuse the normal expression parser, but LHS/RHS tokens become
  // special identifiers that the interpreter will substitute.
  // This works because our expression parser handles identifiers,
  // and we pre-process: TOK_LHS -> IdentifierExpr("__LHS__"), etc.
  // We just need to parse a full expression here.
  return parseExpression();
}

// ============================================================================
// Function:   Identifier ( ParameterList ) Block
//          |  main ( ) Block
// ============================================================================

std::unique_ptr<FunctionDef> Parser::parseFunction() {
  std::string name;

  if (check(TokenType::TOK_MAIN)) {
    advance();
    name = "main";
  } else {
    Token nameTok = expect(TokenType::TOK_IDENTIFIER, "Expected function name");
    name = nameTok.lexeme;
  }

  expect(TokenType::TOK_LPAREN, "Expected '(' after function name");

  // Parse parameter list
  std::vector<std::pair<std::string, std::string>> params;
  if (!check(TokenType::TOK_RPAREN)) {
    // First parameter
    std::string ptype = parseType();
    Token pname = expect(TokenType::TOK_IDENTIFIER, "Expected parameter name");
    params.push_back({ptype, pname.lexeme});

    while (match(TokenType::TOK_COMMA)) {
      ptype = parseType();
      pname = expect(TokenType::TOK_IDENTIFIER, "Expected parameter name");
      params.push_back({ptype, pname.lexeme});
    }
  }

  expect(TokenType::TOK_RPAREN, "Expected ')'");

  auto body = parseBlock();
  return std::make_unique<FunctionDef>(name, std::move(params),
                                       std::move(body));
}

// ============================================================================
// Block:  { StatementList }  or  : IndentedStatementList
// ============================================================================

std::unique_ptr<BlockStmt> Parser::parseBlock() {
  if (check(TokenType::TOK_LBRACE)) {
    // Brace-delimited block: { ... }
    advance(); // consume '{'
    std::vector<StmtPtr> stmts;
    while (!check(TokenType::TOK_RBRACE) && !check(TokenType::TOK_EOF)) {
      stmts.push_back(parseStatement());
    }
    expect(TokenType::TOK_RBRACE, "Expected '}'");
    return std::make_unique<BlockStmt>(std::move(stmts));
  } else if (check(TokenType::TOK_COLON)) {
    // Indentation-delimited block: : ...
    return parseIndentedBlock();
  } else {
    error("Expected '{' or ':' at line " + std::to_string(current().line) +
          " (got '" + current().lexeme + "')");
  }
}

std::unique_ptr<BlockStmt> Parser::parseIndentedBlock() {
  Token colonTok = advance(); // consume ':'

  if (check(TokenType::TOK_EOF) || check(TokenType::TOK_NIRUTHUANNA)) {
    error("Expected indented block after ':' at line " +
          std::to_string(colonTok.line));
  }

  // First token after ':' determines the block's indentation level
  int blockIndent = current().col;

  std::vector<StmtPtr> stmts;
  while (!check(TokenType::TOK_EOF) &&
         !check(TokenType::TOK_NIRUTHUANNA) &&
         current().col >= blockIndent) {
    stmts.push_back(parseStatement());
  }

  if (stmts.empty()) {
    error("Expected at least one statement in indented block after ':' at line " +
          std::to_string(colonTok.line));
  }

  return std::make_unique<BlockStmt>(std::move(stmts));
}

// ============================================================================
// Statement dispatcher
// ============================================================================

StmtPtr Parser::parseStatement() {
  // Declaration or DeclarationAssignment: starts with a type keyword
  if (isTypeToken()) {
    return parseDeclarationOrAssignment();
  }

  // If statement
  if (check(TokenType::TOK_IF)) {
    return parseIfStatement();
  }

  // While statement
  if (check(TokenType::TOK_WHILE)) {
    return parseWhileStatement();
  }

  // Print statement — starts with a string literal
  if (check(TokenType::TOK_STRING_LIT)) {
    return parsePrintStatement();
  }

  // Nested block
  if (check(TokenType::TOK_LBRACE)) {
    auto blk = parseBlock();
    return blk;
  }

  // Assignment or expression statement
  // Check for:  identifier = expr ;   or   identifier [ expr ] = expr ;
  if (check(TokenType::TOK_IDENTIFIER)) {
    Token idTok = current();

    // Peek ahead to differentiate
    if (peek().type == TokenType::TOK_ASSIGN) {
      advance(); // consume identifier
      advance(); // consume =
      auto val = parseExpression();
      expect(TokenType::TOK_SEMICOLON, "Expected ';'");
      return std::make_unique<AssignmentStmt>(idTok.lexeme, std::move(val));
    }

    if (peek().type == TokenType::TOK_LBRACKET) {
      advance(); // consume identifier
      advance(); // consume [
      auto idx = parseExpression();
      expect(TokenType::TOK_RBRACKET, "Expected ']'");
      expect(TokenType::TOK_ASSIGN, "Expected '='");
      auto val = parseExpression();
      expect(TokenType::TOK_SEMICOLON, "Expected ';'");
      return std::make_unique<ArrayAssignStmt>(idTok.lexeme, std::move(idx),
                                               std::move(val));
    }

    // Otherwise it's an expression statement (e.g. function call)
    auto expr = parseExpression();
    expect(TokenType::TOK_SEMICOLON, "Expected ';'");
    return std::make_unique<ExprStmt>(std::move(expr));
  }

  error("Unexpected token '" + current().lexeme + "' at line " +
        std::to_string(current().line));
}

// ============================================================================
// Declaration / DeclAssignment
//   Type Identifier ;
//   Type Identifier = expression ;
// ============================================================================

StmtPtr Parser::parseDeclarationOrAssignment() {
  std::string typeName = parseType();
  Token nameTok = expect(TokenType::TOK_IDENTIFIER, "Expected variable name");

  if (match(TokenType::TOK_SEMICOLON)) {
    return std::make_unique<DeclarationStmt>(typeName, nameTok.lexeme);
  }

  expect(TokenType::TOK_ASSIGN, "Expected '=' or ';'");
  auto val = parseExpression();
  expect(TokenType::TOK_SEMICOLON, "Expected ';'");
  return std::make_unique<DeclAssignStmt>(typeName, nameTok.lexeme,
                                          std::move(val));
}

// ============================================================================
// If / elif / else
// ============================================================================

StmtPtr Parser::parseIfStatement() {
  expect(TokenType::TOK_IF, "Expected 'if'");
  expect(TokenType::TOK_LPAREN, "Expected '('");
  auto cond = parseExpression();
  expect(TokenType::TOK_RPAREN, "Expected ')'");
  auto thenBlock = parseBlock();

  // Parse elif chain
  std::vector<std::pair<ExprPtr, StmtPtr>> elifs;
  while (check(TokenType::TOK_ELIF)) {
    advance();
    expect(TokenType::TOK_LPAREN, "Expected '('");
    auto elifCond = parseExpression();
    expect(TokenType::TOK_RPAREN, "Expected ')'");
    auto elifBody = parseBlock();
    elifs.push_back({std::move(elifCond), std::move(elifBody)});
  }

  StmtPtr elseBlock = nullptr;
  if (match(TokenType::TOK_ELSE)) {
    elseBlock = parseBlock();
  }

  return std::make_unique<IfStmt>(std::move(cond), std::move(thenBlock),
                                  std::move(elifs), std::move(elseBlock));
}

// ============================================================================
// While
// ============================================================================

StmtPtr Parser::parseWhileStatement() {
  expect(TokenType::TOK_WHILE, "Expected 'while'");
  expect(TokenType::TOK_LPAREN, "Expected '('");
  auto cond = parseExpression();
  expect(TokenType::TOK_RPAREN, "Expected ')'");
  auto body = parseBlock();
  return std::make_unique<WhileStmt>(std::move(cond), std::move(body));
}

// ============================================================================
// Print:  "string" + Print   |   "string" sollu
// ============================================================================

StmtPtr Parser::parsePrintStatement() {
  std::vector<ExprPtr> parts;

  // First part must be a string literal or expression
  auto first = parseExpression();
  parts.push_back(std::move(first));

  // Check for + (concatenation) or sollu (terminate print)
  while (match(TokenType::TOK_PLUS)) {
    // Next part
    auto next = parseExpression();
    parts.push_back(std::move(next));
  }

  expect(TokenType::TOK_SOLLU, "Expected 'sollu'");
  // optional semicolon
  match(TokenType::TOK_SEMICOLON);

  return std::make_unique<PrintStmt>(std::move(parts));
}

// ============================================================================
// Expressions — precedence climbing
// ============================================================================

ExprPtr Parser::parseExpression() { return parseAssignmentExpr(); }

// assignment_expr = Identifier = assignment_expr | logical_expr
ExprPtr Parser::parseAssignmentExpr() {
  // Try logical_expr first; if it's an identifier followed by =, treat as
  // assignment
  auto expr = parseLogicalExpr();

  // We don't do inline assignment in expressions to keep things simpler.
  // Assignment is handled at the statement level.
  return expr;
}

// logical_expr = relational_expr ((&& | ||) relational_expr)*
ExprPtr Parser::parseLogicalExpr() {
  auto left = parseRelationalExpr();

  while (check(TokenType::TOK_AND) || check(TokenType::TOK_OR)) {
    std::string op = advance().lexeme;
    auto right = parseRelationalExpr();
    left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
  }
  return left;
}

// relational_expr = additive_expr ((< | > | <= | >= | == | !=) additive_expr)*
ExprPtr Parser::parseRelationalExpr() {
  auto left = parseAdditiveExpr();

  while (check(TokenType::TOK_LT) || check(TokenType::TOK_GT) ||
         check(TokenType::TOK_LE) || check(TokenType::TOK_GE) ||
         check(TokenType::TOK_EQ) || check(TokenType::TOK_NE)) {
    std::string op = advance().lexeme;
    auto right = parseAdditiveExpr();
    left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
  }
  return left;
}

// additive_expr = multiplicative_expr ((+ | -) multiplicative_expr)*
ExprPtr Parser::parseAdditiveExpr() {
  auto left = parseMultiplicativeExpr();

  while (check(TokenType::TOK_PLUS) || check(TokenType::TOK_MINUS)) {
    std::string op = advance().lexeme;
    auto right = parseMultiplicativeExpr();
    left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
  }
  return left;
}

// multiplicative_expr = unary_expr ((* | / | %) unary_expr)*
ExprPtr Parser::parseMultiplicativeExpr() {
  auto left = parsePowerExpr();

  while (check(TokenType::TOK_STAR) || check(TokenType::TOK_SLASH) ||
         check(TokenType::TOK_PERCENT)) {
    std::string op = advance().lexeme;
    auto right = parsePowerExpr();
    left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
  }
  return left;
}

// power_expr = unary_expr (^ unary_expr)*
ExprPtr Parser::parsePowerExpr() {
  auto left = parseUnaryExpr();

  while (check(TokenType::TOK_CARET)) {
    std::string op = advance().lexeme;
    auto right = parseUnaryExpr();
    left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
  }
  return left;
}

// unary_expr = (! | -) unary_expr | primary
ExprPtr Parser::parseUnaryExpr() {
  if (check(TokenType::TOK_BANG) || check(TokenType::TOK_MINUS)) {
    std::string op = advance().lexeme;
    auto operand = parseUnaryExpr();
    return std::make_unique<UnaryExpr>(op, std::move(operand));
  }
  return parsePrimary();
}

// primary = Number | Float | Character | String
//         | Identifier
//         | Identifier [ expression ]
//         | Functioncall               (Identifier ( ArgumentList ))
//         | ( expression )
//         | LHS  (special, for operator overload bodies)
//         | RHS  (special, for operator overload bodies)
ExprPtr Parser::parsePrimary() {
  // LHS / RHS — special tokens used inside operator overload bodies
  if (check(TokenType::TOK_LHS)) {
    advance();
    return std::make_unique<IdentifierExpr>("__LHS__");
  }
  if (check(TokenType::TOK_RHS)) {
    advance();
    return std::make_unique<IdentifierExpr>("__RHS__");
  }

  // Number literal
  if (check(TokenType::TOK_NUMBER)) {
    int val = std::stoi(advance().lexeme);
    return std::make_unique<NumberExpr>(val);
  }

  // Float literal
  if (check(TokenType::TOK_FLOAT_LIT)) {
    double val = std::stod(advance().lexeme);
    return std::make_unique<FloatExpr>(val);
  }

  // Char literal
  if (check(TokenType::TOK_CHAR_LIT)) {
    char val = advance().lexeme[0];
    return std::make_unique<CharExpr>(val);
  }

  // String literal
  if (check(TokenType::TOK_STRING_LIT)) {
    std::string val = advance().lexeme;
    return std::make_unique<StringExpr>(val);
  }

  // Identifier — could be variable, array access, or function call
  if (check(TokenType::TOK_IDENTIFIER)) {
    Token idTok = advance();

    // Function call: identifier ( args )
    if (check(TokenType::TOK_LPAREN)) {
      advance(); // consume (
      auto args = parseArgumentList();
      expect(TokenType::TOK_RPAREN, "Expected ')'");
      return std::make_unique<CallExpr>(idTok.lexeme, std::move(args));
    }

    // Array access: identifier [ expr ]
    if (check(TokenType::TOK_LBRACKET)) {
      advance(); // consume [
      auto idx = parseExpression();
      expect(TokenType::TOK_RBRACKET, "Expected ']'");
      return std::make_unique<ArrayAccessExpr>(idTok.lexeme, std::move(idx));
    }

    // Plain identifier
    return std::make_unique<IdentifierExpr>(idTok.lexeme);
  }

  // Parenthesized expression
  if (match(TokenType::TOK_LPAREN)) {
    auto expr = parseExpression();
    expect(TokenType::TOK_RPAREN, "Expected ')'");
    return expr;
  }

  error("Unexpected token '" + current().lexeme + "' in expression at line " +
        std::to_string(current().line));
}

// ============================================================================
// Argument list for function calls
// ============================================================================

std::vector<ExprPtr> Parser::parseArgumentList() {
  std::vector<ExprPtr> args;
  if (check(TokenType::TOK_RPAREN))
    return args; // empty arg list

  args.push_back(parseExpression());
  while (match(TokenType::TOK_COMMA)) {
    args.push_back(parseExpression());
  }
  return args;
}
