/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/compiler/analysis/function_scope.h"
#include <folly/Conv.h>
#include <utility>
#include <vector>
#include "hphp/compiler/analysis/analysis_result.h"
#include "hphp/compiler/expression/array_pair_expression.h"
#include "hphp/compiler/expression/constant_expression.h"
#include "hphp/compiler/expression/modifier_expression.h"
#include "hphp/compiler/expression/expression_list.h"
#include "hphp/compiler/expression/function_call.h"
#include "hphp/compiler/expression/scalar_expression.h"
#include "hphp/compiler/expression/unary_op_expression.h"
#include "hphp/compiler/analysis/code_error.h"
#include "hphp/compiler/statement/statement_list.h"
#include "hphp/compiler/analysis/file_scope.h"
#include "hphp/compiler/analysis/variable_table.h"
#include "hphp/compiler/parser/parser.h"
#include "hphp/util/logger.h"
#include "hphp/compiler/option.h"
#include "hphp/compiler/statement/method_statement.h"
#include "hphp/compiler/statement/exp_statement.h"
#include "hphp/compiler/expression/parameter_expression.h"
#include "hphp/compiler/analysis/class_scope.h"
#include "hphp/util/atomic.h"
#include "hphp/runtime/base/class-info.h"
#include "hphp/runtime/base/type-conversions.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/parser/hphp.tab.hpp"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/base/zend-string.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////

FunctionScope::FunctionScope(AnalysisResultConstPtr ar, bool method,
                             const std::string &name, StatementPtr stmt,
                             bool reference, int minParam, int numDeclParam,
                             ModifierExpressionPtr modifiers,
                             int attribute, const std::string &docComment,
                             FileScopePtr file,
                             const std::vector<UserAttributePtr> &attrs,
                             bool inPseudoMain /* = false */)
    : BlockScope(name, docComment, stmt, BlockScope::FunctionScope),
      m_minParam(minParam), m_numDeclParams(numDeclParam),
      m_attribute(attribute), m_modifiers(modifiers), m_hasVoid(false),
      m_method(method), m_refReturn(reference), m_virtual(false),
      m_hasOverride(false), m_overriding(false),
      m_volatile(false), m_persistent(false), m_pseudoMain(inPseudoMain),
      m_magicMethod(false), m_system(false),
      m_containsThis(false), m_containsBareThis(0),
      m_generator(false),
      m_async(false),
      m_noLSB(false), m_nextLSB(false),
      m_hasTry(false), m_hasGoto(false), m_localRedeclaring(false),
      m_redeclaring(-1), m_inlineIndex(0), m_optFunction(0), m_nextID(0) {
  init(ar);

  for (unsigned i = 0; i < attrs.size(); ++i) {
    if (m_userAttributes.find(attrs[i]->getName()) != m_userAttributes.end()) {
      attrs[i]->parseTimeFatal(file,
                               Compiler::DeclaredAttributeTwice,
                               "Redeclared attribute %s",
                               attrs[i]->getName().c_str());
    }
    m_userAttributes[attrs[i]->getName()] = attrs[i]->getExp();
  }

  // Support for systemlib functions implemented in PHP
  if (!m_method &&
      m_userAttributes.find("__Overridable") != m_userAttributes.end()) {
    setAllowOverride();
  }

  // Try to find if the function have __Native("VariadicByRef")
  auto params = getUserAttributeStringParams("__native");
  for (auto &param : params) {
    if (param.compare("VariadicByRef") == 0) {
      setVariableArgument(1);
      break;
    }
  }

  if (isNative()) {
    // Support ParamCoerceMode in HNI
    if (hasUserAttr("__ParamCoerceModeFalse")) {
      setClassInfoAttribute(ClassInfo::ParamCoerceModeFalse);
    } else {
      // Default for HNI is __ParamCoerceModeNull
      setClassInfoAttribute(ClassInfo::ParamCoerceModeNull);
    }
  }
}

