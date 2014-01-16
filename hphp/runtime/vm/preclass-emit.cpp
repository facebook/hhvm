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
#include "hphp/runtime/vm/preclass-emit.h"

#include <limits>

#include "folly/Memory.h"

#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/blob-helper.h"
#include "hphp/runtime/vm/native.h"

namespace HPHP {

//=============================================================================
// PreClassEmitter::Prop.

PreClassEmitter::Prop::Prop(const PreClassEmitter* pce,
                            const StringData* n,
                            Attr attrs,
                            const StringData* typeConstraint,
                            const StringData* docComment,
                            const TypedValue* val,
                            DataType hphpcType)
  : m_name(n)
  , m_attrs(attrs)
  , m_typeConstraint(typeConstraint)
  , m_docComment(docComment)
  , m_hphpcType(hphpcType)
{
  m_mangledName = PreClass::manglePropName(pce->name(), n, attrs);
  memcpy(&m_val, val, sizeof(TypedValue));
}

PreClassEmitter::Prop::~Prop() {
}

//=============================================================================
// PreClassEmitter.

PreClassEmitter::PreClassEmitter(UnitEmitter& ue,
                                 Id id,
                                 const StringData* n,
                                 PreClass::Hoistable hoistable)
  : m_ue(ue)
  , m_name(n)
  , m_id(id)
  , m_hoistable(hoistable)
{}

void PreClassEmitter::init(int line1, int line2, Offset offset, Attr attrs,
                           const StringData* parent,
                           const StringData* docComment) {
  m_line1 = line1;
  m_line2 = line2;
  m_offset = offset;
  m_attrs = attrs;
  m_parent = parent;
  m_docComment = docComment;
  if (!SystemLib::s_inited) {
    m_attrs = m_attrs | AttrBuiltin;
  }
}

PreClassEmitter::~PreClassEmitter() {
  for (MethodVec::const_iterator it = m_methods.begin();
       it != m_methods.end(); ++it) {
    delete *it;
  }
}

void PreClassEmitter::addInterface(const StringData* n) {
  m_interfaces.push_back(n);
}

bool PreClassEmitter::addMethod(FuncEmitter* method) {
  MethodMap::const_iterator it = m_methodMap.find(method->name());
  if (it != m_methodMap.end()) {
    return false;
  }
  m_methods.push_back(method);
  m_methodMap[method->name()] = method;
  return true;
}

bool PreClassEmitter::addProperty(const StringData* n, Attr attrs,
                                  const StringData* typeConstraint,
                                  const StringData* docComment,
                                  const TypedValue* val,
                                  DataType hphpcType) {
  PropMap::Builder::const_iterator it = m_propMap.find(n);
  if (it != m_propMap.end()) {
    return false;
  }
  PreClassEmitter::Prop prop(this, n, attrs, typeConstraint, docComment, val,
    hphpcType);
  m_propMap.add(prop.name(), prop);
  return true;
}

const PreClassEmitter::Prop&
PreClassEmitter::lookupProp(const StringData* propName) const {
  PropMap::Builder::const_iterator it = m_propMap.find(propName);
  assert(it != m_propMap.end());
  Slot idx = it->second;
  return m_propMap[idx];
}

bool PreClassEmitter::addConstant(const StringData* n,
                                  const StringData* typeConstraint,
                                  const TypedValue* val,
                                  const StringData* phpCode) {
  ConstMap::Builder::const_iterator it = m_constMap.find(n);
  if (it != m_constMap.end()) {
    return false;
  }
  PreClassEmitter::Const const_(n, typeConstraint, val, phpCode);
  m_constMap.add(const_.name(), const_);
  return true;
}

void PreClassEmitter::addUsedTrait(const StringData* traitName) {
  m_usedTraits.push_back(traitName);
}

void PreClassEmitter::addTraitPrecRule(
    const PreClass::TraitPrecRule &rule) {
  m_traitPrecRules.push_back(rule);
}

void PreClassEmitter::addTraitAliasRule(
    const PreClass::TraitAliasRule &rule) {
  m_traitAliasRules.push_back(rule);
}

void PreClassEmitter::addUserAttribute(const StringData* name, TypedValue tv) {
  m_userAttributes[name] = tv;
}

void PreClassEmitter::commit(RepoTxn& txn) const {
  Repo& repo = Repo::get();
  PreClassRepoProxy& pcrp = repo.pcrp();
  int repoId = m_ue.repoId();
  int64_t usn = m_ue.sn();
  pcrp.insertPreClass(repoId)
      .insert(*this, txn, usn, m_id, m_name, m_hoistable);

  for (MethodVec::const_iterator it = m_methods.begin();
       it != m_methods.end(); ++it) {
    (*it)->commit(txn);
  }
}

void PreClassEmitter::setBuiltinClassInfo(const ClassInfo* info,
                                          BuiltinCtorFunction ctorFunc,
                                          BuiltinDtorFunction dtorFunc,
                                          BuiltinObjExtents extents) {
  if (info->getAttribute() & ClassInfo::IsFinal) {
    m_attrs = m_attrs | AttrFinal;
  }
  if (info->getAttribute() & ClassInfo::IsAbstract) {
    m_attrs = m_attrs | AttrAbstract;
  }
  if (info->getAttribute() & ClassInfo::IsTrait) {
    m_attrs = m_attrs | AttrTrait;
  }
  m_attrs = m_attrs | AttrUnique;
  m_instanceCtor = ctorFunc;
  m_instanceDtor = dtorFunc;

  assert(extents.totalSizeBytes <= std::numeric_limits<uint32_t>::max());
  assert(extents.odOffsetBytes  <= std::numeric_limits<int32_t>::max());
  m_builtinObjSize  = extents.totalSizeBytes - sizeof(ObjectData);
  m_builtinODOffset = extents.odOffsetBytes;
}

PreClass* PreClassEmitter::create(Unit& unit) const {
  Attr attrs = m_attrs;
  if (attrs & AttrPersistent &&
      !RuntimeOption::RepoAuthoritative && SystemLib::s_inited) {
    attrs = Attr(attrs & ~AttrPersistent);
  }

  auto pc = folly::make_unique<PreClass>(
    &unit, m_line1, m_line2, m_offset, m_name,
    attrs, m_parent, m_docComment, m_id,
    m_hoistable);
  pc->m_instanceCtor = m_instanceCtor;
  pc->m_instanceDtor = m_instanceDtor;
  pc->m_builtinObjSize = m_builtinObjSize;
  pc->m_builtinODOffset = m_builtinODOffset;
  pc->m_interfaces = m_interfaces;
  pc->m_usedTraits = m_usedTraits;
  pc->m_traitRequirements = m_traitRequirements;
  pc->m_traitPrecRules = m_traitPrecRules;
  pc->m_traitAliasRules = m_traitAliasRules;
  pc->m_userAttributes = m_userAttributes;

  PreClass::MethodMap::Builder methodBuild;
  for (MethodVec::const_iterator it = m_methods.begin();
       it != m_methods.end(); ++it) {
    Func* f = (*it)->create(unit, pc.get());
    methodBuild.add(f->name(), f);
  }
  pc->m_methods.create(methodBuild);

  PreClass::PropMap::Builder propBuild;
  for (unsigned i = 0; i < m_propMap.size(); ++i) {
    const Prop& prop = m_propMap[i];
    propBuild.add(prop.name(), PreClass::Prop(pc.get(),
                                              prop.name(),
                                              prop.attrs(),
                                              prop.typeConstraint(),
                                              prop.docComment(),
                                              prop.val(),
                                              prop.hphpcType()));
  }
  pc->m_properties.create(propBuild);

  PreClass::ConstMap::Builder constBuild;
  for (unsigned i = 0; i < m_constMap.size(); ++i) {
    const Const& const_ = m_constMap[i];
    constBuild.add(const_.name(), PreClass::Const(pc.get(),
                                                  const_.name(),
                                                  const_.typeConstraint(),
                                                  const_.val(),
                                                  const_.phpCode()));
  }
  if (auto nativeConsts = Native::getClassConstants(m_name)) {
    for (auto cnsMap : *nativeConsts) {
      auto tv = cnsMap.second;
      constBuild.add(cnsMap.first, PreClass::Const(pc.get(),
                                                   cnsMap.first,
                                                   empty_string.get(),
                                                   tv,
                                                   empty_string.get()));
    }
  }

  pc->m_constants.create(constBuild);
  return pc.release();
}

template<class SerDe> void PreClassEmitter::serdeMetaData(SerDe& sd) {
  // NOTE: name, hoistable, and a few other fields currently
  // serialized outside of this.
  sd(m_line1)
    (m_line2)
    (m_offset)
    (m_attrs)
    (m_parent)
    (m_docComment)

    (m_interfaces)
    (m_usedTraits)
    (m_traitRequirements)
    (m_traitPrecRules)
    (m_traitAliasRules)
    (m_userAttributes)
    (m_propMap)
    (m_constMap)
    ;
}

//=============================================================================
// PreClassRepoProxy.

PreClassRepoProxy::PreClassRepoProxy(Repo& repo)
  : RepoProxy(repo)
#define PCRP_OP(c, o) \
  , m_##o##Local(repo, RepoIdLocal), m_##o##Central(repo, RepoIdCentral)
    PCRP_OPS
#undef PCRP_OP
{
#define PCRP_OP(c, o) \
  m_##o[RepoIdLocal] = &m_##o##Local; \
  m_##o[RepoIdCentral] = &m_##o##Central;
  PCRP_OPS
#undef PCRP_OP
}

PreClassRepoProxy::~PreClassRepoProxy() {
}

void PreClassRepoProxy::createSchema(int repoId, RepoTxn& txn) {
  {
    std::stringstream ssCreate;
    ssCreate << "CREATE TABLE " << m_repo.table(repoId, "PreClass")
             << "(unitSn INTEGER, preClassId INTEGER, name TEXT,"
                " hoistable INTEGER, extraData BLOB,"
                " PRIMARY KEY (unitSn, preClassId));";
    txn.exec(ssCreate.str());
  }
}

void PreClassRepoProxy::InsertPreClassStmt
                      ::insert(const PreClassEmitter& pce, RepoTxn& txn,
                               int64_t unitSn, Id preClassId,
                               const StringData* name,
                               PreClass::Hoistable hoistable) {
  if (!prepared()) {
    std::stringstream ssInsert;
    ssInsert << "INSERT INTO " << m_repo.table(m_repoId, "PreClass")
             << " VALUES(@unitSn, @preClassId, @name, @hoistable, "
                "@extraData);";
    txn.prepare(*this, ssInsert.str());
  }

  BlobEncoder extraBlob;
  RepoTxnQuery query(txn, *this);
  query.bindInt64("@unitSn", unitSn);
  query.bindId("@preClassId", preClassId);
  query.bindStaticString("@name", name);
  query.bindInt("@hoistable", hoistable);
  const_cast<PreClassEmitter&>(pce).serdeMetaData(extraBlob);
  query.bindBlob("@extraData", extraBlob, /* static */ true);
  query.exec();
}

void PreClassRepoProxy::GetPreClassesStmt
                      ::get(UnitEmitter& ue) {
  RepoTxn txn(m_repo);
  if (!prepared()) {
    std::stringstream ssSelect;
    ssSelect << "SELECT preClassId,name,hoistable,extraData FROM "
             << m_repo.table(m_repoId, "PreClass")
             << " WHERE unitSn == @unitSn ORDER BY preClassId ASC;";
    txn.prepare(*this, ssSelect.str());
  }
  RepoTxnQuery query(txn, *this);
  query.bindInt64("@unitSn", ue.sn());
  do {
    query.step();
    if (query.row()) {
      Id preClassId;          /**/ query.getId(0, preClassId);
      StringData* name;       /**/ query.getStaticString(1, name);
      int hoistable;          /**/ query.getInt(2, hoistable);
      BlobDecoder extraBlob = /**/ query.getBlob(3);
      PreClassEmitter* pce = ue.newPreClassEmitter(
        name, (PreClass::Hoistable)hoistable);
      pce->serdeMetaData(extraBlob);
      if (!SystemLib::s_inited) {
        assert(pce->attrs() & AttrPersistent);
        assert(pce->attrs() & AttrUnique);
      }
      assert(pce->id() == preClassId);
    }
  } while (!query.done());
  txn.commit();
}

} // HPHP
