/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#ifndef __CLASS_SCOPE_H__
#define __CLASS_SCOPE_H__

#include <compiler/analysis/block_scope.h>
#include <compiler/analysis/function_container.h>
#include <compiler/statement/class_statement.h>
#include <util/json.h>
#include <util/case_insensitive.h>
#include <compiler/option.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(StatementList);
DECLARE_BOOST_TYPES(FunctionScope);
DECLARE_BOOST_TYPES(ClassScope);
DECLARE_BOOST_TYPES(FileScope);

class Symbol;

/**
 * A class scope corresponds to a class declaration. We store all
 * inferred types and analyzed results here, so not to pollute syntax trees.
 */
class ClassScope : public BlockScope, public FunctionContainer,
                   public JSON::ISerializable {

public:
  enum KindOf {
    KindOfObjectClass,
    KindOfAbstractClass,
    KindOfFinalClass,
    KindOfInterface,
  };

#define DECLARE_MAGIC(prefix, prev)                                     \
  prefix ## UnknownPropGetter          = prev << 1, /* __get */         \
  prefix ## UnknownPropSetter          = prev << 2, /* __set */         \
  prefix ## UnknownPropTester          = prev << 3, /* __isset */       \
  prefix ## PropUnsetter               = prev << 4, /* __unset */       \
  prefix ## UnknownMethodHandler       = prev << 5, /* __call */        \
  prefix ## UnknownStaticMethodHandler = prev << 6, /* __callStatic */  \
  prefix ## InvokeMethod               = prev << 7, /* __invoke */      \
  prefix ## ArrayAccess                = prev << 8  /* Implements ArrayAccess */

  enum Attribute {
    System                        = 0x001,
    Extension                     = 0x002,
    /**
     * set iff there is a __construct method. check ClassNameConstructor if
     * you want to know whether there is a class-name constructor.
     */
    HasConstructor                = 0x0004,
    ClassNameConstructor          = 0x0008,
    HasDestructor                 = 0x0010,
    NotFinal                      = 0x0020,
    DECLARE_MAGIC(Has, NotFinal),
    DECLARE_MAGIC(MayHave, HasArrayAccess),
    DECLARE_MAGIC(Inherits, MayHaveArrayAccess)
  };
  enum Modifier {
    Public = 1,
    Protected = 2,
    Private = 4,
    Static = 8,
    Abstract = 16,
    Final = 32
  };
  enum Derivation {
    FromNormal = 0,
    DirectFromRedeclared,
    IndirectFromRedeclared
  };

  enum JumpTableName {
    JumpTableInvoke,
    JumpTableStaticInvoke
  };

public:
  ClassScope(KindOf kindOf, const std::string &name,
             const std::string &parent,
             const std::vector<std::string> &bases,
             const std::string &docComment, StatementPtr stmt);


  /**
   * Special constructor for extension classes.
   */
  ClassScope(AnalysisResultPtr ar,
             const std::string &name, const std::string &parent,
             const std::vector<std::string> &bases,
             const FunctionScopePtrVec &methods);

  bool classNameCtor() const {
    return getAttribute(ClassNameConstructor);
  }
  const std::string &getOriginalName() const;


  virtual std::string getId(CodeGenerator &cg) const;

  void checkDerivation(AnalysisResultPtr ar, hphp_string_set &seen);
  const std::string &getParent() const { return m_parent;}
  std::string getHeaderFilename(CodeGenerator &old_cg);
  std::string getForwardHeaderFilename(CodeGenerator &old_cg);

  /**
   * Returns topmost parent class that has the method.
   */
  ClassScopePtr getRootParent(AnalysisResultConstPtr ar,
                              const std::string &methodName = "");
  void getRootParents(AnalysisResultConstPtr ar, const std::string &methodName,
                      ClassScopePtrVec &roots, ClassScopePtr curClass);

  /**
   * Whether this is a user-defined class.
   */
  bool isUserClass() const { return !getAttribute(System);}
  bool isExtensionClass() const { return getAttribute(Extension); }
  bool isDynamic() const { return m_dynamic; }
  bool isBaseClass() const { return m_bases.empty(); }

  /**
   * Whether this class name was declared twice or more.
   */
  void setRedeclaring(AnalysisResultConstPtr ar, int redecId);
  bool isRedeclaring() const { return m_redeclaring >= 0;}
  int getRedeclaringId() { return m_redeclaring; }

  void setStaticDynamic(AnalysisResultConstPtr ar);
  void setDynamic(AnalysisResultConstPtr ar, const std::string &name);

  void addReferer(BlockScopePtr ref, int useKinds);

  /* For class_exists */
  void setVolatile();
  bool isVolatile() { return m_volatile;}

  /* For code generation of os_static_initializer */
  void setNeedStaticInitializer() { m_needStaticInitializer = true;}
  bool needStaticInitializer() { return m_needStaticInitializer;}

  bool needLazyStaticInitializer();

  Derivation derivesFromRedeclaring() const {
    return m_derivesFromRedeclaring;
  }

  bool derivedByDynamic() const {
    return m_derivedByDynamic;
  }

  /* Whether this class is brought in by a separable extension */
  void setSepExtension() { m_sep = true;}
  bool isSepExtension() const { return m_sep;}

  /**
   * Get/set attributes.
   */
  void setSystem();
  void setAttribute(Attribute attr) { m_attribute |= attr;}
  void clearAttribute(Attribute attr) { m_attribute &= ~attr;}
  bool getAttribute(Attribute attr) const {
    return m_attribute & attr;
  }

  void addMissingMethod(const std::string &name) {
    m_missingMethods.insert(name);
  }

  /**
   * Called by ClassScope to prepare name => method/property map.
   */
  void collectMethods(AnalysisResultPtr ar,
                      StringToFunctionScopePtrMap &func,
                      bool collectPrivate = true,
                      bool forInvoke = false);

  /**
   * Whether or not we can directly call c_ObjectData::o_invoke() when lookup
   * in this class fails. If false, we need to call parent::o_invoke(), which
   * may be redeclared or may have private methods that need to check class
   * context.
   */
  bool needsInvokeParent(AnalysisResultConstPtr ar, bool considerSelf = true);

  /*
    void collectProperties(AnalysisResultPtr ar,
    std::set<std::string> &names,
    bool collectPrivate = true) const;

  */
  /**
   * Testing whether this class derives from another.
   */
  bool derivesDirectlyFrom(const std::string &base) const;
  bool derivesFrom(AnalysisResultConstPtr ar, const std::string &base,
                   bool strict, bool def) const;

 /**
  * Find a common parent of two classes; returns "" if there is no such.
  */
  static std::string findCommonParent(AnalysisResultConstPtr ar,
                                      const std::string cn1,
                                      const std::string cn2);

  /**
   * Look up function by name.
   */
  FunctionScopePtr findFunction(AnalysisResultConstPtr ar,
                                const std::string &name,
                                bool recursive,
                                bool exclIntfBase = false);

  /**
   * Look up constructor, both __construct and class-name constructor.
   */
  FunctionScopePtr findConstructor(AnalysisResultConstPtr ar,
                                   bool recursive);

  Symbol *findProperty(ClassScopePtr &cls, const std::string &name,
                       AnalysisResultConstPtr ar, ConstructPtr construct);
  TypePtr checkProperty(Symbol *sym, TypePtr type,
                        bool coerce, AnalysisResultConstPtr ar);
  TypePtr checkConst(const std::string &name, TypePtr type,
                     bool coerce, AnalysisResultConstPtr ar,
                     ConstructPtr construct,
                     const std::vector<std::string> &bases,
                     BlockScope *&defScope);

  /**
   * Collect parent class names.
   */
  void getAllParents(AnalysisResultConstPtr ar,
                     std::vector<std::string> &names) {
    if (m_stmt) {
      if (isInterface()) {
        boost::dynamic_pointer_cast<InterfaceStatement>
          (m_stmt)->getAllParents(ar, names);
      } else {
        boost::dynamic_pointer_cast<ClassStatement>
          (m_stmt)->getAllParents(ar, names);
      }
    }
  }

  std::vector<std::string> &getBases() { return m_bases;}

  ClassScopePtr getParentScope(AnalysisResultConstPtr ar);

  void addUsedLiteralStringHeader(const std::string &s) {
    m_usedLiteralStringsHeader.insert(s);
  }

  void addUsedLitVarStringHeader(const std::string &s) {
    m_usedLitVarStringsHeader.insert(s);
  }

  void addUsedScalarVarInteger(int64 i) {
    m_usedScalarVarIntegers.insert(i);
  }

  std::set<int64> &getUsedScalarVarIntegers() {
    return m_usedScalarVarIntegers;
  }

  void addUsedScalarVarIntegerHeader(int64 i) {
    m_usedScalarVarIntegersHeader.insert(i);
  }

  void addUsedScalarVarDouble(double d) {
    m_usedScalarVarDoubles.insert(d);
  }

  std::set<double> &getUsedScalarVarDoubles() {
    return m_usedScalarVarDoubles;
  }

  void addUsedScalarVarDoubleHeader(double d) {
    m_usedScalarVarDoublesHeader.insert(d);
  }

  void addUsedDefaultValueScalarArray(const std::string &s) {
    m_usedDefaultValueScalarArrays.insert(s);
  }

  void addUsedDefaultValueScalarVarArray(const std::string &s) {
    m_usedDefaultValueScalarVarArrays.insert(s);
  }

  void addUsedConstHeader(const std::string &s) {
    m_usedConstsHeader.insert(s);
  }

  void addUsedClassConstHeader(const std::string &cls, const std::string &s) {
    m_usedClassConstsHeader.insert(UsedClassConst(cls, s));
  }

  void addUsedClassHeader(const std::string &s) {
    m_usedClassesHeader.insert(s);
  }

  void addUsedClassFullHeader(const std::string &s) {
    m_usedClassesFullHeader.insert(s);
  }

  /**
   * Output class meta info for g_class_map.
   */
  void outputCPPClassMap(CodeGenerator &cg, AnalysisResultPtr ar);

  /**
   * Serialize the iface, not everything.
   */
  void serialize(JSON::OutputStream &out) const;

  static void outputCPPClassVarInitImpl(
    CodeGenerator &cg, const StringToClassScopePtrVecMap &classScopes,
    const std::vector<const char *> &classes);

  void outputCPPDynamicClassDecl(CodeGenerator &cg);
  void outputCPPDynamicClassImpl(CodeGenerator &cg, AnalysisResultPtr ar);
  static void outputCPPDynamicClassCreateDecl(CodeGenerator &cg);
  static void outputCPPDynamicClassCreateImpl
  (CodeGenerator &cg, const StringToClassScopePtrVecMap &classScopes,
   const std::vector<const char*> &classes);
  static void outputCPPGetCallInfoStaticMethodImpl
  (CodeGenerator &cg, const StringToClassScopePtrVecMap &classScopes,
   const std::vector<const char*> &classes);
  static void outputCPPGetStaticPropertyImpl
  (CodeGenerator &cg, const StringToClassScopePtrVecMap &classScopes,
   const std::vector<const char*> &classes);
  static void outputCPPGetClassConstantImpl
  (CodeGenerator &cg, const StringToClassScopePtrVecMap &classScopes,
   const std::vector<const char*> &classes);
  void outputCPPStaticInitializerDecl(CodeGenerator &cg);
  bool isInterface() { return m_kindOf == KindOfInterface; }
  bool isFinal() { return m_kindOf == KindOfFinalClass; }
  bool isAbstract() { return m_kindOf == KindOfAbstractClass; }
  bool hasProperty(const std::string &name);
  bool hasConst(const std::string &name);
  void outputCPPHeader(CodeGenerator &cg, AnalysisResultPtr ar,
                       CodeGenerator::Output output);
  void outputCPPForwardHeader(CodeGenerator &old_cg, AnalysisResultPtr ar,
                              CodeGenerator::Output output);
  void outputCPPJumpTableDecl(CodeGenerator &cg, AnalysisResultPtr ar);

  void outputMethodWrappers(CodeGenerator &cg, AnalysisResultPtr ar);

  /**
   * This prints out all the support methods (invoke, create, destructor,
   * etc.)
   * This is here instead of ClassStatement because I want to be able to call
   * these for extension classes that don't have a statement.
   */
  void outputCPPSupportMethodsImpl(CodeGenerator &cg, AnalysisResultPtr ar);

  /**
   * These output wrappers for class methods so that class definitions don't
   * have to be used in generating global jump tables.
   */
  void outputCPPGlobalTableWrappersDecl(CodeGenerator &cg,
                                        AnalysisResultPtr ar);
  void outputCPPGlobalTableWrappersImpl(CodeGenerator &cg,
                                        AnalysisResultPtr ar);

  static bool NeedStaticArray(ClassScopePtr cls, FunctionScopePtr func);
  void inheritedMagicMethods(ClassScopePtr super);
  void derivedMagicMethods(ClassScopePtr super);
  /* true if it might, false if it doesnt */
  bool implementsArrayAccess();
  /* true if it might, false if it doesnt */
  bool implementsAccessor(int prop);

  void outputForwardDeclaration(CodeGenerator &cg);

  void clearBases() {
    m_bases.clear();
    m_parent = "";
  }

  void outputCPPStaticMethodWrappers(CodeGenerator &cg, AnalysisResultPtr ar,
                                     std::set<std::string> &done,
                                     const char *cls);
  /**
   * Override function container
   */
  virtual bool addFunction(AnalysisResultConstPtr ar,
                           FunctionScopePtr funcScope);

  enum TableType {
    Invoke,
    CallInfo
  };
  void outputCPPJumpTable(CodeGenerator &cg, AnalysisResultPtr ar,
                          bool staticOnly, bool dynamicObject = false,
                          TableType type = Invoke);

  void outputVolatileCheckBegin(CodeGenerator &cg,
                                AnalysisResultPtr ar,
                                BlockScopePtr bs,
                                const std::string &name);
  void outputVolatileCheckEnd(CodeGenerator &cg);
  static void OutputVolatileCheckBegin(CodeGenerator &cg,
                                       AnalysisResultPtr ar,
                                       BlockScopePtr bs,
                                       const std::string &origName);
  static void OutputVolatileCheckEnd(CodeGenerator &cg);
  static void OutputVolatileCheck(CodeGenerator &cg, AnalysisResultPtr ar,
                                  BlockScopePtr bs,
                                  const std::string &origName, bool noThrow);


  /**
   * Whether or not the specified jump table is empty.
   */
  bool hasAllJumpTables() const {
    return m_emptyJumpTables.empty();
  }
  bool hasJumpTable(JumpTableName name) const {
    return m_emptyJumpTables.find(name) == m_emptyJumpTables.end();
  }

protected:
  void findJumpTableMethods(CodeGenerator &cg, AnalysisResultPtr ar,
                            bool staticOnly, std::vector<const char *> &funcs);
private:
  // need to maintain declaration order for ClassInfo map
  FunctionScopePtrVec m_functionsVec;

  KindOf m_kindOf;
  std::string m_parent;
  mutable std::vector<std::string> m_bases;
  mutable int m_attribute;
  bool m_dynamic;
  int m_redeclaring; // multiple definition of the same class
  bool m_volatile; // for class_exists
  bool m_needStaticInitializer; // for os_static_initializer
  Derivation m_derivesFromRedeclaring;
  bool m_derivedByDynamic;
  std::set<std::string> m_missingMethods;
  bool m_sep;

  std::set<JumpTableName> m_emptyJumpTables;
  std::set<std::string> m_usedLiteralStringsHeader;
  std::set<std::string> m_usedLitVarStringsHeader;
  std::set<int64> m_usedScalarVarIntegers;
  std::set<int64> m_usedScalarVarIntegersHeader;
  std::set<double> m_usedScalarVarDoubles;
  std::set<double> m_usedScalarVarDoublesHeader;
  std::set<std::string> m_usedDefaultValueScalarArrays;
  std::set<std::string> m_usedDefaultValueScalarVarArrays;
  std::set<std::string> m_usedConstsHeader;
  typedef std::pair<std::string, std::string> UsedClassConst;
  std::set<UsedClassConst> m_usedClassConstsHeader;
  std::set<std::string> m_usedClassesHeader;
  std::set<std::string> m_usedClassesFullHeader;

  std::string getBaseHeaderFilename(CodeGenerator &cg);

  void outputCPPMethodInvokeBareObjectSupport(
    CodeGenerator &cg, AnalysisResultPtr ar, 
    FunctionScopePtr func, bool fewArgs);

  static void outputCPPClassJumpTable
  (CodeGenerator &cg, const StringToClassScopePtrVecMap &classScopes,
   const std::vector<const char*> &classes, const char* macro,
   bool useString = false);

  static void outputCPPHashTableClassVarInit
    (CodeGenerator &cg, const StringToClassScopePtrVecMap &classScopes,
     const std::vector<const char*> &classes);

  void outputCPPMethodInvokeTable
    (CodeGenerator &cg, AnalysisResultPtr ar,
     const std::vector <const char*> &keys,
     const StringToFunctionScopePtrVecMap &funcScopes, bool fewArgs,
     bool staticOnly, TableType type);
  void outputCPPMethodInvokeTableSupport(CodeGenerator &cg,
      AnalysisResultPtr ar, const std::vector<const char*> &keys,
      const StringToFunctionScopePtrVecMap &funcScopes, bool fewArgs);
  hphp_const_char_imap<int> m_implemented;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // __CLASS_SCOPE_H__