FunctionScope::FunctionScope(FunctionScopePtr orig,
                             AnalysisResultConstPtr ar,
                             const string &name,
                             const string &originalName,
                             StatementPtr stmt,
                             ModifierExpressionPtr modifiers,
                             bool user)
    : BlockScope(name, orig->m_docComment, stmt,
                 BlockScope::FunctionScope),
      m_minParam(orig->m_minParam), m_numDeclParams(orig->m_numDeclParams),
      m_attribute(orig->m_attribute), m_modifiers(modifiers),
      m_userAttributes(orig->m_userAttributes), m_hasVoid(orig->m_hasVoid),
      m_method(orig->m_method), m_refReturn(orig->m_refReturn),
      m_virtual(orig->m_virtual), m_hasOverride(orig->m_hasOverride),
      m_overriding(orig->m_overriding), m_volatile(orig->m_volatile),
      m_persistent(orig->m_persistent),
      m_pseudoMain(orig->m_pseudoMain), m_magicMethod(orig->m_magicMethod),
      m_system(!user),
      m_containsThis(orig->m_containsThis),
      m_containsBareThis(orig->m_containsBareThis),
      m_generator(orig->m_generator),
      m_async(orig->m_async),
      m_noLSB(orig->m_noLSB),
      m_nextLSB(orig->m_nextLSB), m_hasTry(orig->m_hasTry),
      m_hasGoto(orig->m_hasGoto), m_localRedeclaring(orig->m_localRedeclaring),
      m_redeclaring(orig->m_redeclaring),
      m_inlineIndex(orig->m_inlineIndex), m_optFunction(orig->m_optFunction),
      m_nextID(0) {
  init(ar);
  m_originalName = originalName;
  setParamCounts(ar, m_minParam, m_numDeclParams);
}

void FunctionScope::init(AnalysisResultConstPtr ar) {
  m_dynamicInvoke = false;
  if (m_pseudoMain) {
    m_variables->forceVariants(ar, VariableTable::AnyVars);
    setReturnType(ar, Type::Variant);
  }

  if (m_refReturn) {
    m_returnType = Type::Variant;
  }

  if (!strcasecmp(m_name.c_str(), "__autoload")) {
    setVolatile();
  }

  // FileScope's flags are from parser, but VariableTable has more flags
  // coming from type inference phase. So we are tranferring these flags
  // just for better modularization between FileScope and VariableTable.
  if (m_attribute & FileScope::ContainsDynamicVariable) {
    m_variables->setAttribute(VariableTable::ContainsDynamicVariable);
  }
  if (m_attribute & FileScope::ContainsLDynamicVariable) {
    m_variables->setAttribute(VariableTable::ContainsLDynamicVariable);
  }
  if (m_attribute & FileScope::ContainsExtract) {
    m_variables->setAttribute(VariableTable::ContainsExtract);
  }
  if (m_attribute & FileScope::ContainsAssert) {
    m_variables->setAttribute(VariableTable::ContainsAssert);
  }
  if (m_attribute & FileScope::ContainsCompact) {
    m_variables->setAttribute(VariableTable::ContainsCompact);
  }
  if (m_attribute & FileScope::ContainsUnset) {
    m_variables->setAttribute(VariableTable::ContainsUnset);
  }
  if (m_attribute & FileScope::ContainsGetDefinedVars) {
    m_variables->setAttribute(VariableTable::ContainsGetDefinedVars);
  }

  if (m_stmt && Option::AllVolatile && !m_pseudoMain && !m_method) {
    m_volatile = true;
  }

  m_dynamic = Option::IsDynamicFunction(m_method, m_name) ||
    Option::EnableEval == Option::FullEval || Option::AllDynamic;
  if (!m_method && Option::DynamicInvokeFunctions.find(m_name) !=
      Option::DynamicInvokeFunctions.end()) {
    setDynamicInvoke();
  }
  if (m_modifiers) {
    m_virtual = m_modifiers->isAbstract();
  }

  if (m_stmt) {
    MethodStatementPtr stmt = dynamic_pointer_cast<MethodStatement>(m_stmt);
    StatementListPtr stmts = stmt->getStmts();
    if (stmts) {
      for (int i = 0; i < stmts->getCount(); i++) {
        StatementPtr stmt = (*stmts)[i];
        stmt->setFileLevel();
        if (stmt->is(Statement::KindOfExpStatement)) {
          ExpStatementPtr expStmt = dynamic_pointer_cast<ExpStatement>(stmt);
          ExpressionPtr exp = expStmt->getExpression();
          exp->setTopLevel();
        }
      }
    }
  }
}

