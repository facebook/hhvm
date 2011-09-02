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
#include <runtime/eval/ast/global_statement.h>
#include <runtime/eval/ast/name.h>
#include <runtime/eval/runtime/variable_environment.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

GlobalStatement::GlobalStatement(STATEMENT_ARGS,
                                 const std::vector<NamePtr> &vars)
  : Statement(STATEMENT_PASS), m_vars(vars) {}
GlobalStatement::GlobalStatement(STATEMENT_ARGS) : Statement(STATEMENT_PASS) {}

SimpleGlobalStatement::SimpleGlobalStatement(STATEMENT_ARGS,
  const std::vector<LvalExpressionPtr> &vars)
  : Statement(STATEMENT_PASS), m_vars(vars) {}

void GlobalStatement::eval(VariableEnvironment &env) const {
  if (env.isGotoing()) return;
  ENTER_STMT;
  for (std::vector<NamePtr>::const_iterator it = m_vars.begin();
       it != m_vars.end(); ++it) {
    env.flagGlobal((*it)->get(env));
  }
}

void SimpleGlobalStatement::eval(VariableEnvironment &env) const {
  if (env.isGotoing()) return;
  ENTER_STMT;
  for (std::vector<LvalExpressionPtr>::const_iterator it = m_vars.begin();
       it != m_vars.end(); ++it) {
    ASSERT(dynamic_cast<VariableExpression *>((*it).get()));
    VariableExpression *var = static_cast<VariableExpression *>((*it).get());
    Name *name = var->getName();
    ASSERT(dynamic_cast<StringName *>(name));
    StringName *sname = static_cast<StringName *>(name);
    StringData *sd = sname->getName();
    env.flagGlobal(sd, var->getIdx());
  }
}

void GlobalStatement::dump(std::ostream &out) const {
  out << "global ";

  for (uint i = 0; i < m_vars.size(); i++) {
    if (i > 0) out << ", ";
    if (m_vars[i]->get().isNull()) {
      out << "${";
      m_vars[i]->dump(out);
      out << "}";
    } else {
      out << "$";
      m_vars[i]->dump(out);
    }
  }

  out << ";\n";
}

void SimpleGlobalStatement::dump(std::ostream &out) const {
  out << "global ";

  for (uint i = 0; i < m_vars.size(); i++) {
    if (i > 0) out << ", ";
    ASSERT(dynamic_cast<VariableExpression *>(m_vars[i].get()));
    VariableExpression *var =
      static_cast<VariableExpression *>(m_vars[i].get());
    Name *name = var->getName();
    if (name->get().isNull()) {
      out << "${";
      name->dump(out);
      out << "}";
    } else {
      out << "$";
      name->dump(out);
    }
  }

  out << ";\n";
}
///////////////////////////////////////////////////////////////////////////////
}
}

