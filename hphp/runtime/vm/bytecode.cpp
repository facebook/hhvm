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
#include "hphp/runtime/vm/bytecode.h"

#include <algorithm>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cinttypes>

#include <boost/format.hpp>

#include <libgen.h>
#include <sys/mman.h>

#include "folly/String.h"

#include "hphp/runtime/base/tv-comparisons.h"
#include "hphp/runtime/base/tv-conversions.h"
#include "hphp/runtime/base/tv-arith.h"
#include "hphp/compiler/builtin_symbols.h"
#include "hphp/runtime/vm/event-hook.h"
#include "hphp/runtime/vm/func-inline.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/translator-runtime.h"
#include "hphp/runtime/vm/srckey.h"
#include "hphp/runtime/vm/member-operations.h"
#include "hphp/runtime/base/class-info.h"
#include "hphp/runtime/base/code-coverage.h"
#include "hphp/runtime/base/file-repository.h"
#include "hphp/runtime/base/base-includes.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/hphp-array.h"
#include "hphp/runtime/base/strings.h"
#include "hphp/runtime/base/apc-typed-value.h"
#include "hphp/util/text-util.h"
#include "hphp/util/trace.h"
#include "hphp/util/debug.h"
#include "hphp/runtime/base/stat-cache.h"
#include "hphp/runtime/vm/debug/debug.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/php-debug.h"
#include "hphp/runtime/vm/debugger-hook.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/runtime-type-profiler.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/vm/treadmill.h"
#include "hphp/runtime/vm/type-constraint.h"
#include "hphp/runtime/vm/unwind.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/native.h"
#include "hphp/runtime/ext/ext_math.h"
#include "hphp/runtime/ext/ext_string.h"
#include "hphp/runtime/ext/ext_error.h"
#include "hphp/runtime/ext/ext_closure.h"
#include "hphp/runtime/ext/ext_continuation.h"
#include "hphp/runtime/ext/ext_function.h"
#include "hphp/runtime/ext/std/ext_std_variable.h"
#include "hphp/runtime/ext/ext_array.h"
#include "hphp/runtime/ext/ext_apc.h"
#include "hphp/runtime/ext/asio/async_function_wait_handle.h"
#include "hphp/runtime/ext/asio/static_result_wait_handle.h"
#include "hphp/runtime/ext/asio/static_exception_wait_handle.h"
#include "hphp/runtime/ext/asio/wait_handle.h"
#include "hphp/runtime/ext/asio/waitable_wait_handle.h"
#include "hphp/runtime/ext/reflection/ext_reflection.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/vm/type-profile.h"
#include "hphp/runtime/server/source-root-info.h"
#include "hphp/runtime/base/extended-logger.h"
#include "hphp/runtime/base/tracer.h"
#include "hphp/runtime/base/memory-profile.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/container-functions.h"

#include "hphp/system/systemlib.h"
#include "hphp/runtime/ext/ext_collections.h"

#include "hphp/runtime/vm/name-value-table-wrapper.h"

