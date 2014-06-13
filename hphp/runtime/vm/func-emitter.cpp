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

#include "hphp/runtime/vm/func-emitter.h"

#include "hphp/parser/parser.h"

#include "hphp/runtime/base/base-includes.h"
#include "hphp/runtime/base/file-repository.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/strings.h"

#include "hphp/runtime/vm/blob-helper.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/func-inline.h"
#include "hphp/runtime/vm/native.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/runtime.h"

#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/types.h"

#include "hphp/runtime/vm/verifier/cfg.h"

#include "hphp/system/systemlib.h"

#include "hphp/util/atomic-vector.h"
#include "hphp/util/debug.h"
#include "hphp/util/trace.h"
#include "hphp/runtime/ext_zend_compat/hhvm/zend-wrap-func.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// FuncEmitter.

FuncEmitter::FuncEmitter(UnitEmitter& ue, int sn, Id id, const StringData* n)
  : m_ue(ue)
  , m_pce(nullptr)
  , m_sn(sn)
  , m_id(id)
  , m_name(n)
  , m_numLocals(0)
  , m_numUnnamedLocals(0)
  , m_activeUnnamedLocals(0)
  , m_numIterators(0)
  , m_nextFreeIterator(0)
  , m_retTypeConstraint(TypeConstraint())
  , m_retUserType(nullptr)
  , m_ehTabSorted(false)
  , m_returnType(KindOfInvalid)
  , m_top(false)
  , m_isClosureBody(false)
  , m_isAsync(false)
  , m_isGenerator(false)
  , m_isPairGenerator(false)
  , m_containsCalls(false)
  , m_info(nullptr)
  , m_builtinFuncPtr(nullptr)
  , m_originalFilename(nullptr)
{}

FuncEmitter::FuncEmitter(UnitEmitter& ue, int sn, const StringData* n,
                         PreClassEmitter* pce)
  : m_ue(ue)
  , m_pce(pce)
  , m_sn(sn)
  , m_name(n)
  , m_numLocals(0)
  , m_numUnnamedLocals(0)
  , m_activeUnnamedLocals(0)
  , m_numIterators(0)
  , m_nextFreeIterator(0)
  , m_retTypeConstraint(TypeConstraint())
  , m_retUserType(nullptr)
  , m_ehTabSorted(false)
  , m_returnType(KindOfInvalid)
  , m_top(false)
  , m_isClosureBody(false)
  , m_isAsync(false)
  , m_isGenerator(false)
  , m_isPairGenerator(false)
  , m_containsCalls(false)
  , m_info(nullptr)
  , m_builtinFuncPtr(nullptr)
  , m_originalFilename(nullptr)
{}

FuncEmitter::~FuncEmitter() {
}

void FuncEmitter::init(int line1, int line2, Offset base, Attr attrs, bool top,
                       const StringData* docComment) {
  m_line1 = line1;
  m_line2 = line2;
  m_base = base;
  m_attrs = attrs;
  m_top = top;
  m_docComment = docComment;
  if (!isPseudoMain()) {
    if (!SystemLib::s_inited) {
      assert(m_attrs & AttrBuiltin);
    }
    if ((m_attrs & AttrBuiltin) && !pce()) {
      m_attrs = m_attrs | AttrSkipFrame;
    }
  }
}

void FuncEmitter::finish(Offset past, bool load) {
  m_past = past;
  sortEHTab();
  sortFPITab(load);
}

EHEnt& FuncEmitter::addEHEnt() {
  assert(!m_ehTabSorted
    || "should only mark the ehtab as sorted after adding all of them");
  m_ehtab.push_back(EHEnt());
  m_ehtab.back().m_parentIndex = 7777;
  return m_ehtab.back();
}

void FuncEmitter::setEhTabIsSorted() {
  m_ehTabSorted = true;
  if (!debug) return;

  Offset curBase = 0;
  for (size_t i = 0; i < m_ehtab.size(); ++i) {
    auto& eh = m_ehtab[i];

    // Base offsets must be monotonically increasing.
    always_assert(curBase <= eh.m_base);
    curBase = eh.m_base;

    // Parent should come before, and must enclose this guy.
    always_assert(eh.m_parentIndex == -1 || eh.m_parentIndex < i);
    if (eh.m_parentIndex != -1) {
      auto& parent = m_ehtab[eh.m_parentIndex];
      always_assert(parent.m_base <= eh.m_base &&
                    parent.m_past >= eh.m_past);
    }
  }
}

