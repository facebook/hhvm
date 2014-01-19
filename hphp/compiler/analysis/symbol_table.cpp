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

#include "hphp/compiler/analysis/symbol_table.h"
#include "hphp/compiler/analysis/type.h"
#include "hphp/compiler/analysis/analysis_result.h"
#include "hphp/compiler/analysis/class_scope.h"
#include "hphp/compiler/analysis/file_scope.h"
#include "hphp/compiler/analysis/function_scope.h"

#include "hphp/compiler/expression/assignment_expression.h"
#include "hphp/compiler/expression/constant_expression.h"
#include "hphp/compiler/expression/expression_list.h"
#include "hphp/compiler/expression/parameter_expression.h"
#include "hphp/compiler/expression/simple_variable.h"

#include "hphp/runtime/base/class-info.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/variable-serializer.h"

#include "hphp/util/logger.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// Symbol
TypePtr Symbol::getFinalType() const {
  if (m_coerced &&
      !m_coerced->is(Type::KindOfSome) &&
      !m_coerced->is(Type::KindOfAny) &&
      !m_coerced->is(Type::KindOfVoid)) {
    return m_coerced;
  }
  return Type::Variant;
}

TypePtr Symbol::CoerceTo(AnalysisResultConstPtr ar,
                         TypePtr &curType, TypePtr type) {
  if (!curType) {
    curType = type;
  } else {
    curType = Type::Coerce(ar, curType, type);
  }

  return curType;
}

TypePtr Symbol::setType(AnalysisResultConstPtr ar, BlockScopeRawPtr scope,
                        TypePtr type, bool coerced) {
  if (!type) return type;
  if (ar->getPhase() == AnalysisResult::FirstInference) {
    // at this point, you *must* have a lock (if you are user scope)
    if (scope->is(BlockScope::FunctionScope)) {
      FunctionScopeRawPtr f =
        static_pointer_cast<FunctionScope>(scope);
      if (f->isUserFunction()) {
        f->getInferTypesMutex().assertOwnedBySelf();
      }
    } else if (scope->is(BlockScope::ClassScope)) {
      ClassScopeRawPtr c =
        static_pointer_cast<ClassScope>(scope);
      if (c->isUserClass()) {
        c->getInferTypesMutex().assertOwnedBySelf();
      }
    }
  }
  TypePtr oldType = m_coerced;
  if (!oldType) oldType = Type::Some;
  if (!coerced) return oldType;

  type = CoerceTo(ar, m_coerced, type);
  assert(!isRefClosureVar() || (type && type->is(Type::KindOfVariant)));

  if (ar->getPhase() >= AnalysisResult::AnalyzeAll &&
      !Type::SameType(oldType, type)) {
    triggerUpdates(scope);
  }

  return type;
}

void Symbol::beginLocal(BlockScopeRawPtr scope) {
  m_prevCoerced = m_coerced;
  if (isClosureVar()) {
    ExpressionListPtr useVars =
      scope->getContainingFunction()->getClosureVars();
    assert(useVars);
    // linear scan for now, since most use var lists are
    // fairly short
    bool found = false;
    for (int i = 0; i < useVars->getCount(); i++) {
      ParameterExpressionPtr param =
        dynamic_pointer_cast<ParameterExpression>((*useVars)[i]);
      if (m_name == param->getName()) {
        // bootstrap use var with parameter type
        m_coerced = param->getType();
        found = true;
        break;
      }
    }
    if (!found) assert(false);
    assert(!isRefClosureVar() ||
           (m_coerced && m_coerced->is(Type::KindOfVariant)));
  } else {
    m_coerced.reset();
  }
}

void Symbol::resetLocal(BlockScopeRawPtr scope) {
  if (!m_prevCoerced) return;
  if (!m_coerced) {
    // We either A) have not processed this symbol yet or B) we did not process
    // it in lvalue context. Either way, restore the previous type information,
    // since we can get away with it (we haven't broadcast any updates about
    // this symbol's type)
    m_coerced = m_prevCoerced;
    m_prevCoerced.reset();
    return;
  }
  // At this point, we've processed some type information about this symbol.
  // Since we might have broadcast an update about this symbol (it could have
  // been a parameter, constant, or global variable), we need to keep this type
  // information around (even though it is potentially partially incomplete).
  // Note that this is always the conservative thing to do (since we know this
  // scope is going to be run again).
  if (m_coerced->is(Type::KindOfSome) ||
      m_coerced->is(Type::KindOfAny)) {
    m_coerced = Type::Variant;
  }
  if (!Type::SameType(m_coerced, m_prevCoerced)) {
    triggerUpdates(scope);
  }
  m_prevCoerced.reset();
}

