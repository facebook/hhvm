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

#include "hphp/runtime/vm/func-emitter.h"

#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/coeffects-config.h"
#include "hphp/runtime/base/unit-cache.h"
#include "hphp/runtime/base/record-replay.h"
#include "hphp/runtime/base/runtime-option.h"

#include "hphp/runtime/ext/extension.h"

#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/native.h"
#include "hphp/runtime/vm/preclass-emitter.h"
#include "hphp/runtime/vm/reified-generics.h"
#include "hphp/runtime/vm/repo-autoload-map-builder.h"
#include "hphp/runtime/vm/repo-file.h"
#include "hphp/runtime/vm/runtime.h"

#include "hphp/runtime/vm/jit/types.h"

#include "hphp/runtime/vm/verifier/cfg.h"

#include "hphp/system/systemlib.h"

#include "hphp/util/atomic-vector.h"
#include "hphp/util/blob-encoder.h"
#include "hphp/util/file.h"
#include "hphp/util/trace.h"
#include <iterator>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////
// FuncEmitter.

FuncEmitter::FuncEmitter(UnitEmitter& ue, int sn, Id id, const StringData* n)
  : m_ue(ue)
  , m_pce(nullptr)
  , m_sn(sn)
  , m_id(id)
  , m_bc()
  , m_bclen(0)
  , m_bcmax(0)
  , name(n)
  , maxStackCells(0)
  , retUserType(nullptr)
  , docComment(nullptr)
  , originalFilename(nullptr)
  , originalModuleName(nullptr)
  , memoizePropName(nullptr)
  , memoizeGuardPropName(nullptr)
  , memoizeSharedPropIndex(0)
  , m_numLocals(0)
  , m_numUnnamedLocals(0)
  , m_numIterators(0)
  , m_nextFreeIterator(0)
  , m_ehTabSorted(false)
{}

FuncEmitter::FuncEmitter(UnitEmitter& ue, int sn, const StringData* n,
                         PreClassEmitter* pce)
  : m_ue(ue)
  , m_pce(pce)
  , m_sn(sn)
  , m_id(kInvalidId)
  , m_bc()
  , m_bclen(0)
  , m_bcmax(0)
  , name(n)
  , maxStackCells(0)
  , retUserType(nullptr)
  , docComment(nullptr)
  , originalFilename(nullptr)
  , originalModuleName(nullptr)
  , memoizePropName(nullptr)
  , memoizeGuardPropName(nullptr)
  , memoizeSharedPropIndex(0)
  , m_numLocals(0)
  , m_numUnnamedLocals(0)
  , m_numIterators(0)
  , m_nextFreeIterator(0)
  , m_ehTabSorted(false)
{}

FuncEmitter::~FuncEmitter() {
  if (m_bc.isPtr()) {
    if (auto const p = m_bc.ptr()) free(p);
  }
  if (m_lineTable.isPtr()) {
    if (auto const p = m_lineTable.ptr()) delete p;
  }
}


///////////////////////////////////////////////////////////////////////////////
// Source locations.

SourceLocTable FuncEmitter::createSourceLocTable() const {
  assertx(m_sourceLocTab.size() != 0);
  SourceLocTable locations;
  for (size_t i = 0; i < m_sourceLocTab.size(); ++i) {
    Offset endOff = i < m_sourceLocTab.size() - 1
      ? m_sourceLocTab[i + 1].first
      : m_bclen;
    locations.push_back(SourceLocEntry(endOff, m_sourceLocTab[i].second));
  }
  return locations;
}

void FuncEmitter::setLineTable(LineTable table) {
  if (m_lineTable.isPtr()) {
    if (auto p = m_lineTable.ptr()) {
      *p = std::move(table);
      return;
    }
  }
  m_lineTable = Func::LineTablePtr::FromPtr(new LineTable{std::move(table)});
}

