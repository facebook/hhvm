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
#include "hphp/runtime/base/autoload-handler.h"
#include "hphp/runtime/base/collections.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/enum-cache.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/strings.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/base/type-structure.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/server/memory-stats.h"
#include "hphp/runtime/vm/frame-restore.h"
#include "hphp/runtime/vm/jit/irgen-minstr.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/instance-bits.h"
#include "hphp/runtime/vm/memo-cache.h"
#include "hphp/runtime/vm/named-entity.h"
#include "hphp/runtime/vm/named-entity-defs.h"
#include "hphp/runtime/vm/native-data.h"
#include "hphp/runtime/vm/native-prop-handler.h"
#include "hphp/runtime/vm/property-profile.h"
#include "hphp/runtime/vm/reified-generics.h"
#include "hphp/runtime/vm/reverse-data-map.h"
#include "hphp/runtime/vm/trait-method-import-data.h"
#include "hphp/runtime/vm/treadmill.h"
#include "hphp/runtime/vm/unit-util.h"
#include "hphp/runtime/vm/vm-regs.h"

#include "hphp/runtime/ext/collections/ext_collections.h"
#include "hphp/runtime/ext/string/ext_string.h"
#include "hphp/runtime/ext/std/ext_std_closure.h"

#include "hphp/util/logger.h"

#include <folly/Bits.h>
#include <folly/MapUtil.h>
#include <folly/Optional.h>

#include <algorithm>
#include <iostream>

TRACE_SET_MOD(class_load);

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

const StaticString s_86cinit("86cinit");
const StaticString s_86pinit("86pinit");
const StaticString s_86sinit("86sinit");
const StaticString s_86linit("86linit");
const StaticString s_86ctor("86ctor");
const StaticString s_86metadata("86metadata");
const StaticString s_86reified_prop("86reified_prop");
const StaticString s_86reifiedinit("86reifiedinit");
const StaticString s___MockClass("__MockClass");
const StaticString s___Reified("__Reified");

Mutex g_classesMutex;

///////////////////////////////////////////////////////////////////////////////
// Class::PropInitVec.

Class::PropInitVec::~PropInitVec() {
  if (m_capacity > 0) {
    // allocated in low heap
    lower_free(m_data);
  }
}

Class::PropInitVec*
Class::PropInitVec::allocWithReqAllocator(const PropInitVec& src) {
  uint32_t size = src.m_size;
  auto const props_size = ObjectProps::sizeFor(size);
  auto const bits_size = BitsetView<true>::sizeFor(size);

  PropInitVec* p = reinterpret_cast<PropInitVec*>(req::malloc(
    sizeof(PropInitVec) + props_size + bits_size,
    type_scan::getIndexForMalloc<
      PropInitVec,
      type_scan::Action::WithSuffix<char>
    >()
  ));

  p->m_size = size;
  p->m_capacity = ~size;
  p->m_data = reinterpret_cast<ObjectProps*>(p + 1);

  // these are two distinct memcpys because the props and deep init bits
  // aren't necessarily contiguous in src (because capacity >= size)
  // but will be in the destination (which is exactly sized)
  memcpy16_inline(p->m_data, src.m_data, props_size);
  memcpy(reinterpret_cast<char*>(p->m_data) +
         ObjectProps::sizeFor(size),
         src.deepInitBits().buffer(),
         bits_size);

  return p;
}

const Class::PropInitVec&
Class::PropInitVec::operator=(const PropInitVec& piv) {
  assertx(!reqAllocated());
  if (this != &piv) {
    if (UNLIKELY(m_capacity)) {
      vm_free(m_data);
      m_data = nullptr;
    }
    m_size = m_capacity = piv.size();
    auto const props_size = ObjectProps::sizeFor(m_size);
    auto const bits_size = BitsetView<true>::sizeFor(m_size);
    if (m_size == 0) return *this;

    m_data = reinterpret_cast<ObjectProps*>(
      lower_malloc(props_size + bits_size)
    );
    // these are two distinct memcpys because the props and deep init bits
    // aren't necessarily contiguous in src (because capacity >= size)
    // but will be in the destination (which is exactly sized)
    memcpy16_inline(m_data, piv.m_data, props_size);
    memcpy(deepInitBits().buffer(),
           piv.deepInitBits().buffer(),
           bits_size);

    assertx(m_data);
    assertx(m_data->checkInvariants(m_capacity));
  }
  return *this;
}

void Class::PropInitVec::push_back(const TypedValue& v) {
  assertx(!reqAllocated());
  if (m_size == m_capacity) {
    unsigned newCap = folly::nextPowTwo(m_size + 1);
    auto const props_size = ObjectProps::sizeFor(newCap);
    auto const bits_size = BitsetView<true>::sizeFor(newCap);

    auto newData = reinterpret_cast<char*>(
      lower_malloc(props_size + bits_size)
    );

    auto const oldSize = ObjectProps::sizeFor(m_size);

    // TODO(jgriego) this is a kludge to preserve invariants of
    // ObjectProps (when using 7up layouts) and should be replaced
    // with some kind of `grow` fn on the layout class
    memset(newData + oldSize, 0, props_size - oldSize);

    if (m_data) {
      // these two memcpys are separate because we're going from
      // contiguous memory (since the size == capacity) to noncontiguous memory
      // (because the structure has grown)
      memcpy16_inline(newData, m_data, oldSize);
      memcpy(newData + props_size,
             deepInitBits().buffer(),
             (m_size + 7) / 8);
      lower_free(m_data);
    }

    m_data = reinterpret_cast<ObjectProps*>(newData);
    m_capacity = static_cast<int32_t>(newCap);

    assertx(m_data);
    assertx(m_data->checkInvariants(m_capacity));
  }
  auto const idx = m_size++;
  tvDup(v, m_data->at(idx));
  deepInitBits()[idx] = false;
}


///////////////////////////////////////////////////////////////////////////////
// Class.

