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

#ifndef __SYMBOL_TABLE_H__
#define __SYMBOL_TABLE_H__

#include <compiler/hphp.h>
#include <util/json.h>
#include <util/util.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class BlockScope;
class CodeGenerator;
class Variant;
DECLARE_BOOST_TYPES(Construct);
DECLARE_BOOST_TYPES(Type);
DECLARE_BOOST_TYPES(AnalysisResult);
DECLARE_BOOST_TYPES(SymbolTable);

/**
 * Base class of VariableTable and ConstantTable.
 */
class SymbolTable : public boost::enable_shared_from_this<SymbolTable>,
                    public JSON::ISerializable {
public:
  static SymbolTablePtrVec AllSymbolTables; // for stats purpose
  static void CountTypes(std::map<std::string, int> &counts);
  BlockScope *getScope() const { return &m_blockScope; }

public:
  SymbolTable(BlockScope &blockScope);
  SymbolTable();
  virtual ~SymbolTable();

  /**
   * Import system symbols into this.
   */
  void import(SymbolTablePtr src);

  /**
   * Whether it's defined.
   */
  bool isPresent(const std::string &name) const;

  /**
   * Whether a symbol is a system symbol.
   */
  bool isSystem(const std::string &name) const;

  /**
   * Whether or not this is inherited from a parent scope. For examples,
   * properties from base classes or a global variable.
   */
  virtual bool isInherited(const std::string &name) const {
    return isSystem(name);
  }

  /**
   * Implements JSON::ISerializable.
   */
  virtual void serialize(JSON::OutputStream &out) const;

  /**
   * Find a symbol's inferred type.
   */
  TypePtr getType(const std::string &name, bool coerced);
  TypePtr getFinalType(const std::string &name);

  /**
   * Find declaration construct.
   */
  bool isExplicitlyDeclared(const std::string &name) const;
  ConstructPtr getDeclaration(const std::string &name);
  ConstructPtr getValue(const std::string &name);

  /* Whether this constant is brought in by a separable extension */
  void setSepExtension(const std::string &name) { m_sep.insert(name);}
  bool isSepExtension(const std::string &name) const {
    return m_sep.find(name) != m_sep.end();
  }

  /**
   * How big of a hash table for generate C++ switch statements.
   */
  int getJumpTableSize() const {
    return Util::roundUpToPowerOfTwo(m_symbols.size() * 2);
  }

  void getSymbols(std::vector<std::string> &syms);

  virtual TypePtr setType(AnalysisResultPtr ar, const std::string &name,
                          TypePtr type, bool coerced);
protected:
  BlockScope &m_blockScope;     // parent
  std::vector<std::string> m_symbols; // in declaration order
  std::set<std::string> m_system;
  std::set<std::string> m_sep;
  StringToConstructPtrMap m_declarations; // declarations
  StringToConstructPtrMap m_values; // explicit declarations
  StringToTypePtrMap m_coerced; // symbols that have been coerced
  StringToTypePtrMap m_rtypes;  // r-value types for m_symbols

  TypePtr coerceTo(AnalysisResultPtr ar, StringToTypePtrMap &typeMap,
                   const std::string &name, TypePtr type);
  void countTypes(std::map<std::string, int> &counts);
  std::string getEscapedText(Variant v, int &len);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __SYMBOL_TABLE_H__
