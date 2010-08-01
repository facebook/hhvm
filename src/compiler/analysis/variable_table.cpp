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

using namespace HPHP;
using namespace std;
using namespace boost;

void (*VariableTable::m_hookHandler)(AnalysisResultPtr ar,
                                     VariableTable *variables,
                                     ExpressionPtr variable,
                                     HphpHookUniqueId id);

///////////////////////////////////////////////////////////////////////////////
// StaticGlobalInfo

CodeGenerator s_dummy_code_generator;

string VariableTable::StaticGlobalInfo::getName
(ClassScopePtr cls, FunctionScopePtr func, const string &name) {
  ASSERT(cls || func);

  // format: <class>$$<func>$$name
  string id;
  if (cls) {
    id += cls->getId(s_dummy_code_generator);
    id += Option::IdPrefix;
  }
  if (func) {
    id += func->getId(s_dummy_code_generator);
    id += Option::IdPrefix;
  }
  id += name;

  return id;
}

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
  : SymbolTable(blockScope), m_attribute(0), m_allVariants(false),
    m_hookData(NULL) {
}

VariableTable::~VariableTable() {
  if (m_hookData) {
    ASSERT(m_hookHandler);
    m_hookHandler(AnalysisResultPtr(), this, ExpressionPtr(), hphpUniqueDtor);
  }
}

void VariableTable::getNames(std::set<string> &names,
                             bool collectPrivate /* = true */) const {
  for (unsigned int i = 0; i < m_symbols.size(); i++) {
    if (collectPrivate || !isPrivate(m_symbols[i])) {
      names.insert(m_symbols[i]);
    }
  }
}

bool VariableTable::isParameter(const string &name) const {
  return m_parameters.find(name) != m_parameters.end();
}

bool VariableTable::isPublic(const string &name) const {
  return !isProtected(name) && !isPrivate(name);
}

bool VariableTable::isProtected(const string &name) const {
  return m_protected.find(name) != m_protected.end();
}

bool VariableTable::isPrivate(const string &name) const {
  return m_private.find(name) != m_private.end();
}

bool VariableTable::isStatic(const string &name) const {
  return m_static.find(name) != m_static.end();
}

bool VariableTable::isGlobal(const string &name) const {
  return m_global.find(name) != m_global.end();
}

bool VariableTable::isRedeclared(const string &name) const {
  return m_redeclared.find(name) != m_redeclared.end();
}

bool VariableTable::isLocalGlobal(const string &name) const {
  return m_localGlobal.find(name) != m_localGlobal.end();
}

bool VariableTable::isNestedStatic(const string &name) const {
  return m_nestedStatic.find(name) != m_nestedStatic.end();
}

bool VariableTable::isLvalParam(const string &name) const {
  return m_lvalParam.find(name) != m_lvalParam.end();
}

bool VariableTable::isUsed(const string &name) const {
  return m_used.find(name) != m_used.end();
}

bool VariableTable::isNeeded(const string &name) const {
  return m_needed.find(name) != m_needed.end();
}

bool VariableTable::isSuperGlobal(const string &name) const {
  return m_superGlobals.find(name) != m_superGlobals.end();
}

bool VariableTable::isLocal(const string &name) const {
  FunctionScope *func = dynamic_cast<FunctionScope*>(getScope());
  if (func) {
    /*
      isSuperGlobal is not wanted here. It just means that
      $GLOBALS[name] was referenced in this scope.
      It doesnt say anything about the variable $name.
     */
    return (!isStatic(name) &&
            !isGlobal(name) &&
            !isParameter(name));
  } else {
    return false;
  }
}

bool VariableTable::needLocalCopy(const string &name) const {
  return (isGlobal(name) || isStatic(name)) &&
         (isRedeclared(name) ||
          isNestedStatic(name) ||
          isLocalGlobal(name) ||
          getAttribute(ContainsDynamicVariable) ||
          getAttribute(ContainsExtract) ||
          getAttribute(ContainsUnset));
}

bool VariableTable::needGlobalPointer() const {
  return !m_global.empty() ||
    !m_static.empty() ||
    getAttribute(ContainsDynamicVariable) ||
    getAttribute(ContainsExtract) ||
    getAttribute(ContainsUnset) ||
    getAttribute(NeedGlobalPointer);
}

bool VariableTable::isInherited(const string &name) const {
  return !isGlobal(name) && !isSystem(name) &&
    m_declarations.find(name) == m_declarations.end();
}

bool VariableTable::definedByParent(AnalysisResultPtr ar,
                                    const string &name) {
  if (isPrivate(name)) return false;
  ClassScopePtr cls = findParent(ar, name);
  if (cls) {
    return !cls->getVariables()->isPrivate(name);
  } else {
    return false;
  }
}

const char *VariableTable::getVariablePrefix(AnalysisResultPtr ar,
                                             const string &name) const {
  if (isStatic(name)) {
    if (!needLocalCopy(name)) {
      return Option::StaticVariablePrefix;
    }
    return Option::VariablePrefix;
  }

  if (getAttribute(ForceGlobal)) {
    return Option::GlobalVariablePrefix;
  }

  if (isGlobal(name)) {
    if (!needLocalCopy(name)) {
      return Option::GlobalVariablePrefix;
    }
  }
  return Option::VariablePrefix;
}

string VariableTable::getVariableName(CodeGenerator &cg, AnalysisResultPtr ar,
                                      const string &name) const {
  if (isStatic(name)) {
    if (!needLocalCopy(name)) {
      return string(Option::StaticVariablePrefix) + cg.formatLabel(name);
    }
    return string(Option::VariablePrefix) + cg.formatLabel(name);
  }

  if (getAttribute(ForceGlobal)) {
    return getGlobalVariableName(cg, ar, name);
  }

  if (isGlobal(name)) {
    if (!needLocalCopy(name)) {
      return getGlobalVariableName(cg, ar, name);
    }
  }
  return string(Option::VariablePrefix) + cg.formatLabel(name);
}

string
VariableTable::getGlobalVariableName(CodeGenerator &cg, AnalysisResultPtr ar,
                                     const string &name) const {
  if (ar->getVariables()->isSystem(name)) {
    return string(Option::GlobalVariablePrefix) + cg.formatLabel(name);
  }
  return string("GV(") + cg.formatLabel(name) + ")";
}

ConstructPtr VariableTable::getStaticInitVal(string varName) {
  StringToConstructPtrMap::const_iterator iter = m_staticInitVal.find(varName);
  if (iter != m_staticInitVal.end()) return iter->second;
  return ConstructPtr();
}

bool VariableTable::setStaticInitVal(string varName,
                                     ConstructPtr value) {
  bool exists = (m_staticInitVal.find(varName) != m_staticInitVal.end());
  m_staticInitVal[varName] = value;
  return exists;
}

ConstructPtr VariableTable::getClassInitVal(string varName) {
  StringToConstructPtrMap::const_iterator iter = m_clsInitVal.find(varName);
  if (iter != m_clsInitVal.end()) return iter->second;
  return ConstructPtr();
}

bool VariableTable::setClassInitVal(string varName, ConstructPtr value) {
  bool exists = (m_clsInitVal.find(varName) != m_clsInitVal.end());
  m_clsInitVal[varName] = value;
  return exists;
}

///////////////////////////////////////////////////////////////////////////////

TypePtr VariableTable::addParam(const string &name, TypePtr type,
                                AnalysisResultPtr ar, ConstructPtr construct) {
  if (m_parameters.find(name) == m_parameters.end()) {
    int index = m_parameters.size();
    m_parameters[name] = index;
  }
  return add(name, type, false, ar, construct, ModifierExpressionPtr());
}

