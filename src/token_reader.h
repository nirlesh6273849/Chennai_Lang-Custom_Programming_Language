#pragma once
#include "lexer.h"
#include <string>
#include <vector>

// ============================================================================
// Token JSON Reader — bridges Java Lexer output → C++ Token vector
// ============================================================================

class TokenReader {
public:
  // Read a JSON token array from a file and return C++ Token objects
  static std::vector<Token> readFromFile(const std::string &filename);

  // Read a JSON token array from a string
  static std::vector<Token> readFromString(const std::string &json);

private:
  // Lightweight JSON parsing helpers
  static void skipWs(const std::string &s, size_t &pos);
  static std::string readJsonString(const std::string &s, size_t &pos);
  static int readJsonInt(const std::string &s, size_t &pos);
  static TokenType stringToTokenType(const std::string &name);
  static std::string unescapeJsonString(const std::string &s);
};
