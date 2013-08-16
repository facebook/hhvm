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

#ifndef incl_HPHP_SYMBOL_TABLE_H_
#define incl_HPHP_SYMBOL_TABLE_H_

#include "hphp/compiler/hphp.h"
#include "hphp/util/json.h"
#include "hphp/util/util.h"
#include "hphp/util/lock.h"

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
typedef std::set<std::string> SymbolSet;
typedef std::map<std::string, std::set<std::string> > Type2SymbolSetMap;

class Symbol;
typedef hphp_string_map<Symbol*> StringToSymbolMap;
typedef std::vector<Symbol*>     SymbolVec;

class Symbol {
public:
  Symbol() : m_hash(0), m_parameter(-1) { m_flags_val = 0; }

  void import(BlockScopeRawPtr scope, const Symbol &src_sym);

  void beginLocal(BlockScopeRawPtr scope);
  void endLocal  (BlockScopeRawPtr scope);
  void resetLocal(BlockScopeRawPtr scope);

  void setName(const std::string &name) {
    m_name = name;
    m_hash = (unsigned int) hash_string_inline(m_name.c_str(), m_name.size());
  }
  const std::string &getName() const { return m_name; }
  unsigned int getHash() const { return m_hash; }

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
  void setSystem() { m_flags.m_system = true; }
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
  bool isReferenced() const { return !m_flags.m_notReferenced; }
  bool isHidden() const { return m_flags.m_hidden; }
  bool isClosureVar() const { return m_flags.m_closureVar; }
  bool isRefClosureVar() const { return m_flags.m_refClosureVar; }
  bool isPassClosureVar() const { return m_flags.m_passClosureVar; }
  bool isClassName() const { return m_flags.m_className; }
  bool isShrinkWrapped() const { return m_flags.m_shrinkWrapped; }
  bool isStashedVal() const { return m_flags.m_stashedVal; }
  bool isReseated() const { return m_flags. m_reseated; }

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
  void setReferenced() { m_flags.m_notReferenced = false; }
  void setHidden() { m_flags.m_hidden = true; }
  void setClosureVar() { m_flags.m_closureVar = true; }
  void setRefClosureVar() { m_flags.m_refClosureVar = true; }
  void setPassClosureVar() { m_flags.m_passClosureVar = true; }
  void setClassName() { m_flags.m_className = true; }
  void setShrinkWrapped() { m_flags.m_shrinkWrapped = true; }
  void setStashedVal() { m_flags.m_stashedVal = true; }
  void setReseated() { m_flags.m_reseated = true; }

  void clearGlobal() { m_flags.m_global = false; }
  void clearUsed() { m_flags.m_used = false; }
  void clearNeeded() { m_flags.m_needed = false; }
  void clearReferenced() { m_flags.m_notReferenced = true; }
  void clearReseated() { m_flags.m_reseated = false; }
  void clearRefClosureVar() { m_flags.m_refClosureVar = false; }

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

  // we avoid implementing ISerialize for Symbols, since we don't
  // want Symbols to need a vtable. Instead, we use the
  // wrappers below
  void serializeParam(JSON::DocTarget::OutputStream &out) const;
  void serializeClassVar(JSON::DocTarget::OutputStream &out) const;
private:
  std::string  m_name;
  unsigned int m_hash;
  union {
    uint32_t m_flags_val;
    struct {
      /* internal */
      unsigned m_declaration_set : 1;
      unsigned m_value_set : 1;
      unsigned m_hasStaticInit : 1;
      unsigned m_hasClassInit : 1;
      unsigned m_constant : 1;

      /* common */
      unsigned m_system : 1;

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
      unsigned m_notReferenced : 1;
      unsigned m_hidden : 1;
      unsigned m_closureVar : 1;
      unsigned m_refClosureVar : 1;
      unsigned m_passClosureVar : 1;
      unsigned m_className : 1;
      unsigned m_shrinkWrapped : 1;
      unsigned m_stashedVal : 1;
      unsigned m_reseated : 1;
    } m_flags;


  };
  static_assert(
    sizeof(m_flags_val) == sizeof(m_flags),
    "m_flags_val must cover all the flags");

