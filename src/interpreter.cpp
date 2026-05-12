#include "interpreter.h"
#include <cmath>
#include <iostream>
#include <stdexcept>

// ============================================================================
// Constructor
// ============================================================================

Interpreter::Interpreter() : inOverrideEval(false) {
  pushScope(); // global scope
}

// ============================================================================
// Scope management
// ============================================================================

void Interpreter::pushScope() { scopes.push_back(Scope()); }

void Interpreter::popScope() { scopes.pop_back(); }

void Interpreter::declareVar(const std::string &name, const Value &val) {
  scopes.back()[name] = val;
}

Value &Interpreter::lookupVar(const std::string &name) {
  for (int i = static_cast<int>(scopes.size()) - 1; i >= 0; i--) {
    auto it = scopes[i].find(name);
    if (it != scopes[i].end())
      return it->second;
  }
  throw std::runtime_error("Undefined variable: " + name);
}

bool Interpreter::hasVar(const std::string &name) const {
  for (int i = static_cast<int>(scopes.size()) - 1; i >= 0; i--) {
    if (scopes[i].count(name))
      return true;
  }
  return false;
}

// ============================================================================
// Default values for types
// ============================================================================

Value Interpreter::defaultForType(const std::string &typeName) {
  if (typeName == "int")
    return Value(0);
  if (typeName == "float")
    return Value(0.0);
  if (typeName == "char")
    return Value('\0');
  if (typeName == "string")
    return Value(std::string(""));
  if (typeName == "int[]")
    return Value::makeIntArray({});
  if (typeName == "float[]")
    return Value::makeFloatArray({});
  if (typeName == "char[]")
    return Value::makeCharArray({});
  throw std::runtime_error("Unknown type: " + typeName);
}

// ============================================================================
// Entry point: run the program
// ============================================================================

void Interpreter::run(const Program &program) {
  // Register operator overrides
  for (auto &ov : program.operators) {
    opOverrides[ov.op] = ov.body.get();
  }

  // Register functions
  for (auto &fn : program.functions) {
    functions[fn->name] = fn.get();
  }

  // Find and execute main
  auto it = functions.find("main");
  if (it == functions.end()) {
    throw std::runtime_error("No 'main' function found");
  }

  execBlock(it->second->body.get());
}

// ============================================================================
// Block execution
// ============================================================================

void Interpreter::execBlock(const BlockStmt *block) {
  pushScope();
  for (auto &stmt : block->statements) {
    execStatement(stmt.get());
  }
  popScope();
}

// ============================================================================
// Statement dispatch
// ============================================================================

void Interpreter::execStatement(const Stmt *stmt) {
  if (auto *s = dynamic_cast<const DeclarationStmt *>(stmt)) {
    execDeclaration(s);
  } else if (auto *s = dynamic_cast<const AssignmentStmt *>(stmt)) {
    execAssignment(s);
  } else if (auto *s = dynamic_cast<const ArrayAssignStmt *>(stmt)) {
    execArrayAssign(s);
  } else if (auto *s = dynamic_cast<const DeclAssignStmt *>(stmt)) {
    execDeclAssign(s);
  } else if (auto *s = dynamic_cast<const IfStmt *>(stmt)) {
    execIf(s);
  } else if (auto *s = dynamic_cast<const WhileStmt *>(stmt)) {
    execWhile(s);
  } else if (auto *s = dynamic_cast<const PrintStmt *>(stmt)) {
    execPrint(s);
  } else if (auto *s = dynamic_cast<const BlockStmt *>(stmt)) {
    execBlock(s);
  } else if (auto *s = dynamic_cast<const ExprStmt *>(stmt)) {
    eval(s->expr.get());
  } else {
    throw std::runtime_error("Unknown statement type");
  }
}

void Interpreter::execDeclaration(const DeclarationStmt *stmt) {
  declareVar(stmt->varName, defaultForType(stmt->typeName));
}

void Interpreter::execAssignment(const AssignmentStmt *stmt) {
  Value val = eval(stmt->value.get());
  lookupVar(stmt->varName) = val;
}

void Interpreter::execArrayAssign(const ArrayAssignStmt *stmt) {
  Value &arr = lookupVar(stmt->varName);
  int idx = eval(stmt->index.get()).toInt();

  Value val = eval(stmt->value.get());

  switch (arr.type) {
  case ValueType::INT_ARRAY:
    if (idx < 0)
      throw std::runtime_error("Negative array index");
    if (static_cast<size_t>(idx) >= arr.intArr.size())
      arr.intArr.resize(idx + 1, 0);
    arr.intArr[idx] = val.toInt();
    break;
  case ValueType::FLOAT_ARRAY:
    if (idx < 0)
      throw std::runtime_error("Negative array index");
    if (static_cast<size_t>(idx) >= arr.floatArr.size())
      arr.floatArr.resize(idx + 1, 0.0);
    arr.floatArr[idx] = val.toDouble();
    break;
  case ValueType::CHAR_ARRAY:
    if (idx < 0)
      throw std::runtime_error("Negative array index");
    if (static_cast<size_t>(idx) >= arr.charArr.size())
      arr.charArr.resize(idx + 1, '\0');
    arr.charArr[idx] = val.charVal;
    break;
  default:
    throw std::runtime_error("Cannot index non-array variable: " +
                             stmt->varName);
  }
}

