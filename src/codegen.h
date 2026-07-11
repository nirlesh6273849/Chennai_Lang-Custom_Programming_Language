#pragma once
#include "ast.h"
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
  // NASM x86-64 Code Generator for Chennai Lang
//
  // Walks the AST and emits NASM assembly targeting x86-64 Windows (win64).
  // Uses the Microsoft x64 calling convention for interop with C runtime
  // (printf, malloc, etc.)
//
  // Strategy:
  //   - All variables are 8 bytes (qword) on the stack
  //   - Expressions evaluated into RAX (result register)
  //   - Strings stored as null-terminated literals in .data section
  //   - Printf called via extern for sollu (print)
  //   - Integer-only arithmetic in first version
class CodeGenerator {
public:
  CodeGenerator();

  // Generate NASM assembly for the entire program
  std::string generate(const Program &program);

private:
  // Output stream
  std::ostringstream dataSection;  // .data section (string literals, formats)
  std::ostringstream bssSection;   // .bss section (uninitialized data)
  std::ostringstream textSection;  // .text section (code)
  // Label management
  int labelCounter;
  std::string newLabel(const std::string &prefix);
  // String literal table
  int stringCounter;
  std::unordered_map<std::string, std::string> stringLiterals;
  std::string addStringLiteral(const std::string &str);
  // Variable / symbol table
  struct VarInfo {
    int stackOffset; // offset from RBP (negative)
    std::string type; // "int", "char", "string", "int[]", etc.
  };

  struct Scope {
    std::unordered_map<std::string, VarInfo> vars;
    int nextOffset; // next available stack offset
  };

  std::vector<Scope> scopes;
  void pushScope();
  void popScope();
  VarInfo &declareVar(const std::string &name, const std::string &type);
  VarInfo *lookupVar(const std::string &name);
  // Function tracking
  std::unordered_map<std::string, const FunctionDef *> functions;
  const FunctionDef *currentFunction;
  int currentStackSize; // total stack allocation for current function
  // Operator overloads
  std::unordered_map<std::string, const Expr *> opOverrides;
  bool inOverrideEval; // prevents infinite recursion in override codegen
  // Code emission helpers
  void emit(const std::string &line);
  void emitLabel(const std::string &label);
  void emitComment(const std::string &comment);
  // Top-level generation
  void genProgram(const Program &program);
  void genFunction(const FunctionDef *fn);
  // Statement generation
  void genBlock(const BlockStmt *block);
  void genStatement(const Stmt *stmt);
  void genDeclaration(const DeclarationStmt *stmt);
  void genAssignment(const AssignmentStmt *stmt);
  void genArrayAssign(const ArrayAssignStmt *stmt);
  void genDeclAssign(const DeclAssignStmt *stmt);
  void genIf(const IfStmt *stmt);
  void genWhile(const WhileStmt *stmt);
  void genPrint(const PrintStmt *stmt);
  void genExprStmt(const ExprStmt *stmt);
  // Expression generation (result in RAX)
  void genExpr(const Expr *expr);
  void genBinary(const BinaryExpr *expr);
  void genUnary(const UnaryExpr *expr);
  void genCall(const CallExpr *expr);
  void genArrayAccess(const ArrayAccessExpr *expr);
  // Helpers
  void genPrintValue(const Expr *expr, const std::string &type);
  std::string inferExprType(const Expr *expr);
};
