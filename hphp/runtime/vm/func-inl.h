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

#ifndef incl_HPHP_VM_FUNC_INL_H_
#error "func-inl.h should only be included by func.h"
#endif

#include "hphp/runtime/vm/unit-util.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// EH table.

template<class SerDe>
void EHEnt::serde(SerDe& sd) {
  folly::Optional<Offset> end;
  if (!SerDe::deserializing) {
    end = (m_end == kInvalidOffset) ? folly::none : folly::make_optional(m_end);
  }

  sd(m_base)
    (m_past)
    (m_iterId)
    (m_handler)
    (end)
    (m_parentIndex)
    ;

  if (SerDe::deserializing) {
    m_end = end.value_or(kInvalidOffset);
  }
}

///////////////////////////////////////////////////////////////////////////////
// ParamInfo.

inline Func::ParamInfo::ParamInfo()
  : defaultValue(make_tv<KindOfUninit>())
{}

template<class SerDe>
inline void Func::ParamInfo::serde(SerDe& sd) {
  sd(builtinType)
    (funcletOff)
    (defaultValue)
    (phpCode)
    (typeConstraint)
    (flags)
    (userAttributes)
    (userType)
    ;
}

inline bool Func::ParamInfo::hasDefaultValue() const {
  return funcletOff != kInvalidOffset;
}

inline bool Func::ParamInfo::hasScalarDefaultValue() const {
  return hasDefaultValue() && defaultValue.m_type != KindOfUninit;
}

inline bool Func::ParamInfo::isInOut() const {
  return flags & (1 << static_cast<int32_t>(Flags::InOut));
}

inline bool Func::ParamInfo::isVariadic() const {
  return flags & (1 << static_cast<int32_t>(Flags::Variadic));
}

inline bool Func::ParamInfo::isNativeArg() const {
  return flags & (1 << static_cast<int32_t>(Flags::NativeArg));
}

inline bool Func::ParamInfo::isTakenAsVariant() const {
  return flags & (1 << static_cast<int32_t>(Flags::AsVariant));
}

inline bool Func::ParamInfo::isTakenAsTypedValue() const {
  return flags & (1 << static_cast<int32_t>(Flags::AsTypedValue));
}

inline void Func::ParamInfo::setFlag(Func::ParamInfo::Flags flag) {
  flags |= 1 << static_cast<int32_t>(flag);
}

///////////////////////////////////////////////////////////////////////////////
// Func.

inline const void* Func::mallocEnd() const {
  return reinterpret_cast<const char*>(this)
         + Func::prologueTableOff()
         + numPrologues() * sizeof(m_prologueTable[0]);
}

