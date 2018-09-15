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

#ifndef incl_HPHP_VM_PRECLASS_H_
#define incl_HPHP_VM_PRECLASS_H_

#include "hphp/runtime/base/atomic-shared-ptr.h"
#include "hphp/runtime/base/attr.h"
#include "hphp/runtime/base/repo-auth-type.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/user-attributes.h"
#include "hphp/runtime/base/atomic-countable.h"
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
struct NamedEntity;
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
 *
 * Hoistability:
 *
 *    When a Unit is loaded at run time, each PreClass in the Unit which is
 *    determined to be `hoistable' will be loaded by the runtime (in the order
 *    they appear in the source) before the Unit's pseudo-main is executed.
 *    The hoistability rules ensure that loading a PreClass which is determined
 *    to be hoistable will never cause the autoload facility to be invoked.
 *
 *    A class is considered `hoistable' iff the following conditions apply:
 *
 *      - It's defined at the top level.
 *
 *      - There is no other definition for a class of the same name in
 *        its Unit.
 *
 *      - It uses no traits.  (It may however *be* a trait.)
 *
 *      - It implements no interfaces.  (It may however *be* an interface.)
 *
 *      - It is not an enum.
 *
 *      - It has no parent OR
 *           The parent is hoistable and defined earlier in the unit OR
 *           The parent is already defined when the unit is required
 *
 *    The very last condition here (parent already defined when the unit is
 *    required) is not known at parse time.  This leads to the Maybe/Always
 *    split below.
 *
 */
struct PreClass : AtomicCountable {
  friend struct PreClassEmitter;

  /////////////////////////////////////////////////////////////////////////////
  // Types.

  enum Hoistable {
    NotHoistable,
    Mergeable,
    MaybeHoistable,
    AlwaysHoistable,
  };

  /*
   * Instance property information.
   */
  struct Prop {
    Prop(PreClass* preClass,
         const StringData* name,
         Attr attrs,
         const StringData* userType,
         const TypeConstraint& typeConstraint,
         const StringData* docComment,
         const TypedValue& val,
         RepoAuthType repoAuthType,
         UserAttributeMap userAttributes);

    void prettyPrint(std::ostream&, const PreClass*) const;

    const StringData* name()           const { return m_name; }
    const StringData* mangledName()    const { return m_mangledName; }
    Attr              attrs()          const { return m_attrs; }
    const StringData* userType()       const { return m_userType; }
    const TypeConstraint& typeConstraint() const { return m_typeConstraint; }
    const StringData* docComment()     const { return m_docComment; }
    const TypedValue& val()            const { return m_val; }
    RepoAuthType      repoAuthType()   const { return m_repoAuthType; }
    const UserAttributeMap&
                      userAttributes() const { return m_userAttributes; }

  private:
    LowStringPtr m_name;
    LowStringPtr m_mangledName;
    Attr m_attrs;
    LowStringPtr m_userType;
    LowStringPtr m_docComment;
    TypedValue m_val;
    RepoAuthType m_repoAuthType;
    TypeConstraint m_typeConstraint;
    UserAttributeMap m_userAttributes;
  };

  /*
   * Class constant information.
   */
  struct Const {
    Const(const StringData* name,
          const TypedValueAux& val,
          const StringData* phpCode);

    void prettyPrint(std::ostream&, const PreClass*) const;

    const StringData* name()     const { return m_name; }
    const TypedValueAux& val()   const { return m_val; }
    const StringData* phpCode()  const { return m_phpCode; }
    bool isAbstract()      const { return m_val.constModifiers().isAbstract; }
    bool isType()          const { return m_val.constModifiers().isType; }

    template<class SerDe> void serde(SerDe& sd);

  private:
    LowStringPtr m_name;
    /* m_aux.u_isAbstractConst indicates an abstract constant. A TypedValue
     * with KindOfUninit represents a constant whose value is not
     * statically available (e.g. "const X = self::Y + 5;") */
    TypedValueAux m_val;
    LowStringPtr m_phpCode;
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
   * Represents a `require implements' or `require extends' declaration.
   */
  struct ClassRequirement {
    ClassRequirement();
    ClassRequirement(const StringData* req, bool isExtends);

