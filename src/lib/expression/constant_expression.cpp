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

#include <lib/analysis/file_scope.h>
#include <lib/expression/constant_expression.h>
#include <lib/analysis/block_scope.h>
#include <lib/analysis/class_scope.h>
#include <lib/analysis/constant_table.h>
#include <lib/analysis/variable_table.h>
#include <lib/analysis/dependency_graph.h>
#include <lib/analysis/code_error.h>
#include <util/util.h>
#include <lib/option.h>
#include <lib/parser/parser.h>
#include <lib/expression/scalar_expression.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////

// constructors/destructors

ConstantExpression::ConstantExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS, const string &name)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES),
    m_name(name), m_valid(true), m_dynamic(false), m_visited(false) {
}

ExpressionPtr ConstantExpression::clone() {
  ConstantExpressionPtr exp(new ConstantExpression(*this));
  Expression::deepCopy(exp);
  return exp;
}

bool ConstantExpression::isScalar() const {
  string lower = Util::toLower(m_name);
  return lower == "true" || lower == "false" || lower == "null";
}

bool ConstantExpression::isBoolean() const {
  string lower = Util::toLower(m_name);
  return (lower == "true" || lower == "false");
}

bool ConstantExpression::isNull() const {
  string lower = Util::toLower(m_name);
  return (lower == "null");
}

bool ConstantExpression::getBooleanValue() const {
  string lower = Util::toLower(m_name);
  ASSERT(lower == "true" || lower == "false");
  return lower == "true";
}

bool ConstantExpression::getScalarValue(Variant &value) {
  if (!isScalar()) return false;
  if (isBoolean()) {
    value = getBooleanValue();
  } else {
    value.unset();
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void ConstantExpression::analyzeProgram(AnalysisResultPtr ar) {
  if (ar->getPhase() == AnalysisResult::AnalyzeAll &&
      !(m_context & LValue)) {
    ar->getFileScope()->addConstantDependency(ar, m_name);
    if (!m_dynamic) {
      ConstantTablePtr constants = ar->getConstants();
      if (!constants->getValue(m_name)) {
        BlockScopePtr block = ar->findConstantDeclarer(m_name);
        if (block) {
          constants = block->getConstants();
          if (constants->isDynamic(m_name)) m_dynamic = true;
        }
      }
    }
  }
}

ExpressionPtr ConstantExpression::preOptimize(AnalysisResultPtr ar) {
  if (ar->getPhase() < AnalysisResult::FirstPreOptimize) {
    return ExpressionPtr();
  }
  ConstructPtr decl;
  if (!isScalar() && !m_dynamic && !(m_context & LValue)) {
    ConstantTablePtr constants = ar->getConstants();
    decl = constants->getValue(m_name);
    if (!decl) {
      BlockScopePtr block = ar->findConstantDeclarer(m_name);
      if (block) {
        constants = block->getConstants();
        if (!constants->isDynamic(m_name)) decl = constants->getValue(m_name);
      }
    }
    if (decl) {
      ExpressionPtr value = dynamic_pointer_cast<Expression>(decl);
      if (!m_visited) {
        m_visited = true;
        ExpressionPtr optExp = value->preOptimize(ar);
        m_visited = false;
        if (optExp) value = optExp;
      }
      if (value->isScalar()) {
        // inline the value
        if (value->is(Expression::KindOfScalarExpression)) {
          ScalarExpressionPtr exp =
            dynamic_pointer_cast<ScalarExpression>(Clone(value));
          exp->setComment(getText());
          return exp;
        } else if (value->is(Expression::KindOfConstantExpression)) {
          // inline the value
          ConstantExpressionPtr exp =
            dynamic_pointer_cast<ConstantExpression>(Clone(value));
          exp->setComment(getText());
          return exp;
        }
      }
    }
  }
  return ExpressionPtr();
}

ExpressionPtr ConstantExpression::postOptimize(AnalysisResultPtr ar) {
  return ExpressionPtr();
}

TypePtr ConstantExpression::inferTypes(AnalysisResultPtr ar, TypePtr type,
                                       bool coerce) {
  if (m_context & LValue) return type; // ClassConstantExpression statement

  // special cases: STDIN, STDOUT, STDERR
  if (m_name == "STDIN" || m_name == "STDOUT" || m_name == "STDERR") {
    return Type::Variant;
  }

  string lower = Util::toLower(m_name);
  TypePtr actualType;
  ConstructPtr self = shared_from_this();
  if (lower == "true" || lower == "false") {
    actualType = Type::Boolean;
  } else if (lower == "null") {
    actualType = Type::Variant;
  } else {
    BlockScopePtr scope = ar->findConstantDeclarer(m_name);
    if (!scope) {
      scope = ar->getFileScope();
      ar->getFileScope()->declareConstant(ar, m_name);
    }
    ConstantTablePtr constants = scope->getConstants();
    ConstructPtr decl = constants->getDeclaration(m_name);
    if (decl) {
      ar->getDependencyGraph()->add(DependencyGraph::KindOfConstant, "",
                                    m_name, self, m_name, decl);
    }
    bool p;
    actualType = constants->check(m_name, type, coerce, ar, self, p);
    if (!m_dynamic && constants->isDynamic(m_name)) {
      m_dynamic = true;
      actualType = Type::Variant;
    }
  }

  return actualType;
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void ConstantExpression::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg.printf("%s", m_name.c_str());
}

void ConstantExpression::outputCPPImpl(CodeGenerator &cg,
                                       AnalysisResultPtr ar) {
  // special cases: STDIN, STDOUT, STDERR
  if (m_name == "STDIN" || m_name == "STDOUT" || m_name == "STDERR") {
    cg.printf("%s", m_name.c_str());
    return;
  }

  string lower = Util::toLower(m_name);
  if (lower == "true" || lower == "false" || lower == "null") {
    cg.printf("%s", lower.c_str());
  } else if (m_valid) {
    if (m_dynamic) {
      cg.printf("%s->%s%s", cg.getGlobals(), Option::ConstantPrefix,
                m_name.c_str());
    } else {
      cg.printf("%s%s", Option::ConstantPrefix, m_name.c_str());
    }
  } else {
    cg.printf("\"%s\"", m_name.c_str());
  }
}
