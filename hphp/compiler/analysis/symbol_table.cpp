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

#include "hphp/compiler/analysis/symbol_table.h"
#include <map>
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
#include "hphp/runtime/base/variable-serializer.h"

#include "hphp/util/logger.h"
#include "hphp/util/text-util.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// Symbol

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
      return p->defaultValue()->getText(ar);
    }
  default:
    // TODO(stephentu): this doesn't allow us to tell the difference between
    // something like:
    //   class X { public $x;        } versus
    //   class X { public $x = null; }
    // we'll just end up treating both cases like the latter
    return e->getText(ar);
  }
  return "";
}

void Symbol::serializeParam(JSON::DocTarget::OutputStream &out) const {
  assert(isParameter());

  JSON::DocTarget::MapStream ms(out);
  ms.add("name",       m_name);
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
  ms.add("line", m_declaration ? m_declaration->line0() : 0);

  int mods = 0;
  if (isPublic())    mods |= ClassInfo::IsPublic;
  if (isProtected()) mods |= ClassInfo::IsProtected;
  if (isPrivate())   mods |= ClassInfo::IsPrivate;
  if (isStatic())    mods |= ClassInfo::IsStatic;
  ms.add("modifiers", mods);

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

SymbolTable::SymbolTable(BlockScope &blockScope) :
    m_blockScope(blockScope) {
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
  for (Symbol *sym: m_symbolVec) {
    if (!filterHidden || !sym->isHidden()) syms.push_back(sym);
  }
}

void SymbolTable::getSymbols(vector<string> &syms) const {
  for (Symbol *sym: m_symbolVec) {
    syms.push_back(sym->getName());
  }
}

///////////////////////////////////////////////////////////////////////////////

void SymbolTable::serialize(JSON::CodeError::OutputStream &out) const {
  vector<string> symbols;
  getSymbols(symbols);

  out << symbols;
}

string SymbolTable::getEscapedText(Variant v, int &len) {
  VariableSerializer vs(VariableSerializer::Type::Serialize);
  String str = vs.serialize(v, true);
  len = str.length();
  string output = escapeStringForCPP(str.data(), len);
  return output;
}
