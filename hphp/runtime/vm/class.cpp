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
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/hphp-array.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/enum-cache.h"
#include "hphp/util/debug.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/treadmill.h"
#include "hphp/runtime/vm/native-data.h"
#include "hphp/system/systemlib.h"
#include "hphp/util/logger.h"
#include "hphp/parser/parser.h"
#include "folly/Bits.h"
#include <iostream>
#include <algorithm>

namespace HPHP {

static StringData* sd86ctor = makeStaticString("86ctor");
static StringData* sd86pinit = makeStaticString("86pinit");
static StringData* sd86sinit = makeStaticString("86sinit");

hphp_hash_map<const StringData*, const HhbcExtClassInfo*,
              string_data_hash, string_data_isame> Class::s_extClassHash;

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
    return makeStaticString(mangledName);
  }
  case AttrPrivate: {
    std::string mangledName = "";
    mangledName.push_back('\0');
    mangledName += className->data();
    mangledName.push_back('\0');
    mangledName += propName->data();
    return makeStaticString(mangledName);
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
                     RepoAuthType repoAuthType)
  : m_preClass(preClass)
  , m_name(n)
  , m_attrs(attrs)
  , m_typeConstraint(typeConstraint)
  , m_docComment(docComment)
  , m_repoAuthType{repoAuthType}
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
  : m_unit(unit)
  , m_line1(line1)
  , m_line2(line2)
  , m_offset(o)
  , m_id(id)
  , m_attrs(attrs)
  , m_hoistable(hoistable)
  , m_name(n)
  , m_parent(parent)
  , m_docComment(docComment)
{
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
  if (m_attrs & AttrUnique)     out << " (unique)";
  if (m_attrs & AttrPersistent) out << " (persistent)";
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

const StaticString s_nativedata("__nativedata");
void PreClass::setUserAttributes(const UserAttributeMap &ua) {
  m_userAttributes = ua;
  m_nativeDataInfo = nullptr;
  if (!ua.size()) return;

  // Check for <<__NativeData("Type")>>
  auto it = ua.find(s_nativedata.get());
  if (it == ua.end()) return;

  TypedValue ndiInfo = it->second;
  if (ndiInfo.m_type != KindOfArray) return;

  // Use the first string label which references a registered type
  // In practice, there should generally only be one item and
  // it should be a string, but maybe that'll be extended...
  for (ArrayIter it(ndiInfo.m_data.parr); it; ++it) {
    Variant val = it.second();
    if (!val.isString()) continue;
    if ((m_nativeDataInfo = Native::getNativeDataInfo(val.toString().get()))) {
      break;
    }
  }
}

//=============================================================================
// Class.

static_assert(sizeof(Class) == 376, "Change this only on purpose");

Class* Class::newClass(PreClass* preClass, Class* parent) {
  auto const classVecLen = parent != nullptr ? parent->m_classVecLen + 1 : 1;
  auto const size = offsetof(Class, m_classVec) + sizeof(Class*) * classVecLen;
  auto const mem = low_malloc(size);
  try {
    return new (mem) Class(preClass, parent, classVecLen);
  } catch (...) {
    low_free(mem);
    throw;
  }
}

void (*Class::MethodCreateHook)(Class* cls, MethodMap::Builder& builder);

Class::Class(PreClass* preClass, Class* parent, unsigned classVecLen)
  : m_parent(parent)
  , m_preClass(PreClassPtr(preClass))
  , m_classVecLen(classVecLen)
{
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
  checkTraitConstraints();
  setNativeDataInfo();
}

Class::~Class() {
  releaseRefs();

  auto methods = methodRange();
  while (!methods.empty()) {
    Func* meth = methods.popFront();
    if (meth) Func::destroy(meth);
  }
  // clean enum cache
  EnumCache::deleteValues(this);
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
  m_declInterfaces.reset();
  m_usedTraits.clear();
}

void Class::destroy() {
  /*
   * If we were never put on NamedEntity::classList, or
   * we've already been destroy'd, there's nothing to do
   */
  if (!m_cachedClass.bound()) return;

  Lock l(Unit::s_classesMutex);
  // Need to recheck now we have the lock
  if (!m_cachedClass.bound()) return;
  // Only do this once.
  m_cachedClass = RDS::Link<Class*>(RDS::kInvalidHandle);

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
  Treadmill::enqueue(
    [this] { if (!this->decAtomicCount()) this->atomicRelease(); }
  );
}

void Class::atomicRelease() {
  assert(!m_cachedClass.bound());
  assert(!getCount());
  this->~Class();
  low_free(this);
}

Class *Class::getCached() const {
  return *m_cachedClass;
}

void Class::setCached() {
  *m_cachedClass = this;
}

bool Class::verifyPersistent() const {
  if (!(attrs() & AttrPersistent)) return false;
  if (m_parent.get() &&
      !RDS::isPersistentHandle(m_parent->classHandle())) {
    return false;
  }
  for (auto const& declInterface : declInterfaces()) {
    if (!RDS::isPersistentHandle(declInterface->classHandle())) {
      return false;
    }
  }
  for (auto const& usedTrait : m_usedTraits) {
    if (!RDS::isPersistentHandle(usedTrait->classHandle())) {
      return false;
    }
  }
  return true;
}

const Func* Class::getDeclaredCtor() const {
  const Func* f = getCtor();
  return f->name() != sd86ctor ? f : nullptr;
}

/*
 * Check whether a Class from a previous request is available to be
 * defined.  The caller should check that it has the same preClass that is
 * being defined.  Being available means that the parent, the interfaces
 * and the traits are already defined (or become defined via autoload, if
 * tryAutoload is true).
 *
 * returns Avail::True - if it is available
 *         Avail::Fail - if it is impossible to define the class at this point
 *         Avail::False- if this particular Class* cant be defined at this point
 *
 * Note that Fail means that at least one of the parent, interfaces and
 * traits was not defined at all, while False means that at least one was
 * defined but did not correspond to this Class*
 *
 * The parent parameter is used for two purposes: first it avoids looking
 * up the active parent class for each potential Class*; and second its
 * used on Fail to return the problem class so the caller can report the
 * error correctly.
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
  for (auto const& di : declInterfaces()) {
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

const Class* Class::commonAncestor(const Class* cls) const {
  assert(isNormalClass(this) && isNormalClass(cls));

  // Walk up m_classVec for both classes to look for a common ancestor.
  auto vecIdx = std::min(m_classVecLen, cls->m_classVecLen) - 1;
  do {
    assert(vecIdx >= 0 &&
           vecIdx < m_classVecLen && vecIdx < cls->m_classVecLen);
    if (m_classVec[vecIdx] == cls->m_classVec[vecIdx]) {
      return m_classVec[vecIdx];
    }
  } while (vecIdx--);

  return nullptr;
}

void Class::initialize(TypedValue*& sProps) const {
  if (m_pinitVec.size() > 0) {
    if (getPropData() == nullptr) {
      initProps();
    }
  }
  // The asymmetry between the logic around initProps() above and initSProps()
  // below is due to the fact that instance properties only require storage in
  // g_context if there are non-scalar initializers involved, whereas static
  // properties *always* require storage in g_context.
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
  auto propVec = PropInitVec::allocWithSmartAllocator(m_declPropInit);

  setPropData(propVec);

  try {
    // Iteratively invoke 86pinit() methods upward
    // through the inheritance chain.
    for (auto it = m_pinitVec.rbegin(); it != m_pinitVec.rend(); ++it) {
      TypedValue retval;
      g_context->invokeFunc(&retval, *it, init_null_variant, nullptr,
                              const_cast<Class*>(this));
      assert(retval.m_type == KindOfNull);
    }
  } catch (...) {
    // Undo the allocation of propVec
    smart_delete_array(propVec->begin(), propVec->size());
    smart_delete(propVec);
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
        !g_context->debuggerSettings.bypassCheck) {
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
        if (ctx == (Class*)-1 || ctx->classof(baseClass)) {
          // the special ctx (Class*)-1 is used by unserialization to
          // mean that protected properties are ok. Otherwise,
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
  if (ctx && ctx != (Class*)-1 && classof(ctx)) {
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
  assert(getSPropData() == nullptr);

  // Initialize static props for parent
  Class* parent = this->parent();
  if (parent && parent->getSPropData() == nullptr) {
    parent->initSPropsImpl();
  }

  if (!numStaticProperties()) return nullptr;

  // Create an array that is initially large enough to hold all static
  // properties.
  TypedValue* const spropTable =
    smart_new_array<TypedValue>(m_staticProperties.size());
  memset(spropTable, 0, m_staticProperties.size() * sizeof(TypedValue));
  setSPropData(spropTable);

  const bool hasNonscalarInit = !m_sinitVec.empty();

  // If there are non-scalar initializers (i.e. 86sinit methods), run them now.
  // They'll put their initialized values into an array, and we'll read any
  // values we need out of the array later.
  if (hasNonscalarInit) {
    for (unsigned i = 0; i < m_sinitVec.size(); i++) {
      TypedValue retval;
      g_context->invokeFunc(&retval, m_sinitVec[i], init_null_variant,
                              nullptr, const_cast<Class*>(this));
      assert(retval.m_type == KindOfNull);
    }
  }

  for (Slot slot = 0; slot < m_staticProperties.size(); ++slot) {
    auto const& sProp = m_staticProperties[slot];
    auto const* propName = sProp.m_name;

    if (sProp.m_class == this) {
      if (spropTable[slot].m_type == KindOfUninit) {
        spropTable[slot] = sProp.m_val;
      }
    } else {
      bool visible, accessible;
      auto* storage = sProp.m_class->getSProp(nullptr, propName,
                                              visible, accessible);
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
        accessible = g_context->debuggerSettings.bypassCheck; break;
      default:            not_reached();
      }
    } else {
      // Property access is in an effectively anonymous context, so only public
      // properties are accessible.
      switch (sPropAttrs & (AttrPublic|AttrProtected|AttrPrivate)) {
      case AttrPublic:    accessible = true; break;
      case AttrProtected:
      case AttrPrivate:
        accessible = g_context->debuggerSettings.bypassCheck; break;
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

Cell Class::clsCnsGet(const StringData* clsCnsName) const {
  Slot clsCnsInd;
  Cell* clsCns = cnsNameToTV(clsCnsName, clsCnsInd);
  if (!clsCns) return make_tv<KindOfUninit>();
  if (clsCns->m_type != KindOfUninit) {
    return *clsCns;
  }

  // This constant has a non-scalar initializer, meaning it will be
  // potentially different in different requests, which we store
  // separately in an array living off in RDS.
  m_nonScalarConstantCache.bind();
  auto& clsCnsData = *m_nonScalarConstantCache;

  if (clsCnsData.get() == nullptr) {
    clsCnsData = Array::attach(HphpArray::MakeReserve(m_constants.size()));
  } else {
    clsCns = clsCnsData->nvGet(clsCnsName);
    if (clsCns) return *clsCns;
  }

  // The class constant has not been initialized yet; do so.
  static auto const sd86cinit = makeStaticString("86cinit");
  auto const meth86cinit =
    m_constants[clsCnsInd].m_class->lookupMethod(sd86cinit);
  TypedValue args[1] = {
    make_tv<KindOfStaticString>(
      const_cast<StringData*>(m_constants[clsCnsInd].m_name))
  };

  Cell ret;
  g_context->invokeFuncFew(
    &ret,
    meth86cinit,
    ActRec::encodeClass(this),
    nullptr,
    1,
    args
  );
  assert(isUncounted(ret));

  clsCnsData.set(StrNR(clsCnsName), cellAsCVarRef(ret), true /* isKey */);

  assert(cellIsPlausible(ret));
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

