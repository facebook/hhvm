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

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/collections.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/enum-cache.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/strings.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/base/type-structure.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/globals-array.h"
#include "hphp/runtime/vm/instance-bits.h"
#include "hphp/runtime/vm/native-data.h"
#include "hphp/runtime/vm/native-prop-handler.h"
#include "hphp/runtime/vm/vm-regs.h"
#include "hphp/runtime/vm/treadmill.h"

#include "hphp/runtime/ext/collections/ext_collections.h"
#include "hphp/runtime/ext/string/ext_string.h"
#include "hphp/runtime/ext/std/ext_std_closure.h"

#include "hphp/parser/parser.h"

#include "hphp/util/debug.h"
#include "hphp/util/logger.h"

#include <folly/Bits.h>
#include <folly/Optional.h>

#include <algorithm>
#include <iostream>

TRACE_SET_MOD(class_load);

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

const StaticString s_86ctor("86ctor");
const StaticString s_86pinit("86pinit");
const StaticString s_86sinit("86sinit");

Mutex g_classesMutex;

///////////////////////////////////////////////////////////////////////////////

/*
 * We clone methods with static locals into derived classes, but the clone
 * still points to the class the method was defined in (because it needs to
 * have the right context class).  For data profiling, we need to find the
 * actual class that a Func belongs to so we put such Funcs into this map.
 */
typedef tbb::concurrent_hash_map<uint64_t, const Class*> FuncIdToClassMap;
static FuncIdToClassMap* s_funcIdToClassMap;

const Class* getOwningClassForFunc(const Func* f) {
  // We only populate s_funcIdToClassMap when EvalPerfDataMap is true.
  assert(RuntimeOption::EvalPerfDataMap);

  if (s_funcIdToClassMap) {
    FuncIdToClassMap::const_accessor acc;
    if (s_funcIdToClassMap->find(acc, f->getFuncId())) {
      return acc->second;
    }
  }
  return f->cls();
}


///////////////////////////////////////////////////////////////////////////////
// Class::PropInitVec.

Class::PropInitVec::~PropInitVec() {
  if (m_capacity > 0) {
    free_huge(m_data);
  }
}

Class::PropInitVec*
Class::PropInitVec::allocWithReqAllocator(const PropInitVec& src) {
  PropInitVec* p = req::make_raw<PropInitVec>();
  uint32_t sz = src.m_size;
  p->m_size = sz;
  p->m_capacity = ~sz;
  p->m_data = req::make_raw_array<TypedValueAux>(sz);
  memcpy(p->m_data, src.m_data, sz * sizeof(*p->m_data));
  return p;
}

const Class::PropInitVec&
Class::PropInitVec::operator=(const PropInitVec& piv) {
  assert(!reqAllocated());
  if (this != &piv) {
    if (UNLIKELY(m_capacity)) {
      free_huge(m_data);
      m_data = nullptr;
    }
    unsigned sz = m_size = m_capacity = piv.size();
    if (sz == 0) return *this;
    m_data = (TypedValueAux*)malloc_huge(sz * sizeof(*m_data));
    assert(m_data);
    memcpy(m_data, piv.m_data, sz * sizeof(*m_data));
  }
  return *this;
}

void Class::PropInitVec::push_back(const TypedValue& v) {
  assert(!reqAllocated());
  if (m_size == m_capacity) {
    unsigned newCap = folly::nextPowTwo(m_size + 1);
    m_capacity = static_cast<int32_t>(newCap);
    auto newData = malloc_huge(newCap * sizeof(TypedValue));
    if (m_data) {
      memcpy(newData, m_data, m_size * sizeof(*m_data));
      free_huge(m_data);
    }
    m_data = reinterpret_cast<TypedValueAux*>(newData);
    assert(m_data);
  }
  cellDup(v, m_data[m_size++]);
}


///////////////////////////////////////////////////////////////////////////////
// Class.

namespace {

/*
 * Load used traits of PreClass `preClass', and append the trait Class*'s to
 * 'usedTraits'.  Return an estimate of the method count of all used traits.
 */
unsigned loadUsedTraits(PreClass* preClass,
                        std::vector<ClassPtr>& usedTraits) {
  unsigned methodCount = 0;
  for (auto const& traitName : preClass->usedTraits()) {
    Class* classPtr = Unit::loadClass(traitName);
    if (classPtr == nullptr) {
      raise_error(Strings::TRAITS_UNKNOWN_TRAIT, traitName->data());
    }
    if (!(classPtr->attrs() & AttrTrait)) {
      raise_error("%s cannot use %s - it is not a trait",
                  preClass->name()->data(),
                  classPtr->name()->data());
    }

    if (RuntimeOption::RepoAuthoritative) {
      // In RepoAuthoritative mode (with the WholeProgram compiler
      // optimizations), the contents of traits are flattened away into the
      // preClasses of "use"r classes. Continuing here allows us to avoid
      // unnecessarily attempting to re-import trait methods and
      // properties, only to fail due to (surprise surprise!) the same
      // method/property existing on m_preClass.
      continue;
    }

    usedTraits.push_back(ClassPtr(classPtr));
    methodCount += classPtr->numMethods();

  }

  if (!RuntimeOption::RepoAuthoritative) {
    // Trait aliases can increase method count. Get an estimate of the
    // number of aliased functions. This doesn't need to be done in
    // RepoAuthoritative mode due to trait flattening ensuring that added
    // methods are already present in the preclass.
    for (auto const& rule : preClass->traitAliasRules()) {
      auto origName = rule.origMethodName();
      auto newName = rule.newMethodName();
      if (origName != newName) methodCount++;
    }
  }
  return methodCount;
}

/*
 * Class ends with a dynamically sized array, m_classVec. C++ doesn't allow
 * declaring empty arrays like C does, so we give it size 1 and use
 * m_classVec's offset as the true size of Class when allocating memory to
 * construct one.
 */
constexpr size_t sizeof_Class = Class::classVecOff();

template<size_t sz>
struct assert_sizeof_class {
  // If this static_assert fails, the compiler error will have the real value
  // of sizeof_Class in it since it's in this struct's type.
#ifdef DEBUG
  static_assert(sz == (use_lowptr ? 260 : 304), "Change this only on purpose");
#else
  static_assert(sz == (use_lowptr ? 252 : 296), "Change this only on purpose");
#endif
};
template struct assert_sizeof_class<sizeof_Class>;

/*
 * R/W lock for caching scopings of closures.
 */
ReadWriteMutex s_scope_cache_mutex;

}

Class* Class::newClass(PreClass* preClass, Class* parent) {
  auto const classVecLen = parent != nullptr ? parent->m_classVecLen + 1 : 1;
  auto funcVecLen = (parent != nullptr ? parent->m_methods.size() : 0)
    + preClass->numMethods();

  std::vector<ClassPtr> usedTraits;
  auto numTraitMethodsEstimate = loadUsedTraits(preClass, usedTraits);
  // In RepoAuthoritative mode, trait methods are already flattened
  // into the preClass, so we don't need to add in the estimate here.
  if (!RuntimeOption::RepoAuthoritative) {
    funcVecLen += numTraitMethodsEstimate;
  }

  // We need to pad this allocation so that the actual start of the Class is
  // 8-byte aligned.
  auto const mask = alignof(Class) - 1;
  auto const funcvec_sz = sizeof(LowPtr<Func>) * funcVecLen;
  auto const prefix_sz = (funcvec_sz + mask) & ~mask;

  auto const size = sizeof_Class + prefix_sz
                    + sizeof(m_classVec[0]) * classVecLen;

  auto const mem = low_malloc_data(size);
  auto const classPtr = reinterpret_cast<void*>(
    reinterpret_cast<uintptr_t>(mem) + prefix_sz
  );
  try {
    return new (classPtr) Class(preClass, parent, std::move(usedTraits),
                                classVecLen, funcVecLen);
  } catch (...) {
    low_free_data(mem);
    throw;
  }
}

Class* Class::rescope(Class* ctx, Attr attrs /* = AttrNone */) {
  assert(parent() == c_Closure::classof());
  assert(m_invoke);

  bool const is_dynamic = (attrs != AttrNone);

  // Look up the generated template class for this particular subclass of
  // Closure.  This class maintains the table of scoped clones of itself, and
  // if we create a new scoped clone, we need to map it there.
  auto template_cls = is_dynamic ? preClass()->namedEntity()->clsList() : this;
  auto const invoke = template_cls->m_invoke;

  assert(IMPLIES(is_dynamic, m_scoped));
  assert(IMPLIES(is_dynamic, template_cls->m_scoped));

  auto const try_template = [&]() -> Class* {
    bool const ctx_match = invoke->cls() == ctx;
    bool const attrs_match = (attrs == AttrNone || attrs == invoke->attrs());

    // The first scoping will never be for `template_cls' (since it's
    // impossible to define a closure in the context of its own Closure
    // subclass), so if this happens, it means that `template_cls' is in a
    // "de-scoped" state and we shouldn't use it.  (See Class::releaseRefs().)
    bool const ctx_override = template_cls->m_scoped &&
                              invoke->cls() == template_cls;

    return ctx_match && attrs_match && !ctx_override ? template_cls : nullptr;
  };

  // If the template class has already been scoped to `ctx', we're done.  This
  // is the common case in repo mode.
  if (auto cls = try_template()) return cls;

  template_cls->allocExtraData();
  auto& scopedClones = template_cls->m_extra.raw()->m_scopedClones;

  auto const key = CloneScope { ctx, attrs };

  auto const try_cache = [&] {
    auto it = scopedClones.find(key);
    return it != scopedClones.end() ? it->second.get() : nullptr;
  };

  { // Return the cached clone if we have one.
    ReadLock l(s_scope_cache_mutex);

    // If this succeeds, someone raced us to scoping the template.  We may have
    // unnecessarily allocated an ExtraData, but whatever.
    if (auto cls = try_template()) return cls;

    if (auto cls = try_cache()) return cls;
  }

  auto cloneClass = [&] {
    auto const cls = newClass(m_preClass.get(), m_parent.get());
    return cls;
  };

  // We use the French for closure because using the English crashes gcc in the
  // implicit lambda capture below.  (This is fixed in gcc 4.8.5.)
  auto fermeture = ClassPtr {
    template_cls->m_scoped ? cloneClass() : template_cls
  };

  WriteLock l(s_scope_cache_mutex);

  if (fermeture->m_scoped) {
    // We raced with someone who scoped the template_cls just as we were about
    // to, so make a new Class.  (Note that we don't want to do this with the
    // lock held, since it's very expensive.)
    //
    // This race should be far less likely than a race between two attempted
    // first-scopings for `template_cls', which is why we don't do an test-and-
    // set when we first check `m_scoped' before acquiring the lock.
    s_scope_cache_mutex.release();
    SCOPE_EXIT { s_scope_cache_mutex.acquireWrite(); };
    fermeture = ClassPtr { cloneClass() };
  }

  // Check the caches again.
  if (auto cls = try_template()) return cls;
  if (auto cls = try_cache()) return cls;

  fermeture->m_invoke->rescope(ctx, attrs);
  fermeture->m_scoped = true;

  if (ctx != nullptr &&
      !RuntimeOption::RepoAuthoritative &&
      !classHasPersistentRDS(ctx)) {
    // If the context Class might be destroyed, we need to do extra accounting
    // so that we can drop all clones scoped to it at the time of destruction.
    ctx->allocExtraData();
    ctx->m_extra.raw()->m_clonesWithThisScope.push_back(
      ScopedCloneBackref { ClassPtr(template_cls), attrs });
  }

  auto updateClones = [&] {
    if (template_cls != fermeture.get()) {
      scopedClones[key] = fermeture;
      fermeture.get()->setClassHandle(template_cls->m_cachedClass);
    }
  };

  InstanceBits::ifInitElse(
    [&] {
      fermeture->setInstanceBits();
      updateClones();
    },
    [&] {
      updateClones();
    }
  );

  return fermeture.get();
}

EnumValues* Class::setEnumValues(EnumValues* values) {
  auto extra = m_extra.ensureAllocated();
  EnumValues* expected = nullptr;
  if (!extra->m_enumValues.compare_exchange_strong(
        expected, values, std::memory_order_relaxed)) {
    // Already set by someone else, use theirs.
    delete values;
    return expected;
  } else {
    return values;
  }
}

Class::ExtraData::~ExtraData() {
  delete m_enumValues.load(std::memory_order_relaxed);
}

void Class::destroy() {
  /*
   * If we were never put on NamedEntity::classList, or
   * we've already been destroy'd, there's nothing to do
   */
  if (!m_cachedClass.bound()) return;

  Lock l(g_classesMutex);
  // Need to recheck now we have the lock
  if (!m_cachedClass.bound()) return;
  // Only do this once.
  m_cachedClass = rds::Link<LowPtr<Class>>(rds::kInvalidHandle);

  /*
   * Regardless of refCount, this Class is now unusable.  Remove it
   * from the class list.
   *
   * Needs to be under the lock, because multiple threads could call
   * destroy, or want to manipulate the class list.  (It's safe for
   * other threads to concurrently read the class list without the
   * lock.)
   */
  auto const pcls = m_preClass.get();
  pcls->namedEntity()->removeClass(this);

  if (m_sPropCache) {
    // Other threads find this class via rds::s_handleTable.
    // Remove our sprop entries before the treadmill delay.
    for (Slot i = 0, n = numStaticProperties(); i < n; ++i) {
      if (m_staticProperties[i].cls == this) {
        auto const &link = m_sPropCache[i];
        if (link.bound()) {
          rds::unbind(rds::SPropCache{this, i}, link.handle());
        }
      }
    }
  }

  Treadmill::enqueue(
    [this] {
      releaseRefs();
      if (!this->decAtomicCount()) this->atomicRelease();
    }
  );
}

