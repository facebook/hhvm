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

#include <compiler/analysis/variable_table.h>
#include <compiler/analysis/analysis_result.h>
#include <compiler/analysis/file_scope.h>
#include <compiler/analysis/code_error.h>
#include <compiler/analysis/type.h>
#include <compiler/code_generator.h>
#include <compiler/expression/modifier_expression.h>
#include <compiler/analysis/function_scope.h>
#include <compiler/expression/simple_variable.h>
#include <compiler/option.h>
#include <compiler/expression/simple_function_call.h>
#include <compiler/analysis/class_scope.h>
#include <compiler/expression/static_member_expression.h>
#include <runtime/base/class_info.h>
#include <util/util.h>
#include <util/parser/location.h>
#include <util/parser/parser.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// StaticGlobalInfo

string VariableTable::StaticGlobalInfo::GetId
(ClassScopePtr cls, FunctionScopePtr func,
 const string &name) {
  ASSERT(cls || func);

  // format: <class>$$<func>$$name
  string id;
  if (cls) {
    id += cls->getId();
    id += Option::IdPrefix;
  }
  if (func) {
    id += func->getId();
    id += Option::IdPrefix;
  }
  id += name;

  return id;
}

///////////////////////////////////////////////////////////////////////////////

VariableTable::VariableTable(BlockScope &blockScope)
    : SymbolTable(blockScope, false), m_attribute(0), m_nextParam(0),
      m_hasGlobal(false), m_hasStatic(false),
      m_hasPrivate(false), m_hasNonStaticPrivate(false),
      m_forcedVariants(0) {
}

void VariableTable::getLocalVariableNames(vector<string> &syms) const {
  FunctionScopeRawPtr fs = getScopePtr()->getContainingFunction();
  bool dollarThisIsSpecial = (fs->getContainingClass() ||
                              fs->inPseudoMain());

  for (unsigned int i = 0; i < m_symbolVec.size(); i++) {
    const string& name = m_symbolVec[i]->getName();
    if (name == "this" && dollarThisIsSpecial) {
      // The "this" variable in methods and pseudo-main is special and is
      // handled separately below.
      continue;
    }
    syms.push_back(name);
  }

  if (fs->needsLocalThis()) {
    ASSERT(dollarThisIsSpecial);
    // We only need a local variable named "this" if the current function
    // contains an occurrence of "$this" that is not part of a property
    // expression or object method call expression
    syms.push_back("this");
  }
}

void VariableTable::getNames(std::set<string> &names,
                             bool collectPrivate /* = true */) const {
  for (unsigned int i = 0; i < m_symbolVec.size(); i++) {
    if (collectPrivate || !m_symbolVec[i]->isPrivate()) {
      names.insert(m_symbolVec[i]->getName());
    }
  }
}

bool VariableTable::isParameter(const string &name) const {
  const Symbol *sym = getSymbol(name);
  return sym && sym->isParameter();
}

bool VariableTable::isPublic(const string &name) const {
  const Symbol *sym = getSymbol(name);
  return sym && sym->isPublic();
}

bool VariableTable::isProtected(const string &name) const {
  const Symbol *sym = getSymbol(name);
  return sym && sym->isProtected();
}

bool VariableTable::isPrivate(const string &name) const {
  const Symbol *sym = getSymbol(name);
  return sym && sym->isPrivate();
}

bool VariableTable::isStatic(const string &name) const {
  const Symbol *sym = getSymbol(name);
  return sym && sym->isStatic();
}

bool VariableTable::isGlobal(const string &name) const {
  const Symbol *sym = getSymbol(name);
  return sym && sym->isGlobal();
}

bool VariableTable::isRedeclared(const string &name) const {
  const Symbol *sym = getSymbol(name);
  return sym && sym->isRedeclared();
}

bool VariableTable::isLocalGlobal(const string &name) const {
  const Symbol *sym = getSymbol(name);
  return sym && sym->isLocalGlobal();
}

bool VariableTable::isNestedStatic(const string &name) const {
  const Symbol *sym = getSymbol(name);
  return sym && sym->isNestedStatic();
}

bool VariableTable::isLvalParam(const string &name) const {
  const Symbol *sym = getSymbol(name);
  return sym && sym->isLvalParam();
}

bool VariableTable::isUsed(const string &name) const {
  const Symbol *sym = getSymbol(name);
  return sym && sym->isUsed();
}

bool VariableTable::isNeeded(const string &name) const {
  const Symbol *sym = getSymbol(name);
  return sym && sym->isNeeded();
}

bool VariableTable::isSuperGlobal(const string &name) const {
  const Symbol *sym = getSymbol(name);
  return sym && sym->isSuperGlobal();
}

bool VariableTable::isLocal(const string &name) const {
  return isLocal(getSymbol(name));
}

bool VariableTable::isLocal(const Symbol *sym) const {
  if (!sym) return false;
  if (getScopePtr()->is(BlockScope::FunctionScope)) {
    /*
      isSuperGlobal is not wanted here. It just means that
      $GLOBALS[name] was referenced in this scope.
      It doesnt say anything about the variable $name.
    */
    return (!sym->isStatic() &&
            !sym->isGlobal() &&
            !sym->isParameter());
  }
  return false;
}

bool VariableTable::needLocalCopy(const string &name) const {
  return needLocalCopy(getSymbol(name));
}

bool VariableTable::needLocalCopy(const Symbol *sym) const {
  return sym &&
    (sym->isGlobal() || sym->isStatic()) &&
    (sym->isRedeclared() ||
     sym->isNestedStatic() ||
     sym->isLocalGlobal() ||
     getAttribute(ContainsDynamicVariable) ||
     getAttribute(ContainsExtract) ||
     getAttribute(ContainsUnset));
}


bool VariableTable::needGlobalPointer() const {
  return !isPseudoMainTable() &&
    (m_hasGlobal ||
     m_hasStatic ||
     getAttribute(ContainsDynamicVariable) ||
     getAttribute(ContainsExtract) ||
     getAttribute(ContainsUnset) ||
     getAttribute(NeedGlobalPointer));
}

bool VariableTable::isInherited(const string &name) const {
  const Symbol *sym = getSymbol(name);
  return !sym ||
    (!sym->isGlobal() && !sym->isSystem() && !sym->getDeclaration());
}

const char *VariableTable::getVariablePrefix(const string &name) const {
  return getVariablePrefix(getSymbol(name));
}

const char *VariableTable::getVariablePrefix(const Symbol *sym) const {
  if (sym && sym->isStatic()) {
    if (!needLocalCopy(sym)) {
      return Option::StaticVariablePrefix;
    }
    return Option::VariablePrefix;
  }

  if (getAttribute(ForceGlobal)) {
    return Option::GlobalVariablePrefix;
  }

  if (sym && sym->isGlobal()) {
    if (!needLocalCopy(sym)) {
      return Option::GlobalVariablePrefix;
    }
  }

  if (sym && sym->isHidden() && !sym->isParameter()) {
    return Option::HiddenVariablePrefix;
  }

  return Option::VariablePrefix;
}

string VariableTable::getVariableName(AnalysisResultConstPtr ar,
                                      const string &name) const {
  return getVariableName(ar, getSymbol(name));
}

string VariableTable::getVariableName(AnalysisResultConstPtr ar,
                                      const Symbol *sym) const {
  const string &name = sym->getName();
  if (sym && sym->isStatic()) {
    if (!needLocalCopy(sym)) {
      return string(Option::StaticVariablePrefix) +
             CodeGenerator::FormatLabel(name);
    }
    return string(Option::VariablePrefix) + CodeGenerator::FormatLabel(name);
  }

  if (getAttribute(ForceGlobal)) {
    return getGlobalVariableName(ar, name);
  }

  if (sym->isGlobal()) {
    if (!needLocalCopy(sym)) {
      return getGlobalVariableName(ar, name);
    }
  }
  return (sym->isHidden() && !sym->isParameter() ?
          Option::HiddenVariablePrefix : Option::VariablePrefix) +
    CodeGenerator::FormatLabel(name);
}

string
VariableTable::getGlobalVariableName(AnalysisResultConstPtr ar,
                                     const string &name) const {
  return string("GV(") + CodeGenerator::FormatLabel(name) + ")";
}

ConstructPtr VariableTable::getStaticInitVal(string varName) {
  if (Symbol *sym = getSymbol(varName)) {
    return sym->getStaticInitVal();
  }
  return ConstructPtr();
}

bool VariableTable::setStaticInitVal(string varName,
                                     ConstructPtr value) {
  Symbol *sym = addSymbol(varName);
  bool exists = sym->getStaticInitVal();
  sym->setStaticInitVal(value);
  return exists;
}

ConstructPtr VariableTable::getClassInitVal(string varName) {
  if (Symbol *sym = getSymbol(varName)) {
    return sym->getClassInitVal();
  }
  return ConstructPtr();
}

bool VariableTable::setClassInitVal(string varName, ConstructPtr value) {
  Symbol *sym = addSymbol(varName);
  bool exists = sym->getClassInitVal();
  sym->setClassInitVal(value);
  return exists;
}

