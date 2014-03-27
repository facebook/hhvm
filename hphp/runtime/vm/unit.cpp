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

#include "hphp/runtime/vm/unit.h"

#include "hphp/compiler/option.h"

#include "hphp/parser/parser.h"

#include "hphp/runtime/base/file-repository.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/base/strings.h"

#include "hphp/runtime/ext/std/ext_std_variable.h"
#include "hphp/runtime/vm/blob-helper.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/disas.h"
#include "hphp/runtime/vm/func-inline.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/treadmill.h"
#include "hphp/runtime/vm/unit-util.h"

#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/translator-inline.h"

#include "hphp/runtime/vm/verifier/check.h"

#include "hphp/util/atomic.h"
#include "hphp/util/file-util.h"
#include "hphp/util/lock.h"
#include "hphp/util/read-only-arena.h"

#include "folly/Memory.h"
#include "folly/ScopeGuard.h"

#include <boost/algorithm/string.hpp>
#include <sys/mman.h>
#include <tbb/concurrent_unordered_map.h>
#include <iostream>
#include <iomanip>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(hhbc);

const StaticString s_stdin("STDIN");
const StaticString s_stdout("STDOUT");
const StaticString s_stderr("STDERR");

/**
 * Read typed data from an offset relative to a base address
 */
template <class T>
T& getDataRef(void* base, unsigned offset) {
  return *(T*)((char*)base + offset);
}

ReadOnlyArena& get_readonly_arena() {
  static ReadOnlyArena arena(RuntimeOption::EvalHHBCArenaChunkSize);
  return arena;
}

// Exports for the admin server.
size_t hhbc_arena_capacity() {
  if (!RuntimeOption::RepoAuthoritative) return 0;
  return get_readonly_arena().capacity();
}

static const unsigned char*
allocateBCRegion(const unsigned char* bc, size_t bclen) {
  if (RuntimeOption::RepoAuthoritative) {
    // In RepoAuthoritative, we assume we won't ever deallocate units
    // and that this is read-only, mostly cold data.  So we throw it
    // in a bump-allocator that's mprotect'd to prevent writes.
    return static_cast<const unsigned char*>(
      get_readonly_arena().allocate(bc, bclen)
    );
  }
  auto mem = static_cast<unsigned char*>(malloc(bclen));
  std::copy(bc, bc + bclen, mem);
  return mem;
}

Mutex Unit::s_classesMutex;
/*
 * We hold onto references to elements of this map. If we use a different
 * map, we must use one that doesnt invalidate references to its elements
 * (unless they are deleted, which never happens here). Any standard
 * associative container will meet this requirement.
 */
static NamedEntityMap *s_namedDataMap;

static NEVER_INLINE
NamedEntity* getNamedEntityHelper(const StringData* str) {
  if (!str->isStatic()) {
    str = makeStaticString(str);
  }

  auto res = s_namedDataMap->insert(str, NamedEntity());
  return &res.first->second;
}

size_t Unit::GetNamedEntityTableSize() {
  return s_namedDataMap ? s_namedDataMap->size() : 0;
}

NEVER_INLINE
static void initializeNamedDataMap() {
  NamedEntityMap::Config config;
  config.growthFactor = 1;
  s_namedDataMap =
    new NamedEntityMap(RuntimeOption::EvalInitialNamedEntityTableSize,
                       config);
}

NamedEntity* Unit::GetNamedEntity(const StringData* str,
                                  bool allowCreate /*= true*/,
                                  String* normalizedStr /*= nullptr*/) {
  if (UNLIKELY(!s_namedDataMap)) {
    initializeNamedDataMap();
  }
  NamedEntityMap::iterator it = s_namedDataMap->find(str);
  if (LIKELY(it != s_namedDataMap->end())) return &it->second;
  if (needsNSNormalization(str)) {
    auto normStr = normalizeNS(StrNR(str).asString());
    if (normalizedStr) {
      *normalizedStr = normStr;
    }
    return GetNamedEntity(normStr.get(), allowCreate, normalizedStr);
  }
  if (LIKELY(allowCreate)) { return getNamedEntityHelper(str); }
  return nullptr;
}

void NamedEntity::setCachedFunc(Func* f) {
  *m_cachedFunc = f;
}

Func* NamedEntity::getCachedFunc() const {
  return LIKELY(m_cachedFunc.bound()) ? *m_cachedFunc : nullptr;
}

void NamedEntity::setCachedClass(Class* f) {
  *m_cachedClass = f;
}

Class* NamedEntity::getCachedClass() const {
  return LIKELY(m_cachedClass.bound()) ? *m_cachedClass : nullptr;
}

void NamedEntity::setCachedTypeAlias(const TypeAliasReq& td) {
  *m_cachedTypeAlias = td;
}

const TypeAliasReq* NamedEntity::getCachedTypeAlias() const {
  // TODO(#2103214): support persistent typeAliases
  m_cachedTypeAlias.bind();
  return m_cachedTypeAlias->name ? m_cachedTypeAlias.get() : nullptr;
}

void NamedEntity::pushClass(Class* cls) {
  assert(!cls->m_nextClass);
  cls->m_nextClass = m_clsList;
  atomic_release_store(&m_clsList, cls); // TODO(#2054448): ARMv8
}

void NamedEntity::removeClass(Class* goner) {
  Class** cls = &m_clsList; // TODO(#2054448): ARMv8
  while (*cls != goner) {
    assert(*cls);
    cls = &(*cls)->m_nextClass;
  }
  *cls = goner->m_nextClass;
}

UnitMergeInfo* UnitMergeInfo::alloc(size_t size) {
  UnitMergeInfo* mi = (UnitMergeInfo*)malloc(
    sizeof(UnitMergeInfo) + size * sizeof(void*));
  mi->m_firstHoistableFunc = 0;
  mi->m_firstHoistablePreClass = 0;
  mi->m_firstMergeablePreClass = 0;
  mi->m_mergeablesSize = size;
  return mi;
}

Array Unit::getFunctions(bool system) {
  // Return an array of all defined functions.  This method is used
  // to support get_defined_functions().
  Array a = Array::Create();
  if (s_namedDataMap) {
    for (NamedEntityMap::const_iterator it = s_namedDataMap->begin();
         it != s_namedDataMap->end(); ++it) {
      Func* func_ = it->second.getCachedFunc();
      if (!func_ || (system ^ func_->isBuiltin()) || func_->isGenerated()) {
        continue;
      }
      a.append(func_->nameRef());
    }
  }
  return a;
}

AllClasses::AllClasses()
  : m_next(s_namedDataMap->begin())
  , m_end(s_namedDataMap->end())
  , m_current(m_next != m_end ? m_next->second.clsList() : nullptr) {
  if (!empty()) skip();
}

void AllClasses::skip() {
  if (!m_current) {
    assert(!empty());
    ++m_next;
    while (!empty()) {
      m_current = m_next->second.clsList();
      if (m_current) break;
      ++m_next;
    }
  }
  assert(empty() || front());
}

void AllClasses::next() {
  m_current = m_current->m_nextClass;
  skip();
}

bool AllClasses::empty() const {
  return m_next == m_end;
}

Class* AllClasses::front() const {
  assert(!empty());
  assert(m_current);
  return m_current;
}

Class* AllClasses::popFront() {
  Class* cls = front();
  next();
  return cls;
}

class AllCachedClasses {
  NamedEntityMap::iterator m_next, m_end;

  void skip() {
    Class* cls;
    while (!empty()) {
      cls = m_next->second.clsList();
      if (cls && cls->getCached() &&
          (cls->parent() != SystemLib::s_ClosureClass)) break;
      ++m_next;
    }
  }

public:
  AllCachedClasses()
    : m_next(s_namedDataMap->begin())
    , m_end(s_namedDataMap->end())
  {
    skip();
  }
  bool empty() const {
    return m_next == m_end;
  }
  Class* front() {
    assert(!empty());
    Class* c = m_next->second.clsList();
    assert(c);
    c = c->getCached();
    assert(c);
    return c;
  }
  Class* popFront() {
    Class* c = front();
    ++m_next;
    skip();
    return c;
  }
};

Array Unit::getClassesWithAttrInfo(Attr attrs, bool inverse) {
  Array a = Array::Create();
  if (s_namedDataMap) {
    for (AllCachedClasses ac; !ac.empty();) {
      Class* c = ac.popFront();
      if ((c->attrs() & attrs) ? !inverse : inverse) {
        if (c->isBuiltin()) {
          a.prepend(c->nameRef());
        } else {
          a.append(c->nameRef());
        }
      }
    }
  }
  return a;
}

Array Unit::getClassesInfo() {
  // Return an array of all defined class names.  This method is used to
  // support get_declared_classes().
  return getClassesWithAttrInfo(AttrInterface | AttrTrait,
                                 /* inverse = */ true);
}

Array Unit::getInterfacesInfo() {
  // Return an array of all defined interface names.  This method is used to
  // support get_declared_interfaces().
  return getClassesWithAttrInfo(AttrInterface);
}

Array Unit::getTraitsInfo() {
  // Returns an array with all defined trait names.  This method is used to
  // support get_declared_traits().
  return getClassesWithAttrInfo(AttrTrait);
}

bool Unit::checkStringId(Id id) const {
  if (isGlobalLitstrId(id)) {
    return decodeGlobalLitstrId(id) <  LitstrTable::get().numLitstrs();
  }
  return id >= 0 && unsigned(id) < numLitstrs();
}

bool Unit::MetaHandle::findMeta(const Unit* unit, Offset offset) {
  if (!unit->m_bc_meta_len) return false;
  assert(unit->m_bc_meta);
  Offset* index1 = (Offset*)unit->m_bc_meta;
  Offset* index2 = index1 + *index1 + 1;

  assert(index1[*index1 + 1] == INT_MAX); // sentinel
  assert(offset >= 0 && (unsigned)offset < unit->m_bclen);
  assert(cur == 0 || index == index1);
  if (cur && offset >= index[cur]) {
    while (offset >= index[cur+1]) cur++;
  } else {
    int hi = *index1 + 2;
    int lo = 1;
    while (hi - lo > 1) {
      int mid = (hi + lo) >> 1;
      if (offset >= index1[mid]) {
        lo = mid;
      } else {
        hi = mid;
      }
    }
    index = index1;
    cur = lo;
  }
  assert(cur <= (unsigned)*index1);
  assert((unsigned)index2[cur] <= unit->m_bc_meta_len);
  ptr = unit->m_bc_meta + index2[cur];
  return index[cur] == offset;
}

bool Unit::MetaHandle::nextArg(MetaInfo& info) {
  assert(index && cur && ptr);
  uint8_t* end = (uint8_t*)index + index[*index + cur + 2];
  assert(ptr <= end);
  if (ptr == end) return false;
  info.m_kind = (Unit::MetaInfo::Kind)*ptr++;
  info.m_arg = *ptr++;
  info.m_data = decodeVariableSizeImm(&ptr);
  return true;
}

//=============================================================================
// LitstrTable.

LitstrTable* LitstrTable::s_litstrTable = nullptr;

Id LitstrTable::mergeLitstr(const StringData* litstr) {
  m_mutex.lock();
  assert(!m_safeToRead);
  auto it = m_litstr2id.find(litstr);
  if (it == m_litstr2id.end()) {
    const StringData* str = makeStaticString(litstr);
    Id id = m_litstrs.size();
    NamedEntityPair np = { str, nullptr };
    m_litstr2id[str] = id;
    m_litstrs.push_back(str);
    m_namedInfo.push_back(np);
    m_mutex.unlock();
    return id;
  } else {
    m_mutex.unlock();
    return it->second;
  }
}

