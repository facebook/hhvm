/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/compiler/analysis/variable_table.h"
#include <map>
#include <set>
#include <utility>
#include "hphp/compiler/analysis/analysis_result.h"
#include "hphp/compiler/analysis/file_scope.h"
#include "hphp/compiler/analysis/code_error.h"
#include "hphp/compiler/code_generator.h"
#include "hphp/compiler/expression/modifier_expression.h"
#include "hphp/compiler/analysis/function_scope.h"
#include "hphp/compiler/expression/simple_variable.h"
#include "hphp/compiler/builtin_symbols.h"
#include "hphp/compiler/option.h"
#include "hphp/compiler/expression/simple_function_call.h"
#include "hphp/compiler/analysis/class_scope.h"
#include "hphp/compiler/expression/parameter_expression.h"
#include "hphp/compiler/expression/static_member_expression.h"
#include "hphp/parser/location.h"
#include "hphp/parser/parser.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

VariableTable::VariableTable(BlockScope &blockScope)
    : SymbolTable(blockScope), m_attribute(0), m_nextParam(0),
      m_hasGlobal(false), m_hasStatic(false),
      m_hasPrivate(false), m_hasNonStaticPrivate(false),
      m_forcedVariants(0) {
}

void VariableTable::getLocalVariableNames(
  std::vector<std::string>& syms
) const {
  FunctionScopeRawPtr fs = getScopePtr()->getContainingFunction();
  bool dollarThisIsSpecial = fs->getContainingClass() ||
                             fs->inPseudoMain() ||
                             // In closures, $this is "sometimes"
                             // special (if it's a closure in a method
                             // body), but it easiest to just always
                             // treat it special.
                             fs->isClosure();

  bool hadThisSym = false;
  for (unsigned int i = 0; i < m_symbolVec.size(); i++) {
    auto const& name = m_symbolVec[i]->getName();
    if (name == "this" && dollarThisIsSpecial) {
      /*
       * The "this" variable in methods, pseudo-main, or closures is
       * special and is handled separately below.
       *
       * Closures are the specialest.
       */
      hadThisSym = true;
      continue;
    }
    syms.push_back(name);
  }

  if (fs->needsLocalThis() || (hadThisSym && fs->isClosure())) {
    assert(dollarThisIsSpecial);
    // We only need a local variable named "this" if the current function
    // contains an occurrence of "$this" that is not part of a property
    // expression or object method call expression
    syms.push_back("this");
  }
}

void VariableTable::getNames(std::set<std::string> &names,
                             bool collectPrivate /* = true */) const {
  for (unsigned int i = 0; i < m_symbolVec.size(); i++) {
    if (collectPrivate || !m_symbolVec[i]->isPrivate()) {
      names.insert(m_symbolVec[i]->getName());
    }
  }
}

bool VariableTable::isParameter(const std::string &name) const {
  const Symbol *sym = getSymbol(name);
  return sym && sym->isParameter();
}

bool VariableTable::isPublic(const std::string &name) const {
  const Symbol *sym = getSymbol(name);
  return sym && sym->isPublic();
}

bool VariableTable::isProtected(const std::string &name) const {
  const Symbol *sym = getSymbol(name);
  return sym && sym->isProtected();
}

bool VariableTable::isPrivate(const std::string &name) const {
  const Symbol *sym = getSymbol(name);
  return sym && sym->isPrivate();
}

bool VariableTable::isStatic(const std::string &name) const {
  const Symbol *sym = getSymbol(name);
  return sym && sym->isStatic();
}

bool VariableTable::isGlobal(const std::string &name) const {
  const Symbol *sym = getSymbol(name);
  return sym && sym->isGlobal();
}

bool VariableTable::isSuperGlobal(const std::string &name) const {
  const Symbol *sym = getSymbol(name);
  return sym && sym->isSuperGlobal();
}

bool VariableTable::isInherited(const std::string &name) const {
  const Symbol *sym = getSymbol(name);
  return !sym ||
    (!sym->isGlobal() && !sym->isSystem() && !sym->getDeclaration());
}

ConstructPtr VariableTable::getStaticInitVal(std::string varName) {
  if (Symbol *sym = getSymbol(varName)) {
    return sym->getStaticInitVal();
  }
  return ConstructPtr();
}

