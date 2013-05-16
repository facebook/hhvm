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
#include "hphp/runtime/vm/translator/targetcache.h"
#include "hphp/runtime/base/complex_types.h"
#include "hphp/runtime/base/execution_context.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/strings.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/translator/annotation.h"
#include "hphp/runtime/vm/translator/translator-inline.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/util/trace.h"
#include "hphp/util/base.h"
#include "hphp/util/maphuge.h"

#include <string>
#include <stdio.h>
#include <sys/mman.h>

using namespace HPHP::MethodLookup;
using namespace HPHP::Util;
using std::string;

/*
 * The targetcache module provides a set of per-request caches.
 */
namespace HPHP {

/*
 * Put this where the compiler has a chance to inline it.
 */
inline const Func* Class::wouldCall(const Func* prev) const {
  if (LIKELY(m_methods.size() > prev->methodSlot())) {
    const Func* cand = m_methods[prev->methodSlot()];
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
      if (this->classof(prev->cls())) return prev;
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

namespace Transl {
namespace TargetCache {

TRACE_SET_MOD(targetcache);

static StaticString s___call(LITSTR_INIT("__call"));

// Shorthand.
typedef CacheHandle Handle;

// Helper for lookup failures. msg should be a printf-style static
// format with one %s parameter, which name will be substituted into.
void
undefinedError(const char* msg, const char* name) {
  raise_error(msg, name);
}

// Targetcache memory. See the comment in targetcache.h
__thread void* tl_targetCaches = nullptr;
__thread HphpArray* s_constants = nullptr;

static_assert(kConditionFlagsOff + sizeof(ssize_t) <= 64,
              "kConditionFlagsOff too large");
size_t s_frontier = kConditionFlagsOff + 64;
size_t s_persistent_frontier = 0;
size_t s_persistent_start = 0;
static size_t s_next_bit;
static size_t s_bits_to_go;
static int s_tc_fd;
static const size_t kPreAllocatedBytes = kConditionFlagsOff + 64;

// Mapping from names to targetcache locations. Protected by the translator
// write lease.
typedef tbb::concurrent_hash_map<const StringData*, Handle,
        StringDataHashICompare>
  HandleMapIS;

typedef tbb::concurrent_hash_map<const StringData*, Handle,
        StringDataHashCompare>
  HandleMapCS;

// handleMaps[NSConstant]['FOO'] is the cache associated with the constant
// FOO, eg. handleMaps is a rare instance of shared, mutable state across
// the request threads in the translator: it is essentially a lazily
// constructed link table for tl_targetCaches.
HandleMapIS handleMapsIS[NumInsensitive];
HandleMapCS handleMapsCS[NumCaseSensitive];

// Vector of cache handles
typedef std::vector<Handle> HandleVector;

// Set of FuncCache handles for dynamic function callsites, used for
// invalidation when a function is renamed.
HandleVector funcCacheEntries;

static Mutex s_handleMutex(false /*recursive*/, RankLeaf);

inline Handle
ptrToHandle(const void* ptr) {
  ptrdiff_t retval = uintptr_t(ptr) - uintptr_t(tl_targetCaches);
  assert(retval < RuntimeOption::EvalJitTargetCacheSize);
  return retval;
}

template <bool sensitive>
class HandleInfo {
public:
  typedef HandleMapIS Map;
  static Map &getHandleMap(int where) {
    return handleMapsIS[where];
  }
};

template <>
class HandleInfo<true> {
public:
  typedef HandleMapCS Map;
  static Map &getHandleMap(int where) {
    return handleMapsCS[where - FirstCaseSensitive];
  }
};

#define getHMap(where) \
  HandleInfo<where >= FirstCaseSensitive>::getHandleMap(where)

static size_t allocBitImpl(const StringData* name, PHPNameSpace ns) {
  ASSERT_NOT_IMPLEMENTED(ns == NSInvalid || ns >= FirstCaseSensitive);
  HandleMapCS& map = HandleInfo<true>::getHandleMap(ns);
  HandleMapCS::const_accessor a;
  if (name != nullptr && ns != NSInvalid && map.find(a, name)) {
    return a->second;
  }
  Lock l(s_handleMutex);
  if (name != nullptr && ns != NSInvalid && map.find(a, name)) {
    // Retry under the lock.
    return a->second;
  }
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
  if (name != nullptr && ns != NSInvalid) {
    if (!name->isStatic()) name = StringData::GetStaticString(name);
    if (!map.insert(HandleMapCS::value_type(name, s_next_bit)))
      NOT_REACHED();
  }
  return s_next_bit++;
}

size_t allocBit() {
  return allocBitImpl(nullptr, NSInvalid);
}

Handle bitOffToHandleAndMask(size_t bit, uint8_t &mask) {
  static_assert(!(8 % CHAR_BIT), "Unexpected size of char");
  mask = (uint8_t)1 << (bit % 8);
  size_t off = bit / CHAR_BIT;
  off -= off % (8 / CHAR_BIT);
  return off;
}

bool testBit(size_t bit) {
  Handle handle = bit / CHAR_BIT;
  unsigned char mask = 1 << (bit % CHAR_BIT);
  return *(unsigned char*)handleToPtr(handle) & mask;
}

bool testBit(Handle handle, uint32_t mask) {
  assert(!(mask & (mask - 1)));
  return *(uint32_t*)handleToPtr(handle) & mask;
}

bool testAndSetBit(size_t bit) {
  Handle handle = bit / CHAR_BIT;
  unsigned char mask = 1 << (bit % CHAR_BIT);
  bool ret = *(unsigned char*)handleToPtr(handle) & mask;
  *(unsigned char*)handleToPtr(handle) |= mask;
  return ret;
}

bool testAndSetBit(Handle handle, uint32_t mask) {
  assert(!(mask & (mask - 1)));
  bool ret = *(uint32_t*)handleToPtr(handle) & mask;
  *(uint32_t*)handleToPtr(handle) |= mask;
  return ret;
}

bool isPersistentHandle(Handle handle) {
  return handle >= (unsigned)s_persistent_start;
}

bool classIsPersistent(const Class* cls) {
  return (RuntimeOption::RepoAuthoritative &&
          cls &&
          isPersistentHandle(cls->m_cachedOffset));
}

static Handle allocLocked(bool persistent, int numBytes, int align) {
  s_handleMutex.assertOwnedBySelf();
  align = Util::roundUpToPowerOfTwo(align);
  size_t &frontier = persistent ? s_persistent_frontier : s_frontier;

  frontier += align - 1;
  frontier &= ~(align - 1);
  frontier += numBytes;

  always_assert(frontier < (persistent ?
                     RuntimeOption::EvalJitTargetCacheSize :
                     s_persistent_start));

  return frontier - numBytes;
}

// namedAlloc --
//   Many targetcache entries (Func, Class, Constant, ...) have
//   request-unique values. There is no reason to allocate more than
//   one item for all such calls in a request.
//
//   handleMaps acts as a de-facto dynamic link table that lives
//   across requests; the translator can write out code that assumes
//   that a given named entity's location in tl_targetCaches is
//   stable from request to request.
template<bool sensitive>
Handle
namedAlloc(PHPNameSpace where, const StringData* name,
           int numBytes, int align) {
  assert(!name || (where >= 0 && where < NumNameSpaces));
  typedef HandleInfo<sensitive> HI;
  typename HI::Map& map = HI::getHandleMap(where);
  typename HI::Map::const_accessor a;
  if (name && map.find(a, name)) {
    TRACE(2, "TargetCache: hit \"%s\", %d\n", name->data(), int(a->second));
    return a->second;
  }
  Lock l(s_handleMutex);
  if (name && map.find(a, name)) { // Retry under the lock
    TRACE(2, "TargetCache: hit \"%s\", %d\n", name->data(), int(a->second));
    return a->second;
  }
  Handle retval = allocLocked(where == NSPersistent, numBytes, align);
  if (name) {
    if (!name->isStatic()) name = StringData::GetStaticString(name);
    if (!map.insert(typename HI::Map::value_type(name, retval))) NOT_REACHED();
    TRACE(1, "TargetCache: inserted \"%s\", %d\n", name->data(), int(retval));
  } else if (where == NSDynFunction) {
    funcCacheEntries.push_back(retval);
  }
  return retval;
}

template
Handle namedAlloc<true>(PHPNameSpace where, const StringData* name,
                        int numBytes, int align);
template
Handle namedAlloc<false>(PHPNameSpace where, const StringData* name,
                         int numBytes, int align);

void
invalidateForRename(const StringData* name) {
  assert(name);
  Lock l(s_handleMutex);

  for (HandleVector::iterator i = funcCacheEntries.begin();
       i != funcCacheEntries.end(); ++i) {
    FuncCache::invalidate(*i, name);
  }
}

void initPersistentCache() {
  Lock l(s_handleMutex);
  if (s_tc_fd) return;
  char tmpName[] = "/tmp/tcXXXXXX";
  s_tc_fd = mkstemp(tmpName);
  always_assert(s_tc_fd != -1);
  unlink(tmpName);
  s_persistent_start = RuntimeOption::EvalJitTargetCacheSize * 3 / 4;
  s_persistent_start -= s_persistent_start & (4 * 1024 - 1);
  ftruncate(s_tc_fd,
            RuntimeOption::EvalJitTargetCacheSize - s_persistent_start);
  s_persistent_frontier = s_persistent_start;
}

void threadInit() {
  if (!s_tc_fd) {
    initPersistentCache();
  }

  tl_targetCaches = mmap(nullptr, RuntimeOption::EvalJitTargetCacheSize,
                         PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
  always_assert(tl_targetCaches != MAP_FAILED);
  hintHuge(tl_targetCaches, RuntimeOption::EvalJitTargetCacheSize);

  void *shared_base = (char*)tl_targetCaches + s_persistent_start;
  /*
   * map the upper portion of the target cache to a shared area
   * This is used for persistent classes and functions, so they
   * are always defined, and always visible to all threads.
   */
  void *mem = mmap(shared_base,
                   RuntimeOption::EvalJitTargetCacheSize - s_persistent_start,
                   PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, s_tc_fd, 0);
  always_assert(mem == shared_base);
}

void threadExit() {
  munmap(tl_targetCaches, RuntimeOption::EvalJitTargetCacheSize);
}

static const bool zeroViaMemset = true;

void
requestInit() {
  assert(tl_targetCaches);
  assert(!s_constants);
  TRACE(1, "TargetCache: @%p\n", tl_targetCaches);
  if (zeroViaMemset) {
    TRACE(1, "TargetCache: bzeroing %zd bytes: %p\n", s_frontier,
          tl_targetCaches);
    memset(tl_targetCaches, 0, s_frontier);
  }
}

void
requestExit() {
  if (!zeroViaMemset) {
    flush();
  }
  s_constants = nullptr; // it will be swept
}

void
flush() {
  TRACE(1, "TargetCache: MADV_DONTNEED %zd bytes: %p\n", s_frontier,
        tl_targetCaches);
  if (madvise(tl_targetCaches, s_frontier, MADV_DONTNEED) < 0) {
    not_reached();
  }
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
template<>
inline int
FuncCache::hashKey(const StringData* sd) {
  return sd->hash();
}

template<>
const Func*
FuncCache::lookup(Handle handle, StringData *sd, const void* /* ignored */) {
  FuncCache* thiz = cacheAtHandle(handle);
  Func* func;
  Pair* pair = thiz->keyToPair(sd);
  const StringData* pairSd = pair->m_key;
  if (!stringMatches(pairSd, sd)) {
    // Miss. Does it actually exist?
    func = Unit::lookupFunc(sd);
    if (UNLIKELY(!func)) {
      VMRegAnchor _;
      func = Unit::loadFunc(sd);
      if (!func) {
        undefinedError("Undefined function: %s", sd->data());
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
// FixedFuncCache

const Func* FixedFuncCache::lookupUnknownFunc(StringData* name) {
  VMRegAnchor _;
  Func* func = Unit::loadFunc(name);
  if (UNLIKELY(!func)) {
    undefinedError("Undefined function: %s", name->data());
  }
  return func;
}

//=============================================================================
// MethodCache

template<>
inline int
MethodCache::hashKey(uintptr_t c) {
  pointer_hash<Class> h;
  return h(reinterpret_cast<const Class*>(c));
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
                 ((func = cls->wouldCall(mce->m_value)) != nullptr) &&
                 !isMagicCall)) {
        Stats::inc(Stats::TgtCache_MethodHit, func != nullptr);
        isMagicCall = false;
      } else {
        Class* ctx = arGetContextClass((ActRec*)ar->m_savedRbp);
        Stats::inc(Stats::TgtCache_MethodMiss);
        TRACE(2, "MethodCache: miss class %p name %s!\n", cls, name->data());
        func = g_vmContext->lookupMethodCtx(cls, name, ctx,
                                            MethodLookup::ObjMethod, false);
        if (UNLIKELY(!func)) {
          isMagicCall = true;
          func = cls->lookupMethod(s___call.get());
          if (UNLIKELY(!func)) {
            // Do it again, but raise the error this time.
            (void) g_vmContext->lookupMethodCtx(cls, name, ctx,
                                                MethodLookup::ObjMethod, true);
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
    throw;
  }
}

template<>
HOT_FUNC_VM
void
MethodCache::lookup(Handle handle, ActRec* ar, const void* extraKey) {
  assert(ar->hasThis());
  auto* cls = ar->getThis()->getVMClass();
  auto* pair = MethodCache::cacheAtHandle(handle)->keyToPair(uintptr_t(cls));

  /*
   * The MethodCache line consists of a Class* key (stored as a
   * uintptr_t) and a Func*.  The low bit of the key is set if the
   * function call is a magic call (in which case the cached Func* is
   * the __call function).  The second lowest bit of the key is set if
   * the cached Func has AttrStatic.
   *
   * For this fast path, we just check if the key is bitwise equal to
   * the Class* on the object.  If either of the special bits are set
   * in the key we'll bail to the slow path.
   */
  if (LIKELY(pair->m_key == reinterpret_cast<uintptr_t>(cls))) {
    ar->m_func = pair->m_value;
  } else {
    auto* name = static_cast<const StringData*>(extraKey);
    methodCacheSlowPath(pair, ar, const_cast<StringData*>(name), cls);
  }
}

//=============================================================================
// GlobalCache
//  | - BoxedGlobalCache

template<bool isBoxed>
inline TypedValue*
GlobalCache::lookupImpl(StringData *name, bool allowCreate) {
  bool hit ATTRIBUTE_UNUSED;

  TypedValue* retval;
  if (!m_tv) {
    hit = false;

    VarEnv* ve = g_vmContext->m_globalVarEnv;
    assert(ve->isGlobalScope());
    if (allowCreate) {
      m_tv = ve->lookupAddRawPointer(name);
    } else {
      m_tv = ve->lookupRawPointer(name);
      if (!m_tv) {
        retval = 0;
        goto miss;
      }
    }
  } else {
    hit = true;
  }

  retval = tvDerefIndirect(m_tv);
  if (retval->m_type == KindOfUninit) {
    if (!allowCreate) {
      retval = 0;
      goto miss;
    } else {
      tvWriteNull(retval);
    }
  }
  if (isBoxed && retval->m_type != KindOfRef) {
    tvBox(retval);
  }
  if (!isBoxed && retval->m_type == KindOfRef) {
    retval = retval->m_data.pref->tv();
  }
  assert(!isBoxed || retval->m_type == KindOfRef);
  assert(!allowCreate || retval);

miss:
  // decRef the name if we consumed it.  If we didn't get a global, we
  // need to leave the name for the caller to use before decrefing (to
  // emit warnings).
  if (retval) decRefStr(name);
  TRACE(5, "%sGlobalCache::lookup(\"%s\") tv@%p %p -> (%s) %p t%d\n",
        isBoxed ? "Boxed" : "",
        name->data(),
        m_tv,
        retval,
        hit ? "hit" : "miss",
        retval ? retval->m_data.pref : 0,
        retval ? retval->m_type : 0);
  return retval;
}

TypedValue*
GlobalCache::lookup(Handle handle, StringData* name) {
  GlobalCache* thiz = (GlobalCache*)GlobalCache::cacheAtHandle(handle);
  TypedValue* retval = thiz->lookupImpl<false>(name, false /* allowCreate */);
  assert(!retval || retval->m_type != KindOfRef);
  return retval;
}

TypedValue*
GlobalCache::lookupCreate(Handle handle, StringData* name) {
  GlobalCache* thiz = (GlobalCache*)GlobalCache::cacheAtHandle(handle);
  TypedValue* retval = thiz->lookupImpl<false>(name, true /* allowCreate */);
  assert(retval->m_type != KindOfRef);
  return retval;
}

TypedValue*
GlobalCache::lookupCreateAddr(void* cacheAddr, StringData* name) {
  GlobalCache* thiz = (GlobalCache*)cacheAddr;
  TypedValue* retval = thiz->lookupImpl<false>(name, true /* allowCreate */);
  assert(retval->m_type != KindOfRef);
  return retval;
}

TypedValue*
BoxedGlobalCache::lookup(Handle handle, StringData* name) {
  BoxedGlobalCache* thiz = (BoxedGlobalCache*)
    BoxedGlobalCache::cacheAtHandle(handle);
  TypedValue* retval = thiz->lookupImpl<true>(name, false /* allowCreate */);
  assert(!retval || retval->m_type == KindOfRef);
  return retval;
}

TypedValue*
BoxedGlobalCache::lookupCreate(Handle handle, StringData* name) {
  BoxedGlobalCache* thiz = (BoxedGlobalCache*)
    BoxedGlobalCache::cacheAtHandle(handle);
  TypedValue* retval = thiz->lookupImpl<true>(name, true /* allowCreate */);
  assert(retval->m_type == KindOfRef);
  return retval;
}

static CacheHandle allocFuncOrClass(const unsigned* handlep, bool persistent) {
  if (UNLIKELY(!*handlep)) {
    Lock l(s_handleMutex);
    if (!*handlep) {
      *const_cast<unsigned*>(handlep) =
        allocLocked(persistent, sizeof(void*), sizeof(void*));
    }
  }
  return *handlep;
}

CacheHandle allocKnownClass(const Class* cls) {
  const NamedEntity* ne = cls->preClass()->namedEntity();
  if (ne->m_cachedClassOffset) return ne->m_cachedClassOffset;

  return allocKnownClass(ne,
                         RuntimeOption::RepoAuthoritative &&
                         cls->verifyPersistent());
}

CacheHandle allocKnownClass(const NamedEntity* ne,
                            bool persistent) {
  return allocFuncOrClass(&ne->m_cachedClassOffset, persistent);
}

CacheHandle allocKnownClass(const StringData* name) {
  return allocKnownClass(Unit::GetNamedEntity(name), false);
}

CacheHandle allocClassInitProp(const StringData* name) {
  return namedAlloc<NSClsInitProp>(name, sizeof(Class::PropInitVec*),
                                   sizeof(Class::PropInitVec*));
}

CacheHandle allocClassInitSProp(const StringData* name) {
  return namedAlloc<NSClsInitSProp>(name, sizeof(TypedValue*),
                                    sizeof(TypedValue*));
}

CacheHandle allocFixedFunction(const NamedEntity* ne, bool persistent) {
  return allocFuncOrClass(&ne->m_cachedFuncOffset, persistent);
}

CacheHandle allocFixedFunction(const StringData* name) {
  return allocFixedFunction(Unit::GetNamedEntity(name), false);
}

CacheHandle allocNameDef(const NamedEntity* ne) {
  if (ne->m_cachedNameDefOffset) {
    return ne->m_cachedNameDefOffset;
  }
  return allocFuncOrClass(&ne->m_cachedNameDefOffset,
    false /* persistent */); // TODO(#2103214): support persistent
}

template<bool checkOnly>
Class*
lookupKnownClass(Class** cache, const StringData* clsName, bool isClass) {
  if (!checkOnly) {
    Stats::inc(Stats::TgtCache_KnownClsHit, -1);
    Stats::inc(Stats::TgtCache_KnownClsMiss, 1);
  }

  Class* cls = *cache;
  assert(!cls); // the caller should already have checked
  AutoloadHandler::s_instance->invokeHandler(
    StrNR(const_cast<StringData*>(clsName)));
  cls = *cache;

  if (checkOnly) {
    // If the class still doesn't exist, return flags causing the
    // attribute check in the translated code that called us to fail.
    return (Class*)(uintptr_t)(cls ? cls->attrs() :
      (isClass ? (AttrTrait | AttrInterface) : AttrNone));
  } else if (UNLIKELY(!cls)) {
    undefinedError(Strings::UNKNOWN_CLASS, clsName->data());
  }
  return cls;
}

template Class* lookupKnownClass<true>(Class**, const StringData*, bool);
template Class* lookupKnownClass<false>(Class**, const StringData*, bool);

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
  ClassCache* thiz = cacheAtHandle(handle);
  Pair *pair = thiz->keyToPair(name);
  const StringData* pairSd = pair->m_key;
  if (!stringMatches(pairSd, name)) {
    TRACE(1, "ClassCache miss: %s\n", name->data());
    const NamedEntity *ne = Unit::GetNamedEntity(name);
    Class *c = Unit::lookupClass(ne);
    if (UNLIKELY(!c)) {
      c = Unit::loadMissingClass(ne, name);
      if (UNLIKELY(!c)) {
        undefinedError(Strings::UNKNOWN_CLASS, name->data());
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

/*
 * Constants are raw TypedValues read from TLS storage by emitted code.
 * We must represent the undefined value as KindOfUninit == 0. Constant
 * definition is hooked in the runtime to allocate and update these
 * structures.
 */
CacheHandle allocConstant(uint32_t* handlep, bool persistent) {
  if (UNLIKELY(!*handlep)) {
    Lock l(s_handleMutex);
    if (!*handlep) {
      *handlep =
        allocLocked(persistent, sizeof(TypedValue), sizeof(TypedValue));
    }
  }
  return *handlep;
}


CacheHandle allocStatic() {
  return namedAlloc<NSInvalid>(nullptr, sizeof(TypedValue*),
                               sizeof(TypedValue*));
}

CacheHandle allocClassConstant(StringData* name) {
  return namedAlloc<NSClassConstant>(name,
                                     sizeof(TypedValue), sizeof(TypedValue));
}

TypedValue*
lookupClassConstant(TypedValue* cache,
                    const NamedEntity* ne,
                    const StringData* cls,
                    const StringData* cns) {
  Stats::inc(Stats::TgtCache_ClsCnsHit, -1);
  Stats::inc(Stats::TgtCache_ClsCnsMiss, 1);

  TypedValue* clsCns;
  clsCns = g_vmContext->lookupClsCns(ne, cls, cns);
  *cache = *clsCns;

  return cache;
}

TypedValue
lookupClassConstantTv(TypedValue* cache,
                      const NamedEntity* ne,
                      const StringData* cls,
                      const StringData* cns) {
  return *lookupClassConstant(cache, ne, cls, cns);
}

//=============================================================================
// *SPropCache
//

TypedValue*
SPropCache::lookup(Handle handle, const Class *cls, const StringData *name) {
  // The fast path is in-TC. If we get here, we have already missed.
  SPropCache* thiz = cacheAtHandle(handle);
  Stats::inc(Stats::TgtCache_SPropMiss);
  Stats::inc(Stats::TgtCache_SPropHit, -1);
  assert(cls && name);
  assert(!thiz->m_tv);
  TRACE(3, "SPropCache miss: %s::$%s\n", cls->name()->data(),
        name->data());
  // This is valid only if the lookup comes from an in-class method
  Class *ctx = const_cast<Class*>(cls);
  if (debug) {
    VMRegAnchor _;
    assert(ctx == arGetContextClass((ActRec*)vmfp()));
  }
  bool visible, accessible;
  TypedValue* val;
  val = cls->getSProp(ctx, name, visible, accessible);
  if (UNLIKELY(!visible)) {
    string methodName;
    string_printf(methodName, "%s::$%s",
                  cls->name()->data(), name->data());
    undefinedError("Invalid static property access: %s", methodName.c_str());
  }
  // We only cache in class references, thus we can always cache them
  // once the property is known to exist
  assert(accessible);
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

template<bool raiseOnError>
TypedValue*
SPropCache::lookupSProp(const Class *cls, const StringData *name, Class* ctx) {
  bool visible, accessible;
  TypedValue* val;
  val = cls->getSProp(ctx, name, visible, accessible);
  if (UNLIKELY(!visible || !accessible)) {
    if (!raiseOnError) return NULL;
    string propertyName;
    string_printf(propertyName, "%s::%s",
                  cls->name()->data(), name->data());
    undefinedError("Invalid static property access: %s", propertyName.c_str());
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
SPropCache::lookupIR(Handle handle, const Class *cls, const StringData *name,
                     Class* ctx) {
  // The fast path is in-TC. If we get here, we have already missed.
  SPropCache* thiz = cacheAtHandle(handle);
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

template TypedValue* SPropCache::lookupIR<true>(Handle handle,
                                                const Class *cls,
                                                const StringData *name,
                                                Class* ctx);

template TypedValue* SPropCache::lookupIR<false>(Handle handle,
                                                 const Class *cls,
                                                 const StringData *name,
                                                 Class* ctx);

//=============================================================================
// StaticMethodCache
//

template<typename T, PHPNameSpace ns>
static inline CacheHandle
allocStaticMethodCache(const StringData* clsName,
                       const StringData* methName,
                       const char* ctxName) {
  // Implementation detail of FPushClsMethodD/F: we use "C::M:ctx" as
  // the key for invoking static method "M" on class "C". This
  // composes such a key. "::" is semi-arbitrary, though whatever we
  // choose must delimit possible class and method names, so we might
  // as well ape the source syntax
  const StringData* joinedName =
    StringData::GetStaticString(String(clsName->data()) + String("::") +
                                String(methName->data()) + String(":") +
                                String(ctxName));

  return namedAlloc<ns>(joinedName, sizeof(T), sizeof(T));
}

CacheHandle
StaticMethodCache::alloc(const StringData* clsName,
                         const StringData* methName,
                         const char* ctxName) {
  return allocStaticMethodCache<StaticMethodCache, NSStaticMethod>(
    clsName, methName, ctxName);
}

CacheHandle
StaticMethodFCache::alloc(const StringData* clsName,
                          const StringData* methName,
                          const char* ctxName) {
  return allocStaticMethodCache<StaticMethodFCache, NSStaticMethodF>(
    clsName, methName, ctxName);
}

const Func*
StaticMethodCache::lookupIR(Handle handle, const NamedEntity *ne,
                            const StringData* clsName,
                            const StringData* methName) {
  StaticMethodCache* thiz = static_cast<StaticMethodCache*>
    (handleToPtr(handle));
  Stats::inc(Stats::TgtCache_StaticMethodMiss);
  Stats::inc(Stats::TgtCache_StaticMethodHit, -1);
  TRACE(1, "miss %s :: %s caller %p\n",
        clsName->data(), methName->data(), __builtin_return_address(0));
  VMRegAnchor _; // needed for lookupClsMethod.

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
                                         false /*raise*/);
  if (LIKELY(res == MethodFoundNoThis &&
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
  assert(res != MethodFoundWithThis); // Not possible: no this supplied.
  // We've already sync'ed regs; this is some hard case, we might as well
  // just let the interpreter handle this entirely.
  assert(*vmpc() == OpFPushClsMethodD);

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
  VMRegAnchor _; // needed for lookupClsMethod.

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
                                         false /*raise*/);
  if (LIKELY(res == MethodFoundNoThis &&
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
  assert(res != MethodFoundWithThis); // Not possible: no this supplied.
  // We've already sync'ed regs; this is some hard case, we might as well
  // just let the interpreter handle this entirely.
  assert(*vmpc() == OpFPushClsMethodD);
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
                             const StringData* methName) {
  assert(cls);
  StaticMethodFCache* thiz = static_cast<StaticMethodFCache*>
    (handleToPtr(handle));
  Stats::inc(Stats::TgtCache_StaticMethodFMiss);
  Stats::inc(Stats::TgtCache_StaticMethodFHit, -1);
  VMRegAnchor _; // needed for lookupClsMethod.

  const Func* f;
  VMExecutionContext* ec = g_vmContext;
  LookupResult res = ec->lookupClsMethod(f, cls, methName,
                                         nullptr,
                                         false /*raise*/);
  assert(res != MethodFoundWithThis); // Not possible: no this supplied.
  if (LIKELY(res == MethodFoundNoThis && !f->isAbstract())) {
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

const Func*
StaticMethodFCache::lookup(Handle handle, const Class* cls,
                           const StringData* methName) {
  const Func* f = lookupIR(handle, cls, methName);
  if (f) return f;

  VMRegAnchor _; // needed for opFPushClsMethodF

  // We've already sync'ed regs; this is some hard case, we might as well
  // just let the interpreter handle this entirely.
  assert(*vmpc() == OpFPushClsMethodF);
  Stats::inc(Stats::Instr_TC, -1);
  Stats::inc(Stats::Instr_InterpOneFPushClsMethodF);
  g_vmContext->opFPushClsMethodF();

  // We already did all the work so tell our caller to do nothing.
  TRACE(1, "miss staticfcache %s :: %s -> intractable null\n",
        cls->name()->data(), methName->data());
  return nullptr;
}

} } } // HPHP::Transl::TargetCache
