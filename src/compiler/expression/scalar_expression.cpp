/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <sstream>
#include <limits.h>
#include <compiler/expression/scalar_expression.h>
#include <util/parser/hphp.tab.hpp>
#include <util/util.h>
#include <compiler/analysis/code_error.h>
#include <compiler/analysis/block_scope.h>
#include <compiler/analysis/variable_table.h>
#include <compiler/analysis/constant_table.h>
#include <compiler/statement/statement_list.h>
#include <compiler/analysis/function_scope.h>
#include <compiler/analysis/class_scope.h>
#include <compiler/parser/parser.h>
#include <util/hash.h>
#include <runtime/base/string_data.h>
#include <runtime/base/type_conversions.h>
#include <runtime/base/builtin_functions.h>
#include <runtime/base/zend/zend_printf.h>
#include <runtime/base/zend/zend_printf.h>
#include <runtime/ext/ext_variable.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

ScalarExpression::ScalarExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 int type, const std::string &value, bool quoted /* = false */)
    : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES),
      m_type(type), m_value(value), m_originalValue(value), m_quoted(quoted) {
}

ScalarExpression::ScalarExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 CVarRef value, bool quoted /* = true */)
    : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES),
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
  case KindOfInt32:
  case KindOfInt64:
    m_type = T_LNUMBER;
    break;
  case KindOfDouble:
    m_type = T_DNUMBER;
    break;
  default:
    ASSERT(false);
  }
  CStrRef s = value.toString();
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
  ASSERT(funcCall || !m_quoted);
  m_value = Util::toLower(m_value);
}

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

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
      case T_CLASS_C:
      case T_METHOD_C: {
        BlockScopeRawPtr b = getScope();
        while (b && b->is(BlockScope::FunctionScope)) {
          b = b->getOuterScope();
        }
        m_translated.clear();
        if (b && b->is(BlockScope::ClassScope)) {
          m_translated =
            dynamic_pointer_cast<ClassScope>(b)->getOriginalName();
        }
        if (m_type == T_METHOD_C) {
          if (FunctionScopePtr func = getFunctionScope()) {
            if (b && b->is(BlockScope::ClassScope)) {
              m_translated += "::";
            }
            m_translated += func->getOriginalName();
          }
        }
        break;
      }
      case T_FUNC_C:
        if (getFunctionScope()) {
          m_translated = getFunctionScope()->getOriginalName();
          if (m_translated[0] == '0') {
            m_translated = "{closure}";
          }
        }
        break;
      default:
        break;
    }
  }
}

unsigned ScalarExpression::getCanonHash() const {
  int64 val = getHash();
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
  if (!m_expectedType)
    return ExpressionPtr();

  Variant orig = getVariant();
  Variant cast;
  bool match = false;

  switch (m_expectedType->getKindOf()) {
  case Type::KindOfBoolean: match = true; cast = orig.toBoolean(); break;
  case Type::KindOfInt64:   match = true; cast = orig.toInt64();   break;
  case Type::KindOfDouble:  match = true; cast = orig.toDouble();  break;
  case Type::KindOfString:  match = true; cast = orig.toString();  break;
  }

  if (!match || same(orig, cast))
    // no changes need to be made
    return ExpressionPtr();
  
  ExpressionPtr p = makeScalarExpression(ar, cast);
  p->setActualType(m_expectedType);
  return p;
}

TypePtr ScalarExpression::inferTypes(AnalysisResultPtr ar, TypePtr type,
                                     bool coerce) {
  ASSERT(false);
  return TypePtr();
}

TypePtr ScalarExpression::inferenceImpl(AnalysisResultConstPtr ar, TypePtr type,
                                        bool coerce) {
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
    actualType = Type::Int32;
    break;

  case T_CONSTANT_ENCAPSED_STRING:
  case T_ENCAPSED_AND_WHITESPACE:
  case T_CLASS_C:
  case T_NS_C:
  case T_METHOD_C:
  case T_FUNC_C:
    actualType = Type::String;
    break;

  default:
    ASSERT(false);
    break;
  }

  return checkTypesImpl(ar, type, actualType, coerce);
}

