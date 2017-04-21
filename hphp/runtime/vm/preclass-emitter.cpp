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
#include "hphp/runtime/vm/preclass-emitter.h"

#include <limits>

#include <folly/Memory.h>

#include "hphp/parser/parser.h"

#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/blob-helper.h"
#include "hphp/runtime/vm/native.h"
#include "hphp/runtime/vm/native-data.h"

namespace HPHP {

namespace {

const StringData* preClassName(const std::string& name) {
  static std::atomic<uint32_t> next_anon_class;
  if (ParserBase::IsAnonymousClassName(name)) {
    if (name.find(';') == std::string::npos) {
      return makeStaticString(
        folly::sformat("{};{}", name, next_anon_class.fetch_add(1)));
    }
  }
  return makeStaticString(name);
}

}

//=============================================================================
// PreClassEmitter::Prop.

PreClassEmitter::Prop::Prop(const PreClassEmitter* pce,
                            const StringData* n,
                            Attr attrs,
                            const StringData* typeConstraint,
                            const StringData* docComment,
                            const TypedValue* val,
                            RepoAuthType repoAuthType)
  : m_name(n)
  , m_attrs(attrs)
  , m_typeConstraint(typeConstraint)
  , m_docComment(docComment)
  , m_repoAuthType(repoAuthType)
{
  m_mangledName = PreClass::manglePropName(pce->name(), n, attrs);
  memcpy(&m_val, val, sizeof(TypedValue));
}

PreClassEmitter::Prop::~Prop() {
}

//=============================================================================
// PreClassEmitter.

extern const StaticString s_Closure;

PreClassEmitter::PreClassEmitter(UnitEmitter& ue,
                                 Id id,
                                 const std::string& n,
                                 PreClass::Hoistable hoistable)
  : m_ue(ue)
  , m_name(preClassName(n))
  , m_id(id)
  , m_hoistable(hoistable) {
  if (m_name->isame(s_Closure.get())) {
    setClosurePreClass();
  }
}

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
  MethodMap::const_iterator it = m_methodMap.find(method->name);
  if (it != m_methodMap.end()) {
    return false;
  }
  m_methods.push_back(method);
  m_methodMap[method->name] = method;
  return true;
}

void PreClassEmitter::renameMethod(const StringData* oldName,
                                   const StringData* newName) {
  MethodMap::const_iterator it = m_methodMap.find(oldName);
  assert(it != m_methodMap.end());
  it->second->name = newName;
  m_methodMap[newName] = it->second;
  m_methodMap.erase(oldName);
}

