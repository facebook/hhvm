/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "hphp/compiler/expression/scalar_expression.h"
#include "hphp/parser/hphp.tab.hpp"
#include "hphp/util/util.h"
#include "hphp/compiler/analysis/code_error.h"
#include "hphp/compiler/analysis/block_scope.h"
#include "hphp/compiler/analysis/variable_table.h"
#include "hphp/compiler/analysis/constant_table.h"
#include "hphp/compiler/statement/statement_list.h"
#include "hphp/compiler/analysis/function_scope.h"
#include "hphp/compiler/analysis/class_scope.h"
#include "hphp/compiler/parser/parser.h"
#include "hphp/util/hash.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/type-conversions.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/ext/ext_variable.h"
#include "hphp/compiler/analysis/file_scope.h"

#include <sstream>
#include <cmath>
#include <limits.h>

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

ScalarExpression::ScalarExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 int type, const std::string &value, bool quoted /* = false */)
    : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES(ScalarExpression)),
      m_type(type), m_value(value), m_originalValue(value), m_quoted(quoted) {
}

ScalarExpression::ScalarExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 int type, const std::string &value, const std::string &translated,
 bool quoted /* false */)
    : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES(ScalarExpression)),
      m_type(type), m_value(value), m_originalValue(value),
      m_translated(translated) {
}

ScalarExpression::ScalarExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 CVarRef value, bool quoted /* = true */)
    : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES(ScalarExpression)),
      m_quoted(quoted) {
  if (!value.isNull()) {
    String serialized = f_serialize(value);
    m_serializedValue = string(serialized.data(), serialized.size());
    if (value.isDouble()) {
      m_dval = value.toDouble();
    }
  }
  switch (value.getType()) {
  case KindOfStaticString:
  case KindOfString:
    m_type = T_STRING;
    break;
  case KindOfInt64:
    m_type = T_LNUMBER;
    break;
  case KindOfDouble:
    m_type = T_DNUMBER;
    break;
  default:
    assert(false);
  }
  const String& s = value.toString();
  m_value = string(s->data(), s->size());
  if (m_type == T_DNUMBER && m_value.find_first_of(".eE", 0) == string::npos) {
    m_value += ".";
  }
  m_originalValue = m_value;
}

ExpressionPtr ScalarExpression::clone() {
  ScalarExpressionPtr exp(new ScalarExpression(*this));
  Expression::deepCopy(exp);
  return exp;
}

void ScalarExpression::appendEncapString(const std::string &value) {
  m_value += value;
}

void ScalarExpression::toLower(bool funcCall /* = false */) {
  assert(funcCall || !m_quoted);
  m_value = Util::toLower(m_value);
}

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

bool ScalarExpression::needsTranslation() const {
  switch (m_type) {
    case T_LINE:
    case T_NS_C:
    case T_CLASS_C:
    case T_METHOD_C:
    case T_FUNC_C:
      return true;
    default:
      return false;
  }
}

void ScalarExpression::analyzeProgram(AnalysisResultPtr ar) {
  if (ar->getPhase() == AnalysisResult::AnalyzeAll) {
    string id = Util::toLower(getIdentifier());

    switch (m_type) {
      case T_LINE:
        if (getLocation()) {
          m_translated = lexical_cast<string>(getLocation()->line1);
        } else {
          m_translated = "0";
        }
        break;
      case T_NS_C:
        m_translated = m_value;
        break;
     //  case T_TRAIT_C: Note: T_TRAIT_C is translated at parse time
      case T_CLASS_C:
      case T_METHOD_C: {
        if (!m_translated.empty()) break;

        BlockScopeRawPtr b = getScope();
        while (b && b->is(BlockScope::FunctionScope)) {
          b = b->getOuterScope();
        }
        m_translated.clear();
        if (b && b->is(BlockScope::ClassScope)) {
          ClassScopePtr clsScope = dynamic_pointer_cast<ClassScope>(b);
          if (!clsScope->isTrait()) {
            m_translated = clsScope->getOriginalName();
          }
        }
        if (m_type == T_METHOD_C) {
          if (FunctionScopePtr func = getFunctionScope()) {
            if (b && b->is(BlockScope::ClassScope)) {
              m_translated += "::";
            }
            if (func->isClosure()) {
              m_translated += "{closure}";
            } else {
              m_translated += func->getOriginalName();
            }
          }
        }
        break;
      }
      case T_FUNC_C:
        if (FunctionScopePtr func = getFunctionScope()) {
          if (func->isClosure()) {
            m_translated = "{closure}";
          } else {
            m_translated = func->getOriginalName();
          }
        }
        break;
      default:
        break;
    }
  }
}

