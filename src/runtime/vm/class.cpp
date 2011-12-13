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

#include <iostream>

#include "runtime/base/base_includes.h"
#include "runtime/base/tv_macros.h"
#include "util/util.h"
#include "util/debug.h"
#include "runtime/vm/core_types.h"
#include "runtime/vm/hhbc.h"
#include "runtime/vm/class.h"
#include "system/lib/systemlib.h"
#include "util/logger.h"

namespace HPHP {
namespace VM {

hphp_hash_map<const StringData*, const HhbcExtClassInfo*,
              string_data_hash, string_data_isame> Class::s_extClassHash;

static const bool ErrFinalOverride = false;

//=============================================================================
// PreClass::Prop.

PreClass::Prop::Prop(PreClass* preClass, const StringData* n, Attr attrs,
                     const StringData* docComment, TypedValue* val)
  : m_preClass(preClass), m_name(n), m_attrs(attrs),
    m_docComment(docComment) {
  m_mangledName = mangleName();
  memcpy(&m_val, val, sizeof(TypedValue));
}

PreClass::Prop::~Prop() {
}

const StringData* PreClass::Prop::mangleName() const {
  switch (m_attrs & (AttrPublic|AttrProtected|AttrPrivate)) {
  case AttrPublic: {
    return m_name;
  }
  case AttrProtected: {
    std::string mangledName = "";
    mangledName.push_back('\0');
    mangledName.push_back('*');
    mangledName.push_back('\0');
    mangledName += m_name->data();
    return StringData::GetStaticString(mangledName);
  }
  case AttrPrivate: {
    std::string mangledName = "";
    mangledName.push_back('\0');
    mangledName += m_preClass->m_name->data();
    mangledName.push_back('\0');
    mangledName += m_name->data();
    return StringData::GetStaticString(mangledName);
  }
  default: not_reached();
  }
}

void PreClass::Prop::prettyPrint(std::ostream& out) {
  out << "Property ";
  if (m_attrs & AttrStatic) { out << "static "; }
  if (m_attrs & AttrPublic) { out << "public "; }
  if (m_attrs & AttrProtected) { out << "protected "; }
  if (m_attrs & AttrPrivate) { out << "private "; }
  out << m_preClass->m_name->data() << "::" << m_name->data() << " = ";
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

PreClass::Const::Const(PreClass* preClass, const StringData* n, TypedValue* val,
                       const StringData* phpCode)
  : m_preClass(preClass), m_name(n), m_phpCode(phpCode) {
  memcpy(&m_val, val, sizeof(TypedValue));
}

PreClass::Const::~Const() {
}

void PreClass::Const::prettyPrint(std::ostream& out) {
  out << "Constant " << m_preClass->m_name->data() << "::" << m_name->data()
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

PreClass::PreClass(Unit* unit, const Location* sLoc, Offset o,
                   const StringData* n, Attr attrs, const StringData* parent,
                   const StringData* docComment, Id id, bool hoistable)
  : m_unit(unit), m_line1(sLoc->line0), m_line2(sLoc->line1), m_offset(o),
    m_name(n), m_attrs(attrs), m_parent(parent), m_docComment(docComment),
    m_id(id), m_hoistable(hoistable) {
}

PreClass::~PreClass() {
  destroyMembers(m_methods);
  destroyMembers(m_propertyVec);
  destroyMembers(m_constantVec);
}

void PreClass::release() {
  delete this;
}

void PreClass::addInterface(const StringData* n) {
  m_interfaces.push_back(n);
}

bool PreClass::addMethod(Func* method) {
  MethodMap::const_iterator it = m_methodMap.find(method->m_name);
  if (it != m_methodMap.end()) {
    return false;
  }
  m_methods.push_back(method);
  m_methodMap[method->m_name] = method;
  return true;
}

bool PreClass::addProperty(const StringData* n, Attr attrs,
                           const StringData* docComment, TypedValue* val) {
  PropertyMap::const_iterator it = m_propertyMap.find(n);
  if (it != m_propertyMap.end()) {
    return false;
  }
  Prop* prop = new Prop(this, n, attrs, docComment, val);
  m_propertyVec.push_back(prop);
  m_propertyMap[prop->m_name] = prop;
  return true;
}

bool PreClass::addConstant(const StringData* n, TypedValue* val,
                           const StringData* phpCode) {
  ConstantMap::const_iterator it = m_constantMap.find(n);
  if (it != m_constantMap.end()) {
    return false;
  }
  Const* const_ = new Const(this, n, val, phpCode);
  m_constantMap[const_->m_name] = m_constantVec.size();
  m_constantVec.push_back(const_);
  return true;
}

void PreClass::prettyPrint(std::ostream &out) const {
  out << "Class ";
  if (m_attrs & AttrAbstract) { out << "abstract "; }
  if (m_attrs & AttrFinal) { out << "final "; }
  if (m_attrs & AttrInterface) { out << "interface "; }
  out << m_name->data() << " at " << m_offset;
  if (m_hoistable) {
    out << " (hoistable)";
  }
  if (m_id != -1) {
    out << " (ID " << m_id << ")";
  }
  out << std::endl;
  for (MethodVec::const_iterator it = m_methods.begin();
       it != m_methods.end(); ++it) {
    out << " ";
    (*it)->prettyPrint(out);
  }
  for (PropertyVec::const_iterator it = m_propertyVec.begin();
       it != m_propertyVec.end(); ++it) {
    out << " ";
    (*it)->prettyPrint(out);
  }
  for (ConstantVec::const_iterator it = m_constantVec.begin();
       it != m_constantVec.end(); ++it) {
    out << " ";
    (*it)->prettyPrint(out);
  }
}

//=============================================================================
// Class.

ClassPtr Class::newClass(PreClass* preClass, Class* parent, bool failIsFatal) {
  unsigned classVecLen = (parent != NULL) ? parent->m_classVecLen+1 : 1;
  void* mem = malloc(sizeForNClasses(classVecLen));
  bool fail = false;
  Class* c = new (mem) Class(preClass, parent, classVecLen, failIsFatal, fail);
  if (fail) {
    ASSERT(!failIsFatal);
    c->release();
    return ClassPtr(NULL);
  }
  if (parent != NULL) {
    c->m_builtinPropSize = parent->m_builtinPropSize;
    if (parent->m_derivesFromBuiltin) {
      c->m_derivesFromBuiltin = true;
      c->m_baseBuiltinCls = parent->m_baseBuiltinCls;
    } else if (parent->m_isCppExtClass) {
      c->m_derivesFromBuiltin = true;
      c->m_baseBuiltinCls = parent;
    }
    c->m_InstanceCtor = parent->m_InstanceCtor;
  }
  return ClassPtr(c);
}

Class::Class(PreClass* preClass, Class* parent, unsigned classVecLen,
             bool failIsFatal, bool& fail)
  : m_preClass(PreClassPtr(preClass)), m_parent(ClassPtr(parent)),
    m_builtinPropSize(0), m_InstanceCtor(NULL),
    m_isCppExtClass(false), m_derivesFromBuiltin(false),
    m_baseBuiltinCls(NULL), m_classVecLen(classVecLen) {
  if (!validateParent(failIsFatal)
      || setUsedTraits(failIsFatal)
      || setMethods(failIsFatal)
      || setCtor(failIsFatal)
      || setODAttributes(failIsFatal)
      || setInterfaces(failIsFatal)
      || setConstants(failIsFatal)
      || setProperties(failIsFatal)
      || setInitializers(failIsFatal)
      || setClassVec(failIsFatal)) {
    ASSERT(!failIsFatal);
    fail = true;
  }
}

void Class::release() {
  this->~Class();
  free(this);
}

Class::Equiv Class::equiv(const PreClass* preClass,
                          bool tryAutoload /*=false*/) const {
  // Classes can't possibly be compatible unless they come from the same source
  // code.
  if (m_preClass.get() != preClass) {
    return EquivFalse;
  }
  // Check parent.
  if (preClass->m_parent->size() != 0) {
    Class* parent = g_context->getClass(preClass->m_parent, tryAutoload);
    if (parent != m_parent.get()) {
      return (parent == NULL) ? EquivFail : EquivFalse;
    }
  }
  // Check interfaces.
  if (m_interfaces.size() != preClass->m_interfaces.size()) {
    return EquivFalse;
  }
  for (size_t i = 0, nInterfaces = m_interfaces.size(); i < nInterfaces; ++i) {
    Class* interface = g_context->getClass(preClass->m_interfaces[i],
                                           tryAutoload);
    if (interface != m_interfaces[i].get()) {
      return (interface == NULL) ? EquivFail : EquivFalse;
    }
  }
  // Check used traits.
  if (m_usedTraits.size() != preClass->m_usedTraits.size()) {
    return EquivFalse;
  }
  for (size_t i = 0; i < m_usedTraits.size(); i++) {
    Class* trait = g_context->getClass(preClass->m_usedTraits[i], tryAutoload);
    if (trait != m_usedTraits[i].get()) {
      return (trait == NULL) ? EquivFail : EquivFalse;
    }
  }
  return EquivTrue;
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
    std::vector<ClassPtr>& interfaces = class_->m_interfaces;
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

void Class::initialize(HphpArray*& sProps) const {
  if (m_pinitVec.size() > 0) {
    if (g_context->getPropData(this) == NULL) {
      // Initialization was not done, do so for the first time.
      PropInitVec* props = initProps();
      g_context->setPropData(this, props);
    }
  }
  // The asymmetry between the logic around initProps() above and initSProps()
  // below is due to the fact that instance properties only require storage in
  // g_context if there are non-scalar initializers involved, whereas static
  // properties *always* require storage in g_context.
  if (m_sPropInfo.size() > 0) {
    if ((sProps = g_context->getSPropData(this)) == NULL) {
      sProps = initSProps();
      g_context->setSPropData(this, sProps);
    }
  } else {
    sProps = NULL;
  }
}

void Class::initialize() const {
  HphpArray* sProps;
  initialize(sProps);
}

void Class::addBuiltinClassInfo(const HhbcExtClassInfo *info) {
  m_builtinPropSize = info->m_sizeof - sizeof(ObjectData);
  m_InstanceCtor = info->m_InstanceCtor;
  m_isCppExtClass = true;
}

Class::PropInitVec* Class::initProps() const {
  ASSERT(m_pinitVec.size() > 0);
  // Copy initial values for properties to a new vector that can be used to
  // complete initialization for non-scalar properties via the iterative
  // 86pinit() calls below.  86pinit() takes a reference to an array that
  // contains the initial property values; alias propVec inside propArr such
  // that propVec contains complete property initialization values as soon as
  // the 86pinit() calls are done.
  PropInitVec* propVec = new PropInitVec();
  *propVec = m_declPropInit;
  unsigned nProps = m_declPropInit.size();
  HphpArray* propArr = NEW(HphpArray)(nProps);
  propArr->incRefCount();
  // Create a sentinel that uniquely identifies uninitialized properties.
  ObjectData* sentinel = SystemLib::AllocPinitSentinel();
  sentinel->incRefCount();
  // Insert propArr and sentinel into the args array, transferring ownership.
  HphpArray* args = NEW(HphpArray)(2);
  args->incRefCount();
  {
    TypedValue tv;
    tv.m_data.parr = (ArrayData*)propArr;
    tv._count = 0;
    tv.m_type = KindOfArray;
    args->nvAppend(&tv, false);
    propArr->decRefCount();
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
  for (unsigned i = 0; i < nProps; ++i) {
    TypedValue& prop = (*propVec)[i];
    if (prop.m_type == KindOfUninit) {
      // Replace undefined values with propArr, which acts as a unique sentinel
      // for undefined properties in 86pinit().
      tvDup(tvSentinel, &prop);
    }
    const StringData* k = (m_declPropInfo[i].m_attrs & AttrPrivate)
                           ? m_declPropInfo[i].m_mangledName
                           : m_declPropInfo[i].m_name;
    propArr->migrateAndSet((StringData*)k, &prop);
  }
  // Iteratively invoke 86pinit() methods upward through the inheritance chain.
  for (Class::InitVec::const_reverse_iterator it = m_pinitVec.rbegin();
       it != m_pinitVec.rend(); ++it) {
    TypedValue retval;
    g_context->invokeFunc(&retval, *it, args, NULL, const_cast<Class*>(this));
    ASSERT(!IS_REFCOUNTED_TYPE(retval.m_type));
  }

  // Promote non-static arrays (that came from 86pinit) to static. This allows
  // us to use memcpy to initialize object properties, without conflicting with
  // the refcounting that happens on object destruction.
  for (PropInitVec::iterator it = propVec->begin();
       it != propVec->end(); ++it) {
    if (it->m_type == KindOfArray && !it->m_data.parr->isStatic()) {
      it->m_data.parr->setStatic();
      it->m_data.parr->onSetStatic();
    } else {
      // Properties can't be initialized to object values, and any strings in
      // here had better be static.
      ASSERT(!IS_REFCOUNTED_TYPE(it->m_type) || it->m_data.pvar->isStatic());
    }
  }

  // Free the args array, which in turn will free propArr (contained within).
  if (args->decRefCount() == 0) {
    args->release();
  } else {
    ASSERT(false);
  }
  return propVec;
}

HphpArray* Class::initSProps() const {
  ASSERT(m_sPropInfo.size() > 0);
  // Create an array that is initially large enough to hold all static
  // properties.
  HphpArray* sProps = NEW(HphpArray)(m_sPropInfo.size());
  sProps->incRefCount();
  // Iteratively initialize properties.  Non-scalar initializers are
  // initialized to KindOfUninit here, and the 86sinit()-based initialization
  // finishes the job later.
  for (SPropInfoVec::const_iterator it = m_sPropInfo.begin();
       it != m_sPropInfo.end(); ++it) {
    const SProp& sProp = *it;
    if (sProp.m_class == this) {
      // Embed static property value directly in array.
      sProps->nvSet((StringData*)sProp.m_name, (TypedValue*)&sProp.m_val,
                    false);
    } else {
      // Alias parent class's static property.  This is safe because the array
      // layout never changes after initialization.
      bool visible, accessible;
      TypedValue* val = sProp.m_class->getSProp(NULL, sProp.m_name, visible,
                                                accessible);
      sProps->migrateAndSet((StringData*)sProp.m_name, val);
    }
  }
  // Invoke 86sinit() if one exists for this class.
  if (m_sinit != NULL) {
    TypedValue retval;
    HphpArray* args = NEW(HphpArray)(1);
    args->incRefCount();
    // Insert sProps into the args array, temporarily transferring ownership of
    // sProps to the args array (so that COW is not triggered). When 86sinit()
    // returns, we will reclaim ownership of sProps.
    {
      TypedValue tv;
      tv.m_data.parr = (ArrayData*)sProps;
      tv._count = 0;
      tv.m_type = KindOfArray;
      args->nvAppend(&tv, false);
      sProps->decRefCount();
    }
    g_context->invokeFunc(&retval, m_sinit, args, NULL,
                          const_cast<Class*>(this));
    ASSERT(!IS_REFCOUNTED_TYPE(retval.m_type));
    // Reclaim ownership of sProps before freeing the args array
    sProps->incRefCount();
    // Release the args array; this won't free the sProps array because we
    // added a reference to it above
    ASSERT(args->getCount() == 1);
    args->release();
  }
  return sProps;
}

TypedValue* Class::getSProp(PreClass* ctx, const StringData* sPropName,
                            bool& visible, bool& accessible) const {
  HphpArray* sProps;
  initialize(sProps);

  int sPropInd = lookupSProp(sPropName);
  if (sPropInd == -1) {
    // Non-existant property.
    visible = false;
    accessible = false;
    return NULL;
  }

  visible = true;
  if (ctx == m_preClass.get()) {
    // Property access is from within a method of this class, so the property
    // is accessible.
    accessible = true;
  } else {
    Attr sPropAttrs = m_sPropInfo[sPropInd].m_attrs;
    bool related = (ctx != NULL);
    if (related) {
      if (classof(ctx) == NULL) {
        // Given an inheritance hierarchy, e.g. C extends B extends A, assuming
        // 'this' is of type B, we now know that ctx is not of type A, but we
        // also need to check whether ctx is of type C, as follows.
        Class* ctxClass = g_context->lookupClass(ctx->m_name);
        ASSERT(ctxClass != NULL);
        if (ctxClass->classof(m_preClass.get()) == NULL) {
          related = false;
        }
      }
    }
    if (related) {
      // Property access is from within a parent class's method, which is
      // allowed for protected/public properties.
      switch (sPropAttrs & (AttrPublic|AttrProtected|AttrPrivate)) {
      case AttrPublic:
      case AttrProtected: accessible = true; break;
      case AttrPrivate:   accessible = false; break;
      default:            not_reached();
      }
    } else {
      // Property access is in an effectively anonymous context, so only public
      // properties are accessible.
      switch (sPropAttrs & (AttrPublic|AttrProtected|AttrPrivate)) {
      case AttrPublic:    accessible = true; break;
      case AttrProtected:
      case AttrPrivate:   accessible = false; break;
      default:            not_reached();
      }
    }
  }

  ASSERT(sProps != NULL);
  TypedValue* sProp = sProps->nvGetValueRef(sPropInd);
  // NB: nvGet() returns NULL if sProp is KindOfUninit, so the following
  // assertion wouldn't always hold:
  //   ASSERT(sProps->nvGet(sPropName, false) == sProp);
  return sProp;
}

HphpArray* Class::getStaticLocals() {
  return g_context->getClsStaticCtx(this);
}

TypedValue Class::getStaticPropInitVal(const SProp& prop) {
  Class* declCls = prop.m_class;
  SPropMap::const_iterator propIt = declCls->m_sPropMap.find(prop.m_name);
  ASSERT(propIt != declCls->m_sPropMap.end());
  return declCls->m_sPropInfo[propIt->second].m_val;
}

HphpArray* Class::initClsCnsData() {
  unsigned nConstants = m_constantVec.size();
  HphpArray* constants = NEW(HphpArray)(nConstants);
  constants->incRefCount();

  if (m_parent.get() != NULL) {
    if (g_context->getClsCnsData(m_parent.get()) == NULL) {
      // Initialize recursively up the inheritance chain.
      m_parent->initClsCnsData();
    }
  }

  for (unsigned i = 0; i < nConstants; ++i) {
    Const& constant = m_constantVec[i];
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

  g_context->setClsCnsData(this, constants);
  return constants;
}

TypedValue* Class::clsCnsGet(const StringData* clsCnsName) {
  ConstantMap::const_iterator it = m_constantMap.find(clsCnsName);
  if (it == m_constantMap.end()) {
    return NULL;
  }
  unsigned clsCnsInd = it->second;
  TypedValue* clsCns = &m_constantVec[clsCnsInd].m_val;
  if (clsCns->m_type != KindOfUninit) {
    return clsCns;
  }
  // This constant has a non-scalar initializer, so look in g_context for
  // an entry associated with this class.
  HphpArray* clsCnsData = g_context->getClsCnsData(this);
  if (clsCnsData == NULL) {
    clsCnsData = initClsCnsData();
  }

  clsCns = clsCnsData->nvGetValueRef(clsCnsInd);
  if (clsCns->m_type == KindOfUninit) {
    // The class constant has not been initialized yet; do so.
    static StringData* sd86cinit = StringData::GetStaticString("86cinit");
    const Func* meth86cinit =
      m_constantVec[clsCnsInd].m_class->lookupMethod(sd86cinit);
    TypedValue tv;
    tv.m_data.pstr = (StringData*)clsCnsName;
    tv._count = 0;
    tv.m_type = KindOfString;
    g_context->invokeFunc(clsCns, meth86cinit,
                          CREATE_VECTOR1(tvAsCVarRef(&tv)), NULL, this);
  }
  return clsCns;
}

// Returns whether or not m_parent is a valid parent
bool Class::validateParent(bool failIsFatal) {
  if (m_parent.get() != NULL) {
    Attr attrs = m_parent->m_preClass->m_attrs;
    if (UNLIKELY(attrs & (AttrFinal | AttrInterface | AttrTrait))) {
      if (failIsFatal) {
        raise_error("Class %s may not inherit from %s (%s)",
                    m_preClass->m_name->data(),
                    ((attrs & AttrFinal)     ? "final class" :
                     (attrs & AttrInterface) ? "interface"   : "trait"),
                    m_parent->m_preClass->m_name->data());
      }
      return false;
    }
  }

  return true;
}

bool Class::setCtor(bool failIsFatal) {
  // Look for __construct() declared in either this class or a trait
  static StringData* sd__construct = StringData::GetStaticString("__construct");
  const MethodEntry* me_construct = lookupMethodEntry(sd__construct);
  if (me_construct && (me_construct->func->m_preClass == m_preClass.get() ||
                       me_construct->func->m_preClass->m_attrs & AttrTrait)) {
    m_ctor = *me_construct;
    return false;
  }

  // Look for Foo::Foo() declared in this class (cannot be via trait).
  const MethodEntry* me_namedCtor = lookupMethodEntry(m_preClass->m_name);
  if (me_namedCtor && me_namedCtor->func->m_preClass == m_preClass.get()) {
    m_ctor = *me_namedCtor;
    return false;
  }

  // Look for parent constructor other than 86ctor().
  static StringData* sd86ctor = StringData::GetStaticString("86ctor");
  if (m_parent.get() != NULL &&
      m_parent->m_ctor.func->m_name->compare(sd86ctor)) {
    m_ctor = m_parent->m_ctor;
    return false;
  }

  // Use 86ctor(), since no program-supplied constructor exists.
  PreClass::MethodMap::const_iterator
    it = m_preClass->m_methodMap.find(sd86ctor);
  ASSERT(it != m_preClass->m_methodMap.end());
  m_ctor.func            = it->second;
  m_ctor.baseClass       = this;
  m_ctor.ancestorPrivate = false;
  m_ctor.attrs           = AttrPublic;
  return false;
}

// returns true on error
bool Class::applyTraitPrecRule(const PreClass::TraitPrecRule& rule,
                               bool failIsFatal) {
  const StringData* methName          = rule.getMethodName();
  const StringData* selectedTraitName = rule.getSelectedTraitName();
  TraitNameSet      otherTraitNames;
  rule.getOtherTraitNames(otherTraitNames);

  MethodToTraitListMap::iterator methIter =
    m_importMethToTraitMap.find(methName);
  if (methIter == m_importMethToTraitMap.end()) {
    if (failIsFatal) {
      raise_error("unknown method '%s'", methName);
    }
    return true;
  }

  bool foundSelectedTrait = false;

  TraitMethodList &methList = methIter->second;
  for (TraitMethodList::iterator nextTraitIter = methList.begin();
       nextTraitIter != methList.end(); ) {
    TraitMethodList::iterator traitIter = nextTraitIter++;
    const StringData* availTraitName = traitIter->m_trait->m_preClass->m_name;
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
    if (failIsFatal) {
      raise_error("unknown trait '%s'", selectedTraitName);
    }
    return true;
  }
  if (otherTraitNames.size()) {
    if (failIsFatal) {
      raise_error("unknown trait '%s'", (*otherTraitNames.begin())->data());
    }
    return true;
  }

  return false;
}

ClassPtr Class::findSingleTraitWithMethod(const StringData* methName) {
  // Note: m_methodMap includes methods from parents / traits recursively
  ClassPtr traitCls = ClassPtr();
  for (size_t t = 0; t < m_usedTraits.size(); t++) {
    if (m_usedTraits[t]->m_methodMap.find(methName) !=
        m_usedTraits[t]->m_methodMap.end()) {
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
  const StringData* origName = m_preClass->m_unit->lookupLitstrStr(buf);
  m_traitAliases.push_back(std::pair<const StringData*, const StringData*>
                           (newMethName, origName));
}

// returns true on error
bool Class::applyTraitAliasRule(const PreClass::TraitAliasRule& rule,
                                bool failIsFatal) {
  const StringData* traitName    = rule.getTraitName();
  const StringData* origMethName = rule.getOrigMethodName();
  const StringData* newMethName  = rule.getNewMethodName();

  ClassPtr traitCls;
  if (traitName->empty()) {
    traitCls = findSingleTraitWithMethod(origMethName);
  } else {
    traitCls = g_context->loadClass(traitName);
  }

  if (!traitCls.get() || (!(traitCls->m_preClass->m_attrs & AttrTrait))) {
    if (failIsFatal) {
      raise_error("unknown trait '%s'", traitName->data());
    }
    return true;
  }

  // Save info to support ReflectionClass::getTraitAliases
  addTraitAlias(traitName, origMethName, newMethName);

  MethodMap::iterator mm_iter = traitCls->m_methodMap.find(origMethName);
  if (mm_iter == traitCls->m_methodMap.end()) {
    if (failIsFatal) {
      raise_error("unknown trait method '%s'", origMethName->data());
    }
    return true;
  }

  Attr ruleModifiers;
  if (origMethName == newMethName) {
    ruleModifiers = rule.getModifiers();
    setImportTraitMethodModifiers(origMethName, traitCls, ruleModifiers);
  } else {
    ruleModifiers = rule.getModifiers();
    Func *method = traitCls->m_methods[mm_iter->second].func;
    TraitMethod traitMethod(traitCls, method, ruleModifiers);
    addImportTraitMethod(traitMethod, newMethName);
  }
  if (ruleModifiers & AttrStatic) {
    if (failIsFatal) {
      raise_error("cannot use 'static' as access modifier");
    }
    return true;
  }
  return false;
}

// returns true on error
bool Class::applyTraitRules(bool failIsFatal) {
  for (size_t i = 0; i < m_preClass->m_traitPrecRules.size(); i++) {
    if (applyTraitPrecRule(m_preClass->m_traitPrecRules[i], failIsFatal)) {
      return true;
    }
  }
  for (size_t i = 0; i < m_preClass->m_traitAliasRules.size(); i++) {
    if (applyTraitAliasRule(m_preClass->m_traitAliasRules[i], failIsFatal)) {
      return true;
    }
  }
  return false;
}

void Class::addImportTraitMethod(const TraitMethod &traitMethod,
                                 const StringData  *methName) {
  if (strcmp("86ctor", methName->data()) != 0) {
    m_importMethToTraitMap[methName].push_back(traitMethod);
  }
}

bool Class::importTraitMethod(const TraitMethod &traitMethod,
                              const StringData  *methName,
                              bool               failIsFatal) {
  ClassPtr trait     = traitMethod.m_trait;
  Func*    method    = traitMethod.m_method;
  Attr     modifiers = traitMethod.m_modifiers;

  // For abstract methods, simply return if method already declared
  if ((modifiers & AttrAbstract) &&
      m_methodMap.find(methName) != m_methodMap.end()) {
    return false;
  }

  if (modifiers == AttrNone) {
    modifiers = method->m_attrs;
  } else {
    // Keep the AttrReference and AttrStatic bits per original declaration
    modifiers = (Attr)((modifiers       & ~(AttrReference | AttrStatic)) |
                       (method->m_attrs &  (AttrReference | AttrStatic)));
  }

  MethodMap::iterator mm_iter = m_methodMap.find(methName);
  if (mm_iter != m_methodMap.end()) {
    MethodEntry *methEntry = &m_methods[mm_iter->second];
    methEntry->func            = method;
    methEntry->baseClass       = this;
    methEntry->ancestorPrivate = false;
    methEntry->attrs           = modifiers;
  } else {
    m_methodMap[methName] = m_methods.size();
    MethodEntry methEntry = { method, this, false, modifiers};
    m_methods.push_back(methEntry);
  }

  return false;
}

// This handles the case of trait abstract methods that are provided
// by other traits
void Class::removeImplTraitAbstractMethods() {

  for (MethodToTraitListMap::iterator iter = m_importMethToTraitMap.begin();
       iter != m_importMethToTraitMap.end(); iter++) {

    TraitMethodList& tMethList = iter->second;
    // Check if there's any non-abstract method imported
    bool hasNonAbstractMeth = false;
    for (TraitMethodList::const_iterator traitMethIter = tMethList.begin();
         traitMethIter != tMethList.end(); traitMethIter++) {
      if (!(traitMethIter->m_modifiers & AttrAbstract)) {
        hasNonAbstractMeth = true;
        break;
      }
    }
    if (hasNonAbstractMeth) {
      // Erase abstract declarations
      for (TraitMethodList::iterator nextTraitIter = tMethList.begin();
           nextTraitIter != tMethList.end(); ) {
        TraitMethodList::iterator traitIter = nextTraitIter++;
        if (traitIter->m_modifiers & AttrAbstract) {
          tMethList.erase(traitIter);
        }
      }
    }
  }
}

// returns true on error
bool Class::importTraitMethods(bool failIsFatal) {
  // 1. Find all methods to be imported
  for (size_t t = 0; t < m_usedTraits.size(); t++) {
    ClassPtr trait = m_usedTraits[t];
    for (MethodMap::const_iterator iter = trait->m_methodMap.begin();
         iter != trait->m_methodMap.end(); iter++) {
      const StringData* methName = iter->first;
      MethodEntry &method = trait->m_methods[iter->second];
      TraitMethod traitMethod(trait, method.func, method.attrs);
      addImportTraitMethod(traitMethod, methName);
    }
  }

  // 2. Apply trait rules
  if (applyTraitRules(failIsFatal)) return true;

  // 3. Check for trait abstract methods provided by other traits
  removeImplTraitAbstractMethods();

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

      if (failIsFatal) {
        raise_error("method '%s' declared in multiple traits",
                    iter->first->data());
      }
      return true;
    }

    TraitMethodList::const_iterator traitMethIter = iter->second.begin();
    if (importTraitMethod(*traitMethIter, iter->first, failIsFatal)) {
      return true;
    }
  }
  return false;
}

bool Class::setMethods(bool failIsFatal) {
  if (m_parent.get() != NULL) {
    // Copy down the parent's method entries. These may be overridden below.
    m_methodMap = m_parent->m_methodMap;
    std::vector<MethodEntry>::const_iterator it;
    for (it = m_parent->m_methods.begin();
         it != m_parent->m_methods.end(); ++it) {
      if (it->func && (it->attrs & AttrPrivate)) {
        // When copying down an entry for a private method, we set
        // set the ancestorPrivate flag to true.
        MethodEntry e = { it->func, it->baseClass, true, it->func->m_attrs};
        m_methods.push_back(e);
      } else {
        m_methods.push_back(*it);
      }
    }
  }

  if (m_usedTraits.size()) {
    if (importTraitMethods(failIsFatal)) return true;
  }

  ASSERT(AttrPublic < AttrProtected && AttrProtected < AttrPrivate);
  // Overlay/append this class's public/protected methods onto/to those of the
  // parent.
  for (PreClass::MethodVec::const_iterator it = m_preClass->m_methods.begin();
       it != m_preClass->m_methods.end(); ++it) {
    Func* method = *it;
    MethodMap::iterator it2 = m_methodMap.find(method->m_name);
    if (it2 != m_methodMap.end()) {
      MethodEntry* parentEntry = &m_methods[it2->second];
      Func* parentMethod = parentEntry->func;
      bool isTraitMethod = ((parentMethod->m_preClass) &&
                            (parentMethod->m_preClass->m_attrs & AttrTrait));
      // We should never have null func pointers to deal with
      ASSERT(parentMethod);
      if (parentEntry->attrs & AttrFinal) {
        if (failIsFatal) {
          warn_or_error<ErrFinalOverride>("Cannot override final method %s::%s()",
                                          m_parent->m_preClass->m_name->data(),
                                          parentMethod->m_name->data());
        }
        if (ErrFinalOverride) {
          return true;
        }
      }
      if ((method->m_attrs & (AttrPublic|AttrProtected|AttrPrivate))
          > (parentEntry->attrs & (AttrPublic|AttrProtected|AttrPrivate)) &&
          !isTraitMethod) {
        if (failIsFatal) {
          raise_error(
            "Access level to %s::%s() must be %s (as in class %s) or weaker",
            m_preClass->m_name->data(), method->m_name->data(),
            (parentEntry->attrs & AttrPublic) ? "public" :
            ((parentEntry->attrs & AttrProtected) ? "protected" : "private"),
            m_parent->m_preClass->m_name->data());
        }
        return true;
      }
      Class* baseClass;
      if ((method->m_attrs & AttrPrivate) ||
          (parentEntry->attrs & AttrPrivate)) {
        baseClass = this;
      } else {
        // An ancestor class already declared this method
        baseClass = parentEntry->baseClass;
      }
      // Overlay.
      MethodEntry e = { method, baseClass, parentEntry->ancestorPrivate,
                        method->m_attrs };
      m_methods[it2->second] = e;
    } else {
      // This is the first class that declares the method
      Class* baseClass = this;
      // Append.
      m_methodMap[method->m_name] = m_methods.size();
      MethodEntry e = { method, baseClass, false, method->m_attrs };
      m_methods.push_back(e);
    }
  }

  // If class is not abstract, check that all abstract methods have been defined
  if (!(m_preClass->m_attrs & (AttrTrait | AttrInterface | AttrAbstract))) {
    for (size_t i = 0; i < m_methods.size(); i++) {
      if (m_methods[i].attrs & AttrAbstract) {
        if (failIsFatal) {
          raise_error("Class %s contains abstract method (%s) and "
                      "must therefore be declared abstract or implement "
                      "the remaining methods", m_preClass->m_name->data(),
                      m_methods[i].func->m_name->data());
        }
        return true;
      }
    }
  }
  return false;
}

bool Class::setODAttributes(bool failIsFatal) {
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
  return false;
}

bool Class::setConstants(bool failIsFatal) {
  if (m_parent.get() != NULL) {
    for (ConstantVec::const_iterator it = m_parent->m_constantVec.begin();
         it != m_parent->m_constantVec.end(); ++it) {
      // Copy parent's constant.  These may be overlaid below by this class's
      // constants.
      m_constantMap[(*it).m_name] = m_constantVec.size();
      m_constantVec.push_back(*it);
    }
  }

  // Copy in interface constants.
  for (std::vector<ClassPtr>::const_iterator it = m_interfaces.begin();
       it != m_interfaces.end(); ++it) {
    for (ConstantVec::const_iterator cit = (*it)->m_constantVec.begin();
         cit != (*it)->m_constantVec.end(); ++cit) {
      // If you're inheriting a constant with the same name as an existing one,
      // they must originate from the same place
      ConstantMap::iterator existing = m_constantMap.find((*cit).m_name);
      if (existing != m_constantMap.end() &&
          m_constantVec[existing->second].m_class != (*cit).m_class) {
        if (failIsFatal) {
          raise_error("Cannot inherit previously-inherited constant %s",
                      (*cit).m_name->data());
        }
        return true;
      }
      m_constantMap[(*cit).m_name] = m_constantVec.size();
      m_constantVec.push_back(*cit);
    }
  }

  for (PreClass::ConstantVec::const_iterator it =
       m_preClass->m_constantVec.begin();
       it != m_preClass->m_constantVec.end(); ++it) {
    ConstantMap::iterator it2 = m_constantMap.find((*it)->m_name);
    if (it2 != m_constantMap.end()) {
      // Overlay ancestor's constant.
      m_constantVec[it2->second].m_class = this;
      m_constantVec[it2->second].m_val = (*it)->m_val;
    } else {
      // Append constant.
      Const constant;
      constant.m_class = this;
      constant.m_name = (*it)->m_name;
      constant.m_val = (*it)->m_val;
      constant.m_phpCode = (*it)->m_phpCode;
      m_constantMap[(*it)->m_name] = m_constantVec.size();
      m_constantVec.push_back(constant);
    }
  }
  return false;
}

bool Class::setProperties(bool failIsFatal) {
  int numInaccessible = 0;

  if (m_parent.get() != NULL) {
    for (PropInfoVec::const_iterator
         it = m_parent->m_declPropInfo.begin();
         it != m_parent->m_declPropInfo.end(); ++it) {
      // Copy parent's declared property.  Protected properties may be
      // weakened to public below, but otherwise, the parent's properties
      // will stay the same for this class.
      Prop prop;
      if (!((*it).m_attrs & AttrPrivate)) {
        prop.m_name = (*it).m_name;
        m_declPropMap[prop.m_name] = m_declPropInfo.size();
        // parent declares this property
        prop.m_class = (*it).m_class;
      } else {
        // Use a blank name for inaccessible properties.
        static StringData* sd = StringData::GetStaticString("");
        prop.m_name = sd;
        prop.m_class = (*it).m_class;
        ++numInaccessible;
      }
      prop.m_mangledName = (*it).m_mangledName;
      prop.m_attrs = (*it).m_attrs;
      prop.m_docComment = (*it).m_docComment;
      m_declPropInfo.push_back(prop);
    }
    m_declPropInit = m_parent->m_declPropInit;
    for (SPropInfoVec::const_iterator it = m_parent->m_sPropInfo.begin();
         it != m_parent->m_sPropInfo.end(); ++it) {
      if (!((*it).m_attrs & AttrPrivate)) {
        // Alias parent's static property.
        SProp sProp;
        sProp.m_name = (*it).m_name;
        sProp.m_attrs = (*it).m_attrs;
        sProp.m_docComment = (*it).m_docComment;
        sProp.m_class = (*it).m_class;
        TV_WRITE_UNINIT(&sProp.m_val);
        m_sPropMap[sProp.m_name] = m_sPropInfo.size();
        m_sPropInfo.push_back(sProp);
      }
    }
  }

  ASSERT(AttrPublic < AttrProtected && AttrProtected < AttrPrivate);
  for (PreClass::PropertyVec::const_iterator
       it = m_preClass->m_propertyVec.begin();
       it != m_preClass->m_propertyVec.end(); ++it) {
    PreClass::Prop* preProp = *it;

    if (!(preProp->m_attrs & AttrStatic)) {
      // Overlay/append this class's protected and public properties onto/to
      // those of the parent, and append this class's private properties.
      // Append order doesn't matter here (unlike in setMethods()).

      // Prohibit static-->non-static redeclaration.
      int sPropInd = lookupSProp(preProp->m_name);
      if (sPropInd != -1) {
        if (failIsFatal) {
          raise_error("Cannot redeclare static %s::%s as non-static %s::%s",
                      m_sPropInfo[sPropInd].m_class->m_preClass->m_name->data(),
                      preProp->m_name->data(), m_preClass->m_name->data(),
                      preProp->m_name->data());
        }
        return true;
      }
      // Get parent's equivalent property, if one exists.
      Prop* parentProp = NULL;
      if (m_parent.get() != NULL) {
        PropMap::const_iterator it2 =
          m_parent->m_declPropMap.find(preProp->m_name);
        if (it2 != m_parent->m_declPropMap.end()) {
          parentProp = &m_parent->m_declPropInfo.at(it2->second);
        }
      }
      // Prohibit strengthening.
      if (parentProp
          && (preProp->m_attrs & (AttrPublic|AttrProtected|AttrPrivate))
             > (parentProp->m_attrs & (AttrPublic|AttrProtected|AttrPrivate))) {
        if (failIsFatal) {
          raise_error(
            "Access level to %s::%s() must be %s (as in class %s) or weaker",
            m_preClass->m_name->data(), preProp->m_name->data(),
            (parentProp->m_attrs & AttrPublic) ? "public" :
            ((parentProp->m_attrs & AttrProtected) ? "protected" : "private"),
            m_parent->m_preClass->m_name->data());
        }
        return true;
      }
      switch (preProp->m_attrs & (AttrPublic|AttrProtected|AttrPrivate)) {
      case AttrPrivate: {
        // Append a new private property.
        Prop prop;
        prop.m_name = preProp->m_name;
        prop.m_mangledName = preProp->m_mangledName;
        prop.m_attrs = preProp->m_attrs;
        // This is the first class to declare this property
        prop.m_class = this;
        prop.m_docComment = preProp->m_docComment;
        m_declPropInfo.push_back(prop);
        m_declPropInit.push_back(m_preClass->m_propertyMap[preProp->m_name]
                                 ->m_val);
        m_declPropMap[preProp->m_name] = m_declPropInfo.size() - 1;
        break;
      }
      case AttrProtected: {
        // Check whether a superclass has already declared this protected
        // property.
        PropMap::const_iterator it2 = m_declPropMap.find(preProp->m_name);
        if (it2 != m_declPropMap.end()) {
          ASSERT((m_declPropInfo.at(it2->second).m_attrs
                 & (AttrPublic|AttrProtected|AttrPrivate)) == AttrProtected);
          m_declPropInit[it2->second] = m_preClass->m_propertyMap[preProp
                                        ->m_name]->m_val;
          break;
        }
        // Append a new protected property.
        Prop prop;
        prop.m_name = preProp->m_name;
        prop.m_mangledName = preProp->m_mangledName;
        prop.m_attrs = preProp->m_attrs;
        // This is the first class to declare this property
        prop.m_class = this;
        prop.m_docComment = preProp->m_docComment;
        m_declPropInfo.push_back(prop);
        m_declPropInit.push_back(m_preClass->m_propertyMap[preProp->m_name]
                                 ->m_val);
        m_declPropMap[preProp->m_name] = m_declPropInfo.size() - 1;
        break;
      }
      case AttrPublic: {
        // Check whether a superclass has already declared this as a
        // protected/public property.
        PropMap::const_iterator it2 = m_declPropMap.find(preProp->m_name);
        if (it2 != m_declPropMap.end()) {
          Prop& prop = m_declPropInfo.at(it2->second);
          if ((prop.m_attrs & (AttrPublic|AttrProtected|AttrPrivate))
              == AttrProtected) {
            // Weaken protected property to public.
            prop.m_mangledName = preProp->m_mangledName;
            prop.m_attrs = Attr(prop.m_attrs ^ (AttrProtected|AttrPublic));
          }
          m_declPropInit[it2->second] = m_preClass->m_propertyMap[preProp
                                        ->m_name]->m_val;
          break;
        }
        // Append a new public property.
        Prop prop;
        prop.m_name = preProp->m_name;
        prop.m_mangledName = preProp->m_mangledName;
        prop.m_attrs = preProp->m_attrs;
        // This is the first class to declare this property
        prop.m_class = this;
        prop.m_docComment = preProp->m_docComment;
        m_declPropInfo.push_back(prop);
        m_declPropInit.push_back(m_preClass->m_propertyMap[preProp->m_name]
                                 ->m_val);
        m_declPropMap[preProp->m_name] = m_declPropInfo.size() - 1;
        break;
      }
      default: ASSERT(false);
      }
    } else { // Static property.
      // Prohibit non-static-->static redeclaration.
      int declPropInd = lookupDeclProp(preProp->m_name);
      if (declPropInd != -1) {
        if (failIsFatal) {
          // Find class that declared non-static property.
          Class* ancestor;
          for (ancestor = m_parent.get();
               ancestor->m_preClass->m_propertyMap.find(preProp->m_name)
               == ancestor->m_preClass->m_propertyMap.end();
               ancestor = ancestor->m_parent.get()) {
          }
          raise_error("Cannot redeclare non-static %s::%s as static %s::%s",
                      ancestor->m_preClass->m_name->data(),
                      preProp->m_name->data(),
                      m_preClass->m_name->data(),
                      preProp->m_name->data());
        }
        return true;
      }
      // Get parent's equivalent property, if one exists.
      int sPropInd = lookupSProp(preProp->m_name);
      // Prohibit strengthening.
      if (sPropInd != -1) {
        SProp& parentSProp = m_sPropInfo[sPropInd];
        if ((preProp->m_attrs & (AttrPublic|AttrProtected|AttrPrivate))
            > (parentSProp.m_attrs & (AttrPublic|AttrProtected|AttrPrivate))) {
          if (failIsFatal) {
            raise_error(
              "Access level to %s::%s() must be %s (as in class %s) or weaker",
              m_preClass->m_name->data(), preProp->m_name->data(),
              (parentSProp.m_attrs & AttrPublic) ? "public" :
              ((parentSProp.m_attrs & AttrProtected) ? "protected" : "private"),
              m_parent->m_preClass->m_name->data());
          }
          return true;
        }
      }
      // Create a new property, or overlay ancestor's property if one exists.
      if (sPropInd == -1) {
        SProp sProp;
        sProp.m_name = preProp->m_name;

        sPropInd = m_sPropInfo.size();
        m_sPropMap[sProp.m_name] = sPropInd;
        m_sPropInfo.push_back(sProp);
      }
      SProp& sProp = m_sPropInfo[sPropInd];
      // Finish initializing.
      sProp.m_attrs = preProp->m_attrs;
      sProp.m_docComment = preProp->m_docComment;
      sProp.m_class = this;
      sProp.m_val = m_preClass->m_propertyMap[preProp->m_name]->m_val;
    }
  }

  if (importTraitProps(failIsFatal)) return true;

  m_declPropNumAccessible = m_declPropInfo.size() - numInaccessible;
  return false;
}

bool Class::compatibleTraitPropInit(TypedValue& tv1, TypedValue& tv2) {
  if (tv1.m_type != tv2.m_type) return false;
  switch (tv1.m_type) {
    case KindOfNull: return true;
    case KindOfBoolean:
    case KindOfInt32:
    case KindOfInt64:
    case KindOfDouble:
    case KindOfStaticString:
    case KindOfString:
      return same(tvAsVariant(&tv1), tvAsVariant(&tv2));
    default: return false;
  }
}

// returns true on failure, false on success
bool Class::importTraitInstanceProp(ClassPtr    trait,
                                    Prop&       traitProp,
                                    TypedValue& traitPropVal,
                                    bool        failIsFatal) {
  PropMap::const_iterator prevIt = m_declPropMap.find(traitProp.m_name);

  if (prevIt == m_declPropMap.end()) {
    // New prop, go ahead and add it
    Prop prop = traitProp;
    prop.m_class = this; // set current class as the first declaring prop
    m_declPropInfo.push_back(prop);
    m_declPropInit.push_back(traitPropVal);
    m_declPropMap[prop.m_name] = m_declPropInfo.size() - 1;
  } else {
    // Redeclared prop, make sure it matches previous declarations
    Prop&       prevProp    = m_declPropInfo[prevIt->second];
    TypedValue& prevPropVal = m_declPropInit[prevIt->second];
    if (prevProp.m_attrs != traitProp.m_attrs ||
        !compatibleTraitPropInit(prevPropVal, traitPropVal)) {
      if (failIsFatal) {
        raise_error("trait declaration of property '%s' is incompatible with "
                    "previous declaration", traitProp.m_name->data());
      }
      return true;
    }
  }
  return false;
}

// returns true on failure, false on success
bool Class::importTraitStaticProp(ClassPtr trait,
                                  SProp&   traitProp,
                                  bool     failIsFatal) {
  // Check if prop already declared as non-static
  if (m_declPropMap.find(traitProp.m_name) != m_declPropMap.end()) {
    if (failIsFatal) {
      raise_error("trait declaration of property '%s' is incompatible with "
                  "previous declaration", traitProp.m_name->data());
    }
    return true;
  }

  SPropMap::const_iterator prevIt = m_sPropMap.find(traitProp.m_name);
  if (prevIt == m_sPropMap.end()) {
    // New prop, go ahead and add it
    SProp prop = traitProp;
    prop.m_class = this; // set current class as the first declaring prop
    m_sPropInfo.push_back(prop);
    m_sPropMap[prop.m_name] = m_sPropInfo.size() - 1;
  } else {
    // Redeclared prop, make sure it matches previous declaration
    SProp&     prevProp    = m_sPropInfo[prevIt->second];
    TypedValue prevPropVal = getStaticPropInitVal(prevProp);
    if (prevProp.m_attrs != traitProp.m_attrs ||
        !compatibleTraitPropInit(traitProp.m_val, prevPropVal)) {
      if (failIsFatal) {
        raise_error("trait declaration of property '%s' is incompatible with "
                    "previous declaration", traitProp.m_name->data());
      }
      return true;
    }
    prevProp.m_class = this;
    prevProp.m_val   = prevPropVal;
  }
  return false;
}

// returns true in case of error, false on success
bool Class::importTraitProps(bool failIsFatal) {
  for (size_t t = 0; t < m_usedTraits.size(); t++) {
    ClassPtr trait = m_usedTraits[t];

    // instance properties
    for (size_t p = 0; p < trait->m_declPropInfo.size(); p++) {
      Prop&       traitProp    = trait->m_declPropInfo[p];
      TypedValue& traitPropVal = trait->m_declPropInit[p];
      if (importTraitInstanceProp(trait, traitProp, traitPropVal,
                                  failIsFatal)) {
        return true;
      }
    }

    // static properties
    for (size_t p = 0; p < trait->m_sPropInfo.size(); p++) {
      SProp& traitProp = trait->m_sPropInfo[p];
      if (importTraitStaticProp(trait, traitProp, failIsFatal)) {
        return true;
      }
    }
  }
  return false;
}

bool Class::setInitializers(bool failIsFatal) {
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
  static StringData* sd86pinit = StringData::GetStaticString("86pinit");
  const Func* meth86pinit = lookupMethod(sd86pinit);
  if (meth86pinit != NULL) {
    m_pinitVec.push_back(meth86pinit);
  }
  static StringData* sd86sinit = StringData::GetStaticString("86sinit");
  m_sinit = lookupMethod(sd86sinit);

  m_needInitialization = (m_pinitVec.size() > 0 || m_sPropInfo.size() > 0);
  return false;
}

bool Class::setInterfaces(bool failIsFatal) {
  for (std::vector<const StringData*>::const_iterator it =
         m_preClass->m_interfaces.begin();
       it != m_preClass->m_interfaces.end(); ++it) {
    ClassPtr cp = g_context->loadClass(*it);
    if (cp.get() == NULL) {
      if (failIsFatal) {
        raise_error("Undefined interface: %s", (*it)->data());
      }
      return true;
    }
    if (!(cp->m_preClass->m_attrs & AttrInterface)) {
      if (failIsFatal) {
        raise_error("%s cannot implement %s - it is not an interface",
                    m_preClass->m_name->data(), cp->m_preClass->m_name->data());
      }
      return true;
    }
    m_interfaces.push_back(cp);

    // For non-abstract classes, check that all interface methods have
    // been implemented
    if (!(m_preClass->m_attrs & (AttrTrait | AttrInterface | AttrAbstract))) {
      for (size_t m = 0; m < cp->m_methods.size(); m++) {
        const StringData* methName = cp->m_methods[m].func->m_name;
        // Skip special methods
        if (strncmp("86", methName->data(), 2) == 0) continue;
        MethodMap::const_iterator iter = m_methodMap.find(methName);
        if (iter == m_methodMap.end() ||
            (m_methods[iter->second].attrs & AttrAbstract)) {
          if (failIsFatal) {
            raise_error("Class %s contains abstract method (%s) and "
                          "must therefore be declared abstract or implement "
                        "the remaining methods", m_preClass->m_name->data(),
                        methName->data());
          }
          return true;
        }
      }
    }
  }
  return false;
}

bool Class::setUsedTraits(bool failIsFatal) {
  for (std::vector<const StringData*>::const_iterator
         it = m_preClass->m_usedTraits.begin();
       it != m_preClass->m_usedTraits.end(); it++) {
    ClassPtr classPtr = g_context->loadClass(*it);
    if (classPtr.get() == NULL) {
      if (failIsFatal) {
        raise_error("Trait '%s' not found", (*it)->data());
      }
      return true;
    }
    if (!(classPtr->m_preClass->m_attrs & AttrTrait)) {
      if (failIsFatal) {
        raise_error("%s cannot use %s - it is not a trait",
                    m_preClass->m_name->data(),
                    classPtr->m_preClass->m_name->data());
      }
      return true;
    }
    m_usedTraits.push_back(classPtr);
  }
  return false;
}

bool Class::setClassVec(bool failIsFatal) {
  if (m_classVecLen > 1) {
    ASSERT(m_parent.get() != NULL);
    memcpy(m_classVec, m_parent.get()->m_classVec,
           (m_classVecLen-1) * sizeof(Class*));
  }
  m_classVec[m_classVecLen-1] = this;
  return false;
}

void Class::getClassInfo(ClassInfoVM* ci) {
  ASSERT(ci);

  // Miscellaneous.
  Attr clsAttrs = m_preClass->m_attrs;
  int attr = 0;
  if (clsAttrs & AttrInterface) attr |= ClassInfo::IsInterface;
  if (clsAttrs & AttrAbstract)  attr |= ClassInfo::IsAbstract;
  if (clsAttrs & AttrFinal)     attr |= ClassInfo::IsFinal;
  if (clsAttrs & AttrTrait)     attr |= ClassInfo::IsTrait;
  if (attr == 0)                attr  = ClassInfo::IsNothing;
  ci->m_attribute = (ClassInfo::Attribute)attr;

  ci->m_name = m_preClass->m_name->data();

  ci->m_file = m_preClass->m_unit->m_filepath->data();
  ci->m_line1 = m_preClass->m_line1;
  ci->m_line2 = m_preClass->m_line2;
  ci->m_docComment = (m_preClass->m_docComment != NULL)
                     ? m_preClass->m_docComment->data() : "";

  // Parent class.
  if (m_parent.get()) {
    ci->m_parentClass = m_parent->m_preClass->m_name->data();
  } else {
    ci->m_parentClass = "";
  }

  // Interfaces.
  for (unsigned i = 0; i < m_interfaces.size(); i++) {
    ci->m_interfacesVec.push_back(
        m_interfaces[i]->m_preClass->m_name->data());
    ci->m_interfaces.insert(
        m_interfaces[i]->m_preClass->m_name->data());
  }

  // Used traits.
  for (unsigned t = 0; t < m_usedTraits.size(); t++) {
    const char* traitName = m_usedTraits[t]->m_preClass->m_name->data();
    ci->m_traitsVec.push_back(traitName);
    ci->m_traits.insert(traitName);
  }

  // Trait aliases.
  for (unsigned a = 0; a < m_traitAliases.size(); a++) {
    ci->m_traitAliasesVec.push_back(std::pair<String, String>
                                    (m_traitAliases[a].first->data(),
                                     m_traitAliases[a].second->data()));
  }

  // Methods.
  for (unsigned i = 0; i < m_methods.size(); ++i) {
    Func* func = m_methods[i].func;
    // Filter out 86ctor() and 86*init().
    ASSERT(func);
    if (func && findMethodBaseClass(func->m_name) == this &&
        !isdigit(func->m_name->data()[0])) {
      ClassInfo::MethodInfo *m = new ClassInfo::MethodInfo;
      func->getFuncInfo(m);
      ci->m_methods[func->m_name->data()] = m;
      ci->m_methodsVec.push_back(m);
    }
  }

  // Properties.
  for (unsigned i = 0; i < m_declPropInfo.size(); ++i) {
    if (m_declPropInfo[i].m_class != this) continue;
    ClassInfo::PropertyInfo *pi = new ClassInfo::PropertyInfo;
    pi->owner = ci;
    pi->name = m_declPropInfo[i].m_name->data();
    Attr propAttrs = m_declPropInfo[i].m_attrs;
    attr = 0;
    if (propAttrs & AttrProtected) attr |= ClassInfo::IsProtected;
    if (propAttrs & AttrPrivate) attr |= ClassInfo::IsPrivate;
    if (attr == 0) attr |= ClassInfo::IsPublic;
    if (propAttrs & AttrStatic) attr |= ClassInfo::IsStatic;
    pi->attribute = (ClassInfo::Attribute)attr;
    pi->docComment = (m_declPropInfo[i].m_docComment != NULL)
                     ? m_declPropInfo[i].m_docComment->data() : "";

    ci->m_properties[pi->name] = pi;
    ci->m_propertiesVec.push_back(pi);
  }

  for (unsigned i = 0; i < m_sPropInfo.size(); ++i) {
    if (m_sPropInfo[i].m_class != this) continue;
    ClassInfo::PropertyInfo *pi = new ClassInfo::PropertyInfo;
    pi->owner = ci;
    pi->name = m_sPropInfo[i].m_name->data();
    Attr propAttrs = m_sPropInfo[i].m_attrs;
    attr = 0;
    if (propAttrs & AttrProtected) attr |= ClassInfo::IsProtected;
    if (propAttrs & AttrPrivate) attr |= ClassInfo::IsPrivate;
    if (attr == 0) attr |= ClassInfo::IsPublic;
    if (propAttrs & AttrStatic) attr |= ClassInfo::IsStatic;
    pi->attribute = (ClassInfo::Attribute)attr;
    pi->docComment = (m_sPropInfo[i].m_docComment != NULL)
                     ? m_sPropInfo[i].m_docComment->data() : "";

    ci->m_properties[pi->name] = pi;
    ci->m_propertiesVec.push_back(pi);
  }

  // Constants.
  for (unsigned i = 0; i < m_constantVec.size(); ++i) {
    ClassInfo::ConstantInfo *ki = new ClassInfo::ConstantInfo;
    ki->name = m_constantVec[i].m_name->data();
    ki->valueLen = m_constantVec[i].m_phpCode->size();
    ki->valueText = m_constantVec[i].m_phpCode->data();
    ki->setValue(tvAsCVarRef(clsCnsGet(m_constantVec[i].m_name)));

    ci->m_constants[ki->name] = ki;
    ci->m_constantsVec.push_back(ki);
  }
}

} } // HPHP::VM