bool VariableTable::setStaticInitVal(std::string varName,
                                     ConstructPtr value) {
  Symbol *sym = addSymbol(varName);
  bool exists = (sym->getStaticInitVal() != nullptr);
  sym->setStaticInitVal(value);
  return exists;
}

ConstructPtr VariableTable::getClassInitVal(std::string varName) {
  if (Symbol *sym = getSymbol(varName)) {
    return sym->getClassInitVal();
  }
  return ConstructPtr();
}

bool VariableTable::setClassInitVal(std::string varName, ConstructPtr value) {
  Symbol *sym = addSymbol(varName);
  bool exists = (sym->getClassInitVal() != nullptr);
  sym->setClassInitVal(value);
  return exists;
}

///////////////////////////////////////////////////////////////////////////////

void VariableTable::addParam(const std::string &name,
                             AnalysisResultConstPtr ar,
                             ConstructPtr construct) {
  Symbol *sym = addDeclaredSymbol(name, construct);
  if (!sym->isParameter()) {
    sym->setParameterIndex(m_nextParam++);
  }
  add(sym, false, ar, construct, ModifierExpressionPtr());
}

void VariableTable::addParamLike(const std::string &name,
                                 AnalysisResultPtr ar,
                                 ConstructPtr construct, bool firstPass) {
  if (firstPass) {
    add(name, false, ar, construct, ModifierExpressionPtr());
  } else {
    checkVariable(name, ar, construct);
  }
}

void VariableTable::addStaticVariable(Symbol* sym, AnalysisResultPtr ar,
                                      bool /*member*/ /* = false */) {
  if (isGlobalTable(ar) ||
      sym->isStatic()) {
    return; // a static variable at global scope is the same as non-static
  }

  sym->setStatic();
  m_hasStatic = true;
}

void VariableTable::addStaticVariable(Symbol *sym,
                                      AnalysisResultConstPtr ar,
                                      bool member /* = false */) {
  if (isGlobalTable(ar) ||
      sym->isStatic()) {
    return; // a static variable at global scope is the same as non-static
  }

  addStaticVariable(sym, ar->lock().get(), member);
}

void VariableTable::cleanupForError(AnalysisResultConstPtr /*ar*/) {}

bool VariableTable::markOverride(AnalysisResultConstPtr ar,
                                 const std::string &name) {
  Symbol *sym = getSymbol(name);
  assert(sym && sym->isPresent());
  bool ret = false;
  if (!sym->isStatic() ||
      (sym->isPublic() && !sym->getClassInitVal())) {
    Symbol *s2;
    ClassScopePtr parent = findParent(ar, name, s2);
    if (parent) {
      assert(s2 && s2->isPresent());
      if (!s2->isPrivate()) {
        if (!sym->isStatic() || s2->isProtected()) {
          if (sym->isPrivate() || sym->isStatic()) {
            // don't mark the symbol as overridden
            return true;
          }
          if (sym->isProtected() && s2->isPublic()) {
            // still mark the symbol as overridden
            ret = true;
          }
          sym->setOverride();
        }
      }
    }
  }
  return ret;
}

void VariableTable::add(const std::string &name,
                        bool implicit, AnalysisResultConstPtr ar,
                        ConstructPtr construct,
                        ModifierExpressionPtr modifiers) {
  add(addSymbol(name), implicit, ar, construct, modifiers);
}