FPIEnt& FuncEmitter::addFPIEnt() {
  m_fpitab.push_back(FPIEnt());
  return m_fpitab.back();
}

Id FuncEmitter::newLocal() {
  return m_numLocals++;
}

void FuncEmitter::appendParam(const StringData* name, const ParamInfo& info) {
  allocVarId(name);
  m_params.push_back(info);
}

void FuncEmitter::allocVarId(const StringData* name) {
  assert(name != nullptr);
  // Unnamed locals are segregated (they all come after the named locals).
  assert(m_numUnnamedLocals == 0);
  UNUSED Id id;
  if (m_localNames.find(name) == m_localNames.end()) {
    id = newLocal();
    assert(id == (int)m_localNames.size());
    m_localNames.add(name, name);
  }
}

Id FuncEmitter::lookupVarId(const StringData* name) const {
  assert(this->hasVar(name));
  return m_localNames.find(name)->second;
}

bool FuncEmitter::hasVar(const StringData* name) const {
  assert(name != nullptr);
  return m_localNames.find(name) != m_localNames.end();
}

Id FuncEmitter::allocIterator() {
  assert(m_numIterators >= m_nextFreeIterator);
  Id id = m_nextFreeIterator++;
  if (m_numIterators < m_nextFreeIterator) {
    m_numIterators = m_nextFreeIterator;
  }
  return id;
}

void FuncEmitter::freeIterator(Id id) {
  --m_nextFreeIterator;
  assert(id == m_nextFreeIterator);
}

void FuncEmitter::setNumIterators(Id numIterators) {
  assert(m_numIterators == 0);
  m_numIterators = numIterators;
}

Id FuncEmitter::allocUnnamedLocal() {
  ++m_activeUnnamedLocals;
  if (m_activeUnnamedLocals > m_numUnnamedLocals) {
    newLocal();
    ++m_numUnnamedLocals;
  }
  return m_numLocals - m_numUnnamedLocals + (m_activeUnnamedLocals - 1);
}

void FuncEmitter::freeUnnamedLocal(Id id) {
  assert(m_activeUnnamedLocals > 0);
  --m_activeUnnamedLocals;
}

void FuncEmitter::setNumLocals(Id numLocals) {
  assert(numLocals >= m_numLocals);
  m_numLocals = numLocals;
}

void FuncEmitter::addStaticVar(Func::SVInfo svInfo) {
  m_staticVars.push_back(svInfo);
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
  bool operator()(const EHEnt& e1, const EHEnt& e2) const {
    if (e1.m_base == e2.m_base) {
      if (e1.m_past == e2.m_past) {
        return e1.m_type == EHEnt::Type::Catch;
      }
      return e1.m_past > e2.m_past;
    }
    return e1.m_base < e2.m_base;
  }
};

}

void FuncEmitter::sortEHTab() {
  if (m_ehTabSorted) return;

  std::sort(m_ehtab.begin(), m_ehtab.end(), EHEntComp());

  for (unsigned int i = 0; i < m_ehtab.size(); i++) {
    m_ehtab[i].m_parentIndex = -1;
    for (int j = i - 1; j >= 0; j--) {
      if (m_ehtab[j].m_past >= m_ehtab[i].m_past) {
        // parent EHEnt better enclose this one.
        assert(m_ehtab[j].m_base <= m_ehtab[i].m_base);
        m_ehtab[i].m_parentIndex = j;
        break;
      }
    }
  }

  setEhTabIsSorted();
}

