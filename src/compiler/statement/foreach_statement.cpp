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

#include <compiler/statement/foreach_statement.h>
#include <compiler/expression/assignment_expression.h>
#include <compiler/analysis/analysis_result.h>
#include <compiler/analysis/block_scope.h>
#include <compiler/expression/simple_variable.h>
#include <compiler/option.h>
#include <compiler/analysis/code_error.h>
#include <compiler/analysis/class_scope.h>
#include <util/util.h>

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
    case ArrayExpr:
      return m_array;
    case NameExpr:
      return m_name;
    case ValueExpr:
      return m_value;
    case BodyStmt:
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
    case ArrayExpr:
      m_array = boost::dynamic_pointer_cast<Expression>(cp);
      break;
    case NameExpr:
      m_name = boost::dynamic_pointer_cast<Expression>(cp);
      break;
    case ValueExpr:
      m_value = boost::dynamic_pointer_cast<Expression>(cp);
      break;
    case BodyStmt:
      m_stmt = boost::dynamic_pointer_cast<Statement>(cp);
      break;
    default:
      ASSERT(false);
      break;
  }
}

void ForEachStatement::inferTypes(AnalysisResultPtr ar) {
  m_array->inferAndCheck(ar, m_ref ? Type::Variant : Type::Array, m_ref);
  if (m_name) {
    m_name->inferAndCheck(ar, Type::Primitive, true);
  }
  m_value->inferAndCheck(ar, Type::Variant, true);
  if (m_ref) {
    TypePtr actualType = m_array->getActualType();
    if (!actualType ||
        actualType->is(Type::KindOfVariant) ||
        actualType->is(Type::KindOfObject)) {
      ar->forceClassVariants(getClassScope(), false);
    }
  }
  if (m_stmt) {
    getScope()->incLoopNestedLevel();
    m_stmt->inferTypes(ar);
    getScope()->decLoopNestedLevel();
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
  int labelId = cg.createNewLocalId(shared_from_this());
  cg.pushBreakScope(labelId);

  int mapId = cg.createNewLocalId(shared_from_this());
  bool passTemp = true;
  bool nameSimple = !m_name || m_name->is(Expression::KindOfSimpleVariable);
  bool valueSimple = m_value->is(Expression::KindOfSimpleVariable);

  if (m_ref ||
      !m_array->is(Expression::KindOfSimpleVariable) ||
      m_array->isThis()) {

    if (m_ref) {
      if (!nameSimple) {
        cg_printf("Variant %s%d_n;\n", Option::MapPrefix, mapId);
      }
      if (!valueSimple) {
        cg_printf("Variant %s%d_v;\n", Option::MapPrefix, mapId);
      }
    }
    cg_printf("Variant %s%d", Option::MapPrefix, mapId);
    TypePtr expectedType = m_array->getExpectedType();
    // Clear m_expectedType to avoid type cast (toArray).
    m_array->setExpectedType(TypePtr());
    bool wrap = m_array->preOutputCPP(cg, ar, 0);
    if (wrap) {
      cg_printf(";\n");
      m_array->outputCPPBegin(cg, ar);
      cg_printf("%s%d", Option::MapPrefix, mapId);
    }
    cg_printf(" = ");
    m_array->outputCPP(cg, ar);
    cg.printf(";\n");
    if (m_ref) {
      cg.printf("%s%d.escalate(true);\n", Option::MapPrefix, mapId);
    }
    m_array->setExpectedType(expectedType);
    if (wrap) {
      m_array->outputCPPEnd(cg, ar);
    }
  } else {
    passTemp = false;
  }

  /* We need to generate sets for $a[x] etc
     Without AssignmentLHS context, we get lvalAt(...) = instead.
     But if we set AssignmentLHS earlier, a simple variable can end up
     being marked "unused"
  */
  if (m_name) m_name->setContext(Expression::AssignmentLHS);
  m_value->setContext(Expression::AssignmentLHS);

  bool orderName = m_name && m_name->preOutputCPP(cg, ar, 0);
  bool orderValue = m_value->preOutputCPP(cg, ar, 0);

  cppDeclareBufs(cg, ar);
  int iterId = cg.createNewLocalId(shared_from_this());
  cg_printf("for (");
  if (m_ref) {
    cg_printf("MutableArrayIter %s%d = %s%d.begin(",
              Option::IterPrefix, iterId, Option::MapPrefix, mapId);
    if (!nameSimple) {
      cg_printf("&%s%d_n", Option::MapPrefix, mapId);
    } else if (m_name) {
      cg_printf("&");
      m_name->outputCPP(cg, ar);
    } else {
      cg_printf("NULL");
    }
    cg_printf(", ");
    if (!valueSimple) {
      cg_printf("%s%d_v", Option::MapPrefix, mapId);
    } else {
      m_value->outputCPP(cg, ar);
    }
    ClassScopePtr cls = getClassScope();
    if (cls) {
      cg_printf(", %sclass_name", Option::StaticPropertyPrefix);
    } else {
      cg_printf(", null_string");
    }
    cg_printf(", true); %s%d.advance();", Option::IterPrefix, iterId);
  } else {
    if (passTemp) {
      cg_printf("ArrayIter %s%d = %s%d.begin(",
                Option::IterPrefix, iterId,
                Option::MapPrefix, mapId);
      ClassScopePtr cls = getClassScope();
      if (cls) {
        cg_printf("%sclass_name", Option::StaticPropertyPrefix);
      } else {
        cg_printf("null_string");
      }
      cg_printf(", true); ");
      cg_printf("!%s%d.end(); %s%d.next()",
                Option::IterPrefix, iterId,
                Option::IterPrefix, iterId);
    } else {
      cg_printf("ArrayIter %s%d = ", Option::IterPrefix, iterId);
      TypePtr expectedType = m_array->getExpectedType();
      // Clear m_expectedType to avoid type cast (toArray).
      m_array->setExpectedType(TypePtr());
      m_array->outputCPP(cg, ar);
      m_array->setExpectedType(expectedType);
      cg_printf(".begin(");
      ClassScopePtr cls = getClassScope();
      if (cls) {
        cg_printf("%sclass_name", Option::StaticPropertyPrefix);
      } else {
        cg_printf("null_string");
      }
      cg_printf(", true); ");
      cg_printf("!%s%d.end(); ", Option::IterPrefix, iterId);
      cg_printf("++%s%d", Option::IterPrefix, iterId);
    }
  }
  cg_indentBegin(") {\n");
  cg_printf("LOOP_COUNTER_CHECK(%d);\n", labelId);

  /*
    The order of evaluation here is.
    - second() (if !m_ref)
    - first() (if !m_ref && m_name is set)
    - side effects of m_name (if set)
    - side effects of m_value
    - assignment to m_value
    - assignment to m_name
    This order is observable, and we have tests to prove it.

    Optimize for the usual case, where m_name and m_value are
    simple variables.
  */

  string valueStr, nameStr;

  if (!m_ref) {
    Util::string_printf(valueStr, "%s%d.second()",
                        Option::IterPrefix, iterId);
    Util::string_printf(nameStr, "%s%d.first()",
                        Option::IterPrefix, iterId);
    if (valueSimple) {
      cg_printf("%s%d.second(", Option::IterPrefix, iterId);
      m_value->outputCPP(cg, ar);
      cg_printf(");\n");
    } else if (m_name || m_value->hasEffect()) {
      string tmp;
      Util::string_printf(tmp, "%s%d_v", Option::MapPrefix, mapId);
      cg_printf("CVarRef %s = %s;\n",
                tmp.c_str(), valueStr.c_str());
      valueStr = tmp;
    }
    if (m_name && m_name->hasEffect()) {
      string tmp;
      Util::string_printf(tmp, "%s%d_n", Option::MapPrefix, mapId);
      cg_printf("CVarRef %s = %s;\n",
                tmp.c_str(), nameStr.c_str());
      nameStr = tmp;
    }
  } else {
    Util::string_printf(valueStr, "%s%d_v", Option::MapPrefix, mapId);
    Util::string_printf(nameStr, "%s%d_n", Option::MapPrefix, mapId);
  }

  bool wrap = false;
  if (m_name) {
    if (orderName || m_value->hasEffect()) {
      cg.setInExpression(true);
      m_name->preOutputCPP(cg, ar,
                           m_value->hasEffect() ? Expression::FixOrder: 0);
      wrap = true;
    }
  }

  if (!valueSimple) {
    if (orderValue) {
      m_value->outputCPPBegin(cg, ar);
      wrap = true;
    }
    if (!AssignmentExpression::SpecialAssignment(
          cg, ar, m_value, ExpressionPtr(), valueStr.c_str(), m_ref)) {
      m_value->outputCPP(cg, ar);
      cg_printf(".assign%s(%s)", m_ref ? "Ref" : "Val", valueStr.c_str());
    }
    cg_printf(";\n");
  }

  if (m_name && (!nameSimple || !m_ref)) {
    if (orderName) {
      m_name->outputCPPBegin(cg, ar);
    }
    if (!AssignmentExpression::SpecialAssignment(
          cg, ar, m_name, ExpressionPtr(), nameStr.c_str(), m_ref)) {
      m_name->outputCPP(cg, ar);
      cg_printf(".assign%s(%s)", m_ref ? "Ref" : "Val", nameStr.c_str());
    }
    cg_printf(";\n");
  }
  if (wrap) {
    cg.wrapExpressionEnd();
    cg.setInExpression(false);
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