namespace {

using SrcLoc = std::vector<std::pair<Offset, SourceLoc>>;

/*
 * Create a LineTable from `srcLoc'.
 */
LineTable createLineTable(const SrcLoc& srcLoc, Offset bclen) {
  LineTable lines;
  if (srcLoc.empty()) {
    return lines;
  }

  auto prev = srcLoc.begin();
  for (auto it = prev + 1; it != srcLoc.end(); ++it) {
    if (prev->second.line1 != it->second.line1) {
      lines.push_back(LineEntry(it->first, prev->second.line1));
      prev = it;
    }
  }

  lines.push_back(LineEntry(bclen, prev->second.line1));
  return lines;
}

}

void FuncEmitter::recordSourceLocation(const Location::Range& sLoc,
                                       Offset start) {
  // Some byte codes, such as for the implicit "return 0" at the end of a
  // a source file do not have valid source locations. This check makes
  // sure we don't record a (dummy) source location in this case.
  if (start > 0 && sLoc.line0 == -1) return;
  SourceLoc newLoc(sLoc);
  if (!m_sourceLocTab.empty()) {
    if (m_sourceLocTab.back().second == newLoc) {
      // Combine into the interval already at the back of the vector.
      assertx(start >= m_sourceLocTab.back().first);
      return;
    }
    assertx(m_sourceLocTab.back().first < start &&
           "source location offsets must be added to UnitEmitter in "
           "increasing order");
  } else {
    // First record added should be for bytecode offset zero or very rarely one
    // when the source starts with a label and a Nop is inserted.
    assertx(start == 0 || start == 1);
  }
  m_sourceLocTab.push_back(std::make_pair(start, newLoc));
}

///////////////////////////////////////////////////////////////////////////////
// Initialization and execution.

void FuncEmitter::init(int l1, int l2, Attr attrs_,
                       const StringData* docComment_) {
  line1 = l1;
  line2 = l2;
  docComment = docComment_;
  attrs = attrs_;

  assertx(!ue().isASystemLib() || attrs & AttrBuiltin);
}

void FuncEmitter::finish() {
  sortEHTab();
}

const StaticString
  s_construct("__construct"),
  s_DynamicallyCallable("__DynamicallyCallable"),
  s_Memoize("__Memoize"),
  s_MemoizeLSB("__MemoizeLSB"),
  s_KeyedByIC("KeyedByIC"),
  s_MakeICInaccessible("MakeICInaccessible"),
  s_SoftMakeICInaccessible("SoftMakeICInaccessible"),
  s_Uncategorized("Uncategorized"),
  s_SoftInternal("__SoftInternal");

namespace {
  bool is_interceptable(const PreClass* preClass, const StringData* name) {
    if (RO::RepoAuthoritative) return false;
    if (RO::NonInterceptableFunctions.empty()) return true;
    auto fullname = [&]() {
      auto n = name->toCppString();
      if (!preClass) {
        return n;
      }
      return preClass->name()->toCppString() +"::"+ n;
    }();
    return RO::NonInterceptableFunctions.count(fullname) == 0;
  }
}