void FuncEmitter::sortFPITab(bool load) {
  // Sort it and fill in parent info
  std::sort(
    begin(m_fpitab), end(m_fpitab),
    [&] (const FPIEnt& a, const FPIEnt& b) {
      return a.m_fpushOff < b.m_fpushOff;
    }
  );
  for (unsigned int i = 0; i < m_fpitab.size(); i++) {
    m_fpitab[i].m_parentIndex = -1;
    m_fpitab[i].m_fpiDepth = 1;
    for (int j = i - 1; j >= 0; j--) {
      if (m_fpitab[j].m_fcallOff > m_fpitab[i].m_fcallOff) {
        m_fpitab[i].m_parentIndex = j;
        m_fpitab[i].m_fpiDepth = m_fpitab[j].m_fpiDepth + 1;
        break;
      }
    }
    if (!load) {
      // m_fpOff does not include the space taken up by locals, iterators and
      // the AR itself. Fix it here.
      m_fpitab[i].m_fpOff += m_numLocals
        + m_numIterators * kNumIterCells
        + (m_fpitab[i].m_fpiDepth) * kNumActRecCells;
    }
  }
}

void FuncEmitter::addUserAttribute(const StringData* name, TypedValue tv) {
  m_userAttributes[name] = tv;
}

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
 *
 *  e.g.   <<__Native("ActRec")>> function foo():mixed;
 */
static const StaticString
  s_native("__Native"),
  s_actrec("ActRec"),
  s_nofcallbuiltin("NoFCallBuiltin"),
  s_variadicbyref("VariadicByRef"),
  s_noinjection("NoInjection"),
  s_zendcompat("ZendCompat");

int FuncEmitter::parseNativeAttributes(Attr &attrs) const {
  int ret = Native::AttrNone;

  auto it = m_userAttributes.find(s_native.get());
  assert(it != m_userAttributes.end());
  const TypedValue userAttr = it->second;
  assert(userAttr.m_type == KindOfArray);
  for (ArrayIter it(userAttr.m_data.parr); it; ++it) {
    Variant userAttrVal = it.second();
    if (userAttrVal.isString()) {
      String userAttrStrVal = userAttrVal.toString();
      if (userAttrStrVal.get()->isame(s_actrec.get())) {
        ret = ret | Native::AttrActRec;
        attrs = attrs | AttrMayUseVV;
      } else if (userAttrStrVal.get()->isame(s_nofcallbuiltin.get())) {
        attrs = attrs | AttrNoFCallBuiltin;
      } else if (userAttrStrVal.get()->isame(s_variadicbyref.get())) {
        attrs = attrs | AttrVariadicByRef;
      } else if (userAttrStrVal.get()->isame(s_noinjection.get())) {
        attrs = attrs | AttrNoInjection;
      } else if (userAttrStrVal.get()->isame(s_zendcompat.get())) {
        ret = ret | Native::AttrZendCompat;
        // ZendCompat implies ActRec, no FCallBuiltin
        attrs = attrs | AttrMayUseVV | AttrNoFCallBuiltin;
        ret = ret | Native::AttrActRec;
      }
    }
  }
  return ret;
}

void FuncEmitter::commit(RepoTxn& txn) const {
  Repo& repo = Repo::get();
  FuncRepoProxy& frp = repo.frp();
  int repoId = m_ue.repoId();
  int64_t usn = m_ue.sn();

  frp.insertFunc(repoId)
     .insert(*this, txn, usn, m_sn, m_pce ? m_pce->id() : -1, m_name, m_top);
}