void VariableTable::addStaticVariable(const string &name,
                                      AnalysisResultPtr ar,
                                      bool member /* = false */) {
  if (isGlobalTable(ar)) {
    return; // a static variable at global scope is the same as non-static
  }

  // redeclaring the same static variable twice
  if (m_static.find(name) != m_static.end()) {
    return;
  }

  m_static.insert(name);

  VariableTablePtr globalVariables = ar->getVariables();
  StaticGlobalInfoPtr sgi(new StaticGlobalInfo());
  sgi->name = name;
  sgi->variables = this;
  sgi->cls = ar->getClassScope();
  sgi->func = member ? FunctionScopePtr() : ar->getFunctionScope();

  string id = StaticGlobalInfo::getName(sgi->cls, sgi->func, name);
  ASSERT(globalVariables->m_staticGlobals.find(id) ==
         globalVariables->m_staticGlobals.end());
  globalVariables->m_staticGlobals[id] = sgi;
}

TypePtr VariableTable::add(const string &name, TypePtr type,
                           bool implicit, AnalysisResultPtr ar,
                           ConstructPtr construct,
                           ModifierExpressionPtr modifiers,
                           bool checkError /* = true */) {
  if (getAttribute(InsideStaticStatement)) {
    addStaticVariable(name, ar);
    if (ar->needStaticArray(ar->getClassScope())) {
      forceVariant(ar, name);
    }
  } else if (getAttribute(InsideGlobalStatement)) {
    m_global.insert(name);
    if (!isGlobalTable(ar)) {
      ar->getVariables()->add(name, type, implicit, ar, construct, modifiers,
                              false);
    }
    ASSERT(type->is(Type::KindOfSome) || type->is(Type::KindOfAny));
    TypePtr varType = ar->getVariables()->getFinalType(name);
    if (varType) {
      type = varType;
    } else {
      ar->getVariables()->setType(ar, name, type, true);
    }
  } else if (ar->getPhase() == AnalysisResult::FirstInference &&
             isPseudoMainTable()) {
    // A variable used in a pseudomain
    ar->getVariables()->add(name, type, implicit, ar,
                            construct, modifiers,
                            checkError);
  }

  if (modifiers) {
    if (modifiers->isProtected()) {
      m_protected.insert(name);
    } else if (modifiers->isPrivate()) {
      m_private.insert(name);
    }
    if (modifiers->isStatic()) {
      addStaticVariable(name, ar);
    }
  }
  type = setType(ar, name, type, true);
  m_declarations[name] = construct;

  if (!implicit && ar->isFirstPass()) {
    StringToConstructPtrMap::const_iterator iter = m_values.find(name);
    if (iter == m_values.end()) {
      m_values[name] = construct;
    } else if (construct != iter->second && checkError && !isGlobalTable(ar)) {
      ar->getCodeError()->record(CodeError::DeclaredVariableTwice, construct,
                                 iter->second);
    }
  }
  return type;
}

TypePtr VariableTable::checkVariable(const string &name, TypePtr type,
                                     bool coerce, AnalysisResultPtr ar,
                                     ConstructPtr construct, int &properties) {
  properties = 0;

  // Variable used in pseudomain
  if (ar->getPhase() == AnalysisResult::FirstInference &&
      isPseudoMainTable()) {
    ar->getVariables()->checkVariable(name, type,
                                      coerce, ar, construct, properties);
  }

  if (m_declarations.find(name) == m_declarations.end()) {
    ClassScopePtr parent = findParent(ar, name);
    if (parent) {
      return parent->checkStatic(name, type, coerce, ar,
                                 construct, properties);
    }

    bool isLocal = !isGlobal(name) && !isSystem(name);
    if (isLocal && !getAttribute(ContainsLDynamicVariable) &&
        ar->isFirstPass()) {
      CodeError::ErrorType error = (ar->getScope()->getLoopNestedLevel() == 0 ?
                                    CodeError::UseUndeclaredVariable :
                                    CodeError::PossibleUndeclaredVariable);
      ar->getCodeError()->record(error, construct);
      type = Type::Variant;
      coerce = true;
    }

    type = setType(ar, name, type, coerce);
    m_declarations[name] = construct;
    return type;
  }

  properties = VariablePresent;
  if (isStatic(name)) {
    properties |= VariableStatic;
  }
  return setType(ar, name, type, coerce);
}

TypePtr VariableTable::checkProperty(const string &name, TypePtr type,
                                     bool coerce, AnalysisResultPtr ar,
                                     ConstructPtr construct, int &properties) {
  properties = VariablePresent;
  if (m_declarations.find(name) == m_declarations.end()) {
    ClassScopePtr parent = findParent(ar, name);
    if (parent) {
      TypePtr ret = parent->checkProperty(name, type, coerce, ar, construct,
          properties);
      if (!(properties & VariablePrivate)) {
        return ret;
      }
    }
    if (ar->isFirstPass()) {
      ar->getCodeError()->record(CodeError::UseUndeclaredVariable, construct);
    }
    properties = 0;
    return type;
  }

  TypePtr ret = setType(ar, name, type, coerce);

  // walk up to make sure all parents are happy with this type
  ClassScopePtr parent = findParent(ar, name);
  if (parent) {
    return parent->checkProperty(name, type, coerce, ar, construct,
                                 properties);
  }
  if (isStatic(name)) {
    properties |= VariableStatic;
  }
  if (isPrivate(name)) {
    properties |= VariablePrivate;
  }
  return ret;
}

bool VariableTable::checkRedeclared(const string &name,
                                    Statement::KindOf kindOf)
{
  ASSERT(kindOf == Statement::KindOfStaticStatement ||
         kindOf == Statement::KindOfGlobalStatement);
  if (kindOf == Statement::KindOfStaticStatement && isPresent(name)) {
    if (isStatic(name)) {
      return true;
    } else if (!isRedeclared(name)) {
      m_redeclared.insert(name);
      return true;
    } else {
      return false;
    }
  } else if (kindOf == Statement::KindOfGlobalStatement &&
             isPresent(name) && !isGlobal(name) && !isRedeclared(name)) {
    m_redeclared.insert(name);
    return true;
  } else {
    return false;
  }
}

void VariableTable::addLocalGlobal(const string &name) {
  m_localGlobal.insert(name);
}

void VariableTable::addNestedStatic(const string &name) {
  m_nestedStatic.insert(name);
}

void VariableTable::addLvalParam(const string &name) {
  m_lvalParam.insert(name);
}

void VariableTable::addUsed(const string &name) {
  m_used.insert(name);
}

void VariableTable::addNeeded(const string &name)
{
  m_needed.insert(name);
}

bool VariableTable::checkUnused(const string &name) {
  return (!isPseudoMainTable() &&
          !getAttribute(VariableTable::ContainsDynamicVariable) &&
          !isUsed(name) &&
          isLocal(name));
}

void VariableTable::clearUsed()
{
  m_used.clear();
  m_needed.clear();
}

void VariableTable::forceVariants(AnalysisResultPtr ar) {
  if (!m_allVariants) {
    for (unsigned int i = 0; i < m_symbols.size(); i++) {
      setType(ar, m_symbols[i], Type::Variant, true);
    }
    m_allVariants = true;

    ClassScopePtr parent = m_blockScope.getParentScope(ar);
    if (parent) {
      parent->getVariables()->forceVariants(ar);
    }
  }
}

