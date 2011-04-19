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

#include <compiler/analysis/file_scope.h>
#include <compiler/expression/constant_expression.h>
#include <compiler/analysis/block_scope.h>
#include <compiler/analysis/class_scope.h>
#include <compiler/analysis/constant_table.h>
#include <compiler/analysis/variable_table.h>
#include <compiler/analysis/code_error.h>
#include <util/hash.h>
#include <util/util.h>
#include <compiler/option.h>
#include <compiler/parser/parser.h>
#include <util/parser/hphp.tab.hpp>
#include <compiler/expression/scalar_expression.h>
#include <runtime/ext/ext_misc.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

ConstantExpression::ConstantExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS, const string &name)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES),
    m_name(name), m_valid(false), m_dynamic(false), m_visited(false) {
}

ExpressionPtr ConstantExpression::clone() {
  ConstantExpressionPtr exp(new ConstantExpression(*this));
  Expression::deepCopy(exp);
  return exp;
}

bool ConstantExpression::isScalar() const {
  if (m_name == "INF" || m_name == "NAN") return true;
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
  } else if (m_name == "INF") {
    value = k_INF;
  } else if (m_name == "NAN") {
    value = k_NAN;
  } else {
    value.unset();
  }
  return true;
}

unsigned ConstantExpression::getCanonHash() const {
  int64 val = hash_string(Util::toLower(m_name).c_str(), m_name.size());
  return ~unsigned(val) ^ unsigned(val >> 32);
}

