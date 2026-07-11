#include "token_reader.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

// ============================================================================
// TokenType string → enum mapping
// ============================================================================

static const std::unordered_map<std::string, TokenType> tokenTypeMap = {
    {"TOK_POLAMANNA", TokenType::TOK_POLAMANNA},
    {"TOK_NIRUTHUANNA", TokenType::TOK_NIRUTHUANNA},
    {"TOK_OP", TokenType::TOK_OP},
    {"TOK_SOLLU", TokenType::TOK_SOLLU},
    {"TOK_IF", TokenType::TOK_IF},
    {"TOK_ELIF", TokenType::TOK_ELIF},
    {"TOK_ELSE", TokenType::TOK_ELSE},
    {"TOK_WHILE", TokenType::TOK_WHILE},
    {"TOK_MAIN", TokenType::TOK_MAIN},
    {"TOK_INT", TokenType::TOK_INT},
    {"TOK_FLOAT", TokenType::TOK_FLOAT},
    {"TOK_CHAR", TokenType::TOK_CHAR},
    {"TOK_STRING", TokenType::TOK_STRING},
    {"TOK_INT_ARR", TokenType::TOK_INT_ARR},
    {"TOK_FLOAT_ARR", TokenType::TOK_FLOAT_ARR},
    {"TOK_CHAR_ARR", TokenType::TOK_CHAR_ARR},
    {"TOK_LHS", TokenType::TOK_LHS},
    {"TOK_RHS", TokenType::TOK_RHS},
    {"TOK_OPERATOR", TokenType::TOK_OPERATOR},
    {"TOK_NUMBER", TokenType::TOK_NUMBER},
    {"TOK_FLOAT_LIT", TokenType::TOK_FLOAT_LIT},
    {"TOK_CHAR_LIT", TokenType::TOK_CHAR_LIT},
    {"TOK_STRING_LIT", TokenType::TOK_STRING_LIT},
    {"TOK_IDENTIFIER", TokenType::TOK_IDENTIFIER},
    {"TOK_PLUS", TokenType::TOK_PLUS},
    {"TOK_MINUS", TokenType::TOK_MINUS},
    {"TOK_STAR", TokenType::TOK_STAR},
    {"TOK_SLASH", TokenType::TOK_SLASH},
    {"TOK_PERCENT", TokenType::TOK_PERCENT},
    {"TOK_BANG", TokenType::TOK_BANG},
    {"TOK_LT", TokenType::TOK_LT},
    {"TOK_GT", TokenType::TOK_GT},
    {"TOK_LE", TokenType::TOK_LE},
    {"TOK_GE", TokenType::TOK_GE},
    {"TOK_EQ", TokenType::TOK_EQ},
    {"TOK_NE", TokenType::TOK_NE},
    {"TOK_AND", TokenType::TOK_AND},
    {"TOK_OR", TokenType::TOK_OR},
    {"TOK_ASSIGN", TokenType::TOK_ASSIGN},
    {"TOK_COLON_ASSIGN", TokenType::TOK_COLON_ASSIGN},
    {"TOK_LPAREN", TokenType::TOK_LPAREN},
    {"TOK_RPAREN", TokenType::TOK_RPAREN},
    {"TOK_LBRACE", TokenType::TOK_LBRACE},
    {"TOK_RBRACE", TokenType::TOK_RBRACE},
    {"TOK_LBRACKET", TokenType::TOK_LBRACKET},
    {"TOK_RBRACKET", TokenType::TOK_RBRACKET},
    {"TOK_COMMA", TokenType::TOK_COMMA},
    {"TOK_SEMICOLON", TokenType::TOK_SEMICOLON},
    {"TOK_COLON", TokenType::TOK_COLON},
    {"TOK_CARET", TokenType::TOK_CARET},
    {"TOK_NEWLINE", TokenType::TOK_NEWLINE},
    {"TOK_EOF", TokenType::TOK_EOF},
};

// ============================================================================
// JSON parsing helpers (minimal, no dependencies)
// ============================================================================

void TokenReader::skipWs(const std::string &s, size_t &pos) {
  while (pos < s.size() &&
         (s[pos] == ' ' || s[pos] == '\t' || s[pos] == '\n' ||
          s[pos] == '\r')) {
    pos++;
  }
}

