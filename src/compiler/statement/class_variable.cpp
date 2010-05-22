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

void ClassVariable::onParse(AnalysisResultPtr ar) {
  ModifierExpressionPtr modifiers =
    ar->getScope()->setModifiers(m_modifiers);

  for (int i = 0; i < m_declaration->getCount(); i++) {
    VariableTablePtr variables = ar->getScope()->getVariables();
    ExpressionPtr exp = (*m_declaration)[i];
    if (exp->is(Expression::KindOfAssignmentExpression)) {
      AssignmentExpressionPtr assignment =
        dynamic_pointer_cast<AssignmentExpression>(exp);
      ExpressionPtr var = assignment->getVariable();
      const std::string &name =
        dynamic_pointer_cast<SimpleVariable>(var)->getName();
      if (variables->isPresent(name)) {
        ar->getCodeError()->record(CodeError::DeclaredVariableTwice, exp);
      }
      IParseHandlerPtr ph = dynamic_pointer_cast<IParseHandler>(exp);
      ph->onParse(ar);
    } else {
      const std::string &name =
        dynamic_pointer_cast<SimpleVariable>(exp)->getName();
      if (variables->isPresent(name)) {
        ar->getCodeError()->record(CodeError::DeclaredVariableTwice, exp);
      }
      variables->add(name, TypePtr(), false, ar, exp, m_modifiers);
    }
  }

  ar->getScope()->setModifiers(modifiers);
}

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void ClassVariable::analyzeProgramImpl(AnalysisResultPtr ar) {
  m_declaration->analyzeProgram(ar);
  if (ar->getPhase() != AnalysisResult::AnalyzeInclude) return;
  ClassScopePtr scope = ar->getClassScope();
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

StatementPtr ClassVariable::preOptimize(AnalysisResultPtr ar) {
  ar->preOptimize(m_modifiers);
  ar->preOptimize(m_declaration);
  return StatementPtr();
}

StatementPtr ClassVariable::postOptimize(AnalysisResultPtr ar) {
  ar->postOptimize(m_modifiers);
  ar->postOptimize(m_declaration);
  return StatementPtr();
}

void ClassVariable::inferTypes(AnalysisResultPtr ar) {
  m_declaration->inferAndCheck(ar, NEW_TYPE(Some), false);

  if (m_modifiers->isStatic()) {
    ClassScopePtr scope = ar->getClassScope();
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
            scope->getVariables()->forceVariant(ar, var->getName());
          }
        }
        ExpressionPtr value = assignment->getValue();
        if (value->containsDynamicConstant(ar)) {
          scope->getVariables()->
            setAttribute(VariableTable::ContainsDynamicStatic);
        }
      } else {
        SimpleVariablePtr var = dynamic_pointer_cast<SimpleVariable>(exp);
        TypePtr type = scope->getVariables()->getFinalType(var->getName());
        // If the class variable's type is Object, we have to
        // force it to be a Variant, because we don't include
        // the class header files in global_variables.h
        if (type->is(Type::KindOfObject)) {
          scope->getVariables()->forceVariant(ar, var->getName());
        }
        const char *initializer = type->getCPPInitializer();
        if (initializer) scope->setNeedStaticInitializer();
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void ClassVariable::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  m_modifiers->outputPHP(cg, ar);
  cg.printf(" ");
  m_declaration->outputPHP(cg, ar);
  cg.printf(";\n");
}

void ClassVariable::outputCPPImpl(CodeGenerator &cg, AnalysisResultPtr ar) {
  ClassScopePtr scope = ar->getClassScope();
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
          cg.printf("%sset(\"%s\",-1, ", Option::ObjectPrefix,
                    var->getName().c_str());
          value->outputCPP(cg, ar);
          cg.printf(")");
        } else {
          cg.printf("%s%s = ", Option::PropertyPrefix, var->getName().c_str());
          value->outputCPP(cg, ar);
        }
        cg.printf(";\n");
        value->outputCPPEnd(cg, ar);
      } else {
        var = dynamic_pointer_cast<SimpleVariable>(exp);
        if (derivFromRedec) {
          cg.printf("%sset(\"%s\",-1, null);\n", Option::ObjectPrefix,
                    var->getName().c_str());
        } else  {
          type = scope->getVariables()->getFinalType(var->getName());
          const char *initializer = type->getCPPInitializer();
          if (initializer) {
            cg.printf("%s%s = %s;\n", Option::PropertyPrefix,
                      var->getName().c_str(), initializer);
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

          var = dynamic_pointer_cast<SimpleVariable>(assignment->getVariable());
          ExpressionPtr value = assignment->getValue();
          if (!value->isScalar()) continue;
          cg.printf("g->%s%s%s%s = ",
                    Option::StaticPropertyPrefix, scope->getId().c_str(),
                    Option::IdPrefix.c_str(), var->getName().c_str());


          value->outputCPP(cg, ar);
        } else {
          var = dynamic_pointer_cast<SimpleVariable>(exp);
          type = scope->getVariables()->getFinalType(var->getName());
          const char *initializer = type->getCPPInitializer();
          if (initializer) {
            cg.printf("g->%s%s%s%s = %s",
                      Option::StaticPropertyPrefix, scope->getId().c_str(),
                      Option::IdPrefix.c_str(), var->getName().c_str(),
                      initializer);
          }
        }
        cg.printf(";\n");
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
        if (value->isScalar()) continue;
        value->outputCPPBegin(cg, ar);
        cg.printf("g->%s%s%s%s = ",
                  Option::StaticPropertyPrefix, scope->getId().c_str(),
                  Option::IdPrefix.c_str(), var->getName().c_str());
        value->outputCPP(cg, ar);
        cg.printf(";\n");
        value->outputCPPEnd(cg, ar);
      }
      break;
    default:
      break;
    }
  }
}