void Interpreter::execDeclAssign(const DeclAssignStmt *stmt) {
  Value val = eval(stmt->value.get());
  declareVar(stmt->varName, val);
}

void Interpreter::execIf(const IfStmt *stmt) {
  Value cond = eval(stmt->condition.get());
  if (cond.toBool()) {
    execBlock(dynamic_cast<const BlockStmt *>(stmt->thenBlock.get()));
    return;
  }

  for (size_t i = 0; i < stmt->elifs.size(); i++) {
    Value ec = eval(stmt->elifs[i].first.get());
    if (ec.toBool()) {
      execBlock(dynamic_cast<const BlockStmt *>(stmt->elifs[i].second.get()));
      return;
    }
  }

  if (stmt->elseBlock) {
    execBlock(dynamic_cast<const BlockStmt *>(stmt->elseBlock.get()));
  }
}

void Interpreter::execWhile(const WhileStmt *stmt) {
  while (eval(stmt->condition.get()).toBool()) {
    execBlock(dynamic_cast<const BlockStmt *>(stmt->body.get()));
  }
}

void Interpreter::execPrint(const PrintStmt *stmt) {
  std::string output;
  for (auto &part : stmt->parts) {
    Value v = eval(part.get());
    output += v.toString();
  }
  std::cout << output << std::endl;
}

// ============================================================================
// Expression evaluation
// ============================================================================

Value Interpreter::eval(const Expr *expr) {
  if (auto *e = dynamic_cast<const NumberExpr *>(expr)) {
    return Value(e->value);
  }
  if (auto *e = dynamic_cast<const FloatExpr *>(expr)) {
    return Value(e->value);
  }
  if (auto *e = dynamic_cast<const CharExpr *>(expr)) {
    return Value(e->value);
  }
  if (auto *e = dynamic_cast<const StringExpr *>(expr)) {
    return Value(e->value);
  }
  if (auto *e = dynamic_cast<const IdentifierExpr *>(expr)) {
    return lookupVar(e->name);
  }
  if (auto *e = dynamic_cast<const BinaryExpr *>(expr)) {
    return evalBinary(e);
  }
  if (auto *e = dynamic_cast<const UnaryExpr *>(expr)) {
    return evalUnary(e);
  }
  if (auto *e = dynamic_cast<const CallExpr *>(expr)) {
    return evalCall(e);
  }
  if (auto *e = dynamic_cast<const ArrayAccessExpr *>(expr)) {
    return evalArrayAccess(e);
  }
  if (auto *e = dynamic_cast<const AssignExpr *>(expr)) {
    Value val = eval(e->value.get());
    lookupVar(e->name) = val;
    return val;
  }
  throw std::runtime_error("Unknown expression type");
}

// ============================================================================
// Binary expression — with operator override support
// ============================================================================

