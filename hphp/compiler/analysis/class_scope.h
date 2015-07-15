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

#ifndef incl_HPHP_CLASS_SCOPE_H_
#define incl_HPHP_CLASS_SCOPE_H_

#include "hphp/compiler/analysis/block_scope.h"
#include "hphp/compiler/analysis/exceptions.h"
#include "hphp/compiler/analysis/function_container.h"
#include "hphp/compiler/expression/user_attribute.h"
#include "hphp/compiler/json.h"
#include "hphp/compiler/option.h"
#include "hphp/compiler/statement/class_statement.h"
#include "hphp/compiler/statement/method_statement.h"
#include "hphp/compiler/statement/trait_alias_statement.h"
#include "hphp/compiler/statement/trait_prec_statement.h"

#include "hphp/util/functional.h"
#include "hphp/util/hash-map-typedefs.h"
#include "hphp/util/text-util.h"

#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <utility>
#include <vector>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(StatementList);
DECLARE_BOOST_TYPES(FunctionScope);
DECLARE_BOOST_TYPES(ClassScope);
DECLARE_BOOST_TYPES(FileScope);

class Symbol;

enum class Derivation {
  Normal,
  Redeclaring,    // At least one ancestor class or interface is redeclared.
};

/**
 * A class scope corresponds to a class declaration. We store all
 * inferred types and analyzed results here, so not to pollute syntax trees.
 */