namespace {

/*
 * Load used traits of PreClass `preClass', and append the trait Class*'s to
 * 'usedTraits'.  Return an estimate of the method count of all used traits.
 */
unsigned loadUsedTraits(PreClass* preClass,
                        VMCompactVector<ClassPtr>& usedTraits) {
  unsigned methodCount = 0;

  auto const traitsFlattened = !!(preClass->attrs() & AttrNoExpandTrait);
  for (auto const& traitName : preClass->usedTraits()) {
    Class* classPtr = Class::load(traitName);
    if (classPtr == nullptr) {
      raise_error(Strings::TRAITS_UNKNOWN_TRAIT, traitName->data());
    }
    if (!(classPtr->attrs() & AttrTrait)) {
      raise_error("%s cannot use %s - it is not a trait",
                  preClass->name()->data(),
                  classPtr->name()->data());
    }

    preClass->enforceInMaybeSealedParentWhitelist(classPtr->preClass());
    if (traitsFlattened) {
      // In RepoAuthoritative mode (with the WholeProgram compiler
      // optimizations), the contents of traits can be flattened into
      // the preClasses of "use"r classes. Continuing here allows us
      // to avoid unnecessarily attempting to re-import trait methods
      // and properties, only to fail due to (surprise surprise!) the
      // same method/property existing on m_preClass.
      continue;
    }

    usedTraits.push_back(ClassPtr(classPtr));
    methodCount += classPtr->numMethods();

  }

  if (!traitsFlattened) {
    // Trait aliases can increase method count. Get an estimate of the
    // number of aliased functions. This doesn't need to be done in
    // classes with flattened traits because those methods are already
    // present in the preclass.
    for (auto const& rule : preClass->traitAliasRules()) {
      auto origName = rule.origMethodName();
      auto newName = rule.newMethodName();
      if (origName != newName) {
        methodCount++;
      }
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
#ifndef NDEBUG
  static_assert(sz == (use_lowptr ? 276 : 328), "Change this only on purpose");
#else
  static_assert(sz == (use_lowptr ? 272 : 320), "Change this only on purpose");
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
  if (parent == nullptr && !(preClass->attrs() & AttrNoReifiedInit)) {
      // We will need to add 86reifiedinit method
      funcVecLen++;
    }

  VMCompactVector<ClassPtr> usedTraits;
  auto numTraitMethodsEstimate = loadUsedTraits(preClass, usedTraits);
  funcVecLen += numTraitMethodsEstimate;

  // We need to pad this allocation so that the actual start of the Class is
  // 8-byte aligned.
  auto const mask = alignof(Class) - 1;
  auto const funcvec_sz = sizeof(LowPtr<Func>) * funcVecLen;
  auto const prefix_sz = (funcvec_sz + mask) & ~mask;

  auto const size = sizeof_Class + prefix_sz
                    + sizeof(m_classVec[0]) * classVecLen;

  auto const mem = RO::RepoAuthoritative ? static_alloc(size)
                                         : lower_malloc(size);
  MemoryStats::LogAlloc(AllocKind::Class, size);
  auto const classPtr = reinterpret_cast<void*>(
    reinterpret_cast<uintptr_t>(mem) + prefix_sz
  );
  try {
    return new (classPtr) Class(preClass, parent, std::move(usedTraits),
                                classVecLen, funcVecLen);
  } catch (...) {
    if (RO::RepoAuthoritative) static_try_free(mem, size);
    else lower_sized_free(mem, size);
    throw;
  }
}

Class* Class::rescope(Class* ctx) {
  assertx(parent() == c_Closure::classof());
  assertx(m_invoke);

  // Look up the generated template class for this particular subclass of
  // Closure.  This class maintains the table of scoped clones of itself, and
  // if we create a new scoped clone, we need to map it there.
  auto template_cls = this;
  auto const invoke = template_cls->m_invoke;

  auto const try_template = [&]() -> Class* {
    if (invoke->cls() != ctx) return nullptr;

    // The first scoping will never be for `template_cls' (since it's
    // impossible to define a closure in the context of its own Closure
    // subclass), so if this happens, it means that `template_cls' is in a
    // "de-scoped" state and we shouldn't use it.  (See Class::releaseRefs().)
    if (template_cls->m_scoped && invoke->cls() == template_cls) {
      return nullptr;
    }
    return template_cls;
  };

  // If the template class has already been scoped to `ctx', we're done.  This
  // is the common case in repo mode.
  if (auto cls = try_template()) return cls;

  template_cls->allocExtraData();
  auto& scopedClones = template_cls->m_extra.raw()->m_scopedClones;

  auto const key = ctx;

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

  fermeture->m_invoke->rescope(ctx);
  fermeture->m_invoke->setHasForeignThis(false);
  fermeture->m_scoped = true;

  if (ctx != nullptr &&
      !RuntimeOption::RepoAuthoritative &&
      !classHasPersistentRDS(ctx)) {
    // If the context Class might be destroyed, we need to do extra accounting
    // so that we can drop all clones scoped to it at the time of destruction.
    ctx->allocExtraData();
    ctx->m_extra.raw()->m_clonesWithThisScope.push_back(
      ClassPtr(template_cls));
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
  if (m_lsbMemoExtra.m_handles) {
    vm_free(m_lsbMemoExtra.m_handles);
  }
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
  m_cachedClass = rds::Link<LowPtr<Class>, rds::Mode::NonLocal>{};

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
  assertx(!m_cachedClass.bound());
  assertx(!getCount());
  this->~Class();
  if (!RuntimeOption::RepoAuthoritative) {
    lower_free(mallocPtr());
  }
}

Class::~Class() {
  releaseRefs(); // must be called for Func-nulling side effects

  if (m_sPropCache) {
    for (unsigned i = 0, n = numStaticProperties(); i < n; ++i) {
      m_sPropCache[i].~Link();
    }
    vm_sized_free(m_sPropCache, numStaticProperties() * sizeof(*m_sPropCache));
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

  if (auto p = m_vtableVec.get()) {
    low_free(p);
  }

#ifndef NDEBUG
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
        meth->cls() != this) {
      setMethod(i, nullptr);
      okToReleaseParent = false;
    }
  }

  if (okToReleaseParent) {
    m_parent.reset();
  }

  m_declInterfaces.clear();
  m_requirements.clear();

  if (m_extra) {
    auto xtra = m_extra.raw();
    xtra->m_usedTraits.clear();

    if (xtra->m_clonesWithThisScope.size() > 0) {
      WriteLock l(s_scope_cache_mutex);

      // Purge all references to scoped closure clones that are scoped to
      // `this'---there is no way anyone can find them at this point.
      for (auto const template_cls : xtra->m_clonesWithThisScope) {
        auto const invoke = template_cls->m_invoke;

        if (invoke->cls() == this) {
          // We only hijack the `template_cls' as a clone for static rescopings
          // (which were signified by CloneAttr::None).  To undo this, we need
          // to make sure that /no/ scoping will match with that of
          // `template_cls'.  We can accomplish this by using `template_cls'
          // itself as the context.
          // Any instance of this closure will have its own scoped clone, and
          // we are de-scoping `template_cls' here, so the only way to obtain
          // it for dynamic binding is to reference it by name, which is
          // logically private (in PHP7, it's always just "Closure").  In HHVM,
          // this is possible by obtaining the Closure subclass's name, in
          // which case we'll just force a fresh clone via a special-case check
          // in Class::rescope().
          invoke->rescope(template_cls.get());
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

          auto const it = scopedClones.find(this);
          assertx(it != scopedClones.end());
          it->second->m_cachedClass =
            rds::Link<LowPtr<Class>, rds::Mode::NonLocal>{};
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
      parent = Class::get(ppcls->namedEntity(),
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

  for (size_t i = 0; i < m_declInterfaces.size(); i++) {
    auto di = m_declInterfaces[i].get();
    const StringData* pdi = m_preClass.get()->interfaces()[i];
    assertx(pdi->isame(di->name()));

    PreClass *pint = di->m_preClass.get();
    Class* interface = Class::get(pint->namedEntity(), pdi,
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
        Class* interface = Class::get(pint->namedEntity(), pint->name(),
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
      auto usedTrait = m_extra->m_usedTraits[i].get();
      const StringData* usedTraitName = m_preClass.get()->usedTraits()[i];
      PreClass* ptrait = usedTrait->m_preClass.get();
      Class* trait = Class::get(ptrait->namedEntity(), usedTraitName,
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
  assertx(isNormalClass(this) && isNormalClass(cls));

  // Walk up m_classVec for both classes to look for a common ancestor.
  auto vecIdx = std::min(m_classVecLen, cls->m_classVecLen) - 1;
  do {
    assertx(vecIdx < m_classVecLen && vecIdx < cls->m_classVecLen);
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
  return f != SystemLib::s_nullCtor ? f : nullptr;
}

const Func* Class::getCachedInvoke() const {
  assertx(IMPLIES(m_invoke, !m_invoke->isStaticInPrologue()));
  return m_invoke;
}

///////////////////////////////////////////////////////////////////////////////
// Builtin classes.

bool Class::isCppSerializable() const {
  assertx(instanceCtor()); // Only call this on CPP classes
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
  if (m_maybeRedefsPropTy) checkPropTypeRedefinitions();
  if (m_needsPropInitialCheck) checkPropInitialValues();

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
  if (m_maybeRedefsPropTy &&
      (!m_extra->m_checkedPropTypeRedefs.bound() ||
       !m_extra->m_checkedPropTypeRedefs.isInit())) {
    return false;
  }
  if (m_needsPropInitialCheck &&
      (!m_extra->m_checkedPropInitialValues.bound() ||
       !m_extra->m_checkedPropInitialValues.isInit())) {
    return false;
  }
  return true;
}

void Class::initProps() const {
  assertx(m_pinitVec.size() > 0);
  assertx(getPropData() == nullptr);
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
        *it, init_null_variant, nullptr, const_cast<Class*>(this), false
      );
      assertx(retval.m_type == KindOfNull);
    }
  } catch (...) {
    // Undo the allocation of propVec
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
  for (Slot slot = 0; slot < propVec->size(); slot++) {
    auto index = propSlotToIndex(slot);
    auto piv_entry = (*propVec)[index];
    assertx(!piv_entry.deepInit);
    // Set deepInit if the property requires "deep" initialization.
    if (m_declProperties[slot].attrs & AttrDeepInit) {
      piv_entry.deepInit = true;
    } else {
      TypedValue tv = piv_entry.val.tv();
      tvAsVariant(&tv).setEvalScalar();
      tvCopy(tv, piv_entry.val);
    }
  }
}

bool Class::needsInitSProps() const {
  return !m_sPropCacheInit.bound() || !m_sPropCacheInit.isInit();
}

void Class::initSProps() const {
  assertx(needsInitSProps() || m_sPropCacheInit.isPersistent());

  const bool hasNonscalarInit = !m_sinitVec.empty() || !m_linitVec.empty();
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
    // TODO(T61738946): We can remove the temporary here once we no longer
    // coerce class_meth types.
    auto val = sProp.val;

    if ((sProp.cls == this && !m_sPropCache[slot].isPersistent()) ||
        sProp.attrs & AttrLSB) {
      if (RuntimeOption::EvalCheckPropTypeHints > 0 &&
          !(sProp.attrs & (AttrInitialSatisfiesTC|AttrSystemInitialValue)) &&
          sProp.val.m_type != KindOfUninit) {
        if (sProp.typeConstraint.isCheckable()) {
          sProp.typeConstraint.verifyStaticProperty(
            &val,
            this,
            sProp.cls,
            sProp.name
          );
        }
        if (RuntimeOption::EvalEnforceGenericsUB > 0) {
          for (auto const& ub : sProp.ubs) {
            if (ub.isCheckable()) {
              ub.verifyStaticProperty(&val, this, sProp.cls, sProp.name);
            }
          }
        }
      }
      m_sPropCache[slot]->val = val;
    }
  }

  // If there are non-scalar initializers (i.e. 86sinit or 86linit methods),
  // run them now.
  // They will override the KindOfUninit values set by scalar initialization.
  if (hasNonscalarInit) {
    for (unsigned i = 0, n = m_sinitVec.size(); i < n; i++) {
      DEBUG_ONLY auto retval = g_context->invokeFunc(
        m_sinitVec[i], init_null_variant, nullptr, const_cast<Class*>(this),
        false
      );
      assertx(retval.m_type == KindOfNull);
    }
    for (unsigned i = 0, n = m_linitVec.size(); i < n; i++) {
      DEBUG_ONLY auto retval = g_context->invokeFunc(
        m_linitVec[i], init_null_variant, nullptr, const_cast<Class*>(this),
        false
      );
      assertx(retval.m_type == KindOfNull);
    }
  }

  m_sPropCacheInit.initWith(true);
}

Slot Class::lsbMemoSlot(const Func* func, bool forValue) const {
  assertx(m_extra);
  if (forValue) {
    assertx(func->numParams() == 0);
  } else {
    assertx(func->numParams() > 0);
  }
  const auto& slots = m_extra->m_lsbMemoExtra.m_slots;
  auto it = slots.find(func->getFuncId());
  always_assert(it != slots.end());
  return it->second;
}

void Class::checkPropInitialValues() const {
  assertx(m_needsPropInitialCheck);
  assertx(RuntimeOption::EvalCheckPropTypeHints > 0);
  assertx(m_extra.get() != nullptr);

  auto extra = m_extra.get();
  checkedPropInitialValuesHandle(); // init handle
  if (extra->m_checkedPropInitialValues.isInit()) return;

  for (Slot slot = 0; slot < m_declProperties.size(); ++slot) {
    auto const& prop = m_declProperties[slot];
    if (prop.attrs & (AttrInitialSatisfiesTC|AttrSystemInitialValue)) continue;
    auto const& tc = prop.typeConstraint;
    auto const index = propSlotToIndex(slot);
    auto const rval = m_declPropInit[index].val;
    if (type(rval) == KindOfUninit) continue;
    auto tv = rval.tv();
    if (tc.isCheckable()) tc.verifyProperty(&tv, this, prop.cls, prop.name);
    if (RuntimeOption::EvalEnforceGenericsUB > 0) {
      for (auto const& ub : prop.ubs) {
        if (ub.isCheckable()) {
          ub.verifyProperty(&tv, this, prop.cls, prop.name);
        }
      }
    }

    // No coercion for statically initialized properties.
    assertx(type(tv) == type(rval));
    assertx(val(tv).num == val(rval).num);
  }

  extra->m_checkedPropInitialValues.initWith(true);
}

void Class::checkPropTypeRedefinitions() const {
  assertx(m_maybeRedefsPropTy);
  assertx(RuntimeOption::EvalCheckPropTypeHints > 0);
  assertx(m_parent);
  assertx(m_extra.get() != nullptr);

  auto extra = m_extra.get();
  checkedPropTypeRedefinesHandle(); // init handle
  if (extra->m_checkedPropTypeRedefs.isInit()) return;

  if (m_parent->m_maybeRedefsPropTy) m_parent->checkPropTypeRedefinitions();

  if (m_selfMaybeRedefsPropTy) {
    for (Slot slot = 0; slot < m_declProperties.size(); slot++) {
      auto const& prop = m_declProperties[slot];
      if (prop.attrs & AttrNoBadRedeclare) continue;
      checkPropTypeRedefinition(slot);
    }
  }

  extra->m_checkedPropTypeRedefs.initWith(true);
}

void Class::checkPropTypeRedefinition(Slot slot) const {
  assertx(m_maybeRedefsPropTy);
  assertx(RuntimeOption::EvalCheckPropTypeHints > 0);
  assertx(m_parent);
  assertx(slot != kInvalidSlot);
  assertx(slot < numDeclProperties());

  auto const& prop = m_declProperties[slot];
  assertx(!(prop.attrs & AttrNoBadRedeclare));

  auto const& oldProp = m_parent->m_declProperties[slot];

  auto const& oldTC = oldProp.typeConstraint;
  auto const& newTC = prop.typeConstraint;

  if (!oldTC.equivalentForProp(newTC)) {
    auto const oldTCName =
      oldTC.hasConstraint() ? oldTC.displayName() : "mixed";
    auto const newTCName =
      newTC.hasConstraint() ? newTC.displayName() : "mixed";

    auto const msg = folly::sformat(
      "Type-hint of '{}::{}' must be {}{} (as in class {}), not {}",
      prop.cls->name(),
      prop.name,
      oldTC.isUpperBound()? "upper-bounded by " : "",
      oldTCName,
      oldProp.cls->name(),
      newTCName
    );
    raise_property_typehint_error(
      msg,
      oldTC.isSoft() && newTC.isSoft(),
      oldTC.isUpperBound() || newTC.isUpperBound()
    );
  }

  if (RuntimeOption::EvalEnforceGenericsUB > 0 &&
      (!prop.ubs.empty() || !oldProp.ubs.empty())) {
    std::vector<TypeConstraint> newTCs = {newTC};
    for (auto const& ub : prop.ubs) newTCs.push_back(ub);
    std::vector<TypeConstraint> oldTCs = {oldTC};
    for (auto const& ub : oldProp.ubs) oldTCs.push_back(ub);

    for (auto const& ub : newTCs) {
      if (std::none_of(oldTCs.begin(), oldTCs.end(),
            [&](const TypeConstraint& tc) {
              return tc.equivalentForProp(ub);
            })) {
        auto const ubName = ub.hasConstraint() ? ub.displayName() : "mixed";
        auto const msg = folly::sformat(
          "Upper-bound {} of {}::{} has no equivalent upper-bound in {}",
          ubName, prop.cls->name(), prop.name, oldProp.cls->name()
        );
        raise_property_typehint_error(msg, ub.isSoft(), true);
      }
    }
    for (auto const& ub : oldTCs) {
      if (std::none_of(newTCs.begin(), newTCs.end(),
            [&](const TypeConstraint& tc) {
              return tc.equivalentForProp(ub);
            })) {
        auto const ubName = ub.hasConstraint() ? ub.displayName() : "mixed";
        auto const msg = folly::sformat(
          "Upper-bound {} of {}::{} has no equivalent upper-bound in {}",
          ubName, oldProp.cls->name(), oldProp.name, prop.cls->name()
        );
        raise_property_typehint_error(msg, ub.isSoft(), true);
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// Property storage.

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

    if (sProp.cls == this || (sProp.attrs & AttrLSB)) {
      if (usePersistentHandles && (sProp.attrs & AttrPersistent)) {
        static_assert(sizeof(StaticPropData) == sizeof(sProp.val),
                      "StaticPropData must be a simple wrapper "
                      "around TypedValue");
        propHandle.bind(
          rds::Mode::Persistent,
          rds::SPropCache{this, slot},
          reinterpret_cast<const StaticPropData*>(&sProp.val)
        );
      } else {
        propHandle.bind(
          rds::Mode::Local,
          rds::SPropCache{this, slot}
        );
      }
    } else {
      auto const realSlot = sProp.cls->lookupSProp(sProp.name);
      propHandle = sProp.cls->m_sPropCache[realSlot];
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
    m_sPropCacheInit = rds::s_persistentTrue;
  } else {
    m_sPropCacheInit.bind(
      rds::Mode::Normal,
      rds::LinkName{"PropCacheInit", name()}
    );
  }
  rds::recordRds(m_sPropCacheInit.handle(),
                 sizeof(bool), "SPropCacheInit", name()->slice());
}

Class::PropInitVec* Class::getPropData() const {
  return (m_propDataCache.bound() && m_propDataCache.isInit())
    ? *m_propDataCache
    : nullptr;
}

TypedValue* Class::getSPropData(Slot index) const {
  assertx(numStaticProperties() > index);
  return m_sPropCache[index].bound()
    ? &m_sPropCache[index].get()->val
    : nullptr;
}


///////////////////////////////////////////////////////////////////////////////
// Property lookup and accessibility.

Class::PropSlotLookup Class::getDeclPropSlot(
  const Class* ctx,
  const StringData* key
) const {
  auto const propSlot = lookupDeclProp(key);

  auto accessible = false;

  if (propSlot != kInvalidSlot) {
    auto const attrs = m_declProperties[propSlot].attrs;
    if ((attrs & (AttrProtected|AttrPrivate)) &&
        (g_context.isNull() || !g_context->debuggerSettings.bypassCheck)) {
      // Fetch the class in the inheritance tree which first declared the
      // property
      auto const baseClass = m_declProperties[propSlot].cls;
      assertx(baseClass);

      // If ctx == baseClass, we have the right property and we can stop here.
      if (ctx == baseClass) return PropSlotLookup { propSlot, true };

      // The anonymous context cannot access protected or private properties, so
      // we can fail fast here.
      if (ctx == nullptr) return PropSlotLookup { propSlot, false };

      assertx(ctx);
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
          return PropSlotLookup { propSlot, true };
        }
        if (!baseClass->classof(ctx)) {
          // ctx is not the same, an ancestor, or a descendent of baseClass,
          // so the property is not accessible. Also, we know that ctx cannot
          // be the same or an ancestor of this, so we don't need to check if
          // ctx declares a private property with the same name and we can
          // fail fast here.
          return PropSlotLookup { propSlot, false };
        }
        // We now know this protected property is accessible, but we need to
        // keep going because ctx may define a private property with the same
        // name.
        accessible = true;
        assertx(baseClass->classof(ctx));
      }
    } else {
      // The property is public (or we're in the debugger and we are bypassing
      // accessibility checks).
      accessible = true;
      // If ctx == this, we don't have to check if ctx defines a private
      // property with the same name and we can stop here.
      if (ctx == this) return PropSlotLookup { propSlot, true };

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
    auto const ctxPropSlot = ctx->lookupDeclProp(key);

    if (ctxPropSlot != kInvalidSlot &&
        ctx->m_declProperties[ctxPropSlot].cls == ctx &&
        (ctx->m_declProperties[ctxPropSlot].attrs & AttrPrivate)) {
      // A private property from ctx trumps any other property we may
      // have found.
      return PropSlotLookup { ctxPropSlot, true };
    }
  }

  if (propSlot == kInvalidSlot &&
      !g_context.isNull() &&
      g_context->debuggerSettings.bypassCheck &&
      m_parent) {
    // If the property could not be located on the current class, and this
    // class has a parent class, and the current evaluation is a debugger
    // eval with bypassCheck == true, search for the property as a member of
    // the parent class. The debugger access is not subject to visibilty checks.
    return m_parent->getDeclPropSlot(ctx, key);
  }

  return PropSlotLookup { propSlot, accessible };
}

Class::PropSlotLookup Class::findSProp(
  const Class* ctx,
  const StringData* sPropName
) const {
  auto const sPropInd = lookupSProp(sPropName);

  // Non-existent property.
  if (sPropInd == kInvalidSlot)
    return PropSlotLookup { kInvalidSlot, false, false };

  auto const& sProp = m_staticProperties[sPropInd];
  auto const sPropAttrs = sProp.attrs;
  auto const sPropConst = bool(sPropAttrs & AttrIsConst);
  const Class* baseCls = this;
  if (sPropAttrs & AttrLSB) {
    // For an LSB static, accessibility attributes are relative to the class
    // that originally declared it.
    baseCls = sProp.cls;
  }
  // Property access within this Class's context.
  if (ctx == baseCls) return PropSlotLookup { sPropInd, true, sPropConst };

  auto const accessible = [&] {
    switch (sPropAttrs & (AttrPublic | AttrProtected | AttrPrivate)) {
      // Public properties are always accessible.
      case AttrPublic:
        return true;

      // Property access is from within a parent class's method, which is
      // allowed for protected properties.
      case AttrProtected:
        return ctx != nullptr &&
               (baseCls->classof(ctx) || ctx->classof(baseCls));

      // Can only access private properties via the debugger.
      case AttrPrivate:
        return g_context->debuggerSettings.bypassCheck;

      default: break;
    }
    not_reached();
  }();

  return PropSlotLookup { sPropInd, accessible, sPropConst };
}

Class::PropValLookup Class::getSPropIgnoreLateInit(
  const Class* ctx,
  const StringData* sPropName
) const {
  initialize();

  auto const lookup = findSProp(ctx, sPropName);
  if (lookup.slot == kInvalidSlot) {
    return PropValLookup { nullptr, kInvalidSlot, false, false };
  }

  auto const sProp = getSPropData(lookup.slot);

  if (debug) {
    auto const& decl = m_staticProperties[lookup.slot];
    auto const lateInit = bool(decl.attrs & AttrLateInit);

    always_assert(
      sProp && (sProp->m_type != KindOfUninit || lateInit) &&
      "Static property initialization failed to initialize a property."
    );

    if (sProp->m_type != KindOfUninit) {
      if (RuntimeOption::RepoAuthoritative) {
        auto const repoTy = staticPropRepoAuthType(lookup.slot);
        always_assert(tvMatchesRepoAuthType(*sProp, repoTy));
      }

      if (RuntimeOption::EvalCheckPropTypeHints > 2) {
        auto const typeOk = [&]{
          auto skipCheck =
            !decl.typeConstraint.isCheckable() ||
            decl.typeConstraint.isSoft() ||
            (decl.typeConstraint.isUpperBound() &&
             RuntimeOption::EvalEnforceGenericsUB < 2) ||
            (sProp->m_type == KindOfNull &&
             !(decl.attrs & AttrNoImplicitNullable));

          auto res = skipCheck ? true : decl.typeConstraint.assertCheck(sProp);
          if (RuntimeOption::EvalEnforceGenericsUB >= 2) {
            for (auto const& ub : decl.ubs) {
              if (ub.isCheckable() && !ub.isSoft()) {
                res = res && ub.assertCheck(sProp);
              }
            }
          }
          return res;
        }();
        always_assert(typeOk);
      }
    }
  }

  return
    PropValLookup { sProp, lookup.slot, lookup.accessible, lookup.constant };
}

Class::PropValLookup Class::getSProp(
  const Class* ctx,
  const StringData* sPropName
) const {
  auto const lookup = getSPropIgnoreLateInit(ctx, sPropName);
  if (lookup.val && UNLIKELY(lookup.val->m_type == KindOfUninit)) {
    auto const& decl = m_staticProperties[lookup.slot];
    if (decl.attrs & AttrLateInit) {
      throw_late_init_prop(decl.cls, sPropName, true);
    }
  }
  return lookup;
}

bool Class::IsPropAccessible(const Prop& prop, Class* ctx) {
  if (prop.attrs & AttrPublic) return true;
  if (prop.attrs & AttrPrivate) return prop.cls == ctx;
  if (!ctx) return false;

  return prop.cls->classof(ctx) || ctx->classof(prop.cls);
}

Slot Class::propIndexToSlot(uint16_t index) const {
  auto const nprops = m_declProperties.size();
  for (Slot slot = 0; slot < nprops; slot++) {
    if (m_slotIndex[slot] == index) return slot;
  }
  always_assert_flog(0, "propIndexToSlot: no slot found for index = {}", index);
}

///////////////////////////////////////////////////////////////////////////////
// Constants.

const StaticString s_classname("classname");

TypedValue Class::clsCnsGet(const StringData* clsCnsName, ClsCnsLookup what) const {
  Slot clsCnsInd;
  auto cnsVal = cnsNameToTV(clsCnsName, clsCnsInd, what);
  if (!cnsVal) return make_tv<KindOfUninit>();

  auto& cns = m_constants[clsCnsInd];
  ArrayData* typeCns = nullptr;

  if (cnsVal->m_type != KindOfUninit) {
    if (cns.isType()) {
      // Type constants with the low bit set are already resolved and can be
      // returned after masking out that bit.
      //
      // We can't check isHAMSafeDArray here because we're using that low bit as
      // a marker; instead, check isArrayLike and do the stricter check below.
      assertx(isArrayLikeType(type(cnsVal)));
      assertx(!isRefcountedType(type(cnsVal)));
      typeCns = val(cnsVal).parr;
      auto const rawData = reinterpret_cast<intptr_t>(typeCns);
      if (rawData & 0x1) {
        auto const resolved = reinterpret_cast<ArrayData*>(rawData ^ 0x1);
        assertx(resolved->isHAMSafeDArray());
        return make_persistent_array_like_tv(resolved);
      }
      if (what == ClsCnsLookup::IncludeTypesPartial) {
        return make_tv<KindOfUninit>();
      }
    } else {
      return *cnsVal;
    }
  }

  /*
   * We use a sentinel static array to mark constants that are being evaluated
   * during recursive resolution. This array is never exposed to the rest of
   * the runtime, so we can test for it by pointer equality.
   */
  static auto const s_sentinelVec = PackedArray::CopyStatic(staticEmptyVec());
  assertx(s_sentinelVec != staticEmptyVec());

  /*
   * We have either a constant with a non-scalar initializer or an unresolved
   * type constant, meaning it will be potentially different in different
   * requests, which we store separately in an array in RDS.
   *
   * We need a special marker value in the non-scalar constant cache to indicate
   * that we're currently evaluating the value of a (type) constant. If we
   * attempt to evaluate the value of a (type) constant, and the special marker
   * is present, that means the (type) constant is recursively defined and
   * we'll raise an error. The globals array is never a valid value of a (type)
   * constant, so we use it as the marker.
   */
  m_nonScalarConstantCache.bind(
    rds::Mode::Normal,
    rds::LinkName{"ClassNonScalarCnsCache", name()}
  );
  auto& clsCnsData = *m_nonScalarConstantCache;
  if (m_nonScalarConstantCache.isInit()) {
    auto const cCns = clsCnsData->get(clsCnsName);
    if (cCns.is_init()) {
      // There's an entry in the cache for this (type) constant. If it's the
      // sentinel value, the constant is recursively defined - throw an error.
      // Otherwise, return the cached result.
      if (UNLIKELY(tvIsVec(cCns) && val(cCns).parr == s_sentinelVec)) {
        raise_error(
          folly::sformat(
            "Cannot declare self-referencing {} '{}::{}'",
            cns.isType() ? "type constant" : "constant",
            name(),
            clsCnsName
          )
        );
      }
      return cCns;
    }
  } else {
    // Because RDS uses a generation number scheme, when isInit was false we
    // may have a pointer to a (no-longer extant) ArrayData in our RDS entry.
    // Use detach to clear our Array so we don't attempt to decref the stale
    // ArrayData.
    clsCnsData.detach();
    clsCnsData = Array::attach(
      MixedArray::MakeReserveDict(m_constants.size())
    );
    m_nonScalarConstantCache.markInit();
  }

  // We're going to run the 86cinit to get the constant's value, or try to
  // resolve the type constant. Store the sentinel value as this (type)
  // constant's cache entry // so that we can detect recursion.
  auto marker = make_array_like_tv(s_sentinelVec);
  clsCnsData.set(StrNR(clsCnsName), tvAsCVarRef(&marker), true /* isKey */);

  if (cns.isType()) {
    // Resolve type constant, if needed
    if (what == ClsCnsLookup::IncludeTypesPartial) {
      return make_tv<KindOfUninit>();
    }
    Array resolvedTS;
    bool persistent = true;
    try {
      resolvedTS = TypeStructure::resolve(typeCns, cns.name, cns.cls, this,
                                          persistent);
    } catch (const Exception& e) {
      raise_error(e.getMessage());
    }
    assertx(resolvedTS.isHAMSafeDArray());

    auto const ad = ArrayData::GetScalarArray(std::move(resolvedTS));
    if (persistent) {
      auto const rawData = reinterpret_cast<intptr_t>(ad);
      assertx((rawData & 0x7) == 0 && "ArrayData not 8-byte aligned");
      auto taggedData = reinterpret_cast<ArrayData*>(rawData | 0x1);

      // Multiple threads might create and store the resolved type structure
      // here, but that's fine since they'll all store the same thing thanks to
      // GetScalarArray(). Ditto for the pointed class.
      // We could avoid a little duplicated work during warmup with more
      // complexity but it's not worth it.
      auto& cns_nc = const_cast<Const&>(cns);
      auto const classname_field = ad->get(s_classname.get());
      if (isStringType(classname_field.type())) {
        cns_nc.setPointedClsName(classname_field.val().pstr);
      }
      cns_nc.val.m_data.parr = taggedData;
      return make_persistent_array_like_tv(ad);
    }

    auto tv = make_persistent_array_like_tv(ad);
    clsCnsData.set(StrNR(clsCnsName), tvAsCVarRef(&tv), true /* isKey */);
    return tv;
  }

  // The class constant has not been initialized yet; do so.
  auto const meth86cinit = cns.cls->lookupMethod(s_86cinit.get());
  TypedValue args[1] = {
    make_tv<KindOfPersistentString>(const_cast<StringData*>(cns.name.get()))
  };

  auto ret = g_context->invokeFuncFew(
    meth86cinit,
    const_cast<Class*>(this),
    1,
    args,
    false,
    false
  );

  switch (tvAsCVarRef(&ret).isAllowedAsConstantValue()) {
    case Variant::AllowedAsConstantValue::Allowed:
      break;
    case Variant::AllowedAsConstantValue::NotAllowed: {
      always_assert(false);
    }
    case Variant::AllowedAsConstantValue::ContainsObject: {
      // Generally, objects are not allowed as class constants.
      if (!(attrs() & AttrEnumClass)) {
        raise_error("Value unsuitable as class constant");
      }
      break;
    }
  }

  clsCnsData.set(StrNR(clsCnsName), tvAsCVarRef(ret), true /* isKey */);

  // The caller will inc-ref the returned value, so undo the inc-ref caused by
  // storing it in the cache.
  tvDecRefGenNZ(&ret);
  return ret;
}

const TypedValue* Class::cnsNameToTV(const StringData* clsCnsName,
                               Slot& clsCnsInd,
                               ClsCnsLookup what) const {
  clsCnsInd = m_constants.findIndex(clsCnsName);
  if (clsCnsInd == kInvalidSlot) {
    return nullptr;
  }
  if (m_constants[clsCnsInd].isAbstract()) {
    return nullptr;
  }
  if (what == ClsCnsLookup::NoTypes && m_constants[clsCnsInd].isType()) {
    return nullptr;
  }
  auto const ret = &m_constants[clsCnsInd].val;
  assertx(m_constants[clsCnsInd].isType() || tvIsPlausible(*ret));
  return ret;
}

Slot Class::clsCnsSlot(
  const StringData* name, bool wantTypeCns, bool allowAbstract
) const {
  auto slot = m_constants.findIndex(name);
  if (slot == kInvalidSlot) return slot;
  if (!allowAbstract && m_constants[slot].isAbstract()) return kInvalidSlot;
  return m_constants[slot].isType() == wantTypeCns ? slot : kInvalidSlot;
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

namespace {
const ReifiedGenericsInfo k_defaultReifiedGenericsInfo{0, false, 0, {}};
} // namespace

const ReifiedGenericsInfo& Class::getReifiedGenericsInfo() const {
  if (!m_hasReifiedGenerics) return k_defaultReifiedGenericsInfo;
  assertx(m_extra);
  return m_extra.raw()->m_reifiedGenericsInfo;
}

///////////////////////////////////////////////////////////////////////////////
// Private methods.
//
// These are mostly for the class creation path.

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
    if ((m_attrCopy & AttrIsConst) && !(parentAttrs & AttrIsConst)) {
      raise_error(
        "Const class %s cannot extend non-const parent %s",
        m_preClass->name()->data(),
        m_parent->name()->data()
      );
    }
    if (!(m_attrCopy & AttrEnumClass) && (parentAttrs & AttrEnumClass)) {
      raise_error(
        "Non-enum class %s cannot extend enum class parent %s",
        m_preClass->name()->data(),
        m_parent->name()->data()
      );
    }
    if ((m_attrCopy & AttrEnumClass) && !(parentAttrs & AttrEnumClass)){
      raise_error(
        "Enum class %s cannot extend non-enum class parent %s",
        m_preClass->name()->data(),
        m_parent->name()->data()
      );
    }

    m_preClass->enforceInMaybeSealedParentWhitelist(m_parent->preClass());
    if (m_parent->m_maybeRedefsPropTy) m_maybeRedefsPropTy = true;
  }

  // Handle stuff specific to cppext classes
  if (m_parent.get() && m_parent->m_extra->m_instanceCtor) {
    allocExtraData();
    m_extra.raw()->m_instanceCtor = m_parent->m_extra->m_instanceCtor;
    m_extra.raw()->m_instanceCtorUnlocked =
      m_parent->m_extra->m_instanceCtorUnlocked;
    m_extra.raw()->m_instanceDtor = m_parent->m_extra->m_instanceDtor;
    assertx(m_parent->m_releaseFunc == m_parent->m_extra->m_instanceDtor);
    m_releaseFunc = m_parent->m_extra->m_instanceDtor;
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
  s_invoke("__invoke"),
  s_sleep("__sleep"),
  s_debugInfo("__debugInfo"),
  s_clone("__clone");

void Class::setSpecial() {
  m_toString = lookupMethod(s_toString.get());

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
  m_invoke = lookupMethod(s_invoke.get());
  if (m_invoke && m_invoke->isStaticInPrologue()) {
    m_invoke = nullptr;
  }

  // Look for __construct() declared in either this class or a trait
  auto const func = lookupMethod(s_construct.get());
  if (func && func->cls() == this) {
    if (func->takesInOutParams()) {
      raise_error("Parameters may not be marked inout on constructors");
    }

    m_ctor = func;
    return;
  };

  // Look for parent constructor.
  if (m_parent.get() != nullptr && m_parent->m_ctor) {
    m_ctor = m_parent->m_ctor;
    return;
  }

  if (UNLIKELY(!SystemLib::s_nullCtor)) {
    SystemLib::setupNullCtor(this);
  }

  m_ctor = SystemLib::s_nullCtor;
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

inline void checkRefCompat(const char* kind, const Func* self,
                           const Func* inherit) {
  // Shadowing is okay, if we inherit a private method we can't access it
  // anyway.
  if (inherit->attrs() & AttrPrivate) return;

  if (!self->takesInOutParams() && !inherit->takesInOutParams()) return;

  auto const max = std::max(
    self->numNonVariadicParams(),
    inherit->numNonVariadicParams()
  );

  for (int i = 0; i < max; ++i) {
    // Since we're looking at ref wrappers of inout functions we need to check
    // inout, but if one of the functions isn't a wrapper we do actually have
    // a mismatch.
    auto const smode = self->isInOut(i);
    auto const imode = inherit->isInOut(i);
    if (smode != imode) {
      auto const msg = [&] {
        auto const sname = self->fullName()->data();
        auto const iname = inherit->fullName()->data();

        if (smode) {
          auto const idecl =
            i >= inherit->numNonVariadicParams() ? "" : "inout ";
          return folly::sformat(
            "Parameter {} on function {} was declared inout but is not "
            "declared {}on {} function {}", i + 1, sname, idecl,
            kind, iname);
        } else {
          auto const sdecl = i >= self->numNonVariadicParams() ? "" : "inout ";
          return folly::sformat(
            "Parameter {} on function {} was not declared {}but is "
            "declared inout on {} function {}", i + 1, sname, sdecl,
            kind, iname);
        }
      }();
      raise_error(msg);
    }
  }
}

// Check compatibility vs interface and abstract declarations
void checkDeclarationCompat(const PreClass* preClass,
                            const Func* func, const Func* imeth) {
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
      if (!iparams[i].hasDefaultValue()) {
        // The leftmost of imeth's contiguous trailing optional parameters
        // must start somewhere to the right of this parameter (which may
        // be the variadic param)
        firstOptional = i + 1;
      }
    }
    assertx(!ivariadic || iparams[iparams.size() - 1].isVariadic());
    assertx(!ivariadic || params[params.size() - 1].isVariadic());
  }

  // Verify that meth provides defaults, starting with the parameter that
  // corresponds to the leftmost of imeth's contiguous trailing optional
  // parameters and *not* including any variadic last param (variadics
  // don't have any default values).
  for (unsigned i = firstOptional; i < func->numNonVariadicParams(); ++i) {
    if (!params[i].hasDefaultValue()) {
      raiseIncompat(preClass, imeth);
    }
  }

  checkRefCompat(
    imeth->attrs() & AttrAbstract ? "abstract" : "interface",
    func,
    imeth
  );
}

} // namespace

Class::Class(PreClass* preClass, Class* parent,
             VMCompactVector<ClassPtr>&& usedTraits,
             unsigned classVecLen, unsigned funcVecLen)
  : m_releaseFunc{ObjectData::release}
  , m_preClass(PreClassPtr(preClass))
  , m_classVecLen(always_safe_cast<decltype(m_classVecLen)>(classVecLen))
  , m_funcVecLen(always_safe_cast<decltype(m_funcVecLen)>(funcVecLen))
  , m_maybeRedefsPropTy{false}
  , m_selfMaybeRedefsPropTy{false}
  , m_needsPropInitialCheck{false}
  , m_hasReifiedGenerics{false}
  , m_hasReifiedParent{false}
  , m_serialized(false)
  , m_parent(parent)
#ifndef NDEBUG
  , m_magic{kMagic}
#endif
{
  if (usedTraits.size()) {
    allocExtraData();
    m_extra.raw()->m_usedTraits = std::move(usedTraits);
  }
  setParent();
  setMethods();
  setSpecial();       // must run before setRTAttributes
  setRTAttributes();
  setInterfaces();
  setConstants();
  setProperties();    // must run before setInitializers
  setReifiedData();
  setInitializers();
  setClassVec();
  setRequirements();
  setNativeDataInfo();
  initClosure();
  setEnumType();
  setInstanceMemoCacheInfo();
  setLSBMemoCacheInfo();

  // A class is allowed to implement two interfaces that share the same slot if
  // we'll fatal trying to define that class, so this has to happen after all
  // of those fatals could be thrown.
  setInterfaceVtables();

  // Calculates the base pointer offset and
  // the MemoryManager index of the class size.
  setReleaseData();
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

  if ((method->attrs() & AttrAbstract) &&
      !(parentMethod->attrs() & AttrAbstract) &&
      !method->isFromTrait()) {
    raise_error("Cannot re-declare non-abstract method %s::%s() abstract in "
                "class %s",
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
  } else {
    checkRefCompat("parent", method, parentMethod);
  }
}

void Class::setMethods() {
  MethodMapBuilder builder;

  ITRACE(5, "----------\nsetMethods() for {}:\n", this->name()->data());
  if (m_parent.get() != nullptr) {
    // Copy down the parent's method entries. These may be overridden below.
    for (Slot i = 0; i < m_parent->m_methods.size(); ++i) {
      Func* f = m_parent->getMethod(i);
      assertx(f);
      ITRACE(5, "  - adding parent method {}\n", f->name()->data());
      assertx(builder.size() == i);
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
      if (method->name() == s_86sinit.get() ||
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
      assertx(parentMethod);
      // An abstract method that came from a trait doesn't override another
      // method.
      if (method->isFromTrait() && (method->attrs() & AttrAbstract)) continue;
      methodOverrideCheck(parentMethod, method);
      // Overlay.
      Func* f = method->clone(this);
      f->setNewFuncId();
      Class* baseClass;
      assertx(!(f->attrs() & AttrPrivate) ||
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

  // Add 86reifiedinit to base classes that do not have AttrNoReifiedInit set
  if (m_parent.get() == nullptr && !(attrs() & AttrNoReifiedInit)) {
    assertx(builder.find(s_86reifiedinit.get()) == builder.end());
    auto f = SystemLib::getNull86reifiedinit(this);
    builder.add(f->name(), f);
  }

  auto const traitsBeginIdx = builder.size();
  if (m_extra->m_usedTraits.size()) {
    importTraitMethods(builder);
  }
  auto const traitsEndIdx = builder.size();

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

/*
 * Initialize m_RTAttrs by inspecting the class methods and parents.
 */
void Class::setRTAttributes() {
  m_RTAttrs = 0;
  if (lookupMethod(s_sleep.get())) { m_RTAttrs |= Class::HasSleep; }
  if (lookupMethod(s_clone.get())) { m_RTAttrs |= Class::HasClone; }

  if ((isBuiltin() && Native::getNativePropHandler(name())) ||
      (m_parent && m_parent->hasNativePropHandler())) {
    m_RTAttrs |= Class::HasNativePropHandler;
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
        for (auto const& interface : m_declInterfaces) {
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
        for (auto interface : m_declInterfaces) {
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
      assertx(idx != -1);
      cns.val = preConsts[idx].val();
    }
#ifndef USE_LOWPTR
    cns.pointedClsName = nullptr;
#endif
  }

  m_constants.create(builder);
}

namespace {

void copyDeepInitAttr(const PreClass::Prop* pclsProp, Class::Prop* clsProp) {
  if (pclsProp->attrs() & AttrDeepInit) {
    clsProp->attrs = (Attr)(clsProp->attrs | AttrDeepInit);
  } else {
    clsProp->attrs = (Attr)(clsProp->attrs & ~AttrDeepInit);
  }
}

/*
 * KeyFn should be a function that takes an index and returns a key to sort by.
 * To sort lexicographically by multiple fields, the key can be a tuple type.
 */
template<typename KeyFn>
void sortByKey(std::vector<uint32_t>& indices, KeyFn keyFn) {
  std::sort(indices.begin(), indices.end(), [&](uint32_t a, uint32_t b) {
    return keyFn(a) < keyFn(b);
  });
}

}

void Class::sortOwnProps(const PropMap::Builder& curPropMap,
                         uint32_t first,
                         uint32_t past,
                         std::vector<uint16_t>& slotIndex) {
  auto const serverMode = RuntimeOption::ServerExecutionMode();
  FTRACE(3, "Class::sortOwnProps: PreClass: {}\n", m_preClass->name()->data());
  if (serverMode && RuntimeOption::ServerLogReorderProps) {
    Logger::FInfo("Class::sortOwnProps: PreClass: {}",
                  m_preClass->name()->data());
  }
  auto const size = past - first;
  if (size == 0) return; // no own props
  std::vector<uint32_t> order(size);
  for (uint32_t i = 0; i < size; i++) {
    order[i] = first + i;
  }
  // We don't change the order of the properties for closures.
  if (c_Closure::initialized() && parent() != c_Closure::classof()) {
    if (RuntimeOption::EvalReorderProps == "hotness") {
      /* Sort the properties in decreasing order of their profile counts,
       * according to the serialized JIT profile data.  In case of ties, we
       * preserve the logical order among the properties.  Note that, in the
       * absence of profile data (e.g. if jumpstart failed to deserialize the
       * JIT profile data), then the physical layout of the properties in memory
       * will match their logical order. */
      sortByKey(order, [&](uint32_t index) {
        auto const& prop = curPropMap[index];
        int64_t count = PropertyProfile::getCount(prop.cls->name(), prop.name);
        return std::make_tuple(-count, index);
      });
    } else if (RuntimeOption::EvalReorderProps == "alphabetical") {
      std::sort(order.begin(), order.end(),
                [&] (uint32_t a, uint32_t b) {
                  auto const& propa = curPropMap[a];
                  auto const& propb = curPropMap[b];
                  return strcmp(propa.name->data(), propb.name->data()) < 0;
                });
    } else if (RuntimeOption::EvalReorderProps == "countedness") {
      // Countable properties come earlier. Break ties by logical order.
      sortByKey(order, [&](uint32_t index) {
        auto const& prop = curPropMap[index];
        int64_t countable = jit::irgen::propertyMayBeCountable(prop);
        return std::make_tuple(-countable, index);
      });
    } else if (RuntimeOption::EvalReorderProps == "countedness-hotness") {
      // Countable properties come earlier. Break ties by profile counts
      // (assuming that we have them from jumpstart), then by logical order.
      sortByKey(order, [&](uint32_t index) {
        auto const& prop = curPropMap[index];
        int64_t count = PropertyProfile::getCount(prop.cls->name(), prop.name);
        int64_t countable = jit::irgen::propertyMayBeCountable(prop);
        return std::make_tuple(-countable, -count, index);
      });
    }
  }
  assertx(slotIndex.size() == past);
  for (uint32_t i = 0; i < size; i++) {
    auto slot = order[i];
    auto index = first + i;
    slotIndex[slot] = index;
    auto const& prop = curPropMap[slot];
    FTRACE(
      3, "  index={}: slot={}, prop={}, count={}, countable={}\n",
      index, slot, prop.name->data(),
      PropertyProfile::getCount(prop.cls->name(), prop.name),
      jit::irgen::propertyMayBeCountable(prop)
    );
    if (serverMode && RuntimeOption::ServerLogReorderProps) {
      Logger::FInfo(
        "  index={}: slot={}, prop={}, count={}, countable={}",
        index, slot, prop.name->data(),
        PropertyProfile::getCount(prop.cls->name(), prop.name),
        jit::irgen::propertyMayBeCountable(prop)
      );
    }
  }
  sortOwnPropsInitVec(first, past, slotIndex);
}

void Class::sortOwnPropsInitVec(uint32_t first, uint32_t past,
                                const std::vector<uint16_t>& slotIndex) {
  auto const size = past - first;
  std::vector<TypedValue> tmpPropInitVec(size);
  std::vector<bool> tmpDeepInit(size);
  for (uint32_t i = first; i < past; i++) {
    tvCopy(m_declPropInit[i].val.tv(), tmpPropInitVec[i - first]);
    tmpDeepInit[i - first] = m_declPropInit[i].deepInit;
  }
  for (uint32_t slot = first; slot < past; slot++) {
    auto index = slotIndex[slot];
    FTRACE(3, "  - placing propInit for slot {} at index {}\n", slot, index);
    tvCopy(tmpPropInitVec[slot - first], m_declPropInit[index].val);
    m_declPropInit[index].deepInit = tmpDeepInit[slot - first];
  }
}

void Class::setProperties() {
  int numInaccessible = 0;
  PropMap::Builder curPropMap;
  SPropMap::Builder curSPropMap;
  m_hasDeepInitProps = false;
  std::vector<uint16_t> slotIndex;

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
      prop.preProp             = parentProp.preProp;
      prop.cls                 = parentProp.cls;
      prop.baseCls             = parentProp.baseCls;
      prop.mangledName         = parentProp.mangledName;
      prop.attrs               = parentProp.attrs | AttrNoBadRedeclare;
      prop.typeConstraint      = parentProp.typeConstraint;
      prop.ubs                 = parentProp.ubs;
      prop.name                = parentProp.name;
      prop.repoAuthType        = parentProp.repoAuthType;

      if (!(parentProp.attrs & AttrPrivate)) {
        curPropMap.add(prop.name, prop);
      } else {
        ++numInaccessible;
        curPropMap.addUnnamed(prop);
      }
    }
    m_declPropInit = m_parent->m_declPropInit;
    m_needsPropInitialCheck = m_parent->m_needsPropInitialCheck;
    auto& parentSlotIndex = m_parent->m_slotIndex;
    slotIndex.insert(slotIndex.end(),
                     parentSlotIndex.begin(), parentSlotIndex.end());
    for (auto const& parentProp : m_parent->staticProperties()) {
      if ((parentProp.attrs & AttrPrivate) &&
          !(parentProp.attrs & AttrLSB)) continue;

      // Alias parent's static property.
      SProp sProp;
      sProp.preProp        = parentProp.preProp;
      sProp.name           = parentProp.name;
      sProp.attrs          = parentProp.attrs | AttrNoBadRedeclare;
      sProp.typeConstraint = parentProp.typeConstraint;
      sProp.ubs            = parentProp.ubs;
      sProp.cls            = parentProp.cls;
      sProp.repoAuthType   = parentProp.repoAuthType;
      tvWriteUninit(sProp.val);
      curSPropMap.add(sProp.name, sProp);
    }
  }

  Slot traitIdx = m_preClass->numProperties();
  if (m_preClass->attrs() & AttrNoExpandTrait) {
    while (traitIdx &&
           (m_preClass->properties()[traitIdx - 1].attrs() & AttrTrait)) {
      traitIdx--;
    }
  }

  Slot serializationIdx = 0;
  std::vector<bool> serializationVisited(curPropMap.size(), false);
  Slot staticSerializationIdx = 0;
  std::vector<bool> staticSerializationVisited(curSPropMap.size(), false);

  static_assert(AttrPublic < AttrProtected && AttrProtected < AttrPrivate, "");
  auto const firstOwnPropSlot = curPropMap.size();

  for (Slot slot = 0; slot < traitIdx; ++slot) {
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
          && (preProp->attrs() & VisibilityAttrs)
             > (parentProp->attrs & VisibilityAttrs)) {
        raise_error(
          "Access level to %s::$%s must be %s (as in class %s) or weaker",
          m_preClass->name()->data(), preProp->name()->data(),
          attrToVisibilityStr(parentProp->attrs),
          m_parent->name()->data());
      }
      if (preProp->attrs() & AttrDeepInit) {
        m_hasDeepInitProps = true;
      }

      auto addNewProp = [&] {
        Prop prop;
        initProp(prop, preProp);

        curPropMap.add(preProp->name(), prop);
        curPropMap[serializationIdx++].serializationIdx = curPropMap.size() - 1;
        serializationVisited.push_back(true);
        m_declPropInit.push_back(preProp->val());
        slotIndex.push_back(slotIndex.size());
      };

      auto const lateInitCheck = [&] (const Class::Prop& prop) {
        if ((prop.attrs ^ preProp->attrs()) & AttrLateInit) {
          raise_error(
            "Property %s::$%s must %sbe <<__LateInit>> (as in class %s)",
            m_preClass->name()->data(),
            preProp->name()->data(),
            prop.attrs & AttrLateInit ? "" : "not ",
            m_parent->name()->data()
          );
        }
      };

      auto const redeclareProp = [&] (const Slot slot) {
        // For duplicate property name, add the slot # defined in curPropMap,
        // and mark the property visited.
        assertx(serializationVisited.size() > slot);
        if (!serializationVisited[slot]) {
          curPropMap[serializationIdx++].serializationIdx = slot;
          serializationVisited[slot] = true;
        }

        auto& prop = curPropMap[slot];
        assertx((preProp->attrs() & VisibilityAttrs)
                <= (prop.attrs & VisibilityAttrs));
        assertx(!(prop.attrs & AttrNoImplicitNullable) ||
                (preProp->attrs() & AttrNoImplicitNullable));
        assertx(prop.attrs & AttrNoBadRedeclare);

        if ((preProp->attrs() & AttrIsConst) != (prop.attrs & AttrIsConst)) {
          raise_error("Cannot redeclare property %s of class %s with different "
                      "constness in class %s", preProp->name()->data(),
                      m_parent->name()->data(), m_preClass->name()->data());
        }

        lateInitCheck(prop);

        prop.preProp = preProp;
        prop.cls = this;
        if ((preProp->attrs() & VisibilityAttrs)
            < (prop.attrs & VisibilityAttrs)) {
          assertx((prop.attrs & VisibilityAttrs) == AttrProtected);
          assertx((preProp->attrs() & VisibilityAttrs) == AttrPublic);
          // Weaken protected property to public.
          prop.mangledName = preProp->mangledName();
          prop.attrs = Attr(prop.attrs ^ (AttrProtected|AttrPublic));
        }

        auto const& tc = preProp->typeConstraint();
        if (RuntimeOption::EvalCheckPropTypeHints > 0 &&
            !(preProp->attrs() & AttrNoBadRedeclare) &&
            (tc.maybeInequivalentForProp(prop.typeConstraint) ||
             !preProp->upperBounds().empty() ||
             !prop.ubs.empty())) {
          // If this property isn't obviously not redeclaring a property in
          // the parent, we need to check that when we initialize the class.
          prop.attrs = Attr(prop.attrs & ~AttrNoBadRedeclare);
          m_selfMaybeRedefsPropTy = true;
          m_maybeRedefsPropTy = true;
        }
        prop.typeConstraint = tc;

        prop.ubs = preProp->upperBounds();

        if (preProp->attrs() & AttrNoImplicitNullable) {
          prop.attrs |= AttrNoImplicitNullable;
        }
        attrSetter(prop.attrs,
                   preProp->attrs() & AttrSystemInitialValue,
                   AttrSystemInitialValue);
        if (preProp->attrs() & AttrInitialSatisfiesTC) {
          prop.attrs |= AttrInitialSatisfiesTC;
        }

        checkPrePropVal(prop, preProp);
        auto index = slotIndex[slot];

        tvCopy(preProp->val(), m_declPropInit[index].val);
        copyDeepInitAttr(preProp, &prop);
      };

      switch (preProp->attrs() & VisibilityAttrs) {
        case AttrPrivate: {
          addNewProp();
          break;
        }
        case AttrProtected: {
          // Check whether a superclass has already declared this protected
          // property.
          PropMap::Builder::iterator it2 = curPropMap.find(preProp->name());
          if (it2 == curPropMap.end()) {
            addNewProp();
          } else {
            redeclareProp(it2->second);
          }
          break;
        }
        case AttrPublic: {
          // Check whether a superclass has already declared this as a
          // protected/public property.
          auto it2 = curPropMap.find(preProp->name());
          if (it2 == curPropMap.end()) {
            addNewProp();
          } else {
            redeclareProp(it2->second);
          }
          break;
        }
        default: always_assert(false);
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
        if ((preProp->attrs() & VisibilityAttrs)
            > (parentSProp.attrs & VisibilityAttrs)) {
          raise_error(
            "Access level to %s::$%s must be %s (as in class %s) or weaker",
            m_preClass->name()->data(), preProp->name()->data(),
            attrToVisibilityStr(parentSProp.attrs),
            m_parent->name()->data());
        }
        // Prohibit overlaying LSB static properties.
        if (parentSProp.attrs & AttrLSB) {
          raise_error(
            "Cannot redeclare LSB static %s::%s as %s::%s",
            parentSProp.cls->name()->data(),
            preProp->name()->data(),
            m_preClass->name()->data(),
            preProp->name()->data());
        }
        sPropInd = it3->second;
      }

      if (sPropInd == kInvalidSlot) {
        SProp sProp;
        initProp(sProp, preProp);
        curSPropMap.add(sProp.name, sProp);

        curSPropMap[staticSerializationIdx++].serializationIdx =
          curSPropMap.size() - 1;
        staticSerializationVisited.push_back(true);
      } else {
        // Overlay ancestor's property.
        auto& sProp = curSPropMap[sPropInd];
        initProp(sProp, preProp);

        assertx(staticSerializationVisited.size() > sPropInd);
        if (!staticSerializationVisited[sPropInd]) {
          staticSerializationVisited[sPropInd] = true;
          curSPropMap[staticSerializationIdx++].serializationIdx = sPropInd;
        }
      }
    }
  }

  importTraitProps(traitIdx, curPropMap, curSPropMap, slotIndex,
                   serializationIdx, serializationVisited,
                   staticSerializationIdx, staticSerializationVisited);

  auto const pastOwnPropSlot = curPropMap.size();
  sortOwnProps(curPropMap, firstOwnPropSlot, pastOwnPropSlot, slotIndex);
  assertx(slotIndex.size() == curPropMap.size());

  // set serialization index for parent properties at the end
  if (m_parent.get() != nullptr) {
    for (Slot i = 0; i < m_parent->declProperties().size(); ++i) {
      // For non-static properties, slot is always equal to parentSlot
      Slot slot = m_parent->declProperties()[i].serializationIdx;
      assertx(serializationVisited.size() > slot);
      if (!serializationVisited[slot]) {
        curPropMap[serializationIdx++].serializationIdx = slot;
      }
    }

    for (Slot i = 0; i < m_parent->staticProperties().size(); ++i) {
      Slot parentSlot = m_parent->staticProperties()[i].serializationIdx;
      auto const& prop = m_parent->staticProperties()[parentSlot];

      if ((prop.attrs & AttrPrivate) && !(prop.attrs & AttrLSB)) continue;

      auto it = curSPropMap.find(prop.name);
      assertx(it != curSPropMap.end());

      Slot slot = it->second;
      assertx(staticSerializationVisited.size() > slot);
      if (!staticSerializationVisited[slot]) {
        curSPropMap[staticSerializationIdx++].serializationIdx = slot;
      }
    }

    assertx(serializationIdx == curPropMap.size());
    assertx(staticSerializationIdx == curSPropMap.size());
  }

  // LSB static properties that were inherited must be initialized separately.
  for (Slot slot = 0; slot < curSPropMap.size(); ++slot) {
    auto& sProp = curSPropMap[slot];
    if ((sProp.attrs & AttrLSB) && sProp.cls != this) {
      auto const& prevSProps = sProp.cls->m_staticProperties;
      auto prevInd = prevSProps.findIndex(sProp.name);
      assertx(prevInd != kInvalidSlot);
      sProp.val = prevSProps[prevInd].val;
    }
  }

  m_declProperties.create(curPropMap);
  m_staticProperties.create(curSPropMap);

  std::vector<ObjectProps::quick_index> slotQuickIndex;
  slotQuickIndex.reserve(slotIndex.size());
  for (auto i : slotIndex) {
    slotQuickIndex.push_back(ObjectProps::quickIndex(i));
  }
  m_slotIndex = slotQuickIndex;

  if (unsigned n = numStaticProperties()) {
    using LinkT = std::remove_pointer<decltype(m_sPropCache)>::type;
    m_sPropCache = static_cast<LinkT*>(vm_malloc(n * sizeof(LinkT)));
    for (unsigned i = 0; i < n; ++i) {
      new (&m_sPropCache[i]) LinkT;
    }
  }

  m_declPropNumAccessible = m_declProperties.size() - numInaccessible;

  // To speed up ObjectData::release, we only iterate over property indices
  // up to the last countable property index. Here, "index" refers to the
  // position of the property in memory, and "slot" to its logical slot.
  uint16_t countablePropsEnd = 0;
  for (Slot slot = 0; slot < m_declProperties.size(); ++slot) {
    if (jit::irgen::propertyMayBeCountable(m_declProperties[slot])) {
      auto const index =
        safe_cast<uint16_t>(propSlotToIndex(slot) + uint16_t{1});
      countablePropsEnd = std::max(countablePropsEnd, index);
    }
  }

  assertx(m_declProperties.size() <= ObjectProps::max_index + 1);
  assertx(countablePropsEnd <= ObjectProps::max_index);

  m_countablePropsEnd = ObjectProps::quickIndex(countablePropsEnd);
  FTRACE(3, "numDeclProperties = {}\n", m_declProperties.size());
  FTRACE(3, "countablePropsEnd = {}\n", countablePropsEnd);
}

bool Class::compatibleTraitPropInit(const TypedValue& tv1,
                                    const TypedValue& tv2) {
  if (tv1.m_type != tv2.m_type) return false;

  switch (tv1.m_type) {
    case KindOfNull:
      return true;

    case KindOfBoolean:
    case KindOfInt64:
    case KindOfDouble:
    case KindOfPersistentString:
    case KindOfString:
      return same(tvAsCVarRef(&tv1), tvAsCVarRef(&tv2));

    case KindOfUninit:
    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfPersistentDArray:
    case KindOfDArray:
    case KindOfPersistentVArray:
    case KindOfVArray:
    case KindOfObject:
    case KindOfResource:
    case KindOfRFunc:
    case KindOfFunc:
    case KindOfClass:
    case KindOfLazyClass:
    case KindOfClsMeth:
    case KindOfRClsMeth:
    case KindOfRecord:
      return false;
  }
  not_reached();
}

namespace {

const constexpr Attr kRedeclarePropAttrMask =
  Attr(
    ~(AttrNoBadRedeclare |
      AttrSystemInitialValue |
      AttrNoImplicitNullable |
      AttrInitialSatisfiesTC |
      AttrPersistent)
  );

}

void Class::importTraitInstanceProp(Prop& traitProp,
                                    TypedValue traitPropVal,
                                    PropMap::Builder& curPropMap,
                                    SPropMap::Builder& curSPropMap,
                                    std::vector<uint16_t>& slotIndex,
                                    Slot& serializationIdx,
                                    std::vector<bool>& serializationVisited) {
  // Check if prop already declared as static
  if (curSPropMap.find(traitProp.name) != curSPropMap.end()) {
    raise_error("trait declaration of property '%s' is incompatible with "
                "previous declaration", traitProp.name->data());
  }

  if ((attrs() & AttrIsConst) && !(traitProp.attrs & AttrIsConst)) {
    raise_error("trait's non-const declaration of property '%s' is "
                "incompatible with a const class", traitProp.name->data());
  }

  auto prevIt = curPropMap.find(traitProp.name);

  if (prevIt == curPropMap.end()) {
    // New prop, go ahead and add it
    auto prop = traitProp;
    prop.baseCls = this;
    // private props' mangled names contain the class name, so regenerate them
    if (prop.attrs & AttrPrivate) {
      prop.mangledName = PreClass::manglePropName(m_preClass->name(),
                                                  prop.name,
                                                  prop.attrs);
    }
    if (prop.attrs & AttrDeepInit) {
      m_hasDeepInitProps = true;
    }
    if (traitProp.cls != this) {
      // this was a non-flattened trait property.
      prop.cls = this;

      // Clear NoImplicitNullable on the property. HHBBC analyzed the property
      // in the context of the trait, not this class, so we cannot predict what
      // derived class' will do with it. Be conservative.
      prop.attrs = Attr(
        prop.attrs & ~AttrNoImplicitNullable) | AttrNoBadRedeclare;
    } else {
      assertx(prop.attrs & AttrNoBadRedeclare);
    }

    curPropMap.add(prop.name, prop);
    curPropMap[serializationIdx++].serializationIdx = curPropMap.size() - 1;
    serializationVisited.push_back(true);
    slotIndex.push_back(slotIndex.size());
    m_declPropInit.push_back(traitPropVal);
  } else {
    // Redeclared prop, make sure it matches previous declarations
    auto& prevProp    = curPropMap[prevIt->second];
    auto  prevPropIndex = slotIndex[prevIt->second];
    auto const prevPropVal = m_declPropInit[prevPropIndex].val.tv();
    if (((prevProp.attrs ^ traitProp.attrs) & kRedeclarePropAttrMask) ||
        (!(prevProp.attrs & AttrSystemInitialValue) &&
         !(traitProp.attrs & AttrSystemInitialValue) &&
         !compatibleTraitPropInit(prevPropVal, traitPropVal))) {
      raise_error("trait declaration of property '%s' is incompatible with "
                    "previous declaration", traitProp.name->data());
    }

    assertx(serializationVisited.size() > prevIt->second);
    if (!serializationVisited[prevIt->second]) {
      curPropMap[serializationIdx++].serializationIdx = prevIt->second;
      serializationVisited[prevIt->second] = true;
    }
  }
}

void Class::importTraitStaticProp(
  SProp& traitProp,
  PropMap::Builder& curPropMap,
  SPropMap::Builder& curSPropMap,
  Slot& staticSerializationIdx,
  std::vector<bool>& staticSerializationVisited) {
  // Check if prop already declared as non-static
  if (curPropMap.find(traitProp.name) != curPropMap.end()) {
    raise_error("trait declaration of property '%s' is incompatible with "
                "previous declaration", traitProp.name->data());
  }

  auto prevIt = curSPropMap.find(traitProp.name);
  if (prevIt == curSPropMap.end()) {
    // New prop, go ahead and add it
    auto prop = traitProp;
    prop.cls = this; // set current class as the first declaring prop
    prop.attrs |= AttrNoBadRedeclare;

    curSPropMap.add(prop.name, prop);
    curSPropMap[staticSerializationIdx++].serializationIdx =
      curSPropMap.size() - 1;
    staticSerializationVisited.push_back(true);
  } else {
    // Redeclared prop, make sure it matches previous declaration
    auto& prevProp = curSPropMap[prevIt->second];
    TypedValue prevPropVal;
    if (prevProp.attrs & AttrLSB) {
      raise_error("trait declaration of property '%s' would redeclare "
                  "LSB static", traitProp.name->data());
    }
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
      assertx(prevPropInd != kInvalidSlot);

      prevPropVal = prevSProps[prevPropInd].val;
    }
    if (((prevProp.attrs ^ traitProp.attrs) & kRedeclarePropAttrMask) ||
        (!(prevProp.attrs & AttrSystemInitialValue) &&
         !(traitProp.attrs & AttrSystemInitialValue) &&
         !compatibleTraitPropInit(traitProp.val, prevPropVal))) {
      raise_error("trait declaration of property '%s' is incompatible with "
                  "previous declaration", traitProp.name->data());
    }
    prevProp.cls = this;
    prevProp.val = prevPropVal;

    assertx(staticSerializationVisited.size() > prevIt->second);
    if (!staticSerializationVisited[prevIt->second]) {
      curSPropMap[staticSerializationIdx++].serializationIdx = prevIt->second;
      staticSerializationVisited[prevIt->second] = true;
    }
  }
}

template<typename XProp>
void Class::checkPrePropVal(XProp& prop, const PreClass::Prop* preProp) {
  auto const& tv = preProp->val();
  auto const& tc = preProp->typeConstraint();

  assertx(
    !(preProp->attrs() & AttrSystemInitialValue) ||
    tv.m_type != KindOfNull ||
    !(preProp->attrs() & AttrNoImplicitNullable)
  );

  auto const alwaysPassesAll = [&] {
    if (!tc.alwaysPasses(&tv)) return false;
    if (RuntimeOption::EvalEnforceGenericsUB > 0) {
      auto& ubs = const_cast<PreClass::UpperBoundVec&>(preProp->upperBounds());
      for (auto& ub : ubs) {
        if (!ub.alwaysPasses(&tv)) return false;
      }
    }
    return true;
  };

  if (RuntimeOption::EvalCheckPropTypeHints > 0 &&
      !(preProp->attrs() & AttrInitialSatisfiesTC) &&
      tv.m_type != KindOfUninit) {
    // System provided initial values should always be correct
    if ((preProp->attrs() & AttrSystemInitialValue) || alwaysPassesAll()) {
      prop.attrs |= AttrInitialSatisfiesTC;
    } else if (preProp->attrs() & AttrStatic) {
      prop.attrs = Attr(prop.attrs & ~AttrPersistent);
    } else {
      m_needsPropInitialCheck = true;
    }
  }
}

template<typename XProp>
void Class::initProp(XProp& prop, const PreClass::Prop* preProp) {
  prop.preProp             = preProp;
  prop.name                = preProp->name();
  prop.attrs               = Attr(preProp->attrs() & ~AttrTrait) |
                             AttrNoBadRedeclare;
  // This is the first class to declare this property
  prop.cls                 = this;
  prop.typeConstraint      = preProp->typeConstraint();
  prop.ubs                 = preProp->upperBounds();
  prop.repoAuthType        = preProp->repoAuthType();

  // If type constraint has soft or nullable flags, apply them to upper-bounds.
  for (auto &ub : prop.ubs) applyFlagsToUB(ub, prop.typeConstraint);
  // Check if this property's initial value needs to be type checked at
  // runtime.
  checkPrePropVal(prop, preProp);
}

void Class::initProp(Prop& prop, const PreClass::Prop* preProp) {
  initProp<Prop>(prop, preProp);
  prop.mangledName = preProp->mangledName();
  prop.baseCls     = this;
}

void Class::initProp(SProp& prop, const PreClass::Prop* preProp) {
  initProp<SProp>(prop, preProp);
  prop.val = preProp->val();
}

void Class::importTraitProps(int traitIdx,
                             PropMap::Builder& curPropMap,
                             SPropMap::Builder& curSPropMap,
                             std::vector<uint16_t>& slotIndex,
                             Slot& serializationIdx,
                             std::vector<bool>& serializationVisited,
                             Slot& staticSerializationIdx,
                             std::vector<bool>& staticSerializationVisited) {
  if (attrs() & AttrNoExpandTrait) {
    for (Slot p = traitIdx; p < m_preClass->numProperties(); p++) {
      auto const* preProp = &m_preClass->properties()[p];
      assertx(preProp->attrs() & AttrTrait);
      if (!(preProp->attrs() & AttrStatic)) {
        Prop prop;
        initProp(prop, preProp);
        importTraitInstanceProp(prop, preProp->val(),
                                curPropMap, curSPropMap, slotIndex,
                                serializationIdx, serializationVisited);
      } else {
        SProp prop;
        initProp(prop, preProp);
        importTraitStaticProp(
          prop, curPropMap, curSPropMap,
          staticSerializationIdx, staticSerializationVisited);
      }
    }
    return;
  }

  for (auto const& t : m_extra->m_usedTraits) {
    auto trait = t.get();

    m_needsPropInitialCheck |= trait->m_needsPropInitialCheck;

    // instance properties
    for (Slot p = 0; p < trait->m_declProperties.size(); p++) {
      auto& traitProp    = trait->m_declProperties[p];
      auto  traitPropIndex = trait->propSlotToIndex(p);
      auto traitPropVal = trait->m_declPropInit[traitPropIndex].val.tv();
      importTraitInstanceProp(traitProp, traitPropVal,
                              curPropMap, curSPropMap, slotIndex,
                              serializationIdx, serializationVisited);
    }

    // static properties
    for (Slot p = 0; p < trait->m_staticProperties.size(); ++p) {
      auto& traitProp = trait->m_staticProperties[p];
      importTraitStaticProp(
        traitProp, curPropMap, curSPropMap,
        staticSerializationIdx, staticSerializationVisited);
    }
  }
}

void Class::addTraitPropInitializers(std::vector<const Func*>& thisInitVec,
                                     Attr which) {
  if (attrs() & AttrNoExpandTrait) return;
  auto getInitVec = [&](Class* trait) -> auto& {
    switch (which) {
    case AttrNone: return trait->m_pinitVec;
    case AttrStatic: return trait->m_sinitVec;
    case AttrLSB: return trait->m_linitVec;
    default: always_assert(false);
    }
  };
  for (auto const& t : m_extra->m_usedTraits) {
    Class* trait = t.get();
    auto& traitInitVec = getInitVec(trait);
    // Insert trait's 86[psl]init into the current class, avoiding repetitions.
    for (unsigned m = 0; m < traitInitVec.size(); m++) {
      // Clone 86[psl]init methods, and set the class to the current class.
      // This allows 86[psl]init to determine the property offset for the
      // initializer array corectly.
      Func *f = traitInitVec[m]->clone(this);
      f->setNewFuncId();
      f->setBaseCls(this);
      f->setHasPrivateAncestor(false);
      thisInitVec.push_back(f);
    }
  }
}

void Class::setReifiedData() {
  auto const ua = m_preClass->userAttributes();
  auto const it = ua.find(s___Reified.get());
  if (it != ua.end()) m_hasReifiedGenerics = true;
  if (m_parent.get()) {
    m_hasReifiedParent =
      m_parent->m_hasReifiedGenerics || m_parent->m_hasReifiedParent;
  }
  if (m_hasReifiedGenerics) {
    auto tv = it->second;
    assertx(tvIsHAMSafeVArray(tv));
    allocExtraData();
    m_extra.raw()->m_reifiedGenericsInfo =
      extractSizeAndPosFromReifiedAttribute(tv.m_data.parr);
  }
}

namespace {
  const StaticString s_Error("Error");
  const StaticString s_Exception("Exception");
}

void Class::setInitializers() {
  std::vector<const Func*> pinits;
  std::vector<const Func*> sinits;
  std::vector<const Func*> linits;

  if (m_parent.get() != nullptr) {
    // Copy parent's 86pinit() vector, so that the 86pinit() methods can be
    // called in reverse order without any search/recursion during
    // initialization.
    pinits.assign(m_parent->m_pinitVec.begin(), m_parent->m_pinitVec.end());

    // Copy parent's 86linit into the current class, and set class to the
    // current class.
    for (const auto& linit : m_parent->m_linitVec) {
      Func *f = linit->clone(this);
      f->setNewFuncId();
      f->setBaseCls(this);
      f->setHasPrivateAncestor(false);
      linits.push_back(f);
    }
  }

  // Clone 86pinit methods from traits
  addTraitPropInitializers(pinits, AttrNone);

  // If this class has an 86pinit method, append it last so that
  // reverse iteration of the vector runs this class's 86pinit
  // first, in case multiple classes in the hierarchy initialize
  // the same property.
  const Func* meth86pinit = findSpecialMethod(this, s_86pinit.get());
  if (meth86pinit != nullptr) {
    pinits.push_back(meth86pinit);
  }

  // If this class has an 86sinit method, it must be the last element
  // in the vector. See get86sinit().
  addTraitPropInitializers(sinits, AttrStatic);
  const Func* sinit = findSpecialMethod(this, s_86sinit.get());
  if (sinit) {
    sinits.push_back(sinit);
  }

  addTraitPropInitializers(linits, AttrLSB);
  const Func* meth86linit = findSpecialMethod(this, s_86linit.get());
  if (meth86linit != nullptr) {
    linits.push_back(meth86linit);
  }

  m_pinitVec = pinits;
  m_sinitVec = sinits;
  m_linitVec = linits;

  m_needInitialization =
    (m_pinitVec.size() > 0 ||
     m_staticProperties.size() > 0 ||
     m_maybeRedefsPropTy ||
     m_needsPropInitialCheck);

  if (m_maybeRedefsPropTy || m_needsPropInitialCheck) allocExtraData();

  // Implementations of Throwable get special treatment.
  if (m_parent.get() != nullptr) {
    m_needsInitThrowable = m_parent->needsInitThrowable();
  } else {
    m_needsInitThrowable = name()->same(s_Exception.get()) ||
                           name()->same(s_Error.get());
  }
}

const StaticString s_HH_Iterator("HH\\Iterator");
const StaticString s_IteratorAggregate("IteratorAggregate");
void Class::checkInterfaceConstraints() {
  if (UNLIKELY(m_interfaces.contains(s_HH_Iterator.get()) &&
      m_interfaces.contains(s_IteratorAggregate.get()))) {
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
    auto const trait = Class::lookup(traitName);
    assertx(trait->attrs() & AttrTrait);
    int numIfcs = trait->m_interfaces.size();

    for (int i = 0; i < numIfcs; i++) {
      auto interface = trait->m_interfaces[i];
      if (!builder.contains(interface->name())) {
        builder.add(interface->name(), interface);
      }
    }
  }
}

const StaticString s_Stringish("Stringish");
const StaticString s_XHPChild("XHPChild");

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
    auto cp = Class::load(*it);
    if (cp == nullptr) {
      raise_error("Undefined interface: %s", (*it)->data());
    }
    if (!(cp->attrs() & AttrInterface)) {
      raise_error("%s cannot implement %s - it is not an interface",
                  m_preClass->name()->data(), cp->name()->data());
    }
    m_preClass->enforceInMaybeSealedParentWhitelist(cp->preClass());
    declInterfaces.push_back(ClassPtr(cp));
    if (!interfacesBuilder.contains(cp->name())) {
      interfacesBuilder.add(cp->name(), LowPtr<Class>(cp));
    }
    int size = cp->m_interfaces.size();
    for (int i = 0; i < size; i++) {
      auto interface = cp->m_interfaces[i];
      if (!interfacesBuilder.contains(interface->name())) {
        interfacesBuilder.add(interface->name(), interface);
      }
    }
  }

  if (auto sz = declInterfaces.size()) {
    m_declInterfaces.reserve(sz);
    for (auto interface : declInterfaces) {
      m_declInterfaces.push_back(interface);
    }
  }

  addInterfacesFromUsedTraits(interfacesBuilder);

  if (m_toString) {
    if (!interfacesBuilder.contains(s_Stringish.get()) &&
        (!(attrs() & AttrInterface) ||
         !m_preClass->name()->isame(s_Stringish.get()))) {
      // Add Stringish
      Class* stringish = Class::lookup(s_Stringish.get());
      assertx(stringish != nullptr);
      assertx((stringish->attrs() & AttrInterface));
      interfacesBuilder.add(stringish->name(), LowPtr<Class>(stringish));

      if (!m_preClass->name()->isame(s_XHPChild.get()) &&
          !interfacesBuilder.contains(s_XHPChild.get())) {
        // All Stringish are also XHPChild
        Class* xhpChild = Class::lookup(s_XHPChild.get());
        assertx(xhpChild != nullptr);
        assertx((xhpChild->attrs() & AttrInterface));
        interfacesBuilder.add(xhpChild->name(), LowPtr<Class>(xhpChild));
      }
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
  auto const mem = static_cast<char*>(low_malloc(memSz));
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
    if (vtableVec[slot].vtable != nullptr) {
      raise_error("Static analysis failure: "
                  "%s was expected to fatal at runtime, but didn't",
                  m_preClass->name()->data());
    }
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
      assertx(req.is_extends());
      auto const reqName = req.name();
      auto const reqCls = Class::lookup(reqName);
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
  } else if (attrs() & (AttrTrait | AttrNoExpandTrait)) {
    // Check that requirements are semantically valid.
    // Note that trait flattening could have migrated
    // requirements onto a class's preClass.
    for (auto const& req : m_preClass->requirements()) {
      auto const reqName = req.name();
      if (auto const reqCls = Class::lookup(reqName)) {
        if (req.is_extends()) {
          if (reqCls->attrs() & (AttrTrait | AttrInterface | AttrFinal)) {
            raise_error(Strings::TRAIT_BAD_REQ_EXTENDS,
                        m_preClass->name()->data(),
                        reqName->data(),
                        reqName->data());
          }
        } else {
          assertx(req.is_implements());
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

void Class::setLSBMemoCacheInfo() {
  boost::container::flat_map<FuncId, Slot> slots;
  Slot numSlots = 0;

  /* Inherit slots from parent */
  if (m_parent && m_parent->m_extra) {
    numSlots = m_parent->m_extra->m_lsbMemoExtra.m_numSlots;
  }

  /* If any of our methods need slots, insert them at the end */
  auto const methodCount = numMethods();
  uint64_t assignCount = 0;
  for (Slot i = 0; i < methodCount; ++i) {
    auto const f = getMethod(i);
    if (f->isMemoizeWrapperLSB() && f->cls() == this) {
      slots.emplace(f->getFuncId(), numSlots++);
      ++assignCount;
    }
  }
  always_assert(assignCount == slots.size());

  if (numSlots != 0) {
    allocExtraData();
    auto& mx = m_extra->m_lsbMemoExtra;
    mx.m_slots = std::move(slots);
    mx.m_numSlots = numSlots;
    initLSBMemoHandles();
  }
}

void Class::initLSBMemoHandles() {
  /* Allocate handles array */
  auto& mx = m_extra->m_lsbMemoExtra;
  auto const numSlots = mx.m_numSlots;
  assertx(numSlots > 0);
  rds::Handle* handles = static_cast<rds::Handle*>(
    vm_malloc(numSlots * sizeof(rds::Handle)));

  /* Assign handle to every slot */
  uint64_t assignCount = 0;
  uint64_t assignSum = 0;
  const Class* cls = this;
  while (cls != nullptr && cls->m_extra) {
    for (const auto& kv : cls->m_extra->m_lsbMemoExtra.m_slots) {
      auto const func = Func::fromFuncId(kv.first);
      auto const slot = kv.second;
      assertx(slot >= 0 && slot < numSlots);
      if (func->numParams() == 0) {
        handles[slot] = rds::bindLSBMemoValue(this, func).handle();
      } else {
        handles[slot] = rds::bindLSBMemoCache(this, func).handle();
      }
      ++assignCount;
      assignSum += slot;
    }
    cls = cls->m_parent.get();
  }
  /* Sanity check: Make sure we assigned every slot exactly once */
  always_assert(numSlots == assignCount);
  always_assert(assignSum == ((numSlots - 1) * numSlots) / 2);

  assertx(mx.m_handles == nullptr);
  mx.m_handles = handles;
}

void Class::setInstanceMemoCacheInfo() {
  // Inherit the data from the parent, if any.
  if (m_parent && m_parent->hasMemoSlots()) {
    allocExtraData();
    m_extra.raw()->m_nextMemoSlot = m_parent->m_extra->m_nextMemoSlot;
    m_extra.raw()->m_sharedMemoSlots = m_parent->m_extra->m_sharedMemoSlots;
  }

  // If we have memo slots defined already, and this class is adding (or
  // changing) native data, forbid it. The parent class won't realize the native
  // data has changed size, so code we generate in it to access the slot will be
  // incorrect (it won't find the slot in the right place). If we ever have a
  // need to actually do this, we'll have to revisit this.
  if (m_preClass->nativeDataInfo() && m_extra->m_nextMemoSlot > 0) {
    raise_error(
      "Class '%s' with existing memoized methods cannot have native-data added",
      m_preClass->name()->data()
    );
  }

  auto const forEachMeth = [&](auto f) {
    auto const methodCount = numMethods();
    for (Slot i = 0; i < methodCount; ++i) {
      auto const m = getMethod(i);
      if (m->isMemoizeWrapper() && !m->isStatic() && m->cls() == this) f(m);
    }
  };

  auto const addNonShared = [&](const Func* f) {
    allocExtraData();
    auto extra = m_extra.raw();
    extra->m_memoMappings.emplace(
      f->getFuncId(),
      std::make_pair(extra->m_nextMemoSlot++, false)
    );
  };

  auto const addShared = [&](const Func* f) {
    allocExtraData();
    auto extra = m_extra.raw();

    // There's no point in allocating slots for parameter counts greater than
    // kMemoCacheMaxSpecializedKeys, since they'll all be generic caches
    // anyways. Cap the count at that.
    auto const keyCount =
      std::min<size_t>(f->numParams(), kMemoCacheMaxSpecializedKeys + 1);
    auto const it = extra->m_sharedMemoSlots.find(keyCount);
    if (it != extra->m_sharedMemoSlots.end()) {
      extra->m_memoMappings.emplace(
        f->getFuncId(),
        std::make_pair(it->second, true)
      );
      return;
    }

    extra->m_sharedMemoSlots.emplace(keyCount, extra->m_nextMemoSlot);
    extra->m_memoMappings.emplace(
      f->getFuncId(),
      std::make_pair(extra->m_nextMemoSlot++, true)
    );
  };

  // First count how many methods we have without parameters or with parameters.
  size_t methNoKeys = 0;
  size_t methWithKeys = 0;
  forEachMeth(
    [&](const Func* f) {
      if (f->numParams() == 0) {
        ++methNoKeys;
      } else {
        ++methWithKeys;
      }
    }
  );

  // We only want to assign up to EvalNonSharedInstanceMemoCaches non-shared
  // caches. With the remaining non-assigned slots, we give preference to the
  // parameter-less methods first. This is because there's a greater benefit to
  // giving parameter-less methods non-shared slots (they can just be a
  // TypedValue). However, we only give the methods non-shared slots if we can give
  // them to all the methods. If there's more methods than we have available
  // non-shared slots, its not clear which ones should get the non-shared slots
  // and which ones shouldn't, so we treat them all equally and give them all
  // shared slots. After processing the parameter-less methods, we apply the
  // same logic to the methods with parameters.

  // How many non-shared slots do we have left to allocate?
  auto slotsLeft =
    RuntimeOption::EvalNonSharedInstanceMemoCaches -
    std::min(
      m_extra->m_nextMemoSlot,
      RuntimeOption::EvalNonSharedInstanceMemoCaches
    );

  if (methNoKeys > 0) {
    forEachMeth(
      [&](const Func* f) {
        if (f->numParams() == 0) {
          (methNoKeys <= slotsLeft) ? addNonShared(f) : addShared(f);
        }
      }
    );
    if (methNoKeys <= slotsLeft) slotsLeft -= methNoKeys;
  }

  if (methWithKeys > 0) {
    forEachMeth(
      [&](const Func* f) {
        if (f->numParams() > 0) {
          (methWithKeys <= slotsLeft) ? addNonShared(f) : addShared(f);
        }
      }
    );
  }
}

namespace {
ObjectData* throwingInstanceCtor(Class* cls) {
  SystemLib::throwExceptionObject(
    folly::sformat("Class {} may not be directly instantiated", cls->name())
  );
}
} // namespace

void Class::setNativeDataInfo() {
  for (auto cls = this; cls; cls = cls->parent()) {
    if (auto ndi = cls->preClass()->nativeDataInfo()) {
      allocExtraData();
      m_extra.raw()->m_nativeDataInfo = ndi;
      if (ndi->ctor_throws) {
        m_extra.raw()->m_instanceCtor = throwingInstanceCtor;
        m_extra.raw()->m_instanceCtorUnlocked = throwingInstanceCtor;
      } else {
        m_extra.raw()->m_instanceCtor = Native::nativeDataInstanceCtor<false>;
        m_extra.raw()->m_instanceCtorUnlocked =
          Native::nativeDataInstanceCtor<true>;
      }
      m_extra.raw()->m_instanceDtor = Native::nativeDataInstanceDtor;
      m_releaseFunc = Native::nativeDataInstanceDtor;
      m_RTAttrs |= ndi->rt_attrs;
      break;
    }
  }
}

void Class::initClosure() {
  if (!isBuiltin() || !m_preClass->name()->equal(s_Closure.get())) return;

  c_Closure::cls_Closure = this;
  assertx(!hasMemoSlots());
  allocExtraData();
  auto& extraData = *m_extra.raw();
  extraData.m_instanceCtor = c_Closure::instanceCtor;
  extraData.m_instanceCtorUnlocked = c_Closure::instanceCtor;
  extraData.m_instanceDtor = c_Closure::instanceDtor;
  m_releaseFunc = c_Closure::instanceDtor;
}

bool Class::hasNativePropHandler() const {
  return m_RTAttrs & Class::HasNativePropHandler;
}

const Native::NativePropHandler* Class::getNativePropHandler() const {
  assertx(hasNativePropHandler());

  for (auto cls = this; cls; cls = cls->parent()) {
    auto propHandler = Native::getNativePropHandler(cls->name());
    if (propHandler != nullptr) {
      return propHandler;
    }
  }

  not_reached();
}

void Class::raiseUnsatisfiedRequirement(const PreClass::ClassRequirement* req)  const {
  assertx(!(attrs() & (AttrInterface | AttrTrait)));

  auto const reqName = req->name();
  if (req->is_implements()) {
    // "require implements" is only allowed on traits.

    assertx(attrs() & AttrNoExpandTrait ||
           (m_extra && m_extra->m_usedTraits.size() > 0));
    for (auto const& traitCls : m_extra->m_usedTraits) {
      if (traitCls->allRequirements().contains(reqName)) {
        raise_error(Strings::TRAIT_REQ_IMPLEMENTS,
                    m_preClass->name()->data(),
                    reqName->data(),
                    traitCls->preClass()->name()->data());
      }
    }

    if (attrs() & AttrNoExpandTrait) {
      // As a result of trait flattening, the PreClass of this normal class
      // contains a requirement. To save space, we don't include the source
      // trait in the requirement. For details, see
      // ClassScope::importUsedTraits in the compiler.
      assertx(!m_extra || m_extra->m_usedTraits.size() == 0);
      assertx(m_preClass->requirements().size() > 0);
      raise_error(Strings::TRAIT_REQ_IMPLEMENTS,
                  m_preClass->name()->data(),
                  reqName->data(),
                  "<<flattened>>");
    }

    always_assert(false); // requirements cannot spontaneously generate
    return;
  }

  assertx(req->is_extends());
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

  if (attrs() & AttrNoExpandTrait) {
    // A result of trait flattening, as with the is_implements case above
    assertx(!m_extra || m_extra->m_usedTraits.size() == 0);
    assertx(m_preClass->requirements().size() > 0);
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
      auto reqExtCls = Class::lookup(reqName);
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
    assertx(m_parent.get() != nullptr);
    memcpy(m_classVec, m_parent->m_classVec,
           (m_classVecLen-1) * sizeof(m_classVec[0]));
  }
  m_classVec[m_classVecLen-1] = this;
}

void Class::setFuncVec(MethodMapBuilder& builder) {
  auto funcVec = this->funcVec();

  memset(funcVec, 0, m_funcVecLen * sizeof(LowPtr<Func>));

  funcVec = (LowPtr<Func>*)this;
  assertx(builder.size() <= m_funcVecLen);

  for (Slot i = 0; i < builder.size(); i++) {
    assertx(builder[i]->methodSlot() < builder.size());
    funcVec[-((int32_t)builder[i]->methodSlot() + 1)] = builder[i];
  }
}

void Class::setReleaseData() {
  if (getNativeDataInfo()) return;
  if (hasMemoSlots()) {
    m_memoSize = ObjectData::objOffFromMemoNode(this);
  }
  auto const nProps = numDeclProperties();
  auto const size = m_memoSize + ObjectData::sizeForNProps(nProps);
  m_sizeIdx = MemoryManager::size2Index(size);
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
        out.set(lowerName, methName);
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
  for (auto& iface : cls->m_declInterfaces) {
    getMethodNames(iface.get(), ctx, out);
  }
}


///////////////////////////////////////////////////////////////////////////////
// Trait method import.

namespace {

struct TraitMethod {
  TraitMethod(const Class* trait_, const Func* method_, Attr modifiers_)
      : trait(trait_)
      , method(method_)
      , modifiers(modifiers_)
    {}

  using class_type = const Class*;
  using method_type = const Func*;

  const Class* trait;
  const Func* method;
  Attr modifiers;
};

struct TMIOps {
  // Return the name for the trait class.
  static const StringData* clsName(const Class* traitCls) {
    return traitCls->name();
  }

  static const StringData* methName(const Func* method) {
    return method->name();
  }

  // Is-a methods.
  static bool isTrait(const Class* traitCls) {
    return traitCls->attrs() & AttrTrait;
  }

  static bool isAbstract(Attr modifiers) {
    return modifiers & AttrAbstract;
  }

  // Whether to exclude methods with name `methName' when adding.
  static bool exclude(const StringData* methName) {
    return Func::isSpecial(methName);
  }

  // TraitMethod constructor.
  static TraitMethod traitMethod(const Class* traitCls,
                                 const Func* traitMeth,
                                 const PreClass::TraitAliasRule& rule) {
    return TraitMethod { traitCls, traitMeth, rule.modifiers() };
  }

  // Register a trait alias once the trait class is found.
  static void addTraitAlias(const Class* cls,
                            const PreClass::TraitAliasRule& rule,
                            const Class* traitCls) {
    PreClass::TraitAliasRule newRule { traitCls->name(),
        rule.origMethodName(),
        rule.newMethodName(),
        rule.modifiers() };
    cls->addTraitAlias(newRule);
  }

  // Trait class/method finders.
  static const Class* findSingleTraitWithMethod(const Class* cls,
                                                const StringData* methName)  {
    Class* traitCls = nullptr;

    for (auto const& t : cls->usedTraitClasses()) {
      // Note: m_methods includes methods from parents/traits recursively.
      if (t->lookupMethod(methName)) {
        if (traitCls != nullptr) {
          raise_error("more than one trait contains method '%s'",
                      methName->data());
        }
        traitCls = t.get();
      }
    }
    return traitCls;
  }

  static const Class* findTraitClass(const Class* cls,
                                     const StringData* traitName) {
    auto ret = Class::load(traitName);
    if (!ret) return nullptr;
    auto const& usedTraits = cls->preClass()->usedTraits();
    if (std::find_if(usedTraits.begin(), usedTraits.end(),
                     [&] (auto const name) {
                       return traitName->isame(name);
                     }) == usedTraits.end()) {
      return nullptr;
    }
    return ret;
  }
  static const Func* findTraitMethod(const Class* traitCls,
                                     const StringData* origMethName) {
    return traitCls->lookupMethod(origMethName);
  }

  // Errors.
  static void errorUnknownMethod(const StringData* methName) {
    raise_error(Strings::TRAITS_UNKNOWN_TRAIT_METHOD, methName->data());
  }

  static void errorUnknownTrait(const StringData* traitName) {
    raise_error(Strings::TRAITS_UNKNOWN_TRAIT, traitName->data());
  }
  static void errorDuplicateMethod(const Class* cls,
                                   const StringData* methName,
                                   const std::list<TraitMethod>& methods) {
    // No error if the class will override the method.
    if (cls->preClass()->hasMethod(methName)) return;

    std::vector<folly::StringPiece> traitNames;
    for (auto& method : methods) {
      traitNames.push_back(method.trait->name()->slice());
    }
    std::string traits;
    folly::join(", ", traitNames, traits);

    raise_error(Strings::METHOD_IN_MULTIPLE_TRAITS, methName->data(), traits.c_str());
  }
  static void errorInconsistentInsteadOf(const Class* cls,
                                         const StringData* methName) {
    raise_error(Strings::INCONSISTENT_INSTEADOF, methName->data(),
                cls->name()->data(), cls->name()->data());
  }
  static void errorMultiplyExcluded(const StringData* traitName,
                                    const StringData* methName) {
    raise_error(Strings::MULTIPLY_EXCLUDED,
                traitName->data(), methName->data());
  }
};

using TMIData = TraitMethodImportData<TraitMethod, TMIOps>;

void applyTraitRules(Class* cls, TMIData& tmid) {
  for (auto const& precRule : cls->preClass()->traitPrecRules()) {
    tmid.applyPrecRule(precRule, cls);
  }
  for (auto const& aliasRule : cls->preClass()->traitAliasRules()) {
    tmid.applyAliasRule(aliasRule, cls);
  }
}

void importTraitMethod(Class* cls,
                       const TMIData::MethodData& mdata,
                       Class::MethodMapBuilder& builder) {
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
    if (existingMethod->cls() == cls) {
      // Don't override an existing method if this class provided an
      // implementation
      return;
    }
    parentMethod = existingMethod;
  }
  Func* f = method->clone(cls, mdata.name);
  f->setNewFuncId();
  f->setAttrs(modifiers | AttrTrait);
  if (!parentMethod) {
    // New method
    builder.add(mdata.name, f);
    f->setBaseCls(cls);
    f->setHasPrivateAncestor(false);
  } else {
    // Override an existing method
    Class* baseClass;

    cls->methodOverrideCheck(parentMethod, f);

    assertx(!(f->attrs() & AttrPrivate) ||
           (parentMethod->attrs() & AttrPrivate));
    if ((parentMethod->attrs() & AttrPrivate) || (f->attrs() & AttrPrivate)) {
      baseClass = cls;
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

}

void Class::importTraitMethods(MethodMapBuilder& builder) {
  TMIData tmid;

  // Find all methods to be imported.
  for (auto const& t : m_extra->m_usedTraits) {
    Class* trait = t.get();
    for (Slot i = 0; i < trait->m_methods.size(); ++i) {
      auto const method = trait->getMethod(i);

      TraitMethod traitMethod { trait, method, method->attrs() };
      tmid.add(traitMethod, method->name());
    }
  }

  // Apply trait rules and import the methods.
  applyTraitRules(this, tmid);
  auto traitMethods = tmid.finish(this);

  // Import the methods.
  for (auto const& mdata : traitMethods) {
    importTraitMethod(this, mdata, builder);
  }
}

///////////////////////////////////////////////////////////////////////////////
// Lookup.

namespace {

void setupClass(Class* newClass, NamedEntity* nameList) {
  bool const isPersistent =
    (!SystemLib::s_inited || RuntimeOption::RepoAuthoritative) &&
    newClass->verifyPersistent();
  nameList->m_cachedClass.bind(
    isPersistent ? rds::Mode::Persistent : rds::Mode::Normal,
    rds::LinkName{"NEClass", newClass->name()}
  );

  if (newClass->isBuiltin()) {
    assertx(newClass->isUnique());
    for (auto i = newClass->numMethods(); i--;) {
      auto const func = newClass->getMethod(i);
      if (func->isCPPBuiltin() && func->isStatic()) {
        assertx(func->isUnique());
        NamedEntity::get(func->fullName())->setUniqueFunc(func);
      }
    }
  }

  newClass->setClassHandle(nameList->m_cachedClass);
  newClass->incAtomicCount();

  InstanceBits::ifInitElse(
    [&] { newClass->setInstanceBits();
          nameList->pushClass(newClass); },
    [&] { nameList->pushClass(newClass); }
  );

  if (RuntimeOption::EvalEnableReverseDataMap) {
    // The corresponding deregister is in NamedEntity::removeClass().
    data_map::register_start(newClass);
    for (unsigned i = 0, n = newClass->numMethods(); i < n; i++) {
      if (auto meth = newClass->getMethod(i)) {
        if (meth->cls() == newClass) {
          meth->registerInDataMap();
        }
      }
    }
  }
}

}

Class* Class::def(const PreClass* preClass, bool failIsFatal /* = true */) {
  FTRACE(3, "  Defining cls {} failIsFatal {}\n",
         preClass->name()->data(), failIsFatal);
  NamedEntity* const nameList = preClass->namedEntity();
  Class* top = nameList->clsList();

  /*
   * Check if there is already a name defined in this request for this
   * NamedEntity.
   *
   * Raise a fatal unless the existing class definition is identical to the
   * one this invocation would create.
   */
  auto existingKind = nameList->checkSameName<PreClass>();
  if (existingKind) {
    FrameRestore fr(preClass);
    raise_error("Cannot declare class with the same name (%s) as an "
                "existing %s", preClass->name()->data(), existingKind);
    return nullptr;
  }

  // If there was already a class declared with DefClass, check if it's
  // compatible.
  if (Class* cls = nameList->getCachedClass()) {
    if (cls->preClass() != preClass) {
      if (failIsFatal) {
        FrameRestore fr(preClass);
        raise_error("Class already declared: %s", preClass->name()->data());
      }
      return nullptr;
    }
    assertx(!RO::RepoAuthoritative ||
            (cls->isPersistent() && classHasPersistentRDS(cls)));
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
      class_ = class_->m_next;
      if (cur->preClass() != preClass) continue;
      Class::Avail avail = cur->avail(parent, failIsFatal /*tryAutoload*/);
      if (LIKELY(avail == Class::Avail::True)) {
        cur->setCached();
        DEBUGGER_ATTACHED_ONLY(phpDebuggerDefClassHook(cur));
        assertx(!RO::RepoAuthoritative ||
                (cur->isPersistent() && classHasPersistentRDS(cur)));
        return cur;
      }
      if (avail == Class::Avail::Fail) {
        if (failIsFatal) {
          FrameRestore fr(preClass);
          raise_error("unknown class %s", parent->name()->data());
        }
        return nullptr;
      }
      assertx(avail == Class::Avail::False);
    }

    if (!parent && preClass->parent()->size() != 0) {
      parent = Class::get(preClass->parent(), failIsFatal);
      if (parent == nullptr) {
        if (failIsFatal) {
          FrameRestore fr(preClass);
          raise_error("unknown class %s", preClass->parent()->data());
        }
        return nullptr;
      }
    }

    if (!failIsFatal) {
      // Check interfaces
      for (auto it = preClass->interfaces().begin();
                 it != preClass->interfaces().end(); ++it) {
        if (!Class::get(*it, false)) return nullptr;
      }
      // traits
      for (auto const& traitName : preClass->usedTraits()) {
        if (!Class::get(traitName, false)) return nullptr;
      }
      // enum
      if (preClass->attrs() & AttrEnum) {
        auto const enumBaseTy =
          preClass->enumBaseTy().underlyingDataTypeResolved();
        if (!enumBaseTy ||
            (!isIntType(*enumBaseTy) && !isStringType(*enumBaseTy))) {
          return nullptr;
        }
      }
    }

    // Create a new class.
    ClassPtr newClass;
    {
      FrameRestore fr(preClass);
      newClass = Class::newClass(const_cast<PreClass*>(preClass), parent);
    }
    Lock l(g_classesMutex);

    if (UNLIKELY(top != nameList->clsList())) {
      top = nameList->clsList();
      continue;
    }

    setupClass(newClass.get(), nameList);

    /*
     * call setCached after adding to the class list, otherwise the
     * target-cache short circuit at the top could return a class
     * which is not yet on the clsList().
     */
    newClass.get()->setCached();
    DEBUGGER_ATTACHED_ONLY(phpDebuggerDefClassHook(newClass.get()));
    assertx(!RO::RepoAuthoritative ||
            (newClass.get()->isPersistent() &&
             classHasPersistentRDS(newClass.get())));
    return newClass.get();
  }
}

Class* Class::defClosure(const PreClass* preClass) {
  auto const nameList = preClass->namedEntity();

  if (nameList->clsList()) return nameList->clsList();

  auto const parent = c_Closure::classof();

  assertx(preClass->parent() == parent->name());
  // Create a new class.

  ClassPtr newClass {
    Class::newClass(const_cast<PreClass*>(preClass), parent)
  };

  Lock l(g_classesMutex);

  if (UNLIKELY(nameList->clsList() != nullptr)) return nameList->clsList();

  setupClass(newClass.get(), nameList);

  if (classHasPersistentRDS(newClass.get())) newClass.get()->setCached();
  return newClass.get();
}

Class* Class::load(const NamedEntity* ne, const StringData* name) {
  Class* cls;
  if (LIKELY((cls = ne->getCachedClass()) != nullptr)) {
    return cls;
  }
  return loadMissing(ne, name);
}

Class* Class::loadMissing(const NamedEntity* ne, const StringData* name) {
  VMRegAnchor _;
  AutoloadHandler::s_instance->autoloadClass(
    StrNR(const_cast<StringData*>(name)));
  return Class::lookup(ne);
}

Class* Class::get(const NamedEntity* ne, const StringData *name, bool tryAutoload) {
  Class *cls = lookup(ne);
  if (UNLIKELY(!cls)) {
    if (tryAutoload) {
      return loadMissing(ne, name);
    }
  }
  return cls;
}

bool Class::exists(const StringData* name, bool autoload, ClassKind kind) {
  Class* cls = Class::get(name, autoload);
  return cls &&
    (cls->attrs() & (AttrInterface | AttrTrait)) == classKindAsAttr(kind);
}

///////////////////////////////////////////////////////////////////////////////
}