void Class::atomicRelease() {
  assert(!m_cachedClass.bound());
  assert(!getCount());
  this->~Class();
  low_free_data(mallocPtr());
}

Class::~Class() {
  releaseRefs(); // must be called for Func-nulling side effects

  if (m_sPropCache) {
    for (unsigned i = 0, n = numStaticProperties(); i < n; ++i) {
      m_sPropCache[i].~Link();
    }
    free_huge(m_sPropCache);
  }

  for (auto i = size_t{}, n = numMethods(); i < n; i++) {
    if (auto meth = getMethod(i)) {
      if (meth->isPreFunc()) {
        meth->freeClone();
      } else {
        Func::destroy(meth);
      }
    }
  }

  if (m_extra) {
    delete m_extra.raw();
  }

  // clean enum cache
  EnumCache::deleteValues(this);

  low_free_data(m_vtableVec.get());

#ifdef DEBUG
  validate();
  m_magic = ~m_magic;
#endif
}

void Class::releaseRefs() {
  /*
   * We have to be careful here.
   * We want to free up as much as possible as early as possible, but
   * some of our methods may actually belong to our parent
   * This means we can't destroy *our* Funcs until our refCount
   * hits zero (ie when Class::~Class gets called), because there
   * could be a child class which hasn't yet been destroyed, which
   * will need to inspect them. Also, we need to inspect the Funcs
   * now (while we still have a references to parent) to determine
   * which ones we will eventually need to free.
   * Similarly, if any of our func's belong to a parent class, we
   * can't free the parent, because one of our children could also
   * have a reference to those func's (and its only reference to
   * our parent is via this class).
   */
  auto num = numMethods();
  bool okToReleaseParent = true;
  for (auto i = 0; i < num; i++) {
    Func* meth = getMethod(i);
    if (meth /* releaseRefs can be called more than once */ &&
        meth->cls() != this &&
        ((meth->attrs() & AttrPrivate) || !meth->hasStaticLocals())) {
      setMethod(i, nullptr);
      okToReleaseParent = false;
    }
  }

  if (okToReleaseParent) {
    m_parent.reset();
  }

  m_numDeclInterfaces = 0;
  m_declInterfaces.reset();
  m_requirements.clear();

  if (m_extra) {
    auto xtra = m_extra.raw();
    xtra->m_usedTraits.clear();

    if (xtra->m_clonesWithThisScope.size() > 0) {
      WriteLock l(s_scope_cache_mutex);

      // Purge all references to scoped closure clones that are scoped to
      // `this'---there is no way anyone can find them at this point.
      for (auto const& cloneref : xtra->m_clonesWithThisScope) {
        auto const template_cls = cloneref.template_cls;
        auto const attrs = cloneref.ctx_attrs;

        auto const invoke = template_cls->m_invoke;

        if (invoke->cls() == this && attrs == AttrNone) {
          // We only hijack the `template_cls' as a clone for static rescopings
          // (which are signified by AttrNone).  To undo this, we need to make
          // sure that /no/ scoping will match with that of `template_cls'.  We
          // can accomplish this by using `template_cls' itself as the context.
          // Any instance of this closure will have its own scoped clone, and
          // we are de-scoping `template_cls' here, so the only way to obtain
          // it for dynamic binding is to reference it by name, which is
          // logically private (in PHP7, it's always just "Closure").  In HHVM,
          // this is possible by obtaining the Closure subclass's name, in
          // which case we'll just force a fresh clone via a special-case check
          // in Class::rescope().
          invoke->rescope(template_cls.get(), AttrNone);
          // We explicitly decline to reset template_cls->m_scoped.  This lets
          // us simplify some assertions in rescope(), gives us a nice sanity
          // check for debugging, and avoids having to play around too much
          // with how Func::rescope() works, at the cost of preventing the
          // template from being scoped again.  This should only happen outside
          // of RepoAuthoritative mode while code is being modified, so the
          // extra memory usage is not a substantial concern.
        } else {
          assertx(template_cls->m_extra);
          auto& scopedClones = template_cls->m_extra.raw()->m_scopedClones;

          auto const it = scopedClones.find(CloneScope { this, attrs });
          assertx(it != scopedClones.end());
          it->second->m_cachedClass =
            rds::Link<LowPtr<Class>>(rds::kInvalidHandle);
          scopedClones.erase(it);
        }
      }
    }
    xtra->m_clonesWithThisScope.clear();
  }
}

Class::Avail Class::avail(Class*& parent,
                          bool tryAutoload /* = false */) const {
  if (Class *ourParent = m_parent.get()) {
    if (!parent) {
      PreClass *ppcls = ourParent->m_preClass.get();
      parent = Unit::getClass(ppcls->namedEntity(),
                              m_preClass.get()->parent(), tryAutoload);
      if (!parent) {
        parent = ourParent;
        return Avail::Fail;
      }
    }
    if (parent != ourParent) {
      if (UNLIKELY(ourParent->isZombie())) {
        const_cast<Class*>(this)->destroy();
      }
      return Avail::False;
    }
  }

  for (size_t i = 0; i < m_numDeclInterfaces; i++) {
    auto di = m_declInterfaces.get()[i].get();
    const StringData* pdi = m_preClass.get()->interfaces()[i];
    assert(pdi->isame(di->name()));

    PreClass *pint = di->m_preClass.get();
    Class* interface = Unit::getClass(pint->namedEntity(), pdi,
                                      tryAutoload);
    if (interface != di) {
      if (interface == nullptr) {
        parent = di;
        return Avail::Fail;
      }
      if (UNLIKELY(di->isZombie())) {
        const_cast<Class*>(this)->destroy();
      }
      return Avail::False;
    }
  }

  if (RuntimeOption::RepoAuthoritative) {
    if (m_preClass->usedTraits().size()) {
      int numIfaces = m_interfaces.size();
      for (int i = 0; i < numIfaces; i++) {
        auto di = m_interfaces[i].get();

        PreClass *pint = di->m_preClass.get();
        Class* interface = Unit::getClass(pint->namedEntity(), pint->name(),
                                          tryAutoload);
        if (interface != di) {
          if (interface == nullptr) {
            parent = di;
            return Avail::Fail;
          }
          if (UNLIKELY(di->isZombie())) {
            const_cast<Class*>(this)->destroy();
          }
          return Avail::False;
        }
      }
    }
  } else {
    for (size_t i = 0, n = m_extra->m_usedTraits.size(); i < n; ++i) {
      auto usedTrait = m_extra->m_usedTraits.at(i).get();
      const StringData* usedTraitName = m_preClass.get()->usedTraits()[i];
      PreClass* ptrait = usedTrait->m_preClass.get();
      Class* trait = Unit::getClass(ptrait->namedEntity(), usedTraitName,
                                    tryAutoload);
      if (trait != usedTrait) {
        if (trait == nullptr) {
          parent = usedTrait;
          return Avail::Fail;
        }
        if (UNLIKELY(usedTrait->isZombie())) {
          const_cast<Class*>(this)->destroy();
        }
        return Avail::False;
      }
    }
  }

  return Avail::True;
}


///////////////////////////////////////////////////////////////////////////////
// Ancestry.

const Class* Class::commonAncestor(const Class* cls) const {
  assert(isNormalClass(this) && isNormalClass(cls));

  // Walk up m_classVec for both classes to look for a common ancestor.
  auto vecIdx = std::min(m_classVecLen, cls->m_classVecLen) - 1;
  do {
    assert(vecIdx < m_classVecLen && vecIdx < cls->m_classVecLen);
    if (m_classVec[vecIdx] == cls->m_classVec[vecIdx]) {
      return m_classVec[vecIdx];
    }
  } while (vecIdx--);

  return nullptr;
}

const Class* Class::getClassDependency(const StringData* name) const {
  for (auto idx = m_classVecLen; idx--; ) {
    auto cls = m_classVec[idx];
    if (cls->name()->isame(name)) return cls;
  }

  return m_interfaces.lookupDefault(name, nullptr);
}

///////////////////////////////////////////////////////////////////////////////
// Magic methods.

const Func* Class::getDeclaredCtor() const {
  const Func* f = getCtor();
  return f->name() != s_86ctor.get() ? f : nullptr;
}

const Func* Class::getCachedInvoke() const {
  assert(IMPLIES(m_invoke, !m_invoke->isStaticInPrologue()));
  return m_invoke;
}

const StaticString s_call("__call");

bool Class::hasCall() const {
  return this->lookupMethod(s_call.get()) ? true : false;
}

///////////////////////////////////////////////////////////////////////////////
// Builtin classes.

bool Class::isCppSerializable() const {
  assert(instanceCtor()); // Only call this on CPP classes
  auto* ndi = m_extra ? m_extra.raw()->m_nativeDataInfo : nullptr;
  return ndi != nullptr && ndi->isSerializable();
}

bool Class::isCollectionClass() const {
  auto s = name();
  return collections::isTypeName(s);
}


///////////////////////////////////////////////////////////////////////////////
// Property initialization.

void Class::initialize() const {
  if (m_pinitVec.size() > 0 && getPropData() == nullptr) {
    initProps();
  }
  if (numStaticProperties() > 0 && needsInitSProps()) {
    initSProps();
  }
}

bool Class::initialized() const {
  if (m_pinitVec.size() > 0 && getPropData() == nullptr) {
    return false;
  }
  if (numStaticProperties() > 0 && needsInitSProps()) {
    return false;
  }
  return true;
}

void Class::initProps() const {
  assert(m_pinitVec.size() > 0);
  assert(getPropData() == nullptr);
  // Copy initial values for properties to a new vector that can be used to
  // complete initialization for non-scalar properties via the iterative
  // 86pinit() calls below. 86pinit() takes a reference to an array to populate
  // with initial property values; after it completes, we copy the values into
  // the new propVec.
  auto propVec = PropInitVec::allocWithReqAllocator(m_declPropInit);

  VMRegAnchor _;

  initPropHandle();
  m_propDataCache.initWith(propVec);

  try {
    // Iteratively invoke 86pinit() methods upward
    // through the inheritance chain.
    for (auto it = m_pinitVec.rbegin(); it != m_pinitVec.rend(); ++it) {
      DEBUG_ONLY auto retval = g_context->invokeFunc(
        *it, init_null_variant, nullptr, const_cast<Class*>(this)
      );
      assert(retval.m_type == KindOfNull);
    }
  } catch (...) {
    // Undo the allocation of propVec
    req::destroy_raw_array(propVec->begin(), propVec->size());
    req::destroy_raw(propVec);
    *m_propDataCache = nullptr;
    m_propDataCache.markUninit();
    throw;
  }

  // For properties that do not require deep initialization, promote strings
  // and arrays that came from 86pinit to static. This allows us to initialize
  // object properties very quickly because we can just memcpy and we don't
  // have to do any refcounting.
  // For properties that require "deep" initialization, we have to do a little
  // more work at object creation time.
  Slot slot = 0;
  for (auto it = propVec->begin(); it != propVec->end(); ++it, ++slot) {
    TypedValueAux* tv = &(*it);
    // Set deepInit if the property requires "deep" initialization.
    if (m_declProperties[slot].attrs & AttrDeepInit) {
      tv->deepInit() = true;
    } else {
      tvAsVariant(tv).setEvalScalar();
      tv->deepInit() = false;
    }
  }
}

bool Class::needsInitSProps() const {
  return !m_sPropCacheInit.bound() || !m_sPropCacheInit.isInit();
}

void Class::initSProps() const {
  assert(needsInitSProps() || m_sPropCacheInit.isPersistent());

  const bool hasNonscalarInit = !m_sinitVec.empty();
  folly::Optional<VMRegAnchor> _;
  if (hasNonscalarInit) {
    _.emplace();
  }

  // Initialize static props for parent.
  Class* parent = this->parent();
  if (parent && parent->needsInitSProps()) {
    parent->initSProps();
  }

  if (!numStaticProperties()) return;

  initSPropHandles();

  // Perform scalar inits.
  for (Slot slot = 0, n = m_staticProperties.size(); slot < n; ++slot) {
    auto const& sProp = m_staticProperties[slot];

    if (sProp.cls == this && !m_sPropCache[slot].isPersistent()) {
      m_sPropCache[slot]->val = sProp.val;
    }
  }

  // If there are non-scalar initializers (i.e. 86sinit methods), run them now.
  // They will override the KindOfUninit values set by scalar initialization.
  if (hasNonscalarInit) {
    for (unsigned i = 0, n = m_sinitVec.size(); i < n; i++) {
      DEBUG_ONLY auto retval = g_context->invokeFunc(
        m_sinitVec[i], init_null_variant, nullptr, const_cast<Class*>(this)
      );
      assert(retval.m_type == KindOfNull);
    }
  }

  m_sPropCacheInit.initWith(true);
}


///////////////////////////////////////////////////////////////////////////////
// Property storage.

static rds::Link<bool> s_persistentTrue{rds::kInvalidHandle};

