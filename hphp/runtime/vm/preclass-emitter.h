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

#ifndef incl_HPHP_VM_CLASS_EMIT_H_
#define incl_HPHP_VM_CLASS_EMIT_H_

#include "hphp/runtime/base/repo-auth-type.h"
#include "hphp/runtime/base/types.h"

#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/func-emitter.h"
#include "hphp/runtime/vm/repo-helpers.h"

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

class PreClassEmitter {
 public:
  typedef std::vector<FuncEmitter*> MethodVec;

  class Prop {
   public:
    Prop()
      : m_name(nullptr)
      , m_mangledName(nullptr)
      , m_attrs(AttrNone)
      , m_typeConstraint(nullptr)
      , m_docComment(nullptr)
      , m_repoAuthType{}
    {}

    Prop(const PreClassEmitter* pce,
         const StringData* n,
         Attr attrs,
         const StringData* typeConstraint,
         const StringData* docComment,
         const TypedValue* val,
         RepoAuthType repoAuthType);
    ~Prop();

    const StringData* name() const { return m_name; }
    const StringData* mangledName() const { return m_mangledName; }
    Attr attrs() const { return m_attrs; }
    const StringData* typeConstraint() const { return m_typeConstraint; }
    const StringData* docComment() const { return m_docComment; }
    const TypedValue& val() const { return m_val; }
    RepoAuthType repoAuthType() const { return m_repoAuthType; }

    template<class SerDe> void serde(SerDe& sd) {
      sd(m_name)
        (m_mangledName)
        (m_attrs)
        (m_typeConstraint)
        (m_docComment)
        (m_val)
        (m_repoAuthType)
        ;
    }

   private:
    LowStringPtr m_name;
    LowStringPtr m_mangledName;
    Attr m_attrs;
    LowStringPtr m_typeConstraint;
    LowStringPtr m_docComment;
    TypedValue m_val;
    RepoAuthType m_repoAuthType;
  };

  class Const {
   public:
    Const()
      : m_name(nullptr)
      , m_typeConstraint(nullptr)
      , m_val(make_tv<KindOfUninit>())
      , m_phpCode(nullptr)
      , m_typeconst(false)
    {}
    Const(const StringData* n, const StringData* typeConstraint,
          const TypedValue* val, const StringData* phpCode,
          const bool typeconst)
      : m_name(n), m_typeConstraint(typeConstraint), m_phpCode(phpCode),
        m_typeconst(typeconst) {
      if (!val) {
        m_val.clear();
      } else {
        m_val = *val;
      }
    }
    ~Const() {}

    const StringData* name() const { return m_name; }
    const StringData* typeConstraint() const { return m_typeConstraint; }
    const TypedValue& val() const { return m_val.value(); }
    const folly::Optional<TypedValue>& valOption() const { return m_val; }
    const StringData* phpCode() const { return m_phpCode; }
    bool isAbstract()       const { return !m_val.hasValue(); }
    bool isTypeconst() const { return m_typeconst; }

    template<class SerDe> void serde(SerDe& sd) {
      sd(m_name)
        (m_val)
        (m_phpCode)
        (m_typeconst);
    }

   private:
    LowStringPtr m_name;
    LowStringPtr m_typeConstraint;
    folly::Optional<TypedValue> m_val;
    LowStringPtr m_phpCode;
    bool m_typeconst;
  };

  typedef IndexedStringMap<Prop, true, Slot> PropMap;
  typedef IndexedStringMap<Const, true, Slot> ConstMap;

  PreClassEmitter(UnitEmitter& ue, Id id, const StringData* n,
                  PreClass::Hoistable hoistable);
  ~PreClassEmitter();

  void init(int line1, int line2, Offset offset, Attr attrs,
            const StringData* parent, const StringData* docComment);

