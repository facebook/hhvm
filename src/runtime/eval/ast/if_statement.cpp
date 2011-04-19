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

#include <runtime/eval/ast/if_statement.h>
#include <runtime/eval/ast/expression.h>
#include <runtime/eval/ast/lval_expression.h>
#include <runtime/eval/ast/assignment_op_expression.h>
#include <runtime/eval/ast/assignment_ref_expression.h>
#include <runtime/eval/runtime/variable_environment.h>

namespace HPHP {
namespace Eval {
using namespace std;
///////////////////////////////////////////////////////////////////////////////

IfBranch::IfBranch(CONSTRUCT_ARGS, ExpressionPtr cond, StatementPtr body)
  : Construct(CONSTRUCT_PASS), m_cond(cond), m_body(body) {}

bool IfBranch::proc(VariableEnvironment &env) const {
  Variant cond(m_cond->eval(env));
  if (cond) {
    if (m_body) m_body->eval(env);
    return true;
  }
  return false;
}

Variant IfBranch::evalCond(VariableEnvironment &env) const {
  return m_cond->eval(env);
}

void IfBranch::dump(std::ostream &out) const {
  out << "if (";
  m_cond->dump(out);
  out << ") {\n";
  if (m_body) m_body->dump(out);
  out << "}";
}

IfStatement::IfStatement(STATEMENT_ARGS,
                         const vector<IfBranchPtr> &branches,
                         StatementPtr els)
  : Statement(STATEMENT_PASS), m_branches(branches), m_else(els) {}

void IfStatement::eval(VariableEnvironment &env) const {
  if (env.isGotoing()) {
    for (vector<IfBranchPtr>::const_iterator it = m_branches.begin();
         it != m_branches.end(); ++it) {
      if ((*it)->body()) {
        EVAL_STMT((*it)->body(), env);
        if (!env.isGotoing()) {
          return;
        }
      }
    }
    if (m_else) EVAL_STMT(m_else, env);
  } else {
    ENTER_STMT;
    for (vector<IfBranchPtr>::const_iterator it = m_branches.begin();
         it != m_branches.end(); ++it) {
      if ((*it)->evalCond(env)) {
        if ((*it)->body()) {
          EVAL_STMT((*it)->body(), env);
        }
        return;
      }
    }
    if (m_else) EVAL_STMT(m_else, env);
  }
}

void IfStatement::dump(std::ostream &out) const {
  dumpVector(out, m_branches, "else");
  if (m_else) {
    out << "else {\n";
    m_else->dump(out);
    out << "}";
  }
  out << "\n";
}

///////////////////////////////////////////////////////////////////////////////
}
}