void LitstrTable::insert(RepoTxn& txn, UnitOrigin uo) {
  Repo& repo = Repo::get();
  LitstrRepoProxy& lsrp = repo.lsrp();
  int repoId = Repo::get().repoIdForNewUnit(uo);
  for (int i = 0; i < m_litstrs.size(); ++i) {
    lsrp.insertLitstr(repoId).insert(txn, i, m_litstrs[i]);
  }
}

//=============================================================================
// Unit.

Unit::Unit() {
  tvWriteUninit(&m_mainReturn);
}

Unit::~Unit() {
  if (!RuntimeOption::RepoAuthoritative) {
    if (debug) {
      // poison released bytecode
      memset(const_cast<unsigned char*>(m_bc), 0xff, m_bclen);
    }
    free(const_cast<unsigned char*>(m_bc));
    free(const_cast<unsigned char*>(m_bc_meta));
  }

  if (m_mergeInfo) {
    for (auto* func : mutableFuncs()) Func::destroy(func);
  }

  // ExecutionContext and the TC may retain references to Class'es, so
  // it is possible for Class'es to outlive their Unit.
  for (int i = m_preClasses.size(); i--; ) {
    PreClass* pcls = m_preClasses[i].get();
    Class* cls = pcls->namedEntity()->clsList();
    while (cls) {
      Class* cur = cls;
      cls = cls->m_nextClass;
      if (cur->preClass() == pcls) {
        cur->destroy();
      }
    }
  }

  free(m_mergeInfo);

  if (m_pseudoMainCache) {
    for (auto it = m_pseudoMainCache->begin();
         it != m_pseudoMainCache->end(); ++it) {
      delete it->second;
    }
    delete m_pseudoMainCache;
  }
}

void* Unit::operator new(size_t sz) {
  return low_malloc(sz);
}

void Unit::operator delete(void* p, size_t sz) {
  low_free(p);
}

bool Unit::compileTimeFatal(const StringData*& msg, int& line) const {
  // A compile-time fatal is encoded as a pseudomain that contains precisely:
  //
  //   String <id>; Fatal;
  //
  // Decode enough of pseudomain to determine whether it contains a
  // compile-time fatal, and if so, extract the error message and line number.
  auto entry = reinterpret_cast<const Op*>(getMain()->getEntry());
  auto pc = entry;
  // String <id>; Fatal;
  // ^^^^^^
  if (*pc != Op::String) {
    return false;
  }
  pc++;
  // String <id>; Fatal;
  //        ^^^^
  Id id = *(Id*)pc;
  pc += sizeof(Id);
  // String <id>; Fatal;
  //              ^^^^^
  if (*pc != Op::Fatal) {
    return false;
  }
  msg = lookupLitstrId(id);
  line = getLineNumber(Offset(pc - entry));
  return true;
}

bool Unit::parseFatal(const StringData*& msg, int& line) const {
  if (!compileTimeFatal(msg, line)) {
    return false;
  }

  auto pc = getMain()->getEntry();

  // two opcodes + String's ID
  pc += sizeof(Id) + 2;

  auto kind_char = *pc;
  return kind_char == static_cast<uint8_t>(FatalOp::Parse);
}

class FrameRestore {
 public:
  explicit FrameRestore(const PreClass* preClass) {
    auto const ec = g_context.getNoCheck();
    ActRec* fp = ec->getFP();
    PC pc = ec->getPC();

    if (ec->m_stack.top() &&
        (!fp || fp->m_func->unit() != preClass->unit())) {
      m_top = ec->m_stack.top();
      m_fp = fp;
      m_pc = pc;

      /*
        we can be called from Unit::merge, which hasnt yet setup
        the frame (because often it doesnt need to).
        Set up a fake frame here, in case of errors.
        But note that mergeUnit is called for systemlib etc before the
        stack has been setup. So dont do anything if m_stack.top()
        is NULL
      */
      ActRec &tmp = *ec->m_stack.allocA();
      tmp.m_savedRbp = (uint64_t)fp;
      tmp.m_savedRip = 0;
      tmp.m_func = preClass->unit()->getMain();
      tmp.m_soff = !fp
        ? 0
        : fp->m_func->unit()->offsetOf(pc) - fp->m_func->base();
      tmp.setThis(nullptr);
      tmp.m_varEnv = 0;
      tmp.initNumArgs(0);
      ec->m_fp = &tmp;
      ec->m_pc = preClass->unit()->at(preClass->getOffset());
      ec->pushLocalsAndIterators(tmp.m_func);
    } else {
      m_top = nullptr;
      m_fp = nullptr;
      m_pc = nullptr;
    }
  }
  ~FrameRestore() {
    auto const ec = g_context.getNoCheck();
    if (m_top) {
      ec->m_stack.top() = m_top;
      ec->m_fp = m_fp;
      ec->m_pc = m_pc;
    }
  }
 private:
  Cell*   m_top;
  ActRec* m_fp;
  PC      m_pc;
};

Class* Unit::defClass(const PreClass* preClass,
                      bool failIsFatal /* = true */) {
  NamedEntity* const nameList = preClass->namedEntity();
  Class* top = nameList->clsList();

  /*
   * Check if there is already a name defined in this request for this
   * NamedEntity.
   *
   * Raise a fatal unless the existing class definition is identical to the
   * one this invocation would create.
   */
  if (auto current = nameList->getCachedTypeAlias()) {
    FrameRestore fr(preClass);
    raise_error("Cannot declare class with the same name (%s) as an "
                "existing type", current->name->data());
    return nullptr;
  }

  // If there was already a class declared with DefClass, check if
  // it's compatible.
  if (Class* cls = nameList->getCachedClass()) {
    if (cls->preClass() != preClass) {
      if (failIsFatal) {
        FrameRestore fr(preClass);
        raise_error("Class already declared: %s", preClass->name()->data());
      }
      return nullptr;
    }
    return cls;
  }

  // Get a compatible Class, and add it to the list of defined classes.
  Class* parent = nullptr;
  for (;;) {
    // Search for a compatible extant class.  Searching from most to least
    // recently created may have better locality than alternative search orders.
    // In addition, its the only simple way to make this work lock free...
    for (Class* class_ = top; class_ != nullptr; ) {
      Class* cur = class_;
      class_ = class_->m_nextClass;
      if (cur->preClass() != preClass) continue;
      Class::Avail avail = cur->avail(parent, failIsFatal /*tryAutoload*/);
      if (LIKELY(avail == Class::Avail::True)) {
        cur->setCached();
        DEBUGGER_ATTACHED_ONLY(phpDebuggerDefClassHook(cur));
        return cur;
      }
      if (avail == Class::Avail::Fail) {
        if (failIsFatal) {
          FrameRestore fr(preClass);
          raise_error("unknown class %s", parent->name()->data());
        }
        return nullptr;
      }
      assert(avail == Class::Avail::False);
    }

    // Create a new class.
    if (!parent && preClass->parent()->size() != 0) {
      parent = Unit::getClass(preClass->parent(), failIsFatal);
      if (parent == nullptr) {
        if (failIsFatal) {
          FrameRestore fr(preClass);
          raise_error("unknown class %s", preClass->parent()->data());
        }
        return nullptr;
      }
    }

    ClassPtr newClass;
    {
      FrameRestore fr(preClass);
      newClass = Class::newClass(const_cast<PreClass*>(preClass), parent);
    }
    Lock l(Unit::s_classesMutex);

    if (UNLIKELY(top != nameList->clsList())) {
      top = nameList->clsList();
      continue;
    }

    bool const isPersistent =
      (!SystemLib::s_inited || RuntimeOption::RepoAuthoritative) &&
      newClass->verifyPersistent();
    nameList->m_cachedClass.bind(
      isPersistent ? RDS::Mode::Persistent
                   : RDS::Mode::Normal
    );
    newClass->setClassHandle(nameList->m_cachedClass);
    newClass.get()->incAtomicCount();

    if (InstanceBits::initFlag.load(std::memory_order_acquire)) {
      // If the instance bitmap has already been set up, we can just
      // initialize our new class's bits and add ourselves to the class
      // list normally.
      newClass->setInstanceBits();
      nameList->pushClass(newClass.get());
    } else {
      // Otherwise, we have to grab the read lock. If the map has been
      // initialized since we checked, initialize the bits normally. If not,
      // we must add the new class to the class list before dropping the lock
      // to ensure its bits are initialized when the time comes.
      ReadLock l(InstanceBits::lock);
      if (InstanceBits::initFlag.load(std::memory_order_acquire)) {
        newClass->setInstanceBits();
      }
      nameList->pushClass(newClass.get());
    }
    /*
     * call setCached after adding to the class list, otherwise the
     * target-cache short circuit at the top could return a class
     * which is not yet on the clsList().
     */
    newClass.get()->setCached();
    DEBUGGER_ATTACHED_ONLY(phpDebuggerDefClassHook(newClass.get()));
    return newClass.get();
  }
}

bool Unit::aliasClass(Class* original, const StringData* alias) {
  auto const aliasNe = Unit::GetNamedEntity(alias);
  aliasNe->m_cachedClass.bind();

  auto const aliasClass = aliasNe->getCachedClass();
  if (aliasClass) {
    raise_warning("Cannot redeclare class %s", alias->data());
    return false;
  }
  aliasNe->setCachedClass(original);
  return true;
}

void Unit::defTypeAlias(Id id) {
  assert(id < m_typeAliases.size());
  auto thisType = &m_typeAliases[id];
  auto nameList = GetNamedEntity(thisType->name);
  const StringData* typeName = thisType->value;

  /*
   * Check if this name already was defined as a type alias, and if so
   * make sure it is compatible.
   */
  if (auto current = nameList->getCachedTypeAlias()) {
    if (thisType->kind != current->kind ||
        thisType->nullable != current->nullable ||
        Unit::lookupClass(typeName) != current->klass) {
      raise_error("The type %s is already defined to an incompatible type",
                  thisType->name->data());
    }
    return;
  }

  // There might also be a class with this name already.
  if (nameList->getCachedClass()) {
    raise_error("The name %s is already defined as a class",
                thisType->name->data());
    return;
  }

  // TODO(#2103214): persistent type alias support
  nameList->m_cachedTypeAlias.bind();

  /*
   * If this type alias is a KindOfObject and the name on the right
   * hand side was another type alias, we will bind the name to the
   * other side for this request (i.e. resolve that type alias now).
   *
   * We need to inspect the right hand side and figure out what it was
   * first.
   *
   * If the right hand side was a class, we need to autoload and
   * ensure it exists at this point.
   */

  if (thisType->kind != KindOfObject) {
    nameList->setCachedTypeAlias(
      TypeAliasReq { thisType->kind,
                     thisType->nullable,
                     nullptr,
                     thisType->name }
    );
    return;
  }

  auto targetNameList = GetNamedEntity(typeName);
  if (auto targetTd = getTypeAliasWithAutoload(targetNameList, typeName)) {
    nameList->setCachedTypeAlias(
      TypeAliasReq { targetTd->kind,
                     thisType->nullable || targetTd->nullable,
                     targetTd->klass,
                     thisType->name }
    );
    return;
  }
  if (auto klass = Unit::loadClass(typeName)) {
    nameList->setCachedTypeAlias(
      TypeAliasReq { KindOfObject,
                     thisType->nullable,
                     klass,
                     thisType->name }
    );
    return;
  }

  raise_error("Unknown type or class %s", typeName->data());
}