std::string TokenReader::unescapeJsonString(const std::string &s) {
  std::string result;
  for (size_t i = 0; i < s.size(); i++) {
    if (s[i] == '\\' && i + 1 < s.size()) {
      i++;
      switch (s[i]) {
      case '"':
        result += '"';
        break;
      case '\\':
        result += '\\';
        break;
      case 'n':
        result += '\n';
        break;
      case 'r':
        result += '\r';
        break;
      case 't':
        result += '\t';
        break;
      case 'u': {
        // Parse \uXXXX
        if (i + 4 < s.size()) {
          std::string hex = s.substr(i + 1, 4);
          char ch = static_cast<char>(std::stoi(hex, nullptr, 16));
          result += ch;
          i += 4;
        }
        break;
      }
      default:
        result += s[i];
        break;
      }
    } else {
      result += s[i];
    }
  }
  return result;
}

std::string TokenReader::readJsonString(const std::string &s, size_t &pos) {
  if (pos >= s.size() || s[pos] != '"')
    throw std::runtime_error("Expected '\"' in JSON");

  pos++; // consume opening "
  std::string result;
  while (pos < s.size() && s[pos] != '"') {
    if (s[pos] == '\\') {
      result += s[pos++]; // add backslash
      if (pos < s.size()) {
        result += s[pos++]; // add escaped char
      }
    } else {
      result += s[pos++];
    }
  }

  if (pos >= s.size())
    throw std::runtime_error("Unterminated string in JSON");

  pos++; // consume closing "
  return unescapeJsonString(result);
}

int TokenReader::readJsonInt(const std::string &s, size_t &pos) {
  size_t start = pos;
  if (pos < s.size() && (s[pos] == '-' || s[pos] == '+'))
    pos++;
  while (pos < s.size() && isdigit(s[pos]))
    pos++;
  return std::stoi(s.substr(start, pos - start));
}

TokenType TokenReader::stringToTokenType(const std::string &name) {
  auto it = tokenTypeMap.find(name);
  if (it != tokenTypeMap.end())
    return it->second;
  throw std::runtime_error("Unknown token type: " + name);
}

// ============================================================================
// Parse JSON token array
// ============================================================================

std::vector<Token> TokenReader::readFromString(const std::string &json) {
  std::vector<Token> tokens;
  size_t pos = 0;

  skipWs(json, pos);
  if (pos >= json.size() || json[pos] != '[')
    throw std::runtime_error("Expected '[' at start of JSON token array");
  pos++; // consume [

  skipWs(json, pos);

  while (pos < json.size() && json[pos] != ']') {
    // Parse one token object: { "type": "...", "lexeme": "...", "line": N,
    // "col": N }
    skipWs(json, pos);
    if (json[pos] != '{')
      throw std::runtime_error("Expected '{' for token object");
    pos++; // consume {

    std::string type_str;
    std::string lexeme_str;
    int line_val = 0;
    int col_val = 0;

    // Parse key-value pairs
    while (pos < json.size() && json[pos] != '}') {
      skipWs(json, pos);
      std::string key = readJsonString(json, pos);

      skipWs(json, pos);
      if (json[pos] != ':')
        throw std::runtime_error("Expected ':' in JSON object");
      pos++; // consume :
      skipWs(json, pos);

      if (key == "type") {
        type_str = readJsonString(json, pos);
      } else if (key == "lexeme") {
        lexeme_str = readJsonString(json, pos);
      } else if (key == "line") {
        line_val = readJsonInt(json, pos);
      } else if (key == "col") {
        col_val = readJsonInt(json, pos);
      } else {
        // Skip unknown value (simple: skip until comma or })
        while (pos < json.size() && json[pos] != ',' && json[pos] != '}')
          pos++;
      }

      skipWs(json, pos);
      if (pos < json.size() && json[pos] == ',')
        pos++; // consume comma between fields
    }

    if (pos < json.size() && json[pos] == '}')
      pos++; // consume }

    // Create Token
    TokenType ttype = stringToTokenType(type_str);
    tokens.push_back(Token(ttype, lexeme_str, line_val, col_val));

    skipWs(json, pos);
    if (pos < json.size() && json[pos] == ',')
      pos++; // consume comma between tokens
  }

  if (pos < json.size() && json[pos] == ']')
    pos++; // consume ]

  return tokens;
}

std::vector<Token> TokenReader::readFromFile(const std::string &filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    throw std::runtime_error("Cannot open token file: " + filename);
  }

  std::stringstream buf;
  buf << file.rdbuf();
  return readFromString(buf.str());
}
