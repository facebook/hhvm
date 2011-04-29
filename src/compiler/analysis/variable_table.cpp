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

using namespace std;
using namespace boost;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// StaticGlobalInfo

string VariableTable::StaticGlobalInfo::getId
(CodeGenerator &cg, ClassScopePtr cls, FunctionScopePtr func,
 const string &name) {
  ASSERT(cls || func);

  // format: <class>$$<func>$$name
  string id;
  if (cls) {
    id += cls->getId(cg);
    id += Option::IdPrefix;
  }
  if (func) {
    id += func->getId(cg);
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

bool VariableTable::definedByParent(AnalysisResultConstPtr ar,
                                    const string &name) {
  if (isPrivate(name)) return false;
  ClassScopePtr cls = findParent(ar, name);
  if (cls) {
    return !cls->getVariables()->isPrivate(name);
  } else {
    return false;
  }
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

  if (sym->isHidden()) return Option::HiddenVariablePrefix;
  return Option::VariablePrefix;
}

string VariableTable::getVariableName(CodeGenerator &cg,
                                      AnalysisResultConstPtr ar,
                                      const string &name) const {
  const Symbol *sym = getSymbol(name);
  if (sym && sym->isStatic()) {
    if (!needLocalCopy(sym)) {
      return string(Option::StaticVariablePrefix) + cg.formatLabel(name);
    }
    return string(Option::VariablePrefix) + cg.formatLabel(name);
  }

  if (getAttribute(ForceGlobal)) {
    return getGlobalVariableName(cg, ar, name);
  }

  if (sym->isGlobal()) {
    if (!needLocalCopy(sym)) {
      return getGlobalVariableName(cg, ar, name);
    }
  }
  return (sym->isHidden() ?
          Option::HiddenVariablePrefix : Option::VariablePrefix) +
    cg.formatLabel(name);
}

string
VariableTable::getGlobalVariableName(CodeGenerator &cg,
                                     AnalysisResultConstPtr ar,
                                     const string &name) const {
  return string("GV(") + cg.formatLabel(name) + ")";
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
  Symbol *sym = addSymbol(name);
  if (!sym->isParameter()) {
    sym->setParameterIndex(m_nextParam++);
  }
  return type ?
    add(sym, type, false, ar, construct, ModifierExpressionPtr()) : type;
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

  VariableTablePtr globalVariables = ar->getVariables();
  StaticGlobalInfoPtr sgi(new StaticGlobalInfo());
  sgi->sym = sym;
  sgi->variables = this;
  sgi->cls = getClassScope();
  sgi->func = member ? FunctionScopeRawPtr() : getFunctionScope();

  globalVariables->m_staticGlobalsVec.push_back(sgi);
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

void VariableTable::markOverride(AnalysisResultPtr ar, const string &name) {
  Symbol *sym = getSymbol(name);
  assert(sym && sym->isPresent());
  if (!sym->isStatic() ||
      (sym->isPublic() && !sym->getClassInitVal())) {
    ClassScopePtr parent = findParent(ar, name);
    if (parent) {
      Symbol *s2 = parent->getVariables()->getSymbol(name);
      assert(s2);
      if (!s2->isPrivate()) {
        if (!sym->isStatic() || s2->isProtected()) {
          sym->setOverride();
        }
      }
    }
  }
}

TypePtr VariableTable::add(const string &name, TypePtr type,
                           bool implicit, AnalysisResultConstPtr ar,
                           ConstructPtr construct,
                           ModifierExpressionPtr modifiers,
                           bool checkError /* = true */) {
  return add(addSymbol(name), type, implicit, ar,
             construct, modifiers, checkError);
}

TypePtr VariableTable::add(Symbol *sym, TypePtr type,
                           bool implicit, AnalysisResultConstPtr ar,
                           ConstructPtr construct,
                           ModifierExpressionPtr modifiers,
                           bool checkError /* = true */) {
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
                                ar, construct, modifiers, false);
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
                                    construct, modifiers, checkError);
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
                                     ConstructPtr construct, int &properties) {
  return checkVariable(addSymbol(name), type,
                       coerce, ar, construct, properties);
}

TypePtr VariableTable::checkVariable(Symbol *sym, TypePtr type,
                                     bool coerce, AnalysisResultConstPtr ar,
                                     ConstructPtr construct, int &properties) {
  properties = 0;

  // Variable used in pseudomain
  if (!sym->isHidden() && isPseudoMainTable()) {
    // only need to do this once... should mark the sym.
    ar->lock()->getVariables()->checkVariable(sym->getName(), type,
                                              coerce, ar, construct,
                                              properties);
  }

  if (!sym->declarationSet()) {
    bool isLocal = !sym->isGlobal() && !sym->isSystem();
    if (isLocal && !getAttribute(ContainsLDynamicVariable) &&
        m_blockScope.isFirstPass()) {
      type = Type::Variant;
      coerce = true;
    }

    type = setType(ar, sym, type, coerce);
    sym->setDeclaration(construct);
    return type;
  }

  properties = VariablePresent;
  if (sym->isStatic()) {
    properties |= VariableStatic;
  }

  return setType(ar, sym, type, coerce);
}

Symbol *VariableTable::findProperty(ClassScopePtr &cls,
                                    const string &name,
                                    AnalysisResultConstPtr ar,
                                    ConstructPtr construct) {
  Symbol *sym = getSymbol(name);
  if (sym && sym->declarationSet()) {
    if (!sym->isOverride() || !sym->isStatic()) {
      return sym;
    }
    sym = NULL;
  }

  if (!sym) {
    if (ClassScopePtr parent = findParent(ar, name)) {
      sym = parent->findProperty(parent, name, ar, construct);
      if (sym) {
        cls = parent;
        return sym;
      }
    }

    if (!cls) {
      sym = addSymbol(name);
    }
  }

  return sym;
}

TypePtr VariableTable::checkProperty(Symbol *sym, TypePtr type,
                                     bool coerce, AnalysisResultConstPtr ar) {
  if (sym->isOverride()) {
    ClassScopePtr parent = findParent(ar, sym->getName());
    assert(parent);
    VariableTablePtr variables = parent->getVariables();
    Symbol *base = variables->getSymbol(sym->getName());
    assert(base && !base->isPrivate());
    type = variables->setType(ar, base, type, coerce);
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

void VariableTable::forceVariants(AnalysisResultConstPtr ar, int varClass) {
  int mask = varClass & ~m_forcedVariants;
  if (mask) {
    if (!m_hasPrivate) mask &= ~AnyPrivateVars;
    if (!m_hasStatic) mask &= ~AnyStaticVars;

    if (mask) {
      for (unsigned int i = 0; i < m_symbolVec.size(); i++) {
        Symbol *sym = m_symbolVec[i];
        if (!sym->isHidden() && sym->declarationSet() &&
            mask & GetVarClassMask(sym->isPrivate(), sym->isStatic())) {
          setType(ar, sym, Type::Variant, true);
          sym->setIndirectAltered();
        }
      }
    }
    m_forcedVariants |= varClass;

    ClassScopePtr parent = m_blockScope.getParentScope(ar);
    if (parent) {
      parent->getVariables()->forceVariants(ar, varClass & ~AnyPrivateVars);
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
        mask & GetVarClassMask(sym->isPrivate(), sym->isStatic())) {
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
  int mask = GetVarClassMask(sym->isPrivate(), sym->isStatic());
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
                                        const string &name) {
  for (ClassScopePtr parent = m_blockScope.getParentScope(ar);
       parent && !parent->isRedeclaring();
       parent = parent->getParentScope(ar)) {
    if (parent->hasProperty(name)) {
      return parent;
    }
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
    set<string> convertibles;
    typedef std::pair<const string,Symbol> symPair;
    BOOST_FOREACH(symPair &sym, m_symbolMap) {
      if (sym.second.isSuperGlobal() && !sym.second.declarationSet()) {
        convertibles.insert(sym.second.getName());
      }
    }
    if (!convertibles.empty()) {
      cg_printf("/* converted super globals */ global ");
      for (set<string>::const_iterator iter = convertibles.begin();
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

void VariableTable::canonicalizeStaticGlobals(CodeGenerator &cg) {
  ASSERT(m_staticGlobals.empty());

  sort(m_staticGlobalsVec.begin(), m_staticGlobalsVec.end(), by_location);

  for (unsigned int i = 0; i < m_staticGlobalsVec.size(); i++) {
    StaticGlobalInfoPtr &sgi = m_staticGlobalsVec[i];
    if (!sgi->sym->getDeclaration()) continue;
    string id = StaticGlobalInfo::getId(cg, sgi->cls, sgi->func,
                                        sgi->sym->getName());
    ASSERT(m_staticGlobals.find(id) == m_staticGlobals.end());
    m_staticGlobals[id] = sgi;
  }
}

void VariableTable::outputCPPGlobalVariablesHeader(CodeGenerator &cg,
                                                   AnalysisResultPtr ar) {
  ASSERT(!m_staticGlobals.empty() || m_staticGlobalsVec.empty());

  cg.printSection("Class Forward Declarations\n");
  for (StringToStaticGlobalInfoPtrMap::const_iterator iter =
         m_staticGlobals.begin(); iter != m_staticGlobals.end(); ++iter) {
    StaticGlobalInfoPtr sgi = iter->second;
    if (!sgi->func && !sgi->sym->isOverride()) {
      TypePtr varType = sgi->sym->getFinalType();
      if (varType->isSpecificObject()) {
        cg_printf("FORWARD_DECLARE_CLASS(%s);\n", varType->getName().c_str());
      }
    }
  }

  if (cg.getOutput() == CodeGenerator::SystemCPP) {
    cg_printf("class SystemGlobals : public Globals {\n");
    cg_indentBegin("public:\n");
    cg_printf("SystemGlobals();\n");
  } else {
    cg_printf("class GlobalVariables : public SystemGlobals {\n");
    cg_printf("DECLARE_SMART_ALLOCATION_NOCALLBACKS(GlobalVariables);\n");
    cg_indentBegin("public:\n");
    cg_printf("GlobalVariables();\n");
    cg_printf("~GlobalVariables();\n");
  }
  cg_printf("static void initialize();\n");

  cg_printf("\n");

  // We will create one variable[] per type.
  Type2SymbolListMap type2names;
  SymbolList &variants = type2names["Variant"];
  SymbolList &bools = type2names["bool"];

  // Global Variables
  if (cg.getOutput() != CodeGenerator::SystemCPP) {
    for (unsigned int i = 0; i < m_symbolVec.size(); i++) {
      const Symbol *sym = m_symbolVec[i];
      if (!sym->isSystem()) {
        variants.push_back(string("gvm_") + cg.formatLabel(sym->getName()));
      }
    }
  } else {
    for (unsigned int i = 0; i < m_symbolVec.size(); i++) {
      const Symbol *sym = m_symbolVec[i];
      TypePtr type = sym->getFinalType();
      type2names[type->getCPPDecl(cg, ar, BlockScopeRawPtr())].push_back
        (string("gvm_") + cg.formatLabel(sym->getName()));
    }
  }

  // Dynamic Constants
  ar->outputCPPDynamicConstantDecl(cg, type2names);

  // Function/Method Static Variables
  for (StringToStaticGlobalInfoPtrMap::const_iterator iter =
         m_staticGlobals.begin(); iter != m_staticGlobals.end(); ++iter) {
    const string &id = iter->first;
    StaticGlobalInfoPtr sgi = iter->second;
    if (sgi->func) {
      TypePtr varType = sgi->sym->getFinalType();
      type2names[varType->getCPPDecl(cg, ar, BlockScopeRawPtr())].push_back
        (string(Option::StaticVariablePrefix) + id);
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
        variants.push_back(name);
      } else {
        bools.push_back(name);
      }
    }
  }

  // Class Static Variables
  for (StringToStaticGlobalInfoPtrMap::const_iterator iter =
         m_staticGlobals.begin(); iter != m_staticGlobals.end(); ++iter) {
    StaticGlobalInfoPtr sgi = iter->second;
    // id can change if we discover it is redeclared
    const string &id = StaticGlobalInfo::getId(cg, sgi->cls, sgi->func,
                                               sgi->sym->getName());
    if (!sgi->func && !sgi->sym->isOverride()) {
      TypePtr varType = sgi->sym->getFinalType();
      type2names[varType->getCPPDecl(cg, ar, BlockScopeRawPtr())].push_back
        (string(Option::StaticPropertyPrefix) + id);
    }
  }

  // Class Static Initializer Flags
  ar->outputCPPClassStaticInitializerFlags(cg, type2names);

  // PseudoMain Variables
  ar->outputCPPFileRunDecls(cg, type2names);

  if (cg.getOutput() != CodeGenerator::SystemCPP) {
    // Volatile class declared flags
    ar->outputCPPClassDeclaredFlags(cg, type2names);
    cg_printf("virtual bool class_exists(CStrRef name);\n");
  }

  // Redeclared Functions
  ar->outputCPPRedeclaredFunctionDecl(cg, type2names);

  // Redeclared Classes
  ar->outputCPPRedeclaredClassDecl(cg, type2names);

  // generating arr[1] even if it's empty just to make
  // outputCPPGlobalVariablesImpl()'s memset easier to generate
  type2names.operator[]("bool");
  type2names.operator[]("CallInfo*");
  type2names.operator[]("ObjectStaticCallbacks*");

  const char *prefix = (cg.getOutput() == CodeGenerator::SystemCPP) ?
    "stgv_" : "tgv_"; // (system) typed global variables
  for (Type2SymbolListMap::const_iterator iter = type2names.begin();
       iter != type2names.end(); ++iter) {
    const string &type = iter->first;
    string typeName = type;
    Util::replaceAll(typeName, "*", "Ptr");
    const SymbolList &names = iter->second;

    // generating arr[1] even if it's empty just to make
    // outputCPPGlobalVariablesImpl()'s memset easier to generate
    cg_printf("%s %s%s[%d];\n", type.c_str(), prefix, typeName.c_str(),
              (names.empty() ? 1 : (int)names.size()));

    int i = 0;
    for (SymbolList::const_iterator iterName = names.begin();
         iterName != names.end(); ++iterName) {
      cg_printf("#define %s %s%s[%d]\n", iterName->c_str(), prefix,
                typeName.c_str(), i);
      i++;
    }
  }

  if (cg.getOutput() != CodeGenerator::SystemCPP) {
    cg.printSection("Global Array Wrapper Methods");
    cg_indentBegin("virtual ssize_t staticSize() const {\n");
    cg_printf("return %d;\n", m_symbolVec.size());
    cg_indentEnd("}\n");

    cg.printSection("LVariableTable Methods");
    cg_printf("virtual CVarRef getRefByIdx(ssize_t idx, Variant &k);\n");
    cg_printf("virtual ssize_t getIndex(const char *s, int64 prehash)"
              " const;\n");
    cg_printf("virtual Variant &getImpl(CStrRef s);\n");
    cg_printf("virtual bool exists(CStrRef s) const;\n");

  }
  cg_indentEnd("};\n");

  // generating scalar arrays
  cg.printSection("Scalar Arrays");
  if (cg.getOutput() == CodeGenerator::SystemCPP) {
    cg_printf("class SystemScalarArrays {\n");
  } else {
    cg_printf("class ScalarArrays : public SystemScalarArrays {\n");
  }
  cg_indentBegin("public:\n");
  cg_printf("static void initialize();\n");
  cg_printf("static void initializeNamed();\n");
  if (cg.getOutput() != CodeGenerator::SystemCPP &&
      Option::ScalarArrayFileCount > 1) {
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

void VariableTable::collectCPPGlobalSymbols(StringPairVecVec &symbols,
                                            CodeGenerator &cg,
                                            AnalysisResultPtr ar) {
  ASSERT(symbols.size() == AnalysisResult::GlobalSymbolTypeCount);
  ASSERT(!m_staticGlobals.empty() || m_staticGlobalsVec.empty());

  // static global variables
  StringPairVec *names = &symbols[AnalysisResult::KindOfStaticGlobalVariable];
  names->resize(m_symbolVec.size());
  for (unsigned int i = 0; i < m_symbolVec.size(); i++) {
    const string &name = m_symbolVec[i]->getName();
    (*names)[i].first = Option::GlobalVariablePrefix + cg.escapeLabel(name);
    (*names)[i].second = getGlobalVariableName(cg, ar, name);
  }

  // method static variables
  names = &symbols[AnalysisResult::KindOfMethodStaticVariable];
  for (StringToStaticGlobalInfoPtrMap::const_iterator iter =
       m_staticGlobals.begin(); iter != m_staticGlobals.end(); ++iter) {
    const string &id = iter->first;
    StaticGlobalInfoPtr sgi = iter->second;
    if (sgi->func) {
      string name = Option::StaticVariablePrefix + id;
      names->push_back(pair<string, string>(name, name));
    }
  }

  // class static variables
  names = &symbols[AnalysisResult::KindOfClassStaticVariable];
  for (StringToStaticGlobalInfoPtrMap::const_iterator iter =
       m_staticGlobals.begin(); iter != m_staticGlobals.end(); ++iter) {
    StaticGlobalInfoPtr sgi = iter->second;
    // id can change if we discover it is redeclared
    const string &id = StaticGlobalInfo::getId(cg, sgi->cls, sgi->func,
                                               sgi->sym->getName());
    if (!sgi->func && !sgi->sym->isOverride()) {
      string name = Option::StaticPropertyPrefix + id;
      names->push_back(pair<string, string>(name, name));
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

void VariableTable::outputCPPGlobalVariablesImpl(CodeGenerator &cg,
                                                 AnalysisResultPtr ar) {
  bool system = (cg.getOutput() == CodeGenerator::SystemCPP);
  ASSERT(!m_staticGlobals.empty() || m_staticGlobalsVec.empty());

  if (!system) {
    cg_printf("IMPLEMENT_SMART_ALLOCATION_NOCALLBACKS(GlobalVariables)\n");
  }

  const char *clsname = system ? "SystemGlobals" : "GlobalVariables";
  cg_indentBegin("%s::%s() {\n", clsname, clsname);

  const char *prefix = system ? "stgv_" : "tgv_";
  cg_printf("memset(&%sbool, 0, sizeof(%sbool));\n", prefix, prefix);
  cg_printf("memset(&%sCallInfoPtr, 0, sizeof(%sCallInfoPtr));\n",
            prefix, prefix);
  cg_printf("memset(&%sObjectStaticCallbacksPtr, 0, "
            "sizeof(%sObjectStaticCallbacksPtr));\n", prefix, prefix);

  cg.printSection("Primitive Function/Method Static Variables");
  for (StringToStaticGlobalInfoPtrMap::const_iterator iter =
       m_staticGlobals.begin(); iter != m_staticGlobals.end(); ++iter) {
    StaticGlobalInfoPtr sgi = iter->second;
    if (sgi->func) {
      TypePtr varType = sgi->sym->getFinalType();
      if (varType->isPrimitive()) {
        const string &id = iter->first;
        const char *initializer = varType->getCPPInitializer();
        ASSERT(initializer);
        cg_printf("%s%s = %s;\n",
                  Option::StaticVariablePrefix, id.c_str(), initializer);
      }
    }
  }

  cg.printSection("Primitive Class Static Variables");
  for (StringToStaticGlobalInfoPtrMap::const_iterator iter =
       m_staticGlobals.begin(); iter != m_staticGlobals.end(); ++iter) {
    StaticGlobalInfoPtr sgi = iter->second;
    // id can change if we discover it is redeclared
    if (!sgi->func && !sgi->sym->isOverride()) {
      TypePtr varType = sgi->sym->getFinalType();
      if (varType->isPrimitive()) {
        const string &id = StaticGlobalInfo::getId(cg, sgi->cls, sgi->func,
                                                   sgi->sym->getName());
        const char *initializer = varType->getCPPInitializer();
        ASSERT(initializer);
        cg_printf("%s%s = %s;\n",
                  Option::StaticPropertyPrefix, id.c_str(), initializer);
      }
    }
  }

  cg.printSection("Redeclared Classes");
  ar->outputCPPRedeclaredClassImpl(cg);

  cg_indentEnd("}\n");

  cg_printf("\n");
  // generating top level statements in system PHP files
  if (system) {
    cg_indentBegin("void SystemGlobals::initialize() {\n");
    ar->outputCPPSystemImplementations(cg);
  } else {
    cg_indentBegin("void GlobalVariables::initialize() {\n");
    cg_printf("SystemGlobals::initialize();\n");
  }

  for (StringToStaticGlobalInfoPtrMap::const_iterator iter =
         m_staticGlobals.begin(); iter != m_staticGlobals.end(); ++iter) {
    StaticGlobalInfoPtr sgi = iter->second;
    if (!sgi->func && !sgi->sym->isOverride() &&
        sgi->cls->needStaticInitializer()) {
      cg_printf("%s%s();\n", Option::ClassStaticInitializerPrefix,
                sgi->cls->getId(cg).c_str());
    }
  }
  cg_indentEnd("}\n");

  if (!system) {
    cg_printf("\n");

    cg_indentBegin("void init_static_variables() {\n");
    cg_printf("ScalarArrays::initialize();\n");
    if (Option::GenHashTableGetConstant) {
      cg_printf("init_constant_table();\n");
    }
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
      "#endif /* USE_GCC_FAST_TLS */\n"
      "void init_global_variables() {\n"
      "  ThreadInfo::s_threadInfo->m_globals =\n"
      "    get_global_variables_check();\n"
      "  GlobalVariables::initialize();\n"
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
    if (!sgi->func && !sgi->sym->isOverride()) {
      TypePtr varType = sgi->sym->getFinalType();
      if (varType->isSpecificObject()) {
        ClassScopePtr cls = ar->findClass(varType->getName());
        ASSERT(cls && !cls->isRedeclaring());
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

void VariableTable::outputCPPGlobalVariablesGetImpl(CodeGenerator &cg,
                                                    AnalysisResultPtr ar) {
  cg.ifdefBegin(false, "OMIT_JUMP_TABLE_GLOBAL_GETIMPL");
  cg_indentBegin("Variant &GlobalVariables::getImpl(CStrRef s) {\n");
  cg_printf("GlobalVariables *g __attribute__((__unused__)) = this;\n");
  if (!outputCPPJumpTable(cg, ar, NULL, true, true, EitherStatic,
                          JumpReturnString)) {
    m_emptyJumpTables.insert(JumpTableGlobalGetImpl);
  }
  cg_printf("return LVariableTable::getImpl(s);\n");
  cg_indentEnd("}\n");
  cg.ifdefEnd("OMIT_JUMP_TABLE_GLOBAL_GETIMPL");
}

void VariableTable::outputCPPGlobalVariablesExists(CodeGenerator &cg,
                                                   AnalysisResultPtr ar) {
  cg.ifdefBegin(false, "OMIT_JUMP_TABLE_GLOBAL_EXISTS");
  cg_indentBegin("bool GlobalVariables::exists(CStrRef s) const {\n");
  cg_printf("const GlobalVariables *g __attribute__((__unused__)) = this;\n");
  if (!outputCPPJumpTable(cg, ar, NULL, true, false,
                          EitherStatic, JumpInitializedString)) {
    m_emptyJumpTables.insert(JumpTableGlobalExists);
  }
  cg_printf("if (!LVariableTable::exists(s)) return false;\n");
  cg_printf("return isInitialized("
            "const_cast<GlobalVariables*>(this)->get(s));\n");
  cg_indentEnd("}\n");
  cg.ifdefEnd("OMIT_JUMP_TABLE_GLOBAL_EXISTS");
}

void VariableTable::outputCPPGlobalVariablesGetIndex(CodeGenerator &cg,
                                                     AnalysisResultPtr ar) {
  cg.ifdefBegin(false, "OMIT_JUMP_TABLE_GLOBAL_GETINDEX");
  cg_indentBegin("ssize_t GlobalVariables::getIndex(const char* s, "
                 "int64 hash) const {\n");
  cg_printf("const GlobalVariables *g __attribute__((__unused__)) = this;\n");
  if (!outputCPPJumpTable(cg, ar, NULL, false, true, EitherStatic, JumpIndex)) {
    m_emptyJumpTables.insert(JumpTableGlobalGetIndex);
  }
  cg_printf("return m_px ? (m_px->getIndex(s) + %d) : %d;\n",
            m_symbolVec.size(), ArrayData::invalid_index);
  cg_indentEnd("}\n");
  cg.ifdefEnd("OMIT_JUMP_TABLE_GLOBAL_GETINDEX");
}

void VariableTable::outputCPPGlobalVariablesMethods(CodeGenerator &cg,
                                                    AnalysisResultPtr ar) {
  int maxIdx = m_symbolVec.size();

  cg_indentBegin("CVarRef GlobalVariables::getRefByIdx(ssize_t idx, "
                 "Variant &k) {\n");
  cg_printf("GlobalVariables *g __attribute__((__unused__)) = this;\n");
  cg_indentBegin("static const char *names[] = {\n");
  for (int i = 0; i < maxIdx; i++) {
    const string &name = m_symbolVec[i]->getName();
    cg_printf("\"%s\",\n", cg.escapeLabel(name).c_str());
  }
  cg_indentEnd("};\n");
  cg_indentBegin("if (idx >= 0 && idx < %d) {\n", maxIdx);
  cg_printf("k = names[idx];\n");
  cg_printf("switch (idx) {\n");
  for (int i = 0; i < maxIdx; i++) {
    const string &name = m_symbolVec[i]->getName();
    cg_printf("case %d: return %s;\n", i,
              getGlobalVariableName(cg, ar, name).c_str());
  }
  cg_printf("}\n");
  cg_indentEnd("}\n");
  cg_printf("return Globals::getRefByIdx(idx, k);\n");
  cg_indentEnd("}\n");
}

void VariableTable::outputCPPVariableInit(CodeGenerator &cg,
                                          AnalysisResultPtr ar,
                                          bool inPseudoMain,
                                          const string &name) {
  if (inPseudoMain) {
    cg_printf(" __attribute__((__unused__)) = ");
    if (cg.getOutput() != CodeGenerator::SystemCPP) {
      cg_printf("(variables != gVariables) ? variables->get(");
      cg_printString(name, ar, getBlockScope());
      cg_printf(") : ");
    }
    cg_printf("g->");
    if (cg.getOutput() != CodeGenerator::SystemCPP) {
      cg_printf(getGlobalVariableName(cg, ar, name).c_str());
    } else {
      cg_printf("GV(%s)", name.c_str());
    }
  }
}

void VariableTable::outputCPPImpl(CodeGenerator &cg, AnalysisResultPtr ar) {
  bool inPseudoMain = isPseudoMainTable();
  if (inPseudoMain) {
    if (m_forcedVariants) {
      cg_printf("LVariableTable *gVariables __attribute__((__unused__)) = "
                "(LVariableTable *)g;\n");
    } else {
      ASSERT(false);
      cg_printf("RVariableTable *gVariables __attribute__((__unused__)) = "
                "(RVariableTable *)g;\n");
    }
  }

  bool declared = false;
  for (unsigned int i = 0; i < m_symbolVec.size(); i++) {
    const Symbol *sym = m_symbolVec[i];
    const string &name = sym->getName();
    string fname = cg.formatLabel(name);
    if (sym->isShrinkWrapped()) continue;
    if (sym->isSystem() && cg.getOutput() != CodeGenerator::SystemCPP) {
      continue;
    }

    if (sym->isStatic()) {
      string id = StaticGlobalInfo::getId
        (cg, getClassScope(), getFunctionScope(), name);

      TypePtr type = sym->getFinalType();
      type->outputCPPDecl(cg, ar, getBlockScope());
      if (ClassScope::NeedStaticArray(getClassScope(), getFunctionScope())) {
        const char *cname = getFunctionScope()->isStatic() ? "cls" :
          "this->o_getClassName()";
        cg_printf(" &%s%s __attribute__((__unused__)) = "
                  "g->%s%s.lvalAt(%s);\n",
                  Option::StaticVariablePrefix, fname.c_str(),
                  Option::StaticVariablePrefix, id.c_str(),
                  cname);
        cg_printf("Variant &%s%s%s __attribute__((__unused__)) = "
                  "g->%s%s%s.lvalAt(%s);\n",
                  Option::InitPrefix, Option::StaticVariablePrefix,
                  fname.c_str(),
                  Option::InitPrefix, Option::StaticVariablePrefix,
                  id.c_str(), cname);
      } else {
        cg_printf(" &%s%s __attribute__((__unused__)) = g->%s%s;\n",
                  Option::StaticVariablePrefix, fname.c_str(),
                  Option::StaticVariablePrefix, id.c_str());
        cg_printf("bool &%s%s%s __attribute__((__unused__)) = g->%s%s%s;\n",
                  Option::InitPrefix, Option::StaticVariablePrefix,
                  fname.c_str(),
                  Option::InitPrefix, Option::StaticVariablePrefix,
                  id.c_str());
      }

      if (needLocalCopy(sym) && !sym->isParameter()) {
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
      cg_printf(" &%s%s __attribute__((__unused__)) = g->%s;\n",
                Option::GlobalVariablePrefix, fname.c_str(),
                getGlobalVariableName(cg, ar, name).c_str());

      if (needLocalCopy(name)) {
        type->outputCPPDecl(cg, ar, getBlockScope());
        cg_printf(" %s%s%s", prefix, Option::VariablePrefix,
                  fname.c_str());
        outputCPPVariableInit(cg, ar, inPseudoMain, name);
        cg_printf(";\n");
        declared = true;
      }
      continue;
    }

    // local variables
    if (((getAttribute(ContainsDynamicVariable) || inPseudoMain) &&
         !sym->isHidden()) ||
        sym->isUsed() || sym->isNeeded()) {
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
    outputCPPVariableTable(cg, ar);
    ar->m_variableTableFunctions.insert(getScopePtr()->getName());
  }
}

void VariableTable::outputCPPVariableTable(CodeGenerator &cg,
                                           AnalysisResultPtr ar) {
  bool inGlobalScope = isGlobalTable(ar);

  string varDecl, initializer, memDecl, params;
  for (unsigned int i = 0; i < m_symbolVec.size(); i++) {
    const Symbol *sym = m_symbolVec[i];
    if (sym->isHidden()) continue;
    const string &name = sym->getName();
    string varName = string(getVariablePrefix(sym)) +
      cg.formatLabel(name);
    TypePtr type = sym->getFinalType();
    if (!inGlobalScope) {
      if (!varDecl.empty()) {
        varDecl += ", ";
        initializer += ", ";
        memDecl += "; ";
        params += ", ";
      }
      varDecl += type->getCPPDecl(cg, ar, getBlockScope()) + " &" +
        Option::TempVariablePrefix + cg.formatLabel(name);
      initializer += varName + "(" + Option::TempVariablePrefix +
        cg.formatLabel(name) + ")";
      memDecl += type->getCPPDecl(cg, ar, getBlockScope()) + " &" + varName;
      params += varName;
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
        varName = string("g->") + getGlobalVariableName(cg, ar, name);
      } else {
        varName = string(prefix) + cg.formatLabel(name);
      }
      cg_printf("if (%s.isInitialized()) ret.lval(\"%s\").setWithRef(%s);\n",
                varName.c_str(), cg.escapeLabel(name).c_str(),
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
    cg_printf("%sVariableTable* __attribute__((__unused__)) "
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

void VariableTable::outputCPPPropertyDecl(CodeGenerator &cg,
    AnalysisResultPtr ar, bool dynamicObject /* = false */) {
  for (unsigned int i = 0; i < m_symbolVec.size(); i++) {
    const Symbol *sym = m_symbolVec[i];
    if (dynamicObject && !sym->isPrivate()) continue;

    // we don't redefine a property that's already defined by a parent class
    // unless it is private or the parent's one is private
    if (sym->isStatic() || sym->isOverride()) continue;

    const string &name = sym->getName();
    sym->getFinalType()->outputCPPDecl(cg, ar, getBlockScope());
    cg_printf(" %s%s;\n", Option::PropertyPrefix,
              cg.formatLabel(name).c_str());
  }
}

void VariableTable::outputCPPPropertyClone(CodeGenerator &cg,
                                           AnalysisResultPtr ar,
                                           bool dynamicObject /* = false */) {

  for (unsigned int i = 0; i < m_symbolVec.size(); i++) {
    const Symbol *sym = m_symbolVec[i];
    const string &name = sym->getName();
    string formatted = cg.formatLabel(name);
    if (sym->isStatic() || sym->isOverride()) continue;
    if (sym->getFinalType()->is(Type::KindOfVariant)) {
      if (!dynamicObject || isPrivate(name)) {
        cg_printf("clone->%s%s.setWithRef(%s%s);\n",
                  Option::PropertyPrefix, formatted.c_str(),
                  Option::PropertyPrefix, formatted.c_str());
      }
    } else {
      cg_printf("clone->%s%s = %s%s;\n",
                Option::PropertyPrefix, formatted.c_str(),
                Option::PropertyPrefix, formatted.c_str());
    }
  }
}

void VariableTable::outputCPPPropertyTable(CodeGenerator &cg,
    AnalysisResultPtr ar, const char *parent, const char *parentName,
    ClassScope::Derivation dynamicObject /* = ClassScope::FromNormal */) {
  string clsStr = m_blockScope.getId(cg);
  const char *cls = clsStr.c_str();

  const char *cprefix = Option::ClassPrefix;
  const char *op = "::";
  const char *gl = "";
  if (dynamicObject == ClassScope::DirectFromRedeclared) {
    cprefix = Option::ClassStaticsObjectPrefix;
    op = "->";
    gl = "g->";
    parent = parentName;
  }
  // Statics
  bool gdec = false;
  cg.ifdefBegin(false, "OMIT_JUMP_TABLE_CLASS_STATIC_GETINIT_%s", cls);
  cg_indentBegin("Variant %s%s::%sgetInit(CStrRef s) {\n",
                 Option::ClassPrefix, cls, Option::ObjectStaticPrefix);
  if (!outputCPPJumpTable(cg, ar, NULL, true, false, EitherStatic,
                          JumpReturnInit, EitherPrivate, &gdec)) {
    m_emptyJumpTables.insert(JumpTableClassStaticGetInit);
  }
  if (!gdec && dynamicObject == 1) cg.printDeclareGlobals();
  cg_printf("return %s%s%s%s%sgetInit(s);\n", gl, cprefix,
            parent, op, Option::ObjectStaticPrefix);
  cg_indentEnd("}\n");
  cg.ifdefEnd("OMIT_JUMP_TABLE_CLASS_STATIC_GETINIT_%s", cls);

  cg.ifdefBegin(false, "OMIT_JUMP_TABLE_CLASS_STATIC_GET_%s", cls);
  cg_indentBegin("Variant %s%s::%sget(CStrRef s) {\n",
                 Option::ClassPrefix, cls, Option::ObjectStaticPrefix);
  if (!outputCPPJumpTable(cg, ar, Option::StaticPropertyPrefix, true, false,
                          Static, JumpReturnString, NonPrivate, &gdec)) {
    m_emptyJumpTables.insert(JumpTableClassStaticGet);
  }
  if (!gdec && dynamicObject == 1) cg.printDeclareGlobals();
  cg_printf("return %s%s%s%s%sget(s);\n", gl, cprefix,
            parent, op, Option::ObjectStaticPrefix);
  cg_indentEnd("}\n");
  cg.ifdefEnd("OMIT_JUMP_TABLE_CLASS_STATIC_GET_%s", cls);

  cg.ifdefBegin(false, "OMIT_JUMP_TABLE_CLASS_STATIC_LVAL_%s", cls);
  cg_indentBegin("Variant &%s%s::%slval(CStrRef s) {\n",
                 Option::ClassPrefix, cls, Option::ObjectStaticPrefix);
  if (!outputCPPJumpTable(cg, ar, Option::StaticPropertyPrefix, true, true,
                          Static, JumpReturnString, NonPrivate, &gdec)) {
    m_emptyJumpTables.insert(JumpTableClassStaticLval);
  }
  if (!gdec && dynamicObject == 1) cg.printDeclareGlobals();
  cg_printf("return %s%s%s%s%slval(s);\n", gl, cprefix,
            parent, op, Option::ObjectStaticPrefix);
  cg_indentEnd("}\n");
  cg.ifdefEnd("OMIT_JUMP_TABLE_CLASS_STATIC_LVAL_%s", cls);

  if (dynamicObject == ClassScope::DirectFromRedeclared) {
    parent = "DynamicObjectData";
    cprefix = Option::ClassPrefix;
    op = "::";
    gl = "";
  }

  cg.ifdefBegin(false, "OMIT_JUMP_TABLE_CLASS_GETARRAY_%s", cls);
  cg_indentBegin("void %s%s::%sgetArray(Array &props, bool pubOnly) const {\n",
                 Option::ClassPrefix, cls, Option::ObjectPrefix);
  bool empty = true;
  for (unsigned int i = 0; i < m_symbolVec.size(); i++) {
    const Symbol *sym = m_symbolVec[i];
    bool priv = sym->isPrivate();
    bool prot = sym->isProtected();
    if (dynamicObject && !priv) continue;
    const char *s = sym->getName().c_str();
    string prop(sym->getName());
    if (!sym->isStatic()) {
      empty = false;
      if (priv) {
        ClassScope &clsScope = dynamic_cast<ClassScope &>(m_blockScope);
        prop = '\0' + clsScope.getOriginalName() + '\0' + prop;
      }
      if (prot || priv) {
        cg_printf("if (!pubOnly) ");
      }
      if (sym->isOverride()) {
        /* The actual property is stored in a base class,
           but we need to set the entry here, to get the
           iteration order right */
        cg_printf("props.set(");
        cg_printString(prop, ar, getBlockScope());
        cg_printf(", null_variant, true);\n");
      } else if (sym->getFinalType()->is(Type::KindOfVariant)) {
        cg_printf("if (isInitialized(%s%s)) props.lvalAt(",
                  Option::PropertyPrefix, s);
        cg_printString(prop, ar, getBlockScope());
        cg_printf(", AccessFlags::Key).setWithRef(%s%s);\n",
                  Option::PropertyPrefix, s);
      } else {
        cg_printf("props.%s(", priv ? "add" : "set");
        cg_printString(prop, ar, getBlockScope());
        cg_printf(", %s%s, true);\n", Option::PropertyPrefix, s);
      }
    }
  }
  cg_printf("%s%s::%sgetArray(props, pubOnly);\n", Option::ClassPrefix, parent,
            Option::ObjectPrefix);
  cg_indentEnd("}\n");
  cg.ifdefEnd("OMIT_JUMP_TABLE_CLASS_GETARRAY_%s", cls);
  if (empty) m_emptyJumpTables.insert(JumpTableClassGetArray);

  cg.ifdefBegin(false, "OMIT_JUMP_TABLE_CLASS_SETARRAY_%s", cls);
  cg_indentBegin("void %s%s::%ssetArray(CArrRef props) {\n",
                 Option::ClassPrefix, cls, Option::ObjectPrefix);
  empty = true;
  for (unsigned int i = 0; i < m_symbolVec.size(); i++) {
    const Symbol *sym = m_symbolVec[i];
    if (!sym->isPrivate() || sym->isStatic()) continue;
    empty = false;
    const char *s = sym->getName().c_str();
    ClassScope &clsScope = dynamic_cast<ClassScope &>(m_blockScope);
    string prop = '\0' + clsScope.getOriginalName() + '\0' + sym->getName();
    if (sym->getFinalType()->is(Type::KindOfVariant)) {
      cg_printf("props->load(");
      cg_printString(prop, ar, getBlockScope());
      cg_printf(", %s%s);\n", Option::PropertyPrefix, s);
    } else {
      cg_printf("if (props->exists(");
      cg_printString(prop, ar, getBlockScope());
      cg_printf(")) %s%s = props->get(", Option::PropertyPrefix, s);
      cg_printString(prop, ar, getBlockScope());
      cg_printf(");\n");
    }
  }
  cg_printf("%s%s::%ssetArray(props);\n", Option::ClassPrefix, parent,
            Option::ObjectPrefix);
  cg_indentEnd("}\n");
  cg.ifdefEnd("OMIT_JUMP_TABLE_CLASS_SETARRAY_%s", cls);
  if (empty) m_emptyJumpTables.insert(JumpTableClassSetArray);

  outputCPPPropertyOp(cg, ar, cls, parent, "realProp", ", int flags", ", flags",
                      "Variant *", true, JumpRealProp, false, dynamicObject,
                      JumpTableClassRealProp);
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
      cg_printf("HASH_GUARD_STRING(0x%016llXLL, %s) "
                "{ return %s%sPrivate(prop%s); }\n",
                hash_string(name), name, Option::ObjectPrefix, op, args);
    } else {
      cg_printf("HASH_GUARD_STRING(0x%016llXLL, %s) "
                "{ return %s%s::%s%sPrivate(prop%s); }\n",
                hash_string(name), name, Option::ClassPrefix,
                name, Option::ObjectPrefix, op, args);
    }
  }
  return true;
}

void VariableTable::outputCPPPropertyOp
(CodeGenerator &cg, AnalysisResultPtr ar,
 const char *cls, const char *parent, const char *op, const char *argsDec,
 const char *args, const char *ret, bool cnst, JumpTableType type,
 bool varOnly,  ClassScope::Derivation dynamicObject, JumpTableName jtname) {

  cg.ifdefBegin(false, "OMIT_JUMP_TABLE_CLASS_%s_%s", op, cls);
  cg_indentBegin("%s %s%s::%s%s(CStrRef prop%s, CStrRef context)%s {\n",
      ret, Option::ClassPrefix,
      cls, Option::ObjectPrefix, op, argsDec, cnst ? " const" : "");
  if (!outputCPPPrivateSelector(cg, ar, op, args)) {
    m_emptyJumpTables.insert(jtname);
  }
  if (!dynamicObject) {
    cg_printf("return %s%sPublic(prop%s);\n",
              Option::ObjectPrefix, op, args);
  } else {
    cg_printf("return DynamicObjectData::%s%s(prop%s, context);\n",
              Option::ObjectPrefix, op, args);
  }
  cg_indentEnd("}\n");
  cg.ifdefEnd("OMIT_JUMP_TABLE_CLASS_%s_%s", op, cls);

  if (!dynamicObject) {
    cg.ifdefBegin(false, "OMIT_JUMP_TABLE_CLASS_%s_PUBLIC_%s", op, cls);
    cg_indentBegin("%s %s%s::%s%sPublic(CStrRef s%s)%s {\n",
                   ret, Option::ClassPrefix, cls,
                   Option::ObjectPrefix, op, argsDec, cnst ? " const" : "");
    if (!outputCPPJumpTable(cg, ar, Option::PropertyPrefix, true, varOnly,
                            NonStatic, type)) {
      // offset 1 based on enum order
      m_emptyJumpTables.insert((JumpTableName)(jtname + 1));
    }
    cg_printf("return %s%s::%s%sPublic(s%s);\n",
              Option::ClassPrefix, parent, Option::ObjectPrefix, op, args);
    cg_indentEnd("}\n");
    cg.ifdefEnd("OMIT_JUMP_TABLE_CLASS_%s_PUBLIC_%s", op, cls);
  }

  cg.ifdefBegin(false, "OMIT_JUMP_TABLE_CLASS_%s_PRIVATE_%s", op, cls);
  cg_indentBegin("%s %s%s::%s%sPrivate(CStrRef s%s)%s {\n",
                 ret, Option::ClassPrefix, cls, Option::ObjectPrefix, op,
                 argsDec, cnst ? " const" : "");
  if (!outputCPPJumpTable(cg, ar, Option::PropertyPrefix, true, varOnly,
                          NonStatic, type, Private)) {
    // offset 2 based on enum order
    m_emptyJumpTables.insert((JumpTableName)(jtname + 2));
  }
  if (!dynamicObject) {
    // Fall back to public
    cg_printf("return %s%sPublic(s%s);\n",
              Option::ObjectPrefix, op, args);
  } else {
    cg_printf("return DynamicObjectData::%s%s(s%s, empty_string);\n",
              Option::ObjectPrefix, op, args);
  }
  cg_indentEnd("}\n");
  cg.ifdefEnd("OMIT_JUMP_TABLE_CLASS_%s_PRIVATE_%s", op, cls);
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
  bool needsGlobals = type == JumpReturnInit;
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
      if (type == JumpRealProp &&
          !Type::SameType(sym->getFinalType(), Type::Variant)) {
        needsGlobals = true;
      }
    }
  }
  if (strings.empty()) return false;

  if (hasStatic || needsGlobals) {
    cg.printDeclareGlobals();
    if (declaredGlobals) *declaredGlobals = true;
  }

  if (hasStatic) {
    ClassScopePtr cls = getClassScope();
    if (cls && cls->needLazyStaticInitializer()) {
      cg_printf("lazy_initializer(g);\n");
    }
  }

  bool useString = (type == JumpRealProp) || (type == JumpSet) ||
                   (type == JumpReturnString) ||
                   (type == JumpInitializedString) || (type == JumpReturnInit);

  for (JumpTable jt(cg, strings, false, !defineHash, useString); jt.ready();
       jt.next()) {
    const char *name = jt.key();
    const char *symbol_prefix =
      prefix ? prefix : getVariablePrefix(name);
    string varName;
    if (prefix == Option::StaticPropertyPrefix) {
      varName = string(prefix) + getClassScope()->getId(cg) +
        Option::IdPrefix + cg.formatLabel(name);
    } else {
      varName = string(symbol_prefix) + cg.formatLabel(name);
    }
    if (symbol_prefix == Option::GlobalVariablePrefix) {
      varName = string("g->") + getGlobalVariableName(cg, ar, name);
    } else if (symbol_prefix != Option::VariablePrefix &&
               symbol_prefix != Option::PropertyPrefix) {
      varName = string("g->") + varName;
    }
    switch (type) {
    case VariableTable::JumpRealProp:
      cg_printf("HASH_REALPROP_%sSTRING(0x%016llXLL, \"%s\", %d, %s);\n",
                Type::SameType(getFinalType(name), Type::Variant) ?
                "" : "TYPED_",
                hash_string(name), cg.escapeLabel(name).c_str(),
                strlen(name), cg.formatLabel(name).c_str());
      break;
    case VariableTable::JumpReturn:
      cg_printf("HASH_RETURN(0x%016llXLL, %s,\n",
                hash_string(name), varName.c_str());
      cg_printf("            \"%s\");\n", cg.escapeLabel(name).c_str());
      break;
    case VariableTable::JumpSet:
      cg_printf("HASH_SET_STRING(0x%016llXLL, %s,\n",
                hash_string(name), varName.c_str());
      cg_printf("                \"%s\", %d);\n",
                cg.escapeLabel(name).c_str(), strlen(name));
      break;
    case VariableTable::JumpInitialized:
      cg_printf("HASH_INITIALIZED(0x%016llXLL, %s,\n",
                hash_string(name), varName.c_str());
      cg_printf("                 \"%s\");\n", cg.escapeLabel(name).c_str());
      break;
    case VariableTable::JumpInitializedString: {
      int index = -1;
      int stringId = cg.checkLiteralString(name, index, ar, getBlockScope());
      assert(index >= 0);
      string lisnam = ar->getLiteralStringName(stringId, index);
      cg_printf("HASH_INITIALIZED_NAMSTR(0x%016llXLL, %s, %s,\n",
                hash_string(name), lisnam.c_str(), varName.c_str());
      cg_printf("                   %d);\n", strlen(name));
      break;
    }
    case VariableTable::JumpIndex:
      {
        hphp_const_char_map<ssize_t>::const_iterator it = varIdx.find(name);
        ASSERT(it != varIdx.end());
        ssize_t idx = it->second;
        cg_printf("HASH_INDEX(0x%016llXLL, \"%s\", %d);\n",
                  hash_string(name), cg.escapeLabel(name).c_str(), idx);
      }
      break;
    case VariableTable::JumpReturnString: {
      int index = -1;
      int stringId = cg.checkLiteralString(name, index, ar, getBlockScope());
      assert(index >= 0);
      string lisnam = ar->getLiteralStringName(stringId, index);
      cg_printf("HASH_RETURN_NAMSTR(0x%016llXLL, %s, %s,\n",
                hash_string(name), lisnam.c_str(), varName.c_str());
      cg_printf("                   %d);\n", strlen(name));
      break;
    }
    case VariableTable::JumpReturnInit:
      ExpressionPtr value =
        dynamic_pointer_cast<Expression>(getClassInitVal(name));
      if (value) {
        cg_printf("HASH_RETURN_NAMSTR(0x%016llXLL, ", hash_string(name));
        cg_printString(name, ar, getBlockScope());
        cg_printf(",\n");
        cg_printf("                   ");
        CodeGenerator::Context oldContext = cg.getContext();
        cg.setContext(CodeGenerator::CppStaticInitializer);
        value->outputCPP(cg, ar);
        cg.setContext(oldContext);
        cg_printf(", %d);\n", strlen(name));
      }
      break;
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
              cg.escapeLabel(name).c_str());
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
                  cg.escapeLabel(name).c_str(), len, output.c_str());
      }
    }
  }
  cg_printf("NULL,\n");
}

///////////////////////////////////////////////////////////////////////////////
}
