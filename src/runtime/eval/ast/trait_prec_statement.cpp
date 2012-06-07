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

#include <runtime/eval/ast/trait_prec_statement.h>
#include <runtime/eval/runtime/variable_environment.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

TraitPrecStatement::TraitPrecStatement(
  STATEMENT_ARGS, NamePtr traitName, NamePtr methodName,
                     const std::vector<NamePtr> &names)
  : Statement(STATEMENT_PASS), m_traitName(traitName),
    m_methodName(methodName), m_names(names) {}

void TraitPrecStatement::eval(VariableEnvironment &env) const {
  ENTER_STMT;
  ASSERT(false);
}

void TraitPrecStatement::dump(std::ostream &out) const {
  out << "use \n";
  m_traitName->dump(out);
  out << "::";
  m_methodName->dump(out);
  out << " insteadof ";
  dumpVector(out, m_names, ",");
  out << "\n";
}

///////////////////////////////////////////////////////////////////////////////
}
}