FunctionScope::FunctionScope(bool method, const std::string &name,
                             bool reference)
    : BlockScope(name, "", StatementPtr(), BlockScope::FunctionScope),
      m_minParam(0), m_numDeclParams(0), m_attribute(0),
      m_modifiers(ModifierExpressionPtr()), m_hasVoid(false),
      m_method(method), m_refReturn(reference), m_virtual(false),
      m_hasOverride(false), m_overriding(false),
      m_volatile(false), m_persistent(false), m_pseudoMain(false),
      m_magicMethod(false), m_system(true),
      m_containsThis(false), m_containsBareThis(0),
      m_generator(false),
      m_async(false),
      m_noLSB(false), m_nextLSB(false),
      m_hasTry(false), m_hasGoto(false), m_localRedeclaring(false),
      m_redeclaring(-1), m_inlineIndex(0),
      m_optFunction(0) {
  m_dynamic = Option::IsDynamicFunction(method, m_name) ||
    Option::EnableEval == Option::FullEval || Option::AllDynamic;
  m_dynamicInvoke = false;
  if (!method && Option::DynamicInvokeFunctions.find(m_name) !=
      Option::DynamicInvokeFunctions.end()) {
    setDynamicInvoke();
  }
}

void FunctionScope::setDynamicInvoke() {
  m_returnType = Type::Variant;
  m_dynamicInvoke = true;
}

void FunctionScope::setParamCounts(AnalysisResultConstPtr ar, int minParam,
                                   int numDeclParam) {
  if (minParam >= 0) {
    m_minParam = minParam;
    m_numDeclParams = numDeclParam;
  } else {
    assert(numDeclParam == minParam);
  }
  assert(m_minParam >= 0 && m_numDeclParams >= m_minParam);
  assert(IMPLIES(hasVariadicParam(), m_numDeclParams > 0));
  if (m_numDeclParams > 0) {
    m_paramNames.resize(m_numDeclParams);
    m_paramTypes.resize(m_numDeclParams);
    m_refs.resize(m_numDeclParams);

    if (m_stmt) {
      MethodStatementPtr stmt = dynamic_pointer_cast<MethodStatement>(m_stmt);
      ExpressionListPtr params = stmt->getParams();

      for (int i = 0; i < m_numDeclParams; i++) {
        if (stmt->isRef(i)) m_refs[i] = true;

        ParameterExpressionPtr param =
          dynamic_pointer_cast<ParameterExpression>((*params)[i]);
        m_paramNames[i] = param->getName();
      }
      assert(m_paramNames.size() == m_numDeclParams);
    }
  }
}

bool FunctionScope::hasUserAttr(const char *attr) const {
  return m_userAttributes.find(attr) != m_userAttributes.end();
}

bool FunctionScope::isParamCoerceMode() const {
  return m_attributeClassInfo &
    (ClassInfo::ParamCoerceModeNull | ClassInfo::ParamCoerceModeFalse);
}

bool FunctionScope::isPublic() const {
  return m_modifiers && m_modifiers->isPublic();
}

bool FunctionScope::isProtected() const {
  return m_modifiers && m_modifiers->isProtected();
}

bool FunctionScope::isPrivate() const {
  return m_modifiers && m_modifiers->isPrivate();
}

bool FunctionScope::isStatic() const {
  return m_modifiers && m_modifiers->isStatic();
}

bool FunctionScope::isAbstract() const {
  return m_modifiers && m_modifiers->isAbstract();
}

bool FunctionScope::isNative() const {
  return hasUserAttr("__native");
}

bool FunctionScope::isFinal() const {
  return m_modifiers && m_modifiers->isFinal();
}

bool FunctionScope::hasVariadicParam() const {
  return (m_attribute & FileScope::VariadicArgumentParam);
}

bool FunctionScope::allowsVariableArguments() const {
  return hasVariadicParam() || usesVariableArgumentFunc();
}

bool FunctionScope::usesVariableArgumentFunc() const {
  bool res = (m_attribute & FileScope::VariableArgument) && !m_overriding;
  return res;
}

bool FunctionScope::allowOverride() const {
  return m_attribute & FileScope::AllowOverride;
}

