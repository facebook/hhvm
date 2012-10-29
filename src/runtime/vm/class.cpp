/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <boost/checked_delete.hpp>
#include <boost/optional.hpp>
#include <boost/utility/typed_in_place_factory.hpp>

#include <iostream>
#include <algorithm>

#include "runtime/base/base_includes.h"
#include "runtime/base/tv_macros.h"
#include "util/util.h"
#include "util/debug.h"
#include "runtime/vm/core_types.h"
#include "runtime/vm/hhbc.h"
#include "runtime/vm/class.h"
#include "runtime/vm/repo.h"
#include "runtime/vm/translator/targetcache.h"
#include "runtime/vm/blob_helper.h"
#include "runtime/vm/treadmill.h"
#include "runtime/vm/name_value_table.h"
#include "runtime/vm/name_value_table_wrapper.h"
#include "runtime/vm/request_arena.h"
#include "system/lib/systemlib.h"
#include "util/logger.h"

namespace HPHP {
namespace VM {

static StringData* sd86ctor = StringData::GetStaticString("86ctor");
static StringData* sd86pinit = StringData::GetStaticString("86pinit");
static StringData* sd86sinit = StringData::GetStaticString("86sinit");

hphp_hash_map<const StringData*, const HhbcExtClassInfo*,
              string_data_hash, string_data_isame> Class::s_extClassHash;

static const StringData* manglePropName(const StringData* className,
                                        const StringData* propName,
                                        Attr              attrs) {
  switch (attrs & (AttrPublic|AttrProtected|AttrPrivate)) {
  case AttrPublic: {
    return propName;
  }
  case AttrProtected: {
    std::string mangledName = "";
    mangledName.push_back('\0');
    mangledName.push_back('*');
    mangledName.push_back('\0');
    mangledName += propName->data();
    return StringData::GetStaticString(mangledName);
  }
  case AttrPrivate: {
    std::string mangledName = "";
    mangledName.push_back('\0');
    mangledName += className->data();
    mangledName.push_back('\0');
    mangledName += propName->data();
    return StringData::GetStaticString(mangledName);
  }
  default: not_reached();
  }
}

//=============================================================================
// PreClass::Prop.

PreClass::Prop::Prop(PreClass* preClass, const StringData* n, Attr attrs,
                     const StringData* docComment, const TypedValue& val)
  : m_preClass(preClass), m_name(n), m_attrs(attrs),
    m_docComment(docComment) {
  m_mangledName = manglePropName(preClass->name(), n, attrs);
  memcpy(&m_val, &val, sizeof(TypedValue));
}

void PreClass::Prop::prettyPrint(std::ostream& out) const {
  out << "Property ";
  if (m_attrs & AttrStatic) { out << "static "; }
  if (m_attrs & AttrPublic) { out << "public "; }
  if (m_attrs & AttrProtected) { out << "protected "; }
  if (m_attrs & AttrPrivate) { out << "private "; }
  out << m_preClass->name()->data() << "::" << m_name->data() << " = ";
  if (m_val.m_type == KindOfUninit) {
    out << "<non-scalar>";
  } else {
    std::stringstream ss;
    staticStreamer(&m_val, ss);
    out << ss.str();
  }
  out << std::endl;
}

//=============================================================================
// PreClass::Const.

PreClass::Const::Const(PreClass* preClass, const StringData* n,
                       const TypedValue& val, const StringData* phpCode)
  : m_preClass(preClass), m_name(n), m_phpCode(phpCode) {
  memcpy(&m_val, &val, sizeof(TypedValue));
}

void PreClass::Const::prettyPrint(std::ostream& out) const {
  out << "Constant " << m_preClass->name()->data() << "::" << m_name->data()
      << " = ";
  if (m_val.m_type == KindOfUninit) {
    out << "<non-scalar>";
  } else {
    std::stringstream ss;
    staticStreamer(&m_val, ss);
    out << ss.str();
  }
  out << std::endl;
}

//=============================================================================
// PreClass.

PreClass::PreClass(Unit* unit, int line1, int line2, Offset o,
                   const StringData* n, Attr attrs, const StringData* parent,
                   const StringData* docComment, Id id, Hoistable hoistable)
    : m_unit(unit), m_line1(line1), m_line2(line2), m_offset(o), m_id(id),
      m_builtinPropSize(0), m_attrs(attrs), m_hoistable(hoistable),
      m_name(n), m_parent(parent), m_docComment(docComment),
      m_InstanceCtor(NULL) {
  m_namedEntity = Unit::GetNamedEntity(n);
}

PreClass::~PreClass() {
  std::for_each(methods(), methods() + numMethods(),
                boost::checked_deleter<Func>());
}

void PreClass::atomicRelease() {
  delete this;
}

void PreClass::prettyPrint(std::ostream &out) const {
  out << "Class ";
  if (m_attrs & AttrAbstract) { out << "abstract "; }
  if (m_attrs & AttrFinal) { out << "final "; }
  if (m_attrs & AttrInterface) { out << "interface "; }
  out << m_name->data() << " at " << m_offset;
  if (m_hoistable == MaybeHoistable) {
    out << " (maybe-hoistable)";
  } else if (m_hoistable == AlwaysHoistable) {
    out << " (always-hoistable)";
  }
  if (m_id != -1) {
    out << " (ID " << m_id << ")";
  }
  out << std::endl;

  for (Func* const* it = methods(); it != methods() + numMethods(); ++it) {
    out << " ";
    (*it)->prettyPrint(out);
  }
  for (const Prop* it = properties();
      it != properties() + numProperties();
      ++it) {
    out << " ";
    it->prettyPrint(out);
  }
  for (const Const* it = constants();
      it != constants() + numConstants();
      ++it) {
    out << " ";
    it->prettyPrint(out);
  }
}

//=============================================================================
// PreClassEmitter::Prop.

PreClassEmitter::Prop::Prop(const PreClassEmitter* pce, const StringData* n,
                            Attr attrs, const StringData* docComment,
                            TypedValue* val)
  : m_name(n), m_attrs(attrs), m_docComment(docComment) {
  m_mangledName = manglePropName(pce->name(), n, attrs);
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
  , m_InstanceCtor(NULL)
  , m_builtinPropSize(0)
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
                                  const StringData* docComment,
                                  TypedValue* val) {
  PropMap::Builder::const_iterator it = m_propMap.find(n);
  if (it != m_propMap.end()) {
    return false;
  }
  PreClassEmitter::Prop prop(this, n, attrs, docComment, val);
  m_propMap.add(prop.name(), prop);
  return true;
}

const PreClassEmitter::Prop&
PreClassEmitter::lookupProp(const StringData* propName) const {
  PropMap::Builder::const_iterator it = m_propMap.find(propName);
  ASSERT(it != m_propMap.end());
  Slot idx = it->second;
  return m_propMap[idx];
}

bool PreClassEmitter::addConstant(const StringData* n, TypedValue* val,
                                  const StringData* phpCode) {
  ConstMap::Builder::const_iterator it = m_constMap.find(n);
  if (it != m_constMap.end()) {
    return false;
  }
  PreClassEmitter::Const const_(n, val, phpCode);
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
  int64 usn = m_ue.sn();
  pcrp.insertPreClass(repoId)
      .insert(*this, txn, usn, m_id, m_name, m_hoistable);

  for (MethodVec::const_iterator it = m_methods.begin();
       it != m_methods.end(); ++it) {
    (*it)->commit(txn);
  }
}

void PreClassEmitter::setBuiltinClassInfo(const ClassInfo* info,
                                          BuiltinCtorFunction ctorFunc,
                                          int sz) {
  if (info->getAttribute() & ClassInfo::IsFinal) {
    m_attrs = m_attrs | AttrFinal;
  }
  if (info->getAttribute() & ClassInfo::IsAbstract) {
    m_attrs = m_attrs | AttrAbstract;
  }
  m_attrs = m_attrs | AttrUnique;
  m_InstanceCtor = ctorFunc;
  m_builtinPropSize = sz - sizeof(ObjectData);
}

PreClass* PreClassEmitter::create(Unit& unit) const {
  Attr attrs = m_attrs;
  if (attrs & AttrPersistent &&
      !RuntimeOption::RepoAuthoritative && SystemLib::s_inited) {
    attrs = Attr(attrs & ~AttrPersistent);
  }
  PreClass* pc = new PreClass(&unit, m_line1, m_line2, m_offset, m_name,
                              attrs, m_parent, m_docComment, m_id,
                              m_hoistable);
  pc->m_InstanceCtor = m_InstanceCtor;
  pc->m_builtinPropSize = m_builtinPropSize;
  pc->m_interfaces = m_interfaces;
  pc->m_usedTraits = m_usedTraits;
  pc->m_traitPrecRules = m_traitPrecRules;
  pc->m_traitAliasRules = m_traitAliasRules;
  pc->m_userAttributes = m_userAttributes;

  PreClass::MethodMap::Builder methodBuild;
  for (MethodVec::const_iterator it = m_methods.begin();
       it != m_methods.end(); ++it) {
    Func* f = (*it)->create(unit, pc);
    methodBuild.add(f->name(), f);
  }
  pc->m_methods.create(methodBuild);

  PreClass::PropMap::Builder propBuild;
  for (unsigned i = 0; i < m_propMap.size(); ++i) {
    const Prop& prop = m_propMap[i];
    propBuild.add(prop.name(), PreClass::Prop(pc,
                                              prop.name(),
                                              prop.attrs(),
                                              prop.docComment(),
                                              prop.val()));
  }
  pc->m_properties.create(propBuild);

  PreClass::ConstMap::Builder constBuild;
  for (unsigned i = 0; i < m_constMap.size(); ++i) {
    const Const& const_ = m_constMap[i];
    constBuild.add(const_.name(), PreClass::Const(pc,
                                                  const_.name(),
                                                  const_.val(),
                                                  const_.phpCode()));
  }
  pc->m_constants.create(constBuild);
  return pc;
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
             << "(unitSn INTEGER, preClassId INTEGER, name TEXT, "
                " hoistable INTEGER, extraData BLOB, "
                " PRIMARY KEY (unitSn, preClassId));";
    txn.exec(ssCreate.str());
  }
}

void PreClassRepoProxy::InsertPreClassStmt
                      ::insert(const PreClassEmitter& pce, RepoTxn& txn,
                               int64 unitSn, Id preClassId,
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
      ASSERT(pce->id() == preClassId);
    }
  } while (!query.done());
  txn.commit();
}

//=============================================================================
// Class.

