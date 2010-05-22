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

void GlobalStatement::eval(VariableEnvironment &env) const {
  ENTER_STMT;
  for (std::vector<NamePtr>::const_iterator it = m_vars.begin();
       it != m_vars.end(); ++it) {
    String n = (*it)->get(env);
    env.flagGlobal(n.data(), (*it)->hash());
  }
}

void GlobalStatement::dump() const {
  printf("global ");
  dumpVector(m_vars, ", ");
  printf(";");
}

///////////////////////////////////////////////////////////////////////////////
}
}

