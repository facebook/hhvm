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
// EH and FPI tables.

template<class SerDe>
void FPIEnt::serde(SerDe& sd) {
  sd(m_fpushOff)
    (m_fpiEndOff)
    (m_fpOff)
    // These fields are recomputed by sortFPITab:
    // (m_parentIndex)
    // (m_fpiDepth)
    ;
}

template<class SerDe>
void EHEnt::serde(SerDe& sd) {
  sd(m_type)
    (m_base)
    (m_past)
    (m_iterId)
    (m_handler)
    (m_end)
    (m_itRef)
    (m_parentIndex)
    ;
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
    (variadic)
    (userAttributes)
    (userType)
    (inout)
    ;
}

inline bool Func::ParamInfo::hasDefaultValue() const {
  return funcletOff != InvalidAbsoluteOffset;
}

inline bool Func::ParamInfo::hasScalarDefaultValue() const {
  return hasDefaultValue() && defaultValue.m_type != KindOfUninit;
}

inline bool Func::ParamInfo::isVariadic() const {
  return variadic;
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

inline bool Func::top() const {
  return shared()->m_top;
}

inline Unit* Func::unit() const {
  return m_unit;
}

inline Class* Func::cls() const {
  return m_cls;
}

inline PreClass* Func::preClass() const {
  return shared()->m_preClass;
}

inline Class* Func::baseCls() const {
  return m_baseCls;
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
  return m_fullName;
}

inline StrNR Func::fullNameStr() const {
  assertx(m_fullName != nullptr);
  return StrNR(m_fullName);
}

inline const StringData* Func::displayName() const {
  return LIKELY(!takesInOutParams()) ? name() : stripInOutSuffix(name());
}

inline const StringData* Func::fullDisplayName() const {
  return
    LIKELY(!takesInOutParams()) ? fullName() : stripInOutSuffix(fullName());
}

inline const StringData* funcToStringHelper(const Func* func) {
  if (RuntimeOption::EvalRaiseFuncConversionWarning) {
    raise_warning("Func to string conversion");
  }
  return func->fullDisplayName();
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

inline PC Func::getEntry() const {
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
  return shared()->m_returnByValue;
}

inline bool Func::isReturnRef() const {
  return m_attrs & AttrReference;
}

inline const TypeConstraint& Func::returnTypeConstraint() const {
  return shared()->m_retTypeConstraint;
}

inline const StringData* Func::returnUserType() const {
  return shared()->m_retUserType;
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

inline bool Func::hasVariadicCaptureParam() const {
#ifndef NDEBUG
  assertx(bool(m_attrs & AttrVariadicParam) ==
         (numParams() && params()[numParams() - 1].variadic));
#endif
  return m_attrs & AttrVariadicParam;
}

inline bool Func::discardExtraArgs() const {
  return !(m_attrs & (AttrMayUseVV | AttrVariadicParam));
}

inline bool Func::takesInOutParams() const {
  return m_attrs & AttrTakesInOutParams;
}

inline bool Func::isInOutWrapper() const {
  return m_attrs & AttrIsInOutWrapper;
}

inline uint32_t Func::numInOutParams() const {
  if (!takesInOutParams()) return 0;
  uint32_t count = 0;
  for (uint32_t i = 0; i < numParams(); ++i) {
    if (params()[i].inout) ++count;
  }
  return count;
}

///////////////////////////////////////////////////////////////////////////////
// Locals, iterators, and stack.

inline int Func::numLocals() const {
  return shared()->m_numLocals;
}

inline int Func::numIterators() const {
  return shared()->m_numIterators;
}

inline int Func::numClsRefSlots() const {
  auto const ex = extShared();
  if (LIKELY(!ex)) return shared()->m_numClsRefSlots;
  return ex->m_actualNumClsRefSlots;
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
  return shared()->m_numLocals +
    shared()->m_numIterators * (sizeof(Iter) / sizeof(Cell)) +
    (numClsRefSlots() * sizeof(Class*) + sizeof(Cell) - 1) / sizeof(Cell);
}

inline bool Func::hasForeignThis() const {
  return m_hasForeignThis;
}

inline void Func::setHasForeignThis(bool hasForeignThis) {
  m_hasForeignThis = hasForeignThis;
}

///////////////////////////////////////////////////////////////////////////////
// Static locals.

inline const Func::SVInfoVec& Func::staticVars() const {
  return shared()->m_staticVars;
}

inline bool Func::hasStaticLocals() const {
  return !shared()->m_staticVars.empty();
}

inline int Func::numStaticLocals() const {
  return shared()->m_staticVars.size();
}

///////////////////////////////////////////////////////////////////////////////
// Definition context.

inline bool Func::isPseudoMain() const {
  return m_name->empty();
}

inline bool Func::isMethod() const {
  return !isPseudoMain() && (bool)baseCls();
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
  return (m_attrs & (AttrStatic | AttrRequiresThis)) == AttrStatic;
}

inline bool Func::requiresThisInBody() const {
  return (m_attrs & AttrRequiresThis) && !isClosureBody();
}

inline bool Func::hasThisVaries() const {
  return mayHaveThis() && !requiresThisInBody();
}

inline bool Func::isAbstract() const {
  return m_attrs & AttrAbstract;
}

inline bool Func::mayHaveThis() const {
  return cls() && !isStatic();
}

inline bool Func::isPreFunc() const {
  return m_isPreFunc;
}

inline bool Func::isMemoizeWrapper() const {
  return shared()->m_isMemoizeWrapper;
}

inline bool Func::isMemoizeWrapperLSB() const {
  return shared()->m_isMemoizeWrapperLSB;
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

inline bool Func::readsCallerFrame() const {
  return m_attrs & AttrReadsCallerFrame;
}

inline bool Func::writesCallerFrame() const {
  return m_attrs & AttrWritesCallerFrame;
}

inline bool Func::accessesCallerFrame() const {
  return m_attrs & (AttrReadsCallerFrame | AttrWritesCallerFrame);
}

inline bool Func::takesNumArgs() const {
  return shared()->m_takesNumArgs;
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
  return shared()->m_isClosureBody;
}

///////////////////////////////////////////////////////////////////////////////
// Resumables.

inline bool Func::isAsync() const {
  return shared()->m_isAsync;
}

inline bool Func::isGenerator() const {
  return shared()->m_isGenerator;
}

inline bool Func::isPairGenerator() const {
  return shared()->m_isPairGenerator;
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
  return shared()->m_isGenerated;
}

inline bool Func::isDestructor() const {
  return !strcmp(m_name->data(), "__destruct");
}

inline bool Func::isMagic() const {
  return isMagicCallMethod() || isMagicCallStaticMethod();
}

inline bool Func::isMagicCallMethod() const {
  return m_name->isame(s___call);
}

inline bool Func::isMagicCallStaticMethod() const {
  return m_name->isame(s___callStatic);
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
  return m_attrs & AttrSkipFrame;
}

inline bool Func::isFoldable() const {
  return m_attrs & AttrIsFoldable;
}

inline bool Func::isParamCoerceMode() const {
  return attrs() & (AttrParamCoerceModeFalse | AttrParamCoerceModeNull);
}

inline bool Func::isDynamicallyCallable() const {
  return m_attrs & AttrDynamicallyCallable;
}

inline bool Func::isHot() const {
  return m_hot;
}

inline void Func::setHot() {
  m_hot = true;
}

inline bool Func::isPhpLeafFn() const {
  return shared()->m_isPhpLeafFn;
}

///////////////////////////////////////////////////////////////////////////////
// Unit table entries.

inline const Func::EHEntVec& Func::ehtab() const {
  return shared()->m_ehtab;
}

inline const Func::FPIEntVec& Func::fpitab() const {
  return shared()->m_fpitab;
}

inline const EHEnt* Func::findEH(Offset o) const {
  assertx(o >= base() && o < past());
  return findEH(shared()->m_ehtab, o);
}

inline const EHEnt* Func::findEHbyHandler(Offset o) const {
  assertx(o >= base() && o < past());
  return findEHbyHandler(shared()->m_ehtab, o);
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

template<class Container>
const typename Container::value_type*
Func::findEHbyHandler(const Container& ehtab, Offset o) {
  const typename Container::value_type* eh = nullptr;
  Offset closest = 0;

  // We cannot rely on m_end to be a useful value (not kInvalidOffset), so
  // instead we take the handler whose start is the closest without going
  // over the offset we are looking for. We can rely on the fault handlers
  // to be both contigous and dominated by an Unwind/Catch because we
  // verify this in checkSection() in the verifier

  for (uint32_t i = 0, sz = ehtab.size(); i < sz; ++i) {
    if (ehtab[i].m_handler <= o && ehtab[i].m_handler > closest) {
      eh = &ehtab[i];
      closest = ehtab[i].m_handler;
    }
  }
  return eh;
}


inline const FPIEnt* Func::findFPI(Offset o) const {
  assertx(o >= base() && o < past());
  return findFPI(fpitab().begin(), fpitab().end(), o);
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
  assertx(IMPLIES(accessesCallerFrame(), isBuiltin() && !isMethod()));
}

inline void Func::setBaseCls(Class* baseCls) {
  m_baseCls = baseCls;
}

inline void Func::setFuncHandle(rds::Link<LowPtr<Func>,
                                          rds::Mode::NonLocal> l) {
  // TODO(#2950356): This assertion fails for create_function with an existing
  // declared function named __lambda_func.
  //assert(!m_cachedFunc.valid());
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
  return UNLIKELY(s->m_hasExtendedSharedData)
    ? static_cast<ExtendedSharedData*>(s)
    : nullptr;
}

///////////////////////////////////////////////////////////////////////////////
}
