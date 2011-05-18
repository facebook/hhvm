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

#include <compiler/statement/class_variable.h>
#include <compiler/analysis/analysis_result.h>
#include <compiler/expression/modifier_expression.h>
#include <compiler/expression/expression_list.h>
#include <compiler/analysis/block_scope.h>
#include <compiler/analysis/class_scope.h>
#include <compiler/analysis/variable_table.h>
#include <compiler/expression/simple_variable.h>
#include <compiler/expression/assignment_expression.h>
#include <compiler/option.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

ClassVariable::ClassVariable
(STATEMENT_CONSTRUCTOR_PARAMETERS,
 ModifierExpressionPtr modifiers, ExpressionListPtr declaration)
  : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES),
    m_modifiers(modifiers), m_declaration(declaration) {
}

StatementPtr ClassVariable::clone() {
  ClassVariablePtr stmt(new ClassVariable(*this));
  stmt->m_modifiers = Clone(m_modifiers);
  stmt->m_declaration = Clone(m_declaration);
  return stmt;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

void ClassVariable::onParseRecur(AnalysisResultConstPtr ar,
                                 ClassScopePtr scope) {
  ModifierExpressionPtr modifiers =
    scope->setModifiers(m_modifiers);

  for (int i = 0; i < m_declaration->getCount(); i++) {
    VariableTablePtr variables = scope->getVariables();
    ExpressionPtr exp = (*m_declaration)[i];
    if (exp->is(Expression::KindOfAssignmentExpression)) {
      AssignmentExpressionPtr assignment =
        dynamic_pointer_cast<AssignmentExpression>(exp);
      ExpressionPtr var = assignment->getVariable();
      const std::string &name =
        dynamic_pointer_cast<SimpleVariable>(var)->getName();
      if (variables->isPresent(name)) {
        Compiler::Error(Compiler::DeclaredVariableTwice, exp);
        m_declaration->removeElement(i--);
      } else {
        assignment->onParseRecur(ar, scope);
      }
    } else {
      const std::string &name =
        dynamic_pointer_cast<SimpleVariable>(exp)->getName();
      if (variables->isPresent(name)) {
        Compiler::Error(Compiler::DeclaredVariableTwice, exp);
        m_declaration->removeElement(i--);
      } else {
        variables->add(name, Type::Variant, false, ar, exp, m_modifiers);
      }
    }
  }

  scope->setModifiers(modifiers);
}

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void ClassVariable::analyzeProgramImpl(AnalysisResultPtr ar) {
  m_declaration->analyzeProgram(ar);
  AnalysisResult::Phase phase = ar->getPhase();
  if (phase != AnalysisResult::AnalyzeAll) {
    return;
  }
  ClassScopePtr scope = getClassScope();
  for (int i = 0; i < m_declaration->getCount(); i++) {
    ExpressionPtr exp = (*m_declaration)[i];
    if (exp->is(Expression::KindOfAssignmentExpression)) {
      AssignmentExpressionPtr assignment =
        dynamic_pointer_cast<AssignmentExpression>(exp);
      SimpleVariablePtr var =
        dynamic_pointer_cast<SimpleVariable>(assignment->getVariable());
      ExpressionPtr value = assignment->getValue();
      scope->getVariables()->setClassInitVal(var->getName(), value);
      scope->getVariables()->markOverride(ar, var->getName());
    } else {
      SimpleVariablePtr var =
        dynamic_pointer_cast<SimpleVariable>(exp);
      scope->getVariables()->markOverride(ar, var->getName());
      scope->getVariables()->setClassInitVal(var->getName(),
                                             makeConstant(ar, "null"));
    }
  }
}

ConstructPtr ClassVariable::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_modifiers;
    case 1:
      return m_declaration;
    default:
      ASSERT(false);
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
      m_modifiers = boost::dynamic_pointer_cast<ModifierExpression>(cp);
      break;
    case 1:
      m_declaration = boost::dynamic_pointer_cast<ExpressionList>(cp);
      break;
    default:
      ASSERT(false);
      break;
  }
}

StatementPtr ClassVariable::preOptimize(AnalysisResultConstPtr ar) {
  ClassScopePtr scope = getClassScope();
  for (int i = 0; i < m_declaration->getCount(); i++) {
    ExpressionPtr exp = (*m_declaration)[i];
    if (exp->is(Expression::KindOfAssignmentExpression)) {
      AssignmentExpressionPtr assignment =
        dynamic_pointer_cast<AssignmentExpression>(exp);
      SimpleVariablePtr var =
        dynamic_pointer_cast<SimpleVariable>(assignment->getVariable());
      ExpressionPtr value = assignment->getValue();
      scope->getVariables()->setClassInitVal(var->getName(), value);
    }
  }
  return StatementPtr();
}

