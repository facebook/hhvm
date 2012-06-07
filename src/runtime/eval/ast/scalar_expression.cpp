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
#include <runtime/eval/ast/expression.h>
#include <runtime/eval/ast/scalar_expression.h>
#include <runtime/eval/ast/scalar_value_expression.h>
#include <runtime/eval/runtime/file_repository.h>
#include <runtime/eval/runtime/variable_environment.h>
#include <runtime/eval/runtime/eval_state.h>
#include <runtime/eval/ast/name.h>
#include <util/parser/hphp.tab.hpp>
#include <util/util.h>

namespace HPHP {
namespace Eval {

///////////////////////////////////////////////////////////////////////////////

static StaticString s_trait_marker("[trait]");
static StaticString s_get_class_marker("[get_class]");
static StaticString s_get_parent_class_marker("[get_parent_class]");

ScalarExpression::ScalarExpression(EXPRESSION_ARGS, int type,
                                   const string &value, int subtype /* = 0 */)
    : Expression(KindOfScalarExpression, EXPRESSION_PASS),
      m_value(StringData::GetStaticString(value)),
      m_type(type), m_subtype(subtype), m_binary(false) {
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
    m_num.dbl = m_value->toDouble();
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
    : Expression(KindOfScalarExpression, EXPRESSION_PASS),
  m_type(0), m_subtype(0), m_kind(SNull) {}

ScalarExpression::ScalarExpression(EXPRESSION_ARGS, bool b)
    : Expression(KindOfScalarExpression, EXPRESSION_PASS),
    m_type(0), m_subtype(0), m_kind(SBool) {
  m_num.num = b ? 1 : 0;
}

ScalarExpression::ScalarExpression(EXPRESSION_ARGS, const string &s)
    : Expression(KindOfScalarExpression, EXPRESSION_PASS),
      m_value(StringData::GetStaticString(s)),
      m_type(0), m_subtype(0), m_kind(SString) {}

ScalarExpression::ScalarExpression(EXPRESSION_ARGS, const char *s)
    : Expression(KindOfScalarExpression, EXPRESSION_PASS),
      m_value(StringData::GetStaticString(s)),
      m_type(0), m_subtype(0), m_kind(SString) {}

ScalarExpression::ScalarExpression(EXPRESSION_ARGS, CStrRef s)
    : Expression(KindOfScalarExpression, EXPRESSION_PASS),
      m_value(StringData::GetStaticString(s.get())),
      m_type(0), m_subtype(0), m_kind(SString) {}

Expression *ScalarExpression::optimize(VariableEnvironment &env) {
  Variant v;
  if (evalScalar(env, v)) {
    return new ScalarValueExpression(v, loc());
  }
  return NULL;
}

bool ScalarExpression::evalScalar(VariableEnvironment &env, Variant &r) const {
  if (m_kind == SString) {
    switch (m_subtype) {
    case T_FILE:
      return false;
    case T_CLASS_C:
      if (m_value->same(s_trait_marker.get()) ||
          m_value->same(s_get_class_marker.get()) ||
          m_value->same(s_get_parent_class_marker.get())) {
        return false;
      }
      break;
    default:
      break;
    }
  }
  r = getValue(env);
  return true;
}

Variant ScalarExpression::eval(VariableEnvironment &env) const {
  return getValue(env);
}

Variant ScalarExpression::getValue(VariableEnvironment &env) const {
  switch (m_kind) {
  case SNull:
    return null_variant;
  case SBool:
    return (bool)m_num.num;
  case SString:
    switch (m_subtype) {
    case T_FILE:
      if (RuntimeOption::SandboxCheckMd5) {
        return FileRepository::translateFileName(m_value);
      }
      break;
    case T_CLASS_C: {
      bool tm = m_value->same(s_trait_marker.get());
      bool gcm = m_value->same(s_get_class_marker.get());
      bool gpcm = m_value->same(s_get_parent_class_marker.get());
      if (tm || gcm || gpcm) {
        DECLARE_THREAD_INFO;
        FrameInjection *fi = info->m_top;
        // T_CLASS_C is turned into "" by parser in non-class context
        ASSERT(fi->isEvalFrame());
        EvalFrameInjection* efi = static_cast<Eval::EvalFrameInjection*>(fi);
        String clsName = efi->getClass();
        if (!gpcm) return clsName;
        const ClassStatement *cls = RequestEvalState::findClass(clsName);
        return cls->parent();
      }
      break;
    }
    default:
      break;
    }
    return m_value;
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
        out << string(m_value->data(), m_value->size());
      } else {
        out << Util::escapeStringForPHP(m_value->data(), m_value->size());
      }
      break;
    case SInt:
    case SDouble:
      out << string(m_value->data(), m_value->size());
      break;
    default:
      ASSERT(false);
  }
}

///////////////////////////////////////////////////////////////////////////////
}
}