  UnitEmitter& ue() const { return m_ue; }
  const StringData* name() const { return m_name; }
  Attr attrs() const { return m_attrs; }
  void setAttrs(Attr attrs) { m_attrs = attrs; }
  void setHoistable(PreClass::Hoistable h) { m_hoistable = h; }
  PreClass::Hoistable hoistability() const { return m_hoistable; }
  void setOffset(Offset off) { m_offset = off; }
  void setEnumBaseTy(TypeConstraint ty) { m_enumBaseTy = ty; }
  const TypeConstraint& enumBaseTy() const { return m_enumBaseTy; }
  Id id() const { return m_id; }
  int32_t numDeclMethods() const { return m_numDeclMethods; }
  void setNumDeclMethods(uint32_t n) { m_numDeclMethods = n; }
  void setIfaceVtableSlot(Slot s) { m_ifaceVtableSlot = s; }
  const MethodVec& methods() const { return m_methods; }
  FuncEmitter* findMethod(const StringData* name) { return m_methodMap[name]; }
  const PropMap::Builder& propMap() const { return m_propMap; }
  const ConstMap::Builder& constMap() const { return m_constMap; }
  const StringData* docComment() const { return m_docComment; }
  const StringData* parentName() const { return m_parent; }

  void addInterface(const StringData* n);
  const std::vector<LowStringPtr>& interfaces() const {
    return m_interfaces;
  }
  bool addMethod(FuncEmitter* method);
  void renameMethod(const StringData* oldName, const StringData *newName);
  bool addProperty(const StringData* n,
                   Attr attrs,
                   const StringData* typeConstraint,
                   const StringData* docComment,
                   const TypedValue* val,
                   RepoAuthType);
  const Prop& lookupProp(const StringData* propName) const;
  bool addConstant(const StringData* n, const StringData* typeConstraint,
                   const TypedValue* val, const StringData* phpCode,
                   const bool typeConst);
  bool addAbstractConstant(const StringData* n,
                           const StringData* typeConstraint,
                           const bool typeConst);
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

  void addUserAttribute(const StringData* name, TypedValue tv);
  void setUserAttributes(UserAttributeMap map) {
    m_userAttributes = std::move(map);
  }
  UserAttributeMap userAttributes() const { return m_userAttributes; }

  void commit(RepoTxn& txn) const;

  void setBuiltinClassInfo(const ClassInfo* info,
                           BuiltinCtorFunction ctorFunc,
                           BuiltinDtorFunction dtorFunc,
                           BuiltinObjExtents extents);

  PreClass* create(Unit& unit) const;

  template<class SerDe> void serdeMetaData(SerDe&);

  std::pair<int,int> getLocation() const {
    return std::make_pair(m_line1, m_line2);
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
  Offset m_offset;  // offset of the DefCls for this class
  LowStringPtr m_name;
  Attr m_attrs;
  LowStringPtr m_parent;
  LowStringPtr m_docComment;
  TypeConstraint m_enumBaseTy;
  Id m_id;
  PreClass::Hoistable m_hoistable;
  BuiltinCtorFunction m_instanceCtor{nullptr};
  BuiltinDtorFunction m_instanceDtor{nullptr};
  uint32_t m_builtinObjSize{0};
  int32_t m_builtinODOffset{0};
  int32_t m_numDeclMethods{-1};
  Slot m_ifaceVtableSlot{kInvalidSlot};
  int m_memoizeInstanceSerial{0};

  std::vector<LowStringPtr> m_interfaces;
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

class PreClassRepoProxy : public RepoProxy {
  friend class PreClass;
  friend class PreClassEmitter;
 public:
  explicit PreClassRepoProxy(Repo& repo);
  ~PreClassRepoProxy();
  void createSchema(int repoId, RepoTxn& txn);

  struct InsertPreClassStmt : public RepoProxy::Stmt {
    InsertPreClassStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void insert(const PreClassEmitter& pce, RepoTxn& txn, int64_t unitSn,
                Id preClassId, const StringData* name,
                PreClass::Hoistable hoistable);
  };

  struct GetPreClassesStmt : public RepoProxy::Stmt {
    GetPreClassesStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void get(UnitEmitter& ue);
  };

  InsertPreClassStmt insertPreClass[RepoIdCount];
  GetPreClassesStmt getPreClasses[RepoIdCount];
};

///////////////////////////////////////////////////////////////////////////////
}

#endif
