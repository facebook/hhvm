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

#include <compiler/statement/foreach_statement.h>
#include <compiler/analysis/analysis_result.h>
#include <compiler/analysis/block_scope.h>
#include <compiler/expression/simple_variable.h>
#include <compiler/option.h>
#include <compiler/analysis/code_error.h>
#include <compiler/analysis/class_scope.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

ForEachStatement::ForEachStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS,
 ExpressionPtr array, ExpressionPtr name, bool nameRef,
 ExpressionPtr value, bool valueRef, StatementPtr stmt)
  : LoopStatement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES),
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

void ForEachStatement::analyzeProgramImpl(AnalysisResultPtr ar) {
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
      ASSERT(false);
      break;
  }
  return ConstructPtr();
}

int ForEachStatement::getKidCount() const {
  return 4;
}

void ForEachStatement::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_array = boost::dynamic_pointer_cast<Expression>(cp);
      break;
    case 1:
      m_name = boost::dynamic_pointer_cast<Expression>(cp);
      break;
    case 2:
      m_value = boost::dynamic_pointer_cast<Expression>(cp);
      break;
    case 3:
      m_stmt = boost::dynamic_pointer_cast<Statement>(cp);
      break;
    default:
      ASSERT(false);
      break;
  }
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
  if (m_ref) {
    TypePtr actualType = m_array->getActualType();
    if (!actualType ||
        actualType->is(Type::KindOfVariant) ||
        actualType->is(Type::KindOfObject)) {
      ar->forceClassVariants();
    }
  }
  if (m_stmt) {
    ar->getScope()->incLoopNestedLevel();
    m_stmt->inferTypes(ar);
    ar->getScope()->decLoopNestedLevel();
  }
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void ForEachStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg_printf("foreach (");
  m_array->outputPHP(cg, ar);
  cg_printf(" as ");
  if (m_name) {
    m_name->outputPHP(cg, ar);
    cg_printf(" => ");
  }
  if (m_ref) cg_printf("&");
  m_value->outputPHP(cg, ar);
  cg_printf(") ");
  if (m_stmt) {
    m_stmt->outputPHP(cg, ar);
  } else {
    cg_printf("{}\n");
  }
}

void ForEachStatement::outputCPPImpl(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg_indentBegin("{\n");
  int labelId = cg.createNewLocalId(ar);
  cg.pushBreakScope(labelId);

  int mapId = cg.createNewLocalId(ar);
  bool passTemp = true;
  bool isArray = false;

  if (m_ref ||
      !m_array->is(Expression::KindOfSimpleVariable) ||
      m_array->isThis()) {
    cg_printf("Variant %s%d", Option::MapPrefix, mapId);
    bool wrap = m_array->preOutputCPP(cg, ar, 0);
    if (wrap) {
      cg_printf(";\n");
      m_array->outputCPPBegin(cg, ar);
      cg_printf("%s%d", Option::MapPrefix, mapId);
    }
    if (m_ref) {
      cg_printf(" = ref(");
      m_array->outputCPPImpl(cg, ar);
      cg.printf(");\n");
      cg.printf("%s%d.escalate(true);\n", Option::MapPrefix, mapId);
    } else {
      TypePtr expectedType = m_array->getExpectedType();
      // Clear m_expectedType to avoid type cast (toArray).
      m_array->setExpectedType(TypePtr());
      cg_printf(" = ");
      m_array->outputCPP(cg, ar);
      cg_printf(";\n");
      m_array->setExpectedType(expectedType);
    }
    if (wrap) {
      m_array->outputCPPEnd(cg, ar);
    }
  } else {
    passTemp = false;
  }

  cppDeclareBufs(cg, ar);
  int iterId = cg.createNewLocalId(ar);
  cg_printf("for (");
  if (m_ref) {
    cg_printf("MutableArrayIterPtr %s%d = %s%d.begin(",
              Option::IterPrefix, iterId, Option::MapPrefix, mapId);
    if (m_name) {
      cg_printf("&");
      m_name->outputCPP(cg, ar);
    } else {
      cg_printf("NULL");
    }
    cg_printf(", ");
    m_value->outputCPP(cg, ar);
    cg_printf("); %s%d->advance();", Option::IterPrefix, iterId);
  } else {
    if (passTemp) {
      cg_printf("ArrayIterPtr %s%d = %s%d.begin(",
                Option::IterPrefix, iterId,
                Option::MapPrefix, mapId);
      ClassScopePtr cls = ar->getClassScope();
      if (cls) {
        cg_printf("%sclass_name", Option::StaticPropertyPrefix);
      }
      cg_printf("); ");
      cg_printf("!%s%d->end(); %s%d->next()",
                Option::IterPrefix, iterId,
                Option::IterPrefix, iterId);
    } else {
      TypePtr actualType = m_array->getActualType();
      if (actualType && actualType->is(Type::KindOfArray)) {
        isArray = true;
        cg_printf("ArrayIter %s%d = ", Option::IterPrefix, iterId);
      } else {
        cg_printf("ArrayIterPtr %s%d = ", Option::IterPrefix, iterId);
      }
      TypePtr expectedType = m_array->getExpectedType();
      // Clear m_expectedType to avoid type cast (toArray).
      m_array->setExpectedType(TypePtr());
      m_array->outputCPP(cg, ar);
      m_array->setExpectedType(expectedType);
      cg_printf(".begin(");
      ClassScopePtr cls = ar->getClassScope();
      if (cls) {
        cg_printf("%sclass_name", Option::StaticPropertyPrefix);
      }
      cg_printf("); ");
      if (isArray) {
        cg_printf("!%s%d.end(); ", Option::IterPrefix, iterId);
        cg_printf("++%s%d", Option::IterPrefix, iterId);
      } else {
        cg_printf("!%s%d->end(); ", Option::IterPrefix, iterId);
        cg_printf("%s%d->next()", Option::IterPrefix, iterId);
      }
    }
  }
  cg_indentBegin(") {\n");
  cg_printf("LOOP_COUNTER_CHECK(%d);\n", labelId);

  if (!m_ref) {
    cg_printf(isArray ? "%s%d.second(" : "%s%d->second(",
              Option::IterPrefix, iterId);
    m_value->outputCPP(cg, ar);
    cg_printf(");\n");
    if (m_name) {
      m_name->outputCPP(cg, ar);
      cg_printf(isArray ? " = %s%d.first();\n" : " = %s%d->first();\n",
                Option::IterPrefix, iterId);
    }
  }
  if (m_stmt) {
    m_stmt->outputCPP(cg, ar);
  }
  if (cg.findLabelId("continue", labelId)) {
    cg_printf("continue%d:;\n", labelId);
  }
  cg_indentEnd("}\n");
  if (cg.findLabelId("break", labelId)) {
    cg_printf("break%d:;\n", labelId);
  }
  cg.popBreakScope();
  cppEndBufs(cg, ar);
  cg_indentEnd("}\n");
}
