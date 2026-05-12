#pragma once
#include "ast.h"
#include "value.h"
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

class Interpreter {
public:
  Interpreter();
  void run(const Program &program);

private:
  // ── Scope / environment ────────────────────────────────────────────────
  using Scope = std::unordered_map<std::string, Value>;
  std::vector<Scope> scopes;

  void pushScope();
  void popScope();
  void declareVar(const std::string &name, const Value &val);
  Value &lookupVar(const std::string &name);
  bool hasVar(const std::string &name) const;

  // ── Operator overrides ─────────────────────────────────────────────────
  // Maps operator string (e.g. "+") to the override expression body
  std::unordered_map<std::string, const Expr *> opOverrides;
  bool inOverrideEval; // prevents infinite recursion in overrides

  // ── Function table ─────────────────────────────────────────────────────
  std::unordered_map<std::string, const FunctionDef *> functions;

  // ── Statement execution ────────────────────────────────────────────────
  void execBlock(const BlockStmt *block);
  void execStatement(const Stmt *stmt);
  void execDeclaration(const DeclarationStmt *stmt);
  void execAssignment(const AssignmentStmt *stmt);
  void execArrayAssign(const ArrayAssignStmt *stmt);
  void execDeclAssign(const DeclAssignStmt *stmt);
  void execIf(const IfStmt *stmt);
  void execWhile(const WhileStmt *stmt);
  void execPrint(const PrintStmt *stmt);

  // ── Expression evaluation ──────────────────────────────────────────────
  Value eval(const Expr *expr);
  Value evalBinary(const BinaryExpr *expr);
  Value evalUnary(const UnaryExpr *expr);
  Value evalCall(const CallExpr *expr);
  Value evalArrayAccess(const ArrayAccessExpr *expr);

  Value defaultForType(const std::string &typeName);
};
