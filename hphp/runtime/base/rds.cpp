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
#include "hphp/runtime/base/rds.h"

#include <cassert>
#include <vector>
#include <string>
#include <cstdio>
#include <mutex>

#include <sys/mman.h>

#include "folly/Hash.h"

#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/strings.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/jit/annotation.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/util/trace.h"
#include "hphp/util/base.h"
#include "hphp/util/maphuge.h"

using namespace HPHP::MethodLookup;
using namespace HPHP::Util;

namespace HPHP { namespace RDS {

//////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(targetcache);

namespace {

//////////////////////////////////////////////////////////////////////

// Current allocation frontier for the non-persistent region.
size_t s_frontier = sizeof(Header);

// Frontier and base of the persistent region.
size_t s_persistent_base = 0;
size_t s_persistent_frontier = 0;

//////////////////////////////////////////////////////////////////////

struct SymbolEq : boost::static_visitor<bool> {
  template<class T, class U>
  typename std::enable_if<
    !std::is_same<T,U>::value,
    bool
  >::type operator()(const T&, const U&) const { return false; }

  bool operator()(StaticLocal k1, StaticLocal k2) const {
    assert(k1.name->isStatic() && k2.name->isStatic());
    return k1.funcId == k2.funcId && k1.name == k2.name;
  }

  bool operator()(ClsConstant k1, ClsConstant k2) const {
    assert(k1.clsName->isStatic() && k1.cnsName->isStatic());
    assert(k2.clsName->isStatic() && k2.cnsName->isStatic());
    return k1.clsName->isame(k2.clsName) &&
           k1.cnsName == k2.cnsName;
  }

  bool operator()(StaticProp k1, StaticProp k2) const {
    assert(k1.name->isStatic() && k2.name->isStatic());
    return k1.name == k2.name;
  }

  template<class T>
  typename std::enable_if<
    std::is_same<T,StaticMethod>::value ||
      std::is_same<T,StaticMethodF>::value,
    bool
  >::type operator()(const T& t1, const T& t2) const {
    assert(t1.name->isStatic() && t2.name->isStatic());
    return t1.name->isame(t2.name);
  }
};

struct SymbolHash : boost::static_visitor<size_t> {
  size_t operator()(StaticLocal k) const {
    return folly::hash::hash_128_to_64(
      std::hash<FuncId>()(k.funcId),
      k.name->hash()
    );
  }

  size_t operator()(ClsConstant k) const {
    return folly::hash::hash_128_to_64(
      k.clsName->hash(),
      k.cnsName->hash()
    );
  }

  size_t operator()(StaticProp k)    const { return k.name->hash(); }
  size_t operator()(StaticMethod k)  const { return k.name->hash(); }
  size_t operator()(StaticMethodF k) const { return k.name->hash(); }
};

struct HashCompare {
  bool equal(const Symbol& k1, const Symbol& k2) const {
    return boost::apply_visitor(SymbolEq(), k1, k2);
  }