void Symbol::endLocal(BlockScopeRawPtr scope) {
  if (!m_prevCoerced) return;
  if (!m_coerced ||
      m_coerced->is(Type::KindOfSome) ||
      m_coerced->is(Type::KindOfAny)) {
    m_coerced = Type::Variant;
  }
  if (!Type::SameType(m_coerced, m_prevCoerced)) {
    triggerUpdates(scope);
  }
  m_prevCoerced.reset();
}

void Symbol::triggerUpdates(BlockScopeRawPtr scope) const {
  int useKind = BlockScope::GetNonStaticRefUseKind(getHash());
  if (isConstant()) {
    useKind = BlockScope::UseKindConstRef;
    if (m_declaration) {
      BlockScopeRawPtr declScope(m_declaration->getScope());

      /**
       * Constants can only belong to a file or class scope
       */
      assert(scope->is(BlockScope::FileScope) ||
             scope->is(BlockScope::ClassScope));

      /**
       * Constants can only be declared in a function or
       * class scope
       */
      assert(declScope->is(BlockScope::FunctionScope) ||
             declScope->is(BlockScope::ClassScope));

      /**
       * For class scopes, the declaration scope *must*
       * match the scope the symbol lives in
       */
      assert(!scope->is(BlockScope::ClassScope) ||
             scope == declScope);

      /**
       * For file scopes, the declaration scope *must*
       * live in a function scope
       */
      assert(!scope->is(BlockScope::FileScope) ||
             declScope->is(BlockScope::FunctionScope));

      /**
       * This is really only for file scopes (constants created with
       * define('FOO', ...)). const FOO = 1 outside of a class is re-written
       * into a define('FOO', 1) by earlier phases of the compiler
       */
      if (scope->is(BlockScope::FileScope)) {
        declScope->announceUpdates(BlockScope::UseKindConstRef);
        return;
      }
    }
  } else if (isStatic()) {
    useKind = BlockScope::UseKindStaticRef;
  } else if (isParameter()) {
    useKind = BlockScope::UseKindCallerParam;
  }
  if (isPassClosureVar()) {
    useKind |= BlockScope::UseKindClosure;
  }
  scope->addUpdates(useKind);
}

void Symbol::import(BlockScopeRawPtr scope, const Symbol &src_sym) {
  setName(src_sym.getName());
  setSystem();
  if (src_sym.declarationSet()) {
    ConstructPtr sc = src_sym.getDeclaration();
    if (sc) sc->resetScope(scope);
    setDeclaration(sc);
  }
  if (src_sym.valueSet()) {
    ConstructPtr sc = src_sym.getValue();
    if (sc) sc->resetScope(scope);
    setValue(sc);
  }
  if (src_sym.isDynamic()) {
    setDynamic();
  }
  if (src_sym.isConstant()) {
    setConstant();
  }
  m_coerced = src_sym.m_coerced;
}

bool Symbol::checkDefined() {
  if (isSystem()) return true;
  always_assert(m_flags.m_declaration_set);
  if (!m_declaration) return false;
  if (!m_declaration.unique()) return true;
  if (!m_flags.m_replaced) {
    Lock lock(BlockScope::s_constMutex);
    setDynamic();
    return false;
  }
  return true;
}

static inline
std::string ExtractInitializer(AnalysisResultPtr ar, ExpressionPtr e) {
  switch (e->getKindOf()) {
  case Expression::KindOfParameterExpression:
    {
      ParameterExpressionPtr p(
        static_pointer_cast<ParameterExpression>(e));
      if (!p->defaultValue()) return "";
      return p->defaultValue()->getText(false, false, ar);
    }
  default:
    // TODO(stephentu): this doesn't allow us to tell the difference between
    // something like:
    //   class X { public $x;        } versus
    //   class X { public $x = null; }
    // we'll just end up treating both cases like the latter
    return e->getText(false, false, ar);
  }
  return "";
}

void Symbol::serializeParam(JSON::DocTarget::OutputStream &out) const {
  assert(isParameter());

  JSON::DocTarget::MapStream ms(out);
  ms.add("name",       m_name);
  ms.add("type",       getFinalType());
  ms.add("referenced", isReferenced());

  ms.add("initializer");
  if (m_value) {
    ExpressionPtr valueExp(
      dynamic_pointer_cast<Expression>(m_value));
    assert(valueExp);
    const string &init = ExtractInitializer(out.analysisResult(), valueExp);
    if (!init.empty()) out << init;
    else               out << JSON::Null();
  } else {
    out << JSON::Null();
  }

  ms.done();
}