bool FunctionScope::isReferenceVariableArgument() const {
  bool res = (m_attribute & FileScope::ReferenceVariableArgument) &&
             !m_overriding;
  // If this method returns true, then usesVariableArgumentFunc() must also
  // return true.
  assert(!res || usesVariableArgumentFunc());
  return res;
}

bool FunctionScope::noFCallBuiltin() const {
  bool res = (m_attribute & FileScope::NoFCallBuiltin);
  return res;
}

bool FunctionScope::needsFinallyLocals() const {
  bool res = (m_attribute & FileScope::NeedsFinallyLocals);
  return res;
}

bool FunctionScope::mayContainThis() {
  return inPseudoMain() || getContainingClass() ||
    (isClosure() && !m_modifiers->isStatic());
}

bool FunctionScope::isClosure() const {
  return ParserBase::IsClosureName(name());
}

void FunctionScope::setVariableArgument(int reference) {
  m_attribute |= FileScope::VariableArgument;
  if (reference) {
    m_attribute |= FileScope::ReferenceVariableArgument;
  }
}

void FunctionScope::setAllowOverride() {
  m_attribute |= FileScope::AllowOverride;
}

bool FunctionScope::hasEffect() const {
  return (m_attribute & FileScope::NoEffect) == 0;
}

void FunctionScope::setNoEffect() {
  if (!(m_attribute & FileScope::NoEffect)) {
    m_attribute |= FileScope::NoEffect;
  }
}

bool FunctionScope::isFoldable() const {
  if (m_attribute & FileScope::IsFoldable) {
    // IDL based builtins
    return true;
  }
  // Systemlib (PHP&HNI) builtins
  auto f = Unit::lookupFunc(String(getName()).get());
  return f && f->isFoldable();
}

void FunctionScope::setIsFoldable() {
  m_attribute |= FileScope::IsFoldable;
}

void FunctionScope::setNoFCallBuiltin() {
  m_attribute |= FileScope::NoFCallBuiltin;
}

void FunctionScope::setHelperFunction() {
  m_attribute |= FileScope::HelperFunction;
}

bool FunctionScope::containsReference() const {
  return m_attribute & FileScope::ContainsReference;
}

void FunctionScope::setContainsThis(bool f /* = true */) {
  m_containsThis = f;

  BlockScopePtr bs(this->getOuterScope());
  while (bs && bs->is(BlockScope::FunctionScope)) {
    FunctionScopePtr fs = static_pointer_cast<FunctionScope>(bs);
    if (!fs->isClosure()) {
      break;
    }
    fs->setContainsThis(f);
    bs = bs->getOuterScope();
  }

  for (auto it = m_clonedTraitOuterScope.begin(); it != m_clonedTraitOuterScope.end(); it++) {
    (*it)->setContainsThis(f);
  }
}

void FunctionScope::setContainsBareThis(bool f, bool ref /* = false */) {
  if (f) {
    m_containsBareThis |= ref ? 2 : 1;
  } else {
    m_containsBareThis = 0;
  }

  BlockScopePtr bs(this->getOuterScope());
  while (bs && bs->is(BlockScope::FunctionScope)) {
    FunctionScopePtr fs = static_pointer_cast<FunctionScope>(bs);
    if (!fs->isClosure()) {
      break;
    }
    fs->setContainsBareThis(f, ref);
    bs = bs->getOuterScope();
  }

  for (auto it = m_clonedTraitOuterScope.begin(); it != m_clonedTraitOuterScope.end(); it++) {
    (*it)->setContainsBareThis(f, ref);
  }
}

bool FunctionScope::hasImpl() const {
  if (!isUserFunction()) {
    return !isAbstract();
  }
  if (m_stmt) {
    MethodStatementPtr stmt = dynamic_pointer_cast<MethodStatement>(m_stmt);
    return stmt->getStmts() != nullptr;
  }
  return false;
}

bool FunctionScope::isConstructor(ClassScopePtr cls) const {
  return m_stmt && cls
    && (getName() == "__construct"
     || (cls->classNameCtor() && getName() == cls->getName()));
}

bool FunctionScope::isMagic() const {
  return m_name.size() >= 2 && m_name[0] == '_' && m_name[1] == '_';
}

bool FunctionScope::needsLocalThis() const {
  return containsBareThis() &&
    (inPseudoMain() ||
     containsRefThis() ||
     isStatic() ||
     getVariables()->getAttribute(
       VariableTable::ContainsDynamicVariable));
}

