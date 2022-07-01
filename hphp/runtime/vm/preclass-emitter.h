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

#include "hphp/runtime/base/repo-auth-type.h"
#include "hphp/runtime/base/array-data.h"

#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/func-emitter.h"

#include <vector>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct UnitEmitter;

///////////////////////////////////////////////////////////////////////////////

/*
 * Information about an extension subclass of ObjectData.
 */
struct BuiltinObjExtents {
  // Total sizeof(c_Foo).
  size_t totalSizeBytes;

  // The offset of the ObjectData subobject in c_Foo.
  ptrdiff_t odOffsetBytes;
};

struct PreClassEmitter {
  typedef std::vector<FuncEmitter*> MethodVec;
  using UpperBoundVec = CompactVector<TypeConstraint>;
  using UpperBoundMap =
    std::unordered_map<const StringData*, CompactVector<TypeConstraint>>;

  struct Prop {
    Prop()
      : m_name(nullptr)
      , m_attrs(AttrNone)
      , m_userType(nullptr)
      , m_docComment(nullptr)
      , m_repoAuthType{}
      , m_userAttributes{}
    {}

    Prop(const PreClassEmitter* pce,
         const StringData* n,
         Attr attrs,
         const StringData* userType,
         const TypeConstraint& typeConstraint,
         const UpperBoundVec& ubs,
         const StringData* docComment,
         const TypedValue* val,
         RepoAuthType repoAuthType,
         UserAttributeMap userAttributes);
    ~Prop();

    const StringData* name() const { return m_name; }
    Attr attrs() const { return m_attrs; }
    const StringData* userType() const { return m_userType; }
    const TypeConstraint& typeConstraint() const { return m_typeConstraint; }
    const UpperBoundVec& upperBounds() const { return m_ubs; }
    const StringData* docComment() const { return m_docComment; }
    const TypedValue& val() const { return m_val; }
    RepoAuthType repoAuthType() const { return m_repoAuthType; }
    UserAttributeMap userAttributes() const { return m_userAttributes; }

    template<class SerDe> void serde(SerDe& sd) {
      sd(m_name)
        (m_attrs)
        (m_userType)
        (m_docComment)
        (m_val)
        (m_repoAuthType)
        (m_typeConstraint)
        (m_ubs)
        (m_userAttributes)
        ;
    }

  private:
    friend struct PreClassEmitter;

    LowStringPtr m_name;
    Attr m_attrs;
    LowStringPtr m_userType;
    LowStringPtr m_docComment;
    TypedValue m_val;
    RepoAuthType m_repoAuthType;
    TypeConstraint m_typeConstraint;
    UpperBoundVec m_ubs{};
    UserAttributeMap m_userAttributes;
  };

  struct Const {
    using CoeffectsVec = std::vector<LowStringPtr>;
    using Invariance = PreClass::Const::Invariance;

    Const()
      : m_name(nullptr)
      , m_cls(nullptr)
      , m_val(make_tv<KindOfUninit>())
      , m_coeffects({})
      , m_resolvedTypeStructure()
      , m_kind(ConstModifiers::Kind::Value)
      , m_invariance(Invariance::None)
      , m_isAbstract(false)
      , m_fromTrait(false)
    {}
    Const(const StringData* n,
          const StringData* cls,
          const TypedValue* val,
          CoeffectsVec coeffects,
          Array resolvedTypeStructure,
          ConstModifiers::Kind kind,
          Invariance invariance,
          bool isAbstract,
          bool fromTrait)
      : m_name(n)
      , m_cls(cls)
      , m_coeffects(std::move(coeffects))
      , m_resolvedTypeStructure(std::move(resolvedTypeStructure))
      , m_kind(kind)
      , m_invariance(invariance)
      , m_isAbstract(isAbstract)
      , m_fromTrait(fromTrait) {
      if (!val) {
        m_val.reset();
      } else {
        m_val = *val;
      }
    }

    const StringData* name() const { return m_name; }
    const StringData* cls() const { return m_cls; }
    const TypedValue& val() const { return m_val.value(); }
    const Optional<TypedValue>& valOption() const { return m_val; }
    bool isAbstract() const { return m_isAbstract; }
    const CoeffectsVec& coeffects() const { return m_coeffects; }
    const Array& resolvedTypeStructure() const {
      return m_resolvedTypeStructure;
    }
    ConstModifiers::Kind kind() const { return m_kind; }
    Invariance invariance() const { return m_invariance; }
    bool isFromTrait() const { return m_fromTrait; }

    template<class SerDe> void serde(SerDe& sd) {
      sd(m_name)
        (m_cls)
        (m_val)
        (m_coeffects)
        (m_resolvedTypeStructure)
        (m_kind)
        (m_invariance)
        (m_isAbstract)
        (m_fromTrait);
      assertx(IMPLIES(!m_resolvedTypeStructure.isNull(),
                      m_resolvedTypeStructure.isDict() &&
                      !m_resolvedTypeStructure.empty() &&
                      m_resolvedTypeStructure->isStatic() &&
                      m_kind == ConstModifiers::Kind::Type &&
                      m_val.has_value()));
      assertx(IMPLIES(m_invariance != Invariance::None,
                      !m_resolvedTypeStructure.isNull()));
    }

