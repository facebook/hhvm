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

#pragma once

#include "hphp/runtime/base/atomic-shared-ptr.h"
#include "hphp/runtime/base/attr.h"
#include "hphp/runtime/base/repo-auth-type.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/user-attributes.h"
#include "hphp/runtime/base/atomic-countable.h"
#include "hphp/runtime/vm/containers.h"
#include "hphp/runtime/vm/indexed-string-map.h"
#include "hphp/runtime/vm/type-constraint.h"

#include "hphp/util/fixed-vector.h"

#include <type_traits>
#include <unordered_set>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct Class;
struct Func;
struct ObjectData;
struct NamedType;
struct StaticCoeffects;
struct StringData;
struct Unit;

struct PreClassEmitter;

namespace Native { struct NativeDataInfo; }

///////////////////////////////////////////////////////////////////////////////

using TraitNameSet = std::set<LowStringPtr,
                              string_data_lti>;

using BuiltinCtorFunction = LowPtr<ObjectData*(Class*)>;
using BuiltinDtorFunction = LowPtr<void(ObjectData*, const Class*)>;

/*
 * A PreClass represents the source-level definition of a PHP class, interface,
 * or trait.  Includes things like the names of the parent class (if any), and
 * the names of any interfaces implemented or traits used.  Also contains
 * metadata about properties and methods of the class.
 *
 * This is separate from an actual Class because in different requests
 * (depending on include order), the actual instantiation of a PreClass may
 * differ since names may have different meanings.  For example, if the
 * PreClass "extends Foo", and Foo is defined differently in different
 * requests, we will make a different Class.
 */
struct PreClass : AtomicCountable {
  friend struct PreClassEmitter;
  using UpperBoundVec = VMTypeIntersectionConstraint;
  using UpperBoundMap = vm_flat_map<const StringData*, UpperBoundVec>;

  /////////////////////////////////////////////////////////////////////////////
  // Types.

  /*
   * Instance property information.
   */
  struct Prop {
    Prop(PreClass* preClass,
         const StringData* name,
         Attr attrs,
         const StringData* userType,
         const TypeConstraint& typeConstraint,
         const TypeIntersectionConstraint& ubs,
         const StringData* docComment,
         const TypedValue& val,
         RepoAuthType repoAuthType,
         UserAttributeMap userAttributes);

    void prettyPrint(std::ostream&, const PreClass*) const;

    const StringData* name()           const { return m_name; }
    Attr              attrs()          const { return m_attrs; }
    const StringData* userType()       const { return m_userType; }
    const TypeConstraint& typeConstraint() const { return m_typeConstraint; }
    const UpperBoundVec& upperBounds() const { return m_ubs; }
    const StringData* docComment()     const { return m_docComment; }
    const TypedValue& val()            const { return m_val; }
    RepoAuthType      repoAuthType()   const { return m_repoAuthType; }
    const UserAttributeMap&
                      userAttributes() const { return m_userAttributes; }

  private:
    LowStringPtr m_name;
    Attr m_attrs;
    LowStringPtr m_userType;
    LowStringPtr m_docComment;
    TypedValue m_val;
    RepoAuthType m_repoAuthType;
    TypeConstraint m_typeConstraint;
    UpperBoundVec m_ubs;
    UserAttributeMap m_userAttributes;
  };

  /*
   * Class constant information.
   */
  struct Const {
    // Invariance defines the relationship of m_resolvedTypeStructure
    // to any derived classes which might override this
    // type-constant. It is only meaningful for type-constants with a
    // non-null m_resolvedTypeStructure.
    enum class Invariance : uint8_t {
      // Nothing can be said
      None,
      // m_resolvedTypeStructure is present in all derived classes
      // (not necessarily the same contents, however).
      Present,
      // m_resolvedTypeStructure is present in all derived classes and
      // always has a classname field.
      ClassnamePresent,
      // m_resolvedTypeStructure is present and identical in all
      // derived classes
      Same
    };