  size_t hash(const Symbol& k) const {
    return boost::apply_visitor(SymbolHash(), k);
  }
};

typedef tbb::concurrent_hash_map<
  Symbol,
  Handle,
  HashCompare
> LinkTable;

LinkTable s_linkTable;

//////////////////////////////////////////////////////////////////////

/*
 * This mutex protects actually allocating from RDS.  It is ordered
 * *after* the locks in s_linkTable.
 */
SimpleMutex s_allocMutex(false /*recursive*/, RankLeaf);

//////////////////////////////////////////////////////////////////////

template<class T = void>
T* handleToPtr(Handle h) {
  assert(h < RuntimeOption::EvalJitTargetCacheSize);
  return (T*)((char*)tl_base + h);
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

namespace detail {

  Handle alloc(Mode mode, size_t numBytes, size_t align) {
    s_allocMutex.assertOwnedBySelf();
    align = Util::roundUpToPowerOfTwo(align);
    auto& frontier = mode == Mode::Persistent ? s_persistent_frontier
                                              : s_frontier;

    frontier += align - 1;
    frontier &= ~(align - 1);
    frontier += numBytes;

    auto const limit = mode == Mode::Persistent
      ? RuntimeOption::EvalJitTargetCacheSize
      : s_persistent_base;
    always_assert(frontier < limit);

    return frontier - numBytes;
  }

  Handle allocUnlocked(Mode mode, size_t numBytes, size_t align) {
    SimpleLock l(s_allocMutex);
    return alloc(mode, numBytes, align);
  }

  /*
   * TODO: the locking here is still a little too weird
   */
  Handle bindImpl(Symbol key, Mode mode, size_t sizeBytes, size_t align) {
    LinkTable::const_accessor acc;
    if (s_linkTable.find(acc, key)) return acc->second;

    SimpleLock l(s_allocMutex);
    if (s_linkTable.find(acc, key)) return acc->second;

    auto const retval = alloc(mode, sizeBytes, align);
    if (!s_linkTable.insert(LinkTable::value_type(key, retval))) {
      always_assert(0);
    }
    return retval;
  }

  void bindOnLinkImpl(std::atomic<Handle>& handle,
                      Mode mode,
                      size_t sizeBytes,
                      size_t align) {
    SimpleLock l(s_allocMutex);
    if (handle.load(std::memory_order_relaxed) == kInvalidHandle) {
      handle.store(alloc(mode, sizeBytes, align), std::memory_order_relaxed);
    }
  }

}


//////////////////////////////////////////////////////////////////////

const StaticString s_call("__call");

__thread void* tl_base = nullptr;
static __thread std::aligned_storage<
  sizeof(Array),
  alignof(Array)
>::type s_constantsStorage;

//////////////////////////////////////////////////////////////////////

static size_t s_next_bit;
static size_t s_bits_to_go;
static int s_tc_fd;

// Mapping from names to targetcache locations.
typedef tbb::concurrent_hash_map<const StringData*, Handle,
        StringDataHashICompare>
  HandleMapIS;

typedef tbb::concurrent_hash_map<const StringData*, Handle,
        StringDataHashCompare>
  HandleMapCS;

//////////////////////////////////////////////////////////////////////

void requestInit() {
  assert(tl_base);
  new (&s_constantsStorage) Array();
  assert(!s_constants().get());
  TRACE(1, "RDS: @%p\n", tl_base);
  memset(tl_base, 0, s_frontier);
}

void requestExit() {
  s_constants().detach(); // it will be swept
  // Don't bother running the dtor ...
}

void flush() {
  TRACE(1, "RDS: MADV_DONTNEED %zd bytes: %p\n", s_frontier,
        tl_base);
  if (madvise(tl_base, s_frontier, MADV_DONTNEED) < 0) {
    not_reached();
  }
}

size_t usedBytes() {
  return s_frontier;
}

size_t usedPersistentBytes() {
  return s_persistent_frontier - s_persistent_base;
}

Array& s_constants() {
  void* vp = &s_constantsStorage;
  return *static_cast<Array*>(vp);
}

//////////////////////////////////////////////////////////////////////

size_t allocBit() {
  SimpleLock l(s_allocMutex);
  if (!s_bits_to_go) {
    static const int kNumBytes = 512;
    static const int kNumBytesMask = kNumBytes - 1;
    s_next_bit = s_frontier * CHAR_BIT;
    // allocate at least kNumBytes bytes, and make sure we end
    // on a 64 byte aligned boundary.
    int bytes = ((~s_frontier + 1) & kNumBytesMask) + kNumBytes;
    s_bits_to_go = bytes * CHAR_BIT;
    s_frontier += bytes;
  }
  s_bits_to_go--;
  return s_next_bit++;
}

bool testAndSetBit(size_t bit) {
  Handle handle = bit / CHAR_BIT;
  unsigned char mask = 1 << (bit % CHAR_BIT);
  bool ret = *(unsigned char*)handleToPtr(handle) & mask;
  *(unsigned char*)handleToPtr(handle) |= mask;
  return ret;
}

bool isPersistentHandle(Handle handle) {
  assert(handle >= 0 && handle < RuntimeOption::EvalJitTargetCacheSize);
  return handle >= (unsigned)s_persistent_base;
}

bool classIsPersistent(const Class* cls) {
  return (RuntimeOption::RepoAuthoritative &&
          cls &&
          isPersistentHandle(cls->classHandle()));
}

static void initPersistentCache() {
  SimpleLock l(s_allocMutex);
  if (s_tc_fd) return;
  char tmpName[] = "/tmp/tcXXXXXX";
  s_tc_fd = mkstemp(tmpName);
  always_assert(s_tc_fd != -1);
  unlink(tmpName);
  s_persistent_base = RuntimeOption::EvalJitTargetCacheSize * 3 / 4;
  s_persistent_base -= s_persistent_base & (4 * 1024 - 1);
  ftruncate(s_tc_fd,
            RuntimeOption::EvalJitTargetCacheSize - s_persistent_base);
  s_persistent_frontier = s_persistent_base;
}

void threadInit() {
  if (!s_tc_fd) {
    initPersistentCache();
  }

  tl_base = mmap(nullptr, RuntimeOption::EvalJitTargetCacheSize,
                 PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
  always_assert(tl_base != MAP_FAILED);
  Util::numa_bind_to(tl_base, s_persistent_base, Util::s_numaNode);
  if (RuntimeOption::EvalMapTgtCacheHuge) {
    hintHuge(tl_base, RuntimeOption::EvalJitTargetCacheSize);
  }

  void *shared_base = (char*)tl_base + s_persistent_base;
  /*
   * map the upper portion of the RDS to a shared area This is used
   * for persistent classes and functions, so they are always defined,
   * and always visible to all threads.
   */
  void *mem = mmap(shared_base,
                   RuntimeOption::EvalJitTargetCacheSize - s_persistent_base,
                   PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, s_tc_fd, 0);
  always_assert(mem == shared_base);
}

void threadExit() {
  munmap(tl_base, RuntimeOption::EvalJitTargetCacheSize);
}

static inline bool
stringMatches(const StringData* rowString, const StringData* sd) {
  return rowString &&
    (rowString == sd ||
     rowString->data() == sd->data() ||
     (rowString->hash() == sd->hash() &&
      rowString->same(sd)));
}

//=============================================================================
// FuncCache

// Set of FuncCache handles for dynamic function callsites, used for
// invalidation when a function is renamed.
static std::mutex funcCacheMutex;
static std::vector<Link<FuncCache> > funcCacheEntries;

template<>
Handle FuncCache::alloc() {
  auto const link = RDS::alloc<FuncCache,sizeof(Pair)>();
  std::lock_guard<std::mutex> g(funcCacheMutex);
  funcCacheEntries.push_back(link);
  return link.handle();
}

void invalidateForRenameFunction(const StringData* name) {
  assert(name);
  std::lock_guard<std::mutex> g(funcCacheMutex);
  for (auto& h : funcCacheEntries) {
    memset(h.get(), 0, sizeof *h);
  }
}

template<>
inline int
FuncCache::hashKey(const StringData* sd) {
  return sd->hash();
}

template<>
const Func*
FuncCache::lookup(Handle handle, StringData *sd, const void* /* ignored */) {
  auto const thiz = handleToPtr<FuncCache>(handle);
  Func* func;
  Pair* pair = thiz->keyToPair(sd);
  const StringData* pairSd = pair->m_key;
  if (!stringMatches(pairSd, sd)) {
    // Miss. Does it actually exist?
    func = Unit::lookupFunc(sd);
    if (UNLIKELY(!func)) {
      Transl::VMRegAnchor _;
      func = Unit::loadFunc(sd);
      if (!func) {
        raise_error("Undefined function: %s", sd->data());
      }
    }
    func->validate();
    pair->m_key = func->name(); // use a static name
    pair->m_value = func;
  }
  // DecRef the string here; more compact than doing so in callers.
  decRefStr(sd);
  assert(stringMatches(pair->m_key, pair->m_value->name()));
  pair->m_value->validate();
  return pair->m_value;
}

//=============================================================================
// MethodCache

inline int MethodCache::hashKey(uintptr_t c) {
  pointer_hash<Class> h;
  return h(reinterpret_cast<const Class*>(c));
}

/*
 * We have a call site for an object method, which previously invoked
 * func, but this call has a different Class (cls).  See if we can
 * figure out the correct Func to call.
 */
static inline const Func* wouldCall(const Class* cls, const Func* prev) {
  if (LIKELY(cls->numMethods() > prev->methodSlot())) {
    const Func* cand = cls->methods()[prev->methodSlot()];
    /* If this class has the same func at the same method slot
       we're good to go. No need to recheck permissions,
       since we already checked them first time around */
    if (LIKELY(cand == prev)) return cand;
    if (prev->attrs() & AttrPrivate) {
      /* If the previously called function was private, then
         the context class must be prev->cls() - so its
         definitely accessible. So if this derives from
         prev->cls() its the function that would be picked.
         Note that we can only get here if there is a same
         named function deeper in the class hierarchy */
      if (cls->classof(prev->cls())) return prev;
    }
    if (cand->name() == prev->name()) {
      /*
       * We have the same name - so its probably the right function.
       * If its not public, check that both funcs were originally
       * defined in the same base class.
       */
      if ((cand->attrs() & AttrPublic) ||
          cand->baseCls() == prev->baseCls()) {
        return cand;
      }
    }
  }
  return nullptr;
}

/*
 * This is flagged NEVER_INLINE because if gcc inlines it, it will
 * hoist a bunch of initialization code (callee-saved regs pushes,
 * making a frame, and rsp adjustment) above the fast path.  When not
 * inlined, gcc is generating a jmp to this function instead of a
 * call.
 */
HOT_FUNC_VM NEVER_INLINE
void methodCacheSlowPath(MethodCache::Pair* mce,
                         ActRec* ar,
                         StringData* name,
                         Class* cls) {
  assert(ar->hasThis());
  assert(ar->getThis()->getVMClass() == cls);
  assert(IMPLIES(mce->m_key, mce->m_value));

  try {
    bool isMagicCall = mce->m_key & 0x1u;
    bool isStatic;
    const Func* func;

    auto* storedClass = reinterpret_cast<Class*>(mce->m_key & ~0x3u);
    if (storedClass == cls) {
      isStatic = mce->m_key & 0x2u;
      func = mce->m_value;
    } else {
      if (LIKELY(storedClass != nullptr &&
                 ((func = wouldCall(cls, mce->m_value)) != nullptr) &&
                 !isMagicCall)) {
        Stats::inc(Stats::TgtCache_MethodHit, func != nullptr);
        isMagicCall = false;
      } else {
        Class* ctx = arGetContextClass((ActRec*)ar->m_savedRbp);
        Stats::inc(Stats::TgtCache_MethodMiss);
        TRACE(2, "MethodCache: miss class %p name %s!\n", cls, name->data());
        auto const& objMethod = MethodLookup::CallType::ObjMethod;
        func = g_vmContext->lookupMethodCtx(cls, name, ctx, objMethod, false);
        if (UNLIKELY(!func)) {
          isMagicCall = true;
          func = cls->lookupMethod(s_call.get());
          if (UNLIKELY(!func)) {
            // Do it again, but raise the error this time.
            (void) g_vmContext->lookupMethodCtx(cls, name, ctx, objMethod,
                                                true);
            NOT_REACHED();
          }
        } else {
          isMagicCall = false;
        }
      }

      isStatic = func->attrs() & AttrStatic;

      mce->m_key = uintptr_t(cls) | (uintptr_t(isStatic) << 1) |
        uintptr_t(isMagicCall);
      mce->m_value = func;
    }

    assert(func);
    func->validate();
    ar->m_func = func;

    if (UNLIKELY(isStatic && !func->isClosureBody())) {
      decRefObj(ar->getThis());
      if (debug) ar->setThis(nullptr); // suppress assert in setClass
      ar->setClass(cls);
    }

    assert(!ar->hasVarEnv() && !ar->hasInvName());
    if (UNLIKELY(isMagicCall)) {
      ar->setInvName(name);
      assert(name->isStatic()); // No incRef needed.
    }
  } catch (...) {
    /*
     * Barf.
     *
     * If the slow lookup fails, we're going to rewind to the state
     * before the FPushObjMethodD that dumped us here. In this state,
     * the object is still on the stack, but for efficiency reasons,
     * we've smashed this TypedValue* with the ActRec we were trying
     * to push.
     *
     * Reconstitute the virtual object before rethrowing.
     */
    TypedValue* shouldBeObj = reinterpret_cast<TypedValue*>(ar) +
      kNumActRecCells - 1;
    ObjectData* arThis = ar->getThis();
    shouldBeObj->m_type = KindOfObject;
    shouldBeObj->m_data.pobj = arThis;

    // There used to be a half-built ActRec on the stack that we need the
    // unwinder to ignore. We overwrote 1/3 of it with the code above, but
    // because of the emitMarker() in LdObjMethod we need the other two slots
    // to not have any TypedValues.
    tvWriteNull(shouldBeObj - 1);
    tvWriteNull(shouldBeObj - 2);

    throw;
  }
}

//=============================================================================
// ClassCache

template<>
inline int
ClassCache::hashKey(StringData* sd) {
  return sd->hash();
}

template<>
const Class*
ClassCache::lookup(Handle handle, StringData *name,
                   const void* unused) {
  auto const thiz = handleToPtr<ClassCache>(handle);
  Pair *pair = thiz->keyToPair(name);
  const StringData* pairSd = pair->m_key;
  if (!stringMatches(pairSd, name)) {
    TRACE(1, "ClassCache miss: %s\n", name->data());
    const NamedEntity *ne = Unit::GetNamedEntity(name);
    Class *c = Unit::lookupClass(ne);
    if (UNLIKELY(!c)) {
      String normName = normalizeNS(name);
      if (normName) {
        return lookup(handle, normName.get(), unused);
      } else {
        c = Unit::loadMissingClass(ne, name);
      }
      if (UNLIKELY(!c)) {
        raise_error(Strings::UNKNOWN_CLASS, name->data());
      }
    }

    if (pair->m_key) decRefStr(pair->m_key);
    pair->m_key = name;
    name->incRefCount();
    pair->m_value = c;
  } else {
    TRACE(1, "ClassCache hit: %s\n", name->data());
  }
  return pair->m_value;
}

//=============================================================================
// *SPropCache
//

Handle SPropCache::alloc(const StringData* sd) {
  assert(sd->isStatic());
  return bind<SPropCache>(
    StaticProp { sd }
  ).handle();
}

template<bool raiseOnError>
TypedValue*
SPropCache::lookupSProp(const Class *cls, const StringData *name, Class* ctx) {
  bool visible, accessible;
  TypedValue* val;
  val = cls->getSProp(ctx, name, visible, accessible);
  if (UNLIKELY(!visible || !accessible)) {
    if (!raiseOnError) return NULL;
    std::string propertyName;
    string_printf(propertyName, "%s::%s",
                  cls->name()->data(), name->data());
    raise_error("Invalid static property access: %s", propertyName.c_str());
  }
  return val;
}

template TypedValue* SPropCache::lookupSProp<true>(const Class *cls,
                                                   const StringData *name,
                                                   Class* ctx);

template TypedValue* SPropCache::lookupSProp<false>(const Class *cls,
                                                    const StringData *name,
                                                    Class* ctx);

template<bool raiseOnError>
TypedValue*
SPropCache::lookup(Handle handle, const Class *cls, const StringData *name,
                   Class* ctx) {
  // The fast path is in-TC. If we get here, we have already missed.
  auto const thiz = handleToPtr<SPropCache>(handle);
  Stats::inc(Stats::TgtCache_SPropMiss);
  Stats::inc(Stats::TgtCache_SPropHit, -1);
  assert(cls && name);
  assert(!thiz->m_tv);
  TRACE(3, "SPropCache miss: %s::$%s\n", cls->name()->data(),
        name->data());
  TypedValue* val = lookupSProp<raiseOnError>(cls, name, ctx);
  if (!val) {
    assert(!raiseOnError);
    return NULL;
  }
  thiz->m_tv = val;
  TRACE(3, "SPropCache::lookup(\"%s::$%s\") %p -> %p t%d\n",
        cls->name()->data(),
        name->data(),
        val,
        val->m_data.pref,
        val->m_type);
  assert(val->m_type >= MinDataType && val->m_type < MaxNumDataTypes);
  return val;
}

template TypedValue* SPropCache::lookup<true>(Handle handle,
                                              const Class *cls,
                                              const StringData *name,
                                              Class* ctx);

template TypedValue* SPropCache::lookup<false>(Handle handle,
                                               const Class *cls,
                                               const StringData *name,
                                               Class* ctx);

//=============================================================================
// StaticMethodCache
//

static const StringData* mangleStaticMethodCacheName(const StringData* cls,
                                                     const StringData* meth,
                                                     const char* ctx) {
  // Implementation detail of FPushClsMethodD/F: we use "C::M:ctx" as
  // the key for invoking static method "M" on class "C". This
  // composes such a key. "::" is semi-arbitrary, though whatever we
  // choose must delimit possible class and method names, so we might
  // as well ape the source syntax
  return
    makeStaticString(String(cls->data()) + String("::") +
                     String(meth->data()) + String(":") +
                     String(ctx));
}

Handle StaticMethodCache::alloc(const StringData* clsName,
                                const StringData* methName,
                                const char* ctxName) {
  return bind<StaticMethodCache>(
    StaticMethod { mangleStaticMethodCacheName(clsName, methName, ctxName) }
  ).handle();
}

Handle StaticMethodFCache::alloc(const StringData* clsName,
                                 const StringData* methName,
                                 const char* ctxName) {
  return bind<StaticMethodFCache>(
    StaticMethodF { mangleStaticMethodCacheName(clsName, methName, ctxName) }
  ).handle();
}

const Func*
StaticMethodCache::lookupIR(Handle handle, const NamedEntity *ne,
                            const StringData* clsName,
                            const StringData* methName, TypedValue* vmfp,
                            TypedValue* vmsp) {
  StaticMethodCache* thiz = static_cast<StaticMethodCache*>
    (handleToPtr(handle));
  Stats::inc(Stats::TgtCache_StaticMethodMiss);
  Stats::inc(Stats::TgtCache_StaticMethodHit, -1);
  TRACE(1, "miss %s :: %s caller %p\n",
        clsName->data(), methName->data(), __builtin_return_address(0));

  ActRec* ar = reinterpret_cast<ActRec*>(vmsp - kNumActRecCells);
  const Func* f;
  VMExecutionContext* ec = g_vmContext;
  const Class* cls = Unit::loadClass(ne, clsName);
  if (UNLIKELY(!cls)) {
    raise_error(Strings::UNKNOWN_CLASS, clsName->data());
  }
  LookupResult res = ec->lookupClsMethod(f, cls, methName,
                                         nullptr, // there may be an active this,
                                               // but we can just fall through
                                               // in that case.
                                         arGetContextClass((ActRec*)vmfp),
                                         false /*raise*/);
  if (LIKELY(res == LookupResult::MethodFoundNoThis &&
             !f->isAbstract() &&
             f->isStatic())) {
    f->validate();
    TRACE(1, "fill %s :: %s -> %p\n", clsName->data(),
          methName->data(), f);
    // Do the | here instead of on every call.
    thiz->m_cls = (Class*)(uintptr_t(cls) | 1);
    thiz->m_func = f;
    ar->setClass(const_cast<Class*>(cls));
    return f;
  }
  assert(res != LookupResult::MethodFoundWithThis); // Not possible: no this.

  // Indicate to the IR that it should take even slower path
  return nullptr;
}

const Func*
StaticMethodCache::lookup(Handle handle, const NamedEntity *ne,
                          const StringData* clsName,
                          const StringData* methName) {
  StaticMethodCache* thiz = static_cast<StaticMethodCache*>
    (handleToPtr(handle));
  Stats::inc(Stats::TgtCache_StaticMethodMiss);
  Stats::inc(Stats::TgtCache_StaticMethodHit, -1);
  TRACE(1, "miss %s :: %s caller %p\n",
        clsName->data(), methName->data(), __builtin_return_address(0));
  Transl::VMRegAnchor _; // needed for lookupClsMethod.

  ActRec* ar = reinterpret_cast<ActRec*>(vmsp() - kNumActRecCells);
  const Func* f;
  VMExecutionContext* ec = g_vmContext;
  const Class* cls = Unit::loadClass(ne, clsName);
  if (UNLIKELY(!cls)) {
    raise_error(Strings::UNKNOWN_CLASS, clsName->data());
  }
  LookupResult res = ec->lookupClsMethod(f, cls, methName,
                                         nullptr, // there may be an active this,
                                               // but we can just fall through
                                               // in that case.
                                         arGetContextClass(ec->getFP()),
                                         false /*raise*/);
  if (LIKELY(res == LookupResult::MethodFoundNoThis &&
             !f->isAbstract() &&
             f->isStatic())) {
    f->validate();
    TRACE(1, "fill %s :: %s -> %p\n", clsName->data(),
          methName->data(), f);
    // Do the | here instead of on every call.
    thiz->m_cls = (Class*)(uintptr_t(cls) | 1);
    thiz->m_func = f;
    ar->setClass(const_cast<Class*>(cls));
    return f;
  }
  assert(res != LookupResult::MethodFoundWithThis); // Not possible: no this.
  // We've already sync'ed regs; this is some hard case, we might as well
  // just let the interpreter handle this entirely.
  assert(toOp(*vmpc()) == OpFPushClsMethodD);
  Stats::inc(Stats::Instr_InterpOneFPushClsMethodD);
  Stats::inc(Stats::Instr_TC, -1);
  ec->opFPushClsMethodD();
  // Return whatever func the instruction produced; if nothing was
  // possible we'll either have fataled or thrown.
  assert(ar->m_func);
  ar->m_func->validate();
  // Don't update the cache; this case was too scary to memoize.
  TRACE(1, "unfillable miss %s :: %s -> %p\n", clsName->data(),
        methName->data(), ar->m_func);
  // Indicate to the caller that there is no work to do.
  return nullptr;
}

const Func*
StaticMethodFCache::lookupIR(Handle handle, const Class* cls,
                             const StringData* methName, TypedValue* vmfp) {
  assert(cls);
  StaticMethodFCache* thiz = static_cast<StaticMethodFCache*>
    (handleToPtr(handle));
  Stats::inc(Stats::TgtCache_StaticMethodFMiss);
  Stats::inc(Stats::TgtCache_StaticMethodFHit, -1);

  const Func* f;
  VMExecutionContext* ec = g_vmContext;
  LookupResult res = ec->lookupClsMethod(f, cls, methName,
                                         nullptr,
                                         arGetContextClass((ActRec*)vmfp),
                                         false /*raise*/);
  assert(res != LookupResult::MethodFoundWithThis); // Not possible: no this.
  if (LIKELY(res == LookupResult::MethodFoundNoThis && !f->isAbstract())) {
    // We called lookupClsMethod with a NULL this and got back a
    // method that may or may not be static. This implies that
    // lookupClsMethod, given the same class and the same method name,
    // will never return MagicCall*Found or MethodNotFound. It will
    // always return the same f and if we do give it a this it will
    // return MethodFoundWithThis iff (this->instanceof(cls) &&
    // !f->isStatic()). this->instanceof(cls) is always true for
    // FPushClsMethodF because it is only used for self:: and parent::
    // calls. So, if we store f and its staticness we can handle calls
    // with and without this completely in assembly.
    f->validate();
    thiz->m_func = f;
    thiz->m_static = f->isStatic();
    TRACE(1, "fill staticfcache %s :: %s -> %p\n",
          cls->name()->data(), methName->data(), f);
    Stats::inc(Stats::TgtCache_StaticMethodFFill);
    return f;
  }

  return nullptr;
}

//////////////////////////////////////////////////////////////////////

}}