void VariableTable::forceVariant(AnalysisResultPtr ar,
                                 const string &name) {
  if (m_declarations.find(name) != m_declarations.end()) {
    setType(ar, name, Type::Variant, true);
  }
}

TypePtr VariableTable::setType(AnalysisResultPtr ar, const string &name,
                               TypePtr type, bool coerce) {
  if (m_allVariants) type = Type::Variant;
  TypePtr ret = SymbolTable::setType(ar, name, type, coerce || m_allVariants);
  if (!ret) return ret;

  if (isGlobal(name)) {
    VariableTablePtr v = ar->getVariables();
    if (v.get() != this) {
      v->setType(ar, name, type, coerce);
    }
  }

  if (coerce) {
    std::map<string, int>::const_iterator iter = m_parameters.find(name);
    if (iter != m_parameters.end()) {
      FunctionScope *func = dynamic_cast<FunctionScope *>(&m_blockScope);
      ASSERT(func);
      TypePtr paramType = func->setParamType(ar, iter->second, type);
      if (!Type::SameType(paramType, type)) {
        return setType(ar, name, paramType, true); // recursively
      }
    }
  }
  return ret;
}

void VariableTable::dumpStats(std::map<string, int> &typeCounts) {
  for (unsigned int i = 0; i < m_symbols.size(); i++) {
    if (isGlobal(m_symbols[i])) continue;
    typeCounts[getFinalType(m_symbols[i])->toString()]++;
  }
}

void VariableTable::addSuperGlobal(const string &name) {
  m_superGlobals.insert(name);
}

bool VariableTable::isConvertibleSuperGlobal(const string &name) const {
  return !getAttribute(ContainsDynamicVariable) && isSuperGlobal(name);
}


ClassScopePtr VariableTable::findParent(AnalysisResultPtr ar,
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

bool VariableTable::isGlobalTable(AnalysisResultPtr ar) const {
  return ar->getVariables().get() == this;
}
bool VariableTable::isPseudoMainTable() const {
  return m_blockScope.inPseudoMain();
}
bool VariableTable::hasPrivate() const {
  return !m_private.empty();
}

void VariableTable::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (Option::GenerateInferredTypes) {
    for (unsigned int i = 0; i < m_symbols.size(); i++) {
      const string &name = m_symbols[i];
      if (isInherited(name)) continue;

      if (isParameter(name)) {
        cg_printf("// @param  ");
      } else if (isGlobal(name)) {
        cg_printf("// @global ");
      } else if (isStatic(name)) {
        cg_printf("// @static ");
      } else {
        cg_printf("// @local  ");
      }
      cg_printf("%s\t$%s\n", getFinalType(name)->toString().c_str(),
                name.c_str());
    }
  }
  if (Option::ConvertSuperGlobals && !getAttribute(ContainsDynamicVariable)) {
    set<string> convertibles;
    for (set<string>::const_iterator iter = m_superGlobals.begin();
         iter != m_superGlobals.end(); ++iter) {
      if (m_declarations.find(*iter) == m_declarations.end()) {
        convertibles.insert(*iter);
      }
    }
    m_superGlobals = convertibles;
    if (!m_superGlobals.empty()) {
      cg_printf("/* converted super globals */ global ");
      for (set<string>::const_iterator iter = m_superGlobals.begin();
           iter != m_superGlobals.end(); ++iter) {
        if (iter != m_superGlobals.begin()) cg_printf(",");
        cg_printf("$%s", iter->c_str());
      }
      cg_printf(";\n");
    }
  }
}

