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

#include <runtime/eval/ast/switch_statement.h>
#include <runtime/eval/ast/expression.h>
#include <runtime/eval/runtime/variable_environment.h>

using namespace std;

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

CaseStatement::CaseStatement(STATEMENT_ARGS, ExpressionPtr match,
                             StatementPtr body)
  : Statement(STATEMENT_PASS), m_match(match), m_body(body) {}

bool CaseStatement::match(VariableEnvironment &env, CVarRef value) const {
  ASSERT(m_match);
  Variant match(m_match->eval(env));
  return equal(match, value);
}

void CaseStatement::eval(VariableEnvironment &env) const {
  if (env.isGotoing()) return;
  if (m_body) m_body->eval(env);
}

bool CaseStatement::isDefault() const {
  return !m_match;
}

void CaseStatement::dump(std::ostream &out) const {
  if (m_match) {
    out << "\ncase ";
    m_match->dump(out);
  } else {
    out << "\ndefault";
  }
  out << ":\n";
  m_body->dump(out);
}

SwitchStatement::SwitchStatement(STATEMENT_ARGS, ExpressionPtr source,
                const std::vector<CaseStatementPtr> &cases)
  : Statement(STATEMENT_PASS), m_source(source), m_cases(cases) {}

void SwitchStatement::eval(VariableEnvironment &env) const {
  if (env.isGotoing()) return;
  ENTER_STMT;
  Variant source(m_source->eval(env));
  bool matched = false;
  vector<CaseStatementPtr>::const_iterator defaultPos = m_cases.end();
  for (vector<CaseStatementPtr>::const_iterator iter = m_cases.begin();
       iter != m_cases.end(); ++iter) {
    if ((*iter)->isDefault()) {
      defaultPos = iter;
    } else if (!matched && (*iter)->match(env, source)) {
      matched = true;
    }
    if (matched) {
      EVAL_STMT_HANDLE_GOTO_BEGIN(restart);
      EVAL_STMT_HANDLE_BREAK_CONT(*iter, env);
      EVAL_STMT_HANDLE_GOTO_END(restart);
    }
  }
  if (!matched && defaultPos != m_cases.end()) {
    for (; defaultPos != m_cases.end(); ++defaultPos) {
      EVAL_STMT_HANDLE_BREAK_CONT(*defaultPos, env);
    }
  }
}

void SwitchStatement::dump(std::ostream &out) const {
  out << "switch (";
  m_source->dump(out);
  out << ") {";
  if (m_cases.empty()) {
    out << "\n";
  } else {
    dumpVector(out, m_cases, "");
  }
  out << "}\n";
}

///////////////////////////////////////////////////////////////////////////////
}
}

