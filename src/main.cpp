#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "codegen.h"
#include "parser.h"
#include "token_reader.h"

// Prints the command line usage instructions
static void printUsage() {
  std::cerr << "Chennai Lang Compiler v2.0 (Java + C++ Hybrid)\n\n";
  std::cerr << "Usage:\n";
  std::cerr << "  chennai_lang <filename.ch>                  Compile to executable\n";
  std::cerr << "  chennai_lang --compile <filename.ch>        Compile to executable\n";
  std::cerr << "  chennai_lang --compile <filename.ch> -o out Compile with custom output name\n";
  std::cerr << "  chennai_lang --asm-only <filename.ch>       Generate .asm file only (no assemble/link)\n";
  std::cerr << "  chennai_lang --tokens <tokens.json>         Compile from pre-generated JSON token file\n\n";
}

// Reads the entire contents of a file into a string
static std::string readFile(const std::string &path) {
  std::ifstream file(path);
  if (!file.is_open()) {
    std::cerr << "Error: Cannot open file '" << path << "'\n";
    exit(1);
  }
  std::stringstream buf;
  buf << file.rdbuf();
  return buf.str();
}

// Writes a string to a file
static void writeFile(const std::string &path, const std::string &content) {
  std::ofstream file(path);
  if (!file.is_open()) {
    std::cerr << "Error: Cannot write file '" << path << "'\n";
    exit(1);
  }
  file << content;
}

// Extracts the base name from a file path
static std::string deriveBaseName(const std::string &path) {
  std::string name = path;
  size_t lastSlash = name.find_last_of("/\\");
  if (lastSlash != std::string::npos) {
    name = name.substr(lastSlash + 1);
  }
  size_t dot = name.find_last_of('.');
  if (dot != std::string::npos) {
    name = name.substr(0, dot);
  }
  return name;
}

// Executes the compilation pipeline: Parse -> CodeGen -> NASM -> Link
static int compilePipeline(std::vector<Token> &tokens,
                           const std::string &inputFile,
                           const std::string &outputName, bool asmOnly) {
  Parser parser(tokens);
  auto program = parser.parseProgram();

  CodeGenerator codegen;
  std::string asmCode = codegen.generate(*program);

  std::string baseName = outputName.empty() ? deriveBaseName(inputFile) : outputName;
  std::string asmFile = baseName + ".asm";

  writeFile(asmFile, asmCode);
  std::cout << "[Chennai Lang] Generated: " << asmFile << "\n";

  if (asmOnly) {
    return 0;
  }

  std::string objFile = baseName + ".obj";
  std::string nasmCmd = "nasm -f win64 \"" + asmFile + "\" -o \"" + objFile + "\"";
  std::cout << "[Chennai Lang] Assembling: " << nasmCmd << "\n";
  int nasmResult = system(nasmCmd.c_str());
  if (nasmResult != 0) {
    std::cerr << "Error: NASM assembly failed (exit code " << nasmResult << ")\n";
    std::cerr << "       Make sure NASM is installed and on your PATH.\n";
    return 1;
  }

  std::string exeFile = baseName + ".exe";
  std::string linkCmd = "gcc \"" + objFile + "\" -o \"" + exeFile + "\" -lmsvcrt";
  std::cout << "[Chennai Lang] Linking: " << linkCmd << "\n";
  int linkResult = system(linkCmd.c_str());
  if (linkResult != 0) {
    std::cerr << "Error: Linking failed (exit code " << linkResult << ")\n";
    std::cerr << "       Make sure GCC (MinGW-w64) is installed and on your PATH.\n";
    return 1;
  }

  std::cout << "[Chennai Lang] Successfully compiled: " << exeFile << "\n";
  return 0;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printUsage();
    return 1;
  }

  std::string mode = "";
  std::string inputFile = "";
  std::string outputName = "";

  for (int i = 1; i < argc; i++) {
    std::string arg = argv[i];
    if (arg == "--help" || arg == "-h") {
      printUsage();
      return 0;
    } else if (arg == "--compile" || arg == "--asm-only" || arg == "--tokens") {
      mode = arg;
    } else if (arg == "-o" && i + 1 < argc) {
      outputName = argv[++i];
    } else {
      inputFile = arg;
    }
  }

  if (inputFile.empty()) {
    std::cerr << "Error: No input file specified.\n";
    printUsage();
    return 1;
  }

  try {
    if (mode == "--tokens") {
      auto tokens = TokenReader::readFromFile(inputFile);
      return compilePipeline(tokens, inputFile, outputName, false);
    }

    std::string baseName = deriveBaseName(inputFile);
    std::string tokensFile = baseName + "_tokens.json";

    std::string javaCmd = "java -cp build/java com.chennai.lexer.ChennaiLexer \"" +
                          inputFile + "\" > \"" + tokensFile + "\" 2>NUL";
    int javaResult = system(javaCmd.c_str());

    if (javaResult != 0) {
      std::cerr << "Error: Java lexer failed to run. Ensure Java is installed and the lexer is compiled.\n";
      return 1;
    }

    std::vector<Token> tokens = TokenReader::readFromFile(tokensFile);
    std::cout << "[Chennai Lang] Lexer: Java (JSON bridge)\n";

    return compilePipeline(tokens, inputFile, outputName, mode == "--asm-only");

  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