void VariableTable::outputCPPGlobalVariablesHeader(CodeGenerator &cg,
                                                   AnalysisResultPtr ar) {
  cg.printSection("Class Forward Declarations\n");
  for (StringToStaticGlobalInfoPtrMap::const_iterator iter =
         m_staticGlobals.begin(); iter != m_staticGlobals.end(); ++iter) {
    StaticGlobalInfoPtr sgi = iter->second;
    if (!sgi->func) {
      TypePtr varType = sgi->variables->getFinalType(sgi->name);
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
    cg_printf("static GlobalVariables *Create() "
              "{ return NEW(GlobalVariables)(); }\n");
    cg_printf("static void Delete(GlobalVariables *p) "
              "{ DELETE(GlobalVariables)(p); }\n");
    cg_printf("static void OnThreadExit(GlobalVariables *p) {}\n");
  }
  cg_printf("static void initialize();\n");

  cg_printf("\n");
  cg_printf("bool dummy; // for easier constructor initializer output\n");

  cg.printSection("Global Variables");
  if (cg.getOutput() != CodeGenerator::SystemCPP) {
    cg_printf("BEGIN_GVS()\n");
    int count = 0;
    for (unsigned int i = 0; i < m_symbols.size(); i++) {
      const string &name = m_symbols[i];
      if (!isSystem(name)) {
        count++;
        cg_printf("  GVS(%s)\n", cg.formatLabel(name).c_str());
      }
    }
    cg_printf("END_GVS(%d)\n", count);
  } else {
    for (unsigned int i = 0; i < m_symbols.size(); i++) {
      const string &name = m_symbols[i];
      TypePtr type = getFinalType(name);
      type->outputCPPDecl(cg, ar);
      cg_printf(" %s%s;\n", Option::GlobalVariablePrefix,
                cg.formatLabel(name).c_str());
    }
  }

  cg.printSection("Dynamic Constants");
  ar->outputCPPDynamicConstantDecl(cg);

  cg.printSection("Function/Method Static Variables");
  for (StringToStaticGlobalInfoPtrMap::const_iterator iter =
         m_staticGlobals.begin(); iter != m_staticGlobals.end(); ++iter) {
    const string &id = iter->first;
    StaticGlobalInfoPtr sgi = iter->second;
    if (sgi->func) {
      TypePtr varType = sgi->variables->getFinalType(sgi->name);
      varType->outputCPPDecl(cg, ar);
      cg_printf(" %s%s;\n", Option::StaticVariablePrefix, id.c_str());
    }
  }

  cg.printSection("Function/Method Static Variable Initialization Booleans");
  for (StringToStaticGlobalInfoPtrMap::const_iterator iter =
         m_staticGlobals.begin(); iter != m_staticGlobals.end(); ++iter) {
    const string &id = iter->first;
    StaticGlobalInfoPtr sgi = iter->second;
    if (sgi->func) {
      if (ar->needStaticArray(sgi->cls)) {
        cg_printf("Variant %s%s%s;\n", Option::InitPrefix,
                  Option::StaticVariablePrefix, id.c_str());
      } else {
        cg_printf("bool %s%s%s;\n", Option::InitPrefix,
                  Option::StaticVariablePrefix, id.c_str());
      }
    }
  }

  cg.printSection("Class Static Variables");
  int count = 0;
  for (StringToStaticGlobalInfoPtrMap::const_iterator iter =
         m_staticGlobals.begin(); iter != m_staticGlobals.end(); ++iter) {
    StaticGlobalInfoPtr sgi = iter->second;
    // id can change if we discover it is redeclared
    const string &id = StaticGlobalInfo::getId(cg, sgi->cls, sgi->func,
                                               sgi->name);
    if (!sgi->func) {
      TypePtr varType = sgi->variables->getFinalType(sgi->name);
      if (varType->is(Type::KindOfVariant)) {
        cg_printf("#define %s%s csp[%d]\n",
                  Option::StaticPropertyPrefix, id.c_str(), count);
        count++;
      } else {
        varType->outputCPPDecl(cg, ar);
        cg_printf(" %s%s;\n", Option::StaticPropertyPrefix, id.c_str());
      }
    }
  }
  if (count) {
    cg_printf("Variant csp[%d];\n", count);
  }

  cg.printSection("Class Static Initializer Flags");
  ar->outputCPPClassStaticInitializerFlags(cg, false);

  cg.printSection("PseudoMain Variables");
  ar->outputCPPFileRunDecls(cg);

  if (cg.getOutput() != CodeGenerator::SystemCPP) {
    cg.printSection("Volatile class declared flags");
    ar->outputCPPClassDeclaredFlags(cg);
    cg_printf("virtual bool class_exists(const char *name);\n");
  }

  cg.printSection("Redeclared Functions");
  ar->outputCPPRedeclaredFunctionDecl(cg, false);

  cg.printSection("Redeclared Classes");
  ar->outputCPPRedeclaredClassDecl(cg);

  if (cg.getOutput() != CodeGenerator::SystemCPP) {
    cg.printSection("Global Array Wrapper Methods");
    cg_indentBegin("virtual ssize_t staticSize() const {\n");
    cg_printf("return %d;\n", m_symbols.size());
    cg_indentEnd("}\n");

    cg.printSection("LVariableTable Methods");
    cg_printf("virtual CVarRef getRefByIdx(ssize_t idx, Variant &k);\n");
    cg_printf("virtual ssize_t getIndex(const char *s, int64 prehash)"
              " const;\n");
    cg_printf("virtual Variant &getImpl(CStrRef s, int64 hash);\n");
    cg_printf("virtual bool exists(const char *s, int64 hash = -1) const;\n");

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

  // static global variables
  StringPairVec *names = &symbols[AnalysisResult::KindOfStaticGlobalVariable];
  names->resize(m_symbols.size());
  for (unsigned int i = 0; i < m_symbols.size(); i++) {
    const string &name = m_symbols[i];
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
                                               sgi->name);
    if (!sgi->func) {
      string name = Option::StaticPropertyPrefix + id;
      names->push_back(pair<string, string>(name, name));
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

void VariableTable::outputCPPGlobalVariablesImpl(CodeGenerator &cg,
                                                 AnalysisResultPtr ar) {
  bool system = (cg.getOutput() == CodeGenerator::SystemCPP);

  if (!system) {
    cg_printf("IMPLEMENT_SMART_ALLOCATION_NOCALLBACKS(GlobalVariables)\n");
  }

  const char *clsname = system ? "SystemGlobals" : "GlobalVariables";
  cg_printf("%s::%s() : dummy(false)", clsname, clsname);

  set<string> classes;
  for (StringToStaticGlobalInfoPtrMap::const_iterator iter =
         m_staticGlobals.begin(); iter != m_staticGlobals.end(); ++iter) {
    const string &id = iter->first;
    StaticGlobalInfoPtr sgi = iter->second;
    if (sgi->func) {
      if (ar->needStaticArray(sgi->cls)) {
        cg_printf(",\n  %s%s%s()", Option::InitPrefix,
                  Option::StaticVariablePrefix, id.c_str());
      } else {
        cg_printf(",\n  %s%s%s(false)", Option::InitPrefix,
                  Option::StaticVariablePrefix, id.c_str());
      }
    } else if (sgi->cls->needStaticInitializer()) {
      classes.insert(sgi->cls->getId(cg));
    }
  }
  ar->outputCPPClassStaticInitializerFlags(cg, true);
  ar->outputCPPFileRunImpls(cg);
  ar->outputCPPRedeclaredFunctionDecl(cg, true);

  cg_indentBegin(" {\n");

  cg.printSection("Primitive Function/Method Static Variables");
  for (StringToStaticGlobalInfoPtrMap::const_iterator iter =
       m_staticGlobals.begin(); iter != m_staticGlobals.end(); ++iter) {
    StaticGlobalInfoPtr sgi = iter->second;
    if (sgi->func) {
      TypePtr varType = sgi->variables->getFinalType(sgi->name);
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
    if (!sgi->func) {
      TypePtr varType = sgi->variables->getFinalType(sgi->name);
      if (varType->isPrimitive()) {
        const string &id = StaticGlobalInfo::getId(cg, sgi->cls, sgi->func,
                                                   sgi->name);
        const char *initializer = varType->getCPPInitializer();
        ASSERT(initializer);
        cg_printf("%s%s = %s;\n",
                  Option::StaticPropertyPrefix, id.c_str(), initializer);
      }
    }
  }

  cg.printSection("Redeclared Functions");
  ar->outputCPPRedeclaredFunctionImpl(cg);

  cg.printSection("Redeclared Classes");
  ar->outputCPPRedeclaredClassImpl(cg);

  if (!system) {
    cg.printSection("Volatile class declaration flags");
    cg_printf("memset(cdec, 0, sizeof(cdec));\n");
  }

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
  for (set<string>::const_iterator iter = classes.begin();
       iter != classes.end(); ++iter) {
    cg_printf("%s%s();\n", Option::ClassStaticInitializerPrefix,
              iter->c_str());
  }
  cg_indentEnd("}\n");

  if (!system) {
    cg_printf("\n");
    cg_indentBegin("void init_static_variables() { \n");
    if (Option::PrecomputeLiteralStrings && ar->getLiteralStringCount() > 0) {
      cg_printf("LiteralStringInitializer::initialize();\n");
    }
    cg_printf("ScalarArrays::initialize();\n");
    cg_printf("StaticString::FinishInit();\n");
    cg_indentEnd("}\n");
    cg_printf("static ThreadLocalSingleton<GlobalVariables> g_variables;\n");

    cg_printf("static IMPLEMENT_THREAD_LOCAL"
              "(GlobalArrayWrapper, g_array_wrapper);\n");
    cg_indentBegin("GlobalVariables *get_global_variables() {\n");
    cg_printf("return g_variables.get();\n");
    cg_indentEnd("}\n");
    cg_printf("void init_global_variables() "
              "{ GlobalVariables::initialize();}\n");
    cg_indentBegin("void free_global_variables() {\n");
    cg_printf("g_variables.reset();\n");
    cg_printf("g_array_wrapper.reset();\n");
    cg_indentEnd("}\n");
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
  std::set<string> dtorIncludes;
  for (StringToStaticGlobalInfoPtrMap::const_iterator iter =
         m_staticGlobals.begin(); iter != m_staticGlobals.end(); ++iter) {
    StaticGlobalInfoPtr sgi = iter->second;
    if (!sgi->func) {
      TypePtr varType = sgi->variables->getFinalType(sgi->name);
      if (varType->isSpecificObject()) {
        ClassScopePtr cls = ar->findClass(varType->getName());
        ASSERT(cls && !cls->isRedeclaring());
        if (cls->isUserClass()) {
          const string fileBase = cls->getFileScope()->outputFilebase();
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
  cg_indentBegin("Variant &GlobalVariables::getImpl(CStrRef str, "
                 "int64 hash) {\n");
  cg.printDeclareGlobals();
  cg_printf("const char *s __attribute__((__unused__)) = str.data();\n");
  if (!outputCPPJumpTable(cg, ar, NULL, false, true, EitherStatic)) {
    m_emptyJumpTables.insert(JumpTableGlobalGetImpl);
  }
  cg_printf("return LVariableTable::getImpl(str, hash);\n");
  cg_indentEnd("}\n");
  cg.ifdefEnd("OMIT_JUMP_TABLE_GLOBAL_GETIMPL");
}

void VariableTable::outputCPPGlobalVariablesExists(CodeGenerator &cg,
                                                   AnalysisResultPtr ar) {
  cg.ifdefBegin(false, "OMIT_JUMP_TABLE_GLOBAL_EXISTS");
  cg_indentBegin("bool GlobalVariables::exists(const char *s, "
                 "int64 hash /* = -1 */) const {\n");
  cg.printDeclareGlobals();
  if (!outputCPPJumpTable(cg, ar, NULL, false, false,
                          EitherStatic, JumpInitialized)) {
    m_emptyJumpTables.insert(JumpTableGlobalExists);
  }
  cg_printf("if (!LVariableTable::exists(s, hash)) return false;\n");
  cg_printf("return isInitialized("
            "const_cast<GlobalVariables*>(this)->get(s, hash));\n");
  cg_indentEnd("}\n");
  cg.ifdefEnd("OMIT_JUMP_TABLE_GLOBAL_EXISTS");
}

void VariableTable::outputCPPGlobalVariablesGetIndex(CodeGenerator &cg,
                                                     AnalysisResultPtr ar) {
  cg.ifdefBegin(false, "OMIT_JUMP_TABLE_GLOBAL_GETINDEX");
  cg_indentBegin("ssize_t GlobalVariables::getIndex(const char* s, "
                 "int64 prehash) const {\n");
  cg.printDeclareGlobals();
  if (!outputCPPJumpTable(cg, ar, NULL, true, true, EitherStatic, JumpIndex)) {
    m_emptyJumpTables.insert(JumpTableGlobalGetIndex);
  }
  cg_printf("return m_px ? (m_px->getIndex(s, prehash) + %d) : %d;\n",
            m_symbols.size(), ArrayData::invalid_index);
  cg_indentEnd("}\n");
  cg.ifdefEnd("OMIT_JUMP_TABLE_GLOBAL_GETINDEX");
}

void VariableTable::outputCPPGlobalVariablesMethods(CodeGenerator &cg,
                                                    AnalysisResultPtr ar) {
  int maxIdx = m_symbols.size();

  cg_indentBegin("CVarRef GlobalVariables::getRefByIdx(ssize_t idx, "
                 "Variant &k) {\n");
  cg.printDeclareGlobals();
  cg_indentBegin("static const char *names[] = {\n");
  for (int i = 0; i < maxIdx; i++) {
    const string &name = m_symbols[i];
    cg_printf("\"%s\",\n", cg.escapeLabel(name).c_str());
  }
  cg_indentEnd("};\n");
  cg_indentBegin("if (idx >= 0 && idx < %d) {\n", maxIdx);
  cg_printf("k = names[idx];\n");
  cg_printf("switch (idx) {\n");
  for (int i = 0; i < maxIdx; i++) {
    const string &name = m_symbols[i];
    cg_printf("case %d: return %s;\n", i,
              getGlobalVariableName(cg, ar, name).c_str());
  }
  cg_printf("}\n");
  cg_indentEnd("}\n");
  cg_printf("return Globals::getRefByIdx(idx, k);\n");
  cg_indentEnd("}\n");
}

void VariableTable::outputCPPImpl(CodeGenerator &cg, AnalysisResultPtr ar) {
  bool inPseudoMain = isPseudoMainTable();
  if (inPseudoMain || needGlobalPointer()) {
    cg.printDeclareGlobals();
  }
  if (inPseudoMain) {
    if (m_allVariants) {
      cg_printf("LVariableTable *gVariables __attribute__((__unused__)) = "
                "get_variable_table();\n");
    } else {
      ASSERT(false);
      cg_printf("RVariableTable *gVariables __attribute__((__unused__)) = "
                "get_variable_table();\n");
    }
  }

  bool declared = false;
  for (unsigned int i = 0; i < m_symbols.size(); i++) {
    const string &name = m_symbols[i];
    if (isSystem(name) && cg.getOutput() != CodeGenerator::SystemCPP) {
      continue;
    }

    if (isStatic(name)) {
      string id = StaticGlobalInfo::getId
        (cg, ar->getClassScope(), ar->getFunctionScope(), name);

      TypePtr type = getFinalType(name);
      type->outputCPPDecl(cg, ar);
      if (ar->needStaticArray(ar->getClassScope())) {
        const char *cname = ar->getFunctionScope()->isStatic() ? "cls" :
          "this->o_getClassName()";
        cg_printf(" &%s%s __attribute__((__unused__)) = "
                  "g->%s%s.lvalAt(%s);\n",
                  Option::StaticVariablePrefix, cg.formatLabel(name).c_str(),
                  Option::StaticVariablePrefix, id.c_str(),
                  cname);
        cg_printf("Variant &%s%s%s __attribute__((__unused__)) = "
                  "g->%s%s%s.lvalAt(%s);\n",
                  Option::InitPrefix, Option::StaticVariablePrefix,
                  cg.formatLabel(name).c_str(),
                  Option::InitPrefix, Option::StaticVariablePrefix,
                  id.c_str(), cname);
      } else {
        cg_printf(" &%s%s __attribute__((__unused__)) = g->%s%s;\n",
                  Option::StaticVariablePrefix, cg.formatLabel(name).c_str(),
                  Option::StaticVariablePrefix, id.c_str());
        cg_printf("bool &%s%s%s __attribute__((__unused__)) = g->%s%s%s;\n",
                  Option::InitPrefix, Option::StaticVariablePrefix,
                  cg.formatLabel(name).c_str(),
                  Option::InitPrefix, Option::StaticVariablePrefix,
                  id.c_str());
      }

      if (needLocalCopy(name) && !isParameter(name)) {
        type->outputCPPDecl(cg, ar);
        cg_printf(" %s%s;\n", Option::VariablePrefix,
                  cg.formatLabel(name).c_str());
        declared = true;
      }
      continue;
    }

    if (isParameter(name)) continue;

    const char* prefix = "";
    string init = "";
    if (inPseudoMain) {
      prefix = "&";
      init = " __attribute__((__unused__)) = ";
      if (cg.getOutput() != CodeGenerator::SystemCPP) {
        int64 hash = hash_string(name.c_str(), name.length());
        char hbuf[25];
        sprintf(hbuf, "0x%016llXLL", hash);
        init += string("(variables != gVariables) ? "
                       "variables->get(\"") +
          cg.escapeLabel(name) + "\", " + hbuf + ") : ";
      }
      init += "g->";
      if (cg.getOutput() != CodeGenerator::SystemCPP) {
        init += getGlobalVariableName(cg, ar, name);
      } else {
        init += Option::GlobalVariablePrefix;
        init += name;
      }
    }

    if (isGlobal(name)) {
      TypePtr type = getFinalType(name);
      type->outputCPPDecl(cg, ar);
      cg_printf(" &%s%s __attribute__((__unused__)) = g->%s;\n",
                Option::GlobalVariablePrefix, cg.formatLabel(name).c_str(),
                getGlobalVariableName(cg, ar, name).c_str());

      if (needLocalCopy(name)) {
        type->outputCPPDecl(cg, ar);
        cg_printf(" %s%s%s%s;\n", prefix, Option::VariablePrefix,
                  cg.formatLabel(name).c_str(), init.c_str());
        declared = true;
      }
      continue;
    }

    // local variables
    if (getAttribute(ContainsDynamicVariable) ||
        inPseudoMain || isUsed(name) || isNeeded(name)) {
      TypePtr type = getFinalType(name);
      type->outputCPPDecl(cg, ar);
      cg_printf(" %s%s%s", prefix, getVariablePrefix(ar, name),
                cg.formatLabel(name).c_str());
      if (inPseudoMain) {
        cg_printf("%s", init.c_str());
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
    ar->m_variableTableFunctions.insert(getScope()->getName());
  }
}

void VariableTable::outputCPPVariableTable(CodeGenerator &cg,
                                           AnalysisResultPtr ar) {
  bool inGlobalScope = isGlobalTable(ar);

  string varDecl, initializer, memDecl, params;
  for (unsigned int i = 0; i < m_symbols.size(); i++) {
    const string &name = m_symbols[i];
    string varName = string(getVariablePrefix(ar, name)) +
      cg.formatLabel(name);
    TypePtr type = getFinalType(name);
    if (!inGlobalScope) {
      if (!varDecl.empty()) {
        varDecl += ", ";
        initializer += ", ";
        memDecl += "; ";
        params += ", ";
      }
      varDecl += type->getCPPDecl() + " &" + Option::TempVariablePrefix +
        cg.formatLabel(name);
      initializer += varName + "(" + Option::TempVariablePrefix +
        cg.formatLabel(name) + ")";
      memDecl += type->getCPPDecl() + " &" + varName;
      params += varName;
    }
  }

  cg_printf("\n");
  if (m_allVariants) {
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

  if (m_allVariants) {
    cg_indentBegin("virtual Variant &getImpl(CStrRef str, int64 hash) {\n");
    cg_printf("const char *s __attribute__((__unused__)) = str.data();\n");
    if (!outputCPPJumpTable(cg, ar, NULL, false, true, EitherStatic)) {
      m_emptyJumpTables.insert(JumpTableLocalGetImpl);
    }
    cg_printf("return LVariableTable::getImpl(str, hash);\n");
    cg_indentEnd("}\n");

    if (getAttribute(ContainsExtract)) {
      cg_indentBegin("virtual bool exists(const char *s, "
                     "int64 hash /* = -1 */) const {\n");
      if (!outputCPPJumpTable(cg, ar, NULL, false, false,
                              EitherStatic, JumpInitialized)) {
        m_emptyJumpTables.insert(JumpTableLocalExists);
      }
      cg_printf("return LVariableTable::exists(s, hash);\n");
      cg_indentEnd("}\n");
    }
  } else {
    cg_indentBegin("virtual Variant getImpl(const char *s) {\n");
    if (!outputCPPJumpTable(cg, ar, NULL, true, false, EitherStatic)) {
      m_emptyJumpTables.insert(JumpTableLocalGetImpl);
    }
    cg_printf("return rvalAt(s);\n");
    cg_indentEnd("}\n");

    if (getAttribute(ContainsCompact)) {
      cg_indentBegin("virtual bool exists(const char *s) const {\n");
      if (!outputCPPJumpTable(cg, ar, NULL, true, false,
                              EitherStatic, JumpInitialized)) {
        m_emptyJumpTables.insert(JumpTableLocalExists);
      }
      cg_printf("return RVariableTable::exists(s);\n");
      cg_indentEnd("}\n");
    }
  }

  if (getAttribute(ContainsGetDefinedVars)) {
    if (m_allVariants) {
      cg_indentBegin("virtual Array getDefinedVars() {\n");
    } else {
      cg_indentBegin("virtual Array getDefinedVars() const {\n");
    }
    cg_printf("Array ret = %sVariableTable::getDefinedVars();\n",
              m_allVariants ? "L" : "R");
    for (unsigned int i = 0; i < m_symbols.size(); i++) {
      const string &name = m_symbols[i];
      const char *prefix = getVariablePrefix(ar, name);
      string varName;
      if (prefix == Option::GlobalVariablePrefix) {
        varName = string("g->") + getGlobalVariableName(cg, ar, name);
      } else {
        varName = string(prefix) + cg.formatLabel(name);
      }
      cg_printf("ret.set(\"%s\", %s);\n", cg.escapeLabel(name).c_str(),
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
              m_allVariants ? "L" : "R");
  } else {
    cg_indentEnd("};\n");
    cg_printf("static IMPLEMENT_THREAD_LOCAL(VariableTable, "
              "g_variable_tables);\n");
    if (m_allVariants) {
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
  for (unsigned int i = 0; i < m_symbols.size(); i++) {
    const string &name = m_symbols[i];
    if (dynamicObject && !isPrivate(name)) continue;

    // we don't redefine a property that's already defined by a parent class
    // unless it is private or the parent's one is private
    if (isStatic(name) || definedByParent(ar, name)) continue;

    cg_printf("public: ");
    getFinalType(name)->outputCPPDecl(cg, ar);
    cg_printf(" %s%s;\n", Option::PropertyPrefix,
              cg.formatLabel(name).c_str());
  }
}

void VariableTable::outputCPPPropertyClone(CodeGenerator &cg,
                                           AnalysisResultPtr ar,
                                           bool dynamicObject /* = false */) {

  for (unsigned int i = 0; i < m_symbols.size(); i++) {
    const string &name = m_symbols[i];
    string formatted = cg.formatLabel(name);
    string escaped = cg.escapeLabel(name);
    if (isStatic(name)) continue;
    if (getFinalType(name)->is(Type::KindOfVariant)) {
      if (!dynamicObject || isPrivate(name)) {
        cg_printf("clone->%s%s = %s%s.isReferenced() ? ref(%s%s) : %s%s;\n",
                  Option::PropertyPrefix, formatted.c_str(),
                  Option::PropertyPrefix, formatted.c_str(),
                  Option::PropertyPrefix, formatted.c_str(),
                  Option::PropertyPrefix, formatted.c_str());
      } else {
        cg_printf("clone->o_set(\"%s\", -1, "
                  "o_get(\"%s\", -1).isReferenced() ? "
                  "ref(o_get(\"%s\", -1)) : o_get(\"%s\",-1));\n",
                  escaped.c_str(),
                  escaped.c_str(),
                  escaped.c_str(),
                  escaped.c_str());
      }
    } else {
      cg_printf("clone->%s%s = %s%s;\n",
                Option::PropertyPrefix, formatted.c_str(),
                Option::PropertyPrefix, formatted.c_str());
    }
  }
}

void VariableTable::outputCPPPropertyTable(CodeGenerator &cg,
    AnalysisResultPtr ar, const char *parent,
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
  }
  // Statics
  bool gdec = false;
  cg.ifdefBegin(false, "OMIT_JUMP_TABLE_CLASS_STATIC_GETINIT_%s", cls);
  cg_indentBegin("Variant %s%s::%sgetInit(const char *s, int64 hash) {\n",
                 Option::ClassPrefix, cls, Option::ObjectStaticPrefix);
  if (!outputCPPJumpTable(cg, ar, NULL, false, false, EitherStatic,
                          JumpReturnInit, EitherPrivate, &gdec)) {
    m_emptyJumpTables.insert(JumpTableClassStaticGetInit);
  }
  if (!gdec && dynamicObject == 1) cg.printDeclareGlobals();
  cg_printf("return %s%s%s%s%sgetInit(s, hash);\n", gl, cprefix,
             parent, op, Option::ObjectStaticPrefix);
  cg_indentEnd("}\n");
  cg.ifdefEnd("OMIT_JUMP_TABLE_CLASS_STATIC_GETINIT_%s", cls);

  cg.ifdefBegin(false, "OMIT_JUMP_TABLE_CLASS_STATIC_GET_%s", cls);
  cg_indentBegin("Variant %s%s::%sget(const char *s, int64 hash) {\n",
                 Option::ClassPrefix, cls, Option::ObjectStaticPrefix);
  if (!outputCPPJumpTable(cg, ar, Option::StaticPropertyPrefix, false,
                          false, Static, JumpReturn, NonPrivate, &gdec)) {
    m_emptyJumpTables.insert(JumpTableClassStaticGet);
  }
  if (!gdec && dynamicObject == 1) cg.printDeclareGlobals();
  cg_printf("return %s%s%s%s%sget(s, hash);\n", gl, cprefix,
            parent, op, Option::ObjectStaticPrefix);
  cg_indentEnd("}\n");
  cg.ifdefEnd("OMIT_JUMP_TABLE_CLASS_STATIC_GET_%s", cls);

  cg.ifdefBegin(false, "OMIT_JUMP_TABLE_CLASS_STATIC_LVAL_%s", cls);
  cg_indentBegin("Variant &%s%s::%slval(const char *s, int64 hash) {\n",
                 Option::ClassPrefix, cls, Option::ObjectStaticPrefix);
  if (!outputCPPJumpTable(cg, ar, Option::StaticPropertyPrefix, false,
                          true, Static, JumpReturn, NonPrivate, &gdec)) {
    m_emptyJumpTables.insert(JumpTableClassStaticLval);
  }
  if (!gdec && dynamicObject == 1) cg.printDeclareGlobals();
  cg_printf("return %s%s%s%s%slval(s, hash);\n", gl, cprefix,
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
  cg_indentBegin("void %s%s::%sgetArray(Array &props) const {\n",
                 Option::ClassPrefix, cls, Option::ObjectPrefix);
  bool empty = true;
  for (unsigned int i = 0; i < m_symbols.size(); i++) {
    bool priv = isPrivate(m_symbols[i]);
    if (dynamicObject && !priv) continue;
    const char *s = m_symbols[i].c_str();
    string prop(m_symbols[i]);
    int64 prehash = hash_string(prop.c_str(), prop.length());
    if (!isStatic(s)) {
      empty = false;
      if (priv) {
        ClassScope clsScope = dynamic_cast<ClassScope &>(m_blockScope);
        prop = '\0' + clsScope.getOriginalName() + '\0' + prop;
        prehash = hash_string(prop.c_str(), prop.length());
      }
      if (getFinalType(s)->is(Type::KindOfVariant)) {
        cg_printf("if (isInitialized(%s%s)) props.set(",
                  Option::PropertyPrefix, s);
        cg_printString(prop, ar);
        cg_printf(", %s%s.isReferenced() ? ref(%s%s) : %s%s, "
                  "0x%016llXLL, true);\n",
                  Option::PropertyPrefix, s, Option::PropertyPrefix, s,
                  Option::PropertyPrefix, s,
                  prehash);
      } else {
        cg_printf("props.set(");
        cg_printString(prop, ar);
        cg_printf(", %s%s, 0x%016llXLL, true);\n",
                  Option::PropertyPrefix, s, prehash);
      }
    }
  }
  cg_printf("%s%s::%sgetArray(props);\n", Option::ClassPrefix, parent,
            Option::ObjectPrefix);
  cg_indentEnd("}\n");
  cg.ifdefEnd("OMIT_JUMP_TABLE_CLASS_GETARRAY_%s", cls);
  if (empty) m_emptyJumpTables.insert(JumpTableClassGetArray);

  cg.ifdefBegin(false, "OMIT_JUMP_TABLE_CLASS_SETARRAY_%s", cls);
  cg_indentBegin("void %s%s::%ssetArray(CArrRef props) {\n",
                 Option::ClassPrefix, cls, Option::ObjectPrefix);
  empty = true;
  for (unsigned int i = 0; i < m_symbols.size(); i++) {
    if (!isPrivate(m_symbols[i]) || isStatic(m_symbols[i])) continue;
    empty = false;
    const char *s = m_symbols[i].c_str();
    ClassScope clsScope = dynamic_cast<ClassScope &>(m_blockScope);
    string prop = '\0' + clsScope.getOriginalName() + '\0' + m_symbols[i];
    if (getFinalType(s)->is(Type::KindOfVariant)) {
      cg_printf("props->load(");
      cg_printString(prop, ar);
      cg_printf(", %s%s);\n", Option::PropertyPrefix, s);
    } else {
      cg_printf("if (props->exists(");
      cg_printString(prop, ar);
      cg_printf(")) %s%s = props->get(", Option::PropertyPrefix, s);
      cg_printString(prop, ar);
      cg_printf(");\n");
    }
  }
  cg_printf("%s%s::%ssetArray(props);\n", Option::ClassPrefix, parent,
            Option::ObjectPrefix);
  cg_indentEnd("}\n");
  cg.ifdefEnd("OMIT_JUMP_TABLE_CLASS_SETARRAY_%s", cls);
  if (empty) m_emptyJumpTables.insert(JumpTableClassSetArray);

  outputCPPPropertyOp(cg, ar, cls, parent, "get", ", bool error /* = true */",
                      ", error", "Variant", false, JumpReturnString, false,
                      dynamicObject, JumpTableClassGet);
  outputCPPPropertyOp(cg, ar, cls, parent, "exists", "", "", "bool", true,
                      JumpExists, false, dynamicObject, JumpTableClassExists);
  outputCPPPropertyOp(cg, ar, cls, parent, "set",
                      ", CVarRef v, bool forInit /* = false */",
                      ", v, forInit", "Variant",
                      false, JumpSet, false, dynamicObject, JumpTableClassSet);
  outputCPPPropertyOp(cg, ar, cls, parent, "lval", "", "", "Variant&", false,
                      JumpReturnString, true, dynamicObject,
                      JumpTableClassLval);
}

bool VariableTable::outputCPPPrivateSelector(CodeGenerator &cg,
    AnalysisResultPtr ar, const char *op, const char *args) {
  ClassScopePtr cls = ar->getClassScope();
  vector<const char *> classes;
  do {
    if (cls->getVariables()->hasPrivate()) {
      classes.push_back(cls->getName().c_str());
    }
    cls = cls->getParentScope(ar);
  } while (cls && !cls->isRedeclaring()); // allow current class to be redec
  if (classes.empty()) return false;

  cg_printf("const char *s = context;\n");
  cg_printf("if (!s) { context = s = FrameInjection::GetClassName(false);"
      " }\n");
  for (JumpTable jt(cg, classes, true, false, false); jt.ready(); jt.next()) {
    const char *name = jt.key();
    if (strcmp(name, ar->getClassScope()->getName().c_str()) == 0) {
      cg_printf("HASH_GUARD(0x%016llXLL, %s) { return %s%sPrivate(prop, "
          "phash%s); }\n",
          hash_string_i(name), name, Option::ObjectPrefix, op, args);
    } else {
      cg_printf("HASH_GUARD(0x%016llXLL, %s) { return %s%s::%s%sPrivate(prop, "
          "phash%s); }\n",
          hash_string_i(name), name, Option::ClassPrefix, name,
          Option::ObjectPrefix, op, args);
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
  cg_indentBegin("%s %s%s::%s%s(CStrRef prop, int64 phash%s, "
      "const char *context /* = NULL */)%s {\n", ret, Option::ClassPrefix,
      cls, Option::ObjectPrefix, op, argsDec, cnst ? " const" : "");
  if (!outputCPPPrivateSelector(cg, ar, op, args)) {
    m_emptyJumpTables.insert(jtname);
  }
  if (!dynamicObject) {
    cg_printf("return %s%s::%s%sPublic(prop, phash%s);\n",
              Option::ClassPrefix, cls, Option::ObjectPrefix, op, args);
  } else {
    cg_printf("return DynamicObjectData::%s%s(prop, phash%s, context);\n",
              Option::ObjectPrefix, op, args);
  }
  cg_indentEnd("}\n");
  cg.ifdefEnd("OMIT_JUMP_TABLE_CLASS_%s_%s", op, cls);

  if (!dynamicObject) {
    cg.ifdefBegin(false, "OMIT_JUMP_TABLE_CLASS_%s_PUBLIC_%s", op, cls);
    cg_indentBegin("%s %s%s::%s%sPublic(CStrRef s, int64 hash%s)%s {\n",
                   ret, Option::ClassPrefix, cls,
                   Option::ObjectPrefix, op, argsDec, cnst ? " const" : "");
    if (!outputCPPJumpTable(cg, ar, Option::PropertyPrefix, false, varOnly,
                            NonStatic, type)) {
      // offset 1 based on enum order
      m_emptyJumpTables.insert((JumpTableName)(jtname + 1));
    }
    cg_printf("return %s%s::%s%sPublic(s, hash%s);\n",
              Option::ClassPrefix, parent, Option::ObjectPrefix, op, args);
    cg_indentEnd("}\n");
    cg.ifdefEnd("OMIT_JUMP_TABLE_CLASS_%s_PUBLIC_%s", op, cls);
  }

  cg.ifdefBegin(false, "OMIT_JUMP_TABLE_CLASS_%s_PRIVATE_%s", op, cls);
  cg_indentBegin("%s %s%s::%s%sPrivate(CStrRef s, int64 hash%s)%s {\n",
                 ret, Option::ClassPrefix, cls, Option::ObjectPrefix, op,
                 argsDec, cnst ? " const" : "");
  if (!outputCPPJumpTable(cg, ar, Option::PropertyPrefix, false, varOnly,
                          NonStatic, type, Private)) {
    // offset 2 based on enum order
    m_emptyJumpTables.insert((JumpTableName)(jtname + 2));
  }
  if (!dynamicObject) {
    // Fall back to public
    cg_printf("return %s%sPublic(s, hash%s);\n",
              Option::ObjectPrefix, op, args);
  } else {
    cg_printf("return DynamicObjectData::%s%s(s, hash%s, \"\");\n",
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
  strings.reserve(m_symbols.size());
  bool hasStatic = false;
  for (unsigned int i = 0; i < m_symbols.size(); i++) {
    const string &name = m_symbols[i];
    bool stat = isStatic(name);
    if (!stat && (isInherited(name) || definedByParent(ar, name))) continue;
    if (!stat &&
        (isPrivate(name) && privateVar == NonPrivate ||
         !isPrivate(name) && privateVar == Private)) continue;

    if ((!variantOnly || Type::SameType(getFinalType(name), Type::Variant)) &&
        (staticVar & (stat ? Static : NonStatic))) {
      hasStatic |= stat;
      if (type == JumpIndex) varIdx[name.c_str()] = strings.size();
      strings.push_back(name.c_str());
    }
  }
  if (strings.empty()) return false;

  if (hasStatic) {
    cg.printDeclareGlobals();
    ClassScopePtr cls = ar->getClassScope();
    if (cls && cls->needLazyStaticInitializer()) {
      cg_printf("lazy_initializer(g);\n");
    }
    if (declaredGlobals) *declaredGlobals = true;
  } else if (type == JumpReturnInit) {
    cg.printDeclareGlobals();
    if (declaredGlobals) *declaredGlobals = true;
  }

  bool useString = (type == JumpExists) || (type == JumpSet) ||
                   (type == JumpReturnString);

  for (JumpTable jt(cg, strings, false, !defineHash, useString); jt.ready();
       jt.next()) {
    const char *name = jt.key();
    const char *symbol_prefix =
      prefix ? prefix : getVariablePrefix(ar, name);
    string varName;
    if (prefix == Option::StaticPropertyPrefix) {
      varName = string(prefix) + ar->getClassScope()->getId(cg) +
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
    const char *priv = "";
    //if (isPrivate(name) && !isStatic(name)) {
      //priv = "_PRIV";
    //}
    switch (type) {
    case VariableTable::JumpExists:
      cg_printf("HASH_EXISTS_STRING%s(0x%016llXLL, \"%s\", %d);\n",
                priv, hash_string(name), cg.escapeLabel(name).c_str(),
                strlen(name));
      break;
    case VariableTable::JumpReturn:
      cg_printf("HASH_RETURN(0x%016llXLL, %s,\n",
                hash_string(name), varName.c_str());
      cg_printf("            \"%s\");\n", cg.escapeLabel(name).c_str());
      break;
    case VariableTable::JumpSet:
      cg_printf("HASH_SET_STRING%s(0x%016llXLL, %s,\n",
          priv, hash_string(name), varName.c_str());
      cg_printf("                \"%s\", %d);\n",
                cg.escapeLabel(name).c_str(), strlen(name));
      break;
    case VariableTable::JumpInitialized:
      cg_printf("HASH_INITIALIZED(0x%016llXLL, %s,\n",
                hash_string(name), varName.c_str());
      cg_printf("                 \"%s\");\n", cg.escapeLabel(name).c_str());
      break;
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
      int stringId = cg.checkLiteralString(name, ar);
      if (stringId >= 0) {
        cg_printf("HASH_RETURN_LITSTR%s(0x%016llXLL, %d, %s,\n",
            priv, hash_string(name), stringId, varName.c_str());
        cg_printf("                   %d);\n", strlen(name));
      } else {
        cg_printf("HASH_RETURN_STRING%s(0x%016llXLL, %s,\n",
            priv, hash_string(name), varName.c_str());
        cg_printf("                   \"%s\", %d);\n",
                  cg.escapeLabel(name).c_str(), strlen(name));
      }
      break;
    }
    case VariableTable::JumpReturnInit:
      ExpressionPtr value =
        dynamic_pointer_cast<Expression>(getClassInitVal(name));
      if (value) {
        cg_printf("HASH_RETURN(0x%016llXLL, \n", hash_string(name));
        cg_printf("            ");
        CodeGenerator::Context oldContext = cg.getContext();
        cg.setContext(CodeGenerator::CppStaticInitializer);
        value->outputCPP(cg, ar);
        cg.setContext(oldContext);
        cg_printf(", \"%s\");\n", cg.escapeLabel(name).c_str());
      }
      break;
    }
  }

  return true;
}

void VariableTable::outputCPPClassMap(CodeGenerator &cg,
                                      AnalysisResultPtr ar) {
  for (unsigned int i = 0; i < m_symbols.size(); i++) {
    const string &name = m_symbols[i];

    int attribute = ClassInfo::IsNothing;
    if (isProtected(name)) {
      attribute |= ClassInfo::IsProtected;
    } else if (isPrivate(name)) {
      attribute |= ClassInfo::IsPrivate;
    } else {
      attribute |= ClassInfo::IsPublic;
    }
    if (isStatic(name)) {
      attribute |= ClassInfo::IsStatic;
    }

    cg_printf("(const char *)0x%04X, \"%s\",\n", attribute,
              cg.escapeLabel(name).c_str());
  }
  cg_printf("NULL,\n");
}

void VariableTable::outputCPPStaticVariables(CodeGenerator &cg,
                                             AnalysisResultPtr ar) {
  for (unsigned int i = 0; i < m_symbols.size(); i++) {
    const string &name = m_symbols[i];
    if (isStatic(name)) {
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