bool ConstantExpression::canonCompare(ExpressionPtr e) const {
  return Expression::canonCompare(e) &&
    m_name == static_cast<ConstantExpression*>(e.get())->m_name;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void ConstantExpression::analyzeProgram(AnalysisResultPtr ar) {
  getFileScope()->addConstantDependency(ar, m_name);
  if (ar->getPhase() == AnalysisResult::AnalyzeAll &&
      !(m_context & LValue)) {
    if (!m_dynamic) {
      ConstantTablePtr constants = ar->getConstants();
      if (!constants->getValue(m_name)) {
        BlockScopePtr block = ar->findConstantDeclarer(m_name);
        if (block) {
          Symbol *sym = block->getConstants()->getSymbol(m_name);
          assert(sym);
          if (sym->isDynamic()) {
            m_dynamic = true;
          } else {
            ConstructPtr decl = sym->getDeclaration();
            if (decl) {
              if (!decl->getScope()) {
                /*
                   this only happens if a define is parsed, but a
                   later syntax error in the same file prevents
                   completeScope being called on the scope containing
                   the define.
                   Might be better to catch this in the parser...
                */
                sym->setDeclaration(ExpressionPtr());
                sym->setValue(ExpressionPtr());
              } else {
                decl->getScope()->addUse(
                  getScope(), BlockScope::UseKindConstRef);
              }
            }
          }
        }
      }
    }
  }
}

ExpressionPtr ConstantExpression::preOptimize(AnalysisResultConstPtr ar) {
  if (ar->getPhase() < AnalysisResult::FirstPreOptimize) {
    return ExpressionPtr();
  }
  ConstructPtr decl;
  while (!isScalar() && !m_dynamic && !(m_context & LValue)) {
    const Symbol *sym = ar->getConstants()->getSymbol(m_name);
    bool system = true;
    if (!sym || !sym->getValue()) {
      system = false;
      BlockScopeConstPtr block = ar->findConstantDeclarer(m_name);
      if (block) {
        sym = block->getConstants()->getSymbol(m_name);
        if (sym &&
            (!const_cast<Symbol*>(sym)->checkDefined() || sym->isDynamic())) {
          sym = 0;
          m_dynamic = true;
        }
      }
    }
    if (!sym) break;
    if (!system) BlockScope::s_constMutex.lock();
    ExpressionPtr value = dynamic_pointer_cast<Expression>(sym->getValue());
    if (!system) BlockScope::s_constMutex.unlock();

    if (!value || !value->isScalar()) break;

    if (system && !value->is(KindOfScalarExpression)) {
      if (ExpressionPtr opt = value->preOptimize(ar)) {
        value = opt;
      }
    }
    ExpressionPtr rep = Clone(value, getScope());
    bool annotate = Option::FlAnnotate;
    Option::FlAnnotate = false; // avoid nested comments on getText
    rep->setComment(getText());
    Option::FlAnnotate = annotate;
    rep->setLocation(getLocation());
    if (!system && !value->is(KindOfScalarExpression)) {
      value->getScope()->addUse(getScope(), BlockScope::UseKindConstRef);
    }
    return replaceValue(rep);
  }

  return ExpressionPtr();
}

TypePtr ConstantExpression::inferTypes(AnalysisResultPtr ar, TypePtr type,
                                       bool coerce) {
  if (m_context & LValue) return type; // ClassConstantExpression statement

  // special cases: STDIN, STDOUT, STDERR
  if (m_name == "STDIN" || m_name == "STDOUT" || m_name == "STDERR") {
    m_valid = true;
    return Type::Variant;
  }

  if (m_name == "INF" || m_name == "NAN") {
    m_valid = true;
    return Type::Double;
  }

  string lower = Util::toLower(m_name);
  TypePtr actualType;
  ConstructPtr self = shared_from_this();
  if (lower == "true" || lower == "false") {
    m_valid = true;
    actualType = Type::Boolean;
  } else if (lower == "null") {
    actualType = Type::Variant;
    m_valid = true;
  } else {
    BlockScopePtr scope = ar->findConstantDeclarer(m_name);
    if (!scope) {
      scope = getFileScope();
      getFileScope()->declareConstant(ar, m_name);
    }
    ConstantTablePtr constants = scope->getConstants();
    if (!m_valid) {
      if (ar->isSystemConstant(m_name) || constants->getValue(m_name)) {
        m_valid = true;
      }
    }
    BlockScope *defScope = NULL;
    std::vector<std::string> bases;
    actualType = constants->check(m_name, type, coerce, ar, self, bases,
                                  defScope);
    if (!m_dynamic && constants->isDynamic(m_name)) {
      m_dynamic = true;
      actualType = Type::Variant;
    }
    if (m_dynamic) {
      getScope()->getVariables()->
        setAttribute(VariableTable::NeedGlobalPointer);
    }
  }

  return actualType;
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void ConstantExpression::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg_printf("%s", m_name.c_str());
}

void ConstantExpression::outputCPPImpl(CodeGenerator &cg,
                                       AnalysisResultPtr ar) {
  // special cases: STDIN, STDOUT, STDERR, INF, and NAN
  if (m_name == "STDIN" || m_name == "STDOUT" || m_name == "STDERR" ||
      m_name == "INF" || m_name == "NAN") {
    cg_printf("%s%s", Option::ConstantPrefix, m_name.c_str());
    return;
  }

  string lower = Util::toLower(m_name);
  bool requireFwDeclaration = false;
  if (lower == "true" || lower == "false" || lower == "null") {
    cg_printf("%s", lower.c_str());
  } else if (m_valid) {
    if (m_dynamic) {
      cg_printf("getDynamicConstant(%s->%s%s, ",
                cg.getGlobals(ar), Option::ConstantPrefix,
                cg.formatLabel(m_name).c_str());
      cg_printString(m_name, ar, shared_from_this());
      cg_printf(")");
    } else {
      cg_printf("%s%s", Option::ConstantPrefix,
                cg.formatLabel(m_name).c_str());
      requireFwDeclaration = true;
    }
  } else {
    cg_printf("getUndefinedConstant(");
    cg_printString(cg.formatLabel(m_name).c_str(), ar, shared_from_this());
    cg_printf(")");
    requireFwDeclaration = true;
  }

  if (requireFwDeclaration && cg.isFileOrClassHeader()) {
    if (getClassScope()) {
      getClassScope()->addUsedConstHeader(m_name);
    } else {
      getFileScope()->addUsedConstHeader(m_name);
    }
  }
}