void Class::initSPropHandles() const {
  if (m_sPropCacheInit.bound()) return;

  bool usePersistentHandles = m_cachedClass.isPersistent();
  bool allPersistentHandles = usePersistentHandles;

  // Propagate to parents so we can link inherited static props.
  Class* parent = this->parent();
  if (parent) {
    parent->initSPropHandles();
    if (!rds::isPersistentHandle(parent->sPropInitHandle())) {
      allPersistentHandles = false;
    }
  }

  // Bind all the static prop handles.
  for (Slot slot = 0, n = m_staticProperties.size(); slot < n; ++slot) {
    auto& propHandle = m_sPropCache[slot];
    auto const& sProp = m_staticProperties[slot];

    if (!propHandle.bound()) {
      if (sProp.cls == this) {
        if (usePersistentHandles && (sProp.attrs & AttrPersistent)) {
          propHandle.bind(rds::Mode::Persistent);
          propHandle->val = sProp.val;
          rds::recordRds(propHandle.handle(), sizeof(StaticPropData),
                         rds::SPropCache{this, slot});
        } else {
          propHandle = rds::bind<StaticPropData>(
              rds::SPropCache{this, slot}, rds::Mode::Local
          );
        }
      } else {
        auto realSlot = sProp.cls->lookupSProp(sProp.name);
        propHandle = sProp.cls->m_sPropCache[realSlot];
      }
    } else if (propHandle.isPersistent() && sProp.cls == this) {
      /*
       * Avoid a weird race: two threads come through at once, the first
       * gets as far as binding propHandle, but then sleeps. Meanwhile the
       * second sees that its been bound, finishes up, and then tries to
       * read the property, but sees uninit-null for the value (and asserts
       * in a dbg build)
       */
      propHandle->val = sProp.val;
    }
    if (!propHandle.isPersistent()) {
      allPersistentHandles = false;
    }
  }

  // Bind the init handle; this indicates that all handles are bound.
  if (allPersistentHandles) {
    // We must make sure the value stored at the handle is correct before
    // setting m_sPropCacheInit in case another thread tries to read it at just
    // the wrong time. And rather than giving each Class its own persistent
    // handle that always points to an immutable 'true', share one between all
    // of them.
    if (!s_persistentTrue.bound()) {
      s_persistentTrue.bind(rds::Mode::Persistent);
      *s_persistentTrue = true;
    }
    m_sPropCacheInit = s_persistentTrue;
  } else {
    m_sPropCacheInit.bind();
  }
  rds::recordRds(m_sPropCacheInit.handle(),
                 sizeof(bool), "SPropCacheInit", name()->data());
}

Class::PropInitVec* Class::getPropData() const {
  return (m_propDataCache.bound() && m_propDataCache.isInit())
    ? *m_propDataCache
    : nullptr;
}

TypedValue* Class::getSPropData(Slot index) const {
  assert(numStaticProperties() > index);
  return m_sPropCache[index].bound() ? &m_sPropCache[index].get()->val :
         nullptr;
}


///////////////////////////////////////////////////////////////////////////////
// Property lookup and accessibility.

Class::PropLookup<Slot> Class::getDeclPropIndex(
  const Class* ctx,
  const StringData* key
) const {
  auto const propInd = lookupDeclProp(key);

  auto accessible = false;

  if (propInd != kInvalidSlot) {
    auto const attrs = m_declProperties[propInd].attrs;
    if ((attrs & (AttrProtected|AttrPrivate)) &&
        (g_context.isNull() || !g_context->debuggerSettings.bypassCheck)) {
      // Fetch the class in the inheritance tree which first declared the
      // property
      auto const baseClass = m_declProperties[propInd].cls;
      assert(baseClass);

      // If ctx == baseClass, we have the right property and we can stop here.
      if (ctx == baseClass) return PropLookup<Slot> { propInd, true };

      // The anonymous context cannot access protected or private properties, so
      // we can fail fast here.
      if (ctx == nullptr) return PropLookup<Slot> { propInd, false };

      assert(ctx);
      if (attrs & AttrPrivate) {
        // ctx != baseClass and the property is private, so it is not
        // accessible. We need to keep going because ctx may define a private
        // property with this name.
        accessible = false;
      } else {
        if (ctx == (Class*)-1 || ctx->classof(baseClass)) {
          // The special ctx (Class*)-1 is used by unserialization to
          // mean that protected properties are ok. Otherwise,
          // ctx is derived from baseClass, so we know this protected
          // property is accessible and we know ctx cannot have private
          // property with the same name, so we're done.
          return PropLookup<Slot> { propInd, true };
        }
        if (!baseClass->classof(ctx)) {
          // ctx is not the same, an ancestor, or a descendent of baseClass,
          // so the property is not accessible. Also, we know that ctx cannot
          // be the same or an ancestor of this, so we don't need to check if
          // ctx declares a private property with the same name and we can
          // fail fast here.
          return PropLookup<Slot> { propInd, false };
        }
        // We now know this protected property is accessible, but we need to
        // keep going because ctx may define a private property with the same
        // name.
        accessible = true;
        assert(baseClass->classof(ctx));
      }
    } else {
      // The property is public (or we're in the debugger and we are bypassing
      // accessibility checks).
      accessible = true;
      // If ctx == this, we don't have to check if ctx defines a private
      // property with the same name and we can stop here.
      if (ctx == this) return PropLookup<Slot> { propInd, true };

      // We still need to check if ctx defines a private property with the same
      // name.
    }
  } else {
    // We didn't find a visible declared property in this's property map
    accessible = false;
  }

  // If ctx is an ancestor of this, check if ctx has a private property with the
  // same name.
  if (ctx && ctx != (Class*)-1 && classof(ctx)) {
    auto const ctxPropInd = ctx->lookupDeclProp(key);

    if (ctxPropInd != kInvalidSlot &&
        ctx->m_declProperties[ctxPropInd].cls == ctx &&
        (ctx->m_declProperties[ctxPropInd].attrs & AttrPrivate)) {
      // A private property from ctx trumps any other property we may
      // have found.
      return PropLookup<Slot> { ctxPropInd, true };
    }
  }

  return PropLookup<Slot> { propInd, accessible };
}

Class::PropLookup<Slot> Class::findSProp(
  const Class* ctx,
  const StringData* sPropName
) const {
  auto const sPropInd = lookupSProp(sPropName);

  // Non-existent property.
  if (sPropInd == kInvalidSlot) return PropLookup<Slot> { kInvalidSlot, false };

  // Property access within this Class's context.
  if (ctx == this) return PropLookup<Slot> { sPropInd, true };

  auto const sPropAttrs = m_staticProperties[sPropInd].attrs;

  auto const accessible = [&] {
    switch (sPropAttrs & (AttrPublic | AttrProtected | AttrPrivate)) {
      // Public properties are always accessible.
      case AttrPublic:
        return true;

      // Property access is from within a parent class's method, which is
      // allowed for protected properties.
      case AttrProtected:
        return ctx != nullptr && (classof(ctx) || ctx->classof(this));

      // Can only access private properties via the debugger.
      case AttrPrivate:
        return g_context->debuggerSettings.bypassCheck;

      default: break;
    }
    not_reached();
  }();

  return PropLookup<Slot> { sPropInd, accessible };
}

Class::PropLookup<TypedValue*> Class::getSProp(
  const Class* ctx,
  const StringData* sPropName
) const {
  initialize();

  auto const lookup = findSProp(ctx, sPropName);
  if (lookup.prop == kInvalidSlot) {
    return PropLookup<TypedValue*> { nullptr, false };
  }

  auto const sProp = getSPropData(lookup.prop);
  assert(sProp && sProp->m_type != KindOfUninit &&
         "Static property initialization failed to initialize a property.");
  return PropLookup<TypedValue*> { sProp, lookup.accessible };
}

bool Class::IsPropAccessible(const Prop& prop, Class* ctx) {
  if (prop.attrs & AttrPublic) return true;
  if (prop.attrs & AttrPrivate) return prop.cls == ctx;
  if (!ctx) return false;

  return prop.cls->classof(ctx) || ctx->classof(prop.cls);
}


///////////////////////////////////////////////////////////////////////////////
// Constants.

Cell Class::clsCnsGet(const StringData* clsCnsName, bool includeTypeCns) const {
  Slot clsCnsInd;
  auto cnsVal = cnsNameToTV(clsCnsName, clsCnsInd, includeTypeCns);
  if (!cnsVal) return make_tv<KindOfUninit>();

  auto& cns = m_constants[clsCnsInd];
  ArrayData* typeCns = nullptr;

  if (cnsVal->m_type != KindOfUninit) {
    if (cns.isType()) {
      // Type constants with the low bit set are already resolved and can be
      // returned after masking out that bit.
      assert(cnsVal->m_type == KindOfPersistentArray);
      typeCns = cnsVal->m_data.parr;
      auto const rawData = reinterpret_cast<intptr_t>(typeCns);
      if (rawData & 0x1) {
        return make_tv<KindOfPersistentArray>(
          reinterpret_cast<ArrayData*>(rawData ^ 0x1));
      }
    } else {
      return *cnsVal;
    }
  }

  // This constant has a non-scalar initializer, meaning it will be potentially
  // different in different requests, which we store separately in an array
  // living off in RDS.
  m_nonScalarConstantCache.bind();
  auto& clsCnsData = *m_nonScalarConstantCache;

  /*
   * We need a special marker value in the non-scalar constant cache to indicate
   * that we're currently evaluating the value of a constant. If we attempt to
   * evaluate the value of a constant, and the marker for that constant is
   * present, that means the constant is recursively defined and we'll raise an
   * error. We want to only store valid values in the cache to avoid breaking
   * invariants, so we'll use the globals array for this purpose. We don't allow
   * storing the globals array in a class constant, so this is unambiguous.
   */

  if (m_nonScalarConstantCache.isInit()) {
    if (auto cCns = clsCnsData->rval(clsCnsName)) {
      // There's an entry in the cache for this constant. If its the globals
      // array, this constant is recursively defined, so raise an
      // error. Otherwise just return it.
      if (UNLIKELY(isArrayType(cCns.type()) &&
                   cCns.val().parr == get_global_variables())) {
        raise_error(
          folly::sformat(
            "Cannot declare self-referencing constant '{}::{}'",
            name(),
            clsCnsName
          )
        );
      }
      return cCns.tv();
    }
  }

  auto makeCache = [&] {
    if (!m_nonScalarConstantCache.isInit()) {
      clsCnsData.detach();
      clsCnsData = Array::attach(
        MixedArray::MakeReserveMixed(m_constants.size())
      );
      m_nonScalarConstantCache.markInit();
    }
  };

  // Resolve type constant, if needed.
  if (cns.isType()) {
    Array resolvedTS;
    bool persistent = true;
    try {
      // We must give TypeStructure::resolve() the same ArrayData* we tested up
      // above, to avoid reading an already-resolved (by another thread)
      // ArrayData* from cns. Since resolve() takes a Class::Const and no other
      // fields of the Const can change, just copy cns and give the copy our
      // local version of the ArrayData*.
      auto cnsCopy = cns;
      cnsCopy.val.m_data.parr = typeCns;

      resolvedTS = TypeStructure::resolve(cnsCopy, this, persistent);
    } catch (const Exception& e) {
      raise_error(e.getMessage());
    }

    auto const ad = ArrayData::GetScalarArray(resolvedTS.get());
    if (persistent) {
      auto const rawData = reinterpret_cast<intptr_t>(ad);
      assert((rawData & 0x7) == 0 && "ArrayData not 8-byte aligned");
      auto taggedData = reinterpret_cast<ArrayData*>(rawData | 0x1);

      // Multiple threads might create and store the resolved type structure
      // here, but that's fine since they'll all store the same thing thanks to
      // GetScalarArray(). We could avoid a little duplicated work during
      // warmup with more complexity but it's not worth it.
      const_cast<TypedValueAux&>(cns.val).m_data.parr = taggedData;
      return make_tv<KindOfPersistentArray>(ad);
    }

    auto tv = make_tv<KindOfPersistentArray>(ad);
    makeCache();
    clsCnsData.set(StrNR(clsCnsName), tvAsCVarRef(&tv), true /* isKey */);
    return tv;
  }

  // We're going to run the 86cinit to get the constant's value. Store the
  // globals array in the constant's cache entry to prevent recursion.
  makeCache();
  auto marker = make_tv<KindOfArray>(get_global_variables());
  clsCnsData.set(StrNR(clsCnsName), tvAsCVarRef(&marker), true /* isKey */);

  // The class constant has not been initialized yet; do so.
  static auto const sd86cinit = makeStaticString("86cinit");
  auto const meth86cinit = cns.cls->lookupMethod(sd86cinit);
  TypedValue args[1] = {
    make_tv<KindOfPersistentString>(const_cast<StringData*>(cns.name.get()))
  };

  auto ret = g_context->invokeFuncFew(
    meth86cinit,
    ActRec::encodeClass(this),
    nullptr,
    1,
    args
  );

  assertx(tvAsCVarRef(&ret).isAllowedAsConstantValue());
  clsCnsData.set(StrNR(clsCnsName), cellAsCVarRef(ret), true /* isKey */);

  // The caller will inc-ref the returned value, so undo the inc-ref caused by
  // storing it in the cache.
  tvDecRefGenNZ(&ret);
  return ret;
}