    Const(const StringData* name,
          const StringData* cls,
          const TypedValueAux& val,
          Array resolvedTypeStructure,
          Invariance invariance,
          bool fromTrait);

    void prettyPrint(std::ostream&, const PreClass*) const;

    const StringData* name()     const { return m_name; }
    const StringData* cls()      const { return m_cls; }
    const TypedValueAux& val()   const { return m_val; }
    const Array& resolvedTypeStructure() const {
      return m_resolvedTypeStructure;
    }
    bool isFromTrait()     const { return m_fromTrait; }
    bool isAbstractAndUninit()   const {
      return (m_val.constModifiers().isAbstract() &&
              !m_val.is_init() &&
              m_val.is_const_val_missing());
    }
    bool isAbstract()            const {
      return m_val.constModifiers().isAbstract();
    }
    ConstModifiers::Kind kind()  const { return m_val.constModifiers().kind(); }
    Invariance invariance() const { return m_invariance; }

    StaticCoeffects coeffects()  const;

  private:
    LowStringPtr m_name;
    // The original class which defined this constant, if
    // non-null. This is normally the PreClass where this constant
    // exists. However if this is non-null, HHBBC has propagated the
    // constant and we need to preserve the original declaring class.
    LowStringPtr m_cls;
    /* m_aux.u_isAbstractConst indicates an abstract constant. A TypedValue
     * with KindOfUninit represents a constant whose value is not
     * statically available (e.g. "const X = self::Y + 5;") */
    TypedValueAux m_val;
    // HHBBC pre-resolved type-structure. Only present if this is a
    // non-abstract type-constant.
    Array m_resolvedTypeStructure;
    Invariance m_invariance : 2;
    bool m_fromTrait : 1;
  };

  /*
   * Trait precedence rule.  Describes a usage of the `insteadof' operator.
   *
   * @see: http://php.net/manual/en/language.oop5.traits.php#language.oop5.traits.conflict
   */
  struct TraitPrecRule {
    TraitPrecRule();
    TraitPrecRule(const StringData* selectedTraitName,
                  const StringData* methodName);

    const StringData* methodName()        const { return m_methodName; }
    const StringData* selectedTraitName() const { return m_selectedTraitName; }
    const TraitNameSet& otherTraitNames() const { return m_otherTraitNames; }

    void addOtherTraitName(const StringData* traitName);

    template<class SerDe> void serde(SerDe& sd) {
      sd(m_methodName)(m_selectedTraitName)(m_otherTraitNames);
    }

  private:
    LowStringPtr m_methodName;
    LowStringPtr m_selectedTraitName;
    TraitNameSet m_otherTraitNames;
  };

  /*
   * Trait alias rule.  Describes a usage of the `as' operator.
   *
   * @see: http://php.net/manual/en/language.oop5.traits.php#language.oop5.traits.conflict
   */
  struct TraitAliasRule {
    TraitAliasRule();
    TraitAliasRule(const StringData* traitName,
                   const StringData* origMethodName,
                   const StringData* newMethodName,
                   Attr modifiers);

    const StringData* traitName()      const { return m_traitName; }
    const StringData* origMethodName() const { return m_origMethodName; }
    const StringData* newMethodName()  const { return m_newMethodName; }
    Attr              modifiers()      const { return m_modifiers; }

    template<class SerDe> void serde(SerDe& sd) {
      sd(m_traitName)(m_origMethodName)(m_newMethodName)(m_modifiers);
    }

    /*
     * Pair of (new name, original name) representing the rule.
     *
     * This is the format for alias rules expected by reflection.
     */
    using NamePair = std::pair<LowStringPtr,LowStringPtr>;

    /*
     * Get the rule as a NamePair.
     */
    NamePair asNamePair() const;

  private:
    LowStringPtr m_traitName;
    LowStringPtr m_origMethodName;
    LowStringPtr m_newMethodName;
    Attr         m_modifiers;
  };

