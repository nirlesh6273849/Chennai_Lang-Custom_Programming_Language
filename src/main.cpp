#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "codegen.h"
#include "interpreter.h"
#include "lexer.h"
#include "parser.h"
#include "token_reader.h"

// ============================================================================
// Usage
// ============================================================================

static void printUsage() {
  std::cerr << "Chennai Lang Compiler v2.0 (Java + C++ Hybrid)\n";
  std::cerr << "================================================\n\n";
  std::cerr << "Usage:\n";
  std::cerr << "  chennai_lang <filename.ch>                  "
               " Compile (lex with C++ fallback + compile to NASM)\n";
  std::cerr << "  chennai_lang --compile <filename.ch>        "
               " Compile to NASM assembly + assemble + link\n";
  std::cerr << "  chennai_lang --compile <filename.ch> -o out "
               " Compile with custom output name\n";
  std::cerr << "  chennai_lang --interpret <filename.ch>      "
               " Interpret (legacy mode)\n";
  std::cerr << "  chennai_lang --asm-only <filename.ch>       "
               " Generate .asm file only (no assemble/link)\n";
  std::cerr << "  chennai_lang --tokens <tokens.json>         "
               " Compile from Java lexer JSON token file\n";
  std::cerr << "\n";
}

// ============================================================================
// Read source file helper
// ============================================================================

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

// ============================================================================
// Write output file helper
// ============================================================================

static void writeFile(const std::string &path, const std::string &content) {
  std::ofstream file(path);
  if (!file.is_open()) {
    std::cerr << "Error: Cannot write file '" << path << "'\n";
    exit(1);
  }
  file << content;
}

// ============================================================================
// Derive output filename from input
// ============================================================================

static std::string deriveBaseName(const std::string &path) {
  // Remove directory
  std::string name = path;
  size_t lastSlash = name.find_last_of("/\\");
  if (lastSlash != std::string::npos) {
    name = name.substr(lastSlash + 1);
  }
  // Remove extension
  size_t dot = name.find_last_of('.');
  if (dot != std::string::npos) {
    name = name.substr(0, dot);
  }
  return name;
}

// ============================================================================
// Compilation pipeline
// ============================================================================

static int compilePipeline(std::vector<Token> &tokens,
                           const std::string &inputFile,
                           const std::string &outputName, bool asmOnly) {
  // Parse
  Parser parser(tokens);
  auto program = parser.parseProgram();

  // Generate NASM assembly
  CodeGenerator codegen;
  std::string asmCode = codegen.generate(*program);

  std::string baseName =
      outputName.empty() ? deriveBaseName(inputFile) : outputName;
  std::string asmFile = baseName + ".asm";

  writeFile(asmFile, asmCode);
  std::cout << "[Chennai Lang] Generated: " << asmFile << "\n";

  if (asmOnly) {
    return 0;
  }

  // Assemble with NASM
  std::string objFile = baseName + ".obj";
  std::string nasmCmd =
      "nasm -f win64 \"" + asmFile + "\" -o \"" + objFile + "\"";
  std::cout << "[Chennai Lang] Assembling: " << nasmCmd << "\n";
  int nasmResult = system(nasmCmd.c_str());
  if (nasmResult != 0) {
    std::cerr << "Error: NASM assembly failed (exit code " << nasmResult
              << ")\n";
    std::cerr << "       Make sure NASM is installed and on your PATH.\n";
    return 1;
  }

  // Link with GCC (MinGW)
  std::string exeFile = baseName + ".exe";
  std::string linkCmd =
      "gcc \"" + objFile + "\" -o \"" + exeFile + "\" -lmsvcrt";
  std::cout << "[Chennai Lang] Linking: " << linkCmd << "\n";
  int linkResult = system(linkCmd.c_str());
  if (linkResult != 0) {
    std::cerr << "Error: Linking failed (exit code " << linkResult << ")\n";
    std::cerr
        << "       Make sure GCC (MinGW-w64) is installed and on your PATH.\n";
    return 1;
  }

  std::cout << "[Chennai Lang] Successfully compiled: " << exeFile << "\n";
  return 0;
}

// ============================================================================
// Main entry point
// ============================================================================

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printUsage();
    return 1;
  }

  std::string mode = "";
  std::string inputFile = "";
  std::string outputName = "";

  // Parse command-line arguments
  for (int i = 1; i < argc; i++) {
    std::string arg = argv[i];
    if (arg == "--help" || arg == "-h") {
      printUsage();
      return 0;
    } else if (arg == "--compile" || arg == "--interpret" ||
               arg == "--asm-only" || arg == "--tokens") {
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
    // ── Mode: Interpret (legacy) ──────────────────────────────────────────
    if (mode == "--interpret") {
      std::string source = readFile(inputFile);
      Lexer lexer(source);
      auto tokens = lexer.tokenize();
      Parser parser(tokens);
      auto program = parser.parseProgram();
      Interpreter interpreter;
      interpreter.run(*program);
      return 0;
    }

    // ── Mode: Compile from JSON tokens (from Java lexer) ──────────────────
    if (mode == "--tokens") {
      auto tokens = TokenReader::readFromFile(inputFile);
      return compilePipeline(tokens, inputFile, outputName, false);
    }

    // ── Mode: ASM only ────────────────────────────────────────────────────
    if (mode == "--asm-only") {
      std::string source = readFile(inputFile);
      Lexer lexer(source);
      auto tokens = lexer.tokenize();
      return compilePipeline(tokens, inputFile, outputName, true);
    }

    // ── Mode: Full compile (default) ──────────────────────────────────────
    // Try to use Java lexer first, fall back to C++ lexer
    {
      std::string baseName = deriveBaseName(inputFile);
      std::string tokensFile = baseName + "_tokens.json";
      bool usedJavaLexer = false;

      // Attempt Java lexer
      std::string javaCmd = "java -cp build/java com.chennai.lexer.ChennaiLexer \"" +
                            inputFile + "\" > \"" + tokensFile + "\" 2>NUL";
      int javaResult = system(javaCmd.c_str());

      std::vector<Token> tokens;

      if (javaResult == 0) {
        // Try to read Java lexer output
        try {
          tokens = TokenReader::readFromFile(tokensFile);
          usedJavaLexer = true;
          std::cout << "[Chennai Lang] Lexer: Java (JSON bridge)\n";
        } catch (...) {
          // Fall through to C++ lexer
        }
      }

      if (!usedJavaLexer) {
        // Fall back to C++ lexer
        std::string source = readFile(inputFile);
        Lexer lexer(source);
        tokens = lexer.tokenize();
        std::cout << "[Chennai Lang] Lexer: C++ (native fallback)\n";
      }

      return compilePipeline(tokens, inputFile, outputName,
                             mode == "--asm-only");
    }

  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
