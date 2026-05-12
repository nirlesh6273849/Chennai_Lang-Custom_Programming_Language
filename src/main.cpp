#include <fstream>
#include <iostream>
#include <sstream>
#include <string>


#include "interpreter.h"
#include "lexer.h"
#include "parser.h"


int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "Chennai Lang Interpreter v1.0\n";
    std::cerr << "Usage: chennai_lang <filename.ch>\n";
    return 1;
  }

  // Read source file
  std::ifstream file(argv[1]);
  if (!file.is_open()) {
    std::cerr << "Error: Cannot open file '" << argv[1] << "'\n";
    return 1;
  }

  std::stringstream buf;
  buf << file.rdbuf();
  std::string source = buf.str();

  try {
    // Lex
    Lexer lexer(source);
    auto tokens = lexer.tokenize();

    // Parse
    Parser parser(tokens);
    auto program = parser.parseProgram();

    // Interpret
    Interpreter interpreter;
    interpreter.run(*program);

  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