Func* FuncEmitter::create(Unit& unit, PreClass* preClass /* = NULL */) const {
  auto attrs = this->attrs;

  auto persistent = RO::RepoAuthoritative || (ue().isASystemLib() && (!RO::funcIsRenamable(name) || preClass));
  assertx(IMPLIES(attrs & AttrPersistent, persistent));
  attrSetter(attrs, persistent, AttrPersistent);

  auto interceptable = is_interceptable(preClass, name);
  attrSetter(attrs, interceptable, AttrInterceptable);

  if (!(attrs & AttrPersistent) && attrs & AttrBuiltin) {
    assertx(!preClass);
    SystemLib::s_anyNonPersistentBuiltins = true;
  }

  if (isAsync && !isGenerator) {
    // Async functions can return results directly.
    attrs |= AttrSupportsAsyncEagerReturn;
  }
  if (!coeffectRules.empty()) attrs |= AttrHasCoeffectRules;

  if (attrs & AttrInternal &&
      userAttributes.find(s_SoftInternal.get()) != userAttributes.end()) {
    attrs |= AttrInternalSoft;
  }

  if (hasVar(s_86productAttributionData.get())) {
    attrs |= AttrHasAttributionData;
  }

  auto const dynCallSampleRate = [&] () -> Optional<int64_t> {
    if (!(attrs & AttrDynamicallyCallable)) return {};

    auto const uattr = userAttributes.find(s_DynamicallyCallable.get());
    if (uattr == userAttributes.end()) return {};

    auto const tv = uattr->second;
    assertx(isArrayLikeType(type(tv)));
    auto const rate = val(tv).parr->get(int64_t(0));
    if (!isIntType(type(rate)) || val(rate).num < 0) return {};

    attrs = Attr(attrs & ~AttrDynamicallyCallable);
    return val(rate).num;
  }();

  assertx(!m_pce == !preClass);
  auto f = m_ue.newFunc(this, unit, name, attrs, params.size());

  f->m_isPreFunc = !!preClass;

  auto const uait = userAttributes.find(s___Reified.get());
  auto const hasReifiedGenerics = uait != userAttributes.end();

  // Returns (static coeffects, escapes)
  auto const coeffectsInfo = getCoeffectsInfoFromList(
    staticCoeffects,
    preClass && name == s_construct.get());
  f->m_requiredCoeffects = coeffectsInfo.first.toRequired();

  Func::MemoizeICType icType = Func::MemoizeICType::NoIC;
  uint32_t softMakeICInaccessibleSampleRate = 1;
  if (isMemoizeWrapper || isMemoizeWrapperLSB) {
    auto const getICType = [&] (TypedValue tv) {
      assertx(tvIsVec(tv));
      IterateV(tv.m_data.parr, [&](TypedValue elem) {
        if (tvIsString(elem)) {
          assertx(icType == Func::MemoizeICType::NoIC);
          if (elem.m_data.pstr->same(s_KeyedByIC.get())) {
            assertx(tv.m_data.parr->size() == 1);
            icType = Func::MemoizeICType::KeyedByIC;
          } else if (elem.m_data.pstr->same(s_MakeICInaccessible.get())) {
            assertx(tv.m_data.parr->size() == 1);
            icType = Func::MemoizeICType::MakeICInaccessible;
          } else if (elem.m_data.pstr->same(s_SoftMakeICInaccessible.get())) {
            assertx(tv.m_data.parr->size() <= 2);
            icType = Func::MemoizeICType::SoftMakeICInaccessible;
          } else if (elem.m_data.pstr->same(s_Uncategorized.get())) {
            assertx(tv.m_data.parr->size() == 1);
            icType = Func::MemoizeICType::NoIC; // explicitly so
          } else {
            assertx(false && "invalid string");
          }
        } else if (tvIsInt(elem)) {
          assertx(icType == Func::MemoizeICType::SoftMakeICInaccessible);
          assertx(softMakeICInaccessibleSampleRate == 1);
          softMakeICInaccessibleSampleRate =
            std::max(1u, static_cast<uint32_t>(elem.m_data.num));
        } else {
          assertx(false && "invalid input");
        }
      });
    };
    auto const attrName = isMemoizeWrapperLSB ? s_MemoizeLSB : s_Memoize;
    auto const it = userAttributes.find(attrName.get());
    if (it != userAttributes.end()) {
      getICType(it->second);
      assertx((icType & 0x3) == icType);
    }
  }

  bool const needsExtendedSharedData =
    isNative ||
    line2 - line1 >= Func::kSmallDeltaLimit ||
    m_bclen >= Func::kSmallDeltaLimit ||
    m_sn >= Func::kSmallDeltaLimit ||
    hasReifiedGenerics ||
    hasParamsWithMultiUBs ||
    hasReturnWithMultiUBs ||
    dynCallSampleRate ||
    softMakeICInaccessibleSampleRate > 1 ||
    coeffectsInfo.second.value() != 0 ||
    !coeffectRules.empty() ||
    (docComment && !docComment->empty()) ||
    requiresFromOriginalModule;

  f->m_shared.reset(
    needsExtendedSharedData
      ? new Func::ExtendedSharedData(m_bc, m_bclen, preClass, m_sn, line1, line2,
                                     !containsCalls)
      : new Func::SharedData(m_bc, m_bclen, preClass, m_sn, line1, line2,
                             !containsCalls)
  );

  f->init(params.size());

  if (auto const ex = f->extShared()) {
    ex->m_allFlags.m_hasExtendedSharedData = true;
    ex->m_arFuncPtr = nullptr;
    ex->m_nativeFuncPtr = nullptr;
    ex->m_bclen = m_bclen;
    ex->m_sn = m_sn;
    ex->m_line2 = line2;
    ex->m_dynCallSampleRate = dynCallSampleRate.value_or(-1);
    ex->m_softMakeICInaccessibleSampleRate = softMakeICInaccessibleSampleRate;
    ex->m_allFlags.m_returnByValue = false;
    ex->m_allFlags.m_isUntrustedReturnType = false;
    ex->m_allFlags.m_isMemoizeWrapper = false;
    ex->m_allFlags.m_isMemoizeWrapperLSB = false;
    ex->m_allFlags.m_memoizeICType = Func::MemoizeICType::NoIC;

    if (!coeffectRules.empty()) ex->m_coeffectRules = coeffectRules;
    ex->m_coeffectEscapes = coeffectsInfo.second;
    ex->m_docComment = docComment;
    ex->m_originalModuleName =
      originalModuleName ? originalModuleName : LowStringPtr(unit.moduleName());
    assertx(ex->m_originalModuleName);
  }

  std::vector<Func::ParamInfo> fParams;
  for (unsigned i = 0; i < params.size(); ++i) {
    Func::ParamInfo pi = params[i];
    fParams.push_back(pi);
    auto const& fromUBs = params[i].upperBounds;
    if (!fromUBs.isTop()) {
      auto& ub = f->extShared()->m_paramUBs[i];
      ub.m_constraints.resize(fromUBs.m_constraints.size());
      std::copy(fromUBs.m_constraints.begin(), fromUBs.m_constraints.end(), ub.m_constraints.begin());
      f->shared()->m_allFlags.m_hasParamsWithMultiUBs = true;
    }
  }

  auto const originalFullName =
    (!originalFilename ||
     !RuntimeOption::RepoAuthoritative ||
     FileUtil::isAbsolutePath(originalFilename->slice())) ?
    originalFilename :
    makeStaticString(RuntimeOption::SourceRoot +
                     originalFilename->toCppString());

  f->shared()->m_localNames.create(m_localNames);
  f->shared()->m_numLocals = m_numLocals;
  f->shared()->m_numIterators = m_numIterators;
  f->m_maxStackCells = maxStackCells;
  f->shared()->m_ehtab = ehtab;
  f->shared()->m_allFlags.m_isClosureBody = isClosureBody;
  f->shared()->m_allFlags.m_isAsync = isAsync;
  f->shared()->m_allFlags.m_isGenerator = isGenerator;
  f->shared()->m_allFlags.m_isPairGenerator = isPairGenerator;
  f->shared()->m_userAttributes = userAttributes;
  f->shared()->m_retTypeConstraint = retTypeConstraint;
  f->shared()->m_retUserType = retUserType;
  if (!retUpperBounds.isTop()) {
    f->extShared()->m_returnUBs.m_constraints.resize(retUpperBounds.m_constraints.size());
    std::copy(retUpperBounds.m_constraints.begin(), retUpperBounds.m_constraints.end(),
              f->extShared()->m_returnUBs.m_constraints.begin());
    f->shared()->m_allFlags.m_hasReturnWithMultiUBs = true;
  }
  f->shared()->m_originalFilename = originalFullName;
  f->shared()->m_allFlags.m_isGenerated = HPHP::is_generated(name);
  f->shared()->m_repoReturnType = repoReturnType;
  f->shared()->m_repoAwaitedReturnType = repoAwaitedReturnType;
  f->shared()->m_allFlags.m_isMemoizeWrapper = isMemoizeWrapper;
  f->shared()->m_allFlags.m_isMemoizeWrapperLSB = isMemoizeWrapperLSB;
  f->shared()->m_allFlags.m_memoizeICType = icType;
  f->shared()->m_allFlags.m_hasReifiedGenerics = hasReifiedGenerics;

  for (auto const& name : staticCoeffects) {
    f->shared()->m_staticCoeffectNames.push_back(name);
  }

  if (hasReifiedGenerics) {
    auto const tv = uait->second;
    assertx(tvIsVec(tv));
    f->extShared()->m_reifiedGenericsInfo =
      extractSizeAndPosFromReifiedAttribute(tv.m_data.parr);
  }

  /*
   * If we have a m_sourceLocTab, use that to create the line
   * table. Otherwise use the line table we loaded out of the repo, or
   * a token to lazy load it.
   */
  if (m_sourceLocTab.size() != 0) {
    f->setLineTable(createLineTable(m_sourceLocTab, m_bclen));
    // If the debugger is enabled, or we plan to dump hhas we will
    // need the extended line table information in the output, and if
    // we're not writing the repo, stashing it here is necessary for
    // it to make it through.
    if (needs_extended_line_table()) {
      f->stashExtendedLineTable(createSourceLocTable());
    }
  } else if (m_lineTable.isPtr()) {
    f->setLineTable(*m_lineTable.ptr());
  } else {
    assertx(RO::RepoAuthoritative);
    f->setLineTable(m_lineTable.token());
  }

  if (isNative) {
    auto const ex = f->extShared();

    auto const info = getNativeInfo();

    Attr dummy = AttrNone;
    auto nativeAttributes = parseNativeAttributes(dummy);
    Native::getFunctionPointers(
      info,
      nativeAttributes,
      ex->m_arFuncPtr,
      ex->m_nativeFuncPtr
    );

    if (ex->m_nativeFuncPtr) {
      if (UNLIKELY(RO::EvalRecordReplay)) {
        rr::addNativeFuncAttrs(ex->m_nativeFuncPtr, attrs);
      }
      if (info.sig.ret == Native::NativeSig::Type::MixedTV  ||
          info.sig.ret == Native::NativeSig::Type::StringNN ||
          info.sig.ret == Native::NativeSig::Type::ArrayNN  ||
          info.sig.ret == Native::NativeSig::Type::ObjectNN) {
        // the non null ref or mixed TV should be return by value
        ex->m_allFlags.m_returnByValue = true;
      }
      if (info.sig.ret == Native::NativeSig::Type::String ||
          info.sig.ret == Native::NativeSig::Type::Array   ||
          info.sig.ret == Native::NativeSig::Type::Object  ||
          info.sig.ret == Native::NativeSig::Type::Resource ) {
        // these types are nullable
        // set return by value flag
        ex->m_allFlags.m_isUntrustedReturnType = true;
      }

      int extra = isMethod() ? 1 : 0;
      assertx(info.sig.args.size() == params.size() + extra);
      for (auto i = params.size(); i--; ) {
        switch (info.sig.args[extra + i]) {
          case Native::NativeSig::Type::ObjectNN:
          case Native::NativeSig::Type::StringNN:
          case Native::NativeSig::Type::ArrayNN:
          case Native::NativeSig::Type::ResourceArg:
            fParams[i].setFlag(Func::ParamInfo::Flags::NativeArg);
            break;
          case Native::NativeSig::Type::MixedTV:
            fParams[i].setFlag(Func::ParamInfo::Flags::NativeArg);
            fParams[i].setFlag(Func::ParamInfo::Flags::AsTypedValue);
            break;
          case Native::NativeSig::Type::Mixed:
            fParams[i].setFlag(Func::ParamInfo::Flags::AsVariant);
            break;
          default:
            break;
        }
      }
    }
  }

  f->finishedEmittingParams(fParams);

  if (RuntimeOption::EvalEnableReverseDataMap && !preClass) {
    f->registerInDataMap();
  }
  return f;
}

