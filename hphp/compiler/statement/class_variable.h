/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#ifndef incl_HPHP_CLASS_VARIABLE_H_
#define incl_HPHP_CLASS_VARIABLE_H_

#include "hphp/compiler/statement/statement.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(ModifierExpression);
DECLARE_BOOST_TYPES(ExpressionList);
DECLARE_BOOST_TYPES(ClassVariable);
DECLARE_BOOST_TYPES(SimpleVariable);

struct ClassVariable : Statement, IParseHandler {
  ClassVariable(STATEMENT_CONSTRUCTOR_PARAMETERS,
                ModifierExpressionPtr modifiers,
                std::string typeConstraint,
                ExpressionListPtr declaration);

  DECLARE_STATEMENT_VIRTUAL_FUNCTIONS;
  StatementPtr preOptimize(AnalysisResultConstPtr ar) override;

  // implementing IParseHandler
  void onParseRecur(AnalysisResultConstPtr ar, FileScopeRawPtr fs,
                    ClassScopePtr scope) override;

  std::string getTypeConstraint() const { return m_typeConstraint; }
  ExpressionListPtr getVarList() const { return m_declaration; }
  ModifierExpressionPtr getModifiers() const { return m_modifiers; }

  void addTraitPropsToScope(AnalysisResultPtr ar, ClassScopePtr scope);

private:

  ModifierExpressionPtr m_modifiers;
  std::string m_typeConstraint;
  ExpressionListPtr m_declaration;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_CLASS_VARIABLE_H_
