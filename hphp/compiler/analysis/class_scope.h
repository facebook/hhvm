/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#include "hphp/compiler/option.h"
#include "hphp/compiler/statement/class_statement.h"
#include "hphp/compiler/statement/method_statement.h"
#include "hphp/compiler/statement/trait_alias_statement.h"
#include "hphp/compiler/statement/trait_prec_statement.h"

#include "hphp/util/functional.h"
#include "hphp/util/hash-map.h"
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

/**
 * A class scope corresponds to a class declaration. We store all
 * inferred types and analyzed results here, so not to pollute syntax trees.
 */
struct ClassScope : BlockScope, FunctionContainer {
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
             const std::string& originalName, const std::string& parent,
             const std::vector<std::string>& bases,
             const std::vector<FunctionScopePtr>& methods);

  bool isNamed(const char* n) const;
  bool isNamed(const std::string& n) const {
    return isNamed(n.c_str());
  }
  bool classNameCtor() const {
    return getAttribute(ClassNameConstructor);
  }
  const std::string &getOriginalName() const;
  std::string getDocName() const;

  /**
   * Unmangle XHP class method scopes like xhp_x__composable_element::foo back
   * to x:composable_element::foo for user-visible messages like deprecation
   * warnings.
   */
  std::string getUnmangledScopeName() const;

  const std::string &getOriginalParent() const { return m_parent; }

  /**
   * Whether this is a user-defined class.
   */
  bool isUserClass() const { return !getAttribute(System);}
  bool isBuiltin() const override {
    return !getStmt();
  }

  /**
   * Helpers for parsing class functions and variables.
   */
  ModifierExpressionPtr setModifiers(ModifierExpressionPtr modifiers);
  ModifierExpressionPtr getModifiers() { return m_modifiers;}

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
  KindOf getKind() {
    return m_kindOf;
  }

  std::vector<std::string> const& getBases() const { return m_bases; }

  using UserAttributeMap = hphp_hash_map<std::string, ExpressionPtr,
                                         string_hashi, string_eqstri>;

  UserAttributeMap& userAttributes() { return m_userAttributes;}

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

  bool addClassRequirement(const std::string &requiredName, bool isExtends);

  void importUsedTraits(AnalysisResultPtr ar);

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

  void inheritedMagicMethods(ClassScopePtr super);
  void derivedMagicMethods(ClassScopePtr super);

  /**
   * Override function container
   */
  bool addFunction(AnalysisResultConstRawPtr ar,
                   FileScopeRawPtr fileScope,
                   FunctionScopePtr funcScope);

  const StringData* getFatalMessage() const {
    return m_fatal_error_msg;
  }

  void setFatal(const AnalysisTimeFatalException& fatal) {
    assert(m_fatal_error_msg == nullptr);
    m_fatal_error_msg = makeStaticString(fatal.getMessage());
    assert(m_fatal_error_msg != nullptr);
  }

  const std::vector<FunctionScopePtr>& allFunctions() const {
    return m_functionsVec;
  }

private:

  bool hasMethod(const std::string &methodName) const;
  bool usesTrait(const std::string &traitName) const;

  /////////////////////////////////////////////////////////////////////////////
  // Data members.

private:
  // need to maintain declaration order for ClassInfo map
  std::vector<FunctionScopePtr> m_functionsVec;

  std::string m_parent;
  mutable std::vector<std::string> m_bases;
  UserAttributeMap m_userAttributes;
  ModifierExpressionPtr m_modifiers;

  std::vector<std::string> m_usedTraitNames;
  boost::container::flat_set<std::string> m_requiredExtends;
  boost::container::flat_set<std::string> m_requiredImplements;

  mutable int m_attribute;
  KindOf m_kindOf;
  int32_t m_numDeclMethods{-1};

  // holds the fact that accessing this class declaration is a fatal error
  const StringData* m_fatal_error_msg = nullptr;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_CLASS_SCOPE_H_
