/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_TRAIT_ALIAS_STATEMENT_H_
#define incl_HPHP_TRAIT_ALIAS_STATEMENT_H_

#include "hphp/compiler/statement/statement.h"
#include "hphp/compiler/expression/scalar_expression.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(ExpressionList);
DECLARE_BOOST_TYPES(StatementList);
DECLARE_BOOST_TYPES(TraitAliasStatement);

class TraitAliasStatement : public Statement {
public:
  TraitAliasStatement(STATEMENT_CONSTRUCTOR_PARAMETERS,
                      ScalarExpressionPtr traitName,
                      ScalarExpressionPtr methodName,
                      ModifierExpressionPtr modifiers,
                      ScalarExpressionPtr newMethodName);

  DECLARE_STATEMENT_VIRTUAL_FUNCTIONS;
  const std::string& getMethodName() const { return m_methodName->getString(); }
  const std::string& getTraitName()  const { return m_traitName->getString(); }
  const std::string& getNewMethodName() const {
    return m_newMethodName->getString();
  }
  const ModifierExpressionPtr getModifiers() const { return m_modifiers; }

  void setNewMethodName(ScalarExpressionPtr newMethodName) {
    m_newMethodName = newMethodName;
  }

  void setModifiers(ModifierExpressionPtr modifiers) {
    m_modifiers = modifiers;
  }

private:
  ScalarExpressionPtr   m_traitName;  // selected trait name
  ScalarExpressionPtr   m_methodName; // method name in the trait
  ModifierExpressionPtr m_modifiers;
  ScalarExpressionPtr   m_newMethodName;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_TRAIT_ALIAS_STATEMENT_H_
