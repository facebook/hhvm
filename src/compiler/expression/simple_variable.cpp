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
#include <util/parser/hphp.tab.hpp>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

SimpleVariable::SimpleVariable
(EXPRESSION_CONSTRUCTOR_PARAMETERS, const std::string &name)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES),
    m_name(name), m_sym(NULL), m_originalSym(NULL),
    m_this(false), m_globals(false),
    m_superGlobal(false), m_alwaysStash(false),
    m_guardedThis(false) {
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

int SimpleVariable::getLocalEffects() const {
  if (m_context == Declaration &&
      m_sym && m_sym->isShrinkWrapped()) {
    return LocalEffect;
  }
  return NoEffect;
}

void SimpleVariable::updateSymbol(SimpleVariablePtr src) {
  m_sym = getScope()->getVariables()->addSymbol(m_name);
  if (src && src->m_sym) {
    m_sym->update(src->m_sym);
  }
}

bool SimpleVariable::couldBeAliased() const {
  if (m_globals || m_superGlobal) return true;
  assert(m_sym);
  return m_sym->isReferenced() || m_sym->isGlobal() || m_sym->isStatic();
}

void SimpleVariable::coalesce(SimpleVariablePtr other) {
  assert(m_sym);
  assert(other->m_sym);
  if (!m_originalSym) m_originalSym = m_sym;
  m_sym->clearUsed();
  m_sym->clearNeeded();
  m_sym = other->m_sym;
  m_name = m_sym->getName();
}

/*
  This simple variable is about to go out of scope.
  Is it ok to kill the last assignment?
  What if its a reference assignment (or an unset)?
*/
bool SimpleVariable::canKill(bool isref) const {
  if (m_globals || m_superGlobal) return false;
  assert(m_sym);
  if (m_sym->isGlobal() || m_sym->isStatic()) {
    return isref && !getScope()->inPseudoMain();
  }

  return isref || !m_sym->isReferenced();
}

void SimpleVariable::analyzeProgram(AnalysisResultPtr ar) {
  m_superGlobal = BuiltinSymbols::IsSuperGlobal(m_name);
  m_superGlobalType = BuiltinSymbols::GetSuperGlobalType(m_name);

  VariableTablePtr variables = getScope()->getVariables();
  if (m_superGlobal) {
    variables->setAttribute(VariableTable::NeedGlobalPointer);
  } else if (m_name == "GLOBALS") {
    m_globals = true;
  } else {
    m_sym = variables->addSymbol(m_name);
  }

  if (ar->getPhase() == AnalysisResult::AnalyzeAll) {
    if (FunctionScopePtr func = getFunctionScope()) {
      if (m_name == "this" && getClassScope()) {
        func->setContainsThis();
        m_this = true;
        if (!hasContext(ObjectContext)) {
          func->setContainsBareThis();
          if (variables->getAttribute(VariableTable::ContainsDynamicVariable)) {
            ClassScopePtr cls = getClassScope();
            TypePtr t = cls->isRedeclaring() ?
              Type::Variant : Type::CreateObjectType(cls->getName());
            variables->add(m_sym, t, true, ar, shared_from_this(),
                           getScope()->getModifiers());
          }
        }
      }
      if (m_sym && !(m_context & AssignmentLHS) &&
          !((m_context & UnsetContext) && (m_context & LValue))) {
        m_sym->setUsed();
      }
    }
  } else if (ar->getPhase() == AnalysisResult::AnalyzeFinal) {
    if (m_sym) {
      if (!m_sym->isSystem() &&
          !(getContext() &
            (LValue|RefValue|RefParameter|UnsetContext|ExistContext)) &&
          m_sym->getDeclaration().get() == this &&
          !variables->getAttribute(VariableTable::ContainsLDynamicVariable) &&
          !getScope()->is(BlockScope::ClassScope)) {
        if (getScope()->inPseudoMain()) {
          Compiler::Error(Compiler::UseUndeclaredGlobalVariable,
                          shared_from_this());
        } else {
          Compiler::Error(Compiler::UseUndeclaredVariable, shared_from_this());
        }
      }
      // check function parameter that can occur in lval context
      if (m_sym->isParameter() &&
          m_context & (LValue | RefValue | DeepReference |
                       UnsetContext | InvokeArgument | OprLValue |
                       DeepOprLValue)) {
        m_sym->setLvalParam();
      }
    }
  }
}

bool SimpleVariable::canonCompare(ExpressionPtr e) const {
  return Expression::canonCompare(e) &&
    getName() == static_cast<SimpleVariable*>(e.get())->getName();
}

TypePtr SimpleVariable::inferTypes(AnalysisResultPtr ar, TypePtr type,
                                   bool coerce) {
  ASSERT(false);
  return TypePtr();
}

bool SimpleVariable::checkUnused() const {
  return !m_superGlobal && !m_globals &&
    getScope()->getVariables()->checkUnused(m_sym);
}

TypePtr SimpleVariable::inferAndCheck(AnalysisResultPtr ar, TypePtr type,
                                      bool coerce) {
  TypePtr ret;
  ConstructPtr construct = shared_from_this();
  BlockScopePtr scope = getScope();
  VariableTablePtr variables = scope->getVariables();

  // check function parameter that can occur in lval context
  if (m_sym && m_sym->isParameter() &&
      m_context & (LValue | RefValue | DeepReference |
                   UnsetContext | InvokeArgument | OprLValue | DeepOprLValue)) {
    m_sym->setLvalParam();
  }
  if (m_this) {
    ClassScopePtr cls = getOriginalClass();
    if (!hasContext(ObjectContext) && cls->derivedByDynamic()) {
      ret = Type::Object;
    } else {
      ret = Type::CreateObjectType(cls->getName());
    }
    if (!hasContext(ObjectContext) &&
        variables->getAttribute(VariableTable::ContainsDynamicVariable)) {
      ret = variables->add(m_sym, ret, true, ar,
                           construct, scope->getModifiers());
    }
  } else if ((m_context & (LValue|Declaration)) &&
             !(m_context & (ObjectContext|RefValue))) {
    if (m_globals) {
      ret = Type::Variant;
    } else if (m_superGlobal) {
      ret = m_superGlobalType;
    } else if (m_superGlobalType) { // For system
      ret = variables->add(m_sym, m_superGlobalType,
                           ((m_context & Declaration) != Declaration), ar,
                           construct, scope->getModifiers());
    } else {
      ret = variables->add(m_sym, type,
                           ((m_context & Declaration) != Declaration), ar,
                           construct, scope->getModifiers());
    }
  } else {
    if (m_superGlobalType) {
      ret = m_superGlobalType;
    } else if (m_globals) {
      ret = Type::Array;
    } else if (scope->is(BlockScope::ClassScope)) {
      // ClassVariable expression will come to this block of code
      ret = getClassScope()->checkProperty(m_sym, type, true, ar);
    } else {
      TypePtr tmpType = type;
      if (m_context & RefValue) {
        tmpType = Type::Variant;
        coerce = true;
      }
      int p;
      ret = variables->checkVariable(m_sym, tmpType, coerce,
                                     ar, construct, p);
    }
  }

  TypePtr actual = propagateTypes(ar, ret);
  setTypes(ar, actual, type);
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
  cg_printf("$%s", m_name.c_str());
}

void SimpleVariable::preOutputStash(CodeGenerator &cg, AnalysisResultPtr ar,
                                    int state)
{
  if (getContext() & (LValue|RefValue|RefParameter)) return;
  if (!m_alwaysStash && !(state & StashVars)) return;
  Expression::preOutputStash(cg, ar, state);
}

void SimpleVariable::outputCPPImpl(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (m_this) {
    ASSERT((getContext() & ObjectContext) == 0);
    if (hasContext(OprLValue) || hasContext(AssignmentLHS)) {
      cg_printf("throw_assign_this()");
    } else if (hasContext(DeepOprLValue) ||
               hasContext(DeepAssignmentLHS) ||
               hasContext(LValue)) {
      // $this[] op= ...; or $this[] = ...
      cg_printf("Variant(GET_THIS())");
    } else {
      ClassScopePtr cls = getOriginalClass();
      if (cls->derivedByDynamic()) {
        cg_printf("Object(GET_THIS())");
      } else {
        cg_printf("GET_THIS_TYPED(%s)", cls->getId(cg).c_str());
      }
    }
  } else if (m_superGlobal) {
    VariableTablePtr variables = getScope()->getVariables();
    string name = variables->getGlobalVariableName(cg, ar, m_name);
    cg_printf("g->%s", name.c_str());
  } else if (m_globals) {
    cg_printf("get_global_array_wrapper()");
  } else {
    bool sw = false;
    if (m_sym->isShrinkWrapped() &&
        m_context == Declaration) {
      TypePtr type = m_sym->getFinalType();
      type->outputCPPDecl(cg, ar, getScope());
      sw = true;
      cg_printf(" ");
    }
    const char *prefix =
      getScope()->getVariables()->getVariablePrefix(m_sym);
    cg_printf("%s%s", prefix, cg.formatLabel(m_name).c_str());
    if (m_originalSym) {
      cg.printf(" /* %s */", m_originalSym->getName().c_str());
    }
    if (sw) {
      TypePtr type = m_sym->getFinalType();
      const char *initializer = type->getCPPInitializer();
      if (initializer) {
        cg_printf(" = %s", initializer);
      }
    }
  }
}
