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

#ifndef __CLASS_SCOPE_H__
#define __CLASS_SCOPE_H__

#include <compiler/analysis/block_scope.h>
#include <compiler/analysis/function_container.h>
#include <compiler/statement/class_statement.h>
#include <util/json.h>
#include <compiler/option.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(StatementList);
DECLARE_BOOST_TYPES(FunctionScope);
DECLARE_BOOST_TYPES(ClassScope);
DECLARE_BOOST_TYPES(FileScope);

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

  enum Attribute {
    HasConstructor = 1, // set iff there is a __construct method.
                        // check classNameConstructor if you want to know
                        // whether there is a class-name constructor.
    HasDestructor  = 2,
    HasUnknownMethodHandler = 4,
    System = 8,
    Extension = 16,
    classNameConstructor = 32,
    HasUnknownPropHandler = 64
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

public:
  ClassScope(KindOf kindOf, const std::string &name,
             const std::string &parent,
             const std::vector<std::string> &bases,
             const std::string &docComment, StatementPtr stmt,
             FileScopePtr file);


  /**
   * Special constructor for extension classes.
   */
  ClassScope(AnalysisResultPtr ar,
             const std::string &name, const std::string &parent,
             const std::vector<std::string> &bases,
             const std::vector<FunctionScopePtr> &methods);

  bool classNameCtor() const {
    return getAttribute(classNameConstructor);
  }
  const char *getOriginalName() const;


  std::string getId() const {
    if (m_redeclaring < 0) {
      return getName();
    }
    return getName() + Option::IdPrefix +
      boost::lexical_cast<std::string>(m_redeclaring);
  }

  const std::string &getParent() const { return m_parent;}
  std::string getHeaderFilename();

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
  void setRedeclaring(AnalysisResultPtr ar, int redecId);
  bool isRedeclaring() const { return m_redeclaring >= 0;}
  int getRedeclaringId() { return m_redeclaring; }

  void setStaticDynamic(AnalysisResultPtr ar);
  void setDynamic(AnalysisResultPtr ar, const std::string &name);

  /* For class_exists */
  void setVolatile() { m_volatile = true;}
  bool isVolatile() { return m_volatile;}

  /* For code generation of os_static_initializer */
  void setNeedStaticInitializer() { m_needStaticInitializer = true;}
  bool needStaticInitializer() { return m_needStaticInitializer;}

  bool needLazyStaticInitializer();

  Derivation derivesFromRedeclaring() const {
    return m_derivesFromRedeclaring;
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
  bool hasAttribute(Attribute attr,
                    AnalysisResultPtr ar) const; // recursive

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
  /*
    void collectProperties(AnalysisResultPtr ar,
    std::set<std::string> &names,
    bool collectPrivate = true) const;

  */
  /**
   * Testing whether this class derives from another.
   */
  bool derivesFrom(AnalysisResultPtr ar, const std::string &base) const;

  /**
   * Look up function by name.
   */
  FunctionScopePtr findFunction(AnalysisResultPtr ar,
                                const std::string &name,
                                bool recursive,
                                bool exclIntfBase = false);

  /**
   * Look up constructor, both __construct and class-name constructor.
   */
  FunctionScopePtr findConstructor(AnalysisResultPtr ar,
                                   bool recursive);

  TypePtr checkProperty(const std::string &name, TypePtr type,
                        bool coerce, AnalysisResultPtr ar,
                        ConstructPtr construct, int &properties);
  TypePtr checkStatic(const std::string &name, TypePtr type,
                      bool coerce, AnalysisResultPtr ar,
                      ConstructPtr construct, int &properties);
  TypePtr checkConst(const std::string &name, TypePtr type,
                     bool coerce, AnalysisResultPtr ar,
                     ConstructPtr construct,
                     const std::vector<std::string> &bases,
                     BlockScope *&defScope);

  /**
   * Collect parent class names.
   */
  void getAllParents(AnalysisResultPtr ar,
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

  FileScopePtr getFileScope() {
    FileScopePtr fs = m_file.lock();
    return fs;
  }

  ClassScopePtr getParentScope(AnalysisResultPtr ar);

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
  static void outputCPPInvokeStaticMethodImpl
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
  void outputCPPHeader(AnalysisResultPtr ar,
                       CodeGenerator::Output output);

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
  virtual void addFunction(AnalysisResultPtr ar, FunctionScopePtr funcScope);
  void outputCPPJumpTable(CodeGenerator &cg, AnalysisResultPtr ar,
                          bool staticOnly, bool dynamicObject = false,
                          bool forEval = false);

  void outputVolatileCheckBegin(CodeGenerator &cg,
                                AnalysisResultPtr ar,
                                const std::string &name);
  void outputVolatileCheckEnd(CodeGenerator &cg);
  static void OutputVolatileCheckBegin(CodeGenerator &cg,
                                       AnalysisResultPtr ar,
                                       const std::string &origName);
  static void OutputVolatileCheckEnd(CodeGenerator &cg);
protected:
  void findJumpTableMethods(CodeGenerator &cg, AnalysisResultPtr ar,
                            bool staticOnly, std::vector<const char *> &funcs);
private:
  FileScopeWeakPtr m_file;
  KindOf m_kindOf;
  std::string m_parent;
  mutable std::vector<std::string> m_bases;
  mutable int m_attribute;
  bool m_dynamic;
  int m_redeclaring; // multiple definition of the same class
  bool m_volatile; // for class_exists
  bool m_needStaticInitializer; // for os_static_initializer
  Derivation m_derivesFromRedeclaring;
  std::set<std::string> m_missingMethods;
  bool m_sep;

  static void outputCPPClassJumpTable
  (CodeGenerator &cg, const StringToClassScopePtrVecMap &classScopes,
   const std::vector<const char*> &classes, const char* macro);
  void outputCPPMethodInvokeTable
    (CodeGenerator &cg, AnalysisResultPtr ar,
     const std::vector <const char*> &keys,
     const StringToFunctionScopePtrVecMap &funcScopes, bool fewArgs,
     bool staticOnly, bool forEval);
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // __CLASS_SCOPE_H__