ClassPtr Class::newClass(PreClass* preClass, Class* parent) {
  unsigned classVecLen = (parent != NULL) ? parent->m_classVecLen+1 : 1;
  void* mem = Util::low_malloc(sizeForNClasses(classVecLen));
  assert(IMPLIES(alwaysLowMem(), ptr_is_low_mem(mem)) &&
         "All Classes must be allocated at 32-bit addresses");
  try {
    return ClassPtr(new (mem) Class(preClass, parent, classVecLen));
  } catch (...) {
    Util::low_free(mem);
    throw;
  }
}

Class::Class(PreClass* preClass, Class* parent, unsigned classVecLen)
  : m_preClass(PreClassPtr(preClass)), m_parent(ClassPtr(parent)),
    m_traitsBeginIdx(0), m_traitsEndIdx(0), m_clsInfo(NULL),
    m_builtinPropSize(0), m_classVecLen(classVecLen), m_cachedOffset(0),
    m_propDataCache(-1), m_propSDataCache(-1), m_InstanceCtor(NULL),
    m_nextClass(NULL) {
  setParent();
  setUsedTraits();
  setMethods();
  setSpecial();
  setODAttributes();
  setInterfaces();
  setConstants();
  setProperties();
  setInitializers();
  setClassVec();
}

void Class::atomicRelease() {
  if (m_cachedOffset != 0u) {
    /*
      m_cachedOffset is initialied to 0, and is only set
      when the Class is put on the list, so we only have to
      remove this node if its NOT 0.
      Since we're about to remove it, reset to 0 so we know
      its safe to kill the node during the delayed Treadmill
      callback.
    */
    m_cachedOffset = 0u;
    PreClass* pcls = m_preClass.get();
    {
      Lock l(Unit::s_classesMutex);
      Class *const*cls = pcls->namedEntity()->clsList();
      while (*cls != this) {
        ASSERT(*cls);
        cls = &(*cls)->m_nextClass;
      }
      *const_cast<Class**>(cls) = m_nextClass;
    }
    Treadmill::WorkItem::enqueue(new Treadmill::FreeClassTrigger(this));
    return;
  }

  this->~Class();
  free(this);
}

Class *Class::getCached() const {
  return *(Class**)Transl::TargetCache::handleToPtr(m_cachedOffset);
}

void Class::setCached() {
  *(Class**)Transl::TargetCache::handleToPtr(m_cachedOffset) = this;
}

bool Class::verifyPersistent() const {
  if (!(attrs() & AttrPersistent)) return false;
  if (m_parent.get() &&
      !Transl::TargetCache::isPersistentHandle(m_parent->m_cachedOffset)) {
    return false;
  }
  for (size_t i = 0, nInterfaces = m_declInterfaces.size();
       i < nInterfaces; ++i) {
    Class* declInterface = m_declInterfaces[i].get();
    if (!Transl::TargetCache::isPersistentHandle(
          declInterface->m_cachedOffset)) {
      return false;
    }
  }
  for (size_t i = 0; i < m_usedTraits.size(); i++) {
    Class* usedTrait = m_usedTraits[i].get();
    if (!Transl::TargetCache::isPersistentHandle(
          usedTrait->m_cachedOffset)) {
      return false;
    }
  }
  return true;
}

/*
 * Check whether a Class from a previous request is available to be defined.
 * The caller should check that it has the same preClass that is being defined.
 * Being available means that the parent, the interfaces and the traits are
 * already defined (or become defined via autoload, if tryAutoload is true).
 *
 * returns AvailTrue - if it is available
 *         AvailFail - if it is impossible to define the class at this point
 *         AvailFalse- if this particular Class* cant be defined at this point
 *
 * Note that AvailFail means that at least one of the parent, interfaces and
 * traits was not defined at all, while AvailFalse means that at least one
 * was defined but did not correspond to this Class*
 *
 * The parent parameter is used for two purposes: first it avoids looking up the
 * active parent class for each potential Class*; and second its used on
 * AvailFail to return the problem class so the caller can report the error
 * correctly.
 */
Class::Avail Class::avail(Class*& parent, bool tryAutoload /*=false*/) const {
  if (Class *ourParent = m_parent.get()) {
    if (!parent) {
      PreClass *ppcls = ourParent->m_preClass.get();
      parent = Unit::getClass(ppcls->namedEntity(), ppcls->name(), tryAutoload);
      if (!parent) {
        parent = ourParent;
        return AvailFail;
      }
    }
    if (parent != ourParent) return AvailFalse;
  }
  for (size_t i = 0, nInterfaces = m_declInterfaces.size();
       i < nInterfaces; ++i) {
    Class* declInterface = m_declInterfaces[i].get();
    PreClass *pint = declInterface->m_preClass.get();
    Class* interface = Unit::getClass(pint->namedEntity(), pint->name(),
                                      tryAutoload);
    if (interface != declInterface) {
      if (interface == NULL) {
        parent = declInterface;
        return AvailFail;
      }
      return AvailFalse;
    }
  }
  for (size_t i = 0; i < m_usedTraits.size(); i++) {
    Class* usedTrait = m_usedTraits[i].get();
    PreClass* ptrait = usedTrait->m_preClass.get();
    Class* trait = Unit::getClass(ptrait->namedEntity(), ptrait->name(),
                                  tryAutoload);
    if (trait != usedTrait) {
      if (trait == NULL) {
        parent = usedTrait;
        return AvailFail;
      }
      return AvailFalse;
    }
  }
  return AvailTrue;
}

// If this Class represents the same class as 'preClass' or a descendent of
// 'preClass', this function returns the Class* that corresponds to 'preClass'.
// Otherwise, this function returns NULL.
Class* Class::classof(const PreClass* preClass) const {
  Class* class_ = const_cast<Class*>(this);
  do {
    if (class_->m_preClass.get() == preClass) {
      return class_;
    }
    std::vector<ClassPtr>& interfaces = class_->m_declInterfaces;
    for (unsigned i = 0; i < interfaces.size(); ++i) {
      // Interfaces can extend arbitrarily many interfaces themselves, so
      // search them recursively
      Class* iclass = interfaces[i]->classof(preClass);
      if (iclass) {
        return iclass;
      }
    }
    class_ = class_->m_parent.get();
  } while (class_ != NULL);
  return NULL;
}

void Class::initialize(TypedValue*& sProps) const {
  if (m_pinitVec.size() > 0) {
    if (getPropData() == NULL) {
      // Initialization was not done, do so for the first time.
      PropInitVec* props = initProps();
      setPropData(props);
    }
  }
  // The asymmetry between the logic around initProps() above and initSProps()
  // below is due to the fact that instance properties only require storage in
  // g_vmContext if there are non-scalar initializers involved, whereas static
  // properties *always* require storage in g_vmContext.
  if (numStaticProperties() > 0) {
    if ((sProps = getSPropData()) == NULL) {
      sProps = initSProps();
      setSPropData(sProps);
    }
  } else {
    sProps = NULL;
  }
}

void Class::initialize() const {
  TypedValue* sProps;
  initialize(sProps);
}

Class::PropInitVec* Class::initProps() const {
  ASSERT(m_pinitVec.size() > 0);
  // Copy initial values for properties to a new vector that can be used to
  // complete initialization for non-scalar properties via the iterative
  // 86pinit() calls below.  86pinit() takes a reference to an array that
  // contains the initial property values; alias propVec inside propArr such
  // that propVec contains complete property initialization values as soon as
  // the 86pinit() calls are done.
  PropInitVec* propVec = PropInitVec::allocInRequestArena(m_declPropInit);
  size_t nProps = numDeclProperties();

  // During property initialization, we provide access to the
  // properties by name via this NameValueTable.
  NameValueTable propNvt(numDeclProperties());
  NameValueTableWrapper propArr(&propNvt);
  propArr.incRefCount();

  // Create a sentinel that uniquely identifies uninitialized properties.
  ObjectData* sentinel = SystemLib::AllocPinitSentinel();
  sentinel->incRefCount();
  // Insert propArr and sentinel into the args array, transferring ownership.
  HphpArray* args = NEW(HphpArray)(2);
  args->incRefCount();
  {
    TypedValue tv;
    tv.m_data.parr = &propArr;
    tv._count = 0;
    tv.m_type = KindOfArray;
    args->nvAppend(&tv, false);
    propArr.decRefCount();
  }
  {
    TypedValue tv;
    tv.m_data.pobj = sentinel;
    tv._count = 0;
    tv.m_type = KindOfObject;
    args->nvAppend(&tv, false);
    sentinel->decRefCount();
  }
  TypedValue* tvSentinel = args->nvGetValueRef(1);
  for (size_t i = 0; i < nProps; ++i) {
    TypedValue& prop = (*propVec)[i];
    if (prop.m_type == KindOfUninit) {
      // Replace undefined values with tvSentinel, which acts as a
      // unique sentinel for undefined properties in 86pinit().
      tvDup(tvSentinel, &prop);
    }
    // We have to use m_originalMangledName here because the
    // 86pinit methods for traits depend on it
    const StringData* k = (m_declProperties[i].m_attrs & AttrPrivate)
                           ? m_declProperties[i].m_originalMangledName
                           : m_declProperties[i].m_name;
    propNvt.migrateSet(k, &prop);
  }
  // Iteratively invoke 86pinit() methods upward through the inheritance chain.
  for (Class::InitVec::const_reverse_iterator it = m_pinitVec.rbegin();
       it != m_pinitVec.rend(); ++it) {
    TypedValue retval;
    g_vmContext->invokeFunc(&retval, *it, args, NULL, const_cast<Class*>(this));
    ASSERT(!IS_REFCOUNTED_TYPE(retval.m_type));
  }

  // Promote non-static arrays/strings (that came from 86pinit) to
  // static. This allows us to use memcpy to initialize object
  // properties, without conflicting with the refcounting that happens
  // on object destruction.
  for (PropInitVec::iterator it = propVec->begin();
       it != propVec->end(); ++it) {
    tvAsVariant(&(*it)).setEvalScalar();
  }

  // Free the args array.  propArr is allocated on the stack so it
  // better be only referenced from args.
  ASSERT(propArr.getCount() == 1);
  if (args->decRefCount() == 0) {
    args->release();
  } else {
    ASSERT(false);
  }
  return propVec;
}

