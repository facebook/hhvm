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

#ifndef incl_HPHP_CLASS_SCOPE_H_
#define incl_HPHP_CLASS_SCOPE_H_

#include "hphp/compiler/analysis/block_scope.h"
#include "hphp/compiler/analysis/function_container.h"
#include "hphp/compiler/statement/class_statement.h"
#include "hphp/compiler/statement/method_statement.h"
#include "hphp/compiler/statement/trait_prec_statement.h"
#include "hphp/compiler/statement/trait_alias_statement.h"
#include "hphp/compiler/expression/user_attribute.h"
#include "hphp/util/json.h"
#include "hphp/util/case-insensitive.h"
#include "hphp/compiler/option.h"

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
                   public JSON::CodeError::ISerializable,
                   public JSON::DocTarget::ISerializable {

public:
  enum KindOf {
    KindOfObjectClass,
    KindOfAbstractClass,
    KindOfFinalClass,
    KindOfInterface,
    KindOfTrait
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
    UsesUnknownTrait              = 0x0040,
    DECLARE_MAGIC(Has, UsesUnknownTrait),
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
    JumpTableCallInfo
  };

public:
  ClassScope(KindOf kindOf, const std::string &name,
             const std::string &parent,
             const std::vector<std::string> &bases,
             const std::string &docComment, StatementPtr stmt,
             const std::vector<UserAttributePtr> &attrs);

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
  std::string getDocName() const;

  virtual std::string getId() const;

  void checkDerivation(AnalysisResultPtr ar, hphp_string_iset &seen);
  const std::string &getOriginalParent() const { return m_parent; }

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
  bool isVolatile() const { return m_volatile;}
  bool isPersistent() const { return m_persistent; }
  void setPersistent(bool p) { m_persistent = p; }

  bool needLazyStaticInitializer();

  Derivation derivesFromRedeclaring() const {
    return m_derivesFromRedeclaring;
  }

  bool derivedByDynamic() const {
    return m_derivedByDynamic;
  }

  /**
   * Get/set attributes.
   */
  void setSystem();
  bool isSystem() const { return m_attribute & System; }
  void setAttribute(Attribute attr) { m_attribute |= attr;}
  void clearAttribute(Attribute attr) { m_attribute &= ~attr;}
  bool getAttribute(Attribute attr) const {
    return m_attribute & attr;
  }
  bool hasAttribute(Attribute attr, AnalysisResultConstPtr ar) const {
    if (getAttribute(attr)) return true;
    ClassScopePtr parent = getParentScope(ar);
    return parent && !parent->isRedeclaring() && parent->hasAttribute(attr, ar);
  }
  void setKnownBase(int i) { assert(i < 32); m_knownBases |= 1u << i; }
  bool hasUnknownBases() const {
    int n = m_bases.size();
    if (!n) return false;
    if (n >= 32) n = 0;
    return m_knownBases != (((1u << n) - 1) & 0xffffffff);
  }
  bool hasKnownBase(int i) const {
    return m_knownBases & (1u << (i < 32 ? i : 31));
  }

  const FunctionScopePtrVec &getFunctionsVec() const {
    return m_functionsVec;
  }

  /**
   * Called by ClassScope to prepare name => method/property map.
   */
  void collectMethods(AnalysisResultPtr ar,
                      StringToFunctionScopePtrMap &func,
                      bool collectPrivate = true,
                      bool forInvoke = false);

  /**
   * Whether or not we can directly call ObjectData::o_invoke() when lookup
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
  static ClassScopePtr FindCommonParent(AnalysisResultConstPtr ar,
                                        const std::string &cn1,
                                        const std::string &cn2);

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
                       AnalysisResultConstPtr ar);

  /**
   * Caller is assumed to hold a lock on this scope
   */
  TypePtr checkProperty(BlockScopeRawPtr context,
                        Symbol *sym, TypePtr type,
                        bool coerce, AnalysisResultConstPtr ar);

  /**
   * Caller is *NOT* assumed to hold any locks. Context is
   */
  TypePtr checkConst(BlockScopeRawPtr context,
                     const std::string &name, TypePtr type,
                     bool coerce, AnalysisResultConstPtr ar,
                     ConstructPtr construct,
                     const std::vector<std::string> &bases,
                     BlockScope *&defScope);

  /**
   * Collect parent class names.
   */
  void getAllParents(AnalysisResultConstPtr ar,
                     std::vector<std::string> &names);

  void getInterfaces(AnalysisResultConstPtr ar,
                     std::vector<std::string> &names,
                     bool recursive = true) const;

  std::vector<std::string> &getBases() { return m_bases;}

  typedef hphp_hash_map<std::string, ExpressionPtr, string_hashi,
    string_eqstri> UserAttributeMap;

  UserAttributeMap& userAttributes() { return m_userAttributes;}

  ClassScopePtr getParentScope(AnalysisResultConstPtr ar) const;

  void addUsedTrait(const std::string &s) {
    if (!usesTrait(s)) {
      m_usedTraitNames.push_back(s);
    }
  }

  void addUsedTraits(const std::vector<std::string> &names) {
    for (unsigned i = 0; i < names.size(); i++) {
      if (!usesTrait(names[i])) {
        m_usedTraitNames.push_back(names[i]);
      }
    }
  }

  const boost::container::flat_set<std::string>& getTraitRequiredExtends()
    const {
    return m_traitRequiredExtends;
  }

  const boost::container::flat_set<std::string>& getTraitRequiredImplements()
    const {
    return m_traitRequiredImplements;
  }

  const std::vector<std::string> &getUsedTraitNames() const {
    return m_usedTraitNames;
  }

  const std::vector<std::pair<std::string, std::string>>& getTraitAliases()
    const {
    return m_traitAliases;
  }

  void addTraitAlias(TraitAliasStatementPtr aliasStmt);

  void addTraitRequirement(const std::string &requiredName, bool isExtends);

  void importUsedTraits(AnalysisResultPtr ar);

  /**
   * Serialize the iface, not everything.
   */
  void serialize(JSON::CodeError::OutputStream &out) const;
  void serialize(JSON::DocTarget::OutputStream &out) const;

  bool isInterface() const { return m_kindOf == KindOfInterface; }
  bool isFinal() const { return m_kindOf == KindOfFinalClass ||
                                m_kindOf == KindOfTrait; }
  bool isAbstract() const { return m_kindOf == KindOfAbstractClass ||
                                   m_kindOf == KindOfTrait; }
  bool isTrait() const { return m_kindOf == KindOfTrait; }
  bool hasProperty(const std::string &name) const;
  bool hasConst(const std::string &name) const;

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

  /**
   * Override function container
   */
  bool addFunction(AnalysisResultConstPtr ar,
                   FunctionScopePtr funcScope);

  void setNeedsCppCtor(bool needsCppCtor) {
    m_needsCppCtor = needsCppCtor;
  }

  bool needsCppCtor() const {
    return m_needsCppCtor;
  }

  void setNeedsInitMethod(bool needsInit) {
    m_needsInit = needsInit;
  }

  bool needsInitMethod() const {
    return m_needsInit;
  }

  bool canSkipCreateMethod(AnalysisResultConstPtr ar) const;
  bool checkHasPropTable(AnalysisResultConstPtr ar);

private:
  // need to maintain declaration order for ClassInfo map
  FunctionScopePtrVec m_functionsVec;

  std::string m_parent;
  mutable std::vector<std::string> m_bases;
  UserAttributeMap m_userAttributes;

  std::vector<std::string> m_usedTraitNames;
  boost::container::flat_set<std::string> m_traitRequiredExtends;
  boost::container::flat_set<std::string> m_traitRequiredImplements;
  // m_traitAliases is used to support ReflectionClass::getTraitAliases
  std::vector<std::pair<std::string, std::string> > m_traitAliases;

  struct TraitMethod {
    const ClassScopePtr      m_trait;
    const MethodStatementPtr m_method;
    const std::string        m_originalName;
    ModifierExpressionPtr    m_modifiers;
    const StatementPtr       m_ruleStmt; // for methods imported via aliasing
    TraitMethod(ClassScopePtr trait, MethodStatementPtr method,
                ModifierExpressionPtr modifiers, StatementPtr ruleStmt) :
        m_trait(trait), m_method(method),
        m_originalName(method->getOriginalName()), m_modifiers(modifiers),
        m_ruleStmt(ruleStmt) {
    }
    TraitMethod(ClassScopePtr trait, MethodStatementPtr method,
                ModifierExpressionPtr modifiers, StatementPtr ruleStmt,
                const std::string &originalName) :
        m_trait(trait), m_method(method), m_originalName(originalName),
        m_modifiers(modifiers), m_ruleStmt(ruleStmt) {
    }
  };

  typedef std::list<TraitMethod> TraitMethodList;
  typedef std::map<std::string, TraitMethodList> MethodToTraitListMap;
  MethodToTraitListMap m_importMethToTraitMap;
  typedef std::map<std::string, MethodStatementPtr> ImportedMethodMap;

  mutable int m_attribute;
  int m_redeclaring; // multiple definition of the same class
  KindOf m_kindOf;
  Derivation m_derivesFromRedeclaring;
  enum TraitStatus {
    NOT_FLATTENED,
    BEING_FLATTENED,
    FLATTENED
  } m_traitStatus;
  unsigned m_dynamic:1;
  unsigned m_volatile:1; // for class_exists
  unsigned m_persistent:1;
  unsigned m_derivedByDynamic:1;
  unsigned m_needsCppCtor:1;
  unsigned m_needsInit:1;
  // m_knownBases has a bit for each base class saying whether
  // its known to exist at the point of definition of this class.
  // for classes with more than 31 bases, bit 31 is set iff
  // bases 32 through n are all known.
  unsigned m_knownBases;

  void addImportTraitMethod(const TraitMethod &traitMethod,
                            const std::string &methName);
  void informClosuresAboutScopeClone(ConstructPtr root,
                                     FunctionScopePtr outerScope,
                                     AnalysisResultPtr ar);

  void setImportTraitMethodModifiers(const std::string &methName,
                                     ClassScopePtr traitCls,
                                     ModifierExpressionPtr modifiers);

  MethodStatementPtr importTraitMethod(const TraitMethod&  traitMethod,
                                       AnalysisResultPtr   ar,
                                       std::string         methName,
                                       const ImportedMethodMap &
                                       importedTraitMethods);

  void importTraitProperties(AnalysisResultPtr ar);

  void findTraitMethodsToImport(AnalysisResultPtr ar, ClassScopePtr trait);

  MethodStatementPtr findTraitMethod(AnalysisResultPtr ar,
                                     ClassScopePtr trait,
                                     const std::string &methodName,
                                     std::set<ClassScopePtr> &visitedTraits);

  void applyTraitRules(AnalysisResultPtr ar);

  void applyTraitPrecRule(TraitPrecStatementPtr stmt);

  void applyTraitAliasRule(AnalysisResultPtr ar, TraitAliasStatementPtr stmt);

  ClassScopePtr findSingleTraitWithMethod(AnalysisResultPtr ar,
                                          const std::string &methodName) const;

  void removeSpareTraitAbstractMethods(AnalysisResultPtr ar);

  bool usesTrait(const std::string &traitName) const;

  bool hasMethod(const std::string &methodName) const;

};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_CLASS_SCOPE_H_