namespace HPHP {

// TODO: #1746957, #1756122
// we should skip the call in call_user_func_array, if
// by reference params are passed by value, or if its
// argument is not an array, but currently lots of tests
// depend on actually making the call.
const bool skipCufOnInvalidParams = false;

// RepoAuthoritative has been raptured out of runtime_option.cpp. It needs
// to be closer to other bytecode.cpp data.
bool RuntimeOption::RepoAuthoritative = false;

using std::string;

using JIT::VMRegAnchor;
using JIT::tx;
using JIT::mcg;
using JIT::tl_regState;
using JIT::VMRegState;

#if DEBUG
#define OPTBLD_INLINE
#else
#define OPTBLD_INLINE ALWAYS_INLINE
#endif
TRACE_SET_MOD(bcinterp);

ActRec* ActRec::arGetSfp() const {
  ActRec* prevFrame = (ActRec*)m_savedRbp;
  if (LIKELY(((uintptr_t)prevFrame - s_stackLimit) >= s_stackSize)) {
    if (LIKELY(prevFrame != nullptr)) return prevFrame;
  }

  return const_cast<ActRec*>(this);
}

bool
ActRec::skipFrame() const {
  return m_func && m_func->skipFrame();
}

template <>
Class* arGetContextClassImpl<false>(const ActRec* ar) {
  if (ar == nullptr) {
    return nullptr;
  }
  return ar->m_func->cls();
}

template <>
Class* arGetContextClassImpl<true>(const ActRec* ar) {
  if (ar == nullptr) {
    return nullptr;
  }
  if (ar->m_func->isPseudoMain() || ar->m_func->isBuiltin()) {
    // Pseudomains inherit the context of their caller
    auto const context = g_context.getNoCheck();
    ar = context->getPrevVMState(ar);
    while (ar != nullptr &&
             (ar->m_func->isPseudoMain() || ar->m_func->isBuiltin())) {
      ar = context->getPrevVMState(ar);
    }
    if (ar == nullptr) {
      return nullptr;
    }
  }
  return ar->m_func->cls();
}

const StaticString s_call_user_func("call_user_func");
const StaticString s_call_user_func_array("call_user_func_array");
const StaticString s_stdclass("stdclass");
const StaticString s___call("__call");
const StaticString s___callStatic("__callStatic");
const StaticString s_file("file");
const StaticString s_line("line");
const StaticString s_function("function");
const StaticString s_args("args");
const StaticString s_class("class");
const StaticString s_object("object");
const StaticString s_type("type");
const StaticString s_include("include");

///////////////////////////////////////////////////////////////////////////////

//=============================================================================
// Miscellaneous macros.

#define NEXT() pc++
#define DECODE_JMP(type, var)                                                 \
  type var __attribute__((unused)) = *(type*)pc;                              \
  ONTRACE(2,                                                                  \
          Trace::trace("decode:     Immediate %s %" PRIi64"\n", #type,        \
                       (int64_t)var));
#define ITER_SKIP(offset)  pc = origPc + (offset);

#define DECODE(type, var)                                                     \
  DECODE_JMP(type, var);                                                      \
  pc += sizeof(type)
#define DECODE_IVA(var)                                                       \
  int32_t var UNUSED = decodeVariableSizeImm(&pc);                            \
  ONTRACE(2,                                                                  \
          Trace::trace("decode:     Immediate int32 %" PRIi64"\n",            \
                       (int64_t)var));
#define DECODE_LITSTR(var)                                \
  StringData* var;                                        \
  do {                                                    \
    DECODE(Id, id);                                       \
    var = m_fp->m_func->unit()->lookupLitstrId(id);       \
  } while (false)

#define DECODE_LA(var) DECODE_IVA(var)
#define DECODE_IA(var) DECODE_IVA(var)
#define DECODE_OA      DECODE

#define DECODE_ITER_LIST(typeList, idList, vecLen) \
  DECODE(int32_t, vecLen);                         \
  assert(vecLen > 0);                              \
  Id* typeList = (Id*)pc;                          \
  Id* idList   = (Id*)pc + 1;                      \
  pc += 2 * vecLen * sizeof(Id);

#define SYNC() m_pc = pc

//=============================================================================
// Miscellaneous helpers.

static inline Class* frameStaticClass(ActRec* fp) {
  if (fp->hasThis()) {
    return fp->getThis()->getVMClass();
  } else if (fp->hasClass()) {
    return fp->getClass();
  } else {
    return nullptr;
  }
}

static Offset pcOff(const ExecutionContext* env) {
  return env->getFP()->m_func->unit()->offsetOf(env->m_pc);
}

//=============================================================================
// VarEnv.

VarEnv::VarEnv()
  : m_depth(0)
  , m_global(false)
  , m_cfp(0)
{
  m_nvTable.emplace(RuntimeOption::EvalVMInitialGlobalTableSize);

  auto tableWrapper = smart_new<GlobalNameValueTableWrapper>(&*m_nvTable);
  auto globalArray = make_tv<KindOfArray>(tableWrapper);
  globalArray.m_data.parr->incRefCount();
  m_nvTable->set(makeStaticString("GLOBALS"), &globalArray);
  tvRefcountedDecRef(&globalArray);
}

VarEnv::VarEnv(ActRec* fp, ExtraArgs* eArgs)
  : m_extraArgs(eArgs)
  , m_depth(1)
  , m_global(false)
  , m_cfp(fp)
{
  const Func* func = fp->m_func;
  const Id numNames = func->numNamedLocals();

  if (!numNames) return;

  m_nvTable.emplace(numNames);

  TypedValue** origLocs =
    reinterpret_cast<TypedValue**>(uintptr_t(this) + sizeof(VarEnv));
  TypedValue* loc = frame_local(fp, 0);
  for (Id i = 0; i < numNames; ++i, --loc) {
    assert(func->lookupVarId(func->localVarName(i)) == (int)i);
    origLocs[i] = m_nvTable->migrateSet(func->localVarName(i), loc);
  }
}

VarEnv::~VarEnv() {
  TRACE(3, "Destroying VarEnv %p [%s]\n",
           this,
           isGlobalScope() ? "global scope" : "local scope");
  assert(m_restoreLocations.empty());

  if (isGlobalScope()) {
    /*
     * When detaching the global scope, we leak any live objects (and
     * let the smart allocator clean them up).  This is because we're
     * not supposed to run destructors for objects that are live at
     * the end of a request.
     */
    m_nvTable->leak();
  }
}

size_t VarEnv::getObjectSz(ActRec* fp) {
  return sizeof(VarEnv) + sizeof(TypedValue*) * fp->m_func->numNamedLocals();
}

VarEnv* VarEnv::createLocal(ActRec* fp) {
  void* mem = smart_malloc(getObjectSz(fp));
  VarEnv* ret = new (mem) VarEnv(fp, fp->getExtraArgs());
  TRACE(3, "Creating lazily attached VarEnv %p on stack\n", mem);
  return ret;
}

VarEnv* VarEnv::createGlobal() {
  assert(!g_context->m_globalVarEnv);

  // Use smart_malloc instead of smartMallocSize since we need to use it above
  // and don't want to have to record which allocator was used to select the
  // appropriate deallocation function.
  auto const mem = smart_malloc(sizeof(VarEnv));
  auto ret = new (mem) VarEnv();

  TRACE(3, "Creating VarEnv %p [global scope]\n", ret);
  ret->m_global = true;
  g_context->m_globalVarEnv = ret;
  return ret;
}

void VarEnv::destroy(VarEnv* ve) {
  ve->~VarEnv();
  smart_free(ve);
}

void VarEnv::attach(ActRec* fp) {
  TRACE(3, "Attaching VarEnv %p [%s] %d fp @%p\n",
           this,
           isGlobalScope() ? "global scope" : "local scope",
           int(fp->m_func->numNamedLocals()), fp);
  assert(m_depth == 0 || fp->arGetSfp() == m_cfp ||
         (fp->arGetSfp() == fp && g_context->isNested()));
  m_cfp = fp;
  m_depth++;

  // Overlay fp's locals, if it has any.

  const Func* func = fp->m_func;
  const Id numNames = func->numNamedLocals();
  if (!numNames) {
    return;
  }
  if (!m_nvTable) {
    m_nvTable.emplace(numNames);
  }

  TypedValue** origLocs = smart_new_array<TypedValue*>(func->numNamedLocals());
  TypedValue* loc = frame_local(fp, 0);
  for (Id i = 0; i < numNames; ++i, --loc) {
    assert(func->lookupVarId(func->localVarName(i)) == (int)i);
    origLocs[i] = m_nvTable->migrate(func->localVarName(i), loc);
  }
  m_restoreLocations.push_back(origLocs);
}

void VarEnv::detach(ActRec* fp) {
  TRACE(3, "Detaching VarEnv %p [%s] @%p\n",
           this,
           isGlobalScope() ? "global scope" : "local scope",
           fp);
  assert(fp == m_cfp);
  assert(m_depth > 0);

  // Merge/remove fp's overlaid locals, if it had any.
  const Func* func = fp->m_func;
  if (Id const numLocals = func->numNamedLocals()) {
    /*
     * In the case of a lazily attached VarEnv, we have our locations
     * for the first (lazy) attach stored immediately following the
     * VarEnv in memory.  In this case m_restoreLocations will be empty.
     */
    assert((!isGlobalScope() && m_depth == 1) == m_restoreLocations.empty());
    TypedValue** origLocs =
      !m_restoreLocations.empty()
        ? m_restoreLocations.back()
          : reinterpret_cast<TypedValue**>(uintptr_t(this) + sizeof(VarEnv));

    for (Id i = 0; i < numLocals; i++) {
      m_nvTable->resettle(func->localVarName(i), origLocs[i]);
    }
    if (!m_restoreLocations.empty()) {
      m_restoreLocations.pop_back();
    }
  }

  auto const context = g_context.getNoCheck();
  m_cfp = context->getPrevVMState(fp);
  m_depth--;
  if (m_depth == 0) {
    m_cfp = nullptr;
    // don't free global varEnv
    if (context->m_globalVarEnv != this) {
      assert(!isGlobalScope());
      destroy(this);
    }
  }
}

// This helper is creating a NVT because of dynamic variable accesses,
// even though we're already attached to a frame and it had no named
// locals.
void VarEnv::ensureNvt() {
  const size_t kLazyNvtSize = 3;
  if (!m_nvTable) {
    m_nvTable.emplace(kLazyNvtSize);
  }
}

void VarEnv::set(const StringData* name, TypedValue* tv) {
  ensureNvt();
  m_nvTable->set(name, tv);
}

void VarEnv::bind(const StringData* name, TypedValue* tv) {
  ensureNvt();
  m_nvTable->bind(name, tv);
}

void VarEnv::setWithRef(const StringData* name, TypedValue* tv) {
  if (tv->m_type == KindOfRef) {
    bind(name, tv);
  } else {
    set(name, tv);
  }
}

TypedValue* VarEnv::lookup(const StringData* name) {
  if (!m_nvTable) {
    return 0;
  }
  return m_nvTable->lookup(name);
}

TypedValue* VarEnv::lookupAdd(const StringData* name) {
  ensureNvt();
  return m_nvTable->lookupAdd(name);
}

bool VarEnv::unset(const StringData* name) {
  if (!m_nvTable) return true;
  m_nvTable->unset(name);
  return true;
}

Array VarEnv::getDefinedVariables() const {
  Array ret = Array::Create();

  if (!m_nvTable) return ret;

  NameValueTable::Iterator iter(&*m_nvTable);
  for (; iter.valid(); iter.next()) {
    const StringData* sd = iter.curKey();
    const TypedValue* tv = iter.curVal();
    if (tvAsCVarRef(tv).isReferenced()) {
      ret.setRef(StrNR(sd).asString(), tvAsCVarRef(tv));
    } else {
      ret.add(StrNR(sd).asString(), tvAsCVarRef(tv));
    }
  }

  return ret;
}

TypedValue* VarEnv::getExtraArg(unsigned argInd) const {
  return m_extraArgs->getExtraArg(argInd);
}

//=============================================================================

ExtraArgs::ExtraArgs() {}
ExtraArgs::~ExtraArgs() {}

void* ExtraArgs::allocMem(unsigned nargs) {
  return smart_malloc(sizeof(TypedValue) * nargs + sizeof(ExtraArgs));
}

ExtraArgs* ExtraArgs::allocateCopy(TypedValue* args, unsigned nargs) {
  void* mem = allocMem(nargs);
  ExtraArgs* ea = new (mem) ExtraArgs();

  /*
   * The stack grows downward, so the args in memory are "backward"; i.e. the
   * leftmost (in PHP) extra arg is highest in memory.
   */
  std::reverse_copy(args, args + nargs, &ea->m_extraArgs[0]);
  return ea;
}

ExtraArgs* ExtraArgs::allocateUninit(unsigned nargs) {
  void* mem = ExtraArgs::allocMem(nargs);
  return new (mem) ExtraArgs();
}

void ExtraArgs::deallocate(ExtraArgs* ea, unsigned nargs) {
  assert(nargs > 0);

  for (unsigned i = 0; i < nargs; ++i) {
    tvRefcountedDecRef(ea->m_extraArgs + i);
  }
  ea->~ExtraArgs();
  smart_free(ea);
}

void ExtraArgs::deallocate(ActRec* ar) {
  const int numExtra = ar->numArgs() - ar->m_func->numParams();
  deallocate(ar->getExtraArgs(), numExtra);
}

TypedValue* ExtraArgs::getExtraArg(unsigned argInd) const {
  return const_cast<TypedValue*>(&m_extraArgs[argInd]);
}

//=============================================================================
// Stack.

// Store actual stack elements array in a thread-local in order to amortize the
// cost of allocation.
class StackElms {
 public:
  StackElms() : m_elms(nullptr) {}
  ~StackElms() {
    flush();
  }
  TypedValue* elms() {
    if (m_elms == nullptr) {
      // RuntimeOption::EvalVMStackElms-sized and -aligned.
      size_t algnSz = RuntimeOption::EvalVMStackElms * sizeof(TypedValue);
      if (posix_memalign((void**)&m_elms, algnSz, algnSz) != 0) {
        throw std::runtime_error(
          std::string("VM stack initialization failed: ") +
                      folly::errnoStr(errno).c_str());
      }

      madvise(m_elms, algnSz, MADV_DONTNEED);
      numa_bind_to(m_elms, algnSz, s_numaNode);
    }
    return m_elms;
  }
  void flush() {
    if (m_elms != nullptr) {
      free(m_elms);
      m_elms = nullptr;
    }
  }
 private:
  TypedValue* m_elms;
};
IMPLEMENT_THREAD_LOCAL(StackElms, t_se);

const int Stack::sSurprisePageSize = sysconf(_SC_PAGESIZE);
// We reserve the bottom page of each stack for use as the surprise
// page, so the minimum useful stack size is the next power of two.
const uint Stack::sMinStackElms = 2 * sSurprisePageSize / sizeof(TypedValue);

void Stack::ValidateStackSize() {
  if (RuntimeOption::EvalVMStackElms < sMinStackElms) {
    throw std::runtime_error(str(
      boost::format("VM stack size of 0x%llx is below the minimum of 0x%x")
        % RuntimeOption::EvalVMStackElms
        % sMinStackElms));
  }
  if (!folly::isPowTwo(RuntimeOption::EvalVMStackElms)) {
    throw std::runtime_error(str(
      boost::format("VM stack size of 0x%llx is not a power of 2")
        % RuntimeOption::EvalVMStackElms));
  }
}

Stack::Stack()
  : m_elms(nullptr), m_top(nullptr), m_base(nullptr) {
}

Stack::~Stack() {
  requestExit();
}

void
Stack::requestInit() {
  m_elms = t_se->elms();
  // Burn one element of the stack, to satisfy the constraint that
  // valid m_top values always have the same high-order (>
  // log(RuntimeOption::EvalVMStackElms)) bits.
  m_top = m_base = m_elms + RuntimeOption::EvalVMStackElms - 1;

  // Because of the surprise page at the bottom of the stack we lose an
  // additional 256 elements which must be taken into account when checking for
  // overflow.
  UNUSED size_t maxelms =
    RuntimeOption::EvalVMStackElms - sSurprisePageSize / sizeof(TypedValue);
  assert(!wouldOverflow(maxelms - 1));
  assert(wouldOverflow(maxelms));
}

void
Stack::requestExit() {
  if (m_elms != nullptr) {
    m_elms = nullptr;
  }
}

void flush_evaluation_stack() {
  if (g_context.isNull()) {
    // For RPCRequestHandler threads, the ExecutionContext can stay
    // alive across requests, and hold references to the VM stack, and
    // the RDS needs to keep track of which classes are live etc So
    // only flush the VM stack and the RDS if the execution context is
    // dead.

    if (!t_se.isNull()) {
      t_se->flush();
    }
    RDS::flush();
  }
}

static std::string toStringElm(const TypedValue* tv) {
  std::ostringstream os;

  if (tv->m_type < MinDataType || tv->m_type > MaxNumDataTypes) {
    os << " ??? type " << tv->m_type << "\n";
    return os.str();
  }
  if (IS_REFCOUNTED_TYPE(tv->m_type) && tv->m_data.pref->m_count <= 0 &&
      tv->m_data.pref->m_count != StaticValue) {
    // OK in the invoking frame when running a destructor.
    os << " ??? inner_count " << tv->m_data.pref->m_count << " ";
    return os.str();
  }

  auto print_count = [&] {
    if (tv->m_data.pref->m_count == StaticValue) {
      os << ":c(static)";
    } else {
      os << ":c(" << tv->m_data.pref->m_count << ")";
    }
  };

  switch (tv->m_type) {
  case KindOfRef:
    os << "V:(";
    os << "@" << tv->m_data.pref;
    os << toStringElm(tv->m_data.pref->tv());
    os << ")";
    return os.str();
  case KindOfClass:
    os << "A:";
    break;
  default:
    os << "C:";
    break;
  }

  switch (tv->m_type) {
  case KindOfUninit:
    os << "Uninit";
    break;
  case KindOfNull:
    os << "Null";
    break;
  case KindOfBoolean:
    os << (tv->m_data.num ? "True" : "False");
    break;
  case KindOfInt64:
    os << "0x" << std::hex << tv->m_data.num << std::dec;
    break;
  case KindOfDouble:
    os << tv->m_data.dbl;
    break;
  case KindOfStaticString:
  case KindOfString:
    {
      int len = tv->m_data.pstr->size();
      bool truncated = false;
      if (len > 128) {
        len = 128;
        truncated = true;
      }
      os << tv->m_data.pstr;
      print_count();
      os << ":\""
         << escapeStringForCPP(tv->m_data.pstr->data(), len)
         << "\"" << (truncated ? "..." : "");
    }
    break;
  case KindOfArray:
    assert_refcount_realistic_nz(tv->m_data.parr->getCount());
    os << tv->m_data.parr;
    print_count();
    os << ":Array";
    break;
  case KindOfObject:
    assert_refcount_realistic_nz(tv->m_data.pobj->getCount());
    os << tv->m_data.pobj;
    print_count();
    os << ":Object("
       << tv->m_data.pobj->o_getClassName().get()->data()
       << ")";
    break;
  case KindOfResource:
    assert_refcount_realistic_nz(tv->m_data.pres->getCount());
    os << tv->m_data.pres;
    print_count();
    os << ":Resource("
       << const_cast<ResourceData*>(tv->m_data.pres)
            ->o_getClassName().get()->data()
       << ")";
    break;
  case KindOfRef:
    not_reached();
  case KindOfClass:
    os << tv->m_data.pcls
       << ":" << tv->m_data.pcls->name()->data();
    break;
  default:
    os << "?";
    break;
  }

  return os.str();
}

static std::string toStringIter(const Iter* it, bool itRef) {
  if (itRef) return "I:MutableArray";

  // TODO(#2458166): it might be a CufIter, but we're just lucky that
  // the bit pattern for the CufIter is going to have a 0 in
  // getIterType for now.
  switch (it->arr().getIterType()) {
  case ArrayIter::TypeUndefined:
    return "I:Undefined";
  case ArrayIter::TypeArray:
    return "I:Array";
  case ArrayIter::TypeIterator:
    return "I:Iterator";
  }
  assert(false);
  return "I:?";
}

void Stack::toStringFrame(std::ostream& os, const ActRec* fp,
                          int offset, const TypedValue* ftop,
                          const string& prefix) const {
  assert(fp);

  // Use depth-first recursion to output the most deeply nested stack frame
  // first.
  {
    Offset prevPc = 0;
    TypedValue* prevStackTop = nullptr;
    ActRec* prevFp = g_context->getPrevVMState(fp, &prevPc, &prevStackTop);
    if (prevFp != nullptr) {
      toStringFrame(os, prevFp, prevPc, prevStackTop, prefix);
    }
  }

  os << prefix;
  const Func* func = fp->m_func;
  assert(func);
  func->validate();
  string funcName(func->fullName()->data());
  os << "{func:" << funcName
     << ",soff:" << fp->m_soff
     << ",this:0x" << std::hex << (fp->hasThis() ? fp->getThis() : nullptr)
     << std::dec << "}";
  TypedValue* tv = (TypedValue*)fp;
  tv--;

  if (func->numLocals() > 0) {
    os << "<";
    int n = func->numLocals();
    for (int i = 0; i < n; i++, tv--) {
      if (i > 0) {
        os << " ";
      }
      os << toStringElm(tv);
    }
    os << ">";
  }

  assert(!func->methInfo() || func->numIterators() == 0);
  if (func->numIterators() > 0) {
    os << "|";
    Iter* it = &((Iter*)&tv[1])[-1];
    for (int i = 0; i < func->numIterators(); i++, it--) {
      if (i > 0) {
        os << " ";
      }
      bool itRef;
      if (func->checkIterScope(offset, i, itRef)) {
        os << toStringIter(it, itRef);
      } else {
        os << "I:Undefined";
      }
    }
    os << "|";
  }

  std::vector<std::string> stackElems;
  visitStackElems(
    fp, ftop, offset,
    [&](const ActRec* ar) {
      stackElems.push_back(
        folly::format("{{func:{}}}", ar->m_func->fullName()->data()).str()
      );
    },
    [&](const TypedValue* tv) {
      stackElems.push_back(toStringElm(tv));
    }
  );
  std::reverse(stackElems.begin(), stackElems.end());
  os << ' ' << folly::join(' ', stackElems);

  os << '\n';
}

string Stack::toString(const ActRec* fp, int offset,
                       const string prefix/* = "" */) const {
  // The only way to figure out which stack elements are activation records is
  // to follow the frame chain. However, the goal for each stack frame is to
  // print stack fragments from deepest to shallowest -- a then b in the
  // following example:
  //
  //   {func:foo,soff:51}<C:8> {func:bar} C:8 C:1 {func:biz} C:0
  //                           aaaaaaaaaaaaaaaaaa bbbbbbbbbbbbbb
  //
  // Use depth-first recursion to get the output order correct.

  std::ostringstream os;
  auto unit = fp->unit();
  auto func = fp->func();
  os << prefix << "=== Stack at "
     << unit->filepath()->data() << ":"
     << unit->getLineNumber(unit->offsetOf(vmpc()))
     << " func " << func->fullName()->data() << " ===\n";

  toStringFrame(os, fp, offset, m_top, prefix);

  return os.str();
}

bool Stack::wouldOverflow(int numCells) const {
  // The funny approach here is to validate the translator's assembly
  // technique. We've aligned and sized the stack so that the high order
  // bits of valid cells are all the same. In the translator, numCells
  // can be hardcoded, and m_top is wired into a register,
  // so the expression requires no loads.
  intptr_t truncatedTop = intptr_t(m_top) / sizeof(TypedValue);
  truncatedTop &= RuntimeOption::EvalVMStackElms - 1;
  intptr_t diff = truncatedTop - numCells -
    sSurprisePageSize / sizeof(TypedValue);
  return diff < 0;
}

TypedValue* Stack::frameStackBase(const ActRec* fp) {
  assert(!fp->inGenerator());
  const Func* func = fp->m_func;
  return (TypedValue*)((uintptr_t)fp
                       - (uintptr_t)(func->numLocals()) * sizeof(TypedValue)
                       - (uintptr_t)(func->numIterators() * sizeof(Iter)));
}

TypedValue* Stack::generatorStackBase(const ActRec* fp) {
  assert(fp->inGenerator());
  auto const context = g_context.getNoCheck();
  ActRec* sfp = fp->arGetSfp();
  if (sfp == fp) {
    // In the reentrant case, we can consult the savedVM state. We simply
    // use the top of stack of the previous VM frame (since the ActRec,
    // locals, and iters for this frame do not reside on the VM stack).
    return context->m_nestedVMs.back().sp;
  }
  // In the non-reentrant case, we know generators are always called from a
  // function with an empty stack. So we find the caller's FP, compensate for
  // its locals and iterators, and then we've found the base of the generator's
  // stack.
  return (TypedValue*)sfp - sfp->m_func->numSlotsInFrame();
}


//=============================================================================
// ExecutionContext.

using namespace HPHP;

ActRec* ExecutionContext::getOuterVMFrame(const ActRec* ar) {
  ActRec* prevFrame = (ActRec*)ar->m_savedRbp;
  if (LIKELY(((uintptr_t)prevFrame - s_stackLimit) >= s_stackSize)) {
    if (LIKELY(prevFrame != nullptr)) return prevFrame;
  }

  if (LIKELY(!m_nestedVMs.empty())) return m_nestedVMs.back().fp;
  return nullptr;
}

Cell ExecutionContext::lookupClsCns(const NamedEntity* ne,
                                      const StringData* cls,
                                      const StringData* cns) {
  Class* class_ = Unit::loadClass(ne, cls);
  if (class_ == nullptr) {
    raise_error(Strings::UNKNOWN_CLASS, cls->data());
  }
  Cell clsCns = class_->clsCnsGet(cns);
  if (clsCns.m_type == KindOfUninit) {
    raise_error("Couldn't find constant %s::%s",
                cls->data(), cns->data());
  }
  return clsCns;
}

Cell ExecutionContext::lookupClsCns(const StringData* cls,
                                      const StringData* cns) {
  return lookupClsCns(Unit::GetNamedEntity(cls), cls, cns);
}

// Look up the method specified by methodName from the class specified by cls
// and enforce accessibility. Accessibility checks depend on the relationship
// between the class that first declared the method (baseClass) and the context
// class (ctx).
//
// If there are multiple accessible methods with the specified name declared in
// cls and ancestors of cls, the method from the most derived class will be
// returned, except if we are doing an ObjMethod call ("$obj->foo()") and there
// is an accessible private method, in which case the accessible private method
// will be returned.
//
// Accessibility rules:
//
//   | baseClass/ctx relationship | public | protected | private |
//   +----------------------------+--------+-----------+---------+
//   | anon/unrelated             | yes    | no        | no      |
//   | baseClass == ctx           | yes    | yes       | yes     |
//   | baseClass derived from ctx | yes    | yes       | no      |
//   | ctx derived from baseClass | yes    | yes       | no      |
//   +----------------------------+--------+-----------+---------+

const StaticString s_construct("__construct");

const Func* ExecutionContext::lookupMethodCtx(const Class* cls,
                                                const StringData* methodName,
                                                const Class* ctx,
                                                CallType callType,
                                                bool raise /* = false */) {
  const Func* method;
  if (callType == CallType::CtorMethod) {
    assert(methodName == nullptr);
    method = cls->getCtor();
  } else {
    assert(callType == CallType::ObjMethod || callType == CallType::ClsMethod);
    assert(methodName != nullptr);
    method = cls->lookupMethod(methodName);
    while (!method) {
      if (UNLIKELY(methodName == s_construct.get())) {
        // We were looking up __construct and failed to find it. Fall back
        // to old-style constructor: same as class name.
        method = cls->getCtor();
        if (!Func::isSpecial(method->name())) break;
      }
      if (raise) {
        raise_error("Call to undefined method %s::%s from %s%s",
                    cls->name()->data(),
                    methodName->data(),
                    ctx ? "context " : "anonymous context",
                    ctx ? ctx->name()->data() : "");
      }
      return nullptr;
    }
  }
  assert(method);
  bool accessible = true;
  // If we found a protected or private method, we need to do some
  // accessibility checks.
  if ((method->attrs() & (AttrProtected|AttrPrivate)) &&
      !g_context->debuggerSettings.bypassCheck) {
    Class* baseClass = method->baseCls();
    assert(baseClass);
    // If the context class is the same as the class that first
    // declared this method, then we know we have the right method
    // and we can stop here.
    if (ctx == baseClass) {
      return method;
    }
    // The anonymous context cannot access protected or private methods,
    // so we can fail fast here.
    if (ctx == nullptr) {
      if (raise) {
        raise_error("Call to %s method %s::%s from anonymous context",
                    (method->attrs() & AttrPrivate) ? "private" : "protected",
                    cls->name()->data(),
                    method->name()->data());
      }
      return nullptr;
    }
    assert(ctx);
    if (method->attrs() & AttrPrivate) {
      // The context class is not the same as the class that declared
      // this private method, so this private method is not accessible.
      // We need to keep going because the context class may define a
      // private method with this name.
      accessible = false;
    } else {
      // If the context class is derived from the class that first
      // declared this protected method, then we know this method is
      // accessible and we know the context class cannot have a private
      // method with the same name, so we're done.
      if (ctx->classof(baseClass)) {
        return method;
      }
      if (!baseClass->classof(ctx)) {
        // The context class is not the same, an ancestor, or a descendent
        // of the class that first declared this protected method, so
        // this method is not accessible. Because the context class is
        // not the same or an ancestor of the class which first declared
        // the method, we know that the context class is not the same
        // or an ancestor of cls, and therefore we don't need to check
        // if the context class declares a private method with this name,
        // so we can fail fast here.
        if (raise) {
          raise_error("Call to protected method %s::%s from context %s",
                      cls->name()->data(),
                      method->name()->data(),
                      ctx->name()->data());
        }
        return nullptr;
      }
      // We now know this protected method is accessible, but we need to
      // keep going because the context class may define a private method
      // with this name.
      assert(accessible && baseClass->classof(ctx));
    }
  }
  // If this is an ObjMethod call ("$obj->foo()") AND there is an ancestor
  // of cls that declares a private method with this name AND the context
  // class is an ancestor of cls, check if the context class declares a
  // private method with this name.
  if (method->hasPrivateAncestor() && callType == CallType::ObjMethod &&
      ctx && cls->classof(ctx)) {
    const Func* ctxMethod = ctx->lookupMethod(methodName);
    if (ctxMethod && ctxMethod->cls() == ctx &&
        (ctxMethod->attrs() & AttrPrivate)) {
      // For ObjMethod calls a private method from the context class
      // trumps any other method we may have found.
      return ctxMethod;
    }
  }
  if (accessible) {
    return method;
  }
  if (raise) {
    raise_error("Call to private method %s::%s from %s%s",
                method->baseCls()->name()->data(),
                method->name()->data(),
                ctx ? "context " : "anonymous context",
                ctx ? ctx->name()->data() : "");
  }
  return nullptr;
}

LookupResult ExecutionContext::lookupObjMethod(const Func*& f,
                                                 const Class* cls,
                                                 const StringData* methodName,
                                                 const Class* ctx,
                                                 bool raise /* = false */) {
  f = lookupMethodCtx(cls, methodName, ctx, CallType::ObjMethod, false);
  if (!f) {
    f = cls->lookupMethod(s___call.get());
    if (!f) {
      if (raise) {
        // Throw a fatal error
        lookupMethodCtx(cls, methodName, ctx, CallType::ObjMethod, true);
      }
      return LookupResult::MethodNotFound;
    }
    return LookupResult::MagicCallFound;
  }
  if (f->attrs() & AttrStatic && !f->isClosureBody()) {
    return LookupResult::MethodFoundNoThis;
  }
  return LookupResult::MethodFoundWithThis;
}

LookupResult
ExecutionContext::lookupClsMethod(const Func*& f,
                                    const Class* cls,
                                    const StringData* methodName,
                                    ObjectData* obj,
                                    const Class* ctx,
                                    bool raise /* = false */) {
  f = lookupMethodCtx(cls, methodName, ctx, CallType::ClsMethod, false);
  if (!f) {
    if (obj && obj->instanceof(cls)) {
      f = obj->getVMClass()->lookupMethod(s___call.get());
    }
    if (!f) {
      f = cls->lookupMethod(s___callStatic.get());
      if (!f) {
        if (raise) {
          // Throw a fatal error
          lookupMethodCtx(cls, methodName, ctx, CallType::ClsMethod, true);
        }
        return LookupResult::MethodNotFound;
      }
      f->validate();
      assert(f);
      assert(f->attrs() & AttrStatic);
      return LookupResult::MagicCallStaticFound;
    }
    assert(f);
    assert(obj);
    // __call cannot be static, this should be enforced by semantic
    // checks defClass time or earlier
    assert(!(f->attrs() & AttrStatic));
    return LookupResult::MagicCallFound;
  }
  if (obj && !(f->attrs() & AttrStatic) && obj->instanceof(cls)) {
    return LookupResult::MethodFoundWithThis;
  }
  return LookupResult::MethodFoundNoThis;
}

LookupResult ExecutionContext::lookupCtorMethod(const Func*& f,
                                                  const Class* cls,
                                                  bool raise /* = false */) {
  f = cls->getCtor();
  if (!(f->attrs() & AttrPublic)) {
    Class* ctx = arGetContextClass(getFP());
    f = lookupMethodCtx(cls, nullptr, ctx, CallType::CtorMethod, raise);
    if (!f) {
      // If raise was true than lookupMethodCtx should have thrown,
      // so we should only be able to get here if raise was false
      assert(!raise);
      return LookupResult::MethodNotFound;
    }
  }
  return LookupResult::MethodFoundWithThis;
}

ObjectData* ExecutionContext::createObject(StringData* clsName,
                                             const Variant& params,
                                             bool init /* = true */) {
  Class* class_ = Unit::loadClass(clsName);
  if (class_ == nullptr) {
    throw_missing_class(clsName->data());
  }

  Object o;
  o = newInstance(class_);
  if (init) {
    auto ctor = class_->getCtor();
    if (!(ctor->attrs() & AttrPublic)) {
      std::string msg = "Access to non-public constructor of class ";
      msg += class_->name()->data();
      throw Object(Reflection::AllocReflectionExceptionObject(msg));
    }
    // call constructor
    if (!isContainerOrNull(params)) {
      throw_param_is_not_container();
    }
    TypedValue ret;
    invokeFunc(&ret, ctor, params, o.get());
    tvRefcountedDecRef(&ret);
  }

  ObjectData* ret = o.detach();
  ret->decRefCount();
  return ret;
}

ObjectData* ExecutionContext::createObjectOnly(StringData* clsName) {
  return createObject(clsName, init_null_variant, false);
}

ActRec* ExecutionContext::getStackFrame() {
  VMRegAnchor _;
  return getFP();
}

ObjectData* ExecutionContext::getThis() {
  VMRegAnchor _;
  ActRec* fp = getFP();
  if (fp->skipFrame()) {
    fp = getPrevVMState(fp);
    if (!fp) return nullptr;
  }
  if (fp->hasThis()) {
    return fp->getThis();
  }
  return nullptr;
}

Class* ExecutionContext::getContextClass() {
  VMRegAnchor _;
  ActRec* ar = getFP();
  assert(ar != nullptr);
  if (ar->skipFrame()) {
    ar = getPrevVMState(ar);
    if (!ar) return nullptr;
  }
  return ar->m_func->cls();
}

Class* ExecutionContext::getParentContextClass() {
  if (Class* ctx = getContextClass()) {
    return ctx->parent();
  }
  return nullptr;
}

const String& ExecutionContext::getContainingFileName() {
  VMRegAnchor _;
  ActRec* ar = getFP();
  if (ar == nullptr) return empty_string;
  if (ar->skipFrame()) {
    ar = getPrevVMState(ar);
    if (ar == nullptr) return empty_string;
  }
  Unit* unit = ar->m_func->unit();
  return unit->filepathRef();
}

int ExecutionContext::getLine() {
  VMRegAnchor _;
  ActRec* ar = getFP();
  Unit* unit = ar ? ar->m_func->unit() : nullptr;
  Offset pc = unit ? pcOff(this) : 0;
  if (ar == nullptr) return -1;
  if (ar->skipFrame()) {
    ar = getPrevVMState(ar, &pc);
  }
  if (ar == nullptr || (unit = ar->m_func->unit()) == nullptr) return -1;
  return unit->getLineNumber(pc);
}

Array ExecutionContext::getCallerInfo() {
  VMRegAnchor _;
  Array result = Array::Create();
  ActRec* ar = getFP();
  if (ar->skipFrame()) {
    ar = getPrevVMState(ar);
  }
  while (ar->m_func->name()->isame(s_call_user_func.get())
         || ar->m_func->name()->isame(s_call_user_func_array.get())) {
    ar = getPrevVMState(ar);
    if (ar == nullptr) {
      return result;
    }
  }

  Offset pc = 0;
  ar = getPrevVMState(ar, &pc);
  while (ar != nullptr) {
    if (!ar->m_func->name()->isame(s_call_user_func.get())
        && !ar->m_func->name()->isame(s_call_user_func_array.get())) {
      Unit* unit = ar->m_func->unit();
      int lineNumber;
      if ((lineNumber = unit->getLineNumber(pc)) != -1) {
        result.set(s_file, unit->filepath()->data(), true);
        result.set(s_line, lineNumber);
        return result;
      }
    }
    ar = getPrevVMState(ar, &pc);
  }
  return result;
}

VarEnv* ExecutionContext::getVarEnv(int frame) {
  VMRegAnchor _;

  ActRec* fp = getFP();
  for (; frame > 0; --frame) {
    if (!fp) break;
    fp = getPrevVMState(fp);
  }
  if (UNLIKELY(!fp)) return NULL;
  if (fp->skipFrame()) {
    fp = getPrevVMState(fp);
  }
  if (!fp) return nullptr;
  assert(!fp->hasInvName());
  if (!fp->hasVarEnv()) {
    fp->setVarEnv(VarEnv::createLocal(fp));
  }
  return fp->m_varEnv;
}

void ExecutionContext::setVar(StringData* name, TypedValue* v, bool ref) {
  VMRegAnchor _;
  // setVar() should only be called after getVarEnv() has been called
  // to create a varEnv
  ActRec *fp = getFP();
  if (!fp) return;
  if (fp->skipFrame()) {
    fp = getPrevVMState(fp);
  }
  assert(!fp->hasInvName());
  assert(!fp->hasExtraArgs());
  assert(fp->m_varEnv != nullptr);
  if (ref) {
    fp->m_varEnv->bind(name, v);
  } else {
    fp->m_varEnv->set(name, v);
  }
}

Array ExecutionContext::getLocalDefinedVariables(int frame) {
  VMRegAnchor _;
  ActRec *fp = getFP();
  for (; frame > 0; --frame) {
    if (!fp) break;
    fp = getPrevVMState(fp);
  }
  if (!fp) {
    return Array::Create();
  }
  assert(!fp->hasInvName());
  if (fp->hasVarEnv()) {
    return fp->m_varEnv->getDefinedVariables();
  }
  const Func *func = fp->m_func;
  auto numLocals = func->numNamedLocals();
  ArrayInit ret(numLocals);
  for (Id id = 0; id < numLocals; ++id) {
    TypedValue* ptv = frame_local(fp, id);
    if (ptv->m_type == KindOfUninit) {
      continue;
    }
    Variant name(func->localVarName(id));
    ret.add(name, tvAsVariant(ptv));
  }
  return ret.toArray();
}

void ExecutionContext::shuffleMagicArgs(ActRec* ar) {
  // We need to put this where the first argument is
  StringData* invName = ar->getInvName();
  int nargs = ar->numArgs();
  ar->setVarEnv(nullptr);
  assert(!ar->hasVarEnv() && !ar->hasInvName());

  // We need to make an array containing all the arguments passed by
  // the caller and put it where the second argument is.
  auto argArray = Array::attach(
    nargs ? HphpArray::MakePacked(
              nargs, reinterpret_cast<TypedValue*>(ar) - nargs)
          : HphpArray::MakeReserve(0)
  );

  // Remove the arguments from the stack; they were moved into the
  // array so we don't need to decref.
  m_stack.ndiscard(nargs);

  // Move invName to where the first argument belongs, no need
  // to incRef/decRef since we are transferring ownership
  m_stack.pushStringNoRc(invName);

  // Move argArray to where the second argument belongs. We've already
  // incReffed the array above so we don't need to do it here.
  m_stack.pushArrayNoRc(argArray.detach());

  ar->setNumArgs(2);
}

// This helper only does a stack overflow check for the native stack
static inline void checkNativeStack() {
  ThreadInfo* info = ThreadInfo::s_threadInfo.getNoCheck();
  // Check whether func's maximum stack usage would overflow the stack.
  // Both native and VM stack overflows are independently possible.
  if (!stack_in_bounds(info)) {
    TRACE(1, "Maximum stack depth exceeded.\n");
    raise_error("Stack overflow");
  }
}

// This helper does a stack overflow check on *both* the native stack
// and the VM stack.
static inline void checkStack(Stack& stk, const Func* f) {
  ThreadInfo* info = ThreadInfo::s_threadInfo.getNoCheck();
  // Check whether func's maximum stack usage would overflow the stack.
  // Both native and VM stack overflows are independently possible.
  if (!stack_in_bounds(info) ||
      stk.wouldOverflow(f->maxStackCells() + kStackCheckPadding)) {
    TRACE(1, "Maximum stack depth exceeded.\n");
    raise_error("Stack overflow");
  }
}

void ExecutionContext::prepareFuncEntry(ActRec *ar, PC& pc) {
  const Func* func = ar->m_func;
  Offset firstDVInitializer = InvalidAbsoluteOffset;
  bool raiseMissingArgumentWarnings = false;
  int nparams = func->numParams();
  if (UNLIKELY(ar->m_varEnv != nullptr)) {
    /*
     * m_varEnv != nullptr => we have a varEnv, extraArgs, or an invName.
     */
    if (ar->hasInvName()) {
      // shuffleMagicArgs deals with everything. no need for
      // further argument munging
      shuffleMagicArgs(ar);
    } else if (ar->hasVarEnv()) {
      m_fp = ar;
      if (!ar->inGenerator()) {
        assert(func->isPseudoMain());
        pushLocalsAndIterators(func);
        ar->m_varEnv->attach(ar);
      }
      pc = func->getEntry();
      // Nothing more to do; get out
      return;
    } else {
      assert(ar->hasExtraArgs());
      assert(func->numParams() < ar->numArgs());
    }
  } else {
    int nargs = ar->numArgs();
    if (nargs != nparams) {
      if (nargs < nparams) {
        // Push uninitialized nulls for missing arguments. Some of them may end
        // up getting default-initialized, but regardless, we need to make space
        // for them on the stack.
        const Func::ParamInfoVec& paramInfo = func->params();
        for (int i = nargs; i < nparams; ++i) {
          m_stack.pushUninit();
          Offset dvInitializer = paramInfo[i].funcletOff();
          if (dvInitializer == InvalidAbsoluteOffset) {
            // We wait to raise warnings until after all the locals have been
            // initialized. This is important because things need to be in a
            // consistent state in case the user error handler throws.
            raiseMissingArgumentWarnings = true;
          } else if (firstDVInitializer == InvalidAbsoluteOffset) {
            // This is the first unpassed arg with a default value, so
            // this is where we'll need to jump to.
            firstDVInitializer = dvInitializer;
          }
        }
      } else {
        if (func->attrs() & AttrMayUseVV) {
          // Extra parameters must be moved off the stack.
          const int numExtras = nargs - nparams;
          ar->setExtraArgs(ExtraArgs::allocateCopy((TypedValue*)ar - nargs,
                                                   numExtras));
          m_stack.ndiscard(numExtras);
        } else {
          // The function we're calling is not marked as "MayUseVV",
          // so just discard the extra arguments
          int numExtras = nargs - nparams;
          for (int i = 0; i < numExtras; i++) {
            m_stack.popTV();
          }
          ar->setNumArgs(nparams);
        }
      }
    }
  }

  int nlocals = nparams;
  if (UNLIKELY(func->isClosureBody())) {
    int nuse = init_closure(ar, m_stack.top());
    // init_closure doesn't move m_stack
    m_stack.nalloc(nuse);
    nlocals += nuse;
    func = ar->m_func;
  }

  if (LIKELY(!ar->inGenerator())) {
    /*
     * we only get here from callAndResume
     * if we failed to get a translation for
     * a generator's prologue
     */
    pushLocalsAndIterators(func, nlocals);
  }

  m_fp = ar;
  if (firstDVInitializer != InvalidAbsoluteOffset) {
    pc = func->unit()->entry() + firstDVInitializer;
  } else {
    pc = func->getEntry();
  }
  // cppext functions/methods have their own logic for raising
  // warnings for missing arguments, so we only need to do this work
  // for non-cppext functions/methods
  if (raiseMissingArgumentWarnings && !func->isCPPBuiltin()) {
    // need to sync m_pc to pc for backtraces/re-entry
    SYNC();
    const Func::ParamInfoVec& paramInfo = func->params();
    for (int i = ar->numArgs(); i < nparams; ++i) {
      Offset dvInitializer = paramInfo[i].funcletOff();
      if (dvInitializer == InvalidAbsoluteOffset) {
        const char* name = func->name()->data();
        if (nparams == 1) {
          raise_warning(Strings::MISSING_ARGUMENT, name, i);
        } else {
          raise_warning(Strings::MISSING_ARGUMENTS, name, nparams, i);
        }
        break;
      }
    }
  }
}

void ExecutionContext::syncGdbState() {
  if (RuntimeOption::EvalJit && !RuntimeOption::EvalJitNoGdb) {
    mcg->getDebugInfo()->debugSync();
  }
}

void ExecutionContext::enterVMAtAsyncFunc(ActRec* enterFnAr, PC pc,
                                          ObjectData* exception) {
  assert(enterFnAr);
  assert(enterFnAr->inGenerator());
  assert(pc);

  m_fp = enterFnAr;
  m_pc = pc;
  if (!EventHook::FunctionEnter(enterFnAr, EventHook::NormalFunc)) return;
  assert(m_fp->func()->contains(m_pc));

  if (!exception) {
    enterVMAtCurPC();
  } else {
    assert(exception->instanceof(SystemLib::s_ExceptionClass));
    Object e(exception);
    throw e;
  }
}

void ExecutionContext::enterVMAtFunc(ActRec* enterFnAr) {
  assert(enterFnAr);
  assert(!enterFnAr->inGenerator());
  Stats::inc(Stats::VMEnter);

  bool useJit = ThreadInfo::s_threadInfo->m_reqInjectionData.getJit();
  bool useJitPrologue = useJit && m_fp && !enterFnAr->m_varEnv;

  if (LIKELY(useJitPrologue)) {
    int np = enterFnAr->m_func->numParams();
    int na = enterFnAr->numArgs();
    if (na > np) na = np + 1;
    JIT::TCA start = enterFnAr->m_func->getPrologue(na);
    mcg->enterTCAtPrologue(enterFnAr, start);
    return;
  }

  prepareFuncEntry(enterFnAr, m_pc);
  if (!EventHook::FunctionEnter(enterFnAr, EventHook::NormalFunc)) return;
  checkStack(m_stack, enterFnAr->m_func);
  assert(m_fp->func()->contains(m_pc));

  if (useJit) {
    JIT::TCA start = enterFnAr->m_func->getFuncBody();
    mcg->enterTCAfterPrologue(start);
  } else {
    dispatch();
  }
}

void ExecutionContext::enterVMAtCurPC() {
  assert(m_fp);
  assert(m_pc);
  assert(m_fp->func()->contains(m_pc));
  Stats::inc(Stats::VMEnter);

  if (ThreadInfo::s_threadInfo->m_reqInjectionData.getJit()) {
    SrcKey sk(m_fp->func(), m_pc);
    mcg->enterTCAtSrcKey(sk);
  } else {
    dispatch();
  }
}

/**
 * Enter VM and invoke a function or resume an async function. The 'ar'
 * argument points to an ActRec of the invoked/resumed function. When
 * an async function is resumed, a 'pc' pointing to the resume location
 * inside the async function must be provided. Optionally, the resumed
 * async function will throw an 'exception' upon entering VM if passed.
 */
void ExecutionContext::enterVM(ActRec* ar, PC pc, ObjectData* exception) {
  assert(ar);
  assert(ar->m_soff == 0);
  assert(ar->m_savedRbp == 0);

  DEBUG_ONLY int faultDepth = m_faults.size();
  SCOPE_EXIT { assert(m_faults.size() == faultDepth); };

  m_firstAR = ar;
  ar->m_savedRip = reinterpret_cast<uintptr_t>(tx->uniqueStubs.callToExit);
  assert(isReturnHelper(ar->m_savedRip));

  /*
   * When an exception is propagating, each nesting of the VM is
   * responsible for unwinding its portion of the execution stack, and
   * finding user handlers if it is a catchable exception.
   *
   * This try/catch is where all this logic is centered.  The actual
   * unwinding happens under exception_handler in unwind.cpp, which
   * returns a UnwindAction here to indicate what to do next.
   *
   * Either we'll enter the VM loop again at a user error/fault
   * handler, or propagate the exception to a less-nested VM.
   */
  bool first = true;
resume:
  try {
    if (first) {
      first = false;
      if (!pc) {
        enterVMAtFunc(ar);
      } else {
        enterVMAtAsyncFunc(ar, pc, exception);
      }
    } else {
      enterVMAtCurPC();
    }

    // Everything succeeded with no exception---return to the previous
    // VM nesting level.
    return;

  } catch (...) {
    always_assert(JIT::tl_regState == JIT::VMRegState::CLEAN);
    switch (exception_handler()) {
      case UnwindAction::Propagate:
        break;
      case UnwindAction::ResumeVM:
        goto resume;
      case UnwindAction::Return:
        return;
    }
  }

  /*
   * Here we have to propagate an exception out of this VM's nesting
   * level.
   */

  assert(m_faults.size() > 0);
  Fault fault = m_faults.back();
  m_faults.pop_back();

  switch (fault.m_faultType) {
  case Fault::Type::UserException:
    {
      Object obj = fault.m_userException;
      fault.m_userException->decRefCount();
      throw obj;
    }
  case Fault::Type::CppException:
    // throwException() will take care of deleting heap-allocated
    // exception object for us
    fault.m_cppException->throwException();
    not_reached();
  }

  not_reached();
}

void ExecutionContext::invokeFunc(TypedValue* retval,
                                    const Func* f,
                                    const Variant& args_,
                                    ObjectData* this_ /* = NULL */,
                                    Class* cls /* = NULL */,
                                    VarEnv* varEnv /* = NULL */,
                                    StringData* invName /* = NULL */,
                                    InvokeFlags flags /* = InvokeNormal */) {
  assert(retval);
  assert(f);
  // If f is a regular function, this_ and cls must be NULL
  assert(f->preClass() || f->isPseudoMain() || (!this_ && !cls));
  // If f is a method, either this_ or cls must be non-NULL
  assert(!f->preClass() || (this_ || cls));
  // If f is a static method, this_ must be NULL
  assert(!(f->attrs() & AttrStatic && !f->isClosureBody()) ||
         (!this_));
  // invName should only be non-NULL if we are calling __call or
  // __callStatic
  assert(!invName || f->name()->isame(s___call.get()) ||
         f->name()->isame(s___callStatic.get()));
  const auto& args = *args_.asCell();
  assert(isContainerOrNull(args));
  // If we are inheriting a variable environment then args_ must be empty
  assert(!varEnv || cellIsNull(&args) || !getContainerSize(args));

  VMRegAnchor _;

  bool isMagicCall = (invName != nullptr);

  if (this_ != nullptr) {
    this_->incRefCount();
  }
  Cell* savedSP = m_stack.top();

  if (f->attrs() & AttrPhpLeafFn ||
      f->numParams() > kStackCheckReenterPadding - kNumActRecCells) {
    // Check both the native stack and VM stack for overflow
    checkStack(m_stack, f);
  } else {
    // invokeFunc() must always check the native stack for overflow no
    // matter what
    checkNativeStack();
  }

  if (flags & InvokePseudoMain) {
    assert(f->isPseudoMain());
    assert(cellIsNull(&args) || !getContainerSize(args));
    Unit* toMerge = f->unit();
    toMerge->merge();
    if (toMerge->isMergeOnly()) {
      *retval = *toMerge->getMainReturn();
      return;
    }
  }

  ActRec* ar = m_stack.allocA();
  ar->m_soff = 0;
  ar->m_savedRbp = 0;
  ar->m_func = f;
  if (this_) {
    ar->setThis(this_);
  } else if (cls) {
    ar->setClass(cls);
  } else {
    ar->setThis(nullptr);
  }
  auto numPassedArgs = cellIsNull(&args) ? 0 : getContainerSize(args);
  if (isMagicCall) {
    ar->initNumArgs(2);
  } else {
    ar->initNumArgs(numPassedArgs);
  }
  ar->setVarEnv(varEnv);

#ifdef HPHP_TRACE
  if (m_fp == nullptr) {
    TRACE(1, "Reentry: enter %s(%p) from top-level\n",
          f->name()->data(), ar);
  } else {
    TRACE(1, "Reentry: enter %s(pc %p ar %p) from %s(%p)\n",
          f->name()->data(), m_pc, ar,
          m_fp->m_func ? m_fp->m_func->name()->data() : "unknownBuiltin", m_fp);
  }
#endif

  if (isMagicCall) {
    // Put the method name into the location of the first parameter. We
    // are transferring ownership, so no need to incRef/decRef here.
    m_stack.pushStringNoRc(invName);
    // Put array of arguments into the location of the second parameter
    if (args.m_type == KindOfArray && args.m_data.parr->isVectorData()) {
      m_stack.pushArray(args.m_data.parr);
    } else if (!numPassedArgs) {
      m_stack.pushArrayNoRc(empty_array.get());
    } else {
      assert(!cellIsNull(&args));
      PackedArrayInit ai(numPassedArgs);
      for (ArrayIter iter(args); iter; ++iter) {
        ai.appendWithRef(iter.secondRefPlus());
      }
      m_stack.pushArray(ai.create());
    }
  } else if (!cellIsNull(&args)) {
    const int numParams = f->numParams();
    const int numExtraArgs = numPassedArgs - numParams;
    ExtraArgs* extraArgs = nullptr;
    if (numExtraArgs > 0 && (f->attrs() & AttrMayUseVV)) {
      extraArgs = ExtraArgs::allocateUninit(numExtraArgs);
      ar->setExtraArgs(extraArgs);
    }
    int paramId = 0;
    for (ArrayIter iter(args); iter; ++iter, ++paramId) {
      const TypedValue* from = iter.secondRefPlus().asTypedValue();
      TypedValue* to;
      if (LIKELY(paramId < numParams)) {
        to = m_stack.allocTV();
      } else {
        if (!(f->attrs() & AttrMayUseVV)) {
          // Discard extra arguments, since the function cannot
          // possibly use them.
          assert(extraArgs == nullptr);
          ar->setNumArgs(numParams);
          break;
        }
        assert(extraArgs != nullptr && numExtraArgs > 0);
        // VarEnv expects the extra args to be in "reverse" order
        // (i.e. the last extra arg has the lowest address)
        to = extraArgs->getExtraArg(paramId - numParams);
      }
      if (LIKELY(!f->byRef(paramId))) {
        cellDup(*tvToCell(from), *to);
      } else if (from->m_type == KindOfRef && from->m_data.pref->m_count >= 2) {
        refDup(*from, *to);
      } else {
        if (flags & InvokeCuf) {
          try {
            raise_warning("Parameter %d to %s() expected to be "
                          "a reference, value given",
                          paramId + 1, f->fullName()->data());
          } catch (...) {
            // If an exception is thrown by the user error handler,
            // we need to clean up the stack
            m_stack.discard();
            invokeFuncCleanupHelper(retval, ar, paramId);
            throw;
          }
          if (skipCufOnInvalidParams) {
            m_stack.discard();
            invokeFuncCleanupHelper(retval, ar, paramId);
            return;
          }
        }
        cellDup(*tvToCell(from), *to);
      }
    }
  }

  pushVMState(savedSP);
  SCOPE_EXIT { popVMState(); };

  enterVM(ar);

  tvCopy(*m_stack.topTV(), *retval);
  m_stack.discard();
}

void ExecutionContext::invokeFuncCleanupHelper(TypedValue* retval,
                                                 ActRec* ar,
                                                 int numArgsPushed) {
  assert(retval && ar);
  const int numFormalParams = ar->m_func->numParams();
  ExtraArgs* extraArgs = ar->hasExtraArgs() ? ar->getExtraArgs() : nullptr;

  if (extraArgs) {
    int n =
      numArgsPushed > numFormalParams ? numArgsPushed - numFormalParams : 0;
    ExtraArgs::deallocate(extraArgs, n);
    ar->m_varEnv = nullptr;
    numArgsPushed -= n;
  }
  while (numArgsPushed > 0) {
    m_stack.popTV();
    numArgsPushed--;
  }
  m_stack.popAR();
  tvWriteNull(retval);
}

void ExecutionContext::invokeFuncFew(TypedValue* retval,
                                       const Func* f,
                                       void* thisOrCls,
                                       StringData* invName,
                                       int argc,
                                       const TypedValue* argv) {
  assert(retval);
  assert(f);
  // If this is a regular function, this_ and cls must be NULL
  assert(f->preClass() || !thisOrCls);
  // If this is a method, either this_ or cls must be non-NULL
  assert(!f->preClass() || thisOrCls);
  // If this is a static method, this_ must be NULL
  assert(!(f->attrs() & AttrStatic && !f->isClosureBody()) ||
         !ActRec::decodeThis(thisOrCls));
  // invName should only be non-NULL if we are calling __call or
  // __callStatic
  assert(!invName || f->name()->isame(s___call.get()) ||
         f->name()->isame(s___callStatic.get()));

  VMRegAnchor _;

  if (ObjectData* thiz = ActRec::decodeThis(thisOrCls)) {
    thiz->incRefCount();
  }
  Cell* savedSP = m_stack.top();

  if (f->attrs() & AttrPhpLeafFn ||
      argc > kStackCheckReenterPadding - kNumActRecCells) {
    // Check both the native stack and VM stack for overflow
    checkStack(m_stack, f);
  } else {
    // invokeFuncFew() must always check the native stack for overflow
    // no matter what
    checkNativeStack();
  }

  ActRec* ar = m_stack.allocA();
  ar->m_soff = 0;
  ar->m_savedRbp = 0;
  ar->m_func = f;
  ar->m_this = (ObjectData*)thisOrCls;
  ar->initNumArgs(argc);
  if (UNLIKELY(invName != nullptr)) {
    ar->setInvName(invName);
  } else {
    ar->m_varEnv = nullptr;
  }

#ifdef HPHP_TRACE
  if (m_fp == nullptr) {
    TRACE(1, "Reentry: enter %s(%p) from top-level\n",
          f->name()->data(), ar);
  } else {
    TRACE(1, "Reentry: enter %s(pc %p ar %p) from %s(%p)\n",
          f->name()->data(), m_pc, ar,
          m_fp->m_func ? m_fp->m_func->name()->data() : "unknownBuiltin", m_fp);
  }
#endif

  for (ssize_t i = 0; i < argc; ++i) {
    const TypedValue *from = &argv[i];
    TypedValue *to = m_stack.allocTV();
    if (LIKELY(from->m_type != KindOfRef || !f->byRef(i))) {
      cellDup(*tvToCell(from), *to);
    } else {
      refDup(*from, *to);
    }
  }

  pushVMState(savedSP);
  SCOPE_EXIT { popVMState(); };

  enterVM(ar);

  tvCopy(*m_stack.topTV(), *retval);
  m_stack.discard();
}

void ExecutionContext::resumeAsyncFunc(c_Continuation& cont,
                                       Cell& awaitResult) {
  assert(tl_regState == VMRegState::CLEAN);
  SCOPE_EXIT { assert(tl_regState == VMRegState::CLEAN); };

  ActRec* ar = cont.actRec();
  checkStack(m_stack, ar->func());
  ar->m_soff = 0;
  ar->m_savedRbp = 0;

  Cell* savedSP = m_stack.top();
  cellDup(awaitResult, *m_stack.allocC());

  pushVMState(savedSP);
  SCOPE_EXIT { popVMState(); };

  try {
    enterVM(ar, ar->func()->unit()->at(cont.offset()));
    cont.setStopped();
  } catch (...) {
    cont.setDone();
    cellSet(make_tv<KindOfNull>(), cont.m_value);
    throw;
  }
}

void ExecutionContext::resumeAsyncFuncThrow(c_Continuation& cont,
                                            ObjectData* exception) {
  assert(exception);
  assert(exception->instanceof(SystemLib::s_ExceptionClass));
  assert(tl_regState == VMRegState::CLEAN);
  SCOPE_EXIT { assert(tl_regState == VMRegState::CLEAN); };

  ActRec* ar = cont.actRec();
  checkStack(m_stack, ar->func());
  ar->m_soff = 0;
  ar->m_savedRbp = 0;

  pushVMState(m_stack.top());
  SCOPE_EXIT { popVMState(); };

  try {
    enterVM(ar, ar->func()->unit()->at(cont.offset()), exception);
    cont.setStopped();
  } catch (...) {
    cont.setDone();
    cellSet(make_tv<KindOfNull>(), cont.m_value);
    throw;
  }
}

void ExecutionContext::invokeUnit(TypedValue* retval, Unit* unit) {
  Func* func = unit->getMain();
  invokeFunc(retval, func, init_null_variant, nullptr, nullptr,
             m_globalVarEnv, nullptr, InvokePseudoMain);
}

/*
 * Given a pointer to a VM frame, returns the previous VM frame in the call
 * stack. This function will also pass back by reference the previous PC (if
 * prevPc is non-null) and the previous SP (if prevSp is non-null).
 *
 * If there is no previous VM frame, this function returns NULL and does not
 * set prevPc and prevSp.
 */
ActRec* ExecutionContext::getPrevVMState(const ActRec* fp,
                                           Offset* prevPc /* = NULL */,
                                           TypedValue** prevSp /* = NULL */,
                                           bool* fromVMEntry /* = NULL */) {
  if (fp == nullptr) {
    return nullptr;
  }
  ActRec* prevFp = fp->arGetSfp();
  if (prevFp != fp) {
    if (prevSp) {
      if (UNLIKELY(fp->inGenerator())) {
        *prevSp = (TypedValue*)prevFp - prevFp->m_func->numSlotsInFrame();
      } else {
        *prevSp = (TypedValue*)&fp[1];
      }
    }
    if (prevPc) *prevPc = prevFp->m_func->base() + fp->m_soff;
    if (fromVMEntry) *fromVMEntry = false;
    return prevFp;
  }
  // Linear search from end of m_nestedVMs. In practice, we're probably
  // looking for something recently pushed.
  int i = m_nestedVMs.size() - 1;
  ActRec* firstAR = m_firstAR;
  while (i >= 0 && firstAR != fp) {
    firstAR = m_nestedVMs[i--].firstAR;
  }
  if (i == -1) return nullptr;
  const VMState& vmstate = m_nestedVMs[i];
  prevFp = vmstate.fp;
  assert(prevFp);
  assert(prevFp->m_func->unit());
  if (prevSp) *prevSp = vmstate.sp;
  if (prevPc) {
    *prevPc = prevFp->m_func->unit()->offsetOf(vmstate.pc);
  }
  if (fromVMEntry) *fromVMEntry = true;
  return prevFp;
}

Array ExecutionContext::debugBacktrace(bool skip /* = false */,
                                         bool withSelf /* = false */,
                                         bool withThis /* = false */,
                                         VMParserFrame*
                                         parserFrame /* = NULL */,
                                         bool ignoreArgs /* = false */,
                                         int limit /* = 0 */) {
  Array bt = Array::Create();

  // If there is a parser frame, put it at the beginning of
  // the backtrace
  if (parserFrame) {
    bt.append(
      ArrayInit(2)
        .set(s_file, parserFrame->filename, true)
        .set(s_line, parserFrame->lineNumber, true)
        .toVariant()
    );
  }

  VMRegAnchor _;
  if (!getFP()) {
    // If there are no VM frames, we're done
    return bt;
  }

  int depth = 0;
  ActRec* fp = nullptr;
  Offset pc = 0;

  // Get the fp and pc of the top frame (possibly skipping one frame)
  {
    if (skip) {
      fp = getPrevVMState(getFP(), &pc);
      if (!fp) {
        // We skipped over the only VM frame, we're done
        return bt;
      }
    } else {
      fp = getFP();
      Unit *unit = getFP()->m_func->unit();
      assert(unit);
      pc = unit->offsetOf(m_pc);
    }

    // Handle the top frame
    if (withSelf) {
      // Builtins don't have a file and line number
      if (!fp->m_func->isBuiltin()) {
        Unit *unit = fp->m_func->unit();
        assert(unit);
        const char* filename = unit->filepath()->data();
        if (fp->m_func->originalFilename()) {
          filename = fp->m_func->originalFilename()->data();
        }
        assert(filename);
        Offset off = pc;

        ArrayInit frame(parserFrame ? 4 : 2);
        frame.set(s_file, filename, true);
        frame.set(s_line, unit->getLineNumber(off), true);
        if (parserFrame) {
          frame.set(s_function, s_include, true);
          frame.set(s_args, Array::Create(parserFrame->filename), true);
        }
        bt.append(frame.toVariant());
        depth++;
      }
    }
  }

  // Handle the subsequent VM frames
  Offset prevPc = 0;
  for (ActRec* prevFp = getPrevVMState(fp, &prevPc);
       fp != nullptr && (limit == 0 || depth < limit);
       fp = prevFp, pc = prevPc, prevFp = getPrevVMState(fp, &prevPc)) {
    // do not capture frame for HPHP only functions
    if (fp->m_func->isNoInjection()) {
      continue;
    }

    ArrayInit frame(7);

    auto const curUnit = fp->m_func->unit();
    auto const curOp = *reinterpret_cast<const Op*>(curUnit->at(pc));
    auto const isReturning = curOp == Op::RetC || curOp == Op::RetV;

    // Builtins and generators don't have a file and line number
    if (prevFp && !prevFp->m_func->isBuiltin() && !fp->inGenerator()) {
      auto const prevUnit = prevFp->m_func->unit();
      auto prevFile = prevUnit->filepath();
      if (prevFp->m_func->originalFilename()) {
        prevFile = prevFp->m_func->originalFilename();
      }
      assert(prevFile);
      frame.set(s_file, const_cast<StringData*>(prevFile), true);

      // In the normal method case, the "saved pc" for line number printing is
      // pointing at the cell conversion (Unbox/Pop) instruction, not the call
      // itself. For multi-line calls, this instruction is associated with the
      // subsequent line which results in an off-by-n. We're subtracting one
      // in order to look up the line associated with the FCall/FCallArray
      // instruction. Exception handling and the other opcodes (ex. BoxR)
      // already do the right thing. The emitter associates object access with
      // the subsequent expression and this would be difficult to modify.
      auto const opAtPrevPc =
        *reinterpret_cast<const Op*>(prevUnit->at(prevPc));
      Offset pcAdjust = 0;
      if (opAtPrevPc == OpPopR || opAtPrevPc == OpUnboxR) {
        pcAdjust = 1;
      }
      frame.set(s_line,
                prevFp->m_func->unit()->getLineNumber(prevPc - pcAdjust),
                true);
    }

    // check for include
    String funcname = const_cast<StringData*>(fp->m_func->name());
    if (fp->inGenerator()) {
      // retrieve the original function name from the inner continuation
      funcname = frame_continuation(fp)->t_getorigfuncname();
    }

    if (fp->m_func->isClosureBody()) {
      static StringData* s_closure_label =
          makeStaticString("{closure}");
      funcname = s_closure_label;
    }

    // check for pseudomain
    if (funcname.empty()) {
      if (!prevFp) continue;
      funcname = s_include;
    }

    frame.set(s_function, funcname, true);

    if (!funcname.same(s_include)) {
      // Closures have an m_this but they aren't in object context
      Class* ctx = arGetContextClass(fp);
      if (ctx != nullptr && !fp->m_func->isClosureBody()) {
        frame.set(s_class, ctx->name()->data(), true);
        if (fp->hasThis() && !isReturning) {
          if (withThis) {
            frame.set(s_object, Object(fp->getThis()), true);
          }
          frame.set(s_type, "->", true);
        } else {
          frame.set(s_type, "::", true);
        }
      }
    }

    Array args = Array::Create();
    if (ignoreArgs) {
      // do nothing
    } else if (funcname.same(s_include)) {
      if (depth) {
        args.append(const_cast<StringData*>(curUnit->filepath()));
        frame.set(s_args, args, true);
      }
    } else if (!RuntimeOption::EnableArgsInBacktraces || isReturning) {
      // Provide an empty 'args' array to be consistent with hphpc
      frame.set(s_args, args, true);
    } else {
      int nparams = fp->m_func->numParams();
      int nargs = fp->numArgs();
      /* builtin extra args are not stored in varenv */
      if (nargs <= nparams) {
        for (int i = 0; i < nargs; i++) {
          TypedValue *arg = frame_local(fp, i);
          args.append(tvAsVariant(arg));
        }
      } else {
        int i;
        for (i = 0; i < nparams; i++) {
          TypedValue *arg = frame_local(fp, i);
          args.append(tvAsVariant(arg));
        }
        for (; i < nargs; i++) {
          TypedValue *arg = fp->getExtraArg(i - nparams);
          args.append(tvAsVariant(arg));
        }
      }
      frame.set(s_args, args, true);
    }

    bt.append(frame.toVariant());
    depth++;
  }
  return bt;
}

MethodInfoVM::~MethodInfoVM() {
  for (std::vector<const ClassInfo::ParameterInfo*>::iterator it =
       parameters.begin(); it != parameters.end(); ++it) {
    if ((*it)->value != nullptr) {
      free((void*)(*it)->value);
    }
  }
}

ClassInfoVM::~ClassInfoVM() {
  for (auto& m : m_methodsVec) delete m;
  for (auto& p : m_properties) delete p.second;
  for (auto& c : m_constants)  delete c.second;
}

const ClassInfo::MethodInfo* ExecutionContext::findFunctionInfo(
  const String& name) {
  StringIMap<AtomicSmartPtr<MethodInfoVM> >::iterator it =
    m_functionInfos.find(name);
  if (it == m_functionInfos.end()) {
    Func* func = Unit::loadFunc(name.get());
    if (func == nullptr || func->builtinFuncPtr()) {
      return nullptr;
    }
    AtomicSmartPtr<MethodInfoVM> &m = m_functionInfos[name];
    m = new MethodInfoVM();
    func->getFuncInfo(m.get());
    return m.get();
  } else {
    return it->second.get();
  }
}

const ClassInfo* ExecutionContext::findClassInfo(const String& name) {
  if (name.empty()) return nullptr;
  StringIMap<AtomicSmartPtr<ClassInfoVM> >::iterator it =
    m_classInfos.find(name);
  if (it == m_classInfos.end()) {
    Class* cls = Unit::lookupClass(name.get());
    if (cls == nullptr) return nullptr;
    if (cls->clsInfo()) return cls->clsInfo();
    if (cls->attrs() & (AttrInterface | AttrTrait)) {
      // If the specified name matches with something that is not formally
      // a class, return NULL
      return nullptr;
    }
    AtomicSmartPtr<ClassInfoVM> &c = m_classInfos[name];
    c = new ClassInfoVM();
    cls->getClassInfo(c.get());
    return c.get();
  } else {
    return it->second.get();
  }
}

const ClassInfo* ExecutionContext::findInterfaceInfo(const String& name) {
  StringIMap<AtomicSmartPtr<ClassInfoVM> >::iterator it =
    m_interfaceInfos.find(name);
  if (it == m_interfaceInfos.end()) {
    Class* cls = Unit::lookupClass(name.get());
    if (cls == nullptr)  return nullptr;
    if (cls->clsInfo()) return cls->clsInfo();
    if (!(cls->attrs() & AttrInterface)) {
      // If the specified name matches with something that is not formally
      // an interface, return NULL
      return nullptr;
    }
    AtomicSmartPtr<ClassInfoVM> &c = m_interfaceInfos[name];
    c = new ClassInfoVM();
    cls->getClassInfo(c.get());
    return c.get();
  } else {
    return it->second.get();
  }
}

const ClassInfo* ExecutionContext::findTraitInfo(const String& name) {
  StringIMap<AtomicSmartPtr<ClassInfoVM> >::iterator it =
    m_traitInfos.find(name);
  if (it != m_traitInfos.end()) {
    return it->second.get();
  }
  Class* cls = Unit::lookupClass(name.get());
  if (cls == nullptr) return nullptr;
  if (cls->clsInfo()) return cls->clsInfo();
  if (!(cls->attrs() & AttrTrait)) {
    return nullptr;
  }
  AtomicSmartPtr<ClassInfoVM> &classInfo = m_traitInfos[name];
  classInfo = new ClassInfoVM();
  cls->getClassInfo(classInfo.get());
  return classInfo.get();
}

const ClassInfo::ConstantInfo* ExecutionContext::findConstantInfo(
    const String& name) {
  TypedValue* tv = Unit::lookupCns(name.get());
  if (tv == nullptr) {
    return nullptr;
  }
  ConstInfoMap::const_iterator it = m_constInfo.find(name.get());
  if (it != m_constInfo.end()) {
    return it->second;
  }
  StringData* key = makeStaticString(name.get());
  ClassInfo::ConstantInfo* ci = new ClassInfo::ConstantInfo();
  ci->name = *(const String*)&key;
  ci->valueLen = 0;
  ci->valueText = "";
  ci->setValue(tvAsCVarRef(tv));
  m_constInfo[key] = ci;
  return ci;
}

HPHP::Eval::PhpFile* ExecutionContext::lookupPhpFile(StringData* path,
                                                     const char* currentDir,
                                                     bool* initial_opt) {
  bool init;
  bool &initial = initial_opt ? *initial_opt : init;
  initial = true;

  struct stat s;
  String spath = Eval::resolveVmInclude(path, currentDir, &s);
  if (spath.isNull()) return nullptr;

  // Check if this file has already been included.
  auto it = m_evaledFiles.find(spath.get());
  HPHP::Eval::PhpFile* efile = nullptr;
  if (it != end(m_evaledFiles)) {
    // We found it! Return the unit.
    efile = it->second;
    initial = false;
    return efile;
  }
  // We didn't find it, so try the realpath.
  bool alreadyResolved =
    RuntimeOption::RepoAuthoritative ||
    (!RuntimeOption::CheckSymLink && (spath[0] == '/'));
  bool hasRealpath = false;
  String rpath;
  if (!alreadyResolved) {
    std::string rp = StatCache::realpath(spath.data());
    if (rp.size() != 0) {
      rpath = StringData::Make(rp.data(), rp.size(), CopyString);
      if (!rpath.same(spath)) {
        hasRealpath = true;
        it = m_evaledFiles.find(rpath.get());
        if (it != m_evaledFiles.end()) {
          // We found it! Update the mapping for spath and
          // return the unit.
          efile = it->second;
          m_evaledFiles[spath.get()] = efile;
          m_evaledFilesOrder.push_back(efile);
          spath.get()->incRefCount();
          initial = false;
          return efile;
        }
      }
    }
  }
  // This file hasn't been included yet, so we need to parse the file
  efile = HPHP::Eval::FileRepository::checkoutFile(
    hasRealpath ? rpath.get() : spath.get(), s);
  if (efile && initial_opt) {
    // if initial_opt is not set, this shouldnt be recorded as a
    // per request fetch of the file.
    if (RDS::testAndSetBit(efile->getId())) {
      initial = false;
    }
    // if parsing was successful, update the mappings for spath and
    // rpath (if it exists).
    m_evaledFilesOrder.push_back(efile);
    m_evaledFiles[spath.get()] = efile;
    spath.get()->incRefCount();
    // Don't incRef efile; checkoutFile() already counted it.
    if (hasRealpath) {
      m_evaledFiles[rpath.get()] = efile;
      rpath.get()->incRefCount();
    }
    DEBUGGER_ATTACHED_ONLY(phpDebuggerFileLoadHook(efile));
  }
  return efile;
}

Unit* ExecutionContext::evalInclude(StringData* path,
                                      const StringData* curUnitFilePath,
                                      bool* initial) {
  namespace fs = boost::filesystem;
  HPHP::Eval::PhpFile* efile = nullptr;
  if (curUnitFilePath) {
    fs::path currentUnit(curUnitFilePath->data());
    fs::path currentDir(currentUnit.branch_path());
    efile = lookupPhpFile(path, currentDir.string().c_str(), initial);
  } else {
    efile = lookupPhpFile(path, "", initial);
  }
  if (efile) {
    return efile->unit();
  }
  return nullptr;
}

HPHP::Unit* ExecutionContext::evalIncludeRoot(
  StringData* path, InclOpFlags flags, bool* initial) {
  HPHP::Eval::PhpFile* efile = lookupIncludeRoot(path, flags, initial);
  return efile ? efile->unit() : 0;
}

HPHP::Eval::PhpFile* ExecutionContext::lookupIncludeRoot(StringData* path,
                                                           InclOpFlags flags,
                                                           bool* initial,
                                                           Unit* unit) {
  String absPath;
  if (flags & InclOpFlags::Relative) {
    namespace fs = boost::filesystem;
    if (!unit) unit = getFP()->m_func->unit();
    fs::path currentUnit(unit->filepath()->data());
    fs::path currentDir(currentUnit.branch_path());
    absPath = currentDir.string() + '/';
    TRACE(2, "lookupIncludeRoot(%s): relative -> %s\n",
          path->data(),
          absPath.data());
  } else {
    assert(flags & InclOpFlags::DocRoot);
    absPath = SourceRootInfo::GetCurrentPhpRoot();
    TRACE(2, "lookupIncludeRoot(%s): docRoot -> %s\n",
          path->data(),
          absPath.data());
  }

  absPath += StrNR(path);

  auto const it = m_evaledFiles.find(absPath.get());
  if (it != end(m_evaledFiles)) {
    if (initial) *initial = false;
    return it->second;
  }

  return lookupPhpFile(absPath.get(), "", initial);
}

/*
  Instantiate hoistable classes and functions.
  If there is any more work left to do, setup a
  new frame ready to execute the pseudomain.

  return true iff the pseudomain needs to be executed.
*/
bool ExecutionContext::evalUnit(Unit* unit, PC& pc, int funcType) {
  m_pc = pc;
  unit->merge();
  if (unit->isMergeOnly()) {
    Stats::inc(Stats::PseudoMain_Skipped);
    *m_stack.allocTV() = *unit->getMainReturn();
    return false;
  }
  Stats::inc(Stats::PseudoMain_Executed);

  ActRec* ar = m_stack.allocA();
  assert((uintptr_t)&ar->m_func < (uintptr_t)&ar->m_r);
  Class* cls = liveClass();
  if (m_fp->hasThis()) {
    ObjectData *this_ = m_fp->getThis();
    this_->incRefCount();
    ar->setThis(this_);
  } else if (m_fp->hasClass()) {
    ar->setClass(m_fp->getClass());
  } else {
    ar->setThis(nullptr);
  }
  Func* func = unit->getMain(cls);
  assert(!func->isCPPBuiltin());
  ar->m_func = func;
  ar->initNumArgs(0);
  assert(getFP());
  assert(!m_fp->hasInvName());
  arSetSfp(ar, m_fp);
  ar->m_soff = uintptr_t(
    m_fp->m_func->unit()->offsetOf(pc) - m_fp->m_func->base());
  ar->m_savedRip = reinterpret_cast<uintptr_t>(tx->uniqueStubs.retHelper);
  assert(isReturnHelper(ar->m_savedRip));
  pushLocalsAndIterators(func);
  if (!m_fp->hasVarEnv()) {
    m_fp->setVarEnv(VarEnv::createLocal(m_fp));
  }
  ar->m_varEnv = m_fp->m_varEnv;
  ar->m_varEnv->attach(ar);

  m_fp = ar;
  pc = func->getEntry();
  SYNC();
  bool ret = EventHook::FunctionEnter(m_fp, funcType);
  pc = m_pc;
  checkStack(m_stack, func);
  return ret;
}

StaticString
  s_php_namespace("<?php namespace "),
  s_curly_return(" { return "),
  s_semicolon_curly("; }"),
  s_php_return("<?php return "),
  s_semicolon(";");
const Variant& ExecutionContext::getEvaledArg(const StringData* val,
                                         const String& namespacedName) {
  const String& key = *(String*)&val;

  if (m_evaledArgs.get()) {
    const Variant& arg = m_evaledArgs.get()->get(key);
    if (&arg != &null_variant) return arg;
  }

  String code;
  int pos = namespacedName.rfind('\\');
  if (pos != -1) {
    auto ns = namespacedName.substr(0, pos);
    code = s_php_namespace + ns + s_curly_return + key + s_semicolon_curly;
  } else {
    code = s_php_return + key + s_semicolon;
  }
  Unit* unit = compileEvalString(code.get());
  assert(unit != nullptr);
  Variant v;
  // Default arg values are not currently allowed to depend on class context.
  g_context->invokeFunc((TypedValue*)&v, unit->getMain(),
                          init_null_variant, nullptr, nullptr, nullptr, nullptr,
                          InvokePseudoMain);
  Variant &lv = m_evaledArgs.lvalAt(key, AccessFlags::Key);
  lv = v;
  return lv;
}

void ExecutionContext::recordLastError(const Exception &e, int errnum) {
  m_lastError = String(e.getMessage());
  m_lastErrorNum = errnum;
  m_lastErrorPath = getContainingFileName();
  m_lastErrorLine = getLine();
}

/*
 * Helper for function entry, including pseudo-main entry.
 */
void
ExecutionContext::pushLocalsAndIterators(const Func* func,
                                         int nparams /*= 0*/) {
  // Push locals.
  for (int i = nparams; i < func->numLocals(); i++) {
    m_stack.pushUninit();
  }
  // Push iterators.
  for (int i = 0; i < func->numIterators(); i++) {
    m_stack.allocI();
  }
}

void ExecutionContext::enqueueAPCHandle(APCHandle* handle) {
  assert(handle->getUncounted());
  assert(handle->getType() == KindOfString ||
         handle->getType() == KindOfArray);
  m_apcHandles.push_back(handle);
}

// Treadmill solution for the SharedVariant memory management
namespace {
class FreedAPCHandle {
  std::vector<APCHandle*> m_apcHandles;
public:
  explicit FreedAPCHandle(std::vector<APCHandle*>&& shandles)
    : m_apcHandles(std::move(shandles))
  {}
  void operator()() {
    for (auto handle : m_apcHandles) {
      APCTypedValue::fromHandle(handle)->deleteUncounted();
    }
  }
};
}

void ExecutionContext::manageAPCHandle() {
  assert(apcExtension::UseUncounted || m_apcHandles.size() == 0);
  if (apcExtension::UseUncounted) {
    Treadmill::enqueue(FreedAPCHandle(std::move(m_apcHandles)));
    m_apcHandles.clear();
  }
}

void ExecutionContext::destructObjects() {
  if (UNLIKELY(RuntimeOption::EnableObjDestructCall)) {
    while (!m_liveBCObjs.empty()) {
      ObjectData* obj = *m_liveBCObjs.begin();
      obj->destruct(); // Let the instance remove the node.
    }
    m_liveBCObjs.clear();
  }
}

// Evaled units have a footprint in the TC and translation metadata. The
// applications we care about tend to have few, short, stereotyped evals,
// where the same code keeps getting eval'ed over and over again; so we
// keep around units for each eval'ed string, so that the TC space isn't
// wasted on each eval.
typedef RankedCHM<StringData*, HPHP::Unit*,
        StringDataHashCompare,
        RankEvaledUnits> EvaledUnitsMap;
static EvaledUnitsMap s_evaledUnits;
Unit* ExecutionContext::compileEvalString(
    StringData* code,
    const char* evalFilename /* = nullptr */) {
  EvaledUnitsMap::accessor acc;
  // Promote this to a static string; otherwise it may get swept
  // across requests.
  code = makeStaticString(code);
  if (s_evaledUnits.insert(acc, code)) {
    acc->second = compile_string(
      code->data(),
      code->size(),
      evalFilename
    );
  }
  return acc->second;
}

const String& ExecutionContext::createFunction(const String& args,
                                               const String& code) {
  if (UNLIKELY(RuntimeOption::EvalAuthoritativeMode)) {
    // Whole program optimizations need to assume they can see all the
    // code.
    raise_error("You can't use create_function in RepoAuthoritative mode; "
                "use a closure instead");
  }

  VMRegAnchor _;
  // It doesn't matter if there's a user function named __lambda_func; we only
  // use this name during parsing, and then change it to an impossible name
  // with a NUL byte before we merge it into the request's func map.  This also
  // has the bonus feature that the value of __FUNCTION__ inside the created
  // function will match Zend. (Note: Zend will actually fatal if there's a
  // user function named __lambda_func when you call create_function. Huzzah!)
  static StringData* oldName = makeStaticString("__lambda_func");
  std::ostringstream codeStr;
  codeStr << "<?php function " << oldName->data()
          << "(" << args.data() << ") {"
          << code.data() << "}\n";
  StringData* evalCode = makeStaticString(codeStr.str());
  Unit* unit = compile_string(evalCode->data(), evalCode->size());
  // Move the function to a different name.
  std::ostringstream newNameStr;
  newNameStr << '\0' << "lambda_" << ++m_lambdaCounter;
  StringData* newName = makeStaticString(newNameStr.str());
  unit->renameFunc(oldName, newName);
  m_createdFuncs.push_back(unit);
  unit->merge();

  // Technically we shouldn't have to eval the unit right now (it'll execute
  // the pseudo-main, which should be empty) and could get away with just
  // mergeFuncs. However, Zend does it this way, as proven by the fact that you
  // can inject code into the evaled unit's pseudo-main:
  //
  //   create_function('', '} echo "hi"; if (0) {');
  //
  // We have to eval now to emulate this behavior.
  TypedValue retval;
  invokeFunc(&retval, unit->getMain(), init_null_variant,
             nullptr, nullptr, nullptr, nullptr,
             InvokePseudoMain);

  // __lambda_func will be the only hoistable function.
  // Any functions or closures defined in it will not be hoistable.
  Func* lambda = unit->firstHoistable();
  return lambda->nameRef();
}

bool ExecutionContext::evalPHPDebugger(TypedValue* retval, StringData *code,
                                         int frame) {
  assert(retval);
  // The code has "<?php" prepended already
  Unit* unit = compile_string(code->data(), code->size());
  if (unit == nullptr) {
    raise_error("Syntax error");
    tvWriteNull(retval);
    return true;
  }
  // Do not JIT this unit, we are using it exactly once.
  unit->setInterpretOnly();

  bool failed = true;
  VarEnv *varEnv = nullptr;
  ActRec *fp = getFP();
  ActRec *cfpSave = nullptr;
  if (fp) {
    for (; frame > 0; --frame) {
      ActRec* prevFp = getPrevVMState(fp);
      if (!prevFp) {
        // To be safe in case we failed to get prevFp. This would mean we've
        // been asked to eval in a frame which is beyond the top of the stack.
        // This suggests the debugger client has made an error.
        break;
      }
      fp = prevFp;
    }
    if (!fp->hasVarEnv()) {
      fp->setVarEnv(VarEnv::createLocal(fp));
    }
    varEnv = fp->m_varEnv;
    cfpSave = varEnv->getCfp();
  }
  ObjectData *this_ = nullptr;
  // NB: the ActRec and function within the AR may have different classes. The
  // class in the ActRec is the type used when invoking the function (i.e.,
  // Derived in Derived::Foo()) while the class obtained from the function is
  // the type that declared the function Foo, which may be Base. We need both
  // the class to match any object that this function may have been invoked on,
  // and we need the class from the function execution is stopped in.
  Class *frameClass = nullptr;
  Class *functionClass = nullptr;
  if (fp) {
    if (fp->hasThis()) {
      this_ = fp->getThis();
    } else if (fp->hasClass()) {
      frameClass = fp->getClass();
    }
    functionClass = fp->m_func->cls();
    phpDebuggerEvalHook(fp->m_func);
  }

  const static StaticString s_cppException("Hit an exception");
  const static StaticString s_phpException("Hit a php exception");
  const static StaticString s_exit("Hit exit");
  const static StaticString s_fatal("Hit fatal");
  try {
    // Invoke the given PHP, possibly specialized to match the type of the
    // current function on the stack, optionally passing a this pointer or
    // class used to execute the current function.
    invokeFunc(retval, unit->getMain(functionClass), init_null_variant,
               this_, frameClass, varEnv, nullptr, InvokePseudoMain);
    failed = false;
  } catch (FatalErrorException &e) {
    g_context->write(s_fatal);
    g_context->write(" : ");
    g_context->write(e.getMessage().c_str());
    g_context->write("\n");
    g_context->write(ExtendedLogger::StringOfStackTrace(e.getBackTrace()));
  } catch (ExitException &e) {
    g_context->write(s_exit.data());
    g_context->write(" : ");
    std::ostringstream os;
    os << ExitException::ExitCode;
    g_context->write(os.str());
  } catch (Eval::DebuggerException &e) {
  } catch (Exception &e) {
    g_context->write(s_cppException.data());
    g_context->write(" : ");
    g_context->write(e.getMessage().c_str());
    ExtendedException* ee = dynamic_cast<ExtendedException*>(&e);
    if (ee) {
      g_context->write("\n");
      g_context->write(
        ExtendedLogger::StringOfStackTrace(ee->getBackTrace()));
    }
  } catch (Object &e) {
    g_context->write(s_phpException.data());
    g_context->write(" : ");
    g_context->write(e->invokeToString().data());
  } catch (...) {
    g_context->write(s_cppException.data());
  }

  if (varEnv) {
    // The debugger eval frame may have attached to the VarEnv from a
    // frame that was not the top frame, so we need to manually set
    // cfp back to what it was before
    varEnv->setCfp(cfpSave);
  }
  return failed;
}

void ExecutionContext::enterDebuggerDummyEnv() {
  static Unit* s_debuggerDummy = compile_string("<?php?>", 7);
  // Ensure that the VM stack is completely empty (m_fp should be null)
  // and that we're not in a nested VM (reentrancy)
  assert(getFP() == nullptr);
  assert(m_nestedVMs.size() == 0);
  assert(m_nesting == 0);
  assert(m_stack.count() == 0);
  ActRec* ar = m_stack.allocA();
  ar->m_func = s_debuggerDummy->getMain();
  ar->initNumArgs(0);
  ar->setThis(nullptr);
  ar->m_soff = 0;
  ar->m_savedRbp = 0;
  ar->m_savedRip = reinterpret_cast<uintptr_t>(tx->uniqueStubs.callToExit);
  assert(isReturnHelper(ar->m_savedRip));
  m_fp = ar;
  m_pc = s_debuggerDummy->entry();
  m_firstAR = ar;
  m_fp->setVarEnv(m_globalVarEnv);
  m_globalVarEnv->attach(m_fp);
}

void ExecutionContext::exitDebuggerDummyEnv() {
  assert(m_globalVarEnv);
  // Ensure that m_fp is valid
  assert(getFP() != nullptr);
  // Ensure that m_fp points to the only frame on the call stack.
  // In other words, make sure there are no VM frames directly below
  // this one and that we are not in a nested VM (reentrancy)
  assert(m_fp->arGetSfp() == m_fp);
  assert(m_nestedVMs.size() == 0);
  assert(m_nesting == 0);
  // Teardown the frame we erected by enterDebuggerDummyEnv()
  const Func* func = m_fp->m_func;
  try {
    frame_free_locals_inl_no_hook<true>(m_fp, func->numLocals());
  } catch (...) {}
  m_stack.ndiscard(func->numSlotsInFrame());
  m_stack.discardAR();
  // After tearing down this frame, the VM stack should be completely empty
  assert(m_stack.count() == 0);
  m_fp = nullptr;
  m_pc = nullptr;
}

// Identifies the set of return helpers that we may set m_savedRip to in an
// ActRec.
bool ExecutionContext::isReturnHelper(uintptr_t address) {
  auto tcAddr = reinterpret_cast<JIT::TCA>(address);
  auto& u = tx->uniqueStubs;
  return tcAddr == u.retHelper ||
         tcAddr == u.genRetHelper ||
         tcAddr == u.retInlHelper ||
         tcAddr == u.callToExit;
}

// Walk the stack and find any return address to jitted code and bash it to
// the appropriate RetFromInterpreted*Frame helper. This ensures that we don't
// return into jitted code and gives the system the proper chance to interpret
// blacklisted tracelets.
void ExecutionContext::preventReturnsToTC() {
  assert(isDebuggerAttached());
  if (RuntimeOption::EvalJit) {
    ActRec *ar = getFP();
    while (ar) {
      if (!isReturnHelper(ar->m_savedRip) &&
          (mcg->isValidCodeAddress((JIT::TCA)ar->m_savedRip))) {
        TRACE_RB(2, "Replace RIP in fp %p, savedRip 0x%" PRIx64 ", "
                 "func %s\n", ar, ar->m_savedRip,
                 ar->m_func->fullName()->data());
        if (ar->inGenerator()) {
          ar->m_savedRip =
            reinterpret_cast<uintptr_t>(tx->uniqueStubs.genRetHelper);
        } else {
          ar->m_savedRip =
            reinterpret_cast<uintptr_t>(tx->uniqueStubs.retHelper);
        }
        assert(isReturnHelper(ar->m_savedRip));
      }
      ar = getPrevVMState(ar);
    }
  }
}

static inline StringData* lookup_name(TypedValue* key) {
  return prepareKey(*key);
}

static inline void lookup_var(ActRec* fp,
                              StringData*& name,
                              TypedValue* key,
                              TypedValue*& val) {
  name = lookup_name(key);
  const Func* func = fp->m_func;
  Id id = func->lookupVarId(name);
  if (id != kInvalidId) {
    val = frame_local(fp, id);
  } else {
    assert(!fp->hasInvName());
    if (fp->hasVarEnv()) {
      val = fp->m_varEnv->lookup(name);
    } else {
      val = nullptr;
    }
  }
}

static inline void lookupd_var(ActRec* fp,
                               StringData*& name,
                               TypedValue* key,
                               TypedValue*& val) {
  name = lookup_name(key);
  const Func* func = fp->m_func;
  Id id = func->lookupVarId(name);
  if (id != kInvalidId) {
    val = frame_local(fp, id);
  } else {
    assert(!fp->hasInvName());
    if (!fp->hasVarEnv()) {
      fp->setVarEnv(VarEnv::createLocal(fp));
    }
    val = fp->m_varEnv->lookup(name);
    if (val == nullptr) {
      TypedValue tv;
      tvWriteNull(&tv);
      fp->m_varEnv->set(name, &tv);
      val = fp->m_varEnv->lookup(name);
    }
  }
}

static inline void lookup_gbl(ActRec* fp,
                              StringData*& name,
                              TypedValue* key,
                              TypedValue*& val) {
  name = lookup_name(key);
  assert(g_context->m_globalVarEnv);
  val = g_context->m_globalVarEnv->lookup(name);
}

static inline void lookupd_gbl(ActRec* fp,
                               StringData*& name,
                               TypedValue* key,
                               TypedValue*& val) {
  name = lookup_name(key);
  assert(g_context->m_globalVarEnv);
  VarEnv* varEnv = g_context->m_globalVarEnv;
  val = varEnv->lookup(name);
  if (val == nullptr) {
    TypedValue tv;
    tvWriteNull(&tv);
    varEnv->set(name, &tv);
    val = varEnv->lookup(name);
  }
}

static inline void lookup_sprop(ActRec* fp,
                                TypedValue* clsRef,
                                StringData*& name,
                                TypedValue* key,
                                TypedValue*& val,
                                bool& visible,
                                bool& accessible) {
  assert(clsRef->m_type == KindOfClass);
  name = lookup_name(key);
  Class* ctx = arGetContextClass(fp);
  val = clsRef->m_data.pcls->getSProp(ctx, name, visible, accessible);
}

static inline void lookupClsRef(TypedValue* input,
                                TypedValue* output,
                                bool decRef = false) {
  const Class* class_ = nullptr;
  if (IS_STRING_TYPE(input->m_type)) {
    class_ = Unit::loadClass(input->m_data.pstr);
    if (class_ == nullptr) {
      output->m_type = KindOfNull;
      raise_error(Strings::UNKNOWN_CLASS, input->m_data.pstr->data());
    }
  } else if (input->m_type == KindOfObject) {
    class_ = input->m_data.pobj->getVMClass();
  } else {
    output->m_type = KindOfNull;
    raise_error("Cls: Expected string or object");
  }
  if (decRef) {
    tvRefcountedDecRef(input);
  }
  output->m_data.pcls = const_cast<Class*>(class_);
  output->m_type = KindOfClass;
}

static UNUSED int innerCount(const TypedValue* tv) {
  if (IS_REFCOUNTED_TYPE(tv->m_type)) {
    if (tv->m_type == KindOfRef) {
      return tv->m_data.pref->getRealCount();
    } else {
      return tv->m_data.pref->m_count;
    }
  }
  return -1;
}

static inline void ratchetRefs(TypedValue*& result, TypedValue& tvRef,
                               TypedValue& tvRef2) {
  TRACE(5, "Ratchet: result %p(k%d c%d), ref %p(k%d c%d) ref2 %p(k%d c%d)\n",
        result, result->m_type, innerCount(result),
        &tvRef, tvRef.m_type, innerCount(&tvRef),
        &tvRef2, tvRef2.m_type, innerCount(&tvRef2));
  // Due to complications associated with ArrayAccess, it is possible to acquire
  // a reference as a side effect of vector operation processing. Such a
  // reference must be retained until after the next iteration is complete.
  // Therefore, move the reference from tvRef to tvRef2, so that the reference
  // will be released one iteration later. But only do this if tvRef was used in
  // this iteration, otherwise we may wipe out the last reference to something
  // that we need to stay alive until the next iteration.
  if (tvRef.m_type != KindOfUninit) {
    if (IS_REFCOUNTED_TYPE(tvRef2.m_type)) {
      tvDecRef(&tvRef2);
      TRACE(5, "Ratchet: decref tvref2\n");
      tvWriteUninit(&tvRef2);
    }

    memcpy(&tvRef2, &tvRef, sizeof(TypedValue));
    tvWriteUninit(&tvRef);
    // Update result to point to relocated reference. This can be done
    // unconditionally here because we maintain the invariant throughout that
    // either tvRef is KindOfUninit, or tvRef contains a valid object that
    // result points to.
    assert(result == &tvRef);
    result = &tvRef2;
  }
}

#define DECLARE_MEMBERHELPER_ARGS               \
  unsigned ndiscard;                            \
  TypedValue* base;                             \
  TypedValue tvScratch;                         \
  TypedValue tvLiteral;                         \
  Variant tvRef;                                \
  Variant tvRef2;                               \
  MemberCode mcode = MEL;                       \
  TypedValue* curMember = 0;
#define DECLARE_SETHELPER_ARGS DECLARE_MEMBERHELPER_ARGS
#define DECLARE_GETHELPER_ARGS                  \
  DECLARE_MEMBERHELPER_ARGS                     \
  TypedValue* tvRet;

#define MEMBERHELPERPRE_ARGS                                           \
  pc, ndiscard, base, tvScratch, tvLiteral,                            \
    *tvRef.asTypedValue(), *tvRef2.asTypedValue(), mcode, curMember

#define MEMBERHELPERPRE_OUT                                            \
  pc, ndiscard, base, tvScratch, tvLiteral,                            \
    tvRef, tvRef2, mcode, curMember

// The following arguments are outputs:
// pc:         bytecode instruction after the vector instruction
// ndiscard:   number of stack elements to discard
// base:       ultimate result of the vector-get
// tvScratch:  temporary result storage
// tvRef:      temporary result storage
// tvRef2:     temporary result storage
// mcode:      output MemberCode for the last member if LeaveLast
// curMember:  output last member value one if LeaveLast; but undefined
//             if the last mcode == MW
//
// If saveResult is true, then upon completion of getHelperPre(),
// tvScratch contains a reference to the result (a duplicate of what
// base refers to).  getHelperPost<true>(...)  then saves the result
// to its final location.
template <bool warn,
          bool saveResult,
          ExecutionContext::VectorLeaveCode mleave>
OPTBLD_INLINE void ExecutionContext::getHelperPre(
    PC& pc,
    unsigned& ndiscard,
    TypedValue*& base,
    TypedValue& tvScratch,
    TypedValue& tvLiteral,
    TypedValue& tvRef,
    TypedValue& tvRef2,
    MemberCode& mcode,
    TypedValue*& curMember) {
  memberHelperPre<false, warn, false, false,
    false, 0, mleave, saveResult>(MEMBERHELPERPRE_OUT);
}

#define GETHELPERPOST_ARGS ndiscard, tvRet, tvScratch, tvRef, tvRef2
template <bool saveResult>
OPTBLD_INLINE void ExecutionContext::getHelperPost(
    unsigned ndiscard, TypedValue*& tvRet, TypedValue& tvScratch,
    Variant& tvRef, Variant& tvRef2) {
  // Clean up all ndiscard elements on the stack.  Actually discard
  // only ndiscard - 1, and overwrite the last cell with the result,
  // or if ndiscard is zero we actually need to allocate a cell.
  for (unsigned depth = 0; depth < ndiscard; ++depth) {
    TypedValue* tv = m_stack.indTV(depth);
    tvRefcountedDecRef(tv);
  }

  if (!ndiscard) {
    tvRet = m_stack.allocTV();
  } else {
    m_stack.ndiscard(ndiscard - 1);
    tvRet = m_stack.topTV();
  }

  if (saveResult) {
    // If tvRef wasn't just allocated, we've already decref'd it in
    // the loop above.
    tvCopy(tvScratch, *tvRet);
  }
}

#define GETHELPER_ARGS \
  pc, ndiscard, tvRet, base, tvScratch, tvLiteral, \
    tvRef, tvRef2, mcode, curMember
OPTBLD_INLINE void
ExecutionContext::getHelper(PC& pc,
                              unsigned& ndiscard,
                              TypedValue*& tvRet,
                              TypedValue*& base,
                              TypedValue& tvScratch,
                              TypedValue& tvLiteral,
                              Variant& tvRef,
                              Variant& tvRef2,
                              MemberCode& mcode,
                              TypedValue*& curMember) {
  getHelperPre<true, true, VectorLeaveCode::ConsumeAll>(MEMBERHELPERPRE_ARGS);
  getHelperPost<true>(GETHELPERPOST_ARGS);
}

template <bool setMember,
          bool warn,
          bool define,
          bool unset,
          bool reffy,
          unsigned mdepth, // extra args on stack for set (e.g. rhs)
          ExecutionContext::VectorLeaveCode mleave,
          bool saveResult>
OPTBLD_INLINE bool ExecutionContext::memberHelperPre(
    PC& pc, unsigned& ndiscard, TypedValue*& base,
    TypedValue& tvScratch, TypedValue& tvLiteral,
    TypedValue& tvRef, TypedValue& tvRef2,
    MemberCode& mcode, TypedValue*& curMember) {
  // The caller must move pc to the vector immediate before calling
  // {get, set}HelperPre.
  const ImmVector immVec = ImmVector::createFromStream(pc);
  const uint8_t* vec = immVec.vec();
  assert(immVec.size() > 0);

  // PC needs to be advanced before we do anything, otherwise if we
  // raise a notice in the middle of this we could resume at the wrong
  // instruction.
  pc += immVec.size() + sizeof(int32_t) + sizeof(int32_t);

  if (!setMember) {
    assert(mdepth == 0);
    assert(!define);
    assert(!unset);
  }

  ndiscard = immVec.numStackValues();
  int depth = mdepth + ndiscard - 1;
  const LocationCode lcode = LocationCode(*vec++);

  TypedValue* loc = nullptr;
  Class* const ctx = arGetContextClass(getFP());

  StringData* name;
  TypedValue* fr = nullptr;
  TypedValue* cref;
  TypedValue* pname;
  tvWriteUninit(&tvScratch);

  switch (lcode) {
  case LNL:
    loc = frame_local_inner(m_fp, decodeVariableSizeImm(&vec));
    goto lcodeName;
  case LNC:
    loc = m_stack.indTV(depth--);
    goto lcodeName;

  lcodeName:
    if (define) {
      lookupd_var(m_fp, name, loc, fr);
    } else {
      lookup_var(m_fp, name, loc, fr);
    }
    if (fr == nullptr) {
      if (warn) {
        raise_notice(Strings::UNDEFINED_VARIABLE, name->data());
      }
      tvWriteNull(&tvScratch);
      loc = &tvScratch;
    } else {
      loc = fr;
    }
    decRefStr(name);
    break;

  case LGL:
    loc = frame_local_inner(m_fp, decodeVariableSizeImm(&vec));
    goto lcodeGlobal;
  case LGC:
    loc = m_stack.indTV(depth--);
    goto lcodeGlobal;

  lcodeGlobal:
    if (define) {
      lookupd_gbl(m_fp, name, loc, fr);
    } else {
      lookup_gbl(m_fp, name, loc, fr);
    }
    if (fr == nullptr) {
      if (warn) {
        raise_notice(Strings::UNDEFINED_VARIABLE, name->data());
      }
      tvWriteNull(&tvScratch);
      loc = &tvScratch;
    } else {
      loc = fr;
    }
    decRefStr(name);
    break;

  case LSC:
    cref = m_stack.indTV(mdepth);
    pname = m_stack.indTV(depth--);
    goto lcodeSprop;
  case LSL:
    cref = m_stack.indTV(mdepth);
    pname = frame_local_inner(m_fp, decodeVariableSizeImm(&vec));
    goto lcodeSprop;

  lcodeSprop: {
    bool visible, accessible;
    assert(cref->m_type == KindOfClass);
    const Class* class_ = cref->m_data.pcls;
    StringData* name = lookup_name(pname);
    loc = class_->getSProp(ctx, name, visible, accessible);
    if (!(visible && accessible)) {
      raise_error("Invalid static property access: %s::%s",
                  class_->name()->data(),
                  name->data());
    }
    decRefStr(name);
    break;
  }

  case LL: {
    int localInd = decodeVariableSizeImm(&vec);
    loc = frame_local_inner(m_fp, localInd);
    if (warn) {
      if (loc->m_type == KindOfUninit) {
        raise_notice(Strings::UNDEFINED_VARIABLE,
                     m_fp->m_func->localVarName(localInd)->data());
      }
    }
    break;
  }
  case LC:
  case LR:
    loc = m_stack.indTV(depth--);
    break;
  case LH:
    assert(m_fp->hasThis());
    tvScratch.m_type = KindOfObject;
    tvScratch.m_data.pobj = m_fp->getThis();
    loc = &tvScratch;
    break;

  default: not_reached();
  }

  base = loc;
  tvWriteUninit(&tvLiteral);
  tvWriteUninit(&tvRef);
  tvWriteUninit(&tvRef2);

  // Iterate through the members.
  while (vec < pc) {
    mcode = MemberCode(*vec++);
    if (memberCodeHasImm(mcode)) {
      int64_t memberImm = decodeMemberCodeImm(&vec, mcode);
      if (memberCodeImmIsString(mcode)) {
        tvAsVariant(&tvLiteral) =
          m_fp->m_func->unit()->lookupLitstrId(memberImm);
        assert(!IS_REFCOUNTED_TYPE(tvLiteral.m_type));
        curMember = &tvLiteral;
      } else if (mcode == MEI) {
        tvAsVariant(&tvLiteral) = memberImm;
        curMember = &tvLiteral;
      } else {
        assert(memberCodeImmIsLoc(mcode));
        curMember = frame_local_inner(m_fp, memberImm);
      }
    } else {
      curMember = (setMember && mcode == MW) ? nullptr : m_stack.indTV(depth--);
    }

    if (mleave == VectorLeaveCode::LeaveLast) {
      if (vec >= pc) {
        assert(vec == pc);
        break;
      }
    }

    TypedValue* result;
    switch (mcode) {
    case MEL:
    case MEC:
    case MET:
    case MEI:
      if (unset) {
        result = ElemU(tvScratch, tvRef, base, *curMember);
      } else if (define) {
        result = ElemD<warn,reffy>(tvScratch, tvRef, base, *curMember);
      } else {
        result = Elem<warn>(tvScratch, tvRef, base, *curMember);
      }
      break;
    case MPL:
    case MPC:
    case MPT:
      result = Prop<warn, define, unset>(tvScratch, tvRef, ctx, base,
                                         *curMember);
      break;
    case MW:
      if (setMember) {
        assert(define);
        result = NewElem(tvScratch, tvRef, base);
      } else {
        raise_error("Cannot use [] for reading");
        result = nullptr;
      }
      break;
    default:
      assert(false);
      result = nullptr; // Silence compiler warning.
    }
    assert(result != nullptr);
    ratchetRefs(result, tvRef, tvRef2);
    // Check whether an error occurred (i.e. no result was set).
    if (setMember && result == &tvScratch && result->m_type == KindOfUninit) {
      return true;
    }
    base = result;
  }

  if (mleave == VectorLeaveCode::ConsumeAll) {
    assert(vec == pc);
    if (debug) {
      if (lcode == LSC || lcode == LSL) {
        assert(depth == int(mdepth));
      } else {
        assert(depth == int(mdepth) - 1);
      }
    }
  }

  if (saveResult) {
    assert(!setMember);
    // If requested, save a copy of the result.  If base already points to
    // tvScratch, no reference counting is necessary, because (with the
    // exception of the following block), tvScratch is never populated such
    // that it owns a reference that must be accounted for.
    if (base != &tvScratch) {
      // Acquire a reference to the result via tvDup(); base points to the
      // result but does not own a reference.
      tvDup(*base, tvScratch);
    }
  }

  return false;
}

// The following arguments are outputs:  (TODO put them in struct)
// pc:         bytecode instruction after the vector instruction
// ndiscard:   number of stack elements to discard
// base:       ultimate result of the vector-get
// tvScratch:  temporary result storage
// tvRef:      temporary result storage
// tvRef2:     temporary result storage
// mcode:      output MemberCode for the last member if LeaveLast
// curMember:  output last member value one if LeaveLast; but undefined
//             if the last mcode == MW
template <bool warn,
          bool define,
          bool unset,
          bool reffy,
          unsigned mdepth, // extra args on stack for set (e.g. rhs)
          ExecutionContext::VectorLeaveCode mleave>
OPTBLD_INLINE bool ExecutionContext::setHelperPre(
    PC& pc, unsigned& ndiscard, TypedValue*& base,
    TypedValue& tvScratch, TypedValue& tvLiteral,
    TypedValue& tvRef, TypedValue& tvRef2,
    MemberCode& mcode, TypedValue*& curMember) {
  return memberHelperPre<true, warn, define, unset,
    reffy, mdepth, mleave, false>(MEMBERHELPERPRE_OUT);
}

#define SETHELPERPOST_ARGS ndiscard, tvRef, tvRef2
template <unsigned mdepth>
OPTBLD_INLINE void ExecutionContext::setHelperPost(
    unsigned ndiscard, Variant& tvRef, Variant& tvRef2) {
  // Clean up the stack.  Decref all the elements for the vector, but
  // leave the first mdepth (they are not part of the vector data).
  for (unsigned depth = mdepth; depth-mdepth < ndiscard; ++depth) {
    TypedValue* tv = m_stack.indTV(depth);
    tvRefcountedDecRef(tv);
  }

  // NOTE: currently the only instructions using this that have return
  // values on the stack also have more inputs than the -vector, so
  // mdepth > 0.  They also always return the original top value of
  // the stack.
  if (mdepth > 0) {
    assert(mdepth == 1 &&
           "We don't really support mdepth > 1 in setHelperPost");

    if (ndiscard > 0) {
      TypedValue* retSrc = m_stack.topTV();
      TypedValue* dest = m_stack.indTV(ndiscard + mdepth - 1);
      assert(dest != retSrc);
      memcpy(dest, retSrc, sizeof *dest);
    }
  }

  m_stack.ndiscard(ndiscard);
}

OPTBLD_INLINE void ExecutionContext::iopLowInvalid(IOP_ARGS) {
  fprintf(stderr, "invalid bytecode executed\n");
  abort();
}

OPTBLD_INLINE void ExecutionContext::iopNop(IOP_ARGS) {
  NEXT();
}

OPTBLD_INLINE void ExecutionContext::iopPopA(IOP_ARGS) {
  NEXT();
  m_stack.popA();
}

OPTBLD_INLINE void ExecutionContext::iopPopC(IOP_ARGS) {
  NEXT();
  m_stack.popC();
}

OPTBLD_INLINE void ExecutionContext::iopPopV(IOP_ARGS) {
  NEXT();
  m_stack.popV();
}

OPTBLD_INLINE void ExecutionContext::iopPopR(IOP_ARGS) {
  NEXT();
  if (m_stack.topTV()->m_type != KindOfRef) {
    m_stack.popC();
  } else {
    m_stack.popV();
  }
}

OPTBLD_INLINE void ExecutionContext::iopDup(IOP_ARGS) {
  NEXT();
  m_stack.dup();
}

OPTBLD_INLINE void ExecutionContext::iopBox(IOP_ARGS) {
  NEXT();
  m_stack.box();
}

OPTBLD_INLINE void ExecutionContext::iopUnbox(IOP_ARGS) {
  NEXT();
  m_stack.unbox();
}

OPTBLD_INLINE void ExecutionContext::iopBoxR(IOP_ARGS) {
  NEXT();
  TypedValue* tv = m_stack.topTV();
  if (tv->m_type != KindOfRef) {
    tvBox(tv);
  }
}

OPTBLD_INLINE void ExecutionContext::iopBoxRNop(IOP_ARGS) {
  NEXT();
  assert(refIsPlausible(*m_stack.topTV()));
}

OPTBLD_INLINE void ExecutionContext::iopUnboxR(IOP_ARGS) {
  NEXT();
  if (m_stack.topTV()->m_type == KindOfRef) {
    m_stack.unbox();
  }
}

OPTBLD_INLINE void ExecutionContext::iopUnboxRNop(IOP_ARGS) {
  NEXT();
  assert(cellIsPlausible(*m_stack.topTV()));
}

OPTBLD_INLINE void ExecutionContext::iopNull(IOP_ARGS) {
  NEXT();
  m_stack.pushNull();
}

OPTBLD_INLINE void ExecutionContext::iopNullUninit(IOP_ARGS) {
  NEXT();
  m_stack.pushNullUninit();
}

OPTBLD_INLINE void ExecutionContext::iopTrue(IOP_ARGS) {
  NEXT();
  m_stack.pushTrue();
}

OPTBLD_INLINE void ExecutionContext::iopFalse(IOP_ARGS) {
  NEXT();
  m_stack.pushFalse();
}

OPTBLD_INLINE void ExecutionContext::iopFile(IOP_ARGS) {
  NEXT();
  const StringData* s = m_fp->m_func->unit()->filepath();
  m_stack.pushStaticString(const_cast<StringData*>(s));
}

OPTBLD_INLINE void ExecutionContext::iopDir(IOP_ARGS) {
  NEXT();
  const StringData* s = m_fp->m_func->unit()->dirpath();
  m_stack.pushStaticString(const_cast<StringData*>(s));
}

OPTBLD_INLINE void ExecutionContext::iopNameA(IOP_ARGS) {
  NEXT();
  auto const cls  = m_stack.topA();
  auto const name = cls->name();
  m_stack.popA();
  m_stack.pushStaticString(const_cast<StringData*>(name));
}

OPTBLD_INLINE void ExecutionContext::iopInt(IOP_ARGS) {
  NEXT();
  DECODE(int64_t, i);
  m_stack.pushInt(i);
}

OPTBLD_INLINE void ExecutionContext::iopDouble(IOP_ARGS) {
  NEXT();
  DECODE(double, d);
  m_stack.pushDouble(d);
}

OPTBLD_INLINE void ExecutionContext::iopString(IOP_ARGS) {
  NEXT();
  DECODE_LITSTR(s);
  m_stack.pushStaticString(s);
}

OPTBLD_INLINE void ExecutionContext::iopArray(IOP_ARGS) {
  NEXT();
  DECODE(Id, id);
  ArrayData* a = m_fp->m_func->unit()->lookupArrayId(id);
  m_stack.pushStaticArray(a);
}

OPTBLD_INLINE void ExecutionContext::iopNewArray(IOP_ARGS) {
  NEXT();
  DECODE_IVA(capacity);
  m_stack.pushArrayNoRc(HphpArray::MakeReserve(capacity));
}

OPTBLD_INLINE void ExecutionContext::iopNewPackedArray(IOP_ARGS) {
  NEXT();
  DECODE_IVA(n);
  // This constructor moves values, no inc/decref is necessary.
  auto* a = HphpArray::MakePacked(n, m_stack.topC());
  m_stack.ndiscard(n);
  m_stack.pushArrayNoRc(a);
}

OPTBLD_INLINE void ExecutionContext::iopNewStructArray(IOP_ARGS) {
  NEXT();
  DECODE(uint32_t, n); // number of keys and elements
  assert(n > 0 && n <= HphpArray::MaxMakeSize);
  StringData* names[HphpArray::MaxMakeSize];
  for (size_t i = 0; i < n; i++) {
    DECODE_LITSTR(s);
    names[i] = s;
  }
  // This constructor moves values, no inc/decref is necessary.
  auto* a = HphpArray::MakeStruct(n, names, m_stack.topC());
  m_stack.ndiscard(n);
  m_stack.pushArrayNoRc(a);
}

OPTBLD_INLINE void ExecutionContext::iopAddElemC(IOP_ARGS) {
  NEXT();
  Cell* c1 = m_stack.topC();
  Cell* c2 = m_stack.indC(1);
  Cell* c3 = m_stack.indC(2);
  if (c3->m_type != KindOfArray) {
    raise_error("AddElemC: $3 must be an array");
  }
  if (c2->m_type == KindOfInt64) {
    cellAsVariant(*c3).asArrRef().set(c2->m_data.num, tvAsCVarRef(c1));
  } else {
    cellAsVariant(*c3).asArrRef().set(tvAsCVarRef(c2), tvAsCVarRef(c1));
  }
  m_stack.popC();
  m_stack.popC();
}

OPTBLD_INLINE void ExecutionContext::iopAddElemV(IOP_ARGS) {
  NEXT();
  Ref* r1 = m_stack.topV();
  Cell* c2 = m_stack.indC(1);
  Cell* c3 = m_stack.indC(2);
  if (c3->m_type != KindOfArray) {
    raise_error("AddElemV: $3 must be an array");
  }
  if (c2->m_type == KindOfInt64) {
    cellAsVariant(*c3).asArrRef().set(c2->m_data.num, ref(tvAsCVarRef(r1)));
  } else {
    cellAsVariant(*c3).asArrRef().set(tvAsCVarRef(c2), ref(tvAsCVarRef(r1)));
  }
  m_stack.popV();
  m_stack.popC();
}

OPTBLD_INLINE void ExecutionContext::iopAddNewElemC(IOP_ARGS) {
  NEXT();
  Cell* c1 = m_stack.topC();
  Cell* c2 = m_stack.indC(1);
  if (c2->m_type != KindOfArray) {
    raise_error("AddNewElemC: $2 must be an array");
  }
  cellAsVariant(*c2).asArrRef().append(tvAsCVarRef(c1));
  m_stack.popC();
}

OPTBLD_INLINE void ExecutionContext::iopAddNewElemV(IOP_ARGS) {
  NEXT();
  Ref* r1 = m_stack.topV();
  Cell* c2 = m_stack.indC(1);
  if (c2->m_type != KindOfArray) {
    raise_error("AddNewElemV: $2 must be an array");
  }
  cellAsVariant(*c2).asArrRef().append(ref(tvAsCVarRef(r1)));
  m_stack.popV();
}

OPTBLD_INLINE void ExecutionContext::iopNewCol(IOP_ARGS) {
  NEXT();
  DECODE_IVA(cType);
  DECODE_IVA(nElms);
  ObjectData* obj = newCollectionHelper(cType, nElms);
  m_stack.pushObject(obj);
}

OPTBLD_INLINE void ExecutionContext::iopColAddNewElemC(IOP_ARGS) {
  NEXT();
  Cell* c1 = m_stack.topC();
  Cell* c2 = m_stack.indC(1);
  if (c2->m_type == KindOfObject && c2->m_data.pobj->isCollection()) {
    collectionInitAppend(c2->m_data.pobj, c1);
  } else {
    raise_error("ColAddNewElemC: $2 must be a collection");
  }
  m_stack.popC();
}

OPTBLD_INLINE void ExecutionContext::iopColAddElemC(IOP_ARGS) {
  NEXT();
  Cell* c1 = m_stack.topC();
  Cell* c2 = m_stack.indC(1);
  Cell* c3 = m_stack.indC(2);
  if (c3->m_type == KindOfObject && c3->m_data.pobj->isCollection()) {
    collectionInitSet(c3->m_data.pobj, c2, c1);
  } else {
    raise_error("ColAddElemC: $3 must be a collection");
  }
  m_stack.popC();
  m_stack.popC();
}

OPTBLD_INLINE void ExecutionContext::iopCns(IOP_ARGS) {
  NEXT();
  DECODE_LITSTR(s);
  TypedValue* cns = Unit::loadCns(s);
  if (cns == nullptr) {
    raise_notice(Strings::UNDEFINED_CONSTANT, s->data(), s->data());
    m_stack.pushStaticString(s);
    return;
  }
  auto const c1 = m_stack.allocC();
  cellDup(*cns, *c1);
}

OPTBLD_INLINE void ExecutionContext::iopCnsE(IOP_ARGS) {
  NEXT();
  DECODE_LITSTR(s);
  TypedValue* cns = Unit::loadCns(s);
  if (cns == nullptr) {
    raise_error("Undefined constant '%s'", s->data());
  }
  auto const c1 = m_stack.allocC();
  cellDup(*cns, *c1);
}

OPTBLD_INLINE void ExecutionContext::iopCnsU(IOP_ARGS) {
  NEXT();
  DECODE_LITSTR(name);
  DECODE_LITSTR(fallback);
  TypedValue* cns = Unit::loadCns(name);
  if (cns == nullptr) {
    cns = Unit::loadCns(fallback);
    if (cns == nullptr) {
      raise_notice(
        Strings::UNDEFINED_CONSTANT,
        fallback->data(),
        fallback->data()
      );
      m_stack.pushStaticString(fallback);
      return;
    }
  }
  auto const c1 = m_stack.allocC();
  cellDup(*cns, *c1);
}

OPTBLD_INLINE void ExecutionContext::iopDefCns(IOP_ARGS) {
  NEXT();
  DECODE_LITSTR(s);
  bool result = Unit::defCns(s, m_stack.topTV());
  m_stack.replaceTV<KindOfBoolean>(result);
}

OPTBLD_INLINE void ExecutionContext::iopClsCns(IOP_ARGS) {
  NEXT();
  DECODE_LITSTR(clsCnsName);

  auto const cls    = m_stack.topA();
  auto const clsCns = cls->clsCnsGet(clsCnsName);

  if (clsCns.m_type == KindOfUninit) {
    raise_error("Couldn't find constant %s::%s",
                cls->name()->data(), clsCnsName->data());
  }

  cellDup(clsCns, *m_stack.topTV());
}

OPTBLD_INLINE void ExecutionContext::iopClsCnsD(IOP_ARGS) {
  NEXT();
  DECODE_LITSTR(clsCnsName);
  DECODE(Id, classId);
  const NamedEntityPair& classNamedEntity =
    m_fp->m_func->unit()->lookupNamedEntityPairId(classId);

  auto const clsCns = lookupClsCns(classNamedEntity.second,
                                   classNamedEntity.first, clsCnsName);
  auto const c1 = m_stack.allocC();
  cellDup(clsCns, *c1);
}

OPTBLD_INLINE void ExecutionContext::iopConcat(IOP_ARGS) {
  NEXT();
  Cell* c1 = m_stack.topC();
  Cell* c2 = m_stack.indC(1);
  if (IS_STRING_TYPE(c1->m_type) && IS_STRING_TYPE(c2->m_type)) {
    cellAsVariant(*c2) = concat(
      cellAsVariant(*c2).toString(), cellAsCVarRef(*c1).toString());
  } else {
    cellAsVariant(*c2) = concat(cellAsVariant(*c2).toString(),
                                cellAsCVarRef(*c1).toString());
  }
  assert_refcount_realistic_nz(c2->m_data.pstr->getCount());
  m_stack.popC();
}

OPTBLD_INLINE void ExecutionContext::iopNot(IOP_ARGS) {
  NEXT();
  Cell* c1 = m_stack.topC();
  cellAsVariant(*c1) = !cellAsVariant(*c1).toBoolean();
}

OPTBLD_INLINE void ExecutionContext::iopAbs(IOP_ARGS) {
  NEXT();
  auto c1 = m_stack.topC();

  tvAsVariant(c1) = f_abs(tvAsCVarRef(c1));
}

template<class Op>
OPTBLD_INLINE void ExecutionContext::implCellBinOp(IOP_ARGS, Op op) {
  NEXT();
  auto const c1 = m_stack.topC();
  auto const c2 = m_stack.indC(1);
  auto const result = op(*c2, *c1);
  tvRefcountedDecRefCell(c2);
  *c2 = result;
  m_stack.popC();
}

template<class Op>
OPTBLD_INLINE void ExecutionContext::implCellBinOpBool(IOP_ARGS, Op op) {
  NEXT();
  auto const c1 = m_stack.topC();
  auto const c2 = m_stack.indC(1);
  bool const result = op(*c2, *c1);
  tvRefcountedDecRefCell(c2);
  *c2 = make_tv<KindOfBoolean>(result);
  m_stack.popC();
}

OPTBLD_INLINE void ExecutionContext::iopAdd(IOP_ARGS) {
  implCellBinOp(IOP_PASS_ARGS, cellAdd);
}

OPTBLD_INLINE void ExecutionContext::iopSub(IOP_ARGS) {
  implCellBinOp(IOP_PASS_ARGS, cellSub);
}

OPTBLD_INLINE void ExecutionContext::iopMul(IOP_ARGS) {
  implCellBinOp(IOP_PASS_ARGS, cellMul);
}

OPTBLD_INLINE void ExecutionContext::iopAddO(IOP_ARGS) {
  implCellBinOp(IOP_PASS_ARGS, cellAddO);
}

OPTBLD_INLINE void ExecutionContext::iopSubO(IOP_ARGS) {
  implCellBinOp(IOP_PASS_ARGS, cellSubO);
}

OPTBLD_INLINE void ExecutionContext::iopMulO(IOP_ARGS) {
  implCellBinOp(IOP_PASS_ARGS, cellMulO);
}

OPTBLD_INLINE void ExecutionContext::iopDiv(IOP_ARGS) {
  implCellBinOp(IOP_PASS_ARGS, cellDiv);
}

OPTBLD_INLINE void ExecutionContext::iopMod(IOP_ARGS) {
  implCellBinOp(IOP_PASS_ARGS, cellMod);
}

OPTBLD_INLINE void ExecutionContext::iopBitAnd(IOP_ARGS) {
  implCellBinOp(IOP_PASS_ARGS, cellBitAnd);
}

OPTBLD_INLINE void ExecutionContext::iopBitOr(IOP_ARGS) {
  implCellBinOp(IOP_PASS_ARGS, cellBitOr);
}

OPTBLD_INLINE void ExecutionContext::iopBitXor(IOP_ARGS) {
  implCellBinOp(IOP_PASS_ARGS, cellBitXor);
}

OPTBLD_INLINE void ExecutionContext::iopXor(IOP_ARGS) {
  implCellBinOpBool(IOP_PASS_ARGS, [&] (Cell c1, Cell c2) -> bool {
    return cellToBool(c1) ^ cellToBool(c2);
  });
}

OPTBLD_INLINE void ExecutionContext::iopSame(IOP_ARGS) {
  implCellBinOpBool(IOP_PASS_ARGS, cellSame);
}

OPTBLD_INLINE void ExecutionContext::iopNSame(IOP_ARGS) {
  implCellBinOpBool(IOP_PASS_ARGS, [&] (Cell c1, Cell c2) {
    return !cellSame(c1, c2);
  });
}

OPTBLD_INLINE void ExecutionContext::iopEq(IOP_ARGS) {
  implCellBinOpBool(IOP_PASS_ARGS, [&] (Cell c1, Cell c2) {
    return cellEqual(c1, c2);
  });
}

OPTBLD_INLINE void ExecutionContext::iopNeq(IOP_ARGS) {
  implCellBinOpBool(IOP_PASS_ARGS, [&] (Cell c1, Cell c2) {
    return !cellEqual(c1, c2);
  });
}

OPTBLD_INLINE void ExecutionContext::iopLt(IOP_ARGS) {
  implCellBinOpBool(IOP_PASS_ARGS, [&] (Cell c1, Cell c2) {
    return cellLess(c1, c2);
  });
}

OPTBLD_INLINE void ExecutionContext::iopLte(IOP_ARGS) {
  implCellBinOpBool(IOP_PASS_ARGS, cellLessOrEqual);
}

OPTBLD_INLINE void ExecutionContext::iopGt(IOP_ARGS) {
  implCellBinOpBool(IOP_PASS_ARGS, [&] (Cell c1, Cell c2) {
    return cellGreater(c1, c2);
  });
}

OPTBLD_INLINE void ExecutionContext::iopGte(IOP_ARGS) {
  implCellBinOpBool(IOP_PASS_ARGS, cellGreaterOrEqual);
}

OPTBLD_INLINE void ExecutionContext::iopShl(IOP_ARGS) {
  implCellBinOp(IOP_PASS_ARGS, [&] (Cell c1, Cell c2) {
    return make_tv<KindOfInt64>(cellToInt(c1) << cellToInt(c2));
  });
}

OPTBLD_INLINE void ExecutionContext::iopShr(IOP_ARGS) {
  implCellBinOp(IOP_PASS_ARGS, [&] (Cell c1, Cell c2) {
    return make_tv<KindOfInt64>(cellToInt(c1) >> cellToInt(c2));
  });
}

OPTBLD_INLINE void ExecutionContext::iopSqrt(IOP_ARGS) {
  NEXT();
  Cell* c1 = m_stack.topC();

  if (c1->m_type == KindOfNull || c1->m_type == KindOfBoolean ||
      (IS_STRING_TYPE(c1->m_type) && c1->m_data.pstr->isNumeric())) {
    tvCastToDoubleInPlace(c1);
  }

  if (c1->m_type == KindOfInt64) {
    c1->m_type = KindOfDouble;
    c1->m_data.dbl = f_sqrt(c1->m_data.num);
  } else if (c1->m_type == KindOfDouble) {
    c1->m_data.dbl = f_sqrt(c1->m_data.dbl);
  }

  if (c1->m_type != KindOfDouble) {
    raise_param_type_warning(
      "sqrt",
      1,
      KindOfDouble,
      c1->m_type
    );
    tvRefcountedDecRefCell(c1);
    c1->m_type = KindOfNull;
  }
}

OPTBLD_INLINE void ExecutionContext::iopBitNot(IOP_ARGS) {
  NEXT();
  cellBitNot(*m_stack.topC());
}

OPTBLD_INLINE void ExecutionContext::iopCastBool(IOP_ARGS) {
  NEXT();
  Cell* c1 = m_stack.topC();
  tvCastToBooleanInPlace(c1);
}

OPTBLD_INLINE void ExecutionContext::iopCastInt(IOP_ARGS) {
  NEXT();
  Cell* c1 = m_stack.topC();
  tvCastToInt64InPlace(c1);
}

OPTBLD_INLINE void ExecutionContext::iopCastDouble(IOP_ARGS) {
  NEXT();
  Cell* c1 = m_stack.topC();
  tvCastToDoubleInPlace(c1);
}

OPTBLD_INLINE void ExecutionContext::iopCastString(IOP_ARGS) {
  NEXT();
  Cell* c1 = m_stack.topC();
  tvCastToStringInPlace(c1);
}

OPTBLD_INLINE void ExecutionContext::iopCastArray(IOP_ARGS) {
  NEXT();
  Cell* c1 = m_stack.topC();
  tvCastToArrayInPlace(c1);
}

OPTBLD_INLINE void ExecutionContext::iopCastObject(IOP_ARGS) {
  NEXT();
  Cell* c1 = m_stack.topC();
  tvCastToObjectInPlace(c1);
}

OPTBLD_INLINE bool ExecutionContext::cellInstanceOf(
  TypedValue* tv, const NamedEntity* ne) {
  assert(tv->m_type != KindOfRef);
  Class* cls = nullptr;
  switch (tv->m_type) {
    case KindOfObject:
      cls = Unit::lookupClass(ne);
      if (cls) return tv->m_data.pobj->instanceof(cls);
      break;
    case KindOfArray:
      cls = Unit::lookupClass(ne);
      if (cls && interface_supports_array(cls->name())) {
        return true;
      }
      break;
    case KindOfString:
    case KindOfStaticString:
      cls = Unit::lookupClass(ne);
      if (cls && interface_supports_string(cls->name())) {
        return true;
      }
      break;
    case KindOfInt64:
      cls = Unit::lookupClass(ne);
      if (cls && interface_supports_int(cls->name())) {
        return true;
      }
      break;
    case KindOfDouble:
      cls = Unit::lookupClass(ne);
      if (cls && interface_supports_double(cls->name())) {
        return true;
      }
      break;
    default:
      return false;
  }
  return false;
}

ALWAYS_INLINE
bool ExecutionContext::iopInstanceOfHelper(const StringData* str1, Cell* c2) {
  const NamedEntity* rhs = Unit::GetNamedEntity(str1, false);
  // Because of other codepaths, an un-normalized name might enter the
  // table without a Class* so we need to check if it's there.
  if (LIKELY(rhs && rhs->getCachedClass() != nullptr)) {
    return cellInstanceOf(c2, rhs);
  }
  return false;
}

OPTBLD_INLINE void ExecutionContext::iopInstanceOf(IOP_ARGS) {
  NEXT();
  Cell* c1 = m_stack.topC();   // c2 instanceof c1
  Cell* c2 = m_stack.indC(1);
  bool r = false;
  if (IS_STRING_TYPE(c1->m_type)) {
    r = iopInstanceOfHelper(c1->m_data.pstr, c2);
  } else if (c1->m_type == KindOfObject) {
    if (c2->m_type == KindOfObject) {
      ObjectData* lhs = c2->m_data.pobj;
      ObjectData* rhs = c1->m_data.pobj;
      r = lhs->instanceof(rhs->getVMClass());
    }
  } else {
    raise_error("Class name must be a valid object or a string");
  }
  m_stack.popC();
  m_stack.replaceC<KindOfBoolean>(r);
}

OPTBLD_INLINE void ExecutionContext::iopInstanceOfD(IOP_ARGS) {
  NEXT();
  DECODE(Id, id);
  if (shouldProfile()) {
    InstanceBits::profile(m_fp->m_func->unit()->lookupLitstrId(id));
  }
  const NamedEntity* ne = m_fp->m_func->unit()->lookupNamedEntityId(id);
  Cell* c1 = m_stack.topC();
  bool r = cellInstanceOf(c1, ne);
  m_stack.replaceC<KindOfBoolean>(r);
}

OPTBLD_INLINE void ExecutionContext::iopPrint(IOP_ARGS) {
  NEXT();
  Cell* c1 = m_stack.topC();
  echo(cellAsVariant(*c1).toString());
  m_stack.replaceC<KindOfInt64>(1);
}

OPTBLD_INLINE void ExecutionContext::iopClone(IOP_ARGS) {
  NEXT();
  TypedValue* tv = m_stack.topTV();
  if (tv->m_type != KindOfObject) {
    raise_error("clone called on non-object");
  }
  ObjectData* obj = tv->m_data.pobj;
  const Class* class_ UNUSED = obj->getVMClass();
  ObjectData* newobj = obj->clone();
  m_stack.popTV();
  m_stack.pushNull();
  tv->m_type = KindOfObject;
  tv->m_data.pobj = newobj;
}

OPTBLD_INLINE void ExecutionContext::iopExit(IOP_ARGS) {
  NEXT();
  int exitCode = 0;
  Cell* c1 = m_stack.topC();
  if (c1->m_type == KindOfInt64) {
    exitCode = c1->m_data.num;
  } else {
    echo(cellAsVariant(*c1).toString());
  }
  m_stack.popC();
  m_stack.pushNull();
  throw ExitException(exitCode);
}

OPTBLD_INLINE void ExecutionContext::iopFatal(IOP_ARGS) {
  NEXT();
  TypedValue* top = m_stack.topTV();
  std::string msg;
  DECODE_OA(FatalOp, kind_char);
  if (IS_STRING_TYPE(top->m_type)) {
    msg = top->m_data.pstr->data();
  } else {
    msg = "Fatal error message not a string";
  }
  m_stack.popTV();

  switch (kind_char) {
  case FatalOp::RuntimeOmitFrame:
    raise_error_without_first_frame(msg);
    break;
  case FatalOp::Runtime:
  case FatalOp::Parse:
    raise_error(msg);
    break;
  }
}

OPTBLD_INLINE void ExecutionContext::jmpSurpriseCheck(Offset offset) {
  if (offset <= 0 && UNLIKELY(checkConditionFlags())) {
    EventHook::CheckSurprise();
  }
}

OPTBLD_INLINE void ExecutionContext::iopJmp(IOP_ARGS) {
  NEXT();
  DECODE_JMP(Offset, offset);
  jmpSurpriseCheck(offset);
  pc += offset - 1;
}

OPTBLD_INLINE void ExecutionContext::iopJmpNS(IOP_ARGS) {
  NEXT();
  DECODE_JMP(Offset, offset);
  pc += offset - 1;
}

template<Op op>
OPTBLD_INLINE void ExecutionContext::jmpOpImpl(IOP_ARGS) {
  static_assert(op == OpJmpZ || op == OpJmpNZ,
                "jmpOpImpl should only be used by JmpZ and JmpNZ");
  NEXT();
  DECODE_JMP(Offset, offset);
  jmpSurpriseCheck(offset);

  Cell* c1 = m_stack.topC();
  if (c1->m_type == KindOfInt64 || c1->m_type == KindOfBoolean) {
    int64_t n = c1->m_data.num;
    if (op == OpJmpZ ? n == 0 : n != 0) {
      pc += offset - 1;
      m_stack.popX();
    } else {
      pc += sizeof(Offset);
      m_stack.popX();
    }
  } else {
    auto const condition = toBoolean(cellAsCVarRef(*c1));
    if (op == OpJmpZ ? !condition : condition) {
      pc += offset - 1;
      m_stack.popC();
    } else {
      pc += sizeof(Offset);
      m_stack.popC();
    }
  }
}

OPTBLD_INLINE void ExecutionContext::iopJmpZ(IOP_ARGS) {
  jmpOpImpl<OpJmpZ>(IOP_PASS_ARGS);
}

OPTBLD_INLINE void ExecutionContext::iopJmpNZ(IOP_ARGS) {
  jmpOpImpl<OpJmpNZ>(IOP_PASS_ARGS);
}

#define FREE_ITER_LIST(typeList, idList, vecLen) do {           \
  int iterIndex;                                                \
  for (iterIndex = 0; iterIndex < 2 * veclen; iterIndex += 2) { \
    Id iterType = typeList[iterIndex];                          \
    Id iterId   = idList[iterIndex];                            \
                                                                \
    Iter *iter = frame_iter(m_fp, iterId);                      \
                                                                \
    switch (iterType) {                                         \
      case KindOfIter:  iter->free();  break;                   \
      case KindOfMIter: iter->mfree(); break;                   \
      case KindOfCIter: iter->cfree(); break;                   \
    }                                                           \
  }                                                             \
} while(0)