Slot Class::getDeclPropIndex(Class* ctx, const StringData* key,
                             bool& accessible) const {
  Slot propInd = lookupDeclProp(key);
  if (propInd != kInvalidSlot) {
    Attr attrs = m_declProperties[propInd].m_attrs;
    if ((attrs & (AttrProtected|AttrPrivate)) &&
        !g_vmContext->getDebuggerBypassCheck()) {
      // Fetch 'baseClass', which is the class in the inheritance
      // tree which first declared the property
      Class* baseClass = m_declProperties[propInd].m_class;
      ASSERT(baseClass);
      // If ctx == baseClass, we know we have the right property
      // and we can stop here.
      if (ctx == baseClass) {
        accessible = true;
        return propInd;
      }
      // The anonymous context cannot access protected or private
      // properties, so we can fail fast here.
      if (ctx == NULL) {
        accessible = false;
        return propInd;
      }
      ASSERT(ctx);
      if (attrs & AttrPrivate) {
        // ctx != baseClass and the property is private, so it is not
        // accessible. We need to keep going because ctx may define a
        // private property with this name.
        accessible = false;
      } else {
        if (ctx->classof(baseClass)) {
          // ctx is derived from baseClass, so we know this protected
          // property is accessible and we know ctx cannot have private
          // property with the same name, so we're done.
          accessible = true;
          return propInd;
        }
        if (!baseClass->classof(ctx)) {
          // ctx is not the same, an ancestor, or a descendent of baseClass,
          // so the property is not accessible. Also, we know that ctx cannot
          // be the same or an ancestor of this, so we don't need to check if
          // ctx declares a private property with the same name and we can
          // fail fast here.
          accessible = false;
          return propInd;
        }
        // We now know this protected property is accessible, but we need to
        // keep going because ctx may define a private property with the same
        // name.
        accessible = true;
        ASSERT(baseClass->classof(ctx));
      }
    } else {
      // The property is public (or we're in the debugger and we are bypassing
      // accessibility checks).
      accessible = true;
      // If ctx == this, we don't have to check if ctx defines a private
      // property with the same name and we can stop here.
      if (ctx == this) {
        return propInd;
      }
      // We still need to check if ctx defines a private property with the
      // same name.
    }
  } else {
    // We didn't find a visible declared property in this's property map
    accessible = false;
  }
  // If ctx is an ancestor of this, check if ctx has a private method
  // with the same name.
  if (ctx && classof(ctx)) {
    Slot ctxPropInd = ctx->lookupDeclProp(key);
    if (ctxPropInd != kInvalidSlot &&
        ctx->m_declProperties[ctxPropInd].m_class == ctx &&
        (ctx->m_declProperties[ctxPropInd].m_attrs & AttrPrivate)) {
      // A private property from ctx trumps any other property we may
      // have found.
      accessible = true;
      return ctxPropInd;
    }
  }
  return propInd;
}

TypedValue* Class::initSProps() const {
  ASSERT(numStaticProperties() > 0);
  // Create an array that is initially large enough to hold all static
  // properties.
  TypedValue* const spropTable =
    new (request_arena()) TypedValue[m_staticProperties.size()];

  boost::optional<NameValueTable> nvt;
  const bool hasNonscalarInit = !m_sinitVec.empty();
  if (hasNonscalarInit) {
    nvt = boost::in_place<NameValueTable>(m_staticProperties.size());
  }

  // Iteratively initialize properties.  Non-scalar initializers are
  // initialized to KindOfUninit here, and the 86sinit()-based initialization
  // finishes the job later.
  for (Slot slot = 0; slot < m_staticProperties.size(); ++slot) {
    const SProp& sProp = m_staticProperties[slot];

    TypedValue* storage = 0;
    if (sProp.m_class == this) {
      // Embed static property value directly in array.
      ASSERT(tvIsStatic(&sProp.m_val));
      spropTable[slot] = sProp.m_val;
      storage = &spropTable[slot];
    } else {
      // Alias parent class's static property.
      bool visible, accessible;
      storage = sProp.m_class->getSProp(NULL, sProp.m_name, visible,
                                        accessible);
      tvBindIndirect(&spropTable[slot], storage);
    }

    if (hasNonscalarInit) {
      nvt->migrateSet(sProp.m_name, storage);
    }
  }

  // Invoke 86sinit's if necessary, to handle non-scalar initializers.
  if (hasNonscalarInit) {
    NameValueTableWrapper nvtWrapper(&*nvt);
    nvtWrapper.incRefCount();

    HphpArray* args = NEW(HphpArray)(1);
    args->incRefCount();
    {
      TypedValue tv;
      tv.m_data.parr = &nvtWrapper;
      tv._count = 0;
      tv.m_type = KindOfArray;
      args->nvAppend(&tv, false);
    }
    for (unsigned i = 0; i < m_sinitVec.size(); i++) {
      TypedValue retval;
      g_vmContext->invokeFunc(&retval, m_sinitVec[i], args, NULL,
                              const_cast<Class*>(this));
      ASSERT(!IS_REFCOUNTED_TYPE(retval.m_type));
    }
    // Release the args array.  nvtWrapper is on the stack, so it
    // better have a single reference.
    ASSERT(args->getCount() == 1);
    args->release();
    ASSERT(nvtWrapper.getCount() == 1);
  }

  return spropTable;
}

TypedValue* Class::getSProp(Class* ctx, const StringData* sPropName,
                            bool& visible, bool& accessible) const {
  TypedValue* sProps;
  initialize(sProps);

  Slot sPropInd = lookupSProp(sPropName);
  if (sPropInd == kInvalidSlot) {
    // Non-existant property.
    visible = false;
    accessible = false;
    return NULL;
  }

  visible = true;
  if (ctx == this) {
    // Property access is from within a method of this class, so the property
    // is accessible.
    accessible = true;
  } else {
    Attr sPropAttrs = m_staticProperties[sPropInd].m_attrs;
    if ((ctx != NULL) && (classof(ctx) || ctx->classof(this))) {
      // Property access is from within a parent class's method, which is
      // allowed for protected/public properties.
      switch (sPropAttrs & (AttrPublic|AttrProtected|AttrPrivate)) {
      case AttrPublic:
      case AttrProtected: accessible = true; break;
      case AttrPrivate:
        accessible = g_vmContext->getDebuggerBypassCheck(); break;
      default:            not_reached();
      }
    } else {
      // Property access is in an effectively anonymous context, so only public
      // properties are accessible.
      switch (sPropAttrs & (AttrPublic|AttrProtected|AttrPrivate)) {
      case AttrPublic:    accessible = true; break;
      case AttrProtected:
      case AttrPrivate:
        accessible = g_vmContext->getDebuggerBypassCheck(); break;
      default:            not_reached();
      }
    }
  }

  ASSERT(sProps != NULL);
  TypedValue* sProp = tvDerefIndirect(&sProps[sPropInd]);
  ASSERT(sProp->m_type != KindOfUninit &&
         "static property initialization failed to initialize a property");
  return sProp;
}

bool Class::IsPropAccessible(const Prop& prop, Class* ctx) {
  if (prop.m_attrs & AttrPublic) return true;
  if (prop.m_attrs & AttrPrivate) return prop.m_class == ctx;
  if (!ctx) return false;

  return prop.m_class->classof(ctx) || ctx->classof(prop.m_class);
}

TypedValue Class::getStaticPropInitVal(const SProp& prop) {
  Class* declCls = prop.m_class;
  Slot s = declCls->m_staticProperties.findIndex(prop.m_name);
  ASSERT(s != kInvalidSlot);
  return declCls->m_staticProperties[s].m_val;
}

HphpArray* Class::initClsCnsData() const {
  Slot nConstants = m_constants.size();
  HphpArray* constants = NEW(HphpArray)(nConstants);
  constants->incRefCount();

  if (m_parent.get() != NULL) {
    if (g_vmContext->getClsCnsData(m_parent.get()) == NULL) {
      // Initialize recursively up the inheritance chain.
      m_parent->initClsCnsData();
    }
  }

  for (Slot i = 0; i < nConstants; ++i) {
    const Const& constant = m_constants[i];
    TypedValue* tv = (TypedValue*)&constant.m_val;
    constants->nvSet((StringData*)constant.m_name, tv, false);
    // XXX: nvSet() converts KindOfUninit to KindOfNull, but our class
    // constant logic needs to store KindOfUninit to indicate the the
    // constant's value has not been computed yet. We should find a better
    // way to deal with this.
    if (tv->m_type == KindOfUninit) {
      constants->nvGetValueRef(i)->m_type = KindOfUninit;
    }
  }

  g_vmContext->setClsCnsData(this, constants);
  return constants;
}

TypedValue* Class::cnsNameToTV(const StringData* clsCnsName,
                               Slot& clsCnsInd) const {
  clsCnsInd = m_constants.findIndex(clsCnsName);
  if (clsCnsInd == kInvalidSlot) {
    return NULL;
  }
  return const_cast<TypedValue*>(&m_constants[clsCnsInd].m_val);
}

TypedValue* Class::clsCnsGet(const StringData* clsCnsName) const {
  Slot clsCnsInd;
  TypedValue* clsCns = cnsNameToTV(clsCnsName, clsCnsInd);
  if (!clsCns || clsCns->m_type != KindOfUninit) {
    return clsCns;
  }

  // This constant has a non-scalar initializer, so look in g_vmContext for
  // an entry associated with this class.
  HphpArray* clsCnsData = g_vmContext->getClsCnsData(this);
  if (clsCnsData == NULL) {
    clsCnsData = initClsCnsData();
  }

  clsCns = clsCnsData->nvGetValueRef(clsCnsInd);
  if (clsCns->m_type == KindOfUninit) {
    // The class constant has not been initialized yet; do so.
    static StringData* sd86cinit = StringData::GetStaticString("86cinit");
    const Func* meth86cinit =
      m_constants[clsCnsInd].m_class->lookupMethod(sd86cinit);
    TypedValue tv;
    tv.m_data.pstr = (StringData*)clsCnsName;
    tv._count = 0;
    tv.m_type = KindOfString;
    g_vmContext->invokeFunc(clsCns, meth86cinit,
                            CREATE_VECTOR1(tvAsCVarRef(&tv)), NULL,
                            const_cast<Class*>(this));
  }
  return clsCns;
}

/*
 * Class::clsCnsType: provide the current runtime type of this class
 *   constant. This has predictive value for the translator.
 */
DataType Class::clsCnsType(const StringData* cnsName) const {
  Slot slot;
  TypedValue* cns = cnsNameToTV(cnsName, slot);
  if (!cns) return KindOfUninit;
  return cns->m_type;
}