class ClassScope : public BlockScope, public FunctionContainer,
                   public JSON::CodeError::ISerializable,
                   public JSON::DocTarget::ISerializable {

public:
  enum class KindOf : int {
    ObjectClass,
    AbstractClass,
    FinalClass,
    UtilClass,
    Enum,
    Interface,
    Trait,
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

public:
  ClassScope(FileScopeRawPtr fs,
             KindOf kindOf, const std::string &originalName,
             const std::string &parent,
             const std::vector<std::string> &bases,
             const std::string &docComment, StatementPtr stmt,
             const std::vector<UserAttributePtr> &attrs);

  /**
   * Special constructor for extension classes.
   */
  ClassScope(AnalysisResultPtr ar,
             const std::string &originalName, const std::string &parent,
             const std::vector<std::string> &bases,
             const FunctionScopePtrVec &methods);

  bool isNamed(const char* n) const;
  bool isNamed(const std::string& n) const {
    return isNamed(n.c_str());
  }
  bool classNameCtor() const {
    return getAttribute(ClassNameConstructor);
  }
  const std::string &getOriginalName() const;
  std::string getDocName() const;

  void checkDerivation(AnalysisResultPtr ar, hphp_string_iset &seen);
  const std::string &getOriginalParent() const { return m_parent; }

  /**
   * Whether this is a user-defined class.
   */
  bool isUserClass() const { return !getAttribute(System);}
  bool isExtensionClass() const { return getAttribute(Extension); }
  bool isDynamic() const { return m_dynamic; }
  bool isBaseClass() const { return m_bases.empty(); }
  bool isBuiltin() const { return !getStmt(); }

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
  bool isVolatile() const { return m_volatile; }
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
  const FunctionScopePtrVec &getFunctionsVec() const {
    return m_functionsVec;
  }

  /**
   * Called by ClassScope to prepare name => method/property map.
   */
  void collectMethods(AnalysisResultPtr ar,
                      StringToFunctionScopePtrMap &func,
                      bool collectPrivate);

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
   * Collect parent class names.
   */
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

  int32_t getNumDeclMethods() const {
    return m_numDeclMethods;
  }

  const boost::container::flat_set<std::string>& getClassRequiredExtends()
    const {
    return m_requiredExtends;
  }

  const boost::container::flat_set<std::string>& getClassRequiredImplements()
    const {
    return m_requiredImplements;
  }

  const std::vector<std::string> &getUsedTraitNames() const {
    return m_usedTraitNames;
  }

  const std::vector<std::pair<std::string, std::string>>& getTraitAliases()
    const {
    return m_traitAliases;
  }

  void addTraitAlias(TraitAliasStatementPtr aliasStmt);

  void addClassRequirement(const std::string &requiredName, bool isExtends);

  void importUsedTraits(AnalysisResultPtr ar);

  /**
   * Serialize the iface, not everything.
   */
  void serialize(JSON::CodeError::OutputStream &out) const;
  void serialize(JSON::DocTarget::OutputStream &out) const;

  bool isInterface() const { return m_kindOf == KindOf::Interface; }
  bool isFinal() const { return m_kindOf == KindOf::FinalClass ||
                                m_kindOf == KindOf::Trait ||
                                m_kindOf == KindOf::UtilClass ||
                                m_kindOf == KindOf::Enum; }
  bool isAbstract() const { return m_kindOf == KindOf::AbstractClass ||
                                   m_kindOf == KindOf::Trait ||
                                   m_kindOf == KindOf::UtilClass; }
  bool isTrait() const { return m_kindOf == KindOf::Trait; }
  bool isEnum() const { return m_kindOf == KindOf::Enum; }
  bool isStaticUtil() const { return m_kindOf == KindOf::UtilClass; }
  bool hasProperty(const std::string &name) const;
  bool hasConst(const std::string &name) const;

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
                   FileScopeRawPtr fileScope,
                   FunctionScopePtr funcScope);

  void setNeedsCppCtor(bool needsCppCtor) { m_needsCppCtor = needsCppCtor; }
  void setNeedsInitMethod(bool needsInit) { m_needsInit = needsInit; }
  bool needsCppCtor()    const { return m_needsCppCtor; }
  bool needsInitMethod() const { return m_needsInit; }

  bool canSkipCreateMethod(AnalysisResultConstPtr ar) const;
  bool checkHasPropTable(AnalysisResultConstPtr ar);

  const StringData* getFatalMessage() const {
    return m_fatal_error_msg;
  }

  void setFatal(const AnalysisTimeFatalException& fatal) {
    assert(m_fatal_error_msg == nullptr);
    m_fatal_error_msg = makeStaticString(fatal.getMessage());
    assert(m_fatal_error_msg != nullptr);
  }

private:
  void informClosuresAboutScopeClone(ConstructPtr root,
                                     FunctionScopePtr outerScope,
                                     AnalysisResultPtr ar);


  /////////////////////////////////////////////////////////////////////////////
  // Trait flattening.

public:
  struct TraitMethod {
    TraitMethod(ClassScopePtr trait_,
                MethodStatementPtr method_,
                ModifierExpressionPtr modifiers_,
                StatementPtr ruleStmt_)
      : trait(trait_)
      , method(method_)
      , originalName(method_->getOriginalName())
      , modifiers(modifiers_ ? modifiers_ : method_->getModifiers())
      , ruleStmt(ruleStmt_)
    {}

    TraitMethod(ClassScopePtr trait_,
                MethodStatementPtr method_,
                ModifierExpressionPtr modifiers_,
                StatementPtr ruleStmt_,
                const std::string& originalName_)
      : trait(trait_)
      , method(method_)
      , originalName(originalName_)
      , modifiers(modifiers_ ? modifiers_ : method_->getModifiers())
      , ruleStmt(ruleStmt_)
    {}

    using class_type = ClassScopePtr;
    using method_type = MethodStatementPtr;
    using modifiers_type = ModifierExpressionPtr;

    const ClassScopePtr      trait;
    const MethodStatementPtr method;
    const std::string        originalName;
    ModifierExpressionPtr    modifiers;
    const StatementPtr       ruleStmt; // for methods imported via aliasing
  };

private:
  struct TMIOps {
    using prec_type  = TraitPrecStatementPtr;
    using alias_type = TraitAliasStatementPtr;

    static bool strEmpty(const std::string& str)    { return str.empty(); }
    static std::string clsName(ClassScopePtr cls)   {
      return cls->getOriginalName();
    }

    static bool isTrait(ClassScopePtr cls)          { return cls->isTrait(); }
    static bool isAbstract(ModifierExpressionPtr m) { return m->isAbstract(); }

    static bool exclude(const std::string&) { return false; }

    static TraitMethod traitMethod(ClassScopePtr traitCls,
                                   MethodStatementPtr methStmt,
                                   alias_type stmt) {
      return TraitMethod(traitCls, methStmt, stmt->getModifiers(),
                         stmt, stmt->getNewMethodName());
    }

    static std::string precMethodName(prec_type stmt) {
      return stmt->getMethodName();
    }
    static std::string precSelectedTraitName(prec_type stmt) {
      return stmt->getTraitName();
    }
    static hphp_string_iset precOtherTraitNames(prec_type stmt) {
      hphp_string_iset otherTraitNames;
      stmt->getOtherTraitNames(otherTraitNames);
      return otherTraitNames;
    }

    static std::string aliasTraitName(alias_type stmt) {
      return stmt->getTraitName();
    }
    static std::string aliasOrigMethodName(alias_type stmt) {
      return stmt->getMethodName();
    }
    static std::string aliasNewMethodName(alias_type stmt) {
      return stmt->getNewMethodName();
    }
    static ModifierExpressionPtr aliasModifiers(alias_type stmt) {
      return stmt->getModifiers();
    }

    static void addTraitAlias(ClassScope* cs, alias_type stmt,
                              ClassScopePtr traitCls);

    static ClassScopePtr findSingleTraitWithMethod(ClassScope* cs,
                                              const std::string& origMethName);
    static ClassScopePtr findTraitClass(ClassScope* cs,
                                        const std::string& traitName);
    static MethodStatementPtr findTraitMethod(ClassScope* cs,
                                              ClassScopePtr traitCls,
                                              const std::string& origMethName);

    static void errorUnknownMethod(prec_type stmt) {
      Compiler::Error(Compiler::UnknownObjectMethod, stmt);
    }
    static void errorUnknownMethod(alias_type stmt,
                                   const std::string& methName) {
      stmt->analysisTimeFatal(Compiler::UnknownTraitMethod,
                              Strings::TRAITS_UNKNOWN_TRAIT_METHOD,
                              methName.c_str());
    }
    template <class Stmt>
    static void errorUnknownTrait(Stmt stmt,
                                  const std::string& traitName) {
      stmt->analysisTimeFatal(Compiler::UnknownTrait,
                              Strings::TRAITS_UNKNOWN_TRAIT,
                              traitName.c_str());
    }
    static void errorDuplicateMethod(const ClassScope* cs,
                                     const std::string& methName) {
      cs->getStmt()->analysisTimeFatal(
        Compiler::MethodInMultipleTraits,
        Strings::METHOD_IN_MULTIPLE_TRAITS,
        methName.c_str()
      );
    }
  };

  friend class TMIOps;

public:
  using TMIData = TraitMethodImportData<TraitMethod, TMIOps,
                                        std::string,
                                        string_hashi, string_eqstri>;

private:
  MethodStatementPtr importTraitMethod(const TraitMethod& traitMethod,
                                       AnalysisResultPtr ar,
                                       std::string methName);
  void applyTraitRules(TMIData& tmid);

  bool hasMethod(const std::string &methodName) const;
  bool usesTrait(const std::string &traitName) const;

  void importTraitProperties(AnalysisResultPtr ar);
  void importClassRequirements(AnalysisResultPtr ar, ClassScopePtr trait);


  /////////////////////////////////////////////////////////////////////////////
  // Data members.

private:
  // need to maintain declaration order for ClassInfo map
  FunctionScopePtrVec m_functionsVec;

  std::string m_parent;
  mutable std::vector<std::string> m_bases;
  UserAttributeMap m_userAttributes;

  std::vector<std::string> m_usedTraitNames;
  boost::container::flat_set<std::string> m_requiredExtends;
  boost::container::flat_set<std::string> m_requiredImplements;
  // m_traitAliases is used to support ReflectionClass::getTraitAliases
  std::vector<std::pair<std::string, std::string>> m_traitAliases;

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
  int32_t m_numDeclMethods{-1};

  // holds the fact that accessing this class declaration is a fatal error
  const StringData* m_fatal_error_msg = nullptr;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_CLASS_SCOPE_H_
