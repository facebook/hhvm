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
#include "hphp/runtime/base/rds.h"

#include "hphp/runtime/ext/extension.h"

#include "hphp/runtime/vm/blob-helper.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/native.h"
#include "hphp/runtime/vm/reified-generics.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/repo-autoload-map-builder.h"
#include "hphp/runtime/vm/repo-file.h"
#include "hphp/runtime/vm/runtime.h"

#include "hphp/runtime/vm/jit/types.h"

#include "hphp/runtime/vm/verifier/cfg.h"

#include "hphp/system/systemlib.h"

#include "hphp/util/atomic-vector.h"
#include "hphp/util/file.h"
#include "hphp/util/trace.h"

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
  , hniReturnType(folly::none)
  , retUserType(nullptr)
  , docComment(nullptr)
  , originalFilename(nullptr)
  , memoizePropName(nullptr)
  , memoizeGuardPropName(nullptr)
  , memoizeSharedPropIndex(0)
  , m_numLocals(0)
  , m_numUnnamedLocals(0)
  , m_numIterators(0)
  , m_numClosures(0)
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
  , hniReturnType(folly::none)
  , retUserType(nullptr)
  , docComment(nullptr)
  , originalFilename(nullptr)
  , memoizePropName(nullptr)
  , memoizeGuardPropName(nullptr)
  , memoizeSharedPropIndex(0)
  , m_numLocals(0)
  , m_numUnnamedLocals(0)
  , m_numIterators(0)
  , m_numClosures(0)
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

void FuncEmitter::setSourceLocTable(const SourceLocTable& table) {
  m_sourceLocTab.clear();
  for (auto const& e : table) {
    m_sourceLocTab.emplace_back(e.pastOffset(), e.val());
  }
  for (size_t i = m_sourceLocTab.size(); i > 1; --i) {
    m_sourceLocTab[i - 1].first = m_sourceLocTab[i - 2].first;
  }
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
  attrs = fix_attrs(attrs_);
  docComment = docComment_;

  if (!SystemLib::s_inited) assertx(attrs & AttrBuiltin);
}

void FuncEmitter::finish() {
  sortEHTab();
}

void FuncEmitter::commit(RepoTxn& txn) {
  Repo& repo = Repo::get();
  FuncRepoProxy& frp = repo.frp();
  int repoId = m_ue.m_repoId;
  int64_t usn = m_ue.m_sn;

  if (!m_sourceLocTab.empty()) {
    setLineTable(createLineTable(m_sourceLocTab, m_bclen));
  }

  frp.insertFunc[repoId].insert(
    *this, txn, usn, m_sn, m_pce ? m_pce->id() : -1, name, m_bc.ptr(), m_bclen
  );

  if (RuntimeOption::RepoDebugInfo) {
    for (size_t i = 0; i < m_sourceLocTab.size(); ++i) {
      SourceLoc& e = m_sourceLocTab[i].second;
      Offset endOff = i < m_sourceLocTab.size() - 1
                          ? m_sourceLocTab[i + 1].first
                          : m_bclen;

      frp.insertFuncSourceLoc[repoId]
          .insert(txn, usn, m_sn, endOff, e.line0, e.char0, e.line1, e.char1);
    }
  }
}

const StaticString
  s_construct("__construct"),
  s_DynamicallyCallable("__DynamicallyCallable");

