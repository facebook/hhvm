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

#include <lib/statement/foreach_statement.h>
#include <lib/analysis/analysis_result.h>
#include <lib/analysis/block_scope.h>
#include <lib/expression/simple_variable.h>
#include <lib/option.h>
#include <lib/analysis/code_error.h>
#include <lib/analysis/class_scope.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

ForEachStatement::ForEachStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS,
 ExpressionPtr array, ExpressionPtr name, bool nameRef,
 ExpressionPtr value, bool valueRef, StatementPtr stmt)
  : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES),
    m_array(array), m_name(name), m_value(value), m_ref(valueRef),
    m_stmt(stmt) {
  if (!m_value) {
    m_value = m_name;
    m_ref = nameRef;
    m_name.reset();
  }
  if (m_name) {
    m_name->setContext(Expression::LValue);
    m_name->setContext(Expression::NoLValueWrapper);
  }
  m_value->setContext(Expression::LValue);
  m_value->setContext(Expression::NoLValueWrapper);
  if (m_ref) {
    m_array->setContext(Expression::RefValue);
    m_value->setContext(Expression::RefValue);
    m_value->setContext(Expression::NoRefWrapper);
  }
}

StatementPtr ForEachStatement::clone() {
  ForEachStatementPtr stmt(new ForEachStatement(*this));
  stmt->m_array = Clone(m_array);
  stmt->m_name = Clone(m_name);
  stmt->m_value = Clone(m_value);
  stmt->m_stmt = Clone(m_stmt);
  return stmt;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void ForEachStatement::analyzeProgram(AnalysisResultPtr ar) {
  m_array->analyzeProgram(ar);
  if (m_name) m_name->analyzeProgram(ar);
  m_value->analyzeProgram(ar);
  if (m_stmt) m_stmt->analyzeProgram(ar);
}

ConstructPtr ForEachStatement::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_array;
    case 1:
      return m_name;
    case 2:
      return m_value;
    case 3:
      return m_stmt;
    default:
      return ConstructPtr();
  }
  ASSERT(0);
}

int ForEachStatement::getKidCount() const {
  return 4;
}

int ForEachStatement::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_array = boost::dynamic_pointer_cast<Expression>(cp);
      return 1;
    case 1:
      m_name = boost::dynamic_pointer_cast<Expression>(cp);
      return 1;
    case 2:
      m_value = boost::dynamic_pointer_cast<Expression>(cp);
      return 1;
    case 3:
      m_stmt = boost::dynamic_pointer_cast<Statement>(cp);
      return 1;
    default:
      return 0;
  }
  ASSERT(0);
}

StatementPtr ForEachStatement::preOptimize(AnalysisResultPtr ar) {
  ar->preOptimize(m_array);
  ar->preOptimize(m_name);
  ar->preOptimize(m_value);
  ar->preOptimize(m_stmt);
  return StatementPtr();
}

StatementPtr ForEachStatement::postOptimize(AnalysisResultPtr ar) {
  ar->postOptimize(m_array);
  ar->postOptimize(m_name);
  ar->postOptimize(m_value);
  ar->postOptimize(m_stmt);
  return StatementPtr();
}

void ForEachStatement::inferTypes(AnalysisResultPtr ar) {
  if (ar->isFirstPass() &&
      !m_array->is(Expression::KindOfSimpleVariable) &&
      !m_array->is(Expression::KindOfArrayElementExpression) &&
      !m_array->is(Expression::KindOfObjectPropertyExpression)) {
    ConstructPtr self = shared_from_this();
    ar->getCodeError()->record(self, CodeError::ComplexForEach, self);
  }

  m_array->inferAndCheck(ar, Type::Array, true);
  if (m_name) {
    m_name->inferAndCheck(ar, NEW_TYPE(Primitive), true);
  }
  m_value->inferAndCheck(ar, Type::Variant, true);
  if (m_stmt) {
    ar->getScope()->incLoopNestedLevel();
    m_stmt->inferTypes(ar);
    ar->getScope()->decLoopNestedLevel();
  }
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void ForEachStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg.printf("foreach (");
  m_array->outputPHP(cg, ar);
  cg.printf(" as ");
  if (m_name) {
    m_name->outputPHP(cg, ar);
    cg.printf(" => ");
  }
  if (m_ref) cg.printf("&");
  m_value->outputPHP(cg, ar);
  cg.printf(") ");
  if (m_stmt) {
    m_stmt->outputPHP(cg, ar);
  } else {
    cg.printf("{}\n");
  }
}