const Cell* Class::cnsNameToTV(const StringData* clsCnsName,
                               Slot& clsCnsInd,
                               bool includeTypeCns) const {
  clsCnsInd = m_constants.findIndex(clsCnsName);
  if (clsCnsInd == kInvalidSlot) {
    return nullptr;
  }
  if (m_constants[clsCnsInd].isAbstract()) {
    return nullptr;
  }
  if (!includeTypeCns && m_constants[clsCnsInd].isType()) {
    return nullptr;
  }
  auto const ret = &m_constants[clsCnsInd].val;
  assert(m_constants[clsCnsInd].isType() || tvIsPlausible(*ret));
  return ret;
}

DataType Class::clsCnsType(const StringData* cnsName) const {
  Slot slot;
  auto const cns = cnsNameToTV(cnsName, slot);
  // TODO(#2913342): lookup the constant in RDS in case it's dynamic
  // and already initialized.
  if (!cns) return KindOfUninit;
  return cns->m_type;
}


///////////////////////////////////////////////////////////////////////////////
// Objects.

size_t Class::declPropOffset(Slot index) const {
  static_assert(std::is_unsigned<Slot>::value,
                "Slot is supposed to be unsigned");
  return sizeof(ObjectData) + index * sizeof(TypedValue);
}


///////////////////////////////////////////////////////////////////////////////
// Other methods.

bool Class::verifyPersistent() const {
  if (!(attrs() & AttrPersistent)) return false;
  if (m_parent.get() && !classHasPersistentRDS(m_parent.get())) {
    return false;
  }
  int numIfaces = m_interfaces.size();
  for (int i = 0; i < numIfaces; i++) {
    if (!classHasPersistentRDS(m_interfaces[i])) {
      return false;
    }
  }
  for (auto const& usedTrait : m_extra->m_usedTraits) {
    if (!classHasPersistentRDS(usedTrait.get())) {
      return false;
    }
  }
  return true;
}

void Class::setInstanceBits() {
  setInstanceBitsImpl<false>();
}
void Class::setInstanceBitsAndParents() {
  setInstanceBitsImpl<true>();
}

template<bool setParents>
void Class::setInstanceBitsImpl() {
  // Bit 0 is reserved to indicate whether or not the rest of the bits
  // are initialized yet.
  if (m_instanceBits.test(0)) return;

  InstanceBits::BitSet bits;
  bits.set(0);
  auto setBits = [&](Class* c) {
    if (setParents) c->setInstanceBitsAndParents();
    bits |= c->m_instanceBits;
  };
  if (m_parent.get()) setBits(m_parent.get());

  int numIfaces = m_interfaces.size();
  for (int i = 0; i < numIfaces; i++) setBits(m_interfaces[i]);

  // XXX: this assert fails on the initFlag; oops.
  if (unsigned bit = InstanceBits::lookup(m_preClass->name())) {
    bits.set(bit);
  }
  m_instanceBits = bits;
}

///////////////////////////////////////////////////////////////////////////////
// Private methods.
//
// These are mostly for the class creation path.

static const StaticString s___MockClass("__MockClass");
void Class::setParent() {
  // Cache m_preClass->attrs()
  m_attrCopy = m_preClass->attrs();

  // Validate the parent
  if (m_parent.get() != nullptr) {
    Attr parentAttrs = m_parent->attrs();
    if (UNLIKELY(parentAttrs &
                 (AttrFinal | AttrInterface | AttrTrait | AttrEnum))) {
      if (!(parentAttrs & AttrFinal) ||
          (parentAttrs & AttrEnum) ||
          m_preClass->userAttributes().find(s___MockClass.get()) ==
          m_preClass->userAttributes().end() ||
          m_parent->isCollectionClass()) {
        raise_error("Class %s may not inherit from %s (%s)",
                    m_preClass->name()->data(),
                    ((parentAttrs & AttrEnum)      ? "enum" :
                     (parentAttrs & AttrFinal)     ? "final class" :
                     (parentAttrs & AttrInterface) ? "interface"   : "trait"),
                    m_parent->name()->data());
      }
      if ((parentAttrs & AttrAbstract) &&
          ((m_attrCopy & (AttrAbstract|AttrFinal)) != (AttrAbstract|AttrFinal))) {
        raise_error(
          "Class %s with %s inheriting 'abstract final' class %s"
          " must also be 'abstract final'",
          m_preClass->name()->data(),
          s___MockClass.data(),
          m_parent->name()->data()
        );
      }
    }
  }

  // Handle stuff specific to cppext classes
  if (m_preClass->instanceCtor()) {
    allocExtraData();
    m_extra.raw()->m_instanceCtor = m_preClass->instanceCtor();
    m_extra.raw()->m_instanceDtor = m_preClass->instanceDtor();
  } else if (m_parent.get() && m_parent->m_extra->m_instanceCtor) {
    allocExtraData();
    m_extra.raw()->m_instanceCtor = m_parent->m_extra->m_instanceCtor;
    m_extra.raw()->m_instanceDtor = m_parent->m_extra->m_instanceDtor;
    // XXX: should this be copying over the clsInfo also?  Might be broken...
  }
}

static Func* findSpecialMethod(Class* cls, const StringData* name) {
  if (!cls->preClass()->hasMethod(name)) return nullptr;
  Func* f = cls->preClass()->lookupMethod(name);
  f = f->clone(cls);
  f->setNewFuncId();
  f->setBaseCls(cls);
  f->setHasPrivateAncestor(false);
  return f;
}

const StaticString
  s_toString("__toString"),
  s_construct("__construct"),
  s_destruct("__destruct"),
  s_invoke("__invoke"),
  s_sleep("__sleep"),
  s_get("__get"),
  s_set("__set"),
  s_isset("__isset"),
  s_unset("__unset"),
  s_debugInfo("__debugInfo"),
  s_clone("__clone");

static Func* markNonStatic(Func* meth) {
  // Do not use isStaticInPrologue here, since that uses the
  // AttrRequiresThis flag.
  if (meth && (!meth->isStatic() || meth->isClosureBody() ||
      s_construct.equal(meth->name()) ||
      s_destruct.equal(meth->name()))) {
    meth->setAttrs(meth->attrs() | AttrRequiresThis);
  }
  return meth;
}

static Func* markNonStatic(const Class* thiz, const String& meth) {
  return markNonStatic(thiz->lookupMethod(meth.get()));
}

void Class::setSpecial() {
  m_toString = markNonStatic(this, s_toString);
  m_dtor = markNonStatic(this, s_destruct);

  /*
   * The invoke method is only cached in the Class for a fast path JIT
   * translation.  If someone defines a weird __invoke (e.g. as a
   * static method), we don't bother caching it here so the translated
   * code won't have to check for that case.
   *
   * Note that AttrStatic on a closure's __invoke Func* means it is a
   * static closure---but the call to __invoke still works as if it
   * were a non-static method call---so they are excluded from that
   * here.  (The closure prologue uninstalls the $this and installs
   * the appropriate static context.)
   */
  m_invoke = markNonStatic(this, s_invoke);
  if (m_invoke && m_invoke->isStaticInPrologue()) {
    m_invoke = nullptr;
  }

  auto matchedClassOrIsTrait = [this](const StringData* sd) {
    auto func = lookupMethod(sd);
    if (func && (func->preClass() == m_preClass.get() ||
                 func->preClass()->attrs() & AttrTrait)) {
      m_ctor = func;
      return true;
    }
    return false;
  };

  // Look for __construct() declared in either this class or a trait
  if (matchedClassOrIsTrait(s_construct.get())) {
    auto func = lookupMethod(m_preClass->name());
    if (func && (func->preClass()->attrs() & AttrTrait ||
                 m_ctor->preClass()->attrs() & AttrTrait)) {
      throw Exception(
        "%s has colliding constructor definitions coming from traits",
        m_preClass->name()->data()
      );
    }
    return;
  }

  if (!(attrs() & AttrTrait)) {
    // Look for Foo::Foo() (old style constructor) declared in this class
    // and deprecate warning if we are in PHP 7 mode
    if (matchedClassOrIsTrait(m_preClass->name())) {
      // https://wiki.php.net/rfc/remove_php4_constructors
      // Check for PHP 4 style constructors. For PHP 7 support, in certain
      // scenarios, we throw a deprecation warning and then for PHP 8+ support
      // we will not support them at all.
      // We know we Foo:Foo() since we are in this if statement
      // Now, if the PHP 7 runtime option is set and the class:
      //   1. does not have an explicit constructor with __construct
      //   2. is not in a namespace (namespaced classes treat methods with the
      //      same name as the class as just normal methods, not a constructor)
      // then we give the deprecation warning.
      if (
        RuntimeOption::PHP7_DeprecationWarnings && // In PHP 7 mode
        !this->instanceCtor() && // No explicit __construct
        this->name()->toCppString().find("\\") == std::string::npos // no NS
      ) {
        const char *deprecated_msg =
          "Methods with the same name as their class will not be "
          "constructors in a future version of PHP; %s has a deprecated "
          "constructor";
        raise_deprecated(deprecated_msg, this->name()->toCppString().c_str());
      }
      return;
    }
  }

  // Look for parent constructor other than 86ctor().
  if (m_parent.get() != nullptr && m_parent->m_ctor &&
      m_parent->m_ctor->name() != s_86ctor.get()) {
    m_ctor = m_parent->m_ctor;
    return;
  }

  // No need to generate an 86ctor for an abstract final class
  if ((attrs() & AttrAbstract) && (attrs() & AttrFinal)) {
    m_ctor = nullptr;
    return;
  }

  // Use 86ctor(), since no program-supplied constructor exists
  m_ctor = findSpecialMethod(this, s_86ctor.get());
  m_ctor->setAttrs(m_ctor->attrs() | AttrRequiresThis);
  assert(m_ctor && "class had no user-defined constructor or 86ctor");
  assert((m_ctor->attrs() & ~(AttrBuiltin | AttrAbstract |
                              AttrHot | AttrInterceptable |
                              AttrMayUseVV)) ==
         (AttrPublic|AttrNoInjection|AttrPhpLeafFn|AttrRequiresThis));
}

namespace {

inline void raiseIncompat(const PreClass* implementor,
                          const Func* imeth) {
  const char* name = imeth->name()->data();
  raise_error("Declaration of %s::%s() must be compatible with "
              "that of %s::%s()",
              implementor->name()->data(), name,
              imeth->cls()->preClass()->name()->data(), name);
}

static bool checkTypeConstraint(const PreClass* implCls, const Class* iface,
                                TypeConstraint tc, TypeConstraint itc) {
  const StringData* iSelf;
  const StringData* iParent;
  if (isTrait(iface)) {
    iSelf = implCls->name();
    iParent = implCls->parent();
  } else {
    iSelf = iface->name();
    iParent = iface->parent() ? iface->parent()->name() : nullptr;
  }

  if (tc.isExtended() || itc.isExtended()) return true;

  if (tc.isSelf())     tc = TypeConstraint { implCls->name(), tc.flags() };
  if (tc.isParent())   tc = TypeConstraint { implCls->parent(), tc.flags() };
  if (itc.isSelf())   itc = TypeConstraint { iSelf, itc.flags() };
  if (itc.isParent()) itc = TypeConstraint { iParent, itc.flags() };

  return tc.compat(itc);
}

// Check compatibility vs interface and abstract declarations
void checkDeclarationCompat(const PreClass* preClass,
                            const Func* func, const Func* imeth) {
  bool relaxedCheck = !RuntimeOption::EnableHipHopSyntax
                        && func->isCPPBuiltin()
                        && !imeth->unit()->isHHFile();

  const Func::ParamInfoVec& params = func->params();
  const Func::ParamInfoVec& iparams = imeth->params();

  auto const ivariadic = imeth->hasVariadicCaptureParam();
  if (ivariadic && !func->hasVariadicCaptureParam()) {
    raiseIncompat(preClass, imeth);
  }

  // Verify that meth has at least as many parameters as imeth.
  if (func->numParams() < imeth->numParams()) {
    // This check doesn't require special casing for variadics, because
    // it's not ok to turn a variadic function into a non-variadic.
    raiseIncompat(preClass, imeth);
  }
  // Verify that the typehints for meth's parameters are compatible with
  // imeth's corresponding parameter typehints.
  size_t firstOptional = 0;
  {
    size_t i = 0;
    for (; i < imeth->numNonVariadicParams(); ++i) {
      auto const& p = params[i];
      if (p.isVariadic()) { raiseIncompat(preClass, imeth); }
      auto const& ip = iparams[i];
      if (!relaxedCheck) {
        // If the interface parameter is a type constant we require the
        // implementer to specify a type
        if (!p.userType && ip.typeConstraint.isTypeConstant()) {
          raiseIncompat(preClass, imeth);
        }

        if (!checkTypeConstraint(preClass, imeth->cls(),
                                 p.typeConstraint, ip.typeConstraint)) {
          if (!ip.typeConstraint.isTypeVar() &&
              !ip.typeConstraint.isTypeConstant()) {
            raiseIncompat(preClass, imeth);
          }
        }
      }
      if (!iparams[i].hasDefaultValue()) {
        // The leftmost of imeth's contiguous trailing optional parameters
        // must start somewhere to the right of this parameter (which may
        // be the variadic param)
        firstOptional = i + 1;
      }
    }
    if (ivariadic) {
      assert(iparams[iparams.size() - 1].isVariadic());
      assert(params[params.size() - 1].isVariadic());
      // reffiness of the variadics must match
      if (imeth->byRef(iparams.size() - 1) !=
          func->byRef(params.size() - 1)) {
        raiseIncompat(preClass, imeth);
      }

      // To be compatible with a variadic interface, params from the
      // variadic onwards must have a compatible typehint
      auto const& ivarConstraint = iparams[iparams.size() - 1].typeConstraint;
      if (!ivarConstraint.isTypeVar()) {
        for (; i < func->numParams(); ++i) {
          auto const& p = params[i];
          if (!checkTypeConstraint(preClass, imeth->cls(),
                                   p.typeConstraint, ivarConstraint)) {
            raiseIncompat(preClass, imeth);
          }
        }
      }
    }
  }

  if (!relaxedCheck) {
    // Verify that meth provides defaults, starting with the parameter that
    // corresponds to the leftmost of imeth's contiguous trailing optional
    // parameters and *not* including any variadic last param (variadics
    // don't have any default values).
    for (unsigned i = firstOptional; i < func->numNonVariadicParams(); ++i) {
      if (!params[i].hasDefaultValue()) {
        raiseIncompat(preClass, imeth);
      }
    }
  }
}

} // namespace