String FuncEmitter::nativeFullname() const {
  return Native::fullName(name, m_pce ? m_pce->name() : nullptr,
                          (attrs & AttrStatic));
}

Native::NativeFunctionInfo FuncEmitter::getNativeInfo() const {
  assertx(m_ue.m_extension);
  return Native::getNativeFunction(
      m_ue.m_extension->nativeFuncs(),
      name,
      m_pce ? m_pce->name() : nullptr,
      (attrs & AttrStatic)
    );
}

///////////////////////////////////////////////////////////////////////////////
// Locals, iterators, and parameters.

void FuncEmitter::allocVarId(const StringData* name, bool slotless) {
  assertx(name != nullptr);
  UNUSED Id id;
  if (m_localNames.find(name) == m_localNames.end()) {
    // Slotless locals must come after all locals with slots.
    if (!slotless) {
      id = (m_numLocals++);
      assertx(id == (int)m_localNames.size());
    }
    m_localNames.add(name, name);
  }
}

Id FuncEmitter::allocIterator() {
  assertx(m_numIterators >= m_nextFreeIterator);
  Id id = m_nextFreeIterator++;
  if (m_numIterators < m_nextFreeIterator) {
    m_numIterators = m_nextFreeIterator;
  }
  return id;
}

Id FuncEmitter::allocUnnamedLocal() {
  m_localNames.addUnnamed(nullptr);
  ++m_numUnnamedLocals;
  return m_numLocals++;
}