void Unit::renameFunc(const StringData* oldName, const StringData* newName) {
  // renameFunc() should only be used by ExecutionContext::createFunction.
  // We do a linear scan over all the functions in the unit searching for the
  // func with a given name; in practice this is okay because the units created
  // by create_function() will always have the function being renamed at the
  // beginning
  assert(oldName && oldName->isStatic());
  assert(newName && newName->isStatic());

  for (MutableFuncRange fr(hoistableFuncs()); !fr.empty(); ) {
    Func* func = fr.popFront();
    const StringData* name = func->name();
    assert(name);
    if (name->same(oldName)) {
      func->rename(newName);
      break;
    }
  }
}

Class* Unit::loadClass(const NamedEntity* ne,
                       const StringData* name) {
  Class* cls;
  if (LIKELY((cls = ne->getCachedClass()) != nullptr)) {
    return cls;
  }
  return loadMissingClass(ne, name);
}

Class* Unit::loadMissingClass(const NamedEntity* ne,
                              const StringData* name) {
  JIT::VMRegAnchor _;
  AutoloadHandler::s_instance->invokeHandler(
    StrNR(const_cast<StringData*>(name)));
  return Unit::lookupClass(ne);
}

Class* Unit::getClass(const NamedEntity* ne,
                      const StringData *name, bool tryAutoload) {
  Class *cls = lookupClass(ne);
  if (UNLIKELY(!cls)) {
    if (tryAutoload) {
      return loadMissingClass(ne, name);
    }
  }
  return cls;
}

bool Unit::classExists(const StringData* name, bool autoload, Attr typeAttrs) {
  Class* cls = Unit::getClass(name, autoload);
  return cls && (cls->attrs() & (AttrInterface | AttrTrait)) == typeAttrs;
}

void Unit::loadFunc(const Func *func) {
  assert(!func->isMethod());
  auto const ne = func->getNamedEntity();
  auto const isPersistent =
    (RuntimeOption::RepoAuthoritative || !SystemLib::s_inited) &&
    (func->attrs() & AttrPersistent);
  ne->m_cachedFunc.bind(
    isPersistent ? RDS::Mode::Persistent
                 : RDS::Mode::Normal
  );
  const_cast<Func*>(func)->setFuncHandle(ne->m_cachedFunc);
}

static void mergeCns(TypedValue& tv, TypedValue *value,
                     StringData *name) {
  if (LIKELY(tv.m_type == KindOfUninit)) {
    tv = *value;
    return;
  }

  raise_notice(Strings::CONSTANT_ALREADY_DEFINED, name->data());
}

static SimpleMutex unitInitLock(false /* reentrant */, RankUnitInit);

void Unit::initialMerge() {
  unitInitLock.assertOwnedBySelf();
  if (LIKELY(m_mergeState == UnitMergeStateUnmerged)) {
    int state = 0;
    bool needsCompact = false;
    m_mergeState = UnitMergeStateMerging;

    bool allFuncsUnique = RuntimeOption::RepoAuthoritative;
    for (MutableFuncRange fr(nonMainFuncs()); !fr.empty();) {
      Func* f = fr.popFront();
      if (allFuncsUnique) {
        allFuncsUnique = (f->attrs() & AttrUnique);
      }
      loadFunc(f);
      if (RDS::isPersistentHandle(f->funcHandle())) {
        needsCompact = true;
      }
    }
    if (allFuncsUnique) state |= UnitMergeStateUniqueFuncs;
    if (RuntimeOption::RepoAuthoritative || !SystemLib::s_inited) {
      /*
       * The mergeables array begins with the hoistable Func*s,
       * followed by the (potentially) hoistable Class*s.
       *
       * If the Unit is merge only, it then contains enough information
       * to simulate executing the pseudomain. Normally, this is just
       * the Class*s that might not be hoistable. In RepoAuthoritative
       * mode it also includes assignments of the form:
       *  $GLOBALS[string-literal] = scalar;
       * defines of the form:
       *  define(string-literal, scalar);
       * and requires.
       *
       * These cases are differentiated using the bottom 3 bits
       * of the pointer. In the case of a define or a global,
       * the pointer will be followed by a TypedValue representing
       * the value being defined/assigned.
       */
      int ix = m_mergeInfo->m_firstHoistablePreClass;
      int end = m_mergeInfo->m_firstMergeablePreClass;
      while (ix < end) {
        PreClass* pre = (PreClass*)m_mergeInfo->mergeableObj(ix++);
        if (pre->attrs() & AttrUnique) {
          needsCompact = true;
        }
      }
      if (isMergeOnly()) {
        ix = m_mergeInfo->m_firstMergeablePreClass;
        end = m_mergeInfo->m_mergeablesSize;
        while (ix < end) {
          void *obj = m_mergeInfo->mergeableObj(ix);
          UnitMergeKind k = UnitMergeKind(uintptr_t(obj) & 7);
          switch (k) {
            case UnitMergeKindUniqueDefinedClass:
            case UnitMergeKindDone:
              not_reached();
            case UnitMergeKindClass:
              if (((PreClass*)obj)->attrs() & AttrUnique) {
                needsCompact = true;
              }
              break;
            case UnitMergeKindReqDoc: {
              StringData* s = (StringData*)((char*)obj - (int)k);
              auto const efile = g_context->lookupIncludeRoot(s,
                InclOpFlags::DocRoot, nullptr, this);
              assert(efile);
              Unit* unit = efile->unit();
              unit->initialMerge();
              m_mergeInfo->mergeableObj(ix) = (void*)((char*)unit + (int)k);
              break;
            }
            case UnitMergeKindPersistentDefine:
              needsCompact = true;
            case UnitMergeKindDefine: {
              StringData* s = (StringData*)((char*)obj - (int)k);
              auto* v = (TypedValueAux*) m_mergeInfo->mergeableData(ix + 1);
              ix += sizeof(*v) / sizeof(void*);
              v->rdsHandle() = makeCnsHandle(
                s, k == UnitMergeKindPersistentDefine);
              if (k == UnitMergeKindPersistentDefine) {
                mergeCns(RDS::handleToRef<TypedValue>(v->rdsHandle()),
                         v, s);
              }
              break;
            }
            case UnitMergeKindGlobal:
              // Skip over the value of the global, embedded in mergeableData
              ix += sizeof(TypedValueAux) / sizeof(void*);
              break;
          }
          ix++;
        }
      }
      if (needsCompact) state |= UnitMergeStateNeedsCompact;
    }
    m_mergeState = UnitMergeStateMerged | state;
  }
}

Cell* Unit::lookupCns(const StringData* cnsName) {
  auto const handle = lookupCnsHandle(cnsName);
  if (LIKELY(handle != 0)) {
    TypedValue& tv = RDS::handleToRef<TypedValue>(handle);
    if (LIKELY(tv.m_type != KindOfUninit)) {
      assert(cellIsPlausible(tv));
      return &tv;
    }
    if (UNLIKELY(tv.m_data.pref != nullptr)) {
      ClassInfo::ConstantInfo* ci =
        (ClassInfo::ConstantInfo*)(void*)tv.m_data.pref;
      auto const tvRet = const_cast<Variant&>(
        ci->getDeferredValue()).asTypedValue();
      assert(cellIsPlausible(*tvRet));
      if (LIKELY(tvRet->m_type != KindOfUninit)) {
        return tvRet;
      }
    }
  }
  if (UNLIKELY(RDS::s_constants().get() != nullptr)) {
    return RDS::s_constants()->nvGet(cnsName);
  }
  return nullptr;
}

Cell* Unit::lookupPersistentCns(const StringData* cnsName) {
  auto const handle = lookupCnsHandle(cnsName);
  if (!RDS::isPersistentHandle(handle)) return nullptr;
  auto const ret = &RDS::handleToRef<TypedValue>(handle);
  assert(cellIsPlausible(*ret));
  return ret;
}

TypedValue* Unit::loadCns(const StringData* cnsName) {
  TypedValue* tv = lookupCns(cnsName);
  if (LIKELY(tv != nullptr)) return tv;

  if (needsNSNormalization(cnsName)) {
    return loadCns(normalizeNS(cnsName));
  }

  if (!AutoloadHandler::s_instance->autoloadConstant(
        const_cast<StringData*>(cnsName))) {
    return nullptr;
  }
  return lookupCns(cnsName);
}

bool Unit::defCns(const StringData* cnsName, const TypedValue* value,
                  bool persistent /* = false */) {
  auto const handle = makeCnsHandle(cnsName, persistent);

  if (UNLIKELY(handle == 0)) {
    if (UNLIKELY(!RDS::s_constants().get())) {
      /*
       * This only happens when we call define on a non
       * static string. Not worth presizing or otherwise
       * optimizing for.
       */
      RDS::s_constants() =
        Array::attach(HphpArray::MakeReserve(1));
    }
    auto const existed = !!RDS::s_constants()->nvGet(cnsName);
    if (!existed) {
      RDS::s_constants().set(StrNR(cnsName),
        tvAsCVarRef(value), true /* isKey */);
      return true;
    }
    raise_notice(Strings::CONSTANT_ALREADY_DEFINED, cnsName->data());
    return false;
  }
  return defCnsHelper(handle, value, cnsName);
}

uint64_t Unit::defCnsHelper(uint64_t ch,
                            const TypedValue *value,
                            const StringData *cnsName) {
  TypedValue* cns = &RDS::handleToRef<TypedValue>(ch);
  if (UNLIKELY(cns->m_type != KindOfUninit) ||
      UNLIKELY(cns->m_data.pref != nullptr)) {
    raise_notice(Strings::CONSTANT_ALREADY_DEFINED, cnsName->data());
  } else if (UNLIKELY(!tvAsCVarRef(value).isAllowedAsConstantValue())) {
    raise_warning(Strings::CONSTANTS_MUST_BE_SCALAR);
  } else {
    Variant v = tvAsCVarRef(value);
    v.setEvalScalar();
    cns->m_data = v.asTypedValue()->m_data;
    cns->m_type = v.asTypedValue()->m_type;
    return true;
  }
  return false;
}

void Unit::defDynamicSystemConstant(const StringData* cnsName,
                                    const void* data) {
  static const bool kServer = RuntimeOption::ServerExecutionMode();
  // Zend doesn't define the STD* streams in server mode so we don't either
  if (UNLIKELY(kServer &&
       (s_stdin.equal(cnsName) ||
        s_stdout.equal(cnsName) ||
        s_stderr.equal(cnsName)))) {
    return;
  }
  auto const handle = makeCnsHandle(cnsName, true);
  assert(handle);
  TypedValue* cns = &RDS::handleToRef<TypedValue>(handle);
  assert(cns->m_type == KindOfUninit);
  cns->m_data.pref = (RefData*)data;
}

static void setGlobal(StringData* name, TypedValue *value) {
  g_context->m_globalVarEnv->set(name, value);
}

void Unit::merge() {
  if (UNLIKELY(!(m_mergeState & UnitMergeStateMerged))) {
    SimpleLock lock(unitInitLock);
    initialMerge();
  }

  if (UNLIKELY(isDebuggerAttached())) {
    mergeImpl<true>(RDS::tl_base, m_mergeInfo);
  } else {
    mergeImpl<false>(RDS::tl_base, m_mergeInfo);
  }
}