Class::Class(PreClass* preClass, Class* parent,
             std::vector<ClassPtr>&& usedTraits,
             unsigned classVecLen, unsigned funcVecLen)
#ifdef DEBUG
  : m_magic{kMagic}
  , m_parent(parent)
#else
  : m_parent(parent)
#endif
  , m_preClass(PreClassPtr(preClass))
  , m_classVecLen(always_safe_cast<decltype(m_classVecLen)>(classVecLen))
  , m_funcVecLen(always_safe_cast<decltype(m_funcVecLen)>(funcVecLen))
{
  if (usedTraits.size()) {
    allocExtraData();
    m_extra.raw()->m_usedTraits = std::move(usedTraits);
  }
  setParent();
  setMethods();
  setSpecial();       // must run before setODAttributes
  setODAttributes();
  setInterfaces();
  setConstants();
  setProperties();
  setInitializers();
  setClassVec();
  setRequirements();
  setNativeDataInfo();
  setEnumType();

  // A class is allowed to implement two interfaces that share the same slot if
  // we'll fatal trying to define that class, so this has to happen after all
  // of those fatals could be thrown.
  setInterfaceVtables();
}

void Class::methodOverrideCheck(const Func* parentMethod, const Func* method) {
  // Skip special methods
  if (method->isGenerated()) return;

  if ((parentMethod->attrs() & AttrFinal)) {
    if (m_preClass->userAttributes().find(s___MockClass.get()) ==
        m_preClass->userAttributes().end()) {
      raise_error("Cannot override final method %s::%s()",
                  m_parent->name()->data(), parentMethod->name()->data());
    }
  }

  if ((method->attrs() & AttrAbstract) && !method->isFromTrait()) {
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
      (baseMethod->attrs() & AttrAbstract)) {
    checkDeclarationCompat(m_preClass.get(), method, baseMethod);
  }
}

void Class::setMethods() {
  std::vector<Slot> parentMethodsWithStaticLocals;
  MethodMapBuilder builder;

  ITRACE(5, "----------\nsetMethods() for {}:\n", this->name()->data());
  if (m_parent.get() != nullptr) {
    // Copy down the parent's method entries. These may be overridden below.
    for (Slot i = 0; i < m_parent->m_methods.size(); ++i) {
      Func* f = m_parent->getMethod(i);
      assert(f);
      ITRACE(5, "  - adding parent method {}\n", f->name()->data());
      if ((f->attrs() & AttrClone) ||
          (!(f->attrs() & AttrPrivate) && f->hasStaticLocals())) {
        // When copying down an entry for a non-private method that has
        // static locals, we want to make a copy of the Func so that it
        // gets a distinct set of static locals variables. We defer making
        // a copy of the parent method until the end because it might get
        // overriden below.
        parentMethodsWithStaticLocals.push_back(i);
      }
      assert(builder.size() == i);
      builder.add(f->name(), f);
    }
  }

  static_assert(AttrPublic < AttrProtected && AttrProtected < AttrPrivate, "");
  // Overlay/append this class's public/protected methods onto/to those of the
  // parent.
  for (size_t methI = 0; methI < m_preClass->numMethods(); ++methI) {
    Func* method = m_preClass->methods()[methI];
    ITRACE(5, "  - processing pre-class method {}\n", method->name()->data());
    if (Func::isSpecial(method->name())) {
      if (method->name() == s_86ctor.get() ||
          method->name() == s_86sinit.get() ||
          method->name() == s_86pinit.get()) {
        /*
         * we could also skip the cinit function here, but
         * that would mean storing it somewhere else.
         */
        continue;
      }
    }

    MethodMapBuilder::iterator it2 = builder.find(method->name());
    if (it2 != builder.end()) {
      Func* parentMethod = builder[it2->second];
      // We should never have null func pointers to deal with
      assert(parentMethod);
      // An abstract method that came from a trait doesn't override another
      // method.
      if (method->isFromTrait() && (method->attrs() & AttrAbstract)) continue;
      methodOverrideCheck(parentMethod, method);
      // Overlay.
      Func* f = method->clone(this);
      f->setNewFuncId();
      Class* baseClass;
      assert(!(f->attrs() & AttrPrivate) ||
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
      Func* f = method->clone(this);
      f->setNewFuncId();
      f->setBaseCls(baseClass);
      f->setHasPrivateAncestor(false);
      builder.add(method->name(), f);
    }
  }

  auto const traitsBeginIdx = builder.size();
  if (m_extra->m_usedTraits.size()) {
    importTraitMethods(builder);
  }
  auto const traitsEndIdx = builder.size();

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
      f = f->clone(f->attrs() & AttrClone ? this : f->cls());
      f->setNewFuncId();
      if (RuntimeOption::EvalPerfDataMap) {
        if (!s_funcIdToClassMap) {
          Lock l(g_classesMutex);
          if (!s_funcIdToClassMap) {
            s_funcIdToClassMap = new FuncIdToClassMap;
          }
        }
        FuncIdToClassMap::accessor acc;
        if (!s_funcIdToClassMap->insert(
              acc, FuncIdToClassMap::value_type(f->getFuncId(), this))) {
          // we only just allocated this id, which is supposedly
          // process unique
          assert(false);
        }
      }
    }
  }

  if (m_extra) {
    m_extra.raw()->m_traitsBeginIdx = traitsBeginIdx;
    m_extra.raw()->m_traitsEndIdx = traitsEndIdx;
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

  // If class is abstract final, its static methods should not be abstract
  if ((attrs() & (AttrAbstract | AttrFinal)) == (AttrAbstract | AttrFinal)) {
    for (Slot i = 0; i < builder.size(); i++) {
      const Func* meth = builder[i];
      if ((meth->attrs() & (AttrAbstract | AttrStatic))
          == (AttrAbstract | AttrStatic)) {
        raise_error(
          "Class %s contains abstract static method (%s) and "
          "therefore cannot be declared 'abstract final'",
          m_preClass->name()->data(), meth->name()->data());
      }
    }
  }

  builder.create(m_methods);
  for (Slot i = 0; i < builder.size(); ++i) {
    builder[i]->setMethodSlot(i);
  }
  setFuncVec(builder);
}

void Class::setODAttributes() {
  m_ODAttrs = 0;
  if (lookupMethod(s_sleep.get()     )) { m_ODAttrs |= ObjectData::HasSleep; }
  if (markNonStatic(this, s_get      )) { m_ODAttrs |= ObjectData::UseGet;   }
  if (markNonStatic(this, s_set      )) { m_ODAttrs |= ObjectData::UseSet;   }
  if (markNonStatic(this, s_isset    )) { m_ODAttrs |= ObjectData::UseIsset; }
  if (markNonStatic(this, s_unset    )) { m_ODAttrs |= ObjectData::UseUnset; }
  if (markNonStatic(this, s_clone    )) { m_ODAttrs |= ObjectData::HasClone; }

  markNonStatic(this, s_call);
  markNonStatic(this, s_debugInfo);
  if (!((attrs() & AttrAbstract) && (attrs() & AttrFinal))) {
    markNonStatic(m_ctor);
  }

  if (m_dtor == nullptr) m_ODAttrs |= ObjectData::NoDestructor;

  if ((isBuiltin() && Native::getNativePropHandler(name())) ||
      (m_parent && m_parent->hasNativePropHandler())) {
    m_ODAttrs |= ObjectData::HasNativePropHandler;
  }
}

void Class::setConstants() {
  ConstMap::Builder builder;

  if (m_parent.get() != nullptr) {
    for (Slot i = 0; i < m_parent->m_constants.size(); ++i) {
      // Copy parent's constants.
      builder.add(m_parent->m_constants[i].name, m_parent->m_constants[i]);
    }
  }

  // Copy in interface constants.
  for (int i = 0, size = m_interfaces.size(); i < size; ++i) {
    const Class* iface = m_interfaces[i];

    for (Slot slot = 0; slot < iface->m_constants.size(); ++slot) {
      auto const iConst = iface->m_constants[slot];

      // If you're inheriting a constant with the same name as an existing
      // one, they must originate from the same place, unless the constant
      // was defined as abstract.
      auto const existing = builder.find(iConst.name);

      if (existing == builder.end()) {
        builder.add(iConst.name, iConst);
        continue;
      }
      auto& existingConst = builder[existing->second];

      if (iConst.isType() != existingConst.isType()) {
        raise_error("%s cannot inherit the %sconstant %s from %s, because it "
                    "was previously inherited as a %sconstant from %s",
                    m_preClass->name()->data(),
                    iConst.isType() ? "type " : "",
                    iConst.name->data(),
                    iConst.cls->name()->data(),
                    iConst.isType() ? "" : "type ",
                    existingConst.cls->name()->data());
      }

      if (iConst.isAbstract()) {
        continue;
      }

      if (existingConst.isAbstract()) {
        existingConst.cls = iConst.cls;
        existingConst.val = iConst.val;
        continue;
      }

      if (existingConst.cls != iConst.cls) {
        // It's only an error if the constant comes from the declared
        // interfaces.
        for (auto const& interface : declInterfaces()) {
          if (interface.get() == iface) {
            raise_error("%s cannot inherit the %sconstant %s from %s, because "
                        "it was previously inherited from %s",
                        m_preClass->name()->data(),
                        iConst.isType() ? "type " : "",
                        iConst.name->data(),
                        iConst.cls->name()->data(),
                        existingConst.cls->name()->data());
          }
        }
      }
    }
  }

  for (Slot i = 0, sz = m_preClass->numConstants(); i < sz; ++i) {
    const PreClass::Const* preConst = &m_preClass->constants()[i];
    ConstMap::Builder::iterator it2 = builder.find(preConst->name());
    if (it2 != builder.end()) {
      auto definingClass = builder[it2->second].cls;
      // Forbid redefining constants from interfaces, but not superclasses.
      // Constants from interfaces implemented by superclasses can be
      // overridden.
      if (definingClass->attrs() & AttrInterface) {
        for (auto interface : declInterfaces()) {
          if (interface->hasConstant(preConst->name()) ||
              interface->hasTypeConstant(preConst->name())) {
            raise_error("Cannot override previously defined %sconstant "
                        "%s::%s in %s",
                        builder[it2->second].isType() ? "type " : "",
                        builder[it2->second].cls->name()->data(),
                        preConst->name()->data(),
                        m_preClass->name()->data());
          }
        }
      }

      if (preConst->isAbstract() &&
          !builder[it2->second].isAbstract()) {
        raise_error("Cannot re-declare as abstract previously defined "
                    "%sconstant %s::%s in %s",
                    builder[it2->second].isType() ? "type " : "",
                    builder[it2->second].cls->name()->data(),
                    preConst->name()->data(),
                    m_preClass->name()->data());
      }

      if (preConst->isType() != builder[it2->second].isType()) {
        raise_error("Cannot re-declare as a %sconstant previously defined "
                    "%sconstant %s::%s in %s",
                    preConst->isType() ? "type " : "",
                    preConst->isType() ? "" : "type ",
                    builder[it2->second].cls->name()->data(),
                    preConst->name()->data(),
                    m_preClass->name()->data());
      }
      builder[it2->second].cls = this;
      builder[it2->second].val = preConst->val();
    } else {
      // Append constant.
      Const constant;
      constant.cls = this;
      constant.name = preConst->name();
      constant.val = preConst->val();
      builder.add(preConst->name(), constant);
    }
  }

  // If class is not abstract, all abstract constants should have been
  // defined
  if (!(attrs() & (AttrTrait | AttrInterface | AttrAbstract))) {
    for (Slot i = 0; i < builder.size(); i++) {
      const Const& constant = builder[i];
      if (constant.isAbstract()) {
        raise_error("Class %s contains abstract %sconstant (%s) and "
                    "must therefore be declared abstract or define "
                    "the remaining constants",
                    m_preClass->name()->data(),
                    constant.isType() ? "type " : "",
                    constant.name->data());
      }
    }
  }

  // If class is abstract final, its constants should not be abstract
  else if (
    (attrs() & (AttrAbstract | AttrFinal)) == (AttrAbstract | AttrFinal)) {
    for (Slot i = 0; i < builder.size(); i++) {
      const Const& constant = builder[i];
      if (constant.isAbstract()) {
        raise_error(
          "Class %s contains abstract %sconstant (%s) and "
          "therefore cannot be declared 'abstract final'",
          m_preClass->name()->data(),
          constant.isType() ? "type " : "",
          constant.name->data());
      }
    }
  }


  // For type constants, we have to use the value from the PreClass of the
  // declaring class, because the parent class or interface we got it from may
  // have replaced it with a resolved value.
  for (auto& pair : builder) {
    auto& cns = builder[pair.second];
    if (cns.isType()) {
      auto& preConsts = cns.cls->preClass()->constantsMap();
      auto const idx = preConsts.findIndex(cns.name.get());
      assert(idx != -1);
      cns.val = preConsts[idx].val();
    }
  }

  m_constants.create(builder);
}