void ClassVariable::inferTypes(AnalysisResultPtr ar) {
  m_declaration->inferAndCheck(ar, Type::Variant, false);

  if (m_modifiers->isStatic()) {
    ClassScopePtr scope = getClassScope();
    for (int i = 0; i < m_declaration->getCount(); i++) {
      ExpressionPtr exp = (*m_declaration)[i];
      if (exp->is(Expression::KindOfAssignmentExpression)) {
        scope->setNeedStaticInitializer();
        AssignmentExpressionPtr assignment =
          dynamic_pointer_cast<AssignmentExpression>(exp);
        // If the class variable's type is Object, we have to
        // force it to be a Variant, because we don't include
        // the class header files in global_variables.h
        SimpleVariablePtr var =
          dynamic_pointer_cast<SimpleVariable>(assignment->getVariable());
        if (var) {
          TypePtr type = scope->getVariables()->getFinalType(var->getName());
          if (type->is(Type::KindOfObject)) {
            scope->getVariables()->forceVariant(ar, var->getName(),
                                                VariableTable::AnyVars);
          }
        }
        ExpressionPtr value = assignment->getValue();
        if (value->containsDynamicConstant(ar)) {
          scope->getVariables()->
            setAttribute(VariableTable::ContainsDynamicStatic);
        }
      } else {
        scope->setNeedStaticInitializer();
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void ClassVariable::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  m_modifiers->outputPHP(cg, ar);
  cg_printf(" ");
  m_declaration->outputPHP(cg, ar);
  cg_printf(";\n");
}

void ClassVariable::outputCPPImpl(CodeGenerator &cg, AnalysisResultPtr ar) {
  ClassScopePtr scope = getClassScope();
  bool derivFromRedec = scope->derivesFromRedeclaring() &&
    !m_modifiers->isPrivate();
  for (int i = 0; i < m_declaration->getCount(); i++) {
    ExpressionPtr exp = (*m_declaration)[i];

    SimpleVariablePtr var;
    TypePtr type;

    switch (cg.getContext()) {
    case CodeGenerator::CppConstructor:
      if (m_modifiers->isStatic()) continue;

      if (exp->is(Expression::KindOfAssignmentExpression)) {
        AssignmentExpressionPtr assignment =
          dynamic_pointer_cast<AssignmentExpression>(exp);

        var = dynamic_pointer_cast<SimpleVariable>(assignment->getVariable());
        ExpressionPtr value = assignment->getValue();
        value->outputCPPBegin(cg, ar);
        if (derivFromRedec) {
          cg_printf("%sset(", Option::ObjectPrefix);
          cg_printString(var->getName(), ar, shared_from_this());
          cg_printf(", ");
          value->outputCPP(cg, ar);
          cg_printf(")");
        } else if (value->isLiteralNull()) {
          cg_printf("setNull(%s%s)", Option::PropertyPrefix,
                    var->getName().c_str());
        } else {
          cg_printf("%s%s = ", Option::PropertyPrefix, var->getName().c_str());
          value->outputCPP(cg, ar);
        }
        cg_printf(";\n");
        value->outputCPPEnd(cg, ar);
      } else {
        var = dynamic_pointer_cast<SimpleVariable>(exp);
        if (derivFromRedec) {
          cg_printf("%sset(", Option::ObjectPrefix);
          cg_printString(var->getName(), ar, shared_from_this());
          cg_printf(", null_variant);\n");
        } else {
          type = scope->getVariables()->getFinalType(var->getName());
          if (type->is(Type::KindOfVariant)) {
            cg_printf("setNull(%s%s);\n", Option::PropertyPrefix,
                      var->getName().c_str());
          } else {
            const char *initializer = type->is(Type::KindOfVariant) ? "null" :
              type->getCPPInitializer();
            if (initializer) {
              cg_printf("%s%s = %s;\n", Option::PropertyPrefix,
                        var->getName().c_str(), initializer);
            }
          }
        }
      }
      break;

    case CodeGenerator::CppStaticInitializer:
      {
        if (!m_modifiers->isStatic()) continue;

        VariableTablePtr variables = scope->getVariables();
        if (exp->is(Expression::KindOfAssignmentExpression)) {
          AssignmentExpressionPtr assignment =
            dynamic_pointer_cast<AssignmentExpression>(exp);

          var = dynamic_pointer_cast<SimpleVariable>
            (assignment->getVariable());
          ExpressionPtr value = assignment->getValue();
          if (value->containsDynamicConstant(ar)) continue;
          Symbol *sym = scope->getVariables()->getSymbol(var->getName());
          if (sym->isOverride()) continue;
          if (value->isLiteralNull()) {
            cg_printf("setNull(g->%s%s%s%s)",
                      Option::StaticPropertyPrefix, scope->getId(cg).c_str(),
                      Option::IdPrefix.c_str(), var->getName().c_str());
          } else {
            cg_printf("g->%s%s%s%s = ",
                      Option::StaticPropertyPrefix, scope->getId(cg).c_str(),
                      Option::IdPrefix.c_str(), var->getName().c_str());
            value->outputCPP(cg, ar);
          }
        } else {
          var = dynamic_pointer_cast<SimpleVariable>(exp);
          type = scope->getVariables()->getFinalType(var->getName());
          const char *initializer = type->getCPPInitializer();
          if (initializer) {
            cg_printf("g->%s%s%s%s = %s",
                      Option::StaticPropertyPrefix, scope->getId(cg).c_str(),
                      Option::IdPrefix.c_str(), var->getName().c_str(),
                      initializer);
          }
        }
        cg_printf(";\n");
      }
      break;
    case CodeGenerator::CppLazyStaticInitializer:
      {
        if (!m_modifiers->isStatic()) continue;
        if (!exp->is(Expression::KindOfAssignmentExpression)) continue;
        VariableTablePtr variables = scope->getVariables();
        AssignmentExpressionPtr assignment =
          dynamic_pointer_cast<AssignmentExpression>(exp);
        var = dynamic_pointer_cast<SimpleVariable>(assignment->getVariable());
        ExpressionPtr value = assignment->getValue();
        if (!value->containsDynamicConstant(ar)) continue;
        Symbol *sym = scope->getVariables()->getSymbol(var->getName());
        if (sym->isOverride()) continue;
        value->outputCPPBegin(cg, ar);
        cg_printf("g->%s%s%s%s = ",
                  Option::StaticPropertyPrefix, scope->getId(cg).c_str(),
                  Option::IdPrefix.c_str(), var->getName().c_str());
        value->outputCPP(cg, ar);
        cg_printf(";\n");
        value->outputCPPEnd(cg, ar);
      }
      break;
    default:
      break;
    }
  }
}
