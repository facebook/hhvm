/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/compiler/analysis/constant_table.h"
#include "hphp/compiler/analysis/analysis_result.h"
#include "hphp/compiler/analysis/code_error.h"
#include "hphp/compiler/analysis/type.h"
#include "hphp/compiler/code_generator.h"
#include "hphp/compiler/expression/expression.h"
#include "hphp/compiler/expression/scalar_expression.h"
#include "hphp/compiler/option.h"
#include "hphp/util/util.h"
#include "hphp/util/hash.h"
#include "hphp/compiler/analysis/class_scope.h"
#include "hphp/runtime/base/complex-types.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////

ConstantTable::ConstantTable(BlockScope &blockScope)
    : SymbolTable(blockScope, true),
      m_hasDynamic(false) {
}

///////////////////////////////////////////////////////////////////////////////

TypePtr ConstantTable::add(const std::string &name, TypePtr type,
                           ExpressionPtr exp, AnalysisResultConstPtr ar,
                           ConstructPtr construct) {

  if (name == "true" || name == "false") {
    return Type::Boolean;
  }

  Symbol *sym = genSymbol(name, true);
  if (!sym->declarationSet()) {
    assert(!sym->valueSet());
    setType(ar, sym, type, true);
    sym->setDeclaration(construct);
    sym->setValue(exp);
    return type;
  }
  assert(sym->declarationSet() && sym->valueSet());

  if (m_blockScope.isFirstPass()) {
    if (construct) {
      if (exp != sym->getValue()) {
        Compiler::Error(Compiler::DeclaredConstantTwice, construct,
                        sym->getDeclaration());
        if (!sym->isDynamic()) {
          sym->setDynamic();
          m_hasDynamic = true;
        }
        type = Type::Variant;
      }
    } else if (exp) {
      sym->setValue(exp);
    }
    setType(ar, sym, type, true);
  }

  return type;
}

void ConstantTable::setDynamic(AnalysisResultConstPtr ar,
                               const std::string &name, bool forceVariant) {
  Symbol *sym = genSymbol(name, true);
  if (!sym->isDynamic()) {
    Lock lock(BlockScope::s_constMutex);
    sym->setDynamic();
    if (sym->getDeclaration()) {
      sym->getDeclaration()->getScope()->
        addUpdates(BlockScope::UseKindConstRef);
    }
    m_hasDynamic = true;
    if (forceVariant) {
      setType(ar, sym, Type::Variant, true);
    }
  }
}

void ConstantTable::setValue(AnalysisResultConstPtr ar, const std::string &name,
                             ExpressionPtr value) {
  Symbol *sym = getSymbol(name);
  assert(sym && sym->isPresent());
  sym->setValue(value);
}

bool ConstantTable::isRecursivelyDeclared(AnalysisResultConstPtr ar,
                                          const std::string &name) const {
  if (const Symbol *sym ATTRIBUTE_UNUSED = getSymbol(name)) {
    assert(sym->isPresent() && sym->valueSet());
    return true;
  }
  ClassScopePtr parent = findParent(ar, name);
  if (parent) {
    return parent->getConstants()->isRecursivelyDeclared(ar, name);
  }
  return false;
}

ConstructPtr ConstantTable::getValueRecur(AnalysisResultConstPtr ar,
                                          const std::string &name,
                                          ClassScopePtr &defClass) const {
  if (const Symbol *sym = getSymbol(name)) {
    assert(sym->isPresent() && sym->valueSet());
    if (sym->getValue()) return sym->getValue();
  }
  ClassScopePtr parent = findParent(ar, name);
  if (parent) {
    defClass = parent;
    return parent->getConstants()->getValueRecur(ar, name, defClass);
  }
  return ConstructPtr();
}

ConstructPtr ConstantTable::getDeclarationRecur(AnalysisResultConstPtr ar,
                                                const std::string &name,
                                                ClassScopePtr &defClass)