///////////////////////////////////////////////////////////////////////////////
// Unit tables.

EHEnt& FuncEmitter::addEHEnt() {
  assertx(!m_ehTabSorted
    || "should only mark the ehtab as sorted after adding all of them");
  ehtab.emplace_back();
  ehtab.back().m_parentIndex = 7777;
  return ehtab.back();
}

namespace {

/*
 * Ordering on EHEnts where e1 < e2 iff
 *
 *    a) e1 and e2 do not overlap, and e1 comes first
 *    b) e1 encloses e2
 */
struct EHEntComp {
  bool operator()(const EHEnt& e1, const EHEnt& e2) const {
    if (e1.m_base == e2.m_base) {
      return e1.m_past > e2.m_past;
    }
    return e1.m_base < e2.m_base;
  }
};

}

void FuncEmitter::sortEHTab() {
  if (m_ehTabSorted) return;

  std::sort(ehtab.begin(), ehtab.end(), EHEntComp());

  for (unsigned int i = 0; i < ehtab.size(); i++) {
    ehtab[i].m_parentIndex = -1;
    for (int j = i - 1; j >= 0; j--) {
      if (ehtab[j].m_past >= ehtab[i].m_past) {
        // parent EHEnt better enclose this one.
        assertx(ehtab[j].m_base <= ehtab[i].m_base);
        ehtab[i].m_parentIndex = j;
        break;
      }
    }
  }

  setEHTabIsSorted();
}

