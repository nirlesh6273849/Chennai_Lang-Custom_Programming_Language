#include "lexer.h"
#include <stdexcept>
#include <unordered_map>


// ============================================================================
// Keyword table
// ============================================================================

static const std::unordered_map<std::string, TokenType> keywords = {
    {"polamanna", TokenType::TOK_POLAMANNA},
    {"niruthuanna", TokenType::TOK_NIRUTHUANNA},
    {"Op", TokenType::TOK_OP},
    {"sollu", TokenType::TOK_SOLLU},
    {"if", TokenType::TOK_IF},
    {"elif", TokenType::TOK_ELIF},
    {"else", TokenType::TOK_ELSE},
    {"while", TokenType::TOK_WHILE},
    {"main", TokenType::TOK_MAIN},
    {"int", TokenType::TOK_INT},
    {"float", TokenType::TOK_FLOAT},
    {"char", TokenType::TOK_CHAR},
    {"string", TokenType::TOK_STRING},
    {"LHS", TokenType::TOK_LHS},
    {"RHS", TokenType::TOK_RHS},
    {"operator", TokenType::TOK_OPERATOR},
};

// ============================================================================
// Lexer implementation
// ============================================================================

Lexer::Lexer(const std::string &source)
    : src(source), pos(0), line(1), col(1) {}

char Lexer::current() const {
  if (pos >= src.size())
    return '\0';
  return src[pos];
}

char Lexer::peek() const {
  if (pos + 1 >= src.size())
    return '\0';
  return src[pos + 1];
}

char Lexer::advance() {
  char c = current();
  pos++;
  if (c == '\n') {
    line++;
    col = 1;
  } else {
    col++;
  }
  return c;
}

void Lexer::skipWhitespace() {
  while (pos < src.size()) {
    char c = current();
    if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
      advance();
    } else if (c == '/' && peek() == '/') {
      skipLineComment();
    } else {
      break;
    }
  }
}

void Lexer::skipLineComment() {
  // skip until end of line
  while (pos < src.size() && current() != '\n') {
    advance();
  }
}

Token Lexer::makeToken(TokenType t, const std::string &lex) {
  return Token(t, lex, line, col);
}

Token Lexer::readNumber() {
  int startCol = col;
  std::string num;
  while (pos < src.size() && isdigit(current())) {
    num += advance();
  }
  // Check for float
  if (current() == '.' && isdigit(peek())) {
    num += advance(); // consume '.'
    while (pos < src.size() && isdigit(current())) {
      num += advance();
    }
    return Token(TokenType::TOK_FLOAT_LIT, num, line, startCol);
  }
  return Token(TokenType::TOK_NUMBER, num, line, startCol);
}

Token Lexer::readString() {
  int startCol = col;
  advance(); // consume opening quote
  std::string str;
  while (pos < src.size() && current() != '"') {
    if (current() == '\\') {
      advance();
      char esc = advance();
      switch (esc) {
      case 'n':
        str += '\n';
        break;
      case 't':
        str += '\t';
        break;
      case '\\':
        str += '\\';
        break;
      case '"':
        str += '"';
        break;
      default:
        str += esc;
        break;
      }
    } else {
      str += advance();
    }
  }
  if (pos >= src.size())
    throw std::runtime_error("Unterminated string at line " +
                             std::to_string(line));
  advance(); // consume closing quote
  return Token(TokenType::TOK_STRING_LIT, str, line, startCol);
}

Token Lexer::readChar() {
  int startCol = col;
  advance(); // consume opening quote
  std::string ch;
  if (current() == '\\') {
    advance();
    char esc = advance();
    switch (esc) {
    case 'n':
      ch = "\n";
      break;
    case 't':
      ch = "\t";
      break;
    case '\\':
      ch = "\\";
      break;
    case '\'':
      ch = "'";
      break;
    default:
      ch = std::string(1, esc);
      break;
    }
  } else {
    ch = std::string(1, advance());
  }
  if (current() != '\'')
    throw std::runtime_error("Unterminated char literal at line " +
                             std::to_string(line));
  advance(); // consume closing quote
  return Token(TokenType::TOK_CHAR_LIT, ch, line, startCol);
}