static inline std::string ExtractDocComment(ExpressionPtr e) {
  if (!e) return "";
  switch (e->getKindOf()) {
  case Expression::KindOfAssignmentExpression: {
    AssignmentExpressionPtr ae(static_pointer_cast<AssignmentExpression>(e));
    return ExtractDocComment(ae->getVariable());
  }
  case Expression::KindOfSimpleVariable: {
    SimpleVariablePtr sv(static_pointer_cast<SimpleVariable>(e));
    return sv->getDocComment();
  }
  case Expression::KindOfConstantExpression: {
    ConstantExpressionPtr ce(static_pointer_cast<ConstantExpression>(e));
    return ce->getDocComment();
  }
  default: return "";
  }
  return "";
}

void Symbol::serializeClassVar(JSON::DocTarget::OutputStream &out) const {
  assert(!isParameter());

  JSON::DocTarget::MapStream ms(out);
  ms.add("name", m_name);
  ms.add("line", m_declaration ? m_declaration->getLocation()->line0 : 0);

  int mods = 0;
  if (isPublic())    mods |= ClassInfo::IsPublic;
  if (isProtected()) mods |= ClassInfo::IsProtected;
  if (isPrivate())   mods |= ClassInfo::IsPrivate;
  if (isStatic())    mods |= ClassInfo::IsStatic;
  ms.add("modifiers", mods);

  ms.add("type", getFinalType());

  ms.add("initializer");
  if (m_initVal) {
    ExpressionPtr initExp(
      dynamic_pointer_cast<Expression>(m_initVal));
    assert(initExp);
    const string &init = ExtractInitializer(out.analysisResult(), initExp);
    if (!init.empty()) out << init;
    else               out << JSON::Null();
  } else {
    out << JSON::Null();
  }

  const string &docs = ExtractDocComment(
      m_declaration ?
        dynamic_pointer_cast<Expression>(m_declaration) : ExpressionPtr());
  ms.add("docs", docs);

  ms.done();
}

///////////////////////////////////////////////////////////////////////////////
// statics

Mutex SymbolTable::AllSymbolTablesMutex;
SymbolTablePtrList SymbolTable::AllSymbolTables;

void SymbolTable::CountTypes(std::map<std::string, int> &counts) {
  for (SymbolTablePtrList::iterator it = AllSymbolTables.begin(),
         end = AllSymbolTables.end(); it != end; ++it) {
    (*it)->countTypes(counts);
  }
}

void SymbolTable::Purge() {
  Lock lock(AllSymbolTablesMutex);
  AllSymbolTables.clear();
}

///////////////////////////////////////////////////////////////////////////////

SymbolTable::SymbolTable(BlockScope &blockScope, bool isConst) :
    m_blockScope(blockScope), m_const(isConst) {
}

SymbolTable::~SymbolTable() {
}

void SymbolTable::import(SymbolTablePtr src) {
  assert(m_symbolMap.empty());

  for (unsigned int i = 0; i < src->m_symbolVec.size(); i++) {
    Symbol &src_sym = *src->m_symbolVec[i];
    Symbol &dst_sym = m_symbolMap[src_sym.getName()];
    m_symbolVec.push_back(&dst_sym);
    dst_sym.import(getBlockScope(), src_sym);
  }
}

void SymbolTable::beginLocal() {
  BlockScopeRawPtr p(&m_blockScope);
  for (unsigned int i = 0, s = m_symbolVec.size(); i < s; i++) {
    Symbol *sym = m_symbolVec[i];
    sym->beginLocal(p);
  }
}

void SymbolTable::endLocal() {
  BlockScopeRawPtr p(&m_blockScope);
  for (unsigned int i = 0, s = m_symbolVec.size(); i < s; i++) {
    Symbol *sym = m_symbolVec[i];
    sym->endLocal(p);
  }
}

void SymbolTable::resetLocal() {
  BlockScopeRawPtr p(&m_blockScope);
  for (unsigned int i = 0, s = m_symbolVec.size(); i < s; i++) {
    Symbol *sym = m_symbolVec[i];
    sym->resetLocal(p);
  }
}

FileScopeRawPtr SymbolTable::getFileScope() {
  return m_blockScope.getContainingFile();
}

FunctionScopeRawPtr SymbolTable::getFunctionScope() {
  return m_blockScope.getContainingFunction();
}

ClassScopeRawPtr SymbolTable::getClassScope() {
  return m_blockScope.getContainingClass();
}

bool SymbolTable::isPresent(const std::string &name) const {
  if (const Symbol *sym = getSymbol(name)) {
    return sym->isPresent();
  }
  return false;
}

bool SymbolTable::checkDefined(const std::string &name) {
  if (Symbol *sym = getSymbol(name)) {
    return sym->checkDefined();
  }
  return false;
}