void VariableTable::add(Symbol *sym,
                        bool implicit, AnalysisResultConstPtr ar,
                        ConstructPtr construct,
                        ModifierExpressionPtr modifiers) {
  if (getAttribute(InsideStaticStatement)) {
    addStaticVariable(sym, ar);
  } else if (getAttribute(InsideGlobalStatement)) {
    sym->setGlobal();
    m_hasGlobal = true;
    AnalysisResult::Locker lock(ar);
    if (!isGlobalTable(ar)) {
      lock->getVariables()->add(sym->getName(), implicit,
                                ar, construct, modifiers);
    }
  } else if (!sym->isHidden() && isPseudoMainTable()) {
    // A variable used in a pseudomain
    // only need to do this once... should mark the sym.
    ar->lock()->getVariables()->add(sym->getName(), implicit, ar,
                                    construct, modifiers);
  }

  if (modifiers) {
    if (modifiers->isProtected()) {
      sym->setProtected();
    } else if (modifiers->isPrivate()) {
      sym->setPrivate();
      m_hasPrivate = true;
      if (!sym->isStatic() && !modifiers->isStatic()) {
        m_hasNonStaticPrivate = true;
      }
    }
    if (modifiers->isStatic()) {
      addStaticVariable(sym, ar);
    }
  }

  if (sym->isParameter()) {
    auto p = dynamic_pointer_cast<ParameterExpression>(construct);
    if (p) {
      sym->setDeclaration(construct);
    }
  } else {
    sym->setDeclaration(construct);
  }

  if (!implicit && m_blockScope.isFirstPass()) {
    if (!sym->getValue()) {
      sym->setValue(construct);
    }
  }
}

void VariableTable::checkVariable(const std::string &name,
                                  AnalysisResultConstPtr ar,
                                  ConstructPtr construct) {
  checkVariable(addSymbol(name), ar, construct);
}

void VariableTable::checkVariable(Symbol *sym,
                                  AnalysisResultConstPtr ar,
                                  ConstructPtr construct) {

  // Variable used in pseudomain
  if (!sym->isHidden() && isPseudoMainTable()) {
    // only need to do this once... should mark the sym.
    ar->lock()->getVariables()->checkVariable(sym->getName(), ar, construct);
  }

  if (!sym->declarationSet()) {
    sym->setDeclaration(construct);
  }
}

Symbol *VariableTable::findProperty(ClassScopePtr &cls,
                                    const std::string &name,
                                    AnalysisResultConstPtr ar) {
  Symbol *sym = getSymbol(name);
  if (sym) {
    assert(sym->declarationSet());
    if (!sym->isOverride()) {
      return sym;
    }
    assert(!sym->isStatic());
    sym = nullptr;
  }

  if (!sym) {
    if (ClassScopePtr parent = findParent(ar, name, sym)) {
      sym = parent->findProperty(parent, name, ar);
      if (sym) {
        cls = parent;
        return sym;
      }
    }
  }

  return sym;
}

void VariableTable::addSuperGlobal(const std::string &name) {
  addSymbol(name)->setSuperGlobal();
}

bool VariableTable::isConvertibleSuperGlobal(const std::string &name) const {
  return !getAttribute(ContainsDynamicVariable) && isSuperGlobal(name);
}

ClassScopePtr VariableTable::findParent(AnalysisResultConstPtr ar,
                                        const std::string &name,
                                        const Symbol *&sym) const {
  sym = nullptr;
  for (ClassScopePtr parent = m_blockScope.getParentScope(ar);
       parent && !parent->isRedeclaring();
       parent = parent->getParentScope(ar)) {
    sym = parent->getVariables()->getSymbol(name);
    assert(!sym || sym->isPresent());
    if (sym) return parent;
  }
  return ClassScopePtr();
}

bool VariableTable::isGlobalTable(AnalysisResultConstPtr ar) const {
  return ar->getVariables().get() == this;
}

bool VariableTable::isPseudoMainTable() const {
  return m_blockScope.inPseudoMain();
}

bool VariableTable::hasPrivate() const {
  return m_hasPrivate;
}

bool VariableTable::hasNonStaticPrivate() const {
  return m_hasNonStaticPrivate;
}

void VariableTable::outputPHP(CodeGenerator& cg, AnalysisResultPtr /*ar*/) {
  if (Option::ConvertSuperGlobals && !getAttribute(ContainsDynamicVariable)) {
    std::set<std::string> convertibles;
    typedef std::pair<const std::string,Symbol> symPair;
    for (symPair &sym: m_symbolMap) {
      if (sym.second.isSuperGlobal() && !sym.second.declarationSet()) {
        convertibles.insert(sym.second.getName());
      }
    }
    if (!convertibles.empty()) {
      cg_printf("/* converted super globals */ global ");
      for (auto iter = convertibles.begin();
           iter != convertibles.end(); ++iter) {
        if (iter != convertibles.begin()) cg_printf(",");
        cg_printf("$%s", iter->c_str());
      }
      cg_printf(";\n");
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
}
