#pragma once
#include <string>
#include <vector>

// ============================================================================
// Token types for Chennai Lang
// ============================================================================

enum class TokenType {
  // Keywords
  TOK_POLAMANNA,   // program start
  TOK_NIRUTHUANNA, // program end
  TOK_OP,          // operator overload keyword
  TOK_SOLLU,       // print
  TOK_IF,
  TOK_ELIF,
  TOK_ELSE,
  TOK_WHILE,
  TOK_MAIN,

  // Types
  TOK_INT,
  TOK_FLOAT,
  TOK_CHAR,
  TOK_STRING,
  TOK_INT_ARR,
  TOK_FLOAT_ARR,
  TOK_CHAR_ARR,

  // Operator-body keywords
  TOK_LHS,
  TOK_RHS,
  TOK_OPERATOR, // the word "operator" inside Op blocks

  // Literals
  TOK_NUMBER,     // integer literal
  TOK_FLOAT_LIT,  // float literal
  TOK_CHAR_LIT,   // character literal
  TOK_STRING_LIT, // string literal

  // Identifier
  TOK_IDENTIFIER,

  // Operators
  TOK_PLUS,         // +
  TOK_MINUS,        // -
  TOK_STAR,         // *
  TOK_SLASH,        // /
  TOK_PERCENT,      // %
  TOK_BANG,         // !
  TOK_LT,           // <
  TOK_GT,           // >
  TOK_LE,           // <=
  TOK_GE,           // >=
  TOK_EQ,           // ==
  TOK_NE,           // !=
  TOK_AND,          // &&
  TOK_OR,           // ||
  TOK_ASSIGN,       // =
  TOK_COLON_ASSIGN, // :=

  // Punctuation
  TOK_LPAREN,    // (
  TOK_RPAREN,    // )
  TOK_LBRACE,    // {
  TOK_RBRACE,    // }
  TOK_LBRACKET,  // [
  TOK_RBRACKET,  // ]
  TOK_COMMA,     // ,
  TOK_SEMICOLON, // ;
  TOK_COLON,     // :
  TOK_CARET,     // ^

  // Special
  TOK_NEWLINE,
  TOK_EOF
};

struct Token {
  TokenType type;
  std::string lexeme;
  int line;
  int col;

  Token() : type(TokenType::TOK_EOF), line(0), col(0) {}
  Token(TokenType t, const std::string &lex, int ln, int c)
      : type(t), lexeme(lex), line(ln), col(c) {}
};

// ============================================================================
// Lexer class
// ============================================================================

class Lexer {
public:
  explicit Lexer(const std::string &source);
  std::vector<Token> tokenize();

private:
  std::string src;
  size_t pos;
  int line;
  int col;

  char current() const;
  char peek() const;
  char advance();
  void skipWhitespace();
  void skipLineComment();
  Token makeToken(TokenType t, const std::string &lex);
  Token readNumber();
  Token readString();
  Token readChar();
  Token readIdentifierOrKeyword();
  Token nextToken();
};

std::string tokenTypeName(TokenType t);