bool PreClassEmitter::addProperty(const StringData* n, Attr attrs,
                                  const StringData* typeConstraint,
                                  const StringData* docComment,
                                  const TypedValue* val,
                                  RepoAuthType repoAuthType) {
  PropMap::Builder::const_iterator it = m_propMap.find(n);
  if (it != m_propMap.end()) {
    return false;
  }
  PreClassEmitter::Prop prop(this, n, attrs, typeConstraint, docComment, val,
    repoAuthType);
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

bool PreClassEmitter::addAbstractConstant(const StringData* n,
                                          const StringData* typeConstraint,
                                          const bool typeconst) {
  auto it = m_constMap.find(n);
  if (it != m_constMap.end()) {
    return false;
  }
  PreClassEmitter::Const cns(n, typeConstraint, nullptr, nullptr, typeconst);
  m_constMap.add(cns.name(), cns);
  return true;
}

bool PreClassEmitter::addConstant(const StringData* n,
                                  const StringData* typeConstraint,
                                  const TypedValue* val,
                                  const StringData* phpCode,
                                  const bool typeconst,
                                  const Array& typeStructure) {
  ConstMap::Builder::const_iterator it = m_constMap.find(n);
  if (it != m_constMap.end()) {
    return false;
  }
  TypedValue tvVal;
  if (typeconst && !typeStructure.empty())  {
    tvVal = make_tv<KindOfPersistentArray>(typeStructure.get());
    assert(tvIsPlausible(tvVal));
  } else {
    tvVal = *val;
  }
  PreClassEmitter::Const cns(n, typeConstraint, &tvVal, phpCode, typeconst);
  m_constMap.add(cns.name(), cns);
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
  int repoId = m_ue.m_repoId;
  int64_t usn = m_ue.m_sn;
  pcrp.insertPreClass[repoId]
      .insert(*this, txn, usn, m_id, m_name, m_hoistable);

  for (MethodVec::const_iterator it = m_methods.begin();
       it != m_methods.end(); ++it) {
    (*it)->commit(txn);
  }
}

const StaticString s_nativedata("__nativedata");

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
  pc->m_interfaces = m_interfaces;
  pc->m_usedTraits = m_usedTraits;
  pc->m_requirements = m_requirements;
  pc->m_traitPrecRules = m_traitPrecRules;
  pc->m_traitAliasRules = m_traitAliasRules;
  pc->m_enumBaseTy = m_enumBaseTy;
  pc->m_numDeclMethods = m_numDeclMethods;
  pc->m_ifaceVtableSlot = m_ifaceVtableSlot;

  // Set user attributes.
  [&] {
    pc->m_userAttributes = m_userAttributes;
    pc->m_nativeDataInfo = nullptr;
    if (!m_userAttributes.size()) return;

    // Check for <<__NativeData("Type")>>.
    auto it = m_userAttributes.find(s_nativedata.get());
    if (it == m_userAttributes.end()) return;

    TypedValue ndiInfo = it->second;
    if (!isArrayType(ndiInfo.m_type)) return;

    // Use the first string label which references a registered type.  In
    // practice, there should generally only be one item and it should be a
    // string, but maybe that'll be extended...
    for (ArrayIter it(ndiInfo.m_data.parr); it; ++it) {
      Variant val = it.second();
      if (!val.isString()) continue;

      pc->m_nativeDataInfo = Native::getNativeDataInfo(val.toString().get());
      if (pc->m_nativeDataInfo) break;
    }
  }();

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
                                              prop.repoAuthType()));
  }
  pc->m_properties.create(propBuild);

  PreClass::ConstMap::Builder constBuild;
  for (unsigned i = 0; i < m_constMap.size(); ++i) {
    const Const& const_ = m_constMap[i];
    TypedValueAux tvaux;
    if (const_.isAbstract()) {
      tvWriteUninit(&tvaux);
      tvaux.constModifiers().isAbstract = true;
    } else {
      tvCopy(const_.val(), tvaux);
      tvaux.constModifiers().isAbstract = false;
    }

    tvaux.constModifiers().isType = const_.isTypeconst();

    constBuild.add(const_.name(), PreClass::Const(const_.name(),
                                                  tvaux,
                                                  const_.phpCode()));
  }
  if (auto nativeConsts = Native::getClassConstants(m_name)) {
    for (auto cnsMap : *nativeConsts) {
      TypedValueAux tvaux;
      tvCopy(cnsMap.second, tvaux);
      tvaux.constModifiers() = { false, false };
      constBuild.add(cnsMap.first, PreClass::Const(cnsMap.first,
                                                   tvaux,
                                                   staticEmptyString()));
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
    (m_numDeclMethods)
    (m_ifaceVtableSlot)

    (m_interfaces)
    (m_usedTraits)
    (m_requirements)
    (m_traitPrecRules)
    (m_traitAliasRules)
    (m_userAttributes)
    (m_propMap)
    (m_constMap)
    (m_enumBaseTy)
    ;
}

//=============================================================================
// PreClassRepoProxy.

PreClassRepoProxy::PreClassRepoProxy(Repo& repo)
    : RepoProxy(repo),
      insertPreClass{InsertPreClassStmt(repo, 0), InsertPreClassStmt(repo, 1)},
      getPreClasses{GetPreClassesStmt(repo, 0), GetPreClassesStmt(repo, 1)}
{}

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

  auto n = name->slice();
  auto const pos = RuntimeOption::RepoAuthoritative ?
    std::string::npos : qfind(n, ';');
  auto const nm = pos == std::string::npos ?
    n : folly::StringPiece{n.data(), pos};
  BlobEncoder extraBlob;
  RepoTxnQuery query(txn, *this);
  query.bindInt64("@unitSn", unitSn);
  query.bindId("@preClassId", preClassId);
  query.bindStringPiece("@name", nm);
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
  query.bindInt64("@unitSn", ue.m_sn);
  do {
    query.step();
    if (query.row()) {
      Id preClassId;          /**/ query.getId(0, preClassId);
      std::string name;       /**/ query.getStdString(1, name);
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
