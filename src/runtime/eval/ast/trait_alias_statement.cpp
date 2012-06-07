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

#include <runtime/eval/ast/trait_alias_statement.h>
#include <runtime/eval/runtime/variable_environment.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

TraitAliasStatement::TraitAliasStatement(
  STATEMENT_ARGS, NamePtr traitName, NamePtr methodName,
  NamePtr newMethodName, int modifiers)
  : Statement(STATEMENT_PASS), m_traitName(traitName),
    m_methodName(methodName), m_newMethodName(newMethodName),
    m_modifiers(modifiers) {}

void TraitAliasStatement::eval(VariableEnvironment &env) const {
  ENTER_STMT;
  ASSERT(false);
}

void TraitAliasStatement::dump(std::ostream &out) const {
  m_traitName->dump(out);
  out << "::";
  m_methodName->dump(out);
  out << " as ";
  m_newMethodName->dump(out);
}

///////////////////////////////////////////////////////////////////////////////
}
}

