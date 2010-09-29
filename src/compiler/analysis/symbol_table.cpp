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

#include <compiler/analysis/symbol_table.h>
#include <compiler/analysis/type.h>
#include <compiler/analysis/analysis_result.h>
#include <util/logger.h>
#include <compiler/analysis/class_scope.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/variable_serializer.h>

using namespace std;
using namespace HPHP;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// Symbol
TypePtr Symbol::getFinalType() const {
  if (m_coerced &&
      !m_coerced->is(Type::KindOfSome) && !m_coerced->is(Type::KindOfAny)) {
    return m_coerced;
  }
  return Type::Variant;
}

TypePtr Symbol::CoerceTo(AnalysisResultPtr ar,
                         TypePtr &curType, TypePtr type) {
  if (!curType) {
    curType = type;
  } else {
    curType = Type::Coerce(ar, curType, type);
  }

  return curType;
}

TypePtr Symbol::setType(AnalysisResultPtr ar, BlockScopeRawPtr scope,
                        TypePtr type, bool coerced) {
  if (!type) return type;
  TypePtr oldType = m_coerced;
  if (!oldType) oldType = Type::Some;
  if (!coerced) return oldType;

  type = CoerceTo(ar, m_coerced, type);
  if (!Type::SameType(oldType, type)) {
    scope->addUpdates(isStatic() ?
                      BlockScope::UseKindStaticRef :
                      BlockScope::UseKindNonStaticRef);
  }

  return type;
}

void Symbol::beginLocal() {
  m_prevCoerced = m_coerced;
  m_coerced.reset();
}

void Symbol::endLocal(BlockScopeRawPtr scope) {
  if (!m_prevCoerced) return;
  if (!m_coerced || m_coerced->is(Type::KindOfSome) ||
      m_coerced->is(Type::KindOfAny)) {
    m_coerced = Type::Variant;
  }
  if (!Type::SameType(m_coerced, m_prevCoerced)) {
    scope->addUpdates(isStatic() ?
                      BlockScope::UseKindStaticRef :
                      BlockScope::UseKindNonStaticRef);
  }
  m_prevCoerced.reset();
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
  m_coerced = src_sym.m_coerced;
}

///////////////////////////////////////////////////////////////////////////////
// statics

Mutex SymbolTable::AllSymbolTablesMutex;
SymbolTablePtrVec SymbolTable::AllSymbolTables;

void SymbolTable::CountTypes(std::map<std::string, int> &counts) {
  for (unsigned int i = 0; i < AllSymbolTables.size(); i++) {
    AllSymbolTables[i]->countTypes(counts);
  }
}

///////////////////////////////////////////////////////////////////////////////

SymbolTable::SymbolTable(BlockScope &blockScope) : m_blockScope(blockScope) {
}

SymbolTable::~SymbolTable() {
}

void SymbolTable::import(SymbolTablePtr src) {
  ASSERT(m_symbolMap.empty());

  for (unsigned int i = 0; i < src->m_symbolVec.size(); i++) {
    Symbol &src_sym = *src->m_symbolVec[i];
    Symbol &dst_sym = m_symbolMap[src_sym.getName()];
    m_symbolVec.push_back(&dst_sym);
    dst_sym.import(getBlockScope(), src_sym);
  }
}

void SymbolTable::beginLocal() {
  for (unsigned int i = 0, s = m_symbolVec.size(); i < s; i++) {
    Symbol *sym = m_symbolVec[i];
    sym->beginLocal();
  }
}

void SymbolTable::endLocal() {
  for (unsigned int i = 0, s = m_symbolVec.size(); i < s; i++) {
    Symbol *sym = m_symbolVec[i];
    sym->endLocal(BlockScopeRawPtr(&m_blockScope));
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
  if (Symbol *sym = getSymbol(name)) {
    return sym->isPresent();
  }
  return false;
}

bool SymbolTable::isSystem(const std::string &name) const {
  if (Symbol *sym = getSymbol(name)) {
    return sym->isSystem();
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////

Symbol *SymbolTable::getSymbol(const std::string &name) const {
  std::map<std::string,Symbol>::const_iterator it = m_symbolMap.find(name);
  if (it != m_symbolMap.end()) {
    return const_cast<Symbol*>(&it->second);
  }
  return NULL;
}

Symbol *SymbolTable::getSymbol(const std::string &name, bool add) {
  std::map<std::string,Symbol>::iterator it = m_symbolMap.find(name);
  if (it != m_symbolMap.end()) {
    return &it->second;
  }
  if (add) {
    Symbol *sym = &m_symbolMap[name];
    sym->setName(name);
    return sym;
  }
  return NULL;
}

TypePtr SymbolTable::getType(const std::string &name) {
  if (Symbol *sym = getSymbol(name)) {
    return sym->getType();
  }
  return TypePtr();
}

TypePtr SymbolTable::getFinalType(const std::string &name) {
  if (Symbol *sym = getSymbol(name)) {
    return sym->getFinalType();
  }
  return Type::Variant;
}

bool SymbolTable::isExplicitlyDeclared(const std::string &name) const {
  if (Symbol *sym = getSymbol(name)) {
    return sym->valueSet();
  }
  return false;
}

ConstructPtr SymbolTable::getDeclaration(const std::string &name) {
  if (Symbol *sym = getSymbol(name)) {
    return sym->getDeclaration();
  }
  return ConstructPtr();
}

ConstructPtr SymbolTable::getValue(const std::string &name) {
  if (Symbol *sym = getSymbol(name)) {
    return sym->getValue();
  }
  return ConstructPtr();
}

void SymbolTable::setSepExtension(const std::string &name) {
  getSymbol(name, true)->setSep();
}

bool SymbolTable::isSepExtension(const std::string &name) const {
  if (Symbol *sym = getSymbol(name)) {
    return sym->isSep();
  }
  return false;
}

TypePtr SymbolTable::setType(AnalysisResultPtr ar, const std::string &name,
                             TypePtr type, bool coerced) {
  return setType(ar, getSymbol(name, true), type, coerced);
}

TypePtr SymbolTable::setType(AnalysisResultPtr ar, Symbol *sym,
                             TypePtr type, bool coerced) {
  if (!sym->declarationSet()) {
    m_symbolVec.push_back(sym);
    sym->setDeclaration(ConstructPtr());
  }
  return sym->setType(ar, BlockScopeRawPtr(&m_blockScope), type, coerced);
}

static bool by_name(const Symbol *s1, const Symbol *s2) {
  return s1->getName() < s2->getName();
}

void SymbolTable::canonicalizeSymbolOrder() {
  sort(m_symbolVec.begin(), m_symbolVec.end(), by_name);
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

void SymbolTable::serialize(JSON::OutputStream &out) const {
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
  VariableSerializer vs(VariableSerializer::Serialize);
  String str = vs.serialize(v, true);
  len = str.length();
  string output = Util::escapeStringForCPP(str.data(), len);
  return output;
}
