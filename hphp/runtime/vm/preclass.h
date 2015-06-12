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

#ifndef incl_HPHP_VM_PRECLASS_H_
#define incl_HPHP_VM_PRECLASS_H_

#include "hphp/runtime/base/atomic-shared-ptr.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/attr.h"
#include "hphp/runtime/base/repo-auth-type.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/user-attributes.h"
#include "hphp/runtime/base/atomic-countable.h"
#include "hphp/runtime/vm/indexed-string-map.h"
#include "hphp/runtime/vm/type-constraint.h"

#include "hphp/util/fixed-vector.h"
#include "hphp/util/range.h"

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

using TraitNameSet = std::unordered_set<LowStringPtr,
                                        string_data_hash,
                                        string_data_isame>;

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
 *    Closures have a special kind of hoistability, ClosureHoistable, that
 *    requires them to be defined first (before any other classes or
 *    functions), to avoid races if other threads are trying to load the same
 *    unit.  See the comments in Unit::initialMerge for more information.
 *
 */
struct PreClass : AtomicCountable {
  friend class PreClassEmitter;

  /////////////////////////////////////////////////////////////////////////////
  // Types.

  enum Hoistable {
    NotHoistable,
    Mergeable,
    MaybeHoistable,
    AlwaysHoistable,
    ClosureHoistable
  };

  /*
   * Instance property information.
   */
  struct Prop {
    Prop(PreClass* preClass,
         const StringData* name,
         Attr attrs,
         const StringData* typeConstraint,
         const StringData* docComment,
         const TypedValue& val,
         RepoAuthType repoAuthType);

    void prettyPrint(std::ostream&, const PreClass*) const;

    const StringData* name()           const { return m_name; }
    const StringData* mangledName()    const { return m_mangledName; }
    Attr              attrs()          const { return m_attrs; }
    const StringData* typeConstraint() const { return m_typeConstraint; }
    const StringData* docComment()     const { return m_docComment; }
    const TypedValue& val()            const { return m_val; }
    RepoAuthType      repoAuthType()   const { return m_repoAuthType; }

  private:
    LowStringPtr m_name;
    LowStringPtr m_mangledName;
    Attr m_attrs;
    LowStringPtr m_typeConstraint;
    LowStringPtr m_docComment;
    TypedValue m_val;
    RepoAuthType m_repoAuthType;
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
    bool isAbstract()      const { return m_val.constModifiers().m_isAbstract; }
    bool isType()          const { return m_val.constModifiers().m_isType; }

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
   * @see: http://docs.hhvm.com/manual/en/language.oop5.traits.php#language.oop5.traits.conflict
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
   * @see: http://docs.hhvm.com/manual/en/language.oop5.traits.php#language.oop5.traits.conflict
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
   * For a builtin class c_Foo:
   *
   * builtinObjSize is the size of the object, excluding ObjectData (i.e.,
   * sizeof(c_Foo) - sizeof(ObjectData)).
   *
   * builtinODOffset is the offset of the ObjectData subobject in c_Foo.
   */
  uint32_t builtinObjSize() const { return m_builtinObjSize; }
  int32_t builtinODOffset() const { return m_builtinODOffset; }

  /*
   * Extension builtin classes have custom creation and destruction routines.
   */
  BuiltinCtorFunction instanceCtor() const { return m_instanceCtor; }
  BuiltinDtorFunction instanceDtor() const { return m_instanceDtor; }

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
  typedef IterRange<Type const*> TypeName##Range;                             \
  TypeName##Range all##Fields() const {                                       \
    return TypeName##Range(fields(), fields() + m_##fields.size());           \
  }

  DEF_ACCESSORS(Func*, Func, methods, Methods)
  DEF_ACCESSORS(Const, Const, constants, Constants)
  DEF_ACCESSORS(Prop, Prop, properties, Properties)

#undef DEF_ACCESSORS

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
   * Check whether a method or property exists on the PreClass.
   */
  bool hasMethod(const StringData* methName) const;
  bool hasProp(const StringData* propName) const;

  /*
   * Look up a method or property on the PreClass.
   *
   * @requires: hasMethod(), hasProp(), respectively.
   */
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
  NamedEntity* m_namedEntity;
  int m_line1;
  int m_line2;
  Offset m_offset;
  Id m_id;
  uint32_t m_builtinObjSize{0};
  int32_t m_builtinODOffset{0};
  Attr m_attrs;
  Hoistable m_hoistable;
  LowStringPtr m_name;
  LowStringPtr m_parent;
  LowStringPtr m_docComment;
  int32_t m_numDeclMethods;
  Slot m_ifaceVtableSlot{kInvalidSlot};
  TypeConstraint m_enumBaseTy;
  BuiltinCtorFunction m_instanceCtor{nullptr};
  BuiltinDtorFunction m_instanceDtor{nullptr};
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