void Class::setParent() {
  // Validate the parent
  if (m_parent.get() != NULL) {
    Attr attrs = m_parent->attrs();
    if (UNLIKELY(attrs & (AttrFinal | AttrInterface | AttrTrait))) {
      static StringData* sd___MockClass =
        StringData::GetStaticString("__MockClass");
      if (!(attrs & AttrFinal) ||
          m_preClass->userAttributes().find(sd___MockClass) ==
          m_preClass->userAttributes().end()) {
        raise_error("Class %s may not inherit from %s (%s)",
                    m_preClass->name()->data(),
                    ((attrs & AttrFinal)     ? "final class" :
                     (attrs & AttrInterface) ? "interface"   : "trait"),
                    m_parent->name()->data());
      }
    }
  }
  // Cache m_preClass->attrs()
  m_attrCopy = m_preClass->attrs();
  // Handle stuff specific to cppext classes
  if (m_preClass->instanceCtor()) {
    m_InstanceCtor = m_preClass->instanceCtor();
    m_builtinPropSize = m_preClass->builtinPropSize();
    m_clsInfo = ClassInfo::FindSystemClassInterfaceOrTrait(nameRef());
  } else if (m_parent.get()) {
    m_InstanceCtor = m_parent->m_InstanceCtor;
    m_builtinPropSize = m_parent->m_builtinPropSize;
  }
}

static Func* findSpecialMethod(Class* cls, const StringData* name) {
  if (!cls->preClass()->hasMethod(name)) return NULL;
  Func* f = cls->preClass()->lookupMethod(name);
  f = f->clone();
  f->setNewFuncId();
  f->setCls(cls);
  f->setBaseCls(cls);
  f->setHasPrivateAncestor(false);
  return f;
}

void Class::setSpecial() {
  static StringData* sd_toString = StringData::GetStaticString("__toString");
  static StringData* sd_uuconstruct =
    StringData::GetStaticString("__construct");
  static StringData* sd_uudestruct =
    StringData::GetStaticString("__destruct");

  m_toString = lookupMethod(sd_toString);
  m_dtor = lookupMethod(sd_uudestruct);

  // Look for __construct() declared in either this class or a trait
  Func* fConstruct = lookupMethod(sd_uuconstruct);
  if (fConstruct && (fConstruct->preClass() == m_preClass.get() ||
                     fConstruct->preClass()->attrs() & AttrTrait)) {
    m_ctor = fConstruct;
    return;
  }

  if (!(attrs() & AttrTrait)) {
    // Look for Foo::Foo() declared in this class (cannot be via trait).
    Func* fNamedCtor = lookupMethod(m_preClass->name());
    if (fNamedCtor && fNamedCtor->preClass() == m_preClass.get() &&
        !(fNamedCtor->attrs() & AttrTrait)) {
      /*
        Note: AttrTrait was set by the emitter if hphpc inlined a trait
        method into a class (WholeProgram mode only), so that we dont
        accidently mark it as a constructor here
      */
      m_ctor = fNamedCtor;
      return;
    }
  }

  // Look for parent constructor other than 86ctor().
  if (m_parent.get() != NULL &&
      m_parent->m_ctor->name() != sd86ctor) {
    m_ctor = m_parent->m_ctor;
    return;
  }

  // Use 86ctor(), since no program-supplied constructor exists
  m_ctor = findSpecialMethod(this, sd86ctor);
  ASSERT(m_ctor && "class had no user-defined constructor or 86ctor");
  ASSERT(m_ctor->attrs() == AttrPublic);
}

void Class::applyTraitPrecRule(const PreClass::TraitPrecRule& rule) {
  const StringData* methName          = rule.getMethodName();
  const StringData* selectedTraitName = rule.getSelectedTraitName();
  TraitNameSet      otherTraitNames;
  rule.getOtherTraitNames(otherTraitNames);

  MethodToTraitListMap::iterator methIter =
    m_importMethToTraitMap.find(methName);
  if (methIter == m_importMethToTraitMap.end()) {
    raise_error("unknown method '%s'", methName->data());
  }

  bool foundSelectedTrait = false;

  TraitMethodList &methList = methIter->second;
  for (TraitMethodList::iterator nextTraitIter = methList.begin();
       nextTraitIter != methList.end(); ) {
    TraitMethodList::iterator traitIter = nextTraitIter++;
    const StringData* availTraitName = traitIter->m_trait->name();
    if (availTraitName == selectedTraitName) {
      foundSelectedTrait = true;
    } else {
      if (otherTraitNames.find(availTraitName) != otherTraitNames.end()) {
        otherTraitNames.erase(availTraitName);
        methList.erase(traitIter);
      }
    }
  }

  // Check error conditions
  if (!foundSelectedTrait) {
    raise_error("unknown trait '%s'", selectedTraitName->data());
  }
  if (otherTraitNames.size()) {
    raise_error("unknown trait '%s'", (*otherTraitNames.begin())->data());
  }
}

ClassPtr Class::findSingleTraitWithMethod(const StringData* methName) {
  // Note: m_methods includes methods from parents / traits recursively
  ClassPtr traitCls = ClassPtr();
  for (size_t t = 0; t < m_usedTraits.size(); t++) {
    if (m_usedTraits[t]->m_methods.contains(methName)) {
      if (traitCls.get() != NULL) { // more than one trait contains method
        return ClassPtr();
      }
      traitCls = m_usedTraits[t];
    }
  }
  return traitCls;
}

void Class::setImportTraitMethodModifiers(const StringData* methName,
                                          ClassPtr          traitCls,
                                          Attr              modifiers) {
  TraitMethodList &methList = m_importMethToTraitMap[methName];

  for (TraitMethodList::iterator iter = methList.begin();
       iter != methList.end(); iter++) {
    if (iter->m_trait.get() == traitCls.get()) {
      iter->m_modifiers = modifiers;
      return;
    }
  }
}

// Keep track of trait aliases in the class to support
// ReflectionClass::getTraitAliases
void Class::addTraitAlias(const StringData* traitName,
                          const StringData* origMethName,
                          const StringData* newMethName) {
  char buf[traitName->size() + origMethName->size() + 9];
  sprintf(buf, "%s::%s", (traitName->empty() ? "(null)" : traitName->data()),
          origMethName->data());
  const StringData* origName = StringData::GetStaticString(buf);
  m_traitAliases.push_back(std::pair<const StringData*, const StringData*>
                           (newMethName, origName));
}

void Class::applyTraitAliasRule(const PreClass::TraitAliasRule& rule) {
  const StringData* traitName    = rule.getTraitName();
  const StringData* origMethName = rule.getOrigMethodName();
  const StringData* newMethName  = rule.getNewMethodName();

  ClassPtr traitCls;
  if (traitName->empty()) {
    traitCls = findSingleTraitWithMethod(origMethName);
  } else {
    traitCls = Unit::loadClass(traitName);
  }

  if (!traitCls.get() || (!(traitCls->attrs() & AttrTrait))) {
    raise_error("unknown trait '%s'", traitName->data());
  }

  // Save info to support ReflectionClass::getTraitAliases
  addTraitAlias(traitName, origMethName, newMethName);

  Func* traitMeth = traitCls->lookupMethod(origMethName);
  if (!traitMeth) {
    raise_error("unknown trait method '%s'", origMethName->data());
  }

  Attr ruleModifiers;
  if (origMethName == newMethName) {
    ruleModifiers = rule.getModifiers();
    setImportTraitMethodModifiers(origMethName, traitCls, ruleModifiers);
  } else {
    ruleModifiers = rule.getModifiers();
    TraitMethod traitMethod(traitCls, traitMeth, ruleModifiers);
    addImportTraitMethod(traitMethod, newMethName);
  }
  if (ruleModifiers & AttrStatic) {
    raise_error("cannot use 'static' as access modifier");
  }
}

void Class::applyTraitRules() {
  for (size_t i = 0; i < m_preClass->traitPrecRules().size(); i++) {
    applyTraitPrecRule(m_preClass->traitPrecRules()[i]);
  }
  for (size_t i = 0; i < m_preClass->traitAliasRules().size(); i++) {
    applyTraitAliasRule(m_preClass->traitAliasRules()[i]);
  }
}

void Class::addImportTraitMethod(const TraitMethod &traitMethod,
                                 const StringData  *methName) {
  if (!Func::isSpecial(methName)) {
    m_importMethToTraitMap[methName].push_back(traitMethod);
  }
}

void Class::importTraitMethod(const TraitMethod&  traitMethod,
                              const StringData*   methName,
                              MethodMap::Builder& builder) {
  ClassPtr trait     = traitMethod.m_trait;
  Func*    method    = traitMethod.m_method;
  Attr     modifiers = traitMethod.m_modifiers;

  MethodMap::Builder::iterator mm_iter = builder.find(methName);
  // For abstract methods, simply return if method already declared
  if ((modifiers & AttrAbstract) && mm_iter != builder.end()) {
    return;
  }

  if (modifiers == AttrNone) {
    modifiers = method->attrs();
  } else {
    // Trait alias statements are only allowed to change the attributes that
    // are part 'attrMask' below; all other method attributes are preserved
    Attr attrMask = (Attr)(AttrPublic | AttrProtected | AttrPrivate |
                           AttrAbstract | AttrFinal);
    modifiers = (Attr)((modifiers       &  (attrMask)) |
                       (method->attrs() & ~(attrMask)));
  }

  Func* parentMethod = NULL;
  if (mm_iter != builder.end()) {
    Func* existingMethod = builder[mm_iter->second];
    if (existingMethod->cls() == this) {
      // Don't override an existing method if this class provided an
      // implementation
      return;
    }
    parentMethod = existingMethod;
  }
  Func* f = method->clone();
  f->setNewFuncId();
  f->setClsAndName(this, methName);
  f->setAttrs(modifiers);
  if (!parentMethod) {
    // New method
    builder.add(methName, f);
    f->setBaseCls(this);
    f->setHasPrivateAncestor(false);
  } else {
    // Override an existing method
    Class* baseClass;

    methodOverrideCheck(parentMethod, f);

    ASSERT(!(f->attrs() & AttrPrivate) ||
           (parentMethod->attrs() & AttrPrivate));
    if ((parentMethod->attrs() & AttrPrivate) || (f->attrs() & AttrPrivate)) {
      baseClass = this;
    } else {
      baseClass = parentMethod->baseCls();
    }
    f->setBaseCls(baseClass);
    f->setHasPrivateAncestor(
      parentMethod->hasPrivateAncestor() ||
      (parentMethod->attrs() & AttrPrivate));
    builder[mm_iter->second] = f;
  }
}