TypePtr ScalarExpression::inferAndCheck(AnalysisResultPtr ar, TypePtr type,
                                        bool coerce) {
  if (ar->getPhase() == AnalysisResult::FirstInference &&
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

int64 ScalarExpression::getLiteralInteger() const {
  ASSERT(isLiteralInteger());
  return strtoll(m_value.c_str(), NULL, 0);
}

bool ScalarExpression::isLiteralString() const {
  switch (m_type) {
  case T_STRING:
    return m_quoted;
  case T_CONSTANT_ENCAPSED_STRING:
  case T_ENCAPSED_AND_WHITESPACE:
    ASSERT(m_quoted); // fall through
  case T_CLASS_C:
  case T_NS_C:
  case T_METHOD_C:
  case T_FUNC_C:
    return true;
  case T_NUM_STRING:
    {
      char ch = m_value[0];
      if (!(ch == '0' && m_value.size() == 1 || ('1' <= ch && ch <= '9'))) {
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
  string output;
  if (!isLiteralString() && m_type != T_STRING) {
    return output;
  }

  if (m_type == T_CLASS_C || m_type == T_NS_C || m_type == T_METHOD_C ||
      m_type == T_FUNC_C) {
    return m_translated;
  }

  switch (m_type) {
  case T_STRING:
  case T_ENCAPSED_AND_WHITESPACE:
  case T_CONSTANT_ENCAPSED_STRING:
    return m_value;
  case T_NUM_STRING:
    ASSERT(isLiteralString());
    return m_value;
  default:
    ASSERT(false);
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

void ScalarExpression::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  switch (m_type) {
  case T_CONSTANT_ENCAPSED_STRING:
  case T_ENCAPSED_AND_WHITESPACE:
    ASSERT(m_quoted); // fall through
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
    ASSERT(false);
  }
}

std::string ScalarExpression::getCPPLiteralString(CodeGenerator &cg,
                                                  bool *binary /* = NULL */) {
  string output;
  switch (m_type) {
  case T_CONSTANT_ENCAPSED_STRING:
  case T_ENCAPSED_AND_WHITESPACE:
  case T_STRING: {
    output = "\"";
    output += cg.escapeLabel(m_value, binary);
    output += "\"";
    break;
  }
  case T_CLASS_C:
  case T_NS_C:
  case T_METHOD_C:
  case T_FUNC_C:
    output = "\"";
    output += m_translated;
    output += "\"";
    break;
  default:
    ASSERT(false);
  }
  return output;
}

void ScalarExpression::outputCPPString(CodeGenerator &cg,
                                       AnalysisResultPtr ar) {
  switch (m_type) {
  case T_CONSTANT_ENCAPSED_STRING:
  case T_ENCAPSED_AND_WHITESPACE:
    ASSERT(m_quoted); // fall through
  case T_STRING: {
    if (m_quoted) {
      string output = getLiteralString();
      bool constant =
        (cg.getContext() == CodeGenerator::CppConstantsDecl) ||
        (cg.getContext() == CodeGenerator::CppClassConstantsImpl);
      cg_printString(output, ar, shared_from_this(), constant);
    } else {
      cg_printf("%s", m_value.c_str());
    }
    break;
  }
  case T_CLASS_C:
  case T_NS_C:
  case T_METHOD_C:
  case T_FUNC_C:
    cg_printString(m_translated, ar, shared_from_this());
    break;
  default:
    ASSERT(false);
  }
}

void ScalarExpression::outputCPPImpl(CodeGenerator &cg, AnalysisResultPtr ar) {
  switch (m_type) {
  case T_CONSTANT_ENCAPSED_STRING:
  case T_ENCAPSED_AND_WHITESPACE:
  case T_STRING:
  case T_CLASS_C:
  case T_NS_C:
  case T_METHOD_C:
  case T_FUNC_C: {
    outputCPPString(cg, ar);
    break;
  }
  case T_LINE:
    cg_printf("%s", m_translated.c_str());
    break;
  case T_NUM_STRING: {
    const char *s = m_value.c_str();

    if ((*s == '0' && m_value.size() == 1) || ('1' <= *s && *s <= '9')) {
      // Offset could be treated as a long
      cg_printf("%sLL", m_value.c_str());
    } else {
      // Offset must be treated as a string
      cg_printf("\"%s\"", m_value.c_str());
    }
    break;
  }
  case T_LNUMBER: {
    Variant v = getVariant();
    ASSERT(v.isInteger());
    if (v.toInt64() == LONG_MIN) {
      cg_printf("(int64)0x%llxLL", LONG_MIN);
    } else {
      cg_printf("%lldLL", v.toInt64());
    }
    break;
  }
  case T_DNUMBER: {
    double dval = getVariant().getDouble();
    if (finite(dval)) {
      char *buf = NULL;
      if (dval == 0.0) dval = 0.0; // so to avoid "-0" output
      // 17 to ensure lossless conversion
      vspprintf(&buf, 0, "%.*G", 17, dval);
      ASSERT(buf);
      cg_printf("%s", buf);
      if (round(dval) == dval && !strchr(buf, '.') && !strchr(buf, 'E')) {
        cg.printf(".0"); // for integer value, cg_printf would break 0.0 token
      }
      free(buf);
    } else if (isnan(dval)) {
      cg_printf("%sNAN", Option::ConstantPrefix);
    } else if (dval > 0) {
      cg_printf("%sINF", Option::ConstantPrefix);
    } else {
      cg_printf("-%sINF", Option::ConstantPrefix);
    }
    break;
  }
  default:
    ASSERT(false);
  }
}

int64 ScalarExpression::getHash() const {
  int64 hash = -1;
  if (isLiteralInteger()) {
    hash = hash_int64(getLiteralInteger());
  } else if (isLiteralString()) {
    string scs = getLiteralString();
    int64 res;
    if (is_strictly_integer(scs.c_str(), scs.size(), res)) {
      hash = hash_int64(res);
    } else {
      hash = hash_string(scs.c_str(), scs.size());
    }
  }
  return hash;
}

Variant ScalarExpression::getVariant() {
  if (!m_serializedValue.empty()) {
    Variant ret = f_unserialize
      (String(m_serializedValue.data(),
              m_serializedValue.size(), AttachLiteral));
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
      return strtoll(m_value.c_str(), NULL, 0);
    case T_LINE:
      return String(m_translated).toInt64();
    case T_CLASS_C:
    case T_NS_C:
    case T_METHOD_C:
    case T_FUNC_C:
      return String(m_translated);
    case T_DNUMBER:
      return String(m_value).toDouble();
    default:
      ASSERT(false);
  }
  return null;
}

bool ScalarExpression::getString(const std::string *&s) const {
  switch (m_type) {
    case T_ENCAPSED_AND_WHITESPACE:
    case T_CONSTANT_ENCAPSED_STRING:
    case T_STRING:
    case T_NUM_STRING:
      s = &m_value;
      return true;
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

bool ScalarExpression::getInt(int64 &i) const {
  if (m_type == T_LNUMBER) {
    i = strtoll(m_value.c_str(), NULL, 0);
    return true;
  } else if (m_type == T_LINE) {
    i = getLocation() ? getLocation()->line1 : 0;
    return true;
  }

  return false;
}

bool ScalarExpression::getDouble(double &d) const {
  if (m_type == T_DNUMBER) {
    d = String(m_value).toDouble();
    return true;
  }
  return false;
}
