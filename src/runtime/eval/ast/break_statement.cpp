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

#include <runtime/eval/ast/break_statement.h>
#include <runtime/eval/ast/expression.h>
#include <runtime/eval/runtime/variable_environment.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

BreakStatement::BreakStatement(STATEMENT_ARGS, ExpressionPtr level,
                               bool isBreak)
  : Statement(STATEMENT_PASS), m_level(level), m_isBreak(isBreak) {}

void BreakStatement::eval(VariableEnvironment &env) const {
  if (env.isGotoing()) return;
  ENTER_STMT;
  int64 level = m_level ? m_level->eval(env).toInt64() : 1;
  if (level > 0) {
    if (m_isBreak) {
      env.setBreak(level);
    } else {
      env.setBreak(-level);
    }
  }
}

void BreakStatement::dump(std::ostream &out) const {
  if (m_isBreak) {
    out << "break";
  } else {
    out << "continue";
  }
  if (m_level) {
    out << " ";
    m_level->dump(out);
  }
  out << ";\n";
}

///////////////////////////////////////////////////////////////////////////////
}
}

