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

#include <sys/mman.h>

#include <iostream>
#include <iomanip>
#include <tbb/concurrent_unordered_map.h>

#include <util/lock.h>
#include <runtime/ext/ext_variable.h>
#include <runtime/vm/bytecode.h>
#include <runtime/vm/repo.h>
#include <runtime/vm/translator/targetcache.h>
#include <runtime/vm/vm.h>
#include <runtime/vm/translator/translator-deps.h>
#include <runtime/vm/translator/translator-inline.h>
#include <runtime/vm/translator/translator-x64.h>

namespace HPHP {
namespace VM {
///////////////////////////////////////////////////////////////////////////////

static const Trace::Module TRACEMOD = Trace::hhbc;

Mutex Unit::s_classesMutex;
/*
 * We hold onto references to elements of this map. If we use a different
 * map, we must use one that doesnt invalidate references to its elements
 * (unless they are deleted, which never happens here). Any standard
 * associative container will meet this requirement.
 */
typedef tbb::concurrent_unordered_map<const StringData *, NamedEntity,
                                      string_data_hash,
                                      string_data_isame> NamedEntityMap;

static NamedEntityMap *s_namedDataMap;

const NamedEntity* Unit::GetNamedEntity(const StringData *str) {
  if (!s_namedDataMap) s_namedDataMap = new NamedEntityMap();
  NamedEntityMap::const_iterator it = s_namedDataMap->find(str);
  if (it != s_namedDataMap->end()) return &it->second;

  if (!str->isStatic()) {
    str = StringData::GetStaticString(str);
  }

  return &(*s_namedDataMap)[str];
}

void NamedEntity::setCachedFunc(Func* f) {
  *(Func**)Transl::TargetCache::handleToPtr(m_cachedFuncOffset) = f;
}

Func* NamedEntity::getCachedFunc() const {
  if (LIKELY(m_cachedFuncOffset != 0)) {
    return *(Func**)Transl::TargetCache::handleToPtr(m_cachedFuncOffset);
  }
  return NULL;
}

Array Unit::getUserFunctions() {
  // Return an array of all defined functions.  This method is used
  // to support get_defined_functions().
  Array a = Array::Create();
  if (s_namedDataMap) {
    for (NamedEntityMap::const_iterator it = s_namedDataMap->begin();
         it != s_namedDataMap->end(); ++it) {
      Func* func_ = it->second.getCachedFunc();
      if (!func_ || func_->isBuiltin()) continue;
      a.append(func_->nameRef());
    }
  }
  return a;
}

Array Unit::getClassesInfo() {
  // Return an array of all defined class names.  This method is used to
  // support get_declared_classes().
  Array a = Array::Create();
  if (s_namedDataMap) {
    for (NamedEntityMap::const_iterator it = s_namedDataMap->begin();
         it != s_namedDataMap->end(); ++it) {
      Class* class_ = *it->second.clsList();
      if (!class_) continue;
      class_ = class_->getCached();
      if (class_ &&
          !(class_->attrs() & (AttrInterface | AttrTrait))) {
        a.append(class_->nameRef());
      }
    }
  }
  return a;
}

Array Unit::getInterfacesInfo() {
  // Return an array of all defined interface names.  This method is used to
  // support get_declared_interfaces().
  Array a = Array::Create();
  if (s_namedDataMap) {
    for (NamedEntityMap::const_iterator it = s_namedDataMap->begin();
         it != s_namedDataMap->end(); ++it) {
      Class* class_ = *it->second.clsList();
      if (!class_) continue;
      class_ = class_->getCached();
      if (class_ &&
          class_->attrs() & AttrInterface) {
        a.append(class_->nameRef());
      }
    }
  }
  return a;
}

Array Unit::getTraitsInfo() {
  // Returns an array with all defined trait names.  This method is used to
  // support get_declared_traits().
  Array array = Array::Create();
  if (s_namedDataMap) {
    for (NamedEntityMap::const_iterator it = s_namedDataMap->begin();
         it != s_namedDataMap->end(); it++) {
      Class* class_ = *it->second.clsList();
      if (!class_) continue;
      class_ = class_->getCached();
      if (class_ &&
          class_->attrs() & AttrTrait) {
        array.append(class_->nameRef());
      }
    }
  }
  return array;
}

bool Unit::MetaHandle::findMeta(const Unit* unit, Offset offset) {
  if (!unit->m_bc_meta_len) return false;
  ASSERT(unit->m_bc_meta);
  Offset* index1 = (Offset*)unit->m_bc_meta;
  Offset* index2 = index1 + *index1 + 1;

  ASSERT(index1[*index1 + 1] == INT_MAX); // sentinel
  ASSERT(offset >= 0 && (unsigned)offset < unit->m_bclen);
  ASSERT(cur == 0 || index == index1);
  if (cur && offset >= index[cur]) {
    while (offset >= index[cur+1]) cur++;
  } else {
    int hi = *index1 + 2;
    int lo = 1;
    while (hi - lo > 1) {
      int mid = hi + lo >> 1;
      if (offset >= index1[mid]) {
        lo = mid;
      } else {
        hi = mid;
      }
    }
    index = index1;
    cur = lo;
  }
  ASSERT(cur <= (unsigned)*index1);
  ASSERT((unsigned)index2[cur] <= unit->m_bc_meta_len);
  ptr = unit->m_bc_meta + index2[cur];
  return index[cur] == offset;
}

bool Unit::MetaHandle::nextArg(MetaInfo& info) {
  ASSERT(index && cur && ptr);
  uint8* end = (uint8*)index + index[*index + cur + 2];
  ASSERT(ptr <= end);
  if (ptr == end) return false;
  info.m_kind = (Unit::MetaInfo::Kind)*ptr++;
  info.m_arg = *ptr++;
  info.m_data = decodeVariableSizeImm(&ptr);
  return true;
}

//=============================================================================
// Unit.

Unit::Unit()
    : m_main(NULL), m_mainAttrs(AttrNone),
      m_repoId(-1), m_sn(-1), m_bc(NULL), m_bclen(0),
      m_bc_meta(NULL), m_bc_meta_len(0), m_filepath(NULL),
      m_dirpath(NULL), m_md5(), m_preConstsMerged(false),
      m_preConstsLock(false /* reentrant */, RankUnitPreConst) {
}

Unit::~Unit() {
  if (debug) {
    // poison released bytecode
    memset(m_bc, 0xff, m_bclen);
  }
  free(m_bc);
  free(m_bc_meta);

  // Delete all Func's.
  for (FuncVec::const_iterator it = m_funcs.begin(); it != m_funcs.end();
       ++it) {
    delete *it;
  }

  // ExecutionContext and the TC may retain references to Class'es, so
  // it is possible for Class'es to outlive their Unit.
  for (int i = m_preClasses.size(); i--; ) {
    PreClass* pcls = m_preClasses[i].get();
    Class * const* clsh = pcls->namedEntity()->clsList();
    if (clsh) {
      Class *cls = *clsh;
      while (cls) {
        Class* cur = cls;
        cls = cls->m_nextClass;
        if (cur->preClass() == pcls) {
          if (!cur->decAtomicCount()) {
            cur->atomicRelease();
          }
        }
      }
    }
  }

  if (!RuntimeOption::RepoAuthoritative && m_preConstsMerged) {
    Transl::unmergePreConsts(m_preConsts, this);
  }
}

bool Unit::compileTimeFatal(const StringData*& msg, int& line) const {
  // A compile-time fatal is encoded as a pseudomain that contains precisely:
  //
  //   String <id>; Fatal;
  //
  // Decode enough of pseudomain to determine whether it contains a
  // compile-time fatal, and if so, extract the error message and line number.
  const Opcode* entry = getMain()->getEntry();
  const Opcode* pc = entry;
  // String <id>; Fatal;
  // ^^^^^^
  if (*pc != OpString) {
    return false;
  }
  pc++;
  // String <id>; Fatal;
  //        ^^^^
  Id id = *(Id*)pc;
  pc += sizeof(Id);
  // String <id>; Fatal;
  //              ^^^^^
  if (*pc != OpFatal) {
    return false;
  }
  msg = lookupLitstrId(id);
  line = getLineNumber(Offset(pc - entry));
  return true;
}

Class* Unit::defClass(PreClass* preClass,
                      bool failIsFatal /* = true */) {
  Class*const* clsList = preClass->namedEntity()->clsList();
  Class* top = *clsList;
  if (top) {
    Class *cls = top->getCached();
    if (cls) {
      // Raise a fatal unless the existing class definition is identical to the
      // one this invocation would create.
      if (cls->preClass() != preClass) {
        if (failIsFatal) {
          raise_error("Class already declared: %s", preClass->name()->data());
        }
        return NULL;
      }
      return cls;
    }
  }
  // Get a compatible Class, and add it to the list of defined classes.

  Class* parent = NULL;
  for (;;) {
    // Search for a compatible extant class.  Searching from most to least
    // recently created may have better locality than alternative search orders.
    // In addition, its the only simple way to make this work lock free...
    for (Class* class_ = top; class_ != NULL; class_ = class_->m_nextClass) {
      if (class_->preClass() != preClass) continue;

      Class::Avail avail = class_->avail(parent, failIsFatal /*tryAutoload*/);
      if (LIKELY(avail == Class::AvailTrue)) {
        class_->setCached();
        DEBUGGER_ATTACHED_ONLY(phpDefClassHook(class_));
        return class_;
      }
      if (avail == Class::AvailFail) {
        if (failIsFatal) {
          raise_error("unknown class %s", parent->name()->data());
        }
        return NULL;
      }
      ASSERT(avail == Class::AvailFalse);
    }

    // Create a new class.
    if (!parent && preClass->parent()->size() != 0) {
      parent = Unit::getClass(preClass->parent(), failIsFatal);
      if (parent == NULL) {
        if (failIsFatal) {
          raise_error("unknown class %s", preClass->parent()->data());
        }
        return NULL;
      }
    }

    ClassPtr newClass(Class::newClass(preClass, parent, failIsFatal));
    if (!newClass.get()) {
      ASSERT(!failIsFatal);
      return NULL;
    }

    Lock l(Unit::s_classesMutex);
    /*
      We could re-enter via Unit::getClass() or class_->avail(), so
      no need for *clsList to be volatile
    */
    if (UNLIKELY(top != *clsList)) {
      top = *clsList;
      continue;
    }
    if (top) {
      newClass->m_cachedOffset = top->m_cachedOffset;
    } else {
      newClass->m_cachedOffset =
        Transl::TargetCache::allocKnownClass(preClass->name());
    }
    newClass->m_nextClass = top;
    Util::compiler_membar();
    *const_cast<Class**>(clsList) = newClass.get();
    newClass.get()->incAtomicCount();
    newClass.get()->setCached();
    DEBUGGER_ATTACHED_ONLY(phpDefClassHook(newClass.get()));
    return newClass.get();
  }
}

void Unit::renameFunc(const StringData* oldName, const StringData* newName) {
  // renameFunc() should only be used by VMExecutionContext::createFunction.
  // We do a linear scan over all the functions in the unit searching for the
  // func with a given name; in practice this is okay because the units created
  // by create_function() will always have the function being renamed at the
  // beginning of m_hoistableFuncs.
  ASSERT(oldName && oldName->isStatic());
  ASSERT(newName && newName->isStatic());
  for (FuncVec::iterator it = m_hoistableFuncs.begin();
       it != m_hoistableFuncs.end(); ++it) {
    Func* func = *it;
    const StringData* name = func->name();
    ASSERT(name);
    if (name->same(oldName)) {
      func->rename(newName);
      break;
    }
  }
}

Class* Unit::loadClass(const NamedEntity* ne,
                       const StringData* name) {
  Class *cls = *ne->clsList();
  if (LIKELY(cls != NULL)) {
    cls = cls->getCached();
    if (LIKELY(cls != NULL)) return cls;
  }
  VMRegAnchor _;
  AutoloadHandler::s_instance->invokeHandler(name->data());
  return Unit::lookupClass(ne);
}

Class* Unit::loadMissingClass(const NamedEntity* ne,
                              const StringData *name) {
  AutoloadHandler::s_instance->invokeHandler(name->data());
  return Unit::lookupClass(ne);
}

Class* Unit::getClass(const NamedEntity* ne,
                      const StringData *name, bool tryAutoload) {
  Class *cls = lookupClass(ne);
  if (UNLIKELY(!cls && tryAutoload)) {
    return loadMissingClass(ne, name);
  }
  return cls;
}

bool Unit::classExists(const StringData* name, bool autoload, Attr typeAttrs) {
  Class* cls = Unit::getClass(name, autoload);
  return cls && (cls->attrs() & (AttrInterface | AttrTrait)) == typeAttrs;
}

void Unit::mergeFuncs() const {
  for (FuncVec::const_iterator it = m_hoistableFuncs.begin();
       it != m_hoistableFuncs.end(); ++it) {
    (*it)->setCached();
  }
}

void Unit::loadFunc(Func *func) {
  ASSERT(!func->isMethod());
  const NamedEntity *ne = func->getNamedEntity();
  if (UNLIKELY(!ne->m_cachedFuncOffset)) {
    const_cast<NamedEntity*>(ne)->m_cachedFuncOffset =
      Transl::TargetCache::allocFixedFunction(func->name());
  }
  func->m_cachedOffset = ne->m_cachedFuncOffset;
}

/*
  Attempt to instantiate the hoistable classes.

  returns true iff every hoistable class is instantiated
*/
bool Unit::mergeClasses() const {
  bool ret = true;
  for (PreClassVec::const_iterator it = m_hoistablePreClasses.begin();
       it != m_hoistablePreClasses.end(); ++it) {
    if (!defClass(*it, false)) ret = false;
  }
  return ret;
}

void Unit::mergePreConstsWork() {
  ASSERT(!RuntimeOption::RepoAuthoritative);
  SimpleLock lock(m_preConstsLock);
  if (m_preConstsMerged) return;
  Transl::mergePreConsts(m_preConsts);
  atomic_release_store(&m_preConstsMerged, true);
}

int Unit::getLineNumber(Offset pc) const {
  LineEntry key = LineEntry(pc, -1);
  std::vector<LineEntry>::const_iterator it =
    upper_bound(m_lineTable.begin(), m_lineTable.end(), key);
  if (it != m_lineTable.end()) {
    ASSERT(pc < it->pastOffset());
    return it->val();
  }
  return -1;
}

bool Unit::getSourceLoc(Offset pc, SourceLoc& sLoc) const {
  if (m_repoId == RepoIdInvalid) {
    return false;
  }
  return !Repo::get().urp().getSourceLoc(m_repoId).get(m_sn, pc, sLoc);
}

bool Unit::getOffsetRanges(int line, OffsetRangeVec& offsets) const {
  ASSERT(offsets.size() == 0);
  if (m_repoId == RepoIdInvalid) {
    return false;
  }
  UnitRepoProxy& urp = Repo::get().urp();
  if (urp.getSourceLocPastOffsets(m_repoId).get(m_sn, line, offsets)) {
    return false;
  }
  for (OffsetRangeVec::iterator it = offsets.begin(); it != offsets.end();
       ++it) {
    if (urp.getSourceLocBaseOffset(m_repoId).get(m_sn, *it)) {
      return false;
    }
  }
  return true;
}

bool Unit::getOffsetRange(Offset pc, OffsetRange& range) const {
  if (m_repoId == RepoIdInvalid) {
    return false;
  }
  UnitRepoProxy& urp = Repo::get().urp();
  if (urp.getBaseOffsetAtPCLoc(m_repoId).get(m_sn, pc, range.m_base) ||
      urp.getBaseOffsetAfterPCLoc(m_repoId).get(m_sn, pc, range.m_past)) {
    return false;
  }
  return true;
}

const Func* Unit::getFunc(Offset pc) const {
  FuncEntry key = FuncEntry(pc, NULL);
  FuncTable::const_iterator it =
    upper_bound(m_funcTable.begin(), m_funcTable.end(), key);
  if (it != m_funcTable.end()) {
    ASSERT(pc < it->pastOffset());
    return it->val();
  }
  return NULL;
}

void Unit::prettyPrint(std::ostream &out, size_t startOffset,
                       size_t stopOffset) const {
  std::map<Offset,const Func*> funcMap;
  for (FuncVec::const_iterator it = m_funcs.begin(); it != m_funcs.end();
       ++it) {
    funcMap[(*it)->base()] = *it;
  }
  for (PreClassPtrVec::const_iterator it = m_preClasses.begin();
      it != m_preClasses.end(); ++it) {
    const PreClass::MethodVec& methods = (*it)->methods();
    for (PreClass::MethodVec::const_iterator it = methods.begin();
        it != methods.end();
        ++it) {
      funcMap[(*it)->base()] = *it;
    }
  }

  std::map<Offset,const Func*>::const_iterator funcIt =
    funcMap.lower_bound(startOffset);

  const uchar* it = &m_bc[startOffset];
  int prevLineNum = -1;
  while (it < &m_bc[stopOffset]) {
    ASSERT(funcIt == funcMap.end() || funcIt->first >= offsetOf(it));
    if (funcIt != funcMap.end() && funcIt->first == offsetOf(it)) {
      out.put('\n');
      funcIt->second->prettyPrint(out);
      ++funcIt;
    }

    int lineNum = getLineNumber(offsetOf(it));
    if (lineNum != prevLineNum) {
      out << "  // line " << lineNum << std::endl;
      prevLineNum = lineNum;
    }

    out << "  " << std::setw(4) << (it - m_bc) << ": ";
    out << instrToString((Opcode*)it, (Unit*)this) << std::endl;
    it += instrLen((Opcode*)it);
  }
}

void Unit::prettyPrint(std::ostream &out) const {
  prettyPrint(out, 0, m_bclen);
}

std::string Unit::toString() const {
  std::ostringstream ss;
  prettyPrint(ss);
  for (PreClassPtrVec::const_iterator it = m_preClasses.begin();
      it != m_preClasses.end(); ++it) {
    (*it).get()->prettyPrint(ss);
  }
  for (FuncVec::const_iterator it = m_funcs.begin(); it != m_funcs.end();
       ++it) {
    (*it)->prettyPrint(ss);
  }
  return ss.str();
}

void Unit::dumpUnit(Unit* u) {
  std::cerr << u->toString();
}

void Unit::enableIntercepts() {
  TranslatorX64* tx64 = TranslatorX64::Get();
  // Its ok to set maybeIntercepted(), because
  // we are protected by s_mutex in intercept.cpp
  for (unsigned i = 0; i < m_funcs.size(); i++) {
    Func *func = m_funcs[i];
    if (func->isPseudoMain()) {
      ASSERT(func == m_main);
      // pseudomain's can't be intercepted
      continue;
    }
    if (func->maybeIntercepted() == -1) {
      continue;
    }
    func->maybeIntercepted() = -1;
    tx64->interceptPrologues(func);
  }

  {
    Lock lock(s_classesMutex);
    for (int i = m_preClasses.size(); i--; ) {
      PreClass* pcls = m_preClasses[i].get();
      Class *cls = *pcls->namedEntity()->clsList();
      while (cls) {
        size_t numFuncs = cls->numMethods();
        Func* const* funcs = cls->methods();
        for (unsigned i = 0; i < numFuncs; i++) {
          if (funcs[i]->maybeIntercepted() == -1) {
            continue;
          }
          funcs[i]->maybeIntercepted() = -1;
          tx64->interceptPrologues(funcs[i]);
        }
        cls = cls->m_nextClass;
      }
    }
  }
}

Func *Unit::lookupFunc(const NamedEntity *ne, const StringData* name) {
  Func *func = ne->getCachedFunc();
  return func;
}

Func *Unit::lookupFunc(const StringData *funcName) {
  const NamedEntity *ne = GetNamedEntity(funcName);
  Func *func = ne->getCachedFunc();
  return func;
}

//=============================================================================
// UnitRepoProxy.

UnitRepoProxy::UnitRepoProxy(Repo& repo)
  : RepoProxy(repo),
#define URP_OP(c, o) \
    m_##o##Local(repo, RepoIdLocal), m_##o##Central(repo, RepoIdCentral),
    URP_OPS
#undef URP_OP
    m_dummy(0) {
#define URP_OP(c, o) \
  m_##o[RepoIdLocal] = &m_##o##Local; \
  m_##o[RepoIdCentral] = &m_##o##Central;
  URP_OPS
#undef URP_OP
}

UnitRepoProxy::~UnitRepoProxy() {
}

void UnitRepoProxy::createSchema(int repoId, RepoTxn& txn) {
  {
    std::stringstream ssCreate;
    ssCreate << "CREATE TABLE " << m_repo.table(repoId, "Unit")
             << "(unitSn INTEGER PRIMARY KEY, md5 BLOB, bc BLOB,"
                " bc_meta BLOB, lines BLOB, UNIQUE (md5));";
    txn.exec(ssCreate.str());
  }
  {
    std::stringstream ssCreate;
    ssCreate << "CREATE TABLE " << m_repo.table(repoId, "UnitLitstr")
             << "(unitSn INTEGER, litstrId INTEGER, litstr TEXT,"
                " PRIMARY KEY (unitSn, litstrId));";
    txn.exec(ssCreate.str());
  }
  {
    std::stringstream ssCreate;
    ssCreate << "CREATE TABLE " << m_repo.table(repoId, "UnitArray")
             << "(unitSn INTEGER, arrayId INTEGER, array BLOB,"
                " PRIMARY KEY (unitSn, arrayId));";
    txn.exec(ssCreate.str());
  }
  {
    std::stringstream ssCreate;
    ssCreate << "CREATE TABLE " << m_repo.table(repoId, "UnitSourceLoc")
             << "(unitSn INTEGER, pastOffset INTEGER, line0 INTEGER,"
                " char0 INTEGER, line1 INTEGER, char1 INTEGER,"
                " PRIMARY KEY (unitSn, pastOffset));";
    txn.exec(ssCreate.str());
  }
  {
    std::stringstream ssCreate;
    ssCreate << "CREATE TABLE " << m_repo.table(repoId, "UnitPreConst")
             << "(unitSn INTEGER, name TEXT, value BLOB, preConstId INTEGER,"
                " PRIMARY KEY (unitSn, preConstId));";
    txn.exec(ssCreate.str());
  }
}

Unit* UnitRepoProxy::load(const std::string& name, const MD5& md5) {
  UnitEmitter ue(md5);
  ue.setFilepath(StringData::GetStaticString(name));
  // Look for a repo that contains a unit with matching MD5.
  int repoId;
  for (repoId = RepoIdCount - 1; repoId >= 0; --repoId) {
    if (!getUnit(repoId).get(ue, md5)) {
      break;
    }
  }
  if (repoId < 0) {
    TRACE(3, "No repo contains '%s' (0x%016llx%016llx)\n",
             name.c_str(), md5.q[0], md5.q[1]);
    return NULL;
  }
  try {
    getUnitLitstrs(repoId).get(ue);
    getUnitArrays(repoId).get(ue);
    getUnitPreConsts(repoId).get(ue);
    m_repo.pcrp().getPreClasses(repoId).get(ue);
    m_repo.frp().getFuncs(repoId).get(ue);
  } catch (RepoExc& re) {
    TRACE(0, "Repo error loading '%s' (0x%016llx%016llx) from '%s': %s\n",
             name.c_str(), md5.q[0], md5.q[1], m_repo.repoName(repoId).c_str(),
             re.msg().c_str());
    return NULL;
  }
  TRACE(3, "Repo loaded '%s' (0x%016llx%016llx) from '%s'\n",
           name.c_str(), md5.q[0], md5.q[1], m_repo.repoName(repoId).c_str());
  return ue.create();
}

void UnitRepoProxy::InsertUnitStmt
                  ::insert(RepoTxn& txn, int64& unitSn, const MD5& md5,
                           const uchar* bc, size_t bclen,
                           const uchar* bc_meta, size_t bc_meta_len,
                           const LineTable& lines) {
  if (!prepared()) {
    std::stringstream ssInsert;
    ssInsert << "INSERT INTO " << m_repo.table(m_repoId, "Unit")
             << " VALUES(NULL, @md5, @bc, @bc_meta, @lines);";
    txn.prepare(*this, ssInsert.str());
  }
  RepoTxnQuery query(txn, *this);
  query.bindMd5("@md5", md5);
  query.bindBlob("@bc", (const void*)bc, bclen);
  query.bindBlob("@bc_meta",
                 bc_meta_len ? (const void*)bc_meta : (const void*)"",
                 bc_meta_len);
  query.bindBlob("@lines", (const void*)&lines[0],
                 lines.size() * sizeof(LineEntry));
  query.exec();
  unitSn = query.getInsertedRowid();
}

bool UnitRepoProxy::GetUnitStmt
                  ::get(UnitEmitter& ue, const MD5& md5) {
  try {
    RepoTxn txn(m_repo);
    if (!prepared()) {
      std::stringstream ssSelect;
      ssSelect << "SELECT unitSn,bc,bc_meta,lines FROM "
               << m_repo.table(m_repoId, "Unit")
               << " WHERE md5 == @md5;";
      txn.prepare(*this, ssSelect.str());
    }
    RepoTxnQuery query(txn, *this);
    query.bindMd5("@md5", md5);
    query.step();
    if (!query.row()) {
      return true;
    }
    int64 unitSn;                            /**/ query.getInt64(0, unitSn);
    const void* bc; size_t bclen;            /**/ query.getBlob(1, bc, bclen);
    const void* bc_meta; size_t bc_meta_len; /**/ query.getBlob(2, bc_meta,
                                                                bc_meta_len);
    const void* lines; size_t lineslen;      /**/ query.getBlob(3, lines,
                                                                lineslen);
    ue.setRepoId(m_repoId);
    ue.setSn(unitSn);
    ue.setBc((const uchar*)bc, bclen);
    ue.setBcMeta((const uchar*)bc_meta, bc_meta_len);
    ue.setLines((const LineEntry*)lines, lineslen / sizeof(LineEntry));

    txn.commit();
  } catch (RepoExc& re) {
    return true;
  }
  return false;
}

void UnitRepoProxy::InsertUnitLitstrStmt
                  ::insert(RepoTxn& txn, int64 unitSn, Id litstrId,
                           const StringData* litstr) {
  if (!prepared()) {
    std::stringstream ssInsert;
    ssInsert << "INSERT INTO " << m_repo.table(m_repoId, "UnitLitstr")
             << " VALUES(@unitSn, @litstrId, @litstr);";
    txn.prepare(*this, ssInsert.str());
  }
  RepoTxnQuery query(txn, *this);
  query.bindInt64("@unitSn", unitSn);
  query.bindId("@litstrId", litstrId);
  query.bindStaticString("@litstr", litstr);
  query.exec();
}

void UnitRepoProxy::GetUnitLitstrsStmt
                  ::get(UnitEmitter& ue) {
  RepoTxn txn(m_repo);
  if (!prepared()) {
    std::stringstream ssSelect;
    ssSelect << "SELECT litstrId,litstr FROM "
             << m_repo.table(m_repoId, "UnitLitstr")
             << " WHERE unitSn == @unitSn ORDER BY litstrId ASC;";
    txn.prepare(*this, ssSelect.str());
  }
  RepoTxnQuery query(txn, *this);
  query.bindInt64("@unitSn", ue.sn());
  do {
    query.step();
    if (query.row()) {
      Id litstrId;        /**/ query.getId(0, litstrId);
      StringData* litstr; /**/ query.getStaticString(1, litstr);
      Id id UNUSED = ue.mergeLitstr(litstr);
      ASSERT(id == litstrId);
    }
  } while (!query.done());
  txn.commit();
}

void UnitRepoProxy::InsertUnitArrayStmt
                  ::insert(RepoTxn& txn, int64 unitSn, Id arrayId,
                           const StringData* array) {
  if (!prepared()) {
    std::stringstream ssInsert;
    ssInsert << "INSERT INTO " << m_repo.table(m_repoId, "UnitArray")
             << " VALUES(@unitSn, @arrayId, @array);";
    txn.prepare(*this, ssInsert.str());
  }
  RepoTxnQuery query(txn, *this);
  query.bindInt64("@unitSn", unitSn);
  query.bindId("@arrayId", arrayId);
  query.bindStaticString("@array", array);
  query.exec();
}

void UnitRepoProxy::GetUnitArraysStmt
                  ::get(UnitEmitter& ue) {
  RepoTxn txn(m_repo);
  if (!prepared()) {
    std::stringstream ssSelect;
    ssSelect << "SELECT arrayId,array FROM "
             << m_repo.table(m_repoId, "UnitArray")
             << " WHERE unitSn == @unitSn ORDER BY arrayId ASC;";
    txn.prepare(*this, ssSelect.str());
  }
  RepoTxnQuery query(txn, *this);
  query.bindInt64("@unitSn", ue.sn());
  do {
    query.step();
    if (query.row()) {
      Id arrayId;        /**/ query.getId(0, arrayId);
      StringData* array; /**/ query.getStaticString(1, array);
      String s(array);
      Variant v = f_unserialize(s);
      Id id UNUSED = ue.mergeArray(v.asArrRef().get(), array);
      ASSERT(id == arrayId);
    }
  } while (!query.done());
  txn.commit();
}

void UnitRepoProxy::InsertUnitPreConstStmt
                  ::insert(RepoTxn& txn, int64 unitSn, const PreConst& pc,
                           Id id) {
  if (!prepared()) {
    std::stringstream ssInsert;
    ssInsert << "INSERT INTO " << m_repo.table(m_repoId, "UnitPreConst")
             << " VALUES(@unitSn, @name, @value, @preConstId);";
    txn.prepare(*this, ssInsert.str());
  }
  RepoTxnQuery query(txn, *this);
  query.bindInt64("@unitSn", unitSn);
  query.bindStaticString("@name", pc.name);
  query.bindTypedValue("@value", pc.value);
  query.bindId("@preConstId", id);
  query.exec();
}

void UnitRepoProxy::GetUnitPreConstsStmt
                  ::get(UnitEmitter& ue) {
  RepoTxn txn(m_repo);
  if (!prepared()) {
    std::stringstream ssSelect;
    ssSelect << "SELECT name,value,preconstId FROM "
             << m_repo.table(m_repoId, "UnitPreConst")
             << " WHERE unitSn == @unitSn ORDER BY preConstId ASC;";
    txn.prepare(*this, ssSelect.str());
  }
  RepoTxnQuery query(txn, *this);
  query.bindInt64("@unitSn", ue.sn());
  do {
    query.step();
    if (query.row()) {
      StringData* name; /**/ query.getStaticString(0, name);
      TypedValue value; /**/ query.getTypedValue(1, value);
      Id id;            /**/ query.getId(2, id);
      UNUSED Id addedId = ue.addPreConst(name, value);
      ASSERT(id == addedId);
    }
  } while (!query.done());
  txn.commit();
}

void UnitRepoProxy::InsertUnitSourceLocStmt
                  ::insert(RepoTxn& txn, int64 unitSn, Offset pastOffset,
                           int line0, int char0, int line1, int char1) {
  if (!prepared()) {
    std::stringstream ssInsert;
    ssInsert << "INSERT INTO " << m_repo.table(m_repoId, "UnitSourceLoc")
             << " VALUES(@unitSn, @pastOffset, @line0, @char0, @line1,"
                " @char1);";
    txn.prepare(*this, ssInsert.str());
  }
  RepoTxnQuery query(txn, *this);
  query.bindInt64("@unitSn", unitSn);
  query.bindOffset("@pastOffset", pastOffset);
  query.bindInt("@line0", line0);
  query.bindInt("@char0", char0);
  query.bindInt("@line1", line1);
  query.bindInt("@char1", char1);
  query.exec();
}

bool UnitRepoProxy::GetSourceLocStmt
                  ::get(int64 unitSn, Offset pc, SourceLoc& sLoc) {
  try {
    RepoTxn txn(m_repo);
    if (!prepared()) {
      std::stringstream ssSelect;
      ssSelect << "SELECT line0,char0,line1,char1 FROM "
               << m_repo.table(m_repoId, "UnitSourceLoc")
               << " WHERE unitSn == @unitSn AND pastOffset > @pc"
                  " ORDER BY pastOffset ASC LIMIT 1;";
      txn.prepare(*this, ssSelect.str());
    }
    RepoTxnQuery query(txn, *this);
    query.bindInt64("@unitSn", unitSn);
    query.bindOffset("@pc", pc);
    query.step();
    if (!query.row()) {
      return true;
    }
    query.getInt(0, sLoc.line0);
    query.getInt(1, sLoc.char0);
    query.getInt(2, sLoc.line1);
    query.getInt(3, sLoc.char1);
    txn.commit();
  } catch (RepoExc& re) {
    return true;
  }
  return false;
}

bool UnitRepoProxy::GetSourceLocPastOffsetsStmt
                  ::get(int64 unitSn, int line, OffsetRangeVec& ranges) {
  try {
    RepoTxn txn(m_repo);
    if (!prepared()) {
      std::stringstream ssSelect;
      ssSelect << "SELECT pastOffset FROM "
               << m_repo.table(m_repoId, "UnitSourceLoc")
               << " WHERE unitSn == @unitSn AND line0 <= @line"
                  " AND line1 >= @line;";
      txn.prepare(*this, ssSelect.str());
    }
    RepoTxnQuery query(txn, *this);
    query.bindInt64("@unitSn", unitSn);
    query.bindInt("@line", line);
    do {
      query.step();
      if (query.row()) {
        Offset pastOffset; /**/ query.getOffset(0, pastOffset);
        ranges.push_back(OffsetRange(pastOffset, pastOffset));
      }
    } while (!query.done());
    txn.commit();
  } catch (RepoExc& re) {
    return true;
  }
  return false;
}

bool UnitRepoProxy::GetSourceLocBaseOffsetStmt
                  ::get(int64 unitSn, OffsetRange& range) {
  try {
    RepoTxn txn(m_repo);
    if (!prepared()) {
      std::stringstream ssSelect;
      ssSelect << "SELECT pastOffset FROM "
               << m_repo.table(m_repoId, "UnitSourceLoc")
               << " WHERE unitSn == @unitSn AND pastOffset < @pastOffset"
                  " ORDER BY pastOffset DESC LIMIT 1;";
      txn.prepare(*this, ssSelect.str());
    }
    RepoTxnQuery query(txn, *this);
    query.bindInt64("@unitSn", unitSn);
    query.bindOffset("@pastOffset", range.m_past);
    query.step();
    if (!query.row()) {
      // This is the first bytecode range within the unit.
      range.m_base = 0;
    } else {
      query.getOffset(0, range.m_base);
    }
    txn.commit();
  } catch (RepoExc& re) {
    return true;
  }
  return false;
}

bool UnitRepoProxy::GetBaseOffsetAtPCLocStmt
                  ::get(int64 unitSn, Offset pc, Offset& offset) {
  try {
    RepoTxn txn(m_repo);
    if (!prepared()) {
      std::stringstream ssSelect;
      ssSelect << "SELECT pastOffset FROM "
               << m_repo.table(m_repoId, "UnitSourceLoc")
               << " WHERE unitSn == @unitSn AND pastOffset <= @pc"
                  " ORDER BY pastOffset DESC LIMIT 1;";
      txn.prepare(*this, ssSelect.str());
    }
    RepoTxnQuery query(txn, *this);
    query.bindInt64("@unitSn", unitSn);
    query.bindOffset("@pc", pc);
    query.step();
    if (!query.row()) {
      return true;
    }
    query.getOffset(0, offset);
    txn.commit();
  } catch (RepoExc& re) {
    return true;
  }
  return false;
}

bool UnitRepoProxy::GetBaseOffsetAfterPCLocStmt
                  ::get(int64 unitSn, Offset pc, Offset& offset) {
  try {
    RepoTxn txn(m_repo);
    if (!prepared()) {
      std::stringstream ssSelect;
      ssSelect << "SELECT pastOffset FROM "
               << m_repo.table(m_repoId, "UnitSourceLoc")
               << " WHERE unitSn == @unitSn AND pastOffset > @pc"
                  " ORDER BY pastOffset ASC LIMIT 1;";
      txn.prepare(*this, ssSelect.str());
    }
    RepoTxnQuery query(txn, *this);
    query.bindInt64("@unitSn", unitSn);
    query.bindOffset("@pc", pc);
    query.step();
    if (!query.row()) {
      return true;
    }
    query.getOffset(0, offset);
    txn.commit();
  } catch (RepoExc& re) {
    return true;
  }
  return false;
}

//=============================================================================
// UnitEmitter.

UnitEmitter::UnitEmitter(const MD5& md5)
  : m_repoId(-1), m_sn(-1), m_bcmax(BCMaxInit), m_bc((uchar*)malloc(BCMaxInit)),
    m_bclen(0), m_bc_meta(NULL), m_bc_meta_len(0), m_filepath(NULL),
    m_md5(md5), m_nextFuncSn(0) {
}

UnitEmitter::~UnitEmitter() {
  if (m_bc) {
    free(m_bc);
  }
  if (m_bc_meta) {
    free(m_bc_meta);
  }
  for (FeVec::const_iterator it = m_fes.begin(); it != m_fes.end(); ++it) {
    delete *it;
  }
  for (PceVec::const_iterator it = m_pceVec.begin(); it != m_pceVec.end();
       ++it) {
    delete *it;
  }
}

void UnitEmitter::setBc(const uchar* bc, size_t bclen) {
  m_bc = (uchar*)malloc(bclen);
  m_bcmax = bclen;
  memcpy(m_bc, bc, bclen);
  m_bclen = bclen;
}

void UnitEmitter::setBcMeta(const uchar* bc_meta, size_t bc_meta_len) {
  ASSERT(m_bc_meta == NULL);
  if (bc_meta_len) {
    m_bc_meta = (uchar*)malloc(bc_meta_len);
    memcpy(m_bc_meta, bc_meta, bc_meta_len);
  }
  m_bc_meta_len = bc_meta_len;
}

void UnitEmitter::setLines(const LineEntry* lines, size_t nlines) {
  Offset prevPastOffset = 0;
  for (size_t i = 0; i < nlines; ++i) {
    const LineEntry* line = &lines[i];
    Location sLoc;
    sLoc.line0 = sLoc.line1 = line->val();
    Offset pastOffset = line->pastOffset();
    recordSourceLocation(&sLoc, prevPastOffset, pastOffset);
    prevPastOffset = pastOffset;
  }
}

Id UnitEmitter::addPreConst(const StringData* name, const TypedValue& value) {
  ASSERT(value.m_type != KindOfObject && value.m_type != KindOfArray);
  PreConst pc = { value, NULL, name };
  if (pc.value.m_type == KindOfString && !pc.value.m_data.pstr->isStatic()) {
    pc.value.m_data.pstr = StringData::GetStaticString(pc.value.m_data.pstr);
    pc.value.m_type = KindOfStaticString;
  }
  ASSERT(!IS_REFCOUNTED_TYPE(pc.value.m_type));

  Id id = m_preConsts.size();
  m_preConsts.push_back(pc);
  return id;
}

Id UnitEmitter::mergeLitstr(const StringData* litstr) {
  LitstrMap::const_iterator it = m_litstr2id.find(litstr);
  if (it == m_litstr2id.end()) {
    const StringData* str = StringData::GetStaticString(litstr);
    Id id = m_litstrs.size();
    m_litstrs.push_back(str);
    m_litstr2id[str] = id;
    return id;
  } else {
    return it->second;
  }
}

Id UnitEmitter::mergeArray(ArrayData* a, const StringData* key /* = NULL */) {
  if (key == NULL) {
    String s = f_serialize(a);
    key = StringData::GetStaticString(s.get());
  }

  Unit::ArrayIdMap::const_iterator it = m_array2id.find(key);
  if (it == m_array2id.end()) {
    a = ArrayData::GetScalarArray(a, key);

    Id id = m_arrays.size();
    ArrayVecElm ave = {key, a};
    m_arrays.push_back(ave);
    m_array2id[key] = id;
    return id;
  } else {
    return it->second;
  }
}

FuncEmitter* UnitEmitter::getMain() {
  return m_fes[0];
}

void UnitEmitter::initMain(int line1, int line2) {
  ASSERT(m_fes.size() == 0);
  StringData* name = StringData::GetStaticString("");
  FuncEmitter* pseudomain = newFuncEmitter(name, false);
  pseudomain->init(line1, line2, 0, AttrNone, false, name);
}

FuncEmitter* UnitEmitter::newFuncEmitter(const StringData* n, bool top) {
  ASSERT(m_fes.size() > 0 || !strcmp(n->data(), "")); // Pseudomain comes first.
  FuncEmitter* fe = new FuncEmitter(*this, m_nextFuncSn++, m_fes.size(), n);
  m_fes.push_back(fe);
  if (top) {
    if (m_feMap.find(n) != m_feMap.end()) {
      raise_error("Function already defined: %s", n->data());
    }
    m_feMap[n] = fe;
  }
  return fe;
}

FuncEmitter* UnitEmitter::newMethodEmitter(const StringData* n,
                                           PreClassEmitter* pce) {
  return new FuncEmitter(*this, m_nextFuncSn++, n, pce);
}

PreClassEmitter* UnitEmitter::newPreClassEmitter(const StringData* n,
                                                 Attr attrs,
                                                 const StringData* parent,
                                                 const StringData* docComment,
                                                 int line1, int line2, Offset o,
                                                 bool hoistable) {
  PreClassEmitter* pce;
  // A class declaration is hoisted if all of the following are true:
  // 1) It is at the top level of pseudomain (as indicated by the 'hoistable'
  //    parameter).
  // 2) It is the first hoistable declaration for the class name within the
  //    unit.
  // 3) Its parent (if any) has already been defined by the time the attempt
  //    is made to hoist the class.
  // Only the first two conditions are enforced here, because (3) cannot be
  // precomputed.
  if (hoistable && m_hoistablePreClassSet.find(n) ==
      m_hoistablePreClassSet.end()) {
    pce = new PreClassEmitter(*this, line1, line2, o, n, attrs, parent,
                              docComment, m_pceVec.size(), true);
    m_hoistablePreClassSet.insert(n);
    m_hoistablePceVec.push_back(pce);
  } else {
    pce = new PreClassEmitter(*this, line1, line2, o, n, attrs, parent,
                              docComment, m_pceVec.size(), false);
  }
  m_pceVec.push_back(pce);
  return pce;
}

template<typename T>
static void addToIntervalMap(std::map<Offset, IntervalMapEntry<T> > *intMap,
                             Offset start, Offset end,
                             IntervalMapEntry<T> &entry) {
  typename std::map<Offset, IntervalMapEntry<T> >::iterator it =
    intMap->lower_bound(start);
  if (it != intMap->end() && it->second.val == entry.val) {
    entry.startOffset = it->second.startOffset;
    intMap->erase(start);
  } else {
    entry.startOffset = start;
  }
  entry.endOffset = end;
  (*intMap)[end] = entry;
}

void UnitEmitter::recordSourceLocation(const Location *sLoc, Offset start,
                                       Offset end) {
  ASSERT(sLoc);

  SourceLocEntry newEntryLoc;
  newEntryLoc.val.setLoc(sLoc);
  addToIntervalMap(&m_sourceLocTable, start, end, newEntryLoc);
}

void UnitEmitter::recordFunction(FuncEmitter *fe) {
  FuncEmitterEntry newEntry;
  newEntry.val = fe;
  addToIntervalMap(&m_feTable, fe->base(), fe->past(), newEntry);
}

Func* UnitEmitter::newFunc(const FuncEmitter* fe, Unit& unit, Id id, int line1,
                           int line2, Offset base, Offset past,
                           const StringData* name, Attr attrs, bool top,
                           const StringData* docComment, int numParams) {
  Func* f = new (Func::allocFuncMem(name, numParams))
    Func(unit, id, line1, line2, base, past, name, attrs,
         top, docComment, numParams);
  m_fMap[fe] = f;
  return f;
}

Func* UnitEmitter::newFunc(const FuncEmitter* fe, Unit& unit,
                           PreClass* preClass, int line1, int line2,
                           Offset base, Offset past,
                           const StringData* name, Attr attrs, bool top,
                           const StringData* docComment, int numParams) {
  Func* f = new (Func::allocFuncMem(name, numParams))
    Func(unit, preClass, line1, line2, base, past, name,
         attrs, top, docComment, numParams);
  m_fMap[fe] = f;
  return f;
}

void UnitEmitter::commit(UnitOrigin unitOrigin) {
  Repo& repo = Repo::get();
  UnitRepoProxy& urp = repo.urp();
  int repoId = Repo::get().repoIdForNewUnit(unitOrigin);
  if (repoId == RepoIdInvalid) {
    return;
  }
  m_repoId = repoId;
  try {
    RepoTxn txn(repo);
    {
      LineTable lines;
      for (std::map<Offset, SourceLocEntry>::const_iterator
           it = m_sourceLocTable.begin(); it != m_sourceLocTable.end(); ++it) {
        SourceLocEntry e = it->second;
        ASSERT(it->first == e.endOffset);
        lines.push_back(LineEntry(e.endOffset, e.val.line1));
      }
      urp.insertUnit(repoId).insert(txn, m_sn, m_md5, m_bc, m_bclen,
                                    m_bc_meta, m_bc_meta_len, lines);
    }
    int64 usn = m_sn;
    for (unsigned i = 0; i < m_litstrs.size(); ++i) {
      urp.insertUnitLitstr(repoId).insert(txn, usn, i, m_litstrs[i]);
    }
    for (unsigned i = 0; i < m_arrays.size(); ++i) {
      urp.insertUnitArray(repoId).insert(txn, usn, i, m_arrays[i].serialized);
    }
    for (size_t i = 0; i < m_preConsts.size(); ++i) {
      urp.insertUnitPreConst(repoId).insert(txn, usn, m_preConsts[i], i);
    }
    for (FeVec::const_iterator it = m_fes.begin(); it != m_fes.end(); ++it) {
      (*it)->commit(txn);
    }
    for (PceVec::const_iterator it = m_pceVec.begin(); it != m_pceVec.end();
         ++it) {
      (*it)->commit(txn);
    }
    if (RuntimeOption::RepoDebugInfo) {
      for (std::map<Offset, SourceLocEntry>::const_iterator
           it = m_sourceLocTable.begin(); it != m_sourceLocTable.end(); ++it) {
        SourceLocEntry e = it->second;
        ASSERT(it->first == e.endOffset);
        urp.insertUnitSourceLoc(repoId)
           .insert(txn, usn, e.endOffset, e.val.line0, e.val.char0, e.val.line1,
                   e.val.char1);
      }
    }
    txn.commit();
  } catch (RepoExc& re) {
    TRACE(3, "Failed to commit '%s' (0x%016llx%016llx) to '%s': %s\n",
             m_filepath->data(), m_md5.q[0], m_md5.q[1],
             repo.repoName(repoId).c_str(), re.msg().c_str());
  }
}

Unit* UnitEmitter::create() {
  Unit* u = new Unit();
  u->m_repoId = m_repoId;
  u->m_sn = m_sn;
  u->m_bc = (uchar*)malloc(m_bclen);
  memcpy(u->m_bc, m_bc, m_bclen);
  u->m_bclen = m_bclen;
  if (m_bc_meta_len) {
    u->m_bc_meta = (uchar*)malloc(m_bc_meta_len);
    memcpy(u->m_bc_meta, m_bc_meta, m_bc_meta_len);
    u->m_bc_meta_len = m_bc_meta_len;
  }
  u->m_filepath = m_filepath;
  {
    const std::string& dirname = Util::safe_dirname(m_filepath->data(),
                                                    m_filepath->size());
    u->m_dirpath = StringData::GetStaticString(dirname);
  }
  u->m_md5 = m_md5;
  for (unsigned i = 0; i < m_litstrs.size(); ++i) {
    NamedEntityPair np;
    np.first = m_litstrs[i];
    np.second = NULL;
    u->m_namedInfo.push_back(np);
  }
  u->m_array2id = m_array2id;
  for (unsigned i = 0; i < m_arrays.size(); ++i) {
    u->m_arrays.push_back(m_arrays[i].array);
  }
  for (FeVec::const_iterator it = m_fes.begin(); it != m_fes.end(); ++it) {
    Func* func = (*it)->create(*u);
    if (u->m_funcs.size() == 0) {
      ASSERT(func->isPseudoMain());
      u->m_main = func;
      u->m_mainAttrs = func->attrs();
    }
    u->m_funcs.push_back(func);
    if (func->top()) {
      u->m_hoistableFuncs.push_back(func);
    }
  }
  for (PceVec::const_iterator it = m_pceVec.begin(); it != m_pceVec.end();
       ++it) {
    u->m_preClasses.push_back(PreClassPtr((*it)->create(*u)));
  }
  for (PceVec::const_iterator it = m_hoistablePceVec.begin();
       it != m_hoistablePceVec.end(); ++it) {
    u->m_hoistablePreClasses.push_back(u->m_preClasses[(*it)->id()].get());
  }
  for (std::map<Offset, SourceLocEntry>::const_iterator
       it = m_sourceLocTable.begin(); it != m_sourceLocTable.end(); ++it) {
    const SourceLocEntry& e = it->second;
    ASSERT(it->first == e.endOffset);
    u->m_lineTable.push_back(LineEntry(e.endOffset, e.val.line1));
  }
  for (FeTable::const_iterator it = m_feTable.begin(); it != m_feTable.end();
       ++it) {
    const FuncEmitterEntry& e = it->second;
    ASSERT(e.val->base() == e.startOffset);
    ASSERT(e.val->past() == e.endOffset);
    ASSERT(m_fMap.find(e.val) != m_fMap.end());
    u->m_funcTable.push_back(FuncEntry(e.endOffset,
                                       m_fMap.find(e.val)->second));
  }
  m_fMap.clear();

  u->m_preConsts = m_preConsts;
  for (PreConstVec::iterator i = u->m_preConsts.begin();
       i != u->m_preConsts.end(); ++i) {
    i->owner = u;
  }

  if (RuntimeOption::EvalDumpBytecode) {
    // Dump human-readable bytecode.
    std::cout << u->toString();
  }

  return u;
}

///////////////////////////////////////////////////////////////////////////////
}
}