// This method removes trait abstract methods that are either:
//   1) implemented by other traits
//   2) duplicate
void Class::removeSpareTraitAbstractMethods() {

  for (MethodToTraitListMap::iterator iter = m_importMethToTraitMap.begin();
       iter != m_importMethToTraitMap.end(); iter++) {

    TraitMethodList& tMethList = iter->second;
    bool hasNonAbstractMeth = false;
    unsigned countAbstractMeths = 0;
    for (TraitMethodList::const_iterator traitMethIter = tMethList.begin();
         traitMethIter != tMethList.end(); traitMethIter++) {
      if (!(traitMethIter->m_modifiers & AttrAbstract)) {
        hasNonAbstractMeth = true;
      } else {
        countAbstractMeths++;
      }
    }
    if (hasNonAbstractMeth || countAbstractMeths > 1) {
      // Erase spare abstract declarations
      bool firstAbstractMeth = true;
      for (TraitMethodList::iterator nextTraitIter = tMethList.begin();
           nextTraitIter != tMethList.end(); ) {
        TraitMethodList::iterator traitIter = nextTraitIter++;
        if (traitIter->m_modifiers & AttrAbstract) {
          if (hasNonAbstractMeth || !firstAbstractMeth) {
            tMethList.erase(traitIter);
          }
          firstAbstractMeth = false;
        }
      }
    }
  }
}

// fatals on error
void Class::importTraitMethods(MethodMap::Builder& builder) {
  // 1. Find all methods to be imported
  for (size_t t = 0; t < m_usedTraits.size(); t++) {
    ClassPtr trait = m_usedTraits[t];
    for (Slot i = 0; i < trait->m_methods.size(); ++i) {
      Func* method = trait->m_methods[i];
      const StringData* methName = method->name();
      TraitMethod traitMethod(trait, method, method->attrs());
      addImportTraitMethod(traitMethod, methName);
    }
  }

  // 2. Apply trait rules
  applyTraitRules();

  // 3. Remove abstract methods provided by other traits, and also duplicates
  removeSpareTraitAbstractMethods();

  // 4. Actually import the methods
  for (MethodToTraitListMap::const_iterator iter =
         m_importMethToTraitMap.begin();
       iter != m_importMethToTraitMap.end(); iter++) {

    // The rules may rule out a method from all traits.
    // In this case, simply don't import the method.
    if (iter->second.size() == 0) {
      continue;
    }

    // Consistency checking: each name must only refer to one imported method
    if (iter->second.size() > 1) {
      // OK if the class will override the method...
      if (m_preClass->hasMethod(iter->first)) continue;

      raise_error("method '%s' declared in multiple traits",
                  iter->first->data());
    }

    TraitMethodList::const_iterator traitMethIter = iter->second.begin();
    importTraitMethod(*traitMethIter, iter->first, builder);
  }
}


void Class::methodOverrideCheck(const Func* parentMethod, const Func* method) {
  // Skip special methods
  if (isdigit((uchar)method->name()->data()[0])) return;

  if ((parentMethod->attrs() & AttrFinal)) {
    static StringData* sd___MockClass =
      StringData::GetStaticString("__MockClass");
    if (m_preClass->userAttributes().find(sd___MockClass) ==
        m_preClass->userAttributes().end()) {
      raise_error("Cannot override final method %s::%s()",
                  m_parent->name()->data(), parentMethod->name()->data());
    }
  }

  if (method->attrs() & AttrAbstract) {
    raise_error("Cannot re-declare %sabstract method %s::%s() abstract in "
                "class %s",
                (parentMethod->attrs() & AttrAbstract) ? "" : "non-",
                m_parent->m_preClass->name()->data(),
                parentMethod->name()->data(), m_preClass->name()->data());
  }

  if ((method->attrs()       & (AttrPublic | AttrProtected | AttrPrivate)) >
      (parentMethod->attrs() & (AttrPublic | AttrProtected | AttrPrivate))) {
    raise_error(
      "Access level to %s::%s() must be %s (as in class %s) or weaker",
      m_preClass->name()->data(), method->name()->data(),
      attrToVisibilityStr(parentMethod->attrs()),
      m_parent->name()->data());
  }

  if ((method->attrs() & AttrStatic) != (parentMethod->attrs() & AttrStatic)) {
    raise_error("Cannot change %sstatic method %s::%s() to %sstatic in %s",
                (parentMethod->attrs() & AttrStatic) ? "" : "non-",
                parentMethod->baseCls()->name()->data(),
                method->name()->data(),
                (method->attrs() & AttrStatic) ? "" : "non-",
                m_preClass->name()->data());
  }

  Func* baseMethod = parentMethod->baseCls()->lookupMethod(method->name());
  if (!(method->attrs() & AttrAbstract) &&
      (baseMethod->attrs() & AttrAbstract) &&
      (!hphpiCompat || strcmp(method->name()->data(), "__construct"))) {
    method->parametersCompat(m_preClass.get(), baseMethod);
  }
}

void Class::setMethods() {
  std::vector<Slot> parentMethodsWithStaticLocals;
  MethodMap::Builder builder;

  if (m_parent.get() != NULL) {
    // Copy down the parent's method entries. These may be overridden below.
    for (Slot i = 0; i < m_parent->m_methods.size(); ++i) {
      Func* f = m_parent->m_methods[i];
      ASSERT(f);
      if ((f->attrs() & AttrClone) ||
          !(f->attrs() & AttrPrivate) && f->hasStaticLocals()) {
        // When copying down an entry for a non-private method that has
        // static locals, we want to make a copy of the Func so that it
        // gets a distinct set of static locals variables. We defer making
        // a copy of the parent method until the end because it might get
        // overriden below.
        parentMethodsWithStaticLocals.push_back(i);
      }
      ASSERT(builder.size() == i);
      builder.add(f->name(), f);
    }
  }

  ASSERT(AttrPublic < AttrProtected && AttrProtected < AttrPrivate);
  // Overlay/append this class's public/protected methods onto/to those of the
  // parent.
  for (size_t methI = 0; methI < m_preClass->numMethods(); ++methI) {
    Func* method = m_preClass->methods()[methI];
    if (Func::isSpecial(method->name())) {
      if (method->name() == sd86ctor ||
          method->name() == sd86sinit ||
          method->name() == sd86pinit) {
        /*
         * we could also skip the cinit function here, but
         * that would mean storing it somewhere else.
         */
        continue;
      }
    }
    MethodMap::Builder::iterator it2 = builder.find(method->name());
    if (it2 != builder.end()) {
      Func* parentMethod = builder[it2->second];
      // We should never have null func pointers to deal with
      ASSERT(parentMethod);
      methodOverrideCheck(parentMethod, method);
      // Overlay.
      Func* f = method->clone();
      f->setNewFuncId();
      f->setCls(this);
      Class* baseClass;
      ASSERT(!(f->attrs() & AttrPrivate) ||
             (parentMethod->attrs() & AttrPrivate));
      if ((parentMethod->attrs() & AttrPrivate) || (f->attrs() & AttrPrivate)) {
        baseClass = this;
      } else {
        baseClass = parentMethod->baseCls();
      }
      f->setBaseCls(baseClass);
      f->setHasPrivateAncestor(
        parentMethod->hasPrivateAncestor() ||
        (parentMethod->attrs() & AttrPrivate));
      builder[it2->second] = f;
    } else {
      // This is the first class that declares the method
      Class* baseClass = this;
      // Append.
      Func* f = method->clone();
      f->setNewFuncId();
      f->setCls(this);
      f->setBaseCls(baseClass);
      f->setHasPrivateAncestor(false);
      builder.add(method->name(), f);
    }
  }

  m_traitsBeginIdx = builder.size();
  if (m_usedTraits.size()) {
    importTraitMethods(builder);
  }
  m_traitsEndIdx = builder.size();

  // Make copies of Funcs inherited from the parent class that have
  // static locals
  std::vector<Slot>::const_iterator it;
  for (it = parentMethodsWithStaticLocals.begin();
       it != parentMethodsWithStaticLocals.end(); ++it) {
    Func*& f = builder[*it];
    if (f->cls() != this) {
      // Don't update f's m_cls if it doesn't have AttrClone set:
      // we're cloning it so that we get a distinct set of static
      // locals and a separate translation, not a different context
      // class.
      f = f->clone();
      if (f->attrs() & AttrClone) {
        f->setCls(this);
      }
      f->setNewFuncId();
    }
  }

  // If class is not abstract, check that all abstract methods have been defined
  if (!(attrs() & (AttrTrait | AttrInterface | AttrAbstract))) {
    for (Slot i = 0; i < builder.size(); i++) {
      const Func* meth = builder[i];
      if (meth->attrs() & AttrAbstract) {
        raise_error("Class %s contains abstract method (%s) and "
                    "must therefore be declared abstract or implement "
                    "the remaining methods", m_preClass->name()->data(),
                    meth->name()->data());
      }
    }
  }

  m_methods.create(builder);
  for (Slot i = 0; i < m_methods.size(); ++i) {
    m_methods[i]->setMethodSlot(i);
  }
}

void Class::setODAttributes() {
  static StringData* sd__sleep = StringData::GetStaticString("__sleep");
  static StringData* sd__get = StringData::GetStaticString("__get");
  static StringData* sd__set = StringData::GetStaticString("__set");
  static StringData* sd__isset = StringData::GetStaticString("__isset");
  static StringData* sd__unset = StringData::GetStaticString("__unset");
  static StringData* sd___lval = StringData::GetStaticString("___lval");
  static StringData* sd__call = StringData::GetStaticString("__call");
  static StringData* sd__callStatic
    = StringData::GetStaticString("__callStatic");

  m_ODAttrs = 0;
  if (lookupMethod(sd__sleep     )) { m_ODAttrs |= ObjectData::HasSleep;      }
  if (lookupMethod(sd__get       )) { m_ODAttrs |= ObjectData::UseGet;        }
  if (lookupMethod(sd__set       )) { m_ODAttrs |= ObjectData::UseSet;        }
  if (lookupMethod(sd__isset     )) { m_ODAttrs |= ObjectData::UseIsset;      }
  if (lookupMethod(sd__unset     )) { m_ODAttrs |= ObjectData::UseUnset;      }
  if (lookupMethod(sd___lval     )) { m_ODAttrs |= ObjectData::HasLval;       }
  if (lookupMethod(sd__call      )) { m_ODAttrs |= ObjectData::HasCall;       }
  if (lookupMethod(sd__callStatic)) { m_ODAttrs |= ObjectData::HasCallStatic; }
}

