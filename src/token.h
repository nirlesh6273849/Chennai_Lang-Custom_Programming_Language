#pragma once
#include <string>

enum class TokenType {
  TOK_POLAMANNA,
  TOK_NIRUTHUANNA,
  TOK_OP,
  TOK_SOLLU,
  TOK_IF,
  TOK_ELIF,
  TOK_ELSE,
  TOK_WHILE,
  TOK_MAIN,
  TOK_INT,
  TOK_FLOAT,
  TOK_CHAR,
  TOK_STRING,
  TOK_INT_ARR,
  TOK_FLOAT_ARR,
  TOK_CHAR_ARR,
  TOK_LHS,
  TOK_RHS,
  TOK_OPERATOR,
  TOK_NUMBER,
  TOK_FLOAT_LIT,
  TOK_CHAR_LIT,
  TOK_STRING_LIT,
  TOK_IDENTIFIER,
  TOK_PLUS,
  TOK_MINUS,
  TOK_STAR,
  TOK_SLASH,
  TOK_PERCENT,
  TOK_BANG,
  TOK_LT,
  TOK_GT,
  TOK_LE,
  TOK_GE,
  TOK_EQ,
  TOK_NE,
  TOK_AND,
  TOK_OR,
  TOK_ASSIGN,
  TOK_COLON_ASSIGN,
  TOK_LPAREN,
  TOK_RPAREN,
  TOK_LBRACE,
  TOK_RBRACE,
  TOK_LBRACKET,
  TOK_RBRACKET,
  TOK_COMMA,
  TOK_SEMICOLON,
  TOK_COLON,
  TOK_CARET,
  TOK_NEWLINE,
  TOK_EOF,
  TOK_UNKNOWN
};

struct Token {
  TokenType type;
  std::string lexeme;
  int line;
  int col;

  Token() : type(TokenType::TOK_UNKNOWN), lexeme(""), line(0), col(0) {}
  Token(TokenType type, const std::string& lexeme, int line, int col)
      : type(type), lexeme(lexeme), line(line), col(col) {}
};