bool SymbolTable::isSystem(const std::string &name) const {
  if (const Symbol *sym = getSymbol(name)) {
    return sym->isSystem();
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////

const Symbol *SymbolTable::getSymbolImpl(const std::string &name) const {
  std::map<std::string,Symbol>::const_iterator it = m_symbolMap.find(name);
  if (it != m_symbolMap.end()) {
    return &it->second;
  }
  return nullptr;
}

Symbol *SymbolTable::getSymbol(const std::string &name) {
  return const_cast<Symbol*>(getSymbolImpl(name));
}

const Symbol *SymbolTable::getSymbol(const std::string &name) const {
  return getSymbolImpl(name);
}

Symbol *SymbolTable::genSymbol(const std::string &name, bool konst) {
  std::map<std::string,Symbol>::iterator it = m_symbolMap.find(name);
  if (it != m_symbolMap.end()) {
    assert(konst == it->second.isConstant());
    return &it->second;
  }

  Symbol *sym = &m_symbolMap[name];
  sym->setName(name);
  if (konst) sym->setConstant();
  return sym;
}

Symbol *SymbolTable::genSymbol(const std::string &name, bool konst,
                               ConstructPtr construct) {
  Symbol *sym = genSymbol(name, konst);
  if (!sym->declarationSet() && construct) {
    m_symbolVec.push_back(sym);
    sym->setDeclaration(construct);
  }
  return sym;
}

TypePtr SymbolTable::getType(const std::string &name) const {
  if (const Symbol *sym = getSymbol(name)) {
    return sym->getType();
  }
  return TypePtr();
}

TypePtr SymbolTable::getFinalType(const std::string &name) const {
  if (const Symbol *sym = getSymbol(name)) {
    return sym->getFinalType();
  }
  return Type::Variant;
}

bool SymbolTable::isExplicitlyDeclared(const std::string &name) const {
  if (const Symbol *sym = getSymbol(name)) {
    return sym->valueSet();
  }
  return false;
}

ConstructPtr SymbolTable::getDeclaration(const std::string &name) const {
  if (const Symbol *sym = getSymbol(name)) {
    return sym->getDeclaration();
  }
  return ConstructPtr();
}

ConstructPtr SymbolTable::getValue(const std::string &name) const {
  if (const Symbol *sym = getSymbol(name)) {
    return sym->getValue();
  }
  return ConstructPtr();
}

TypePtr SymbolTable::setType(AnalysisResultConstPtr ar, const std::string &name,
                             TypePtr type, bool coerced) {
  return setType(ar, genSymbol(name, m_const), type, coerced);
}

TypePtr SymbolTable::setType(AnalysisResultConstPtr ar, Symbol *sym,
                             TypePtr type, bool coerced) {
  if (!sym->declarationSet()) {
    m_symbolVec.push_back(sym);
    sym->setDeclaration(ConstructPtr());
  }
  return sym->setType(ar, BlockScopeRawPtr(&m_blockScope), type, coerced);
}

static bool canonicalizeSymbolComp(const Symbol *s1, const Symbol *s2) {
  if (s1->isSystem() && !s2->isSystem()) return true;
  if (!s1->isSystem() && s2->isSystem()) return false;
  return s1->getName() < s2->getName();
}

void SymbolTable::canonicalizeSymbolOrder() {
  sort(m_symbolVec.begin(), m_symbolVec.end(), canonicalizeSymbolComp);
}

void SymbolTable::getSymbols(vector<Symbol*> &syms,
                             bool filterHidden /* = false */) const {
  BOOST_FOREACH(Symbol *sym, m_symbolVec) {
    if (!filterHidden || !sym->isHidden()) syms.push_back(sym);
  }
}

void SymbolTable::getSymbols(vector<string> &syms) const {
  BOOST_FOREACH(Symbol *sym, m_symbolVec) {
    syms.push_back(sym->getName());
  }
}

void SymbolTable::getCoerced(StringToTypePtrMap &coerced) const {
  BOOST_FOREACH(Symbol *sym, m_symbolVec) {
    coerced[sym->getName()] = sym->getType();
  }
}

///////////////////////////////////////////////////////////////////////////////

void SymbolTable::serialize(JSON::CodeError::OutputStream &out) const {
  vector<string> symbols;
  StringToTypePtrMap coerced;
  getSymbols(symbols);
  getCoerced(coerced);

  out << symbols << coerced;
}

void SymbolTable::countTypes(std::map<std::string, int> &counts) {
  for (unsigned int i = 0; i < m_symbolVec.size(); i++) {
    const Symbol *sym = m_symbolVec[i];
    if (!isInherited(sym->getName())) {
      sym->getFinalType()->count(counts);
    }
  }
}

string SymbolTable::getEscapedText(Variant v, int &len) {
  VariableSerializer vs(VariableSerializer::Type::Serialize);
  String str = vs.serialize(v, true);
  len = str.length();
  string output = Util::escapeStringForCPP(str.data(), len);
  return output;
}
