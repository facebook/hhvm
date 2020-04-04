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
#include "hphp/runtime/base/unit-cache.h"
#include "hphp/runtime/base/rds.h"

#include "hphp/runtime/ext/extension.h"

#include "hphp/runtime/vm/blob-helper.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/native.h"
#include "hphp/runtime/vm/reified-generics.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/repo-autoload-map-builder.h"
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
  , name(n)
  , top(false)
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
  , m_nextFreeIterator(0)
  , m_ehTabSorted(false)
{}

FuncEmitter::FuncEmitter(UnitEmitter& ue, int sn, const StringData* n,
                         PreClassEmitter* pce)
  : m_ue(ue)
  , m_pce(pce)
  , m_sn(sn)
  , m_id(kInvalidId)
  , name(n)
  , top(false)
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
  , m_nextFreeIterator(0)
  , m_ehTabSorted(false)
{}

FuncEmitter::~FuncEmitter() {
}


///////////////////////////////////////////////////////////////////////////////
// Initialization and execution.

void FuncEmitter::init(int l1, int l2, Offset base_, Attr attrs_, bool top_,
                       const StringData* docComment_) {
  base = base_;
  line1 = l1;
  line2 = l2;
  top = top_;
  attrs = fix_attrs(attrs_);
  docComment = docComment_;

  if (!isPseudoMain()) {
    if (!SystemLib::s_inited) {
      assertx(attrs & AttrBuiltin);
    }
  }
}

void FuncEmitter::finish(Offset past_) {
  past = past_;
  sortEHTab();
}

void FuncEmitter::commit(RepoTxn& txn) const {
  Repo& repo = Repo::get();
  FuncRepoProxy& frp = repo.frp();
  int repoId = m_ue.m_repoId;
  int64_t usn = m_ue.m_sn;

  frp.insertFunc[repoId]
     .insert(*this, txn, usn, m_sn, m_pce ? m_pce->id() : -1, name, top);
}