static std::string s_empty;
const string &FunctionScope::getOriginalName() const {
  if (m_pseudoMain) return s_empty;
  if (m_stmt) {
    MethodStatementPtr stmt = dynamic_pointer_cast<MethodStatement>(m_stmt);
    return stmt->getOriginalName();
  }
  return m_originalName;
}

string FunctionScope::getFullName() const {
  if (m_stmt) {
    MethodStatementPtr stmt = dynamic_pointer_cast<MethodStatement>(m_stmt);
    return stmt->getFullName();
  }
  return m_name;
}

string FunctionScope::getOriginalFullName() const {
  if (m_stmt) {
    MethodStatementPtr stmt = dynamic_pointer_cast<MethodStatement>(m_stmt);
    return stmt->getOriginalFullName();
  }
  return m_name;
}

///////////////////////////////////////////////////////////////////////////////

void FunctionScope::addCaller(BlockScopePtr caller,
                              bool careAboutReturn /* = true */) {
  addUse(caller, UseKindCaller);
}

void FunctionScope::addNewObjCaller(BlockScopePtr caller) {
  addUse(caller, UseKindCaller & ~UseKindCallerReturn);
}

bool FunctionScope::mayUseVV() const {
  VariableTableConstPtr variables = getVariables();
  return
    (inPseudoMain() ||
     usesVariableArgumentFunc() ||
     variables->getAttribute(VariableTable::ContainsDynamicVariable) ||
     variables->getAttribute(VariableTable::ContainsExtract) ||
     variables->getAttribute(VariableTable::ContainsAssert) ||
     variables->getAttribute(VariableTable::ContainsCompact) ||
     variables->getAttribute(VariableTable::ContainsGetDefinedVars) ||
     (!Option::EnableHipHopSyntax &&
      variables->getAttribute(VariableTable::ContainsDynamicFunctionCall)));
}

TypePtr FunctionScope::setParamType(AnalysisResultConstPtr ar, int index,
                                    TypePtr type) {
  assert(index >= 0 && index < (int)m_paramTypes.size());
  TypePtr paramType = m_paramTypes[index];

  if (!paramType) paramType = Type::Some;
  type = Type::Coerce(ar, paramType, type);
  if (type && !Type::SameType(paramType, type)) {
    addUpdates(UseKindCallerParam);
    if (!isFirstPass()) {
      Logger::Verbose("Corrected type of parameter %d of %s: %s -> %s",
                      index, m_name.c_str(),
                      paramType->toString().c_str(), type->toString().c_str());
    }
  }
  m_paramTypes[index] = type;
  return type;
}

TypePtr FunctionScope::getParamType(int index) {
  assert(index >= 0 && index < (int)m_paramTypes.size());
  TypePtr paramType = m_paramTypes[index];
  if (!paramType) {
    paramType = Type::Some;
    m_paramTypes[index] = paramType;
  }
  return paramType;
}

bool FunctionScope::isRefParam(int index) const {
  assert(index >= 0 && index < (int)m_refs.size());
  return m_refs[index];
}

void FunctionScope::setRefParam(int index) {
  assert(index >= 0 && index < (int)m_refs.size());
  m_refs[index] = true;
}

bool FunctionScope::hasRefParam(int max) const {
  assert(max >= 0 && max < (int)m_refs.size());
  for (int i = 0; i < max; i++) {
    if (m_refs[i]) return true;
  }
  return false;
}

const std::string &FunctionScope::getParamName(int index) const {
  assert(index >= 0 && index < (int)m_paramNames.size());
  return m_paramNames[index];
}

void FunctionScope::setParamName(int index, const std::string &name) {
  assert(index >= 0 && index < (int)m_paramNames.size());
  m_paramNames[index] = name;
}

void FunctionScope::addModifier(int mod) {
  if (!m_modifiers) {
    m_modifiers = std::make_shared<ModifierExpression>(shared_from_this(),
                                                       Location::Range());
  }
  m_modifiers->add(mod);
}