///////////////////////////////////////////////////////////////////////////////

TypePtr VariableTable::addParam(const string &name, TypePtr type,
                                AnalysisResultConstPtr ar,
                                ConstructPtr construct) {
  Symbol *sym = addDeclaredSymbol(name, construct);
  if (!sym->isParameter()) {
    sym->setParameterIndex(m_nextParam++);
  }
  return type ?
    add(sym, type, false, ar, construct, ModifierExpressionPtr()) : type;
}

TypePtr VariableTable::addParamLike(const string &name, TypePtr type,
                                    AnalysisResultPtr ar,
                                    ConstructPtr construct, bool firstPass) {
  TypePtr ret = type;
  if (firstPass) {
    ret = add(name, ret, false, ar,
              construct, ModifierExpressionPtr());
  } else {
    ret = checkVariable(name, ret, true, ar, construct);
    if (ret->is(Type::KindOfSome)) {
      // This is probably too conservative. The problem is that
      // a function never called will have parameter types of Any.
      // Functions that it calls won't be able to accept variant unless
      // it is forced here.
      forceVariant(ar, name, VariableTable::AnyVars);
      ret = Type::Variant;
    }
  }
  return ret;
}

void VariableTable::addStaticVariable(Symbol *sym,
                                      AnalysisResultPtr ar,
                                      bool member /* = false */) {
  if (isGlobalTable(ar) ||
      sym->isStatic()) {
    return; // a static variable at global scope is the same as non-static
  }

  sym->setStatic();
  m_hasStatic = true;

  FunctionScopeRawPtr funcScope = getFunctionScope();
  if (funcScope &&
      (funcScope->isClosure() || funcScope->isGeneratorFromClosure())) {
    // static variables for closures/closure generators are local to the
    // function scope
    m_staticLocalsVec.push_back(sym);
  } else {
    VariableTablePtr globalVariables = ar->getVariables();
    StaticGlobalInfoPtr sgi(new StaticGlobalInfo());
    sgi->sym = sym;
    sgi->variables = this;
    sgi->cls = getClassScope();
    sgi->func = member ? FunctionScopeRawPtr() : getFunctionScope();

    globalVariables->m_staticGlobalsVec.push_back(sgi);
  }
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

void VariableTable::cleanupForError(AnalysisResultConstPtr ar) {
  if (!m_hasStatic) return;

  AnalysisResult::Locker lock(ar);
  VariableTablePtr g = lock->getVariables();
  ClassScopeRawPtr cls = getClassScope();

  for (unsigned i = g->m_staticGlobalsVec.size(); i--; ) {
    if (g->m_staticGlobalsVec[i]->cls == cls) {
      g->m_staticGlobalsVec.erase(g->m_staticGlobalsVec.begin() + i);
    }
  }
}

bool VariableTable::markOverride(AnalysisResultPtr ar, const string &name) {
  Symbol *sym = getSymbol(name);
  ASSERT(sym && sym->isPresent());
  bool ret = false;
  if (!sym->isStatic() ||
      (sym->isPublic() && !sym->getClassInitVal())) {
    Symbol *s2;
    ClassScopePtr parent = findParent(ar, name, s2);
    if (parent) {
      ASSERT(s2 && s2->isPresent());
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

TypePtr VariableTable::add(const string &name, TypePtr type,
                           bool implicit, AnalysisResultConstPtr ar,
                           ConstructPtr construct,
                           ModifierExpressionPtr modifiers) {
  return add(addSymbol(name), type, implicit, ar,
             construct, modifiers);
}

TypePtr VariableTable::add(Symbol *sym, TypePtr type,
                           bool implicit, AnalysisResultConstPtr ar,
                           ConstructPtr construct,
                           ModifierExpressionPtr modifiers) {
  if (getAttribute(InsideStaticStatement)) {
    addStaticVariable(sym, ar);
    if (ClassScope::NeedStaticArray(getClassScope(), getFunctionScope())) {
      forceVariant(ar, sym->getName(), AnyVars);
    }
  } else if (getAttribute(InsideGlobalStatement)) {
    sym->setGlobal();
    m_hasGlobal = true;
    AnalysisResult::Locker lock(ar);
    if (!isGlobalTable(ar)) {
      lock->getVariables()->add(sym->getName(), type, implicit,
                                ar, construct, modifiers);
    }
    ASSERT(type->is(Type::KindOfSome) || type->is(Type::KindOfAny));
    TypePtr varType = ar->getVariables()->getFinalType(sym->getName());
    if (varType) {
      type = varType;
    } else {
      lock->getVariables()->setType(ar, sym->getName(), type, true);
    }
  } else if (!sym->isHidden() && isPseudoMainTable()) {
    // A variable used in a pseudomain
    // only need to do this once... should mark the sym.
    ar->lock()->getVariables()->add(sym->getName(), type, implicit, ar,
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

  type = setType(ar, sym, type, true);
  sym->setDeclaration(construct);

  if (!implicit && m_blockScope.isFirstPass()) {
    if (!sym->getValue()) {
      sym->setValue(construct);
    }
  }
  return type;
}

TypePtr VariableTable::checkVariable(const string &name, TypePtr type,
                                     bool coerce, AnalysisResultConstPtr ar,
                                     ConstructPtr construct) {
  return checkVariable(addSymbol(name), type,
                       coerce, ar, construct);
}

TypePtr VariableTable::checkVariable(Symbol *sym, TypePtr type,
                                     bool coerce, AnalysisResultConstPtr ar,
                                     ConstructPtr construct) {

  // Variable used in pseudomain
  if (!sym->isHidden() && isPseudoMainTable()) {
    // only need to do this once... should mark the sym.
    ar->lock()->getVariables()->checkVariable(sym->getName(), type,
                                              coerce, ar, construct);
  }

  if (!sym->declarationSet()) {
    type = setType(ar, sym, type, coerce);
    sym->setDeclaration(construct);
    return type;
  }

  return setType(ar, sym, type, coerce);
}

Symbol *VariableTable::findProperty(ClassScopePtr &cls,
                                    const string &name,
                                    AnalysisResultConstPtr ar) {
  Symbol *sym = getSymbol(name);
  if (sym) {
    ASSERT(sym->declarationSet());
    if (!sym->isOverride()) {
      return sym;
    }
    ASSERT(!sym->isStatic());
    sym = NULL;
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

TypePtr VariableTable::checkProperty(BlockScopeRawPtr context,
                                     Symbol *sym, TypePtr type,
                                     bool coerce, AnalysisResultConstPtr ar) {
  assert(sym->isPresent());
  if (sym->isOverride()) {
    Symbol *base;
    ClassScopePtr parent = findParent(ar, sym->getName(), base);
    ASSERT(parent);
    ASSERT(parent.get() != &m_blockScope);
    ASSERT(base && !base->isPrivate());
    if (context->is(BlockScope::FunctionScope)) {
      GET_LOCK(parent);
      type = parent->getVariables()->setType(ar, base, type, coerce);
    } else {
      TRY_LOCK(parent);
      type = parent->getVariables()->setType(ar, base, type, coerce);
    }
  }
  return setType(ar, sym, type, coerce);
}

bool VariableTable::checkRedeclared(const string &name,
                                    Statement::KindOf kindOf)
{
  Symbol *sym = getSymbol(name);
  ASSERT(kindOf == Statement::KindOfStaticStatement ||
         kindOf == Statement::KindOfGlobalStatement);
  if (kindOf == Statement::KindOfStaticStatement && sym->isPresent()) {
    if (sym->isStatic()) {
      return true;
    } else if (!sym->isRedeclared()) {
      sym->setRedeclared();
      return true;
    } else {
      return false;
    }
  } else if (kindOf == Statement::KindOfGlobalStatement &&
             sym && !sym->isGlobal() && !sym->isRedeclared()) {
    sym->setRedeclared();
    return true;
  } else {
    return false;
  }
}

void VariableTable::addLocalGlobal(const string &name) {
  addSymbol(name)->setLocalGlobal();
}

void VariableTable::addNestedStatic(const string &name) {
  addSymbol(name)->setNestedStatic();
}

void VariableTable::addLvalParam(const string &name) {
  addSymbol(name)->setLvalParam();
}

void VariableTable::addUsed(const string &name) {
  addSymbol(name)->setUsed();
}

void VariableTable::addNeeded(const string &name) {
  addSymbol(name)->setNeeded();
}

bool VariableTable::checkUnused(Symbol *sym) {
  if ((!sym || !sym->isHidden()) &&
      (isPseudoMainTable() || getAttribute(ContainsDynamicVariable))) {
    return false;
  }
  if (sym) {
    return !sym->isUsed() && isLocal(sym);
  }
  return false;
}

void VariableTable::clearUsed() {
  typedef std::pair<const string,Symbol> symPair;
  bool ps = isPseudoMainTable();
  BOOST_FOREACH(symPair &sym, m_symbolMap) {
    if (!ps || sym.second.isHidden()) {
      sym.second.clearUsed();
      sym.second.clearNeeded();
      sym.second.clearReferenced();
      sym.second.clearGlobal();
      sym.second.clearReseated();
    } else {
      sym.second.setReferenced();
    }
  }
}

void VariableTable::forceVariants(AnalysisResultConstPtr ar, int varClass,
                                  bool recur /* = true */) {
  int mask = varClass & ~m_forcedVariants;
  if (mask) {
    if (!m_hasPrivate) mask &= ~AnyPrivateVars;
    if (!m_hasStatic) mask &= ~AnyStaticVars;

    if (mask) {
      for (unsigned int i = 0; i < m_symbolVec.size(); i++) {
        Symbol *sym = m_symbolVec[i];
        if (!sym->isHidden() && sym->declarationSet() &&
            mask & GetVarClassMaskForSym(sym)) {
          setType(ar, sym, Type::Variant, true);
          sym->setIndirectAltered();
        }
      }
    }
    m_forcedVariants |= varClass;

    if (recur) {
      ClassScopePtr parent = m_blockScope.getParentScope(ar);
      if (parent && !parent->isRedeclaring()) {
        parent->getVariables()->forceVariants(ar, varClass & ~AnyPrivateVars);
      }
    }
  }
}

void VariableTable::forceVariant(AnalysisResultConstPtr ar,
                                 const string &name, int varClass) {
  int mask = varClass & ~m_forcedVariants;
  if (!mask) return;
  if (!m_hasPrivate) mask &= ~AnyPrivateVars;
  if (!m_hasStatic) mask &= ~AnyStaticVars;
  if (!mask) return;
  if (Symbol *sym = getSymbol(name)) {
    if (!sym->isHidden() && sym->declarationSet() &&
        mask & GetVarClassMaskForSym(sym)) {
      setType(ar, sym, Type::Variant, true);
      sym->setIndirectAltered();
    }
  }
}

TypePtr VariableTable::setType(AnalysisResultConstPtr ar,
                               const std::string &name,
                               TypePtr type, bool coerce) {
  return setType(ar, addSymbol(name), type, coerce);
}

TypePtr VariableTable::setType(AnalysisResultConstPtr ar, Symbol *sym,
                               TypePtr type, bool coerce) {
  bool force_coerce = coerce;
  int mask = GetVarClassMaskForSym(sym);
  if (m_forcedVariants & mask && !sym->isHidden()) {
    type = Type::Variant;
    force_coerce = true;
  }
  TypePtr ret = SymbolTable::setType(ar, sym, type, force_coerce);
  if (!ret) return ret;

  if (sym->isGlobal() && !isGlobalTable(ar)) {
    ar->lock()->getVariables()->setType(ar, sym->getName(), type, coerce);
  }

  if (coerce) {
    if (sym->isParameter()) {
      FunctionScope *func = dynamic_cast<FunctionScope *>(&m_blockScope);
      ASSERT(func);
      TypePtr paramType = func->setParamType(ar,
                                             sym->getParameterIndex(), type);
      if (!Type::SameType(paramType, type)) {
        return setType(ar, sym, paramType, true); // recursively
      }
    }
  }
  return ret;
}

void VariableTable::dumpStats(std::map<string, int> &typeCounts) {
  for (unsigned int i = 0; i < m_symbolVec.size(); i++) {
    Symbol *sym = m_symbolVec[i];
    if (sym->isGlobal()) continue;
    typeCounts[sym->getFinalType()->toString()]++;
  }
}

void VariableTable::addSuperGlobal(const string &name) {
  addSymbol(name)->setSuperGlobal();
}

bool VariableTable::isConvertibleSuperGlobal(const string &name) const {
  return !getAttribute(ContainsDynamicVariable) && isSuperGlobal(name);
}

ClassScopePtr VariableTable::findParent(AnalysisResultConstPtr ar,
                                        const string &name,
                                        const Symbol *&sym) const {
  sym = NULL;
  for (ClassScopePtr parent = m_blockScope.getParentScope(ar);
       parent && !parent->isRedeclaring();
       parent = parent->getParentScope(ar)) {
    sym = parent->getVariables()->getSymbol(name);
    ASSERT(!sym || sym->isPresent());
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

void VariableTable::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (Option::GenerateInferredTypes) {
    for (unsigned int i = 0; i < m_symbolVec.size(); i++) {
      Symbol *sym = m_symbolVec[i];
      if (isInherited(sym->getName())) continue;

      if (sym->isParameter()) {
        cg_printf("// @param  ");
      } else if (sym->isGlobal()) {
        cg_printf("// @global ");
      } else if (sym->isStatic()) {
        cg_printf("// @static ");
      } else {
        cg_printf("// @local  ");
      }
      cg_printf("%s\t$%s\n", sym->getFinalType()->toString().c_str(),
                sym->getName().c_str());
    }
  }
  if (Option::ConvertSuperGlobals && !getAttribute(ContainsDynamicVariable)) {
    std::set<string> convertibles;
    typedef std::pair<const string,Symbol> symPair;
    BOOST_FOREACH(symPair &sym, m_symbolMap) {
      if (sym.second.isSuperGlobal() && !sym.second.declarationSet()) {
        convertibles.insert(sym.second.getName());
      }
    }
    if (!convertibles.empty()) {
      cg_printf("/* converted super globals */ global ");
      for (std::set<string>::const_iterator iter = convertibles.begin();
           iter != convertibles.end(); ++iter) {
        if (iter != convertibles.begin()) cg_printf(",");
        cg_printf("$%s", iter->c_str());
      }
      cg_printf(";\n");
    }
  }
}

static bool by_location(const VariableTable::StaticGlobalInfoPtr &p1,
                        const VariableTable::StaticGlobalInfoPtr &p2) {
  ConstructRawPtr d1 = p1->sym->getDeclaration();
  ConstructRawPtr d2 = p2->sym->getDeclaration();
  if (!d1) return d2;
  if (!d2) return false;
  return d1->getLocation()->compare(d2->getLocation().get()) < 0;
}

void VariableTable::canonicalizeStaticGlobals() {
  ASSERT(m_staticGlobals.empty());

  sort(m_staticGlobalsVec.begin(), m_staticGlobalsVec.end(), by_location);

  for (unsigned int i = 0; i < m_staticGlobalsVec.size(); i++) {
    StaticGlobalInfoPtr &sgi = m_staticGlobalsVec[i];
    if (!sgi->sym->getDeclaration()) continue;
    string id = StaticGlobalInfo::GetId(sgi->cls, sgi->func,
                                        sgi->sym->getName());
    ASSERT(m_staticGlobals.find(id) == m_staticGlobals.end());
    m_staticGlobals[id] = sgi;
  }
}

// Make sure GlobalVariables::getRefByIdx has the correct indices
void VariableTable::checkSystemGVOrder(SymbolSet &variants,
                                       unsigned int max) {
  static const char *sgvNames[] = {
    "gvm_HTTP_RAW_POST_DATA",
    "gvm__COOKIE",
    "gvm__ENV",
    "gvm__FILES",
    "gvm__GET",
    "gvm__POST",
    "gvm__REQUEST",
    "gvm__SERVER",
    "gvm__SESSION",
    "gvm_argc",
    "gvm_argv",
    "gvm_http_response_header",
  };
  assert(variants.size() >= max &&
         sizeof(sgvNames) / sizeof(sgvNames[0]) == max);
  unsigned int i = 0;
  for (SymbolSet::const_iterator iterName = variants.begin();
       iterName != variants.end(); ++iterName) {
    assert(!strcmp(sgvNames[i], iterName->c_str()));
    i++;
  }
}

void VariableTable::outputCPPGlobalVariablesHeader(CodeGenerator &cg,
                                                   AnalysisResultPtr ar) {
  ASSERT(!m_staticGlobals.empty() || m_staticGlobalsVec.empty());

  cg.printSection("Class Forward Declarations\n");
  for (StringToStaticGlobalInfoPtrMap::const_iterator iter =
         m_staticGlobals.begin(); iter != m_staticGlobals.end(); ++iter) {
    StaticGlobalInfoPtr sgi = iter->second;
    if (!sgi->func) {
      ASSERT(!sgi->sym->isOverride());
      TypePtr varType = sgi->sym->getFinalType();
      if (varType->isSpecificObject()) {
        cg_printf("FORWARD_DECLARE_CLASS(%s);\n",
                  varType->getName().c_str());
      }
    }
  }

  bool system = (cg.getOutput() == CodeGenerator::SystemCPP);
  if (system) {
    cg_printf("class SystemGlobals : public Globals {\n");
    cg_indentBegin("public:\n");
    cg_printf("SystemGlobals();\n");
    cg_printf("void initialize();\n");
  } else {
    cg_printf("class GlobalVariables : public SystemGlobals {\n");
    cg_printf("DECLARE_SMART_ALLOCATION(GlobalVariables);\n");
    cg_indentBegin("public:\n");
    cg_printf("GlobalVariables();\n");
    cg_printf("~GlobalVariables();\n");
  }

  cg_printf("\n");

  // We will create one variable[] per type.
  Type2SymbolSetMap type2names;
  SymbolSet &variants = type2names["Variant"];
  SymbolSet &bools = type2names["bool"];

  type2names["int"], type2names["int64"], type2names["double"];

  // Global Variables
  for (unsigned int i = 0; i < m_symbolVec.size(); i++) {
    const Symbol *sym = m_symbolVec[i];
    if (system || !sym->isSystem()) {
      variants.insert(string("gvm_") +
                      CodeGenerator::FormatLabel(sym->getName()));
    }
  }

  if (system) checkSystemGVOrder(variants, m_symbolVec.size());

  // Dynamic Constants
  ar->getCPPDynamicConstantDecl(cg, type2names);

  string object = "Object";
  std::map<string,string> realClass;

  // Function/Method Static Variables
  for (StringToStaticGlobalInfoPtrMap::const_iterator iter =
         m_staticGlobals.begin(); iter != m_staticGlobals.end(); ++iter) {
    const string &id = iter->first;
    StaticGlobalInfoPtr sgi = iter->second;
    if (sgi->func) {
      TypePtr varType = sgi->sym->getFinalType();
      string s = varType->getCPPDecl(ar, sgi->func);
      string var = Option::StaticVariablePrefix + id;
      if (varType->isSpecificObject() && s != object) {
        realClass[var] = s;
        s = object;
      }
      type2names[s].insert(var);
    }
  }

  // Function/Method Static Variable Initialization Booleans
  for (StringToStaticGlobalInfoPtrMap::const_iterator iter =
         m_staticGlobals.begin(); iter != m_staticGlobals.end(); ++iter) {
    const string &id = iter->first;
    StaticGlobalInfoPtr sgi = iter->second;
    if (sgi->func) {
      string name = string(Option::InitPrefix) +
        Option::StaticVariablePrefix + id;
      if (ClassScope::NeedStaticArray(sgi->cls, sgi->func)) {
        variants.insert(name);
      } else {
        bools.insert(name);
      }
    }
  }

  // Class Static Variables
  for (StringToStaticGlobalInfoPtrMap::const_iterator iter =
         m_staticGlobals.begin(); iter != m_staticGlobals.end(); ++iter) {
    StaticGlobalInfoPtr sgi = iter->second;
    // id can change if we discover it is redeclared
    const string &id = StaticGlobalInfo::GetId(sgi->cls, sgi->func,
                                               sgi->sym->getName());
    if (!sgi->func) {
      ASSERT(!sgi->sym->isOverride());
      TypePtr varType = sgi->sym->getFinalType();
      string s = varType->getCPPDecl(ar, sgi->cls);
      string var = Option::StaticPropertyPrefix + id;
      if (varType->isSpecificObject() && s != object) {
        // realClass[var] = s;
        s = object;
      }
      type2names[s].insert(Option::StaticPropertyPrefix + id);
    }
  }

  // Class Static Initializer Flags
  ar->getCPPClassStaticInitializerFlags(cg, type2names);

  // PseudoMain Variables
  ar->getCPPFileRunDecls(cg, type2names);

  if (!system) {
    // Volatile class declared flags
    ar->getCPPClassDeclaredFlags(cg, type2names);
  }

  // Redeclared Functions
  ar->getCPPRedeclaredFunctionDecl(cg, type2names);

  // Redeclared Classes
  ar->getCPPRedeclaredClassDecl(cg, type2names);

  const char *prefix = system ?
    "stgv_" : "tgv_"; // (system) typed global variables
  for (Type2SymbolSetMap::const_iterator iter = type2names.begin();
       iter != type2names.end(); ++iter) {
    const string &type = iter->first;
    string typeName = type;
    Util::replaceAll(typeName, "*", "Ptr");
    const SymbolSet &names = iter->second;

    // generating arr[1] even if it's empty just to make
    // outputCPPGlobalVariablesImpl()'s memset easier to generate
    cg_printf("%s %s%s[%d];\n", type.c_str(), prefix, typeName.c_str(),
              (names.empty() ? 1 : (int)names.size()));

    int i = 0;
    bool gvmDone = false;
    for (SymbolSet::const_iterator iterName = names.begin();
         iterName != names.end(); ++iterName) {
      bool gvmPrefix = (strncmp(iterName->c_str(), "gvm_", 4) == 0);
      if (!gvmPrefix) {
        gvmDone = true;
      } else {
        assert(!gvmDone);
      }
      string cast;
      if (type == object) {
        std::map<string,string>::iterator it = realClass.find(*iterName);
        if (it != realClass.end()) {
          cast = ".cast<"+it->second+">()";
        }
      }
      cg_printf("#define %s %s%s[%d]%s\n", iterName->c_str(), prefix,
                typeName.c_str(), i, cast.c_str());
      i++;
    }
  }

  if (!system) {
    cg.printSection("Global Array Wrapper Methods");
    cg_indentBegin("virtual ssize_t staticSize() const {\n");
    cg_printf("return %lu;\n", m_symbolVec.size());
    cg_indentEnd("}\n");

    cg.printSection("LVariableTable Methods");
    cg_printf("virtual CVarRef getRefByIdx(ssize_t idx, Variant &k);\n");
    cg_printf("virtual ssize_t getIndex(const char *s, strhash_t prehash)"
              " const;\n");
    cg_printf("virtual Variant &getImpl(CStrRef s);\n");
    cg_printf("virtual bool exists(CStrRef s) const;\n");

  }
  cg_indentEnd("};\n");

  // generating scalar arrays
  cg.printSection("Scalar Arrays");
  if (system) {
    cg_printf("class SystemScalarArrays {\n");
  } else {
    cg_printf("class ScalarArrays : public SystemScalarArrays {\n");
  }
  cg_indentBegin("public:\n");
  cg_printf("static void initialize();\n");
  cg_printf("static void initializeNamed();\n");
  if (!system && Option::ScalarArrayFileCount > 1) {
    for (int i = 0; i < Option::ScalarArrayFileCount; i++) {
      cg_printf("static void initialize_%d();\n", i);
    }
  }
  cg_printf("\n");
  ar->outputCPPScalarArrayDecl(cg);
  cg_indentEnd("};\n");
  cg_printf("\n");
}

///////////////////////////////////////////////////////////////////////////////
// global state

void VariableTable::collectCPPGlobalSymbols(StringPairSetVec &symbols,
                                            CodeGenerator &cg,
                                            AnalysisResultPtr ar) {
  ASSERT(symbols.size() == AnalysisResult::GlobalSymbolTypeCount);
  ASSERT(!m_staticGlobals.empty() || m_staticGlobalsVec.empty());

  // static global variables
  StringPairSet *names = &symbols[AnalysisResult::KindOfStaticGlobalVariable];
  names->clear();
  for (unsigned int i = 0; i < m_symbolVec.size(); i++) {
    const string &name = m_symbolVec[i]->getName();
    names->insert(StringPair(
      Option::GlobalVariablePrefix + CodeGenerator::EscapeLabel(name),
      getGlobalVariableName(ar, name)));
  }

  // method static variables
  names = &symbols[AnalysisResult::KindOfMethodStaticVariable];
  for (StringToStaticGlobalInfoPtrMap::const_iterator iter =
       m_staticGlobals.begin(); iter != m_staticGlobals.end(); ++iter) {
    const string &id = iter->first;
    StaticGlobalInfoPtr sgi = iter->second;
    if (sgi->func) {
      string name = Option::StaticVariablePrefix + id;
      names->insert(StringPair(name, name));
    }
  }

  // class static variables
  names = &symbols[AnalysisResult::KindOfClassStaticVariable];
  for (StringToStaticGlobalInfoPtrMap::const_iterator iter =
       m_staticGlobals.begin(); iter != m_staticGlobals.end(); ++iter) {
    StaticGlobalInfoPtr sgi = iter->second;
    // id can change if we discover it is redeclared
    const string &id = StaticGlobalInfo::GetId(sgi->cls, sgi->func,
                                               sgi->sym->getName());
    if (!sgi->func) {
      ASSERT(!sgi->sym->isOverride());
      string name = Option::StaticPropertyPrefix + id;
      names->insert(StringPair(name, name));
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

void VariableTable::outputCPPGlobalVariablesImpl(CodeGenerator &cg,
                                                 AnalysisResultPtr ar) {
  bool system = (cg.getOutput() == CodeGenerator::SystemCPP);
  ASSERT(!m_staticGlobals.empty() || m_staticGlobalsVec.empty());

  if (!system) {
    cg_printf("IMPLEMENT_SMART_ALLOCATION(GlobalVariables)\n");
  }

  const char *clsname = system ? "SystemGlobals" : "GlobalVariables";
  cg_indentBegin("%s::%s() {\n", clsname, clsname);

  const char *prefix = system ? "stgv_" : "tgv_";
  cg_printf("memset(&%sbool, 0, sizeof(%sbool));\n", prefix, prefix);
  cg_printf("memset(&%sint, 0, sizeof(%sint));\n", prefix, prefix);
  cg_printf("memset(&%sint64, 0, sizeof(%sint64));\n", prefix, prefix);
  cg_printf("memset(&%sdouble, 0, sizeof(%sdouble));\n", prefix, prefix);

  cg_printf("memset(&%sRedeclaredCallInfoConstPtr, 0, "
            "sizeof(%sRedeclaredCallInfoConstPtr));\n",
            prefix, prefix);

  cg.printSection("Redeclared Classes");
  ar->outputCPPRedeclaredClassImpl(cg);

  cg_indentEnd("}\n");

  cg_printf("\n");
  // generating top level statements in system PHP files
  if (system) {
    cg_indentBegin("void SystemGlobals::initialize() {\n");
    cg_printf("Globals::initialize();\n");
    ar->outputCPPSystemImplementations(cg);
    cg_indentEnd("}\n");
  }

  if (!system) {
    cg_printf("\n");

    cg_indentBegin("void init_static_variables() {\n");
    cg_printf("ScalarArrays::initialize();\n");
    cg_printf("init_constant_table();\n");
    cg_indentEnd("}\n");
    cg_printf("static IMPLEMENT_THREAD_LOCAL"
              "(GlobalArrayWrapper, g_array_wrapper);\n");
    cg_printf(
      "#if defined(USE_GCC_FAST_TLS)\n"
      "static __thread GlobalVariables *g_variables;\n"
      "GlobalVariables *get_global_variables() {\n"
      "  ASSERT(g_variables);\n"
      "  return g_variables;\n"
      "}\n"
      "GlobalVariables *get_global_variables_check() {\n"
      "  if (!g_variables) g_variables = NEW(GlobalVariables)();\n"
      "  return g_variables;\n"
      "}\n"
      "void free_global_variables() {\n"
      "  if (g_variables) DELETE(GlobalVariables)(g_variables);\n"
      "  g_variables = NULL;\n"
      "  g_array_wrapper.destroy();\n"
      "}\n"
      "void free_global_variables_after_sweep() {\n"
      "  g_variables = NULL;\n"
      "  g_array_wrapper.destroy();\n"
      "}\n"
      "\n"
      "#else /* USE_GCC_FAST_TLS */\n"
      "static ThreadLocal<GlobalVariables *> g_variables;\n"
      "GlobalVariables *get_global_variables() {\n"
      "  GlobalVariables *g = *(g_variables.getNoCheck());\n"
      "  ASSERT(g);\n"
      "  return g;\n"
      "}\n"
      "GlobalVariables *get_global_variables_check() {\n"
      "  if (!*g_variables) *g_variables = NEW(GlobalVariables)();\n"
      "  return *g_variables;\n"
      "}\n"
      "void free_global_variables() {\n"
      "  GlobalVariables *g = *(g_variables.getNoCheck());\n"
      "  if (g) DELETE(GlobalVariables)(g);\n"
      "  g_variables.destroy();\n"
      "  g_array_wrapper.destroy();\n"
      "}\n"
      "void free_global_variables_after_sweep() {\n"
      "  g_variables.nullOut();\n"
      "  g_array_wrapper.destroy();\n"
      "}\n"
      "#endif /* USE_GCC_FAST_TLS */\n"
      "void init_global_variables() {\n"
      "  GlobalVariables *g = get_global_variables_check();\n"
      "  ThreadInfo::s_threadInfo->m_globals = g;\n"
      "  g->initialize();\n"
      "}\n");
    cg_printf("LVariableTable *get_variable_table() "
              "{ return (LVariableTable*)get_global_variables();}\n");
    cg_printf("Globals *get_globals() "
              "{ return (Globals*)get_global_variables();}\n");
    cg_printf("SystemGlobals *get_system_globals() "
              "{ return (SystemGlobals*)get_global_variables();}\n");
    cg_printf("Array get_global_array_wrapper()");
    cg_printf("{ return g_array_wrapper.get();}\n");
  }
}

void VariableTable::outputCPPGlobalVariablesDtorIncludes(CodeGenerator &cg,
                                                         AnalysisResultPtr ar) {
  ASSERT(!m_staticGlobals.empty() || m_staticGlobalsVec.empty());

  std::set<string> dtorIncludes;
  for (StringToStaticGlobalInfoPtrMap::const_iterator iter =
         m_staticGlobals.begin(); iter != m_staticGlobals.end(); ++iter) {
    StaticGlobalInfoPtr sgi = iter->second;
    if (!sgi->func) {
      ASSERT(!sgi->sym->isOverride());
      TypePtr varType = sgi->sym->getFinalType();
      if (varType->isSpecificObject()) {
        ClassScopePtr cls = varType->getClass(ar, sgi->cls);
        ASSERT(cls);
        if (cls->isUserClass()) {
          const string fileBase = cls->getContainingFile()->outputFilebase();
          if (dtorIncludes.find(fileBase) == dtorIncludes.end()) {
            cg_printInclude(fileBase + ".h");
            dtorIncludes.insert(fileBase);
          }
        }
      }
    }
  }
}

void VariableTable::outputCPPGlobalVariablesDtor(CodeGenerator &cg) {
  cg_printf("GlobalVariables::~GlobalVariables() {}\n");
}

void VariableTable::outputCPPGVHashTableGetImpl(CodeGenerator &cg,
                                                AnalysisResultPtr ar) {
  ASSERT(cg.getCurrentIndentation() == 0);
  const char text1[] =
    "class hashNodeGV {\n"
    "public:\n"
    "  hashNodeGV() {}\n"
    "  hashNodeGV(int64 h, const char *n, int64 l, int64 o, int64 i) :\n"
    "    hash(h), name(n), len(l), off(o), index(i), next(NULL) {}\n"
    "  int64 hash;\n"
    "  const char *name;\n"
    "  int64 len;\n"
    "  int64 off;\n"
    "  int64 index;\n"
    "  hashNodeGV *next;\n"
    "};\n"
    "static hashNodeGV *gvMapTable[%d];\n"
    "static hashNodeGV gvBuckets[%zd];\n"
    "\n"
    "#define GET_GV_OFFSET(n) (offsetof(GlobalVariables, n))\n"
    "const char *gvMapData[] = {\n";

  const char text2[] =
    "  NULL, NULL, NULL, NULL,\n"
    "};\n\n"
    "static class GVTableInitializer {\n"
    "  public: GVTableInitializer() {\n"
    "    hashNodeGV *b = gvBuckets;\n"
    "    for (const char **s = gvMapData; *s; s++, b++) {\n"
    "      const char *name = *s++;\n"
    "      int64 len = (int64)(*s++);\n"
    "      int64 off = (int64)(*s++);\n"
    "      int64 index = (int64)(*s);\n"
    "      int64 hash = hash_string(name, len);\n"
    "      hashNodeGV *node = new(b) hashNodeGV\n"
    "        (hash, name, len, off, index);\n"
    "      int h = hash & %d;\n"
    "      if (gvMapTable[h]) node->next = gvMapTable[h];\n"
    "      gvMapTable[h] = node;\n"
    "    }\n"
    "  }\n"
    "} gv_table_initializer;\n"
    "\n"
    "static inline const hashNodeGV *\n"
    "findGV(const char *name, int64 hash) {\n"
    "  for (const hashNodeGV *p = gvMapTable[hash & %d];\n"
    "       p; p = p->next) {\n"
    "    if (p->hash == hash && strcmp(p->name, name) == 0) return p;\n"
    "  }\n"
    "  return NULL;\n"
    "}\n"
    "static inline const hashNodeGV *\n"
    "findGV(const char *name, int64 len, int64 hash) {\n"
    "  for (const hashNodeGV *p = gvMapTable[hash & %d];\n"
    "       p; p = p->next) {\n"
    "    if (p->hash == hash && p->len == len &&\n"
    "        memcmp(p->name, name, len) == 0) {\n"
    "      return p;\n"
    "    }\n"
    "  }\n"
    "  return NULL;\n"
    "}\n"
    "\n";

  const char text3[] =
    "Variant &GlobalVariables::getImpl(CStrRef s) {\n"
    "  const hashNodeGV *p = findGV(s.data(), s.size(), s->hash());\n"
    "  if (p) return *(Variant *)((char *)this + p->off);\n"
    "  return LVariableTable::getImpl(s);\n"
    "}\n";

  cg.ifdefBegin(false, "OMIT_JUMP_TABLE_GLOBAL_GETIMPL");
  int tableSize = Util::roundUpToPowerOfTwo(m_symbolVec.size() * 2);
  cg_printf(text1, tableSize, m_symbolVec.size());
  for (unsigned int i = 0; i < m_symbolVec.size(); i++) {
    const string &name = m_symbolVec[i]->getName();
    string escaped = CodeGenerator::EscapeLabel(name);
    string varName = string("gvm_") + CodeGenerator::FormatLabel(name);
    cg_printf("  (const char *)\"%s\",\n"
              "  (const char *)%lld,\n"
              "  (const char *)GET_GV_OFFSET(%s),\n"
              "  (const char *)%u,\n",
              escaped.c_str(),
              (int64)name.size(),
              varName.c_str(),
              i);
  }
  cg_printf(text2, tableSize - 1, tableSize - 1, tableSize - 1);
  cg_printf(text3);
  cg.ifdefEnd("OMIT_JUMP_TABLE_GLOBAL_GETIMPL");
}

void VariableTable::outputCPPGlobalVariablesGetImpl(CodeGenerator &cg,
                                                    AnalysisResultPtr ar) {
  outputCPPGVHashTableGetImpl(cg, ar);
}

void VariableTable::outputCPPGVHashTableExists(CodeGenerator &cg,
                                               AnalysisResultPtr ar) {
  ASSERT(cg.getCurrentIndentation() == 0);
  cg.ifdefBegin(false, "OMIT_JUMP_TABLE_GLOBAL_EXISTS");
  const char text[] =
    "HOT_FUNC_HPHP\n"
    "bool GlobalVariables::exists(CStrRef s) const {\n"
    "  const hashNodeGV *p = findGV(s.data(), s.size(), s->hash());\n"
    "  if (p) return isInitialized(*(Variant *)((char *)this + p->off));\n"
    "  if (!LVariableTable::exists(s)) return false;\n"
    "  return isInitialized(const_cast<GlobalVariables*>(this)->get(s));\n"
    "}\n";
  cg_printf(text);
  cg.ifdefEnd("OMIT_JUMP_TABLE_GLOBAL_EXISTS");
}

void VariableTable::outputCPPGlobalVariablesExists(CodeGenerator &cg,
                                                   AnalysisResultPtr ar) {
  outputCPPGVHashTableExists(cg, ar);
}

void VariableTable::outputCPPGVHashTableGetIndex(CodeGenerator &cg,
                                                 AnalysisResultPtr ar) {
  ASSERT(cg.getCurrentIndentation() == 0);
  cg.ifdefBegin(false, "OMIT_JUMP_TABLE_GLOBAL_GETINDEX");
  const char text[] =
    "ssize_t GlobalVariables::getIndex(const char* s, strhash_t hash) const {\n"
    "  const GlobalVariables *g ATTRIBUTE_UNUSED = this;\n"
    "  if (hash < 0) hash = hash_string(s);\n"
    "  const hashNodeGV *p = findGV(s, hash);\n"
    "  if (p) return p->index;\n"
    "  return m_px ? (m_px->getIndex(s) + %zd) : -1;\n"
    "}\n";
  cg_printf(text, m_symbolVec.size());
  cg.ifdefEnd("OMIT_JUMP_TABLE_GLOBAL_GETINDEX");
}

void VariableTable::outputCPPGlobalVariablesGetIndex(CodeGenerator &cg,
                                                     AnalysisResultPtr ar) {
  outputCPPGVHashTableGetIndex(cg, ar);
}

void VariableTable::outputCPPGlobalVariablesMethods(CodeGenerator &cg,
                                                    AnalysisResultPtr ar) {
  SymbolSet variants;
  int maxIdx = m_symbolVec.size();
  int maxSysIdx = 0;
  bool sysDone = false;
  for (unsigned int i = 0; i < m_symbolVec.size(); i++) {
    const Symbol *sym = m_symbolVec[i];
    if (sym->isSystem()) {
      assert(!sysDone);
      variants.insert(string("gvm_") +
                             CodeGenerator::FormatLabel(sym->getName()));
      maxSysIdx++;
    } else {
      sysDone = true;
    }
  }

  checkSystemGVOrder(variants, maxSysIdx);
  ASSERT(cg.getCurrentIndentation() == 0);
  const char text[] =
    "\n"
    "CVarRef GlobalVariables::getRefByIdx(ssize_t idx, Variant &k) {\n"
    "  GlobalVariables *g ATTRIBUTE_UNUSED = this;\n"
    "  if (idx >= 0 && idx < %d) {\n"
    "    k = gvMapData[idx << 2]; // idx * 4\n"
    "    if (idx < %d) {\n"
    "      return g->stgv_Variant[idx];\n"
    "    } else {\n"
    "      return g->tgv_Variant[idx - %d];\n"
    "    }\n"
    "  }\n"
    "  return Globals::getRefByIdx(idx, k);\n"
    "}\n";

  cg_printf(text, maxIdx, maxSysIdx, maxSysIdx);
}

void VariableTable::outputCPPVariableInit(CodeGenerator &cg,
                                          AnalysisResultPtr ar,
                                          bool inPseudoMain,
                                          const string &name) {
  if (inPseudoMain) {
    cg_printf(" ATTRIBUTE_UNUSED = ");
    if (cg.getOutput() != CodeGenerator::SystemCPP) {
      cg_printf("(variables != gVariables) ? variables->get(");
      cg_printString(name, ar, getBlockScope());
      cg_printf(") : ");
    }
    cg_printf("g->");
    if (cg.getOutput() != CodeGenerator::SystemCPP) {
      cg_printf(getGlobalVariableName(ar, name).c_str());
    } else {
      cg_printf("GV(%s)", name.c_str());
    }
  }
}

void VariableTable::outputCPPImpl(CodeGenerator &cg, AnalysisResultPtr ar) {
  bool inPseudoMain = isPseudoMainTable();
  if (inPseudoMain) {
    ASSERT(m_forcedVariants);
    cg_printf("LVariableTable *gVariables ATTRIBUTE_UNUSED = "
              "(LVariableTable *)g;\n");
  }

  bool isGenScope = false;
  FunctionScopeRawPtr func;
  if (m_blockScope.is(BlockScope::FunctionScope)) {
    func = FunctionScopeRawPtr(static_cast<FunctionScope*>(&m_blockScope));
    if (func->isGenerator()) {
      isGenScope = true;
    }
  }

  ParameterExpressionPtrVec useVars;
  if (func->needsAnonClosureClass(useVars)) {
    cg_printf("%sClosure$%s *closure ATTRIBUTE_UNUSED = "
              "(%sClosure$%s*)extra;\n",
              Option::ClassPrefix,
              CodeGenerator::FormatLabel(func->getName()).c_str(),
              Option::ClassPrefix,
              CodeGenerator::FormatLabel(func->getName()).c_str());
  }

  if (isGenScope) {
    const string &name = CodeGenerator::FormatLabel(m_blockScope.getName());
    cg_printf("%sContinuation$%s *%s ATTRIBUTE_UNUSED = "
              "(%sContinuation$%s*) %s%s.get();\n",
              Option::ClassPrefix,    name.c_str(),
              TYPED_CONTINUATION_OBJECT_NAME,
              Option::ClassPrefix,    name.c_str(),
              Option::VariablePrefix, CONTINUATION_OBJECT_NAME);
  }

  bool declared = false;
  for (unsigned int i = 0; i < m_symbolVec.size(); i++) {
    const Symbol *sym = m_symbolVec[i];
    const string &name = sym->getName();
    string fname = CodeGenerator::FormatLabel(name);
    if (sym->isShrinkWrapped()) continue;
    if (sym->isSystem() && cg.getOutput() != CodeGenerator::SystemCPP) {
      continue;
    }

    if (sym->isStatic()) {
      string id = StaticGlobalInfo::GetId
        (getClassScope(), getFunctionScope(), name);

      TypePtr type = sym->getFinalType();
      type->outputCPPDecl(cg, ar, getBlockScope());
      if (ClassScope::NeedStaticArray(getClassScope(), getFunctionScope())) {
        const char *cname = getFunctionScope()->isStatic() ? "cls" :
          "this->o_getClassName()";
        cg_printf(" &%s%s ATTRIBUTE_UNUSED = "
                  "g->%s%s.lvalAt(%s);\n",
                  Option::StaticVariablePrefix, fname.c_str(),
                  Option::StaticVariablePrefix, id.c_str(),
                  cname);
        cg_printf("Variant &%s%s%s ATTRIBUTE_UNUSED = "
                  "g->%s%s%s.lvalAt(%s);\n",
                  Option::InitPrefix, Option::StaticVariablePrefix,
                  fname.c_str(),
                  Option::InitPrefix, Option::StaticVariablePrefix,
                  id.c_str(), cname);
      } else {
        const char *g =
          getFunctionScope()->isClosure() ?
            "closure" :
            getFunctionScope()->isGeneratorFromClosure() ?
              TYPED_CONTINUATION_OBJECT_NAME :
              "g";
        cg_printf(" &%s%s ATTRIBUTE_UNUSED = %s->%s%s;\n",
                  Option::StaticVariablePrefix, fname.c_str(),
                  g, Option::StaticVariablePrefix, id.c_str());
        cg_printf("bool &%s%s%s ATTRIBUTE_UNUSED = %s->%s%s%s;\n",
                  Option::InitPrefix, Option::StaticVariablePrefix,
                  fname.c_str(),
                  g, Option::InitPrefix, Option::StaticVariablePrefix,
                  id.c_str());
      }

      if (needLocalCopy(sym) && !sym->isParameter() && !isGenScope) {
        type->outputCPPDecl(cg, ar, getBlockScope());
        cg_printf(" %s%s;\n", Option::VariablePrefix,
                  fname.c_str());
        declared = true;
      }
      continue;
    }

    if (sym->isParameter()) continue;

    const char* prefix = "";
    if (inPseudoMain && !sym->isHidden()) prefix = "&";

    if (sym->isGlobal()) {
      TypePtr type = sym->getFinalType();
      type->outputCPPDecl(cg, ar, getBlockScope());
      cg_printf(" &%s%s ATTRIBUTE_UNUSED = g->%s;\n",
                Option::GlobalVariablePrefix, fname.c_str(),
                getGlobalVariableName(ar, name).c_str());

      if (needLocalCopy(name) && !isGenScope) {
        type->outputCPPDecl(cg, ar, getBlockScope());
        cg_printf(" %s%s%s", prefix, Option::VariablePrefix,
                  fname.c_str());
        outputCPPVariableInit(cg, ar, inPseudoMain, name);
        cg_printf(";\n");
        declared = true;
      }
      continue;
    }

    bool condition = isGenScope ?
      // omit local variables in continuations (since they will
      // be part of the continuation object). we however need
      // the hidden variables which are used and/or needed
      (sym->isHidden() && (sym->isUsed() || sym->isNeeded())) :

      // local variables
      (((getAttribute(ContainsDynamicVariable) || inPseudoMain) &&
         !sym->isHidden()) ||
        sym->isUsed() || sym->isNeeded());

    if (condition) {
      TypePtr type = sym->getFinalType();
      type->outputCPPDecl(cg, ar, getBlockScope());
      cg_printf(" %s%s%s", prefix, getVariablePrefix(sym),
                fname.c_str());
      if (inPseudoMain && !sym->isHidden()) {
        outputCPPVariableInit(cg, ar, inPseudoMain, name);
      } else {
        const char *initializer = type->getCPPInitializer();
        if (initializer) {
          cg_printf(" = %s", initializer);
        }
      }
      cg_printf(";\n");
      declared = true;
    }
  }

  if (declared) {
    cg_printf("\n");
  }

  if (Option::GenerateCPPMacros && getAttribute(ContainsDynamicVariable) &&
      cg.getOutput() != CodeGenerator::SystemCPP && !inPseudoMain) {
    const char *paramPrefix = isGenScope ?
      TYPED_CONTINUATION_OBJECT_NAME "->" : "";
    outputCPPVariableTable(cg, ar, paramPrefix);
    ar->m_variableTableFunctions.insert(getScopePtr()->getName());
  }
}

void VariableTable::outputCPPVariableTable(CodeGenerator &cg,
                                           AnalysisResultPtr ar,
                                           const char *paramPrefix) {
  bool inGlobalScope = isGlobalTable(ar);

  string varDecl, initializer, memDecl, params;
  for (unsigned int i = 0; i < m_symbolVec.size(); i++) {
    const Symbol *sym = m_symbolVec[i];
    if (sym->isHidden()) continue;
    const string &name = sym->getName();
    string varName = string(getVariablePrefix(sym)) +
      CodeGenerator::FormatLabel(name);
    TypePtr type = sym->getFinalType();
    if (!inGlobalScope) {
      if (!varDecl.empty()) {
        varDecl += ", ";
        initializer += ", ";
        memDecl += "; ";
        params += ", ";
      }
      varDecl += type->getCPPDecl(ar, getBlockScope()) + " &" +
        Option::TempVariablePrefix + CodeGenerator::FormatLabel(name);
      initializer += varName + "(" + Option::TempVariablePrefix +
        CodeGenerator::FormatLabel(name) + ")";
      memDecl += type->getCPPDecl(ar, getBlockScope()) + " &" + varName;
      params += string(paramPrefix) + varName;
    }
  }

  cg_printf("\n");
  if (m_forcedVariants) {
    cg_printf("class VariableTable : public LVariableTable {\n");
  } else {
    cg_printf("class VariableTable : public RVariableTable {\n");
  }
  cg_indentBegin("public:\n");
  if (!inGlobalScope) {
    cg_printf("%s;\n", memDecl.c_str());
    if (!initializer.empty()) {
      cg_printf("VariableTable(%s) : %s {}\n", varDecl.c_str(),
                initializer.c_str());
    } else {
      cg_printf("VariableTable(%s) {}\n", varDecl.c_str());
    }
  }

  if (m_forcedVariants) {
    cg_indentBegin("virtual Variant &getImpl(CStrRef s) {\n");
    if (!outputCPPJumpTable(cg, ar, NULL, true, true, EitherStatic,
                            JumpReturnString)) {
      m_emptyJumpTables.insert(JumpTableLocalGetImpl);
    }
    cg_printf("return LVariableTable::getImpl(s);\n");
    cg_indentEnd("}\n");

    if (getAttribute(ContainsExtract)) {
      cg_indentBegin("virtual bool exists(CStrRef s) const {\n");
      if (!outputCPPJumpTable(cg, ar, NULL, true, false,
                              EitherStatic, JumpInitializedString)) {
        m_emptyJumpTables.insert(JumpTableLocalExists);
      }
      cg_printf("return LVariableTable::exists(s);\n");
      cg_indentEnd("}\n");
    }
  } else {
    cg_indentBegin("virtual Variant getImpl(CStrRef s) {\n");
    if (!outputCPPJumpTable(cg, ar, NULL, true, false, EitherStatic,
                            JumpReturnString)) {
      m_emptyJumpTables.insert(JumpTableLocalGetImpl);
    }
    // Valid variable names cannot be numerical.
    cg_printf("return rvalAt(s, AccessFlags::Key);\n");
    cg_indentEnd("}\n");

    if (getAttribute(ContainsCompact)) {
      cg_indentBegin("virtual bool exists(CStrRef s) const {\n");
      if (!outputCPPJumpTable(cg, ar, NULL, true, false,
                              EitherStatic, JumpInitializedString)) {
        m_emptyJumpTables.insert(JumpTableLocalExists);
      }
      cg_printf("return RVariableTable::exists(s);\n");
      cg_indentEnd("}\n");
    }
  }

  if (getAttribute(ContainsGetDefinedVars)) {
    if (m_forcedVariants) {
      cg_indentBegin("virtual Array getDefinedVars() {\n");
    } else {
      cg_indentBegin("virtual Array getDefinedVars() const {\n");
    }
    cg_printf("Array ret = %sVariableTable::getDefinedVars();\n",
              m_forcedVariants ? "L" : "R");
    for (unsigned int i = 0; i < m_symbolVec.size(); i++) {
      Symbol *sym = m_symbolVec[i];
      if (sym->isHidden()) continue;
      const string &name = sym->getName();
      const char *prefix = getVariablePrefix(sym);
      string varName;
      if (prefix == Option::GlobalVariablePrefix) {
        varName = string("g->") + getGlobalVariableName(ar, name);
      } else {
        varName = string(prefix) + CodeGenerator::FormatLabel(name);
      }
      cg_printf("if (%s.isInitialized()) ret.lval(\"%s\").setWithRef(%s);\n",
                varName.c_str(), CodeGenerator::EscapeLabel(name).c_str(),
                varName.c_str());
    }
    cg_printf("return ret;\n");
    cg_indentEnd("}\n");
  }

  if (!inGlobalScope) {
    if (!params.empty()) {
      cg_indentEnd("} variableTable(%s);\n", params.c_str());
    } else {
      cg_indentEnd("} variableTable;\n");
    }
    cg_printf("%sVariableTable* ATTRIBUTE_UNUSED "
              "variables = &variableTable;\n",
              m_forcedVariants ? "L" : "R");
  } else {
    cg_indentEnd("};\n");
    cg_printf("static IMPLEMENT_THREAD_LOCAL(VariableTable, "
              "g_variable_tables);\n");
    if (m_forcedVariants) {
      cg_printf("LVariableTable *get_variable_table() "
                "{ return g_variable_tables.get();}\n");
    } else {
      cg_printf("RVariableTable *get_variable_table() "
                "{ return g_variable_tables.get();}\n");
    }
  }
}

void VariableTable::outputCPP(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (isGlobalTable(ar)) {
    if (cg.getContext() == CodeGenerator::CppImplementation ||
        cg.getContext() == CodeGenerator::CppPseudoMain) {
      outputCPPGlobalVariablesImpl(cg, ar);
    } else {
      outputCPPGlobalVariablesHeader(cg, ar);
    }
  } else {
    outputCPPImpl(cg, ar);
  }
}

bool VariableTable::outputCPPPropertyDecl(CodeGenerator &cg,
    AnalysisResultPtr ar, bool dynamicObject /* = false */) {
  bool destruct = false;
  for (unsigned int i = 0; i < m_symbolVec.size(); i++) {
    const Symbol *sym = m_symbolVec[i];
    if (dynamicObject && !sym->isPrivate()) continue;

    // we don't redefine a property that's already defined by a parent class
    // unless it is private or the parent's one is private
    if (sym->isStatic() || sym->isOverride()) continue;

    destruct = true;
    const string &name = sym->getName();
    sym->getFinalType()->outputCPPDecl(cg, ar, getBlockScope());
    cg_printf(" %s%s;\n", Option::PropertyPrefix,
              CodeGenerator::FormatLabel(name).c_str());
  }

  return destruct;
}

bool VariableTable::outputCPPPrivateSelector(CodeGenerator &cg,
                                             AnalysisResultPtr ar,
                                             const char *op, const char *args) {
  ClassScopePtr cls = getClassScope();
  vector<const char *> classes;
  do {
    // Note: outputCPPPrivateSelector() is only used for non-static properties.
    if (cls->getVariables()->hasNonStaticPrivate()) {
      classes.push_back(cls->getOriginalName().c_str());
    }
    cls = cls->getParentScope(ar);
  } while (cls && !cls->isRedeclaring()); // allow current class to be redec
  if (classes.empty()) return false;

  cg_printf("CStrRef s = context.isNull() ? "
            "FrameInjection::GetClassName(false) : context;\n");
  for (JumpTable jt(cg, classes, true, false, true); jt.ready(); jt.next()) {
    const char *name = jt.key();
    if (!strcasecmp(name, getClassScope()->getOriginalName().c_str())) {
      cg_printf("HASH_GUARD_STRING(" STRHASH_FMT ", %s) "
                "{ return %s%sPrivate(prop%s); }\n",
                hash_string(name), name, Option::ObjectPrefix, op, args);
    } else {
      cg_printf("HASH_GUARD_STRING(" STRHASH_FMT ", %s) "
                "{ return %s%s::%s%sPrivate(prop%s); }\n",
                hash_string(name), name, Option::ClassPrefix,
                name, Option::ObjectPrefix, op, args);
    }
  }
  return true;
}

bool VariableTable::outputCPPJumpTable(CodeGenerator &cg, AnalysisResultPtr ar,
      const char *prefix, bool defineHash, bool variantOnly,
      StaticSelection staticVar, JumpTableType type /* = JumpReturn */,
      PrivateSelection privateVar /* = NonPrivate */,
      bool *declaredGlobals /* = NULL */) {
  if (declaredGlobals) *declaredGlobals = false;

  vector<const char *> strings;
  hphp_const_char_map<ssize_t> varIdx;
  strings.reserve(m_symbolVec.size());
  bool hasStatic = false;
  for (unsigned int i = 0; i < m_symbolVec.size(); i++) {
    const Symbol *sym = m_symbolVec[i];
    const string &name = sym->getName();
    bool stat = sym->isStatic();
    if (sym->isOverride() || sym->isHidden()) continue;
    if (!stat && isInherited(name)) continue;
    if (!stat &&
        (sym->isPrivate() && privateVar == NonPrivate ||
         !sym->isPrivate() && privateVar == Private)) continue;

    if ((!variantOnly || Type::SameType(sym->getFinalType(), Type::Variant)) &&
        (staticVar & (stat ? Static : NonStatic))) {
      hasStatic |= stat;
      if (type == JumpIndex) varIdx[name.c_str()] = strings.size();
      strings.push_back(name.c_str());
    }
  }
  if (strings.empty()) return false;

  if (hasStatic) {
    cg.printDeclareGlobals();
    if (declaredGlobals) *declaredGlobals = true;
  }

  bool useString = (type == JumpSet) ||
                   (type == JumpReturnString) ||
                   (type == JumpInitializedString);

  for (JumpTable jt(cg, strings, false, !defineHash, useString); jt.ready();
       jt.next()) {
    const char *name = jt.key();
    const char *symbol_prefix =
      prefix ? prefix : getVariablePrefix(name);
    string varName;
    if (prefix == Option::StaticPropertyPrefix) {
      varName = string(prefix) + getClassScope()->getId() +
        Option::IdPrefix + CodeGenerator::FormatLabel(name);
    } else {
      varName = string(symbol_prefix) + CodeGenerator::FormatLabel(name);
    }
    if (symbol_prefix == Option::GlobalVariablePrefix) {
      varName = string("g->") + getGlobalVariableName(ar, name);
    } else if (symbol_prefix != Option::VariablePrefix &&
               symbol_prefix != Option::PropertyPrefix) {
      varName = string("g->") + varName;
    }
    switch (type) {
      case VariableTable::JumpReturn:
        cg_printf("HASH_RETURN(" STRHASH_FMT ", %s,\n",
                  hash_string(name), varName.c_str());
        cg_printf("            \"%s\");\n",
                  CodeGenerator::EscapeLabel(name).c_str());
        break;
      case VariableTable::JumpSet:
        cg_printf("HASH_SET_STRING(" STRHASH_FMT ", %s,\n",
                  hash_string(name), varName.c_str());
        cg_printf("                \"%s\", %lu);\n",
                  CodeGenerator::EscapeLabel(name).c_str(), strlen(name));
        break;
      case VariableTable::JumpInitialized:
        cg_printf("HASH_INITIALIZED(" STRHASH_FMT ", %s,\n",
                  hash_string(name), varName.c_str());
        cg_printf("                 \"%s\");\n",
                  CodeGenerator::EscapeLabel(name).c_str());
        break;
      case VariableTable::JumpInitializedString: {
        int index = -1;
        int stringId = cg.checkLiteralString(name, index, ar, getBlockScope());
        assert(index >= 0);
        string lisnam = ar->getLiteralStringName(stringId, index);
        cg_printf("HASH_INITIALIZED_NAMSTR(" STRHASH_FMT ", %s, %s,\n",
                  hash_string(name), lisnam.c_str(), varName.c_str());
        cg_printf("                   %lu);\n", strlen(name));
        break;
      }
      case VariableTable::JumpIndex: {
        hphp_const_char_map<ssize_t>::const_iterator it = varIdx.find(name);
        ASSERT(it != varIdx.end());
        ssize_t idx = it->second;
        cg_printf("HASH_INDEX(" STRHASH_FMT ", \"%s\", %ld);\n",
                  hash_string(name),
                  CodeGenerator::EscapeLabel(name).c_str(), idx);
        break;
      }
      case VariableTable::JumpReturnString: {
        int index = -1;
        int stringId = cg.checkLiteralString(name, index, ar, getBlockScope());
        assert(index >= 0);
        string lisnam = ar->getLiteralStringName(stringId, index);
        cg_printf("HASH_RETURN_NAMSTR(" STRHASH_FMT ", %s, %s,\n",
                  hash_string(name), lisnam.c_str(), varName.c_str());
        cg_printf("                   %lu);\n", strlen(name));
        break;
      }
    }
  }

  return true;
}

void VariableTable::outputCPPClassMap(CodeGenerator &cg,
                                      AnalysisResultPtr ar) {
  for (unsigned int i = 0; i < m_symbolVec.size(); i++) {
    const Symbol *sym = m_symbolVec[i];
    const string &name = sym->getName();

    int attribute = ClassInfo::IsNothing;
    if (sym->isProtected()) {
      attribute |= ClassInfo::IsProtected;
    } else if (sym->isPrivate()) {
      attribute |= ClassInfo::IsPrivate;
    } else {
      attribute |= ClassInfo::IsPublic;
    }
    if (sym->isStatic()) {
      attribute |= ClassInfo::IsStatic;
    }

    cg_printf("(const char *)0x%04X, \"%s\",\n", attribute,
              CodeGenerator::EscapeLabel(name).c_str());
  }
  cg_printf("NULL,\n");
}

void VariableTable::outputCPPStaticVariables(CodeGenerator &cg,
                                             AnalysisResultPtr ar) {
  for (unsigned int i = 0; i < m_symbolVec.size(); i++) {
    const Symbol *sym = m_symbolVec[i];
    const string &name = sym->getName();
    if (sym->isStatic()) {
      ExpressionPtr initValue =
        dynamic_pointer_cast<Expression>(getStaticInitVal(name));
      Variant v;
      if (initValue->getScalarValue(v)) {
        int len;
        string output = getEscapedText(v, len);
        // This isn't right, we should store the location of the
        // static variable in order to get the current value (as opposed to
        // the initial value) at runtime
        cg_printf("\"%s\", (const char *)%d, \"%s\",\n",
                  CodeGenerator::EscapeLabel(name).c_str(),
                  len, output.c_str());
      }
    }
  }
  cg_printf("NULL,\n");
}

void VariableTable::outputCPPStaticLocals(CodeGenerator &cg,
                                          AnalysisResultPtr ar,
                                          bool forInitList) {
  bool hasEmit = false;
  for (SymbolVec::const_iterator it = m_staticLocalsVec.begin();
       it != m_staticLocalsVec.end(); ++it) {
    const Symbol *sym = *it;
    ASSERT(sym->getDeclaration());
    FunctionScopeRawPtr func = m_blockScope.getContainingFunction();
    ASSERT(func && (func->isClosure() || func->isGeneratorFromClosure()));
    const string &id = StaticGlobalInfo::GetId(ClassScopePtr(),
                                               func, sym->getName());
    TypePtr varType(sym->getFinalType());

    if (!forInitList) {
      // static variable
      varType->outputCPPDecl(cg, ar, BlockScopeRawPtr());
      cg_printf(" %s%s;\n", Option::StaticVariablePrefix, id.c_str());

      // initializer
      cg_printf("bool %s%s%s;\n",
                Option::InitPrefix, Option::StaticVariablePrefix, id.c_str());
    } else {
      if (hasEmit) cg_printf(", ");
      // only initializer
      cg_printf("%s%s%s(false)",
                Option::InitPrefix, Option::StaticVariablePrefix, id.c_str());
    }
    hasEmit = true;
  }
}

///////////////////////////////////////////////////////////////////////////////
}
