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

#ifndef incl_HPHP_VM_CLASS_INL_H_
#error "class-inl.h should only be included by class.h"
#endif

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

inline bool Class::isZombie() const {
  return !m_cachedClass.bound();
}

///////////////////////////////////////////////////////////////////////////////
// Class::PropInitVec.

inline Class::PropInitVec::iterator Class::PropInitVec::begin() {
  return m_data;
}

inline Class::PropInitVec::iterator Class::PropInitVec::end() {
  return m_data + m_size;
}

inline size_t Class::PropInitVec::size() const {
  return m_size;
}

inline TypedValueAux& Class::PropInitVec::operator[](size_t i) {
  assert(i < m_size);
  return m_data[i];
}

inline const TypedValueAux& Class::PropInitVec::operator[](size_t i) const {
  assert(i < m_size);
  return m_data[i];
}

///////////////////////////////////////////////////////////////////////////////
// Pre- and post-allocations.

inline const LowClassPtr* Class::classVec() const {
  return m_classVec;
}

inline unsigned Class::classVecLen() const {
  return m_classVecLen;
}

///////////////////////////////////////////////////////////////////////////////
// Ancestry.

inline uint64_t Class::classof(const Class* cls) const {
  // If `cls' is an interface, we can simply check to see if cls is in
  // this->m_interfaces.  Otherwise, if `this' is not an interface, the
  // classVec check will determine whether it's an instance of cls (including
  // the case where this and cls are the same trait).  Otherwise, `this' is an
  // interface, and `cls' is not, so we need to return false.  But the classVec
  // check can never return true in that case (cls's classVec contains only
  // non-interfaces, while this->classVec is either empty, or contains
  // interfaces).
  if (UNLIKELY(cls->attrs() & AttrInterface)) {
    return this == cls ||
      m_interfaces.lookupDefault(cls->m_preClass->name(), nullptr) == cls;
  }
  if (m_classVecLen >= cls->m_classVecLen) {
    return (m_classVec[cls->m_classVecLen-1] == cls);
  }
  return false;
}

inline bool Class::ifaceofDirect(const StringData* name) const {
  return m_interfaces.contains(name);
}

///////////////////////////////////////////////////////////////////////////////
// Basic info.

inline const StringData* Class::name() const {
  return m_preClass->name();
}

inline const PreClass* Class::preClass() const {
  return m_preClass.get();
}

inline Class* Class::parent() const {
  return m_parent.get();
}

inline StrNR Class::nameStr() const {
  return m_preClass->nameStr();
}

inline StrNR Class::parentStr() const {
  return m_preClass->parentStr();
}

inline Attr Class::attrs() const {
  assert(Attr(m_attrCopy) == m_preClass->attrs());
  return Attr(m_attrCopy);
}

inline int Class::getODAttrs() const {
  return m_ODAttrs;
}

inline bool Class::isPersistent() const {
  return attrs() & AttrPersistent;
}

///////////////////////////////////////////////////////////////////////////////
// Magic methods.

inline const Func* Class::getCtor() const {
  return m_ctor;
}

inline const Func* Class::getDtor() const {
  return m_dtor;
}

inline const Func* Class::getToString() const {
  return m_toString;
}

///////////////////////////////////////////////////////////////////////////////
// Builtin classes.

inline bool Class::isBuiltin() const {
  return attrs() & AttrBuiltin;
}

inline const ClassInfo* Class::clsInfo() const {
  return m_clsInfo;
}

inline BuiltinCtorFunction Class::instanceCtor() const {
  return m_instanceCtor;
}

inline BuiltinDtorFunction Class::instanceDtor() const {
  return m_instanceDtor;
}

inline int32_t Class::builtinODTailSize() const {
  return m_builtinODTailSize;
}

///////////////////////////////////////////////////////////////////////////////
// Methods.

inline size_t Class::numMethods() const {
  return m_methods.size();
}

inline Func* Class::getMethod(Slot idx) const {
  Func** funcVec = (Func**)this;
  return funcVec[-((int32_t)idx + 1)];
}

inline void Class::setMethod(Slot idx, Func* func) {
  Func** funcVec = (Func**)this;
  funcVec[-((int32_t)idx + 1)] = func;
}

inline Func* Class::lookupMethod(const StringData* methName) const {
  Slot* idx = m_methods.find(methName);
  if (!idx) return nullptr;
  return getMethod(*idx);
}

///////////////////////////////////////////////////////////////////////////////
// Property metadata.

inline size_t Class::numDeclProperties() const {
  return m_declProperties.size();
}

inline size_t Class::numStaticProperties() const {
  return m_staticProperties.size();
}

