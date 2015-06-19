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

#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/strings.h"

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

inline const LowPtr<Class>* Class::classVec() const {
  return m_classVec;
}

inline Class::veclen_t Class::classVecLen() const {
  return m_classVecLen;
}

///////////////////////////////////////////////////////////////////////////////
// Ancestry.

inline bool Class::classofNonIFace(const Class* cls) const {
  assert(!(cls->attrs() & AttrInterface));
  if (m_classVecLen >= cls->m_classVecLen) {
    return (m_classVec[cls->m_classVecLen-1] == cls);
  }
  return false;
}

inline bool Class::classof(const Class* cls) const {
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
  return classofNonIFace(cls);
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
  return m_extra->m_clsInfo;
}

inline BuiltinCtorFunction Class::instanceCtor() const {
  return m_extra->m_instanceCtor;
}

inline BuiltinDtorFunction Class::instanceDtor() const {
  return m_extra->m_instanceDtor;
}

inline int32_t Class::builtinODTailSize() const {
  return m_extra->m_builtinODTailSize;
}

///////////////////////////////////////////////////////////////////////////////
// Methods.

inline size_t Class::numMethods() const {
  return m_methods.size();
}

inline Func* Class::getMethod(Slot idx) const {
  auto funcVec = (LowPtr<Func>*)this;
  return funcVec[-((int32_t)idx + 1)];
}