   private:
    LowStringPtr m_name;
    // Class which originally defined this const, if HHBBC has
    // propagated the definition (nullptr otherwise).
    LowStringPtr m_cls;
    Optional<TypedValue> m_val;
    CoeffectsVec m_coeffects;
    Array m_resolvedTypeStructure;
    ConstModifiers::Kind m_kind;
    Invariance m_invariance;
    bool m_isAbstract;
    bool m_fromTrait;
  };

  typedef IndexedStringMap<Prop, Slot> PropMap;
  typedef IndexedStringMap<Const, Slot> ConstMap;

  PreClassEmitter(UnitEmitter& ue, Id id, const std::string& name);
  ~PreClassEmitter();

  void init(int line1, int line2, Attr attrs,
            const StringData* parent, const StringData* docComment);

  UnitEmitter& ue() const { return m_ue; }
  const StringData* name() const { return m_name; }
  Attr attrs() const { return m_attrs; }
  void setAttrs(Attr attrs) { m_attrs = attrs; }
  void setEnumBaseTy(TypeConstraint ty) { m_enumBaseTy = ty; }
  const TypeConstraint& enumBaseTy() const { return m_enumBaseTy; }
  Id id() const { return m_id; }
  void setIfaceVtableSlot(Slot s) { m_ifaceVtableSlot = s; }
  const MethodVec& methods() const { return m_methods; }
  bool hasMethod(const StringData* name) const {
    return m_methodMap.find(name) != m_methodMap.end();
  }
  FuncEmitter* lookupMethod(const StringData* name) const {
    return folly::get_default(m_methodMap, name);
  }
  size_t numProperties() const { return m_propMap.size(); }
  bool hasProp(const StringData* name) const {
    return m_propMap.find(name) != m_propMap.end();
  }
  const PropMap::Builder& propMap() const { return m_propMap; }
  const ConstMap::Builder& constMap() const { return m_constMap; }
  const StringData* docComment() const { return m_docComment; }
  const StringData* parentName() const { return m_parent; }
  static bool IsAnonymousClassName(const std::string& name) {
    return name.find('$') != std::string::npos;
  }

  void setDocComment(const StringData* sd) { m_docComment = sd; }

  void addInterface(const StringData* n);
  const std::vector<LowStringPtr>& interfaces() const {
    return m_interfaces;
  }
  void addEnumInclude(const StringData* n);
  const std::vector<LowStringPtr>& enumIncludes() const {
    return m_enumIncludes;
  }
  bool addMethod(FuncEmitter* method);
  bool addProperty(const StringData* n,
                   Attr attrs,
                   const StringData* userType,
                   const TypeConstraint& typeConstraint,
                   const UpperBoundVec& ubs,
                   const StringData* docComment,
                   const TypedValue* val,
                   RepoAuthType,
                   UserAttributeMap);
  bool addConstant(const StringData* n,
                   const StringData* cls,
                   const TypedValue* val,
                   Array resolvedTypeStructure,
                   ConstModifiers::Kind kind,
                   Const::Invariance invariance,
                   bool fromTrait,
                   bool isAbstract);
  bool addContextConstant(const StringData* n,
                          Const::CoeffectsVec coeffects,
                          bool isAbstract,
                          bool fromTrait = false);
  bool addAbstractConstant(const StringData* n,
                           ConstModifiers::Kind kind =
                             ConstModifiers::Kind::Value,
                           bool fromTrait = false);
  void addUsedTrait(const StringData* traitName);
  void addClassRequirement(const PreClass::ClassRequirement req) {
    m_requirements.push_back(req);
  }
  const std::vector<LowStringPtr>& usedTraits() const {
    return m_usedTraits;
  }
  const std::vector<PreClass::ClassRequirement>& requirements() const {
    return m_requirements;
  }

  void setUserAttributes(UserAttributeMap map) {
    m_userAttributes = std::move(map);
  }
  UserAttributeMap userAttributes() const { return m_userAttributes; }

  PreClass* create(Unit& unit) const;

  template<class SerDe> void serdeMetaData(SerDe&);

  std::pair<int,int> getLocation() const {
    return std::make_pair(m_line1, m_line2);
  }

  bool areMemoizeCacheKeysAllocated() const {
    return m_memoizeInstanceSerial > 0;
  }
  int getNextMemoizeCacheKey() {
    return m_memoizeInstanceSerial++;
  }

 private:
  typedef hphp_hash_map<LowStringPtr,
                        FuncEmitter*,
                        string_data_hash,
                        string_data_isame> MethodMap;

  UnitEmitter& m_ue;
  int m_line1;
  int m_line2;
  LowStringPtr m_name;
  Attr m_attrs;
  LowStringPtr m_parent;
  LowStringPtr m_docComment;
  TypeConstraint m_enumBaseTy;
  Id m_id;
  Slot m_ifaceVtableSlot{kInvalidSlot};
  int m_memoizeInstanceSerial{0};

  std::vector<LowStringPtr> m_interfaces;
  std::vector<LowStringPtr> m_enumIncludes;
  std::vector<LowStringPtr> m_usedTraits;
  std::vector<PreClass::ClassRequirement> m_requirements;
  UserAttributeMap m_userAttributes;
  MethodVec m_methods;
  MethodMap m_methodMap;
  PropMap::Builder m_propMap;
  ConstMap::Builder m_constMap;
};

///////////////////////////////////////////////////////////////////////////////
}
