/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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
#include <compiler/parser/hphp.tab.hpp>
#include <util/util.h>
#include <compiler/analysis/dependency_graph.h>
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
#include <runtime/base/builtin_functions.h>
#include <runtime/base/zend/zend_printf.h>
#include <runtime/base/zend/zend_printf.h>
#include <compiler/hphp_unique.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// statics

void (*ScalarExpression::m_hookHandler)
  (AnalysisResultPtr ar, ScalarExpressionPtr sc, HphpHookUniqueId id);

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

ScalarExpression::ScalarExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 int type, const std::string &value, bool quoted /* = false */)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES),
    m_type(type), m_value(value), m_quoted(quoted) {
}

ScalarExpression::ScalarExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 CVarRef value, bool quoted /* = true */)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES),
    m_quoted(quoted), m_variant(value) {
  switch (value.getType()) {
  case KindOfString:
  case LiteralString:
    m_type = T_STRING;
    break;
  case KindOfByte:
  case KindOfInt16:
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
  m_value = string(value.toString()->data(), value.toString()->size());
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
// parser functions

void ScalarExpression::onParse(AnalysisResultPtr ar) {
  if (m_hookHandler) {
    ScalarExpressionPtr self =
      dynamic_pointer_cast<ScalarExpression>(shared_from_this());
    m_hookHandler(ar, self, onScalarExpressionParse);
  }
}

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void ScalarExpression::analyzeProgram(AnalysisResultPtr ar) {
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

ExpressionPtr ScalarExpression::preOptimize(AnalysisResultPtr ar) {
  string id = Util::toLower(getIdentifier());
  addUserFunction(ar, id, false);
  addUserClass(ar, id, false);

  if (ar->isFirstPass()) {
    switch (m_type) {
    case T_LINE:
      if (getLocation()) {
        m_translated = lexical_cast<string>(getLocation()->line1);
      } else {
        m_translated = "0";
      }
      break;
    case T_CLASS_C:
      if (ar->getClassScope()) {
        m_translated = ar->getClassScope()->getOriginalName();
      }
      break;
    case T_METHOD_C:
      if (ar->getFunctionScope()) {
        m_translated.clear();
        FunctionScopePtr func = ar->getFunctionScope();
        ClassScopePtr cls = ar->getClassScope();
        if (cls) {
          m_translated = cls->getOriginalName();
          m_translated += "::";
        }
        m_translated += func->getOriginalName();
      }
      break;
    case T_FUNC_C:
      if (ar->getFunctionScope()) {
        m_translated = ar->getFunctionScope()->getOriginalName();
      }
      break;
    default:
      break;
    }
  }
  return ExpressionPtr();
}

ExpressionPtr ScalarExpression::postOptimize(AnalysisResultPtr ar) {
  return ExpressionPtr();
}

TypePtr ScalarExpression::inferTypes(AnalysisResultPtr ar, TypePtr type,
                                     bool coerce) {
  ASSERT(false);
  return TypePtr();
}

TypePtr ScalarExpression::inferAndCheck(AnalysisResultPtr ar, TypePtr type,
                                        bool coerce) {
  if (ar->getPhase() == AnalysisResult::FirstInference &&
      isLiteralString() && m_value.find(' ') == string::npos) {
    setDynamicByIdentifier(ar, m_value);
  }

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
  case T_METHOD_C:
  case T_FUNC_C:
    actualType = Type::String;
    break;

  default:
    ASSERT(false);
    break;
  }

  if (Option::PrecomputeLiteralStrings &&
      Type::SameType(actualType, Type::String) &&
      m_quoted) {
    ScalarExpressionPtr self =
      dynamic_pointer_cast<ScalarExpression>(shared_from_this());
    ar->addLiteralString(getLiteralString(), self);
  }

  TypePtr ret = checkTypesImpl(ar, type, actualType, coerce);
  if (coerce && m_expectedType &&
      !Type::IsLegalCast(ar, actualType, m_expectedType)) {
    ar->getCodeError()->record(shared_from_this(), m_expectedType->getKindOf(),
                               actualType->getKindOf());
  }
  return ret;
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
  if(!isLiteralString()) {
    return output;
  }

  if (m_type == T_CLASS_C || m_type == T_METHOD_C || m_type == T_FUNC_C) {
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
    if (isIdentifier(m_value)) {
      return m_value;
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
      string output;
      output.reserve((m_value.length() << 1) + 2);
      output = "'";
      for (unsigned int i = 0; i < m_value.length(); i++) {
        unsigned char ch = m_value[i];
        switch (ch) {
        case '\n': output += "'.\"\\n\".'";  break;
        case '\r': output += "'.\"\\r\".'";  break;
        case '\t': output += "'.\"\\t\".'";  break;
        case '\'': output += "'.\"'\".'";    break;
        case '\\': output += "'.\"\\\\\".'"; break;
        default:
          output += ch;
          break;
        }
      }
      output += "'";
      Util::replaceAll(output, ".''.", ".");
      Util::replaceAll(output, "''.", "");
      Util::replaceAll(output, ".''", "");
      Util::replaceAll(output, "\".\"", "");
      cg.printf("%s", output.c_str());
    } else {
      cg.printf("%s", m_value.c_str());
    }
    break;
  case T_NUM_STRING:
  case T_LNUMBER:
  case T_DNUMBER:
    cg.printf("%s", m_value.c_str());
    break;
  case T_LINE:
  case T_CLASS_C:
  case T_METHOD_C:
  case T_FUNC_C:
    cg.printf("%s",
              (cg.translatePredefined() ? m_translated : m_value).c_str());
    break;
  default:
    ASSERT(false);
  }
}

void ScalarExpression::outputCPPString(CodeGenerator &cg,
                                       AnalysisResultPtr ar) {
  switch (m_type) {
  case T_CONSTANT_ENCAPSED_STRING:
  case T_ENCAPSED_AND_WHITESPACE:
    ASSERT(m_quoted); // fall through
  case T_STRING: {
    bool hasEmbeddedNull = false;
    if (m_quoted) {
      string output;
      output.reserve((m_value.length() << 1) + 2);
      output = "\"";
      for (unsigned int i = 0; i < m_value.length(); i++) {
        unsigned char ch = m_value[i];
        switch (ch) {
        case '\n': output += "\\n";  break;
        case '\r': output += "\\r";  break;
        case '\t': output += "\\t";  break;
        case '\a': output += "\\a";  break;
        case '\b': output += "\\b";  break;
        case '\f': output += "\\f";  break;
        case '\v': output += "\\v";  break;
        case '\0': output += "\\0";  hasEmbeddedNull = true; break;
        case '\"': output += "\\\""; break;
        case '\\': output += "\\\\"; break;
        case '?':  output += "\\?";  break; // avoiding trigraph errors
        default:
          if (isprint(ch)) {
            output += ch;
          } else {
            // output in octal notation
            char buf[10];
            snprintf(buf, sizeof(buf), "\\%03o", ch);
            output += buf;
          }
          break;
        }
      }
      output += "\"";
      if (hasEmbeddedNull) {
        char length[20];
        snprintf(length, sizeof(length), "%ld", m_value.length());
        bool constant =
          (cg.getContext() == CodeGenerator::CppConstantsDecl) ||
          (cg.getContext() == CodeGenerator::CppClassConstantsImpl);
        output = // e.g., "a\0b" => String("a\0b", 3, AttachLiteral)
          string(constant ? "Static" : "") +
          string("String(") + output + string(", ") +
          string(length) +
          string(constant ? ")" : ", AttachLiteral)");
      }
      cg.printf("%s", output.c_str());
    } else {
      cg.printf("%s", m_value.c_str());
    }
    break;
  }
  case T_CLASS_C:
  case T_METHOD_C:
  case T_FUNC_C:
    cg.printf("\"%s\"", m_translated.c_str());
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
  case T_METHOD_C:
  case T_FUNC_C:
    if (Option::PrecomputeLiteralStrings && cg.getOutput() !=
        CodeGenerator::SystemCPP && m_quoted &&
        cg.getContext() != CodeGenerator::CppConstantsDecl &&
        cg.getContext() != CodeGenerator::CppClassConstantsImpl) {
      int stringId = ar->getLiteralStringId(getLiteralString());
      ASSERT(stringId >= 0);
      cg.printf("literalStrings[%d]", stringId);
    } else {
      outputCPPString(cg, ar);
    }
    break;
  case T_LINE:
    cg.printf("%s", m_translated.c_str());
    break;
  case T_NUM_STRING: {
    const char *s = m_value.c_str();

    if ((*s == '0' && m_value.size() == 1) || ('1' <= *s && *s <= '9')) {
      // Offset could be treated as a long
      cg.printf("%sLL", m_value.c_str());
    } else {
      // Offset must be treated as a string
      cg.printf("\"%s\"", m_value.c_str());
    }
    break;
  }
  case T_LNUMBER: {
    Variant v = getVariant();
    ASSERT(v.isInteger());
    if (v.toInt64() == LONG_MIN) {
      cg.printf("0x%llxLL", LONG_MIN);
    } else {
      cg.printf("%lldLL", v.toInt64());
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
      cg.printf("%s", buf);
      if (round(dval) == dval && !strchr(buf, '.') && !strchr(buf, 'E')) {
        cg.printf(".0"); // for integer value
      }
      free(buf);
    } else if (isnan(dval)) {
      cg.printf("Limits::nan_double");
    } else if (dval > 0) {
      cg.printf("Limits::inf_double");
    } else {
      cg.printf("-Limits::inf_double");
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

Variant &ScalarExpression::getVariant() {
  if (!m_variant.isNull()) return m_variant;
  switch (m_type) {
  case T_ENCAPSED_AND_WHITESPACE:
  case T_CONSTANT_ENCAPSED_STRING:
  case T_STRING:
  case T_NUM_STRING:
    m_variant = String(m_value);
    break;
  case T_LNUMBER:
    m_variant = strtoll(m_value.c_str(), NULL, 0);
    break;
  case T_LINE:
    m_variant = String(m_translated).toInt64();
    break;
  case T_CLASS_C:
  case T_METHOD_C:
  case T_FUNC_C:
    m_variant = String(m_translated);
    break;
  case T_DNUMBER:
    m_variant = String(m_value).toDouble();
    break;
  default:
    ASSERT(false);
  }
  return m_variant;
}