void FunctionScope::setReturnType(AnalysisResultConstPtr ar, TypePtr type) {
  if (inTypeInference()) {
    getInferTypesMutex().assertOwnedBySelf();
  }
  // no change can be made to virtual function's prototype
  if (m_overriding || m_dynamicInvoke) return;

  if (!type) {
    m_hasVoid = true;
    if (!m_returnType) return;
  }

  if (m_hasVoid) {
    type = Type::Variant;
  } else if (m_returnType) {
    type = Type::Coerce(ar, m_returnType, type);
  }
  m_returnType = type;
  assert(m_returnType);
}

void FunctionScope::setOverriding(TypePtr returnType,
                                  TypePtr param1 /* = TypePtr() */,
                                  TypePtr param2 /* = TypePtr() */) {
  m_returnType = returnType;
  m_overriding = true;

  if (param1 && m_paramTypes.size() >= 1) m_paramTypes[0] = param1;
  if (param2 && m_paramTypes.size() >= 2) m_paramTypes[1] = param2;

  // TODO: remove this block and replace with stronger typing
  // Right now, we have to avoid a situation where a parameter is assigned
  // with different values, making them a Variant.
  for (unsigned int i = 0; i < m_paramTypes.size(); i++) {
    m_paramTypes[i] = Type::Variant;
  }
}

std::vector<std::string> FunctionScope::getUserAttributeStringParams(
    const std::string& key) {

  std::vector<std::string> ret;
  auto native = m_userAttributes.find(key);
  if (native == m_userAttributes.end()) {
    return ret;
  }

  auto arrayExp = static_pointer_cast<UnaryOpExpression>(native->second);
  if (!arrayExp->getExpression()) {
    return ret;
  }

  auto memberExp = static_pointer_cast<ExpressionList>(
                     arrayExp->getExpression());

  for (int i = 0; i < memberExp->getCount(); i++) {
    auto pairExp = dynamic_pointer_cast<ArrayPairExpression>((*memberExp)[i]);
    if (!pairExp) {
      continue;
    }

    auto value = dynamic_pointer_cast<ScalarExpression>(pairExp->getValue());
    if (value) {
      ret.push_back(value->getString());
    }
  }

  return ret;
}

std::string FunctionScope::getDocName() const {
  string name = getOriginalName();
  if (m_redeclaring < 0) {
    return name;
  }
  return name + Option::IdPrefix + folly::to<std::string>(m_redeclaring);
}

std::string FunctionScope::getDocFullName() const {
  FunctionScope *self = const_cast<FunctionScope*>(this);
  const string &docName = getDocName();
  if (ClassScopeRawPtr cls = self->getContainingClass()) {
    return cls->getDocName() + string("::") + docName;
  }
  return docName;
}

///////////////////////////////////////////////////////////////////////////////

void FunctionScope::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  BlockScope::outputPHP(cg, ar);
}

void FunctionScope::serialize(JSON::CodeError::OutputStream &out) const {
  JSON::CodeError::MapStream ms(out);
  int vis = 0;
  if (isPublic()) vis = ClassScope::Public;
  else if (isProtected()) vis = ClassScope::Protected;
  else if (isPrivate()) vis = ClassScope::Protected;

  int mod = 0;
  if (isAbstract()) mod = ClassScope::Abstract;
  else if (isFinal()) mod = ClassScope::Final;

  if (!m_returnType) {
    ms.add("retTp", -1);
  } else if (m_returnType->isSpecificObject()) {
    ms.add("retTp", m_returnType->getName());
  } else {
    ms.add("retTp", m_returnType->getKindOf());
  }
  ms.add("minArgs", m_minParam)
    .add("maxArgs", m_numDeclParams)
    .add("varArgs", allowsVariableArguments())
    .add("static", isStatic())
    .add("modifier", mod)
    .add("visibility", vis)
    .add("argIsRef", m_refs)
    .done();
}