unsigned ScalarExpression::getCanonHash() const {
  int64_t val = getHash();
  if (val == -1) {
    val = hash_string(m_value.c_str(), m_value.size());
  }
  return unsigned(val) ^ unsigned(val >> 32);
}

bool ScalarExpression::canonCompare(ExpressionPtr e) const {
  if (!Expression::canonCompare(e)) return false;
  ScalarExpressionPtr s =
    static_pointer_cast<ScalarExpression>(e);

  return
    m_value == s->m_value &&
    m_type == s->m_type &&
    m_quoted == s->m_quoted;
}

ExpressionPtr ScalarExpression::postOptimize(AnalysisResultConstPtr ar) {
  if (!m_expectedType || Type::SameType(m_actualType, m_expectedType)) {
    return ExpressionPtr();
  }

  Variant orig = getVariant();
  Variant cast;
  bool match = false;

  switch (m_expectedType->getKindOf()) {
  case Type::KindOfBoolean: match = true; cast = orig.toBoolean(); break;
  case Type::KindOfInt64:   match = true; cast = orig.toInt64();   break;
  case Type::KindOfDouble:  match = true; cast = orig.toDouble();  break;
  case Type::KindOfString:  match = true; cast = orig.toString();  break;
  }

  if (!match || same(orig, cast)) {
    // no changes need to be made
    return ExpressionPtr();
  }

  ExpressionPtr p = makeScalarExpression(ar, cast);
  p->setActualType(m_expectedType);
  return p;
}

TypePtr ScalarExpression::inferTypes(AnalysisResultPtr ar, TypePtr type,
                                     bool coerce) {
  assert(false);
  return TypePtr();
}

TypePtr ScalarExpression::inferenceImpl(AnalysisResultConstPtr ar,
                                        TypePtr type, bool coerce) {
  TypePtr actualType;
  switch (m_type) {
  case T_STRING:
    actualType = Type::String;
    break;
  case T_NUM_STRING:
  case T_LNUMBER:
    actualType = Type::Int64;
    break;
  case T_DNUMBER:
    actualType = Type::Double;
    break;

  case T_LINE:
  case T_COMPILER_HALT_OFFSET:
    actualType = Type::Int64;
    break;

  case T_CONSTANT_ENCAPSED_STRING:
  case T_ENCAPSED_AND_WHITESPACE:
  case T_TRAIT_C:
  case T_CLASS_C:
  case T_NS_C:
  case T_METHOD_C:
  case T_FUNC_C:
    actualType = Type::String;
    break;

  default:
    assert(false);
    break;
  }

  return checkTypesImpl(ar, type, actualType, coerce);
}

