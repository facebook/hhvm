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

#ifndef incl_HPHP_VM_CLASS_EMIT_H_
#define incl_HPHP_VM_CLASS_EMIT_H_

#include "hphp/runtime/base/complex_types.h"
#include "hphp/runtime/vm/repo-helpers.h"

namespace HPHP {

class PreClassEmitter {
 public:
  typedef std::vector<FuncEmitter*> MethodVec;

  class Prop {
   public:
    Prop()
      : m_name(0)
      , m_mangledName(0)
      , m_attrs(AttrNone)
      , m_typeConstraint(0)
      , m_docComment(0)
      , m_hphpcType(KindOfInvalid)
    {}

    Prop(const PreClassEmitter* pce,
         const StringData* n,
         Attr attrs,
         const StringData* typeConstraint,
         const StringData* docComment,
         TypedValue* val,
         DataType hphpcType);
    ~Prop();

    const StringData* name() const { return m_name; }
    const StringData* mangledName() const { return m_mangledName; }
    Attr attrs() const { return m_attrs; }
    const StringData* typeConstraint() const { return m_typeConstraint; }
    const StringData* docComment() const { return m_docComment; }
    const TypedValue& val() const { return m_val; }
    DataType hphpcType() const { return m_hphpcType; }

    template<class SerDe> void serde(SerDe& sd) {
      sd(m_name)
        (m_mangledName)
        (m_attrs)
        (m_typeConstraint)
        (m_docComment)
        (m_val)
        (m_hphpcType)
        ;
    }

   private:
    const StringData* m_name;
    const StringData* m_mangledName;
    Attr m_attrs;
    const StringData* m_typeConstraint;
    const StringData* m_docComment;
    TypedValue m_val;
    DataType m_hphpcType;
  };

  class Const {
   public:
    Const()
      : m_name(0)
      , m_typeConstraint(0)
      , m_phpCode(0)
    {}
    Const(const StringData* n, const StringData* typeConstraint,
          TypedValue* val, const StringData* phpCode)
      : m_name(n), m_typeConstraint(typeConstraint), m_phpCode(phpCode) {
      memcpy(&m_val, val, sizeof(TypedValue));
    }
    ~Const() {}

    const StringData* name() const { return m_name; }
    const StringData* typeConstraint() const { return m_typeConstraint; }
    const TypedValue& val() const { return m_val; }
    const StringData* phpCode() const { return m_phpCode; }

    template<class SerDe> void serde(SerDe& sd) {
      sd(m_name)(m_val)(m_phpCode);
    }

   private:
    const StringData* m_name;
    const StringData* m_typeConstraint;
    TypedValue m_val;
    const StringData* m_phpCode;
  };

  PreClassEmitter(UnitEmitter& ue, Id id, const StringData* n,
                  PreClass::Hoistable hoistable);
  ~PreClassEmitter();

  void init(int line1, int line2, Offset offset, Attr attrs,
            const StringData* parent, const StringData* docComment);

  UnitEmitter& ue() const { return m_ue; }
  const StringData* name() const { return m_name; }
  Attr attrs() const { return m_attrs; }
  void setHoistable(PreClass::Hoistable h) { m_hoistable = h; }
  Id id() const { return m_id; }
  const MethodVec& methods() const { return m_methods; }

  void addInterface(const StringData* n);
  bool addMethod(FuncEmitter* method);
  bool addProperty(const StringData* n,
                   Attr attrs,
                   const StringData* typeConstraint,
                   const StringData* docComment,
                   TypedValue* val,
                   DataType hphpcType);
  const Prop& lookupProp(const StringData* propName) const;
  bool addConstant(const StringData* n, const StringData* typeConstraint,
                   TypedValue* val, const StringData* phpCode);
  void addUsedTrait(const StringData* traitName);
  void addTraitPrecRule(const PreClass::TraitPrecRule &rule);
  void addTraitAliasRule(const PreClass::TraitAliasRule &rule);
  void addUserAttribute(const StringData* name, TypedValue tv);
  void commit(RepoTxn& txn) const;

  void setBuiltinClassInfo(const ClassInfo* info,
                           BuiltinCtorFunction ctorFunc,
                           int sz);

  PreClass* create(Unit& unit) const;

  template<class SerDe> void serdeMetaData(SerDe&);

 private:
  typedef IndexedStringMap<Prop,true,Slot> PropMap;
  typedef IndexedStringMap<Const,true,Slot> ConstMap;
  typedef hphp_hash_map<const StringData*, FuncEmitter*, string_data_hash,
                        string_data_isame> MethodMap;

  UnitEmitter& m_ue;
  int m_line1;
  int m_line2;
  Offset m_offset;
  const StringData* m_name;
  Attr m_attrs;
  const StringData* m_parent;
  const StringData* m_docComment;
  Id m_id;
  PreClass::Hoistable m_hoistable;
  BuiltinCtorFunction m_InstanceCtor;
  int m_builtinPropSize;

  std::vector<const StringData*> m_interfaces;
  std::vector<const StringData*> m_usedTraits;
  std::vector<PreClass::TraitPrecRule> m_traitPrecRules;
  std::vector<PreClass::TraitAliasRule> m_traitAliasRules;
  PreClass::UserAttributeMap m_userAttributes;
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

#define PCRP_IOP(o) PCRP_OP(Insert##o, insert##o)
#define PCRP_GOP(o) PCRP_OP(Get##o, get##o)
#define PCRP_OPS \
  PCRP_IOP(PreClass) \
  PCRP_GOP(PreClasses)
  class InsertPreClassStmt : public RepoProxy::Stmt {
   public:
    InsertPreClassStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void insert(const PreClassEmitter& pce, RepoTxn& txn, int64_t unitSn,
                Id preClassId, const StringData* name,
                PreClass::Hoistable hoistable);
  };
  class GetPreClassesStmt : public RepoProxy::Stmt {
   public:
    GetPreClassesStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void get(UnitEmitter& ue);
  };
#define PCRP_OP(c, o) \
 public: \
  c##Stmt& o(int repoId) { return *m_##o[repoId]; } \
 private: \
  c##Stmt m_##o##Local; \
  c##Stmt m_##o##Central; \
  c##Stmt* m_##o[RepoIdCount];
  PCRP_OPS
#undef PCRP_OP
};

} // HPHP

#endif