OPTBLD_INLINE void ExecutionContext::iopIterBreak(IOP_ARGS) {
  PC savedPc = pc;
  NEXT();
  DECODE_ITER_LIST(iterTypeList, iterIdList, veclen);
  DECODE_JMP(Offset, offset);

  FREE_ITER_LIST(iterTypeList, iterIdList, veclen);
  pc = savedPc + offset;
}

#undef FREE_ITER_LIST

enum class SwitchMatch {
  NORMAL,  // value was converted to an int: match normally
  NONZERO, // can't be converted to an int: match first nonzero case
  DEFAULT, // can't be converted to an int: match default case
};

static SwitchMatch doubleCheck(double d, int64_t& out) {
  if (int64_t(d) == d) {
    out = d;
    return SwitchMatch::NORMAL;
  } else {
    return SwitchMatch::DEFAULT;
  }
}

OPTBLD_INLINE void ExecutionContext::iopSwitch(IOP_ARGS) {
  PC origPC = pc;
  NEXT();
  DECODE(int32_t, veclen);
  assert(veclen > 0);
  Offset* jmptab = (Offset*)pc;
  pc += veclen * sizeof(*jmptab);
  DECODE(int64_t, base);
  DECODE_IVA(bounded);

  TypedValue* val = m_stack.topTV();
  if (!bounded) {
    assert(val->m_type == KindOfInt64);
    // Continuation switch: no bounds checking needed
    int64_t label = val->m_data.num;
    m_stack.popX();
    assert(label >= 0 && label < veclen);
    pc = origPC + jmptab[label];
  } else {
    // Generic integer switch
    int64_t intval;
    SwitchMatch match = SwitchMatch::NORMAL;

    switch (val->m_type) {
      case KindOfUninit:
      case KindOfNull:
        intval = 0;
        break;

      case KindOfBoolean:
        // bool(true) is equal to any non-zero int, bool(false) == 0
        if (val->m_data.num) {
          match = SwitchMatch::NONZERO;
        } else {
          intval = 0;
        }
        break;

      case KindOfInt64:
        intval = val->m_data.num;
        break;

      case KindOfDouble:
        match = doubleCheck(val->m_data.dbl, intval);
        break;

      case KindOfStaticString:
      case KindOfString: {
        double dval = 0.0;
        DataType t = val->m_data.pstr->isNumericWithVal(intval, dval, 1);
        switch (t) {
          case KindOfNull:
            intval = 0;
            break;

          case KindOfDouble:
            match = doubleCheck(dval, intval);
            break;

          case KindOfInt64:
            // do nothing
            break;

          default:
            not_reached();
        }
        tvRefcountedDecRef(val);
        break;
      }

      case KindOfArray:
        match = SwitchMatch::DEFAULT;
        tvDecRef(val);
        break;

      case KindOfObject:
        intval = val->m_data.pobj->o_toInt64();
        tvDecRef(val);
        break;

      case KindOfResource:
        intval = val->m_data.pres->o_toInt64();
        tvDecRef(val);
        break;

      default:
        not_reached();
    }
    m_stack.discard();

    if (match != SwitchMatch::NORMAL ||
        intval < base || intval >= (base + veclen - 2)) {
      switch (match) {
        case SwitchMatch::NORMAL:
        case SwitchMatch::DEFAULT:
          pc = origPC + jmptab[veclen - 1];
          break;

        case SwitchMatch::NONZERO:
          pc = origPC + jmptab[veclen - 2];
          break;
      }
    } else {
      pc = origPC + jmptab[intval - base];
    }
  }
}

