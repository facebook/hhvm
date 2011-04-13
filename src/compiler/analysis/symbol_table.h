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
#include <util/lock.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class BlockScope;
class CodeGenerator;
class Variant;
DECLARE_BOOST_TYPES(Construct);
DECLARE_BOOST_TYPES(Type);
DECLARE_BOOST_TYPES(AnalysisResult);
DECLARE_BOOST_TYPES(SymbolTable);
DECLARE_BOOST_TYPES(FunctionScope);
DECLARE_BOOST_TYPES(ClassScope);
DECLARE_BOOST_TYPES(FileScope);
DECLARE_BOOST_TYPES(BlockScope);

// used by global_variables.h generation in variable_table.cpp
typedef std::list<std::string> SymbolList;
typedef std::map<std::string, std::list<std::string> > Type2SymbolListMap;

class Symbol {
public:
  Symbol() : m_parameter(-1) { m_flags_val = 0; }

  void import(BlockScopeRawPtr scope, const Symbol &src_sym);
  void beginLocal();
  void endLocal(BlockScopeRawPtr scope);

  void setName(const std::string &name) { m_name = name; }
  const std::string &getName() const { return m_name; }

  TypePtr getType() const { return m_coerced; }
  TypePtr getFinalType() const;
  TypePtr setType(AnalysisResultConstPtr ar, BlockScopeRawPtr scope,
                  TypePtr type, bool coerced);

  bool isPresent() const { return m_flags.m_declaration_set; }
  bool checkDefined();
  bool declarationSet() const { return m_flags.m_declaration_set; }
  bool isReplaced() const { return m_flags.m_replaced; }
  void setReplaced() { m_flags.m_replaced = true; }
  bool valueSet() const { return m_flags.m_value_set; }
  bool isSystem() const { return m_flags.m_system; }
  bool isSep() const { return m_flags.m_sep; }
  void setSystem() { m_flags.m_system = true; }
  void setSep() { m_flags.m_sep = true; }
  bool isConstant() const { return m_flags.m_constant; }
  void setConstant() { m_flags.m_constant = true; }

  ConstructPtr getValue() const { return m_value; }
  ConstructPtr getDeclaration() const { return m_declaration; }
  void setValue(ConstructPtr value) {
    m_flags.m_value_set = true;
    m_value = value;
  }
  void setDeclaration(ConstructPtr declaration) {
    m_flags.m_declaration_set = true;
    m_declaration = declaration;
  }

  /* ConstantTable */
  bool isDynamic() const { return m_flags.m_dynamic; }
  void setDynamic() { m_flags.m_dynamic = true; }

  /* VariableTable */
  bool isParameter() const { return m_parameter >= 0; }
  int  getParameterIndex() const { return m_parameter; }
  bool isPublic() const { return !isProtected() && !isPrivate(); }
  bool isProtected() const { return m_flags.m_protected; }
  bool isPrivate() const { return m_flags.m_private; }
  bool isStatic() const { return m_flags.m_static; }
  bool isGlobal() const { return m_flags.m_global; }
  bool isRedeclared() const { return m_flags.m_redeclared; }
  bool isLocalGlobal() const { return m_flags.m_localGlobal; }
  bool isNestedStatic() const { return m_flags.m_nestedStatic; }
  bool isLvalParam() const { return m_flags.m_lvalParam; }
  bool isCallTimeRef() const { return m_flags.m_callTimeRef; }
  bool isUsed() const { return m_flags.m_used; }
  bool isNeeded() const { return m_flags.m_needed; }
  bool isSuperGlobal() const { return m_flags.m_superGlobal; }
  bool isOverride() const { return m_flags.m_override; }
  bool isIndirectAltered() const { return m_flags.m_indirectAltered; }
  bool isReferenced() const { return m_flags.m_referenced; }
  bool isHidden() const { return m_flags.m_hidden; }
  bool isClosureVar() const { return m_flags.m_closureVar; }
  bool isRefClosureVar() const { return m_flags.m_refClosureVar; }
  bool isClassName() const { return m_flags.m_className; }
  bool isShrinkWrapped() const { return m_flags.m_shrinkWrapped; }

  void setParameterIndex(int ix) { m_parameter = ix; }
  void setProtected() { m_flags.m_protected = true; }
  void setPrivate() { m_flags.m_private = true; }
  void setStatic() { m_flags.m_static = true; }
  void setGlobal() { m_flags.m_global = true; }
  void setRedeclared() { m_flags.m_redeclared = true; }
  void setLocalGlobal() { m_flags.m_localGlobal = true; }
  void setNestedStatic() { m_flags.m_nestedStatic = true; }
  void setLvalParam() { m_flags.m_lvalParam = true; }
  void setCallTimeRef() { m_flags.m_callTimeRef = true; }
  void setUsed() { m_flags.m_used = true; }
  void setNeeded() { m_flags.m_needed = true; }
  void setSuperGlobal() { m_flags.m_superGlobal = true; }
  void setOverride() { m_flags.m_override = true; }
  void setIndirectAltered() { m_flags.m_indirectAltered = true; }
  void setReferenced() { m_flags.m_referenced = true; }
  void setHidden() { m_flags.m_hidden = true; }
  void setClosureVar() { m_flags.m_closureVar = true; }
  void setRefClosureVar() { m_flags.m_refClosureVar = true; }
  void setClassName() { m_flags.m_className = true; }
  void setShrinkWrapped() { m_flags.m_shrinkWrapped = true; }