  /*
   * Trait and interface requirements.
   *
   * Represents a `require implements', `require extends', or `require class' declaration.
   */
  enum RequirementKind {
    RequirementImplements = 0,
    RequirementExtends    = 0x1,
    RequirementClass      = 0x2,
  };

  struct ClassRequirement {
    ClassRequirement();
    ClassRequirement(const StringData* req, const RequirementKind reqKind);

    const StringData* name() const;
    RequirementKind kind() const;
    bool is_same(const ClassRequirement* other) const;
    size_t hash() const;

    template<class SerDe>
    typename std::enable_if<SerDe::deserializing>::type serde(SerDe& sd);

    template<class SerDe>
    typename std::enable_if<!SerDe::deserializing>::type serde(SerDe& sd);

  private:
    static uintptr_t pack(const StringData* req, bool isExtends);

    uintptr_t m_word{0};
  };

private:
  using MethodMap = IndexedStringMap<Func*,Slot>;
  using PropMap = IndexedStringMap<Prop,Slot>;
  using ConstMap = IndexedStringMap<Const,Slot>;

public:
  using InterfaceVec = VMFixedVector<LowStringPtr>;
  using IncludedEnumsVec = VMFixedVector<LowStringPtr>;
  using UsedTraitVec = VMFixedVector<LowStringPtr>;
  using ClassRequirementsVec = VMFixedVector<ClassRequirement>;
  using TraitPrecRuleVec = VMFixedVector<TraitPrecRule>;
  using TraitAliasRuleVec = VMFixedVector<TraitAliasRule>;


  /////////////////////////////////////////////////////////////////////////////
  // Construction and destruction.

  PreClass(Unit* unit, int line1, int line2, const StringData* n,
           Attr attrs, const StringData* parent, const StringData* docComment);
  ~PreClass();

  void atomicRelease();


  /////////////////////////////////////////////////////////////////////////////
  // Direct data accessors.

  /*
   * Basic info.
   */
  Unit*             unit()         const { return m_unit; }
  NamedType*        namedType()    const { return m_namedType; }
  int               line1()        const { return m_line1; }
  int               line2()        const { return m_line2; }
  Attr              attrs()        const { return m_attrs; }
  const StringData* name()         const { return m_name; }
  const StringData* parent()       const { return m_parent; }
  const StringData* docComment()   const { return m_docComment; }

  int64_t dynConstructSampleRate() const {
    return m_dynConstructSampleRate;
  }

  /*
   * Number of methods declared on this class (as opposed to included via
   * traits).
   *
   * This value is only valid when trait methods are flattened; otherwise, it
   * is -1.
   */
  int32_t numDeclMethods() const { return m_numDeclMethods; }

  /*
   * The interface vtable slot for this PreClass, or kInvalidSlot if it wasn't
   * assigned one or isn't an interface.
   */
  Slot ifaceVtableSlot() const { return m_ifaceVtableSlot; }

  /*
   * If this is an enum class, return the type of its enum values.
   */
  const TypeConstraint& enumBaseTy() const { return m_enumBaseTy; }

  /*
   * Accessors for vectory data.
   */
  const InterfaceVec& interfaces()           const { return m_interfaces; }
  const IncludedEnumsVec& includedEnums()    const { return m_includedEnums; }
  const UsedTraitVec& usedTraits()           const { return m_usedTraits; }
  const ClassRequirementsVec& requirements() const { return m_requirements; }
  const UserAttributeMap& userAttributes()   const { return m_userAttributes; }

  /*
   * If the parent is sealed, enforce that we are in the whitelist.
   * Note that parent may be derived through a using, extending, or
   * implementing relationship.
   */
  void
  enforceInMaybeSealedParentWhitelist(const PreClass* parentPreClass) const;

  bool enableMethodTraitDiamond();