OPTBLD_INLINE void ExecutionContext::iopSSwitch(IOP_ARGS) {
  PC origPC = pc;
  NEXT();
  DECODE(int32_t, veclen);
  assert(veclen > 1);
  unsigned cases = veclen - 1; // the last vector item is the default case
  StrVecItem* jmptab = (StrVecItem*)pc;
  pc += veclen * sizeof(*jmptab);

  Cell* val = tvToCell(m_stack.topTV());
  Unit* u = m_fp->m_func->unit();
  unsigned i;
  for (i = 0; i < cases; ++i) {
    auto& item = jmptab[i];
    const StringData* str = u->lookupLitstrId(item.str);
    if (cellEqual(*val, str)) {
      pc = origPC + item.dest;
      break;
    }
  }
  if (i == cases) {
    // default case
    pc = origPC + jmptab[veclen-1].dest;
  }
  m_stack.popC();
}

OPTBLD_INLINE void ExecutionContext::iopRetC(IOP_ARGS) {
  NEXT();
  uint soff = m_fp->m_soff;
  assert(!m_fp->inGenerator());

  // Call the runtime helpers to free the local variables and iterators
  frame_free_locals_inl(m_fp, m_fp->m_func->numLocals());
  ActRec* sfp = m_fp->arGetSfp();
  // Memcpy the the return value on top of the activation record. This works
  // the same regardless of whether the return value is boxed or not.
  TypedValue* retval_ptr = &m_fp->m_r;
  memcpy(retval_ptr, m_stack.topTV(), sizeof(TypedValue));
  if (RuntimeOption::EvalRuntimeTypeProfile) {
    profileOneArgument(*retval_ptr, -1, m_fp->m_func);
  }
  // Adjust the stack
  m_stack.ndiscard(m_fp->m_func->numSlotsInFrame() + 1);

  if (LIKELY(sfp != m_fp)) {
    // Restore caller's execution state.
    m_fp = sfp;
    pc = m_fp->m_func->unit()->entry() + m_fp->m_func->base() + soff;
    m_stack.ret();
    assert(m_stack.topTV() == retval_ptr);
  } else {
    // No caller; terminate.
    m_stack.ret();
#ifdef HPHP_TRACE
    {
      std::ostringstream os;
      os << toStringElm(m_stack.topTV());
      ONTRACE(1,
              Trace::trace("Return %s from ExecutionContext::dispatch("
                           "%p)\n", os.str().c_str(), m_fp));
    }
#endif
    pc = 0;
  }
}