void* Unit::replaceUnit() const {
  if (m_mergeState & UnitMergeStateEmpty) return nullptr;
  if (isMergeOnly() &&
      m_mergeInfo->m_mergeablesSize == m_mergeInfo->m_firstHoistableFunc + 1) {
    void* obj =
      m_mergeInfo->mergeableObj(m_mergeInfo->m_firstHoistableFunc);
    if (m_mergeInfo->m_firstMergeablePreClass ==
        m_mergeInfo->m_firstHoistableFunc) {
      int k = uintptr_t(obj) & 7;
      if (k != UnitMergeKindClass) return obj;
    } else if (m_mergeInfo->m_firstHoistablePreClass ==
               m_mergeInfo->m_firstHoistableFunc) {
      if (uintptr_t(obj) & 1) {
        return (char*)obj - 1 + (int)UnitMergeKindUniqueDefinedClass;
      }
    }
  }
  return const_cast<Unit*>(this);
}

size_t compactUnitMergeInfo(UnitMergeInfo* in, UnitMergeInfo* out) {
  Func** it = in->funcHoistableBegin();
  Func** fend = in->funcEnd();
  Func** iout = 0;
  unsigned ix, end, oix = 0;

  if (out) {
    if (in != out) memcpy(out, in, uintptr_t(it) - uintptr_t(in));
    iout = out->funcHoistableBegin();
  }

  size_t delta = 0;
  while (it != fend) {
    Func* func = *it++;
    if (RDS::isPersistentHandle(func->funcHandle())) {
      delta++;
    } else if (iout) {
      *iout++ = func;
    }
  }

  if (out) {
    oix = out->m_firstHoistablePreClass -= delta;
  }

  ix = in->m_firstHoistablePreClass;
  end = in->m_firstMergeablePreClass;
  for (; ix < end; ++ix) {
    void* obj = in->mergeableObj(ix);
    assert((uintptr_t(obj) & 1) == 0);
    PreClass* pre = (PreClass*)obj;
    if (pre->attrs() & AttrUnique) {
      Class* cls = pre->namedEntity()->clsList();
      assert(cls && !cls->m_nextClass);
      assert(cls->preClass() == pre);
      if (RDS::isPersistentHandle(cls->classHandle())) {
        delta++;
      } else if (out) {
        out->mergeableObj(oix++) = (void*)(uintptr_t(cls) | 1);
      }
    } else if (out) {
      out->mergeableObj(oix++) = obj;
    }
  }

  if (out) {
    out->m_firstMergeablePreClass = oix;
  }

  end = in->m_mergeablesSize;
  while (ix < end) {
    void* obj = in->mergeableObj(ix++);
    UnitMergeKind k = UnitMergeKind(uintptr_t(obj) & 7);
    switch (k) {
      case UnitMergeKindClass: {
        PreClass* pre = (PreClass*)obj;
        if (pre->attrs() & AttrUnique) {
          Class* cls = pre->namedEntity()->clsList();
          assert(cls && !cls->m_nextClass);
          assert(cls->preClass() == pre);
          if (RDS::isPersistentHandle(cls->classHandle())) {
            delta++;
          } else if (out) {
            out->mergeableObj(oix++) =
              (void*)(uintptr_t(cls) | UnitMergeKindUniqueDefinedClass);
          }
        } else if (out) {
          out->mergeableObj(oix++) = obj;
        }
        break;
      }
      case UnitMergeKindUniqueDefinedClass:
        not_reached();

      case UnitMergeKindPersistentDefine:
        delta += 1 + sizeof(TypedValueAux) / sizeof(void*);
        ix += sizeof(TypedValueAux) / sizeof(void*);
        break;

      case UnitMergeKindDefine:
      case UnitMergeKindGlobal:
        if (out) {
          out->mergeableObj(oix++) = obj;
          *(TypedValueAux*)out->mergeableData(oix) =
            *(TypedValueAux*)in->mergeableData(ix);
          oix += sizeof(TypedValueAux) / sizeof(void*);
        }
        ix += sizeof(TypedValueAux) / sizeof(void*);
        break;

      case UnitMergeKindReqDoc: {
        Unit *unit = (Unit*)((char*)obj - (int)k);
        void *rep = unit->replaceUnit();
        if (!rep) {
          delta++;
        } else if (out) {
          if (rep == unit) {
            out->mergeableObj(oix++) = obj;
          } else {
            out->mergeableObj(oix++) = rep;
          }
        }
        break;
      }
      case UnitMergeKindDone:
        not_reached();
    }
  }
  if (out) {
    // copy the UnitMergeKindDone marker
    out->mergeableObj(oix) = in->mergeableObj(ix);
    out->m_mergeablesSize = oix;
  }
  return delta;
}

template <bool debugger>
void Unit::mergeImpl(void* tcbase, UnitMergeInfo* mi) {
  assert(m_mergeState & UnitMergeStateMerged);

  Func** it = mi->funcHoistableBegin();
  Func** fend = mi->funcEnd();
  if (it != fend) {
    if (LIKELY((m_mergeState & UnitMergeStateUniqueFuncs) != 0)) {
      do {
        Func* func = *it;
        assert(func->top());
        getDataRef<Func*>(tcbase, func->funcHandle()) = func;
        if (debugger) phpDebuggerDefFuncHook(func);
      } while (++it != fend);
    } else {
      do {
        Func* func = *it;
        assert(func->top());
        setCachedFunc(func, debugger);
      } while (++it != fend);
    }
  }

  bool redoHoistable = false;
  int ix = mi->m_firstHoistablePreClass;
  int end = mi->m_firstMergeablePreClass;
  // iterate over all the potentially hoistable classes
  // with no fatals on failure
  if (ix < end) {
    do {
      // The first time this unit is merged, if the classes turn out to be all
      // unique and defined, we replace the PreClass*'s with the corresponding
      // Class*'s, with the low-order bit marked.
      PreClass* pre = (PreClass*)mi->mergeableObj(ix);
      if (LIKELY(uintptr_t(pre) & 1)) {
        Stats::inc(Stats::UnitMerge_hoistable);
        Class* cls = (Class*)(uintptr_t(pre) & ~1);
        if (cls->isPersistent()) {
          Stats::inc(Stats::UnitMerge_hoistable_persistent);
        }
        if (Stats::enabled() &&
            RDS::isPersistentHandle(cls->classHandle())) {
          Stats::inc(Stats::UnitMerge_hoistable_persistent_cache);
        }
        if (Class* parent = cls->parent()) {
          if (parent->isPersistent()) {
            Stats::inc(Stats::UnitMerge_hoistable_persistent_parent);
          }
          if (Stats::enabled() &&
              RDS::isPersistentHandle(parent->classHandle())) {
            Stats::inc(Stats::UnitMerge_hoistable_persistent_parent_cache);
          }
          if (UNLIKELY(!getDataRef<Class*>(tcbase, parent->classHandle()))) {
            redoHoistable = true;
            continue;
          }
        }
        getDataRef<Class*>(tcbase, cls->classHandle()) = cls;
        if (debugger) phpDebuggerDefClassHook(cls);
      } else {
        if (UNLIKELY(!defClass(pre, false))) {
          redoHoistable = true;
        }
      }
    } while (++ix < end);
    if (UNLIKELY(redoHoistable)) {
      // if this unit isnt mergeOnly, we're done
      if (!isMergeOnly()) return;
      // as a special case, if all the classes are potentially
      // hoistable, we dont list them twice, but instead
      // iterate over them again
      // At first glance, it may seem like we could leave
      // the maybe-hoistable classes out of the second list
      // and then always reset ix to 0; but that gets this
      // case wrong if there's an autoloader for C, and C
      // extends B:
      //
      // class A {}
      // class B implements I {}
      // class D extends C {}
      //
      // because now A and D go on the maybe-hoistable list
      // B goes on the never hoistable list, and we
      // fatal trying to instantiate D before B
      Stats::inc(Stats::UnitMerge_redo_hoistable);
      if (end == (int)mi->m_mergeablesSize) {
        ix = mi->m_firstHoistablePreClass;
        do {
          void* obj = mi->mergeableObj(ix);
          if (UNLIKELY(uintptr_t(obj) & 1)) {
            Class* cls = (Class*)(uintptr_t(obj) & ~1);
            defClass(cls->preClass(), true);
          } else {
            defClass((PreClass*)obj, true);
          }
        } while (++ix < end);
        return;
      }
    }
  }

  // iterate over all but the guaranteed hoistable classes
  // fataling if we fail.
  void* obj = mi->mergeableObj(ix);
  UnitMergeKind k = UnitMergeKind(uintptr_t(obj) & 7);
  do {
    switch(k) {
      case UnitMergeKindClass:
        do {
          Stats::inc(Stats::UnitMerge_mergeable);
          Stats::inc(Stats::UnitMerge_mergeable_class);
          defClass((PreClass*)obj, true);
          obj = mi->mergeableObj(++ix);
          k = UnitMergeKind(uintptr_t(obj) & 7);
        } while (!k);
        continue;

      case UnitMergeKindUniqueDefinedClass:
        do {
          Stats::inc(Stats::UnitMerge_mergeable);
          Stats::inc(Stats::UnitMerge_mergeable_unique);
          Class* other = nullptr;
          Class* cls = (Class*)((char*)obj - (int)k);
          if (cls->isPersistent()) {
            Stats::inc(Stats::UnitMerge_mergeable_unique_persistent);
          }
          if (Stats::enabled() &&
              RDS::isPersistentHandle(cls->classHandle())) {
            Stats::inc(Stats::UnitMerge_mergeable_unique_persistent_cache);
          }
          Class::Avail avail = cls->avail(other, true);
          if (UNLIKELY(avail == Class::Avail::Fail)) {
            raise_error("unknown class %s", other->name()->data());
          }
          assert(avail == Class::Avail::True);
          getDataRef<Class*>(tcbase, cls->classHandle()) = cls;
          if (debugger) phpDebuggerDefClassHook(cls);
          obj = mi->mergeableObj(++ix);
          k = UnitMergeKind(uintptr_t(obj) & 7);
        } while (k == UnitMergeKindUniqueDefinedClass);
        continue;

      case UnitMergeKindPersistentDefine:
        // will be removed by compactUnitMergeInfo
        // but could be hit by other threads before
        // that happens
        do {
          ix += 1 + sizeof(TypedValueAux) / sizeof(void*);
          obj = mi->mergeableObj(ix);
          k = UnitMergeKind(uintptr_t(obj) & 7);
        } while (k == UnitMergeKindDefine);
        continue;

      case UnitMergeKindDefine:
        do {
          Stats::inc(Stats::UnitMerge_mergeable);
          Stats::inc(Stats::UnitMerge_mergeable_define);
          StringData* name = (StringData*)((char*)obj - (int)k);
          auto* v = (TypedValueAux*)mi->mergeableData(ix + 1);
          assert(v->m_type != KindOfUninit);
          mergeCns(getDataRef<TypedValue>(tcbase, v->rdsHandle()), v, name);
          ix += 1 + sizeof(*v) / sizeof(void*);
          obj = mi->mergeableObj(ix);
          k = UnitMergeKind(uintptr_t(obj) & 7);
        } while (k == UnitMergeKindDefine);
        continue;

      case UnitMergeKindGlobal:
        do {
          Stats::inc(Stats::UnitMerge_mergeable);
          Stats::inc(Stats::UnitMerge_mergeable_global);
          StringData* name = (StringData*)((char*)obj - (int)k);
          auto* v = (TypedValueAux*)mi->mergeableData(ix + 1);
          setGlobal(name, v);
          ix += 1 + sizeof(*v) / sizeof(void*);
          obj = mi->mergeableObj(ix);
          k = UnitMergeKind(uintptr_t(obj) & 7);
        } while (k == UnitMergeKindGlobal);
        continue;

      case UnitMergeKindReqDoc:
        do {
          Stats::inc(Stats::UnitMerge_mergeable);
          Stats::inc(Stats::UnitMerge_mergeable_require);
          Unit *unit = (Unit*)((char*)obj - (int)k);
          unsigned char& unitLoadedFlags =
            getDataRef<unsigned char>(tcbase, unit->m_cacheOffset);
          if (!(unitLoadedFlags & unit->m_cacheMask)) {
            unitLoadedFlags |= unit->m_cacheMask;
            unit->mergeImpl<debugger>(tcbase, unit->m_mergeInfo);
            if (UNLIKELY(!unit->isMergeOnly())) {
              Stats::inc(Stats::PseudoMain_Reentered);
              TypedValue ret;
              VarEnv* ve = nullptr;
              ActRec* fp = g_context->m_fp;
              if (!fp) {
                ve = g_context->m_globalVarEnv;
              } else {
                if (fp->hasVarEnv()) {
                  ve = fp->m_varEnv;
                } else {
                  // Nothing to do. If there is no varEnv, the enclosing
                  // file was called by fb_autoload_map, which wants a
                  // local scope.
                }
              }
              g_context->invokeFunc(&ret, unit->getMain(), init_null_variant,
                                      nullptr, nullptr, ve);
              tvRefcountedDecRef(&ret);
            } else {
              Stats::inc(Stats::PseudoMain_SkipDeep);
            }
          } else {
            Stats::inc(Stats::PseudoMain_Guarded);
          }
          obj = mi->mergeableObj(++ix);
          k = UnitMergeKind(uintptr_t(obj) & 7);
        } while (isMergeKindReq(k));
        continue;
      case UnitMergeKindDone:
        Stats::inc(Stats::UnitMerge_mergeable, -1);
        assert((unsigned)ix == mi->m_mergeablesSize);
        if (UNLIKELY(m_mergeState & UnitMergeStateNeedsCompact)) {
          SimpleLock lock(unitInitLock);
          if (!(m_mergeState & UnitMergeStateNeedsCompact)) return;
          if (!redoHoistable) {
            /*
             * All the classes are known to be unique, and we just got
             * here, so all were successfully defined. We can now go
             * back and convert all UnitMergeKindClass entries to
             * UnitMergeKindUniqueDefinedClass, and all hoistable
             * classes to their Class*'s instead of PreClass*'s.
             *
             * We can also remove any Persistent Class/Func*'s,
             * and any requires of modules that are (now) empty
             */
            size_t delta = compactUnitMergeInfo(mi, nullptr);
            UnitMergeInfo* newMi = mi;
            if (delta) {
              newMi = UnitMergeInfo::alloc(mi->m_mergeablesSize - delta);
            }
            /*
             * In the case where mi == newMi, there's an apparent
             * race here. Although we have a lock, so we're the only
             * ones modifying this, there could be any number of
             * readers. But thats ok, because it doesnt matter
             * whether they see the old contents or the new.
             */
            compactUnitMergeInfo(mi, newMi);
            if (newMi != mi) {
              this->m_mergeInfo = newMi;
              Treadmill::deferredFree(mi);
              if (isMergeOnly() &&
                  newMi->m_firstHoistableFunc == newMi->m_mergeablesSize) {
                m_mergeState |= UnitMergeStateEmpty;
              }
            }
            assert(newMi->m_firstMergeablePreClass == newMi->m_mergeablesSize ||
                   isMergeOnly());
          }
          m_mergeState &= ~UnitMergeStateNeedsCompact;
        }
        return;
    }
    // Normal cases should continue, KindDone returns
    NOT_REACHED();
  } while (true);
}