  /*
   * Funcs, Consts, and Props all behave similarly.  Define raw accessors
   * foo() and numFoos() for people munging by hand, and ranges.
   *
   *   methods();           constants();            properties();
   *   mutableMethods();    mutableConstants();     mutableProperties();
   *   numMethods();        numConstants();         numProperties();
   *   FuncRange            ConstRange              PropRange
   *   allMethods();        allConstants();         allProperties();
   */
#define DEF_ACCESSORS(Type, TypeName, fields, Fields)                         \
  Type const* fields() const      { return m_##fields.accessList(); }         \
  Type*       mutable##Fields()   { return m_##fields.mutableAccessList(); }  \
  size_t      num##Fields() const { return m_##fields.size(); }               \
  using TypeName##Range = folly::Range<Type const*>;                          \
  TypeName##Range all##Fields() const {                                       \
    return TypeName##Range(fields(), m_##fields.size());                      \
  }

  DEF_ACCESSORS(Func*, Func, methods, Methods)
  DEF_ACCESSORS(Const, Const, constants, Constants)
  DEF_ACCESSORS(Prop, Prop, properties, Properties)

#undef DEF_ACCESSORS

  const ConstMap& constantsMap() const { return m_constants; }

  /*
   * NativeData type declared in <<__NativeData>>.
   */
  const Native::NativeDataInfo* nativeDataInfo() const {
    return m_nativeDataInfo;
  }


  /////////////////////////////////////////////////////////////////////////////
  // Other accessors.

  /*
   * StrNR wrappers for name() and parent().
   */
  StrNR nameStr()   const { return StrNR(m_name); }
  StrNR parentStr() const { return StrNR(m_parent); }

  /*
   * Attribute shortcut accessors.
   */
  bool isPersistent() const;
  bool isBuiltin() const;

  /*
   * Check whether a constant, method, or property exists on the PreClass.
   */
  bool hasConstant(const StringData* cnsName) const;
  bool hasMethod(const StringData* methName) const;
  bool hasProp(const StringData* propName) const;

  /*
   * Look up a constant, method, or property on the PreClass.
   *
   * @requires: hasConstant(), hasMethod(), hasProp(), respectively.
   */
  const Const* lookupConstant(const StringData* cnsName) const;
  Func* lookupMethod(const StringData* methName) const;

  /*
   * Static offset accessors, used by the JIT.
   */
  static constexpr Offset nameOffset()  { return offsetof(PreClass, m_name); }
  static constexpr Offset attrsOffset() { return offsetof(PreClass, m_attrs); }
  static constexpr Offset unitOffset() { return offsetof(PreClass, m_unit); }


  /////////////////////////////////////////////////////////////////////////////
  // Other methods.

  void prettyPrint(std::ostream& out) const;

  /*
   * Munge a property's accessibility into its name.
   */
  static const StringData* manglePropName(const StringData* className,
                                          const StringData* propName,
                                          Attr attrs);


  /////////////////////////////////////////////////////////////////////////////
  // Data members.

private:
  Unit* m_unit;
  LowPtr<NamedType> m_namedType;
  int m_line1;
  int m_line2;
  Attr m_attrs;
  LowStringPtr m_name;
  LowStringPtr m_parent;
  LowStringPtr m_docComment;
  int32_t m_numDeclMethods;
  Slot m_ifaceVtableSlot{kInvalidSlot};
  TypeConstraint m_enumBaseTy;
  InterfaceVec m_interfaces;
  IncludedEnumsVec m_includedEnums;
  UsedTraitVec m_usedTraits;
  ClassRequirementsVec m_requirements;
  UserAttributeMap m_userAttributes;
  const Native::NativeDataInfo *m_nativeDataInfo{nullptr};
  MethodMap m_methods;
  PropMap m_properties;
  ConstMap m_constants;
  int64_t m_dynConstructSampleRate;
};

using PreClassPtr = AtomicSharedPtr<PreClass>;

///////////////////////////////////////////////////////////////////////////////
}

#define incl_HPHP_VM_PRECLASS_INL_H_
#include "hphp/runtime/vm/preclass-inl.h"
#undef incl_HPHP_VM_PRECLASS_INL_H_