OPTBLD_INLINE void ExecutionContext::iopRetV(IOP_ARGS) {
  iopRetC(IOP_PASS_ARGS);
}

OPTBLD_INLINE void ExecutionContext::iopUnwind(IOP_ARGS) {
  assert(!m_faults.empty());
  assert(m_faults.back().m_raiseOffset != kInvalidOffset);
  throw VMPrepareUnwind();
}

OPTBLD_INLINE void ExecutionContext::iopThrow(IOP_ARGS) {
  Cell* c1 = m_stack.topC();
  if (c1->m_type != KindOfObject ||
      !c1->m_data.pobj->instanceof(SystemLib::s_ExceptionClass)) {
    raise_error("Exceptions must be valid objects derived from the "
                "Exception base class");
  }

  Object obj(c1->m_data.pobj);
  m_stack.popC();
  DEBUGGER_ATTACHED_ONLY(phpDebuggerExceptionThrownHook(obj.get()));
  throw obj;
}

OPTBLD_INLINE void ExecutionContext::iopAGetC(IOP_ARGS) {
  NEXT();
  TypedValue* tv = m_stack.topTV();
  lookupClsRef(tv, tv, true);
}

OPTBLD_INLINE void ExecutionContext::iopAGetL(IOP_ARGS) {
  NEXT();
  DECODE_LA(local);
  TypedValue* top = m_stack.allocTV();
  TypedValue* fr = frame_local_inner(m_fp, local);
  lookupClsRef(fr, top);
}

static void raise_undefined_local(ActRec* fp, Id pind) {
  assert(pind < fp->m_func->numNamedLocals());
  raise_notice(Strings::UNDEFINED_VARIABLE,
               fp->m_func->localVarName(pind)->data());
}

static inline void cgetl_inner_body(TypedValue* fr, TypedValue* to) {
  assert(fr->m_type != KindOfUninit);
  cellDup(*tvToCell(fr), *to);
}

static inline void cgetl_body(ActRec* fp,
                              TypedValue* fr,
                              TypedValue* to,
                              Id pind) {
  if (fr->m_type == KindOfUninit) {
    // `to' is uninitialized here, so we need to tvWriteNull before
    // possibly causing stack unwinding.
    tvWriteNull(to);
    raise_undefined_local(fp, pind);
  } else {
    cgetl_inner_body(fr, to);
  }
}

OPTBLD_INLINE void ExecutionContext::iopCGetL(IOP_ARGS) {
  NEXT();
  DECODE_LA(local);
  Cell* to = m_stack.allocC();
  TypedValue* fr = frame_local(m_fp, local);
  cgetl_body(m_fp, fr, to, local);
}

OPTBLD_INLINE void ExecutionContext::iopCGetL2(IOP_ARGS) {
  NEXT();
  DECODE_LA(local);
  TypedValue* oldTop = m_stack.topTV();
  TypedValue* newTop = m_stack.allocTV();
  memcpy(newTop, oldTop, sizeof *newTop);
  Cell* to = oldTop;
  TypedValue* fr = frame_local(m_fp, local);
  cgetl_body(m_fp, fr, to, local);
}

OPTBLD_INLINE void ExecutionContext::iopCGetL3(IOP_ARGS) {
  NEXT();
  DECODE_LA(local);
  TypedValue* oldTop = m_stack.topTV();
  TypedValue* oldSubTop = m_stack.indTV(1);
  TypedValue* newTop = m_stack.allocTV();
  memmove(newTop, oldTop, sizeof *oldTop * 2);
  Cell* to = oldSubTop;
  TypedValue* fr = frame_local(m_fp, local);
  cgetl_body(m_fp, fr, to, local);
}

OPTBLD_INLINE void ExecutionContext::iopPushL(IOP_ARGS) {
  NEXT();
  DECODE_LA(local);
  TypedValue* locVal = frame_local(m_fp, local);
  assert(locVal->m_type != KindOfUninit);
  assert(locVal->m_type != KindOfRef);

  TypedValue* dest = m_stack.allocTV();
  *dest = *locVal;
  locVal->m_type = KindOfUninit;
}

OPTBLD_INLINE void ExecutionContext::iopCGetN(IOP_ARGS) {
  NEXT();
  StringData* name;
  TypedValue* to = m_stack.topTV();
  TypedValue* fr = nullptr;
  lookup_var(m_fp, name, to, fr);
  if (fr == nullptr || fr->m_type == KindOfUninit) {
    raise_notice(Strings::UNDEFINED_VARIABLE, name->data());
    tvRefcountedDecRefCell(to);
    tvWriteNull(to);
  } else {
    tvRefcountedDecRefCell(to);
    cgetl_inner_body(fr, to);
  }
  decRefStr(name); // TODO(#1146727): leaks during exceptions
}

OPTBLD_INLINE void ExecutionContext::iopCGetG(IOP_ARGS) {
  NEXT();
  StringData* name;
  TypedValue* to = m_stack.topTV();
  TypedValue* fr = nullptr;
  lookup_gbl(m_fp, name, to, fr);
  if (fr == nullptr) {
    if (MoreWarnings) {
      raise_notice(Strings::UNDEFINED_VARIABLE, name->data());
    }
    tvRefcountedDecRefCell(to);
    tvWriteNull(to);
  } else if (fr->m_type == KindOfUninit) {
    raise_notice(Strings::UNDEFINED_VARIABLE, name->data());
    tvRefcountedDecRefCell(to);
    tvWriteNull(to);
  } else {
    tvRefcountedDecRefCell(to);
    cgetl_inner_body(fr, to);
  }
  decRefStr(name); // TODO(#1146727): leaks during exceptions
}

#define SPROP_OP_PRELUDE                                  \
  NEXT();                                                 \
  TypedValue* clsref = m_stack.topTV();                   \
  TypedValue* nameCell = m_stack.indTV(1);                \
  TypedValue* output = nameCell;                          \
  TypedValue* val;                                        \
  bool visible, accessible;                               \
  lookup_sprop(m_fp, clsref, name, nameCell, val, visible, \
               accessible);

#define SPROP_OP_POSTLUDE                     \
  decRefStr(name);

#define GETS(box) do {                                    \
  SPROP_OP_PRELUDE                                        \
  if (!(visible && accessible)) {                         \
    raise_error("Invalid static property access: %s::%s", \
                clsref->m_data.pcls->name()->data(),      \
                name->data());                            \
  }                                                       \
  if (box) {                                              \
    if (val->m_type != KindOfRef) {                       \
      tvBox(val);                                         \
    }                                                     \
    refDup(*val, *output);                                \
  } else {                                                \
    cellDup(*tvToCell(val), *output);                     \
  }                                                       \
  m_stack.popA();                                         \
  SPROP_OP_POSTLUDE                                       \
} while (0)

OPTBLD_INLINE void ExecutionContext::iopCGetS(IOP_ARGS) {
  StringData* name;
  GETS(false);
  if (shouldProfile() && name && name->isStatic()) {
    recordType(TypeProfileKey(TypeProfileKey::StaticPropName, name),
               m_stack.top()->m_type);
  }
}

OPTBLD_INLINE void ExecutionContext::iopCGetM(IOP_ARGS) {
  PC oldPC = pc;
  NEXT();
  DECLARE_GETHELPER_ARGS
  getHelper(GETHELPER_ARGS);
  if (tvRet->m_type == KindOfRef) {
    tvUnbox(tvRet);
  }
  assert(hasImmVector(*reinterpret_cast<const Op*>(oldPC)));
  const ImmVector& immVec = ImmVector::createFromStream(oldPC + 1);
  StringData* name;
  MemberCode mc;
  if (immVec.decodeLastMember(m_fp->unit(), name, mc)) {
    recordType(TypeProfileKey(mc, name), m_stack.top()->m_type);
  }
}

static inline void vgetl_body(TypedValue* fr, TypedValue* to) {
  if (fr->m_type != KindOfRef) {
    tvBox(fr);
  }
  refDup(*fr, *to);
}

OPTBLD_INLINE void ExecutionContext::iopVGetL(IOP_ARGS) {
  NEXT();
  DECODE_LA(local);
  Ref* to = m_stack.allocV();
  TypedValue* fr = frame_local(m_fp, local);
  vgetl_body(fr, to);
}

OPTBLD_INLINE void ExecutionContext::iopVGetN(IOP_ARGS) {
  NEXT();
  StringData* name;
  TypedValue* to = m_stack.topTV();
  TypedValue* fr = nullptr;
  lookupd_var(m_fp, name, to, fr);
  assert(fr != nullptr);
  tvRefcountedDecRefCell(to);
  vgetl_body(fr, to);
  decRefStr(name);
}

OPTBLD_INLINE void ExecutionContext::iopVGetG(IOP_ARGS) {
  NEXT();
  StringData* name;
  TypedValue* to = m_stack.topTV();
  TypedValue* fr = nullptr;
  lookupd_gbl(m_fp, name, to, fr);
  assert(fr != nullptr);
  tvRefcountedDecRefCell(to);
  vgetl_body(fr, to);
  decRefStr(name);
}

OPTBLD_INLINE void ExecutionContext::iopVGetS(IOP_ARGS) {
  StringData* name;
  GETS(true);
}
#undef GETS

OPTBLD_INLINE void ExecutionContext::iopVGetM(IOP_ARGS) {
  NEXT();
  DECLARE_SETHELPER_ARGS
  TypedValue* tv1 = m_stack.allocTV();
  tvWriteUninit(tv1);
  if (!setHelperPre<false, true, false, true, 1,
      VectorLeaveCode::ConsumeAll>(MEMBERHELPERPRE_ARGS)) {
    if (base->m_type != KindOfRef) {
      tvBox(base);
    }
    refDup(*base, *tv1);
  } else {
    tvWriteNull(tv1);
    tvBox(tv1);
  }
  setHelperPost<1>(SETHELPERPOST_ARGS);
}

OPTBLD_INLINE void ExecutionContext::iopIssetN(IOP_ARGS) {
  NEXT();
  StringData* name;
  TypedValue* tv1 = m_stack.topTV();
  TypedValue* tv = nullptr;
  bool e;
  lookup_var(m_fp, name, tv1, tv);
  if (tv == nullptr) {
    e = false;
  } else {
    e = !cellIsNull(tvToCell(tv));
  }
  m_stack.replaceC<KindOfBoolean>(e);
  decRefStr(name);
}

OPTBLD_INLINE void ExecutionContext::iopIssetG(IOP_ARGS) {
  NEXT();
  StringData* name;
  TypedValue* tv1 = m_stack.topTV();
  TypedValue* tv = nullptr;
  bool e;
  lookup_gbl(m_fp, name, tv1, tv);
  if (tv == nullptr) {
    e = false;
  } else {
    e = !cellIsNull(tvToCell(tv));
  }
  m_stack.replaceC<KindOfBoolean>(e);
  decRefStr(name);
}

OPTBLD_INLINE void ExecutionContext::iopIssetS(IOP_ARGS) {
  StringData* name;
  SPROP_OP_PRELUDE
  bool e;
  if (!(visible && accessible)) {
    e = false;
  } else {
    e = !cellIsNull(tvToCell(val));
  }
  m_stack.popA();
  output->m_data.num = e;
  output->m_type = KindOfBoolean;
  SPROP_OP_POSTLUDE
}

template <bool isEmpty>
OPTBLD_INLINE void ExecutionContext::isSetEmptyM(IOP_ARGS) {
  NEXT();
  DECLARE_GETHELPER_ARGS
  getHelperPre<false, false, VectorLeaveCode::LeaveLast>(MEMBERHELPERPRE_ARGS);
  // Process last member specially, in order to employ the IssetElem/IssetProp
  // operations.
  bool isSetEmptyResult = false;
  switch (mcode) {
  case MEL:
  case MEC:
  case MET:
  case MEI: {
    isSetEmptyResult = IssetEmptyElem<isEmpty>(tvScratch, *tvRef.asTypedValue(),
        base, *curMember);
    break;
  }
  case MPL:
  case MPC:
  case MPT: {
    Class* ctx = arGetContextClass(m_fp);
    isSetEmptyResult = IssetEmptyProp<isEmpty>(ctx, base, *curMember);
    break;
  }
  default: assert(false);
  }
  getHelperPost<false>(GETHELPERPOST_ARGS);
  tvRet->m_data.num = isSetEmptyResult;
  tvRet->m_type = KindOfBoolean;
}

OPTBLD_INLINE void ExecutionContext::iopIssetM(IOP_ARGS) {
  isSetEmptyM<false>(IOP_PASS_ARGS);
}

OPTBLD_INLINE void ExecutionContext::iopIssetL(IOP_ARGS) {
  NEXT();
  DECODE_LA(local);
  TypedValue* tv = frame_local(m_fp, local);
  bool ret = is_not_null(tvAsCVarRef(tv));
  TypedValue* topTv = m_stack.allocTV();
  topTv->m_data.num = ret;
  topTv->m_type = KindOfBoolean;
}

OPTBLD_INLINE static bool isTypeHelper(TypedValue* tv, IsTypeOp op) {
  switch (op) {
  case IsTypeOp::Null:   return is_null(tvAsCVarRef(tv));
  case IsTypeOp::Bool:   return is_bool(tvAsCVarRef(tv));
  case IsTypeOp::Int:    return is_int(tvAsCVarRef(tv));
  case IsTypeOp::Dbl:    return is_double(tvAsCVarRef(tv));
  case IsTypeOp::Arr:    return is_array(tvAsCVarRef(tv));
  case IsTypeOp::Obj:    return is_object(tvAsCVarRef(tv));
  case IsTypeOp::Str:    return is_string(tvAsCVarRef(tv));
  case IsTypeOp::Scalar: return HHVM_FN(is_scalar)(tvAsCVarRef(tv));
  }
  not_reached();
}

OPTBLD_INLINE void ExecutionContext::iopIsTypeL(IOP_ARGS) {
  NEXT();
  DECODE_LA(local);
  DECODE_OA(IsTypeOp, op);
  TypedValue* tv = frame_local(m_fp, local);
  if (tv->m_type == KindOfUninit) {
    raise_undefined_local(m_fp, local);
  }
  TypedValue* topTv = m_stack.allocTV();
  topTv->m_data.num = isTypeHelper(tv, op);
  topTv->m_type = KindOfBoolean;
}

OPTBLD_INLINE void ExecutionContext::iopIsTypeC(IOP_ARGS) {
  NEXT();
  DECODE_OA(IsTypeOp, op);
  TypedValue* topTv = m_stack.topTV();
  assert(topTv->m_type != KindOfRef);
  bool ret = isTypeHelper(topTv, op);
  tvRefcountedDecRefCell(topTv);
  topTv->m_data.num = ret;
  topTv->m_type = KindOfBoolean;
}

OPTBLD_INLINE static void implAssertT(TypedValue* tv, AssertTOp op) {
  switch (op) {
  case AssertTOp::Uninit:
    assert(tv->m_type == KindOfUninit);
    break;
  case AssertTOp::InitNull:
    assert(tv->m_type == KindOfNull);
    break;
  case AssertTOp::Null:
    assert(IS_NULL_TYPE(tv->m_type));
    break;
  case AssertTOp::Int:
    assert(tv->m_type == KindOfInt64);
    break;
  case AssertTOp::OptInt:
    assert(tv->m_type == KindOfNull || tv->m_type == KindOfInt64);
    break;
  case AssertTOp::Dbl:
    assert(tv->m_type == KindOfDouble);
    break;
  case AssertTOp::OptDbl:
    assert(tv->m_type == KindOfNull || tv->m_type == KindOfDouble);
    break;
  case AssertTOp::Bool:
    assert(tv->m_type == KindOfBoolean);
    break;
  case AssertTOp::OptBool:
    assert(tv->m_type == KindOfNull || tv->m_type == KindOfBoolean);
    break;
  case AssertTOp::Res:
    assert(tv->m_type == KindOfResource);
    break;
  case AssertTOp::OptRes:
    assert(tv->m_type == KindOfNull || tv->m_type == KindOfResource);
    break;
  case AssertTOp::SStr:
    assert(IS_STRING_TYPE(tv->m_type) && tv->m_data.pstr->isStatic());
    break;
  case AssertTOp::OptSStr:
    assert(tv->m_type == KindOfNull ||
           (IS_STRING_TYPE(tv->m_type) && tv->m_data.pstr->isStatic()));
    break;
  case AssertTOp::Str:
    assert(IS_STRING_TYPE(tv->m_type));
    break;
  case AssertTOp::OptStr:
    assert(tv->m_type == KindOfNull || IS_STRING_TYPE(tv->m_type));
    break;
  case AssertTOp::SArr:
    assert(tv->m_type == KindOfArray && tv->m_data.parr->isStatic());
    break;
  case AssertTOp::OptSArr:
    assert(tv->m_type == KindOfNull ||
           (tv->m_type == KindOfArray && tv->m_data.parr->isStatic()));
    break;
  case AssertTOp::Arr:
    assert(tv->m_type == KindOfArray);
    break;
  case AssertTOp::OptArr:
    assert(tv->m_type == KindOfNull || tv->m_type == KindOfArray);
    break;
  case AssertTOp::Obj:
    assert(tv->m_type == KindOfObject);
    break;
  case AssertTOp::OptObj:
    assert(tv->m_type == KindOfNull || tv->m_type == KindOfObject);
    break;
  case AssertTOp::InitUnc:
    assert(tv->m_type != KindOfUninit);
    /* fallthrough */
  case AssertTOp::Unc:
    assert(
      !IS_REFCOUNTED_TYPE(tv->m_type) ||
      (tv->m_type == KindOfString && tv->m_data.pstr->isStatic()) ||
      (tv->m_type == KindOfArray  && tv->m_data.parr->isStatic())
    );
    break;
  case AssertTOp::InitCell:
    assert(tv->m_type != KindOfUninit && tv->m_type != KindOfRef);
    break;
  case AssertTOp::Cell:
    assert(tv->m_type != KindOfRef);
    break;
  case AssertTOp::Ref:
    assert(tv->m_type == KindOfRef);
    break;
  }
}

OPTBLD_INLINE void ExecutionContext::iopAssertTL(IOP_ARGS) {
  NEXT();
  DECODE_LA(localId);
  DECODE_OA(AssertTOp, op);
  implAssertT(frame_local(m_fp, localId), op);
}

OPTBLD_INLINE void ExecutionContext::iopAssertTStk(IOP_ARGS) {
  NEXT();
  DECODE_IVA(stkSlot);
  DECODE_OA(AssertTOp, op);
  implAssertT(m_stack.indTV(stkSlot), op);
}

OPTBLD_INLINE void ExecutionContext::iopPredictTL(IOP_ARGS) {
  NEXT();
  DECODE_LA(localId);
  DECODE_OA(AssertTOp, op);
}

OPTBLD_INLINE void ExecutionContext::iopPredictTStk(IOP_ARGS) {
  NEXT();
  DECODE_IVA(stkSlot);
  DECODE_OA(AssertTOp, op);
}

OPTBLD_INLINE static void implAssertObj(TypedValue* tv,
                                        const StringData* str,
                                        AssertObjOp subop) {
  DEBUG_ONLY auto const cls = Unit::lookupClass(str);
  auto cls_defined = [&] {
    assert(cls && "asserted class was not defined at AssertObj{L,Stk}");
  };

  switch (subop) {
  case AssertObjOp::Exact:
    assert(tv->m_type == KindOfObject);
    cls_defined();
    assert(tv->m_data.pobj->getVMClass() == cls);
    return;
  case AssertObjOp::Sub:
    assert(tv->m_type == KindOfObject);
    cls_defined();
    assert(tv->m_data.pobj->getVMClass()->classof(cls));
    return;
  case AssertObjOp::OptExact:
    assert(tv->m_type == KindOfObject || tv->m_type == KindOfNull);
    if (tv->m_type == KindOfObject) {
      cls_defined();
      assert(tv->m_data.pobj->getVMClass() == cls);
    }
    return;
  case AssertObjOp::OptSub:
    assert(tv->m_type == KindOfObject || tv->m_type == KindOfNull);
    if (tv->m_type == KindOfObject) {
      cls_defined();
      assert(tv->m_data.pobj->getVMClass()->classof(cls));
    }
    return;
  }
  not_reached();
}

OPTBLD_INLINE void ExecutionContext::iopAssertObjL(IOP_ARGS) {
  NEXT();
  DECODE_LA(localId);
  DECODE(Id, strId);
  DECODE_OA(AssertObjOp, subop);
  auto const str = m_fp->m_func->unit()->lookupLitstrId(strId);
  implAssertObj(frame_local(m_fp, localId), str, subop);
}

OPTBLD_INLINE void ExecutionContext::iopAssertObjStk(IOP_ARGS) {
  NEXT();
  DECODE_IVA(stkSlot);
  DECODE(Id, strId);
  DECODE_OA(AssertObjOp, subop);
  auto const str = m_fp->m_func->unit()->lookupLitstrId(strId);
  implAssertObj(m_stack.indTV(stkSlot), str, subop);
}