inline void Class::setMethod(Slot idx, Func* func) {
  auto funcVec = (LowPtr<Func>*)this;
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

inline const FixedVector<const Func*>& Class::pinitVec() const {
  return m_pinitVec;
}

///////////////////////////////////////////////////////////////////////////////
// Property storage.

inline void Class::initPropHandle() const {
  m_propDataCache.bind();
}

inline rds::Handle Class::propHandle() const {
  return m_propDataCache.handle();
}

inline rds::Handle Class::sPropInitHandle() const {
  return m_sPropCacheInit.handle();
}

inline rds::Handle Class::sPropHandle(Slot index) const {
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
  // m_constants.contains(clsCnsName) returns abstract constants
  auto clsCnsInd = m_constants.findIndex(clsCnsName);
  return (clsCnsInd != kInvalidSlot) &&
    !m_constants[clsCnsInd].isAbstract() &&
    !m_constants[clsCnsInd].isType();
}

inline bool Class::hasTypeConstant(const StringData* typeConstName) const {
  auto typeConstInd = m_constants.findIndex(typeConstName);
  return (typeConstInd != kInvalidSlot) &&
    !m_constants[typeConstInd].isAbstract() &&
    m_constants[typeConstInd].isType();
}

///////////////////////////////////////////////////////////////////////////////
// Interfaces and traits.

inline folly::Range<const ClassPtr*> Class::declInterfaces() const {
  return folly::range(
    m_declInterfaces.get(),
    m_declInterfaces.get() + m_numDeclInterfaces
  );
}

inline const Class::InterfaceMap& Class::allInterfaces() const {
  return m_interfaces;
}

inline Slot Class::traitsBeginIdx() const {
  return m_extra->m_traitsBeginIdx;
}

inline Slot Class::traitsEndIdx() const   {
  return m_extra->m_traitsEndIdx;
}

inline const std::vector<ClassPtr>& Class::usedTraitClasses() const {
  return m_extra->m_usedTraits;
}

inline const Class::TraitAliasVec& Class::traitAliases() const {
  return m_extra->m_traitAliases;
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
// JIT data.

inline rds::Handle Class::classHandle() const {
  return m_cachedClass.handle();
}

inline void Class::setClassHandle(rds::Link<Class*> link) const {
  assert(!m_cachedClass.bound());
  m_cachedClass = link;
}

inline Class* Class::getCached() const {
  return *m_cachedClass;
}

inline void Class::setCached() {
  *m_cachedClass = this;
}

///////////////////////////////////////////////////////////////////////////////
// Native data.

inline const Native::NativeDataInfo* Class::getNativeDataInfo() const {
  return m_extra->m_nativeDataInfo;
}

///////////////////////////////////////////////////////////////////////////////
// Closure subclasses.

inline bool Class::isScopedClosure() const {
  return m_scoped;
}

inline const Class::ScopedClonesMap& Class::scopedClones() const {
  return m_extra->m_scopedClones;
}

///////////////////////////////////////////////////////////////////////////////
// Other methods.

inline MaybeDataType Class::enumBaseTy() const {
  return m_enumBaseTy;
}

///////////////////////////////////////////////////////////////////////////////
// ExtraData.

inline void Class::allocExtraData() {
  if (!m_extra) {
    m_extra = new ExtraData();
  }
}

///////////////////////////////////////////////////////////////////////////////
// Trait method import.

inline bool Class::TMIOps::strEmpty(const StringData* str) {
  return str->empty();
}

inline const StringData* Class::TMIOps::clsName(const Class* traitCls) {
  return traitCls->name();
}

inline bool Class::TMIOps::isTrait(const Class* traitCls) {
  return traitCls->attrs() & AttrTrait;
}

inline bool Class::TMIOps::isAbstract(Attr modifiers) {
  return modifiers & AttrAbstract;
}

inline Class::TraitMethod
Class::TMIOps::traitMethod(const Class* traitCls,
                           const Func* traitMeth,
                           Class::TMIOps::alias_type rule) {
  return TraitMethod { traitCls, traitMeth, rule.modifiers() };
}

inline const StringData*
Class::TMIOps::precMethodName(Class::TMIOps::prec_type rule) {
  return rule.methodName();
}

inline const StringData*
Class::TMIOps::precSelectedTraitName(Class::TMIOps::prec_type rule) {
  return rule.selectedTraitName();
}

inline TraitNameSet
Class::TMIOps::precOtherTraitNames(Class::TMIOps::prec_type rule) {
  return rule.otherTraitNames();
}

inline const StringData*
Class::TMIOps::aliasTraitName(Class::TMIOps::alias_type rule) {
  return rule.traitName();
}

inline const StringData*
Class::TMIOps::aliasOrigMethodName(Class::TMIOps::alias_type rule) {
  return rule.origMethodName();
}

inline const StringData*
Class::TMIOps::aliasNewMethodName(Class::TMIOps::alias_type rule) {
  return rule.newMethodName();
}

inline Attr
Class::TMIOps::aliasModifiers(Class::TMIOps::alias_type rule) {
  return rule.modifiers();
}

inline const Func*
Class::TMIOps::findTraitMethod(const Class* cls,
                               const Class* traitCls,
                               const StringData* origMethName) {
  return traitCls->lookupMethod(origMethName);
}

inline void
Class::TMIOps::errorUnknownMethod(Class::TMIOps::prec_type rule) {
  raise_error("unknown method '%s'", rule.methodName()->data());
}

inline void
Class::TMIOps::errorUnknownMethod(Class::TMIOps::alias_type rule,
                                  const StringData* methName) {
  raise_error(Strings::TRAITS_UNKNOWN_TRAIT_METHOD, methName->data());
}

template <class Rule>
inline void
Class::TMIOps::errorUnknownTrait(const Rule& rule,
                                 const StringData* traitName) {
  raise_error(Strings::TRAITS_UNKNOWN_TRAIT, traitName->data());
}

inline void
Class::TMIOps::errorDuplicateMethod(const Class* cls,
                                    const StringData* methName) {
  // No error if the class will override the method.
  if (cls->preClass()->hasMethod(methName)) return;
  raise_error(Strings::METHOD_IN_MULTIPLE_TRAITS, methName->data());
}

///////////////////////////////////////////////////////////////////////////////
// Non-member functions.

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

inline bool isNormalClass(const Class* cls) {
  return !(cls->attrs() & (AttrTrait | AttrInterface | AttrEnum));
}

inline bool isAbstract(const Class* cls) {
  return cls->attrs() & AttrAbstract;
}

inline bool classHasPersistentRDS(const Class* cls) {
  return cls && rds::isPersistentHandle(cls->classHandle());
}

///////////////////////////////////////////////////////////////////////////////
}