static void copyDeepInitAttr(const PreClass::Prop* pclsProp,
                             Class::Prop* clsProp) {
  if (pclsProp->attrs() & AttrDeepInit) {
    clsProp->attrs = (Attr)(clsProp->attrs | AttrDeepInit);
  } else {
    clsProp->attrs = (Attr)(clsProp->attrs & ~AttrDeepInit);
  }
}

void Class::setProperties() {
  int numInaccessible = 0;
  PropMap::Builder curPropMap;
  SPropMap::Builder curSPropMap;
  m_hasDeepInitProps = false;
  Slot traitOffset = 0;

  if (m_parent.get() != nullptr) {
    // m_hasDeepInitProps indicates if there are properties that require
    // deep initialization. Note there are cases where m_hasDeepInitProps is
    // true but none of the properties require deep initialization; this can
    // happen if a derived class redeclares a public or protected property
    // from an ancestor class. We still get correct behavior in these cases,
    // so it works out okay.
    m_hasDeepInitProps = m_parent->m_hasDeepInitProps;
    for (auto const& parentProp : m_parent->declProperties()) {
      // Copy parent's declared property.  Protected properties may be
      // weakened to public below, but otherwise, the parent's properties
      // will stay the same for this class.
      Prop prop;
      prop.cls                 = parentProp.cls;
      prop.mangledName         = parentProp.mangledName;
      prop.originalMangledName = parentProp.originalMangledName;
      prop.attrs               = parentProp.attrs;
      prop.docComment          = parentProp.docComment;
      prop.typeConstraint      = parentProp.typeConstraint;
      prop.name                = parentProp.name;
      prop.repoAuthType        = parentProp.repoAuthType;
      // Temporarily assign parent properties' indexes to their additive
      // inverses minus one.  After assigning current properties' indexes, we
      // will use these negative indexes to assign new indexes to parent
      // properties that haven't been overlayed.
      prop.idx = -parentProp.idx - 1;
      if (traitOffset < -prop.idx) {
        traitOffset = -prop.idx;
      }
      if (!(parentProp.attrs & AttrPrivate)) {
        curPropMap.add(prop.name, prop);
      } else {
        ++numInaccessible;
        curPropMap.addUnnamed(prop);
      }
    }
    m_declPropInit = m_parent->m_declPropInit;
    for (auto const& parentProp : m_parent->staticProperties()) {
      if (parentProp.attrs & AttrPrivate) continue;

      // Alias parent's static property.
      SProp sProp;
      sProp.name           = parentProp.name;
      sProp.attrs          = parentProp.attrs;
      sProp.typeConstraint = parentProp.typeConstraint;
      sProp.docComment     = parentProp.docComment;
      sProp.cls            = parentProp.cls;
      sProp.idx            = -parentProp.idx - 1;
      if (traitOffset < -sProp.idx) {
        traitOffset = -sProp.idx;
      }
      tvWriteUninit(&sProp.val);
      curSPropMap.add(sProp.name, sProp);
    }
  }

  Slot traitIdx = m_preClass->numProperties();
  if (RuntimeOption::RepoAuthoritative) {
    for (auto const& traitName : m_preClass->usedTraits()) {
      Class* classPtr = Unit::loadClass(traitName);
      traitIdx -= classPtr->m_declProperties.size() +
                  classPtr->m_staticProperties.size();
    }
  }

  static_assert(AttrPublic < AttrProtected && AttrProtected < AttrPrivate, "");
  for (Slot slot = 0; slot < m_preClass->numProperties(); ++slot) {
    const PreClass::Prop* preProp = &m_preClass->properties()[slot];

    if (!(preProp->attrs() & AttrStatic)) {
      // Overlay/append this class's protected and public properties onto/to
      // those of the parent, and append this class's private properties.
      // Append order doesn't matter here (unlike in setMethods()).
      // Prohibit static-->non-static redeclaration.
      SPropMap::Builder::iterator it5 = curSPropMap.find(preProp->name());
      if (it5 != curSPropMap.end()) {
        raise_error("Cannot redeclare static %s::$%s as non-static %s::$%s",
                    curSPropMap[it5->second].cls->name()->data(),
                    preProp->name()->data(), m_preClass->name()->data(),
                    preProp->name()->data());
      }
      // Get parent's equivalent property, if one exists.
      const Prop* parentProp = nullptr;
      if (m_parent.get() != nullptr) {
        Slot id = m_parent->m_declProperties.findIndex(preProp->name());
        if (id != kInvalidSlot) {
          parentProp = &m_parent->m_declProperties[id];
        }
      }
      // Prohibit strengthening.
      if (parentProp
          && (preProp->attrs() & (AttrPublic|AttrProtected|AttrPrivate))
             > (parentProp->attrs & (AttrPublic|AttrProtected|AttrPrivate))) {
        raise_error(
          "Access level to %s::$%s() must be %s (as in class %s) or weaker",
          m_preClass->name()->data(), preProp->name()->data(),
          attrToVisibilityStr(parentProp->attrs),
          m_parent->name()->data());
      }
      if (preProp->attrs() & AttrDeepInit) {
        m_hasDeepInitProps = true;
      }
      auto addNewProp = [&] {
        Prop prop;
        prop.name                = preProp->name();
        prop.mangledName         = preProp->mangledName();
        prop.originalMangledName = preProp->mangledName();
        prop.attrs               = preProp->attrs();
        // This is the first class to declare this property
        prop.cls                 = this;
        prop.typeConstraint      = preProp->typeConstraint();
        prop.docComment          = preProp->docComment();
        prop.repoAuthType        = preProp->repoAuthType();
        if (slot < traitIdx) {
          prop.idx = slot;
        } else {
          prop.idx = slot + m_preClass->numProperties() + traitOffset;
        }
        curPropMap.add(preProp->name(), prop);
        m_declPropInit.push_back(preProp->val());
      };

      switch (preProp->attrs() & (AttrPublic|AttrProtected|AttrPrivate)) {
      case AttrPrivate: {
        addNewProp();
        break;
      }
      case AttrProtected: {
        // Check whether a superclass has already declared this protected
        // property.
        PropMap::Builder::iterator it2 = curPropMap.find(preProp->name());
        if (it2 != curPropMap.end()) {
          auto& prop = curPropMap[it2->second];
          assert((prop.attrs & (AttrPublic|AttrProtected|AttrPrivate)) ==
                 AttrProtected);
          prop.cls = this;
          prop.docComment = preProp->docComment();
          if (slot < traitIdx) {
            prop.idx = slot;
          } else {
            prop.idx = slot + m_preClass->numProperties() + traitOffset;
          }
          const TypedValue& tv = preProp->val();
          TypedValueAux& tvaux = m_declPropInit[it2->second];
          tvaux.m_data = tv.m_data;
          tvaux.m_type = tv.m_type;
          copyDeepInitAttr(preProp, &prop);
          break;
        }
        addNewProp();
        break;
      }
      case AttrPublic: {
        // Check whether a superclass has already declared this as a
        // protected/public property.
        auto it2 = curPropMap.find(preProp->name());
        if (it2 != curPropMap.end()) {
          auto& prop = curPropMap[it2->second];
          prop.cls = this;
          prop.docComment = preProp->docComment();
          if ((prop.attrs & (AttrPublic|AttrProtected|AttrPrivate))
              == AttrProtected) {
            // Weaken protected property to public.
            prop.mangledName = preProp->mangledName();
            prop.originalMangledName = preProp->mangledName();
            prop.attrs = Attr(prop.attrs ^ (AttrProtected|AttrPublic));
            prop.typeConstraint = preProp->typeConstraint();
          }
          if (slot < traitIdx) {
            prop.idx = slot;
          } else {
            prop.idx = slot + m_preClass->numProperties() + traitOffset;
          }
          auto const& tv = preProp->val();
          TypedValueAux& tvaux = m_declPropInit[it2->second];
          tvaux.m_data = tv.m_data;
          tvaux.m_type = tv.m_type;
          copyDeepInitAttr(preProp, &prop);
          break;
        }
        addNewProp();
        break;
      }
      default: assert(false);
      }
    } else { // Static property.
      // Prohibit non-static-->static redeclaration.
      auto const it2 = curPropMap.find(preProp->name());
      if (it2 != curPropMap.end()) {
        auto& prop = curPropMap[it2->second];
        raise_error("Cannot redeclare non-static %s::$%s as static %s::$%s",
                    prop.cls->name()->data(),
                    preProp->name()->data(),
                    m_preClass->name()->data(),
                    preProp->name()->data());
      }
      // Get parent's equivalent property, if one exists.
      auto const it3 = curSPropMap.find(preProp->name());
      Slot sPropInd = kInvalidSlot;
      // Prohibit strengthening.
      if (it3 != curSPropMap.end()) {
        const SProp& parentSProp = curSPropMap[it3->second];
        if ((preProp->attrs() & (AttrPublic|AttrProtected|AttrPrivate))
            > (parentSProp.attrs & (AttrPublic|AttrProtected|AttrPrivate))) {
          raise_error(
            "Access level to %s::$%s() must be %s (as in class %s) or weaker",
            m_preClass->name()->data(), preProp->name()->data(),
            attrToVisibilityStr(parentSProp.attrs),
            m_parent->name()->data());
        }
        sPropInd = it3->second;
      }
      // Create a new property, or overlay ancestor's property if one exists.
      if (sPropInd == kInvalidSlot) {
        SProp sProp;
        sProp.name = preProp->name();
        sPropInd = curSPropMap.size();
        curSPropMap.add(sProp.name, sProp);
      }
      // Finish initializing.
      auto& sProp = curSPropMap[sPropInd];
      sProp.attrs          = preProp->attrs();
      sProp.typeConstraint = preProp->typeConstraint();
      sProp.docComment     = preProp->docComment();
      sProp.cls            = this;
      sProp.val            = preProp->val();
      sProp.repoAuthType   = preProp->repoAuthType();
      if (slot < traitIdx) {
        sProp.idx = slot;
      } else {
        sProp.idx = slot + m_preClass->numProperties() + traitOffset;
      }
    }
  }

  // After assigning indexes for current properties, we reassign indexes to
  // parent properties that haven't been overlayed to make sure that they
  // are greater than those of current properties.
  int idxOffset = m_preClass->numProperties() - 1;
  int curIdx = idxOffset;
  for (Slot slot = 0; slot < curPropMap.size(); ++slot) {
    auto& prop = curPropMap[slot];
    if (prop.idx < 0) {
      prop.idx = idxOffset - prop.idx;
      if (curIdx < prop.idx) {
        curIdx = prop.idx;
      }
    }
  }
  for (Slot slot = 0; slot < curSPropMap.size(); ++slot) {
    auto& sProp = curSPropMap[slot];
    if (sProp.idx < 0) {
      sProp.idx = idxOffset - sProp.idx;
      if (curIdx < sProp.idx) {
        curIdx = sProp.idx;
      }
    }
  }

  importTraitProps(curIdx + 1, curPropMap, curSPropMap);

  m_declProperties.create(curPropMap);
  m_staticProperties.create(curSPropMap);

  m_sPropCache = (rds::Link<StaticPropData>*)
    malloc_huge(numStaticProperties() * sizeof(*m_sPropCache));
  for (unsigned i = 0, n = numStaticProperties(); i < n; ++i) {
    new (&m_sPropCache[i]) rds::Link<StaticPropData>(rds::kInvalidHandle);
  }

  m_declPropNumAccessible = m_declProperties.size() - numInaccessible;
}

bool Class::compatibleTraitPropInit(TypedValue& tv1, TypedValue& tv2) {
  if (tv1.m_type != tv2.m_type) return false;

  switch (tv1.m_type) {
    case KindOfNull:
      return true;

    case KindOfBoolean:
    case KindOfInt64:
    case KindOfDouble:
    case KindOfPersistentString:
    case KindOfString:
      return same(tvAsVariant(&tv1), tvAsVariant(&tv2));

    case KindOfUninit:
    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfPersistentArray:
    case KindOfArray:
    case KindOfObject:
    case KindOfResource:
    case KindOfRef:
      return false;
  }
  not_reached();
}

