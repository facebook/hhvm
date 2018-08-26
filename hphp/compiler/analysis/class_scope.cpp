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

#include "hphp/compiler/analysis/class_scope.h"

#include "hphp/compiler/analysis/analysis_result.h"
#include "hphp/compiler/analysis/file_scope.h"
#include "hphp/compiler/analysis/function_scope.h"
#include "hphp/compiler/construct.h"
#include "hphp/compiler/expression/class_constant_expression.h"
#include "hphp/compiler/expression/closure_expression.h"
#include "hphp/compiler/expression/constant_expression.h"
#include "hphp/compiler/expression/scalar_expression.h"
#include "hphp/compiler/expression/simple_function_call.h"
#include "hphp/compiler/expression/unary_op_expression.h"
#include "hphp/compiler/option.h"
#include "hphp/compiler/parser/parser.h"
#include "hphp/compiler/statement/class_constant.h"
#include "hphp/compiler/statement/class_require_statement.h"
#include "hphp/compiler/statement/class_variable.h"
#include "hphp/compiler/statement/function_statement.h"
#include "hphp/compiler/statement/interface_statement.h"
#include "hphp/compiler/statement/method_statement.h"
#include "hphp/compiler/statement/statement_list.h"
#include "hphp/compiler/statement/trait_alias_statement.h"
#include "hphp/compiler/statement/trait_prec_statement.h"
#include "hphp/compiler/statement/use_trait_statement.h"

#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/zend-string.h"

#include "hphp/util/text-util.h"

#include <folly/Conv.h>

#include <boost/tuple/tuple.hpp>

#include <map>
#include <set>
#include <unordered_set>
#include <utility>
#include <vector>

using namespace HPHP;
using std::map;

///////////////////////////////////////////////////////////////////////////////

ClassScope::ClassScope(FileScopeRawPtr fs,
                       KindOf kindOf, const std::string &originalName,
                       const std::string &parent,
                       const std::vector<std::string> &bases,
                       const std::string &docComment, StatementPtr stmt,
                       const std::vector<UserAttributePtr> &attrs)
  : BlockScope(originalName, docComment, stmt, BlockScope::ClassScope),
    m_parent(parent), m_bases(bases), m_attribute(0), m_kindOf(kindOf) {

  for (unsigned i = 0; i < attrs.size(); ++i) {
    if (m_userAttributes.find(attrs[i]->getName()) != m_userAttributes.end()) {
      attrs[i]->parseTimeFatal(fs,
                               "Redeclared attribute %s",
                               attrs[i]->getName().c_str());
    }
    m_userAttributes[attrs[i]->getName()] = attrs[i]->getExp();
  }

  assert(m_parent.empty() || (!m_bases.empty() && m_bases[0] == m_parent));
}

// System
ClassScope::ClassScope(AnalysisResultPtr ar,
                       const std::string& originalName,
                       const std::string& parent,
                       const std::vector<std::string>& bases,
                       const std::vector<FunctionScopePtr>& methods)
  : BlockScope(originalName, "", StatementPtr(), BlockScope::ClassScope),
    m_parent(parent), m_bases(bases),
    m_attribute(0), m_kindOf(KindOf::ObjectClass) {
  for (auto f : methods) {
    if (f->isNamed("__construct")) setAttribute(HasConstructor);
    else if (f->isNamed("__destruct")) setAttribute(HasDestructor);
    else if (f->isNamed("__get"))  setAttribute(HasUnknownPropGetter);
    else if (f->isNamed("__set"))  setAttribute(HasUnknownPropSetter);
    else if (f->isNamed("__call")) setAttribute(HasUnknownMethodHandler);
    else if (f->isNamed("__callstatic")) {
      setAttribute(HasUnknownStaticMethodHandler);
    } else if (f->isNamed("__isset")) setAttribute(HasUnknownPropTester);
    else if (f->isNamed("__unset"))   setAttribute(HasPropUnsetter);
    else if (f->isNamed("__invoke"))  setAttribute(HasInvokeMethod);
    addFunction(ar, FileScopeRawPtr(), f);
  }
  setAttribute(Extension);
  setAttribute(System);

  assert(m_parent.empty() || (!m_bases.empty() && m_bases[0] == m_parent));
}

bool ClassScope::isNamed(const char* n) const {
  return !strcasecmp(getOriginalName().c_str(), n);
}

const std::string &ClassScope::getOriginalName() const {
  return m_scopeName;
}

std::string ClassScope::getDocName() const {
  return getOriginalName();
}

std::string ClassScope::getUnmangledScopeName() const {
  static const std::string xhp_prefix{ "xhp_" };
  std::string name{ getScopeName() };

  if (name.compare(0, xhp_prefix.length(), xhp_prefix) == 0) {
    name.replace(0, xhp_prefix.length(), ":");
    replaceAll(name, "__", ":");
    replaceAll(name, "_", "-");
  }

  return name;
}

///////////////////////////////////////////////////////////////////////////////

