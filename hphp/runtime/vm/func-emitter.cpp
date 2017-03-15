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

#include "hphp/parser/parser.h"

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/unit-cache.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/strings.h"

#include "hphp/runtime/vm/blob-helper.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/func-inline.h"
#include "hphp/runtime/vm/native.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/runtime.h"

#include "hphp/runtime/vm/jit/types.h"

#include "hphp/runtime/vm/verifier/cfg.h"

#include "hphp/system/systemlib.h"

#include "hphp/util/atomic-vector.h"
#include "hphp/util/debug.h"
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
  , m_activeUnnamedLocals(0)
  , m_numIterators(0)
  , m_nextFreeIterator(0)
  , m_numClsRefSlots(0)
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
  , m_activeUnnamedLocals(0)
  , m_numIterators(0)
  , m_nextFreeIterator(0)
  , m_numClsRefSlots(0)
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
  attrs = attrs_;
  docComment = docComment_;

  if (!isPseudoMain()) {
    if (!SystemLib::s_inited) {
      assert(attrs & AttrBuiltin);
    }
    if ((attrs & AttrBuiltin) && !pce()) {
      attrs |= AttrSkipFrame;
    }
  }
}

void FuncEmitter::finish(Offset past_, bool load) {
  past = past_;
  sortEHTab();
  sortFPITab(load);
}

void FuncEmitter::commit(RepoTxn& txn) const {
  Repo& repo = Repo::get();
  FuncRepoProxy& frp = repo.frp();
  int repoId = m_ue.m_repoId;
  int64_t usn = m_ue.m_sn;

  frp.insertFunc[repoId]
     .insert(*this, txn, usn, m_sn, m_pce ? m_pce->id() : -1, name, top);
}

static std::vector<EHEnt> toFixed(const std::vector<EHEntEmitter>& vec) {
  std::vector<EHEnt> ret;
  for (auto const& ehe : vec) {
    EHEnt e;
    e.m_type = ehe.m_type;
    e.m_itRef = ehe.m_itRef;
    e.m_base = ehe.m_base;
    e.m_past = ehe.m_past;
    e.m_iterId = ehe.m_iterId;
    e.m_parentIndex = ehe.m_parentIndex;
    e.m_handler = ehe.m_handler;
    ret.emplace_back(std::move(e));
  }
  return ret;
}