Func* Unit::getMain(Class* cls /*= NULL*/) const {
  if (!cls) return *m_mergeInfo->funcBegin();
  Lock lock(s_classesMutex);
  if (!m_pseudoMainCache) {
    m_pseudoMainCache = new PseudoMainCacheMap;
  }
  auto it = m_pseudoMainCache->find(cls);
  if (it != m_pseudoMainCache->end()) {
    return it->second;
  }
  Func* f = (*m_mergeInfo->funcBegin())->clone(cls);
  f->setNewFuncId();
  f->setBaseCls(cls);
  (*m_pseudoMainCache)[cls] = f;
  return f;
}

SourceLocTable Unit::getSourceLocTable() const {
  if (m_sourceLocTable.size() > 0 || m_repoId == RepoIdInvalid) {
    return m_sourceLocTable;
  }
  Lock lock(s_classesMutex);
  UnitRepoProxy& urp = Repo::get().urp();
  urp.getSourceLocTab(m_repoId).get(m_sn, ((Unit*)this)->m_sourceLocTable);
  return m_sourceLocTable;
}

LineToOffsetRangeVecMap Unit::getLineToOffsetRangeVecMap() const {
  if (m_lineToOffsetRangeVecMap.size() > 0) {
    return m_lineToOffsetRangeVecMap;
  }
  auto srcLoc = this->getSourceLocTable();
  LineToOffsetRangeVecMap map;
  Offset baseOff = 0;
  for (size_t i = 0; i < srcLoc.size(); ++i) {
    Offset pastOff = srcLoc[i].pastOffset();
    OffsetRange range(baseOff, pastOff);
    auto line0 = srcLoc[i].val().line0;
    auto line1 = srcLoc[i].val().line1;
    for (int line = line0; line <= line1; line++) {
      auto it = map.find(line);
      if (it != map.end()) {
        it->second.push_back(range);
      } else {
        OffsetRangeVec v(1);
        v.push_back(range);
        map[line] = v;
      }
    }
    baseOff = pastOff;
  }
  const_cast<Unit*>(this)->m_lineToOffsetRangeVecMap = map;
  return m_lineToOffsetRangeVecMap;
}

// This uses range lookups so offsets in the middle of instructions are
// supported.
int getLineNumber(const LineTable& table, Offset pc) {
  auto const key = LineEntry(pc, -1);
  auto it = std::upper_bound(begin(table), end(table), key);
  if (it != end(table)) {
    assert(pc < it->pastOffset());
    return it->val();
  }
  return -1;
}

int Unit::getLineNumber(Offset pc) const {
  return HPHP::getLineNumber(m_lineTable, pc);
}

bool getSourceLoc(const SourceLocTable& table, Offset pc, SourceLoc& sLoc) {
  SourceLocEntry key(pc, sLoc);
  auto it = std::upper_bound(table.begin(), table.end(), key);
  if (it != table.end()) {
    assert(pc < it->pastOffset());
    sLoc = it->val();
    return true;
  }
  return false;
}

// Sets sLoc to the source location of the first source location
// entry that contains pc in its range of source locations.
// Returns
bool Unit::getSourceLoc(Offset pc, SourceLoc& sLoc) const {
  auto sourceLocTable = getSourceLocTable();
  return HPHP::getSourceLoc(sourceLocTable, pc, sLoc);
}

bool Unit::getOffsetRanges(int line, OffsetRangeVec& offsets) const {
  assert(offsets.size() == 0);
  auto map = this->getLineToOffsetRangeVecMap();
  auto it = map.find(line);
  if (it == map.end()) return false;
  offsets = it->second;
  return true;
}

bool Unit::getOffsetRange(Offset pc, OffsetRange& range) const {
  LineEntry key = LineEntry(pc, -1);
  std::vector<LineEntry>::const_iterator it =
    upper_bound(m_lineTable.begin(), m_lineTable.end(), key);
  if (it != m_lineTable.end()) {
    assert(pc < it->pastOffset());
    Offset base = it == m_lineTable.begin() ? 0 : (it-1)->pastOffset();
    range.m_base = base;
    range.m_past = it->pastOffset();
    return true;
  }
  return false;
}

const Func* Unit::getFunc(Offset pc) const {
  FuncEntry key = FuncEntry(pc, nullptr);
  auto it = std::upper_bound(m_funcTable.begin(), m_funcTable.end(), key);
  if (it != m_funcTable.end()) {
    assert(pc < it->pastOffset());
    return it->val();
  }
  return nullptr;
}

void Unit::prettyPrint(std::ostream& out, PrintOpts opts) const {
  auto startOffset = opts.startOffset != kInvalidOffset
    ? opts.startOffset : 0;
  auto stopOffset = opts.stopOffset != kInvalidOffset
    ? opts.stopOffset : m_bclen;

  std::map<Offset,const Func*> funcMap;
  for (FuncRange fr(funcs()); !fr.empty();) {
    const Func* f = fr.popFront();
    funcMap[f->base()] = f;
  }
  for (PreClassPtrVec::const_iterator it = m_preClasses.begin();
      it != m_preClasses.end(); ++it) {
    Func* const* methods = (*it)->methods();
    size_t const numMethods = (*it)->numMethods();
    for (size_t i = 0; i < numMethods; ++i) {
      funcMap[methods[i]->base()] = methods[i];
    }
  }

  auto funcIt = funcMap.lower_bound(startOffset);

  const auto* it = &m_bc[startOffset];
  int prevLineNum = -1;
  MetaHandle metaHand;
  while (it < &m_bc[stopOffset]) {
    assert(funcIt == funcMap.end() ||
      funcIt->first >= offsetOf(it));
    if (opts.showFuncs) {
      if (funcIt != funcMap.end() &&
          funcIt->first == offsetOf(it)) {
        out.put('\n');
        funcIt->second->prettyPrint(out);
        ++funcIt;
      }
    }

    if (opts.showLines) {
      int lineNum = getLineNumber(offsetOf(it));
      if (lineNum != prevLineNum) {
        out << "  // line " << lineNum << std::endl;
        prevLineNum = lineNum;
      }
    }

    out << std::string(opts.indentSize, ' ')
        << std::setw(4) << (it - m_bc) << ": ";
    out << instrToString((Op*)it, this);
    if (metaHand.findMeta(this, offsetOf(it))) {
      out << " #";
      Unit::MetaInfo info;
      while (metaHand.nextArg(info)) {
        int arg = info.m_arg & ~MetaInfo::VectorArg;
        const char *argKind = info.m_arg & MetaInfo::VectorArg ? "M" : "";
        switch (info.m_kind) {
          case Unit::MetaInfo::Kind::DataTypeInferred:
          case Unit::MetaInfo::Kind::DataTypePredicted:
            out << " i" << argKind << arg
                << ":t=" << tname(DataType(info.m_data));
            if (info.m_kind == Unit::MetaInfo::Kind::DataTypePredicted) {
              out << "*";
            }
            break;
          case Unit::MetaInfo::Kind::Class: {
            const StringData* sd = lookupLitstrId(info.m_data);
            out << " i" << argKind << arg << ":c=" << sd->data();
            break;
          }
          case Unit::MetaInfo::Kind::MVecPropClass: {
            const StringData* sd = lookupLitstrId(info.m_data);
            out << " i" << argKind << arg << ":pc=" << sd->data();
            break;
          }
          case Unit::MetaInfo::Kind::GuardedThis:
            out << " GuardedThis";
            break;
          case Unit::MetaInfo::Kind::GuardedCls:
            out << " GuardedCls";
            break;
          case Unit::MetaInfo::Kind::None:
            assert(false);
            break;
        }
      }
    }
    out << std::endl;
    it += instrLen((Op*)it);
  }
}

std::string Unit::toString() const {
  std::ostringstream ss;
  prettyPrint(ss);
  for (auto& pc : m_preClasses) {
    pc->prettyPrint(ss);
  }
  for (FuncRange fr(funcs()); !fr.empty();) {
    fr.popFront()->prettyPrint(ss);
  }
  return ss.str();
}