void Class::setConstants() {
  ConstMap::Builder builder;

  if (m_parent.get() != NULL) {
    for (Slot i = 0; i < m_parent->m_constants.size(); ++i) {
      // Copy parent's constants.
      builder.add(m_parent->m_constants[i].m_name, m_parent->m_constants[i]);
    }
  }

  // Copy in interface constants.
  for (std::vector<ClassPtr>::const_iterator it = m_declInterfaces.begin();
       it != m_declInterfaces.end(); ++it) {
    for (Slot slot = 0; slot < (*it)->m_constants.size(); ++slot) {
      const Const& iConst = (*it)->m_constants[slot];

      // If you're inheriting a constant with the same name as an
      // existing one, they must originate from the same place.
      ConstMap::Builder::iterator existing = builder.find(iConst.m_name);
      if (existing != builder.end() &&
          builder[existing->second].m_class != iConst.m_class) {
        raise_error("Cannot inherit previously-inherited constant %s",
                    iConst.m_name->data());
      }

      builder.add(iConst.m_name, iConst);
    }
  }

  for (Slot i = 0, sz = m_preClass->numConstants(); i < sz; ++i) {
    const PreClass::Const* preConst = &m_preClass->constants()[i];
    ConstMap::Builder::iterator it2 = builder.find(preConst->name());
    if (it2 != builder.end()) {
      if (!(builder[it2->second].m_class->attrs() & AttrInterface)) {
        // Overlay ancestor's constant, only if it was not an interface const.
        builder[it2->second].m_class = this;
        builder[it2->second].m_val = preConst->val();
      } else {
        raise_error("Cannot override previously defined constant %s::%s in %s",
                  builder[it2->second].m_class->name()->data(),
                  preConst->name()->data(),
                  m_preClass->name()->data());
      }
    } else {
      // Append constant.
      Const constant;
      constant.m_class = this;
      constant.m_name = preConst->name();
      constant.m_val = preConst->val();
      constant.m_phpCode = preConst->phpCode();
      builder.add(preConst->name(), constant);
    }
  }

  m_constants.create(builder);
}

void Class::setProperties() {
  int numInaccessible = 0;
  PropMap::Builder curPropMap;
  SPropMap::Builder curSPropMap;

  if (m_parent.get() != NULL) {
    for (Slot slot = 0; slot < m_parent->m_declProperties.size(); ++slot) {
      const Prop& parentProp = m_parent->m_declProperties[slot];

      // Copy parent's declared property.  Protected properties may be
      // weakened to public below, but otherwise, the parent's properties
      // will stay the same for this class.
      Prop prop;
      prop.m_class = parentProp.m_class;
      prop.m_mangledName = parentProp.m_mangledName;
      prop.m_originalMangledName = parentProp.m_originalMangledName;
      prop.m_attrs = parentProp.m_attrs;
      prop.m_docComment = parentProp.m_docComment;
      prop.m_name = parentProp.m_name;
      if (!(parentProp.m_attrs & AttrPrivate)) {
        curPropMap.add(prop.m_name, prop);
      } else {
        ++numInaccessible;
        curPropMap.addUnnamed(prop);
      }
    }
    m_declPropInit = m_parent->m_declPropInit;
    for (Slot slot = 0; slot < m_parent->m_staticProperties.size(); ++slot) {
      const SProp& parentProp = m_parent->m_staticProperties[slot];
      if (parentProp.m_attrs & AttrPrivate) continue;

      // Alias parent's static property.
      SProp sProp;
      sProp.m_name = parentProp.m_name;
      sProp.m_attrs = parentProp.m_attrs;
      sProp.m_docComment = parentProp.m_docComment;
      sProp.m_class = parentProp.m_class;
      TV_WRITE_UNINIT(&sProp.m_val);
      curSPropMap.add(sProp.m_name, sProp);
    }
  }

  ASSERT(AttrPublic < AttrProtected && AttrProtected < AttrPrivate);
  for (Slot slot = 0; slot < m_preClass->numProperties(); ++slot) {
    const PreClass::Prop* preProp = &m_preClass->properties()[slot];

    if (!(preProp->attrs() & AttrStatic)) {
      // Overlay/append this class's protected and public properties onto/to
      // those of the parent, and append this class's private properties.
      // Append order doesn't matter here (unlike in setMethods()).

      // Prohibit static-->non-static redeclaration.
      SPropMap::Builder::iterator it2 = curSPropMap.find(preProp->name());
      if (it2 != curSPropMap.end()) {
        raise_error("Cannot redeclare static %s::$%s as non-static %s::$%s",
                    curSPropMap[it2->second].m_class->name()->data(),
                    preProp->name()->data(), m_preClass->name()->data(),
                    preProp->name()->data());
      }
      // Get parent's equivalent property, if one exists.
      const Prop* parentProp = NULL;
      if (m_parent.get() != NULL) {
        Slot id = m_parent->m_declProperties.findIndex(preProp->name());
        if (id != kInvalidSlot) {
          parentProp = &m_parent->m_declProperties[id];
        }
      }
      // Prohibit strengthening.
      if (parentProp
          && (preProp->attrs() & (AttrPublic|AttrProtected|AttrPrivate))
             > (parentProp->m_attrs & (AttrPublic|AttrProtected|AttrPrivate))) {
        raise_error(
          "Access level to %s::$%s() must be %s (as in class %s) or weaker",
          m_preClass->name()->data(), preProp->name()->data(),
          attrToVisibilityStr(parentProp->m_attrs),
          m_parent->name()->data());
      }
      switch (preProp->attrs() & (AttrPublic|AttrProtected|AttrPrivate)) {
      case AttrPrivate: {
        // Append a new private property.
        Prop prop;
        prop.m_name = preProp->name();
        prop.m_mangledName = preProp->mangledName();
        prop.m_originalMangledName = preProp->mangledName();
        prop.m_attrs = preProp->attrs();
        // This is the first class to declare this property
        prop.m_class = this;
        prop.m_docComment = preProp->docComment();
        curPropMap.add(preProp->name(), prop);
        m_declPropInit.push_back(m_preClass->lookupProp(preProp->name())
                                 ->val());
        break;
      }
      case AttrProtected: {
        // Check whether a superclass has already declared this protected
        // property.
        PropMap::Builder::iterator it2 = curPropMap.find(preProp->name());
        if (it2 != curPropMap.end()) {
          ASSERT((curPropMap[it2->second].m_attrs
                 & (AttrPublic|AttrProtected|AttrPrivate)) == AttrProtected);
          m_declPropInit[it2->second] = m_preClass->lookupProp(preProp
                                        ->name())->val();
          break;
        }
        // Append a new protected property.
        Prop prop;
        prop.m_name = preProp->name();
        prop.m_mangledName = preProp->mangledName();
        prop.m_originalMangledName = preProp->mangledName();
        prop.m_attrs = preProp->attrs();
        // This is the first class to declare this property
        prop.m_class = this;
        prop.m_docComment = preProp->docComment();
        curPropMap.add(preProp->name(), prop);
        m_declPropInit.push_back(m_preClass->lookupProp(preProp->name())
                                 ->val());
        break;
      }
      case AttrPublic: {
        // Check whether a superclass has already declared this as a
        // protected/public property.
        PropMap::Builder::iterator it2 = curPropMap.find(preProp->name());
        if (it2 != curPropMap.end()) {
          Prop& prop = curPropMap[it2->second];
          if ((prop.m_attrs & (AttrPublic|AttrProtected|AttrPrivate))
              == AttrProtected) {
            // Weaken protected property to public.
            prop.m_mangledName = preProp->mangledName();
            prop.m_originalMangledName = preProp->mangledName();
            prop.m_attrs = Attr(prop.m_attrs ^ (AttrProtected|AttrPublic));
          }
          m_declPropInit[it2->second] = m_preClass->lookupProp(preProp
                                        ->name())->val();
          break;
        }
        // Append a new public property.
        Prop prop;
        prop.m_name = preProp->name();
        prop.m_mangledName = preProp->mangledName();
        prop.m_originalMangledName = preProp->mangledName();
        prop.m_attrs = preProp->attrs();
        // This is the first class to declare this property
        prop.m_class = this;
        prop.m_docComment = preProp->docComment();
        curPropMap.add(preProp->name(), prop);
        m_declPropInit.push_back(m_preClass->lookupProp(preProp->name())
                                 ->val());
        break;
      }
      default: ASSERT(false);
      }
    } else { // Static property.
      // Prohibit non-static-->static redeclaration.
      PropMap::Builder::iterator it2 = curPropMap.find(preProp->name());
      if (it2 != curPropMap.end()) {
        // Find class that declared non-static property.
        Class* ancestor;
        for (ancestor = m_parent.get();
             !ancestor->m_preClass->hasProp(preProp->name());
             ancestor = ancestor->m_parent.get()) {
        }
        raise_error("Cannot redeclare non-static %s::$%s as static %s::$%s",
                    ancestor->name()->data(),
                    preProp->name()->data(),
                    m_preClass->name()->data(),
                    preProp->name()->data());
      }
      // Get parent's equivalent property, if one exists.
      SPropMap::Builder::iterator it3 = curSPropMap.find(preProp->name());
      Slot sPropInd = kInvalidSlot;
      // Prohibit strengthening.
      if (it3 != curSPropMap.end()) {
        const SProp& parentSProp = curSPropMap[it3->second];
        if ((preProp->attrs() & (AttrPublic|AttrProtected|AttrPrivate))
            > (parentSProp.m_attrs & (AttrPublic|AttrProtected|AttrPrivate))) {
          raise_error(
            "Access level to %s::$%s() must be %s (as in class %s) or weaker",
            m_preClass->name()->data(), preProp->name()->data(),
            attrToVisibilityStr(parentSProp.m_attrs),
            m_parent->name()->data());
        }
        sPropInd = it3->second;
      }
      // Create a new property, or overlay ancestor's property if one exists.
      if (sPropInd == kInvalidSlot) {
        SProp sProp;
        sProp.m_name = preProp->name();
        sPropInd = curSPropMap.size();
        curSPropMap.add(sProp.m_name, sProp);
      }
      SProp& sProp = curSPropMap[sPropInd];
      // Finish initializing.
      sProp.m_attrs = preProp->attrs();
      sProp.m_docComment = preProp->docComment();
      sProp.m_class = this;
      sProp.m_val = m_preClass->lookupProp(preProp->name())->val();
    }
  }

  importTraitProps(curPropMap, curSPropMap);

  m_declProperties.create(curPropMap);
  m_staticProperties.create(curSPropMap);

  m_declPropNumAccessible = m_declProperties.size() - numInaccessible;
}