void Class::importTraitInstanceProp(Class* /*trait*/, Prop& traitProp,
                                    TypedValue& traitPropVal,
                                    const int idxOffset,
                                    PropMap::Builder& curPropMap) {
  auto prevIt = curPropMap.find(traitProp.name);

  if (prevIt == curPropMap.end()) {
    // New prop, go ahead and add it
    Prop prop = traitProp;
    prop.cls = this; // set current class as the first declaring prop
    // private props' mangled names contain the class name, so regenerate them
    if (prop.attrs & AttrPrivate) {
      prop.mangledName = PreClass::manglePropName(m_preClass->name(),
                                                  prop.name,
                                                  prop.attrs);
    }
    if (prop.attrs & AttrDeepInit) {
      m_hasDeepInitProps = true;
    }
    prop.idx += idxOffset;
    curPropMap.add(prop.name, prop);
    m_declPropInit.push_back(traitPropVal);
  } else {
    // Redeclared prop, make sure it matches previous declarations
    auto& prevProp    = curPropMap[prevIt->second];
    auto& prevPropVal = m_declPropInit[prevIt->second];
    if (prevProp.attrs != traitProp.attrs ||
        !compatibleTraitPropInit(prevPropVal, traitPropVal)) {
      raise_error("trait declaration of property '%s' is incompatible with "
                    "previous declaration", traitProp.name->data());
    }
  }
}

void Class::importTraitStaticProp(Class* /*trait*/, SProp& traitProp,
                                  const int idxOffset,
                                  PropMap::Builder& curPropMap,
                                  SPropMap::Builder& curSPropMap) {
  // Check if prop already declared as non-static
  if (curPropMap.find(traitProp.name) != curPropMap.end()) {
    raise_error("trait declaration of property '%s' is incompatible with "
                "previous declaration", traitProp.name->data());
  }

  auto prevIt = curSPropMap.find(traitProp.name);
  if (prevIt == curSPropMap.end()) {
    // New prop, go ahead and add it
    SProp prop = traitProp;
    prop.cls = this; // set current class as the first declaring prop
    prop.idx += idxOffset;
    curSPropMap.add(prop.name, prop);
  } else {
    // Redeclared prop, make sure it matches previous declaration
    auto& prevProp = curSPropMap[prevIt->second];
    TypedValue prevPropVal;
    if (prevProp.cls == this) {
      // If this static property was declared by this class, we can get the
      // initial value directly from its value.
      prevPropVal = prevProp.val;
    } else {
      // If this static property was declared in a parent class, its value will
      // be KindOfUninit, and we'll need to consult the appropriate parent class
      // to get the initial value.
      auto const& prevSProps = prevProp.cls->m_staticProperties;

      auto prevPropInd = prevSProps.findIndex(prevProp.name);
      assert(prevPropInd != kInvalidSlot);

      prevPropVal = prevSProps[prevPropInd].val;
    }
    if (prevProp.attrs != traitProp.attrs ||
        !compatibleTraitPropInit(traitProp.val, prevPropVal)) {
      raise_error("trait declaration of property '%s' is incompatible with "
                  "previous declaration", traitProp.name->data());
    }
    prevProp.cls = this;
    prevProp.val = prevPropVal;
  }
}

void Class::importTraitProps(int idxOffset,
                             PropMap::Builder& curPropMap,
                             SPropMap::Builder& curSPropMap) {
  if (attrs() & AttrNoExpandTrait) return;
  for (auto const& t : m_extra->m_usedTraits) {
    auto trait = t.get();

    // instance properties
    for (Slot p = 0; p < trait->m_declProperties.size(); p++) {
      auto& traitProp    = trait->m_declProperties[p];
      auto& traitPropVal = trait->m_declPropInit[p];
      importTraitInstanceProp(trait, traitProp, traitPropVal, idxOffset,
                              curPropMap);
    }

    // static properties
    for (Slot p = 0; p < trait->m_staticProperties.size(); ++p) {
      auto& traitProp = trait->m_staticProperties[p];
      importTraitStaticProp(trait, traitProp, idxOffset, curPropMap,
                            curSPropMap);
    }

    idxOffset += trait->m_declProperties.size() +
                 trait->m_staticProperties.size();
  }
}

void Class::addTraitPropInitializers(std::vector<const Func*>& thisInitVec,
                                     bool staticProps) {
  if (attrs() & AttrNoExpandTrait) return;
  for (auto const& t : m_extra->m_usedTraits) {
    Class* trait = t.get();
    auto& traitInitVec = staticProps ? trait->m_sinitVec : trait->m_pinitVec;
    // Insert trait's 86[ps]init into the current class, avoiding repetitions.
    for (unsigned m = 0; m < traitInitVec.size(); m++) {
      // Clone 86[ps]init methods, and set the class to the current class.
      // This allows 86[ps]init to determine the property offset for the
      // initializer array corectly.
      Func *f = traitInitVec[m]->clone(this);
      f->setNewFuncId();
      f->setBaseCls(this);
      f->setHasPrivateAncestor(false);
      thisInitVec.push_back(f);
    }
  }
}

namespace {
  const StaticString s_Error("__SystemLib\\Error");
  const StaticString s_Exception("Exception");
}

void Class::setInitializers() {
  std::vector<const Func*> pinits;
  std::vector<const Func*> sinits;

  if (m_parent.get() != nullptr) {
    // Copy parent's 86pinit() vector, so that the 86pinit() methods can be
    // called in reverse order without any search/recursion during
    // initialization.
    pinits.assign(m_parent->m_pinitVec.begin(), m_parent->m_pinitVec.end());
  }

  // This class only has a __[ps]init() method if it's needed.  Append to the
  // vectors of __[ps]init() methods, so that reverse iteration of the vectors
  // runs this class's __[ps]init() first, in case multiple classes in the
  // hierarchy initialize the same property.
  const Func* meth86pinit = findSpecialMethod(this, s_86pinit.get());
  if (meth86pinit != nullptr) {
    pinits.push_back(meth86pinit);
  }
  addTraitPropInitializers(pinits, false);
  const Func* sinit = findSpecialMethod(this, s_86sinit.get());
  if (sinit) {
    sinits.push_back(sinit);
  }
  addTraitPropInitializers(sinits, true);

  m_pinitVec = pinits;
  m_sinitVec = sinits;

  m_needInitialization = (m_pinitVec.size() > 0 ||
    m_staticProperties.size() > 0);

  // Implementations of Throwable get special treatment.
  if (m_parent.get() != nullptr) {
    m_needsInitThrowable = m_parent->needsInitThrowable();
  } else {
    m_needsInitThrowable = name()->same(s_Exception.get()) ||
                           name()->same(s_Error.get());
  }
}

void Class::checkInterfaceConstraints() {
  if (UNLIKELY(m_interfaces.contains(String("Iterator").get()) &&
      m_interfaces.contains(String("IteratorAggregate").get()))) {
    raise_error("Class %s cannot implement both IteratorAggregate and Iterator"
                " at the same time", name()->data());
  }
}

