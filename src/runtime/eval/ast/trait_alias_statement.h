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

#ifndef __EVAL_TRAIT_ALIAS_STATEMENT_H__
#define __EVAL_TRAIT_ALIAS_STATEMENT_H__

#include <runtime/eval/ast/expression.h>
#include <runtime/eval/ast/name.h>
#include <runtime/eval/ast/statement.h>
#include <runtime/eval/ast/statement_list_statement.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_AST_PTR(TraitAliasStatement);

class TraitAliasStatement : public Statement {
public:
  TraitAliasStatement(STATEMENT_ARGS, NamePtr traitName, NamePtr methodName,
                      NamePtr newMethodName, int modifiers);
  Name *getTraitName() const { return m_traitName.get(); }
  Name *getMethodName() const { return m_methodName.get(); }
  Name *getNewMethodName() const { return m_newMethodName.get(); }
  int getModifiers() const { return m_modifiers; }
  void setNewMethodName(NamePtr newMethodName) {
    m_newMethodName = newMethodName;
  }
  void setModifiers(int modifiers) { m_modifiers = modifiers; }
  virtual void eval(VariableEnvironment &env) const;
  virtual void dump(std::ostream &out) const;
private:
  NamePtr m_traitName;
  NamePtr m_methodName;
  NamePtr m_newMethodName;
  int m_modifiers;
};

///////////////////////////////////////////////////////////////////////////////
}
}

#endif /* __EVAL_TRAIT_ALIAS_STATEMENT_H__ */
