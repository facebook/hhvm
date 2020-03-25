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

#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/repo-autoload-map-builder.h"
#include "hphp/runtime/vm/blob-helper.h"
#include "hphp/runtime/vm/native.h"
#include "hphp/runtime/vm/native-data.h"

namespace HPHP {

namespace {

/*
 * Important: We rely on generating unique anonymous class names (ie
 * Closures) by tacking ";<next_anon_class>" onto the end of the name.
 *
 * Its important that code that creates new closures goes through
 * preClassName, or NewAnonymousClassName to make sure this works.
 */
static std::atomic<uint32_t> next_anon_class{};

const StringData* preClassName(const std::string& name) {
  if (PreClassEmitter::IsAnonymousClassName(name)) {
    auto const pos = name.find(';');
    if (pos == std::string::npos) {
      return makeStaticString(NewAnonymousClassName(name));
    }
    auto const id = strtol(name.c_str() + pos + 1, nullptr, 10);
    if (id > 0 && id < INT_MAX) {
      auto next = next_anon_class.load(std::memory_order_relaxed);
      while (id >= next &&
             next_anon_class.compare_exchange_weak(
               next, id + 1, std::memory_order_relaxed
             )) {
        // nothing to do; just try again.
      }
    }
  }
  return makeStaticString(name);
}

}

std::string NewAnonymousClassName(folly::StringPiece name) {
  return folly::sformat("{};{}", name, next_anon_class.fetch_add(1));
}

//=============================================================================
// PreClassEmitter::Prop.

PreClassEmitter::Prop::Prop(const PreClassEmitter* pce,
                            const StringData* n,
                            Attr attrs,
                            const StringData* userType,
                            const TypeConstraint& typeConstraint,
                            const StringData* docComment,
                            const TypedValue* val,
                            RepoAuthType repoAuthType,
                            UserAttributeMap userAttributes)
  : m_name(n)
  , m_attrs(attrs)
  , m_userType(userType)
  , m_docComment(docComment)
  , m_repoAuthType(repoAuthType)
  , m_typeConstraint(typeConstraint)
  , m_userAttributes(userAttributes)
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
                                 const std::string& n,
                                 PreClass::Hoistable hoistable)
  : m_ue(ue)
  , m_name(preClassName(n))
  , m_id(id)
  , m_hoistable(hoistable) {}

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
  assertx(m_methodMap.count(oldName));
  auto it = m_methodMap.find(oldName);
  auto fe = it->second;
  m_methodMap.erase(it);
  fe->name = newName;
  m_methodMap[newName] = fe;
}

bool PreClassEmitter::addProperty(const StringData* n, Attr attrs,
                                  const StringData* userType,
                                  const TypeConstraint& typeConstraint,
                                  const StringData* docComment,
                                  const TypedValue* val,
                                  RepoAuthType repoAuthType,
                                  UserAttributeMap userAttributes) {
  assertx(typeConstraint.validForProp());
  PropMap::Builder::const_iterator it = m_propMap.find(n);
  if (it != m_propMap.end()) {
    return false;
  }
  PreClassEmitter::Prop prop{
    this,
    n,
    attrs,
    userType,
    typeConstraint,
    docComment,
    val,
    repoAuthType,
    userAttributes
  };
  m_propMap.add(prop.name(), prop);
  return true;
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
    assertx(typeStructure.isDictOrDArray());
    tvVal = make_persistent_array_like_tv(typeStructure.get());
    assertx(tvIsPlausible(tvVal));
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

  assertx(attrs & AttrPersistent || SystemLib::s_inited);

  auto pc = std::make_unique<PreClass>(
    &unit, m_line1, m_line2, m_offset, m_name,
    attrs, m_parent, m_docComment, m_id,
    m_hoistable);
  pc->m_interfaces = m_interfaces;
  pc->m_usedTraits = m_usedTraits;
  pc->m_requirements = m_requirements;
  pc->m_traitPrecRules = m_traitPrecRules;
  pc->m_traitAliasRules = m_traitAliasRules;
  pc->m_enumBaseTy = m_enumBaseTy;
  pc->m_numDeclMethods = -1;
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
    if (!isArrayLikeType(ndiInfo.m_type)) return;

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
    if (f->attrs() & AttrTrait) {
      if (pc->m_numDeclMethods == -1) {
        pc->m_numDeclMethods = it - m_methods.begin();
      }
    } else if (!f->isGenerated()) {
      assertx(pc->m_numDeclMethods == -1);
    }
    methodBuild.add(f->name(), f);
  }
  pc->m_methods.create(methodBuild);

  PreClass::PropMap::Builder propBuild;
  for (unsigned i = 0; i < m_propMap.size(); ++i) {
    const Prop& prop = m_propMap[i];
    propBuild.add(prop.name(), PreClass::Prop(pc.get(),
                                              prop.name(),
                                              prop.attrs(),
                                              prop.userType(),
                                              prop.typeConstraint(),
                                              prop.docComment(),
                                              prop.val(),
                                              prop.repoAuthType(),
                                              prop.userAttributes()));
  }
  pc->m_properties.create(propBuild);

  PreClass::ConstMap::Builder constBuild;
  for (unsigned i = 0; i < m_constMap.size(); ++i) {
    const Const& const_ = m_constMap[i];
    TypedValueAux tvaux;
    tvaux.constModifiers() = {};
    if (const_.isAbstract()) {
      tvWriteUninit(tvaux);
      tvaux.constModifiers().setIsAbstract(true);
    } else {
      tvCopy(const_.val(), tvaux);
      tvaux.constModifiers().setIsAbstract(false);
    }

    tvaux.constModifiers().setIsType(const_.isTypeconst());

    constBuild.add(const_.name(), PreClass::Const(const_.name(),
                                                  tvaux,
                                                  const_.phpCode()));
  }
  if (auto nativeConsts = Native::getClassConstants(m_name)) {
    for (auto cnsMap : *nativeConsts) {
      TypedValueAux tvaux;
      tvCopy(cnsMap.second, tvaux);
      tvaux.constModifiers() = {};
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
    (m_ifaceVtableSlot)

    (m_interfaces)
    (m_usedTraits)
    (m_requirements)
    (m_traitPrecRules)
    (m_traitAliasRules)
    (m_userAttributes)
    (m_propMap, [](Prop p) { return p.name(); })
    (m_constMap, [](Const c) { return c.name(); })
    (m_enumBaseTy)
    ;

    if (SerDe::deserializing) {
      for (unsigned i = 0; i < m_propMap.size(); ++i) {
        m_propMap[i].updateAfterDeserialize(this);
      }
    }
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
    auto createQuery = folly::sformat(
      "CREATE TABLE {} "
      "(unitSn INTEGER, preClassId INTEGER, name TEXT, hoistable INTEGER, "
      " extraData BLOB, PRIMARY KEY (unitSn, preClassId));",
      m_repo.table(repoId, "PreClass"));
    txn.exec(createQuery);
  }
}

