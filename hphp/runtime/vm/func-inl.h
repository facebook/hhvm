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
  Optional<Offset> end;
  if (!SerDe::deserializing) {
    end = (m_end == kInvalidOffset) ? std::nullopt : make_optional(m_end);
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
  sd(funcletOff)
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

inline bool Func::ParamInfo::hasTrivialDefaultValue() const {
  return hasScalarDefaultValue() && typeConstraint.alwaysPasses(&defaultValue);
}

inline bool Func::ParamInfo::isInOut() const {
  return flags & (1 << static_cast<int32_t>(Flags::InOut));
}

inline bool Func::ParamInfo::isReadonly() const {
  return flags & (1 << static_cast<int32_t>(Flags::Readonly));
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

inline MaybeDataType Func::ParamInfo::builtinType() const {
  return isVariadic() ? KindOfVec : typeConstraint.asSystemlibType();
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
#ifdef USE_LOWPTR
  assertx(fromFuncId(FuncId{this}) == this);
  return FuncId{this};
#else
  assertx(!m_funcId.isInvalid());
  assertx(fromFuncId(m_funcId) == this);
  return m_funcId;
#endif
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

inline String Func::nameWithClosureName() const {
  if (!isClosureBody()) return String(const_cast<StringData*>(name()));
  // Strip the file hash from the closure name.
  String name{const_cast<StringData*>(baseCls()->name())};
  auto const pos = name.find(';');
  if (pos < 0) return name;
  return name.substr(0, pos);
}

inline StrNR Func::nameStr() const {
  assertx(m_name != nullptr);
  return StrNR(m_name);
}

inline size_t Func::stableHash() const {
  return folly::hash::hash_combine(
    name()->hashStatic(),
    cls() ? cls()->name()->hashStatic() : 0,
    unit()->sn()
  );
}

inline const StringData* Func::fullName() const {
  if (m_fullName == nullptr) return m_name;
  if (UNLIKELY((intptr_t)m_fullName.get() == kNeedsFullName)) {
    m_fullName = makeStaticString(
      std::string(cls()->name()->data()) + "::" + m_name->data());
  }
  return m_fullName;
}

inline String Func::fullNameWithClosureName() const {
  if (!isClosureBody()) return String(const_cast<StringData*>(fullName()));
  return nameWithClosureName();
}

inline StrNR Func::fullNameStr() const {
  assertx(m_fullName != nullptr);
  return StrNR(fullName());
}

inline void invalidFuncConversion(const char* type) {
  SystemLib::throwInvalidOperationExceptionObject(folly::sformat(
    "Cannot convert function to {}", type
  ));
}

inline NamedFunc* Func::getNamedFunc() {
  assertx(!shared()->m_preClass);
  return *reinterpret_cast<LowPtr<NamedFunc>*>(&m_namedFunc);
}

inline const NamedFunc* Func::getNamedFunc() const {
  assertx(!shared()->m_preClass);
  return *reinterpret_cast<const LowPtr<const NamedFunc>*>(&m_namedFunc);
}

inline void Func::setNamedFunc(const NamedFunc* e) {
  *reinterpret_cast<LowPtr<const NamedFunc>*>(&m_namedFunc) = e;
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

inline int Func::sn() const {
  auto const sd = shared();
  auto small = sd->m_sn;
  if (UNLIKELY(small == kSmallDeltaLimit)) {
    assertx(extShared());
    return static_cast<const ExtendedSharedData*>(sd)->m_sn;
  }
  return small;
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
  auto const ex = extShared();
  return ex ? ex->m_docComment : staticEmptyString();
}

///////////////////////////////////////////////////////////////////////////////
// Bytecode.

inline PC Func::entry() const {
  auto const bc = shared()->m_bc.copy();
  return bc.isPtr() ? bc.ptr() : const_cast<Func*>(this)->loadBytecode();
}

inline Offset Func::bclen() const {
  return shared()->bclen();
}

inline Offset Func::SharedData::bclen() const {
  auto const len = this->m_bclenSmall;
  if (UNLIKELY(len == kSmallDeltaLimit)) {
    assertx(m_allFlags.m_hasExtendedSharedData);
    return static_cast<const ExtendedSharedData*>(this)->m_bclen;
  }
  return len;
}

inline bool Func::contains(PC pc) const {
  return uintptr_t(pc - entry()) < bclen();
}

inline bool Func::contains(Offset offset) const {
  assertx(offset >= 0);
  return offset < bclen();
}

inline PC Func::at(Offset off) const {
  // We don't use contains because we want to allow past becase it is often
  // used in loops
  assertx(off >= 0 && off <= bclen());
  return entry() + off;
}

inline Offset Func::offsetOf(PC pc) const {
  assertx(contains(pc));
  return pc - entry();
}

inline Op Func::getOp(Offset instrOffset) const {
  assertx(contains(instrOffset));
  return peek_op(entry() + instrOffset);
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

inline RepoAuthType Func::repoReturnType() const {
  return shared()->m_repoReturnType;
}

inline RepoAuthType Func::repoAwaitedReturnType() const {
  return shared()->m_repoAwaitedReturnType;
}

inline bool Func::isReturnByValue() const {
  return shared()->m_allFlags.m_returnByValue;
}

inline bool Func::hasUntrustedReturnType() const {
  return shared()->m_allFlags.m_isUntrustedReturnType;
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

inline uint64_t Func::inOutBits() const {
  return m_inoutBits;
}

inline bool Func::takesInOutParams() const {
  return m_inoutBits != 0;
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

inline uint32_t Func::reifiedGenericsLocalId() const {
  assertx(hasReifiedGenerics());
  return numParams();
}

inline uint32_t Func::coeffectsLocalId() const {
  assertx(hasCoeffectsLocal());
  auto id = numParams();
  if (hasReifiedGenerics()) ++id;
  return id;
}

inline uint32_t Func::numFuncEntryInputs() const {
  auto id = numParams();
  if (hasReifiedGenerics()) ++id;
  if (hasCoeffectsLocal()) ++id;
  return id;
}

inline uint32_t Func::firstClosureUseLocalId() const {
  assertx(isClosureBody());
  return numFuncEntryInputs();
}

inline uint32_t Func::firstRegularLocalId() const {
  auto id = numParams();
  if (hasReifiedGenerics()) ++id;
  if (hasCoeffectsLocal()) ++id;
  if (isClosureBody()) id += numClosureUseLocals();
  return id;
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

inline bool Func::isInternal() const {
  return m_attrs & AttrInternal;
}

inline const StringData* Func::moduleName() const {
  if (RO::EvalModuleLevelTraits) {
    auto const ex = extShared();
    if (ex && ex->m_originalModuleName) {
      return ex->m_originalModuleName;
    }
  }
  return unit()->moduleName();
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

inline Func::MemoizeICType Func::memoizeICType() const {
  assertx(isMemoizeWrapper());
  return shared()->m_allFlags.m_memoizeICType;
}

inline bool Func::isNoICMemoize() const {
  return memoizeICType() == MemoizeICType::NoIC;
}

inline bool Func::isKeyedByImplicitContextMemoize() const {
  return memoizeICType() == MemoizeICType::KeyedByIC;
}

inline bool Func::isMakeICInaccessibleMemoize() const {
  return memoizeICType() == MemoizeICType::MakeICInaccessible;
}

inline bool Func::isSoftMakeICInaccessibleMemoize() const {
  return memoizeICType() == MemoizeICType::SoftMakeICInaccessible;
}

inline uint32_t Func::softMakeICInaccessibleSampleRate() const {
  assertx(isSoftMakeICInaccessibleMemoize());
  auto const ex = extShared();
  return ex ? ex->m_softMakeICInaccessibleSampleRate : 1;
}

inline bool Func::isMemoizeImpl() const {
  return isMemoizeImplName(name());
}

inline const StringData* Func::memoizeImplName() const {
  assertx(isMemoizeWrapper());
  return genMemoizeImplName(name());
}

inline size_t Func::numKeysForMemoize() const {
  return numParams()
         + (hasReifiedGenerics() ? 1 : 0)
         + (isKeyedByImplicitContextMemoize() ? 1 : 0);
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
// Coeffects.

inline RuntimeCoeffects Func::requiredCoeffects() const {
  return m_requiredCoeffects;
}

inline RuntimeCoeffects Func::coeffectEscapes() const {
  return extShared() ? extShared()->m_coeffectEscapes
                     : RuntimeCoeffects::none();
}

inline void Func::setRequiredCoeffects(RuntimeCoeffects c) {
  m_requiredCoeffects = c;
}

inline StaticCoeffectNamesMap Func::staticCoeffectNames() const {
  return shared()->m_staticCoeffectNames;
}

inline bool Func::hasCoeffectsLocal() const {
  return hasCoeffectRules() &&
         !(getCoeffectRules().size() == 1 &&
           getCoeffectRules()[0].isGeneratorThis());
}

inline bool Func::hasCoeffectRules() const {
  return attrs() & AttrHasCoeffectRules;
}

inline const Func::CoeffectRules& Func::getCoeffectRules() const {
  assertx(extShared());
  assertx(hasCoeffectRules());
  return extShared()->m_coeffectRules;
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

inline Optional<int64_t> Func::dynCallSampleRate() const {
  if (auto const ex = extShared()) {
    if (ex->m_dynCallSampleRate >= 0) return ex->m_dynCallSampleRate;
  }
  return std::nullopt;
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
  assertx(o >= 0 && o < bclen());
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

inline jit::TCA Func::getFuncEntry() const {
  return m_funcEntry;
}

inline void Func::setFuncEntry(jit::TCA funcEntry) {
  m_funcEntry = funcEntry;
}

inline uint8_t* Func::getPrologue(int index) const {
  return m_prologueTable[index];
}

inline void Func::setPrologue(int index, unsigned char* tca) {
  m_prologueTable[index] = tca;
}

inline uint8_t Func::incJitReqCount() const {
  auto curr = m_jitReqCount.load(std::memory_order_acquire);
  do {
    // Saturate
    if (curr >= std::numeric_limits<uint8_t>::max()) return curr;
    if (m_jitReqCount.compare_exchange_weak(curr, curr + 1,
                                            std::memory_order_acq_rel)) {
      return curr;
    }
  } while (true);
  not_reached();
}

inline void Func::resetJitReqCount() const {
  m_jitReqCount.store(0, std::memory_order_release);
}

///////////////////////////////////////////////////////////////////////////////
// Other methods.

inline bool Func::maybeIntercepted() const {
  return atomicFlags().check(Func::Flags::MaybeIntercepted);
}

inline void Func::setMaybeIntercepted() {
  assertx(isInterceptable());
  atomicFlags().set(Func::Flags::MaybeIntercepted);
}

///////////////////////////////////////////////////////////////////////////////
// Public setters.

inline void Func::setAttrs(Attr attrs) {
  m_attrs = attrs;
}

inline void Func::setBaseCls(Class* baseCls) {
  m_baseCls = to_low(baseCls);
}

inline void Func::setHasPrivateAncestor(bool b) {
  m_hasPrivateAncestor = b;
}

inline void Func::setMethodSlot(Slot s) {
  assertx(isMethod());
  m_methodSlot = s;
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