void Class::setParent() {
  // Validate the parent
  if (m_parent.get() != nullptr) {
    Attr attrs = m_parent->attrs();
    if (UNLIKELY(attrs & (AttrFinal | AttrInterface | AttrTrait))) {
      static StringData* sd___MockClass =
        makeStaticString("__MockClass");
      if (!(attrs & AttrFinal) ||
          m_preClass->userAttributes().find(sd___MockClass) ==
          m_preClass->userAttributes().end() ||
          m_parent->isCollectionClass()) {
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
    m_instanceCtor = m_preClass->instanceCtor();
    m_instanceDtor = m_preClass->instanceDtor();
    m_builtinODTailSize = m_preClass->builtinObjSize() -
      m_preClass->builtinODOffset();
    m_clsInfo = ClassInfo::FindSystemClassInterfaceOrTrait(nameRef());
  } else if (m_parent.get()) {
    m_instanceCtor = m_parent->m_instanceCtor;
    m_instanceDtor = m_parent->m_instanceDtor;
    m_builtinODTailSize = m_parent->m_builtinODTailSize;
    // XXX: should this be copying over the clsInfo also?  Might be
    // broken...
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
  s_call("__call"),
  s_callStatic("__callStatic"),
  s_clone("__clone");

void Class::setSpecial() {
  m_toString = lookupMethod(s_toString.get());
  m_dtor = lookupMethod(s_destruct.get());

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
  if (m_invoke &&
      (m_invoke->attrs() & AttrStatic) &&
       !m_invoke->isClosureBody()) {
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
    // Look for Foo::Foo() declared in this class
    if (matchedClassOrIsTrait(m_preClass->name())) {
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
  assert((m_ctor->attrs() & ~AttrBuiltin & ~AttrAbstract) ==
         (AttrPublic|AttrNoInjection|AttrPhpLeafFn));
}

void Class::applyTraitPrecRule(const PreClass::TraitPrecRule& rule,
                               MethodToTraitListMap& importMethToTraitMap) {
  const StringData* methName          = rule.getMethodName();
  const StringData* selectedTraitName = rule.getSelectedTraitName();
  auto otherTraitNames = rule.getOtherTraitNames();

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
    raise_error(Strings::TRAITS_UNKNOWN_TRAIT, selectedTraitName->data());
  }
  if (otherTraitNames.size()) {
    raise_error(Strings::TRAITS_UNKNOWN_TRAIT,
                (*otherTraitNames.begin())->data());
  }
}

Class* Class::findSingleTraitWithMethod(const StringData* methName) {
  // Note: m_methods includes methods from parents / traits recursively
  Class* traitCls = nullptr;
  for (auto const& t : m_usedTraits) {
    if (t->m_methods.contains(methName)) {
      if (traitCls != nullptr) { // more than one trait contains method
        raise_error("more than one trait contains method '%s'",
          methName->data());
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
  const StringData* origName = makeStaticString(buf);
  m_traitAliases.push_back(std::pair<const StringData*, const StringData*>
                           (newMethName, origName));
}

// not const due to memoization
const Class::TraitAliasVec& Class::traitAliases() {

  // We keep track of trait aliases in the class only to support
  // ReflectionClass::getTraitAliases. So let's load this info from the
  // preClass only on demand.
  auto const& preClassRules = m_preClass->traitAliasRules();
  if (m_traitAliases.size() != preClassRules.size()) {
    for (auto const& rule : preClassRules) {
      addTraitAlias(rule.getTraitName(),
                    rule.getOrigMethodName(),
                    rule.getNewMethodName());
    }
  }
  return m_traitAliases;
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
    raise_error(Strings::TRAITS_UNKNOWN_TRAIT, traitName->data());
  }

  // Save info to support ReflectionClass::getTraitAliases
  addTraitAlias(traitName, origMethName, newMethName);

  Func* traitMeth = traitCls->lookupMethod(origMethName);
  if (!traitMeth) {
    raise_error(Strings::TRAITS_UNKNOWN_TRAIT_METHOD, origMethName->data());
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
}

void Class::applyTraitRules(MethodToTraitListMap& importMethToTraitMap) {
  for (auto const& precRule : m_preClass->traitPrecRules()) {
    applyTraitPrecRule(precRule, importMethToTraitMap);
  }
  for (auto const& aliasRule : m_preClass->traitAliasRules()) {
    applyTraitAliasRule(aliasRule, importMethToTraitMap);
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

      raise_error(Strings::METHOD_IN_MULTIPLE_TRAITS,
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
      makeStaticString("__MockClass");
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

  // This used to be a global bool that guarded all bug-for-bug
  // compatibility with hphpi, but it's moved here because this is the
  // last use site.  (We need to evaluate if we can remove this one.)
  const bool hphpiCompat = true;

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

  if (Class::MethodCreateHook) {
    Class::MethodCreateHook(this, builder);
    // running MethodCreateHook may add methods to builder
    m_traitsEndIdx = builder.size();
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
  m_ODAttrs = 0;
  if (lookupMethod(s_sleep.get()     )) { m_ODAttrs |= ObjectData::HasSleep; }
  if (lookupMethod(s_get.get()       )) { m_ODAttrs |= ObjectData::UseGet;   }
  if (lookupMethod(s_set.get()       )) { m_ODAttrs |= ObjectData::UseSet;   }
  if (lookupMethod(s_isset.get()     )) { m_ODAttrs |= ObjectData::UseIsset; }
  if (lookupMethod(s_unset.get()     )) { m_ODAttrs |= ObjectData::UseUnset; }
  if (lookupMethod(s_call.get()      )) { m_ODAttrs |= ObjectData::HasCall;  }
  if (lookupMethod(s_clone.get()     )) { m_ODAttrs |= ObjectData::HasClone; }
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
  for (int i = 0, size = m_interfaces.size(); i < size; ++i) {
    const Class* iface = m_interfaces[i];

    for (Slot slot = 0; slot < iface->m_constants.size(); ++slot) {
      auto const iConst = iface->m_constants[slot];

      // If you're inheriting a constant with the same name as an
      // existing one, they must originate from the same place.
      auto const existing = builder.find(iConst.m_name);
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
  Slot traitOffset = 0;

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
      prop.m_repoAuthType = parentProp.m_repoAuthType;
      // Temporarily assign parent properties' indexes to their additive
      // inverses minus one. After assigning current properties' indexes,
      // we will use these negative indexes to assign new indexes to
      // parent properties that haven't been overlayed.
      prop.m_idx = -parentProp.m_idx - 1;
      if (traitOffset < -prop.m_idx) {
        traitOffset = -prop.m_idx;
      }
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
      sProp.m_idx = -parentProp.m_idx - 1;
      if (traitOffset < -sProp.m_idx) {
        traitOffset = -sProp.m_idx;
      }
      tvWriteUninit(&sProp.m_val);
      curSPropMap.add(sProp.m_name, sProp);
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
        prop.m_repoAuthType = preProp->repoAuthType();
        if (slot < traitIdx) {
          prop.m_idx = slot;
        } else {
          prop.m_idx = slot + m_preClass->numProperties() + traitOffset;
        }
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
          Prop& prop = curPropMap[it2->second];
          assert((prop.m_attrs & (AttrPublic|AttrProtected|AttrPrivate)) ==
                 AttrProtected);
          prop.m_class = this;
          prop.m_docComment = preProp->docComment();
          if (slot < traitIdx) {
            prop.m_idx = slot;
          } else {
            prop.m_idx = slot + m_preClass->numProperties() + traitOffset;
          }
          const TypedValue& tv = m_preClass->lookupProp(preProp->name())->val();
          TypedValueAux& tvaux = m_declPropInit[it2->second];
          tvaux.m_data = tv.m_data;
          tvaux.m_type = tv.m_type;
          copyDeepInitAttr(preProp, &prop);
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
        prop.m_repoAuthType = preProp->repoAuthType();
        if (slot < traitIdx) {
          prop.m_idx = slot;
        } else {
          prop.m_idx = slot + m_preClass->numProperties() + traitOffset;
        }
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
          prop.m_class = this;
          prop.m_docComment = preProp->docComment();
          if ((prop.m_attrs & (AttrPublic|AttrProtected|AttrPrivate))
              == AttrProtected) {
            // Weaken protected property to public.
            prop.m_mangledName = preProp->mangledName();
            prop.m_originalMangledName = preProp->mangledName();
            prop.m_attrs = Attr(prop.m_attrs ^ (AttrProtected|AttrPublic));
            prop.m_typeConstraint = preProp->typeConstraint();
          }
          if (slot < traitIdx) {
            prop.m_idx = slot;
          } else {
            prop.m_idx = slot + m_preClass->numProperties() + traitOffset;
          }
          const TypedValue& tv = m_preClass->lookupProp(preProp->name())->val();
          TypedValueAux& tvaux = m_declPropInit[it2->second];
          tvaux.m_data = tv.m_data;
          tvaux.m_type = tv.m_type;
          copyDeepInitAttr(preProp, &prop);
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
        prop.m_repoAuthType = preProp->repoAuthType();
        if (slot < traitIdx) {
          prop.m_idx = slot;
        } else {
          prop.m_idx = slot + m_preClass->numProperties() + traitOffset;
        }
        curPropMap.add(preProp->name(), prop);
        m_declPropInit.push_back(m_preClass->lookupProp(preProp->name())
                                 ->val());
        break;
      }
      default: assert(false);
      }
    } else { // Static property.
      // Prohibit non-static-->static redeclaration.
      auto const it2 = curPropMap.find(preProp->name());
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
      auto const it3 = curSPropMap.find(preProp->name());
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
      // Finish initializing.
      auto& sProp = curSPropMap[sPropInd];
      sProp.m_attrs          = preProp->attrs();
      sProp.m_typeConstraint = preProp->typeConstraint();
      sProp.m_docComment     = preProp->docComment();
      sProp.m_class          = this;
      sProp.m_val            = m_preClass->lookupProp(preProp->name())->val();
      sProp.m_repoAuthType   = preProp->repoAuthType();
      if (slot < traitIdx) {
        sProp.m_idx = slot;
      } else {
        sProp.m_idx = slot + m_preClass->numProperties() + traitOffset;
      }
    }
  }

  // After assigning indexes for current properties, we reassign indexes to
  // parent properties that haven't been overlayed to make sure that they
  // are greater than those of current properties.
  int idxOffset = m_preClass->numProperties() - 1;
  int curIdx = idxOffset;
  for (Slot slot = 0; slot < curPropMap.size(); ++slot) {
    Prop& prop = curPropMap[slot];
    if (prop.m_idx < 0) {
      prop.m_idx = idxOffset - prop.m_idx;
      if (curIdx < prop.m_idx) {
        curIdx = prop.m_idx;
      }
    }
  }
  for (Slot slot = 0; slot < curSPropMap.size(); ++slot) {
    SProp& sProp = curSPropMap[slot];
    if (sProp.m_idx < 0) {
      sProp.m_idx = idxOffset - sProp.m_idx;
      if (curIdx < sProp.m_idx) {
        curIdx = sProp.m_idx;
      }
    }
  }

  importTraitProps(curIdx + 1, curPropMap, curSPropMap);

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
                                    const int idxOffset,
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
    if (prop.m_attrs & AttrDeepInit) {
      m_hasDeepInitProps = true;
    }
    prop.m_idx += idxOffset;
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
                                  const int idxOffset,
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
    prop.m_idx += idxOffset;
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

void Class::importTraitProps(int idxOffset,
                             PropMap::Builder& curPropMap,
                             SPropMap::Builder& curSPropMap) {
  if (attrs() & AttrNoExpandTrait) return;
  for (auto& t : m_usedTraits) {
    Class* trait = t.get();

    // instance properties
    for (Slot p = 0; p < trait->m_declProperties.size(); p++) {
      Prop& traitProp          = trait->m_declProperties[p];
      TypedValue& traitPropVal = trait->m_declPropInit[p];
      importTraitInstanceProp(trait, traitProp, traitPropVal, idxOffset,
                              curPropMap);
    }

    // static properties
    for (Slot p = 0; p < trait->m_staticProperties.size(); ++p) {
      SProp& traitProp = trait->m_staticProperties[p];
      importTraitStaticProp(trait, traitProp, idxOffset, curPropMap,
                            curSPropMap);
    }

    idxOffset += trait->m_declProperties.size() +
                 trait->m_staticProperties.size();
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
  static StringData* sd__init__ = makeStaticString("__init__");
  static StringData* sd_exn = makeStaticString("Exception");
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

/*
 * Look up the interfaces implemented by traits used by the class, and add them
 * to the provided builder.
 */
void Class::addInterfacesFromUsedTraits(InterfaceMap::Builder& builder) const {

  for (auto const& trait: m_usedTraits) {
    int numIfcs = trait->m_interfaces.size();

    for (int i = 0; i < numIfcs; i++) {
      Class* interface = trait->m_interfaces[i];
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
      Class* interface = m_parent->m_interfaces[i];
      interfacesBuilder.add(interface->name(), interface);
    }
  }

  std::vector<ClassPtr> declInterfaces;

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
    declInterfaces.push_back(ClassPtr(cp));
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

  m_numDeclInterfaces = declInterfaces.size();
  m_declInterfaces.reset(new ClassPtr[declInterfaces.size()]);
  std::copy(begin(declInterfaces), end(declInterfaces),
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
      interfacesBuilder.add(stringish->name(), stringish);
    }
  }

  m_interfaces.create(interfacesBuilder);
  checkInterfaceMethods();
}

void Class::setNativeDataInfo() {
  for (auto cls = this; cls; cls = cls->parent()) {
    if ((m_nativeDataInfo = cls->preClass()->nativeDataInfo())) {
      m_instanceCtor = Native::nativeDataInstanceCtor;
      m_instanceDtor = Native::nativeDataInstanceDtor;
      break;
    }
  }
}

void Class::setUsedTraits() {
  for (auto const& traitName : m_preClass->usedTraits()) {
    Class* classPtr = Unit::loadClass(traitName);
    if (classPtr == nullptr) {
      raise_error(Strings::TRAITS_UNKNOWN_TRAIT, traitName->data());
    }
    if (!(classPtr->attrs() & AttrTrait)) {
      raise_error("%s cannot use %s - it is not a trait",
                  m_preClass->name()->data(),
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

    m_usedTraits.push_back(ClassPtr(classPtr));
  }
}

void Class::checkTraitConstraints() const {

  if (attrs() & AttrInterface) {
    return; // nothing to do
  }

  if (attrs() & AttrTrait) {
    for (auto const& req : m_preClass->traitRequirements()) {
      auto const reqName = req.name();
      auto const reqCls = Unit::loadClass(reqName);
      if (!reqCls) {
        raise_error("%s '%s' required by trait '%s' cannot be loaded",
                    req.is_extends() ? "Class" : "Interface",
                    reqName->data(),
                    m_preClass->name()->data());
      }

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
    return;
  }

  checkTraitConstraintsRec(usedTraitClasses(), nullptr);
}

void Class::checkTraitConstraintsRec(const std::vector<ClassPtr>& usedTraits,
                                     const StringData* recName) const {

  if (!usedTraits.size()) { return; }

  for (auto const& ut : usedTraits) {
    auto const usedTrait = ut.get();
    auto const ptrait = usedTrait->preClass();

    for (auto const& req : ptrait->traitRequirements()) {
      auto const reqName = req.name();
      if (req.is_extends()) {
        auto reqExtCls = Unit::lookupClass(reqName);
        // errors should've been raised for the following when the
        // usedTrait was first loaded
        assert(reqExtCls != nullptr);
        assert(!(reqExtCls->attrs() & (AttrTrait | AttrInterface)));

        if ((m_classVecLen < reqExtCls->m_classVecLen) ||
            (m_classVec[reqExtCls->m_classVecLen-1] != reqExtCls)) {
          raise_error(Strings::TRAIT_REQ_EXTENDS,
                      m_preClass->name()->data(),
                      reqName->data(),
                      ptrait->name()->data(),
                      ((recName == nullptr) ? "use" : recName->data()));
        }
        continue;
      }

      assert(req.is_implements());
      if (!ifaceofDirect(reqName)) {
        raise_error(Strings::TRAIT_REQ_IMPLEMENTS,
                    m_preClass->name()->data(),
                    reqName->data(),
                    ptrait->name()->data(),
                    ((recName == nullptr) ? "use" : recName->data()));
      }
    }
  }

  // separate loop for recursive checks
  for (auto const& ut : usedTraits) {
    Class* usedTrait = ut.get();
    checkTraitConstraintsRec(
      usedTrait->usedTraitClasses(),
      recName == nullptr ? usedTrait->preClass()->name() : recName
    );
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

// Finds the base class defining the given method (NULL if none).
// Note: for methods imported via traits, the base class is the one that
// uses/imports the trait.
Class* Class::findMethodBaseClass(const StringData* methName) {
  const Func* f = lookupMethod(methName);
  if (f == nullptr) return nullptr;
  return f->baseCls();
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
  ci->m_parentClass = (m_parent.get()) ? m_parent->name()->data() : "";

  // Interfaces.
  for (auto const& ifaceName: m_preClass->interfaces()) {
    ci->m_interfacesVec.push_back(ifaceName->data());
    ci->m_interfaces.insert(ifaceName->data());
  }
  if (m_interfaces.size() > m_preClass->interfaces().size()) {
    for (int i = 0; i < m_interfaces.size(); ++i) {
      auto const& ifaceName = m_interfaces[i]->name();

      if (ci->m_interfaces.find(ifaceName->data()) == ci->m_interfaces.end()) {
        ci->m_interfacesVec.push_back(ifaceName->data());
        ci->m_interfaces.insert(ifaceName->data());
      }
    }
  }
  assert(ci->m_interfaces.size() == ci->m_interfacesVec.size());

  // Used traits.
  for (auto const& traitName : m_preClass->usedTraits()) {
    // Use the preclass list of trait names to avoid accounting for
    // trait flattening.
    const char* traitNameChars = traitName->data();
    ci->m_traitsVec.push_back(traitNameChars);
    ci->m_traits.insert(traitNameChars);
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
    auto const cell = clsCnsGet(m_constants[i].m_name);
    assert(cell.m_type != KindOfUninit);
    ki->setValue(cellAsCVarRef(cell));

    ci->m_constants[ki->name] = ki;
    ci->m_constantsVec.push_back(ki);
  }
}

size_t Class::declPropOffset(Slot index) const {
  assert(index >= 0);
  return sizeof(ObjectData) + m_builtinODTailSize + index * sizeof(TypedValue);
}

Class::PropInitVec::~PropInitVec() {
  if (!m_smart) free(m_data);
}

Class::PropInitVec::PropInitVec() : m_data(nullptr), m_size(0), m_smart(false) {}

Class::PropInitVec*
Class::PropInitVec::allocWithSmartAllocator(const PropInitVec& src) {
  PropInitVec* p = smart_new<PropInitVec>();
  p->m_size = src.size();
  p->m_data = smart_new_array<TypedValueAux>(src.size());
  memcpy(p->m_data, src.m_data, src.size() * sizeof(*p->m_data));
  p->m_smart = true;
  return p;
}

const Class::PropInitVec&
Class::PropInitVec::operator=(const PropInitVec& piv) {
  assert(!m_smart);
  if (this != &piv) {
    unsigned sz = m_size = piv.size();
    if (sz) sz = folly::nextPowTwo(sz);
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
  if (!m_size || folly::isPowTwo(m_size)) {
    unsigned size = m_size ? m_size * 2 : 1;
    m_data = (TypedValueAux*)realloc(m_data, size * sizeof(*m_data));
    assert(m_data);
  }
  cellDup(v, m_data[m_size++]);
}

Class::PropInitVec* Class::getPropData() const {
  return m_propDataCache.bound() ? *m_propDataCache : nullptr;
}

void Class::initPropHandle() const {
  m_propDataCache.bind();
}

void Class::initProps() const {
  initPropsImpl();
}

void Class::setPropData(PropInitVec* propData) const {
  assert(getPropData() == nullptr);
  initPropHandle();
  *m_propDataCache = propData;
}

TypedValue* Class::getSPropData() const {
  return m_propSDataCache.bound() ? *m_propSDataCache : nullptr;
}

void Class::initSPropHandle() const {
  m_propSDataCache.bind();
}

TypedValue* Class::initSProps() const {
  TypedValue* sprops = initSPropsImpl();
  return sprops;
}

void Class::setSPropData(TypedValue* sPropData) const {
  assert(getSPropData() == nullptr);
  initSPropHandle();
  *m_propSDataCache = sPropData;
}

void Class::getChildren(std::vector<TypedValue*>& out) {
  for (Slot i = 0; i < m_staticProperties.size(); ++i) {
    if (m_staticProperties[i].m_class != this) continue;
    out.push_back(&m_staticProperties[i].m_val);
  }
}

// True if a CPP extension class has opted into serialization.
bool Class::isCppSerializable() const {
  assert(instanceCtor()); // Only call this on CPP classes
  auto info = clsInfo();
  auto p = this;
  while ((!info) && (p = p->parent())) {
    info = p->clsInfo();
  }
  return info &&
    (info->getAttribute() & ClassInfo::IsCppSerializable);
}

bool Class::isCollectionClass() const {
  auto s = name();
  return Collection::stringToType(s->data(), s->size()) !=
         Collection::InvalidType;
}

RefData* Class::zGetSProp(Class* ctx, const StringData* sPropName,
                          bool& visible, bool& accessible) const {
  auto tv = getSProp(ctx, sPropName, visible, accessible);
  if (tv->m_type != KindOfRef) {
    tvBox(tv);
  }
  return tv->m_data.pref;
}

} // HPHP::VM
