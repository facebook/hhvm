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

std::string NewAnonymousClassName(folly::StringPiece name);

folly::StringPiece StripIdFromAnonymousClassName(folly::StringPiece name);

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
    friend PreClassEmitter;
    void updateAfterDeserialize(const PreClassEmitter* pce) {
      m_repoAuthType.resolveArray(pce->ue());
    }

  private:
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

    Const()
      : m_name(nullptr)
      , m_typeConstraint(nullptr)
      , m_val(make_tv<KindOfUninit>())
      , m_phpCode(nullptr)
      , m_coeffects({})
      , m_kind(ConstModifiers::Kind::Value)
      , m_isAbstract(false)
      , m_fromTrait(false)
    {}
    Const(const StringData* n, const StringData* typeConstraint,
          const TypedValue* val, const StringData* phpCode,
          const CoeffectsVec&& coeffects, const ConstModifiers::Kind kind,
          const bool isAbstract, const bool fromTrait)
      : m_name(n)
      , m_typeConstraint(typeConstraint)
      , m_phpCode(phpCode)
      , m_coeffects(coeffects)
      , m_kind(kind)
      , m_isAbstract(isAbstract)
      , m_fromTrait(fromTrait) {
      if (!val) {
        m_val.reset();
      } else {
        m_val = *val;
      }
    }
    ~Const() {}

    const StringData* name() const { return m_name; }
    const StringData* typeConstraint() const { return m_typeConstraint; }
    const TypedValue& val() const { return m_val.value(); }
    const Optional<TypedValue>& valOption() const { return m_val; }
    const StringData* phpCode() const { return m_phpCode; }
    bool isAbstract() const { return m_isAbstract; }
    const CoeffectsVec& coeffects() const { return m_coeffects; }
    ConstModifiers::Kind kind() const { return m_kind; }
    bool isFromTrait() const { return m_fromTrait; }

    template<class SerDe> void serde(SerDe& sd) {
      sd(m_name)
        (m_val)
        (m_phpCode)
        (m_coeffects)
        (m_kind)
        (m_isAbstract)
        (m_fromTrait);
    }

   private:
    LowStringPtr m_name;
    LowStringPtr m_typeConstraint;
    Optional<TypedValue> m_val;
    LowStringPtr m_phpCode;
    CoeffectsVec m_coeffects;
    ConstModifiers::Kind m_kind;
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
  bool useGlobalIds() const { return m_ue.useGlobalIds(); }
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
  void renameMethod(const StringData* oldName, const StringData *newName);
  bool addProperty(const StringData* n,
                   Attr attrs,
                   const StringData* userType,
                   const TypeConstraint& typeConstraint,
                   const UpperBoundVec& ubs,
                   const StringData* docComment,
                   const TypedValue* val,
                   RepoAuthType,
                   UserAttributeMap);
  bool addConstant(const StringData* n, const StringData* typeConstraint,
                   const TypedValue* val, const StringData* phpCode,
                   const ConstModifiers::Kind kind =
                    ConstModifiers::Kind::Value,
                   const bool fromTrait = false,
                   const Array& typeStructure = Array{},
                   const bool isAbstract = false);
  bool addContextConstant(const StringData* n,
                          Const::CoeffectsVec&& coeffects,
                          const bool isAbstract, const bool fromTrait = false);
  bool addAbstractConstant(const StringData* n,
                           const StringData* typeConstraint,
                           const ConstModifiers::Kind kind =
                             ConstModifiers::Kind::Value,
                           const bool fromTrait = false);
  void addUsedTrait(const StringData* traitName);
  void addClassRequirement(const PreClass::ClassRequirement req) {
    m_requirements.push_back(req);
  }
  void addTraitPrecRule(const PreClass::TraitPrecRule &rule);
  void addTraitAliasRule(const PreClass::TraitAliasRule &rule);
  const std::vector<LowStringPtr>& usedTraits() const {
    return m_usedTraits;
  }
  const std::vector<PreClass::ClassRequirement>& requirements() const {
    return m_requirements;
  }
  const std::vector<PreClass::TraitAliasRule>& traitAliasRules() const {
    return m_traitAliasRules;
  }
  const std::vector<PreClass::TraitPrecRule>& traitPrecRules() const {
    return m_traitPrecRules;
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
  std::vector<PreClass::TraitPrecRule> m_traitPrecRules;
  std::vector<PreClass::TraitAliasRule> m_traitAliasRules;
  UserAttributeMap m_userAttributes;
  MethodVec m_methods;
  MethodMap m_methodMap;
  PropMap::Builder m_propMap;
  ConstMap::Builder m_constMap;
};

///////////////////////////////////////////////////////////////////////////////
}
