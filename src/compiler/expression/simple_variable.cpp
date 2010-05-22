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

#include <compiler/expression/simple_variable.h>
#include <compiler/analysis/function_scope.h>
#include <compiler/analysis/variable_table.h>
#include <compiler/analysis/class_scope.h>
#include <compiler/option.h>
#include <compiler/builtin_symbols.h>
#include <compiler/expression/scalar_expression.h>
#include <compiler/parser/hphp.tab.hpp>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

SimpleVariable::SimpleVariable
(EXPRESSION_CONSTRUCTOR_PARAMETERS, const std::string &name)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES),
    m_name(name), m_this(false), m_globals(false), m_superGlobal(false) {
  setContext(Expression::NoLValueWrapper);
}

ExpressionPtr SimpleVariable::clone() {
  SimpleVariablePtr exp(new SimpleVariable(*this));
  Expression::deepCopy(exp);
  return exp;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void SimpleVariable::analyzeProgram(AnalysisResultPtr ar) {
  if (m_name == "argc" || m_name == "argv") {
    // special case: they are NOT superglobals when not in global scope
    if (ar->getScope() == ar) {
      m_superGlobal = BuiltinSymbols::IsSuperGlobal(m_name);
      m_superGlobalType = BuiltinSymbols::GetSuperGlobalType(m_name);
    }
  } else {
    m_superGlobal = BuiltinSymbols::IsSuperGlobal(m_name);
    m_superGlobalType = BuiltinSymbols::GetSuperGlobalType(m_name);
  }

  if (m_superGlobal) {
    ar->getScope()->getVariables()->
      setAttribute(VariableTable::NeedGlobalPointer);
  }

  if (m_name == "this" && ar->getClassScope()) {
    FunctionScopePtr func =
      dynamic_pointer_cast<FunctionScope>(ar->getScope());
    func->setContainsThis();
    if (!func->isStatic() || (m_context & ObjectContext)) {
      m_this = true;
    }
  } else if (m_name == "GLOBALS") {
    m_globals = true;
  }
  if (!(m_context & AssignmentLHS)) {
    BlockScopePtr scope = ar->getScope();
    FunctionScopePtr func = dynamic_pointer_cast<FunctionScope>(scope);
    if (func) {
      func->getVariables()->addUsed(m_name);
    }
  }
}

bool SimpleVariable::canonCompare(ExpressionPtr e) const {
  return Expression::canonCompare(e) &&
    getName() == static_cast<SimpleVariable*>(e.get())->getName();
}

ExpressionPtr SimpleVariable::preOptimize(AnalysisResultPtr ar) {
  return ExpressionPtr();
}

ExpressionPtr SimpleVariable::postOptimize(AnalysisResultPtr ar) {
  return ExpressionPtr();
}

TypePtr SimpleVariable::inferTypes(AnalysisResultPtr ar, TypePtr type,
                                   bool coerce) {
  ASSERT(false);
  return TypePtr();
}

TypePtr SimpleVariable::inferAndCheck(AnalysisResultPtr ar, TypePtr type,
                                      bool coerce) {
  TypePtr ret;
  ConstructPtr construct = shared_from_this();
  BlockScopePtr scope = ar->getScope();
  VariableTablePtr variables = scope->getVariables();

  // check function parameter that can occur in lval context
  if (m_context & (LValue | RefValue)) {
    FunctionScopePtr func = dynamic_pointer_cast<FunctionScope>(scope);
    if (func) {
      if (variables->isParameter(m_name)) {
        variables->addLvalParam(m_name);
      }
    }
  }
  if (m_name == "this") {
    ClassScopePtr cls = ar->getClassScope();
    if (cls) {
      bool isStaticFunc = false;
      FunctionScopePtr func = dynamic_pointer_cast<FunctionScope>(scope);
      if (func->isStatic()) isStaticFunc = true;
      if (cls->isRedeclaring()) {
        ret = Type::Variant;
      } else {
        ret = Type::CreateObjectType(cls->getName());
      }
      if (!isStaticFunc || (m_context & ObjectContext)) m_this = true;
    }
  }
  if (m_context & (LValue|Declaration)) {
    if (m_superGlobal) {
      ret = m_superGlobalType;
    } else if (m_superGlobalType) { // For system
      if (!m_this) {
        ret = variables->add(m_name, m_superGlobalType,
                             ((m_context & Declaration) != Declaration), ar,
                             construct, scope->getModifiers());
      }
    } else {
      if (m_globals) {
        ret = Type::Variant; // this can happen with "unset($GLOBALS)"
      } else if (!m_this) {
        ret = variables->add(m_name, type,
                             ((m_context & Declaration) != Declaration), ar,
                             construct, scope->getModifiers());
      }
    }
  } else {
    if (!m_this) {
      if (m_superGlobalType) {
        ret = m_superGlobalType;
      } else if (m_globals) {
        ret = Type::Array;
      } else if (scope->is(BlockScope::ClassScope)) {
        // ClassVariable expression will come to this block of code
        int properties;
        ret = variables->checkProperty(m_name, type, true, ar, construct,
                                       properties);
      } else {
        TypePtr tmpType = type;
        if (m_context & RefValue) {
          tmpType = Type::Variant;
          coerce = true;
        }
        int p;
        ret = variables->checkVariable(m_name, tmpType, coerce, ar, construct,
                                       p);
      }
    }
  }

  TypePtr actual = propagateTypes(ar, ret);
  setTypes(actual, type);
  if (Type::SameType(actual, ret)) {
    m_implementedType.reset();
  } else {
    m_implementedType = ret;
  }
  return actual;
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void SimpleVariable::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg.printf("$%s", m_name.c_str());
}

void SimpleVariable::preOutputStash(CodeGenerator &cg, AnalysisResultPtr ar,
                                    int state)
{
  if (getContext() & (LValue|RefValue|RefParameter)) return;
  if (!(state & StashVars)) return;
  Expression::preOutputStash(cg, ar, state);
}

void SimpleVariable::outputCPPImpl(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (m_this) {
    ClassScopePtr cls = ar->getClassScope();
    if (cls->isRedeclaring() || cls->derivesFromRedeclaring() !=
        ClassScope::FromNormal) {
      cg.printf("root");
    } else {
      cg.printf("GET_THIS()");
    }
  } else if (m_superGlobal) {
    VariableTablePtr variables = ar->getScope()->getVariables();
    cg.printf("g->%s", variables->getGlobalVariableName(ar, m_name).c_str());
  } else if (m_globals) {
    cg.printf("get_global_array_wrapper()");
  } else {
    const char *prefix =
      ar->getScope()->getVariables()->getVariablePrefix(ar, m_name);
    cg.printf("%s%s", prefix, m_name.c_str());
  }
}