void FunctionScope::serialize(JSON::DocTarget::OutputStream &out) const {
  JSON::DocTarget::MapStream ms(out);

  ms.add("name", getDocName());
  ms.add("line", getStmt() ? getStmt()->line0() : 0);
  ms.add("docs", m_docComment);

  int mods = 0;
  if (isPublic())    mods |= ClassInfo::IsPublic;
  if (isProtected()) mods |= ClassInfo::IsProtected;
  if (isPrivate())   mods |= ClassInfo::IsPrivate;
  if (isStatic())    mods |= ClassInfo::IsStatic;
  if (isFinal())     mods |= ClassInfo::IsFinal;
  if (isAbstract())  mods |= ClassInfo::IsAbstract;
  ms.add("modifiers", mods);

  ms.add("refreturn", isRefReturn());
  ms.add("return",    getReturnType());

  vector<SymParamWrapper> paramSymbols;
  auto const limit = getDeclParamCount();
  for (int i = 0; i < limit; i++) {
    const string &name = getParamName(i);
    const Symbol *sym = getVariables()->getSymbol(name);
    assert(sym && sym->isParameter());
    paramSymbols.push_back(SymParamWrapper(sym));
  }
  ms.add("parameters", paramSymbols);

  // scopes that call this scope (callers)
  vector<string> callers;
  const BlockScopeRawPtrFlagsPtrVec &deps = getDeps();
  for (BlockScopeRawPtrFlagsPtrVec::const_iterator it = deps.begin();
       it != deps.end(); ++it) {
    const BlockScopeRawPtrFlagsPtrPair &p(*it);
    if ((*p.second & BlockScope::UseKindCaller) &&
        p.first->is(BlockScope::FunctionScope)) {
      FunctionScopeRawPtr f(
          static_pointer_cast<FunctionScope>(p.first));
      callers.push_back(f->getDocFullName());
    }
  }
  ms.add("callers", callers);

  // scopes that this scope calls (callees)
  // TODO(stephentu): this list only contains *user* functions,
  // we should also include builtins
  vector<string> callees;
  const BlockScopeRawPtrFlagsVec &users = getOrderedUsers();
  for (BlockScopeRawPtrFlagsVec::const_iterator uit = users.begin();
       uit != users.end(); ++uit) {
    BlockScopeRawPtrFlagsVec::value_type pf = *uit;
    if ((pf->second & BlockScope::UseKindCaller) &&
        pf->first->is(BlockScope::FunctionScope)) {
      FunctionScopeRawPtr f(
          static_pointer_cast<FunctionScope>(pf->first));
      callees.push_back(f->getDocFullName());
    }
  }
  ms.add("callees", callees);

  ms.done();
}

void FunctionScope::getClosureUseVars(
    ParameterExpressionPtrIdxPairVec &useVars,
    bool filterUsed /* = true */) {
  useVars.clear();
  if (!m_closureVars) return;
  assert(isClosure());
  VariableTablePtr variables = getVariables();
  for (int i = 0; i < m_closureVars->getCount(); i++) {
    ParameterExpressionPtr param =
      dynamic_pointer_cast<ParameterExpression>((*m_closureVars)[i]);
    const string &name = param->getName();
    if (!filterUsed || variables->isUsed(name)) {
      useVars.push_back(ParameterExpressionPtrIdxPair(param, i));
    }
  }
}

FunctionScope::StringToFunctionInfoPtrMap FunctionScope::s_refParamInfo;
static Mutex s_refParamInfoLock;

void FunctionScope::RecordFunctionInfo(string fname, FunctionScopePtr func) {
  VariableTablePtr variables = func->getVariables();
  if (Option::WholeProgram) {
    Lock lock(s_refParamInfoLock);
    FunctionInfoPtr &info = s_refParamInfo[fname];
    if (!info) {
      info = FunctionInfoPtr(new FunctionInfo());
    }
    if (func->isStatic()) {
      info->setMaybeStatic();
    }
    if (func->isRefReturn()) {
      info->setMaybeRefReturn();
    }
    if (func->isReferenceVariableArgument()) {
      info->setRefVarArg(func->getMaxParamCount());
    }
    for (int i = 0; i < func->getMaxParamCount(); i++) {
      if (func->isRefParam(i)) info->setRefParam(i);
    }
  }
  auto limit = func->getDeclParamCount();
  for (int i = 0; i < limit; i++) {
    variables->addParam(func->getParamName(i),
                        TypePtr(), AnalysisResultPtr(), ConstructPtr());
  }
}

FunctionScope::FunctionInfoPtr FunctionScope::GetFunctionInfo(string fname) {
  assert(Option::WholeProgram);
  StringToFunctionInfoPtrMap::iterator it = s_refParamInfo.find(fname);
  if (it == s_refParamInfo.end()) {
    return FunctionInfoPtr();
  }
  return it->second;
}
