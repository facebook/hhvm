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

#include <runtime/eval/ast/scalar_expression.h>
#include <util/parser/hphp.tab.hpp>
#include <util/util.h>

namespace HPHP {
namespace Eval {
using namespace std;
///////////////////////////////////////////////////////////////////////////////

ScalarExpression::ScalarExpression(EXPRESSION_ARGS, int type,
                                   const string &value, int subtype /* = 0 */)
    : Expression(EXPRESSION_PASS),
      m_value(value), m_type(type), m_subtype(subtype), m_binary(false) {
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
    m_binary = String(m_value).find('\0') != String::npos;
    break;
  default:
    ASSERT(false);
  }
}

ScalarExpression::ScalarExpression(EXPRESSION_ARGS)
    : Expression(EXPRESSION_PASS), m_type(0), m_subtype(0), m_kind(SNull) {}

ScalarExpression::ScalarExpression(EXPRESSION_ARGS, bool b)
    : Expression(EXPRESSION_PASS), m_type(0), m_subtype(0), m_kind(SBool) {
  m_num.num = b ? 1 : 0;
}

ScalarExpression::ScalarExpression(EXPRESSION_ARGS, const string &s)
    : Expression(EXPRESSION_PASS),
      m_value(s), m_type(0), m_subtype(0), m_kind(SString) {}

ScalarExpression::ScalarExpression(EXPRESSION_ARGS, const char *s)
    : Expression(EXPRESSION_PASS),
      m_value(s, CopyString), m_type(0), m_subtype(0), m_kind(SString) {}

ScalarExpression::ScalarExpression(EXPRESSION_ARGS, CStrRef s)
    : Expression(EXPRESSION_PASS),
      m_value(s.get()), m_type(0), m_subtype(0), m_kind(SString) {}

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
    return String(m_value);
  case SInt:
    return m_num.num;
  case SDouble:
    return m_num.dbl;
  default:
    ASSERT(false);
  }
  return Variant();
}

void ScalarExpression::dump(std::ostream &out) const {
  switch (m_subtype) {
    case T_CLASS_C:  out << "__CLASS__";    return;
    case T_METHOD_C: out << "__METHOD__";   return;
    case T_FUNC_C:   out << "__FUNCTION__"; return;
    case T_FILE:     out << "__FILE__";     return;
    case T_DIR:      out << "__DIR__";      return;
    case T_LINE:     out << "__LINE__";     return;
  }

  switch (m_kind) {
    case SNull:
      out << "null";
      break;
    case SBool:
      out << (m_num.num ? "true" : "false");
      break;
    case SString:
      if (m_type == T_NUM_STRING) {
        out << string(m_value.c_str(), m_value.size());
      } else {
        out << Util::escapeStringForPHP(m_value.c_str(), m_value.size());
      }
      break;
    case SInt:
    case SDouble:
      out << string(m_value.c_str(), m_value.size());
      break;
    default:
      ASSERT(false);
  }
}

///////////////////////////////////////////////////////////////////////////////
}
}