Func* FuncEmitter::create(Unit& unit, PreClass* preClass /* = NULL */) const {
  bool isGenerated = isdigit(name->data()[0]);

  auto attrs = fix_attrs(this->attrs);
  if (preClass && preClass->attrs() & AttrInterface) {
    attrs |= AttrAbstract;
  }
  if (attrs & AttrIsMethCaller && RuntimeOption::RepoAuthoritative) {
    attrs |= AttrPersistent | AttrUnique;
  }
  if (attrs & AttrPersistent && !preClass) {
    if ((RuntimeOption::EvalJitEnableRenameFunction ||
         attrs & AttrInterceptable ||
         (!RuntimeOption::RepoAuthoritative && SystemLib::s_inited))) {
      if (attrs & AttrBuiltin) {
        SystemLib::s_anyNonPersistentBuiltins = true;
      }
      attrs = Attr(attrs & ~AttrPersistent);
    }
  } else {
    assertx(preClass || !(attrs & AttrBuiltin) || (attrs & AttrIsMethCaller));
  }
  if (!RuntimeOption::RepoAuthoritative) {
    // In non-RepoAuthoritative mode, any function could get a VarEnv because
    // of evalPHPDebugger.
    attrs |= AttrMayUseVV;
  } else if ((attrs & AttrInterceptable) &&
             !name->empty() &&
             !Func::isSpecial(name) &&
             !isClosureBody) {
    // intercepted functions need to pass all args through
    // to the interceptee
    attrs |= AttrMayUseVV;
  }
  if (isVariadic()) {
    attrs |= AttrVariadicParam;
  }
  if (isAsync && !isGenerator) {
    // Async functions can return results directly.
    attrs |= AttrSupportsAsyncEagerReturn;
  }

  assertx(!m_pce == !preClass);
  auto f = m_ue.newFunc(this, unit, name, attrs, params.size());

  f->m_isPreFunc = !!preClass;

  auto const uait = userAttributes.find(s___Reified.get());
  auto const hasReifiedGenerics = uait != userAttributes.end();

  bool const needsExtendedSharedData =
    isNative ||
    line2 - line1 >= Func::kSmallDeltaLimit ||
    past - base >= Func::kSmallDeltaLimit ||
    hasReifiedGenerics ||
    hasParamsWithMultiUBs ||
    hasReturnWithMultiUBs;

  f->m_shared.reset(
    needsExtendedSharedData
      ? new Func::ExtendedSharedData(preClass, base, past, line1, line2,
                                     top, !containsCalls, docComment)
      : new Func::SharedData(preClass, base, past,
                             line1, line2, top, !containsCalls, docComment)
  );

  f->init(params.size());

  if (auto const ex = f->extShared()) {
    ex->m_hasExtendedSharedData = true;
    ex->m_arFuncPtr = nullptr;
    ex->m_nativeFuncPtr = nullptr;
    ex->m_line2 = line2;
    ex->m_past = past;
    ex->m_returnByValue = false;
    ex->m_isMemoizeWrapper = false;
    ex->m_isMemoizeWrapperLSB = false;
  }

  std::vector<Func::ParamInfo> fParams;
  for (unsigned i = 0; i < params.size(); ++i) {
    Func::ParamInfo pi = params[i];
    if (pi.isVariadic()) {
      pi.builtinType = RuntimeOption::EvalHackArrDVArrs
        ? KindOfVec : KindOfArray;
    }
    f->appendParam(params[i].isInOut(), pi, fParams);
    auto const& fromUBs = params[i].upperBounds;
    if (!fromUBs.empty()) {
      auto& ub = f->extShared()->m_paramUBs[i];
      ub.resize(fromUBs.size());
      std::copy(fromUBs.begin(), fromUBs.end(), ub.begin());
      f->shared()->m_hasParamsWithMultiUBs = true;
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
  f->shared()->m_isClosureBody = isClosureBody;
  f->shared()->m_isAsync = isAsync;
  f->shared()->m_isGenerator = isGenerator;
  f->shared()->m_isPairGenerator = isPairGenerator;
  f->shared()->m_userAttributes = userAttributes;
  f->shared()->m_retTypeConstraint = retTypeConstraint;
  f->shared()->m_retUserType = retUserType;
  if (!retUpperBounds.empty()) {
    f->extShared()->m_returnUBs.resize(retUpperBounds.size());
    std::copy(retUpperBounds.begin(), retUpperBounds.end(),
              f->extShared()->m_returnUBs.begin());
    f->shared()->m_hasReturnWithMultiUBs = true;
  }
  f->shared()->m_originalFilename = originalFullName;
  f->shared()->m_isGenerated = isGenerated;
  f->shared()->m_repoReturnType = repoReturnType;
  f->shared()->m_repoAwaitedReturnType = repoAwaitedReturnType;
  f->shared()->m_isMemoizeWrapper = isMemoizeWrapper;
  f->shared()->m_isMemoizeWrapperLSB = isMemoizeWrapperLSB;
  f->shared()->m_hasReifiedGenerics = hasReifiedGenerics;
  f->shared()->m_isRxDisabled = isRxDisabled;

  if (hasReifiedGenerics) {
    auto tv = uait->second;
    assertx(tvIsVecOrVArray(tv));
    f->extShared()->m_reifiedGenericsInfo =
      extractSizeAndPosFromReifiedAttribute(tv.m_data.parr);
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
        ex->m_returnByValue = true;
      }
      int extra = isMethod() ? 1 : 0;
      assertx(info.sig.args.size() == params.size() + extra);
      for (auto i = params.size(); i--; ) {
        switch (info.sig.args[extra + i]) {
          case Native::NativeSig::Type::ObjectArg:
          case Native::NativeSig::Type::StringArg:
          case Native::NativeSig::Type::ArrayArg:
          case Native::NativeSig::Type::ResourceArg:
          case Native::NativeSig::Type::MixedTV:
            fParams[i].setFlag(Func::ParamInfo::Flags::NativeArg);
            break;
          default:
            break;
        }
      }
    }
  }

  f->finishedEmittingParams(fParams);
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

  if (!RuntimeOption::DynamicInvokeFunctions.empty()) {
    auto const fullName = [&] {
      if (m_pce) return folly::sformat("{}::{}", m_pce->name(), name);
      return name->toCppString();
    }();
    if (RuntimeOption::DynamicInvokeFunctions.count(fullName)) {
      return a | AttrInterceptable;
    }
  }
  return a;
}

