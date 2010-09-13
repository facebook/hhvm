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
  TypePtr coerced = getType(true);
  if (coerced &&
      !coerced->is(Type::KindOfSome) && !coerced->is(Type::KindOfAny)) {
    return coerced;
  }
  return Type::Variant;
}

TypePtr Symbol::coerceTo(AnalysisResultPtr ar,
                         TypePtr &curType, TypePtr type) {
  if (!curType) {
    curType = type;
  } else {
    curType = Type::Coerce(ar, curType, type);
  }

  return curType;
}

TypePtr Symbol::setType(AnalysisResultPtr ar, TypePtr type, bool coerced) {
  TypePtr oldType = getType(true);
  if (!oldType) oldType = NEW_TYPE(Some);
  if (type) {
    TypePtr ret = coerceTo(ar, coerced ? m_coerced : m_rtype, type);
    TypePtr newType = getType(true);
    if (!newType) newType = NEW_TYPE(Some);
    if (!Type::SameType(oldType, newType)) {
      ar->incNewlyInferred();
    }
    return newType;
  }
  return TypePtr();
}

///////////////////////////////////////////////////////////////////////////////
// statics

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
    dst_sym.setName(src_sym.getName());
    dst_sym.setSystem();
    if (src_sym.declarationSet()) {
      dst_sym.setDeclaration(src_sym.getDeclaration());
    }
    if (src_sym.valueSet()) {
      dst_sym.setValue(src_sym.getValue());
    }
    dst_sym.setCoerced(src_sym.getCoerced());
    dst_sym.setRType(src_sym.getRType());
  }
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

TypePtr SymbolTable::getType(const std::string &name, bool coerced) {
  if (Symbol *sym = getSymbol(name)) {
    return sym->getType(coerced);
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
  return sym->setType(ar, type, coerced);
}

void SymbolTable::getSymbols(vector<string> &syms) const {
  BOOST_FOREACH(Symbol *sym, m_symbolVec) {
    syms.push_back(sym->getName());
  }
}

void SymbolTable::getCoerced(StringToTypePtrMap &coerced) const {
  BOOST_FOREACH(Symbol *sym, m_symbolVec) {
    coerced[sym->getName()] = sym->getCoerced();
  }
}

void SymbolTable::getRTypes(StringToTypePtrMap &rtypes) const {
  BOOST_FOREACH(Symbol *sym, m_symbolVec) {
    rtypes[sym->getName()] = sym->getRType();
  }
}

///////////////////////////////////////////////////////////////////////////////

void SymbolTable::serialize(JSON::OutputStream &out) const {
  vector<string> symbols;
  StringToTypePtrMap coerced;
  StringToTypePtrMap rtypes;
  getSymbols(symbols);
  getCoerced(coerced);
  getRTypes(rtypes);

  out << symbols << coerced << rtypes;
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
  string output;
  for (int64 i = 0; i < str.length(); i++) {
    unsigned char ch = str.charAt(i);
    switch (ch) {
    case '\n': output += "\\n";  break;
    case '\r': output += "\\r";  break;
    case '\t': output += "\\t";  break;
    case '\a': output += "\\a";  break;
    case '\b': output += "\\b";  break;
    case '\f': output += "\\f";  break;
    case '\v': output += "\\v";  break;
    case '\"': output += "\\\""; break;
    case '\\': output += "\\\\"; break;
    case '?':  output += "\\?";  break;
    default:
      if (isprint(ch)) {
        output += ch;
      } else {
        // output in octal notation
        char buf[10];
        snprintf(buf, sizeof(buf), "\\%03o", ch);
        output += buf;
      }
    }
  }
  len = str.length();
  return output;
}