OPTBLD_INLINE void ExecutionContext::iopBreakTraceHint(IOP_ARGS) {
  NEXT();
}

OPTBLD_INLINE void ExecutionContext::iopEmptyL(IOP_ARGS) {
  NEXT();
  DECODE_LA(local);
  TypedValue* loc = frame_local(m_fp, local);
  bool e = !cellToBool(*tvToCell(loc));
  m_stack.pushBool(e);
}

OPTBLD_INLINE void ExecutionContext::iopEmptyN(IOP_ARGS) {
  NEXT();
  StringData* name;
  TypedValue* tv1 = m_stack.topTV();
  TypedValue* tv = nullptr;
  bool e;
  lookup_var(m_fp, name, tv1, tv);
  if (tv == nullptr) {
    e = true;
  } else {
    e = !cellToBool(*tvToCell(tv));
  }
  m_stack.replaceC<KindOfBoolean>(e);
  decRefStr(name);
}

OPTBLD_INLINE void ExecutionContext::iopEmptyG(IOP_ARGS) {
  NEXT();
  StringData* name;
  TypedValue* tv1 = m_stack.topTV();
  TypedValue* tv = nullptr;
  bool e;
  lookup_gbl(m_fp, name, tv1, tv);
  if (tv == nullptr) {
    e = true;
  } else {
    e = !cellToBool(*tvToCell(tv));
  }
  m_stack.replaceC<KindOfBoolean>(e);
  decRefStr(name);
}

OPTBLD_INLINE void ExecutionContext::iopEmptyS(IOP_ARGS) {
  StringData* name;
  SPROP_OP_PRELUDE
  bool e;
  if (!(visible && accessible)) {
    e = true;
  } else {
    e = !cellToBool(*tvToCell(val));
  }
  m_stack.popA();
  output->m_data.num = e;
  output->m_type = KindOfBoolean;
  SPROP_OP_POSTLUDE
}

OPTBLD_INLINE void ExecutionContext::iopEmptyM(IOP_ARGS) {
  isSetEmptyM<true>(IOP_PASS_ARGS);
}

OPTBLD_INLINE void ExecutionContext::iopAKExists(IOP_ARGS) {
  NEXT();
  TypedValue* arr = m_stack.topTV();
  TypedValue* key = arr + 1;
  bool result = f_array_key_exists(tvAsCVarRef(key), tvAsCVarRef(arr));
  m_stack.popTV();
  m_stack.replaceTV<KindOfBoolean>(result);
}

OPTBLD_INLINE void ExecutionContext::iopIdx(IOP_ARGS) {
  NEXT();
  TypedValue* def = m_stack.topTV();
  TypedValue* key = m_stack.indTV(1);
  TypedValue* arr = m_stack.indTV(2);

  TypedValue result = JIT::genericIdx(*arr, *key, *def);
  m_stack.popTV();
  m_stack.popTV();
  tvRefcountedDecRef(arr);
  *arr = result;
}

OPTBLD_INLINE void ExecutionContext::iopArrayIdx(IOP_ARGS) {
  NEXT();
  TypedValue* def = m_stack.topTV();
  TypedValue* key = m_stack.indTV(1);
  TypedValue* arr = m_stack.indTV(2);

  Variant result = f_hphp_array_idx(tvAsCVarRef(arr),
                                    tvAsCVarRef(key),
                                    tvAsCVarRef(def));
  m_stack.popTV();
  m_stack.popTV();
  tvAsVariant(arr) = result;
}

OPTBLD_INLINE void ExecutionContext::iopSetL(IOP_ARGS) {
  NEXT();
  DECODE_LA(local);
  assert(local < m_fp->m_func->numLocals());
  Cell* fr = m_stack.topC();
  TypedValue* to = frame_local(m_fp, local);
  tvSet(*fr, *to);
}

OPTBLD_INLINE void ExecutionContext::iopSetN(IOP_ARGS) {
  NEXT();
  StringData* name;
  Cell* fr = m_stack.topC();
  TypedValue* tv2 = m_stack.indTV(1);
  TypedValue* to = nullptr;
  lookupd_var(m_fp, name, tv2, to);
  assert(to != nullptr);
  tvSet(*fr, *to);
  memcpy((void*)tv2, (void*)fr, sizeof(TypedValue));
  m_stack.discard();
  decRefStr(name);
}

OPTBLD_INLINE void ExecutionContext::iopSetG(IOP_ARGS) {
  NEXT();
  StringData* name;
  Cell* fr = m_stack.topC();
  TypedValue* tv2 = m_stack.indTV(1);
  TypedValue* to = nullptr;
  lookupd_gbl(m_fp, name, tv2, to);
  assert(to != nullptr);
  tvSet(*fr, *to);
  memcpy((void*)tv2, (void*)fr, sizeof(TypedValue));
  m_stack.discard();
  decRefStr(name);
}

OPTBLD_INLINE void ExecutionContext::iopSetS(IOP_ARGS) {
  NEXT();
  TypedValue* tv1 = m_stack.topTV();
  TypedValue* classref = m_stack.indTV(1);
  TypedValue* propn = m_stack.indTV(2);
  TypedValue* output = propn;
  StringData* name;
  TypedValue* val;
  bool visible, accessible;
  lookup_sprop(m_fp, classref, name, propn, val, visible, accessible);
  if (!(visible && accessible)) {
    raise_error("Invalid static property access: %s::%s",
                classref->m_data.pcls->name()->data(),
                name->data());
  }
  tvSet(*tv1, *val);
  tvRefcountedDecRefCell(propn);
  memcpy(output, tv1, sizeof(TypedValue));
  m_stack.ndiscard(2);
  decRefStr(name);
}

OPTBLD_INLINE void ExecutionContext::iopSetM(IOP_ARGS) {
  NEXT();
  DECLARE_SETHELPER_ARGS
  if (!setHelperPre<false, true, false, false, 1,
      VectorLeaveCode::LeaveLast>(MEMBERHELPERPRE_ARGS)) {
    Cell* c1 = m_stack.topC();

    if (mcode == MW) {
      SetNewElem<true>(base, c1);
    } else {
      switch (mcode) {
      case MEL:
      case MEC:
      case MET:
      case MEI: {
        StringData* result = SetElem<true>(base, *curMember, c1);
        if (result) {
          tvRefcountedDecRefCell(c1);
          c1->m_type = KindOfString;
          c1->m_data.pstr = result;
        }
        break;
      }
      case MPL:
      case MPC:
      case MPT: {
        Class* ctx = arGetContextClass(m_fp);
        SetProp<true>(ctx, base, *curMember, c1);
        break;
      }
      default: assert(false);
      }
    }
  }
  setHelperPost<1>(SETHELPERPOST_ARGS);
}

OPTBLD_INLINE void ExecutionContext::iopSetWithRefLM(IOP_ARGS) {
  NEXT();
  DECLARE_SETHELPER_ARGS
  bool skip = setHelperPre<false, true, false, false, 0,
                           VectorLeaveCode::ConsumeAll>(MEMBERHELPERPRE_ARGS);
  DECODE_LA(local);
  if (!skip) {
    TypedValue* from = frame_local(m_fp, local);
    tvAsVariant(base) = withRefBind(tvAsVariant(from));
  }
  setHelperPost<0>(SETHELPERPOST_ARGS);
}

OPTBLD_INLINE void ExecutionContext::iopSetWithRefRM(IOP_ARGS) {
  NEXT();
  DECLARE_SETHELPER_ARGS
  bool skip = setHelperPre<false, true, false, false, 1,
                           VectorLeaveCode::ConsumeAll>(MEMBERHELPERPRE_ARGS);
  if (!skip) {
    TypedValue* from = m_stack.top();
    tvAsVariant(base) = withRefBind(tvAsVariant(from));
  }
  setHelperPost<0>(SETHELPERPOST_ARGS);
  m_stack.popTV();
}

OPTBLD_INLINE void ExecutionContext::iopSetOpL(IOP_ARGS) {
  NEXT();
  DECODE_LA(local);
  DECODE_OA(SetOpOp, op);
  Cell* fr = m_stack.topC();
  Cell* to = tvToCell(frame_local(m_fp, local));
  SETOP_BODY_CELL(to, op, fr);
  tvRefcountedDecRefCell(fr);
  cellDup(*to, *fr);
}

OPTBLD_INLINE void ExecutionContext::iopSetOpN(IOP_ARGS) {
  NEXT();
  DECODE_OA(SetOpOp, op);
  StringData* name;
  Cell* fr = m_stack.topC();
  TypedValue* tv2 = m_stack.indTV(1);
  TypedValue* to = nullptr;
  // XXX We're probably not getting warnings totally correct here
  lookupd_var(m_fp, name, tv2, to);
  assert(to != nullptr);
  SETOP_BODY(to, op, fr);
  tvRefcountedDecRef(fr);
  tvRefcountedDecRef(tv2);
  cellDup(*tvToCell(to), *tv2);
  m_stack.discard();
  decRefStr(name);
}

OPTBLD_INLINE void ExecutionContext::iopSetOpG(IOP_ARGS) {
  NEXT();
  DECODE_OA(SetOpOp, op);
  StringData* name;
  Cell* fr = m_stack.topC();
  TypedValue* tv2 = m_stack.indTV(1);
  TypedValue* to = nullptr;
  // XXX We're probably not getting warnings totally correct here
  lookupd_gbl(m_fp, name, tv2, to);
  assert(to != nullptr);
  SETOP_BODY(to, op, fr);
  tvRefcountedDecRef(fr);
  tvRefcountedDecRef(tv2);
  cellDup(*tvToCell(to), *tv2);
  m_stack.discard();
  decRefStr(name);
}

OPTBLD_INLINE void ExecutionContext::iopSetOpS(IOP_ARGS) {
  NEXT();
  DECODE_OA(SetOpOp, op);
  Cell* fr = m_stack.topC();
  TypedValue* classref = m_stack.indTV(1);
  TypedValue* propn = m_stack.indTV(2);
  TypedValue* output = propn;
  StringData* name;
  TypedValue* val;
  bool visible, accessible;
  lookup_sprop(m_fp, classref, name, propn, val, visible, accessible);
  if (!(visible && accessible)) {
    raise_error("Invalid static property access: %s::%s",
                classref->m_data.pcls->name()->data(),
                name->data());
  }
  SETOP_BODY(val, op, fr);
  tvRefcountedDecRefCell(propn);
  tvRefcountedDecRef(fr);
  cellDup(*tvToCell(val), *output);
  m_stack.ndiscard(2);
  decRefStr(name);
}

OPTBLD_INLINE void ExecutionContext::iopSetOpM(IOP_ARGS) {
  NEXT();
  DECODE_OA(SetOpOp, op);
  DECLARE_SETHELPER_ARGS
  if (!setHelperPre<MoreWarnings, true, false, false, 1,
      VectorLeaveCode::LeaveLast>(MEMBERHELPERPRE_ARGS)) {
    TypedValue* result;
    Cell* rhs = m_stack.topC();

    if (mcode == MW) {
      result = SetOpNewElem(tvScratch, *tvRef.asTypedValue(), op, base, rhs);
    } else {
      switch (mcode) {
      case MEL:
      case MEC:
      case MET:
      case MEI:
        result = SetOpElem(tvScratch, *tvRef.asTypedValue(), op, base,
            *curMember, rhs);
        break;
      case MPL:
      case MPC:
      case MPT: {
        Class *ctx = arGetContextClass(m_fp);
        result = SetOpProp(tvScratch, *tvRef.asTypedValue(), ctx, op, base,
                           *curMember, rhs);
        break;
      }
      default:
        assert(false);
        result = nullptr; // Silence compiler warning.
      }
    }

    tvRefcountedDecRef(rhs);
    cellDup(*tvToCell(result), *rhs);
  }
  setHelperPost<1>(SETHELPERPOST_ARGS);
}

OPTBLD_INLINE void ExecutionContext::iopIncDecL(IOP_ARGS) {
  NEXT();
  DECODE_LA(local);
  DECODE_OA(IncDecOp, op);
  TypedValue* to = m_stack.allocTV();
  tvWriteUninit(to);
  TypedValue* fr = frame_local(m_fp, local);
  IncDecBody<true>(op, fr, to);
}

OPTBLD_INLINE void ExecutionContext::iopIncDecN(IOP_ARGS) {
  NEXT();
  DECODE_OA(IncDecOp, op);
  StringData* name;
  TypedValue* nameCell = m_stack.topTV();
  TypedValue* local = nullptr;
  lookupd_var(m_fp, name, nameCell, local);
  assert(local != nullptr);
  IncDecBody<true>(op, local, nameCell);
  decRefStr(name);
}

OPTBLD_INLINE void ExecutionContext::iopIncDecG(IOP_ARGS) {
  NEXT();
  DECODE_OA(IncDecOp, op);
  StringData* name;
  TypedValue* nameCell = m_stack.topTV();
  TypedValue* gbl = nullptr;
  lookupd_gbl(m_fp, name, nameCell, gbl);
  assert(gbl != nullptr);
  IncDecBody<true>(op, gbl, nameCell);
  decRefStr(name);
}

OPTBLD_INLINE void ExecutionContext::iopIncDecS(IOP_ARGS) {
  StringData* name;
  SPROP_OP_PRELUDE
  DECODE_OA(IncDecOp, op);
  if (!(visible && accessible)) {
    raise_error("Invalid static property access: %s::%s",
                clsref->m_data.pcls->name()->data(),
                name->data());
  }
  tvRefcountedDecRefCell(nameCell);
  IncDecBody<true>(op, val, output);
  m_stack.discard();
  SPROP_OP_POSTLUDE
}

OPTBLD_INLINE void ExecutionContext::iopIncDecM(IOP_ARGS) {
  NEXT();
  DECODE_OA(IncDecOp, op);
  DECLARE_SETHELPER_ARGS
  TypedValue to = make_tv<KindOfUninit>();
  if (!setHelperPre<MoreWarnings, true, false, false, 0,
      VectorLeaveCode::LeaveLast>(MEMBERHELPERPRE_ARGS)) {
    if (mcode == MW) {
      IncDecNewElem<true>(tvScratch, *tvRef.asTypedValue(), op, base, to);
    } else {
      switch (mcode) {
      case MEL:
      case MEC:
      case MET:
      case MEI:
        IncDecElem<true>(tvScratch, *tvRef.asTypedValue(), op, base,
            *curMember, to);
        break;
      case MPL:
      case MPC:
      case MPT: {
        Class* ctx = arGetContextClass(m_fp);
        IncDecProp<true>(tvScratch, *tvRef.asTypedValue(), ctx, op, base,
                         *curMember, to);
        break;
      }
      default: assert(false);
      }
    }
  }
  setHelperPost<0>(SETHELPERPOST_ARGS);
  Cell* c1 = m_stack.allocC();
  memcpy(c1, &to, sizeof(TypedValue));
}

OPTBLD_INLINE void ExecutionContext::iopBindL(IOP_ARGS) {
  NEXT();
  DECODE_LA(local);
  Ref* fr = m_stack.topV();
  TypedValue* to = frame_local(m_fp, local);
  tvBind(fr, to);
}

OPTBLD_INLINE void ExecutionContext::iopBindN(IOP_ARGS) {
  NEXT();
  StringData* name;
  TypedValue* fr = m_stack.topTV();
  TypedValue* nameTV = m_stack.indTV(1);
  TypedValue* to = nullptr;
  lookupd_var(m_fp, name, nameTV, to);
  assert(to != nullptr);
  tvBind(fr, to);
  memcpy((void*)nameTV, (void*)fr, sizeof(TypedValue));
  m_stack.discard();
  decRefStr(name);
}

OPTBLD_INLINE void ExecutionContext::iopBindG(IOP_ARGS) {
  NEXT();
  StringData* name;
  TypedValue* fr = m_stack.topTV();
  TypedValue* nameTV = m_stack.indTV(1);
  TypedValue* to = nullptr;
  lookupd_gbl(m_fp, name, nameTV, to);
  assert(to != nullptr);
  tvBind(fr, to);
  memcpy((void*)nameTV, (void*)fr, sizeof(TypedValue));
  m_stack.discard();
  decRefStr(name);
}

OPTBLD_INLINE void ExecutionContext::iopBindS(IOP_ARGS) {
  NEXT();
  TypedValue* fr = m_stack.topTV();
  TypedValue* classref = m_stack.indTV(1);
  TypedValue* propn = m_stack.indTV(2);
  TypedValue* output = propn;
  StringData* name;
  TypedValue* val;
  bool visible, accessible;
  lookup_sprop(m_fp, classref, name, propn, val, visible, accessible);
  if (!(visible && accessible)) {
    raise_error("Invalid static property access: %s::%s",
                classref->m_data.pcls->name()->data(),
                name->data());
  }
  tvBind(fr, val);
  tvRefcountedDecRefCell(propn);
  memcpy(output, fr, sizeof(TypedValue));
  m_stack.ndiscard(2);
  decRefStr(name);
}

OPTBLD_INLINE void ExecutionContext::iopBindM(IOP_ARGS) {
  NEXT();
  DECLARE_SETHELPER_ARGS
  TypedValue* tv1 = m_stack.topTV();
  if (!setHelperPre<false, true, false, true, 1,
      VectorLeaveCode::ConsumeAll>(MEMBERHELPERPRE_ARGS)) {
    // Bind the element/property with the var on the top of the stack
    tvBind(tv1, base);
  }
  setHelperPost<1>(SETHELPERPOST_ARGS);
}

OPTBLD_INLINE void ExecutionContext::iopUnsetL(IOP_ARGS) {
  NEXT();
  DECODE_LA(local);
  assert(local < m_fp->m_func->numLocals());
  TypedValue* tv = frame_local(m_fp, local);
  tvRefcountedDecRef(tv);
  tvWriteUninit(tv);
}

OPTBLD_INLINE void ExecutionContext::iopUnsetN(IOP_ARGS) {
  NEXT();
  StringData* name;
  TypedValue* tv1 = m_stack.topTV();
  TypedValue* tv = nullptr;
  lookup_var(m_fp, name, tv1, tv);
  assert(!m_fp->hasInvName());
  if (tv != nullptr) {
    tvRefcountedDecRef(tv);
    tvWriteUninit(tv);
  }
  m_stack.popC();
  decRefStr(name);
}

OPTBLD_INLINE void ExecutionContext::iopUnsetG(IOP_ARGS) {
  NEXT();
  TypedValue* tv1 = m_stack.topTV();
  StringData* name = lookup_name(tv1);
  VarEnv* varEnv = m_globalVarEnv;
  assert(varEnv != nullptr);
  varEnv->unset(name);
  m_stack.popC();
  decRefStr(name);
}

OPTBLD_INLINE void ExecutionContext::iopUnsetM(IOP_ARGS) {
  NEXT();
  DECLARE_SETHELPER_ARGS
  if (!setHelperPre<false, false, true, false, 0,
      VectorLeaveCode::LeaveLast>(MEMBERHELPERPRE_ARGS)) {
    switch (mcode) {
    case MEL:
    case MEC:
    case MET:
    case MEI:
      UnsetElem(base, *curMember);
      break;
    case MPL:
    case MPC:
    case MPT: {
      Class* ctx = arGetContextClass(m_fp);
      UnsetProp(ctx, base, *curMember);
      break;
    }
    default: assert(false);
    }
  }
  setHelperPost<0>(SETHELPERPOST_ARGS);
}

OPTBLD_INLINE ActRec* ExecutionContext::fPushFuncImpl(
    const Func* func,
    int numArgs) {
  DEBUGGER_IF(phpBreakpointEnabled(func->name()->data()));
  ActRec* ar = m_stack.allocA();
  arSetSfp(ar, m_fp);
  ar->m_func = func;
  ar->initNumArgs(numArgs);
  ar->setVarEnv(nullptr);
  return ar;
}

OPTBLD_INLINE void ExecutionContext::iopFPushFunc(IOP_ARGS) {
  NEXT();
  DECODE_IVA(numArgs);
  Cell* c1 = m_stack.topC();
  const Func* func = nullptr;

  // Throughout this function, we save obj/string/array and defer
  // refcounting them until after the stack has been discarded.

  if (IS_STRING_TYPE(c1->m_type)) {
    StringData* origSd = c1->m_data.pstr;
    func = Unit::loadFunc(origSd);
    if (func == nullptr) {
      raise_error("Call to undefined function %s()", c1->m_data.pstr->data());
    }

    m_stack.discard();
    ActRec* ar = fPushFuncImpl(func, numArgs);
    ar->setThis(nullptr);
    decRefStr(origSd);
    return;
  }

  if (c1->m_type == KindOfObject) {
    // this covers both closures and functors
    static StringData* invokeName = makeStaticString("__invoke");
    ObjectData* origObj = c1->m_data.pobj;
    const Class* cls = origObj->getVMClass();
    func = cls->lookupMethod(invokeName);
    if (func == nullptr) {
      raise_error(Strings::FUNCTION_NAME_MUST_BE_STRING);
    }

    m_stack.discard();
    ActRec* ar = fPushFuncImpl(func, numArgs);
    if (func->attrs() & AttrStatic && !func->isClosureBody()) {
      ar->setClass(origObj->getVMClass());
      decRefObj(origObj);
    } else {
      ar->setThis(origObj);
      // Teleport the reference from the destroyed stack cell to the
      // ActRec. Don't try this at home.
    }
    return;
  }

  if (c1->m_type == KindOfArray) {
    // support: array($instance, 'method') and array('Class', 'method')
    // which are both valid callables
    ArrayData* origArr = c1->m_data.parr;
    ObjectData* arrThis = nullptr;
    HPHP::Class* arrCls = nullptr;
    StringData* invName = nullptr;

    func = vm_decode_function(
      tvAsCVarRef(c1),
      getFP(),
      /* forwarding */ false,
      arrThis,
      arrCls,
      invName,
      /* warn */ false
    );
    if (func == nullptr) {
      raise_error("Invalid callable (array)");
    }
    assert(arrCls != nullptr);

    m_stack.discard();
    ActRec* ar = fPushFuncImpl(func, numArgs);
    if (arrThis) {
      arrThis->incRefCount();
      ar->setThis(arrThis);
    } else {
      ar->setClass(arrCls);
    }
    if (UNLIKELY(invName != nullptr)) {
      ar->setInvName(invName);
    }
    decRefArr(origArr);
    return;
  }

  raise_error(Strings::FUNCTION_NAME_MUST_BE_STRING);
}

OPTBLD_INLINE void ExecutionContext::iopFPushFuncD(IOP_ARGS) {
  NEXT();
  DECODE_IVA(numArgs);
  DECODE(Id, id);
  const NamedEntityPair nep = m_fp->m_func->unit()->lookupNamedEntityPairId(id);
  Func* func = Unit::loadFunc(nep.second, nep.first);
  if (func == nullptr) {
    raise_error("Call to undefined function %s()",
                m_fp->m_func->unit()->lookupLitstrId(id)->data());
  }
  ActRec* ar = fPushFuncImpl(func, numArgs);
  ar->setThis(nullptr);
}

OPTBLD_INLINE void ExecutionContext::iopFPushFuncU(IOP_ARGS) {
  NEXT();
  DECODE_IVA(numArgs);
  DECODE(Id, nsFunc);
  DECODE(Id, globalFunc);
  Unit* unit = m_fp->m_func->unit();
  const NamedEntityPair nep = unit->lookupNamedEntityPairId(nsFunc);
  Func* func = Unit::loadFunc(nep.second, nep.first);
  if (func == nullptr) {
    const NamedEntityPair nep2 = unit->lookupNamedEntityPairId(globalFunc);
    func = Unit::loadFunc(nep2.second, nep2.first);
    if (func == nullptr) {
      const char *funcName = unit->lookupLitstrId(nsFunc)->data();
      raise_error("Call to undefined function %s()", funcName);
    }
  }
  ActRec* ar = fPushFuncImpl(func, numArgs);
  ar->setThis(nullptr);
}

void ExecutionContext::fPushObjMethodImpl(
    Class* cls, StringData* name, ObjectData* obj, int numArgs) {
  const Func* f;
  LookupResult res = lookupObjMethod(f, cls, name,
                                     arGetContextClass(getFP()), true);
  assert(f);
  ActRec* ar = m_stack.allocA();
  arSetSfp(ar, m_fp);
  ar->m_func = f;
  if (res == LookupResult::MethodFoundNoThis) {
    decRefObj(obj);
    ar->setClass(cls);
  } else {
    assert(res == LookupResult::MethodFoundWithThis ||
           res == LookupResult::MagicCallFound);
    /* Transfer ownership of obj to the ActRec*/
    ar->setThis(obj);
  }
  ar->initNumArgs(numArgs);
  if (res == LookupResult::MagicCallFound) {
    ar->setInvName(name);
  } else {
    ar->setVarEnv(NULL);
    decRefStr(name);
  }
}

static void throw_call_non_object(const char* methodName) {
  std::string msg;
  folly::format(&msg, "Call to a member function {}() on a non-object",
    methodName);

  if (RuntimeOption::ThrowExceptionOnBadMethodCall) {
    Object e(SystemLib::AllocBadMethodCallExceptionObject(String(msg)));
    throw e;
  }
  throw FatalErrorException(msg.c_str());
}

OPTBLD_INLINE void ExecutionContext::iopFPushObjMethod(IOP_ARGS) {
  NEXT();
  DECODE_IVA(numArgs);
  Cell* c1 = m_stack.topC(); // Method name.
  if (!IS_STRING_TYPE(c1->m_type)) {
    raise_error(Strings::METHOD_NAME_MUST_BE_STRING);
  }
  Cell* c2 = m_stack.indC(1); // Object.
  if (c2->m_type != KindOfObject) {
    throw_call_non_object(c1->m_data.pstr->data());
  }
  ObjectData* obj = c2->m_data.pobj;
  Class* cls = obj->getVMClass();
  StringData* name = c1->m_data.pstr;
  // We handle decReffing obj and name in fPushObjMethodImpl
  m_stack.ndiscard(2);
  fPushObjMethodImpl(cls, name, obj, numArgs);
}

OPTBLD_INLINE void ExecutionContext::iopFPushObjMethodD(IOP_ARGS) {
  NEXT();
  DECODE_IVA(numArgs);
  DECODE_LITSTR(name);
  Cell* c1 = m_stack.topC();
  if (c1->m_type != KindOfObject) {
    throw_call_non_object(name->data());
  }
  ObjectData* obj = c1->m_data.pobj;
  Class* cls = obj->getVMClass();
  // We handle decReffing obj in fPushObjMethodImpl
  m_stack.discard();
  fPushObjMethodImpl(cls, name, obj, numArgs);
}

template<bool forwarding>
void ExecutionContext::pushClsMethodImpl(Class* cls,
                                           StringData* name,
                                           ObjectData* obj,
                                           int numArgs) {
  const Func* f;
  LookupResult res = lookupClsMethod(f, cls, name, obj,
                                     arGetContextClass(getFP()), true);
  if (res == LookupResult::MethodFoundNoThis ||
      res == LookupResult::MagicCallStaticFound) {
    obj = nullptr;
  } else {
    assert(obj);
    assert(res == LookupResult::MethodFoundWithThis ||
           res == LookupResult::MagicCallFound);
    obj->incRefCount();
  }
  assert(f);
  ActRec* ar = m_stack.allocA();
  arSetSfp(ar, m_fp);
  ar->m_func = f;
  if (obj) {
    ar->setThis(obj);
  } else {
    if (!forwarding) {
      ar->setClass(cls);
    } else {
      /* Propagate the current late bound class if there is one, */
      /* otherwise use the class given by this instruction's input */
      if (m_fp->hasThis()) {
        cls = m_fp->getThis()->getVMClass();
      } else if (m_fp->hasClass()) {
        cls = m_fp->getClass();
      }
      ar->setClass(cls);
    }
  }
  ar->initNumArgs(numArgs);
  if (res == LookupResult::MagicCallFound ||
      res == LookupResult::MagicCallStaticFound) {
    ar->setInvName(name);
  } else {
    ar->setVarEnv(nullptr);
    decRefStr(const_cast<StringData*>(name));
  }
}

OPTBLD_INLINE void ExecutionContext::iopFPushClsMethod(IOP_ARGS) {
  NEXT();
  DECODE_IVA(numArgs);
  Cell* c1 = m_stack.indC(1); // Method name.
  if (!IS_STRING_TYPE(c1->m_type)) {
    raise_error(Strings::FUNCTION_NAME_MUST_BE_STRING);
  }
  TypedValue* tv = m_stack.top();
  assert(tv->m_type == KindOfClass);
  Class* cls = tv->m_data.pcls;
  StringData* name = c1->m_data.pstr;
  // pushClsMethodImpl will take care of decReffing name
  m_stack.ndiscard(2);
  assert(cls && name);
  ObjectData* obj = m_fp->hasThis() ? m_fp->getThis() : nullptr;
  pushClsMethodImpl<false>(cls, name, obj, numArgs);
}

OPTBLD_INLINE void ExecutionContext::iopFPushClsMethodD(IOP_ARGS) {
  NEXT();
  DECODE_IVA(numArgs);
  DECODE_LITSTR(name);
  DECODE(Id, classId);
  const NamedEntityPair &nep =
    m_fp->m_func->unit()->lookupNamedEntityPairId(classId);
  Class* cls = Unit::loadClass(nep.second, nep.first);
  if (cls == nullptr) {
    raise_error(Strings::UNKNOWN_CLASS, nep.first->data());
  }
  ObjectData* obj = m_fp->hasThis() ? m_fp->getThis() : nullptr;
  pushClsMethodImpl<false>(cls, name, obj, numArgs);
}

OPTBLD_INLINE void ExecutionContext::iopFPushClsMethodF(IOP_ARGS) {
  NEXT();
  DECODE_IVA(numArgs);
  Cell* c1 = m_stack.indC(1); // Method name.
  if (!IS_STRING_TYPE(c1->m_type)) {
    raise_error(Strings::FUNCTION_NAME_MUST_BE_STRING);
  }
  TypedValue* tv = m_stack.top();
  assert(tv->m_type == KindOfClass);
  Class* cls = tv->m_data.pcls;
  assert(cls);
  StringData* name = c1->m_data.pstr;
  // pushClsMethodImpl will take care of decReffing name
  m_stack.ndiscard(2);
  ObjectData* obj = m_fp->hasThis() ? m_fp->getThis() : nullptr;
  pushClsMethodImpl<true>(cls, name, obj, numArgs);
}

OPTBLD_INLINE void ExecutionContext::iopFPushCtor(IOP_ARGS) {
  NEXT();
  DECODE_IVA(numArgs);
  TypedValue* tv = m_stack.topTV();
  assert(tv->m_type == KindOfClass);
  Class* cls = tv->m_data.pcls;
  assert(cls != nullptr);
  // Lookup the ctor
  const Func* f;
  LookupResult res UNUSED = lookupCtorMethod(f, cls, true);
  assert(res == LookupResult::MethodFoundWithThis);
  // Replace input with uninitialized instance.
  ObjectData* this_ = newInstance(cls);
  TRACE(2, "FPushCtor: just new'ed an instance of class %s: %p\n",
        cls->name()->data(), this_);
  this_->incRefCount();
  this_->incRefCount();
  tv->m_type = KindOfObject;
  tv->m_data.pobj = this_;
  // Push new activation record.
  ActRec* ar = m_stack.allocA();
  arSetSfp(ar, m_fp);
  ar->m_func = f;
  ar->setThis(this_);
  ar->initNumArgsFromFPushCtor(numArgs);
  arSetSfp(ar, m_fp);
  ar->setVarEnv(nullptr);
}

OPTBLD_INLINE void ExecutionContext::iopFPushCtorD(IOP_ARGS) {
  NEXT();
  DECODE_IVA(numArgs);
  DECODE(Id, id);
  const NamedEntityPair &nep =
    m_fp->m_func->unit()->lookupNamedEntityPairId(id);
  Class* cls = Unit::loadClass(nep.second, nep.first);
  if (cls == nullptr) {
    raise_error(Strings::UNKNOWN_CLASS,
                m_fp->m_func->unit()->lookupLitstrId(id)->data());
  }
  // Lookup the ctor
  const Func* f;
  LookupResult res UNUSED = lookupCtorMethod(f, cls, true);
  assert(res == LookupResult::MethodFoundWithThis);
  // Push uninitialized instance.
  ObjectData* this_ = newInstance(cls);
  TRACE(2, "FPushCtorD: new'ed an instance of class %s: %p\n",
        cls->name()->data(), this_);
  this_->incRefCount();
  m_stack.pushObject(this_);
  // Push new activation record.
  ActRec* ar = m_stack.allocA();
  arSetSfp(ar, m_fp);
  ar->m_func = f;
  ar->setThis(this_);
  ar->initNumArgsFromFPushCtor(numArgs);
  ar->setVarEnv(nullptr);
}

OPTBLD_INLINE void ExecutionContext::iopDecodeCufIter(IOP_ARGS) {
  PC origPc = pc;
  NEXT();
  DECODE_IA(itId);
  DECODE(Offset, offset);

  Iter* it = frame_iter(m_fp, itId);
  CufIter &cit = it->cuf();

  ObjectData* obj = nullptr;
  HPHP::Class* cls = nullptr;
  StringData* invName = nullptr;
  TypedValue *func = m_stack.topTV();

  ActRec* ar = m_fp;
  if (m_fp->m_func->isBuiltin()) {
    ar = getOuterVMFrame(ar);
  }
  const Func* f = vm_decode_function(tvAsVariant(func),
                                     ar, false,
                                     obj, cls, invName,
                                     false);

  if (f == nullptr) {
    pc = origPc + offset;
  } else {
    cit.setFunc(f);
    if (obj) {
      cit.setCtx(obj);
      obj->incRefCount();
    } else {
      cit.setCtx(cls);
    }
    cit.setName(invName);
  }
  m_stack.popC();
}