Func* FuncEmitter::create(Unit& unit, PreClass* preClass /* = NULL */) const {
  bool isGenerated = isdigit(name->data()[0]) ||
    ParserBase::IsClosureName(name->toCppString());

  Attr attrs = this->attrs;
  if (preClass && preClass->attrs() & AttrInterface) {
    attrs |= AttrAbstract;
  }
  if (!RuntimeOption::RepoAuthoritative) {
    if (RuntimeOption::EvalJitEnableRenameFunction) {
      attrs |= AttrInterceptable;
    } else {
      attrs = Attr(attrs & ~AttrInterceptable);
    }
  }
  if (attrs & AttrPersistent && !preClass &&
      (RuntimeOption::EvalJitEnableRenameFunction ||
       attrs & AttrInterceptable ||
       (!RuntimeOption::RepoAuthoritative && SystemLib::s_inited))) {
    if (attrs & AttrBuiltin) {
      SystemLib::s_anyNonPersistentBuiltins = true;
    }
    attrs = Attr(attrs & ~AttrPersistent);
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
    if (isVariadicByRef()) {
      attrs |= AttrVariadicByRef;
    }
  }

  if (!containsCalls) { attrs |= AttrPhpLeafFn; }

  assert(!m_pce == !preClass);
  auto f = m_ue.newFunc(this, unit, name, attrs, params.size());

  f->m_isPreFunc = !!preClass;

  bool const needsExtendedSharedData =
    isNative ||
    line2 - line1 >= Func::kSmallDeltaLimit ||
    past - base >= Func::kSmallDeltaLimit ||
    m_numClsRefSlots > 3;

  f->m_shared.reset(
    needsExtendedSharedData
      ? new Func::ExtendedSharedData(preClass, base, past, line1, line2,
                                     top, docComment)
      : new Func::SharedData(preClass, base, past,
                             line1, line2, top, docComment)
  );

  f->init(params.size());

  if (auto const ex = f->extShared()) {
    ex->m_hasExtendedSharedData = true;
    ex->m_builtinFuncPtr = nullptr;
    ex->m_nativeFuncPtr = nullptr;
    ex->m_line2 = line2;
    ex->m_past = past;
    ex->m_returnByValue = false;
    ex->m_isMemoizeWrapper = false;
    ex->m_actualNumClsRefSlots = m_numClsRefSlots;
  }

  std::vector<Func::ParamInfo> fParams;
  for (unsigned i = 0; i < params.size(); ++i) {
    Func::ParamInfo pi = params[i];
    if (pi.isVariadic()) {
      pi.builtinType = KindOfArray;
    }
    f->appendParam(params[i].byRef, pi, fParams);
  }

  f->shared()->m_localNames.create(m_localNames);
  f->shared()->m_numLocals = m_numLocals;
  f->shared()->m_numIterators = m_numIterators;
  f->m_maxStackCells = maxStackCells;
  f->shared()->m_staticVars = staticVars;
  f->shared()->m_ehtab = toFixed(ehtab);
  f->shared()->m_fpitab = fpitab;
  f->shared()->m_isClosureBody = isClosureBody;
  f->shared()->m_isAsync = isAsync;
  f->shared()->m_isGenerator = isGenerator;
  f->shared()->m_isPairGenerator = isPairGenerator;
  f->shared()->m_userAttributes = userAttributes;
  f->shared()->m_retTypeConstraint = retTypeConstraint;
  f->shared()->m_retUserType = retUserType;
  f->shared()->m_originalFilename = originalFilename;
  f->shared()->m_isGenerated = isGenerated;
  f->shared()->m_repoReturnType = repoReturnType;
  f->shared()->m_repoAwaitedReturnType = repoAwaitedReturnType;
  f->shared()->m_isMemoizeWrapper = isMemoizeWrapper;
  f->shared()->m_numClsRefSlots = m_numClsRefSlots;

  if (isNative) {
    auto const ex = f->extShared();

    ex->m_hniReturnType = hniReturnType;

    auto const& info = Native::GetBuiltinFunction(
      name,
      m_pce ? m_pce->name() : nullptr,
      f->isStatic()
    );

    Attr dummy = AttrNone;
    auto nativeAttributes = parseNativeAttributes(dummy);
    Native::getFunctionPointers(
      info,
      nativeAttributes,
      ex->m_builtinFuncPtr,
      ex->m_nativeFuncPtr
    );

    if (ex->m_nativeFuncPtr &&
        !(nativeAttributes & Native::AttrZendCompat)) {
      if (info.sig.ret == Native::NativeSig::Type::MixedTV) {
        ex->m_returnByValue = true;
      }
      int extra =
        (attrs & AttrNumArgs ? 1 : 0) +
        (isMethod() ? 1 : 0);
      assert(info.sig.args.size() == params.size() + extra);
      for (auto i = params.size(); i--; ) {
        switch (info.sig.args[extra + i]) {
          case Native::NativeSig::Type::ObjectArg:
          case Native::NativeSig::Type::StringArg:
          case Native::NativeSig::Type::ArrayArg:
          case Native::NativeSig::Type::ResourceArg:
          case Native::NativeSig::Type::OutputArg:
          case Native::NativeSig::Type::MixedTV:
            fParams[i].nativeArg = true;
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

template<class SerDe>
void FuncEmitter::serdeMetaData(SerDe& sd) {
  // NOTE: name, top, and a few other fields currently serialized
  // outside of this.
  sd(line1)
    (line2)
    (base)
    (past)
    (attrs)
    (hniReturnType)
    (repoReturnType)
    (repoAwaitedReturnType)
    (docComment)
    (m_numLocals)
    (m_numIterators)
    (m_numClsRefSlots)
    (maxStackCells)
    (isClosureBody)
    (isAsync)
    (isGenerator)
    (isPairGenerator)
    (containsCalls)
    (isNative)
    (isMemoizeWrapper)

    (params)
    (m_localNames)
    (staticVars)
    (ehtab)
    (fpitab)
    (userAttributes)
    (retTypeConstraint)
    (retUserType)
    (originalFilename)

    (dynCallWrapperId)
    ;
}


///////////////////////////////////////////////////////////////////////////////
// Locals, iterators, and parameters.

void FuncEmitter::allocVarId(const StringData* name) {
  assert(name != nullptr);
  // Unnamed locals are segregated (they all come after the named locals).
  assert(m_numUnnamedLocals == 0);
  UNUSED Id id;
  if (m_localNames.find(name) == m_localNames.end()) {
    id = (m_numLocals++);
    assert(id == (int)m_localNames.size());
    m_localNames.add(name, name);
  }
}

Id FuncEmitter::allocIterator() {
  assert(m_numIterators >= m_nextFreeIterator);
  Id id = m_nextFreeIterator++;
  if (m_numIterators < m_nextFreeIterator) {
    m_numIterators = m_nextFreeIterator;
  }
  return id;
}

Id FuncEmitter::allocUnnamedLocal() {
  ++m_activeUnnamedLocals;
  if (m_activeUnnamedLocals > m_numUnnamedLocals) {
    ++m_numLocals;
    ++m_numUnnamedLocals;
  }
  return m_numLocals - m_numUnnamedLocals + (m_activeUnnamedLocals - 1);
}


///////////////////////////////////////////////////////////////////////////////
// Unit tables.

EHEntEmitter& FuncEmitter::addEHEnt() {
  assert(!m_ehTabSorted
    || "should only mark the ehtab as sorted after adding all of them");
  ehtab.push_back(EHEntEmitter());
  ehtab.back().m_parentIndex = 7777;
  return ehtab.back();
}

FPIEnt& FuncEmitter::addFPIEnt() {
  fpitab.push_back(FPIEnt());
  return fpitab.back();
}

namespace {

/*
 * Ordering on EHEnts where e1 < e2 iff
 *
 *    a) e1 and e2 do not overlap, and e1 comes first
 *    b) e1 encloses e2
 *    c) e1 and e2 have the same region, but e1 is a Catch funclet and
 *       e2 is a Fault funclet.
 */
struct EHEntComp {
  bool operator()(const EHEntEmitter& e1, const EHEntEmitter& e2) const {
    if (e1.m_base == e2.m_base) {
      if (e1.m_past == e2.m_past) {
        static_assert(!static_cast<uint8_t>(EHEnt::Type::Catch),
            "Catch should be the smallest type");
        return e1.m_type < e2.m_type;
      }
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
        assert(ehtab[j].m_base <= ehtab[i].m_base);
        ehtab[i].m_parentIndex = j;
        break;
      }
    }
  }

  setEHTabIsSorted();
}

void FuncEmitter::sortFPITab(bool load) {
  // Sort it and fill in parent info
  std::sort(
    begin(fpitab), end(fpitab),
    [&] (const FPIEnt& a, const FPIEnt& b) {
      return a.m_fpushOff < b.m_fpushOff;
    }
  );
  for (unsigned int i = 0; i < fpitab.size(); i++) {
    fpitab[i].m_parentIndex = -1;
    fpitab[i].m_fpiDepth = 1;
    for (int j = i - 1; j >= 0; j--) {
      if (fpitab[j].m_fpiEndOff >= fpitab[i].m_fpiEndOff) {
        fpitab[i].m_parentIndex = j;
        fpitab[i].m_fpiDepth = fpitab[j].m_fpiDepth + 1;
        break;
      }
    }
    if (!load) {
      // m_fpOff does not include the space taken up by locals, iterators and
      // the AR itself. Fix it here.
      fpitab[i].m_fpOff += m_numLocals
        + m_numIterators * kNumIterCells
        + clsRefCountToCells(m_numClsRefSlots)
        + (fpitab[i].m_fpiDepth) * kNumActRecCells;
    }
  }
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
 *  "ActRec": The internal function takes a fixed prototype
 *      TypedValue* funcname(ActRec *ar);
 *      Note that systemlib declaration must still be hack annotated
 *  "NoFCallBuiltin": Prevent FCallBuiltin optimization
 *      Effectively forces functions to generate an ActRec
 *  "NoInjection": Do not include this frame in backtraces
 *  "ZendCompat": Use zend compat wrapper
 *  "ReadsCallerFrame": Function might read from the caller's frame
 *  "WritesCallerFrame": Function might write to the caller's frame
 *
 *  e.g.   <<__Native("ActRec")>> function foo():mixed;
 */
static const StaticString
  s_native("__Native"),
  s_actrec("ActRec"),
  s_nofcallbuiltin("NoFCallBuiltin"),
  s_variadicbyref("VariadicByRef"),
  s_noinjection("NoInjection"),
  s_zendcompat("ZendCompat"),
  s_numargs("NumArgs"),
  s_opcodeimpl("OpCodeImpl"),
  s_readsCallerFrame("ReadsCallerFrame"),
  s_writesCallerFrame("WritesCallerFrame");

int FuncEmitter::parseNativeAttributes(Attr& attrs_) const {
  int ret = Native::AttrNone;

  auto it = userAttributes.find(s_native.get());
  assert(it != userAttributes.end());
  const TypedValue userAttr = it->second;
  assert(isArrayType(userAttr.m_type));
  for (ArrayIter it(userAttr.m_data.parr); it; ++it) {
    Variant userAttrVal = it.second();
    if (userAttrVal.isString()) {
      String userAttrStrVal = userAttrVal.toString();
      if (userAttrStrVal.get()->isame(s_actrec.get())) {
        ret |= Native::AttrActRec;
        attrs_ |= AttrMayUseVV;
      } else if (userAttrStrVal.get()->isame(s_nofcallbuiltin.get())) {
        attrs_ |= AttrNoFCallBuiltin;
      } else if (userAttrStrVal.get()->isame(s_variadicbyref.get())) {
        attrs_ |= AttrVariadicByRef;
      } else if (userAttrStrVal.get()->isame(s_noinjection.get())) {
        attrs_ |= AttrNoInjection;
      } else if (userAttrStrVal.get()->isame(s_zendcompat.get())) {
        ret |= Native::AttrZendCompat;
        // ZendCompat implies ActRec, no FCallBuiltin
        attrs_ |= AttrMayUseVV | AttrNoFCallBuiltin;
        ret |= Native::AttrActRec;
      } else if (userAttrStrVal.get()->isame(s_numargs.get())) {
        attrs_ |= AttrNumArgs;
      } else if (userAttrStrVal.get()->isame(s_opcodeimpl.get())) {
        ret |= Native::AttrOpCodeImpl;
      } else if (userAttrStrVal.get()->isame(s_readsCallerFrame.get())) {
        attrs_ |= AttrReadsCallerFrame;
      } else if (userAttrStrVal.get()->isame(s_writesCallerFrame.get())) {
        attrs_ |= AttrWritesCallerFrame;
      }
    }
  }
  return ret;
}

void FuncEmitter::setBuiltinFunc(Attr attrs_, Offset base_) {
  isNative = true;
  base = base_;
  top = true;
  // TODO: Task #1137917: See if we can avoid marking most builtins with
  // "MayUseVV" and still make things work
  attrs = attrs_ | AttrBuiltin | AttrSkipFrame | AttrMayUseVV;
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
  std::stringstream ssCreate;
  ssCreate << "CREATE TABLE " << m_repo.table(repoId, "Func")
           << "(unitSn INTEGER, funcSn INTEGER, preClassId INTEGER,"
              " name TEXT, top INTEGER,"
              " extraData BLOB,"
              " PRIMARY KEY (unitSn, funcSn));";
  txn.exec(ssCreate.str());
}

void FuncRepoProxy::InsertFuncStmt
                  ::insert(const FuncEmitter& fe,
                           RepoTxn& txn, int64_t unitSn, int funcSn,
                           Id preClassId, const StringData* name,
                           bool top) {
  if (!prepared()) {
    std::stringstream ssInsert;
    ssInsert << "INSERT INTO " << m_repo.table(m_repoId, "Func")
             << " VALUES(@unitSn, @funcSn, @preClassId, @name, "
                "        @top, @extraData);";
    txn.prepare(*this, ssInsert.str());
  }

  BlobEncoder extraBlob;
  RepoTxnQuery query(txn, *this);
  query.bindInt64("@unitSn", unitSn);
  query.bindInt("@funcSn", funcSn);
  query.bindId("@preClassId", preClassId);
  query.bindStaticString("@name", name);
  query.bindBool("@top", top);
  const_cast<FuncEmitter&>(fe).serdeMetaData(extraBlob);
  query.bindBlob("@extraData", extraBlob, /* static */ true);
  query.exec();
}

void FuncRepoProxy::GetFuncsStmt
                  ::get(UnitEmitter& ue) {
  RepoTxn txn(m_repo);
  if (!prepared()) {
    std::stringstream ssSelect;
    ssSelect << "SELECT funcSn,preClassId,name,top,extraData "
                "FROM "
             << m_repo.table(m_repoId, "Func")
             << " WHERE unitSn == @unitSn ORDER BY funcSn ASC;";
    txn.prepare(*this, ssSelect.str());
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
      BlobDecoder extraBlob =   /**/ query.getBlob(4);

      FuncEmitter* fe;
      if (preClassId < 0) {
        fe = ue.newFuncEmitter(name);
      } else {
        PreClassEmitter* pce = ue.pce(preClassId);
        fe = ue.newMethodEmitter(name, pce);
        bool added UNUSED = pce->addMethod(fe);
        assert(added);
      }
      assert(fe->sn() == funcSn);
      fe->top = top;
      fe->serdeMetaData(extraBlob);
      if (!SystemLib::s_inited && !fe->isPseudoMain()) {
        assert(fe->attrs & AttrBuiltin);
        if (preClassId < 0) {
          assert(fe->attrs & AttrPersistent);
          assert(fe->attrs & AttrUnique);
          assert(fe->attrs & AttrSkipFrame);
        }
      }
      fe->setEHTabIsSorted();
      fe->finish(fe->past, true);
      ue.recordFunction(fe);
    }
  } while (!query.done());
  txn.commit();
}

///////////////////////////////////////////////////////////////////////////////
}
