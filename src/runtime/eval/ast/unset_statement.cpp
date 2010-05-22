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

#include <runtime/eval/ast/unset_statement.h>
#include <runtime/eval/runtime/variable_environment.h>
#include <runtime/eval/ast/lval_expression.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

UnsetStatement::UnsetStatement(STATEMENT_ARGS,
                               const std::vector<ExpressionPtr> &vals)
  : Statement(STATEMENT_PASS) {
  m_vals.reserve(vals.size());
  for (std::vector<ExpressionPtr>::const_iterator it = vals.begin();
         it != vals.end(); ++it) {
    LvalExpressionPtr lv = (*it)->cast<LvalExpression>();
    if (!lv) {
      throw FatalErrorException("Cannot unset function return value at %s:%d",
                                loc()->file, loc()->line1);
    }
    m_vals.push_back(lv);
  }
}

void UnsetStatement::eval(VariableEnvironment &env) const {
  ENTER_STMT;
  for (std::vector<LvalExpressionPtr>::const_iterator it = m_vals.begin();
       it != m_vals.end(); ++it) {
    (*it)->unset(env);
  }
}

void UnsetStatement::dump() const {
  printf("unset(");
  dumpVector(m_vals, ", ");
  printf(");");
}

///////////////////////////////////////////////////////////////////////////////
}
}