    const StringData* name() const;
    bool is_extends() const;
    bool is_implements() const;
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
  typedef IndexedStringMap<Func*,false,Slot> MethodMap;
  typedef IndexedStringMap<Prop,true,Slot> PropMap;
  typedef IndexedStringMap<Const,true,Slot> ConstMap;

public:
  typedef FixedVector<LowStringPtr> InterfaceVec;
  typedef FixedVector<LowStringPtr> UsedTraitVec;
  typedef FixedVector<ClassRequirement> ClassRequirementsVec;
  typedef FixedVector<TraitPrecRule> TraitPrecRuleVec;
  typedef FixedVector<TraitAliasRule> TraitAliasRuleVec;


  /////////////////////////////////////////////////////////////////////////////
  // Construction and destruction.

  PreClass(Unit* unit, int line1, int line2, Offset o, const StringData* n,
           Attr attrs, const StringData* parent, const StringData* docComment,
           Id id, Hoistable hoistable);
  ~PreClass();

  void atomicRelease();


  /////////////////////////////////////////////////////////////////////////////
  // Direct data accessors.

  /*
   * Basic info.
   */
  Unit*             unit()         const { return m_unit; }
  NamedEntity*      namedEntity()  const { return m_namedEntity; }
  int               line1()        const { return m_line1; }
  int               line2()        const { return m_line2; }
  Offset            getOffset()    const { return m_offset; }
  Id                id()           const { return m_id; }
  Attr              attrs()        const { return m_attrs; }
  const StringData* name()         const { return m_name; }
  const StringData* parent()       const { return m_parent; }
  const StringData* docComment()   const { return m_docComment; }
  Hoistable         hoistability() const { return m_hoistable; }

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
  const UsedTraitVec& usedTraits()           const { return m_usedTraits; }
  const ClassRequirementsVec& requirements() const { return m_requirements; }
  const TraitPrecRuleVec& traitPrecRules()   const { return m_traitPrecRules; }
  const TraitAliasRuleVec& traitAliasRules() const { return m_traitAliasRules; }
  const UserAttributeMap& userAttributes()   const { return m_userAttributes; }

  /*
   * If the parent is sealed, enforce that we are in the whitelist.
   * Note that parent may be derived through a using, extending, or
   * implementing relationship.
   */
  void
  enforceInMaybeSealedParentWhitelist(const PreClass* parentPreClass) const;

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
   * NativeData type declared in <<__NativeData("Type")>>.
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
  const Prop* lookupProp(const StringData* propName) const;

  /*
   * Static offset accessors, used by the JIT.
   */
  static constexpr Offset nameOffset()  { return offsetof(PreClass, m_name); }
  static constexpr Offset attrsOffset() { return offsetof(PreClass, m_attrs); }


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
  LowPtr<NamedEntity> m_namedEntity;
  int m_line1;
  int m_line2;
  Offset m_offset;
  Id m_id;
  Attr m_attrs;
  Hoistable m_hoistable;
  LowStringPtr m_name;
  LowStringPtr m_parent;
  LowStringPtr m_docComment;
  int32_t m_numDeclMethods;
  Slot m_ifaceVtableSlot{kInvalidSlot};
  TypeConstraint m_enumBaseTy;
  InterfaceVec m_interfaces;
  UsedTraitVec m_usedTraits;
  ClassRequirementsVec m_requirements;
  TraitPrecRuleVec m_traitPrecRules;
  TraitAliasRuleVec m_traitAliasRules;
  UserAttributeMap m_userAttributes;
  const Native::NativeDataInfo *m_nativeDataInfo{nullptr};
  MethodMap m_methods;
  PropMap m_properties;
  ConstMap m_constants;
};

typedef AtomicSharedPtr<PreClass> PreClassPtr;

///////////////////////////////////////////////////////////////////////////////
}

#define incl_HPHP_VM_PRECLASS_INL_H_
#include "hphp/runtime/vm/preclass-inl.h"
#undef incl_HPHP_VM_PRECLASS_INL_H_

#endif // incl_HPHP_VM_PRECLASS_H_
