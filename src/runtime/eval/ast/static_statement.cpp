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

#include <runtime/eval/ast/static_statement.h>
#include <runtime/eval/ast/expression.h>
#include <runtime/eval/runtime/variable_environment.h>
#include <runtime/eval/ast/name.h>

namespace HPHP {
namespace Eval {
using namespace std;
///////////////////////////////////////////////////////////////////////////////

StaticVariable::StaticVariable(CONSTRUCT_ARGS, const string &name,
                               ExpressionPtr val)
  : Construct(CONSTRUCT_PASS), m_name(Name::fromString(CONSTRUCT_PASS, name)),
    m_val(val) {}

void StaticVariable::set(VariableEnvironment &env) const {
  env.flagStatic(m_name->get(env), m_name->hash());
}

void StaticVariable::dump(std::ostream &out) const {
  out << "$";
  m_name->dump(out);
  if (m_val) {
    out << " = ";
    m_val->dump(out);
  }
}

const NamePtr &StaticVariable::name() {
  return m_name;
}

StaticStatement::StaticStatement(STATEMENT_ARGS,
                                 const std::vector<StaticVariablePtr> &vars)
  : Statement(STATEMENT_PASS), m_vars(vars) {}


void StaticStatement::eval(VariableEnvironment &env) const {
  if (env.isGotoing()) return;
  ENTER_STMT;
  for (std::vector<StaticVariablePtr>::const_iterator it = m_vars.begin();
       it != m_vars.end(); ++it) {
    (*it)->set(env);
  }
}

void StaticStatement::dump(std::ostream &out) const {
  out << "static ";
  dumpVector(out, m_vars);
  out << ";\n";
}

///////////////////////////////////////////////////////////////////////////////
}
}