TypePtr ScalarExpression::inferAndCheck(AnalysisResultPtr ar, TypePtr type,
                                        bool coerce) {
  IMPLEMENT_INFER_AND_CHECK_ASSERT(getScope());
  resetTypes();

  if (!Option::AllDynamic &&
      ar->getPhase() == AnalysisResult::FirstInference &&
      getScope()->isFirstPass() &&
      isLiteralString() && m_value.find(' ') == string::npos) {
    setDynamicByIdentifier(ar, m_value);
  }

  return inferenceImpl(ar, type, coerce);
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

bool ScalarExpression::isLiteralInteger() const {
  switch (m_type) {
  case T_NUM_STRING:
    {
      char ch = m_value[0];
      if ((ch == '0' && m_value.size() == 1) || ('1' <= ch && ch <= '9')) {
        // Offset could be treated as a long
        return true;
      }
    }
    break;
  case T_LNUMBER:
    return true;
  default:
    break;
  }
  return false;
}

int64_t ScalarExpression::getLiteralInteger() const {
  assert(isLiteralInteger());
  return strtoll(m_value.c_str(), nullptr, 0);
}

bool ScalarExpression::isLiteralString() const {
  switch (m_type) {
  case T_STRING:
    return m_quoted;
  case T_CONSTANT_ENCAPSED_STRING:
  case T_ENCAPSED_AND_WHITESPACE:
    assert(m_quoted); // fall through
  case T_TRAIT_C:
  case T_CLASS_C:
  case T_NS_C:
  case T_METHOD_C:
  case T_FUNC_C:
    return true;
  case T_NUM_STRING:
    {
      char ch = m_value[0];
      if (!((ch == '0' && m_value.size() == 1) || ('1' <= ch && ch <= '9'))) {
        // Offset must be treated as a string
        return true;
      }
    }
    break;
  default:
    break;
  }
  return false;
}

std::string ScalarExpression::getLiteralString() const {
  return getLiteralStringImpl(false);
}

std::string ScalarExpression::getOriginalLiteralString() const {
  return getLiteralStringImpl(true);
}

std::string ScalarExpression::getLiteralStringImpl(bool original) const {
  string output;
  if (!isLiteralString() && m_type != T_STRING) {
    return output;
  }

  if (m_type == T_CLASS_C || m_type == T_NS_C || m_type == T_METHOD_C ||
      m_type == T_FUNC_C || m_type == T_TRAIT_C) {
    return m_translated;
  }

  switch (m_type) {
  case T_NUM_STRING:
    assert(isLiteralString());
  case T_STRING:
  case T_ENCAPSED_AND_WHITESPACE:
  case T_CONSTANT_ENCAPSED_STRING:
    return original ? m_originalValue : m_value;
  default:
    assert(false);
    break;
  }
  return "";
}

std::string ScalarExpression::getIdentifier() const {
  if (isLiteralString()) {
    std::string id = getLiteralString();
    if (IsIdentifier(id)) {
      return id;
    }
  }
  return "";
}

///////////////////////////////////////////////////////////////////////////////

void ScalarExpression::outputCodeModel(CodeGenerator &cg) {
  switch (m_type) {
    case T_NS_C:
    case T_LINE:
    case T_TRAIT_C:
    case T_CLASS_C:
    case T_METHOD_C:
    case T_FUNC_C: {
      cg.printObjectHeader("SimpleVariableExpression", 2);
      std::string varName;
      switch (m_type) {
        case T_NS_C: varName = "__NAMESPACE__"; break;
        case T_LINE: varName = "__LINE__"; break;
        case T_TRAIT_C: varName = "__TRAIT__"; break;
        case T_CLASS_C: varName = "__CLASS__"; break;
        case T_METHOD_C: varName = "__METHOD__"; break;
        case T_FUNC_C: varName = "__FUNCTION__"; break;
        default: break;
      }
      cg.printPropertyHeader("variableName");
      cg.printValue(varName);
      cg.printPropertyHeader("sourceLocation");
      cg.printLocation(this->getLocation());
      cg.printObjectFooter();
      return;
    }
    default:
      break;
  }

  cg.printObjectHeader("ScalarExpression", 2);
  cg.printPropertyHeader("value");
  cg.printValue(m_originalValue);
  cg.printPropertyHeader("sourceLocation");
  cg.printLocation(this->getLocation());
  cg.printObjectFooter();
}

///////////////////////////////////////////////////////////////////////////////

void ScalarExpression::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  switch (m_type) {
  case T_CONSTANT_ENCAPSED_STRING:
  case T_ENCAPSED_AND_WHITESPACE:
    assert(m_quoted); // fall through
  case T_STRING:
    if (m_quoted) {
      string output = Util::escapeStringForPHP(m_originalValue);
      cg_printf("%s", output.c_str());
    } else {
      cg_printf("%s", m_originalValue.c_str());
    }
    break;
  case T_NUM_STRING:
  case T_LNUMBER:
  case T_DNUMBER:
  case T_COMPILER_HALT_OFFSET:
    cg_printf("%s", m_originalValue.c_str());
    break;
  case T_NS_C:
    if (cg.translatePredefined()) {
      cg_printf("%s", m_translated.c_str());
    } else {
      cg_printf("__NAMESPACE__");
    }
    break;
  case T_LINE:
  case T_TRAIT_C:
  case T_CLASS_C:
  case T_METHOD_C:
  case T_FUNC_C:
    if (cg.translatePredefined()) {
      cg_printf("%s", m_translated.c_str());
    } else {
      cg_printf("%s", m_originalValue.c_str());
    }
    break;
  default:
    assert(false);
  }
}