Value Interpreter::evalBinary(const BinaryExpr *expr) {
  // ── Evaluate both sides first ────────────────────────────────────────
  Value left = eval(expr->lhs.get());
  Value right = eval(expr->rhs.get());

  // ── String concatenation with + always uses default behavior ─────────
  if (expr->op == "+" &&
      (left.type == ValueType::STRING || right.type == ValueType::STRING)) {
    return Value(left.toString() + right.toString());
  }

  // ── Check for operator override (numeric types only) ─────────────────
  if (!inOverrideEval) {
    auto ovIt = opOverrides.find(expr->op);
    if (ovIt != opOverrides.end()) {
      bool lhsNumeric =
          (left.type == ValueType::INT || left.type == ValueType::FLOAT ||
           left.type == ValueType::CHAR);
      bool rhsNumeric =
          (right.type == ValueType::INT || right.type == ValueType::FLOAT ||
           right.type == ValueType::CHAR);
      if (lhsNumeric && rhsNumeric) {
        pushScope();
        declareVar("__LHS__", left);
        declareVar("__RHS__", right);

        inOverrideEval = true;
        Value result = eval(ovIt->second);
        inOverrideEval = false;
        popScope();
        return result;
      }
    }
  }

  // ── Power operator ───────────────────────────────────────────────────
  if (expr->op == "^") {
    return Value(std::pow(left.toDouble(), right.toDouble()));
  }

  // ── Numeric operations — promote to double if either side is float ───
  bool useFloat =
      (left.type == ValueType::FLOAT || right.type == ValueType::FLOAT);

  if (useFloat) {
    double l = left.toDouble();
    double r = right.toDouble();
    if (expr->op == "+")
      return Value(l + r);
    if (expr->op == "-")
      return Value(l - r);
    if (expr->op == "*")
      return Value(l * r);
    if (expr->op == "/") {
      if (r == 0.0)
        throw std::runtime_error("Division by zero");
      return Value(l / r);
    }
    if (expr->op == "%")
      return Value(std::fmod(l, r));
    if (expr->op == "<")
      return Value(static_cast<int>(l < r));
    if (expr->op == ">")
      return Value(static_cast<int>(l > r));
    if (expr->op == "<=")
      return Value(static_cast<int>(l <= r));
    if (expr->op == ">=")
      return Value(static_cast<int>(l >= r));
    if (expr->op == "==")
      return Value(static_cast<int>(l == r));
    if (expr->op == "!=")
      return Value(static_cast<int>(l != r));
  } else {
    int l = left.toInt();
    int r = right.toInt();
    if (expr->op == "+")
      return Value(l + r);
    if (expr->op == "-")
      return Value(l - r);
    if (expr->op == "*")
      return Value(l * r);
    if (expr->op == "/") {
      if (r == 0)
        throw std::runtime_error("Division by zero");
      return Value(l / r);
    }
    if (expr->op == "%")
      return Value(l % r);
    if (expr->op == "<")
      return Value(static_cast<int>(l < r));
    if (expr->op == ">")
      return Value(static_cast<int>(l > r));
    if (expr->op == "<=")
      return Value(static_cast<int>(l <= r));
    if (expr->op == ">=")
      return Value(static_cast<int>(l >= r));
    if (expr->op == "==")
      return Value(static_cast<int>(l == r));
    if (expr->op == "!=")
      return Value(static_cast<int>(l != r));
  }

  // ── Logical operators ────────────────────────────────────────────────
  if (expr->op == "&&")
    return Value(static_cast<int>(left.toBool() && right.toBool()));
  if (expr->op == "||")
    return Value(static_cast<int>(left.toBool() || right.toBool()));

  throw std::runtime_error("Unknown binary operator: " + expr->op);
}

// ============================================================================
// Unary expression
// ============================================================================

Value Interpreter::evalUnary(const UnaryExpr *expr) {
  Value operand = eval(expr->operand.get());
  if (expr->op == "-") {
    if (operand.type == ValueType::FLOAT)
      return Value(-operand.floatVal);
    return Value(-operand.toInt());
  }
  if (expr->op == "!") {
    return Value(static_cast<int>(!operand.toBool()));
  }
  throw std::runtime_error("Unknown unary operator: " + expr->op);
}

// ============================================================================
// Function call
// ============================================================================

Value Interpreter::evalCall(const CallExpr *expr) {
  auto it = functions.find(expr->callee);
  if (it == functions.end()) {
    throw std::runtime_error("Undefined function: " + expr->callee);
  }

  const FunctionDef *fn = it->second;

  // Evaluate arguments
  std::vector<Value> argVals;
  for (auto &arg : expr->args) {
    argVals.push_back(eval(arg.get()));
  }

  if (argVals.size() != fn->params.size()) {
    throw std::runtime_error("Function '" + expr->callee + "' expects " +
                             std::to_string(fn->params.size()) + " args, got " +
                             std::to_string(argVals.size()));
  }

  // Create new scope and bind parameters
  pushScope();
  for (size_t i = 0; i < fn->params.size(); i++) {
    declareVar(fn->params[i].second, argVals[i]);
  }

  // Execute function body
  // We catch a special ReturnValue exception for early returns (future
  // extension)
  execBlock(fn->body.get());

  popScope();
  return Value(); // void return by default
}

// ============================================================================
// Array access
// ============================================================================

Value Interpreter::evalArrayAccess(const ArrayAccessExpr *expr) {
  Value &arr = lookupVar(expr->name);
  int idx = eval(expr->index.get()).toInt();

  switch (arr.type) {
  case ValueType::INT_ARRAY:
    if (idx < 0 || static_cast<size_t>(idx) >= arr.intArr.size())
      throw std::runtime_error("Array index out of bounds");
    return Value(arr.intArr[idx]);
  case ValueType::FLOAT_ARRAY:
    if (idx < 0 || static_cast<size_t>(idx) >= arr.floatArr.size())
      throw std::runtime_error("Array index out of bounds");
    return Value(arr.floatArr[idx]);
  case ValueType::CHAR_ARRAY:
    if (idx < 0 || static_cast<size_t>(idx) >= arr.charArr.size())
      throw std::runtime_error("Array index out of bounds");
    return Value(arr.charArr[idx]);
  case ValueType::STRING:
    if (idx < 0 || static_cast<size_t>(idx) >= arr.stringVal.size())
      throw std::runtime_error("String index out of bounds");
    return Value(arr.stringVal[idx]);
  default:
    throw std::runtime_error("Cannot index non-array variable: " + expr->name);
  }
}