// Checks if interface methods are OK:
//  - there's no requirement if this is a trait, interface, or abstract class
//  - a non-abstract class must implement all methods from interfaces it
//    declares to implement (either directly or indirectly), arity must be
//    compatible (at least as many parameters, additional parameters must have
//    defaults), and typehints must be compatible
void Class::checkInterfaceMethods() {
  for (int i = 0, size = m_interfaces.size(); i < size; i++) {
    const Class* iface = m_interfaces[i];

    for (size_t m = 0; m < iface->m_methods.size(); m++) {
      Func* imeth = iface->getMethod(m);
      const StringData* methName = imeth->name();

      // Skip special methods
      if (Func::isSpecial(methName)) continue;

      Func* meth = lookupMethod(methName);

      if (attrs() & (AttrTrait | AttrInterface | AttrAbstract)) {
        if (meth == nullptr) {
          // Skip unimplemented method.
          continue;
        }
      } else {
        // Verify that method is not abstract within concrete class.
        if (meth == nullptr || (meth->attrs() & AttrAbstract)) {
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
      checkDeclarationCompat(m_preClass.get(), meth, imeth);
    }
  }
}

/*
 * Look up the interfaces implemented by traits used by the class, and add them
 * to the provided builder.
 */
void Class::addInterfacesFromUsedTraits(InterfaceMap::Builder& builder) const {

  for (auto const& traitName : m_preClass->usedTraits()) {
    auto const trait = Unit::lookupClass(traitName);
    assert(trait->attrs() & AttrTrait);
    int numIfcs = trait->m_interfaces.size();

    for (int i = 0; i < numIfcs; i++) {
      auto interface = trait->m_interfaces[i];
      if (builder.find(interface->name()) == builder.end()) {
        builder.add(interface->name(), interface);
      }
    }
  }
}

const StaticString s_Stringish("Stringish");

void Class::setInterfaces() {
  InterfaceMap::Builder interfacesBuilder;
  if (m_parent.get() != nullptr) {
    int size = m_parent->m_interfaces.size();
    for (int i = 0; i < size; i++) {
      auto interface = m_parent->m_interfaces[i];
      interfacesBuilder.add(interface->name(), interface);
    }
  }

  std::vector<ClassPtr> declInterfaces;

  for (auto it = m_preClass->interfaces().begin();
       it != m_preClass->interfaces().end(); ++it) {
    auto cp = Unit::loadClass(*it);
    if (cp == nullptr) {
      raise_error("Undefined interface: %s", (*it)->data());
    }
    if (!(cp->attrs() & AttrInterface)) {
      raise_error("%s cannot implement %s - it is not an interface",
                  m_preClass->name()->data(), cp->name()->data());
    }
    declInterfaces.push_back(ClassPtr(cp));
    if (interfacesBuilder.find(cp->name()) == interfacesBuilder.end()) {
      interfacesBuilder.add(cp->name(), LowPtr<Class>(cp));
    }
    int size = cp->m_interfaces.size();
    for (int i = 0; i < size; i++) {
      auto interface = cp->m_interfaces[i];
      interfacesBuilder.find(interface->name());
      if (interfacesBuilder.find(interface->name()) ==
          interfacesBuilder.end()) {
        interfacesBuilder.add(interface->name(), interface);
      }
    }
  }

  m_numDeclInterfaces = declInterfaces.size();
  m_declInterfaces.reset(new ClassPtr[declInterfaces.size()]);
  std::copy(std::begin(declInterfaces),
            std::end(declInterfaces),
            m_declInterfaces.get());

  addInterfacesFromUsedTraits(interfacesBuilder);

  if (m_toString) {
    auto const present = interfacesBuilder.find(s_Stringish.get());
    if (present == interfacesBuilder.end()
        && (!(attrs() & AttrInterface) ||
            !m_preClass->name()->isame(s_Stringish.get()))) {
      Class* stringish = Unit::lookupClass(s_Stringish.get());
      assert(stringish != nullptr);
      assert((stringish->attrs() & AttrInterface));
      interfacesBuilder.add(stringish->name(), LowPtr<Class>(stringish));
    }
  }

  m_interfaces.create(interfacesBuilder);
  checkInterfaceConstraints();
  checkInterfaceMethods();
}

void Class::setInterfaceVtables() {
  // We only need to set interface vtables for classes that can be instantiated
  // and implement more than 0 interfaces.
  if (!RuntimeOption::RepoAuthoritative ||
      !isNormalClass(this) || isAbstract(this) || m_interfaces.empty()) return;

  size_t totalMethods = 0;
  Slot maxSlot = 0;
  for (auto iface : m_interfaces.range()) {
    auto const slot = iface->preClass()->ifaceVtableSlot();
    if (slot == kInvalidSlot) continue;

    maxSlot = std::max(maxSlot, slot);
    totalMethods += iface->numMethods();
  }

  const size_t nVtables = maxSlot + 1;
  auto const vtableVecSz = nVtables * sizeof(VtableVecSlot);
  auto const memSz = vtableVecSz + totalMethods * sizeof(LowPtr<Func>);
  auto const mem = static_cast<char*>(low_malloc_data(memSz));
  auto cursor = mem;

  ITRACE(3, "Setting interface vtables for class {}. "
         "{} interfaces, {} vtable slots, {} total methods\n",
         name()->data(), m_interfaces.size(), nVtables, totalMethods);
  Trace::Indent indent;

  auto const vtableVec = reinterpret_cast<VtableVecSlot*>(cursor);
  cursor += vtableVecSz;
  m_vtableVecLen = always_safe_cast<decltype(m_vtableVecLen)>(nVtables);
  m_vtableVec = vtableVec;
  memset(vtableVec, 0, vtableVecSz);

  for (auto iface : m_interfaces.range()) {
    auto const slot = iface->preClass()->ifaceVtableSlot();
    if (slot == kInvalidSlot) continue;
    ITRACE(3, "{} @ slot {}\n", iface->name()->data(), slot);
    Trace::Indent indent2;
    always_assert(slot < nVtables);

    auto const nMethods = iface->numMethods();
    auto const vtable = reinterpret_cast<LowPtr<Func>*>(cursor);
    cursor += nMethods * sizeof(LowPtr<Func>);
    always_assert(vtableVec[slot].vtable == nullptr);
    vtableVec[slot].vtable = vtable;
    vtableVec[slot].iface = iface;

    for (size_t i = 0; i < nMethods; ++i) {
      auto ifunc = iface->getMethod(i);
      auto func = lookupMethod(ifunc->name());
      ITRACE(3, "{}:{} @ slot {}\n", ifunc->name()->data(), func, i);
      always_assert(func || Func::isSpecial(ifunc->name()));
      vtable[i] = func;
    }
  }

  always_assert(cursor == mem + memSz);
}

void Class::setRequirements() {
  RequirementMap::Builder reqBuilder;

  auto addReq = [&](const PreClass::ClassRequirement* req) {
    auto it = reqBuilder.find(req->name());
    if (it == reqBuilder.end()) {
      reqBuilder.add(req->name(), req);
      return;
    }
    if (!reqBuilder[it->second]->is_same(req)) {
      raise_error("Conflicting requirements for '%s'",
                  req->name()->data());
    }
  };

  if (m_parent.get() != nullptr) {
    for (auto const& req : m_parent->allRequirements().range()) {
      // no need for addReq; parent won't have duplicates
      reqBuilder.add(req->name(), req);
    }
  }
  for (auto const& iface : m_interfaces.range()) {
    for (auto const& req : iface->allRequirements().range()) {
      addReq(req);
    }
  }
  for (auto const& ut : m_extra->m_usedTraits) {
    for (auto const& req : ut->allRequirements().range()) {
      addReq(req);
    }
  }

  if (attrs() & AttrInterface) {
    // Check that requirements are semantically valid
    // We don't require the class to exist yet, to avoid creating
    // pointless circular dependencies, but if it does, we check
    // that it's the right kind.
    for (auto const& req : m_preClass->requirements()) {
      assert(req.is_extends());
      auto const reqName = req.name();
      auto const reqCls = Unit::lookupClass(reqName);
      if (reqCls) {
        if (reqCls->attrs() & (AttrTrait | AttrInterface | AttrFinal)) {
          raise_error("Interface '%s' requires extension of '%s', but %s "
                      "is not an extendable class",
                      m_preClass->name()->data(),
                      reqName->data(),
                      reqName->data());
        }
      }
      addReq(&req);
    }
  } else if (attrs() & AttrTrait || RuntimeOption::RepoAuthoritative) {
    // Check that requirements are semantically valid.
    // Note that in repo-auth mode, trait flatening could have migrated
    // requirements onto a class's preClass.
    for (auto const& req : m_preClass->requirements()) {
      auto const reqName = req.name();
      if (auto const reqCls = Unit::lookupClass(reqName)) {
        if (req.is_extends()) {
          if (reqCls->attrs() & (AttrTrait | AttrInterface | AttrFinal)) {
            raise_error(Strings::TRAIT_BAD_REQ_EXTENDS,
                        m_preClass->name()->data(),
                        reqName->data(),
                        reqName->data());
          }
        } else {
          assert(req.is_implements());
          if (!(reqCls->attrs() & AttrInterface)) {
            raise_error(Strings::TRAIT_BAD_REQ_IMPLEMENTS,
                        m_preClass->name()->data(),
                        reqName->data(),
                        reqName->data());
          }
        }
      }
      addReq(&req);
    }
  }

  m_requirements.create(reqBuilder);
  checkRequirementConstraints();
}

void Class::setEnumType() {
  if (attrs() & AttrEnum) {
    m_enumBaseTy = m_preClass->enumBaseTy().underlyingDataTypeResolved();

    // Make sure we've loaded a valid underlying type.
    if (m_enumBaseTy &&
        !isIntType(*m_enumBaseTy) &&
        !isStringType(*m_enumBaseTy)) {
      raise_error("Invalid base type for enum %s",
                  m_preClass->name()->data());
    }
  }
}

void Class::setNativeDataInfo() {
  for (auto cls = this; cls; cls = cls->parent()) {
    if (auto ndi = cls->preClass()->nativeDataInfo()) {
      allocExtraData();
      m_extra.raw()->m_nativeDataInfo = ndi;
      m_extra.raw()->m_instanceCtor = Native::nativeDataInstanceCtor;
      m_extra.raw()->m_instanceDtor = Native::nativeDataInstanceDtor;
      break;
    }
  }
}

bool Class::hasNativePropHandler() const {
  return m_ODAttrs & ObjectData::HasNativePropHandler;
}

const Native::NativePropHandler* Class::getNativePropHandler() const {
  assert(hasNativePropHandler());

  for (auto cls = this; cls; cls = cls->parent()) {
    auto propHandler = Native::getNativePropHandler(cls->name());
    if (propHandler != nullptr) {
      return propHandler;
    }
  }

  not_reached();
}

void Class::raiseUnsatisfiedRequirement(const PreClass::ClassRequirement* req)  const {
  assert(!(attrs() & (AttrInterface | AttrTrait)));

  auto const reqName = req->name();
  if (req->is_implements()) {
    // "require implements" is only allowed on traits.

    assert(RuntimeOption::RepoAuthoritative ||
           (m_extra && m_extra->m_usedTraits.size() > 0));
    for (auto const& traitCls : m_extra->m_usedTraits) {
      if (traitCls->allRequirements().contains(reqName)) {
        raise_error(Strings::TRAIT_REQ_IMPLEMENTS,
                    m_preClass->name()->data(),
                    reqName->data(),
                    traitCls->preClass()->name()->data());
      }
    }

    if (RuntimeOption::RepoAuthoritative) {
      // As a result of trait flattening, the PreClass of this normal class
      // contains a requirement. To save space, we don't include the source
      // trait in the requirement. For details, see
      // ClassScope::importUsedTraits in the compiler.
      assert(!m_extra || m_extra->m_usedTraits.size() == 0);
      assert(m_preClass->requirements().size() > 0);
      raise_error(Strings::TRAIT_REQ_IMPLEMENTS,
                  m_preClass->name()->data(),
                  reqName->data(),
                  "<<flattened>>");
    }

    always_assert(false); // requirements cannot spontaneously generate
    return;
  }

  assert(req->is_extends());
  for (auto const& iface : m_interfaces.range()) {
    if (iface->allRequirements().contains(reqName)) {
      raise_error("Class '%s' required to extend class '%s'"
                  " by interface '%s'",
                  m_preClass->name()->data(),
                  reqName->data(),
                  iface->preClass()->name()->data());
    }
  }

  for (auto const& traitCls : m_extra->m_usedTraits) {
    if (traitCls->allRequirements().contains(reqName)) {
      raise_error(Strings::TRAIT_REQ_EXTENDS,
                  m_preClass->name()->data(),
                  reqName->data(),
                  traitCls->preClass()->name()->data());
    }
  }

  if (RuntimeOption::RepoAuthoritative) {
    // A result of trait flattening, as with the is_implements case above
    assert(!m_extra || m_extra->m_usedTraits.size() == 0);
    assert(m_preClass->requirements().size() > 0);
    raise_error(Strings::TRAIT_REQ_EXTENDS,
                m_preClass->name()->data(),
                reqName->data(),
                "<<flattened>>");
  }

  // calls to this method are expected to come as a result of an error due
  // to a requirement coming from traits or interfaces
  always_assert(false);
}

void Class::checkRequirementConstraints() const {
  if (attrs() & (AttrInterface | AttrTrait)) return;

  for (auto const& req : m_requirements.range()) {
    auto const reqName = req->name();
    if (req->is_implements()) {
      if (UNLIKELY(!ifaceofDirect(reqName))) {
        raiseUnsatisfiedRequirement(req);
      }
    } else {
      auto reqExtCls = Unit::lookupClass(reqName);
      if (UNLIKELY(
            (reqExtCls == nullptr) ||
            (reqExtCls->attrs() & (AttrTrait | AttrInterface)))) {
        raiseUnsatisfiedRequirement(req);
      }

      if (UNLIKELY(!classofNonIFace(reqExtCls))) {
        raiseUnsatisfiedRequirement(req);
      }
    }
  }
}

void Class::setClassVec() {
  if (m_classVecLen > 1) {
    assert(m_parent.get() != nullptr);
    memcpy(m_classVec, m_parent->m_classVec,
           (m_classVecLen-1) * sizeof(m_classVec[0]));
  }
  m_classVec[m_classVecLen-1] = this;
}

void Class::setFuncVec(MethodMapBuilder& builder) {
  auto funcVec = this->funcVec();

  memset(funcVec, 0, m_funcVecLen * sizeof(LowPtr<Func>));

  funcVec = (LowPtr<Func>*)this;
  assert(builder.size() <= m_funcVecLen);

  for (Slot i = 0; i < builder.size(); i++) {
    assert(builder[i]->methodSlot() < builder.size());
    funcVec[-((int32_t)builder[i]->methodSlot() + 1)] = builder[i];
  }
}

void Class::getMethodNames(const Class* cls,
                           const Class* ctx,
                           Array& out) {

  // The order of these methods is so that the first ones win on
  // case insensitive name conflicts.

  auto const numMethods = cls->numMethods();

  for (Slot i = 0; i < numMethods; ++i) {
    auto const meth = cls->getMethod(i);
    auto const declCls = meth->cls();
    auto addMeth = [&]() {
      auto const methName = Variant(meth->name(), Variant::PersistentStrInit{});
      auto const lowerName = f_strtolower(methName.toString());
      if (!out.exists(lowerName)) {
        out.add(lowerName, methName);
      }
    };

    // Only pick methods declared in this class, in order to match
    // Zend's order.  Inherited methods will be inserted in the
    // recursive call later.
    if (declCls != cls) continue;

    // Skip generated, internal methods.
    if (meth->isGenerated()) continue;

    // Public methods are always visible.
    if ((meth->attrs() & AttrPublic)) {
      addMeth();
      continue;
    }

    // In anonymous contexts, only public methods are visible.
    if (!ctx) continue;

    // All methods are visible if the context is the class that
    // declared them.  If the context is not the declCls, protected
    // methods are visible in context classes related the declCls.
    if (declCls == ctx ||
        ((meth->attrs() & AttrProtected) &&
         (ctx->classof(declCls) || declCls->classof(ctx)))) {
      addMeth();
    }
  }

  // Now add the inherited methods.
  if (auto const parent = cls->parent()) {
    getMethodNames(parent, ctx, out);
  }

  // Add interface methods that the class may not have implemented yet.
  for (auto& iface : cls->declInterfaces()) {
    getMethodNames(iface.get(), ctx, out);
  }
}


///////////////////////////////////////////////////////////////////////////////
// Trait method import.

bool Class::TMIOps::exclude(const StringData* methName) {
  return Func::isSpecial(methName);
}

void Class::TMIOps::addTraitAlias(Class* cls,
                                  Class::TMIOps::alias_type rule,
                                  const Class* traitCls) {
  PreClass::TraitAliasRule newRule { traitCls->name(),
                                     rule.origMethodName(),
                                     rule.newMethodName(),
                                     rule.modifiers() };
  cls->allocExtraData();
  cls->m_extra.raw()->m_traitAliases.push_back(newRule.asNamePair());
}

const Class*
Class::TMIOps::findSingleTraitWithMethod(const Class* cls,
                                         const StringData* methName) {
  Class* traitCls = nullptr;

  for (auto const& t : cls->m_extra->m_usedTraits) {
    // Note: m_methods includes methods from parents/traits recursively.
    if (t->m_methods.find(methName)) {
      if (traitCls != nullptr) {
        raise_error("more than one trait contains method '%s'",
                    methName->data());
      }
      traitCls = t.get();
    }
  }
  return traitCls;
}

const Class* Class::TMIOps::findTraitClass(const Class* /*cls*/,
                                           const StringData* traitName) {
  return Unit::loadClass(traitName);
}

void Class::applyTraitRules(TMIData& tmid) {
  for (auto const& precRule : m_preClass->traitPrecRules()) {
    tmid.applyPrecRule(precRule, this);
  }
  for (auto const& aliasRule : m_preClass->traitAliasRules()) {
    tmid.applyAliasRule(aliasRule, this);
  }
}

void Class::importTraitMethod(const TMIData::MethodData& mdata,
                              MethodMapBuilder& builder) {
  const Func* method = mdata.tm.method;
  Attr modifiers = mdata.tm.modifiers;

  auto mm_iter = builder.find(mdata.name);

  ITRACE(5, "  - processing trait method {}\n", method->name()->data());

  // For abstract methods, simply return if method already declared.
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

  Func* parentMethod = nullptr;
  if (mm_iter != builder.end()) {
    Func* existingMethod = builder[mm_iter->second];
    if (existingMethod->cls() == this) {
      // Don't override an existing method if this class provided an
      // implementation
      return;
    }
    parentMethod = existingMethod;
  }
  Func* f = method->clone(this, mdata.name);
  f->setNewFuncId();
  f->setAttrs(modifiers | AttrTrait);
  if (!parentMethod) {
    // New method
    builder.add(mdata.name, f);
    f->setBaseCls(this);
    f->setHasPrivateAncestor(false);
  } else {
    // Override an existing method
    Class* baseClass;

    methodOverrideCheck(parentMethod, f);

    assert(!(f->attrs() & AttrPrivate) ||
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

void Class::importTraitMethods(MethodMapBuilder& builder) {
  TMIData tmid;

  // Find all methods to be imported.
  for (auto const& t : m_extra->m_usedTraits) {
    Class* trait = t.get();
    for (Slot i = 0; i < trait->m_methods.size(); ++i) {
      Func* method = trait->getMethod(i);
      const StringData* methName = method->name();

      TraitMethod traitMethod { trait, method, method->attrs() };
      tmid.add(traitMethod, methName);
    }
  }

  // Apply trait rules and import the methods.
  applyTraitRules(tmid);
  auto traitMethods = tmid.finish(this);

  // Import the methods.
  for (auto const& mdata : traitMethods) {
    importTraitMethod(mdata, builder);
  }
}

///////////////////////////////////////////////////////////////////////////////
}