void ClassScope::derivedMagicMethods(ClassScopePtr super) {
  super->setAttribute(NotFinal);
  if (m_attribute & (HasUnknownPropGetter|
                     MayHaveUnknownPropGetter|
                     InheritsUnknownPropGetter)) {
    super->setAttribute(MayHaveUnknownPropGetter);
  }
  if (m_attribute & (HasUnknownPropSetter|
                     MayHaveUnknownPropSetter|
                     InheritsUnknownPropSetter)) {
    super->setAttribute(MayHaveUnknownPropSetter);
  }
  if (m_attribute & (HasUnknownPropTester|
                     MayHaveUnknownPropTester|
                     InheritsUnknownPropTester)) {
    super->setAttribute(MayHaveUnknownPropTester);
  }
  if (m_attribute & (HasPropUnsetter|
                     MayHavePropUnsetter|
                     InheritsPropUnsetter)) {
    super->setAttribute(MayHavePropUnsetter);
  }
  if (m_attribute & (HasUnknownMethodHandler|
                     MayHaveUnknownMethodHandler|
                     InheritsUnknownMethodHandler)) {
    super->setAttribute(MayHaveUnknownMethodHandler);
  }
  if (m_attribute & (HasUnknownStaticMethodHandler|
                     MayHaveUnknownStaticMethodHandler|
                     InheritsUnknownStaticMethodHandler)) {
    super->setAttribute(MayHaveUnknownStaticMethodHandler);
  }
  if (m_attribute & (HasInvokeMethod|
                     MayHaveInvokeMethod|
                     InheritsInvokeMethod)) {
    super->setAttribute(MayHaveInvokeMethod);
  }
  if (m_attribute & (HasArrayAccess|
                     MayHaveArrayAccess|
                     InheritsArrayAccess)) {
    super->setAttribute(MayHaveArrayAccess);
  }
}

void ClassScope::inheritedMagicMethods(ClassScopePtr super) {
  if (super->m_attribute & UsesUnknownTrait) {
    setAttribute(UsesUnknownTrait);
  }
  if (super->m_attribute &
      (HasUnknownPropGetter|InheritsUnknownPropGetter)) {
    setAttribute(InheritsUnknownPropGetter);
  }
  if (super->m_attribute & (HasUnknownPropSetter|InheritsUnknownPropSetter)) {
    setAttribute(InheritsUnknownPropSetter);
  }
  if (super->m_attribute & (HasUnknownPropTester|InheritsUnknownPropTester)) {
    setAttribute(InheritsUnknownPropTester);
  }
  if (super->m_attribute & (HasPropUnsetter|InheritsPropUnsetter)) {
    setAttribute(InheritsPropUnsetter);
  }
  if (super->m_attribute &
      (HasUnknownMethodHandler|InheritsUnknownMethodHandler)) {
    setAttribute(InheritsUnknownMethodHandler);
  }
  if (super->m_attribute &
      (HasUnknownStaticMethodHandler|InheritsUnknownStaticMethodHandler)) {
    setAttribute(InheritsUnknownStaticMethodHandler);
  }
  if (super->m_attribute & (HasInvokeMethod|InheritsInvokeMethod)) {
    setAttribute(InheritsInvokeMethod);
  }
  if (super->m_attribute & (HasArrayAccess|InheritsArrayAccess)) {
    setAttribute(InheritsArrayAccess);
  }
}

bool ClassScope::addClassRequirement(const std::string &requiredName,
                                     bool isExtends) {
  assert(isTrait() || (isInterface() && isExtends)
         // when flattening traits, their requirements get flattened
         || Option::WholeProgram);

  if (isExtends) {
    if (m_requiredImplements.count(requiredName)) return false;
    m_requiredExtends.insert(requiredName);
  } else {
    if (m_requiredExtends.count(requiredName)) return false;
    m_requiredImplements.insert(requiredName);
  }

  return true;
}

bool ClassScope::hasMethod(const std::string &methodName) const {
  return m_functions.find(methodName) != m_functions.end();
}

bool ClassScope::usesTrait(const std::string &traitName) const {
  for (unsigned i = 0; i < m_usedTraitNames.size(); i++) {
    if (traitName == m_usedTraitNames[i]) {
      return true;
    }
  }
  return false;
}

void ClassScope::setSystem() {
  setAttribute(ClassScope::System);
  for (const auto& func : m_functionsVec) {
    func->setSystem();
  }
}

bool ClassScope::addFunction(AnalysisResultConstRawPtr /*ar*/,
                             FileScopeRawPtr fileScope,
                             FunctionScopePtr funcScope) {
  FunctionScopePtr &func = m_functions[funcScope->getScopeName()];
  if (func) {
    func->getStmt()->parseTimeFatal(fileScope,
                                    "Redeclared method %s::%s",
                                    getScopeName().c_str(),
                                    func->getScopeName().c_str());
  }
  func = funcScope;
  m_functionsVec.push_back(funcScope);
  return true;
}

ModifierExpressionPtr
ClassScope::setModifiers(ModifierExpressionPtr modifiers) {
  ModifierExpressionPtr oldModifiers = m_modifiers;
  m_modifiers = modifiers;
  return oldModifiers;
}
