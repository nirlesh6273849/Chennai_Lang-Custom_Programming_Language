#pragma once
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
//This file is a header file only
//It contains the Value class
// Used to convert string to int/float/char/string

enum class ValueType {
  INT,
  FLOAT,
  CHAR,
  STRING,
  INT_ARRAY,
  FLOAT_ARRAY,
  CHAR_ARRAY,
  VOID
};

struct Value {
  ValueType type;

  int intVal = 0;
  double floatVal = 0.0;
  char charVal = '\0';
  std::string stringVal;

  std::vector<int> intArr;
  std::vector<double> floatArr;
  std::vector<char> charArr;

  // ── Constructors ────────────────────────────────────────────────────────
  Value() : type(ValueType::VOID) {}

  explicit Value(int v) : type(ValueType::INT), intVal(v) {}
  explicit Value(double v) : type(ValueType::FLOAT), floatVal(v) {}
  explicit Value(char v) : type(ValueType::CHAR), charVal(v) {}
  explicit Value(const std::string &v)
      : type(ValueType::STRING), stringVal(v) {}

  static Value makeIntArray(const std::vector<int> &a) {
    Value v;
    v.type = ValueType::INT_ARRAY;
    v.intArr = a;
    return v;
  }
  static Value makeFloatArray(const std::vector<double> &a) {
    Value v;
    v.type = ValueType::FLOAT_ARRAY;
    v.floatArr = a;
    return v;
  }
  static Value makeCharArray(const std::vector<char> &a) {
    Value v;
    v.type = ValueType::CHAR_ARRAY;
    v.charArr = a;
    return v;
  }

  // ── Numeric coercion helpers ────────────────────────────────────────────
  double toDouble() const {
    if (type == ValueType::INT)
      return static_cast<double>(intVal);
    if (type == ValueType::FLOAT)
      return floatVal;
    if (type == ValueType::CHAR)
      return static_cast<double>(charVal);
    throw std::runtime_error("Cannot convert to number");
  }

  int toInt() const {
    if (type == ValueType::INT)
      return intVal;
    if (type == ValueType::FLOAT)
      return static_cast<int>(floatVal);
    if (type == ValueType::CHAR)
      return static_cast<int>(charVal);
    throw std::runtime_error("Cannot convert to int");
  }

  bool toBool() const {
    switch (type) {
    case ValueType::INT:
      return intVal != 0;
    case ValueType::FLOAT:
      return floatVal != 0.0;
    case ValueType::CHAR:
      return charVal != '\0';
    case ValueType::STRING:
      return !stringVal.empty();
    default:
      return false;
    }
  }

  // ── Display ─────────────────────────────────────────────────────────────
  std::string toString() const {
    switch (type) {
    case ValueType::INT:
      return std::to_string(intVal);
    case ValueType::FLOAT: {
      std::ostringstream oss;
      oss << floatVal;
      return oss.str();
    }
    case ValueType::CHAR:
      return std::string(1, charVal);
    case ValueType::STRING:
      return stringVal;
    case ValueType::INT_ARRAY: {
      std::string s = "[";
      for (size_t i = 0; i < intArr.size(); i++) {
        if (i > 0)
          s += ", ";
        s += std::to_string(intArr[i]);
      }
      return s + "]";
    }
    case ValueType::FLOAT_ARRAY: {
      std::string s = "[";
      for (size_t i = 0; i < floatArr.size(); i++) {
        if (i > 0)
          s += ", ";
        std::ostringstream oss;
        oss << floatArr[i];
        s += oss.str();
      }
      return s + "]";
    }
    case ValueType::CHAR_ARRAY: {
      return std::string(charArr.begin(), charArr.end());
    }
    case ValueType::VOID:
      return "void";
    }
    return "unknown";
  }
};