void ForEachStatement::outputCPP(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg.indentBegin("{\n");
  int labelId = cg.createNewId(ar);
  cg.pushBreakScope(labelId);

  int mapId = cg.createNewId(ar);
  bool passTemp;
  bool isArray = false;

  if (m_ref) {
    passTemp = true;
    cg.printf("Variant %s%d = ref(", Option::MapPrefix, mapId);
    m_array->outputCPPImpl(cg, ar);
    cg.printf(");\n");
    cg.printf("%s%d.escalate();\n", Option::MapPrefix, mapId);
  } else if (!m_array->is(Expression::KindOfSimpleVariable) ||
             m_array->isThis()) {
    passTemp = true;
    cg.printf("Variant %s%d = ", Option::MapPrefix, mapId);
    TypePtr expectedType = m_array->getExpectedType();
    // Clear m_expectedType to avoid type cast (toArray).
    m_array->setExpectedType(TypePtr());
    m_array->outputCPP(cg, ar);
    m_array->setExpectedType(expectedType);
    cg.printf(";\n");
  } else {
    passTemp = false;
  }

  int iterId = cg.createNewId(ar);
  cg.printf("for (");
  if (m_ref) {
    cg.printf("MutableArrayIterPtr %s%d = %s%d.begin(",
              Option::IterPrefix, iterId, Option::MapPrefix, mapId);
    if (m_name) {
      cg.printf("&");
      m_name->outputCPP(cg, ar);
    } else {
      cg.printf("NULL");
    }
    cg.printf(", ");
    m_value->outputCPP(cg, ar);
    cg.printf("); %s%d->advance();", Option::IterPrefix, iterId);
  } else {
    if (passTemp) {
      cg.printf("ArrayIterPtr %s%d = %s%d.begin(",
                Option::IterPrefix, iterId,
                Option::MapPrefix, mapId);
      ClassScopePtr cls = ar->getClassScope();
      if (cls) {
        cg.printf("\"%s\"", cls->getName().c_str());
      }
      cg.printf("); ");
      cg.printf("!%s%d->end(); %s%d->next()",
                Option::IterPrefix, iterId,
                Option::IterPrefix, iterId);
    } else {
      TypePtr actualType = m_array->getActualType();
      if (actualType && actualType->is(Type::KindOfArray)) {
        isArray = true;
        cg.printf("ArrayIter %s%d = ", Option::IterPrefix, iterId);
      } else {
        cg.printf("ArrayIterPtr %s%d = ", Option::IterPrefix, iterId);
      }
      TypePtr expectedType = m_array->getExpectedType();
      // Clear m_expectedType to avoid type cast (toArray).
      m_array->setExpectedType(TypePtr());
      m_array->outputCPP(cg, ar);
      m_array->setExpectedType(expectedType);
      cg.printf(".begin(");
      ClassScopePtr cls = ar->getClassScope();
      if (cls) {
        cg.printf("\"%s\"", cls->getName().c_str());
      }
      cg.printf("); ");
      if (isArray) {
        cg.printf("!%s%d.end(); ", Option::IterPrefix, iterId);
        cg.printf("++%s%d", Option::IterPrefix, iterId);
      } else {
        cg.printf("!%s%d->end(); ", Option::IterPrefix, iterId);
        cg.printf("%s%d->next()", Option::IterPrefix, iterId);
      }
    }
  }
  cg.indentBegin(") {\n");
  cg.printf("LOOP_COUNTER_CHECK(%d);\n", labelId);

  if (!m_ref) {
    m_value->outputCPP(cg, ar);
    cg.printf(isArray ? " = %s%d.second();\n" : " = %s%d->second();\n",
              Option::IterPrefix, iterId);
    if (m_name) {
      m_name->outputCPP(cg, ar);
      cg.printf(isArray ? " = %s%d.first();\n" : " = %s%d->first();\n",
                Option::IterPrefix, iterId);
    }
  }
  if (m_stmt) {
    m_stmt->outputCPP(cg, ar);
  }
  if (cg.findLabelId("continue", labelId)) {
    cg.printf("continue%d:;\n", labelId);
  }
  cg.indentEnd("}\n");
  if (cg.findLabelId("break", labelId)) {
    cg.printf("break%d:;\n", labelId);
  }
  cg.popBreakScope();

  cg.indentEnd("}\n");
}
