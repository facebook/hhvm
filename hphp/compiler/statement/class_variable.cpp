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

#include "hphp/compiler/statement/class_variable.h"
#include "hphp/compiler/analysis/analysis_result.h"
#include "hphp/compiler/expression/modifier_expression.h"
#include "hphp/compiler/expression/expression_list.h"
#include "hphp/compiler/analysis/block_scope.h"
#include "hphp/compiler/analysis/class_scope.h"
#include "hphp/compiler/expression/simple_variable.h"
#include "hphp/compiler/expression/assignment_expression.h"
#include "hphp/compiler/option.h"

#include "hphp/runtime/base/comparisons.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

ClassVariable::ClassVariable
(STATEMENT_CONSTRUCTOR_PARAMETERS,
 ModifierExpressionPtr modifiers, std::string typeConstraint,
 ExpressionListPtr declaration, TypeAnnotationPtr typeAnnot,
 ExpressionListPtr attrList)
  : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES(ClassVariable)),
    m_modifiers(modifiers), m_typeConstraint(typeConstraint),
    m_declaration(declaration), m_typeAnnotation(typeAnnot),
    m_attributeList(attrList) {
}

StatementPtr ClassVariable::clone() {
  ClassVariablePtr stmt(new ClassVariable(*this));
  stmt->m_modifiers = Clone(m_modifiers);
  stmt->m_declaration = Clone(m_declaration);
  stmt->m_attributeList = Clone(m_attributeList);
  return stmt;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

void ClassVariable::onParseRecur(AnalysisResultConstRawPtr /*ar*/,
                                 FileScopeRawPtr fs, ClassScopePtr scope) {
  ModifierExpressionPtr modifiers =
    scope->setModifiers(m_modifiers);

  if (m_modifiers->isAbstract()) {
    m_modifiers->parseTimeFatal(fs,
                                "Properties cannot be declared abstract");
  }

  if (m_modifiers->isFinal()) {
    m_modifiers->parseTimeFatal(fs,
                                "Properties cannot be declared final");
  }

  if (!m_modifiers->isStatic() && scope->isStaticUtil()) {
    m_modifiers->parseTimeFatal(
      fs,
      "Class %s contains non-static property declaration and "
      "therefore cannot be declared 'abstract final'",
      scope->getOriginalName().c_str()
    );
  }

  if ((m_modifiers->isExplicitlyPublic() +
       m_modifiers->isProtected() +
       m_modifiers->isPrivate()) > 1) {
    m_modifiers->parseTimeFatal(
      fs,
      "%s: properties of %s",
      Strings::PICK_ACCESS_MODIFIER,
      scope->getOriginalName().c_str()
    );
  }

  scope->setModifiers(modifiers);
}

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

ConstructPtr ClassVariable::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_modifiers;
    case 1:
      return m_declaration;
    default:
      assert(false);
      break;
  }
  return ConstructPtr();
}

int ClassVariable::getKidCount() const {
  return 2;
}

void ClassVariable::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_modifiers = dynamic_pointer_cast<ModifierExpression>(cp);
      break;
    case 1:
      m_declaration = dynamic_pointer_cast<ExpressionList>(cp);
      break;
    default:
      assert(false);
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void ClassVariable::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  m_modifiers->outputPHP(cg, ar);
  m_declaration->outputPHP(cg, ar);
  cg_printf(";\n");
}
