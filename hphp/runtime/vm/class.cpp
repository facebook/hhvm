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
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/hphp-array.h"
#include "hphp/util/util.h"
#include "hphp/util/debug.h"
#include "hphp/runtime/vm/jit/target-cache.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/treadmill.h"
#include "hphp/runtime/vm/request-arena.h"
#include "hphp/system/systemlib.h"
#include "hphp/util/logger.h"
#include "hphp/parser/parser.h"

#include <iostream>
#include <algorithm>

namespace HPHP {

static StringData* sd86ctor = StringData::GetStaticString("86ctor");
static StringData* sd86pinit = StringData::GetStaticString("86pinit");
static StringData* sd86sinit = StringData::GetStaticString("86sinit");

hphp_hash_map<const StringData*, const HhbcExtClassInfo*,
              string_data_hash, string_data_isame> Class::s_extClassHash;
Class::InstanceCounts Class::s_instanceCounts;
ReadWriteMutex Class::s_instanceCountsLock(RankInstanceCounts);
Class::InstanceBitsMap Class::s_instanceBits;
ReadWriteMutex Class::s_instanceBitsLock(RankInstanceBits);
std::atomic<bool> Class::s_instanceBitsInit{false};

const StringData* PreClass::manglePropName(const StringData* className,
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

PreClass::Prop::Prop(PreClass* preClass,
                     const StringData* n,
                     Attr attrs,
                     const StringData* typeConstraint,
                     const StringData* docComment,
                     const TypedValue& val,
                     DataType hphpcType)
  : m_preClass(preClass)
  , m_name(n)
  , m_attrs(attrs)
  , m_typeConstraint(typeConstraint)
  , m_docComment(docComment)
  , m_hphpcType(hphpcType)
{
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
                       const StringData* typeConstraint,
                       const TypedValue& val, const StringData* phpCode)
  : m_preClass(preClass), m_name(n), m_typeConstraint(typeConstraint),
    m_phpCode(phpCode) {
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
      m_InstanceCtor(nullptr) {
  m_namedEntity = Unit::GetNamedEntity(n);
}

PreClass::~PreClass() {
  std::for_each(methods(), methods() + numMethods(), Func::destroy);
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
// Class.

Class* Class::newClass(PreClass* preClass, Class* parent) {
  unsigned classVecLen = (parent != nullptr) ? parent->m_classVecLen+1 : 1;
  void* mem = Util::low_malloc(sizeForNClasses(classVecLen));
  try {
    return new (mem) Class(preClass, parent, classVecLen);
  } catch (...) {
    Util::low_free(mem);
    throw;
  }
}

Class::Class(PreClass* preClass, Class* parent, unsigned classVecLen)
  : m_preClass(PreClassPtr(preClass)), m_parent(parent),
    m_traitsBeginIdx(0), m_traitsEndIdx(0), m_clsInfo(nullptr),
    m_builtinPropSize(0), m_classVecLen(classVecLen), m_cachedOffset(0),
    m_propDataCache(-1), m_propSDataCache(-1), m_InstanceCtor(nullptr),
    m_nextClass(nullptr) {
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

Class::~Class() {
  releaseRefs();

  auto methods = methodRange();
  while (!methods.empty()) {
    Func* meth = methods.popFront();
    if (meth) Func::destroy(meth);
  }
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
  auto methods = mutableMethodRange();
  bool okToReleaseParent = true;
  while (!methods.empty()) {
    Func*& meth = methods.popFront();
    if (meth /* releaseRefs can be called more than once */ &&
        meth->cls() != this &&
        ((meth->attrs() & AttrPrivate) || !meth->hasStaticLocals())) {
      meth = nullptr;
      okToReleaseParent = false;
    }
  }

  if (okToReleaseParent) {
    m_parent.reset();
  }
  m_declInterfaces.clear();
  m_usedTraits.clear();
}

namespace {

class FreeClassTrigger : public Treadmill::WorkItem {
  TRACE_SET_MOD(treadmill);
  Class* m_cls;
 public:
  explicit FreeClassTrigger(Class* cls) : m_cls(cls) {
    TRACE(3, "FreeClassTrigger @ %p, cls %p\n", this, m_cls);
  }
  void operator()() {
    TRACE(3, "FreeClassTrigger: Firing @ %p , cls %p\n", this, m_cls);
    if (!m_cls->decAtomicCount()) {
      m_cls->atomicRelease();
    }
  }
};

}

void Class::destroy() {
  /*
   * If we were never put on NamedEntity::classList, or
   * we've already been destroy'd, there's nothing to do
   */
  if (!m_cachedOffset) return;

  Lock l(Unit::s_classesMutex);
  // Need to recheck now we have the lock
  if (!m_cachedOffset) return;
  // Only do this once.
  m_cachedOffset = 0;

  PreClass* pcls = m_preClass.get();
  pcls->namedEntity()->removeClass(this);
  /*
   * Regardless of refCount, this Class is now unusable.
   * Release what we can immediately, to allow dependent
   * classes to be freed.
   * Needs to be under the lock, because multiple threads
   * could call destroy
   */
  releaseRefs();
  Treadmill::WorkItem::enqueue(new FreeClassTrigger(this));
}

void Class::atomicRelease() {
  assert(!m_cachedOffset);
  assert(!getCount());
  this->~Class();
  Util::low_free(this);
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
  for (auto const& declInterface : m_declInterfaces) {
    if (!Transl::TargetCache::isPersistentHandle(
          declInterface->m_cachedOffset)) {
      return false;
    }
  }
  for (auto const& usedTrait : m_usedTraits) {
    if (!Transl::TargetCache::isPersistentHandle(
          usedTrait->m_cachedOffset)) {
      return false;
    }
  }
  return true;
}

const Func* Class::getDeclaredCtor() const {
  const Func* f = getCtor();
  return f->name() != sd86ctor ? f : nullptr;
}

void Class::initInstanceBits() {
  assert(Transl::Translator::WriteLease().amOwner());
  if (s_instanceBitsInit.load(std::memory_order_acquire)) return;

  // First, grab a write lock on s_instanceCounts and grab the current set of
  // counts as quickly as possible to minimize blocking other threads still
  // trying to profile instance checks.
  typedef std::pair<const StringData*, unsigned> Count;
  std::vector<Count> counts;
  uint64_t total = 0;
  {
    // If you think of the read-write lock as a shared-exclusive lock instead,
    // the fact that we're grabbing a write lock to iterate over the table
    // makes more sense: it's safe to concurrently modify a
    // tbb::concurrent_hash_map, but iteration is not guaranteed to be safe
    // with concurrent insertions.
    WriteLock l(s_instanceCountsLock);
    for (auto& pair : s_instanceCounts) {
      counts.push_back(pair);
      total += pair.second;
    }
  }
  std::sort(counts.begin(), counts.end(), [&](const Count& a, const Count& b) {
    return a.second > b.second;
  });

  // Next, initialize s_instanceBits with the top 127 most checked classes. Bit
  // 0 is reserved as an 'initialized' flag
  unsigned i = 1;
  uint64_t accum = 0;
  for (auto& item : counts) {
    if (i >= kInstanceBits) break;
    if (Class* cls = Unit::lookupUniqueClass(item.first)) {
      if (!(cls->attrs() & AttrUnique)) {
        continue;
      }
    }
    s_instanceBits[item.first] = i;
    accum += item.second;
    ++i;
  }

  // Print out stats about what we ended up using
  if (Trace::moduleEnabledRelease(Trace::instancebits, 1)) {
    Trace::traceRelease("%s: %u classes, %" PRIu64 " (%.2f%%) of warmup"
                        " checks\n",
                        __FUNCTION__, i-1, accum, 100.0 * accum / total);
    if (Trace::moduleEnabledRelease(Trace::instancebits, 2)) {
      accum = 0;
      i = 1;
      for (auto& pair : counts) {
        if (i >= 256) {
          Trace::traceRelease("skipping the remainder of the %zu classes\n",
                              counts.size());
          break;
        }
        accum += pair.second;
        Trace::traceRelease("%3u %5.2f%% %7u -- %6.2f%% %7" PRIu64 " %s\n",
                            i++, 100.0 * pair.second / total, pair.second,
                            100.0 * accum / total, accum,
                            pair.first->data());
      }
    }
  }

  // Finally, update m_instanceBits on every Class that currently exists. This
  // must be done while holding a lock that blocks insertion of new Classes
  // into their class lists, but in practice most Classes will already be
  // created by now and this process takes at most 10ms.
  WriteLock l(s_instanceBitsLock);
  for (AllClasses ac; !ac.empty(); ) {
    Class* c = ac.popFront();
    c->setInstanceBitsAndParents();
  }

  s_instanceBitsInit.store(true, std::memory_order_release);
}

void Class::profileInstanceOf(const StringData* name) {
  assert(name->isStatic());
  unsigned inc = 1;
  Class* c = Unit::lookupClass(name);
  if (c && (c->attrs() & AttrInterface)) {
    // Favor interfaces
    inc = 250;
  }
  InstanceCounts::accessor acc;

  // The extra layer of locking is here so that initInstanceBits can safely
  // iterate over s_instanceCounts while building its map of names to bits.
  ReadLock l(s_instanceCountsLock);
  if (!s_instanceCounts.insert(acc, InstanceCounts::value_type(name, inc))) {
    acc->second += inc;
  }
}

bool Class::haveInstanceBit(const StringData* name) {
  assert(Transl::Translator::WriteLease().amOwner());
  assert(s_instanceBitsInit.load(std::memory_order_acquire));
  return mapContains(s_instanceBits, name);
}

bool Class::getInstanceBitMask(const StringData* name,
                               int& offset, uint8_t& mask) {
  assert(Transl::Translator::WriteLease().amOwner());
  assert(s_instanceBitsInit.load(std::memory_order_acquire));
  const size_t bitWidth = sizeof(mask) * CHAR_BIT;
  unsigned bit;
  if (!mapGet(s_instanceBits, name, &bit)) return false;
  assert(bit >= 1 && bit < kInstanceBits);
  offset = offsetof(Class, m_instanceBits) + bit / bitWidth * sizeof(mask);
  mask = 1u << (bit % bitWidth);
  return true;
}

/*
 * Check whether a Class from a previous request is available to be defined.
 * The caller should check that it has the same preClass that is being defined.
 * Being available means that the parent, the interfaces and the traits are
 * already defined (or become defined via autoload, if tryAutoload is true).
 *
 * returns Avail::True - if it is available
 *         Avail::Fail - if it is impossible to define the class at this point
 *         Avail::False- if this particular Class* cant be defined at this point
 *
 * Note that Fail means that at least one of the parent, interfaces and traits
 * was not defined at all, while False means that at least one was defined but
 * did not correspond to this Class*
 *
 * The parent parameter is used for two purposes: first it avoids looking up the
 * active parent class for each potential Class*; and second its used on
 * Fail to return the problem class so the caller can report the error
 * correctly.
 */
Class::Avail Class::avail(Class*& parent, bool tryAutoload /*=false*/) const {
  if (Class *ourParent = m_parent.get()) {
    if (!parent) {
      PreClass *ppcls = ourParent->m_preClass.get();
      parent = Unit::getClass(ppcls->namedEntity(), ppcls->name(), tryAutoload);
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
  for (auto const& di : m_declInterfaces) {
    Class* declInterface = di.get();
    PreClass *pint = declInterface->m_preClass.get();
    Class* interface = Unit::getClass(pint->namedEntity(), pint->name(),
                                      tryAutoload);
    if (interface != declInterface) {
      if (interface == nullptr) {
        parent = declInterface;
        return Avail::Fail;
      }
      if (UNLIKELY(declInterface->isZombie())) {
        const_cast<Class*>(this)->destroy();
      }
      return Avail::False;
    }
  }
  for (auto const& ut : m_usedTraits) {
    Class* usedTrait = ut.get();
    PreClass* ptrait = usedTrait->m_preClass.get();
    Class* trait = Unit::getClass(ptrait->namedEntity(), ptrait->name(),
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
  return Avail::True;
}

void Class::initialize(TypedValue*& sProps) const {
  if (m_pinitVec.size() > 0) {
    if (getPropData() == nullptr) {
      initProps();
    }
  }
  // The asymmetry between the logic around initProps() above and initSProps()
  // below is due to the fact that instance properties only require storage in
  // g_vmContext if there are non-scalar initializers involved, whereas static
  // properties *always* require storage in g_vmContext.
  if (numStaticProperties() > 0) {
    if ((sProps = getSPropData()) == nullptr) {
      sProps = initSProps();
    }
  } else {
    sProps = nullptr;
  }
}

void Class::initialize() const {
  TypedValue* sProps;
  initialize(sProps);
}

Class::PropInitVec* Class::initPropsImpl() const {
  assert(m_pinitVec.size() > 0);
  assert(getPropData() == nullptr);
  // Copy initial values for properties to a new vector that can be used to
  // complete initialization for non-scalar properties via the iterative
  // 86pinit() calls below. 86pinit() takes a reference to an array to populate
  // with initial property values; after it completes, we copy the values into
  // the new propVec.
  request_arena().beginFrame();
  PropInitVec* propVec = PropInitVec::allocInRequestArena(m_declPropInit);
  size_t nProps = numDeclProperties();

  {
    Array args;

    HphpArray* propArr = ArrayData::Make(nProps);
    Variant arg0(propArr);

    args.appendRef(arg0);
    assert(propArr->getCount() == 1);  // Don't want to trigger COW
    {
      // Create a sentinel that uniquely identifies uninitialized properties.
      ObjectData* sentinel = SystemLib::AllocPinitSentinel();
      sentinel->incRefCount();
      TypedValue tv;
      tv.m_data.pobj = sentinel;
      tv.m_type = KindOfObject;
      args.append(tvAsCVarRef(&tv));
      sentinel->decRefCount();
    }

    TypedValue* tvSentinel = args->nvGetValueRef(1);
    for (size_t i = 0; i < nProps; ++i) {
      TypedValue& prop = (*propVec)[i];
      // We have to use m_originalMangledName here because the
      // 86pinit methods for traits depend on it
      auto const* k = (m_declProperties[i].m_attrs & AttrPrivate)
        ? m_declProperties[i].m_originalMangledName
        : m_declProperties[i].m_name;

      // Replace undefined values with tvSentinel, which acts as a
      // unique sentinel for undefined properties in 86pinit().
      if (prop.m_type == KindOfUninit) {
        propArr->nvInsert(const_cast<StringData*>(k), tvSentinel);
      } else {
        // This may seem pointless, but if you don't populate all the keys,
        // you'll get "undefined index" notices in the case where a
        // scalar-initialized property overrides a parent's
        // non-scalar-initialized property of the same name.
        propArr->nvInsert(const_cast<StringData*>(k), &prop);
      }
    }

    try {
      // Iteratively invoke 86pinit() methods upward
      // through the inheritance chain.
      for (Class::InitVec::const_reverse_iterator it = m_pinitVec.rbegin();
           it != m_pinitVec.rend(); ++it) {
        TypedValue retval;
        g_vmContext->invokeFunc(&retval, *it, args, nullptr,
                                const_cast<Class*>(this));
        assert(retval.m_type == KindOfNull);
      }
    } catch (...) {
      // Undo the allocation of propVec
      request_arena().endFrame();
      throw;
    }

    // Pull the values out of the populated array and put them in propVec
    for (size_t i = 0; i < nProps; ++i) {
      TypedValue& prop = (*propVec)[i];
      if (prop.m_type == KindOfUninit) {
        auto const* k = (m_declProperties[i].m_attrs & AttrPrivate)
          ? m_declProperties[i].m_originalMangledName
          : m_declProperties[i].m_name;

        auto const* value = propArr->nvGet(k);
        assert(value);
        tvDup(*value, prop);
      }
    }
  }

  // For properties that do not require deep initialization, promote strings
  // and arrays that came from 86pinit to static. This allows us to initialize
  // object properties very quickly because we can just memcpy and we don't
  // have to do any refcounting.
  // For properties that require "deep" initialization, we have to do a little
  // more work at object creation time.
  Slot slot = 0;
  for (PropInitVec::iterator it = propVec->begin();
       it != propVec->end(); ++it, ++slot) {
    TypedValueAux* tv = &(*it);
    // Set deepInit if the property requires "deep" initialization.
    if (m_declProperties[slot].m_attrs & AttrDeepInit) {
      tv->deepInit() = true;
    } else {
      tvAsVariant(tv).setEvalScalar();
      tv->deepInit() = false;
    }
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
      assert(baseClass);
      // If ctx == baseClass, we know we have the right property
      // and we can stop here.
      if (ctx == baseClass) {
        accessible = true;
        return propInd;
      }
      // The anonymous context cannot access protected or private
      // properties, so we can fail fast here.
      if (ctx == nullptr) {
        accessible = false;
        return propInd;
      }
      assert(ctx);
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
        assert(baseClass->classof(ctx));
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
  // If ctx is an ancestor of this, check if ctx has a private property
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

TypedValue* Class::initSPropsImpl() const {
  assert(numStaticProperties() > 0);
  assert(getSPropData() == nullptr);
  // Create an array that is initially large enough to hold all static
  // properties.
  TypedValue* const spropTable =
    new (request_arena()) TypedValue[m_staticProperties.size()];

  const bool hasNonscalarInit = !m_sinitVec.empty();
  Array propArr;
  TypedValue tvSentinel;
  tvWriteUninit(&tvSentinel);

  SCOPE_EXIT {
    tvRefcountedDecRef(&tvSentinel);
  };

  // If there are non-scalar initializers (i.e. 86sinit methods), run them now.
  // They'll put their initialized values into an array, and we'll read any
  // values we need out of the array later.
  if (hasNonscalarInit) {
    HphpArray* propData = ArrayData::Make(m_staticProperties.size());
    Variant arg0(propData);

    // The 86sinit functions will initialize some subset of the static props.
    // Set all of them to a sentinel object so we can distinguish these.
    tvSentinel.m_type = KindOfObject;
    tvSentinel.m_data.pobj = SystemLib::AllocPinitSentinel();
    tvSentinel.m_data.pobj->incRefCount();

    for (Slot slot = 0; slot < m_staticProperties.size(); ++slot) {
      auto const& sProp = m_staticProperties[slot];
      propData->set(const_cast<StringData*>(sProp.m_name),
                    tvAsCVarRef(&tvSentinel),
                    false);
    }

    // Run the 86sinit functions, going up the inheritance chain.
    Array args;
    args.appendRef(arg0);
    assert(propData->getCount() == 1);  // don't want to trigger COW

    for (unsigned i = 0; i < m_sinitVec.size(); i++) {
      TypedValue retval;
      g_vmContext->invokeFunc(&retval, m_sinitVec[i], args, nullptr,
                              const_cast<Class*>(this));
      assert(retval.m_type == KindOfNull);
    }

    // Transfer ownership of the reference to the outer scope.
    propArr = propData;
  }

  // A helper to look up values produced by 86sinit.
  auto getValueFromArr = [&](const StringData* name) -> const TypedValue* {
    if (!propArr.isNull()) {
      assert(tvSentinel.m_type == KindOfObject);
      auto const* v = propArr.get()->nvGet(name);
      if (v->m_type != KindOfObject ||
          v->m_data.pobj != tvSentinel.m_data.pobj) {
        return v;
      }
    }
    return nullptr;
  };

  for (Slot slot = 0; slot < m_staticProperties.size(); ++slot) {
    auto const& sProp = m_staticProperties[slot];
    auto const* propName = sProp.m_name;

    if (sProp.m_class == this) {
      auto const* value = getValueFromArr(propName);
      if (value) {
        tvDup(*value, spropTable[slot]);
      } else {
        assert(tvIsStatic(&sProp.m_val));
        spropTable[slot] = sProp.m_val;
      }
    } else {
      bool visible, accessible;
      auto* storage = sProp.m_class->getSProp(nullptr, propName,
                                              visible, accessible);
      auto const* value = getValueFromArr(propName);
      if (value) {
        tvDup(*value, *storage);
      }

      tvBindIndirect(&spropTable[slot], storage);
    }
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
    return nullptr;
  }

  visible = true;
  if (ctx == this) {
    // Property access is from within a method of this class, so the property
    // is accessible.
    accessible = true;
  } else {
    Attr sPropAttrs = m_staticProperties[sPropInd].m_attrs;
    if ((ctx != nullptr) && (classof(ctx) || ctx->classof(this))) {
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

  assert(sProps != nullptr);
  TypedValue* sProp = tvDerefIndirect(&sProps[sPropInd]);
  assert(sProp->m_type != KindOfUninit &&
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
  assert(s != kInvalidSlot);
  return declCls->m_staticProperties[s].m_val;
}

HphpArray* Class::initClsCnsData() const {
  Slot nConstants = m_constants.size();
  HphpArray* constants = ArrayData::Make(nConstants);
  constants->incRefCount();

  if (m_parent.get() != nullptr) {
    if (g_vmContext->getClsCnsData(m_parent.get()) == nullptr) {
      // Initialize recursively up the inheritance chain.
      m_parent->initClsCnsData();
    }
  }

  for (Slot i = 0; i < nConstants; ++i) {
    const Const& constant = m_constants[i];
    const TypedValue* tv = &constant.m_val;
    constants->set((StringData*)constant.m_name, tvAsCVarRef(tv), false);
    // XXX: set() converts KindOfUninit to KindOfNull, but our class
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

Cell* Class::cnsNameToTV(const StringData* clsCnsName,
                         Slot& clsCnsInd) const {
  clsCnsInd = m_constants.findIndex(clsCnsName);
  if (clsCnsInd == kInvalidSlot) {
    return nullptr;
  }
  auto const ret = const_cast<Cell*>(&m_constants[clsCnsInd].m_val);
  assert(cellIsPlausible(*ret));
  return ret;
}

Cell* Class::clsCnsGet(const StringData* clsCnsName) const {
  Slot clsCnsInd;
  Cell* clsCns = cnsNameToTV(clsCnsName, clsCnsInd);
  if (!clsCns || clsCns->m_type != KindOfUninit) {
    return clsCns;
  }

  // This constant has a non-scalar initializer, so look in g_vmContext for
  // an entry associated with this class.
  HphpArray* clsCnsData = g_vmContext->getClsCnsData(this);
  if (clsCnsData == nullptr) {
    clsCnsData = initClsCnsData();
  }

  clsCns = clsCnsData->nvGetValueRef(clsCnsInd);
  if (clsCns->m_type == KindOfUninit) {
    // The class constant has not been initialized yet; do so.
    static StringData* sd86cinit = StringData::GetStaticString("86cinit");
    const Func* meth86cinit =
      m_constants[clsCnsInd].m_class->lookupMethod(sd86cinit);
    TypedValue tv[1];
    tv->m_data.pstr = (StringData*)clsCnsName;
    tv->m_type = KindOfString;
    clsCnsName->incRefCount();
    g_vmContext->invokeFuncFew(clsCns, meth86cinit, ActRec::encodeClass(this),
                               nullptr, 1, tv);
  }
  assert(cellIsPlausible(*clsCns));
  return clsCns;
}

DataType Class::clsCnsType(const StringData* cnsName) const {
  Slot slot;
  auto const cns = cnsNameToTV(cnsName, slot);
  // TODO: lookup the constant in target cache in case it's dynamic
  // and already initialized.
  if (!cns) return KindOfUninit;
  return cns->m_type;
}

void Class::setParent() {
  // Validate the parent
  if (m_parent.get() != nullptr) {
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
  if (!cls->preClass()->hasMethod(name)) return nullptr;
  Func* f = cls->preClass()->lookupMethod(name);
  f = f->clone(cls);
  f->setNewFuncId();
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
  if (m_parent.get() != nullptr &&
      m_parent->m_ctor->name() != sd86ctor) {
    m_ctor = m_parent->m_ctor;
    return;
  }

  // Use 86ctor(), since no program-supplied constructor exists
  m_ctor = findSpecialMethod(this, sd86ctor);
  assert(m_ctor && "class had no user-defined constructor or 86ctor");
  assert((m_ctor->attrs() & ~AttrBuiltin) ==
         (AttrPublic|AttrNoInjection|AttrPhpLeafFn));
}

void Class::applyTraitPrecRule(const PreClass::TraitPrecRule& rule,
                               MethodToTraitListMap& importMethToTraitMap) {
  const StringData* methName          = rule.getMethodName();
  const StringData* selectedTraitName = rule.getSelectedTraitName();
  TraitNameSet      otherTraitNames;
  rule.getOtherTraitNames(otherTraitNames);

  auto methIter = importMethToTraitMap.find(methName);
  if (methIter == importMethToTraitMap.end()) {
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

Class* Class::findSingleTraitWithMethod(const StringData* methName) {
  // Note: m_methods includes methods from parents / traits recursively
  Class* traitCls = nullptr;
  for (auto const& t : m_usedTraits) {
    if (t->m_methods.contains(methName)) {
      if (traitCls != nullptr) { // more than one trait contains method
        return nullptr;
      }
      traitCls = t.get();
    }
  }
  return traitCls;
}

void Class::setImportTraitMethodModifiers(TraitMethodList& methList,
                                          Class*           traitCls,
                                          Attr             modifiers) {
  for (TraitMethodList::iterator iter = methList.begin();
       iter != methList.end(); iter++) {
    if (iter->m_trait == traitCls) {
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

void Class::applyTraitAliasRule(const PreClass::TraitAliasRule& rule,
                                MethodToTraitListMap& importMethToTraitMap) {
  const StringData* traitName    = rule.getTraitName();
  const StringData* origMethName = rule.getOrigMethodName();
  const StringData* newMethName  = rule.getNewMethodName();

  Class* traitCls = nullptr;
  if (traitName->empty()) {
    traitCls = findSingleTraitWithMethod(origMethName);
  } else {
    traitCls = Unit::loadClass(traitName);
  }

  if (!traitCls || (!(traitCls->attrs() & AttrTrait))) {
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
    setImportTraitMethodModifiers(importMethToTraitMap[origMethName],
                                  traitCls, ruleModifiers);
  } else {
    ruleModifiers = rule.getModifiers();
    TraitMethod traitMethod(traitCls, traitMeth, ruleModifiers);
    if (!Func::isSpecial(newMethName)) {
      importMethToTraitMap[newMethName].push_back(traitMethod);
    }
  }
  if (ruleModifiers & AttrStatic) {
    raise_error("cannot use 'static' as access modifier");
  }
}

void Class::applyTraitRules(MethodToTraitListMap& importMethToTraitMap) {
  for (size_t i = 0; i < m_preClass->traitPrecRules().size(); i++) {
    applyTraitPrecRule(m_preClass->traitPrecRules()[i],
                       importMethToTraitMap);
  }
  for (size_t i = 0; i < m_preClass->traitAliasRules().size(); i++) {
    applyTraitAliasRule(m_preClass->traitAliasRules()[i],
                        importMethToTraitMap);
  }
}

void Class::importTraitMethod(const TraitMethod&  traitMethod,
                              const StringData*   methName,
                              MethodMap::Builder& builder) {
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
  Func* f = method->clone(this);
  f->setNewFuncId();
  f->setName(methName);
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

// This method removes trait abstract methods that are either:
//   1) implemented by other traits
//   2) duplicate
void Class::removeSpareTraitAbstractMethods(
  MethodToTraitListMap& importMethToTraitMap) {

  for (MethodToTraitListMap::iterator iter = importMethToTraitMap.begin();
       iter != importMethToTraitMap.end(); iter++) {

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
  MethodToTraitListMap importMethToTraitMap;

  // 1. Find all methods to be imported
  for (auto const& t : m_usedTraits) {
    Class* trait = t.get();
    for (Slot i = 0; i < trait->m_methods.size(); ++i) {
      Func* method = trait->m_methods[i];
      const StringData* methName = method->name();
      TraitMethod traitMethod(trait, method, method->attrs());
      if (!Func::isSpecial(methName)) {
        importMethToTraitMap[methName].push_back(traitMethod);
      }
    }
  }

  // 2. Apply trait rules
  applyTraitRules(importMethToTraitMap);

  // 3. Remove abstract methods provided by other traits, and also duplicates
  removeSpareTraitAbstractMethods(importMethToTraitMap);

  // 4. Actually import the methods
  for (MethodToTraitListMap::const_iterator iter =
         importMethToTraitMap.begin();
       iter != importMethToTraitMap.end(); iter++) {

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
  if (method->isGenerated()) return;

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

  if (m_parent.get() != nullptr) {
    // Copy down the parent's method entries. These may be overridden below.
    for (Slot i = 0; i < m_parent->m_methods.size(); ++i) {
      Func* f = m_parent->m_methods[i];
      assert(f);
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

  assert(AttrPublic < AttrProtected && AttrProtected < AttrPrivate);
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
      assert(parentMethod);
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
      f = f->clone(f->attrs() & AttrClone ? this : f->cls());
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
  static StringData* sd__call = StringData::GetStaticString("__call");
  static StringData* sd__callStatic
    = StringData::GetStaticString("__callStatic");

  m_ODAttrs = 0;
  if (lookupMethod(sd__sleep     )) { m_ODAttrs |= ObjectData::HasSleep;      }
  if (lookupMethod(sd__get       )) { m_ODAttrs |= ObjectData::UseGet;        }
  if (lookupMethod(sd__set       )) { m_ODAttrs |= ObjectData::UseSet;        }
  if (lookupMethod(sd__isset     )) { m_ODAttrs |= ObjectData::UseIsset;      }
  if (lookupMethod(sd__unset     )) { m_ODAttrs |= ObjectData::UseUnset;      }
  if (lookupMethod(sd__call      )) { m_ODAttrs |= ObjectData::HasCall;       }
  if (lookupMethod(sd__callStatic)) { m_ODAttrs |= ObjectData::HasCallStatic; }
}

void Class::setConstants() {
  ConstMap::Builder builder;

  if (m_parent.get() != nullptr) {
    for (Slot i = 0; i < m_parent->m_constants.size(); ++i) {
      // Copy parent's constants.
      builder.add(m_parent->m_constants[i].m_name, m_parent->m_constants[i]);
    }
  }

  // Copy in interface constants.
  for (auto it = m_declInterfaces.begin(); it != m_declInterfaces.end(); ++it) {
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

static void copyDeepInitAttr(const PreClass::Prop* pclsProp,
                             Class::Prop* clsProp) {
  if (pclsProp->attrs() & AttrDeepInit) {
    clsProp->m_attrs = (Attr)(clsProp->m_attrs | AttrDeepInit);
  } else {
    clsProp->m_attrs = (Attr)(clsProp->m_attrs & ~AttrDeepInit);
  }
}

void Class::setProperties() {
  int numInaccessible = 0;
  PropMap::Builder curPropMap;
  SPropMap::Builder curSPropMap;
  m_hasDeepInitProps = false;

  if (m_parent.get() != nullptr) {
    // m_hasDeepInitProps indicates if there are properties that require
    // deep initialization. Note there are cases where m_hasDeepInitProps is
    // true but none of the properties require deep initialization; this can
    // happen if a derived class redeclares a public or protected property
    // from an ancestor class. We still get correct behavior in these cases,
    // so it works out okay.
    m_hasDeepInitProps = m_parent->m_hasDeepInitProps;
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
      prop.m_typeConstraint = parentProp.m_typeConstraint;
      prop.m_name = parentProp.m_name;
      prop.m_hphpcType = parentProp.m_hphpcType;
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
      sProp.m_typeConstraint = parentProp.m_typeConstraint;
      sProp.m_docComment = parentProp.m_docComment;
      sProp.m_class = parentProp.m_class;
      tvWriteUninit(&sProp.m_val);
      curSPropMap.add(sProp.m_name, sProp);
    }
  }

  assert(AttrPublic < AttrProtected && AttrProtected < AttrPrivate);
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
             > (parentProp->m_attrs & (AttrPublic|AttrProtected|AttrPrivate))) {
        raise_error(
          "Access level to %s::$%s() must be %s (as in class %s) or weaker",
          m_preClass->name()->data(), preProp->name()->data(),
          attrToVisibilityStr(parentProp->m_attrs),
          m_parent->name()->data());
      }
      if (preProp->attrs() & AttrDeepInit) {
        m_hasDeepInitProps = true;
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
        prop.m_typeConstraint = preProp->typeConstraint();
        prop.m_docComment = preProp->docComment();
        prop.m_hphpcType = preProp->hphpcType();
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
          assert((curPropMap[it2->second].m_attrs
                 & (AttrPublic|AttrProtected|AttrPrivate)) == AttrProtected);
          const TypedValue& tv = m_preClass->lookupProp(preProp->name())->val();
          TypedValueAux& tvaux = m_declPropInit[it2->second];
          tvaux.m_data = tv.m_data;
          tvaux.m_type = tv.m_type;
          copyDeepInitAttr(preProp, &curPropMap[it2->second]);
          break;
        }
        // Append a new protected property.
        Prop prop;
        prop.m_name = preProp->name();
        prop.m_mangledName = preProp->mangledName();
        prop.m_originalMangledName = preProp->mangledName();
        prop.m_attrs = preProp->attrs();
        prop.m_typeConstraint = preProp->typeConstraint();
        // This is the first class to declare this property
        prop.m_class = this;
        prop.m_docComment = preProp->docComment();
        prop.m_hphpcType = preProp->hphpcType();
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
            prop.m_typeConstraint = preProp->typeConstraint();
          }
          const TypedValue& tv = m_preClass->lookupProp(preProp->name())->val();
          TypedValueAux& tvaux = m_declPropInit[it2->second];
          tvaux.m_data = tv.m_data;
          tvaux.m_type = tv.m_type;
          copyDeepInitAttr(preProp, &curPropMap[it2->second]);
          break;
        }
        // Append a new public property.
        Prop prop;
        prop.m_name = preProp->name();
        prop.m_mangledName = preProp->mangledName();
        prop.m_originalMangledName = preProp->mangledName();
        prop.m_attrs = preProp->attrs();
        prop.m_typeConstraint = preProp->typeConstraint();
        // This is the first class to declare this property
        prop.m_class = this;
        prop.m_docComment = preProp->docComment();
        prop.m_hphpcType = preProp->hphpcType();
        curPropMap.add(preProp->name(), prop);
        m_declPropInit.push_back(m_preClass->lookupProp(preProp->name())
                                 ->val());
        break;
      }
      default: assert(false);
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
      sProp.m_typeConstraint = preProp->typeConstraint();
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

void Class::importTraitInstanceProp(Class*      trait,
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
      prop.m_mangledName = PreClass::manglePropName(m_preClass->name(),
                                                    prop.m_name,
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

void Class::importTraitStaticProp(Class*   trait,
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
  for (auto& t : m_usedTraits) {
    Class* trait = t.get();

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
    Class* trait = m_usedTraits[t].get();
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
  if (m_parent.get() != nullptr) {
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
  if (meth86pinit != nullptr) {
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
  m_hasInitMethods = (m_pinitVec.size() > 0 || m_sinitVec.size() > 0);

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
  for (int i = 0, size = m_interfaces.size(); i < size; i++) {
    const Class* iface = m_interfaces[i];

    for (size_t m = 0; m < iface->m_methods.size(); m++) {
      Func* imeth = iface->m_methods[m];
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
      meth->parametersCompat(m_preClass.get(), imeth);
    }
  }
}

void Class::setInterfaces() {
  InterfaceMap::Builder interfacesBuilder;
  if (m_parent.get() != nullptr) {
    int size = m_parent->m_interfaces.size();
    for (int i = 0; i < size; i++) {
      Class* interface = m_parent->m_interfaces[i];
      interfacesBuilder.add(interface->name(), interface);
    }
  }
  for (PreClass::InterfaceVec::const_iterator it =
         m_preClass->interfaces().begin();
       it != m_preClass->interfaces().end(); ++it) {
    Class* cp = Unit::loadClass(*it);
    if (cp == nullptr) {
      raise_error("Undefined interface: %s", (*it)->data());
    }
    if (!(cp->attrs() & AttrInterface)) {
      raise_error("%s cannot implement %s - it is not an interface",
                  m_preClass->name()->data(), cp->name()->data());
    }
    m_declInterfaces.push_back(ClassPtr(cp));
    if (interfacesBuilder.find(cp->name()) == interfacesBuilder.end()) {
      interfacesBuilder.add(cp->name(), cp);
    }
    int size = cp->m_interfaces.size();
    for (int i = 0; i < size; i++) {
      Class* interface = cp->m_interfaces[i];
      interfacesBuilder.find(interface->name());
      if (interfacesBuilder.find(interface->name()) ==
          interfacesBuilder.end()) {
        interfacesBuilder.add(interface->name(), interface);
      }
    }
  }
  m_interfaces.create(interfacesBuilder);
  checkInterfaceMethods();
}

void Class::setUsedTraits() {
  for (PreClass::UsedTraitVec::const_iterator
       it = m_preClass->usedTraits().begin();
       it != m_preClass->usedTraits().end(); it++) {
    Class* classPtr = Unit::loadClass(*it);
    if (classPtr == nullptr) {
      raise_error("Trait '%s' not found", (*it)->data());
    }
    if (!(classPtr->attrs() & AttrTrait)) {
      raise_error("%s cannot use %s - it is not a trait",
                  m_preClass->name()->data(),
                  classPtr->name()->data());
    }
    m_usedTraits.push_back(ClassPtr(classPtr));
  }
}

void Class::setClassVec() {
  if (m_classVecLen > 1) {
    assert(m_parent.get() != nullptr);
    memcpy(m_classVec, m_parent->m_classVec,
           (m_classVecLen-1) * sizeof(Class*));
  }
  m_classVec[m_classVecLen-1] = this;
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

  InstanceBits bits;
  bits.set(0);
  auto setBits = [&](Class* c) {
    if (setParents) c->setInstanceBitsAndParents();
    bits |= c->m_instanceBits;
  };
  if (m_parent.get()) setBits(m_parent.get());
  for (auto& di : m_declInterfaces) setBits(di.get());

  unsigned bit;
  if (mapGet(s_instanceBits, m_preClass->name(), &bit)) {
    bits.set(bit);
  }
  m_instanceBits = bits;
}

// Finds the base class defining the given method (NULL if none).
// Note: for methods imported via traits, the base class is the one that
// uses/imports the trait.
Class* Class::findMethodBaseClass(const StringData* methName) {
  const Func* f = lookupMethod(methName);
  if (f == nullptr) return nullptr;
  return f->baseCls();
}

void Class::getMethodNames(const Class* ctx, HphpArray* methods) const {
  for (Slot i = 0; i < m_methods.size(); i++) {
    Func* meth = m_methods[i];
    StringData* methName = const_cast<StringData*>(meth->name());
    Class* declCls = meth->cls();

    // Only pick methods declared in this class, in order to match Zend's order.
    // Inherited methods will be inserted in the recursive call later.
    if (declCls != this) continue;

    // Skip generated, internal methods.
    if (meth->isGenerated()) continue;

    // Public methods are always visible.
    if ((meth->attrs() & AttrPublic)) {
      methods->set(methName, true_varNR, false);
      continue;
    }

    // In anonymous contexts, only public methods are visible.
    if (!ctx) continue;

    // All methods are visible if the context is the class that declared them.
    // If the context is not the declCls, protected methods are visible in
    // context classes related the declCls.
    if (declCls == ctx ||
        ((meth->attrs() & AttrProtected) &&
         (ctx->classof(declCls) || declCls->classof(ctx)))) {
      methods->set(methName, true_varNR, false);
    }
  }

  // Now add the inherited methods.
  if (m_parent.get()) m_parent->getMethodNames(ctx, methods);

  // Add interface methods that the class may not have implemented yet.
  for (int i = 0, sz = m_declInterfaces.size(); i < sz; i++) {
    m_declInterfaces[i]->getMethodNames(ctx, methods);
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
  assert(ci);

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
  ci->m_docComment = (m_preClass->docComment() != nullptr)
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
    Func* func = lookupMethod(m_preClass->methods()[i]->name());
    // Filter out special methods
    if (!func) {
      DEBUG_ONLY const StringData* name = m_preClass->methods()[i]->name();
      assert(!strcmp(name->data(), "86ctor"));
      continue;
    }
    if (func->isGenerated()) continue;
    assert(func);
    assert(declaredMethod(func));
    SET_FUNCINFO_BODY;
  }

  for (Slot i = m_traitsBeginIdx; i < m_traitsEndIdx; ++i) {
    Func* func = m_methods[i];
    assert(func);
    if (!func->isGenerated()) {
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
    pi->docComment = (m_declProperties[i].m_docComment != nullptr)
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
    pi->docComment = (m_staticProperties[i].m_docComment != nullptr)
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

size_t Class::declPropOffset(Slot index) const {
  assert(index >= 0);
  return sizeof(ObjectData) + m_builtinPropSize
    + index * sizeof(TypedValue);
}

Class::PropInitVec::~PropInitVec() {
  if (!m_smart) free(m_data);
}

Class::PropInitVec::PropInitVec() : m_data(nullptr), m_size(0), m_smart(false) {}

Class::PropInitVec*
Class::PropInitVec::allocInRequestArena(const PropInitVec& src) {
  ThreadInfo* info UNUSED = ThreadInfo::s_threadInfo.getNoCheck();
  PropInitVec* p = new (request_arena()) PropInitVec;
  p->m_size = src.size();
  p->m_data = new (request_arena()) TypedValueAux[src.size()];
  memcpy(p->m_data, src.m_data, src.size() * sizeof(*p->m_data));
  p->m_smart = true;
  return p;
}

const Class::PropInitVec&
Class::PropInitVec::operator=(const PropInitVec& piv) {
  assert(!m_smart);
  if (this != &piv) {
    unsigned sz = m_size = piv.size();
    if (sz) sz = Util::roundUpToPowerOfTwo(sz);
    free(m_data);
    m_data = (TypedValueAux*)malloc(sz * sizeof(*m_data));
    assert(m_data);
    memcpy(m_data, piv.m_data, piv.size() * sizeof(*m_data));
  }
  return *this;
}

void Class::PropInitVec::push_back(const TypedValue& v) {
  assert(!m_smart);
  /*
   * the allocated size is always the next power of two (or zero)
   * so we just need to reallocate when we hit a power of two
   */
  if (!m_size || Util::isPowerOfTwo(m_size)) {
    unsigned size = m_size ? m_size * 2 : 1;
    m_data = (TypedValueAux*)realloc(m_data, size * sizeof(*m_data));
    assert(m_data);
  }
  tvDup(v, m_data[m_size++]);
}

using Transl::TargetCache::handleToRef;

const Class::PropInitVec* Class::getPropData() const {
  if (m_propDataCache == (unsigned)-1) return nullptr;
  return handleToRef<PropInitVec*>(m_propDataCache);
}

void Class::initPropHandle() const {
  if (UNLIKELY(m_propDataCache == (unsigned)-1)) {
    const_cast<unsigned&>(m_propDataCache) =
      Transl::TargetCache::allocClassInitProp(name());
  }
}

void Class::initProps() const {
  setPropData(initPropsImpl());
}

void Class::setPropData(PropInitVec* propData) const {
  assert(getPropData() == nullptr);
  initPropHandle();
  handleToRef<PropInitVec*>(m_propDataCache) = propData;
}

TypedValue* Class::getSPropData() const {
  if (m_propSDataCache == (unsigned)-1) return nullptr;
  return handleToRef<TypedValue*>(m_propSDataCache);
}

void Class::initSPropHandle() const {
  if (UNLIKELY(m_propSDataCache == (unsigned)-1)) {
    const_cast<unsigned&>(m_propSDataCache) =
      Transl::TargetCache::allocClassInitSProp(name());
  }
}

TypedValue* Class::initSProps() const {
  TypedValue* sprops = initSPropsImpl();
  setSPropData(sprops);
  return sprops;
}

void Class::setSPropData(TypedValue* sPropData) const {
  assert(getSPropData() == nullptr);
  initSPropHandle();
  handleToRef<TypedValue*>(m_propSDataCache) = sPropData;
}

void Class::getChildren(std::vector<TypedValue *> &out) {
  for (Slot i = 0; i < m_staticProperties.size(); ++i) {
    if (m_staticProperties[i].m_class != this) continue;
    out.push_back(&m_staticProperties[i].m_val);
  }
}

// True if a CPP extension class has opted into serialization.
bool Class::isCppSerializable() const {
  assert(builtinPropSize() > 0); // Only call this on CPP classes
  return clsInfo() &&
    (clsInfo()->getAttribute() & ClassInfo::IsCppSerializable);
}

 } // HPHP::VM