///////////////////////////////////////////////////////////////////////////////
// Serialization/Deserialization

template<class SerDe>
void FuncEmitter::serdeMetaData(SerDe& sd) {
  // NOTE: name, top, and a few other fields currently handled outside of this.
  Offset past_delta;
  Attr a = attrs;

  if (!SerDe::deserializing) {
    past_delta = past - base;
    a = fix_attrs(attrs);
  }

  sd(line1)
    (line2)
    (base)
    (past_delta)
    (a)
    (hniReturnType)
    (repoReturnType)
    (repoAwaitedReturnType)
    (docComment)
    (m_numLocals)
    (m_numIterators)
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
    ;

  if (SerDe::deserializing) {
    repoReturnType.resolveArray(ue());
    repoAwaitedReturnType.resolveArray(ue());
    past = base + past_delta;
    attrs = fix_attrs(a);
  }
}

///////////////////////////////////////////////////////////////////////////////
// FuncRepoProxy.

FuncRepoProxy::FuncRepoProxy(Repo& repo)
    : RepoProxy(repo),
      insertFunc{InsertFuncStmt(repo, 0), InsertFuncStmt(repo, 1)},
      getFuncs{GetFuncsStmt(repo, 0), GetFuncsStmt(repo, 1)}
{}

FuncRepoProxy::~FuncRepoProxy() {
}

void FuncRepoProxy::createSchema(int repoId, RepoTxn& txn) {
  auto createQuery = folly::sformat(
    "CREATE TABLE {} "
    "(unitSn INTEGER, funcSn INTEGER, preClassId INTEGER, name TEXT, "
    " top INTEGER, extraData BLOB, PRIMARY KEY (unitSn, funcSn));",
    m_repo.table(repoId, "Func"));
  txn.exec(createQuery);
}

void FuncRepoProxy::InsertFuncStmt
                  ::insert(const FuncEmitter& fe,
                           RepoTxn& txn, int64_t unitSn, int funcSn,
                           Id preClassId, const StringData* name,
                           bool top) {
  if (!prepared()) {
    auto insertQuery = folly::sformat(
      "INSERT INTO {} "
      "VALUES(@unitSn, @funcSn, @preClassId, @name, @top, @extraData);",
      m_repo.table(m_repoId, "Func"));
    txn.prepare(*this, insertQuery);
  }

  BlobEncoder extraBlob{fe.useGlobalIds()};
  RepoTxnQuery query(txn, *this);
  query.bindInt64("@unitSn", unitSn);
  query.bindInt("@funcSn", funcSn);
  query.bindId("@preClassId", preClassId);
  query.bindStaticString("@name", name);
  query.bindBool("@top", top);
  const_cast<FuncEmitter&>(fe).serdeMetaData(extraBlob);
  query.bindBlob("@extraData", extraBlob, /* static */ true);
  query.exec();

  RepoAutoloadMapBuilder::get().addFunc(fe, unitSn);
}

void FuncRepoProxy::GetFuncsStmt
                  ::get(UnitEmitter& ue) {
  auto txn = RepoTxn{m_repo.begin()};
  if (!prepared()) {
    auto selectQuery = folly::sformat(
      "SELECT funcSn, preClassId, name, top, extraData "
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
      bool top;                 /**/ query.getBool(3, top);
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
      fe->top = top;
      fe->serdeMetaData(extraBlob);
      if (!SystemLib::s_inited && !fe->isPseudoMain()) {
        assertx(fe->attrs & AttrBuiltin);
        if (preClassId < 0) {
          assertx(fe->attrs & AttrPersistent);
          assertx(fe->attrs & AttrUnique);
        }
      }
      fe->setEHTabIsSorted();
      fe->finish(fe->past);
    }
  } while (!query.done());
  txn.commit();
}

///////////////////////////////////////////////////////////////////////////////
}