int64_t ScalarExpression::getHash() const {
  int64_t hash = -1;
  if (isLiteralInteger()) {
    hash = hash_int64(getLiteralInteger());
  } else if (isLiteralString()) {
    string scs = getLiteralString();
    int64_t res;
    if (is_strictly_integer(scs.c_str(), scs.size(), res)) {
      hash = hash_int64(res);
    } else {
      hash = hash_string(scs.c_str(), scs.size());
    }
  }
  return hash;
}

Variant ScalarExpression::getVariant() const {
  if (!m_serializedValue.empty()) {
    Variant ret = unserialize_from_buffer(
      m_serializedValue.data(), m_serializedValue.size(), null_array);
    if (ret.isDouble()) {
      return m_dval;
    }
    return ret;
  }
  switch (m_type) {
    case T_ENCAPSED_AND_WHITESPACE:
    case T_CONSTANT_ENCAPSED_STRING:
    case T_STRING:
    case T_NUM_STRING:
      return String(m_value);
    case T_LNUMBER:
    case T_COMPILER_HALT_OFFSET:
      return strtoll(m_value.c_str(), nullptr, 0);
    case T_LINE:
      return String(m_translated).toInt64();
    case T_TRAIT_C:
    case T_CLASS_C:
    case T_NS_C:
    case T_METHOD_C:
    case T_FUNC_C:
      return String(m_translated);
    case T_DNUMBER:
      return String(m_value).toDouble();
    default:
      assert(false);
  }
  return uninit_null();
}

bool ScalarExpression::getString(const std::string *&s) const {
  switch (m_type) {
    case T_ENCAPSED_AND_WHITESPACE:
    case T_CONSTANT_ENCAPSED_STRING:
    case T_STRING:
    case T_NUM_STRING:
      s = &m_value;
      return true;
    case T_TRAIT_C:
    case T_CLASS_C:
    case T_NS_C:
    case T_METHOD_C:
    case T_FUNC_C:
      s = &m_translated;
      return true;
    default:
      return false;
  }
}

bool ScalarExpression::getInt(int64_t &i) const {
  if (m_type == T_LNUMBER || m_type == T_COMPILER_HALT_OFFSET) {
    i = strtoll(m_value.c_str(), nullptr, 0);
    return true;
  } else if (m_type == T_LINE) {
    i = getLocation() ? getLocation()->line1 : 0;
    return true;
  }
  return false;
}

bool ScalarExpression::getDouble(double &d) const {
  if (m_type == T_DNUMBER) {
    Variant v = getVariant();
    assert(v.isDouble());
    d = v.toDouble();
    return true;
  }
  return false;
}

void ScalarExpression::setCompilerHaltOffset(int64_t ofs) {
  assert(m_type == T_COMPILER_HALT_OFFSET);
  std::ostringstream ss;
  ss << ofs;
  m_value = ss.str();
  m_originalValue = ss.str();
}