Token Lexer::readIdentifierOrKeyword() {
  int startCol = col;
  std::string id;
  while (pos < src.size() && (isalnum(current()) || current() == '_')) {
    id += advance();
  }

  // Check for array types: "int[]", "float[]", "char[]"
  if ((id == "int" || id == "float" || id == "char") && current() == '[' &&
      peek() == ']') {
    advance();
    advance(); // consume []
    if (id == "int")
      return Token(TokenType::TOK_INT_ARR, "int[]", line, startCol);
    if (id == "float")
      return Token(TokenType::TOK_FLOAT_ARR, "float[]", line, startCol);
    if (id == "char")
      return Token(TokenType::TOK_CHAR_ARR, "char[]", line, startCol);
  }

  // Check keywords
  auto it = keywords.find(id);
  if (it != keywords.end()) {
    return Token(it->second, id, line, startCol);
  }

  return Token(TokenType::TOK_IDENTIFIER, id, line, startCol);
}

Token Lexer::nextToken() {
  skipWhitespace();
  if (pos >= src.size())
    return makeToken(TokenType::TOK_EOF, "");

  int startCol = col;
  char c = current();

  // Numbers
  if (isdigit(c))
    return readNumber();

  // String literal
  if (c == '"')
    return readString();

  // Char literal
  if (c == '\'')
    return readChar();

  // Identifiers / keywords
  if (isalpha(c) || c == '_')
    return readIdentifierOrKeyword();

  // Two-character operators
  if (c == '<' && peek() == '=') {
    advance();
    advance();
    return Token(TokenType::TOK_LE, "<=", line, startCol);
  }
  if (c == '>' && peek() == '=') {
    advance();
    advance();
    return Token(TokenType::TOK_GE, ">=", line, startCol);
  }
  if (c == '=' && peek() == '=') {
    advance();
    advance();
    return Token(TokenType::TOK_EQ, "==", line, startCol);
  }
  if (c == '!' && peek() == '=') {
    advance();
    advance();
    return Token(TokenType::TOK_NE, "!=", line, startCol);
  }
  if (c == '&' && peek() == '&') {
    advance();
    advance();
    return Token(TokenType::TOK_AND, "&&", line, startCol);
  }
  if (c == '|' && peek() == '|') {
    advance();
    advance();
    return Token(TokenType::TOK_OR, "||", line, startCol);
  }
  if (c == ':' && peek() == '=') {
    advance();
    advance();
    return Token(TokenType::TOK_COLON_ASSIGN, ":=", line, startCol);
  }

  // Single-character tokens
  advance();
  switch (c) {
  case '+':
    return Token(TokenType::TOK_PLUS, "+", line, startCol);
  case '-':
    return Token(TokenType::TOK_MINUS, "-", line, startCol);
  case '*':
    return Token(TokenType::TOK_STAR, "*", line, startCol);
  case '/':
    return Token(TokenType::TOK_SLASH, "/", line, startCol);
  case '%':
    return Token(TokenType::TOK_PERCENT, "%", line, startCol);
  case '!':
    return Token(TokenType::TOK_BANG, "!", line, startCol);
  case '<':
    return Token(TokenType::TOK_LT, "<", line, startCol);
  case '>':
    return Token(TokenType::TOK_GT, ">", line, startCol);
  case '=':
    return Token(TokenType::TOK_ASSIGN, "=", line, startCol);
  case '(':
    return Token(TokenType::TOK_LPAREN, "(", line, startCol);
  case ')':
    return Token(TokenType::TOK_RPAREN, ")", line, startCol);
  case '{':
    return Token(TokenType::TOK_LBRACE, "{", line, startCol);
  case '}':
    return Token(TokenType::TOK_RBRACE, "}", line, startCol);
  case '[':
    return Token(TokenType::TOK_LBRACKET, "[", line, startCol);
  case ']':
    return Token(TokenType::TOK_RBRACKET, "]", line, startCol);
  case ',':
    return Token(TokenType::TOK_COMMA, ",", line, startCol);
  case ';':
    return Token(TokenType::TOK_SEMICOLON, ";", line, startCol);
  case ':':
    return Token(TokenType::TOK_COLON, ":", line, startCol);
  case '^':
    return Token(TokenType::TOK_CARET, "^", line, startCol);
  default:
    throw std::runtime_error("Unexpected character '" + std::string(1, c) +
                             "' at line " + std::to_string(line) + " col " +
                             std::to_string(startCol));
  }
}