inline uint32_t Class::declPropNumAccessible() const {
  return m_declPropNumAccessible;
}

inline const Class::Prop* Class::declProperties() const {
  return m_declProperties.accessList();
}

inline const Class::SProp* Class::staticProperties() const {
  return m_staticProperties.accessList();
}

inline Slot Class::lookupDeclProp(const StringData* propName) const {
  return m_declProperties.findIndex(propName);
}

inline Slot Class::lookupSProp(const StringData* sPropName) const {
  return m_staticProperties.findIndex(sPropName);
}

inline RepoAuthType Class::declPropRepoAuthType(Slot index) const {
  return m_declProperties[index].m_repoAuthType;
}

inline RepoAuthType Class::staticPropRepoAuthType(Slot index) const {
  return m_staticProperties[index].m_repoAuthType;
}

inline bool Class::hasDeepInitProps() const {
  return m_hasDeepInitProps;
}

///////////////////////////////////////////////////////////////////////////////
// Property initialization.

inline bool Class::needInitialization() const {
  return m_needInitialization;
}

inline const Class::PropInitVec& Class::declPropInit() const {
  return m_declPropInit;
}

inline const std::vector<const Func*>& Class::pinitVec() const {
  return m_pinitVec;
}

///////////////////////////////////////////////////////////////////////////////
// Property storage.

inline void Class::initPropHandle() const {
  m_propDataCache.bind();
}

inline RDS::Handle Class::propHandle() const {
  return m_propDataCache.handle();
}

inline RDS::Handle Class::sPropInitHandle() const {
  return m_sPropCacheInit.handle();
}

inline RDS::Handle Class::sPropHandle(Slot index) const {
  assert(m_sPropCacheInit.bound());
  assert(numStaticProperties() > index);
  return m_sPropCache[index].handle();
}

///////////////////////////////////////////////////////////////////////////////
// Constants.

inline size_t Class::numConstants() const {
  return m_constants.size();
}

inline const Class::Const* Class::constants() const {
  return m_constants.accessList();
}

inline bool Class::hasConstant(const StringData* clsCnsName) const {
  return m_constants.contains(clsCnsName);
}

///////////////////////////////////////////////////////////////////////////////
// Interfaces and traits.

inline boost::iterator_range<const ClassPtr*> Class::declInterfaces() const {
  return boost::make_iterator_range(
    m_declInterfaces.get(),
    m_declInterfaces.get() + m_numDeclInterfaces
  );
}

inline const Class::InterfaceMap& Class::allInterfaces() const {
  return m_interfaces;
}

inline Slot Class::traitsBeginIdx() const {
  return m_traitsBeginIdx;
}

inline Slot Class::traitsEndIdx() const   {
  return m_traitsEndIdx;
}

inline const std::vector<ClassPtr>& Class::usedTraitClasses() const {
  return m_usedTraits;
}

inline const Class::RequirementMap& Class::allRequirements() const {
  return m_requirements;
}

///////////////////////////////////////////////////////////////////////////////
// Objects.

inline bool Class::callsCustomInstanceInit() const {
  return m_callsCustomInstanceInit;
}

///////////////////////////////////////////////////////////////////////////////
// Other methods.

inline RDS::Handle Class::classHandle() const {
  return m_cachedClass.handle();
}

inline void Class::setClassHandle(RDS::Link<Class*> link) const {
  assert(!m_cachedClass.bound());
  m_cachedClass = link;
}

inline Class* Class::getCached() const {
  return *m_cachedClass;
}

inline void Class::setCached() {
  *m_cachedClass = this;
}

inline const Native::NativeDataInfo* Class::getNativeDataInfo() const {
  return m_nativeDataInfo;
}

inline DataType Class::enumBaseTy() const {
  return m_enumBaseTy;
}

///////////////////////////////////////////////////////////////////////////////

inline Attr classKindAsAttr(ClassKind kind) {
  return static_cast<Attr>(kind);
}

inline bool isTrait(const Class* cls) {
  return cls->attrs() & AttrTrait;
}

inline bool isEnum(const Class* cls) {
  return cls->attrs() & AttrEnum;
}

inline bool isInterface(const Class* cls) {
  return cls->attrs() & AttrInterface;
}

inline bool isNormalClass(const Class* cls ) {
  return !(cls->attrs() & (AttrTrait | AttrInterface | AttrEnum));
}

inline bool isAbstract(const Class* cls) {
  return cls->attrs() & AttrAbstract;
}

inline bool classHasPersistentRDS(const Class* cls) {
  return cls && RDS::isPersistentHandle(cls->classHandle());
}

///////////////////////////////////////////////////////////////////////////////
}