bool Class::compatibleTraitPropInit(TypedValue& tv1, TypedValue& tv2) {
  if (tv1.m_type != tv2.m_type) return false;
  switch (tv1.m_type) {
    case KindOfNull: return true;
    case KindOfBoolean:
    case KindOfInt64:
    case KindOfDouble:
    case KindOfStaticString:
    case KindOfString:
      return same(tvAsVariant(&tv1), tvAsVariant(&tv2));
    default: return false;
  }
}

void Class::importTraitInstanceProp(ClassPtr    trait,
                                    Prop&       traitProp,
                                    TypedValue& traitPropVal,
                                    PropMap::Builder& curPropMap) {
  PropMap::Builder::iterator prevIt = curPropMap.find(traitProp.m_name);

  if (prevIt == curPropMap.end()) {
    // New prop, go ahead and add it
    Prop prop = traitProp;
    prop.m_class = this; // set current class as the first declaring prop
    // private props' mangled names contain the class name, so regenerate them
    if (prop.m_attrs & AttrPrivate) {
      prop.m_mangledName = manglePropName(m_preClass->name(), prop.m_name,
                                          prop.m_attrs);
    }
    curPropMap.add(prop.m_name, prop);
    m_declPropInit.push_back(traitPropVal);
  } else {
    // Redeclared prop, make sure it matches previous declarations
    Prop&       prevProp    = curPropMap[prevIt->second];
    TypedValue& prevPropVal = m_declPropInit[prevIt->second];
    if (prevProp.m_attrs != traitProp.m_attrs ||
        !compatibleTraitPropInit(prevPropVal, traitPropVal)) {
      raise_error("trait declaration of property '%s' is incompatible with "
                    "previous declaration", traitProp.m_name->data());
    }
  }
}

void Class::importTraitStaticProp(ClassPtr trait,
                                  SProp&   traitProp,
                                  PropMap::Builder& curPropMap,
                                  SPropMap::Builder& curSPropMap) {
  // Check if prop already declared as non-static
  if (curPropMap.find(traitProp.m_name) != curPropMap.end()) {
    raise_error("trait declaration of property '%s' is incompatible with "
                "previous declaration", traitProp.m_name->data());
  }

  SPropMap::Builder::iterator prevIt = curSPropMap.find(traitProp.m_name);
  if (prevIt == curSPropMap.end()) {
    // New prop, go ahead and add it
    SProp prop = traitProp;
    prop.m_class = this; // set current class as the first declaring prop
    curSPropMap.add(prop.m_name, prop);
  } else {
    // Redeclared prop, make sure it matches previous declaration
    SProp&     prevProp    = curSPropMap[prevIt->second];
    TypedValue prevPropVal;
    if (prevProp.m_class == this) {
      // If this static property was declared by this class, we can
      // get the initial value directly from m_val
      prevPropVal = prevProp.m_val;
    } else {
      // If this static property was declared in a parent class, m_val
      // will be KindOfUninit, and we'll need to consult the appropriate
      // parent class to get the initial value.
      prevPropVal = getStaticPropInitVal(prevProp);
    }
    if (prevProp.m_attrs != traitProp.m_attrs ||
        !compatibleTraitPropInit(traitProp.m_val, prevPropVal)) {
      raise_error("trait declaration of property '%s' is incompatible with "
                  "previous declaration", traitProp.m_name->data());
    }
    prevProp.m_class = this;
    prevProp.m_val   = prevPropVal;
  }
}

void Class::importTraitProps(PropMap::Builder& curPropMap,
                             SPropMap::Builder& curSPropMap) {
  if (attrs() & AttrNoExpandTrait) return;
  for (size_t t = 0; t < m_usedTraits.size(); t++) {
    ClassPtr trait = m_usedTraits[t];

    // instance properties
    for (Slot p = 0; p < trait->m_declProperties.size(); p++) {
      Prop&       traitProp    = trait->m_declProperties[p];
      TypedValue& traitPropVal = trait->m_declPropInit[p];
      importTraitInstanceProp(trait, traitProp, traitPropVal,
                              curPropMap);
    }

    // static properties
    for (Slot p = 0; p < trait->m_staticProperties.size(); ++p) {
      SProp& traitProp = trait->m_staticProperties[p];
      importTraitStaticProp(trait, traitProp, curPropMap,
                            curSPropMap);
    }
  }
}

void Class::addTraitPropInitializers(bool staticProps) {
  if (attrs() & AttrNoExpandTrait) return;
  for (unsigned t = 0; t < m_usedTraits.size(); t++) {
    ClassPtr trait = m_usedTraits[t];
    InitVec& traitInitVec = staticProps ? trait->m_sinitVec : trait->m_pinitVec;
    InitVec& thisInitVec  = staticProps ? m_sinitVec : m_pinitVec;
    // Insert trait's 86[ps]init into the current class, avoiding repetitions.
    for (unsigned m = 0; m < traitInitVec.size(); m++) {
      // Linear search, but these vectors shouldn't be big.
      if (find(thisInitVec.begin(), thisInitVec.end(), traitInitVec[m]) ==
          thisInitVec.end()) {
        thisInitVec.push_back(traitInitVec[m]);
      }
    }
  }
}

void Class::setInitializers() {
  if (m_parent.get() != NULL) {
    // Copy parent's 86pinit() vector, so that the 86pinit() methods can be
    // called in reverse order without any search/recursion during
    // initialization.
    m_pinitVec = m_parent->m_pinitVec;
  }

  // This class only has a __[ps]init() method if it's needed.  Append to the
  // vectors of __[ps]init() methods, so that reverse iteration of the vectors
  // runs this class's __[ps]init() first, in case multiple classes in the
  // hierarchy initialize the same property.
  const Func* meth86pinit = findSpecialMethod(this, sd86pinit);
  if (meth86pinit != NULL) {
    m_pinitVec.push_back(meth86pinit);
  }
  addTraitPropInitializers(false);
  const Func* sinit = findSpecialMethod(this, sd86sinit);
  if (sinit) {
    m_sinitVec.push_back(sinit);
  }
  addTraitPropInitializers(true);

  m_needInitialization = (m_pinitVec.size() > 0 ||
    m_staticProperties.size() > 0);

  // The __init__ method defined in the Exception class gets special treatment
  static StringData* sd__init__ = StringData::GetStaticString("__init__");
  static StringData* sd_exn = StringData::GetStaticString("Exception");
  const Func* einit = lookupMethod(sd__init__);
  m_callsCustomInstanceInit =
    (einit && einit->preClass()->name()->isame(sd_exn));
}

// Checks if interface methods are OK:
//  - there's no requirement if this is a trait, interface, or abstract class
//  - a non-abstract class must implement all methods from interfaces it
//    declares to implement (either directly or indirectly), arity must be
//    compatible (at least as many parameters, additional parameters must have
//    defaults), and typehints must be compatible
void Class::checkInterfaceMethods() {
  for (ClassSet::const_iterator it = m_allInterfaces.begin();
       it != m_allInterfaces.end(); it++) {
    const Class* iface = *it;

    for (size_t m = 0; m < iface->m_methods.size(); m++) {
      Func* imeth = iface->m_methods[m];
      const StringData* methName = imeth->name();

      // Skip special methods
      if (Func::isSpecial(methName)) continue;

      Func* meth = lookupMethod(methName);

      if (attrs() & (AttrTrait | AttrInterface | AttrAbstract)) {
        if (meth == NULL) {
          // Skip unimplemented method.
          continue;
        }
      } else {
        // Verify that method is not abstract within concrete class.
        if (meth == NULL || (meth->attrs() & AttrAbstract)) {
          raise_error("Class %s contains abstract method (%s) and "
                      "must therefore be declared abstract or implement "
                      "the remaining methods", name()->data(),
                      methName->data());
        }
      }
      bool ifaceStaticMethod = imeth->attrs() & AttrStatic;
      bool classStaticMethod = meth->attrs() & AttrStatic;
      if (classStaticMethod != ifaceStaticMethod) {
        raise_error("Cannot make %sstatic method %s::%s() %sstatic "
                    "in class %s",
                    ifaceStaticMethod ? "" : "non-",
                    iface->m_preClass->name()->data(), methName->data(),
                    classStaticMethod ? "" : "non-",
                    m_preClass->name()->data());
      }
      if ((imeth->attrs() & AttrPublic) &&
          !(meth->attrs() & AttrPublic)) {
        raise_error("Access level to %s::%s() must be public "
                    "(as in interface %s)", m_preClass->name()->data(),
                    methName->data(), iface->m_preClass->name()->data());
      }
      meth->parametersCompat(m_preClass.get(), imeth);
    }
  }
}

void Class::setInterfaces() {
  if (attrs() & AttrInterface) {
    m_allInterfaces.insert(this);
  }
  if (m_parent.get() != NULL) {
    m_allInterfaces.insert(m_parent->m_allInterfaces.begin(),
                           m_parent->m_allInterfaces.end());
  }
  for (PreClass::InterfaceVec::const_iterator it =
         m_preClass->interfaces().begin();
       it != m_preClass->interfaces().end(); ++it) {
    ClassPtr cp = Unit::loadClass(*it);
    if (cp.get() == NULL) {
      raise_error("Undefined interface: %s", (*it)->data());
    }
    if (!(cp->attrs() & AttrInterface)) {
      raise_error("%s cannot implement %s - it is not an interface",
                  m_preClass->name()->data(), cp->name()->data());
    }
    m_declInterfaces.push_back(cp);
    m_allInterfaces.insert(cp->m_allInterfaces.begin(),
                           cp->m_allInterfaces.end());
  }
  checkInterfaceMethods();
}

void Class::setUsedTraits() {
  for (PreClass::UsedTraitVec::const_iterator
       it = m_preClass->usedTraits().begin();
       it != m_preClass->usedTraits().end(); it++) {
    ClassPtr classPtr = Unit::loadClass(*it);
    if (classPtr.get() == NULL) {
      raise_error("Trait '%s' not found", (*it)->data());
    }
    if (!(classPtr->attrs() & AttrTrait)) {
      raise_error("%s cannot use %s - it is not a trait",
                  m_preClass->name()->data(),
                  classPtr->name()->data());
    }
    m_usedTraits.push_back(classPtr);
  }
}