Func* FuncEmitter::create(Unit& unit, PreClass* preClass /* = NULL */) const {
  bool isGenerated = isdigit(m_name->data()[0]) ||
    ParserBase::IsClosureName(m_name->toCppString());

  Attr attrs = m_attrs;
  if (preClass && preClass->attrs() & AttrInterface) {
    attrs = Attr(attrs | AttrAbstract);
  }
  if (attrs & AttrPersistent &&
      ((RuntimeOption::EvalJitEnableRenameFunction && !isGenerated) ||
       (!RuntimeOption::RepoAuthoritative && SystemLib::s_inited) ||
       attrs & AttrInterceptable)) {
    if (attrs & AttrBuiltin) {
      SystemLib::s_anyNonPersistentBuiltins = true;
    }
    attrs = Attr(attrs & ~AttrPersistent);
  }
  if (RuntimeOption::EvalJitEnableRenameFunction &&
      !m_name->empty() &&
      !Func::isSpecial(m_name) &&
      !m_isClosureBody) {
    // intercepted functions need to pass all args through
    // to the interceptee
    attrs = Attr(attrs | AttrMayUseVV);
  }
  if (isVariadic()) { attrs = Attr(attrs | AttrVariadicParam); }

  if (!m_containsCalls) { attrs = Attr(attrs | AttrPhpLeafFn); }

  assert(!m_pce == !preClass);
  Func* f = m_ue.newFunc(this, unit, preClass, m_line1, m_line2, m_base,
                         m_past, m_name, attrs, m_top, m_docComment,
                         m_params.size(), m_isClosureBody);

  f->shared()->m_info = m_info;
  f->shared()->m_returnType = m_returnType;
  std::vector<Func::ParamInfo> fParams;
  for (unsigned i = 0; i < m_params.size(); ++i) {
    Func::ParamInfo pi = m_params[i];
    f->appendParam(m_params[i].ref(), pi, fParams);
  }

  f->shared()->m_localNames.create(m_localNames);
  f->shared()->m_numLocals = m_numLocals;
  f->shared()->m_numIterators = m_numIterators;
  f->m_maxStackCells = m_maxStackCells;
  f->shared()->m_staticVars = m_staticVars;
  f->shared()->m_ehtab = m_ehtab;
  f->shared()->m_fpitab = m_fpitab;
  f->shared()->m_isClosureBody = m_isClosureBody;
  f->shared()->m_isAsync = m_isAsync;
  f->shared()->m_isGenerator = m_isGenerator;
  f->shared()->m_isPairGenerator = m_isPairGenerator;
  f->shared()->m_userAttributes = m_userAttributes;
  f->shared()->m_builtinFuncPtr = m_builtinFuncPtr;
  f->shared()->m_nativeFuncPtr = m_nativeFuncPtr;
  f->shared()->m_retTypeConstraint = m_retTypeConstraint;
  f->shared()->m_retUserType = m_retUserType;
  f->shared()->m_originalFilename = m_originalFilename;
  f->shared()->m_isGenerated = isGenerated;

  f->finishedEmittingParams(fParams);

  if (attrs & AttrNative) {
    auto nif = Native::GetBuiltinFunction(m_name,
                                          m_pce ? m_pce->name() : nullptr,
                                          f->isStatic());
    if (nif) {
      Attr dummy = AttrNone;
      int nativeAttrs = parseNativeAttributes(dummy);
      if (nativeAttrs & Native::AttrZendCompat) {
        f->shared()->m_nativeFuncPtr = nif;
        f->shared()->m_builtinFuncPtr = zend_wrap_func;
      } else {
        if (parseNativeAttributes(dummy) & Native::AttrActRec) {
          f->shared()->m_builtinFuncPtr = nif;
          f->shared()->m_nativeFuncPtr = nullptr;
        } else {
          f->shared()->m_nativeFuncPtr = nif;
          f->shared()->m_builtinFuncPtr = m_pce ? Native::methodWrapper
                                                : Native::functionWrapper;
        }
      }
    } else {
      f->shared()->m_builtinFuncPtr = Native::unimplementedWrapper;
    }
  }
  return f;
}