OPTBLD_INLINE void ExecutionContext::iopFPushCufIter(IOP_ARGS) {
  NEXT();
  DECODE_IVA(numArgs);
  DECODE_IA(itId);

  Iter* it = frame_iter(m_fp, itId);

  auto f = it->cuf().func();
  auto o = it->cuf().ctx();
  auto n = it->cuf().name();

  ActRec* ar = m_stack.allocA();
  arSetSfp(ar, m_fp);
  ar->m_func = f;
  ar->m_this = (ObjectData*)o;
  if (o && !(uintptr_t(o) & 1)) ar->m_this->incRefCount();
  if (n) {
    ar->setInvName(n);
    n->incRefCount();
  } else {
    ar->setVarEnv(nullptr);
  }
  ar->initNumArgs(numArgs);
}

OPTBLD_INLINE void ExecutionContext::doFPushCuf(IOP_ARGS,
                                                  bool forward, bool safe) {
  NEXT();
  DECODE_IVA(numArgs);

  TypedValue func = m_stack.topTV()[safe];

  ObjectData* obj = nullptr;
  HPHP::Class* cls = nullptr;
  StringData* invName = nullptr;

  const Func* f = vm_decode_function(tvAsVariant(&func), getFP(),
                                     forward,
                                     obj, cls, invName,
                                     !safe);

  if (safe) m_stack.topTV()[1] = m_stack.topTV()[0];
  m_stack.ndiscard(1);
  if (f == nullptr) {
    f = SystemLib::s_nullFunc;
    if (safe) {
      m_stack.pushFalse();
    }
  } else if (safe) {
    m_stack.pushTrue();
  }

  ActRec* ar = m_stack.allocA();
  arSetSfp(ar, m_fp);
  ar->m_func = f;
  if (obj) {
    ar->setThis(obj);
    obj->incRefCount();
  } else if (cls) {
    ar->setClass(cls);
  } else {
    ar->setThis(nullptr);
  }
  ar->initNumArgs(numArgs);
  if (invName) {
    ar->setInvName(invName);
  } else {
    ar->setVarEnv(nullptr);
  }
  tvRefcountedDecRef(&func);
}

OPTBLD_INLINE void ExecutionContext::iopFPushCuf(IOP_ARGS) {
  doFPushCuf(IOP_PASS_ARGS, false, false);
}

OPTBLD_INLINE void ExecutionContext::iopFPushCufF(IOP_ARGS) {
  doFPushCuf(IOP_PASS_ARGS, true, false);
}

OPTBLD_INLINE void ExecutionContext::iopFPushCufSafe(IOP_ARGS) {
  doFPushCuf(IOP_PASS_ARGS, false, true);
}

static inline ActRec* arFromInstr(TypedValue* sp, const Op* pc) {
  return arFromSpOffset((ActRec*)sp, instrSpToArDelta(pc));
}

OPTBLD_INLINE void ExecutionContext::iopFPassC(IOP_ARGS) {
  DEBUG_ONLY auto const ar = arFromInstr(m_stack.top(), (Op*)pc);
  NEXT();
  DECODE_IVA(paramId);
  assert(paramId < ar->numArgs());
}

#define FPASSC_CHECKED_PRELUDE                                                \
  ActRec* ar = arFromInstr(m_stack.top(), (Op*)pc);                           \
  NEXT();                                                                     \
  DECODE_IVA(paramId);                                                        \
  assert(paramId < ar->numArgs());                                            \
  const Func* func = ar->m_func;

OPTBLD_INLINE void ExecutionContext::iopFPassCW(IOP_ARGS) {
  FPASSC_CHECKED_PRELUDE
  if (func->mustBeRef(paramId)) {
    TRACE(1, "FPassCW: function %s(%d) param %d is by reference, "
          "raising a strict warning (attr:0x%x)\n",
          func->name()->data(), func->numParams(), paramId,
          func->methInfo() ? func->methInfo()->attribute : 0);
    raise_strict_warning("Only variables should be passed by reference");
  }
}

OPTBLD_INLINE void ExecutionContext::iopFPassCE(IOP_ARGS) {
  FPASSC_CHECKED_PRELUDE
  if (func->mustBeRef(paramId)) {
    TRACE(1, "FPassCE: function %s(%d) param %d is by reference, "
          "throwing a fatal error (attr:0x%x)\n",
          func->name()->data(), func->numParams(), paramId,
          func->methInfo() ? func->methInfo()->attribute : 0);
    raise_error("Cannot pass parameter %d by reference", paramId+1);
  }
}

#undef FPASSC_CHECKED_PRELUDE

OPTBLD_INLINE void ExecutionContext::iopFPassV(IOP_ARGS) {
  ActRec* ar = arFromInstr(m_stack.top(), (Op*)pc);
  NEXT();
  DECODE_IVA(paramId);
  assert(paramId < ar->numArgs());
  const Func* func = ar->m_func;
  if (!func->byRef(paramId)) {
    m_stack.unbox();
  }
}

OPTBLD_INLINE void ExecutionContext::iopFPassVNop(IOP_ARGS) {
  DEBUG_ONLY auto const ar = arFromInstr(m_stack.top(), (Op*)pc);
  NEXT();
  DECODE_IVA(paramId);
  assert(paramId < ar->numArgs());
  assert(ar->m_func->byRef(paramId));
}

OPTBLD_INLINE void ExecutionContext::iopFPassR(IOP_ARGS) {
  ActRec* ar = arFromInstr(m_stack.top(), (Op*)pc);
  NEXT();
  DECODE_IVA(paramId);
  assert(paramId < ar->numArgs());
  const Func* func = ar->m_func;
  if (func->byRef(paramId)) {
    TypedValue* tv = m_stack.topTV();
    if (tv->m_type != KindOfRef) {
      tvBox(tv);
    }
  } else {
    if (m_stack.topTV()->m_type == KindOfRef) {
      m_stack.unbox();
    }
  }
}

OPTBLD_INLINE void ExecutionContext::iopFPassL(IOP_ARGS) {
  ActRec* ar = arFromInstr(m_stack.top(), (Op*)pc);
  NEXT();
  DECODE_IVA(paramId);
  DECODE_LA(local);
  assert(paramId < ar->numArgs());
  TypedValue* fr = frame_local(m_fp, local);
  TypedValue* to = m_stack.allocTV();
  if (!ar->m_func->byRef(paramId)) {
    cgetl_body(m_fp, fr, to, local);
  } else {
    vgetl_body(fr, to);
  }
}

OPTBLD_INLINE void ExecutionContext::iopFPassN(IOP_ARGS) {
  ActRec* ar = arFromInstr(m_stack.top(), (Op*)pc);
  PC origPc = pc;
  NEXT();
  DECODE_IVA(paramId);
  assert(paramId < ar->numArgs());
  if (!ar->m_func->byRef(paramId)) {
    iopCGetN(IOP_PASS(origPc));
  } else {
    iopVGetN(IOP_PASS(origPc));
  }
}

OPTBLD_INLINE void ExecutionContext::iopFPassG(IOP_ARGS) {
  ActRec* ar = arFromInstr(m_stack.top(), (Op*)pc);
  PC origPc = pc;
  NEXT();
  DECODE_IVA(paramId);
  assert(paramId < ar->numArgs());
  if (!ar->m_func->byRef(paramId)) {
    iopCGetG(IOP_PASS(origPc));
  } else {
    iopVGetG(IOP_PASS(origPc));
  }
}

OPTBLD_INLINE void ExecutionContext::iopFPassS(IOP_ARGS) {
  ActRec* ar = arFromInstr(m_stack.top(), (Op*)pc);
  PC origPc = pc;
  NEXT();
  DECODE_IVA(paramId);
  assert(paramId < ar->numArgs());
  if (!ar->m_func->byRef(paramId)) {
    iopCGetS(IOP_PASS(origPc));
  } else {
    iopVGetS(IOP_PASS(origPc));
  }
}

void ExecutionContext::iopFPassM(IOP_ARGS) {
  ActRec* ar = arFromInstr(m_stack.top(), (Op*)pc);
  NEXT();
  DECODE_IVA(paramId);
  assert(paramId < ar->numArgs());
  if (!ar->m_func->byRef(paramId)) {
    DECLARE_GETHELPER_ARGS
    getHelper(GETHELPER_ARGS);
    if (tvRet->m_type == KindOfRef) {
      tvUnbox(tvRet);
    }
  } else {
    DECLARE_SETHELPER_ARGS
    TypedValue* tv1 = m_stack.allocTV();
    tvWriteUninit(tv1);
    if (!setHelperPre<false, true, false, true, 1,
        VectorLeaveCode::ConsumeAll>(MEMBERHELPERPRE_ARGS)) {
      if (base->m_type != KindOfRef) {
        tvBox(base);
      }
      refDup(*base, *tv1);
    } else {
      tvWriteNull(tv1);
      tvBox(tv1);
    }
    setHelperPost<1>(SETHELPERPOST_ARGS);
  }
}

bool ExecutionContext::doFCall(ActRec* ar, PC& pc) {
  assert(getOuterVMFrame(ar) == m_fp);
  ar->m_savedRip =
    reinterpret_cast<uintptr_t>(tx->uniqueStubs.retHelper);
  assert(isReturnHelper(ar->m_savedRip));
  TRACE(3, "FCall: pc %p func %p base %d\n", m_pc,
        m_fp->m_func->unit()->entry(),
        int(m_fp->m_func->base()));
  ar->m_soff = m_fp->m_func->unit()->offsetOf(pc)
    - (uintptr_t)m_fp->m_func->base();
  assert(pcOff(this) >= m_fp->m_func->base());
  prepareFuncEntry(ar, pc);
  SYNC();
  if (EventHook::FunctionEnter(ar, EventHook::NormalFunc)) return true;
  pc = m_pc;
  return false;
}

OPTBLD_INLINE void ExecutionContext::iopFCall(IOP_ARGS) {
  ActRec* ar = arFromInstr(m_stack.top(), (Op*)pc);
  NEXT();
  DECODE_IVA(numArgs);
  assert(numArgs == ar->numArgs());
  checkStack(m_stack, ar->m_func);
  doFCall(ar, pc);
  if (RuntimeOption::EvalRuntimeTypeProfile) {
    profileAllArguments(ar);
  }
}

OPTBLD_INLINE void ExecutionContext::iopFCallD(IOP_ARGS) {
  auto const ar = arFromInstr(m_stack.top(), reinterpret_cast<const Op*>(pc));
  NEXT();
  DECODE_IVA(numArgs);
  DECODE_LITSTR(clsName);
  DECODE_LITSTR(funcName);
  (void) clsName;
  (void) funcName;
  if (!RuntimeOption::EvalJitEnableRenameFunction &&
      !(ar->m_func->attrs() & AttrDynamicInvoke)) {
    assert(ar->m_func->name()->isame(funcName));
  }
  assert(numArgs == ar->numArgs());
  checkStack(m_stack, ar->m_func);
  doFCall(ar, pc);
  if (RuntimeOption::EvalRuntimeTypeProfile) {
    profileAllArguments(ar);
  }
}

OPTBLD_INLINE void ExecutionContext::iopFCallBuiltin(IOP_ARGS) {
  NEXT();
  DECODE_IVA(numArgs);
  DECODE_IVA(numNonDefault);
  DECODE(Id, id);
  const NamedEntity* ne = m_fp->m_func->unit()->lookupNamedEntityId(id);
  Func* func = Unit::lookupFunc(ne);
  if (func == nullptr) {
    raise_error("Call to undefined function %s()",
                m_fp->m_func->unit()->lookupLitstrId(id)->data());
  }
  TypedValue* args = m_stack.indTV(numArgs-1);
  TypedValue ret;
  if (Native::coerceFCallArgs(args, numArgs, numNonDefault, func)) {
    Native::callFunc(func, nullptr, args, numArgs, ret);
  } else {
    bool zendParamModeNull = !func->methInfo() ||
      (func->methInfo()->attribute & ClassInfo::ZendParamModeNull);
    if (zendParamModeNull) {
      ret.m_type = KindOfNull;
    } else {
      assert(func->methInfo()->attribute & ClassInfo::ZendParamModeFalse);
      ret.m_type = KindOfBoolean;
      ret.m_data.num = 0;
    }
  }

  frame_free_args(args, numNonDefault);
  m_stack.ndiscard(numArgs);
  tvCopy(ret, *m_stack.allocTV());
}

bool ExecutionContext::prepareArrayArgs(ActRec* ar, const Variant& arrayArgs) {
  const auto& args = *arrayArgs.asCell();
  assert(isContainer(args));
  if (UNLIKELY(ar->hasInvName())) {
    m_stack.pushStringNoRc(ar->getInvName());
    if (args.m_type == KindOfArray && args.m_data.parr->isVectorData()) {
      m_stack.pushArray(args.m_data.parr);
    } else {
      PackedArrayInit ai(getContainerSize(args));
      for (ArrayIter iter(args); iter; ++iter) {
        ai.appendWithRef(iter.secondRefPlus());
      }
      m_stack.pushArray(ai.create());
    }
    ar->setVarEnv(0);
    ar->initNumArgs(2);
    return true;
  }

  int nargs = getContainerSize(args);
  const Func* f = ar->m_func;
  int nparams = f->numParams();
  int extra = nargs - nparams;
  if (extra < 0) {
    extra = 0;
    nparams = nargs;
  }
  ArrayIter iter(args);
  for (int i = 0; i < nparams; ++i) {
    TypedValue* from = const_cast<TypedValue*>(
      iter.secondRefPlus().asTypedValue());
    TypedValue* to = m_stack.allocTV();
    if (LIKELY(!f->byRef(i))) {
      cellDup(*tvToCell(from), *to);
    } else if (LIKELY(from->m_type == KindOfRef &&
                      from->m_data.pref->m_count >= 2)) {
      refDup(*from, *to);
    } else {
      try {
        raise_warning("Parameter %d to %s() expected to be a reference, "
                      "value given", i + 1, f->fullName()->data());
      } catch (...) {
        // If the user error handler throws an exception, discard the
        // uninitialized TypedValue at the top of the eval stack so
        // that the unwinder doesn't choke
        m_stack.discard();
        throw;
      }
      if (skipCufOnInvalidParams) {
        m_stack.discard();
        while (i--) m_stack.popTV();
        m_stack.popAR();
        m_stack.pushNull();
        return false;
      }
      cellDup(*tvToCell(from), *to);
    }
    ++iter;
  }
  if (extra && (ar->m_func->attrs() & AttrMayUseVV)) {
    ExtraArgs* extraArgs = ExtraArgs::allocateUninit(extra);
    for (int i = 0; i < extra; ++i) {
      TypedValue* to = extraArgs->getExtraArg(i);
      const TypedValue* from = iter.secondRefPlus().asTypedValue();
      if (from->m_type == KindOfRef && from->m_data.pref->isReferenced()) {
        refDup(*from, *to);
      } else {
        cellDup(*tvToCell(from), *to);
      }
      ++iter;
    }
    ar->setExtraArgs(extraArgs);
    ar->initNumArgs(nargs);
  } else {
    ar->initNumArgs(nparams);
  }

  return true;
}

static void cleanupParamsAndActRec(Stack& stack,
                                   ActRec* ar,
                                   ExtraArgs* extraArgs) {
  assert(stack.top() + (extraArgs ?
                        ar->m_func->numParams() :
                        ar->numArgs()) == (void*)ar);
  if (extraArgs) {
    const int numExtra = ar->numArgs() - ar->m_func->numParams();
    ExtraArgs::deallocate(extraArgs, numExtra);
  }
  while (stack.top() != (void*)ar) {
    stack.popTV();
  }
  stack.popAR();
}

bool ExecutionContext::doFCallArray(PC& pc) {
  ActRec* ar = (ActRec*)(m_stack.top() + 1);
  assert(ar->numArgs() == 1);

  Cell* c1 = m_stack.topC();
  if (skipCufOnInvalidParams && UNLIKELY(!isContainer(*c1))) {
    // task #1756122
    // this is what we /should/ do, but our code base depends
    // on the broken behavior of casting the second arg to an
    // array.
    cleanupParamsAndActRec(m_stack, ar, nullptr);
    m_stack.pushNull();
    raise_warning("call_user_func_array() expects parameter 2 to be array");
    return false;
  }

  const Func* func = ar->m_func;
  {
    Variant args(isContainer(*c1) ? tvAsCVarRef(c1) :
                 Variant(tvAsVariant(c1).toArray()));
    m_stack.popTV();
    checkStack(m_stack, func);

    assert(ar->m_savedRbp == (uint64_t)m_fp);
    assert(!ar->inGenerator());
    ar->m_savedRip =
      reinterpret_cast<uintptr_t>(tx->uniqueStubs.retHelper);
    assert(isReturnHelper(ar->m_savedRip));
    TRACE(3, "FCallArray: pc %p func %p base %d\n", m_pc,
          m_fp->unit()->entry(),
          int(m_fp->m_func->base()));
    ar->m_soff = m_fp->unit()->offsetOf(pc)
      - (uintptr_t)m_fp->m_func->base();
    assert(pcOff(this) > m_fp->m_func->base());

    if (UNLIKELY(!prepareArrayArgs(ar, args))) return false;
  }

  prepareFuncEntry(ar, pc);
  SYNC();
  if (UNLIKELY(!EventHook::FunctionEnter(ar, EventHook::NormalFunc))) {
    pc = m_pc;
    return false;
  }
  return true;
}

bool ExecutionContext::doFCallArrayTC(PC pc) {
  JIT::assert_native_stack_aligned();
  assert(tl_regState == VMRegState::DIRTY);
  tl_regState = VMRegState::CLEAN;
  auto const ret = doFCallArray(pc);
  tl_regState = VMRegState::DIRTY;
  return ret;
}

OPTBLD_INLINE void ExecutionContext::iopFCallArray(IOP_ARGS) {
  NEXT();
  (void)doFCallArray(pc);
}

OPTBLD_INLINE void ExecutionContext::iopCufSafeArray(IOP_ARGS) {
  NEXT();
  Array ret;
  ret.append(tvAsVariant(m_stack.top() + 1));
  ret.appendWithRef(tvAsVariant(m_stack.top() + 0));
  m_stack.popTV();
  m_stack.popTV();
  tvAsVariant(m_stack.top()) = ret;
}

OPTBLD_INLINE void ExecutionContext::iopCufSafeReturn(IOP_ARGS) {
  NEXT();
  bool ok = cellToBool(*tvToCell(m_stack.top() + 1));
  tvRefcountedDecRef(m_stack.top() + 1);
  tvRefcountedDecRef(m_stack.top() + (ok ? 2 : 0));
  if (ok) m_stack.top()[2] = m_stack.top()[0];
  m_stack.ndiscard(2);
}

inline bool ExecutionContext::initIterator(PC& pc, PC& origPc, Iter* it,
                                             Offset offset, Cell* c1) {
  bool hasElems = it->init(c1);
  if (!hasElems) {
    ITER_SKIP(offset);
  }
  m_stack.popC();
  return hasElems;
}

OPTBLD_INLINE void ExecutionContext::iopIterInit(IOP_ARGS) {
  PC origPc = pc;
  NEXT();
  DECODE_IA(itId);
  DECODE(Offset, offset);
  DECODE_LA(val);
  Cell* c1 = m_stack.topC();
  Iter* it = frame_iter(m_fp, itId);
  TypedValue* tv1 = frame_local(m_fp, val);
  if (initIterator(pc, origPc, it, offset, c1)) {
    tvAsVariant(tv1) = it->arr().second();
  }
}

OPTBLD_INLINE void ExecutionContext::iopIterInitK(IOP_ARGS) {
  PC origPc = pc;
  NEXT();
  DECODE_IA(itId);
  DECODE(Offset, offset);
  DECODE_LA(val);
  DECODE_LA(key);
  Cell* c1 = m_stack.topC();
  Iter* it = frame_iter(m_fp, itId);
  TypedValue* tv1 = frame_local(m_fp, val);
  TypedValue* tv2 = frame_local(m_fp, key);
  if (initIterator(pc, origPc, it, offset, c1)) {
    tvAsVariant(tv1) = it->arr().second();
    tvAsVariant(tv2) = it->arr().first();
  }
}

OPTBLD_INLINE void ExecutionContext::iopWIterInit(IOP_ARGS) {
  PC origPc = pc;
  NEXT();
  DECODE_IA(itId);
  DECODE(Offset, offset);
  DECODE_LA(val);
  Cell* c1 = m_stack.topC();
  Iter* it = frame_iter(m_fp, itId);
  TypedValue* tv1 = frame_local(m_fp, val);
  if (initIterator(pc, origPc, it, offset, c1)) {
    tvAsVariant(tv1) = withRefBind(it->arr().secondRef());
  }
}

OPTBLD_INLINE void ExecutionContext::iopWIterInitK(IOP_ARGS) {
  PC origPc = pc;
  NEXT();
  DECODE_IA(itId);
  DECODE(Offset, offset);
  DECODE_LA(val);
  DECODE_LA(key);
  Cell* c1 = m_stack.topC();
  Iter* it = frame_iter(m_fp, itId);
  TypedValue* tv1 = frame_local(m_fp, val);
  TypedValue* tv2 = frame_local(m_fp, key);
  if (initIterator(pc, origPc, it, offset, c1)) {
    tvAsVariant(tv1) = withRefBind(it->arr().secondRef());
    tvAsVariant(tv2) = it->arr().first();
  }
}


inline bool ExecutionContext::initIteratorM(PC& pc, PC& origPc, Iter* it,
                                              Offset offset, Ref* r1,
                                              TypedValue *val,
                                              TypedValue *key) {
  bool hasElems = false;
  TypedValue* rtv = r1->m_data.pref->tv();
  if (rtv->m_type == KindOfArray) {
    hasElems = new_miter_array_key(it, r1->m_data.pref, val, key);
  } else if (rtv->m_type == KindOfObject)  {
    Class* ctx = arGetContextClass(g_context->getFP());
    hasElems = new_miter_object(it, r1->m_data.pref, ctx, val, key);
  } else {
    hasElems = new_miter_other(it, r1->m_data.pref);
  }

  if (!hasElems) {
    ITER_SKIP(offset);
  }

  m_stack.popV();
  return hasElems;
}

OPTBLD_INLINE void ExecutionContext::iopMIterInit(IOP_ARGS) {
  PC origPc = pc;
  NEXT();
  DECODE_IA(itId);
  DECODE(Offset, offset);
  DECODE_LA(val);
  Ref* r1 = m_stack.topV();
  assert(r1->m_type == KindOfRef);
  Iter* it = frame_iter(m_fp, itId);
  TypedValue* tv1 = frame_local(m_fp, val);
  initIteratorM(pc, origPc, it, offset, r1, tv1, nullptr);
}

OPTBLD_INLINE void ExecutionContext::iopMIterInitK(IOP_ARGS) {
  PC origPc = pc;
  NEXT();
  DECODE_IA(itId);
  DECODE(Offset, offset);
  DECODE_LA(val);
  DECODE_LA(key);
  Ref* r1 = m_stack.topV();
  assert(r1->m_type == KindOfRef);
  Iter* it = frame_iter(m_fp, itId);
  TypedValue* tv1 = frame_local(m_fp, val);
  TypedValue* tv2 = frame_local(m_fp, key);
  initIteratorM(pc, origPc, it, offset, r1, tv1, tv2);
}

OPTBLD_INLINE void ExecutionContext::iopIterNext(IOP_ARGS) {
  PC origPc = pc;
  NEXT();
  DECODE_IA(itId);
  DECODE(Offset, offset);
  DECODE_LA(val);
  Iter* it = frame_iter(m_fp, itId);
  TypedValue* tv1 = frame_local(m_fp, val);
  if (it->next()) {
    ITER_SKIP(offset);
    tvAsVariant(tv1) = it->arr().second();
  }
}

OPTBLD_INLINE void ExecutionContext::iopIterNextK(IOP_ARGS) {
  PC origPc = pc;
  NEXT();
  DECODE_IA(itId);
  DECODE(Offset, offset);
  DECODE_LA(val);
  DECODE_LA(key);
  Iter* it = frame_iter(m_fp, itId);
  TypedValue* tv1 = frame_local(m_fp, val);
  TypedValue* tv2 = frame_local(m_fp, key);
  if (it->next()) {
    ITER_SKIP(offset);
    tvAsVariant(tv1) = it->arr().second();
    tvAsVariant(tv2) = it->arr().first();
  }
}

OPTBLD_INLINE void ExecutionContext::iopWIterNext(IOP_ARGS) {
  PC origPc = pc;
  NEXT();
  DECODE_IA(itId);
  DECODE(Offset, offset);
  DECODE_LA(val);
  Iter* it = frame_iter(m_fp, itId);
  TypedValue* tv1 = frame_local(m_fp, val);
  if (it->next()) {
    ITER_SKIP(offset);
    tvAsVariant(tv1) = withRefBind(it->arr().secondRef());
  }
}

OPTBLD_INLINE void ExecutionContext::iopWIterNextK(IOP_ARGS) {
  PC origPc = pc;
  NEXT();
  DECODE_IA(itId);
  DECODE(Offset, offset);
  DECODE_LA(val);
  DECODE_LA(key);
  Iter* it = frame_iter(m_fp, itId);
  TypedValue* tv1 = frame_local(m_fp, val);
  TypedValue* tv2 = frame_local(m_fp, key);
  if (it->next()) {
    ITER_SKIP(offset);
    tvAsVariant(tv1) = withRefBind(it->arr().secondRef());
    tvAsVariant(tv2) = it->arr().first();
  }
}

OPTBLD_INLINE void ExecutionContext::iopMIterNext(IOP_ARGS) {
  PC origPc = pc;
  NEXT();
  DECODE_IA(itId);
  DECODE(Offset, offset);
  DECODE_LA(val);
  Iter* it = frame_iter(m_fp, itId);
  TypedValue* tv1 = frame_local(m_fp, val);
  if (miter_next_key(it, tv1, nullptr)) {
    ITER_SKIP(offset);
  }
}

OPTBLD_INLINE void ExecutionContext::iopMIterNextK(IOP_ARGS) {
  PC origPc = pc;
  NEXT();
  DECODE_IA(itId);
  DECODE(Offset, offset);
  DECODE_LA(val);
  DECODE_LA(key);
  Iter* it = frame_iter(m_fp, itId);
  TypedValue* tv1 = frame_local(m_fp, val);
  TypedValue* tv2 = frame_local(m_fp, key);
  if (miter_next_key(it, tv1, tv2)) {
    ITER_SKIP(offset);
  }
}

OPTBLD_INLINE void ExecutionContext::iopIterFree(IOP_ARGS) {
  NEXT();
  DECODE_IA(itId);
  Iter* it = frame_iter(m_fp, itId);
  it->free();
}

OPTBLD_INLINE void ExecutionContext::iopMIterFree(IOP_ARGS) {
  NEXT();
  DECODE_IA(itId);
  Iter* it = frame_iter(m_fp, itId);
  it->mfree();
}

OPTBLD_INLINE void ExecutionContext::iopCIterFree(IOP_ARGS) {
  NEXT();
  DECODE_IA(itId);
  Iter* it = frame_iter(m_fp, itId);
  it->cfree();
}

OPTBLD_INLINE void inclOp(ExecutionContext *ec, IOP_ARGS, InclOpFlags flags) {
  NEXT();
  Cell* c1 = ec->m_stack.topC();
  String path(prepareKey(*c1));
  bool initial;
  TRACE(2, "inclOp %s %s %s %s \"%s\"\n",
        flags & InclOpFlags::Once ? "Once" : "",
        flags & InclOpFlags::DocRoot ? "DocRoot" : "",
        flags & InclOpFlags::Relative ? "Relative" : "",
        flags & InclOpFlags::Fatal ? "Fatal" : "",
        path.data());

  Unit* u = flags & (InclOpFlags::DocRoot|InclOpFlags::Relative) ?
    ec->evalIncludeRoot(path.get(), flags, &initial) :
    ec->evalInclude(path.get(), ec->m_fp->m_func->unit()->filepath(), &initial);
  ec->m_stack.popC();
  if (u == nullptr) {
    ((flags & InclOpFlags::Fatal) ?
     (void (*)(const char *, ...))raise_error :
     (void (*)(const char *, ...))raise_warning)("File not found: %s",
                                                 path.data());
    ec->m_stack.pushFalse();
  } else {
    if (!(flags & InclOpFlags::Once) || initial) {
      ec->evalUnit(u, pc, EventHook::PseudoMain);
    } else {
      Stats::inc(Stats::PseudoMain_Guarded);
      ec->m_stack.pushTrue();
    }
  }
}

OPTBLD_INLINE void ExecutionContext::iopIncl(IOP_ARGS) {
  inclOp(this, IOP_PASS_ARGS, InclOpFlags::Default);
}

OPTBLD_INLINE void ExecutionContext::iopInclOnce(IOP_ARGS) {
  inclOp(this, IOP_PASS_ARGS, InclOpFlags::Once);
}

OPTBLD_INLINE void ExecutionContext::iopReq(IOP_ARGS) {
  inclOp(this, IOP_PASS_ARGS, InclOpFlags::Fatal);
}

OPTBLD_INLINE void ExecutionContext::iopReqOnce(IOP_ARGS) {
  inclOp(this, IOP_PASS_ARGS, InclOpFlags::Fatal | InclOpFlags::Once);
}

OPTBLD_INLINE void ExecutionContext::iopReqDoc(IOP_ARGS) {
  inclOp(this, IOP_PASS_ARGS,
    InclOpFlags::Fatal | InclOpFlags::Once | InclOpFlags::DocRoot);
}

OPTBLD_INLINE void ExecutionContext::iopEval(IOP_ARGS) {
  NEXT();
  Cell* c1 = m_stack.topC();

  if (UNLIKELY(RuntimeOption::EvalAuthoritativeMode)) {
    // Ahead of time whole program optimizations need to assume it can
    // see all the code, or it really can't do much.
    raise_error("You can't use eval in RepoAuthoritative mode");
  }

  String code(prepareKey(*c1));
  String prefixedCode = concat("<?php ", code);

  auto evalFilename = std::string();
  string_printf(
    evalFilename,
    "%s(%d) : eval()'d code",
    getContainingFileName().data(),
    getLine()
  );
  Unit* unit = compileEvalString(prefixedCode.get(), evalFilename.c_str());

  const StringData* msg;
  int line = 0;

  m_stack.popC();
  if (unit->parseFatal(msg, line)) {
    int errnum = static_cast<int>(ErrorConstants::ErrorModes::WARNING);
    if (errorNeedsLogging(errnum)) {
      // manual call to Logger instead of logError as we need to use
      // evalFileName and line as the exception doesn't track the eval()
      Logger::Error(
        "\nFatal error: %s in %s on line %d",
        msg->data(),
        evalFilename.c_str(),
        line
      );
    }

    m_stack.pushFalse();
    return;
  }
  evalUnit(unit, pc, EventHook::Eval);
}

OPTBLD_INLINE void ExecutionContext::iopDefFunc(IOP_ARGS) {
  NEXT();
  DECODE_IVA(fid);
  Func* f = m_fp->m_func->unit()->lookupFuncId(fid);
  setCachedFunc(f, isDebuggerAttached());
}

OPTBLD_INLINE void ExecutionContext::iopDefCls(IOP_ARGS) {
  NEXT();
  DECODE_IVA(cid);
  PreClass* c = m_fp->m_func->unit()->lookupPreClassId(cid);
  Unit::defClass(c);
}

OPTBLD_INLINE void ExecutionContext::iopNopDefCls(IOP_ARGS) {
  NEXT();
  DECODE_IVA(cid);
}

OPTBLD_INLINE void ExecutionContext::iopDefTypeAlias(IOP_ARGS) {
  NEXT();
  DECODE_IVA(tid);
  m_fp->m_func->unit()->defTypeAlias(tid);
}

static inline void checkThis(ActRec* fp) {
  if (!fp->hasThis()) {
    raise_error(Strings::FATAL_NULL_THIS);
  }
}

OPTBLD_INLINE void ExecutionContext::iopThis(IOP_ARGS) {
  NEXT();
  checkThis(m_fp);
  ObjectData* this_ = m_fp->getThis();
  m_stack.pushObject(this_);
}

OPTBLD_INLINE void ExecutionContext::iopBareThis(IOP_ARGS) {
  NEXT();
  DECODE_OA(BareThisOp, bto);
  if (m_fp->hasThis()) {
    ObjectData* this_ = m_fp->getThis();
    m_stack.pushObject(this_);
  } else {
    m_stack.pushNull();
    switch (bto) {
    case BareThisOp::Notice:   raise_notice(Strings::WARN_NULL_THIS); break;
    case BareThisOp::NoNotice: break;
    case BareThisOp::NeverNull:
      assert(!"$this cannot be null in BareThis with NeverNull option");
      break;
    }
  }
}

OPTBLD_INLINE void ExecutionContext::iopCheckThis(IOP_ARGS) {
  NEXT();
  checkThis(m_fp);
}

OPTBLD_INLINE void ExecutionContext::iopInitThisLoc(IOP_ARGS) {
  NEXT();
  DECODE_IVA(id);
  TypedValue* thisLoc = frame_local(m_fp, id);
  tvRefcountedDecRef(thisLoc);
  if (m_fp->hasThis()) {
    thisLoc->m_data.pobj = m_fp->getThis();
    thisLoc->m_type = KindOfObject;
    tvIncRef(thisLoc);
  } else {
    tvWriteUninit(thisLoc);
  }
}

static inline RefData* lookupStatic(StringData* name,
                                    const ActRec* fp,
                                    bool& inited) {
  auto const func = fp->m_func;

  if (UNLIKELY(func->isClosureBody())) {
    return lookupStaticFromClosure(
      frame_local(fp, func->numParams())->m_data.pobj, name, inited);
  }

  auto const refData = RDS::bindStaticLocal(func, name);
  inited = !refData->isUninitializedInRDS();
  if (!inited) refData->initInRDS();
  return refData.get();
}

OPTBLD_INLINE void ExecutionContext::iopStaticLoc(IOP_ARGS) {
  NEXT();
  DECODE_IVA(localId);
  DECODE_LITSTR(var);

  bool inited;
  auto const refData = lookupStatic(var, m_fp, inited);
  if (!inited) {
    refData->tv()->m_type = KindOfNull;
  }

  auto const tvLocal = frame_local(m_fp, localId);
  auto const tmpTV = make_tv<KindOfRef>(refData);
  tvBind(&tmpTV, tvLocal);
  if (inited) {
    m_stack.pushTrue();
  } else {
    m_stack.pushFalse();
  }
}