  ConstructPtr        m_declaration;
  ConstructPtr        m_value;
  TypePtr             m_coerced;
  TypePtr             m_prevCoerced;

  int                 m_parameter;
  ConstructPtr        m_initVal;

  void triggerUpdates(BlockScopeRawPtr scope) const;

  static TypePtr CoerceTo(AnalysisResultConstPtr ar,
                          TypePtr &curType, TypePtr type);
};

class SymParamWrapper : public JSON::DocTarget::ISerializable {
public:
  explicit SymParamWrapper(const Symbol* sym) : m_sym(sym) {
    assert(sym);
  }
  virtual void serialize(JSON::DocTarget::OutputStream &out) const {
    m_sym->serializeParam(out);
  }
private:
  const Symbol *m_sym;
};

class SymClassVarWrapper : public JSON::DocTarget::ISerializable {
public:
  explicit SymClassVarWrapper(const Symbol* sym) : m_sym(sym) {
    assert(sym);
  }
  virtual void serialize(JSON::DocTarget::OutputStream &out) const {
    m_sym->serializeClassVar(out);
  }
private:
  const Symbol *m_sym;
};

/**
 * Base class of VariableTable and ConstantTable.
 */
class SymbolTable : public std::enable_shared_from_this<SymbolTable>,
                    public JSON::CodeError::ISerializable {
public:
  static Mutex AllSymbolTablesMutex;
  static SymbolTablePtrList AllSymbolTables; // for stats purpose
  static void Purge();
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
  void resetLocal();

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
   * Implements JSON::CodeError::ISerializable.
   */
  virtual void serialize(JSON::CodeError::OutputStream &out) const;

  /**
   * Find a symbol's inferred type.
   */
  TypePtr getType(const std::string &name) const;
  TypePtr getFinalType(const std::string &name) const;

  /**
   * Find declaration construct.
   */
  bool isExplicitlyDeclared(const std::string &name) const;
  ConstructPtr getDeclaration(const std::string &name) const;
  ConstructPtr getValue(const std::string &name) const;

  /**
   * How big of a hash table for generate C++ switch statements.
   */
  int getJumpTableSize() const {
    return Util::roundUpToPowerOfTwo(m_symbolVec.size() * 2);
  }

  void canonicalizeSymbolOrder();
  void getSymbols(std::vector<Symbol*> &syms, bool filterHidden = false) const;
  void getSymbols(std::vector<std::string> &syms) const;
  const std::vector<Symbol*> &getSymbols() const { return m_symbolVec; }
  void getCoerced(StringToTypePtrMap &coerced) const;

  virtual TypePtr setType(AnalysisResultConstPtr ar, const std::string &name,
                          TypePtr type, bool coerced);
  virtual TypePtr setType(AnalysisResultConstPtr ar, Symbol *sym,
                          TypePtr type, bool coerced);
  Symbol *getSymbol(const std::string &name);
  const Symbol *getSymbol(const std::string &name) const;

  FunctionScopeRawPtr getFunctionScope();
  ClassScopeRawPtr getClassScope();
  FileScopeRawPtr getFileScope();
  static std::string getEscapedText(Variant v, int &len);
protected:
  Symbol *genSymbol(const std::string &name, bool konst);
  Symbol *genSymbol(const std::string &name, bool konst,
                    ConstructPtr construct);
  typedef std::map<std::string,Symbol> StringToSymbolMap;
  BlockScope &m_blockScope;     // owner

  std::vector<Symbol*>  m_symbolVec; // in declaration order
  StringToSymbolMap     m_symbolMap;

  void countTypes(std::map<std::string, int> &counts);
private:
  const Symbol* getSymbolImpl(const std::string &name) const;
  bool m_const;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_SYMBOL_TABLE_H_