Func* FuncEmitter::create(Unit& unit, PreClass* preClass /* = NULL */) const {
  bool isGenerated = isdigit(name->data()[0]);

  auto attrs = fix_attrs(this->attrs);
  if (preClass && preClass->attrs() & AttrInterface) {
    attrs |= AttrAbstract;
  }
  if (attrs & AttrIsMethCaller && RuntimeOption::RepoAuthoritative) {
    attrs |= AttrPersistent | AttrUnique;
  }
  if (attrs & (AttrPersistent | AttrUnique) && !preClass) {
    if ((RuntimeOption::EvalJitEnableRenameFunction ||
         attrs & AttrInterceptable ||
         (!RuntimeOption::RepoAuthoritative && SystemLib::s_inited))) {
      if (attrs & AttrBuiltin) {
        SystemLib::s_anyNonPersistentBuiltins = true;
      }
      attrs = Attr(attrs & ~(AttrPersistent | AttrUnique));
    }
  } else {
    assertx(preClass || !(attrs & AttrBuiltin) || (attrs & AttrIsMethCaller));
  }
  if (isVariadic()) {
    attrs |= AttrVariadicParam;
  }
  if (isAsync && !isGenerator) {
    // Async functions can return results directly.
    attrs |= AttrSupportsAsyncEagerReturn;
  }

  if (!coeffectRules.empty()) attrs |= AttrHasCoeffectRules;

  auto const dynCallSampleRate = [&] () -> folly::Optional<int64_t> {
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

  auto const coeffects = [&] {
    if (staticCoeffects.empty()) return StaticCoeffects::defaults();
    auto coeffects =
      CoeffectsConfig::fromName(staticCoeffects[0]->toCppString());
    for (auto const& name : staticCoeffects) {
      coeffects &= CoeffectsConfig::fromName(name->toCppString());
    }
    if (preClass && name == s_construct.get()) {
      coeffects &= StaticCoeffects::write_this_props();
    }
    return coeffects;
  }();
  auto const shallowCoeffectsWithLocals = coeffects.toShallowWithLocals();
  f->m_requiredCoeffects = coeffects.toRequired();

  bool const needsExtendedSharedData =
    isNative ||
    params.size() > 64 ||
    line2 - line1 >= Func::kSmallDeltaLimit ||
    m_bclen >= Func::kSmallDeltaLimit ||
    m_sn >= Func::kSmallDeltaLimit ||
    hasReifiedGenerics ||
    hasParamsWithMultiUBs ||
    hasReturnWithMultiUBs ||
    dynCallSampleRate ||
    shallowCoeffectsWithLocals.value() != 0 ||
    !coeffectRules.empty() ||
    (docComment && !docComment->empty());

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
    ex->m_allFlags.m_returnByValue = false;
    ex->m_allFlags.m_isMemoizeWrapper = false;
    ex->m_allFlags.m_isMemoizeWrapperLSB = false;

    if (!coeffectRules.empty()) ex->m_coeffectRules = coeffectRules;
    ex->m_shallowCoeffectsWithLocals = shallowCoeffectsWithLocals;
    ex->m_docComment = docComment;
  }

  std::vector<Func::ParamInfo> fParams;
  for (unsigned i = 0; i < params.size(); ++i) {
    Func::ParamInfo pi = params[i];
    if (pi.isVariadic()) {
      pi.builtinType = KindOfVec;
    }
    f->appendParam(params[i].isInOut(), pi, fParams);
    auto const& fromUBs = params[i].upperBounds;
    if (!fromUBs.empty()) {
      auto& ub = f->extShared()->m_paramUBs[i];
      ub.resize(fromUBs.size());
      std::copy(fromUBs.begin(), fromUBs.end(), ub.begin());
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
  if (!retUpperBounds.empty()) {
    f->extShared()->m_returnUBs.resize(retUpperBounds.size());
    std::copy(retUpperBounds.begin(), retUpperBounds.end(),
              f->extShared()->m_returnUBs.begin());
    f->shared()->m_allFlags.m_hasReturnWithMultiUBs = true;
  }
  f->shared()->m_originalFilename = originalFullName;
  f->shared()->m_allFlags.m_isGenerated = isGenerated;
  f->shared()->m_repoReturnType = repoReturnType;
  f->shared()->m_repoAwaitedReturnType = repoAwaitedReturnType;
  f->shared()->m_allFlags.m_isMemoizeWrapper = isMemoizeWrapper;
  f->shared()->m_allFlags.m_isMemoizeWrapperLSB = isMemoizeWrapperLSB;
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

    ex->m_hniReturnType = hniReturnType;

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
      if (info.sig.ret == Native::NativeSig::Type::MixedTV) {
        ex->m_allFlags.m_returnByValue = true;
      }
      int extra = isMethod() ? 1 : 0;
      assertx(info.sig.args.size() == params.size() + extra);
      for (auto i = params.size(); i--; ) {
        switch (info.sig.args[extra + i]) {
          case Native::NativeSig::Type::ObjectArg:
          case Native::NativeSig::Type::StringArg:
          case Native::NativeSig::Type::ArrayArg:
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
  return Native::getNativeFunction(
      m_ue.m_nativeFuncs,
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

folly::Optional<Func::BCPtr::Token> FuncEmitter::loadBc() {
  if (m_bc.isPtr()) return folly::none;
  assertx(RO::RepoAuthoritative);
  auto const old = m_bc.token();
  auto bc = (unsigned char*)malloc(m_bclen);
  RepoFile::loadBytecode(m_ue.m_sn, old, bc, m_bclen);
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
 *
 *  e.g.   <<__Native("NoFCallBuiltin")>> function foo():mixed;
 */
static const StaticString
  s_native("__Native"),
  s_nofcallbuiltin("NoFCallBuiltin"),
  s_noinjection("NoInjection"),
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
      if (userAttrStrVal.get()->isame(s_nofcallbuiltin.get())) {
        attrs_ |= AttrNoFCallBuiltin;
      } else if (userAttrStrVal.get()->isame(s_noinjection.get())) {
        attrs_ |= AttrNoInjection;
      } else if (userAttrStrVal.get()->isame(s_opcodeimpl.get())) {
        ret |= Native::AttrOpCodeImpl;
      }
    }
  }
  return ret;
}

Attr FuncEmitter::fix_attrs(Attr a) const {
  if (RuntimeOption::RepoAuthoritative) return a;

  a = Attr(a & ~AttrInterceptable);

  if (RuntimeOption::EvalJitEnableRenameFunction) {
    return a | AttrInterceptable;
  }
  return a;
}

///////////////////////////////////////////////////////////////////////////////
// Serialization/Deserialization

template<class SerDe>
void FuncEmitter::serdeMetaData(SerDe& sd) {
  // NOTE: name and a few other fields currently handled outside of this.
  Attr a = attrs;

  if (!SerDe::deserializing) {
    a = fix_attrs(attrs);
  }

  sd(line1)
    (line2)
    (a)
    (m_bclen)
    (staticCoeffects)
    (hniReturnType)
    (repoReturnType)
    (repoAwaitedReturnType)
    (docComment)
    (m_numLocals)
    (m_numIterators)
    (m_numClosures)
    (maxStackCells)
    (m_repoBoolBitset)

    (params)
    (m_localNames, [](auto s) { return s; })
    (ehtab,
      [&](const EHEnt& prev, EHEnt cur) -> EHEnt {
        if (!SerDe::deserializing) {
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
    (coeffectRules)
    ;

  if (SerDe::deserializing) {
    repoReturnType.resolveArray(ue());
    repoAwaitedReturnType.resolveArray(ue());
    attrs = fix_attrs(a);
  }
}

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
        sd(
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
      decoder(
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

size_t FuncEmitter::optDeserializeLineTable(BlobDecoder& decoder,
                                            LineTable& lineTable) {
  // We encoded the size of the line table along with the table. So,
  // peek its size and bail if the decoder doesn't have enough data
  // remaining.
  auto const size = decoder.peekSize();
  if (size <= decoder.remaining()) deserializeLineTable(decoder, lineTable);
  return size;
}

///////////////////////////////////////////////////////////////////////////////
// FuncRepoProxy.

FuncRepoProxy::FuncRepoProxy(Repo& repo)
    : RepoProxy(repo),
      insertFunc{InsertFuncStmt(repo, 0), InsertFuncStmt(repo, 1)},
      getFuncs{GetFuncsStmt(repo, 0), GetFuncsStmt(repo, 1)},
      insertFuncSourceLoc{InsertFuncSourceLocStmt(repo, 0), InsertFuncSourceLocStmt(repo, 1)},
      getSourceLocTab{GetSourceLocTabStmt(repo, 0), GetSourceLocTabStmt(repo, 1)}
{}

FuncRepoProxy::~FuncRepoProxy() {
}

void FuncRepoProxy::createSchema(int repoId, RepoTxn& txn) {
  {
    auto createQuery = folly::sformat(
      "CREATE TABLE {} "
      "(unitSn INTEGER, funcSn INTEGER, preClassId INTEGER, name TEXT, "
      " bc BLOB, extraData BLOB, lineTable BLOB, PRIMARY KEY (unitSn, funcSn));",
      m_repo.table(repoId, "Func"));
    txn.exec(createQuery);
  }
  {
    auto createQuery = folly::sformat(
      "CREATE TABLE {} "
      "(unitSn INTEGER, funcSn INTEGER, pastOffset INTEGER, line0 INTEGER,"
      " char0 INTEGER, line1 INTEGER, char1 INTEGER,"
      " PRIMARY KEY (unitSn, funcSn, pastOffset));",
      m_repo.table(repoId, "FuncSourceLoc"));
    txn.exec(createQuery);
  }
}

void FuncRepoProxy::InsertFuncStmt
                  ::insert(const FuncEmitter& fe,
                           RepoTxn& txn, int64_t unitSn, int funcSn,
                           Id preClassId, const StringData* name,
                           const unsigned char* bc, size_t bclen) {
  if (!prepared()) {
    auto insertQuery = folly::sformat(
      "INSERT INTO {} "
      "VALUES(@unitSn, @funcSn, @preClassId, @name, @bc, @extraData, @lineTable);",
      m_repo.table(m_repoId, "Func"));
    txn.prepare(*this, insertQuery);
  }

  BlobEncoder extraBlob{fe.useGlobalIds()};
  BlobEncoder lineTableBlob{false};

  RepoTxnQuery query(txn, *this);
  query.bindInt64("@unitSn", unitSn);
  query.bindInt("@funcSn", funcSn);
  query.bindId("@preClassId", preClassId);
  query.bindStaticString("@name", name);
  query.bindBlob("@bc", (const void*)bc, bclen);
  const_cast<FuncEmitter&>(fe).serdeMetaData(extraBlob);
  query.bindBlob("@extraData", extraBlob, /* static */ true);

  lineTableBlob(
    const_cast<LineTable&>(fe.lineTable()),
    [&](const LineEntry& prev, const LineEntry& cur) -> LineEntry {
      return LineEntry {
        cur.pastOffset() - prev.pastOffset(),
        cur.val() - prev.val()
      };
    }
  );
  query.bindBlob("@lineTable", lineTableBlob, /* static */ true);

  query.exec();
}

void FuncRepoProxy::GetFuncsStmt
                  ::get(UnitEmitter& ue) {
  auto txn = RepoTxn{m_repo.begin()};
  if (!prepared()) {
    auto selectQuery = folly::sformat(
      "SELECT funcSn, preClassId, name, bc, extraData, lineTable "
      "FROM {} "
      "WHERE unitSn == @unitSn ORDER BY funcSn ASC;",
      m_repo.table(m_repoId, "Func"));
    txn.prepare(*this, selectQuery);
  }
  RepoTxnQuery query(txn, *this);
  query.bindInt64("@unitSn", ue.m_sn);
  do {
    query.step();
    if (query.row()) {
      int funcSn;               /**/ query.getInt(0, funcSn);
      Id preClassId;            /**/ query.getId(1, preClassId);
      StringData* name;         /**/ query.getStaticString(2, name);
      const void* bc; size_t bclen; /**/ query.getBlob(3, bc, bclen);
      BlobDecoder extraBlob =   /**/ query.getBlob(4, ue.useGlobalIds());

      FuncEmitter* fe;
      if (preClassId < 0) {
        fe = ue.newFuncEmitter(name);
      } else {
        PreClassEmitter* pce = ue.pce(preClassId);
        fe = ue.newMethodEmitter(name, pce);
        bool added UNUSED = pce->addMethod(fe);
        assertx(added);
      }
      assertx(fe->sn() == funcSn);
      fe->setBc(static_cast<const unsigned char*>(bc), bclen);
      fe->serdeMetaData(extraBlob);
      if (!SystemLib::s_inited) {
        assertx(fe->attrs & AttrBuiltin);
        if (preClassId < 0) {
          assertx(fe->attrs & AttrPersistent);
          assertx(fe->attrs & AttrUnique);
        }
      }

      BlobDecoder lineTableBlob = query.getBlob(5, false);
      LineTable lineTable;
      lineTableBlob(
        lineTable,
        [&](const LineEntry& prev, const LineEntry& delta) -> LineEntry {
          return LineEntry {
            delta.pastOffset() + prev.pastOffset(),
            delta.val() + prev.val()
          };
        }
      );
      fe->setLineTable(std::move(lineTable));

      fe->setEHTabIsSorted();
      fe->finish();
    }
  } while (!query.done());
  txn.commit();
}

void FuncRepoProxy::InsertFuncSourceLocStmt
                  ::insert(RepoTxn& txn, int64_t unitSn, int64_t funcSn, Offset pastOffset,
                           int line0, int char0, int line1, int char1) {
  if (!prepared()) {
    auto insertQuery = folly::sformat(
      "INSERT INTO {} "
      "VALUES(@unitSn, @funcSn, @pastOffset, @line0, @char0, @line1, @char1);",
      m_repo.table(m_repoId, "FuncSourceLoc"));
    txn.prepare(*this, insertQuery);
  }
  RepoTxnQuery query(txn, *this);
  query.bindInt64("@unitSn", unitSn);
  query.bindInt64("@funcSn", funcSn);
  query.bindOffset("@pastOffset", pastOffset);
  query.bindInt("@line0", line0);
  query.bindInt("@char0", char0);
  query.bindInt("@line1", line1);
  query.bindInt("@char1", char1);
  query.exec();
}

RepoStatus
FuncRepoProxy::GetSourceLocTabStmt::get(int64_t unitSn, int64_t funcSn,
                                        SourceLocTable& sourceLocTab) {
  try {
    auto txn = RepoTxn{m_repo.begin()};
    if (!prepared()) {
      auto selectQuery = folly::sformat(
        "SELECT pastOffset, line0, char0, line1, char1 "
        "FROM {} "
        "WHERE unitSn == @unitSn AND funcSn = @funcSn "
        "ORDER BY pastOffset ASC;",
        m_repo.table(m_repoId, "FuncSourceLoc"));
      txn.prepare(*this, selectQuery);
    }
    RepoTxnQuery query(txn, *this);
    query.bindInt64("@unitSn", unitSn);
    query.bindInt64("@funcSn", funcSn);
    do {
      query.step();
      if (!query.row()) {
        return RepoStatus::error;
      }
      Offset pastOffset;
      query.getOffset(0, pastOffset);
      SourceLoc sLoc;
      query.getInt(1, sLoc.line0);
      query.getInt(2, sLoc.char0);
      query.getInt(3, sLoc.line1);
      query.getInt(4, sLoc.char1);
      SourceLocEntry entry(pastOffset, sLoc);
      sourceLocTab.push_back(entry);
    } while (!query.done());
    txn.commit();
  } catch (RepoExc& re) {
    return RepoStatus::error;
  }
  return RepoStatus::success;
}

///////////////////////////////////////////////////////////////////////////////
}