Func* Unit::lookupFunc(const NamedEntity* ne) {
  return ne->getCachedFunc();
}

Func* Unit::lookupFunc(const StringData* funcName) {
  const NamedEntity* ne = GetNamedEntity(funcName);
  return ne->getCachedFunc();
}

Func* Unit::loadFunc(const NamedEntity* ne, const StringData* funcName) {
  Func* func = ne->getCachedFunc();
  if (LIKELY(func != nullptr)) return func;
  if (AutoloadHandler::s_instance->autoloadFunc(
        const_cast<StringData*>(funcName))) {
    func = ne->getCachedFunc();
  }
  return func;
}

Func* Unit::loadFunc(const StringData* name) {
  String normStr;
  auto ne = GetNamedEntity(name, true, &normStr);
  if (normStr) {
    name = normStr.get();
  }
  return loadFunc(ne, name);
}

//=============================================================================
// UnitRepoProxy.

UnitRepoProxy::UnitRepoProxy(Repo& repo)
  : RepoProxy(repo)
#define URP_OP(c, o) \
  , m_##o##Local(repo, RepoIdLocal), m_##o##Central(repo, RepoIdCentral)
    URP_OPS
#undef URP_OP
{
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
                " bc_meta BLOB, mainReturn BLOB, mergeable INTEGER,"
                "lines BLOB, typeAliases BLOB, UNIQUE (md5));";
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
    ssCreate << "CREATE TABLE " << m_repo.table(repoId, "UnitMergeables")
             << "(unitSn INTEGER, mergeableIx INTEGER,"
                " mergeableKind INTEGER, mergeableId INTEGER,"
                " mergeableValue BLOB,"
                " PRIMARY KEY (unitSn, mergeableIx));";
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
}

bool UnitRepoProxy::loadHelper(UnitEmitter& ue,
                               const std::string& name,
                               const MD5& md5) {
  ue.setFilepath(makeStaticString(name));
  // Look for a repo that contains a unit with matching MD5.
  int repoId;
  for (repoId = RepoIdCount - 1; repoId >= 0; --repoId) {
    if (!getUnit(repoId).get(ue, md5)) {
      break;
    }
  }
  if (repoId < 0) {
    TRACE(3, "No repo contains '%s' (0x%016" PRIx64  "%016" PRIx64 ")\n",
             name.c_str(), md5.q[0], md5.q[1]);
    return false;
  }
  try {
    getUnitLitstrs(repoId).get(ue);
    getUnitArrays(repoId).get(ue);
    m_repo.pcrp().getPreClasses(repoId).get(ue);
    getUnitMergeables(repoId).get(ue);
    m_repo.frp().getFuncs(repoId).get(ue);
  } catch (RepoExc& re) {
    TRACE(0,
          "Repo error loading '%s' (0x%016" PRIx64 "%016"
          PRIx64 ") from '%s': %s\n",
          name.c_str(), md5.q[0], md5.q[1], m_repo.repoName(repoId).c_str(),
          re.msg().c_str());
    return false;
  }
  TRACE(3, "Repo loaded '%s' (0x%016" PRIx64 "%016" PRIx64 ") from '%s'\n",
           name.c_str(), md5.q[0], md5.q[1], m_repo.repoName(repoId).c_str());
  return true;
}

std::unique_ptr<UnitEmitter>
UnitRepoProxy::loadEmitter(const std::string& name, const MD5& md5) {
  auto ue = folly::make_unique<UnitEmitter>(md5);
  if (!loadHelper(*ue, name, md5)) ue.reset();
  return ue;
}

Unit* UnitRepoProxy::load(const std::string& name, const MD5& md5) {
  UnitEmitter ue(md5);
  if (!loadHelper(ue, name, md5)) return nullptr;
  return ue.create();
}

void UnitRepoProxy::InsertUnitStmt
                  ::insert(RepoTxn& txn, int64_t& unitSn, const MD5& md5,
                           const unsigned char* bc, size_t bclen,
                           const unsigned char* bc_meta, size_t bc_meta_len,
                           const TypedValue* mainReturn, bool mergeOnly,
                           const LineTable& lines,
                           const std::vector<TypeAlias>& typeAliases) {
  BlobEncoder linesBlob;
  BlobEncoder typeAliasesBlob;

  if (!prepared()) {
    std::stringstream ssInsert;
    ssInsert << "INSERT INTO " << m_repo.table(m_repoId, "Unit")
             << " VALUES(NULL, @md5, @bc, @bc_meta,"
                " @mainReturn, @mergeable, @lines, @typeAliases);";
    txn.prepare(*this, ssInsert.str());
  }
  RepoTxnQuery query(txn, *this);
  query.bindMd5("@md5", md5);
  query.bindBlob("@bc", (const void*)bc, bclen);
  query.bindBlob("@bc_meta",
                 bc_meta_len ? (const void*)bc_meta : (const void*)"",
                 bc_meta_len);
  query.bindTypedValue("@mainReturn", *mainReturn);
  query.bindBool("@mergeable", mergeOnly);
  query.bindBlob("@lines", linesBlob(lines), /* static */ true);
  query.bindBlob("@typeAliases",
                 typeAliasesBlob(typeAliases), /* static */ true);
  query.exec();
  unitSn = query.getInsertedRowid();
}

bool UnitRepoProxy::GetUnitStmt
                  ::get(UnitEmitter& ue, const MD5& md5) {
  try {
    RepoTxn txn(m_repo);
    if (!prepared()) {
      std::stringstream ssSelect;
      ssSelect << "SELECT unitSn,bc,bc_meta,mainReturn,mergeable,"
                  "lines,typeAliases FROM "
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
    int64_t unitSn;                          /**/ query.getInt64(0, unitSn);
    const void* bc; size_t bclen;            /**/ query.getBlob(1, bc, bclen);
    const void* bc_meta; size_t bc_meta_len; /**/ query.getBlob(2, bc_meta,
                                                                bc_meta_len);
    TypedValue value;                        /**/ query.getTypedValue(3, value);
    bool mergeable;                          /**/ query.getBool(4, mergeable);
    BlobDecoder linesBlob =                  /**/ query.getBlob(5);
    BlobDecoder typeAliasesBlob =            /**/ query.getBlob(6);
    ue.setRepoId(m_repoId);
    ue.setSn(unitSn);
    ue.setBc((const unsigned char*)bc, bclen);
    ue.setBcMeta((const unsigned char*)bc_meta, bc_meta_len);
    ue.setMainReturn(&value);
    ue.setMergeOnly(mergeable);

    LineTable lines;
    linesBlob(lines);
    ue.setLines(lines);

    typeAliasesBlob(ue.m_typeAliases);

    txn.commit();
  } catch (RepoExc& re) {
    return true;
  }
  return false;
}

void UnitRepoProxy::InsertUnitLitstrStmt
                  ::insert(RepoTxn& txn, int64_t unitSn, Id litstrId,
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
      Id id UNUSED = ue.mergeUnitLitstr(litstr);
      assert(id == litstrId);
    }
  } while (!query.done());
  txn.commit();
}

void UnitRepoProxy::InsertUnitArrayStmt
                  ::insert(RepoTxn& txn, int64_t unitSn, Id arrayId,
                           const std::string& array) {
  if (!prepared()) {
    std::stringstream ssInsert;
    ssInsert << "INSERT INTO " << m_repo.table(m_repoId, "UnitArray")
             << " VALUES(@unitSn, @arrayId, @array);";
    txn.prepare(*this, ssInsert.str());
  }
  RepoTxnQuery query(txn, *this);
  query.bindInt64("@unitSn", unitSn);
  query.bindId("@arrayId", arrayId);
  query.bindStdString("@array", array);
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
      std::string key;   /**/ query.getStdString(1, key);
      Variant v = unserialize_from_buffer(key.data(), key.size());
      Id id UNUSED = ue.mergeArray(v.asArrRef().get(), key);
      assert(id == arrayId);
    }
  } while (!query.done());
  txn.commit();
}

void UnitRepoProxy::InsertUnitMergeableStmt
                  ::insert(RepoTxn& txn, int64_t unitSn,
                           int ix, UnitMergeKind kind, Id id,
                           TypedValue* value) {
  if (!prepared()) {
    std::stringstream ssInsert;
    ssInsert << "INSERT INTO " << m_repo.table(m_repoId, "UnitMergeables")
             << " VALUES(@unitSn, @mergeableIx, @mergeableKind,"
                " @mergeableId, @mergeableValue);";
    txn.prepare(*this, ssInsert.str());
  }

  RepoTxnQuery query(txn, *this);
  query.bindInt64("@unitSn", unitSn);
  query.bindInt("@mergeableIx", ix);
  query.bindInt("@mergeableKind", (int)kind);
  query.bindId("@mergeableId", id);
  if (value) {
    assert(kind == UnitMergeKindDefine ||
           kind == UnitMergeKindPersistentDefine ||
           kind == UnitMergeKindGlobal);
    query.bindTypedValue("@mergeableValue", *value);
  } else {
    assert(kind == UnitMergeKindReqDoc);
    query.bindNull("@mergeableValue");
  }
  query.exec();
}

void UnitRepoProxy::GetUnitMergeablesStmt
                  ::get(UnitEmitter& ue) {
  RepoTxn txn(m_repo);
  if (!prepared()) {
    std::stringstream ssSelect;
    ssSelect << "SELECT mergeableIx,mergeableKind,mergeableId,mergeableValue"
                " FROM "
             << m_repo.table(m_repoId, "UnitMergeables")
             << " WHERE unitSn == @unitSn ORDER BY mergeableIx ASC;";
    txn.prepare(*this, ssSelect.str());
  }
  RepoTxnQuery query(txn, *this);
  query.bindInt64("@unitSn", ue.sn());
  do {
    query.step();
    if (query.row()) {
      int mergeableIx;           /**/ query.getInt(0, mergeableIx);
      int mergeableKind;         /**/ query.getInt(1, mergeableKind);
      Id mergeableId;            /**/ query.getInt(2, mergeableId);

      if (UNLIKELY(!RuntimeOption::RepoAuthoritative)) {
        /*
         * We're using a repo generated in WholeProgram mode,
         * but we're not using it in RepoAuthoritative mode
         * (this is dodgy to start with). We're not going to
         * deal with requires at merge time, so drop them
         * here, and clear the mergeOnly flag for the unit.
         * The one exception is persistent constants are allowed in systemlib.
         */
        if (mergeableKind != UnitMergeKindPersistentDefine ||
           SystemLib::s_inited) {
          ue.setMergeOnly(false);
        }
      }
      switch (mergeableKind) {
        case UnitMergeKindReqDoc:
          ue.insertMergeableInclude(mergeableIx,
                                    (UnitMergeKind)mergeableKind, mergeableId);
          break;
        case UnitMergeKindPersistentDefine:
        case UnitMergeKindDefine:
        case UnitMergeKindGlobal: {
          TypedValue mergeableValue; /**/ query.getTypedValue(3,
                                                              mergeableValue);
          ue.insertMergeableDef(mergeableIx, (UnitMergeKind)mergeableKind,
                                mergeableId, mergeableValue);
          break;
        }
      }
    }
  } while (!query.done());
  txn.commit();
}

