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

#include "hphp/runtime/base/attr.h"
#include "hphp/runtime/base/lazy-string-data.h"
#include "hphp/runtime/base/repo-auth-type.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/user-attributes.h"
#include "hphp/runtime/vm/containers.h"
#include "hphp/runtime/vm/generics-info.h"
#include "hphp/runtime/vm/indexed-string-map.h"
#include "hphp/runtime/vm/type-constraint.h"

#include "hphp/util/atomic-countable.h"
#include "hphp/util/atomic-shared-ptr.h"
#include "hphp/util/fixed-vector.h"
#include "hphp/util/ptr.h"

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

/*
 * On ARM not all builtin function pointers are 8 byte aligned. So we can't use
 * PackedPtr for them.
 * We can also not always use SmallPtr because in devbuilds the function pointers
 * are sometimes > 4 GB.
 * So we use FullPtr in devbuilds and SmallPtr otherwise.
 */

#ifdef FULLPTR_FOR_BUILTINS
using BuiltinCtorFunction = FullPtr<ObjectData*(Class*)>;
using BuiltinDtorFunction = FullPtr<void(ObjectData*, const Class*)>;
#else
using BuiltinCtorFunction = SmallPtr<ObjectData*(Class*)>;
using BuiltinDtorFunction = SmallPtr<void(ObjectData*, const Class*)>;
#endif

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
  /////////////////////////////////////////////////////////////////////////////
  // Types.

  /*
   * Instance property information.
   */
  struct Prop {
    Prop(
      PreClass* preClass,
      const StringData* name,
      Attr attrs,
      const StringData* userType,
      TypeIntersectionConstraint&& constraints,
      const StringData* docComment,
      const TypedValue& val,
      RepoAuthType repoAuthType,
      UserAttributeMap userAttributes
    );

    void prettyPrint(std::ostream&, const PreClass*) const;

    const StringData* name()           const { return m_name; }
    Attr              attrs()          const { return m_attrs; }
    const StringData* userType()       const { return m_userType; }
    const TypeIntersectionConstraint& typeConstraints() const {
      return m_typeConstraints;
    }
    const StringData* docComment()     const { return m_docComment; }
    const TypedValue& val()            const { return m_val; }
    RepoAuthType      repoAuthType()   const { return m_repoAuthType; }
    const UserAttributeMap&
                      userAttributes() const { return m_userAttributes; }

  private:
    PackedStringPtr m_name;
    Attr m_attrs;
    PackedStringPtr m_userType;
    PackedStringPtr m_docComment;
    TypedValue m_val;
    RepoAuthType m_repoAuthType;
    TypeIntersectionConstraint m_typeConstraints;
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
      return (m_val.constModifierFlags().isAbstract &&
              !m_val.is_init() &&
              m_val.is_const_val_missing());
    }
    bool isAbstract()            const {
      return m_val.constModifierFlags().isAbstract;
    }
    ConstModifierFlags::Kind kind()  const { return m_val.constModifierFlags().kind; }
    Invariance invariance() const { return m_invariance; }

    StaticCoeffects coeffects()  const;

  private:
    PackedStringPtr m_name;
    // The original class which defined this constant, if
    // non-null. This is normally the PreClass where this constant
    // exists. However if this is non-null, HHBBC has propagated the
    // constant and we need to preserve the original declaring class.
    PackedStringPtr m_cls;
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
   * Trait and interface requirements.
   *
   * Represents a `require implements', `require extends', `require class',
   * or `require this as' declaration.
   */
  enum RequirementKind {
    RequirementImplements = 0,
    RequirementExtends    = 0x1,
    RequirementClass      = 0x2,
    RequirementThisAs     = 0x3,
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
  using InterfaceVec = VMFixedVector<PackedStringPtr>;
  using IncludedEnumsVec = VMFixedVector<PackedStringPtr>;
  using UsedTraitVec = VMFixedVector<PackedStringPtr>;
  using ClassRequirementsVec = VMFixedVector<ClassRequirement>;


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

  const folly::Range<const PackedStringPtr*> typeParamNames() const {
    return folly::Range<const PackedStringPtr*>(
      m_typeParamNames.begin(),
      m_typeParamNames.end()
    );
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
  PackedPtr<NamedType> m_namedType;
  int m_line1;
  int m_line2;
  Attr m_attrs;
  PackedStringPtr m_name;
  PackedStringPtr m_parent;
  PackedStringPtr m_docComment;
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
  FixedVector<PackedStringPtr> m_typeParamNames;
};

using PreClassPtr = AtomicSharedPtr<PreClass>;

///////////////////////////////////////////////////////////////////////////////
}

#define incl_HPHP_VM_PRECLASS_INL_H_
#include "hphp/runtime/vm/preclass-inl.h"
#undef incl_HPHP_VM_PRECLASS_INL_H_