void Class::setClassVec() {
  if (m_classVecLen > 1) {
    ASSERT(m_parent.get() != NULL);
    memcpy(m_classVec, m_parent.get()->m_classVec,
           (m_classVecLen-1) * sizeof(Class*));
  }
  m_classVec[m_classVecLen-1] = this;
}

// Finds the base class defining the given method (NULL if none).
// Note: for methods imported via traits, the base class is the one that
// uses/imports the trait.
Class* Class::findMethodBaseClass(const StringData* methName) {
  const Func* f = lookupMethod(methName);
  if (f == NULL) return NULL;
  return f->baseCls();
}

void Class::getMethodNames(const Class* ctx, HphpArray* methods) const {
  Func* const* pcMethods = m_preClass->methods();
  for (size_t i = 0, sz = m_preClass->numMethods(); i < sz; i++) {
    Func* func = pcMethods[i];
    if (isdigit(func->name()->data()[0])) continue;
    if (!(func->attrs() & AttrPublic)) {
      if (!ctx) continue;
      if (ctx != this) {
        if (func->attrs() & AttrPrivate) continue;
        func = lookupMethod(func->name());
        if (!ctx->classof(func->baseCls()) &&
            !func->baseCls()->classof(ctx)) {
          continue;
        }
      }
    }
    methods->nvSet(const_cast<StringData*>(func->name()),
                   (TypedValue*)&true_varNR, false);
  }
  if (m_parent.get()) m_parent.get()->getMethodNames(ctx, methods);
  for (int i = 0, sz = m_declInterfaces.size(); i < sz; i++) {
    m_declInterfaces[i].get()->getMethodNames(ctx, methods);
  }
}

// Returns true iff this class declared the given method.
// For trait methods, the class declaring them is the one that uses/imports
// the trait.
bool Class::declaredMethod(const Func* method) {
  if (method->preClass()->attrs() & AttrTrait) {
    return findMethodBaseClass(method->name()) == this;
  }
  return method->preClass() == m_preClass.get();
}

void Class::getClassInfo(ClassInfoVM* ci) {
  ASSERT(ci);

  // Miscellaneous.
  Attr clsAttrs = attrs();
  int attr = 0;
  if (clsAttrs & AttrInterface) attr |= ClassInfo::IsInterface;
  if (clsAttrs & AttrAbstract)  attr |= ClassInfo::IsAbstract;
  if (clsAttrs & AttrFinal)     attr |= ClassInfo::IsFinal;
  if (clsAttrs & AttrTrait)     attr |= ClassInfo::IsTrait;
  if (attr == 0)                attr  = ClassInfo::IsNothing;
  ci->m_attribute = (ClassInfo::Attribute)attr;

  ci->m_name = m_preClass->name()->data();

  ci->m_file = m_preClass->unit()->filepath()->data();
  ci->m_line1 = m_preClass->line1();
  ci->m_line2 = m_preClass->line2();
  ci->m_docComment = (m_preClass->docComment() != NULL)
                     ? m_preClass->docComment()->data() : "";

  // Parent class.
  if (m_parent.get()) {
    ci->m_parentClass = m_parent->name()->data();
  } else {
    ci->m_parentClass = "";
  }

  // Interfaces.
  for (unsigned i = 0; i < m_declInterfaces.size(); i++) {
    ci->m_interfacesVec.push_back(
        m_declInterfaces[i]->name()->data());
    ci->m_interfaces.insert(
        m_declInterfaces[i]->name()->data());
  }

  // Used traits.
  for (unsigned t = 0; t < m_usedTraits.size(); t++) {
    const char* traitName = m_usedTraits[t]->name()->data();
    ci->m_traitsVec.push_back(traitName);
    ci->m_traits.insert(traitName);
  }

  // Trait aliases.
  for (unsigned a = 0; a < m_traitAliases.size(); a++) {
    ci->m_traitAliasesVec.push_back(std::pair<String, String>
                                    (m_traitAliases[a].first->data(),
                                     m_traitAliases[a].second->data()));
  }

#define SET_FUNCINFO_BODY                                       \
  ClassInfo::MethodInfo *m = new ClassInfo::MethodInfo;         \
  func->getFuncInfo(m);                                         \
  ci->m_methods[func->name()->data()] = m;                      \
  ci->m_methodsVec.push_back(m);

  // Methods: in source order (from our PreClass), then traits.
  for (size_t i = 0; i < m_preClass->numMethods(); ++i) {
    const StringData* name = m_preClass->methods()[i]->name();
    // Filter out special methods
    if (isdigit(name->data()[0])) continue;
    Func* func = lookupMethod(m_preClass->methods()[i]->name());
    ASSERT(func);
    ASSERT(declaredMethod(func));
    SET_FUNCINFO_BODY;
  }

  for (Slot i = m_traitsBeginIdx; i < m_traitsEndIdx; ++i) {
    Func* func = m_methods[i];
    ASSERT(func);
    if (!isdigit(func->name()->data()[0])) {
      SET_FUNCINFO_BODY;
    }
  }
#undef SET_FUNCINFO_BODY

  // Properties.
  for (Slot i = 0; i < m_declProperties.size(); ++i) {
    if (m_declProperties[i].m_class != this) continue;
    ClassInfo::PropertyInfo *pi = new ClassInfo::PropertyInfo;
    pi->owner = ci;
    pi->name = m_declProperties[i].m_name->data();
    Attr propAttrs = m_declProperties[i].m_attrs;
    attr = 0;
    if (propAttrs & AttrProtected) attr |= ClassInfo::IsProtected;
    if (propAttrs & AttrPrivate) attr |= ClassInfo::IsPrivate;
    if (attr == 0) attr |= ClassInfo::IsPublic;
    if (propAttrs & AttrStatic) attr |= ClassInfo::IsStatic;
    pi->attribute = (ClassInfo::Attribute)attr;
    pi->docComment = (m_declProperties[i].m_docComment != NULL)
                     ? m_declProperties[i].m_docComment->data() : "";

    ci->m_properties[pi->name] = pi;
    ci->m_propertiesVec.push_back(pi);
  }

  for (Slot i = 0; i < m_staticProperties.size(); ++i) {
    if (m_staticProperties[i].m_class != this) continue;
    ClassInfo::PropertyInfo *pi = new ClassInfo::PropertyInfo;
    pi->owner = ci;
    pi->name = m_staticProperties[i].m_name->data();
    Attr propAttrs = m_staticProperties[i].m_attrs;
    attr = 0;
    if (propAttrs & AttrProtected) attr |= ClassInfo::IsProtected;
    if (propAttrs & AttrPrivate) attr |= ClassInfo::IsPrivate;
    if (attr == 0) attr |= ClassInfo::IsPublic;
    if (propAttrs & AttrStatic) attr |= ClassInfo::IsStatic;
    pi->attribute = (ClassInfo::Attribute)attr;
    pi->docComment = (m_staticProperties[i].m_docComment != NULL)
                     ? m_staticProperties[i].m_docComment->data() : "";

    ci->m_properties[pi->name] = pi;
    ci->m_propertiesVec.push_back(pi);
  }

  // Constants.
  for (Slot i = 0; i < m_constants.size(); ++i) {
    // Only include constants declared on this class
    if (m_constants[i].m_class != this) continue;

    ClassInfo::ConstantInfo *ki = new ClassInfo::ConstantInfo;
    ki->name = m_constants[i].m_name->data();
    ki->valueLen = m_constants[i].m_phpCode->size();
    ki->valueText = m_constants[i].m_phpCode->data();
    ki->setValue(tvAsCVarRef(clsCnsGet(m_constants[i].m_name)));

    ci->m_constants[ki->name] = ki;
    ci->m_constantsVec.push_back(ki);
  }
}

Class::PropInitVec::~PropInitVec() {
  if (!m_smart) free(m_data);
}

Class::PropInitVec::PropInitVec() : m_data(NULL), m_size(0), m_smart(false) {}

Class::PropInitVec*
Class::PropInitVec::allocInRequestArena(const PropInitVec& src) {
  ThreadInfo* info UNUSED = ThreadInfo::s_threadInfo.getNoCheck();
  PropInitVec* p = new (request_arena()) PropInitVec;
  p->m_size = src.size();
  p->m_data = new (request_arena()) TypedValue[src.size()];
  memcpy(p->m_data, src.m_data, src.size() * sizeof(*p->m_data));
  p->m_smart = true;
  return p;
}

const Class::PropInitVec&
Class::PropInitVec::operator=(const PropInitVec& piv) {
  ASSERT(!m_smart);
  if (this != &piv) {
    unsigned sz = m_size = piv.size();
    if (sz) sz = Util::roundUpToPowerOfTwo(sz);
    free(m_data);
    m_data = (TypedValue*)malloc(sz * sizeof(*m_data));
    ASSERT(m_data);
    memcpy(m_data, piv.m_data, piv.size() * sizeof(*m_data));
  }
  return *this;
}

void Class::PropInitVec::push_back(const TypedValue& v) {
  ASSERT(!m_smart);
  /*
   * the allocated size is always the next power of two (or zero)
   * so we just need to reallocate when we hit a power of two
   */
  if (!m_size || Util::isPowerOfTwo(m_size)) {
    unsigned size = m_size ? m_size * 2 : 1;
    m_data = (TypedValue*)realloc(m_data, size * sizeof(*m_data));
    ASSERT(m_data);
  }
  tvDup(&v, &m_data[m_size++]);
}

using Transl::TargetCache::handleToRef;

const Class::PropInitVec* Class::getPropData() const {
  if (m_propDataCache == (unsigned)-1) return NULL;
  return handleToRef<PropInitVec*>(m_propDataCache);
}

void Class::setPropData(PropInitVec* propData) const {
  ASSERT(getPropData() == NULL);
  if (UNLIKELY(m_propDataCache == (unsigned)-1)) {
    const_cast<unsigned&>(m_propDataCache) =
      Transl::TargetCache::allocClassInitProp(name());
  }
  handleToRef<PropInitVec*>(m_propDataCache) = propData;
}

TypedValue* Class::getSPropData() const {
  if (m_propSDataCache == (unsigned)-1) return NULL;
  return handleToRef<TypedValue*>(m_propSDataCache);
}

void Class::setSPropData(TypedValue* sPropData) const {
  ASSERT(getSPropData() == NULL);
  if (UNLIKELY(m_propSDataCache == (unsigned)-1)) {
    const_cast<unsigned&>(m_propSDataCache) =
      Transl::TargetCache::allocClassInitSProp(name());
  }
  handleToRef<TypedValue*>(m_propSDataCache) = sPropData;
}

} } // HPHP::VM