void FuncEmitter::setEHTabIsSorted() {
  m_ehTabSorted = true;
  if (!debug) return;

  Offset curBase = 0;
  for (size_t i = 0; i < ehtab.size(); ++i) {
    auto& eh = ehtab[i];

    // Base offsets must be monotonically increasing.
    always_assert(curBase <= eh.m_base);
    curBase = eh.m_base;

    // Parent should come before, and must enclose this guy.
    always_assert(eh.m_parentIndex == -1 || eh.m_parentIndex < i);
    if (eh.m_parentIndex != -1) {
      auto& parent = ehtab[eh.m_parentIndex];
      always_assert(parent.m_base <= eh.m_base &&
                    parent.m_past >= eh.m_past);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// Bytecode

void FuncEmitter::setBc(const unsigned char* bc, size_t bclen) {
  if (m_bc.isPtr()) {
    if (auto const p = m_bc.ptr()) free(p);
  }
  auto const p = (unsigned char*)malloc(bclen);
  m_bcmax = bclen;
  if (bclen) memcpy(p, bc, bclen);
  m_bc = Func::BCPtr::FromPtr(p);
  m_bclen = bclen;
}

void FuncEmitter::setBcToken(Func::BCPtr::Token token, size_t bclen) {
  if (m_bc.isPtr()) {
    if (auto const p = m_bc.ptr()) free(p);
  }
  m_bcmax = bclen;
  m_bclen = bclen;
  m_bc = Func::BCPtr::FromToken(token);
}

Optional<Func::BCPtr::Token> FuncEmitter::loadBc() {
  if (m_bc.isPtr()) return std::nullopt;
  assertx(RO::RepoAuthoritative);
  auto const old = m_bc.token();
  auto bc = (unsigned char*)malloc(m_bclen);
  RepoFile::readRawFromUnit(m_ue.m_sn, old, bc, m_bclen);
  m_bc = Func::BCPtr::FromPtr(bc);
  return old;
}

///////////////////////////////////////////////////////////////////////////////
// Complex setters.

/* <<__Native>> user attribute causes systemlib declarations
 * to hook internal (C++) implementation of funcs/methods
 *
 * The Native attribute may have the following sub-options
 *  "NoFCallBuiltin": Prevent FCallBuiltin optimization
 *      Effectively forces functions to generate an ActRec
 *  "NoInjection": Do not include this frame in backtraces
 *  "NoRecording": Do not include calls to this when recording
 *
 *  e.g.   <<__Native("NoFCallBuiltin")>> function foo():mixed;
 */
static const StaticString
  s_native("__Native"),
  s_nofcallbuiltin("NoFCallBuiltin"),
  s_noinjection("NoInjection"),
  s_norecording("NoRecording"),
  s_opcodeimpl("OpCodeImpl");

int FuncEmitter::parseNativeAttributes(Attr& attrs_) const {
  int ret = Native::AttrNone;

  auto it = userAttributes.find(s_native.get());
  assertx(it != userAttributes.end());
  const TypedValue userAttr = it->second;
  assertx(isArrayLikeType(userAttr.m_type));
  for (ArrayIter it(userAttr.m_data.parr); it; ++it) {
    Variant userAttrVal = it.second();
    if (userAttrVal.isString()) {
      String userAttrStrVal = userAttrVal.toString();
      if (userAttrStrVal.get()->tsame(s_nofcallbuiltin.get())) {
        attrs_ |= AttrNoFCallBuiltin;
      } else if (userAttrStrVal.get()->tsame(s_noinjection.get())) {
        attrs_ |= AttrNoInjection;
      } else if (userAttrStrVal.get()->tsame(s_norecording.get())) {
        attrs_ |= AttrNoRecording;
      } else if (userAttrStrVal.get()->tsame(s_opcodeimpl.get())) {
        ret |= Native::AttrOpCodeImpl;
      }
    }
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// Serialization/Deserialization

template<class SerDe>
void FuncEmitter::serdeMetaData(SerDe& sd) {
  // NOTE: name and a few other fields currently handled outside of this.

  sd(line1)
    (line2)
    (attrs)
    (m_bclen)
    (staticCoeffects)
    (repoReturnType)
    (repoAwaitedReturnType)
    (docComment)
    (m_numLocals)
    (m_numIterators)
    (maxStackCells)
    (m_repoBoolBitset)

    (params)
    (m_localNames, [](auto s) { return s; })
    .delta(
      ehtab,
      [&](const EHEnt& prev, EHEnt cur) -> EHEnt {
        if constexpr (!SerDe::deserializing) {
          cur.m_handler -= cur.m_past;
          cur.m_past -= cur.m_base;
          cur.m_base -= prev.m_base;
        } else {
          cur.m_base += prev.m_base;
          cur.m_past += cur.m_base;
          cur.m_handler += cur.m_past;
        }
        return cur;
      }
    )
    (userAttributes)
    (retTypeConstraint)
    (retUserType)
    (retUpperBounds)
    (originalFilename)
    (originalModuleName)
    (coeffectRules)
    ;
}

template void FuncEmitter::serdeMetaData<>(BlobDecoder&);
template void FuncEmitter::serdeMetaData<>(BlobEncoder&);

template<class SerDe>
void FuncEmitter::serde(SerDe& sd, bool lazy) {
  assertx(IMPLIES(lazy, RO::RepoAuthoritative));
  assertx(IMPLIES(!SerDe::deserializing, !lazy));

  serdeMetaData(sd);

  // Never lazy load any builtins (this avoids issues with HHBBC
  // trying to load data after we've shutdown the repo).
  if (attrs & AttrBuiltin) lazy = false;

  if constexpr (SerDe::deserializing) {
    if (lazy) {
      m_lineTable = Func::LineTablePtr::FromToken(sd.advanced());
      sd.skipWithSize();
    } else {
      LineTable lineTable;
      deserializeLineTable(sd, lineTable);
      setLineTable(std::move(lineTable));
    }
  } else {
    auto const& lines = m_sourceLocTab.empty()
      ? *m_lineTable.ptr()
      : createLineTable(m_sourceLocTab, m_bclen);
    sd.withSize(
      [&] {
        sd.delta(
          lines,
          [&] (const LineEntry& prev, const LineEntry& curDelta) {
            return LineEntry {
              curDelta.pastOffset() - prev.pastOffset(),
              curDelta.val() - prev.val()
            };
          }
        );
      }
    );
  }

  if constexpr (SerDe::deserializing) {
    sd(m_sourceLocTab);
  } else {
    sd(RO::RepoDebugInfo ? m_sourceLocTab : decltype(m_sourceLocTab){});
  }

  // Bytecode
  if constexpr (SerDe::deserializing) {
    assertx(sd.remaining() >= m_bclen);
    if (lazy) {
      setBcToken(sd.advanced(), m_bclen);
    } else {
      setBc(sd.data(), m_bclen);
    }
    sd.advance(m_bclen);
  } else {
    sd.writeRaw((const char*)m_bc.ptr(), m_bclen);
  }
}

template void FuncEmitter::serde<>(BlobDecoder&, bool);
template void FuncEmitter::serde<>(BlobEncoder&, bool);

void FuncEmitter::deserializeLineTable(BlobDecoder& decoder,
                                       LineTable& lineTable) {
  decoder.withSize(
    [&] {
      decoder.delta(
        lineTable,
        [&] (const LineEntry& prev, const LineEntry& curDelta) {
          return LineEntry {
            curDelta.pastOffset() + prev.pastOffset(),
            curDelta.val() + prev.val()
          };
        }
      );
    }
  );
}

LineTable FuncEmitter::loadLineTableFromRepo(int64_t unitSn,
                                             RepoFile::Token token) {
  assertx(RO::RepoAuthoritative);

  auto const remaining = RepoFile::remainingSizeOfUnit(unitSn, token);
  always_assert(remaining >= sizeof(uint64_t));

  size_t actualSize;
  {
    // We encoded the size of the line table along with the table. So,
    // peek its size and bail if the decoder doesn't have enough data
    // remaining.
    auto const size = std::min<size_t>(remaining, 128);
    auto const data = std::make_unique<unsigned char[]>(size);
    RepoFile::readRawFromUnit(unitSn, token, data.get(), size);
    BlobDecoder decoder{data.get(), size};
    actualSize = decoder.peekSize();
    if (actualSize <= decoder.remaining()) {
      LineTable lineTable;
      deserializeLineTable(decoder, lineTable);
      return lineTable;
    }
  }

  constexpr const size_t kLineTableSizeLimit = 1ull << 32;
  always_assert(actualSize <= kLineTableSizeLimit);
  always_assert(actualSize <= remaining);

  LineTable lineTable;
  auto const data = std::make_unique<unsigned char[]>(actualSize);
  RepoFile::readRawFromUnit(unitSn, token, data.get(), actualSize);
  BlobDecoder decoder{data.get(), actualSize};
  deserializeLineTable(decoder, lineTable);
  decoder.assertDone();
  return lineTable;
}

///////////////////////////////////////////////////////////////////////////////
}