void FuncEmitter::setBuiltinFunc(const ClassInfo::MethodInfo* info,
                                 BuiltinFunction bif, BuiltinFunction nif,
                                 Offset base) {
  assert(info);
  m_info = info;
  Attr attrs = AttrBuiltin;
  if (info->attribute & (ClassInfo::RefVariableArguments |
                         ClassInfo::MixedVariableArguments)) {
    attrs = attrs | AttrVariadicByRef;
  }
  if (info->attribute & ClassInfo::IsReference) {
    attrs = attrs | AttrReference;
  }
  if (info->attribute & ClassInfo::NoInjection) {
    attrs = attrs | AttrNoInjection;
  }
  if (info->attribute & ClassInfo::NoFCallBuiltin) {
    attrs = attrs | AttrNoFCallBuiltin;
  }
  if (info->attribute & ClassInfo::ParamCoerceModeNull) {
    attrs = attrs | AttrParamCoerceModeNull;
  } else if (info->attribute & ClassInfo::ParamCoerceModeFalse) {
    attrs = attrs | AttrParamCoerceModeFalse;
  }
  if (pce()) {
    if (info->attribute & ClassInfo::IsStatic) {
      attrs = attrs | AttrStatic;
    }
    if (info->attribute & ClassInfo::IsFinal) {
      attrs = attrs | AttrFinal;
    }
    if (info->attribute & ClassInfo::IsAbstract) {
      attrs = attrs | AttrAbstract;
    }
    if (info->attribute & ClassInfo::IsPrivate) {
      attrs = attrs | AttrPrivate;
    } else if (info->attribute & ClassInfo::IsProtected) {
      attrs = attrs | AttrProtected;
    } else {
      attrs = attrs | AttrPublic;
    }
  } else if (info->attribute & ClassInfo::AllowOverride) {
    attrs = attrs | AttrAllowOverride;
  }

  setReturnType(info->returnType);
  setDocComment(info->docComment);
  setLocation(0, 0);
  setBuiltinFunc(bif, nif, attrs, base);

  for (unsigned i = 0; i < info->parameters.size(); ++i) {
    // For builtin only, we use a dummy ParamInfo
    FuncEmitter::ParamInfo pi;
    const auto& parameter = info->parameters[i];
    pi.setRef((bool)(parameter->attribute & ClassInfo::IsReference));
    pi.builtinType = parameter->argType;
    appendParam(makeStaticString(parameter->name), pi);
  }
}

void FuncEmitter::setBuiltinFunc(BuiltinFunction bif, BuiltinFunction nif,
                                 Attr attrs, Offset base) {
  assert(bif);
  m_builtinFuncPtr = bif;
  m_nativeFuncPtr = nif;
  m_base = base;
  m_top = true;
  // TODO: Task #1137917: See if we can avoid marking most builtins with
  // "MayUseVV" and still make things work
  m_attrs = attrs | AttrBuiltin | AttrSkipFrame | AttrMayUseVV;
}

template<class SerDe>
void FuncEmitter::serdeMetaData(SerDe& sd) {
  // NOTE: name, top, and a few other fields currently serialized
  // outside of this.
  sd(m_line1)
    (m_line2)
    (m_base)
    (m_past)
    (m_attrs)
    (m_returnType)
    (m_docComment)
    (m_numLocals)
    (m_numIterators)
    (m_maxStackCells)
    (m_isClosureBody)
    (m_isAsync)
    (m_isGenerator)
    (m_isPairGenerator)
    (m_containsCalls)

    (m_params)
    (m_localNames)
    (m_staticVars)
    (m_ehtab)
    (m_fpitab)
    (m_userAttributes)
    (m_retTypeConstraint)
    (m_retUserType)
    (m_originalFilename)
    ;
}

///////////////////////////////////////////////////////////////////////////////
// FuncRepoProxy.

FuncRepoProxy::FuncRepoProxy(Repo& repo)
  : RepoProxy(repo)
#define FRP_OP(c, o) \
  , m_##o##Local(repo, RepoIdLocal), m_##o##Central(repo, RepoIdCentral)
    FRP_OPS
#undef FRP_OP
{
#define FRP_OP(c, o) \
  m_##o[RepoIdLocal] = &m_##o##Local; \
  m_##o[RepoIdCentral] = &m_##o##Central;
  FRP_OPS
#undef FRP_OP
}

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
  query.bindInt64("@unitSn", ue.sn());
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
      fe->setTop(top);
      fe->serdeMetaData(extraBlob);
      if (!SystemLib::s_inited && !fe->isPseudoMain()) {
        assert(fe->attrs() & AttrBuiltin);
        if (preClassId < 0) {
          assert(fe->attrs() & AttrPersistent);
          assert(fe->attrs() & AttrUnique);
          assert(fe->attrs() & AttrSkipFrame);
        }
      }
      fe->setEhTabIsSorted();
      fe->finish(fe->past(), true);
      ue.recordFunction(fe);
    }
  } while (!query.done());
  txn.commit();
}

///////////////////////////////////////////////////////////////////////////////
}
