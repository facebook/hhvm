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

#include <runtime/eval/ast/use_trait_statement.h>
#include <runtime/eval/runtime/variable_environment.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

UseTraitStatement::UseTraitStatement(
  STATEMENT_ARGS, const std::vector<NamePtr> &names,
  StatementListStatementPtr stmts)
  : Statement(STATEMENT_PASS), m_names(names), m_stmts(stmts) {}

void UseTraitStatement::eval(VariableEnvironment &env) const {
  ENTER_STMT;
  ASSERT(false);
}

void UseTraitStatement::dump(std::ostream &out) const {
  out << "use \n";
  dumpVector(out, m_names, ",");
  out << "\n";
}

///////////////////////////////////////////////////////////////////////////////
}
}

