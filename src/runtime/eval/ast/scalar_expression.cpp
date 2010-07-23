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

#include <runtime/eval/ast/scalar_expression.h>
#include <runtime/eval/parser/hphp.tab.hpp>

namespace HPHP {
namespace Eval {
using namespace std;
///////////////////////////////////////////////////////////////////////////////

ScalarExpression::ScalarExpression(EXPRESSION_ARGS, int type,
                                   const string &value)
  : Expression(EXPRESSION_PASS), m_value(value), m_binary(false) {
  switch (type) {
  case T_NUM_STRING: {
    const char *s = value.c_str();
    if (!((*s == '0' && value.size() == 1) || ('1' <= *s && *s <= '9'))) {
      // Offset must be treated as a string
      m_kind = SString;
      break;
    }
    m_kind = SInt;
    // fall through
  }
  case T_LNUMBER: {
    m_num.num = strtoll(value.c_str(), NULL, 0);
    m_kind = SInt;
    break;
  }
  case T_DNUMBER: {
    m_num.dbl = String(m_value).toDouble();
    m_kind = SDouble;
    break;
  }
  case T_STRING :
    m_kind = SString;
    m_binary = m_value.find('\0') != string::npos;
    break;
  default:
    ASSERT(false);
  }
}

ScalarExpression::ScalarExpression(EXPRESSION_ARGS)
  : Expression(EXPRESSION_PASS), m_kind(SNull) {}
ScalarExpression::ScalarExpression(EXPRESSION_ARGS, bool b)
  : Expression(EXPRESSION_PASS), m_kind(SBool) {
  m_num.num = b ? 1 : 0;
}
ScalarExpression::ScalarExpression(EXPRESSION_ARGS, const string &s)
  : Expression(EXPRESSION_PASS), m_value(s), m_kind(SString) {}

Variant ScalarExpression::eval(VariableEnvironment &env) const {
  return getValue();
}

Variant ScalarExpression::getValue() const {
  switch (m_kind) {
  case SNull:
    return null_variant;
  case SBool:
    return (bool)m_num.num;
  case SString:
    if (!m_binary) {
      return m_value.c_str();
    } else {
      return String(m_value.c_str(), m_value.size(), AttachLiteral);
    }
  case SInt:
    return m_num.num;
  case SDouble:
    return m_num.dbl;
  default:
    ASSERT(false);
  }
  return Variant();
}

void ScalarExpression::dump() const {
  switch (m_kind) {
  case SNull:
    printf("NULL");
    break;
  case SBool:
    printf("%s", m_num.num ? "true" : "false");
    break;
  case SString:
    printf("\"%s\"", m_value.c_str());
    break;
  case SInt:
    printf("%lld", m_num.num);
    break;
  case SDouble:
    printf("%f", m_num.dbl);
    break;
  default:
    ASSERT(false);
  }
}

///////////////////////////////////////////////////////////////////////////////
}
}