  void clearGlobal() { m_flags.m_global = false; }
  void clearUsed() { m_flags.m_used = false; }
  void clearNeeded() { m_flags.m_needed = false; }
  void clearReferenced() { m_flags.m_referenced = false; }

  void update(Symbol *src) {
    m_flags_val = src->m_flags_val;
  }

  ConstructPtr getStaticInitVal() const {
    return m_flags.m_hasStaticInit ? m_initVal : ConstructPtr();
  }
  ConstructPtr getClassInitVal() const {
    return m_flags.m_hasClassInit ? m_initVal : ConstructPtr();
  }
  void setStaticInitVal(ConstructPtr initVal) {
    if (m_flags.m_hasClassInit) initVal.reset();
    m_flags.m_hasStaticInit = true;
    m_initVal = initVal;
  }
  void setClassInitVal(ConstructPtr initVal) {
    if (m_flags.m_hasStaticInit) initVal.reset();
    m_flags.m_hasClassInit = true;
    m_initVal = initVal;
  }
private:
  std::string m_name;
  union {
    unsigned m_flags_val;
    struct {
      /* internal */
      unsigned m_declaration_set : 1;
      unsigned m_value_set : 1;
      unsigned m_hasStaticInit : 1;
      unsigned m_hasClassInit : 1;
      unsigned m_constant : 1;

      /* common */
      unsigned m_system : 1;
      unsigned m_sep : 1;

      /* ConstantTable */
      unsigned m_dynamic : 1;
      unsigned m_replaced : 1;

      /* VariableTable */
      unsigned m_protected : 1;
      unsigned m_private : 1;
      unsigned m_static : 1;
      unsigned m_global : 1;
      unsigned m_redeclared : 1;
      unsigned m_localGlobal : 1;
      unsigned m_nestedStatic : 1;
      unsigned m_lvalParam : 1;
      unsigned m_callTimeRef : 1;
      unsigned m_used : 1;
      unsigned m_needed : 1;
      unsigned m_superGlobal : 1;
      unsigned m_override : 1;
      unsigned m_indirectAltered : 1;
      unsigned m_referenced : 1;
      unsigned m_hidden : 1;
      unsigned m_closureVar : 1;
      unsigned m_refClosureVar : 1;
      unsigned m_className : 1;
      unsigned m_shrinkWrapped : 1;
    } m_flags;
  };
  ConstructPtr        m_declaration;
  ConstructPtr        m_value;
  TypePtr             m_coerced;
  TypePtr             m_prevCoerced;

  int                 m_parameter;
  ConstructPtr        m_initVal;

  static TypePtr CoerceTo(AnalysisResultConstPtr ar,
                          TypePtr &curType, TypePtr type);
};

/**
 * Base class of VariableTable and ConstantTable.
 */
class SymbolTable : public boost::enable_shared_from_this<SymbolTable>,
                    public JSON::ISerializable {
public:
  static Mutex AllSymbolTablesMutex;
  static SymbolTablePtrList AllSymbolTables; // for stats purpose
  static void CountTypes(std::map<std::string, int> &counts);
  BlockScope *getScopePtr() const { return &m_blockScope; }
  BlockScopeRawPtr getBlockScope() const {
    return BlockScopeRawPtr(&m_blockScope);
  }
public:
  SymbolTable(BlockScope &blockScope, bool isConst);
  SymbolTable();
  virtual ~SymbolTable();

  void beginLocal();
  void endLocal();

  /**
   * Import system symbols into this.
   */
  void import(SymbolTablePtr src);

  /**
   * Whether it's declared.
   */
  bool isPresent(const std::string &name) const;

  /**
   * Whether a symbol is defined.
   */
  bool checkDefined(const std::string &name);

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
  TypePtr getType(const std::string &name);
  TypePtr getFinalType(const std::string &name) const;

  /**
   * Find declaration construct.
   */
  bool isExplicitlyDeclared(const std::string &name) const;
  ConstructPtr getDeclaration(const std::string &name) const;
  ConstructPtr getValue(const std::string &name) const;

  /* Whether this constant is brought in by a separable extension */
  void setSepExtension(const std::string &name);
  bool isSepExtension(const std::string &name) const;

  /**
   * How big of a hash table for generate C++ switch statements.
   */
  int getJumpTableSize() const {
    return Util::roundUpToPowerOfTwo(m_symbolVec.size() * 2);
  }

  void canonicalizeSymbolOrder();
  void getSymbols(std::vector<std::string> &syms) const;
  void getCoerced(StringToTypePtrMap &coerced) const;

  virtual TypePtr setType(AnalysisResultConstPtr ar, const std::string &name,
                          TypePtr type, bool coerced);
  virtual TypePtr setType(AnalysisResultConstPtr ar, Symbol *sym,
                          TypePtr type, bool coerced);
  Symbol *getSymbol(const std::string &name);
  const Symbol *getSymbol(const std::string &name) const {
    return const_cast<SymbolTable*>(this)->getSymbol(name);
  }

  FunctionScopeRawPtr getFunctionScope();
  ClassScopeRawPtr getClassScope();
  FileScopeRawPtr getFileScope();
  static std::string getEscapedText(Variant v, int &len);
protected:
  Symbol *genSymbol(const std::string &name, bool konst);
  typedef std::map<std::string,Symbol> StringToSymbolMap;
  BlockScope &m_blockScope;     // owner

  std::vector<Symbol*>  m_symbolVec; // in declaration order
  StringToSymbolMap     m_symbolMap;

  void countTypes(std::map<std::string, int> &counts);
private:
  bool m_const;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __SYMBOL_TABLE_H__