void PreClassRepoProxy::InsertPreClassStmt
                      ::insert(const PreClassEmitter& pce, RepoTxn& txn,
                               int64_t unitSn, Id preClassId,
                               const StringData* name,
                               PreClass::Hoistable hoistable) {
  if (!prepared()) {
    auto insertQuery = folly::sformat(
      "INSERT INTO {} "
      "VALUES(@unitSn, @preClassId, @name, @hoistable, @extraData);",
      m_repo.table(m_repoId, "PreClass"));
    txn.prepare(*this, insertQuery);
  }

  auto n = name->slice();
  auto const pos = RuntimeOption::RepoAuthoritative ?
    std::string::npos : qfind(n, ';');
  auto const nm = pos == std::string::npos ?
    n : folly::StringPiece{n.data(), pos};
  BlobEncoder extraBlob{pce.useGlobalIds()};
  RepoTxnQuery query(txn, *this);
  query.bindInt64("@unitSn", unitSn);
  query.bindId("@preClassId", preClassId);
  query.bindStringPiece("@name", nm);
  query.bindInt("@hoistable", hoistable);
  const_cast<PreClassEmitter&>(pce).serdeMetaData(extraBlob);
  query.bindBlob("@extraData", extraBlob, /* static */ true);
  query.exec();

  RepoAutoloadMapBuilder::get().addClass(pce, unitSn);
}

void PreClassRepoProxy::GetPreClassesStmt
                      ::get(UnitEmitter& ue) {
  auto txn = RepoTxn{m_repo.begin()};
  if (!prepared()) {
    auto selectQuery = folly::sformat(
      "SELECT preClassId, name, hoistable, extraData "
      "FROM {} "
      "WHERE unitSn == @unitSn ORDER BY preClassId ASC;",
      m_repo.table(m_repoId, "PreClass"));
    txn.prepare(*this, selectQuery);
  }
  RepoTxnQuery query(txn, *this);
  query.bindInt64("@unitSn", ue.m_sn);
  do {
    query.step();
    if (query.row()) {
      Id preClassId;          /**/ query.getId(0, preClassId);
      std::string name;       /**/ query.getStdString(1, name);
      int hoistable;          /**/ query.getInt(2, hoistable);
      BlobDecoder extraBlob = /**/ query.getBlob(3, ue.useGlobalIds());
      PreClassEmitter* pce = ue.newPreClassEmitter(
        name, (PreClass::Hoistable)hoistable);
      pce->serdeMetaData(extraBlob);
      if (!SystemLib::s_inited) {
        assertx(pce->attrs() & AttrPersistent);
        assertx(pce->attrs() & AttrUnique);
      }
      assertx(pce->id() == preClassId);
    }
  } while (!query.done());
  txn.commit();
}

} // HPHP
