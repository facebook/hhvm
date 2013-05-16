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

#include "hphp/compiler/expression/modifier_expression.h"
#include "hphp/util/parser/hphp.tab.hpp"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

ModifierExpression::ModifierExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES(ModifierExpression)) {
}

ExpressionPtr ModifierExpression::clone() {
  ModifierExpressionPtr exp(new ModifierExpression(*this));
  Expression::deepCopy(exp);
  return exp;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

void ModifierExpression::add(int modifier) {
  m_modifiers.push_back(modifier);
}

int ModifierExpression::operator[](int index) {
  assert(index >= 0 && index < getCount());
  return m_modifiers[index];
}

bool ModifierExpression::isPublic() const {
  for (unsigned int i = 0; i < m_modifiers.size(); i++) {
    switch (m_modifiers[i]) {
    case T_PUBLIC:      return true;
    case T_PROTECTED:
    case T_PRIVATE:     return false;
    default:
      break;
    }
  }
  return true;
}

bool ModifierExpression::hasModifier(int modifier) const {
  for (unsigned int i = 0; i < m_modifiers.size(); i++) {
    if (m_modifiers[i] == modifier) {
      return true;
    }
  }
  return false;
}

bool ModifierExpression::isProtected() const {
  return hasModifier(T_PROTECTED);
}

bool ModifierExpression::isPrivate() const {
  return hasModifier(T_PRIVATE);
}

bool ModifierExpression::isStatic() const {
  return hasModifier(T_STATIC);
}

bool ModifierExpression::isAbstract() const {
  return hasModifier(T_ABSTRACT);
}

bool ModifierExpression::isFinal() const {
  return hasModifier(T_FINAL);
}

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void ModifierExpression::analyzeProgram(AnalysisResultPtr ar) {
  // do nothing
}

TypePtr ModifierExpression::inferTypes(AnalysisResultPtr ar, TypePtr type,
                                       bool coerce) {
  assert(false);
  return TypePtr();
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void ModifierExpression::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (m_modifiers.empty()) {
    cg_printf("public");
    return;
  }

  bool printed = false;
  for (unsigned int i = 0; i < m_modifiers.size(); i++) {
    switch (m_modifiers[i]) {
    case T_PUBLIC:    cg_printf("public");    printed = true; break;
    case T_PROTECTED: cg_printf("protected"); printed = true; break;
    case T_PRIVATE:   cg_printf("private");   printed = true; break;
    }
  }
  if (!printed) {
    cg_printf("public");
  }

  for (unsigned int i = 0; i < m_modifiers.size(); i++) {
    switch (m_modifiers[i]) {
    case T_PUBLIC:    break;
    case T_PROTECTED: break;
    case T_PRIVATE:   break;
    case T_STATIC:    cg_printf(" static");    break;
    case T_ABSTRACT:  cg_printf(" abstract");  break;
    case T_FINAL:     cg_printf(" final");     break;
    default:
      assert(false);
    }
  }
}