std::vector<Token> Lexer::tokenize() {
  std::vector<Token> tokens;
  while (true) {
    Token tok = nextToken();
    tokens.push_back(tok);
    if (tok.type == TokenType::TOK_EOF)
      break;
  }
  return tokens;
}

// ============================================================================
// Debug helper
// ============================================================================

std::string tokenTypeName(TokenType t) {
  switch (t) {
  case TokenType::TOK_POLAMANNA:
    return "POLAMANNA";
  case TokenType::TOK_NIRUTHUANNA:
    return "NIRUTHUANNA";
  case TokenType::TOK_OP:
    return "OP";
  case TokenType::TOK_SOLLU:
    return "SOLLU";
  case TokenType::TOK_IF:
    return "IF";
  case TokenType::TOK_ELIF:
    return "ELIF";
  case TokenType::TOK_ELSE:
    return "ELSE";
  case TokenType::TOK_WHILE:
    return "WHILE";
  case TokenType::TOK_MAIN:
    return "MAIN";
  case TokenType::TOK_INT:
    return "INT";
  case TokenType::TOK_FLOAT:
    return "FLOAT";
  case TokenType::TOK_CHAR:
    return "CHAR";
  case TokenType::TOK_STRING:
    return "STRING";
  case TokenType::TOK_INT_ARR:
    return "INT_ARR";
  case TokenType::TOK_FLOAT_ARR:
    return "FLOAT_ARR";
  case TokenType::TOK_CHAR_ARR:
    return "CHAR_ARR";
  case TokenType::TOK_LHS:
    return "LHS";
  case TokenType::TOK_RHS:
    return "RHS";
  case TokenType::TOK_OPERATOR:
    return "OPERATOR";
  case TokenType::TOK_NUMBER:
    return "NUMBER";
  case TokenType::TOK_FLOAT_LIT:
    return "FLOAT_LIT";
  case TokenType::TOK_CHAR_LIT:
    return "CHAR_LIT";
  case TokenType::TOK_STRING_LIT:
    return "STRING_LIT";
  case TokenType::TOK_IDENTIFIER:
    return "IDENTIFIER";
  case TokenType::TOK_PLUS:
    return "PLUS";
  case TokenType::TOK_MINUS:
    return "MINUS";
  case TokenType::TOK_STAR:
    return "STAR";
  case TokenType::TOK_SLASH:
    return "SLASH";
  case TokenType::TOK_PERCENT:
    return "PERCENT";
  case TokenType::TOK_BANG:
    return "BANG";
  case TokenType::TOK_LT:
    return "LT";
  case TokenType::TOK_GT:
    return "GT";
  case TokenType::TOK_LE:
    return "LE";
  case TokenType::TOK_GE:
    return "GE";
  case TokenType::TOK_EQ:
    return "EQ";
  case TokenType::TOK_NE:
    return "NE";
  case TokenType::TOK_AND:
    return "AND";
  case TokenType::TOK_OR:
    return "OR";
  case TokenType::TOK_ASSIGN:
    return "ASSIGN";
  case TokenType::TOK_COLON_ASSIGN:
    return "COLON_ASSIGN";
  case TokenType::TOK_LPAREN:
    return "LPAREN";
  case TokenType::TOK_RPAREN:
    return "RPAREN";
  case TokenType::TOK_LBRACE:
    return "LBRACE";
  case TokenType::TOK_RBRACE:
    return "RBRACE";
  case TokenType::TOK_LBRACKET:
    return "LBRACKET";
  case TokenType::TOK_RBRACKET:
    return "RBRACKET";
  case TokenType::TOK_COMMA:
    return "COMMA";
  case TokenType::TOK_SEMICOLON:
    return "SEMICOLON";
  case TokenType::TOK_COLON:
    return "COLON";
  case TokenType::TOK_CARET:
    return "CARET";
  case TokenType::TOK_NEWLINE:
    return "NEWLINE";
  case TokenType::TOK_EOF:
    return "EOF";
  default:
    return "UNKNOWN";
  }
}
