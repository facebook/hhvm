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

#include <runtime/eval/ast/goto_statement.h>
#include <runtime/eval/runtime/variable_environment.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

GotoStatement::GotoStatement(STATEMENT_ARGS, const std::string &label,
                             bool limited)
    : Statement(STATEMENT_PASS), m_label(label), m_limited(limited) {}

void GotoStatement::eval(VariableEnvironment &env) const {
  if (env.isGotoing()) return;
  ENTER_STMT;
  env.setGoto(m_label, m_limited);
  if (m_limited) {
    throw GotoException(); // for current loop to catch
  } else {
    throw UnlimitedGotoException(); // for function to catch
  }
}

void GotoStatement::dump(std::ostream &out) const {
  out << "goto " << m_label << ";\n";
}

///////////////////////////////////////////////////////////////////////////////
}
}

