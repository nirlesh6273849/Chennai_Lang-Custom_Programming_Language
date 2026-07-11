#pragma once
#include "token.h"
#include <memory>
#include <string>
#include <vector>
  // AST Node base classes
struct ASTNode {
  virtual ~ASTNode() = default;
};

struct Expr : ASTNode {};
struct Stmt : ASTNode {};

using ExprPtr = std::unique_ptr<Expr>;
using StmtPtr = std::unique_ptr<Stmt>;
  // Expression nodes
struct NumberExpr : Expr {
  int value;
  explicit NumberExpr(int v) : value(v) {}
};

struct FloatExpr : Expr {
  double value;
  explicit FloatExpr(double v) : value(v) {}
};

struct CharExpr : Expr {
  char value;
  explicit CharExpr(char v) : value(v) {}
};

struct StringExpr : Expr {
  std::string value;
  explicit StringExpr(const std::string &v) : value(v) {}
};

struct IdentifierExpr : Expr {
  std::string name;
  explicit IdentifierExpr(const std::string &n) : name(n) {}
};

struct BinaryExpr : Expr {
  std::string op; // "+", "-", "*", "/", "%", "<", ">", "<=", ">=", "==", "!=",
                  // "&&", "||", "^"
  ExprPtr lhs;
  ExprPtr rhs;
  BinaryExpr(const std::string &op, ExprPtr l, ExprPtr r)
      : op(op), lhs(std::move(l)), rhs(std::move(r)) {}
};

struct UnaryExpr : Expr {
  std::string op; // "!" or "-"
  ExprPtr operand;
  UnaryExpr(const std::string &op, ExprPtr e) : op(op), operand(std::move(e)) {}
};

struct CallExpr : Expr {
  std::string callee;
  std::vector<ExprPtr> args;
  CallExpr(const std::string &name, std::vector<ExprPtr> args)
      : callee(name), args(std::move(args)) {}
};

struct ArrayAccessExpr : Expr {
  std::string name;
  ExprPtr index;
  ArrayAccessExpr(const std::string &n, ExprPtr idx)
      : name(n), index(std::move(idx)) {}
};

struct AssignExpr : Expr {
  std::string name;
  ExprPtr value;
  AssignExpr(const std::string &n, ExprPtr v) : name(n), value(std::move(v)) {}
};
  // Statement nodes
struct BlockStmt : Stmt {
  std::vector<StmtPtr> statements;
  explicit BlockStmt(std::vector<StmtPtr> stmts)
      : statements(std::move(stmts)) {}
};

struct DeclarationStmt : Stmt {
  std::string typeName;
  std::string varName;
  DeclarationStmt(const std::string &type, const std::string &name)
      : typeName(type), varName(name) {}
};

struct AssignmentStmt : Stmt {
  std::string varName;
  ExprPtr value;
  AssignmentStmt(const std::string &name, ExprPtr v)
      : varName(name), value(std::move(v)) {}
};

struct ArrayAssignStmt : Stmt {
  std::string varName;
  ExprPtr index;
  ExprPtr value;
  ArrayAssignStmt(const std::string &name, ExprPtr idx, ExprPtr v)
      : varName(name), index(std::move(idx)), value(std::move(v)) {}
};

struct DeclAssignStmt : Stmt {
  std::string typeName;
  std::string varName;
  ExprPtr value;
  DeclAssignStmt(const std::string &type, const std::string &name, ExprPtr v)
      : typeName(type), varName(name), value(std::move(v)) {}
};

struct IfStmt : Stmt {
  ExprPtr condition;
  StmtPtr thenBlock;
  std::vector<std::pair<ExprPtr, StmtPtr>> elifs;
  StmtPtr elseBlock; // may be nullptr
  IfStmt(ExprPtr cond, StmtPtr then_,
         std::vector<std::pair<ExprPtr, StmtPtr>> elifs_, StmtPtr else_)
      : condition(std::move(cond)), thenBlock(std::move(then_)),
        elifs(std::move(elifs_)), elseBlock(std::move(else_)) {}
};

struct WhileStmt : Stmt {
  ExprPtr condition;
  StmtPtr body;
  WhileStmt(ExprPtr cond, StmtPtr body_)
      : condition(std::move(cond)), body(std::move(body_)) {}
};

struct PrintStmt : Stmt {
  // A list of string expressions to print, concatenated
  std::vector<ExprPtr> parts;
  explicit PrintStmt(std::vector<ExprPtr> parts_) : parts(std::move(parts_)) {}
};

struct ExprStmt : Stmt {
  ExprPtr expr;
  explicit ExprStmt(ExprPtr e) : expr(std::move(e)) {}
};

struct ReturnStmt : Stmt {
  ExprPtr value; // may be nullptr
  explicit ReturnStmt(ExprPtr v = nullptr) : value(std::move(v)) {}
};
  // Operator overload AST: stores the override expression tree
  // Represents the operator override definition body
  // e.g. Op(+) { operator := LHS ^ 2 + RHS }
  // The OExpression is stored as a regular Expr tree, but uses special
  // IdentifierExpr("__LHS__") and IdentifierExpr("__RHS__") placeholders
struct OperatorOverload {
  std::string op; // the operator being overloaded, e.g. "+"
  ExprPtr body;   // the replacement expression
};
  // Function definition
struct FunctionDef {
  std::string name; // function name or "main"
  std::vector<std::pair<std::string, std::string>> params; // (type, name) pairs
  std::unique_ptr<BlockStmt> body;

  FunctionDef(const std::string &name_,
              std::vector<std::pair<std::string, std::string>> params_,
              std::unique_ptr<BlockStmt> body_)
      : name(name_), params(std::move(params_)), body(std::move(body_)) {}
};
  // Program — top level
struct Program {
  std::vector<OperatorOverload> operators;
  std::vector<std::unique_ptr<FunctionDef>> functions;
};
