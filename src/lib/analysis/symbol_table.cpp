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

#include <lib/analysis/symbol_table.h>
#include <lib/analysis/type.h>
#include <lib/analysis/analysis_result.h>
#include <util/logger.h>
#include <lib/analysis/class_scope.h>
#include <cpp/base/type_string.h>
#include <cpp/base/type_variant.h>
#include <cpp/base/variable_serializer.h>

using namespace std;
using namespace HPHP;
using namespace boost;

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
  ASSERT(m_symbols.empty());

  m_symbols = src->m_symbols;
  m_declarations = src->m_declarations;
  m_values = src->m_values;
  m_coerced = src->m_coerced;
  m_rtypes = src->m_rtypes;
  for (unsigned int i = 0; i < m_symbols.size(); i++) {
    m_system.insert(m_symbols[i]);
  }
}

bool SymbolTable::isPresent(const std::string &name) const {
  return m_declarations.find(name) != m_declarations.end();
}

bool SymbolTable::isSystem(const std::string &name) const {
  return m_system.find(name) != m_system.end();
}

///////////////////////////////////////////////////////////////////////////////

TypePtr SymbolTable::getType(const std::string &name, bool coerced) {
  if (coerced) {
    StringToTypePtrMap::const_iterator iter = m_coerced.find(name);
    if (iter != m_coerced.end()) return iter->second;
  } else {
    StringToTypePtrMap::const_iterator iter = m_rtypes.find(name);
    if (iter != m_rtypes.end()) return iter->second;
  }
  return TypePtr();
}

TypePtr SymbolTable::getFinalType(const std::string &name) {
  TypePtr coerced = getType(name, true);
  if (coerced &&
      !coerced->is(Type::KindOfSome) && !coerced->is(Type::KindOfAny)) {
    return coerced;
  }
  return Type::Variant;
}

bool SymbolTable::isExplicitlyDeclared(const std::string &name) const {
  return m_values.find(name) != m_values.end();
}

ConstructPtr SymbolTable::getDeclaration(const std::string &name) {
  StringToConstructPtrMap::const_iterator iter = m_declarations.find(name);
  if (iter == m_declarations.end()) {
    return ConstructPtr();
  }
  return iter->second;
}

ConstructPtr SymbolTable::getValue(const std::string &name) {
  StringToConstructPtrMap::const_iterator iter = m_values.find(name);
  if (iter == m_values.end()) {
    return ConstructPtr();
  }
  return iter->second;
}

TypePtr SymbolTable::coerceTo(AnalysisResultPtr ar,
                              StringToTypePtrMap &typeMap,
                              const std::string &name, TypePtr type) {
  StringToTypePtrMap::iterator iter = typeMap.find(name);
  if (iter == typeMap.end()) {
    typeMap[name] = type;
    return type;
  }

  TypePtr newType = Type::Coerce(ar, iter->second, type);
  if (!Type::SameType(iter->second, newType)) {
    iter->second = newType;
  }
  return newType;
}

TypePtr SymbolTable::setType(AnalysisResultPtr ar, const std::string &name,
                             TypePtr type, bool coerced) {
  TypePtr oldType = getType(name, true);
  if (!oldType) oldType = NEW_TYPE(Some);

  if (m_declarations.find(name) == m_declarations.end()) {
    m_symbols.push_back(name);
    m_declarations[name] = ConstructPtr();
  }
  if (type) {
    TypePtr ret = coerceTo(ar, coerced? m_coerced : m_rtypes, name, type);
    TypePtr newType = getType(name, true);
    if (!newType) newType = NEW_TYPE(Some);
    if (!Type::SameType(oldType, newType)) {
      ar->incNewlyInferred();
    }
    return newType;
  }
  return type;
}

void SymbolTable::getSymbols(vector<string> &syms) {
  BOOST_FOREACH(string sym, m_symbols) {
    syms.push_back(sym);
  }
}

///////////////////////////////////////////////////////////////////////////////

void SymbolTable::serialize(JSON::OutputStream &out) const {
  out << m_symbols << m_coerced << m_rtypes;
}

void SymbolTable::countTypes(std::map<std::string, int> &counts) {
  for (unsigned int i = 0; i < m_symbols.size(); i++) {
    const string &name = m_symbols[i];
    if (!isInherited(name)) {
      getFinalType(name)->count(counts);
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
    case '\0': output += "\\0";  break;
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