inline bool Func::validate() const {
#ifndef NDEBUG
  assertx(m_magic == kMagic);
#endif
  assertx(m_name != nullptr);
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// FuncId manipulation.

inline FuncId Func::getFuncId() const {
  assertx(m_funcId != InvalidFuncId);
  assertx(fromFuncId(m_funcId) == this);
  return m_funcId;
}

///////////////////////////////////////////////////////////////////////////////
// Basic info.

inline Unit* Func::unit() const {
  return m_unit;
}

inline Class* Func::cls() const {
  return !isMethCaller() ? m_u.cls() : nullptr;
}

inline PreClass* Func::preClass() const {
  return shared()->m_preClass;
}

inline Class* Func::baseCls() const {
  return !(m_baseCls & kMethCallerBit) ?
    reinterpret_cast<Class*>(m_baseCls) : nullptr;
}

inline Class* Func::implCls() const {
  return isClosureBody() ? baseCls() : cls();
}

inline const StringData* Func::name() const {
  assertx(m_name != nullptr);
  return m_name;
}

inline StrNR Func::nameStr() const {
  assertx(m_name != nullptr);
  return StrNR(m_name);
}

inline const StringData* Func::fullName() const {
  if (m_fullName == nullptr) return m_name;
  if (UNLIKELY((intptr_t)m_fullName.get() == kNeedsFullName)) {
    m_fullName = makeStaticString(
      std::string(cls()->name()->data()) + "::" + m_name->data());
  }
  return m_fullName;
}

inline StrNR Func::fullNameStr() const {
  assertx(m_fullName != nullptr);
  return StrNR(fullName());
}

inline void invalidFuncConversion(const char* type) {
  SystemLib::throwInvalidOperationExceptionObject(folly::sformat(
    "Cannot convert func to {}", type
  ));
}

inline NamedEntity* Func::getNamedEntity() {
  assertx(!shared()->m_preClass);
  return *reinterpret_cast<LowPtr<NamedEntity>*>(&m_namedEntity);
}

inline const NamedEntity* Func::getNamedEntity() const {
  assertx(!shared()->m_preClass);
  return *reinterpret_cast<const LowPtr<const NamedEntity>*>(&m_namedEntity);
}

inline void Func::setNamedEntity(const NamedEntity* e) {
  *reinterpret_cast<LowPtr<const NamedEntity>*>(&m_namedEntity) = e;
}

inline const StringData* Func::methCallerClsName() const {
  assertx(isMethCaller() && isBuiltin());
  return m_u.name();
}

inline const StringData* Func::methCallerMethName() const {
  assertx(isMethCaller() && isBuiltin() &&
          (m_methCallerMethName & kMethCallerBit));
  return reinterpret_cast<StringData*>(m_methCallerMethName - kMethCallerBit);
}

///////////////////////////////////////////////////////////////////////////////
// File info.

inline const StringData* Func::originalFilename() const {
  return shared()->m_originalFilename;
}

inline const StringData* Func::filename() const {
  // Builtins don't have filenames
  if (isBuiltin()) {
    return staticEmptyString();
  }

  // Use the original filename if it exists, otherwise grab the filename from
  // the unit
  const StringData* name = originalFilename();
  if (!name) {
    assertx(m_unit);
    name = m_unit->filepath();
    assertx(name);
  }
  return name;
}

inline int Func::line1() const {
  return shared()->m_line1;
}

inline int Func::line2() const {
  auto const sd = shared();
  auto const delta = sd->m_line2Delta;
  if (UNLIKELY(delta == kSmallDeltaLimit)) {
    assertx(extShared());
    return static_cast<const ExtendedSharedData*>(sd)->m_line2;
  }
  return line1() + delta;
}

inline const StringData* Func::docComment() const {
  return shared()->m_docComment;
}

///////////////////////////////////////////////////////////////////////////////
// Bytecode.

inline PC Func::entry() const {
  return m_unit->entry() + shared()->m_base;
}

inline Offset Func::base() const {
  return shared()->m_base;
}

inline Offset Func::past() const {
  auto const sd = shared();
  auto const delta = sd->m_pastDelta;
  if (UNLIKELY(delta == kSmallDeltaLimit)) {
    assertx(extShared());
    return static_cast<const ExtendedSharedData*>(sd)->m_past;
  }
  return base() + delta;
}

inline bool Func::contains(PC pc) const {
  return contains(Offset(pc - unit()->entry()));
}

inline bool Func::contains(Offset offset) const {
  return offset >= base() && offset < past();
}

inline PC Func::at(Offset off) const {
  // We don't use contains because we want to allow past becase it is often
  // used in loops
  assertx(off >= base() && off <= past());
  return unit()->entry() + off;
}

inline Offset Func::offsetOf(PC pc) const {
  assertx(contains(pc));
  return pc - unit()->entry();
}

inline Op Func::getOp(Offset instrOffset) const {
  assertx(contains(instrOffset));
  return peek_op(unit()->entry() + instrOffset);
}

inline Offset Func::ctiEntry() const {
  return shared()->m_cti_base.load(std::memory_order_acquire);
}

inline void Func::setCtiFunclet(int i, Offset cti_funclet) {
  shared()->m_params[i].ctiFunclet = cti_funclet;
}

inline void Func::setCtiEntry(Offset base, uint32_t size) {
  auto sd = shared();
  sd->m_cti_size = size;
  sd->m_cti_base.store(base, std::memory_order_release);
}

///////////////////////////////////////////////////////////////////////////////
// Return type.

inline MaybeDataType Func::hniReturnType() const {
  auto const ex = extShared();
  return ex ? ex->m_hniReturnType : folly::none;
}

inline RepoAuthType Func::repoReturnType() const {
  return shared()->m_repoReturnType;
}

inline RepoAuthType Func::repoAwaitedReturnType() const {
  return shared()->m_repoAwaitedReturnType;
}

inline bool Func::isReturnByValue() const {
  return shared()->m_allFlags.m_returnByValue;
}

inline const TypeConstraint& Func::returnTypeConstraint() const {
  return shared()->m_retTypeConstraint;
}

inline const StringData* Func::returnUserType() const {
  return shared()->m_retUserType;
}

inline bool Func::hasReturnWithMultiUBs() const {
  return shared()->m_allFlags.m_hasReturnWithMultiUBs;
}

inline const Func::UpperBoundVec& Func::returnUBs() const {
  assertx(hasReturnWithMultiUBs());
  return extShared()->m_returnUBs;
}

///////////////////////////////////////////////////////////////////////////////
// Parameters.

inline const Func::ParamInfoVec& Func::params() const {
  return shared()->m_params;
}

inline uint32_t Func::numParams() const {
  assertx(bool(m_attrs & AttrVariadicParam) != bool(m_paramCounts & 1));
  assertx((m_paramCounts >> 1) == params().size());
  return (m_paramCounts) >> 1;
}

inline uint32_t Func::numNonVariadicParams() const {
  assertx(bool(m_attrs & AttrVariadicParam) != bool(m_paramCounts & 1));
  assertx((m_paramCounts >> 1) == params().size());
  return (m_paramCounts - 1) >> 1;
}

inline uint32_t Func::numRequiredParams() const {
  for (auto i = numNonVariadicParams(); i > 0; --i) {
    if (!params()[i - 1].hasDefaultValue()) return i;
  }
  return 0;
}

inline bool Func::hasVariadicCaptureParam() const {
#ifndef NDEBUG
  assertx(bool(m_attrs & AttrVariadicParam) ==
         (numParams() && params()[numParams() - 1].isVariadic()));
#endif
  return m_attrs & AttrVariadicParam;
}

inline bool Func::hasParamsWithMultiUBs() const {
  return shared()->m_allFlags.m_hasParamsWithMultiUBs;
}

inline const Func::ParamUBMap& Func::paramUBs() const {
  assertx(hasParamsWithMultiUBs());
  return extShared()->m_paramUBs;
}

///////////////////////////////////////////////////////////////////////////////
// Locals, iterators, and stack.

inline int Func::numLocals() const {
  return shared()->m_numLocals;
}

inline int Func::numIterators() const {
  return shared()->m_numIterators;
}

inline Id Func::numNamedLocals() const {
  return shared()->m_localNames.size();
}

inline const StringData* Func::localVarName(Id id) const {
  assertx(id >= 0);
  return id < numNamedLocals() ? shared()->m_localNames[id] : nullptr;
}

inline LowStringPtr const* Func::localNames() const {
  return shared()->m_localNames.accessList();
}

inline int Func::maxStackCells() const {
  return m_maxStackCells;
}

inline int Func::numSlotsInFrame() const {
  return
    shared()->m_numLocals +
    shared()->m_numIterators * (sizeof(Iter) / sizeof(TypedValue));
}

inline bool Func::hasForeignThis() const {
  return m_hasForeignThis;
}

inline void Func::setHasForeignThis(bool hasForeignThis) {
  m_hasForeignThis = hasForeignThis;
}

inline void Func::setGenerated(bool isGenerated) {
  shared()->m_allFlags.m_isGenerated = isGenerated;
}

///////////////////////////////////////////////////////////////////////////////
// Definition context.

inline bool Func::isMethod() const {
  return (bool)baseCls();
}

inline bool Func::isFromTrait() const {
  return m_attrs & AttrTrait;
}

inline bool Func::isPublic() const {
  return m_attrs & AttrPublic;
}

inline bool Func::isStatic() const {
  return m_attrs & AttrStatic;
}

inline bool Func::isStaticInPrologue() const {
  return isStatic() && !isClosureBody();
}

inline bool Func::hasThisInBody() const {
  return cls() && !isStatic();
}

inline bool Func::hasNoContextAttr() const {
  return m_attrs & AttrNoContext;
}

inline bool Func::isAbstract() const {
  return m_attrs & AttrAbstract;
}

inline bool Func::isPreFunc() const {
  return m_isPreFunc;
}

inline bool Func::isMemoizeWrapper() const {
  return shared()->m_allFlags.m_isMemoizeWrapper;
}

inline bool Func::isMemoizeWrapperLSB() const {
  return shared()->m_allFlags.m_isMemoizeWrapperLSB;
}

inline bool Func::isMemoizeImpl() const {
  return isMemoizeImplName(name());
}

inline const StringData* Func::memoizeImplName() const {
  assertx(isMemoizeWrapper());
  return genMemoizeImplName(name());
}

///////////////////////////////////////////////////////////////////////////////
// Builtins.

inline bool Func::isBuiltin() const {
  return m_attrs & AttrBuiltin;
}

inline bool Func::isCPPBuiltin() const {
  auto const ex = extShared();
  return UNLIKELY(!!ex) && ex->m_arFuncPtr;
}

inline ArFunction Func::arFuncPtr() const {
  if (auto const ex = extShared()) return ex->m_arFuncPtr;
  return nullptr;
}

inline NativeFunction Func::nativeFuncPtr() const {
  if (auto const ex = extShared()) return ex->m_nativeFuncPtr;
  return nullptr;
}

///////////////////////////////////////////////////////////////////////////////
// Closures.

inline bool Func::isClosureBody() const {
  return shared()->m_allFlags.m_isClosureBody;
}

///////////////////////////////////////////////////////////////////////////////
// Resumables.

inline bool Func::isAsync() const {
  return shared()->m_allFlags.m_isAsync;
}

inline bool Func::isGenerator() const {
  return shared()->m_allFlags.m_isGenerator;
}

inline bool Func::isPairGenerator() const {
  return shared()->m_allFlags.m_isPairGenerator;
}

inline bool Func::isAsyncFunction() const {
  return isAsync() && !isGenerator();
}

inline bool Func::isNonAsyncGenerator() const {
  return !isAsync() && isGenerator();
}

inline bool Func::isAsyncGenerator() const {
  return isAsync() && isGenerator();
}

inline bool Func::isResumable() const {
  return isAsync() || isGenerator();
}

///////////////////////////////////////////////////////////////////////////////
// Reactivity.

inline RxLevel Func::rxLevel() const {
  return rxLevelFromAttr(m_attrs);
}

inline bool Func::isRxDisabled() const {
  return shared()->m_allFlags.m_isRxDisabled;
}

inline bool Func::isRxConditional() const {
  return rxConditionalFromAttr(m_attrs);
}

///////////////////////////////////////////////////////////////////////////////
// Methods.

inline Slot Func::methodSlot() const {
  assertx(isMethod());
  return m_methodSlot;
}

inline bool Func::hasPrivateAncestor() const {
  return m_hasPrivateAncestor;
}

///////////////////////////////////////////////////////////////////////////////
// Magic methods.

inline bool Func::isGenerated() const {
  return shared()->m_allFlags.m_isGenerated;
}

inline bool Func::isSpecial(const StringData* name) {
  return strncmp("86", name->data(), 2) == 0;
}

///////////////////////////////////////////////////////////////////////////////
// Other attributes.

inline Attr Func::attrs() const {
  return m_attrs;
}

inline const UserAttributeMap& Func::userAttributes() const {
  return shared()->m_userAttributes;
}

inline bool Func::isUnique() const {
  return m_attrs & AttrUnique;
}

inline bool Func::isPersistent() const {
  return m_attrs & AttrPersistent;
}

inline bool Func::isInterceptable() const {
  return m_attrs & AttrInterceptable;
}

inline bool Func::isNoInjection() const {
  return m_attrs & AttrNoInjection;
}

inline bool Func::isSkipFrame() const {
  return isCPPBuiltin() || (isBuiltin() && !isMethod());
}

inline bool Func::isProvenanceSkipFrame() const {
  return m_attrs & AttrProvenanceSkipFrame;
}

inline bool Func::isFoldable() const {
  return m_attrs & AttrIsFoldable;
}

inline bool Func::supportsAsyncEagerReturn() const {
  return m_attrs & AttrSupportsAsyncEagerReturn;
}

inline bool Func::isDynamicallyCallable() const {
  return m_attrs & AttrDynamicallyCallable;
}

inline folly::Optional<int64_t> Func::dynCallSampleRate() const {
  if (auto const ex = extShared()) {
    if (ex->m_dynCallSampleRate >= 0) return ex->m_dynCallSampleRate;
  }
  return folly::none;
}

inline bool Func::isMethCaller() const {
  return m_attrs & AttrIsMethCaller;
}

inline bool Func::isPhpLeafFn() const {
  return shared()->m_allFlags.m_isPhpLeafFn;
}

inline bool Func::hasReifiedGenerics() const {
  return shared()->m_allFlags.m_hasReifiedGenerics;
}

///////////////////////////////////////////////////////////////////////////////
// Unit table entries.

inline const Func::EHEntVec& Func::ehtab() const {
  return shared()->m_ehtab;
}

inline const EHEnt* Func::findEH(Offset o) const {
  assertx(o >= base() && o < past());
  return findEH(shared()->m_ehtab, o);
}

template<class Container>
const typename Container::value_type*
Func::findEH(const Container& ehtab, Offset o) {
  const typename Container::value_type* eh = nullptr;

  for (uint32_t i = 0, sz = ehtab.size(); i < sz; ++i) {
    if (ehtab[i].m_base <= o && o < ehtab[i].m_past) {
      eh = &ehtab[i];
    }
  }
  return eh;
}

///////////////////////////////////////////////////////////////////////////////
// JIT data.

inline rds::Handle Func::funcHandle() const {
  return m_cachedFunc.handle();
}

inline unsigned char* Func::getFuncBody() const {
  return m_funcBody;
}

inline void Func::setFuncBody(unsigned char* fb) {
  m_funcBody = fb;
}

inline uint8_t* Func::getPrologue(int index) const {
  return m_prologueTable[index];
}

inline void Func::setPrologue(int index, unsigned char* tca) {
  m_prologueTable[index] = tca;
}

///////////////////////////////////////////////////////////////////////////////
// Other methods.

inline int8_t& Func::maybeIntercepted() const {
  return m_maybeIntercepted;
}

///////////////////////////////////////////////////////////////////////////////
// Public setters.

inline void Func::setAttrs(Attr attrs) {
  m_attrs = attrs;
}

inline void Func::setBaseCls(Class* baseCls) {
  m_baseCls = to_low(baseCls);
}

inline void Func::setFuncHandle(rds::Link<LowPtr<Func>,
                                          rds::Mode::NonLocal> l) {
  // TODO(#2950356): This assertion fails for create_function with an existing
  // declared function named __lambda_func.
  //assertx(!m_cachedFunc.valid());
  m_cachedFunc = l;
}

inline void Func::setHasPrivateAncestor(bool b) {
  m_hasPrivateAncestor = b;
}

inline void Func::setMethodSlot(Slot s) {
  assertx(isMethod());
  m_methodSlot = s;
}

inline bool Func::serialize() const {
  if (m_serialized) return false;
  const_cast<Func*>(this)->m_serialized = true;
  return true;
}

//////////////////////////////////////////////////////////////////////

inline const Func::ExtendedSharedData* Func::extShared() const {
  return const_cast<Func*>(this)->extShared();
}

inline Func::ExtendedSharedData* Func::extShared() {
  auto const s = shared();
  return UNLIKELY(s->m_allFlags.m_hasExtendedSharedData)
    ? static_cast<ExtendedSharedData*>(s)
    : nullptr;
}

///////////////////////////////////////////////////////////////////////////////
}
