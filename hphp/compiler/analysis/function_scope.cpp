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
                             bool reference, int minParam, int maxParam,
                             ModifierExpressionPtr modifiers,
                             int attribute, const std::string &docComment,
                             FileScopePtr file,
                             const std::vector<UserAttributePtr> &attrs,
                             bool inPseudoMain /* = false */)
    : BlockScope(name, docComment, stmt, BlockScope::FunctionScope),
      m_minParam(minParam), m_maxParam(maxParam), m_attribute(attribute),
      m_modifiers(modifiers), m_hasVoid(false),
      m_method(method), m_refReturn(reference), m_virtual(false),
      m_hasOverride(false), m_perfectVirtual(false), m_overriding(false),
      m_volatile(false), m_persistent(false), m_pseudoMain(inPseudoMain),
      m_magicMethod(false), m_system(false), m_inlineable(false),
      m_containsThis(false), m_containsBareThis(0), m_nrvoFix(true),
      m_inlineAsExpr(false), m_inlineSameContext(false),
      m_contextSensitive(false),
      m_directInvoke(false),
      m_generator(false),
      m_async(false),
      m_noLSB(false), m_nextLSB(false),
      m_hasTry(false), m_hasGoto(false), m_localRedeclaring(false),
      m_redeclaring(-1), m_inlineIndex(0), m_optFunction(0), m_nextID(0) {
  init(ar);
  for (unsigned i = 0; i < attrs.size(); ++i) {
    if (m_userAttributes.find(attrs[i]->getName()) != m_userAttributes.end()) {
      attrs[i]->parseTimeFatal(Compiler::DeclaredAttributeTwice,
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
      m_minParam(orig->m_minParam), m_maxParam(orig->m_maxParam),
      m_attribute(orig->m_attribute), m_modifiers(modifiers),
      m_userAttributes(orig->m_userAttributes), m_hasVoid(orig->m_hasVoid),
      m_method(orig->m_method), m_refReturn(orig->m_refReturn),
      m_virtual(orig->m_virtual), m_hasOverride(orig->m_hasOverride),
      m_perfectVirtual(orig->m_perfectVirtual),
      m_overriding(orig->m_overriding), m_volatile(orig->m_volatile),
      m_persistent(orig->m_persistent),
      m_pseudoMain(orig->m_pseudoMain), m_magicMethod(orig->m_magicMethod),
      m_system(!user), m_inlineable(orig->m_inlineable),
      m_containsThis(orig->m_containsThis),
      m_containsBareThis(orig->m_containsBareThis), m_nrvoFix(orig->m_nrvoFix),
      m_inlineAsExpr(orig->m_inlineAsExpr),
      m_inlineSameContext(orig->m_inlineSameContext),
      m_contextSensitive(orig->m_contextSensitive),
      m_directInvoke(orig->m_directInvoke),
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
  setParamCounts(ar, m_minParam, m_maxParam);
}

void FunctionScope::init(AnalysisResultConstPtr ar) {
  m_dynamicInvoke = false;
  bool canInline = true;
  if (m_pseudoMain) {
    canInline = false;
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
      if (stmts->getRecursiveCount() > Option::InlineFunctionThreshold)
        canInline = false;
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
  } else {
    canInline = false;
  }
  m_inlineable = canInline;
}

FunctionScope::FunctionScope(bool method, const std::string &name,
                             bool reference)
    : BlockScope(name, "", StatementPtr(), BlockScope::FunctionScope),
      m_minParam(0), m_maxParam(0), m_attribute(0),
      m_modifiers(ModifierExpressionPtr()), m_hasVoid(false),
      m_method(method), m_refReturn(reference), m_virtual(false),
      m_hasOverride(false), m_perfectVirtual(false), m_overriding(false),
      m_volatile(false), m_persistent(false), m_pseudoMain(false),
      m_magicMethod(false), m_system(true), m_inlineable(false),
      m_containsThis(false), m_containsBareThis(0), m_nrvoFix(true),
      m_inlineAsExpr(false), m_inlineSameContext(false),
      m_contextSensitive(false),
      m_directInvoke(false),
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
                                   int maxParam) {
  if (minParam >= 0) {
    m_minParam = minParam;
    m_maxParam = maxParam;
  } else {
    assert(maxParam == minParam);
  }
  assert(m_minParam >= 0 && m_maxParam >= m_minParam);
  if (m_maxParam > 0) {
    m_paramNames.resize(m_maxParam);
    m_paramTypes.resize(m_maxParam);
    m_paramTypeSpecs.resize(m_maxParam);
    m_paramDefaults.resize(m_maxParam);
    m_paramDefaultTexts.resize(m_maxParam);
    m_refs.resize(m_maxParam);

    if (m_stmt) {
      MethodStatementPtr stmt = dynamic_pointer_cast<MethodStatement>(m_stmt);
      ExpressionListPtr params = stmt->getParams();

      for (int i = 0; i < m_maxParam; i++) {
        if (stmt->isRef(i)) m_refs[i] = true;

        ParameterExpressionPtr param =
          dynamic_pointer_cast<ParameterExpression>((*params)[i]);
        m_paramNames[i] = param->getName();
      }
    }
  }
}

void FunctionScope::setParamSpecs(AnalysisResultPtr ar) {
  if (m_maxParam > 0 && m_stmt) {
    MethodStatementPtr stmt = dynamic_pointer_cast<MethodStatement>(m_stmt);
    ExpressionListPtr params = stmt->getParams();

    for (int i = 0; i < m_maxParam; i++) {
      ParameterExpressionPtr param =
        dynamic_pointer_cast<ParameterExpression>((*params)[i]);
      TypePtr specType = param->getTypeSpec(ar, false);
      if (specType &&
          !specType->is(Type::KindOfSome) &&
          !specType->is(Type::KindOfVariant)) {
        m_paramTypeSpecs[i] = specType;
      }
      ExpressionPtr exp = param->defaultValue();
      if (exp) {
        m_paramDefaults[i] = exp->getText(false, false, ar);
      }
    }
  }
}

bool FunctionScope::hasUserAttr(const char *attr) const {
  return m_userAttributes.find(attr) != m_userAttributes.end();
}

bool FunctionScope::isZendParamMode() const {
  return m_attributeClassInfo &
    (ClassInfo::ZendParamModeNull | ClassInfo::ZendParamModeFalse);
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

bool FunctionScope::isVariableArgument() const {
  bool res = (m_attribute & FileScope::VariableArgument) && !m_overriding;
  return res;
}

bool FunctionScope::allowOverride() const {
  return m_attribute & FileScope::AllowOverride;
}

bool FunctionScope::isReferenceVariableArgument() const {
  bool res = (m_attribute & FileScope::ReferenceVariableArgument) &&
             !m_overriding;
  // If this method returns true, then isVariableArgument() must also
  // return true.
  assert(!res || isVariableArgument());
  return res;
}

bool FunctionScope::isMixedVariableArgument() const {
  bool res = (m_attribute & FileScope::MixedVariableArgument) && !m_overriding;
  // If this method returns true, then isReferenceVariableArgument()
  // must also return true.
  assert(!res || isReferenceVariableArgument());
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
    if (reference < 0) {
      m_attribute |= FileScope::MixedVariableArgument;
    }
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
  return (inPseudoMain() ||
          isVariableArgument() ||
          isGenerator() ||
          isAsync() ||
          variables->getAttribute(VariableTable::ContainsDynamicVariable) ||
          variables->getAttribute(VariableTable::ContainsExtract) ||
          variables->getAttribute(VariableTable::ContainsCompact) ||
          variables->getAttribute(VariableTable::ContainsGetDefinedVars) ||
          variables->getAttribute(VariableTable::ContainsDynamicFunctionCall));
}

bool FunctionScope::matchParams(FunctionScopePtr func) {
  // leaving them alone for now
  if (m_overriding || func->m_overriding) return false;
  if (isStatic() || func->isStatic()) return false;

  // conservative here, as we could normalize them into same counts.
  if (m_minParam != func->m_minParam || m_maxParam != func->m_maxParam) {
    return false;
  }
  if (isVariableArgument() != func->isVariableArgument() ||
      isReferenceVariableArgument() != func->isReferenceVariableArgument() ||
      isMixedVariableArgument() != func->isMixedVariableArgument()) {
    return false;
  }

  // needs perfect match for ref, hint and defaults
  for (int i = 0; i < m_maxParam; i++) {
    if (m_refs[i] != func->m_refs[i]) return false;

    TypePtr type1 = m_paramTypeSpecs[i];
    TypePtr type2 = func->m_paramTypeSpecs[i];
    if ((type1 && !type2) || (!type1 && type2) ||
        (type1 && type2 && !Type::SameType(type1, type2))) return false;

    if (m_paramDefaults[i] != func->m_paramDefaults[i]) return false;
  }

  return true;
}

void FunctionScope::setPerfectVirtual() {
  m_virtual = true;
  m_perfectVirtual = true;

  // conservative here, as we could still try to infer types THEN only
  // force variants on non-matching parameters
  m_returnType = Type::Variant;
  for (unsigned int i = 0; i < m_paramTypes.size(); i++) {
    m_paramTypes[i] = Type::Variant;
    m_variables->addLvalParam(m_paramNames[i]);
  }
}

bool FunctionScope::needsClassParam() {
  if (!isStatic()) return false;
  ClassScopeRawPtr cls = getContainingClass();
  if (!ClassScope::NeedStaticArray(cls, FunctionScopeRawPtr(this))) {
    return false;
  }
  return getVariables()->hasStatic();
}

int FunctionScope::inferParamTypes(AnalysisResultPtr ar, ConstructPtr exp,
                                   ExpressionListPtr params, bool &valid) {
  if (!params) {
    if (m_minParam > 0) {
      if (exp->getScope()->isFirstPass()) {
        Compiler::Error(Compiler::TooFewArgument, exp, m_stmt);
      }
      valid = false;
      if (!Option::AllDynamic) setDynamic();
    }
    return 0;
  }

  int ret = 0;
  if (params->getCount() < m_minParam) {
    if (exp->getScope()->isFirstPass()) {
      Compiler::Error(Compiler::TooFewArgument, exp, m_stmt);
    }
    valid = false;
    if (!Option::AllDynamic) setDynamic();
  }
  if (params->getCount() > m_maxParam) {
    if (isVariableArgument()) {
      ret = params->getCount() - m_maxParam;
    } else {
      if (exp->getScope()->isFirstPass()) {
        Compiler::Error(Compiler::TooManyArgument, exp, m_stmt);
      }
      valid = false;
      if (!Option::AllDynamic) setDynamic();
    }
  }

  bool canSetParamType = isUserFunction() && !m_overriding && !m_perfectVirtual;
  for (int i = 0; i < params->getCount(); i++) {
    ExpressionPtr param = (*params)[i];
    if (i < m_maxParam && param->hasContext(Expression::RefParameter)) {
      /**
       * This should be very un-likely, since call time pass by ref is a
       * deprecated, not very widely used (at least in FB codebase) feature.
       */
      TRY_LOCK_THIS();
      Symbol *sym = getVariables()->addSymbol(m_paramNames[i]);
      sym->setLvalParam();
      sym->setCallTimeRef();
    }
    if (valid && param->hasContext(Expression::InvokeArgument)) {
      param->clearContext(Expression::InvokeArgument);
      param->clearContext(Expression::RefValue);
      param->clearContext(Expression::NoRefWrapper);
    }
    bool isRefVararg = (i >= m_maxParam && isReferenceVariableArgument());
    if ((i < m_maxParam && isRefParam(i)) || isRefVararg) {
      param->setContext(Expression::LValue);
      param->setContext(Expression::RefValue);
      param->inferAndCheck(ar, Type::Variant, true);
    } else if (!(param->getContext() & Expression::RefParameter)) {
      param->clearContext(Expression::LValue);
      param->clearContext(Expression::RefValue);
      param->clearContext(Expression::InvokeArgument);
      param->clearContext(Expression::NoRefWrapper);
    }
    TypePtr expType;
    /**
     * Duplicate the logic of getParamType(i), w/o the mutation
     */
    TypePtr paramType(i < m_maxParam && !isZendParamMode() ?
                      m_paramTypes[i] : TypePtr());
    if (!paramType) paramType = Type::Some;
    if (valid && !canSetParamType && i < m_maxParam &&
        (!Option::HardTypeHints || !m_paramTypeSpecs[i])) {
      /**
       * What is this magic, you might ask?
       *
       * Here, we take advantage of implicit conversion from every type to
       * Variant. Essentially, we don't really care what type comes out of this
       * expression since it'll just get converted anyways. Doing it this way
       * allows us to generate less temporaries along the way.
       */
      TypePtr optParamType(paramType->is(Type::KindOfVariant) ?
                           Type::Some : paramType);
      expType = param->inferAndCheck(ar, optParamType, false);
    } else {
      expType = param->inferAndCheck(ar, Type::Some, false);
    }
    if (i < m_maxParam) {
      if (!Option::HardTypeHints || !m_paramTypeSpecs[i]) {
        if (canSetParamType) {
          if (!Type::SameType(paramType, expType) &&
              !paramType->is(Type::KindOfVariant)) {
            TRY_LOCK_THIS();
            paramType = setParamType(ar, i, expType);
          } else {
            // do nothing - how is this safe?  well, if we ever observe
            // paramType == expType, then this means at some point in the past,
            // somebody called setParamType() with expType.  thus, by calling
            // setParamType() again with expType, we contribute no "new"
            // information. this argument also still applies in the face of
            // concurrency
          }
        }
        // See note above. If we have an implemented type, however, we
        // should set the paramType to the implemented type to avoid an
        // un-necessary cast
        if (paramType->is(Type::KindOfVariant)) {
          TypePtr it(param->getImplementedType());
          paramType = it ? it : expType;
        }
        if (valid) {
          if (!Type::IsLegalCast(ar, expType, paramType) &&
              paramType->isNonConvertibleType()) {
            param->inferAndCheck(ar, paramType, true);
          }
          param->setExpectedType(paramType);
        }
      }
    }
    // we do a best-effort check for bad pass-by-reference and do not report
    // error for some vararg case (e.g., array_multisort can have either ref
    // or value for the same vararg).
    if (!isRefVararg || !isMixedVariableArgument()) {
      Expression::CheckPassByReference(ar, param);
    }
  }
  return ret;
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

void FunctionScope::setParamDefault(int index, const char* value, int64_t len,
                                    const std::string &text) {
  assert(index >= 0 && index < (int)m_paramNames.size());
  m_paramDefaults[index] = std::string(value, len);
  m_paramDefaultTexts[index] = text;
}

void FunctionScope::addModifier(int mod) {
  if (!m_modifiers) {
    m_modifiers =
      ModifierExpressionPtr(new ModifierExpression(
                              shared_from_this(), LocationPtr()));
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

void FunctionScope::pushReturnType() {
  getInferTypesMutex().assertOwnedBySelf();
  m_prevReturn = m_returnType;
  m_hasVoid = false;
  if (m_overriding || m_dynamicInvoke || m_perfectVirtual || m_pseudoMain) {
    return;
  }
  m_returnType.reset();
}

bool FunctionScope::popReturnType() {
  getInferTypesMutex().assertOwnedBySelf();
  if (m_overriding || m_dynamicInvoke || m_perfectVirtual || m_pseudoMain) {
    return false;
  }

  if (m_returnType) {
    if (m_prevReturn) {
      if (Type::SameType(m_returnType, m_prevReturn)) {
        m_prevReturn.reset();
        return false;
      }
      Logger::Verbose("Corrected %s's return type %s -> %s",
                      getFullName().c_str(),
                      m_prevReturn->toString().c_str(),
                      m_returnType->toString().c_str());
    } else {
      Logger::Verbose("Set %s's return type %s",
                      getFullName().c_str(),
                      m_returnType->toString().c_str());
    }
  } else if (!m_prevReturn) {
    return false;
  }

  m_prevReturn.reset();
  addUpdates(UseKindCallerReturn);
#ifdef HPHP_INSTRUMENT_TYPE_INF
  ++RescheduleException::s_NumRetTypesChanged;
#endif /* HPHP_INSTRUMENT_TYPE_INF */
  return true;
}

void FunctionScope::resetReturnType() {
  getInferTypesMutex().assertOwnedBySelf();
  if (m_overriding || m_dynamicInvoke || m_perfectVirtual || m_pseudoMain) {
    return;
  }
  m_returnType = m_prevReturn;
  m_prevReturn.reset();
}

void FunctionScope::addRetExprToFix(ExpressionPtr e) {
  m_retExprsToFix.push_back(e);
}

void FunctionScope::clearRetExprs() {
  m_retExprsToFix.clear();
}

void FunctionScope::fixRetExprs() {
  for (ExpressionPtrVec::iterator it = m_retExprsToFix.begin(),
         end = m_retExprsToFix.end(); it != end; ++it) {
    (*it)->setExpectedType(m_returnType);
  }
  m_retExprsToFix.clear();
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
  return name + Option::IdPrefix +
    boost::lexical_cast<std::string>(m_redeclaring);
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
  if (Option::GenerateInferredTypes && m_returnType) {
    cg_printf("// @return %s\n", m_returnType->toString().c_str());
  }

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
    .add("maxArgs", m_maxParam)
    .add("varArgs", isVariableArgument())
    .add("static", isStatic())
    .add("modifier", mod)
    .add("visibility", vis)
    .add("argIsRef", m_refs)
    .done();
}

void FunctionScope::serialize(JSON::DocTarget::OutputStream &out) const {
  JSON::DocTarget::MapStream ms(out);

  ms.add("name", getDocName());
  ms.add("line", getStmt() ? getStmt()->getLocation()->line0 : 0);
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
  for (int i = 0; i < m_maxParam; i++) {
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
  for (int i = 0; i < func->getMaxParamCount(); i++) {
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
