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

#include <runtime/eval/ast/echo_statement.h>
#include <runtime/eval/ast/expression.h>
#include <runtime/eval/runtime/variable_environment.h>

using namespace std;

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

EchoStatement::EchoStatement(STATEMENT_ARGS,
                             const std::vector<ExpressionPtr> &args)
  : Statement(STATEMENT_PASS), m_args(args) {}

void EchoStatement::eval(VariableEnvironment &env) const {
  if (env.isGotoing()) return;
  ENTER_STMT;
  for (vector<ExpressionPtr>::const_iterator it = m_args.begin();
       it != m_args.end(); ++it) {
    echo((*it)->eval(env));
  }
}

void EchoStatement::dump(std::ostream &out) const {
  out << "echo ";
  dumpVector(out, m_args);
  out << ";\n";
}

///////////////////////////////////////////////////////////////////////////////
}
}