const {
  if (const Symbol *sym = getSymbol(name)) {
    assert(sym->isPresent() && sym->valueSet());
    if (sym->getDeclaration()) return sym->getDeclaration();
  }
  ClassScopePtr parent = findParent(ar, name);
  if (parent) {
    defClass = parent;
    return parent->getConstants()->getDeclarationRecur(ar, name, defClass);
  }
  return ConstructPtr();
}

void ConstantTable::cleanupForError(AnalysisResultConstPtr ar) {
  AnalysisResult::Locker lock(ar);

  BOOST_FOREACH(Symbol *sym, m_symbolVec) {
    if (!sym->isDynamic()) {
      sym->setDynamic();
      sym->setDeclaration(ConstructPtr());
      sym->setValue(ConstructPtr());
    }
  }
}

TypePtr ConstantTable::check(BlockScopeRawPtr context,
                             const std::string &name, TypePtr type,
                             bool coerce, AnalysisResultConstPtr ar,
                             ConstructPtr construct,
                             const std::vector<std::string> &bases,
                             BlockScope *&defScope) {
  assert(!m_blockScope.is(BlockScope::FunctionScope));
  bool isClassScope = m_blockScope.is(BlockScope::ClassScope);
  TypePtr actualType;
  defScope = nullptr;
  if (name == "true" || name == "false") {
    actualType = Type::Boolean;
  } else {
    Symbol *sym = getSymbol(name);
    if (!sym) {
      if (ar->getPhase() >= AnalysisResult::AnalyzeAll) {
        if (isClassScope) {
          ClassScopeRawPtr parent = findBase(ar, name, bases);
          if (parent) {
            actualType = parent->getConstants()->check(
              context, name, type, coerce, ar, construct, bases, defScope);
            if (defScope) return actualType;
          }
        }
        if (!isClassScope || !((ClassScope*)&m_blockScope)->isTrait()) {
          Compiler::Error(Compiler::UseUndeclaredConstant, construct);
        }
        actualType = isClassScope || !Option::WholeProgram ?
          Type::Variant : Type::String;
      }
    } else {
      assert(sym->isPresent());
      assert(sym->getType());
      assert(sym->isConstant());
      defScope = &m_blockScope;
      if (isClassScope) {
        // if the current scope is a function scope, grab the lock.
        // otherwise if it's a class scope, then *try* to grab the lock.
        if (context->is(BlockScope::FunctionScope)) {
          GET_LOCK(BlockScopeRawPtr(&m_blockScope));
          return setType(ar, sym, type, coerce);
        } else {
          TRY_LOCK(BlockScopeRawPtr(&m_blockScope));
          return setType(ar, sym, type, coerce);
        }
      } else {
        Lock lock(m_blockScope.getMutex());
        return setType(ar, sym, type, coerce);
      }
    }
  }

  return actualType;
}

ClassScopePtr ConstantTable::findParent(AnalysisResultConstPtr ar,
                                        const std::string &name) const {
  for (ClassScopePtr parent = m_blockScope.getParentScope(ar);
       parent && !parent->isRedeclaring();
       parent = parent->getParentScope(ar)) {
    if (parent->hasConst(name)) {
      return parent;
    }
  }
  return ClassScopePtr();
}

ClassScopeRawPtr ConstantTable::findBase(
  AnalysisResultConstPtr ar, const std::string &name,
  const std::vector<std::string> &bases) const {
  for (int i = bases.size(); i--; ) {
    ClassScopeRawPtr p = ar->findClass(bases[i]);
    if (!p || p->isRedeclaring()) continue;
    if (p->hasConst(name)) return p;
    ConstantTablePtr constants = p->getConstants();
    p = constants->findBase(ar, name, p->getBases());
    if (p) return p;
  }
  return ClassScopeRawPtr();
}

///////////////////////////////////////////////////////////////////////////////

void ConstantTable::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (Option::GenerateInferredTypes) {
    for (unsigned int i = 0; i < m_symbolVec.size(); i++) {
      Symbol *sym = m_symbolVec[i];
      if (sym->isSystem()) continue;

      cg_printf("// @const  %s\t$%s\n",
                sym->getFinalType()->toString().c_str(),
                sym->getName().c_str());
    }
  }
}