OPTBLD_INLINE void ExecutionContext::iopStaticLocInit(IOP_ARGS) {
  NEXT();
  DECODE_IVA(localId);
  DECODE_LITSTR(var);

  bool inited;
  auto const refData = lookupStatic(var, m_fp, inited);

  if (!inited) {
    auto const initVal = m_stack.topC();
    cellDup(*initVal, *refData->tv());
  }

  auto const tvLocal = frame_local(m_fp, localId);
  auto const tmpTV = make_tv<KindOfRef>(refData);
  tvBind(&tmpTV, tvLocal);
  m_stack.discard();
}

OPTBLD_INLINE void ExecutionContext::iopCatch(IOP_ARGS) {
  NEXT();
  assert(m_faults.size() > 0);
  Fault fault = m_faults.back();
  m_faults.pop_back();
  assert(fault.m_raiseFrame == m_fp);
  assert(fault.m_faultType == Fault::Type::UserException);
  m_stack.pushObjectNoRc(fault.m_userException);
}

OPTBLD_INLINE void ExecutionContext::iopLateBoundCls(IOP_ARGS) {
  NEXT();
  Class* cls = frameStaticClass(m_fp);
  if (!cls) {
    raise_error(HPHP::Strings::CANT_ACCESS_STATIC);
  }
  m_stack.pushClass(cls);
}

OPTBLD_INLINE void ExecutionContext::iopVerifyParamType(IOP_ARGS) {
  SYNC(); // We might need m_pc to be updated to throw.
  NEXT();

  DECODE_IVA(paramId);
  const Func *func = m_fp->m_func;
  assert(paramId < func->numParams());
  assert(func->numParams() == int(func->params().size()));
  const TypeConstraint& tc = func->params()[paramId].typeConstraint();
  assert(tc.hasConstraint());
  if (!tc.isTypeVar()) {
    tc.verifyParam(frame_local(m_fp, paramId), func, paramId);
  }
}

OPTBLD_INLINE void ExecutionContext::implVerifyRetType(IOP_ARGS) {
  if (LIKELY(!RuntimeOption::EvalCheckReturnTypeHints)) {
    NEXT();
    return;
  }
  SYNC();
  NEXT();
  const auto func = m_fp->m_func;
  const auto tc = func->returnTypeConstraint();
  if (!tc.isTypeVar()) {
    tc.verifyReturn(m_stack.topTV(), func);
  }
}

OPTBLD_INLINE void ExecutionContext::iopVerifyRetTypeC(PC& pc) {
  implVerifyRetType(IOP_PASS_ARGS);
}

OPTBLD_INLINE void ExecutionContext::iopVerifyRetTypeV(PC& pc) {
  implVerifyRetType(IOP_PASS_ARGS);
}

OPTBLD_INLINE void ExecutionContext::iopNativeImpl(IOP_ARGS) {
  NEXT();
  uint soff = m_fp->m_soff;
  BuiltinFunction func = m_fp->m_func->builtinFuncPtr();
  assert(func);
  // Actually call the native implementation. This will handle freeing the
  // locals in the normal case. In the case of an exception, the VM unwinder
  // will take care of it.
  func(m_fp);
  // Adjust the stack; the native implementation put the return value in the
  // right place for us already
  m_stack.ndiscard(m_fp->m_func->numSlotsInFrame());
  ActRec* sfp = m_fp->arGetSfp();
  if (LIKELY(sfp != m_fp)) {
    // Restore caller's execution state.
    m_fp = sfp;
    pc = m_fp->m_func->unit()->entry() + m_fp->m_func->base() + soff;
    m_stack.ret();
  } else {
    // No caller; terminate.
    m_stack.ret();
#ifdef HPHP_TRACE
    {
      std::ostringstream os;
      os << toStringElm(m_stack.topTV());
      ONTRACE(1,
              Trace::trace("Return %s from ExecutionContext::dispatch("
                           "%p)\n", os.str().c_str(), m_fp));
    }
#endif
    pc = 0;
  }
}

OPTBLD_INLINE void ExecutionContext::iopHighInvalid(IOP_ARGS) {
  fprintf(stderr, "invalid bytecode executed\n");
  abort();
}

OPTBLD_INLINE void ExecutionContext::iopSelf(IOP_ARGS) {
  NEXT();
  Class* clss = arGetContextClass(m_fp);
  if (!clss) {
    raise_error(HPHP::Strings::CANT_ACCESS_SELF);
  }
  m_stack.pushClass(clss);
}

OPTBLD_INLINE void ExecutionContext::iopParent(IOP_ARGS) {
  NEXT();
  Class* clss = arGetContextClass(m_fp);
  if (!clss) {
    raise_error(HPHP::Strings::CANT_ACCESS_PARENT_WHEN_NO_CLASS);
  }
  Class* parent = clss->parent();
  if (!parent) {
    raise_error(HPHP::Strings::CANT_ACCESS_PARENT_WHEN_NO_PARENT);
  }
  m_stack.pushClass(parent);
}

OPTBLD_INLINE void ExecutionContext::iopCreateCl(IOP_ARGS) {
  NEXT();
  DECODE_IVA(numArgs);
  DECODE_LITSTR(clsName);
  auto const cls = Unit::loadClass(clsName);
  auto const cl = static_cast<c_Closure*>(newInstance(cls));
  cl->init(numArgs, m_fp, m_stack.top());
  m_stack.ndiscard(numArgs);
  m_stack.pushObject(cl);
}

static inline void setContVar(const Func* func,
                              const StringData* name,
                              TypedValue* src,
                              ActRec* genFp) {
  Id destId = func->lookupVarId(name);
  if (destId != kInvalidId) {
    // Copy the value of the local to the cont object and set the
    // local to uninit so that we don't need to change refcounts.
    tvCopy(*src, *frame_local(genFp, destId));
    tvWriteUninit(src);
  } else {
    if (!genFp->hasVarEnv()) {
      genFp->setVarEnv(VarEnv::createLocal(genFp));
    }
    genFp->getVarEnv()->setWithRef(name, src);
  }
}

const StaticString s_this("this");

void ExecutionContext::fillContinuationVars(const Func* func,
                                              ActRec* origFp,
                                              ActRec* genFp) {
  // For functions that contain only named locals, the variable
  // environment is saved and restored by teleporting the values (and
  // their references) between the evaluation stack and the local
  // space at the end of the object using memcpy. Any variables in a
  // VarEnv are saved and restored from m_vars as usual.
  static const StringData* thisStr = s_this.get();
  bool skipThis;
  Id firstLocal;
  if (origFp->hasVarEnv()) {
    // This is currently never executed but it will be needed for eager
    // execution of async functions - should be revisited later.
    assert(false);
    Stats::inc(Stats::Cont_CreateVerySlow);
    Array definedVariables = origFp->getVarEnv()->getDefinedVariables();
    skipThis = definedVariables.exists(s_this, true);
    firstLocal = func->numNamedLocals();

    for (ArrayIter iter(definedVariables); !iter.end(); iter.next()) {
      setContVar(func, iter.first().getStringData(),
        const_cast<TypedValue*>(iter.secondRef().asTypedValue()), genFp);
    }
  } else {
    skipThis = func->lookupVarId(thisStr) != kInvalidId;
    firstLocal = 0;
  }

  for (Id i = firstLocal; i < func->numLocals(); ++i) {
    TypedValue* src = frame_local(origFp, i);
    tvCopy(*src, *frame_local(genFp, i));
    tvWriteUninit(src);
  }

  // If $this is used as a local inside the body and is not provided
  // by our containing environment, just prefill it here instead of
  // using InitThisLoc inside the body
  if (!skipThis && origFp->hasThis()) {
    Id id = func->lookupVarId(thisStr);
    if (id != kInvalidId) {
      tvAsVariant(frame_local(genFp, id)) = origFp->getThis();
    }
  }
}

OPTBLD_INLINE void ExecutionContext::iopCreateCont(IOP_ARGS) {
  NEXT();
  DECODE(Offset, offset);

  const Func* func = m_fp->m_func;
  offset += func->unit()->offsetOf(m_pc);

  c_Continuation* cont = static_cast<c_Continuation*>(func->isMethod()
    ? c_Continuation::CreateMeth(func, m_fp->getThisOrClass(), offset)
    : c_Continuation::CreateFunc(func, offset));

  fillContinuationVars(func, m_fp, cont->actRec());

  TypedValue* ret = m_stack.allocTV();
  ret->m_type = KindOfObject;
  ret->m_data.pobj = cont;
}

static inline c_Continuation* this_continuation(const ActRec* fp) {
  ObjectData* obj = fp->getThis();
  assert(obj->instanceof(c_Continuation::classof()));
  return static_cast<c_Continuation*>(obj);
}

OPTBLD_INLINE void ExecutionContext::contEnterImpl(IOP_ARGS) {
  NEXT();

  // The stack must have one cell! Or else generatorStackBase() won't work!
  assert(m_stack.top() + 1 ==
         (TypedValue*)m_fp - m_fp->m_func->numSlotsInFrame());

  // Do linkage of the continuation's AR.
  assert(m_fp->hasThis());
  c_Continuation* cont = this_continuation(m_fp);
  ActRec* contAR = cont->actRec();
  arSetSfp(contAR, m_fp);

  contAR->m_soff = m_fp->m_func->unit()->offsetOf(pc) -
    (uintptr_t)m_fp->m_func->base();
  contAR->m_savedRip =
    reinterpret_cast<uintptr_t>(tx->uniqueStubs.genRetHelper);
  assert(isReturnHelper(contAR->m_savedRip));

  m_fp = contAR;

  assert(contAR->func()->contains(cont->m_offset));
  pc = contAR->func()->unit()->at(cont->m_offset);
  SYNC();
}

OPTBLD_INLINE void ExecutionContext::iopContEnter(IOP_ARGS) {
  contEnterImpl(IOP_PASS_ARGS);

  if (UNLIKELY(!EventHook::FunctionEnter(m_fp, EventHook::NormalFunc))) {
    pc = m_pc;
  }
}

OPTBLD_INLINE void ExecutionContext::iopContRaise(IOP_ARGS) {
  contEnterImpl(IOP_PASS_ARGS);

  if (UNLIKELY(!EventHook::FunctionEnter(m_fp, EventHook::NormalFunc))) {
    pc = m_pc;
  } else {
    iopThrow(IOP_PASS_ARGS);
  }
}

OPTBLD_INLINE void ExecutionContext::iopContSuspend(IOP_ARGS) {
  NEXT();

  auto cont = frame_continuation(m_fp);
  auto offset = m_fp->func()->unit()->offsetOf(pc);
  cont->suspend(offset, *m_stack.topC());
  m_stack.popTV();

  EventHook::FunctionExit(m_fp);
  ActRec* prevFp = m_fp->arGetSfp();
  if (prevFp == m_fp) {
    pc = nullptr;
    m_fp = nullptr;
  } else {
    pc = prevFp->m_func->getEntry() + m_fp->m_soff;
    m_fp = prevFp;
  }
}

OPTBLD_INLINE void ExecutionContext::iopContSuspendK(IOP_ARGS) {
  NEXT();

  auto cont = frame_continuation(m_fp);
  auto offset = m_fp->func()->unit()->offsetOf(pc);
  cont->suspend(offset, *m_stack.indC(1), *m_stack.topC());
  m_stack.popTV();
  m_stack.popTV();

  EventHook::FunctionExit(m_fp);
  ActRec* prevFp = m_fp->arGetSfp();
  pc = prevFp->m_func->getEntry() + m_fp->m_soff;
  m_fp = prevFp;
}

OPTBLD_INLINE void ExecutionContext::iopContRetC(IOP_ARGS) {
  NEXT();
  c_Continuation* cont = frame_continuation(m_fp);
  cont->setDone();
  tvSetIgnoreRef(*m_stack.topC(), cont->m_value);
  m_stack.popC();

  EventHook::FunctionExit(m_fp);
  ActRec* prevFp = m_fp->arGetSfp();
  if (prevFp == m_fp) {
    pc = nullptr;
    m_fp = nullptr;
  } else {
    pc = prevFp->m_func->getEntry() + m_fp->m_soff;
    m_fp = prevFp;
  }
}

OPTBLD_INLINE void ExecutionContext::iopContCheck(IOP_ARGS) {
  NEXT();
  DECODE_IVA(check_started);
  c_Continuation* cont = this_continuation(m_fp);
  if (check_started) {
    cont->startedCheck();
  }
  cont->preNext();
}

OPTBLD_INLINE void ExecutionContext::iopContValid(IOP_ARGS) {
  NEXT();
  TypedValue* tv = m_stack.allocTV();
  tvWriteUninit(tv);
  tvAsVariant(tv) = !this_continuation(m_fp)->done();
}

OPTBLD_INLINE void ExecutionContext::iopContKey(IOP_ARGS) {
  NEXT();
  c_Continuation* cont = this_continuation(m_fp);
  cont->startedCheck();
  cellDup(cont->m_key, *m_stack.allocC());
}

OPTBLD_INLINE void ExecutionContext::iopContCurrent(IOP_ARGS) {
  NEXT();
  c_Continuation* cont = this_continuation(m_fp);
  cont->startedCheck();
  cellDup(cont->m_value, *m_stack.allocC());
}

OPTBLD_INLINE void ExecutionContext::iopContStopped(IOP_ARGS) {
  NEXT();
  this_continuation(m_fp)->setStopped();
}

OPTBLD_INLINE void ExecutionContext::iopContHandle(IOP_ARGS) {
  NEXT();
  c_Continuation* cont = this_continuation(m_fp);
  cont->setDone();
  cellSet(make_tv<KindOfNull>(), cont->m_value);

  Variant exn = tvAsVariant(m_stack.topTV());
  m_stack.popC();
  assert(exn.asObjRef().instanceof(SystemLib::s_ExceptionClass));
  throw exn.asObjRef();
}

OPTBLD_INLINE void ExecutionContext::iopAsyncAwait(IOP_ARGS) {
  NEXT();
  auto const& c1 = *m_stack.topC();
  if (c1.m_type != KindOfObject ||
      !c1.m_data.pobj->getAttribute(ObjectData::IsWaitHandle)) {
    raise_error("AsyncAwait on a non-WaitHandle");
  }
  auto const wh = static_cast<c_WaitHandle*>(c1.m_data.pobj);
  if (wh->isSucceeded()) {
    cellSet(wh->getResult(), *m_stack.topC());
    m_stack.pushTrue();
    return;
  }
  if (wh->isFailed()) throw Object(wh->getException());
  m_stack.pushFalse();
}

OPTBLD_INLINE void ExecutionContext::iopAsyncESuspend(IOP_ARGS) {
  NEXT();
  DECODE(Offset, offset);
  DECODE_IVA(iters);

  const Func* func = m_fp->m_func;
  offset += func->unit()->offsetOf(m_pc);

  Cell* value = m_stack.topC();
  assert(value->m_type == KindOfObject);
  assert(value->m_data.pobj->instanceof(c_WaitableWaitHandle::classof()));

  auto child = static_cast<c_WaitableWaitHandle*>(value->m_data.pobj);
  assert(!child->isFinished());

  auto waitHandle = static_cast<c_AsyncFunctionWaitHandle*>(func->isMethod()
    ? c_AsyncFunctionWaitHandle::CreateMeth(func, m_fp->getThisOrClass(),
                                            offset, child)
    : c_AsyncFunctionWaitHandle::CreateFunc(func, offset, child));

  m_stack.discard();

  fillContinuationVars(func, m_fp, waitHandle->getActRec());

  // copy the state of all the iterators at once
  memcpy(frame_iter(waitHandle->getActRec(), iters-1),
         frame_iter(m_fp, iters-1),
         iters * sizeof(Iter));

  TypedValue* ret = m_stack.allocTV();
  ret->m_type = KindOfObject;
  ret->m_data.pobj = waitHandle;
}

OPTBLD_INLINE void ExecutionContext::iopAsyncResume(IOP_ARGS) {
  NEXT();
}

OPTBLD_INLINE void ExecutionContext::iopAsyncWrapResult(IOP_ARGS) {
  NEXT();

  auto const top = m_stack.topC();
  top->m_data.pobj = c_StaticResultWaitHandle::CreateFromVM(*top);
  top->m_type = KindOfObject;
}

template<class Op>
OPTBLD_INLINE void ExecutionContext::roundOpImpl(Op op) {
  TypedValue* val = m_stack.topTV();

  tvCastToDoubleInPlace(val);
  val->m_data.dbl = op(val->m_data.dbl);
}

OPTBLD_INLINE void ExecutionContext::iopFloor(IOP_ARGS) {
  NEXT();
  roundOpImpl(floor);
}

OPTBLD_INLINE void ExecutionContext::iopCeil(IOP_ARGS) {
  NEXT();
  roundOpImpl(ceil);
}

OPTBLD_INLINE void ExecutionContext::iopCheckProp(IOP_ARGS) {
  NEXT();
  DECODE_LITSTR(propName);

  auto* cls = m_fp->getClass();
  auto* propVec = cls->getPropData();
  always_assert(propVec);

  auto* ctx = arGetContextClass(getFP());
  auto idx = ctx->lookupDeclProp(propName);

  auto& tv = (*propVec)[idx];
  if (tv.m_type != KindOfUninit) {
    m_stack.pushTrue();
  } else {
    m_stack.pushFalse();
  }
}

OPTBLD_INLINE void ExecutionContext::iopInitProp(IOP_ARGS) {
  NEXT();
  DECODE_LITSTR(propName);
  DECODE_OA(InitPropOp, propOp);

  auto* cls = m_fp->getClass();
  TypedValue* tv;
  Slot idx;

  auto* ctx = arGetContextClass(getFP());
  auto* fr = m_stack.topC();

  switch (propOp) {
    case InitPropOp::Static: {
      auto* propVec = cls->getSPropData();
      always_assert(propVec);
      idx = ctx->lookupSProp(propName);
      tv = &propVec[idx];
    } break;
    case InitPropOp::NonStatic: {
      auto* propVec = cls->getPropData();
      always_assert(propVec);
      idx = ctx->lookupDeclProp(propName);
      tv = &(*propVec)[idx];
    } break;
  }

  cellDup(*fr, *tvToCell(tv));
  m_stack.popC();
}

OPTBLD_INLINE void ExecutionContext::iopStrlen(IOP_ARGS) {
  NEXT();
  TypedValue* subj = m_stack.topTV();
  if (LIKELY(IS_STRING_TYPE(subj->m_type))) {
    int64_t ans = subj->m_data.pstr->size();
    tvRefcountedDecRef(subj);
    subj->m_type = KindOfInt64;
    subj->m_data.num = ans;
  } else {
    Variant ans = f_strlen(tvAsVariant(subj));
    tvAsVariant(subj) = ans;
  }
}

OPTBLD_INLINE void ExecutionContext::iopIncStat(IOP_ARGS) {
  NEXT();
  DECODE_IVA(counter);
  DECODE_IVA(value);
  Stats::inc(Stats::StatCounter(counter), value);
}

void ExecutionContext::classExistsImpl(IOP_ARGS, Attr typeAttr) {
  NEXT();
  TypedValue* aloadTV = m_stack.topTV();
  tvCastToBooleanInPlace(aloadTV);
  assert(aloadTV->m_type == KindOfBoolean);
  bool autoload = aloadTV->m_data.num;
  m_stack.popX();

  TypedValue* name = m_stack.topTV();
  tvCastToStringInPlace(name);
  assert(IS_STRING_TYPE(name->m_type));

  tvAsVariant(name) = Unit::classExists(name->m_data.pstr, autoload, typeAttr);
}

OPTBLD_INLINE void ExecutionContext::iopClassExists(IOP_ARGS) {
  classExistsImpl(IOP_PASS_ARGS, AttrNone);
}

OPTBLD_INLINE void ExecutionContext::iopInterfaceExists(IOP_ARGS) {
  classExistsImpl(IOP_PASS_ARGS, AttrInterface);
}

OPTBLD_INLINE void ExecutionContext::iopTraitExists(IOP_ARGS) {
  classExistsImpl(IOP_PASS_ARGS, AttrTrait);
}

string
ExecutionContext::prettyStack(const string& prefix) const {
  if (!getFP()) {
    string s("__Halted");
    return s;
  }
  int offset = (m_fp->m_func->unit() != nullptr)
               ? pcOff(this)
               : 0;
  string begPrefix = prefix + "__";
  string midPrefix = prefix + "|| ";
  string endPrefix = prefix + "\\/";
  string stack = m_stack.toString(m_fp, offset, midPrefix);
  return begPrefix + "\n" + stack + endPrefix;
}

void ExecutionContext::checkRegStateWork() const {
  assert(JIT::tl_regState == JIT::VMRegState::CLEAN);
}

void ExecutionContext::DumpStack() {
  string s = g_context->prettyStack("");
  fprintf(stderr, "%s\n", s.c_str());
}

void ExecutionContext::DumpCurUnit(int skip) {
  ActRec* fp = g_context->getFP();
  Offset pc = fp->m_func->unit() ? pcOff(g_context.getNoCheck()) : 0;
  while (skip--) {
    fp = g_context->getPrevVMState(fp, &pc);
  }
  if (fp == nullptr) {
    std::cout << "Don't have a valid fp\n";
    return;
  }

  printf("Offset = %d, in function %s\n", pc, fp->m_func->name()->data());
  Unit* u = fp->m_func->unit();
  if (u == nullptr) {
    std::cout << "Current unit is NULL\n";
    return;
  }
  printf("Dumping bytecode for %s(%p)\n", u->filepath()->data(), u);
  std::cout << u->toString();
}

void ExecutionContext::PrintTCCallerInfo() {
  VMRegAnchor _;
  ActRec* fp = g_context->getFP();
  Unit* u = fp->m_func->unit();
  fprintf(stderr, "Called from TC address %p\n",
          mcg->getTranslatedCaller());
  std::cerr << u->filepath()->data() << ':'
            << u->getLineNumber(u->offsetOf(g_context->getPC())) << '\n';
}

static inline void
condStackTraceSep(const char* pfx) {
  TRACE(3, "%s"
        "========================================"
        "========================================\n",
        pfx);
}

#define COND_STACKTRACE(pfx)                                                  \
  ONTRACE(3,                                                                  \
          string stack = prettyStack(pfx);                                    \
          Trace::trace("%s\n", stack.c_str());)

#define O(name, imm, pusph, pop, flags)                     \
void ExecutionContext::op##name() {                         \
  condStackTraceSep("op"#name" ");                          \
  COND_STACKTRACE("op"#name" pre:  ");                      \
  PC pc = m_pc;                                             \
  assert(*reinterpret_cast<const Op*>(pc) == Op##name);     \
  ONTRACE(1,                                                \
          auto offset = m_fp->m_func->unit()->offsetOf(pc); \
          Trace::trace("op"#name" offset: %d\n", offset));  \
  iop##name(IOP_PASS_ARGS);                                 \
  SYNC();                                                   \
  COND_STACKTRACE("op"#name" post: ");                      \
  condStackTraceSep("op"#name" ");                          \
}

OPCODES

#undef O
#undef NEXT
#undef DECODE_JMP
#undef DECODE

static inline void
profileReturnValue(const DataType dt) {
  const Func* f = liveFunc();
  if (f->isPseudoMain() || f->isClosureBody() || f->isMagic() ||
      Func::isSpecial(f->name()))
    return;
  recordType(TypeProfileKey(TypeProfileKey::MethodName, f->name()), dt);
}

template <int dispatchFlags>
inline void ExecutionContext::dispatchImpl(int numInstrs) {
  static const bool limInstrs = dispatchFlags & LimitInstrs;
  static const bool breakOnCtlFlow = dispatchFlags & BreakOnCtlFlow;
  static const bool profile = dispatchFlags & Profile;
  static const void *optabDirect[] = {
#define O(name, imm, push, pop, flags) \
    &&Label##name,
    OPCODES
#undef O
  };
  static const void *optabDbg[] = {
#define O(name, imm, push, pop, flags) \
    &&LabelDbg##name,
    OPCODES
#undef O
  };
  static const void *optabCover[] = {
#define O(name, imm, push, pop, flags) \
    &&LabelCover##name,
    OPCODES
#undef O
  };
  assert(sizeof(optabDirect) / sizeof(const void *) == Op_count);
  assert(sizeof(optabDbg) / sizeof(const void *) == Op_count);
  const void **optab = optabDirect;
  bool collectCoverage = ThreadInfo::s_threadInfo->
    m_reqInjectionData.getCoverage();
  if (collectCoverage) {
    optab = optabCover;
  }
  DEBUGGER_ATTACHED_ONLY(optab = optabDbg);
  /*
   * Trace-only mapping of opcodes to names.
   */
#ifdef HPHP_TRACE
  static const char *nametab[] = {
#define O(name, imm, push, pop, flags) \
    #name,
    OPCODES
#undef O
  };
#endif /* HPHP_TRACE */
  bool isCtlFlow = false;

#define DISPATCH() do {                                                 \
    if ((breakOnCtlFlow && isCtlFlow) ||                                \
        (limInstrs && UNLIKELY(numInstrs-- == 0))) {                    \
      ONTRACE(1,                                                        \
              Trace::trace("dispatch: Halt ExecutionContext::dispatch(%p)\n", \
                           m_fp));                                      \
      return;                                                           \
    }                                                                   \
    Op op = *reinterpret_cast<const Op*>(pc);                           \
    COND_STACKTRACE("dispatch:                    ");                   \
    ONTRACE(1,                                                          \
            Trace::trace("dispatch: %d: %s\n", pcOff(this),             \
                         nametab[uint8_t(op)]));                        \
    if (profile && (op == OpRetC || op == OpRetV)) {                    \
      const_cast<Func*>(liveFunc())->incProfCounter();                  \
      profileReturnValue(m_stack.top()->m_type);                        \
    }                                                                   \
    goto *optab[uint8_t(op)];                                           \
} while (0)

  ONTRACE(1, Trace::trace("dispatch: Enter ExecutionContext::dispatch(%p)\n",
          m_fp));
  PC pc = m_pc;
  DISPATCH();

#define O(name, imm, pusph, pop, flags)                       \
  LabelDbg##name:                                             \
    phpDebuggerOpcodeHook(pc);                                \
  LabelCover##name:                                           \
    if (collectCoverage) {                                    \
      recordCodeCoverage(pc);                                 \
    }                                                         \
  Label##name: {                                              \
    iop##name(pc);                                            \
    SYNC();                                                   \
    if (breakOnCtlFlow) {                                     \
      isCtlFlow = instrIsControlFlow(Op::name);               \
      Stats::incOp(Op::name);                                 \
    }                                                         \
    if (UNLIKELY(!pc)) {                                      \
      DEBUG_ONLY const Op op = Op::name;                      \
      assert(op == OpRetC || op == OpRetV ||                  \
             op == OpContSuspend || op == OpContSuspendK ||   \
             op == OpContRetC || op == OpNativeImpl);         \
      m_fp = 0;                                               \
      return;                                                 \
    }                                                         \
    DISPATCH();                                               \
  }
  OPCODES
#undef O
#undef DISPATCH
}

void ExecutionContext::dispatch() {
  if (shouldProfile()) {
    dispatchImpl<Profile>(0);
  } else {
    dispatchImpl<0>(0);
  }
}

// We are about to go back to translated code, check whether we should
// stick with the interpreter. NB: if we've just executed a return
// from pseudomain, then there's no PC and no more code to interpret.
void ExecutionContext::switchModeForDebugger() {
  if (DEBUGGER_FORCE_INTR && (getPC() != 0)) {
    throw VMSwitchMode();
  }
}

void ExecutionContext::dispatchN(int numInstrs) {
  if (Trace::moduleEnabled(Trace::dispatchN)) {
    auto cat = makeStaticString("dispatchN");
    auto name = makeStaticString(
      folly::format("{} ops @ {}",
                    numInstrs, show(SrcKey(m_fp->m_func, m_pc))).str());
    Stats::incStatGrouped(cat, name, 1);
  }

  dispatchImpl<LimitInstrs | BreakOnCtlFlow>(numInstrs);
  switchModeForDebugger();
}

void ExecutionContext::dispatchBB() {
  if (Trace::moduleEnabled(Trace::dispatchBB)) {
    auto cat = makeStaticString("dispatchBB");
    auto name = makeStaticString(show(SrcKey(m_fp->m_func, m_pc)));
    Stats::incStatGrouped(cat, name, 1);
  }

  dispatchImpl<BreakOnCtlFlow>(0);
  switchModeForDebugger();
}

void ExecutionContext::recordCodeCoverage(PC pc) {
  Unit* unit = getFP()->m_func->unit();
  assert(unit != nullptr);
  if (unit == SystemLib::s_nativeFuncUnit ||
      unit == SystemLib::s_nativeClassUnit ||
      unit == SystemLib::s_hhas_unit) {
    return;
  }
  int line = unit->getLineNumber(pcOff(this));
  assert(line != -1);

  if (unit != m_coverPrevUnit || line != m_coverPrevLine) {
    ThreadInfo* info = ThreadInfo::s_threadInfo.getNoCheck();
    m_coverPrevUnit = unit;
    m_coverPrevLine = line;
    const StringData* filepath = unit->filepath();
    assert(filepath->isStatic());
    info->m_coverage->Record(filepath->data(), line, line);
  }
}

void ExecutionContext::resetCoverageCounters() {
  m_coverPrevLine = -1;
  m_coverPrevUnit = nullptr;
}

void ExecutionContext::pushVMState(Cell* savedSP) {
  if (UNLIKELY(!m_fp)) {
    // first entry
    assert(m_nestedVMs.size() == 0);
    return;
  }

  VMState savedVM = { getPC(), getFP(), m_firstAR, savedSP };
  TRACE(3, "savedVM: %p %p %p %p\n", m_pc, m_fp, m_firstAR, savedSP);

  if (debug && savedVM.fp &&
      savedVM.fp->m_func &&
      savedVM.fp->m_func->unit()) {
    // Some asserts and tracing.
    const Func* func = savedVM.fp->m_func;
    (void) /* bound-check asserts in offsetOf */
      func->unit()->offsetOf(savedVM.pc);
    TRACE(3, "pushVMState: saving frame %s pc %p off %d fp %p\n",
          func->name()->data(),
          savedVM.pc,
          func->unit()->offsetOf(savedVM.pc),
          savedVM.fp);
  }
  m_nestedVMs.push_back(savedVM);
  m_nesting++;
}

void ExecutionContext::popVMState() {
  if (UNLIKELY(m_nestedVMs.empty())) {
    // last exit
    m_fp = nullptr;
    m_pc = nullptr;
    m_firstAR = nullptr;
    return;
  }

  assert(m_nestedVMs.size() >= 1);

  VMState &savedVM = m_nestedVMs.back();
  m_pc = savedVM.pc;
  m_fp = savedVM.fp;
  m_firstAR = savedVM.firstAR;
  assert(m_stack.top() == savedVM.sp);

  if (debug) {
    if (savedVM.fp &&
        savedVM.fp->m_func &&
        savedVM.fp->m_func->unit()) {
      const Func* func = savedVM.fp->m_func;
      (void) /* bound-check asserts in offsetOf */
        func->unit()->offsetOf(savedVM.pc);
      TRACE(3, "popVMState: restoring frame %s pc %p off %d fp %p\n",
            func->name()->data(),
            savedVM.pc,
            func->unit()->offsetOf(savedVM.pc),
            savedVM.fp);
    }
  }

  m_nestedVMs.pop_back();
  m_nesting--;

  TRACE(1, "Reentry: exit fp %p pc %p\n", m_fp, m_pc);
}

static void threadLogger(const char* header, const char* msg,
                         const char* ending, void* data) {
  auto* ec = static_cast<ExecutionContext*>(data);
  ec->write(header);
  ec->write(msg);
  ec->write(ending);
  ec->flush();
}

void ExecutionContext::requestInit() {
  assert(SystemLib::s_unit);
  assert(SystemLib::s_nativeFuncUnit);
  assert(SystemLib::s_nativeClassUnit);

  EnvConstants::requestInit(smart_new<EnvConstants>());
  VarEnv::createGlobal();
  m_stack.requestInit();
  ObjectData::resetMaxId();
  ResourceData::resetMaxId();
  mcg->requestInit();

  if (UNLIKELY(RuntimeOption::EvalJitEnableRenameFunction)) {
    SystemLib::s_unit->merge();
    Extension::MergeSystemlib();
    if (SystemLib::s_hhas_unit) SystemLib::s_hhas_unit->merge();
    SystemLib::s_nativeFuncUnit->merge();
    SystemLib::s_nativeClassUnit->merge();
  } else {
    // System units are always merge only, and
    // everything is persistent.
    assert(SystemLib::s_unit->isEmpty());
    assert(!SystemLib::s_hhas_unit || SystemLib::s_hhas_unit->isEmpty());
    assert(SystemLib::s_nativeFuncUnit->isEmpty());
    assert(SystemLib::s_nativeClassUnit->isEmpty());
  }

  profileRequestStart();

  MemoryProfile::startProfiling();

#ifdef DEBUG
  Class* cls = Unit::GetNamedEntity(s_stdclass.get())->clsList();
  assert(cls);
  assert(cls == SystemLib::s_stdclassClass);
#endif

  if (Logger::UseRequestLog) Logger::SetThreadHook(&threadLogger, this);
}

void ExecutionContext::requestExit() {
  MemoryProfile::finishProfiling();

  manageAPCHandle();
  syncGdbState();
  mcg->requestExit();
  m_stack.requestExit();
  profileRequestEnd();
  EventHook::Disable();
  EnvConstants::requestExit();

  if (m_globalVarEnv) {
    VarEnv::destroy(m_globalVarEnv);
    m_globalVarEnv = 0;
  }

  if (Logger::UseRequestLog) Logger::SetThreadHook(nullptr, nullptr);
}

///////////////////////////////////////////////////////////////////////////////
}
