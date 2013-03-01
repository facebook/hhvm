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

#include "folly/ScopeGuard.h"

#include <iostream>
#include <iomanip>
#include <tbb/concurrent_unordered_map.h>
#include <boost/algorithm/string.hpp>

#include <util/lock.h>
#include <util/util.h>
#include <util/atomic.h>
#include <runtime/ext/ext_variable.h>
#include <runtime/vm/bytecode.h>
#include <runtime/vm/repo.h>
#include <runtime/vm/blob_helper.h>
#include <runtime/vm/translator/targetcache.h>
#include <runtime/vm/translator/translator-deps.h>
#include <runtime/vm/translator/translator-inline.h>
#include <runtime/vm/translator/translator-x64.h>
#include <runtime/vm/verifier/check.h>
#include <runtime/base/strings.h>
#include <runtime/vm/func_inline.h>
#include <runtime/eval/runtime/file_repository.h>
#include <runtime/vm/stats.h>
#include <runtime/vm/treadmill.h>

namespace HPHP {
namespace VM {
///////////////////////////////////////////////////////////////////////////////

using Util::getDataRef;

static const Trace::Module TRACEMOD = Trace::hhbc;

Mutex Unit::s_classesMutex;
/*
 * We hold onto references to elements of this map. If we use a different
 * map, we must use one that doesnt invalidate references to its elements
 * (unless they are deleted, which never happens here). Any standard
 * associative container will meet this requirement.
 */
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
  return nullptr;
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

Array Unit::getUserFunctions() {
  // Return an array of all defined functions.  This method is used
  // to support get_defined_functions().
  Array a = Array::Create();
  if (s_namedDataMap) {
    for (NamedEntityMap::const_iterator it = s_namedDataMap->begin();
         it != s_namedDataMap->end(); ++it) {
      Func* func_ = it->second.getCachedFunc();
      if (!func_ || func_->isBuiltin() ||
          isdigit(func_->name()->data()[0])) {
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
  , m_current(m_next != m_end ? *m_next->second.clsList() : nullptr) {
  if (!empty()) skip();
}

void AllClasses::skip() {
  if (!m_current) {
    assert(!empty());
    ++m_next;
    while (!empty()) {
      m_current = *m_next->second.clsList();
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
      cls = *m_next->second.clsList();
      if (cls && cls->getCached()) break;
      ++m_next;
    }
  }

public:
AllCachedClasses()
  : m_next(s_namedDataMap->begin())
  , m_end(s_namedDataMap->end()) {
    skip();
  }
  bool empty() const {
    return m_next == m_end;
  }
  Class* front() {
    assert(!empty());
    Class* c = *m_next->second.clsList();
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

Array Unit::getClassesInfo() {
  // Return an array of all defined class names.  This method is used to
  // support get_declared_classes().
  Array a = Array::Create();
  if (s_namedDataMap) {
    for (AllCachedClasses ac; !ac.empty();) {
      Class* c = ac.popFront();
      if (!(c->attrs() & (AttrInterface|AttrTrait))) {
        a.append(c->nameRef());
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
    for (AllCachedClasses ac; !ac.empty();) {
      Class* c = ac.popFront();
      if (c->attrs() & AttrInterface) {
        a.append(c->nameRef());
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
    for (AllCachedClasses ac; !ac.empty(); ) {
      Class* c = ac.popFront();
      if (c->attrs() & AttrTrait) {
        array.append(c->nameRef());
      }
    }
  }
  return array;
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
  uint8* end = (uint8*)index + index[*index + cur + 2];
  assert(ptr <= end);
  if (ptr == end) return false;
  info.m_kind = (Unit::MetaInfo::Kind)*ptr++;
  info.m_arg = *ptr++;
  info.m_data = decodeVariableSizeImm(&ptr);
  return true;
}

//=============================================================================
// Unit.

Unit::Unit()
    : m_sn(-1), m_bc(nullptr), m_bclen(0),
      m_bc_meta(nullptr), m_bc_meta_len(0), m_filepath(nullptr),
      m_dirpath(nullptr), m_md5(),
      m_mergeInfo(nullptr),
      m_cacheOffset(0),
      m_repoId(-1),
      m_mergeState(UnitMergeStateUnmerged),
      m_cacheMask(0),
      m_pseudoMainCache(nullptr) {
  tvWriteUninit(&m_mainReturn);
  m_mainReturn._count = 0; // flag for whether or not the unit is mergeable
}

Unit::~Unit() {
  if (debug) {
    // poison released bytecode
    memset(m_bc, 0xff, m_bclen);
  }
  free(m_bc);
  free(m_bc_meta);

  if (m_mergeInfo) {
    // Delete all Func's.
    range_foreach(mutableFuncs(), Func::destroy);
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

  if (!RuntimeOption::RepoAuthoritative &&
      (m_mergeState & UnitMergeStateMerged)) {
    Transl::unmergePreConsts(m_preConsts, this);
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
  return Util::low_malloc(sz);
}

void Unit::operator delete(void* p, size_t sz) {
  Util::low_free(p);
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

class FrameRestore {
 public:
  FrameRestore(const PreClass* preClass) {
    VMExecutionContext* ec = g_vmContext;
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
      tmp.m_soff = !fp ? 0
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
    VMExecutionContext* ec = g_vmContext;
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
  // TODO(#2054448): ARMv8
  Class* const* clsList = preClass->namedEntity()->clsList();
  Class* top = *clsList;
  if (top) {
    Class *cls = top->getCached();
    if (cls) {
      // Raise a fatal unless the existing class definition is identical to the
      // one this invocation would create.
      if (cls->preClass() != preClass) {
        if (failIsFatal) {
          FrameRestore fr(preClass);
          raise_error("Class already declared: %s", preClass->name()->data());
        }
        return nullptr;
      }
      return cls;
    }
  }
  // Get a compatible Class, and add it to the list of defined classes.

  Class* parent = nullptr;
  for (;;) {
    // Search for a compatible extant class.  Searching from most to least
    // recently created may have better locality than alternative search orders.
    // In addition, its the only simple way to make this work lock free...
    for (Class* class_ = top; class_ != nullptr; class_ = class_->m_nextClass) {
      if (class_->preClass() != preClass) continue;

      Class::Avail avail = class_->avail(parent, failIsFatal /*tryAutoload*/);
      if (LIKELY(avail == Class::AvailTrue)) {
        class_->setCached();
        DEBUGGER_ATTACHED_ONLY(phpDefClassHook(class_));
        return class_;
      }
      if (avail == Class::AvailFail) {
        if (failIsFatal) {
          FrameRestore fr(preClass);
          raise_error("unknown class %s", parent->name()->data());
        }
        return nullptr;
      }
      assert(avail == Class::AvailFalse);
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
        Transl::TargetCache::allocKnownClass(newClass.get());
    }
    newClass->m_nextClass = top;

    if (Class::s_instanceBitsInit.load(std::memory_order_acquire)) {
      // If the instance bitmap has already been set up, we can just initialize
      // our new class's bits and add ourselves to the class list normally.
      newClass->setInstanceBits();
      atomic_release_store(const_cast<Class**>(clsList), newClass.get());
    } else {
      // Otherwise, we have to grab the read lock. If the map has been
      // initialized since we checked, initialize the bits normally. If not, we
      // must add the new class to the class list before dropping the lock to
      // ensure its bits are initialized when the time comes.
      ReadLock l(Class::s_instanceBitsLock);
      if (Class::s_instanceBitsInit.load(std::memory_order_acquire)) {
        newClass->setInstanceBits();
      }
      atomic_release_store(const_cast<Class**>(clsList), newClass.get());
    }
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
  Class *cls = *ne->clsList();
  if (LIKELY(cls != nullptr)) {
    cls = cls->getCached();
    if (LIKELY(cls != nullptr)) return cls;
  }
  VMRegAnchor _;
  AutoloadHandler::s_instance->invokeHandler(
    StrNR(const_cast<StringData*>(name)));
  return Unit::lookupClass(ne);
}

Class* Unit::loadMissingClass(const NamedEntity* ne,
                              const StringData *name) {
  AutoloadHandler::s_instance->invokeHandler(
    StrNR(const_cast<StringData*>(name)));
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

void Unit::loadFunc(const Func *func) {
  assert(!func->isMethod());
  const NamedEntity *ne = func->getNamedEntity();
  if (UNLIKELY(!ne->m_cachedFuncOffset)) {
    Transl::TargetCache::allocFixedFunction(ne,
                                            func->attrs() & AttrPersistent &&
                                            RuntimeOption::RepoAuthoritative);
  }
  const_cast<Func*>(func)->m_cachedOffset = ne->m_cachedFuncOffset;
}

static SimpleMutex unitInitLock(false /* reentrant */, RankUnitInit);

void Unit::initialMerge() {
  unitInitLock.assertOwnedBySelf();
  if (LIKELY(m_mergeState == UnitMergeStateUnmerged)) {
    int state = 0;
    m_mergeState = UnitMergeStateMerging;
    bool allFuncsUnique = RuntimeOption::RepoAuthoritative;
    for (MutableFuncRange fr(nonMainFuncs()); !fr.empty();) {
      Func* f = fr.popFront();
      if (allFuncsUnique) {
        allFuncsUnique = (f->attrs() & AttrUnique);
      }
      loadFunc(f);
    }
    if (allFuncsUnique) state |= UnitMergeStateUniqueFuncs;
    if (!RuntimeOption::RepoAuthoritative) {
      Transl::mergePreConsts(m_preConsts);
    } else {
      /*
       * The mergeables array begins with the hoistable Func*s,
       * followed by the (potenitally) hoistable Class*s.
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
      bool allClassesUnique = true;
      int ix = m_mergeInfo->m_firstHoistablePreClass;
      int end = m_mergeInfo->m_firstMergeablePreClass;
      while (ix < end) {
        PreClass* pre = (PreClass*)m_mergeInfo->mergeableObj(ix++);
        if (allClassesUnique) {
          allClassesUnique = pre->attrs() & AttrUnique;
        }
      }
      if (isMergeOnly()) {
        ix = m_mergeInfo->m_firstMergeablePreClass;
        end = m_mergeInfo->m_mergeablesSize;
        while (ix < end) {
          void *obj = m_mergeInfo->mergeableObj(ix);
          InclOpFlags flags = InclOpDefault;
          UnitMergeKind k = UnitMergeKind(uintptr_t(obj) & 7);
          switch (k) {
            case UnitMergeKindUniqueDefinedClass:
            case UnitMergeKindDone:
              not_reached();
            case UnitMergeKindClass:
              if (allClassesUnique) {
                allClassesUnique = ((PreClass*)obj)->attrs() & AttrUnique;
              }
              break;
            case UnitMergeKindReqMod:
              flags = InclOpDocRoot | InclOpLocal;
              goto inc;
            case UnitMergeKindReqSrc:
              flags = InclOpRelative | InclOpLocal;
              goto inc;
            case UnitMergeKindReqDoc:
              flags = InclOpDocRoot;
              goto inc;
            inc: {
                StringData* s = (StringData*)((char*)obj - (int)k);
                HPHP::Eval::PhpFile* efile =
                  g_vmContext->lookupIncludeRoot(s, flags, nullptr, this);
                assert(efile);
                Unit* unit = efile->unit();
                unit->initialMerge();
                m_mergeInfo->mergeableObj(ix) = (void*)((char*)unit + (int)k);
              }
              break;
            case UnitMergeKindDefine: {
              StringData* s = (StringData*)((char*)obj - (int)k);
              TypedValue* v = (TypedValue*)m_mergeInfo->mergeableData(ix + 1);
              ix += sizeof(TypedValue) / sizeof(void*);
              v->_count = TargetCache::allocConstant(s);
              break;
            }
            case UnitMergeKindGlobal: {
              StringData* s = (StringData*)((char*)obj - (int)k);
              TypedValue* v = (TypedValue*)m_mergeInfo->mergeableData(ix + 1);
              ix += sizeof(TypedValue) / sizeof(void*);
              v->_count = TargetCache::GlobalCache::alloc(s);
              break;
            }
          }
          ix++;
        }
      }
      if (allClassesUnique) state |= UnitMergeStateUniqueClasses;
    }
    m_mergeState = UnitMergeStateMerged | state;
  }
}

static void mergeCns(TypedValue& tv, TypedValue *value,
                     StringData *name) {
  if (LIKELY(tv.m_type == KindOfUninit &&
             g_vmContext->insertCns(name, value))) {
    tvDup(value, &tv);
    return;
  }

  raise_warning(Strings::CONSTANT_ALREADY_DEFINED, name->data());
}

static void setGlobal(void* cacheAddr, TypedValue *value,
                      StringData *name) {
  tvSet(value, TargetCache::GlobalCache::lookupCreateAddr(cacheAddr, name));
}

void Unit::merge() {
  if (UNLIKELY(!(m_mergeState & UnitMergeStateMerged))) {
    SimpleLock lock(unitInitLock);
    initialMerge();
  }

  if (UNLIKELY(isDebuggerAttached())) {
    mergeImpl<true>(TargetCache::handleToPtr(0), m_mergeInfo);
  } else {
    mergeImpl<false>(TargetCache::handleToPtr(0), m_mergeInfo);
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
    if (TargetCache::isPersistentHandle(func->getCachedOffset())) {
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
    Class* cls = *pre->namedEntity()->clsList();
    assert(cls && !cls->m_nextClass);
    assert(cls->preClass() == pre);
    if (TargetCache::isPersistentHandle(cls->m_cachedOffset)) {
      delta++;
    } else if (out) {
      out->mergeableObj(oix++) = (void*)(uintptr_t(cls) | 1);
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
        Class* cls = *pre->namedEntity()->clsList();
        assert(cls && !cls->m_nextClass);
        assert(cls->preClass() == pre);
        if (TargetCache::isPersistentHandle(cls->m_cachedOffset)) {
          delta++;
        } else if (out) {
          out->mergeableObj(oix++) =
            (void*)(uintptr_t(cls) | UnitMergeKindUniqueDefinedClass);
        }
        break;
      }
      case UnitMergeKindUniqueDefinedClass:
        not_reached();

      case UnitMergeKindDefine:
      case UnitMergeKindGlobal:
        if (out) {
          out->mergeableObj(oix++) = obj;
          *(TypedValue*)out->mergeableData(oix) =
            *(TypedValue*)in->mergeableData(ix);
          oix += sizeof(TypedValue) / sizeof(void*);
        }
        ix += sizeof(TypedValue) / sizeof(void*);
        break;

      case UnitMergeKindReqMod:
      case UnitMergeKindReqSrc:
      case UnitMergeKindReqDoc: {
        Unit *unit = (Unit*)((char*)obj - (int)k);
        void *rep = unit->replaceUnit();
        if (!rep) {
          delta++;
        } else if (out) {
          if (rep == unit) {
            out->mergeableObj(oix++) = obj;
          } else {
            UnitMergeKind k1 = UnitMergeKind(uintptr_t(rep) & 7);
            switch (k1) {
              case UnitMergeKindReqMod:
              case UnitMergeKindReqSrc:
                break;
              case UnitMergeKindReqDoc:
                if (k != UnitMergeKindReqDoc) {
                  rep = obj;
                }
                break;
              default:
                break;
            }
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
        getDataRef<Func*>(tcbase, func->getCachedOffset()) = func;
        if (debugger) phpDefFuncHook(func);
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
            TargetCache::isPersistentHandle(cls->m_cachedOffset)) {
          Stats::inc(Stats::UnitMerge_hoistable_persistent_cache);
        }
        if (Class* parent = cls->parent()) {
          if (parent->isPersistent()) {
            Stats::inc(Stats::UnitMerge_hoistable_persistent_parent);
          }
          if (Stats::enabled() &&
              TargetCache::isPersistentHandle(parent->m_cachedOffset)) {
            Stats::inc(Stats::UnitMerge_hoistable_persistent_parent_cache);
          }
          if (UNLIKELY(!getDataRef<Class*>(tcbase, parent->m_cachedOffset))) {
            redoHoistable = true;
            continue;
          }
        }
        getDataRef<Class*>(tcbase, cls->m_cachedOffset) = cls;
        if (debugger) phpDefClassHook(cls);
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
              TargetCache::isPersistentHandle(cls->m_cachedOffset)) {
            Stats::inc(Stats::UnitMerge_mergeable_unique_persistent_cache);
          }
          Class::Avail avail = cls->avail(other, true);
          if (UNLIKELY(avail == Class::AvailFail)) {
            raise_error("unknown class %s", other->name()->data());
          }
          assert(avail == Class::AvailTrue);
          getDataRef<Class*>(tcbase, cls->m_cachedOffset) = cls;
          if (debugger) phpDefClassHook(cls);
          obj = mi->mergeableObj(++ix);
          k = UnitMergeKind(uintptr_t(obj) & 7);
        } while (k == UnitMergeKindUniqueDefinedClass);
        continue;

      case UnitMergeKindDefine:
        do {
          Stats::inc(Stats::UnitMerge_mergeable);
          Stats::inc(Stats::UnitMerge_mergeable_define);
          StringData* name = (StringData*)((char*)obj - (int)k);
          TypedValue *v = (TypedValue*)mi->mergeableData(ix + 1);
          mergeCns(getDataRef<TypedValue>(tcbase, v->_count), v, name);
          ix += 1 + sizeof(TypedValue) / sizeof(void*);
          obj = mi->mergeableObj(ix);
          k = UnitMergeKind(uintptr_t(obj) & 7);
        } while (k == UnitMergeKindDefine);
        continue;

      case UnitMergeKindGlobal:
        do {
          Stats::inc(Stats::UnitMerge_mergeable);
          Stats::inc(Stats::UnitMerge_mergeable_global);
          StringData* name = (StringData*)((char*)obj - (int)k);
          TypedValue *v = (TypedValue*)mi->mergeableData(ix + 1);
          setGlobal(&getDataRef<char>(tcbase, v->_count), v, name);
          ix += 1 + sizeof(TypedValue) / sizeof(void*);
          obj = mi->mergeableObj(ix);
          k = UnitMergeKind(uintptr_t(obj) & 7);
        } while (k == UnitMergeKindGlobal);
        continue;

      case UnitMergeKindReqMod:
      case UnitMergeKindReqSrc:
      case UnitMergeKindReqDoc:
        do {
          Stats::inc(Stats::UnitMerge_mergeable);
          Stats::inc(Stats::UnitMerge_mergeable_require);
          Unit *unit = (Unit*)((char*)obj - (int)k);
          uchar& unitLoadedFlags =
            getDataRef<uchar>(tcbase, unit->m_cacheOffset);
          if (!(unitLoadedFlags & unit->m_cacheMask)) {
            unitLoadedFlags |= unit->m_cacheMask;
            unit->mergeImpl<debugger>(tcbase, unit->m_mergeInfo);
            if (UNLIKELY(!unit->isMergeOnly())) {
              Stats::inc(Stats::PseudoMain_Reentered);
              TypedValue ret;
              VarEnv* ve = nullptr;
              if (k == UnitMergeKindReqDoc) {
                ActRec* fp = g_vmContext->m_fp;
                if (!fp) {
                  ve = g_vmContext->m_globalVarEnv;
                } else {
                  if (fp->hasVarEnv()) {
                    ve = fp->m_varEnv;
                  } else {
                    // Nothing to do. If there is no varEnv, the enclosing
                    // file was called by fb_autoload_map, ReqSrc or ReqMod
                    // All those cases wanted a local scope.
                  }
                }
              }
              g_vmContext->invokeFunc(&ret, unit->getMain(), Array(),
                                      nullptr, nullptr, ve, nullptr, nullptr);
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
        if (UNLIKELY((m_mergeState & (UnitMergeStateUniqueClasses|
                                      UnitMergeStateUniqueDefinedClasses)) ==
                     UnitMergeStateUniqueClasses)) {
          SimpleLock lock(unitInitLock);
          if (m_mergeState & UnitMergeStateUniqueDefinedClasses) return;
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
          m_mergeState |= UnitMergeStateUniqueDefinedClasses;
          assert(newMi->m_firstMergeablePreClass == newMi->m_mergeablesSize ||
                 isMergeOnly());
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
  Func* f = (*m_mergeInfo->funcBegin())->clone();
  f->setNewFuncId();
  f->setCls(cls);
  f->setBaseCls(cls);
  (*m_pseudoMainCache)[cls] = f;
  return f;
}

int Unit::getLineNumber(Offset pc) const {
  LineEntry key = LineEntry(pc, -1);
  std::vector<LineEntry>::const_iterator it =
    upper_bound(m_lineTable.begin(), m_lineTable.end(), key);
  if (it != m_lineTable.end()) {
    assert(pc < it->pastOffset());
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
  assert(offsets.size() == 0);
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
  FuncEntry key = FuncEntry(pc, nullptr);
  FuncTable::const_iterator it =
    upper_bound(m_funcTable.begin(), m_funcTable.end(), key);
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

  std::map<Offset,const Func*>::const_iterator funcIt =
    funcMap.lower_bound(startOffset);

  const uchar* it = &m_bc[startOffset];
  int prevLineNum = -1;
  MetaHandle metaHand;
  while (it < &m_bc[stopOffset]) {
    assert(funcIt == funcMap.end() || funcIt->first >= offsetOf(it));
    if (funcIt != funcMap.end() && funcIt->first == offsetOf(it)) {
      out.put('\n');
      funcIt->second->prettyPrint(out);
      ++funcIt;
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
    out << instrToString((Opcode*)it, (Unit*)this);
    if (metaHand.findMeta(this, offsetOf(it))) {
      out << " #";
      Unit::MetaInfo info;
      while (metaHand.nextArg(info)) {
        int arg = info.m_arg & ~MetaInfo::VectorArg;
        const char *argKind = info.m_arg & MetaInfo::VectorArg ? "M" : "";
        switch (info.m_kind) {
          case Unit::MetaInfo::DataTypeInferred:
          case Unit::MetaInfo::DataTypePredicted:
            out << " i" << argKind << arg << ":t=" << (int)info.m_data;
            if (info.m_kind == Unit::MetaInfo::DataTypePredicted) {
              out << "*";
            }
            break;
          case Unit::MetaInfo::String: {
            const StringData* sd = lookupLitstrId(info.m_data);
            out << " i" << argKind << arg << ":s=" <<
              std::string(sd->data(), sd->size());
            break;
          }
          case Unit::MetaInfo::Class: {
            const StringData* sd = lookupLitstrId(info.m_data);
            out << " i" << argKind << arg << ":c=" << sd->data();
            break;
          }
          case Unit::MetaInfo::MVecPropClass: {
            const StringData* sd = lookupLitstrId(info.m_data);
            out << " i" << argKind << arg << ":pc=" << sd->data();
            break;
          }
          case Unit::MetaInfo::NopOut:
            out << " Nop";
            break;
          case Unit::MetaInfo::GuardedThis:
            out << " GuardedThis";
            break;
          case Unit::MetaInfo::GuardedCls:
            out << " GuardedCls";
            break;
          case Unit::MetaInfo::NoSurprise:
            out << " NoSurprise";
            break;
          case Unit::MetaInfo::ArrayCapacity:
            out << " capacity=" << info.m_data;
            break;
          case Unit::MetaInfo::NonRefCounted:
            out << " :nrc=" << info.m_data;
            break;
          case Unit::MetaInfo::None:
            assert(false);
            break;
        }
      }
    }
    out << std::endl;
    it += instrLen((Opcode*)it);
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

void Unit::enableIntercepts() {
  TranslatorX64* tx64 = TranslatorX64::Get();
  // Its ok to set maybeIntercepted(), because
  // we are protected by s_mutex in intercept.cpp
  for (MutableFuncRange fr(nonMainFuncs()); !fr.empty(); ) {
    Func *func = fr.popFront();
    if (func->isPseudoMain()) {
      // pseudomain's can't be intercepted
      continue;
    }
    tx64->interceptPrologues(func);
  }
  {
    Lock lock(s_classesMutex);
    for (int i = m_preClasses.size(); i--; ) {
      PreClass* pcls = m_preClasses[i].get();
      Class *cls = *pcls->namedEntity()->clsList();
      while (cls) {
        /*
         * verify that this class corresponds to the
         * preclass we're looking at. This avoids
         * redundantly iterating over the same class
         * multiple times, but also avoids a hard to
         * repro crash, if the unit owning cls is being
         * destroyed at the time we pick up cls from the
         * list (which is possible). Note that cls
         * itself will be destroyed by treadmill, so
         * it is safe to call preClass()
         */
        if (cls->preClass() == pcls) {
          size_t numFuncs = cls->numMethods();
          Func* const* funcs = cls->methods();
          for (unsigned i = 0; i < numFuncs; i++) {
            if (funcs[i]->cls() != cls) {
              /*
               * This func is defined by a base
               * class. We can skip it now, because
               * we'll hit it when we process
               * the base class. More importantly,
               * the base class's unit may have been
               * destroyed; in which case we have to
               * skip it here, or we'll likely crash.
               *
               * Note that Classes are ref counted,
               * so the the funcs[i]'s Class cant have
               * been freed yet, so the comparison is
               * safe; although we do seem to have a
               * class leak here (sandbox mode only)
               */
              continue;
            }
            tx64->interceptPrologues(funcs[i]);
          }
        }
        cls = cls->m_nextClass;
      }
    }
  }
}

Func* Unit::lookupFunc(const NamedEntity* ne, const StringData* name) {
  Func* func = ne->getCachedFunc();
  return func;
}

Func* Unit::lookupFunc(const StringData* funcName) {
  const NamedEntity* ne = GetNamedEntity(funcName);
  Func* func = ne->getCachedFunc();
  return func;
}

Func* Unit::loadFunc(const NamedEntity* ne, const StringData* funcName) {
  Func* func = ne->getCachedFunc();
  if (UNLIKELY(!func)) {
    if (AutoloadHandler::s_instance->autoloadFunc(StrNR(funcName))) {
      func = ne->getCachedFunc();
    }
  }
  return func;
}

Func* Unit::loadFunc(const StringData* funcName) {
  return loadFunc(GetNamedEntity(funcName), funcName);
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
                "lines BLOB, UNIQUE (md5));";
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
    ssCreate << "CREATE TABLE " << m_repo.table(repoId, "UnitPreConst")
             << "(unitSn INTEGER, name TEXT, value BLOB, preConstId INTEGER,"
                " PRIMARY KEY (unitSn, preConstId));";
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
    return nullptr;
  }
  try {
    getUnitLitstrs(repoId).get(ue);
    getUnitArrays(repoId).get(ue);
    getUnitPreConsts(repoId).get(ue);
    m_repo.pcrp().getPreClasses(repoId).get(ue);
    getUnitMergeables(repoId).get(ue);
    m_repo.frp().getFuncs(repoId).get(ue);
  } catch (RepoExc& re) {
    TRACE(0, "Repo error loading '%s' (0x%016llx%016llx) from '%s': %s\n",
             name.c_str(), md5.q[0], md5.q[1], m_repo.repoName(repoId).c_str(),
             re.msg().c_str());
    return nullptr;
  }
  TRACE(3, "Repo loaded '%s' (0x%016llx%016llx) from '%s'\n",
           name.c_str(), md5.q[0], md5.q[1], m_repo.repoName(repoId).c_str());
  return ue.create();
}

void UnitRepoProxy::InsertUnitStmt
                  ::insert(RepoTxn& txn, int64& unitSn, const MD5& md5,
                           const uchar* bc, size_t bclen,
                           const uchar* bc_meta, size_t bc_meta_len,
                           const TypedValue* mainReturn,
                           const LineTable& lines) {
  BlobEncoder linesBlob;

  if (!prepared()) {
    std::stringstream ssInsert;
    ssInsert << "INSERT INTO " << m_repo.table(m_repoId, "Unit")
             << " VALUES(NULL, @md5, @bc, @bc_meta,"
                " @mainReturn, @mergeable, @lines);";
    txn.prepare(*this, ssInsert.str());
  }
  RepoTxnQuery query(txn, *this);
  query.bindMd5("@md5", md5);
  query.bindBlob("@bc", (const void*)bc, bclen);
  query.bindBlob("@bc_meta",
                 bc_meta_len ? (const void*)bc_meta : (const void*)"",
                 bc_meta_len);
  query.bindTypedValue("@mainReturn", *mainReturn);
  query.bindBool("@mergeable", mainReturn->_count);
  query.bindBlob("@lines", linesBlob(lines), /* static */ true);
  query.exec();
  unitSn = query.getInsertedRowid();
}

bool UnitRepoProxy::GetUnitStmt
                  ::get(UnitEmitter& ue, const MD5& md5) {
  try {
    RepoTxn txn(m_repo);
    if (!prepared()) {
      std::stringstream ssSelect;
      ssSelect << "SELECT unitSn,bc,bc_meta,mainReturn,mergeable,lines FROM "
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
    TypedValue value;                        /**/ query.getTypedValue(3, value);
    bool mergeable;                          /**/ query.getBool(4, mergeable);
    BlobDecoder linesBlob =                  /**/ query.getBlob(5);
    ue.setRepoId(m_repoId);
    ue.setSn(unitSn);
    ue.setBc((const uchar*)bc, bclen);
    ue.setBcMeta((const uchar*)bc_meta, bc_meta_len);
    value._count = mergeable;
    ue.setMainReturn(&value);

    LineTable lines;
    linesBlob(lines);
    ue.setLines(lines);

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
      assert(id == litstrId);
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
      assert(id == arrayId);
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
      assert(id == addedId);
    }
  } while (!query.done());
  txn.commit();
}

void UnitRepoProxy::InsertUnitMergeableStmt
                  ::insert(RepoTxn& txn, int64 unitSn,
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
           kind == UnitMergeKindGlobal);
    query.bindTypedValue("@mergeableValue", *value);
  } else {
    assert(kind == UnitMergeKindReqMod ||
           kind == UnitMergeKindReqSrc ||
           kind == UnitMergeKindReqDoc);
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
      if (UNLIKELY(!RuntimeOption::RepoAuthoritative)) {
        /*
         * We're using a repo generated in WholeProgram mode,
         * but we're not using it in RepoAuthoritative mode
         * (this is dodgy to start with). We're not going to
         * deal with requires at merge time, so drop them
         * here, and clear the mergeOnly flag for the unit
         */
        ue.markNotMergeOnly();
        break;
      }
      int mergeableIx;           /**/ query.getInt(0, mergeableIx);
      int mergeableKind;         /**/ query.getInt(1, mergeableKind);
      Id mergeableId;            /**/ query.getInt(2, mergeableId);
      switch (mergeableKind) {
        case UnitMergeKindReqMod:
        case UnitMergeKindReqSrc:
        case UnitMergeKindReqDoc:
          ue.insertMergeableInclude(mergeableIx,
                                    (UnitMergeKind)mergeableKind, mergeableId);
          break;
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
    m_bclen(0), m_bc_meta(nullptr), m_bc_meta_len(0), m_filepath(nullptr),
    m_md5(md5), m_nextFuncSn(0),
    m_allClassesHoistable(true), m_returnSeen(false) {
  tvWriteUninit(&m_mainReturn);
  m_mainReturn._count = 0;
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
  assert(m_bc_meta == nullptr);
  if (bc_meta_len) {
    m_bc_meta = (uchar*)malloc(bc_meta_len);
    memcpy(m_bc_meta, bc_meta, bc_meta_len);
  }
  m_bc_meta_len = bc_meta_len;
}

void UnitEmitter::setLines(const LineTable& lines) {
  Offset prevPastOffset = 0;
  for (size_t i = 0; i < lines.size(); ++i) {
    const LineEntry* line = &lines[i];
    Location sLoc;
    sLoc.line0 = sLoc.line1 = line->val();
    Offset pastOffset = line->pastOffset();
    recordSourceLocation(&sLoc, prevPastOffset);
    prevPastOffset = pastOffset;
  }
}

Id UnitEmitter::addPreConst(const StringData* name, const TypedValue& value) {
  assert(value.m_type != KindOfObject && value.m_type != KindOfArray);
  PreConst pc = { value, nullptr, name };
  if (pc.value.m_type == KindOfString && !pc.value.m_data.pstr->isStatic()) {
    pc.value.m_data.pstr = StringData::GetStaticString(pc.value.m_data.pstr);
    pc.value.m_type = KindOfStaticString;
  }
  assert(!IS_REFCOUNTED_TYPE(pc.value.m_type));

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
  if (key == nullptr) {
    String s = f_serialize(a);
    key = StringData::GetStaticString(s.get());
  }

  ArrayIdMap::const_iterator it = m_array2id.find(key);
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
  assert(m_fes.size() == 0);
  StringData* name = StringData::GetStaticString("");
  FuncEmitter* pseudomain = newFuncEmitter(name, false);
  Attr attrs = AttrMayUseVV;
  pseudomain->init(line1, line2, 0, attrs, false, name);
}

FuncEmitter* UnitEmitter::newFuncEmitter(const StringData* n, bool top) {
  assert(m_fes.size() > 0 || !strcmp(n->data(), "")); // Pseudomain comes first.
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
    m_hoistablePceIdVec.push_back(pce->id());
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

void UnitEmitter::recordSourceLocation(const Location* sLoc, Offset start) {
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

Func* UnitEmitter::newFunc(const FuncEmitter* fe, Unit& unit, Id id, int line1,
                           int line2, Offset base, Offset past,
                           const StringData* name, Attr attrs, bool top,
                           const StringData* docComment, int numParams,
                           bool isGenerator) {
  Func* f = new (Func::allocFuncMem(name, numParams))
    Func(unit, id, line1, line2, base, past, name, attrs,
         top, docComment, numParams, isGenerator);
  m_fMap[fe] = f;
  return f;
}

Func* UnitEmitter::newFunc(const FuncEmitter* fe, Unit& unit,
                           PreClass* preClass, int line1, int line2,
                           Offset base, Offset past,
                           const StringData* name, Attr attrs, bool top,
                           const StringData* docComment, int numParams,
                           bool isGenerator) {
  Func* f = new (Func::allocFuncMem(name, numParams))
    Func(unit, preClass, line1, line2, base, past, name,
         attrs, top, docComment, numParams, isGenerator);
  m_fMap[fe] = f;
  return f;
}

template<class SourceLocTable>
static LineTable createLineTable(SourceLocTable& srcLoc, Offset bclen) {
  LineTable lines;
  for (size_t i = 0; i < srcLoc.size(); ++i) {
    Offset endOff = i < srcLoc.size() - 1 ? srcLoc[i + 1].first : bclen;
    lines.push_back(LineEntry(endOff, srcLoc[i].second.line1));
  }
  return lines;
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
      LineTable lines = createLineTable(m_sourceLocTab, m_bclen);
      urp.insertUnit(repoId).insert(txn, m_sn, m_md5, m_bc, m_bclen,
                                    m_bc_meta, m_bc_meta_len,
                                    &m_mainReturn, lines);
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
    for (int i = 0, n = m_mergeableStmts.size(); i < n; i++) {
      switch (m_mergeableStmts[i].first) {
        case UnitMergeKindDone:
        case UnitMergeKindUniqueDefinedClass:
          not_reached();
        case UnitMergeKindClass: break;
        case UnitMergeKindReqMod:
        case UnitMergeKindReqSrc:
        case UnitMergeKindReqDoc: {
          urp.insertUnitMergeable(repoId).insert(
            txn, usn, i,
            m_mergeableStmts[i].first, m_mergeableStmts[i].second, nullptr);
          break;
        }
        case UnitMergeKindDefine:
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
    TRACE(3, "Failed to commit '%s' (0x%016llx%016llx) to '%s': %s\n",
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
      TRACE(3, "Failed to commit '%s' (0x%016llx%016llx) to '%s': %s\n",
               m_filepath->data(), m_md5.q[0], m_md5.q[1],
               repo.repoName(repoId).c_str(), re.msg().c_str());
    }
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
  u->m_mainReturn = m_mainReturn;
  {
    const std::string& dirname = Util::safe_dirname(m_filepath->data(),
                                                    m_filepath->size());
    u->m_dirpath = StringData::GetStaticString(dirname);
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
  size_t ix = m_fes.size() + m_hoistablePceIdVec.size();
  if (u->m_mainReturn._count && !m_allClassesHoistable) {
    size_t extra = 0;
    for (MergeableStmtVec::const_iterator it = m_mergeableStmts.begin();
         it != m_mergeableStmts.end(); ++it) {
      extra++;
      if (!RuntimeOption::RepoAuthoritative) {
        if (it->first != UnitMergeKindClass) {
          extra = 0;
          u->m_mainReturn._count = 0;
          break;
        }
      } else switch (it->first) {
          case UnitMergeKindDefine:
          case UnitMergeKindGlobal:
            extra += sizeof(TypedValue) / sizeof(void*);
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
  for (FeVec::const_iterator it = m_fes.begin(); it != m_fes.end(); ++it) {
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
  for (IdVec::const_iterator it = m_hoistablePceIdVec.begin();
       it != m_hoistablePceIdVec.end(); ++it) {
    mi->mergeableObj(ix++) = u->m_preClasses[*it].get();
  }
  mi->m_firstMergeablePreClass = ix;
  if (u->m_mainReturn._count && !m_allClassesHoistable) {
    for (MergeableStmtVec::const_iterator it = m_mergeableStmts.begin();
         it != m_mergeableStmts.end(); ++it) {
      switch (it->first) {
        case UnitMergeKindClass:
          mi->mergeableObj(ix++) = u->m_preClasses[it->second].get();
          break;
        case UnitMergeKindReqMod:
        case UnitMergeKindReqSrc:
        case UnitMergeKindReqDoc: {
          assert(RuntimeOption::RepoAuthoritative);
          void* name = u->lookupLitstrId(it->second);
          mi->mergeableObj(ix++) = (char*)name + (int)it->first;
          break;
        }
        case UnitMergeKindDefine:
        case UnitMergeKindGlobal: {
          assert(RuntimeOption::RepoAuthoritative);
          void* name = u->lookupLitstrId(m_mergeableValues[it->second].first);
          mi->mergeableObj(ix++) = (char*)name + (int)it->first;
          *(TypedValue*)mi->mergeableData(ix) =
            m_mergeableValues[it->second].second;
          ix += sizeof(TypedValue) / sizeof(void*);
          assert(sizeof(TypedValue) % sizeof(void*) == 0);
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
  u->m_lineTable = createLineTable(m_sourceLocTab, m_bclen);
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

  u->m_preConsts = m_preConsts;
  for (PreConstVec::iterator i = u->m_preConsts.begin();
       i != u->m_preConsts.end(); ++i) {
    i->owner = u;
  }

  if (RuntimeOption::EvalDumpBytecode) {
    // Dump human-readable bytecode.
    Trace::trace(u->toString());
  }

  static const bool kAlwaysVerify = getenv("HHVM_ALWAYS_VERIFY");
  static const bool kVerifyNonSystem = getenv("HHVM_VERIFY");
  static const bool kVerifyVerbose = getenv("HHVM_VERIFY_VERBOSE");
  const bool doVerify = kAlwaysVerify ||
     (kVerifyNonSystem && !u->filepath()->empty() &&
      !boost::ends_with(u->filepath()->data(), "systemlib.php"));
  if (doVerify) {
    Verifier::checkUnit(u, kVerifyVerbose);
  }
  return u;
}

///////////////////////////////////////////////////////////////////////////////
}
}
