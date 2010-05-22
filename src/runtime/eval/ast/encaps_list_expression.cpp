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

#include <runtime/eval/ast/encaps_list_expression.h>
#include <runtime/ext/ext_process.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

EncapsListExpression::EncapsListExpression(EXPRESSION_ARGS,
                                           std::vector<ExpressionPtr> encaps,
                                           bool shell)
  : Expression(EXPRESSION_PASS), m_encaps(encaps), m_shell(shell) {}

Variant EncapsListExpression::eval(VariableEnvironment &env) const {
  String result = "";
  for (std::vector<ExpressionPtr>::const_iterator it = m_encaps.begin();
       it != m_encaps.end(); ++it) {
    Variant e((*it)->eval(env));
    concat_assign(result, e.toString());
  }
  if (m_shell) {
    return f_shell_exec(result);
  }
  return result;
}

void EncapsListExpression::dump() const {
  if (m_shell) {
    printf("shell_exec");
  }
  printf("(");
  dumpVector(m_encaps, ".");
  printf(")");
}

///////////////////////////////////////////////////////////////////////////////
}
}