void UnitRepoProxy::InsertUnitSourceLocStmt
                  ::insert(RepoTxn& txn, int64_t unitSn, Offset pastOffset,
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

bool UnitRepoProxy::GetSourceLocTabStmt
     ::get(int64_t unitSn, SourceLocTable& sourceLocTab) {
  try {
    RepoTxn txn(m_repo);
    if (!prepared()) {
      std::stringstream ssSelect;
      ssSelect << "SELECT pastOffset,line0,char0,line1,char1 FROM "
               << m_repo.table(m_repoId, "UnitSourceLoc")
               << " WHERE unitSn == @unitSn"
                  " ORDER BY pastOffset ASC;";
      txn.prepare(*this, ssSelect.str());
    }
    RepoTxnQuery query(txn, *this);
    query.bindInt64("@unitSn", unitSn);
    do {
      query.step();
      if (!query.row()) {
        return true;
      }
      Offset pastOffset;
      query.getOffset(0, pastOffset);
      SourceLoc sLoc;
      query.getInt(1, sLoc.line0);
      query.getInt(2, sLoc.char0);
      query.getInt(3, sLoc.line1);
      query.getInt(4, sLoc.char1);
      SourceLocEntry entry(pastOffset, sLoc);
      sourceLocTab.push_back(entry);
    } while (!query.done());
    txn.commit();
  } catch (RepoExc& re) {
    return true;
  }
  return false;
}

//=============================================================================
// UnitEmitter.

UnitEmitter::UnitEmitter(const MD5& md5)
  : m_repoId(-1), m_sn(-1), m_bcmax(BCMaxInit), m_bc((unsigned char*)malloc(BCMaxInit)),
    m_bclen(0), m_bc_meta(nullptr), m_bc_meta_len(0), m_filepath(nullptr),
    m_md5(md5), m_nextFuncSn(0), m_mergeOnly(false),
    m_allClassesHoistable(true), m_returnSeen(false) {
  tvWriteUninit(&m_mainReturn);
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

void UnitEmitter::addTrivialPseudoMain() {
  initMain(0, 0);
  FuncEmitter* mfe = getMain();
  emitOp(OpInt);
  emitInt64(1);
  emitOp(OpRetC);
  mfe->setMaxStackCells(1);
  mfe->finish(bcPos(), false);
  recordFunction(mfe);

  TypedValue mainReturn;
  mainReturn.m_data.num = 1;
  mainReturn.m_type = KindOfInt64;
  setMainReturn(&mainReturn);
  setMergeOnly(true);
}

void UnitEmitter::setBc(const unsigned char* bc, size_t bclen) {
  if (m_bc) {
    free(m_bc);
  }
  m_bc = (unsigned char*)malloc(bclen);
  m_bcmax = bclen;
  memcpy(m_bc, bc, bclen);
  m_bclen = bclen;
}

void UnitEmitter::setBcMeta(const unsigned char* bc_meta, size_t bc_meta_len) {
  assert(m_bc_meta == nullptr);
  if (bc_meta_len) {
    m_bc_meta = (unsigned char*)malloc(bc_meta_len);
    memcpy(m_bc_meta, bc_meta, bc_meta_len);
  }
  m_bc_meta_len = bc_meta_len;
}

void UnitEmitter::setLines(const LineTable& lines) {
  this->m_lineTable = lines;
}

Id UnitEmitter::mergeUnitLitstr(const StringData* litstr) {
  auto it = m_litstr2id.find(litstr);
  if (it == m_litstr2id.end()) {
    const StringData* str = makeStaticString(litstr);
    Id id = m_litstrs.size();
    m_litstrs.push_back(str);
    m_litstr2id[str] = id;
    return id;
  } else {
    return it->second;
  }
}

Id UnitEmitter::mergeLitstr(const StringData* litstr) {
  if (Option::WholeProgram) {
    return encodeGlobalLitstrId(LitstrTable::get().mergeLitstr(litstr));
  }
  return mergeUnitLitstr(litstr);
}

Id UnitEmitter::mergeArray(const ArrayData* a) {
  Variant v(const_cast<ArrayData*>(a));
  auto key = HHVM_FN(serialize)(v).toCppString();
  return mergeArray(a, key);
}

Id UnitEmitter::mergeArray(const ArrayData* a, const std::string& key) {
  ArrayIdMap::const_iterator it = m_array2id.find(key);
  if (it != m_array2id.end()) {
    return it->second;
  }
  a = ArrayData::GetScalarArray(const_cast<ArrayData*>(a), key);
  Id id = m_arrays.size();
  ArrayVecElm ave = {key, a};
  m_arrays.push_back(ave);
  m_array2id[key] = id;
  return id;
}

const StringData* UnitEmitter::lookupLitstr(Id id) const {
  if (isGlobalLitstrId(id)) {
    return LitstrTable::get().lookupLitstrId(decodeGlobalLitstrId(id));
  }
  assert(id < m_litstrs.size());
  return m_litstrs[id];
}

const ArrayData* UnitEmitter::lookupArray(Id id) const {
  assert(id < m_arrays.size());
  return m_arrays[id].array;
}

FuncEmitter* UnitEmitter::getMain() {
  return m_fes[0];
}

void UnitEmitter::initMain(int line1, int line2) {
  assert(m_fes.size() == 0);
  StringData* name = makeStaticString("");
  FuncEmitter* pseudomain = newFuncEmitter(name);
  Attr attrs = AttrMayUseVV;
  pseudomain->init(line1, line2, 0, attrs, false, name);
}

FuncEmitter* UnitEmitter::newFuncEmitter(const StringData* n) {
  assert(m_fes.size() > 0 || !strcmp(n->data(), "")); // Pseudomain comes first.
  FuncEmitter* fe = new FuncEmitter(*this, m_nextFuncSn++, m_fes.size(), n);
  m_fes.push_back(fe);
  return fe;
}

void UnitEmitter::appendTopEmitter(FuncEmitter* fe) {
  fe->setIds(m_nextFuncSn++, m_fes.size());
  m_fes.push_back(fe);
}

void UnitEmitter::pushMergeableClass(PreClassEmitter* e) {
  m_mergeableStmts.push_back(std::make_pair(UnitMergeKindClass, e->id()));
}

void UnitEmitter::pushMergeableInclude(UnitMergeKind kind,
                                       const StringData* unitName) {
  m_mergeableStmts.push_back(
    std::make_pair(kind, mergeLitstr(unitName)));
  m_allClassesHoistable = false;
}

void UnitEmitter::insertMergeableInclude(int ix, UnitMergeKind kind, int id) {
  assert(size_t(ix) <= m_mergeableStmts.size());
  m_mergeableStmts.insert(m_mergeableStmts.begin() + ix,
                          std::make_pair(kind, id));
  m_allClassesHoistable = false;
}

void UnitEmitter::pushMergeableDef(UnitMergeKind kind,
                                   const StringData* name,
                                   const TypedValue& tv) {
  m_mergeableStmts.push_back(std::make_pair(kind, m_mergeableValues.size()));
  m_mergeableValues.push_back(std::make_pair(mergeLitstr(name), tv));
  m_allClassesHoistable = false;
}

void UnitEmitter::insertMergeableDef(int ix, UnitMergeKind kind,
                                     Id id, const TypedValue& tv) {
  assert(size_t(ix) <= m_mergeableStmts.size());
  m_mergeableStmts.insert(m_mergeableStmts.begin() + ix,
                          std::make_pair(kind, m_mergeableValues.size()));
  m_mergeableValues.push_back(std::make_pair(id, tv));
  m_allClassesHoistable = false;
}

FuncEmitter* UnitEmitter::newMethodEmitter(const StringData* n,
                                           PreClassEmitter* pce) {
  return new FuncEmitter(*this, m_nextFuncSn++, n, pce);
}

PreClassEmitter* UnitEmitter::newPreClassEmitter(const StringData* n,
                                                 PreClass::Hoistable
                                                 hoistable) {
  // See class.h for information about hoistability.
  if (hoistable && m_hoistablePreClassSet.count(n)) {
    hoistable = PreClass::Mergeable;
  }

  PreClassEmitter* pce = new PreClassEmitter(*this, m_pceVec.size(), n,
                                             hoistable);

  if (hoistable >= PreClass::MaybeHoistable) {
    m_hoistablePreClassSet.insert(n);
    if (hoistable == PreClass::ClosureHoistable) {
      // Closures should appear at the VERY top of the file, so if any class in
      // the same file tries to use them, they are already defined. We had a
      // fun race where one thread was autoloading a file, finished parsing the
      // class, then another thread came along and saw the class was already
      // loaded and ran it before the first thread had time to parse the
      // closure class.
      m_hoistablePceIdList.push_front(pce->id());
    } else {
      m_hoistablePceIdList.push_back(pce->id());
    }
  } else {
    m_allClassesHoistable = false;
  }
  if (hoistable >= PreClass::Mergeable &&
      hoistable < PreClass::AlwaysHoistable) {
    if (m_returnSeen) {
      m_allClassesHoistable = false;
    } else {
      pushMergeableClass(pce);
    }
  }
  m_pceVec.push_back(pce);
  return pce;
}

Id UnitEmitter::addTypeAlias(const TypeAlias& td) {
  Id id = m_typeAliases.size();
  m_typeAliases.push_back(td);
  return id;
}

void UnitEmitter::recordSourceLocation(const Location* sLoc, Offset start) {
  // Some byte codes, such as for the implicit "return 0" at the end of a
  // a source file do not have valid source locations. This check makes
  // sure we don't record a (dummy) source location in this case.
  if (start > 0 && sLoc->line0 == 1 && sLoc->char0 == 1 &&
    sLoc->line1 == 1 && sLoc->char1 == 1 && strlen(sLoc->file) == 0) return;
  SourceLoc newLoc(*sLoc);
  if (!m_sourceLocTab.empty()) {
    if (m_sourceLocTab.back().second == newLoc) {
      // Combine into the interval already at the back of the vector.
      assert(start >= m_sourceLocTab.back().first);
      return;
    }
    assert(m_sourceLocTab.back().first < start &&
           "source location offsets must be added to UnitEmitter in "
           "increasing order");
  } else {
    // First record added should be for bytecode offset zero.
    assert(start == 0);
  }
  m_sourceLocTab.push_back(std::make_pair(start, newLoc));
}

void UnitEmitter::recordFunction(FuncEmitter* fe) {
  m_feTab.push_back(std::make_pair(fe->past(), fe));
}

Func* UnitEmitter::newFunc(const FuncEmitter* fe, Unit& unit,
                           Id id, PreClass* preClass, int line1, int line2,
                           Offset base, Offset past,
                           const StringData* name, Attr attrs, bool top,
                           const StringData* docComment, int numParams,
                           bool needsNextClonedClosure) {
  Func* f = new (Func::allocFuncMem(name, numParams,
                                    needsNextClonedClosure,
                                    !preClass))
    Func(unit, id, preClass, line1, line2, base, past, name,
         attrs, top, docComment, numParams);
  m_fMap[fe] = f;
  return f;
}

static LineTable createLineTable(
    std::vector<std::pair<Offset,SourceLoc> >& srcLoc,
    Offset bclen) {
  LineTable lines;
  for (size_t i = 0; i < srcLoc.size(); ++i) {
    Offset endOff = i < srcLoc.size() - 1 ? srcLoc[i + 1].first : bclen;
    lines.push_back(LineEntry(endOff, srcLoc[i].second.line1));
  }
  return lines;
}

SourceLocTable UnitEmitter::createSourceLocTable() const {
  SourceLocTable locations;
  for (size_t i = 0; i < m_sourceLocTab.size(); ++i) {
    Offset endOff = i < m_sourceLocTab.size() - 1
      ? m_sourceLocTab[i + 1].first
      : m_bclen;
    locations.push_back(SourceLocEntry(endOff, m_sourceLocTab[i].second));
  }
  return locations;
}

static LineToOffsetRangeVecMap createLineToOffsetMap(
    std::vector<std::pair<Offset,SourceLoc> >& srcLoc,
    Offset bclen) {
  LineToOffsetRangeVecMap map;
  for (size_t i = 0; i < srcLoc.size(); ++i) {
    Offset baseOff = srcLoc[i].first;
    Offset endOff = i < srcLoc.size() - 1 ? srcLoc[i + 1].first : bclen;
    OffsetRange range(baseOff, endOff);
    auto line0 = srcLoc[i].second.line0;
    auto line1 = srcLoc[i].second.line1;
    for (int line = line0; line <= line1; line++) {
      auto it = map.find(line);
      if (it != map.end()) {
        it->second.push_back(range);
      } else {
        OffsetRangeVec v(1);
        v.push_back(range);
        map[line] = v;
      }
    }
  }
  return map;
}

bool UnitEmitter::insert(UnitOrigin unitOrigin, RepoTxn& txn) {
  Repo& repo = Repo::get();
  UnitRepoProxy& urp = repo.urp();
  int repoId = Repo::get().repoIdForNewUnit(unitOrigin);
  if (repoId == RepoIdInvalid) {
    return true;
  }
  m_repoId = repoId;

  try {
    {
      auto lines = createLineTable(m_sourceLocTab, m_bclen);
      urp.insertUnit(repoId).insert(txn, m_sn, m_md5, m_bc, m_bclen,
                                    m_bc_meta, m_bc_meta_len,
                                    &m_mainReturn, m_mergeOnly, lines,
                                    m_typeAliases);
    }
    int64_t usn = m_sn;
    for (unsigned i = 0; i < m_litstrs.size(); ++i) {
      urp.insertUnitLitstr(repoId).insert(txn, usn, i, m_litstrs[i]);
    }
    for (unsigned i = 0; i < m_arrays.size(); ++i) {
      urp.insertUnitArray(repoId).insert(txn, usn, i, m_arrays[i].serialized);
    }
    for (auto it = m_fes.begin(); it != m_fes.end(); ++it) {
      (*it)->commit(txn);
    }
    for (auto it = m_pceVec.begin(); it != m_pceVec.end(); ++it) {
      (*it)->commit(txn);
    }

    for (int i = 0, n = m_mergeableStmts.size(); i < n; i++) {
      switch (m_mergeableStmts[i].first) {
        case UnitMergeKindDone:
        case UnitMergeKindUniqueDefinedClass:
          not_reached();
        case UnitMergeKindClass: break;
        case UnitMergeKindReqDoc: {
          urp.insertUnitMergeable(repoId).insert(
            txn, usn, i,
            m_mergeableStmts[i].first, m_mergeableStmts[i].second, nullptr);
          break;
        }
        case UnitMergeKindDefine:
        case UnitMergeKindPersistentDefine:
        case UnitMergeKindGlobal: {
          int ix = m_mergeableStmts[i].second;
          urp.insertUnitMergeable(repoId).insert(
            txn, usn, i,
            m_mergeableStmts[i].first,
            m_mergeableValues[ix].first, &m_mergeableValues[ix].second);
          break;
        }
      }
    }
    if (RuntimeOption::RepoDebugInfo) {
      for (size_t i = 0; i < m_sourceLocTab.size(); ++i) {
        SourceLoc& e = m_sourceLocTab[i].second;
        Offset endOff = i < m_sourceLocTab.size() - 1
                          ? m_sourceLocTab[i + 1].first
                          : m_bclen;

        urp.insertUnitSourceLoc(repoId)
           .insert(txn, usn, endOff, e.line0, e.char0, e.line1, e.char1);
      }
    }
    return false;
  } catch (RepoExc& re) {
    TRACE(3, "Failed to commit '%s' (0x%016" PRIx64 "%016" PRIx64 ") to '%s': %s\n",
             m_filepath->data(), m_md5.q[0], m_md5.q[1],
             repo.repoName(repoId).c_str(), re.msg().c_str());
    return true;
  }
}

void UnitEmitter::commit(UnitOrigin unitOrigin) {
  Repo& repo = Repo::get();
  try {
    RepoTxn txn(repo);
    bool err = insert(unitOrigin, txn);
    if (!err) {
      txn.commit();
    }
  } catch (RepoExc& re) {
    int repoId = repo.repoIdForNewUnit(unitOrigin);
    if (repoId != RepoIdInvalid) {
      TRACE(3, "Failed to commit '%s' (0x%016" PRIx64 "%016" PRIx64 ") to '%s': %s\n",
               m_filepath->data(), m_md5.q[0], m_md5.q[1],
               repo.repoName(repoId).c_str(), re.msg().c_str());
    }
  }
}

Unit* UnitEmitter::create() {
  Unit* u = new Unit();
  u->m_repoId = m_repoId;
  u->m_sn = m_sn;
  u->m_bc = allocateBCRegion(m_bc, m_bclen);
  u->m_bclen = m_bclen;
  if (m_bc_meta_len) {
    u->m_bc_meta = allocateBCRegion(m_bc_meta, m_bc_meta_len);
    u->m_bc_meta_len = m_bc_meta_len;
  }
  u->m_filepath = m_filepath;
  u->m_mainReturn = m_mainReturn;
  u->m_mergeOnly = m_mergeOnly;
  {
    const std::string& dirname = FileUtil::safe_dirname(m_filepath->data(),
                                                        m_filepath->size());
    u->m_dirpath = makeStaticString(dirname);
  }
  u->m_md5 = m_md5;
  for (unsigned i = 0; i < m_litstrs.size(); ++i) {
    NamedEntityPair np;
    np.first = m_litstrs[i];
    np.second = nullptr;
    u->m_namedInfo.push_back(np);
  }
  for (unsigned i = 0; i < m_arrays.size(); ++i) {
    u->m_arrays.push_back(m_arrays[i].array);
  }
  for (PceVec::const_iterator it = m_pceVec.begin(); it != m_pceVec.end();
       ++it) {
    u->m_preClasses.push_back(PreClassPtr((*it)->create(*u)));
  }
  u->m_typeAliases = m_typeAliases;

  size_t ix = m_fes.size() + m_hoistablePceIdList.size();
  if (m_mergeOnly && !m_allClassesHoistable) {
    size_t extra = 0;
    for (auto it = m_mergeableStmts.begin();
         it != m_mergeableStmts.end(); ++it) {
      extra++;
      if (!RuntimeOption::RepoAuthoritative && SystemLib::s_inited) {
        if (it->first != UnitMergeKindClass) {
          extra = 0;
          u->m_mergeOnly = false;
          break;
        }
      } else switch (it->first) {
          case UnitMergeKindPersistentDefine:
          case UnitMergeKindDefine:
          case UnitMergeKindGlobal:
            extra += sizeof(TypedValueAux) / sizeof(void*);
            break;
          default:
            break;
        }
    }
    ix += extra;
  }
  UnitMergeInfo *mi = UnitMergeInfo::alloc(ix);
  u->m_mergeInfo = mi;
  ix = 0;
  for (auto it = m_fes.begin(); it != m_fes.end(); ++it) {
    Func* func = (*it)->create(*u);
    if (func->top()) {
      if (!mi->m_firstHoistableFunc) {
        mi->m_firstHoistableFunc = ix;
      }
    } else {
      assert(!mi->m_firstHoistableFunc);
    }
    mi->mergeableObj(ix++) = func;
  }
  assert(u->getMain()->isPseudoMain());
  if (!mi->m_firstHoistableFunc) {
    mi->m_firstHoistableFunc =  ix;
  }
  mi->m_firstHoistablePreClass = ix;
  assert(m_fes.size());
  for (auto& id : m_hoistablePceIdList) {
    mi->mergeableObj(ix++) = u->m_preClasses[id].get();
  }
  mi->m_firstMergeablePreClass = ix;
  if (u->m_mergeOnly && !m_allClassesHoistable) {
    for (auto it = m_mergeableStmts.begin();
         it != m_mergeableStmts.end(); ++it) {
      switch (it->first) {
        case UnitMergeKindClass:
          mi->mergeableObj(ix++) = u->m_preClasses[it->second].get();
          break;
        case UnitMergeKindReqDoc: {
          assert(RuntimeOption::RepoAuthoritative);
          void* name = u->lookupLitstrId(it->second);
          mi->mergeableObj(ix++) = (char*)name + (int)it->first;
          break;
        }
        case UnitMergeKindDefine:
        case UnitMergeKindGlobal:
          assert(RuntimeOption::RepoAuthoritative);
        case UnitMergeKindPersistentDefine: {
          void* name = u->lookupLitstrId(m_mergeableValues[it->second].first);
          mi->mergeableObj(ix++) = (char*)name + (int)it->first;
          auto& tv = m_mergeableValues[it->second].second;
          auto* tva = (TypedValueAux*)mi->mergeableData(ix);
          tva->m_data = tv.m_data;
          tva->m_type = tv.m_type;
          // leave tva->m_aux uninitialized
          ix += sizeof(*tva) / sizeof(void*);
          assert(sizeof(*tva) % sizeof(void*) == 0);
          break;
        }
        case UnitMergeKindDone:
        case UnitMergeKindUniqueDefinedClass:
          not_reached();
      }
    }
  }
  assert(ix == mi->m_mergeablesSize);
  mi->mergeableObj(ix) = (void*)UnitMergeKindDone;
  u->m_sourceLocTable = createSourceLocTable();
  if (m_lineTable.size() == 0) {
    u->m_lineTable = createLineTable(m_sourceLocTab, m_bclen);
  } else {
    u->m_lineTable = m_lineTable;
  }
  u->m_lineToOffsetRangeVecMap = createLineToOffsetMap(m_sourceLocTab, m_bclen);
  for (size_t i = 0; i < m_feTab.size(); ++i) {
    assert(m_feTab[i].second->past() == m_feTab[i].first);
    assert(m_fMap.find(m_feTab[i].second) != m_fMap.end());
    u->m_funcTable.push_back(
      FuncEntry(m_feTab[i].first, m_fMap.find(m_feTab[i].second)->second));
  }

  // Funcs can be recorded out of order when loading them from the
  // repo currently.  So sort 'em here.
  std::sort(u->m_funcTable.begin(), u->m_funcTable.end());

  m_fMap.clear();

  if (RuntimeOption::EvalDumpBytecode) {
    // Dump human-readable bytecode.
    Trace::traceRelease("%s", u->toString().c_str());
  }
  if (RuntimeOption::EvalDumpHhas && SystemLib::s_inited) {
    std::printf("%s", disassemble(u).c_str());
    std::fflush(stdout);
    _Exit(0);
  }

  static const bool kVerify = debug || getenv("HHVM_VERIFY");
  static const bool kVerifyVerboseSystem =
    getenv("HHVM_VERIFY_VERBOSE_SYSTEM");
  static const bool kVerifyVerbose =
    kVerifyVerboseSystem || getenv("HHVM_VERIFY_VERBOSE");

  const bool isSystemLib = u->filepath()->empty() ||
    boost::ends_with(u->filepath()->data(), "systemlib.php");
  const bool doVerify =
    kVerify || boost::ends_with(u->filepath()->data(), "hhas");
  if (doVerify) {
    Verifier::checkUnit(
      u,
      isSystemLib ? kVerifyVerboseSystem : kVerifyVerbose
    );
  }

  return u;
}

///////////////////////////////////////////////////////////////////////////////
}
