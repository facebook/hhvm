/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include <boost/filesystem.hpp>

#include <libgen.h>
#include <sys/mman.h>

#include <folly/String.h>

#include "hphp/util/debug.h"
#include "hphp/util/portability.h"
#include "hphp/util/ringbuffer.h"
#include "hphp/util/text-util.h"
#include "hphp/util/trace.h"

#include "hphp/compiler/builtin_symbols.h"

#include "hphp/system/systemlib.h"

#include "hphp/runtime/base/apc-stats.h"
#include "hphp/runtime/base/apc-typed-value.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/class-info.h"
#include "hphp/runtime/base/code-coverage.h"
#include "hphp/runtime/base/collections.h"
#include "hphp/runtime/base/container-functions.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/externals.h"
#include "hphp/runtime/base/hhprof.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/repo-auth-type-codec.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/shape.h"
#include "hphp/runtime/base/stat-cache.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/base/strings.h"
#include "hphp/runtime/base/struct-array.h"
#include "hphp/runtime/base/tv-arith.h"
#include "hphp/runtime/base/tv-comparisons.h"
#include "hphp/runtime/base/tv-conversions.h"
#include "hphp/runtime/base/unit-cache.h"

#include "hphp/runtime/ext/array/ext_array.h"
#include "hphp/runtime/ext/asio/ext_async-function-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_async-generator-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_async-generator.h"
#include "hphp/runtime/ext/asio/ext_static-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_wait-handle.h"
#include "hphp/runtime/ext/asio/ext_waitable-wait-handle.h"
#include "hphp/runtime/ext/std/ext_std_closure.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/generator/ext_generator.h"
#include "hphp/runtime/ext/hh/ext_hh.h"
#include "hphp/runtime/ext/reflection/ext_reflection.h"
#include "hphp/runtime/ext/std/ext_std_function.h"
#include "hphp/runtime/ext/std/ext_std_math.h"
#include "hphp/runtime/ext/std/ext_std_variable.h"
#include "hphp/runtime/ext/string/ext_string.h"
#include "hphp/runtime/ext/hash/hash_murmur.h"

#include "hphp/runtime/server/rpc-request-handler.h"
#include "hphp/runtime/server/source-root-info.h"

#include "hphp/runtime/vm/act-rec.h"
#include "hphp/runtime/vm/act-rec-defs.h"
#include "hphp/runtime/vm/debug/debug.h"
#include "hphp/runtime/vm/debugger-hook.h"
#include "hphp/runtime/vm/event-hook.h"
#include "hphp/runtime/vm/func-inline.h"
#include "hphp/runtime/vm/globals-array.h"
#include "hphp/runtime/vm/hh-utils.h"
#include "hphp/runtime/vm/hhbc-codec.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/interp-helpers.h"
#include "hphp/runtime/vm/member-operations.h"
#include "hphp/runtime/vm/native.h"
#include "hphp/runtime/vm/php-debug.h"
#include "hphp/runtime/vm/repo-global-data.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/resumable.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/srckey.h"
#include "hphp/runtime/vm/type-constraint.h"
#include "hphp/runtime/vm/type-profile.h"
#include "hphp/runtime/vm/unwind.h"

#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/perf-counters.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/translator-runtime.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/unwind-itanium.h"


namespace HPHP {

TRACE_SET_MOD(bcinterp);

// TODO: #1746957, #1756122
// we should skip the call in call_user_func_array, if
// by reference params are passed by value, or if its
// argument is not an array, but currently lots of tests
// depend on actually making the call.
const bool skipCufOnInvalidParams = false;

// RepoAuthoritative has been raptured out of runtime_option.cpp. It needs
// to be closer to other bytecode.cpp data.
bool RuntimeOption::RepoAuthoritative = false;

using jit::mcg;
using jit::TCA;

#define IOP_ARGS        PC& pc
// GCC 4.8 has some real problems with all the inlining in this file, so don't
// go overboard with that version.
#if DEBUG || ((__GNUC__ == 4) && (__GNUC_MINOR__ == 8))
#define OPTBLD_INLINE
#define OPTBLD_FLT_INLINE
#else
#define OPTBLD_INLINE       ALWAYS_INLINE
#define OPTBLD_FLT_INLINE   INLINE_FLATTEN
#endif

bool isVMFrame(const ActRec* ar) {
  assert(ar);
  // Determine whether the frame pointer is outside the native stack, cleverly
  // using a single unsigned comparison to do both halves of the bounds check.
  bool ret = uintptr_t(ar) - s_stackLimit >= s_stackSize;
  assert(!ret || isValidVMStackAddress(ar) ||
         (ar->m_func->validate(), ar->resumed()));
  return ret;
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

void frame_free_locals_no_hook(ActRec* fp) {
  frame_free_locals_inl_no_hook<false>(fp, fp->func()->numLocals());
}

const StaticString s_call_user_func("call_user_func");
const StaticString s_call_user_func_array("call_user_func_array");
const StaticString s___call("__call");
const StaticString s___callStatic("__callStatic");
const StaticString s_file("file");
const StaticString s_line("line");
const StaticString s_getWaitHandle("getWaitHandle");

///////////////////////////////////////////////////////////////////////////////

//=============================================================================
// Miscellaneous decoders.

inline const char* prettytype(int) { return "int"; }
inline const char* prettytype(long) { return "long"; }
inline const char* prettytype(long long) { return "long long"; }
inline const char* prettytype(double) { return "double"; }
inline const char* prettytype(unsigned) { return "unsigned"; }
inline const char* prettytype(OODeclExistsOp) { return "OpDeclExistsOp"; }
inline const char* prettytype(FatalOp) { return "FatalOp"; }
inline const char* prettytype(IsTypeOp) { return "IsTypeOp"; }
inline const char* prettytype(SetOpOp) { return "SetOpOp"; }
inline const char* prettytype(IncDecOp) { return "IncDecOp"; }
inline const char* prettytype(ObjMethodOp) { return "ObjMethodOp"; }
inline const char* prettytype(BareThisOp) { return "BareThisOp"; }
inline const char* prettytype(InitPropOp) { return "InitPropOp"; }
inline const char* prettytype(SilenceOp) { return "SilenceOp"; }
inline const char* prettytype(SwitchKind) { return "SwitchKind"; }
inline const char* prettytype(MOpFlags) { return "MOpFlags"; }
inline const char* prettytype(QueryMOp) { return "QueryMOp"; }

// load a T value from *pc without incrementing
template<class T> T peek(PC pc) {
  T v;
  std::memcpy(&v, pc, sizeof v); // should compile to a load
  ONTRACE(2, Trace::trace("decode:     Immediate %s %" PRIi64"\n",
          prettytype(v), int64_t(v)));
  return v;
}

template<class T> T decode(PC& pc) {
  auto v = peek<T>(pc);
  pc += sizeof(T);
  return v;
}

inline const StringData* decode_litstr(PC& pc) {
  auto id = decode<Id>(pc);
  return vmfp()->m_func->unit()->lookupLitstrId(id);
}

ALWAYS_INLINE int32_t decode_la(PC& pc) {
  return decode_iva(pc);
}

ALWAYS_INLINE int32_t decode_ia(PC& pc) {
  return decode_iva(pc);
}

ALWAYS_INLINE Offset decode_ba(PC& pc) {
  return decode<Offset>(pc);
}

ALWAYS_INLINE Offset peek_ba(PC& pc) {
  return peek<Offset>(pc);
}

static inline ActRec* arFromInstr(PC pc) {
  const ActRec* fp = vmfp();
  auto const func = fp->m_func;
  if (fp->resumed()) {
    fp = reinterpret_cast<const ActRec*>(Stack::resumableStackBase(fp) +
                                         func->numSlotsInFrame());
  }

  return arAtOffset(fp, -instrFpToArDelta(func, pc));
}

template<Op op>
ALWAYS_INLINE MOpFlags decode_fpass_flags(PC& pc) {
  auto const ar = arFromInstr(pc - encoded_op_size(op));
  auto const paramId = decode_iva(pc);
  assert(paramId < ar->numArgs());
  return ar->m_func->byRef(paramId) ? MOpFlags::DefineReffy : MOpFlags::Warn;
}

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

//=============================================================================
// VarEnv.

const StaticString s_GLOBALS("GLOBALS");

void VarEnv::createGlobal() {
  assert(!g_context->m_globalVarEnv);
  g_context->m_globalVarEnv = req::make_raw<VarEnv>();
}

VarEnv::VarEnv()
  : m_nvTable()
  , m_extraArgs(nullptr)
  , m_depth(0)
  , m_global(true)
{
  TRACE(3, "Creating VarEnv %p [global scope]\n", this);
  auto globals_var = Variant::attach(
    new (MM().objMalloc(sizeof(GlobalsArray))) GlobalsArray(&m_nvTable)
  );
  m_nvTable.set(s_GLOBALS.get(), globals_var.asTypedValue());
}

VarEnv::VarEnv(ActRec* fp, ExtraArgs* eArgs)
  : m_nvTable(fp)
  , m_extraArgs(eArgs)
  , m_depth(1)
  , m_global(false)
{
  assert(fp->func()->attrs() & AttrMayUseVV);
  TRACE(3, "Creating lazily attached VarEnv %p on stack\n", this);
}

VarEnv::VarEnv(const VarEnv* varEnv, ActRec* fp)
  : m_nvTable(varEnv->m_nvTable, fp)
  , m_extraArgs(varEnv->m_extraArgs ? varEnv->m_extraArgs->clone(fp) : nullptr)
  , m_depth(1)
  , m_global(false)
{
  assert(varEnv->m_depth == 1);
  assert(!varEnv->m_global);
  assert(fp->func()->attrs() & AttrMayUseVV);

  TRACE(3, "Cloning VarEnv %p to %p\n", varEnv, this);
}

VarEnv::~VarEnv() {
  TRACE(3, "Destroying VarEnv %p [%s]\n",
           this,
           isGlobalScope() ? "global scope" : "local scope");
  assert(isGlobalScope() == (g_context->m_globalVarEnv == this));

  if (isGlobalScope()) {
    /*
     * When detaching the global scope, we leak any live objects (and
     * let MemoryManager clean them up).  This is because we're
     * not supposed to run destructors for objects that are live at
     * the end of a request.
     */
    m_nvTable.unset(s_GLOBALS.get());
    m_nvTable.leak();
  }
  // at this point, m_nvTable is destructed, and GlobalsArray
  // has a dangling pointer to it.
}

void VarEnv::deallocate(ActRec* fp) {
  fp->m_varEnv->exitFP(fp);
}

VarEnv* VarEnv::createLocal(ActRec* fp) {
  return req::make_raw<VarEnv>(fp, fp->getExtraArgs());
}

VarEnv* VarEnv::clone(ActRec* fp) const {
  return req::make_raw<VarEnv>(this, fp);
}

void VarEnv::suspend(const ActRec* oldFP, ActRec* newFP) {
  m_nvTable.suspend(oldFP, newFP);
}

void VarEnv::enterFP(ActRec* oldFP, ActRec* newFP) {
  TRACE(3, "Attaching VarEnv %p [%s] %d fp @%p\n",
           this,
           isGlobalScope() ? "global scope" : "local scope",
           int(newFP->m_func->numNamedLocals()), newFP);
  assert(newFP);
  if (oldFP == nullptr) {
    assert(isGlobalScope() && m_depth == 0);
  } else {
    assert(m_depth >= 1);
    assert(g_context->getPrevVMState(newFP) == oldFP);
    m_nvTable.detach(oldFP);
  }

  assert(newFP->func()->attrs() & AttrMayUseVV);
  m_nvTable.attach(newFP);
  m_depth++;
}

void VarEnv::exitFP(ActRec* fp) {
  TRACE(3, "Detaching VarEnv %p [%s] @%p\n",
           this,
           isGlobalScope() ? "global scope" : "local scope",
           fp);
  assert(fp);
  assert(m_depth > 0);

  m_depth--;
  m_nvTable.detach(fp);

  if (m_depth == 0) {
    if (m_extraArgs) {
      assert(!isGlobalScope());
      const auto numExtra = fp->numArgs() - fp->m_func->numNonVariadicParams();
      ExtraArgs::deallocate(m_extraArgs, numExtra);
    }

    // don't free global VarEnv
    if (!isGlobalScope()) {
      req::destroy_raw(this);
    }
  } else {
    auto const prevFP = g_context->getPrevVMState(fp);
    assert(prevFP->func()->attrs() & AttrMayUseVV);
    m_nvTable.attach(prevFP);
  }
}

void VarEnv::set(const StringData* name, const TypedValue* tv) {
  m_nvTable.set(name, tv);
}

void VarEnv::bind(const StringData* name, TypedValue* tv) {
  m_nvTable.bind(name, tv);
}

void VarEnv::setWithRef(const StringData* name, TypedValue* tv) {
  if (tv->m_type == KindOfRef) {
    bind(name, tv);
  } else {
    set(name, tv);
  }
}

TypedValue* VarEnv::lookup(const StringData* name) {
  return m_nvTable.lookup(name);
}

TypedValue* VarEnv::lookupAdd(const StringData* name) {
  return m_nvTable.lookupAdd(name);
}

bool VarEnv::unset(const StringData* name) {
  m_nvTable.unset(name);
  return true;
}

const StaticString s_closure_var("0Closure");

Array VarEnv::getDefinedVariables() const {
  Array ret = Array::Create();

  NameValueTable::Iterator iter(&m_nvTable);
  for (; iter.valid(); iter.next()) {
    auto const sd = iter.curKey();
    auto const tv = iter.curVal();
    // Closures have an interal 0Closure variable
    if (s_closure_var.equal(sd)) {
      continue;
    }
    if (tvAsCVarRef(tv).isReferenced()) {
      ret.setWithRef(StrNR(sd).asString(), tvAsCVarRef(tv));
    } else {
      ret.add(StrNR(sd).asString(), tvAsCVarRef(tv));
    }
  }
  {
    // Make result independent of the hashtable implementation.
    ArrayData* sorted = ret->escalateForSort(SORTFUNC_KSORT);
    assert(sorted == ret.get() || sorted->hasExactlyOneRef());
    SCOPE_EXIT {
      if (sorted != ret.get()) {
        ret = Array::attach(sorted);
      }
    };
    sorted->ksort(0, true);
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
  assert(nargs > 0);
  return req::malloc(
    sizeof(TypedValue) * nargs + sizeof(ExtraArgs),
    type_scan::getIndexForMalloc<
      ExtraArgs,
      type_scan::Action::WithSuffix<TypedValue>
    >()
  );
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
  req::free(ea);
}

void ExtraArgs::deallocate(ActRec* ar) {
  const int numExtra = ar->numArgs() - ar->m_func->numNonVariadicParams();
  deallocate(ar->getExtraArgs(), numExtra);
}

ExtraArgs* ExtraArgs::clone(ActRec* ar) const {
  const int numExtra = ar->numArgs() - ar->m_func->numParams();
  auto ret = allocateUninit(numExtra);
  for (int i = 0; i < numExtra; ++i) {
    tvDupFlattenVars(&m_extraArgs[i], &ret->m_extraArgs[i]);
  }
  return ret;
}

TypedValue* ExtraArgs::getExtraArg(unsigned argInd) const {
  return const_cast<TypedValue*>(&m_extraArgs[argInd]);
}

//=============================================================================
// Stack.

// Store actual stack elements array in a thread-local in order to amortize the
// cost of allocation.
namespace {
struct StackElms {
  ~StackElms() { flush(); }
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
  TypedValue* m_elms{nullptr};
};
IMPLEMENT_THREAD_LOCAL(StackElms, t_se);
}

const int Stack::sSurprisePageSize = sysconf(_SC_PAGESIZE);
// We reserve the bottom page of each stack for use as the surprise
// page, so the minimum useful stack size is the next power of two.
const uint32_t Stack::sMinStackElms =
  2 * sSurprisePageSize / sizeof(TypedValue);

void Stack::ValidateStackSize() {
  if (RuntimeOption::EvalVMStackElms < sMinStackElms) {
    throw std::runtime_error(folly::sformat(
      "VM stack size of {:#x} is below the minimum of {:#x}",
      RuntimeOption::EvalVMStackElms,
      sMinStackElms
    ));
  }
  if (!folly::isPowTwo(RuntimeOption::EvalVMStackElms)) {
    throw std::runtime_error(folly::sformat(
      "VM stack size of {:#x} is not a power of 2",
      RuntimeOption::EvalVMStackElms
    ));
  }
}

Stack::Stack()
  : m_elms(nullptr), m_top(nullptr), m_base(nullptr) {
}

Stack::~Stack() {
  requestExit();
}

void Stack::requestInit() {
  m_elms = t_se->elms();
  // Burn one element of the stack, to satisfy the constraint that
  // valid m_top values always have the same high-order (>
  // log(RuntimeOption::EvalVMStackElms)) bits.
  m_top = m_base = m_elms + RuntimeOption::EvalVMStackElms - 1;

  rds::header()->stackLimitAndSurprise.store(
    reinterpret_cast<uintptr_t>(
      reinterpret_cast<char*>(m_elms) + sSurprisePageSize +
        kStackCheckPadding * sizeof(Cell)
    ),
    std::memory_order_release
  );
  assert(!(rds::header()->stackLimitAndSurprise.load() & kSurpriseFlagMask));

  // Because of the surprise page at the bottom of the stack we lose an
  // additional 256 elements which must be taken into account when checking for
  // overflow.
  UNUSED size_t maxelms =
    RuntimeOption::EvalVMStackElms - sSurprisePageSize / sizeof(TypedValue);
  assert(!wouldOverflow(maxelms - 1));
  assert(wouldOverflow(maxelms));
}

void Stack::requestExit() {
  m_elms = nullptr;
}

void flush_evaluation_stack() {
  if (vmStack().isAllocated()) {
    // For RPCRequestHandler threads, the ExecutionContext can stay
    // alive across requests, but its always ok to kill it between
    // requests, so do so now
    RPCRequestHandler::cleanupState();
  }

  MM().flush();

  if (!t_se.isNull()) {
    t_se->flush();
  }
  rds::flush();

  always_assert(MM().empty());
}

static std::string toStringElm(const TypedValue* tv) {
  std::ostringstream os;

  if (tv->m_type < kMinDataType || tv->m_type > kMaxDataType) {
    os << " ??? type " << tv->m_type << "\n";
    return os.str();
  }
  if (isRefcountedType(tv->m_type) &&
      !TV_GENERIC_DISPATCH(*tv, checkCount)) {
    // OK in the invoking frame when running a destructor.
    os << " ??? inner_count " << tvGetCount(tv) << " ";
    return os.str();
  }

  auto print_count = [&] {
    if (TV_GENERIC_DISPATCH(*tv, isStatic)) {
      os << ":c(static)";
    } else if (TV_GENERIC_DISPATCH(*tv, isUncounted)) {
      os << ":c(uncounted)";
    } else {
      os << ":c(" << tvGetCount(tv) << ")";
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
  case KindOfUninit:
  case KindOfNull:
  case KindOfBoolean:
  case KindOfInt64:
  case KindOfDouble:
  case KindOfPersistentString:
  case KindOfString:
  case KindOfPersistentArray:
  case KindOfArray:
  case KindOfObject:
  case KindOfResource:
    os << "C:";
    break;
  }

  do {
    switch (tv->m_type) {
    case KindOfUninit:
      os << "Uninit";
      continue;
    case KindOfNull:
      os << "Null";
      continue;
    case KindOfBoolean:
      os << (tv->m_data.num ? "True" : "False");
      continue;
    case KindOfInt64:
      os << "0x" << std::hex << tv->m_data.num << std::dec;
      continue;
    case KindOfDouble:
      os << tv->m_data.dbl;
      continue;
    case KindOfPersistentString:
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
      continue;
    case KindOfPersistentArray:
    case KindOfArray:
      assert(tv->m_data.parr->checkCount());
      os << tv->m_data.parr;
      print_count();
      os << ":Array";
      continue;
    case KindOfObject:
      assert(tv->m_data.pobj->checkCount());
      os << tv->m_data.pobj;
      print_count();
      os << ":Object("
         << tv->m_data.pobj->getClassName().get()->data()
         << ")";
      continue;
    case KindOfResource:
      assert(tv->m_data.pres->checkCount());
      os << tv->m_data.pres;
      print_count();
      os << ":Resource("
         << tv->m_data.pres->data()->o_getClassName().get()->data()
         << ")";
      continue;
    case KindOfRef:
      break;
    case KindOfClass:
      os << tv->m_data.pcls
         << ":" << tv->m_data.pcls->name()->data();
      continue;
    }
    not_reached();
  } while (0);

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

/*
 * Return true if Offset o is inside the protected region of a fault
 * funclet for iterId, otherwise false. itRef will be set to true if
 * the iterator was initialized with MIterInit*, false if the iterator
 * was initialized with IterInit*.
 */
static bool checkIterScope(const Func* f, Offset o, Id iterId, bool& itRef) {
  assert(o >= f->base() && o < f->past());
  for (auto const& eh : f->ehtab()) {
    if (eh.m_type == EHEnt::Type::Fault &&
        eh.m_base <= o && o < eh.m_past &&
        eh.m_iterId == iterId) {
      itRef = eh.m_itRef;
      return true;
    }
  }
  return false;
}

static void toStringFrame(std::ostream& os, const ActRec* fp,
                          int offset, const TypedValue* ftop,
                          const std::string& prefix, bool isTop = true) {
  assert(fp);

  // Use depth-first recursion to output the most deeply nested stack frame
  // first.
  {
    Offset prevPc = 0;
    TypedValue* prevStackTop = nullptr;
    ActRec* prevFp = g_context->getPrevVMState(fp, &prevPc, &prevStackTop);
    if (prevFp != nullptr) {
      toStringFrame(os, prevFp, prevPc, prevStackTop, prefix, false);
    }
  }

  os << prefix;
  const Func* func = fp->m_func;
  assert(func);
  func->validate();
  std::string funcName(func->fullName()->data());
  os << "{func:" << funcName
     << ",soff:" << fp->m_soff
     << ",this:0x" << std::hex << (fp->hasThis() ? fp->getThis() : nullptr)
     << std::dec << "}";
  TypedValue* tv = (TypedValue*)fp;
  tv--;

  if (func->numLocals() > 0) {
    // Don't print locals for parent frames on a Ret(C|V) since some of them
    // may already be destructed.
    if (isRet(func->unit()->getOp(offset)) && !isTop) {
      os << "<locals destroyed>";
    } else {
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
      if (checkIterScope(func, offset, i, itRef)) {
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
    [&](const ActRec* ar, Offset) {
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

std::string Stack::toString(const ActRec* fp, int offset,
                       const std::string prefix/* = "" */) const {
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

TypedValue* Stack::anyFrameStackBase(const ActRec* fp) {
  return fp->resumed() ? Stack::resumableStackBase(fp)
                       : Stack::frameStackBase(fp);
}

TypedValue* Stack::frameStackBase(const ActRec* fp) {
  assert(!fp->resumed());
  return (TypedValue*)fp - fp->func()->numSlotsInFrame();
}

TypedValue* Stack::resumableStackBase(const ActRec* fp) {
  assert(fp->resumed());
  auto sfp = fp->sfp();
  if (sfp) {
    // The non-reentrant case occurs when a non-async or async generator is
    // resumed via ContEnter or ContRaise opcode. These opcodes leave a single
    // value on the stack that becomes part of the generator's stack. So we
    // find the caller's FP, compensate for its locals and iterators, and then
    // we've found the base of the generator's stack.
    assert(fp->func()->isGenerator());

    // Since resumables are stored on the heap, we need to go back in the
    // callstack a bit to find the base of the stack. Unfortunately, due to
    // generator delegation, this can be pretty far back...
    while (sfp->func()->isGenerator()) {
      sfp = sfp->sfp();
    }

    return (TypedValue*)sfp - sfp->func()->numSlotsInFrame();
  } else {
    // The reentrant case occurs when asio scheduler resumes an async function
    // or async generator. We simply use the top of stack of the previous VM
    // frame (since the ActRec, locals, and iters for this frame do not reside
    // on the VM stack).
    assert(fp->func()->isAsync());
    return g_context.getNoCheck()->m_nestedVMs.back().sp;
  }
}

Array getDefinedVariables(const ActRec* fp) {
  if ((fp->func()->attrs() & AttrMayUseVV) && fp->hasVarEnv()) {
    return fp->m_varEnv->getDefinedVariables();
  }
  auto const func = fp->m_func;
  auto const numLocals = func->numNamedLocals();
  ArrayInit ret(numLocals, ArrayInit::Map{});
  for (Id id = 0; id < numLocals; ++id) {
    TypedValue* ptv = frame_local(fp, id);
    if (ptv->m_type == KindOfUninit) {
      continue;
    }
    Variant name(func->localVarName(id), Variant::PersistentStrInit{});
    ret.add(name, tvAsVariant(ptv));
  }
  return ret.toArray();
}

NEVER_INLINE
static void shuffleExtraStackArgs(ActRec* ar) {
  const Func* func = ar->m_func;
  assert(func);

  // the last (variadic) param is included in numParams (since it has a
  // name), but the arg in that slot should be included as the first
  // element of the variadic array
  const auto numArgs = ar->numArgs();
  const auto numVarArgs = numArgs - func->numNonVariadicParams();
  assert(numVarArgs > 0);

  const auto takesVariadicParam = func->hasVariadicCaptureParam();
  auto& stack = vmStack();
  if (func->attrs() & AttrMayUseVV) {
    auto const tvArgs = reinterpret_cast<TypedValue*>(ar) - numArgs;
    ar->setExtraArgs(ExtraArgs::allocateCopy(tvArgs, numVarArgs));
    if (takesVariadicParam) {
      auto varArgsArray =
        Array::attach(PackedArray::MakePacked(numVarArgs, tvArgs));
      // Incref the args (they're already referenced in extraArgs) but now
      // additionally referenced in varArgsArray ...
      auto tv = tvArgs; uint32_t i = 0;
      for (; i < numVarArgs; ++i, ++tv) { tvRefcountedIncRef(tv); }
      // ... and now remove them from the stack
      stack.ndiscard(numVarArgs);
      auto const ad = varArgsArray.detach();
      assert(ad->hasExactlyOneRef());
      stack.pushArrayNoRc(ad);
      // Before, for each arg: refcount = n + 1 (stack)
      // After, for each arg: refcount = n + 2 (ExtraArgs, varArgsArray)
    } else {
      // Discard the arguments from the stack; they were all moved
      // into the extra args so we don't decref.
      stack.ndiscard(numVarArgs);
    }
    // leave ar->numArgs reflecting the actual number of args passed
  } else {
    assert(takesVariadicParam); // called only if extra args are used
    auto const tvArgs = reinterpret_cast<TypedValue*>(ar) - numArgs;
    auto varArgsArray =
      Array::attach(PackedArray::MakePacked(numVarArgs, tvArgs));
    // Discard the arguments from the stack; they were all moved into the
    // variadic args array so we don't need to decref the values.
    stack.ndiscard(numVarArgs);
    auto const ad = varArgsArray.detach();
    assert(ad->hasExactlyOneRef());
    stack.pushArrayNoRc(ad);
    assert(func->numParams() == (numArgs - numVarArgs + 1));
    ar->setNumArgs(func->numParams());
  }
}

static void shuffleMagicArgs(ActRec* ar) {
  assert(ar->magicDispatch());

  // We need to put this where the first argument is
  auto const invName = ar->clearMagicDispatch();
  int const nargs = ar->numArgs();

  // We need to make an array containing all the arguments passed by
  // the caller and put it where the second argument is.
  auto argArray = Array::attach(
    nargs ? PackedArray::MakePacked(
              nargs, reinterpret_cast<TypedValue*>(ar) - nargs)
          : staticEmptyArray()
  );

  auto& stack = vmStack();
  // Remove the arguments from the stack; they were moved into the
  // array so we don't need to decref.
  stack.ndiscard(nargs);

  // Move invName to where the first argument belongs, no need
  // to incRef/decRef since we are transferring ownership
  stack.pushStringNoRc(invName);

  // Move argArray to where the second argument belongs. We've already
  // incReffed the array above so we don't need to do it here.
  stack.pushArrayNoRc(argArray.detach());

  ar->setNumArgs(2);
  ar->setVarEnv(nullptr);
}

// This helper is meant to be called if an exception or invalidation takes
// place in the process of function entry; the ActRec ar is on the stack
// but is not (yet) the current (executing) frame and is followed by a
// number of params
static NEVER_INLINE void cleanupParamsAndActRec(Stack& stack,
                                                ActRec* ar,
                                                ExtraArgs* extraArgs,
                                                int* numParams) {
  assert(stack.top() + (numParams != nullptr ? (*numParams) :
                        extraArgs != nullptr ? ar->m_func->numParams() :
                        ar->numArgs())
         == (void*)ar);
  if (extraArgs) {
    const int numExtra = ar->numArgs() - ar->m_func->numNonVariadicParams();
    ExtraArgs::deallocate(extraArgs, numExtra);
  }
  while (stack.top() != (void*)ar) {
    stack.popTV();
  }
  stack.popAR();
}

static NEVER_INLINE void shuffleMagicArrayArgs(ActRec* ar, const Cell args,
                                               Stack& stack, int nregular) {
  assert(ar != nullptr && ar->magicDispatch());
  assert(!cellIsNull(&args));
  assert(nregular >= 0);
  assert((stack.top() + nregular) == (void*) ar);
  assert(isContainer(args));
  DEBUG_ONLY const Func* f = ar->m_func;
  assert(f &&
         (f->name()->isame(s___call.get()) ||
          f->name()->isame(s___callStatic.get())));

  // We'll need to make this the first argument
  auto const invName = ar->clearMagicDispatch();

  auto nargs = getContainerSize(args);

  if (UNLIKELY(0 == nargs)) {
    // We need to make an array containing all the arguments passed by
    // the caller and put it where the second argument is.
    auto argArray = Array::attach(
      nregular
      ? PackedArray::MakePacked(
        nregular, reinterpret_cast<TypedValue*>(ar) - nregular)
      : staticEmptyArray()
    );

    // Remove the arguments from the stack; they were moved into the
    // array so we don't need to decref.
    stack.ndiscard(nregular);

    // Move invName to where the first argument belongs, no need
    // to incRef/decRef since we are transferring ownership
    assert(stack.top() == (void*) ar);
    stack.pushStringNoRc(invName);

    // Move argArray to where the second argument belongs. We've already
    // incReffed the array above so we don't need to do it here.
    stack.pushArrayNoRc(argArray.detach());
  } else {
    if (nregular == 0
        && isArrayType(args.m_type)
        && args.m_data.parr->isVectorData()) {
      assert(stack.top() == (void*) ar);
      stack.pushStringNoRc(invName);
      stack.pushArray(args.m_data.parr);
    } else {
      PackedArrayInit ai(nargs + nregular);
      for (int i = 0; i < nregular; ++i) {
        // appendWithRef bumps the refcount and splits if necessary, to
        // compensate for the upcoming pop from the stack
        ai.appendWithRef(tvAsVariant(stack.top()));
        stack.popTV();
      }
      assert(stack.top() == (void*) ar);
      stack.pushStringNoRc(invName);
      for (ArrayIter iter(args); iter; ++iter) {
        ai.appendWithRef(iter.secondRefPlus());
      }
      stack.pushArrayNoRc(ai.create());
    }
  }

  ar->setNumArgs(2);
  ar->setVarEnv(nullptr);
}

// offset is the number of params already on the stack to which the
// contents of args are to be added; for call_user_func_array, this is
// always 0; for unpacked arguments, it may be greater if normally passed
// params precede the unpack.
bool prepareArrayArgs(ActRec* ar, const Cell args, Stack& stack,
                      int nregular, bool doCufRefParamChecks,
                      TypedValue* retval) {
  assert(!cellIsNull(&args));
  assert(nregular >= 0);
  assert((stack.top() + nregular) == (void*) ar);
  const Func* const f = ar->m_func;
  assert(f);

  assert(isContainer(args));
  int const nargs = nregular + getContainerSize(args);
  if (UNLIKELY(ar->magicDispatch())) {
    shuffleMagicArrayArgs(ar, args, stack, nregular);
    return true;
  }

  int const nparams = f->numNonVariadicParams();
  int nextra_regular = std::max(nregular - nparams, 0);
  ArrayIter iter(args);
  if (LIKELY(nextra_regular == 0)) {
    for (int i = nregular; iter && (i < nparams); ++i, ++iter) {
      TypedValue* from = const_cast<TypedValue*>(
        iter.secondRefPlus().asTypedValue());
      TypedValue* to = stack.allocTV();
      if (LIKELY(!f->byRef(i))) {
        cellDup(*tvToCell(from), *to);
      } else if (LIKELY(from->m_type == KindOfRef &&
                        from->m_data.pref->hasMultipleRefs())) {
        refDup(*from, *to);
      } else {
        if (doCufRefParamChecks && f->mustBeRef(i)) {
          try {
            raise_warning("Parameter %d to %s() expected to be a reference, "
                          "value given", i + 1, f->fullName()->data());
          } catch (...) {
            // If the user error handler throws an exception, discard the
            // uninitialized value(s) at the top of the eval stack so that the
            // unwinder doesn't choke
            stack.discard();
            if (retval) { tvWriteNull(retval); }
            throw;
          }
          if (skipCufOnInvalidParams) {
            stack.discard();
            cleanupParamsAndActRec(stack, ar, nullptr, &i);
            if (retval) { tvWriteNull(retval); }
            return false;
          }
        }
        cellDup(*tvToCell(from), *to);
      }
    }

    if (LIKELY(!iter)) {
      // argArray was exhausted, so there are no "extra" arguments but there
      // may be a deficit of non-variadic arguments, and the need to push an
      // empty array for the variadic argument ... that work is left to
      // prepareFuncEntry.  Since the stack state is going to be considered
      // "trimmed" over there, we need to null the extraArgs/varEnv field if
      // the function could read it.
      ar->setNumArgs(nargs);
      ar->trashVarEnv();
      if (!debug || (ar->func()->attrs() & AttrMayUseVV)) {
        ar->setVarEnv(nullptr);
      }
      return true;
    }
  }

  // there are "extra" arguments; passed as standard arguments prior to the
  // ... unpack operator and/or still remaining in argArray
  assert(nargs > nparams);
  assert(nextra_regular > 0 || !!iter);
  if (LIKELY(f->discardExtraArgs())) {
    if (UNLIKELY(nextra_regular > 0)) {
      // if unpacking, any regularly passed arguments on the stack
      // in excess of those expected by the function need to be discarded
      // in addition to the ones held in the arry
      do { stack.popTV(); } while (--nextra_regular);
    }

    // the extra args are not used in the function; no reason to add them
    // to the stack
    ar->setNumArgs(f->numParams());
    return true;
  }

  auto const hasVarParam = f->hasVariadicCaptureParam();
  auto const extra = nargs - nparams;
  if (f->attrs() & AttrMayUseVV) {
    ExtraArgs* extraArgs = ExtraArgs::allocateUninit(extra);
    PackedArrayInit ai(extra);
    if (UNLIKELY(nextra_regular > 0)) {
      // The arguments are pushed in order, so we should refer them by
      // index instead of taking the top, that would lead to reverse order.
      for (int i = nextra_regular - 1; i >= 0; --i) {
        TypedValue* to = extraArgs->getExtraArg(nextra_regular - i - 1);
        const TypedValue* from = stack.indTV(i);
        if (from->m_type == KindOfRef && from->m_data.pref->isReferenced()) {
          refCopy(*from, *to);
        } else {
          cellCopy(*tvToCell(from), *to);
        }
        if (hasVarParam) {
          // appendWithRef bumps the refcount: this accounts for the fact
          // that the extra args values went from being present on the stack
          // to being in (both) ExtraArgs and the variadic args
          ai.appendWithRef(tvAsCVarRef(from));
        }
      }
      stack.ndiscard(nextra_regular);
    }
    for (int i = nextra_regular; i < extra; ++i, ++iter) {
      TypedValue* to = extraArgs->getExtraArg(i);
      const TypedValue* from = iter.secondRefPlus().asTypedValue();
      tvDupWithRef(*from, *to);
      if (hasVarParam) {
        ai.appendWithRef(iter.secondRefPlus());
      }
    }
    assert(!iter); // iter should now be exhausted
    if (hasVarParam) {
      auto const ad = ai.create();
      assert(ad->hasExactlyOneRef());
      stack.pushArrayNoRc(ad);
    }
    ar->setNumArgs(nargs);
    ar->setExtraArgs(extraArgs);
  } else {
    assert(hasVarParam);
    if (nparams == nregular &&
        isArrayType(args.m_type) &&
        args.m_data.parr->isVectorData()) {
      stack.pushArray(args.m_data.parr);
    } else {
      PackedArrayInit ai(extra);
      if (UNLIKELY(nextra_regular > 0)) {
        // The arguments are pushed in order, so we should refer them by
        // index instead of taking the top, that would lead to reverse order.
        for (int i = nextra_regular - 1; i >= 0; --i) {
          // appendWithRef bumps the refcount and splits if necessary,
          // to compensate for the upcoming pop from the stack
          ai.appendWithRef(tvAsVariant(stack.indTV(i)));
        }
        for (int i = 0; i < nextra_regular; ++i) {
          stack.popTV();
        }
      }
      for (int i = nextra_regular; i < extra; ++i, ++iter) {
        // appendWithRef bumps the refcount to compensate for the
        // eventual decref of arrayArgs.
        ai.appendWithRef(iter.secondRefPlus());
      }
      assert(!iter); // iter should now be exhausted
      auto const ad = ai.create();
      assert(ad->hasExactlyOneRef());
      stack.pushArrayNoRc(ad);
    }
    ar->setNumArgs(f->numParams());
  }
  return true;
}

static void prepareFuncEntry(ActRec *ar, PC& pc, StackArgsState stk) {
  assert(!ar->resumed());
  const Func* func = ar->m_func;
  Offset firstDVInitializer = InvalidAbsoluteOffset;
  bool raiseMissingArgumentWarnings = false;
  const int nparams = func->numNonVariadicParams();
  auto& stack = vmStack();

  if (stk == StackArgsState::Trimmed &&
      (ar->func()->attrs() & AttrMayUseVV) &&
      ar->hasExtraArgs()) {
    assert(nparams < ar->numArgs());
  } else if (UNLIKELY(ar->magicDispatch())) {
    // shuffleMagicArgs deals with everything. no need for further
    // argument munging
    shuffleMagicArgs(ar);
  } else {
    int nargs = ar->numArgs();
    if (UNLIKELY(nargs > nparams)) {
      if (LIKELY(stk != StackArgsState::Trimmed && func->discardExtraArgs())) {
        // In the common case, the function won't use the extra arguments,
        // so act as if they were never passed (NOTE: this has the effect
        // of slightly misleading backtraces that don't reflect the
        // discarded args)
        for (int i = nparams; i < nargs; ++i) { stack.popTV(); }
        ar->setNumArgs(nparams);
      } else if (stk == StackArgsState::Trimmed) {
        assert(nargs == func->numParams());
        assert(((TypedValue*)ar - stack.top()) == func->numParams());
      } else {
        shuffleExtraStackArgs(ar);
      }
    } else {
      if (nargs < nparams) {
        // Push uninitialized nulls for missing arguments. Some of them may
        // end up getting default-initialized, but regardless, we need to
        // make space for them on the stack.
        const Func::ParamInfoVec& paramInfo = func->params();
        for (int i = nargs; i < nparams; ++i) {
          stack.pushUninit();
          Offset dvInitializer = paramInfo[i].funcletOff;
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
      }
      if (UNLIKELY(func->hasVariadicCaptureParam())) {
        stack.pushArrayNoRc(staticEmptyArray());
      }
      if (func->attrs() & AttrMayUseVV) {
        ar->setVarEnv(nullptr);
      }
    }
  }

  int nlocals = func->numParams();
  if (UNLIKELY(func->isClosureBody())) {
    int nuse = init_closure(ar, stack.top());
    // init_closure doesn't move stack
    stack.nalloc(nuse);
    nlocals += nuse;
    func = ar->m_func;
  }

  pushLocalsAndIterators(func, nlocals);

  vmfp() = ar;
  if (firstDVInitializer != InvalidAbsoluteOffset) {
    pc = func->unit()->entry() + firstDVInitializer;
  } else {
    pc = func->getEntry();
  }
  // cppext functions/methods have their own logic for raising
  // warnings for missing arguments, so we only need to do this work
  // for non-cppext functions/methods
  if (raiseMissingArgumentWarnings && !func->isCPPBuiltin()) {
    // need to sync vmpc() to pc for backtraces/re-entry
    vmpc() = pc;
    HPHP::jit::raiseMissingArgument(func, ar->numArgs());
  }
}

static void dispatch();

void enterVMAtFunc(ActRec* enterFnAr, StackArgsState stk, VarEnv* varEnv) {
  assert(enterFnAr);
  assert(!enterFnAr->resumed());
  Stats::inc(Stats::VMEnter);

  const bool useJit = RID().getJit();
  const bool useJitPrologue = useJit && vmfp()
    && !enterFnAr->magicDispatch()
    && !varEnv
    && (stk != StackArgsState::Trimmed);
  // The jit prologues only know how to do limited amounts of work; cannot
  // be used for magic call/pseudo-main/extra-args already determined or
  // ... or if the stack args have been explicitly been prepared (e.g. via
  // entry as part of invoke func).

  if (LIKELY(useJitPrologue)) {
    const int np = enterFnAr->m_func->numNonVariadicParams();
    int na = enterFnAr->numArgs();
    if (na > np) na = np + 1;
    jit::TCA start = enterFnAr->m_func->getPrologue(na);
    mcg->enterTCAtPrologue(enterFnAr, start);
    return;
  }

  if (UNLIKELY(varEnv != nullptr)) {
    enterFnAr->setVarEnv(varEnv);
    assert(enterFnAr->func()->isPseudoMain());
    pushLocalsAndIterators(enterFnAr->func());
    enterFnAr->m_varEnv->enterFP(vmfp(), enterFnAr);
    vmfp() = enterFnAr;
    vmpc() = enterFnAr->func()->getEntry();
  } else {
    prepareFuncEntry(enterFnAr, vmpc(), stk);
  }

  if (!EventHook::FunctionCall(enterFnAr, EventHook::NormalFunc)) return;
  checkStack(vmStack(), enterFnAr->m_func, 0);
  assert(vmfp()->func()->contains(vmpc()));

  if (useJit) {
    jit::TCA start = enterFnAr->m_func->getFuncBody();
    mcg->enterTCAfterPrologue(start);
  } else {
    dispatch();
  }
}

void enterVMAtCurPC() {
  assert(vmfp());
  assert(vmpc());
  assert(vmfp()->func()->contains(vmpc()));
  Stats::inc(Stats::VMEnter);
  if (RID().getJit()) {
    mcg->enterTC();
  } else {
    dispatch();
  }
}

/*
 * Helper for function entry, including pseudo-main entry.
 */
void pushLocalsAndIterators(const Func* func, int nparams /*= 0*/) {
  // Push locals.
  for (int i = nparams; i < func->numLocals(); i++) {
    vmStack().pushUninit();
  }
  // Push iterators.
  for (int i = 0; i < func->numIterators(); i++) {
    vmStack().allocI();
  }
}

void unwindPreventReturnToTC(ActRec* ar) {
  auto const savedRip = reinterpret_cast<jit::TCA>(ar->m_savedRip);
  always_assert_flog(mcg->code().isValidCodeAddress(savedRip),
                     "preventReturnToTC({}): {} isn't in TC",
                     ar, savedRip);

  if (isReturnHelper(savedRip)) return;

  auto& ustubs = mcg->ustubs();
  if (ar->resumed()) {
    // async functions use callToExit stub
    assert(ar->func()->isGenerator());
    ar->setJitReturn(ar->func()->isAsync()
      ? ustubs.asyncGenRetHelper : ustubs.genRetHelper);
  } else {
    ar->setJitReturn(ustubs.retHelper);
  }
}

void debuggerPreventReturnToTC(ActRec* ar) {
  auto const savedRip = reinterpret_cast<jit::TCA>(ar->m_savedRip);
  always_assert_flog(mcg->code().isValidCodeAddress(savedRip),
                     "preventReturnToTC({}): {} isn't in TC",
                     ar, savedRip);

  if (isReturnHelper(savedRip) || isDebuggerReturnHelper(savedRip)) return;

  // We're going to smash the return address. Before we do, save the catch
  // block attached to the call in a side table so the return helpers and
  // unwinder can find it when needed.
  jit::stashDebuggerCatch(ar);

  auto& ustubs = mcg->ustubs();
  if (ar->resumed()) {
    // async functions use callToExit stub
    assert(ar->func()->isGenerator());
    ar->setJitReturn(ar->func()->isAsync()
      ? ustubs.debuggerAsyncGenRetHelper : ustubs.debuggerGenRetHelper);
  } else {
    ar->setJitReturn(ustubs.debuggerRetHelper);
  }
}

// Walk the stack and find any return address to jitted code and bash it to the
// appropriate RetFromInterpreted*Frame helper. This ensures that we don't
// return into jitted code and gives the system the proper chance to interpret
// blacklisted tracelets.
void debuggerPreventReturnsToTC() {
  assert(isDebuggerAttached());
  if (!RuntimeOption::EvalJit) return;

  auto& ec = *g_context;
  for (auto ar = vmfp(); ar; ar = ec.getPrevVMState(ar)) {
    debuggerPreventReturnToTC(ar);
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
    assert(fp->func()->attrs() & AttrMayUseVV);
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
  auto const func = fp->m_func;
  Id id = func->lookupVarId(name);
  if (id != kInvalidId) {
    val = frame_local(fp, id);
  } else {
    assert(func->attrs() & AttrMayUseVV);
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
  auto const ctx = arGetContextClass(fp);

  auto const lookup = clsRef->m_data.pcls->getSProp(ctx, name);

  val = lookup.prop;
  visible = lookup.prop != nullptr;
  accessible = lookup.accessible;
}

static inline void lookupClsRef(TypedValue* input,
                                TypedValue* output,
                                bool decRef = false) {
  const Class* class_ = nullptr;
  if (isStringType(input->m_type)) {
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
  if (isRefcountedType(tv->m_type)) {
    return tv->m_type == KindOfRef ? tv->m_data.pref->getRealCount() :
           tvGetCount(tv);
  }
  return -1;
}

static inline TypedValue* ratchetRefs(TypedValue* result, TypedValue& tvRef,
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
    if (isRefcountedType(tvRef2.m_type)) {
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
    return &tvRef2;
  }

  assert(result != &tvRef);
  return result;
}

/*
 * One iop* function exists for every bytecode. They all take a single PC&
 * argument, which should be left pointing to the next bytecode to execute when
 * the instruction is complete. Most return void, though a few return a
 * jit::TCA. The ones that return a TCA return a non-nullptr value to indicate
 * that the caller must resume execution in the TC at the returned
 * address. This is used to maintain certain invariants about how we get into
 * and out of VM frames in jitted code; see comments on jitReturnPre() for more
 * details.
 */

OPTBLD_INLINE void iopLowInvalid(IOP_ARGS) {
  fprintf(stderr, "invalid bytecode executed\n");
  abort();
}

OPTBLD_INLINE void iopHighInvalid(IOP_ARGS) {
  fprintf(stderr, "invalid bytecode executed\n");
  abort();
}

OPTBLD_INLINE void iopNop(IOP_ARGS) {
}

OPTBLD_INLINE void iopPopA(IOP_ARGS) {
  vmStack().popA();
}

OPTBLD_INLINE void iopPopC(IOP_ARGS) {
  vmStack().popC();
}

OPTBLD_INLINE void iopPopV(IOP_ARGS) {
  vmStack().popV();
}

OPTBLD_INLINE void iopPopR(IOP_ARGS) {
  if (vmStack().topTV()->m_type != KindOfRef) {
    vmStack().popC();
  } else {
    vmStack().popV();
  }
}

OPTBLD_INLINE void iopDup(IOP_ARGS) {
  vmStack().dup();
}

OPTBLD_INLINE void iopBox(IOP_ARGS) {
  vmStack().box();
}

OPTBLD_INLINE void iopUnbox(IOP_ARGS) {
  vmStack().unbox();
}

OPTBLD_INLINE void iopBoxR(IOP_ARGS) {
  TypedValue* tv = vmStack().topTV();
  if (tv->m_type != KindOfRef) {
    tvBox(tv);
  }
}

OPTBLD_INLINE void iopBoxRNop(IOP_ARGS) {
  assert(refIsPlausible(*vmStack().topTV()));
}

OPTBLD_INLINE void iopUnboxR(IOP_ARGS) {
  if (vmStack().topTV()->m_type == KindOfRef) {
    vmStack().unbox();
  }
}

OPTBLD_INLINE void iopUnboxRNop(IOP_ARGS) {
  assert(cellIsPlausible(*vmStack().topTV()));
}

OPTBLD_INLINE void iopRGetCNop(IOP_ARGS) {
}

OPTBLD_INLINE void iopNull(IOP_ARGS) {
  vmStack().pushNull();
}

OPTBLD_INLINE void iopNullUninit(IOP_ARGS) {
  vmStack().pushNullUninit();
}

OPTBLD_INLINE void iopTrue(IOP_ARGS) {
  vmStack().pushBool(true);
}

OPTBLD_INLINE void iopFalse(IOP_ARGS) {
  vmStack().pushBool(false);
}

OPTBLD_INLINE void iopFile(IOP_ARGS) {
  auto s = vmfp()->m_func->unit()->filepath();
  vmStack().pushStaticString(s);
}

OPTBLD_INLINE void iopDir(IOP_ARGS) {
  auto s = vmfp()->m_func->unit()->dirpath();
  vmStack().pushStaticString(s);
}

OPTBLD_INLINE void iopNameA(IOP_ARGS) {
  auto const cls  = vmStack().topA();
  auto const name = cls->name();
  vmStack().popA();
  vmStack().pushStaticString(name);
}

OPTBLD_INLINE void iopInt(IOP_ARGS) {
  auto i = decode<int64_t>(pc);
  vmStack().pushInt(i);
}

OPTBLD_INLINE void iopDouble(IOP_ARGS) {
  auto d = decode<double>(pc);
  vmStack().pushDouble(d);
}

OPTBLD_INLINE void iopString(IOP_ARGS) {
  auto s = decode_litstr(pc);
  vmStack().pushStaticString(s);
}

OPTBLD_INLINE void iopArray(IOP_ARGS) {
  auto id = decode<Id>(pc);
  ArrayData* a = vmfp()->m_func->unit()->lookupArrayId(id);
  vmStack().pushStaticArray(a);
}

OPTBLD_INLINE void iopNewArray(IOP_ARGS) {
  auto capacity = decode_iva(pc);
  if (capacity == 0) {
    vmStack().pushArrayNoRc(staticEmptyArray());
  } else {
    vmStack().pushArrayNoRc(PackedArray::MakeReserve(capacity));
  }
}

OPTBLD_INLINE void iopNewMixedArray(IOP_ARGS) {
  auto capacity = decode_iva(pc);
  if (capacity == 0) {
    vmStack().pushArrayNoRc(staticEmptyArray());
  } else {
    vmStack().pushArrayNoRc(MixedArray::MakeReserveMixed(capacity));
  }
}

OPTBLD_INLINE void iopNewDictArray(IOP_ARGS) {
  auto capacity = decode_iva(pc);
  vmStack().pushArrayNoRc(MixedArray::MakeReserveDict(capacity));
}

OPTBLD_INLINE void iopNewLikeArrayL(IOP_ARGS) {
  auto local = decode_la(pc);
  auto capacity = decode_iva(pc);

  ArrayData* arr;
  TypedValue* fr = frame_local(vmfp(), local);

  if (LIKELY(isArrayType(fr->m_type))) {
    arr = MixedArray::MakeReserveLike(fr->m_data.parr, capacity);
  } else {
    capacity = (capacity ? capacity : PackedArray::SmallSize);
    arr = PackedArray::MakeReserve(capacity);
  }
  vmStack().pushArrayNoRc(arr);
}

OPTBLD_INLINE void iopNewPackedArray(IOP_ARGS) {
  auto n = decode_iva(pc);
  // This constructor moves values, no inc/decref is necessary.
  auto* a = PackedArray::MakePacked(n, vmStack().topC());
  vmStack().ndiscard(n);
  vmStack().pushArrayNoRc(a);
}

OPTBLD_INLINE void iopNewStructArray(IOP_ARGS) {
  auto n = decode<uint32_t>(pc); // number of keys and elements
  assert(n > 0 && n <= StructArray::MaxMakeSize);

  req::vector<const StringData*> names;
  names.reserve(n);

  for (size_t i = 0; i < n; ++i) {
    names.push_back(decode_litstr(pc));
  }

  // This constructor moves values, no inc/decref is necessary.
  ArrayData* a;
  Shape* shape;
  if (!RuntimeOption::EvalDisableStructArray &&
      (shape = Shape::create(names.data(), n))) {
    a = MixedArray::MakeStructArray(n, vmStack().topC(), shape);
  } else {
    a = MixedArray::MakeStruct(
      n,
      names.data(),
      vmStack().topC()
    )->asArrayData();
  }
  vmStack().ndiscard(n);
  vmStack().pushArrayNoRc(a);
}

OPTBLD_INLINE void iopAddElemC(IOP_ARGS) {
  Cell* c1 = vmStack().topC();
  Cell* c2 = vmStack().indC(1);
  Cell* c3 = vmStack().indC(2);
  if (!isArrayType(c3->m_type)) {
    raise_error("AddElemC: $3 must be an array");
  }
  if (c2->m_type == KindOfInt64) {
    cellAsVariant(*c3).asArrRef().set(c2->m_data.num, tvAsCVarRef(c1));
  } else {
    cellAsVariant(*c3).asArrRef().set(tvAsCVarRef(c2), tvAsCVarRef(c1));
  }
  vmStack().popC();
  vmStack().popC();
}

OPTBLD_INLINE void iopAddElemV(IOP_ARGS) {
  Ref* r1 = vmStack().topV();
  Cell* c2 = vmStack().indC(1);
  Cell* c3 = vmStack().indC(2);
  if (!isArrayType(c3->m_type)) {
    raise_error("AddElemV: $3 must be an array");
  }
  if (c2->m_type == KindOfInt64) {
    cellAsVariant(*c3).asArrRef().setRef(c2->m_data.num, tvAsVariant(r1));
  } else {
    cellAsVariant(*c3).asArrRef().setRef(tvAsCVarRef(c2), tvAsVariant(r1));
  }
  vmStack().popV();
  vmStack().popC();
}

OPTBLD_INLINE void iopAddNewElemC(IOP_ARGS) {
  Cell* c1 = vmStack().topC();
  Cell* c2 = vmStack().indC(1);
  if (!isArrayType(c2->m_type)) {
    raise_error("AddNewElemC: $2 must be an array");
  }
  cellAsVariant(*c2).asArrRef().append(tvAsCVarRef(c1));
  vmStack().popC();
}

OPTBLD_INLINE void iopAddNewElemV(IOP_ARGS) {
  Ref* r1 = vmStack().topV();
  Cell* c2 = vmStack().indC(1);
  if (!isArrayType(c2->m_type)) {
    raise_error("AddNewElemV: $2 must be an array");
  }
  cellAsVariant(*c2).asArrRef().appendRef(tvAsVariant(r1));
  vmStack().popV();
}

OPTBLD_INLINE void iopNewCol(IOP_ARGS) {
  auto cType = static_cast<CollectionType>(decode_iva(pc));
  // Incref the collection object during construction.
  auto obj = collections::alloc(cType);
  vmStack().pushObjectNoRc(obj);
}

OPTBLD_INLINE void iopColFromArray(IOP_ARGS) {
  auto const cType = static_cast<CollectionType>(decode_iva(pc));
  auto const c1 = vmStack().topC();
  // This constructor reassociates the ArrayData with the collection, so no
  // inc/decref is needed for the array. The collection object itself is
  // increfed.
  auto obj = collections::alloc(cType, c1->m_data.parr);
  vmStack().discard();
  vmStack().pushObjectNoRc(obj);
}

OPTBLD_INLINE void iopColAddNewElemC(IOP_ARGS) {
  Cell* c1 = vmStack().topC();
  Cell* c2 = vmStack().indC(1);
  assert(c2->m_type == KindOfObject && c2->m_data.pobj->isCollection());
  collections::initElem(c2->m_data.pobj, c1);
  vmStack().popC();
}

OPTBLD_INLINE void iopMapAddElemC(IOP_ARGS) {
  Cell* c1 = vmStack().topC();
  Cell* c2 = vmStack().indC(1);
  Cell* c3 = vmStack().indC(2);
  assert(c3->m_type == KindOfObject && c3->m_data.pobj->isCollection());
  collections::initMapElem(c3->m_data.pobj, c2, c1);
  vmStack().popC();
  vmStack().popC();
}

OPTBLD_INLINE void iopCns(IOP_ARGS) {
  auto s = decode_litstr(pc);
  auto const cns = Unit::loadCns(s);
  if (cns == nullptr) {
    raise_notice(Strings::UNDEFINED_CONSTANT, s->data(), s->data());
    vmStack().pushStaticString(s);
    return;
  }
  auto const c1 = vmStack().allocC();
  cellDup(*cns, *c1);
}

OPTBLD_INLINE void iopCnsE(IOP_ARGS) {
  auto s = decode_litstr(pc);
  auto const cns = Unit::loadCns(s);
  if (cns == nullptr) {
    raise_error("Undefined constant '%s'", s->data());
  }
  auto const c1 = vmStack().allocC();
  cellDup(*cns, *c1);
}

OPTBLD_INLINE void iopCnsU(IOP_ARGS) {
  auto name = decode_litstr(pc);
  auto fallback = decode_litstr(pc);
  auto cns = Unit::loadCns(name);
  if (cns == nullptr) {
    cns = Unit::loadCns(fallback);
    if (cns == nullptr) {
      raise_notice(
        Strings::UNDEFINED_CONSTANT,
        fallback->data(),
        fallback->data()
      );
      vmStack().pushStaticString(fallback);
      return;
    }
  }
  auto const c1 = vmStack().allocC();
  cellDup(*cns, *c1);
}

OPTBLD_INLINE void iopDefCns(IOP_ARGS) {
  auto s = decode_litstr(pc);
  bool result = Unit::defCns(s, vmStack().topTV());
  vmStack().replaceTV<KindOfBoolean>(result);
}

OPTBLD_INLINE void iopClsCns(IOP_ARGS) {
  auto clsCnsName = decode_litstr(pc);

  auto const cls    = vmStack().topA();
  auto const clsCns = cls->clsCnsGet(clsCnsName);

  if (clsCns.m_type == KindOfUninit) {
    raise_error("Couldn't find constant %s::%s",
                cls->name()->data(), clsCnsName->data());
  }

  cellDup(clsCns, *vmStack().topTV());
}

OPTBLD_INLINE void iopClsCnsD(IOP_ARGS) {
  auto clsCnsName = decode_litstr(pc);
  auto classId = decode<Id>(pc);
  const NamedEntityPair& classNamedEntity =
    vmfp()->m_func->unit()->lookupNamedEntityPairId(classId);

  auto const clsCns = g_context->lookupClsCns(classNamedEntity.second,
                                       classNamedEntity.first, clsCnsName);
  auto const c1 = vmStack().allocC();
  cellDup(clsCns, *c1);
}

OPTBLD_FLT_INLINE void iopConcat(IOP_ARGS) {
  auto const c1 = vmStack().topC();
  auto const c2 = vmStack().indC(1);
  auto const s2 = cellAsVariant(*c2).toString();
  auto const s1 = cellAsCVarRef(*c1).toString();
  cellAsVariant(*c2) = concat(s2, s1);
  assert(c2->m_data.pstr->checkCount());
  vmStack().popC();
}

OPTBLD_INLINE void iopConcatN(IOP_ARGS) {
  auto n = decode_iva(pc);

  auto const c1 = vmStack().topC();
  auto const c2 = vmStack().indC(1);

  if (n == 2) {
    auto const s2 = cellAsVariant(*c2).toString();
    auto const s1 = cellAsCVarRef(*c1).toString();
    cellAsVariant(*c2) = concat(s2, s1);
    assert(c2->m_data.pstr->checkCount());
  } else if (n == 3) {
    auto const c3 = vmStack().indC(2);
    auto const s3 = cellAsVariant(*c3).toString();
    auto const s2 = cellAsCVarRef(*c2).toString();
    auto const s1 = cellAsCVarRef(*c1).toString();
    cellAsVariant(*c3) = concat3(s3, s2, s1);
    assert(c3->m_data.pstr->checkCount());
  } else {
    assert(n == 4);
    auto const c3 = vmStack().indC(2);
    auto const c4 = vmStack().indC(3);
    auto const s4 = cellAsVariant(*c4).toString();
    auto const s3 = cellAsCVarRef(*c3).toString();
    auto const s2 = cellAsCVarRef(*c2).toString();
    auto const s1 = cellAsCVarRef(*c1).toString();
    cellAsVariant(*c4) = concat4(s4, s3, s2, s1);
    assert(c4->m_data.pstr->checkCount());
  }

  for (int i = 1; i < n; ++i) {
    vmStack().popC();
  }
}

OPTBLD_INLINE void iopNot(IOP_ARGS) {
  Cell* c1 = vmStack().topC();
  cellAsVariant(*c1) = !cellAsVariant(*c1).toBoolean();
}

template<class Fn>
OPTBLD_INLINE void implCellBinOp(Fn fn) {
  auto const c1 = vmStack().topC();
  auto const c2 = vmStack().indC(1);
  auto const result = fn(*c2, *c1);
  tvRefcountedDecRef(c2);
  *c2 = result;
  vmStack().popC();
}

template<class Fn>
OPTBLD_INLINE void implCellBinOpBool(Fn fn) {
  auto const c1 = vmStack().topC();
  auto const c2 = vmStack().indC(1);
  bool const result = fn(*c2, *c1);
  tvRefcountedDecRef(c2);
  *c2 = make_tv<KindOfBoolean>(result);
  vmStack().popC();
}

template<class Fn>
OPTBLD_INLINE void implCellBinOpInt64(Fn fn) {
  auto const c1 = vmStack().topC();
  auto const c2 = vmStack().indC(1);
  auto const result = fn(*c2, *c1);
  tvRefcountedDecRef(c2);
  *c2 = make_tv<KindOfInt64>(result);
  vmStack().popC();
}

OPTBLD_INLINE void iopAdd(IOP_ARGS) {
  implCellBinOp(cellAdd);
}

OPTBLD_INLINE void iopSub(IOP_ARGS) {
  implCellBinOp(cellSub);
}

OPTBLD_INLINE void iopMul(IOP_ARGS) {
  implCellBinOp(cellMul);
}

OPTBLD_INLINE void iopAddO(IOP_ARGS) {
  implCellBinOp(cellAddO);
}

OPTBLD_INLINE void iopSubO(IOP_ARGS) {
  implCellBinOp(cellSubO);
}

OPTBLD_INLINE void iopMulO(IOP_ARGS) {
  implCellBinOp(cellMulO);
}

OPTBLD_INLINE void iopDiv(IOP_ARGS) {
  implCellBinOp(cellDiv);
}

OPTBLD_INLINE void iopPow(IOP_ARGS) {
  implCellBinOp(cellPow);
}

OPTBLD_INLINE void iopMod(IOP_ARGS) {
  implCellBinOp(cellMod);
}

OPTBLD_INLINE void iopBitAnd(IOP_ARGS) {
  implCellBinOp(cellBitAnd);
}

OPTBLD_INLINE void iopBitOr(IOP_ARGS) {
  implCellBinOp(cellBitOr);
}

OPTBLD_INLINE void iopBitXor(IOP_ARGS) {
  implCellBinOp(cellBitXor);
}

OPTBLD_INLINE void iopXor(IOP_ARGS) {
  implCellBinOpBool([&] (Cell c1, Cell c2) -> bool {
    return cellToBool(c1) ^ cellToBool(c2);
  });
}

OPTBLD_INLINE void iopSame(IOP_ARGS) {
  implCellBinOpBool(cellSame);
}

OPTBLD_INLINE void iopNSame(IOP_ARGS) {
  implCellBinOpBool([&] (Cell c1, Cell c2) {
    return !cellSame(c1, c2);
  });
}

OPTBLD_INLINE void iopEq(IOP_ARGS) {
  implCellBinOpBool([&] (Cell c1, Cell c2) {
    return cellEqual(c1, c2);
  });
}

OPTBLD_INLINE void iopNeq(IOP_ARGS) {
  implCellBinOpBool([&] (Cell c1, Cell c2) {
    return !cellEqual(c1, c2);
  });
}

OPTBLD_INLINE void iopLt(IOP_ARGS) {
  implCellBinOpBool([&] (Cell c1, Cell c2) {
    return cellLess(c1, c2);
  });
}

OPTBLD_INLINE void iopLte(IOP_ARGS) {
  implCellBinOpBool(cellLessOrEqual);
}

OPTBLD_INLINE void iopGt(IOP_ARGS) {
  implCellBinOpBool([&] (Cell c1, Cell c2) {
    return cellGreater(c1, c2);
  });
}

OPTBLD_INLINE void iopGte(IOP_ARGS) {
  implCellBinOpBool(cellGreaterOrEqual);
}

OPTBLD_INLINE void iopCmp(IOP_ARGS) {
  implCellBinOpInt64([&] (Cell c1, Cell c2) {
    return cellCompare(c1, c2);
  });
}

OPTBLD_INLINE void iopShl(IOP_ARGS) {
  implCellBinOp(cellShl);
}

OPTBLD_INLINE void iopShr(IOP_ARGS) {
  implCellBinOp(cellShr);
}

OPTBLD_INLINE void iopBitNot(IOP_ARGS) {
  cellBitNot(*vmStack().topC());
}

OPTBLD_INLINE void iopCastBool(IOP_ARGS) {
  Cell* c1 = vmStack().topC();
  tvCastToBooleanInPlace(c1);
}

OPTBLD_INLINE void iopCastInt(IOP_ARGS) {
  Cell* c1 = vmStack().topC();
  tvCastToInt64InPlace(c1);
}

OPTBLD_INLINE void iopCastDouble(IOP_ARGS) {
  Cell* c1 = vmStack().topC();
  tvCastToDoubleInPlace(c1);
}

OPTBLD_INLINE void iopCastString(IOP_ARGS) {
  Cell* c1 = vmStack().topC();
  tvCastToStringInPlace(c1);
}

OPTBLD_INLINE void iopCastArray(IOP_ARGS) {
  Cell* c1 = vmStack().topC();
  tvCastToArrayInPlace(c1);
}

OPTBLD_INLINE void iopCastObject(IOP_ARGS) {
  Cell* c1 = vmStack().topC();
  tvCastToObjectInPlace(c1);
}

OPTBLD_INLINE bool cellInstanceOf(TypedValue* tv, const NamedEntity* ne) {
  assert(tv->m_type != KindOfRef);
  Class* cls = nullptr;
  switch (tv->m_type) {
    case KindOfUninit:
    case KindOfNull:
    case KindOfBoolean:
    case KindOfResource:
      return false;

    case KindOfInt64:
      cls = Unit::lookupClass(ne);
      return cls && interface_supports_int(cls->name());

    case KindOfDouble:
      cls = Unit::lookupClass(ne);
      return cls && interface_supports_double(cls->name());

    case KindOfPersistentString:
    case KindOfString:
      cls = Unit::lookupClass(ne);
      return cls && interface_supports_string(cls->name());

    case KindOfPersistentArray:
    case KindOfArray:
      cls = Unit::lookupClass(ne);
      return cls && interface_supports_array(cls->name());

    case KindOfObject:
      cls = Unit::lookupClass(ne);
      return cls && tv->m_data.pobj->instanceof(cls);

    case KindOfRef:
    case KindOfClass:
      break;
  }
  not_reached();
}

ALWAYS_INLINE
bool implInstanceOfHelper(const StringData* str1, Cell* c2) {
  const NamedEntity* rhs = NamedEntity::get(str1, false);
  // Because of other codepaths, an un-normalized name might enter the
  // table without a Class* so we need to check if it's there.
  if (LIKELY(rhs && rhs->getCachedClass() != nullptr)) {
    return cellInstanceOf(c2, rhs);
  }
  return false;
}

OPTBLD_INLINE void iopInstanceOf(IOP_ARGS) {
  Cell* c1 = vmStack().topC();   // c2 instanceof c1
  Cell* c2 = vmStack().indC(1);
  bool r = false;
  if (isStringType(c1->m_type)) {
    r = implInstanceOfHelper(c1->m_data.pstr, c2);
  } else if (c1->m_type == KindOfObject) {
    if (c2->m_type == KindOfObject) {
      ObjectData* lhs = c2->m_data.pobj;
      ObjectData* rhs = c1->m_data.pobj;
      r = lhs->instanceof(rhs->getVMClass());
    }
  } else {
    raise_error("Class name must be a valid object or a string");
  }
  vmStack().popC();
  vmStack().replaceC<KindOfBoolean>(r);
}

OPTBLD_INLINE void iopInstanceOfD(IOP_ARGS) {
  auto id = decode<Id>(pc);
  if (isProfileRequest()) {
    InstanceBits::profile(vmfp()->m_func->unit()->lookupLitstrId(id));
  }
  const NamedEntity* ne = vmfp()->m_func->unit()->lookupNamedEntityId(id);
  Cell* c1 = vmStack().topC();
  bool r = cellInstanceOf(c1, ne);
  vmStack().replaceC<KindOfBoolean>(r);
}

OPTBLD_INLINE void iopPrint(IOP_ARGS) {
  Cell* c1 = vmStack().topC();
  g_context->write(cellAsVariant(*c1).toString());
  vmStack().replaceC<KindOfInt64>(1);
}

OPTBLD_INLINE void iopClone(IOP_ARGS) {
  TypedValue* tv = vmStack().topTV();
  if (tv->m_type != KindOfObject) {
    raise_error("clone called on non-object");
  }
  ObjectData* obj = tv->m_data.pobj;
  const Class* class_ UNUSED = obj->getVMClass();
  ObjectData* newobj = obj->clone();
  vmStack().popTV();
  vmStack().pushNull();
  tv->m_type = KindOfObject;
  tv->m_data.pobj = newobj;
}

OPTBLD_INLINE void iopExit(IOP_ARGS) {
  int exitCode = 0;
  Cell* c1 = vmStack().topC();
  if (c1->m_type == KindOfInt64) {
    exitCode = c1->m_data.num;
  } else {
    g_context->write(cellAsVariant(*c1).toString());
  }
  vmStack().popC();
  vmStack().pushNull();
  throw ExitException(exitCode);
}

OPTBLD_INLINE void iopFatal(IOP_ARGS) {
  TypedValue* top = vmStack().topTV();
  std::string msg;
  auto kind_char = decode_oa<FatalOp>(pc);
  if (isStringType(top->m_type)) {
    msg = top->m_data.pstr->data();
  } else {
    msg = "Fatal error message not a string";
  }
  vmStack().popTV();

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

OPTBLD_INLINE void jmpSurpriseCheck(Offset offset) {
  if (offset <= 0 && UNLIKELY(checkSurpriseFlags())) {
    auto const flags = check_request_surprise();

    // Memory Threhsold callback should also be fired here
    if (flags & MemThresholdFlag) {
      EventHook::DoMemoryThresholdCallback();
    }
  }
}

OPTBLD_INLINE void iopJmp(IOP_ARGS) {
  auto const origPC = pc - encoded_op_size(Op::Jmp);
  auto offset = peek_ba(pc);
  jmpSurpriseCheck(offset);
  pc = origPC + offset;
}

OPTBLD_INLINE void iopJmpNS(IOP_ARGS) {
  auto const origPC = pc - encoded_op_size(Op::JmpNS);
  auto offset = peek_ba(pc);
  pc = origPC + offset;
}

template<Op op>
OPTBLD_INLINE void jmpOpImpl(PC& pc) {
  static_assert(op == OpJmpZ || op == OpJmpNZ,
                "jmpOpImpl should only be used by JmpZ and JmpNZ");
  auto const origPC = pc - encoded_op_size(op);
  auto offset = decode_ba(pc);
  jmpSurpriseCheck(offset);

  Cell* c1 = vmStack().topC();
  if (c1->m_type == KindOfInt64 || c1->m_type == KindOfBoolean) {
    int64_t n = c1->m_data.num;
    vmStack().popX();
    if (op == OpJmpZ ? n == 0 : n != 0) pc = origPC + offset;
  } else {
    auto const cond = toBoolean(cellAsCVarRef(*c1));
    vmStack().popC();
    if (op == OpJmpZ ? !cond : cond) pc = origPC + offset;
  }
}

OPTBLD_INLINE void iopJmpZ(IOP_ARGS) {
  jmpOpImpl<OpJmpZ>(pc);
}

OPTBLD_INLINE void iopJmpNZ(IOP_ARGS) {
  jmpOpImpl<OpJmpNZ>(pc);
}

OPTBLD_INLINE void iopIterBreak(IOP_ARGS) {
  PC savedPc = pc - encoded_op_size(Op::IterBreak);
  auto offset = decode_ba(pc);
  auto veclen = decode<int32_t>(pc);
  assert(veclen > 0);
  Id* iterTypeList = (Id*)pc;
  Id* iterIdList   = (Id*)pc + 1;
  pc += 2 * veclen * sizeof(Id);
  for (auto i = 0; i < 2 * veclen; i += 2) {
    Id iterType = iterTypeList[i];
    Id iterId   = iterIdList[i];
    Iter *iter = frame_iter(vmfp(), iterId);
    switch (iterType) {
      case KindOfIter:  iter->free();  break;
      case KindOfMIter: iter->mfree(); break;
      case KindOfCIter: iter->cfree(); break;
    }
  }
  pc = savedPc + offset;
}

enum class SwitchMatch {
  NORMAL,  // value was converted to an int: match normally
  NONZERO, // can't be converted to an int: match first nonzero case
  DEFAULT, // can't be converted to an int: match default case
};

static SwitchMatch doubleCheck(double d, int64_t& out) {
  if (int64_t(d) == d) {
    out = d;
    return SwitchMatch::NORMAL;
  }
  return SwitchMatch::DEFAULT;
}

OPTBLD_INLINE void iopSwitch(IOP_ARGS) {
  auto const origPC = pc - encoded_op_size(Op::Switch);
  auto const kind = decode_oa<SwitchKind>(pc);
  auto base = decode<int64_t>(pc);
  auto veclen = decode<int32_t>(pc);
  assert(veclen > 0);
  Offset* jmptab = (Offset*)pc;
  pc += veclen * sizeof(*jmptab);

  TypedValue* val = vmStack().topTV();
  if (kind == SwitchKind::Unbounded) {
    assert(val->m_type == KindOfInt64);
    // Continuation switch: no bounds checking needed
    int64_t label = val->m_data.num;
    vmStack().popX();
    assert(label >= 0 && label < veclen);
    pc = origPC + jmptab[label];
  } else {
    // Generic integer switch
    int64_t intval;
    SwitchMatch match = SwitchMatch::NORMAL;

    [&] {
      switch (val->m_type) {
        case KindOfUninit:
        case KindOfNull:
          intval = 0;
          return;

        case KindOfBoolean:
          // bool(true) is equal to any non-zero int, bool(false) == 0
          if (val->m_data.num) {
            match = SwitchMatch::NONZERO;
          } else {
            intval = 0;
          }
          return;

        case KindOfInt64:
          intval = val->m_data.num;
          return;

        case KindOfDouble:
          match = doubleCheck(val->m_data.dbl, intval);
          return;

        case KindOfPersistentString:
        case KindOfString: {
          double dval = 0.0;
          DataType t = val->m_data.pstr->isNumericWithVal(intval, dval, 1);
          switch (t) {
            case KindOfNull:
              intval = 0;
              break;
            case KindOfInt64:
              // do nothing
              break;
            case KindOfDouble:
              match = doubleCheck(dval, intval);
              break;
            case KindOfUninit:
            case KindOfBoolean:
            case KindOfPersistentString:
            case KindOfString:
            case KindOfPersistentArray:
            case KindOfArray:
            case KindOfObject:
            case KindOfResource:
            case KindOfRef:
            case KindOfClass:
              not_reached();
          }
          tvRefcountedDecRef(val);
          return;
        }

        case KindOfArray:
          tvDecRef(val);
        case KindOfPersistentArray:
          match = SwitchMatch::DEFAULT;
          return;

        case KindOfObject:
          intval = val->m_data.pobj->toInt64();
          tvDecRef(val);
          return;

        case KindOfResource:
          intval = val->m_data.pres->data()->o_toInt64();
          tvDecRef(val);
          return;

        case KindOfRef:
        case KindOfClass:
          break;
      }
      not_reached();
    }();
    vmStack().discard();

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

OPTBLD_INLINE void iopSSwitch(IOP_ARGS) {
  auto const origPC = pc - encoded_op_size(Op::SSwitch);
  auto veclen = decode<int32_t>(pc);
  assert(veclen > 1);
  unsigned cases = veclen - 1; // the last vector item is the default case
  StrVecItem* jmptab = (StrVecItem*)pc;
  pc += veclen * sizeof(*jmptab);

  Cell* val = tvToCell(vmStack().topTV());
  Unit* u = vmfp()->m_func->unit();
  unsigned i;
  for (i = 0; i < cases; ++i) {
    auto& item = jmptab[i];
    const StringData* str = u->lookupLitstrId(item.str);
    if (cellEqual(*val, str)) {
      pc = origPC + item.dest;
      vmStack().popC();
      return;
    }
  }
  // default case
  pc = origPC + jmptab[veclen-1].dest;
  vmStack().popC();
}

/*
 * jitReturnPre and jitReturnPost are used by RetC/V, CreateCont, NativeImpl,
 * Yield, and YieldK to perform a few tasks related to interpreting out of a
 * frame:
 *
 * - If the current frame was entered in the TC and the jit is now off, we
 *   throw a VMSwitchMode at the beginning of the bytecode to execute the
 *   call's catch block (if present) before performing the return.
 * - If the current frame was entered in the TC and the jit is still on,
 *   we wait until the end of the bytecode and throw a VMResumeTC, to return to
 *   our translated caller rather than interpreting back into it.
 * - If the current frame was entered by the interpreter but was active when
 *   the jit called MCGenerator::handleResume() (meaning it's the saved value
 *   of %rbp in handleResume()'s stack frame), throw a VMResumeTC to reenter
 *   handleResume(). This is necessary to update the value of %rbp in the TC
 *   frame, so the unwinder doesn't read from a dead VM frame if something
 *   throws from the interpreter later on.
 */
namespace {
struct JitReturn {
  uint64_t savedRip;
  ActRec* fp;
  ActRec* sfp;
  uint32_t soff;
};

OPTBLD_INLINE JitReturn jitReturnPre(ActRec* fp) {
  auto savedRip = fp->m_savedRip;
  if (isReturnHelper(reinterpret_cast<void*>(savedRip))) {
    // This frame wasn't called from the TC, so it's ok to return using the
    // interpreter. callToExit is special: it's a return helper but we don't
    // treat it like one in here in order to simplify some things higher up in
    // the pipeline.
    if (reinterpret_cast<TCA>(savedRip) != mcg->ustubs().callToExit) {
      savedRip = 0;
    }
  } else if (!RID().getJit()) {
    // We entered this frame in the TC but the jit is now disabled, probably
    // because a debugger is attached. If we leave this frame in the
    // interpreter, we might be skipping a catch block that our caller expects
    // to be run. Switch to the interpreter before even beginning the
    // instruction.
    throw VMSwitchMode();
  }

  return {savedRip, fp, fp->sfp(), fp->m_soff};
}

OPTBLD_INLINE TCA jitReturnPost(JitReturn retInfo) {
  if (retInfo.savedRip) {
    if (isDebuggerReturnHelper(reinterpret_cast<void*>(retInfo.savedRip))) {
      // Our return address was smashed by the debugger. Do the work of the
      // debuggerRetHelper by setting some unwinder RDS info and resuming at
      // the approprate catch trace.
      jit::g_unwind_rds->debuggerReturnSP = vmsp();
      jit::g_unwind_rds->debuggerReturnOff = retInfo.soff;
      return jit::unstashDebuggerCatch(retInfo.fp);
    }

    // This frame was called by translated code so we can't interpret out of
    // it. Resume in the TC right after our caller. This situation most
    // commonly happens when we interpOne a RetC due to having a VarEnv or some
    // other weird case.
    return TCA(retInfo.savedRip);
  }

  if (!retInfo.sfp) {
    // If we don't have an sfp, we're returning from the first frame in this VM
    // nesting level. The vmJitCalledFrame() check below is only important if
    // we might throw before returning to the TC, which is guaranteed to not
    // happen in this situation.
    assert(vmfp() == nullptr);
    return nullptr;
  }


  // Consider a situation with a PHP function f() that calls another function
  // g(). If the call is interpreted, then we spend some time in the TC inside
  // g(), then eventually end in dispatchBB() (called by
  // MCGenerator::handleResume()) for g()'s RetC, the logic here kicks in.
  //
  // g()'s VM frame was in %rbp when the TC called handleResume(), so it's
  // saved somewhere in handleResume()'s stack frame. If we return out of that
  // frame and keep going in the interpreter, that saved %rbp will be pointing
  // to a garbage VM frame. This is a problem if something needs to throw an
  // exception up through handleResume() and the TC frames above it, since the
  // C++ unwinder will attempt to treat parts of the popped VM frame as
  // pointers and segfault.
  //
  // To avoid running with this dangling saved %rbp a few frames up, we
  // immediately throw an exception that is "caught" by the TC frame that
  // called handleResume(). We resume execution in the TC which reloads the new
  // vmfp() into %rbp, then handleResume() is called again, this time with a
  // live VM frame in %rbp.
  if (vmJitCalledFrame() == retInfo.fp) {
    FTRACE(1, "Returning from frame {}; resuming", vmJitCalledFrame());
    return mcg->ustubs().resumeHelper;
  }

  return nullptr;
}

}

OPTBLD_INLINE TCA ret(PC& pc) {
  auto const jitReturn = jitReturnPre(vmfp());

  // Get the return value.
  TypedValue retval = *vmStack().topTV();
  vmStack().discard();

  // Free $this and local variables. Calls FunctionReturn hook. The return
  // value must be removed from the stack, or the unwinder would try to free it
  // if the hook throws---but the event hook routine decrefs the return value
  // in that case if necessary.
  frame_free_locals_inl(vmfp(), vmfp()->func()->numLocals(), &retval);

  // If in an eagerly executed async function, not called by
  // FCallAwait, wrap the return value into succeeded
  // StaticWaitHandle.
  if (UNLIKELY(vmfp()->mayNeedStaticWaitHandle() &&
               vmfp()->func()->isAsyncFunction())) {
    auto const& retvalCell = *tvAssertCell(&retval);
    // Heads up that we're assuming CreateSucceeded can't throw, or we won't
    // decref the return value.  (It can't right now.)
    auto const waitHandle = c_StaticWaitHandle::CreateSucceeded(retvalCell);
    cellCopy(make_tv<KindOfObject>(waitHandle), retval);
  }

  if (isProfileRequest()) {
    profileIncrementFuncCounter(vmfp()->func());
  }

  // Grab caller info from ActRec.
  ActRec* sfp = vmfp()->sfp();
  Offset soff = vmfp()->m_soff;

  if (LIKELY(!vmfp()->resumed())) {
    // Free ActRec and store the return value.
    vmStack().ndiscard(vmfp()->func()->numSlotsInFrame());
    vmStack().ret();
    *vmStack().topTV() = retval;
    assert(vmStack().topTV() == &vmfp()->m_r);
    // In case we were called by a jitted FCallAwait, let it know
    // that we finished eagerly.
    vmStack().topTV()->m_aux.u_fcallAwaitFlag = 0;
  } else if (vmfp()->func()->isAsyncFunction()) {
    // Mark the async function as succeeded and store the return value.
    assert(!sfp);
    auto wh = frame_afwh(vmfp());
    wh->ret(retval);
    decRefObj(wh);
  } else if (vmfp()->func()->isAsyncGenerator()) {
    // Mark the async generator as finished.
    assert(isNullType(retval.m_type));
    auto const gen = frame_async_generator(vmfp());
    auto const eagerResult = gen->ret();
    if (eagerResult) {
      // Eager execution => return StaticWaitHandle.
      assert(sfp);
      vmStack().pushObjectNoRc(eagerResult);
    } else {
      // Resumed execution => return control to the scheduler.
      assert(!sfp);
    }
  } else if (vmfp()->func()->isNonAsyncGenerator()) {
    // Mark the generator as finished and store the return value.
    frame_generator(vmfp())->ret(retval);

    // Push return value of next()/send()/raise().
    vmStack().pushNull();
  } else {
    not_reached();
  }

  // Return control to the caller.
  vmfp() = sfp;
  pc = LIKELY(vmfp() != nullptr) ? vmfp()->func()->getEntry() + soff : nullptr;

  return jitReturnPost(jitReturn);
}

OPTBLD_INLINE TCA iopRetC(IOP_ARGS) {
  return ret(pc);
}

OPTBLD_INLINE TCA iopRetV(IOP_ARGS) {
  assert(!vmfp()->resumed());
  assert(!vmfp()->func()->isResumable());
  return ret(pc);
}

OPTBLD_INLINE void iopUnwind(IOP_ARGS) {
  assert(!g_context->m_faults.empty());
  assert(g_context->m_faults.back().m_raiseOffset != kInvalidOffset);
  throw VMPrepareUnwind();
}

OPTBLD_INLINE void iopThrow(IOP_ARGS) {
  Cell* c1 = vmStack().topC();
  if (c1->m_type != KindOfObject ||
      !c1->m_data.pobj->instanceof(SystemLib::s_ThrowableClass)) {
    raise_error("Exceptions must implement the Throwable interface.");
  }
  auto obj = Object::attach(c1->m_data.pobj);
  vmStack().discard();
  DEBUGGER_ATTACHED_ONLY(phpDebuggerExceptionThrownHook(obj.get()));
  throw req::root<Object>(std::move(obj));
}

OPTBLD_INLINE void iopAGetC(IOP_ARGS) {
  TypedValue* tv = vmStack().topTV();
  lookupClsRef(tv, tv, true);
}

OPTBLD_INLINE void iopAGetL(IOP_ARGS) {
  auto local = decode_la(pc);
  vmStack().pushUninit();
  TypedValue* fr = tvToCell(frame_local(vmfp(), local));
  lookupClsRef(fr, vmStack().top());
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

OPTBLD_INLINE void cgetl_body(ActRec* fp,
                              TypedValue* fr,
                              TypedValue* to,
                              Id pind,
                              bool warn) {
  if (fr->m_type == KindOfUninit) {
    // `to' is uninitialized here, so we need to tvWriteNull before
    // possibly causing stack unwinding.
    tvWriteNull(to);
    if (warn) raise_undefined_local(fp, pind);
  } else {
    cgetl_inner_body(fr, to);
  }
}

OPTBLD_FLT_INLINE void iopCGetL(IOP_ARGS) {
  auto local = decode_la(pc);
  Cell* to = vmStack().allocC();
  TypedValue* fr = frame_local(vmfp(), local);
  cgetl_body(vmfp(), fr, to, local, true);
}

OPTBLD_INLINE void iopCGetQuietL(IOP_ARGS) {
  auto local = decode_la(pc);
  Cell* to = vmStack().allocC();
  TypedValue* fr = frame_local(vmfp(), local);
  cgetl_body(vmfp(), fr, to, local, false);
}

OPTBLD_INLINE void iopCUGetL(IOP_ARGS) {
  const auto local = decode_la(pc);
  auto to = vmStack().allocTV();
  auto fr = frame_local(vmfp(), local);
  tvDup(*tvToCell(fr), *to);
}

OPTBLD_INLINE void iopCGetL2(IOP_ARGS) {
  auto local = decode_la(pc);
  TypedValue* oldTop = vmStack().topTV();
  TypedValue* newTop = vmStack().allocTV();
  memcpy(newTop, oldTop, sizeof *newTop);
  Cell* to = oldTop;
  TypedValue* fr = frame_local(vmfp(), local);
  cgetl_body(vmfp(), fr, to, local, true);
}

OPTBLD_INLINE void iopCGetL3(IOP_ARGS) {
  auto local = decode_la(pc);
  TypedValue* oldTop = vmStack().topTV();
  TypedValue* oldSubTop = vmStack().indTV(1);
  TypedValue* newTop = vmStack().allocTV();
  memmove(newTop, oldTop, sizeof *oldTop * 2);
  Cell* to = oldSubTop;
  TypedValue* fr = frame_local(vmfp(), local);
  cgetl_body(vmfp(), fr, to, local, true);
}

OPTBLD_INLINE void iopPushL(IOP_ARGS) {
  auto local = decode_la(pc);
  TypedValue* locVal = frame_local(vmfp(), local);
  assert(locVal->m_type != KindOfUninit);
  assert(locVal->m_type != KindOfRef);

  TypedValue* dest = vmStack().allocTV();
  *dest = *locVal;
  locVal->m_type = KindOfUninit;
}

OPTBLD_INLINE void cgetn_body(bool warn) {
  StringData* name;
  TypedValue* to = vmStack().topTV();
  TypedValue* fr = nullptr;
  lookup_var(vmfp(), name, to, fr);
  SCOPE_EXIT { decRefStr(name); };
  if (fr == nullptr || fr->m_type == KindOfUninit) {
    if (warn) raise_notice(Strings::UNDEFINED_VARIABLE, name->data());
    tvRefcountedDecRef(to);
    tvWriteNull(to);
  } else {
    tvRefcountedDecRef(to);
    cgetl_inner_body(fr, to);
  }
}

OPTBLD_INLINE void iopCGetN(IOP_ARGS) { cgetn_body(true); }
OPTBLD_INLINE void iopCGetQuietN(IOP_ARGS) { cgetn_body(false); }

OPTBLD_INLINE void cgetg_body(bool warn) {
  StringData* name;
  TypedValue* to = vmStack().topTV();
  TypedValue* fr = nullptr;
  lookup_gbl(vmfp(), name, to, fr);
  SCOPE_EXIT { decRefStr(name); };
  if (fr == nullptr) {
    if (warn && MoreWarnings) {
      raise_notice(Strings::UNDEFINED_VARIABLE, name->data());
    }
    tvRefcountedDecRef(to);
    tvWriteNull(to);
  } else if (fr->m_type == KindOfUninit) {
    if (warn) raise_notice(Strings::UNDEFINED_VARIABLE, name->data());
    tvRefcountedDecRef(to);
    tvWriteNull(to);
  } else {
    tvRefcountedDecRef(to);
    cgetl_inner_body(fr, to);
  }
}

OPTBLD_INLINE void iopCGetG(IOP_ARGS) { cgetg_body(true); }
OPTBLD_INLINE void iopCGetQuietG(IOP_ARGS) { cgetg_body(false); }

struct SpropState {
  explicit SpropState(Stack&);
  ~SpropState();
  StringData* name;
  TypedValue* clsref;
  TypedValue* output;
  TypedValue* val;
  TypedValue oldNameCell;
  bool visible;
  bool accessible;
};

SpropState::SpropState(Stack& vmstack) {
  clsref = vmstack.topTV();
  auto nameCell = output = vmstack.indTV(1);
  lookup_sprop(vmfp(), clsref, name, nameCell, val, visible, accessible);
  oldNameCell = *nameCell;
}

SpropState::~SpropState() {
  decRefStr(name);
  tvRefcountedDecRef(oldNameCell);
}

template<bool box> void getS() {
  SpropState ss(vmStack());
  if (!(ss.visible && ss.accessible)) {
    raise_error("Invalid static property access: %s::%s",
                ss.clsref->m_data.pcls->name()->data(),
                ss.name->data());
  }
  if (box) {
    if (ss.val->m_type != KindOfRef) {
      tvBox(ss.val);
    }
    refDup(*ss.val, *ss.output);
  } else {
    cellDup(*tvToCell(ss.val), *ss.output);
  }
  vmStack().popA();
}

OPTBLD_INLINE void iopCGetS(IOP_ARGS) {
  getS<false>();
}

static inline MInstrState& initMState() {
  auto& mstate = vmMInstrState();
  tvWriteUninit(&mstate.tvRef);
  tvWriteUninit(&mstate.tvRef2);
  return mstate;
}

using LookupNameFn = void (*)(ActRec*, StringData*&, TypedValue*, TypedValue*&);

static inline void baseNGImpl(TypedValue* key, MOpFlags flags,
                              LookupNameFn lookupd, LookupNameFn lookup) {
  auto& mstate = initMState();
  StringData* name;
  TypedValue* baseVal;

  if (flags & MOpFlags::Define) lookupd(vmfp(), name, key, baseVal);
  else                          lookup(vmfp(), name, key, baseVal);
  SCOPE_EXIT { decRefStr(name); };

  if (baseVal == nullptr) {
    assert(!(flags & MOpFlags::Define));
    if (flags & MOpFlags::Warn) {
      raise_notice(Strings::UNDEFINED_VARIABLE, name->data());
    }
    tvWriteNull(&mstate.tvTempBase);
    mstate.base = &mstate.tvTempBase;
    return;
  }

  mstate.base = baseVal;
}

static inline void baseNImpl(TypedValue* key, MOpFlags flags) {
  baseNGImpl(key, flags, lookupd_var, lookup_var);
}

OPTBLD_INLINE void iopBaseNC(IOP_ARGS) {
  auto const idx = decode_iva(pc);
  auto const flags = decode_oa<MOpFlags>(pc);
  baseNImpl(vmStack().indTV(idx), flags);
}

OPTBLD_INLINE void iopBaseNL(IOP_ARGS) {
  auto const localId = decode_la(pc);
  auto const flags = decode_oa<MOpFlags>(pc);
  baseNImpl(tvToCell(frame_local(vmfp(), localId)), flags);
}

OPTBLD_INLINE void iopFPassBaseNC(IOP_ARGS) {
  auto const flags = decode_fpass_flags<Op::FPassBaseNC>(pc);
  auto const idx = decode_iva(pc);
  baseNImpl(vmStack().indTV(idx), flags);
}

OPTBLD_INLINE void iopFPassBaseNL(IOP_ARGS) {
  auto const flags = decode_fpass_flags<Op::FPassBaseNL>(pc);
  auto const localId = decode_la(pc);
  baseNImpl(tvToCell(frame_local(vmfp(), localId)), flags);
}

static inline void baseGImpl(TypedValue* key, MOpFlags flags) {
  baseNGImpl(key, flags, lookupd_gbl, lookup_gbl);
}

OPTBLD_INLINE void iopBaseGC(IOP_ARGS) {
  auto const idx = decode_iva(pc);
  auto const flags = decode_oa<MOpFlags>(pc);
  baseGImpl(vmStack().indTV(idx), flags);
}

OPTBLD_INLINE void iopBaseGL(IOP_ARGS) {
  auto const localId = decode_la(pc);
  auto const flags = decode_oa<MOpFlags>(pc);
  baseGImpl(tvToCell(frame_local(vmfp(), localId)), flags);
}

OPTBLD_INLINE void iopFPassBaseGC(IOP_ARGS) {
  auto const flags = decode_fpass_flags<Op::FPassBaseGC>(pc);
  auto const idx = decode_iva(pc);
  baseGImpl(vmStack().indTV(idx), flags);
}

OPTBLD_INLINE void iopFPassBaseGL(IOP_ARGS) {
  auto const flags = decode_fpass_flags<Op::FPassBaseGL>(pc);
  auto const localId = decode_la(pc);
  baseGImpl(tvToCell(frame_local(vmfp(), localId)), flags);
}

static inline TypedValue* baseSImpl(int32_t clsIdx, TypedValue* key) {
  auto& stack = vmStack();
  auto const class_ = stack.indTV(clsIdx)->m_data.pcls;
  // Discard the class, preserving the RHS value if it's there.
  if (clsIdx == 1) {
    tvCopy(*stack.indTV(0), *stack.indTV(1));
    stack.discard();
  } else {
    stack.popA();
  }

  auto const name = lookup_name(key);
  SCOPE_EXIT { decRefStr(name); };
  auto const lookup = class_->getSProp(arGetContextClass(vmfp()), name);
  if (!lookup.prop || !lookup.accessible) {
    raise_error("Invalid static property access: %s::%s",
                class_->name()->data(),
                name->data());
  }

  return lookup.prop;
}

OPTBLD_INLINE void iopBaseSC(IOP_ARGS) {
  auto const keyIdx = decode_iva(pc);
  auto const clsIdx = decode_iva(pc);
  auto& mstate = initMState();
  mstate.base = baseSImpl(clsIdx, vmStack().indTV(keyIdx));
}

OPTBLD_INLINE void iopBaseSL(IOP_ARGS) {
  auto const keyLoc = decode_la(pc);
  auto const clsIdx = decode_iva(pc);
  auto& mstate = initMState();
  mstate.base = baseSImpl(clsIdx, tvToCell(frame_local(vmfp(), keyLoc)));
}

OPTBLD_INLINE void baseLImpl(int32_t localId, MOpFlags flags) {
  auto& mstate = initMState();

  auto local = tvToCell(frame_local(vmfp(), localId));
  if (flags & MOpFlags::Warn && local->m_type == KindOfUninit) {
    raise_notice(Strings::UNDEFINED_VARIABLE,
                 vmfp()->m_func->localVarName(localId)->data());
  }

  mstate.base = local;
}

OPTBLD_INLINE void iopBaseL(IOP_ARGS) {
  auto localId = decode_la(pc);
  auto flags = decode_oa<MOpFlags>(pc);
  baseLImpl(localId, flags);
}

OPTBLD_INLINE void iopFPassBaseL(IOP_ARGS) {
  auto flags = decode_fpass_flags<Op::FPassBaseL>(pc);
  auto localId = decode_la(pc);
  baseLImpl(localId, flags);
}

OPTBLD_INLINE void iopBaseC(IOP_ARGS) {
  auto const idx = decode_iva(pc);
  auto& mstate = initMState();
  mstate.base = vmStack().indTV(idx);
}

OPTBLD_INLINE void iopBaseR(IOP_ARGS) {
  iopBaseC(pc);
}

OPTBLD_INLINE void iopBaseH(IOP_ARGS) {
  auto& mstate = initMState();

  mstate.tvTempBase = make_tv<KindOfObject>(vmfp()->getThis());
  mstate.base = &mstate.tvTempBase;
}

static OPTBLD_INLINE void propDispatch(MOpFlags flags, TypedValue key) {
  auto& mstate = vmMInstrState();
  auto ctx = arGetContextClass(vmfp());

  auto result = [&]{
    switch (flags) {
      case MOpFlags::None:
        return Prop<MOpFlags::None>(mstate.tvRef, ctx, mstate.base, key);
      case MOpFlags::Warn:
        return Prop<MOpFlags::Warn>(mstate.tvRef, ctx, mstate.base, key);
      case MOpFlags::Define:
        return Prop<MOpFlags::Define>(mstate.tvRef, ctx, mstate.base, key);
      case MOpFlags::DefineReffy:
        return Prop<MOpFlags::DefineReffy>(mstate.tvRef, ctx, mstate.base, key);
      case MOpFlags::Unset:
        return Prop<MOpFlags::Unset>(mstate.tvRef, ctx, mstate.base, key);
    }
    always_assert(false);
  }();

  mstate.base = ratchetRefs(result, mstate.tvRef, mstate.tvRef2);
}

static OPTBLD_INLINE void propQDispatch(MOpFlags flags, TypedValue key) {
  auto& mstate = vmMInstrState();
  auto ctx = arGetContextClass(vmfp());

  TypedValue* result;
  switch (flags) {
    case MOpFlags::None:
    case MOpFlags::Warn:
      assert(key.m_type == KindOfPersistentString);
      result = nullSafeProp(mstate.tvRef, ctx, mstate.base, key.m_data.pstr);
      break;

    case MOpFlags::DefineReffy:
      raise_error(Strings::NULLSAFE_PROP_WRITE_ERROR);

    case MOpFlags::Define:
    case MOpFlags::Unset:
      always_assert(false);
  }

  mstate.base = ratchetRefs(result, mstate.tvRef, mstate.tvRef2);
}

static OPTBLD_INLINE void elemDispatch(MOpFlags flags, TypedValue key) {
  auto& mstate = vmMInstrState();

  auto result = [&] {
    switch (flags) {
      case MOpFlags::None:
        // We're not actually going to modify it, so this is "safe".
        return const_cast<TypedValue*>(
          Elem<MOpFlags::None>(mstate.tvRef, mstate.base, key)
        );
      case MOpFlags::Warn:
        // We're not actually going to modify it, so this is "safe".
        return const_cast<TypedValue*>(
          Elem<MOpFlags::Warn>(mstate.tvRef, mstate.base, key)
        );
      case MOpFlags::Define:
        return ElemD<MOpFlags::Define>(mstate.tvRef, mstate.base, key);
      case MOpFlags::DefineReffy:
        return ElemD<MOpFlags::DefineReffy>(mstate.tvRef, mstate.base, key);
      case MOpFlags::Unset:
        return ElemU(mstate.tvRef, mstate.base, key);
    }
    always_assert(false);
  }();

  mstate.base = ratchetRefs(result, mstate.tvRef, mstate.tvRef2);
}

static inline TypedValue key_tv(MemberKey key) {
  switch (key.mcode) {
    case MW:
      return TypedValue{};
    case MEL: case MPL: {
      auto local = tvToCell(frame_local(vmfp(), key.iva));
      if (local->m_type == KindOfUninit) {
        raise_undefined_local(vmfp(), key.iva);
        return make_tv<KindOfNull>();
      }
      return *local;
    }
    case MEC: case MPC:
      return *vmStack().indTV(key.iva);
    case MEI:
      return make_tv<KindOfInt64>(key.int64);
    case MET: case MPT: case MQT:
      return make_tv<KindOfPersistentString>(key.litstr);
  }
  not_reached();
}

static OPTBLD_INLINE void dimDispatch(MOpFlags flags, MemberKey mk) {
  auto const key = key_tv(mk);
  if (mk.mcode == MQT) {
    propQDispatch(flags, key);
  } else if (mcodeIsProp(mk.mcode)) {
    propDispatch(flags, key);
  } else if (mcodeIsElem(mk.mcode)) {
    elemDispatch(flags, key);
  } else {
    if (flags == MOpFlags::Warn) raise_error("Cannot use [] for reading");

    auto& mstate = vmMInstrState();
    auto result = flags == MOpFlags::DefineReffy
      ? NewElem<true>(mstate.tvRef, mstate.base)
      : NewElem<false>(mstate.tvRef, mstate.base);
    mstate.base = ratchetRefs(result, mstate.tvRef, mstate.tvRef2);
  }
}

OPTBLD_INLINE void iopDim(IOP_ARGS) {
  auto const flags = decode_oa<MOpFlags>(pc);
  auto const mk = decode_member_key(pc, liveUnit());
  dimDispatch(flags, mk);
}

OPTBLD_INLINE void iopFPassDim(IOP_ARGS) {
  auto const flags = decode_fpass_flags<Op::FPassDim>(pc);
  auto const mk = decode_member_key(pc, liveUnit());
  dimDispatch(flags, mk);
}

static OPTBLD_INLINE void mFinal(MInstrState& mstate,
                                 int32_t nDiscard,
                                 folly::Optional<TypedValue> result) {
  auto& stack = vmStack();
  for (auto i = 0; i < nDiscard; ++i) stack.popTV();
  if (result) tvCopy(*result, *stack.allocTV());

  tvUnlikelyRefcountedDecRef(mstate.tvRef);
  tvUnlikelyRefcountedDecRef(mstate.tvRef2);
}

static OPTBLD_INLINE void queryMImpl(PC& pc, int32_t nDiscard, QueryMOp op) {
  auto const mk = decode_member_key(pc, liveUnit());
  auto const key = key_tv(mk);

  auto& mstate = vmMInstrState();
  TypedValue result;
  switch (op) {
    case QueryMOp::CGet:
    case QueryMOp::CGetQuiet:
      dimDispatch(getQueryMOpFlags(op), mk);
      tvDup(*tvToCell(mstate.base), result);
      break;

    case QueryMOp::Isset:
    case QueryMOp::Empty:
      result.m_type = KindOfBoolean;
      if (mcodeIsProp(mk.mcode)) {
        auto const ctx = arGetContextClass(vmfp());
        result.m_data.num = op == QueryMOp::Empty
          ? IssetEmptyProp<true>(ctx, mstate.base, key)
          : IssetEmptyProp<false>(ctx, mstate.base, key);
      } else {
        assert(mcodeIsElem(mk.mcode));
        result.m_data.num = op == QueryMOp::Empty
          ? IssetEmptyElem<true>(mstate.base, key)
          : IssetEmptyElem<false>(mstate.base, key);
      }
      break;
  }

  mFinal(mstate, nDiscard, result);
}

OPTBLD_INLINE void iopQueryM(IOP_ARGS) {
  auto const nDiscard = decode_iva(pc);
  auto const op = decode_oa<QueryMOp>(pc);
  queryMImpl(pc, nDiscard, op);
}

static OPTBLD_INLINE void vGetMImpl(PC& pc, int32_t nDiscard) {
  auto const mk = decode_member_key(pc, liveUnit());

  auto& mstate = vmMInstrState();
  TypedValue result;
  dimDispatch(MOpFlags::DefineReffy, mk);
  if (mstate.base->m_type != KindOfRef) tvBox(mstate.base);
  refDup(*mstate.base, result);

  mFinal(mstate, nDiscard, result);
}

OPTBLD_INLINE void iopVGetM(IOP_ARGS) {
  auto const nDiscard = decode_iva(pc);
  vGetMImpl(pc, nDiscard);
}

OPTBLD_INLINE void iopFPassM(IOP_ARGS) {
  auto const flags = decode_fpass_flags<Op::FPassM>(pc);
  auto const nDiscard = decode_iva(pc);

  if (flags == MOpFlags::Warn) return queryMImpl(pc, nDiscard, QueryMOp::CGet);
  assert(flags == MOpFlags::DefineReffy);
  vGetMImpl(pc, nDiscard);
}

OPTBLD_FLT_INLINE void iopSetM(IOP_ARGS) {
  auto const nDiscard = decode_iva(pc);
  auto const mk = decode_member_key(pc, liveUnit());

  auto& mstate = vmMInstrState();
  auto const topC = vmStack().topC();

  if (mk.mcode == MW) {
    SetNewElem<true>(mstate.base, topC);
  } else {
    auto const key = key_tv(mk);
    if (mcodeIsElem(mk.mcode)) {
      auto const result = SetElem<true>(mstate.base, key, topC);
      if (result) {
        tvRefcountedDecRef(topC);
        topC->m_type = KindOfString;
        topC->m_data.pstr = result;
      }
    } else {
      auto const ctx = arGetContextClass(vmfp());
      SetProp<true>(ctx, mstate.base, key, topC);
    }
  }

  auto const result = *topC;
  vmStack().discard();
  mFinal(mstate, nDiscard, result);
}

OPTBLD_INLINE void iopIncDecM(IOP_ARGS) {
  auto const nDiscard = decode_iva(pc);
  auto const op = decode_oa<IncDecOp>(pc);
  auto const mk = decode_member_key(pc, liveUnit());
  auto const key = key_tv(mk);

  auto& mstate = vmMInstrState();
  TypedValue result;
  if (mcodeIsProp(mk.mcode)) {
    IncDecProp(arGetContextClass(vmfp()), op, mstate.base, key, result);
  } else if (mcodeIsElem(mk.mcode)) {
    IncDecElem(op, mstate.base, key, result);
  } else {
    IncDecNewElem(mstate.tvRef, op, mstate.base, result);
  }

  mFinal(mstate, nDiscard, result);
}

OPTBLD_INLINE void iopSetOpM(IOP_ARGS) {
  auto const nDiscard = decode_iva(pc);
  auto const op = decode_oa<SetOpOp>(pc);
  auto const mk = decode_member_key(pc, liveUnit());
  auto const key = key_tv(mk);
  auto const rhs = vmStack().topC();

  auto& mstate = vmMInstrState();
  TypedValue* result;
  if (mcodeIsProp(mk.mcode)) {
    result = SetOpProp(mstate.tvRef, arGetContextClass(vmfp()), op,
                       mstate.base, key, rhs);
  } else if (mcodeIsElem(mk.mcode)) {
    result = SetOpElem(mstate.tvRef, op, mstate.base, key, rhs);
  } else {
    result = SetOpNewElem(mstate.tvRef, op, mstate.base, rhs);
  }

  vmStack().popC();
  result = tvToCell(result);
  tvRefcountedIncRef(result);
  mFinal(mstate, nDiscard, *result);
}

OPTBLD_INLINE void iopBindM(IOP_ARGS) {
  auto const nDiscard = decode_iva(pc);
  auto const mk = decode_member_key(pc, liveUnit());

  auto& mstate = vmMInstrState();
  auto const rhs = *vmStack().topV();

  dimDispatch(MOpFlags::DefineReffy, mk);
  tvBind(&rhs, mstate.base);

  vmStack().discard();
  mFinal(mstate, nDiscard, rhs);
}

OPTBLD_INLINE void iopUnsetM(IOP_ARGS) {
  auto const nDiscard = decode_iva(pc);
  auto const mk = decode_member_key(pc, liveUnit());
  auto const key = key_tv(mk);

  auto& mstate = vmMInstrState();
  if (mcodeIsProp(mk.mcode)) {
    UnsetProp(arGetContextClass(vmfp()), mstate.base, key);
  } else {
    assert(mcodeIsElem(mk.mcode));
    UnsetElem(mstate.base, key);
  }

  mFinal(mstate, nDiscard, folly::none);
}

static OPTBLD_INLINE void setWithRefImpl(TypedValue key, TypedValue* value) {
  auto& mstate = vmMInstrState();
  mstate.base = ElemD<MOpFlags::DefineReffy>(mstate.tvRef, mstate.base, key);
  tvAsVariant(mstate.base).setWithRef(tvAsVariant(value));

  mFinal(mstate, 0, folly::none);
}

OPTBLD_INLINE void iopSetWithRefLML(IOP_ARGS) {
  auto const key = *tvToCell(frame_local(vmfp(), decode_iva(pc)));
  auto const valLoc = decode_la(pc);
  setWithRefImpl(key, frame_local(vmfp(), valLoc));
}

OPTBLD_INLINE void iopSetWithRefRML(IOP_ARGS) {
  auto const key = *tvToCell(frame_local(vmfp(), decode_iva(pc)));
  setWithRefImpl(key, vmStack().topTV());
  vmStack().popTV();
}

static inline void vgetl_body(TypedValue* fr, TypedValue* to) {
  if (fr->m_type != KindOfRef) {
    tvBox(fr);
  }
  refDup(*fr, *to);
}

OPTBLD_INLINE void iopVGetL(IOP_ARGS) {
  auto local = decode_la(pc);
  Ref* to = vmStack().allocV();
  TypedValue* fr = frame_local(vmfp(), local);
  vgetl_body(fr, to);
}

OPTBLD_INLINE void iopVGetN(IOP_ARGS) {
  StringData* name;
  TypedValue* to = vmStack().topTV();
  TypedValue* fr = nullptr;
  lookupd_var(vmfp(), name, to, fr);
  SCOPE_EXIT { decRefStr(name); };
  assert(fr != nullptr);
  tvRefcountedDecRef(to);
  vgetl_body(fr, to);
}

OPTBLD_INLINE void iopVGetG(IOP_ARGS) {
  StringData* name;
  TypedValue* to = vmStack().topTV();
  TypedValue* fr = nullptr;
  lookupd_gbl(vmfp(), name, to, fr);
  SCOPE_EXIT { decRefStr(name); };
  assert(fr != nullptr);
  tvRefcountedDecRef(to);
  vgetl_body(fr, to);
}

OPTBLD_INLINE void iopVGetS(IOP_ARGS) {
  getS<true>();
}

OPTBLD_INLINE void iopIssetN(IOP_ARGS) {
  StringData* name;
  TypedValue* tv1 = vmStack().topTV();
  TypedValue* tv = nullptr;
  bool e;
  lookup_var(vmfp(), name, tv1, tv);
  SCOPE_EXIT { decRefStr(name); };
  if (tv == nullptr) {
    e = false;
  } else {
    e = !cellIsNull(tvToCell(tv));
  }
  vmStack().replaceC<KindOfBoolean>(e);
}

OPTBLD_INLINE void iopIssetG(IOP_ARGS) {
  StringData* name;
  TypedValue* tv1 = vmStack().topTV();
  TypedValue* tv = nullptr;
  bool e;
  lookup_gbl(vmfp(), name, tv1, tv);
  SCOPE_EXIT { decRefStr(name); };
  if (tv == nullptr) {
    e = false;
  } else {
    e = !cellIsNull(tvToCell(tv));
  }
  vmStack().replaceC<KindOfBoolean>(e);
}

OPTBLD_INLINE void iopIssetS(IOP_ARGS) {
  SpropState ss(vmStack());
  bool e;
  if (!(ss.visible && ss.accessible)) {
    e = false;
  } else {
    e = !cellIsNull(tvToCell(ss.val));
  }
  vmStack().popA();
  ss.output->m_data.num = e;
  ss.output->m_type = KindOfBoolean;
}

OPTBLD_FLT_INLINE void iopIssetL(IOP_ARGS) {
  auto local = decode_la(pc);
  TypedValue* tv = frame_local(vmfp(), local);
  bool ret = is_not_null(tvAsCVarRef(tv));
  TypedValue* topTv = vmStack().allocTV();
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

OPTBLD_INLINE void iopIsTypeL(IOP_ARGS) {
  auto local = decode_la(pc);
  auto op = decode_oa<IsTypeOp>(pc);
  TypedValue* tv = frame_local(vmfp(), local);
  if (tv->m_type == KindOfUninit) {
    raise_undefined_local(vmfp(), local);
  }
  TypedValue* topTv = vmStack().allocTV();
  topTv->m_data.num = isTypeHelper(tv, op);
  topTv->m_type = KindOfBoolean;
}

OPTBLD_INLINE void iopIsTypeC(IOP_ARGS) {
  auto op = decode_oa<IsTypeOp>(pc);
  TypedValue* topTv = vmStack().topTV();
  assert(topTv->m_type != KindOfRef);
  bool ret = isTypeHelper(topTv, op);
  tvRefcountedDecRef(topTv);
  topTv->m_data.num = ret;
  topTv->m_type = KindOfBoolean;
}

OPTBLD_FLT_INLINE void iopAssertRATL(IOP_ARGS) {
  auto localId = decode_la(pc);
  if (debug) {
    auto const rat = decodeRAT(vmfp()->m_func->unit(), pc);
    auto const tv = *frame_local(vmfp(), localId);
    auto const func = vmfp()->func();
    auto vm = &*g_context;
    always_assert_flog(
      tvMatchesRepoAuthType(tv, rat),
      "failed assert RATL on local {}: ${} in {}:{}, expected {}, got {}",
      localId,
      localId < func->numNamedLocals() ? func->localNames()[localId]->data()
                                       : "<unnamed>",
      vm->getContainingFileName()->data(),
      vm->getLine(),
      show(rat),
      toStringElm(&tv)
    );
    return;
  }
  pc += encodedRATSize(pc);
}

OPTBLD_INLINE void iopAssertRATStk(IOP_ARGS) {
  auto stkSlot = decode_iva(pc);
  if (debug) {
    auto const rat = decodeRAT(vmfp()->m_func->unit(), pc);
    auto const tv = *vmStack().indTV(stkSlot);
    auto vm = &*g_context;
    always_assert_flog(
      tvMatchesRepoAuthType(tv, rat),
      "failed assert RATStk {} in {}:{}, expected {}, got {}",
      stkSlot,
      vm->getContainingFileName()->data(),
      vm->getLine(),
      show(rat),
      toStringElm(&tv)
    );
    return;
  }
  pc += encodedRATSize(pc);
}

OPTBLD_INLINE void iopBreakTraceHint(IOP_ARGS) {
}

OPTBLD_INLINE void iopEmptyL(IOP_ARGS) {
  auto local = decode_la(pc);
  TypedValue* loc = frame_local(vmfp(), local);
  bool e = !cellToBool(*tvToCell(loc));
  vmStack().pushBool(e);
}

OPTBLD_INLINE void iopEmptyN(IOP_ARGS) {
  StringData* name;
  TypedValue* tv1 = vmStack().topTV();
  TypedValue* tv = nullptr;
  bool e;
  lookup_var(vmfp(), name, tv1, tv);
  SCOPE_EXIT { decRefStr(name); };
  if (tv == nullptr) {
    e = true;
  } else {
    e = !cellToBool(*tvToCell(tv));
  }
  vmStack().replaceC<KindOfBoolean>(e);
}

OPTBLD_INLINE void iopEmptyG(IOP_ARGS) {
  StringData* name;
  TypedValue* tv1 = vmStack().topTV();
  TypedValue* tv = nullptr;
  bool e;
  lookup_gbl(vmfp(), name, tv1, tv);
  SCOPE_EXIT { decRefStr(name); };
  if (tv == nullptr) {
    e = true;
  } else {
    e = !cellToBool(*tvToCell(tv));
  }
  vmStack().replaceC<KindOfBoolean>(e);
}

OPTBLD_INLINE void iopEmptyS(IOP_ARGS) {
  SpropState ss(vmStack());
  bool e;
  if (!(ss.visible && ss.accessible)) {
    e = true;
  } else {
    e = !cellToBool(*tvToCell(ss.val));
  }
  vmStack().popA();
  ss.output->m_data.num = e;
  ss.output->m_type = KindOfBoolean;
}

OPTBLD_INLINE void iopAKExists(IOP_ARGS) {
  TypedValue* arr = vmStack().topTV();
  TypedValue* key = arr + 1;
  bool result = HHVM_FN(array_key_exists)(tvAsCVarRef(key), tvAsCVarRef(arr));
  vmStack().popTV();
  vmStack().replaceTV<KindOfBoolean>(result);
}

OPTBLD_INLINE void iopGetMemoKey(IOP_ARGS) {
  auto obj = vmStack().topTV();
  auto var = HHVM_FN(serialize_memoize_param)(tvAsCVarRef(obj));
  auto res = var.asTypedValue();
  tvRefcountedIncRef(res);
  vmStack().replaceTV(*res);
}

namespace {
const StaticString s_idx("hh\\idx");

TypedValue genericIdx(TypedValue obj, TypedValue key, TypedValue def) {
  static auto func = Unit::loadFunc(s_idx.get());
  assertx(func != nullptr);
  TypedValue args[] = {
    obj,
    key,
    def
  };
  TypedValue ret;
  g_context->invokeFuncFew(&ret, func, nullptr, nullptr, 3, &args[0]);
  return ret;
}
}

OPTBLD_INLINE void iopIdx(IOP_ARGS) {
  TypedValue* def = vmStack().topTV();
  TypedValue* key = vmStack().indTV(1);
  TypedValue* arr = vmStack().indTV(2);

  TypedValue result;
  if (isArrayType(arr->m_type)) {
    new (&result) Variant(HHVM_FN(hphp_array_idx)(tvAsCVarRef(arr),
                                                  tvAsCVarRef(key),
                                                  tvAsCVarRef(def)));
    vmStack().popTV();
  } else if (isNullType(key->m_type)) {
    tvRefcountedDecRef(arr);
    *arr = *def;
    vmStack().ndiscard(2);
    return;
  } else if (!isStringType(arr->m_type) &&
             arr->m_type != KindOfObject) {
    result = *def;
    vmStack().discard();
  } else {
    result = genericIdx(*arr, *key, *def);
    vmStack().popTV();
  }
  vmStack().popTV();
  tvRefcountedDecRef(arr);
  *arr = result;
}

OPTBLD_INLINE void iopArrayIdx(IOP_ARGS) {
  TypedValue* def = vmStack().topTV();
  TypedValue* key = vmStack().indTV(1);
  TypedValue* arr = vmStack().indTV(2);

  Variant result = HHVM_FN(hphp_array_idx)(tvAsCVarRef(arr),
                                  tvAsCVarRef(key),
                                  tvAsCVarRef(def));
  vmStack().popTV();
  vmStack().popTV();
  tvAsVariant(arr) = result;
}

OPTBLD_INLINE void iopSetL(IOP_ARGS) {
  auto local = decode_la(pc);
  assert(local < vmfp()->m_func->numLocals());
  Cell* fr = vmStack().topC();
  TypedValue* to = frame_local(vmfp(), local);
  tvSet(*fr, *to);
}

OPTBLD_INLINE void iopSetN(IOP_ARGS) {
  StringData* name;
  Cell* fr = vmStack().topC();
  TypedValue* tv2 = vmStack().indTV(1);
  TypedValue* to = nullptr;
  lookupd_var(vmfp(), name, tv2, to);
  SCOPE_EXIT { decRefStr(name); };
  assert(to != nullptr);
  tvSet(*fr, *to);
  memcpy((void*)tv2, (void*)fr, sizeof(TypedValue));
  vmStack().discard();
}

OPTBLD_INLINE void iopSetG(IOP_ARGS) {
  StringData* name;
  Cell* fr = vmStack().topC();
  TypedValue* tv2 = vmStack().indTV(1);
  TypedValue* to = nullptr;
  lookupd_gbl(vmfp(), name, tv2, to);
  SCOPE_EXIT { decRefStr(name); };
  assert(to != nullptr);
  tvSet(*fr, *to);
  memcpy((void*)tv2, (void*)fr, sizeof(TypedValue));
  vmStack().discard();
}

OPTBLD_INLINE void iopSetS(IOP_ARGS) {
  TypedValue* tv1 = vmStack().topTV();
  TypedValue* classref = vmStack().indTV(1);
  TypedValue* propn = vmStack().indTV(2);
  TypedValue* output = propn;
  StringData* name;
  TypedValue* val;
  bool visible, accessible;
  lookup_sprop(vmfp(), classref, name, propn, val, visible, accessible);
  SCOPE_EXIT { decRefStr(name); };
  if (!(visible && accessible)) {
    raise_error("Invalid static property access: %s::%s",
                classref->m_data.pcls->name()->data(),
                name->data());
  }
  tvSet(*tv1, *val);
  tvRefcountedDecRef(propn);
  memcpy(output, tv1, sizeof(TypedValue));
  vmStack().ndiscard(2);
}

OPTBLD_INLINE void iopSetOpL(IOP_ARGS) {
  auto local = decode_la(pc);
  auto op = decode_oa<SetOpOp>(pc);
  Cell* fr = vmStack().topC();
  Cell* to = tvToCell(frame_local(vmfp(), local));
  setopBody(to, op, fr);
  tvRefcountedDecRef(fr);
  cellDup(*to, *fr);
}

OPTBLD_INLINE void iopSetOpN(IOP_ARGS) {
  auto op = decode_oa<SetOpOp>(pc);
  StringData* name;
  Cell* fr = vmStack().topC();
  TypedValue* tv2 = vmStack().indTV(1);
  TypedValue* to = nullptr;
  // XXX We're probably not getting warnings totally correct here
  lookupd_var(vmfp(), name, tv2, to);
  SCOPE_EXIT { decRefStr(name); };
  assert(to != nullptr);
  setopBody(tvToCell(to), op, fr);
  tvRefcountedDecRef(fr);
  tvRefcountedDecRef(tv2);
  cellDup(*tvToCell(to), *tv2);
  vmStack().discard();
}

OPTBLD_INLINE void iopSetOpG(IOP_ARGS) {
  auto op = decode_oa<SetOpOp>(pc);
  StringData* name;
  Cell* fr = vmStack().topC();
  TypedValue* tv2 = vmStack().indTV(1);
  TypedValue* to = nullptr;
  // XXX We're probably not getting warnings totally correct here
  lookupd_gbl(vmfp(), name, tv2, to);
  SCOPE_EXIT { decRefStr(name); };
  assert(to != nullptr);
  setopBody(tvToCell(to), op, fr);
  tvRefcountedDecRef(fr);
  tvRefcountedDecRef(tv2);
  cellDup(*tvToCell(to), *tv2);
  vmStack().discard();
}

OPTBLD_INLINE void iopSetOpS(IOP_ARGS) {
  auto op = decode_oa<SetOpOp>(pc);
  Cell* fr = vmStack().topC();
  TypedValue* classref = vmStack().indTV(1);
  TypedValue* propn = vmStack().indTV(2);
  TypedValue* output = propn;
  StringData* name;
  TypedValue* val;
  bool visible, accessible;
  lookup_sprop(vmfp(), classref, name, propn, val, visible, accessible);
  SCOPE_EXIT { decRefStr(name); };
  if (!(visible && accessible)) {
    raise_error("Invalid static property access: %s::%s",
                classref->m_data.pcls->name()->data(),
                name->data());
  }
  setopBody(tvToCell(val), op, fr);
  tvRefcountedDecRef(propn);
  tvRefcountedDecRef(fr);
  cellDup(*tvToCell(val), *output);
  vmStack().ndiscard(2);
}

OPTBLD_INLINE void iopIncDecL(IOP_ARGS) {
  auto local = decode_la(pc);
  auto op = decode_oa<IncDecOp>(pc);
  TypedValue* to = vmStack().allocTV();
  tvWriteUninit(to);
  TypedValue* fr = frame_local(vmfp(), local);
  if (UNLIKELY(fr->m_type == KindOfUninit)) {
    raise_undefined_local(vmfp(), local);
    tvWriteNull(fr);
  } else {
    fr = tvToCell(fr);
  }
  IncDecBody(op, fr, to);
}

OPTBLD_INLINE void iopIncDecN(IOP_ARGS) {
  auto op = decode_oa<IncDecOp>(pc);
  StringData* name;
  TypedValue* nameCell = vmStack().topTV();
  TypedValue* local = nullptr;
  lookupd_var(vmfp(), name, nameCell, local);
  auto oldNameCell = *nameCell;
  SCOPE_EXIT {
    decRefStr(name);
    tvRefcountedDecRef(oldNameCell);
  };
  assert(local != nullptr);
  IncDecBody(op, tvToCell(local), nameCell);
}

OPTBLD_INLINE void iopIncDecG(IOP_ARGS) {
  auto op = decode_oa<IncDecOp>(pc);
  StringData* name;
  TypedValue* nameCell = vmStack().topTV();
  TypedValue* gbl = nullptr;
  lookupd_gbl(vmfp(), name, nameCell, gbl);
  auto oldNameCell = *nameCell;
  SCOPE_EXIT {
    decRefStr(name);
    tvRefcountedDecRef(oldNameCell);
  };
  assert(gbl != nullptr);
  IncDecBody(op, tvToCell(gbl), nameCell);
}

OPTBLD_INLINE void iopIncDecS(IOP_ARGS) {
  SpropState ss(vmStack());
  auto op = decode_oa<IncDecOp>(pc);
  if (!(ss.visible && ss.accessible)) {
    raise_error("Invalid static property access: %s::%s",
                ss.clsref->m_data.pcls->name()->data(),
                ss.name->data());
  }
  IncDecBody(op, tvToCell(ss.val), ss.output);
  vmStack().discard();
}

OPTBLD_INLINE void iopBindL(IOP_ARGS) {
  auto local = decode_la(pc);
  Ref* fr = vmStack().topV();
  TypedValue* to = frame_local(vmfp(), local);
  tvBind(fr, to);
}

OPTBLD_INLINE void iopBindN(IOP_ARGS) {
  StringData* name;
  TypedValue* fr = vmStack().topTV();
  TypedValue* nameTV = vmStack().indTV(1);
  TypedValue* to = nullptr;
  lookupd_var(vmfp(), name, nameTV, to);
  SCOPE_EXIT { decRefStr(name); };
  assert(to != nullptr);
  tvBind(fr, to);
  memcpy((void*)nameTV, (void*)fr, sizeof(TypedValue));
  vmStack().discard();
}

OPTBLD_INLINE void iopBindG(IOP_ARGS) {
  StringData* name;
  TypedValue* fr = vmStack().topTV();
  TypedValue* nameTV = vmStack().indTV(1);
  TypedValue* to = nullptr;
  lookupd_gbl(vmfp(), name, nameTV, to);
  SCOPE_EXIT { decRefStr(name); };
  assert(to != nullptr);
  tvBind(fr, to);
  memcpy((void*)nameTV, (void*)fr, sizeof(TypedValue));
  vmStack().discard();
}

OPTBLD_INLINE void iopBindS(IOP_ARGS) {
  TypedValue* fr = vmStack().topTV();
  TypedValue* classref = vmStack().indTV(1);
  TypedValue* propn = vmStack().indTV(2);
  TypedValue* output = propn;
  StringData* name;
  TypedValue* val;
  bool visible, accessible;
  lookup_sprop(vmfp(), classref, name, propn, val, visible, accessible);
  SCOPE_EXIT { decRefStr(name); };
  if (!(visible && accessible)) {
    raise_error("Invalid static property access: %s::%s",
                classref->m_data.pcls->name()->data(),
                name->data());
  }
  tvBind(fr, val);
  tvRefcountedDecRef(propn);
  memcpy(output, fr, sizeof(TypedValue));
  vmStack().ndiscard(2);
}

OPTBLD_INLINE void iopUnsetL(IOP_ARGS) {
  auto local = decode_la(pc);
  assert(local < vmfp()->m_func->numLocals());
  TypedValue* tv = frame_local(vmfp(), local);
  tvUnset(tv);
}

OPTBLD_INLINE void iopUnsetN(IOP_ARGS) {
  StringData* name;
  TypedValue* tv1 = vmStack().topTV();
  TypedValue* tv = nullptr;
  lookup_var(vmfp(), name, tv1, tv);
  SCOPE_EXIT { decRefStr(name); };
  if (tv != nullptr) {
    tvUnset(tv);
  }
  vmStack().popC();
}

OPTBLD_INLINE void iopUnsetG(IOP_ARGS) {
  TypedValue* tv1 = vmStack().topTV();
  StringData* name = lookup_name(tv1);
  SCOPE_EXIT { decRefStr(name); };
  VarEnv* varEnv = g_context->m_globalVarEnv;
  assert(varEnv != nullptr);
  varEnv->unset(name);
  vmStack().popC();
}

OPTBLD_INLINE ActRec* fPushFuncImpl(const Func* func, int numArgs) {
  DEBUGGER_IF(phpBreakpointEnabled(func->name()->data()));
  ActRec* ar = vmStack().allocA();
  ar->m_func = func;
  ar->initNumArgs(numArgs);
  ar->trashVarEnv();
  setTypesFlag(vmfp(), ar);
  return ar;
}

OPTBLD_INLINE void iopFPushFunc(IOP_ARGS) {
  auto numArgs = decode_iva(pc);
  Cell* c1 = vmStack().topC();

  if (c1->m_type == KindOfObject) {
    // this covers both closures and functors
    static StringData* invokeName = makeStaticString("__invoke");
    ObjectData* origObj = c1->m_data.pobj;
    const Class* cls = origObj->getVMClass();
    auto const func = cls->lookupMethod(invokeName);
    if (func == nullptr) {
      raise_error(Strings::FUNCTION_NAME_MUST_BE_STRING);
    }

    vmStack().discard();
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

  if (isArrayType(c1->m_type) || isStringType(c1->m_type)) {
    // support:
    //   array($instance, 'method')
    //   array('Class', 'method'),
    //   'func_name'
    //   'class::method'
    // which are all valid callables
    auto origCell = *c1;
    ObjectData* thiz = nullptr;
    HPHP::Class* cls = nullptr;
    StringData* invName = nullptr;

    auto const func = vm_decode_function(
      tvAsCVarRef(c1),
      vmfp(),
      /* forwarding */ false,
      thiz,
      cls,
      invName,
      /* warn */ false
    );
    if (func == nullptr) {
      if (isArrayType(origCell.m_type)) {
        raise_error("Invalid callable (array)");
      } else {
        assert(isStringType(origCell.m_type));
        raise_error("Call to undefined function %s()",
                    origCell.m_data.pstr->data());
      }
    }

    vmStack().discard();
    auto const ar = fPushFuncImpl(func, numArgs);
    if (thiz) {
      thiz->incRefCount();
      ar->setThis(thiz);
    } else if (cls) {
      ar->setClass(cls);
    } else {
      ar->setThis(nullptr);
    }

    if (UNLIKELY(invName != nullptr)) {
      ar->setMagicDispatch(invName);
    }
    if (origCell.m_type == KindOfArray) {
      decRefArr(origCell.m_data.parr);
    } else if (origCell.m_type == KindOfString) {
      decRefStr(origCell.m_data.pstr);
    }
    return;
  }

  raise_error(Strings::FUNCTION_NAME_MUST_BE_STRING);
}

OPTBLD_FLT_INLINE void iopFPushFuncD(IOP_ARGS) {
  auto numArgs = decode_iva(pc);
  auto id = decode<Id>(pc);
  const NamedEntityPair nep =
    vmfp()->m_func->unit()->lookupNamedEntityPairId(id);
  Func* func = Unit::loadFunc(nep.second, nep.first);
  if (func == nullptr) {
    raise_error("Call to undefined function %s()",
                vmfp()->m_func->unit()->lookupLitstrId(id)->data());
  }
  ActRec* ar = fPushFuncImpl(func, numArgs);
  ar->setThis(nullptr);
}

OPTBLD_INLINE void iopFPushFuncU(IOP_ARGS) {
  auto numArgs = decode_iva(pc);
  auto nsFunc = decode<Id>(pc);
  auto globalFunc = decode<Id>(pc);
  Unit* unit = vmfp()->m_func->unit();
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

void fPushObjMethodImpl(Class* cls, StringData* name, ObjectData* obj,
                        int numArgs) {
  const Func* f;
  LookupResult res;
  try {
    res = g_context->lookupObjMethod(
      f, cls, name, arGetContextClass(vmfp()), true);
  } catch (...) {
    decRefObj(obj);
    throw;
  }
  assert(f);
  ActRec* ar = vmStack().allocA();
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
    ar->setMagicDispatch(name);
  } else {
    ar->trashVarEnv();
    decRefStr(name);
  }
  setTypesFlag(vmfp(), ar);
}

void fPushNullObjMethod(int numArgs) {
  assert(SystemLib::s_nullFunc);
  ActRec* ar = vmStack().allocA();
  ar->m_func = SystemLib::s_nullFunc;
  ar->setThis(nullptr);
  ar->initNumArgs(numArgs);
  ar->trashVarEnv();
}

static void throw_call_non_object(const char* methodName,
                                  const char* typeName = nullptr) {
  std::string msg;
  folly::format(&msg, "Call to a member function {}() on a non-object ({})",
    methodName, typeName);

  if (RuntimeOption::ThrowExceptionOnBadMethodCall) {
    SystemLib::throwBadMethodCallExceptionObject(String(msg));
  }
  raise_fatal_error(msg.c_str());
}

OPTBLD_INLINE void iopFPushObjMethod(IOP_ARGS) {
  auto numArgs = decode_iva(pc);
  auto op = decode_oa<ObjMethodOp>(pc);
  Cell* c1 = vmStack().topC(); // Method name.
  if (!isStringType(c1->m_type)) {
    raise_error(Strings::METHOD_NAME_MUST_BE_STRING);
  }
  Cell* c2 = vmStack().indC(1); // Object.
  if (c2->m_type != KindOfObject) {
    if (UNLIKELY(op == ObjMethodOp::NullThrows || !isNullType(c2->m_type))) {
      throw_call_non_object(c1->m_data.pstr->data(),
                            getDataTypeString(c2->m_type).get()->data());
    }
    vmStack().popC();
    vmStack().popC();
    fPushNullObjMethod(numArgs);
    return;
  }
  ObjectData* obj = c2->m_data.pobj;
  Class* cls = obj->getVMClass();
  StringData* name = c1->m_data.pstr;
  // We handle decReffing obj and name in fPushObjMethodImpl
  vmStack().ndiscard(2);
  fPushObjMethodImpl(cls, name, obj, numArgs);
}

OPTBLD_INLINE void iopFPushObjMethodD(IOP_ARGS) {
  auto numArgs = decode_iva(pc);
  auto name = decode_litstr(pc);
  auto op = decode_oa<ObjMethodOp>(pc);
  Cell* c1 = vmStack().topC();
  if (c1->m_type != KindOfObject) {
    if (UNLIKELY(op == ObjMethodOp::NullThrows || !isNullType(c1->m_type))) {
      throw_call_non_object(name->data(),
                            getDataTypeString(c1->m_type).get()->data());
    }
    vmStack().popC();
    fPushNullObjMethod(numArgs);
    return;
  }
  ObjectData* obj = c1->m_data.pobj;
  Class* cls = obj->getVMClass();
  // We handle decReffing obj in fPushObjMethodImpl
  vmStack().discard();
  fPushObjMethodImpl(cls, const_cast<StringData*>(name), obj, numArgs);
}

template<bool forwarding>
void pushClsMethodImpl(Class* cls, StringData* name, ObjectData* obj,
                       int numArgs) {
  const Func* f;
  LookupResult res = g_context->lookupClsMethod(f, cls, name, obj,
                                     arGetContextClass(vmfp()), true);
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
  ActRec* ar = vmStack().allocA();
  ar->m_func = f;
  if (obj) {
    ar->setThis(obj);
  } else {
    if (!forwarding) {
      ar->setClass(cls);
    } else {
      /* Propagate the current late bound class if there is one, */
      /* otherwise use the class given by this instruction's input */
      if (vmfp()->hasThis()) {
        cls = vmfp()->getThis()->getVMClass();
      } else if (vmfp()->hasClass()) {
        cls = vmfp()->getClass();
      }
      ar->setClass(cls);
    }
  }
  ar->initNumArgs(numArgs);
  if (res == LookupResult::MagicCallFound ||
      res == LookupResult::MagicCallStaticFound) {
    ar->setMagicDispatch(name);
  } else {
    ar->trashVarEnv();
    decRefStr(const_cast<StringData*>(name));
  }
  setTypesFlag(vmfp(), ar);
}

OPTBLD_INLINE void iopFPushClsMethod(IOP_ARGS) {
  auto numArgs = decode_iva(pc);
  Cell* c1 = vmStack().indC(1); // Method name.
  if (!isStringType(c1->m_type)) {
    raise_error(Strings::FUNCTION_NAME_MUST_BE_STRING);
  }
  TypedValue* tv = vmStack().top();
  assert(tv->m_type == KindOfClass);
  Class* cls = tv->m_data.pcls;
  StringData* name = c1->m_data.pstr;
  // pushClsMethodImpl will take care of decReffing name
  vmStack().ndiscard(2);
  assert(cls && name);
  ObjectData* obj = vmfp()->hasThis() ? vmfp()->getThis() : nullptr;
  pushClsMethodImpl<false>(cls, name, obj, numArgs);
}

OPTBLD_INLINE void iopFPushClsMethodD(IOP_ARGS) {
  auto numArgs = decode_iva(pc);
  auto name = decode_litstr(pc);
  auto classId = decode<Id>(pc);
  const NamedEntityPair &nep =
    vmfp()->m_func->unit()->lookupNamedEntityPairId(classId);
  Class* cls = Unit::loadClass(nep.second, nep.first);
  if (cls == nullptr) {
    raise_error(Strings::UNKNOWN_CLASS, nep.first->data());
  }
  ObjectData* obj = vmfp()->hasThis() ? vmfp()->getThis() : nullptr;
  pushClsMethodImpl<false>(cls, const_cast<StringData*>(name), obj, numArgs);
}

OPTBLD_INLINE void iopFPushClsMethodF(IOP_ARGS) {
  auto numArgs = decode_iva(pc);
  Cell* c1 = vmStack().indC(1); // Method name.
  if (!isStringType(c1->m_type)) {
    raise_error(Strings::FUNCTION_NAME_MUST_BE_STRING);
  }
  TypedValue* tv = vmStack().top();
  assert(tv->m_type == KindOfClass);
  Class* cls = tv->m_data.pcls;
  assert(cls);
  StringData* name = c1->m_data.pstr;
  // pushClsMethodImpl will take care of decReffing name
  vmStack().ndiscard(2);
  ObjectData* obj = vmfp()->hasThis() ? vmfp()->getThis() : nullptr;
  pushClsMethodImpl<true>(cls, name, obj, numArgs);
}

OPTBLD_INLINE void iopFPushCtor(IOP_ARGS) {
  auto numArgs = decode_iva(pc);
  TypedValue* tv = vmStack().topTV();
  assert(tv->m_type == KindOfClass);
  Class* cls = tv->m_data.pcls;
  assert(cls != nullptr);
  // Lookup the ctor
  const Func* f;
  LookupResult res UNUSED = g_context->lookupCtorMethod(f, cls, true);
  assert(res == LookupResult::MethodFoundWithThis);
  // Replace input with uninitialized instance.
  ObjectData* this_ = newInstance(cls);
  TRACE(2, "FPushCtor: just new'ed an instance of class %s: %p\n",
        cls->name()->data(), this_);
  this_->incRefCount();
  tv->m_type = KindOfObject;
  tv->m_data.pobj = this_;
  // Push new activation record.
  ActRec* ar = vmStack().allocA();
  ar->m_func = f;
  ar->setThis(this_);
  ar->initNumArgs(numArgs);
  ar->trashVarEnv();
  setTypesFlag(vmfp(), ar);
}

OPTBLD_INLINE void iopFPushCtorD(IOP_ARGS) {
  auto numArgs = decode_iva(pc);
  auto id = decode<Id>(pc);
  const NamedEntityPair &nep =
    vmfp()->m_func->unit()->lookupNamedEntityPairId(id);
  Class* cls = Unit::loadClass(nep.second, nep.first);
  if (cls == nullptr) {
    raise_error(Strings::UNKNOWN_CLASS,
                vmfp()->m_func->unit()->lookupLitstrId(id)->data());
  }
  // Lookup the ctor
  const Func* f;
  LookupResult res UNUSED = g_context->lookupCtorMethod(f, cls, true);
  assert(res == LookupResult::MethodFoundWithThis);
  // Push uninitialized instance.
  ObjectData* this_ = newInstance(cls);
  TRACE(2, "FPushCtorD: new'ed an instance of class %s: %p\n",
        cls->name()->data(), this_);
  vmStack().pushObject(this_);
  // Push new activation record.
  ActRec* ar = vmStack().allocA();
  ar->m_func = f;
  ar->setThis(this_);
  ar->initNumArgs(numArgs);
  ar->trashVarEnv();
  setTypesFlag(vmfp(), ar);
}

OPTBLD_INLINE void iopDecodeCufIter(IOP_ARGS) {
  PC origPc = pc - encoded_op_size(Op::DecodeCufIter);
  auto itId = decode_ia(pc);
  auto offset = decode_ba(pc);

  Iter* it = frame_iter(vmfp(), itId);
  CufIter &cit = it->cuf();

  ObjectData* obj = nullptr;
  HPHP::Class* cls = nullptr;
  StringData* invName = nullptr;
  TypedValue *func = vmStack().topTV();

  ActRec* ar = vmfp();
  if (vmfp()->m_func->isBuiltin()) {
    ar = g_context->getOuterVMFrame(ar);
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
  vmStack().popC();
}

OPTBLD_INLINE void iopFPushCufIter(IOP_ARGS) {
  auto numArgs = decode_iva(pc);
  auto itId = decode_ia(pc);

  Iter* it = frame_iter(vmfp(), itId);

  auto f = it->cuf().func();
  auto o = it->cuf().ctx();
  auto n = it->cuf().name();

  ActRec* ar = vmStack().allocA();
  ar->m_func = f;
  ar->m_this = (ObjectData*)o;
  if (o && !(uintptr_t(o) & 1)) ar->m_this->incRefCount();
  ar->initNumArgs(numArgs);
  if (n) {
    ar->setMagicDispatch(n);
    n->incRefCount();
  } else {
    ar->trashVarEnv();
  }
  setTypesFlag(vmfp(), ar);
}

OPTBLD_INLINE void doFPushCuf(int32_t numArgs, bool forward, bool safe) {
  TypedValue func = vmStack().topTV()[safe];

  ObjectData* obj = nullptr;
  HPHP::Class* cls = nullptr;
  StringData* invName = nullptr;

  const Func* f = vm_decode_function(tvAsVariant(&func), vmfp(),
                                     forward,
                                     obj, cls, invName,
                                     !safe);

  if (safe) vmStack().topTV()[1] = vmStack().topTV()[0];
  vmStack().ndiscard(1);
  if (f == nullptr) {
    f = SystemLib::s_nullFunc;
    if (safe) {
      vmStack().pushBool(false);
    }
  } else if (safe) {
    vmStack().pushBool(true);
  }

  ActRec* ar = vmStack().allocA();
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
    ar->setMagicDispatch(invName);
  } else {
    ar->trashVarEnv();
  }
  setTypesFlag(vmfp(), ar);
  tvRefcountedDecRef(&func);
}

OPTBLD_INLINE void iopFPushCuf(IOP_ARGS) {
  doFPushCuf(decode_iva(pc), false, false);
}

OPTBLD_INLINE void iopFPushCufF(IOP_ARGS) {
  doFPushCuf(decode_iva(pc), true, false);
}

OPTBLD_INLINE void iopFPushCufSafe(IOP_ARGS) {
  doFPushCuf(decode_iva(pc), false, true);
}

OPTBLD_INLINE void iopFPassC(IOP_ARGS) {
  DEBUG_ONLY auto const ar = arFromInstr(pc - encoded_op_size(Op::FPassC));
  UNUSED auto paramId = decode_iva(pc);
  assert(paramId < ar->numArgs());
}

OPTBLD_INLINE void iopFPassCW(IOP_ARGS) {
  auto const ar = arFromInstr(pc - encoded_op_size(Op::FPassCW));
  auto paramId = decode_iva(pc);
  assert(paramId < ar->numArgs());
  auto const func = ar->m_func;
  if (func->mustBeRef(paramId)) {
    raise_strict_warning("Only variables should be passed by reference");
  }
}

OPTBLD_INLINE void iopFPassCE(IOP_ARGS) {
  auto const ar = arFromInstr(pc - encoded_op_size(Op::FPassCE));
  auto paramId = decode_iva(pc);
  assert(paramId < ar->numArgs());
  auto const func = ar->m_func;
  if (func->mustBeRef(paramId)) {
    raise_error("Cannot pass parameter %d by reference", paramId+1);
  }
}

OPTBLD_INLINE void iopFPassV(IOP_ARGS) {
  auto const ar = arFromInstr(pc - encoded_op_size(Op::FPassV));
  auto paramId = decode_iva(pc);
  assert(paramId < ar->numArgs());
  const Func* func = ar->m_func;
  if (!func->byRef(paramId)) {
    vmStack().unbox();
  }
}

OPTBLD_INLINE void iopFPassVNop(IOP_ARGS) {
  DEBUG_ONLY auto const ar = arFromInstr(pc - encoded_op_size(Op::FPassVNop));
  UNUSED auto paramId = decode_iva(pc);
  assert(paramId < ar->numArgs());
  assert(ar->m_func->byRef(paramId));
}

OPTBLD_INLINE void iopFPassR(IOP_ARGS) {
  auto const ar = arFromInstr(pc - encoded_op_size(Op::FPassR));
  auto paramId = decode_iva(pc);
  assert(paramId < ar->numArgs());
  const Func* func = ar->m_func;
  if (func->byRef(paramId)) {
    TypedValue* tv = vmStack().topTV();
    if (tv->m_type != KindOfRef) {
      tvBox(tv);
    }
  } else {
    if (vmStack().topTV()->m_type == KindOfRef) {
      vmStack().unbox();
    }
  }
}

OPTBLD_INLINE void iopFPassL(IOP_ARGS) {
  auto const ar = arFromInstr(pc - encoded_op_size(Op::FPassL));
  auto paramId = decode_iva(pc);
  auto local = decode_la(pc);
  assert(paramId < ar->numArgs());
  TypedValue* fr = frame_local(vmfp(), local);
  TypedValue* to = vmStack().allocTV();
  if (!ar->m_func->byRef(paramId)) {
    cgetl_body(vmfp(), fr, to, local, true);
  } else {
    vgetl_body(fr, to);
  }
}

OPTBLD_INLINE void iopFPassN(IOP_ARGS) {
  auto const ar = arFromInstr(pc - encoded_op_size(Op::FPassN));
  PC origPc = pc;
  auto paramId = decode_iva(pc);
  assert(paramId < ar->numArgs());
  if (!ar->m_func->byRef(paramId)) {
    iopCGetN(origPc);
  } else {
    iopVGetN(origPc);
  }
}

OPTBLD_INLINE void iopFPassG(IOP_ARGS) {
  auto const ar = arFromInstr(pc - encoded_op_size(Op::FPassG));
  PC origPc = pc;
  auto paramId = decode_iva(pc);
  assert(paramId < ar->numArgs());
  if (!ar->m_func->byRef(paramId)) {
    iopCGetG(origPc);
  } else {
    iopVGetG(origPc);
  }
}

OPTBLD_INLINE void iopFPassS(IOP_ARGS) {
  auto const ar = arFromInstr(pc - encoded_op_size(Op::FPassS));
  PC origPc = pc;
  auto paramId = decode_iva(pc);
  assert(paramId < ar->numArgs());
  if (!ar->m_func->byRef(paramId)) {
    iopCGetS(origPc);
  } else {
    iopVGetS(origPc);
  }
}

bool doFCall(ActRec* ar, PC& pc) {
  TRACE(3, "FCall: pc %p func %p base %d\n", vmpc(),
        vmfp()->m_func->unit()->entry(),
        int(vmfp()->m_func->base()));
  prepareFuncEntry(ar, pc, StackArgsState::Untrimmed);
  vmpc() = pc;
  if (EventHook::FunctionCall(ar, EventHook::NormalFunc)) return true;
  pc = vmpc();
  return false;
}

OPTBLD_INLINE void iopFCall(IOP_ARGS) {
  auto const ar = arFromInstr(pc - encoded_op_size(Op::FCall));
  UNUSED auto numArgs = decode_iva(pc);
  assert(numArgs == ar->numArgs());
  checkStack(vmStack(), ar->m_func, 0);
  ar->setReturn(vmfp(), pc, mcg->ustubs().retHelper);
  doFCall(ar, pc);
}

OPTBLD_FLT_INLINE void iopFCallD(IOP_ARGS) {
  auto const ar = arFromInstr(pc - encoded_op_size(Op::FCallD));
  UNUSED auto numArgs = decode_iva(pc);
  UNUSED auto clsName = decode_litstr(pc);
  UNUSED auto funcName = decode_litstr(pc);
  if (!RuntimeOption::EvalJitEnableRenameFunction &&
      !(ar->m_func->attrs() & AttrInterceptable)) {
    assert(ar->m_func->name()->isame(funcName));
  }
  assert(numArgs == ar->numArgs());
  checkStack(vmStack(), ar->m_func, 0);
  ar->setReturn(vmfp(), pc, mcg->ustubs().retHelper);
  doFCall(ar, pc);
}

OPTBLD_INLINE void iopFCallAwait(IOP_ARGS) {
  auto const ar = arFromInstr(pc - encoded_op_size(Op::FCallAwait));
  UNUSED auto numArgs = decode_iva(pc);
  UNUSED auto clsName = decode_litstr(pc);
  UNUSED auto funcName = decode_litstr(pc);

  if (!RuntimeOption::EvalJitEnableRenameFunction &&
      !(ar->m_func->attrs() & AttrInterceptable)) {
    assert(ar->m_func->name()->isame(funcName));
  }
  assert(numArgs == ar->numArgs());
  checkStack(vmStack(), ar->m_func, 0);
  ar->setReturn(vmfp(), pc, mcg->ustubs().retHelper);
  ar->setFCallAwait();
  doFCall(ar, pc);
}

OPTBLD_FLT_INLINE void iopFCallBuiltin(IOP_ARGS) {
  auto numArgs = decode_iva(pc);
  auto numNonDefault = decode_iva(pc);
  auto id = decode<Id>(pc);
  const NamedEntity* ne = vmfp()->m_func->unit()->lookupNamedEntityId(id);
  auto unit = vmfp()->func()->unit();
  auto strict = builtinCallUsesStrictTypes(unit);
  Func* func = Unit::lookupFunc(ne);
  if (func == nullptr) {
    raise_error("Call to undefined function %s()",
                vmfp()->m_func->unit()->lookupLitstrId(id)->data());
  }

  TypedValue* args = vmStack().indTV(numArgs-1);
  TypedValue ret;
  if (Native::coerceFCallArgs(args, numArgs, numNonDefault, func, strict)) {
    if (func->hasVariadicCaptureParam()) {
      assertx(numArgs > 0);
      assertx(isArrayType(args[1-numArgs].m_type));
    }
    Native::callFunc<true>(func, nullptr, args, numNonDefault, ret);
  } else {
    if (func->attrs() & AttrParamCoerceModeNull) {
      ret.m_type = KindOfNull;
    } else {
      assert(func->attrs() & AttrParamCoerceModeFalse);
      ret.m_type = KindOfBoolean;
      ret.m_data.num = 0;
    }
  }

  frame_free_args(args, numNonDefault);
  vmStack().ndiscard(numArgs);
  tvCopy(ret, *vmStack().allocTV());
}

enum class CallArrOnInvalidContainer {
  // task #1756122: warning and returning null is what we /should/ always
  // do in call_user_func_array, but some code depends on the broken
  // behavior of casting the list of args to FCallArray to an array.
  CastToArray,
  WarnAndReturnNull,
  WarnAndContinue
};

static bool doFCallArray(PC& pc, int numStackValues,
                         CallArrOnInvalidContainer onInvalid,
                         void* ret = nullptr) {
  assert(numStackValues >= 1);
  ActRec* ar = (ActRec*)(vmStack().top() + numStackValues);
  assert(ar->numArgs() == numStackValues);

  Cell* c1 = vmStack().topC();
  if (UNLIKELY(!isContainer(*c1))) {
    switch (onInvalid) {
      case CallArrOnInvalidContainer::CastToArray:
        tvCastToArrayInPlace(c1);
        break;
      case CallArrOnInvalidContainer::WarnAndReturnNull:
        vmStack().pushNull();
        cleanupParamsAndActRec(vmStack(), ar, nullptr, nullptr);
        raise_warning("call_user_func_array() expects parameter 2 to be array");
        return false;
      case CallArrOnInvalidContainer::WarnAndContinue: {
        Cell tmp = *c1;
        // argument_unpacking RFC dictates "containers and Traversables"
        raise_warning_unsampled("Only containers may be unpacked");
        c1->m_type = KindOfPersistentArray;
        c1->m_data.parr = staticEmptyArray();
        tvRefcountedDecRef(&tmp);
        break;
      }
    }
  }

  const Func* func = ar->m_func;
  {
    Cell args = *c1;
    vmStack().discard(); // prepareArrayArgs will push arguments onto the stack
    numStackValues--;
    SCOPE_EXIT { tvRefcountedDecRef(&args); };
    checkStack(vmStack(), func, 0);

    assert(!ar->resumed());
    TRACE(3, "FCallArray: pc %p func %p base %d\n", vmpc(),
          vmfp()->unit()->entry(),
          int(vmfp()->m_func->base()));
    ar->setReturn(vmfp(), pc, mcg->ustubs().retHelper);

    // When called from the jit, populate the correct return address
    if (ret) {
      ar->setJitReturn(ret);
    }

    auto prepResult = prepareArrayArgs(ar, args, vmStack(), numStackValues,
                                       /* ref param checks */ true, nullptr);
    if (UNLIKELY(!prepResult)) {
      vmStack().pushNull(); // return value is null if args are invalid
      return false;
    }
  }

  prepareFuncEntry(ar, pc, StackArgsState::Trimmed);
  vmpc() = pc;
  if (UNLIKELY(!EventHook::FunctionCall(ar, EventHook::NormalFunc))) {
    pc = vmpc();
    return false;
  }
  return true;
}

bool doFCallArrayTC(PC pc, int32_t numArgs, void* retAddr) {
  assert_native_stack_aligned();
  assert(tl_regState == VMRegState::DIRTY);
  tl_regState = VMRegState::CLEAN;
  auto onInvalid = CallArrOnInvalidContainer::WarnAndContinue;
  if (!numArgs) {
    numArgs = 1;
    onInvalid = CallArrOnInvalidContainer::CastToArray;
  }
  auto const ret = doFCallArray(pc, numArgs, onInvalid, retAddr);
  tl_regState = VMRegState::DIRTY;
  return ret;
}

OPTBLD_INLINE void iopFCallArray(IOP_ARGS) {
  doFCallArray(pc, 1, CallArrOnInvalidContainer::CastToArray);
}

OPTBLD_INLINE void iopFCallUnpack(IOP_ARGS) {
  auto const ar = arFromInstr(pc - encoded_op_size(Op::FCallUnpack));
  auto numArgs = decode_iva(pc);
  assert(numArgs == ar->numArgs());
  checkStack(vmStack(), ar->m_func, 0);
  doFCallArray(pc, numArgs, CallArrOnInvalidContainer::WarnAndContinue);
}

OPTBLD_INLINE void iopCufSafeArray(IOP_ARGS) {
  Array ret;
  ret.append(tvAsVariant(vmStack().top() + 1));
  ret.appendWithRef(tvAsVariant(vmStack().top() + 0));
  vmStack().popTV();
  vmStack().popTV();
  tvAsVariant(vmStack().top()) = ret;
}

OPTBLD_INLINE void iopCufSafeReturn(IOP_ARGS) {
  bool ok = cellToBool(*tvToCell(vmStack().top() + 1));
  tvRefcountedDecRef(vmStack().top() + 1);
  tvRefcountedDecRef(vmStack().top() + (ok ? 2 : 0));
  if (ok) vmStack().top()[2] = vmStack().top()[0];
  vmStack().ndiscard(2);
}

inline bool initIterator(PC& pc, PC origPc, Iter* it,
                         Offset offset, Cell* c1) {
  bool hasElems = it->init(c1);
  if (!hasElems) {
    pc = origPc + offset;
  }
  vmStack().popC();
  return hasElems;
}

OPTBLD_INLINE void iopIterInit(IOP_ARGS) {
  PC origPc = pc - encoded_op_size(Op::IterInit);
  auto itId = decode_ia(pc);
  auto offset = decode_ba(pc);
  auto val = decode_la(pc);
  Cell* c1 = vmStack().topC();
  Iter* it = frame_iter(vmfp(), itId);
  TypedValue* tv1 = frame_local(vmfp(), val);
  if (initIterator(pc, origPc, it, offset, c1)) {
    tvAsVariant(tv1) = it->arr().second();
  }
}

OPTBLD_INLINE void iopIterInitK(IOP_ARGS) {
  PC origPc = pc - encoded_op_size(Op::IterInitK);
  auto itId = decode_ia(pc);
  auto offset = decode_ba(pc);
  auto val = decode_la(pc);
  auto key = decode_la(pc);
  Cell* c1 = vmStack().topC();
  Iter* it = frame_iter(vmfp(), itId);
  TypedValue* tv1 = frame_local(vmfp(), val);
  TypedValue* tv2 = frame_local(vmfp(), key);
  if (initIterator(pc, origPc, it, offset, c1)) {
    tvAsVariant(tv1) = it->arr().second();
    tvAsVariant(tv2) = it->arr().first();
  }
}

OPTBLD_INLINE void iopWIterInit(IOP_ARGS) {
  PC origPc = pc - encoded_op_size(Op::WIterInit);
  auto itId = decode_ia(pc);
  auto offset = decode_ba(pc);
  auto val = decode_la(pc);
  Cell* c1 = vmStack().topC();
  Iter* it = frame_iter(vmfp(), itId);
  TypedValue* tv1 = frame_local(vmfp(), val);
  if (initIterator(pc, origPc, it, offset, c1)) {
    tvAsVariant(tv1).setWithRef(it->arr().secondRefPlus());
  }
}

OPTBLD_INLINE void iopWIterInitK(IOP_ARGS) {
  PC origPc = pc - encoded_op_size(Op::WIterInitK);
  auto itId = decode_ia(pc);
  auto offset = decode_ba(pc);
  auto val = decode_la(pc);
  auto key = decode_la(pc);
  Cell* c1 = vmStack().topC();
  Iter* it = frame_iter(vmfp(), itId);
  TypedValue* tv1 = frame_local(vmfp(), val);
  TypedValue* tv2 = frame_local(vmfp(), key);
  if (initIterator(pc, origPc, it, offset, c1)) {
    tvAsVariant(tv1).setWithRef(it->arr().secondRefPlus());
    tvAsVariant(tv2) = it->arr().first();
  }
}

inline bool initIteratorM(Iter* it, Offset offset, Ref* r1,
                          TypedValue *val, TypedValue *key) {
  TypedValue* rtv = r1->m_data.pref->tv();
  if (isArrayType(rtv->m_type)) {
    return new_miter_array_key(it, r1->m_data.pref, val, key);
  }
  if (rtv->m_type == KindOfObject)  {
    Class* ctx = arGetContextClass(vmfp());
    return new_miter_object(it, r1->m_data.pref, ctx, val, key);
  }
  return new_miter_other(it, r1->m_data.pref);
}

OPTBLD_INLINE void iopMIterInit(IOP_ARGS) {
  PC origPc = pc - encoded_op_size(Op::MIterInit);
  auto itId = decode_ia(pc);
  auto offset = decode_ba(pc);
  auto val = decode_la(pc);
  Ref* r1 = vmStack().topV();
  assert(r1->m_type == KindOfRef);
  Iter* it = frame_iter(vmfp(), itId);
  TypedValue* tv1 = frame_local(vmfp(), val);
  if (!initIteratorM(it, offset, r1, tv1, nullptr)) {
    pc = origPc + offset; // nothing to iterate; exit foreach loop.
  }
  vmStack().popV();
}

OPTBLD_INLINE void iopMIterInitK(IOP_ARGS) {
  PC origPc = pc - encoded_op_size(Op::MIterInitK);
  auto itId = decode_ia(pc);
  auto offset = decode_ba(pc);
  auto val = decode_la(pc);
  auto key = decode_la(pc);
  Ref* r1 = vmStack().topV();
  assert(r1->m_type == KindOfRef);
  Iter* it = frame_iter(vmfp(), itId);
  TypedValue* tv1 = frame_local(vmfp(), val);
  TypedValue* tv2 = frame_local(vmfp(), key);
  if (!initIteratorM(it, offset, r1, tv1, tv2)) {
    pc = origPc + offset; // nothing to iterate; exit foreach loop.
  }
  vmStack().popV();
}

OPTBLD_INLINE void iopIterNext(IOP_ARGS) {
  PC origPc = pc - encoded_op_size(Op::IterNext);
  auto itId = decode_ia(pc);
  auto offset = decode_ba(pc);
  auto val = decode_la(pc);
  jmpSurpriseCheck(offset);
  Iter* it = frame_iter(vmfp(), itId);
  TypedValue* tv1 = frame_local(vmfp(), val);
  if (it->next()) {
    pc = origPc + offset;
    tvAsVariant(tv1) = it->arr().second();
  }
}

OPTBLD_INLINE void iopIterNextK(IOP_ARGS) {
  PC origPc = pc - encoded_op_size(Op::IterNextK);
  auto itId = decode_ia(pc);
  auto offset = decode_ba(pc);
  auto val = decode_la(pc);
  auto key = decode_la(pc);
  jmpSurpriseCheck(offset);
  Iter* it = frame_iter(vmfp(), itId);
  TypedValue* tv1 = frame_local(vmfp(), val);
  TypedValue* tv2 = frame_local(vmfp(), key);
  if (it->next()) {
    pc = origPc + offset;
    tvAsVariant(tv1) = it->arr().second();
    tvAsVariant(tv2) = it->arr().first();
  }
}

OPTBLD_INLINE void iopWIterNext(IOP_ARGS) {
  PC origPc = pc - encoded_op_size(Op::WIterNext);
  auto itId = decode_ia(pc);
  auto offset = decode_ba(pc);
  auto val = decode_la(pc);
  jmpSurpriseCheck(offset);
  Iter* it = frame_iter(vmfp(), itId);
  TypedValue* tv1 = frame_local(vmfp(), val);
  if (it->next()) {
    pc = origPc + offset;
    tvAsVariant(tv1).setWithRef(it->arr().secondRefPlus());
  }
}

OPTBLD_INLINE void iopWIterNextK(IOP_ARGS) {
  PC origPc = pc - encoded_op_size(Op::WIterNextK);
  auto itId = decode_ia(pc);
  auto offset = decode_ba(pc);
  auto val = decode_la(pc);
  auto key = decode_la(pc);
  jmpSurpriseCheck(offset);
  Iter* it = frame_iter(vmfp(), itId);
  TypedValue* tv1 = frame_local(vmfp(), val);
  TypedValue* tv2 = frame_local(vmfp(), key);
  if (it->next()) {
    pc = origPc + offset;
    tvAsVariant(tv1).setWithRef(it->arr().secondRefPlus());
    tvAsVariant(tv2) = it->arr().first();
  }
}

OPTBLD_INLINE void iopMIterNext(IOP_ARGS) {
  PC origPc = pc - encoded_op_size(Op::MIterNext);
  auto itId = decode_ia(pc);
  auto offset = decode_ba(pc);
  auto val = decode_la(pc);
  jmpSurpriseCheck(offset);
  Iter* it = frame_iter(vmfp(), itId);
  TypedValue* tv1 = frame_local(vmfp(), val);
  if (miter_next_key(it, tv1, nullptr)) {
    pc = origPc + offset;
  }
}

OPTBLD_INLINE void iopMIterNextK(IOP_ARGS) {
  PC origPc = pc - encoded_op_size(Op::MIterNextK);
  auto itId = decode_ia(pc);
  auto offset = decode_ba(pc);
  auto val = decode_la(pc);
  auto key = decode_la(pc);
  jmpSurpriseCheck(offset);
  Iter* it = frame_iter(vmfp(), itId);
  TypedValue* tv1 = frame_local(vmfp(), val);
  TypedValue* tv2 = frame_local(vmfp(), key);
  if (miter_next_key(it, tv1, tv2)) {
    pc = origPc + offset;
  }
}

OPTBLD_INLINE void iopIterFree(IOP_ARGS) {
  auto itId = decode_ia(pc);
  Iter* it = frame_iter(vmfp(), itId);
  it->free();
}

OPTBLD_INLINE void iopMIterFree(IOP_ARGS) {
  auto itId = decode_ia(pc);
  Iter* it = frame_iter(vmfp(), itId);
  it->mfree();
}

OPTBLD_INLINE void iopCIterFree(IOP_ARGS) {
  auto itId = decode_ia(pc);
  Iter* it = frame_iter(vmfp(), itId);
  it->cfree();
}

OPTBLD_INLINE void inclOp(PC& pc, InclOpFlags flags, const char* opName) {
  Cell* c1 = vmStack().topC();
  auto path = String::attach(prepareKey(*c1));
  bool initial;
  TRACE(2, "inclOp %s %s %s %s \"%s\"\n",
        flags & InclOpFlags::Once ? "Once" : "",
        flags & InclOpFlags::DocRoot ? "DocRoot" : "",
        flags & InclOpFlags::Relative ? "Relative" : "",
        flags & InclOpFlags::Fatal ? "Fatal" : "",
        path.data());

  auto curUnitFilePath = [&] {
    namespace fs = boost::filesystem;
    fs::path currentUnit(vmfp()->m_func->unit()->filepath()->data());
    fs::path currentDir(currentUnit.branch_path());
    return currentDir.string();
  };

  auto const unit = [&] {
    if (flags & InclOpFlags::Relative) {
      String absPath = curUnitFilePath() + '/';
      absPath += path;
      return lookupUnit(absPath.get(), "", &initial);
    }
    if (flags & InclOpFlags::DocRoot) {
      return lookupUnit(
        SourceRootInfo::RelativeToPhpRoot(path).get(), "", &initial);
    }
    return lookupUnit(path.get(), curUnitFilePath().c_str(), &initial);
  }();

  vmStack().popC();
  if (unit == nullptr) {
    if (flags & InclOpFlags::Fatal) {
      raise_error("%s(%s): File not found", opName, path.data());
    } else {
      raise_warning("%s(%s): File not found", opName, path.data());
    }
    vmStack().pushBool(false);
    return;
  }

  if (!(flags & InclOpFlags::Once) || initial) {
    g_context->evalUnit(unit, pc, EventHook::PseudoMain);
  } else {
    Stats::inc(Stats::PseudoMain_Guarded);
    vmStack().pushBool(true);
  }
}

OPTBLD_INLINE void iopIncl(IOP_ARGS) {
  inclOp(pc, InclOpFlags::Default, "include");
}

OPTBLD_INLINE void iopInclOnce(IOP_ARGS) {
  inclOp(pc, InclOpFlags::Once, "include_once");
}

OPTBLD_INLINE void iopReq(IOP_ARGS) {
  inclOp(pc, InclOpFlags::Fatal, "require");
}

OPTBLD_INLINE void iopReqOnce(IOP_ARGS) {
  inclOp(pc, InclOpFlags::Fatal | InclOpFlags::Once, "require_once");
}

OPTBLD_INLINE void iopReqDoc(IOP_ARGS) {
  inclOp(
    pc,
    InclOpFlags::Fatal | InclOpFlags::Once | InclOpFlags::DocRoot,
    "require_once"
  );
}

OPTBLD_INLINE void iopEval(IOP_ARGS) {
  Cell* c1 = vmStack().topC();

  if (UNLIKELY(RuntimeOption::EvalAuthoritativeMode)) {
    // Ahead of time whole program optimizations need to assume it can
    // see all the code, or it really can't do much.
    raise_error("You can't use eval in RepoAuthoritative mode");
  }

  auto code = String::attach(prepareKey(*c1));
  String prefixedCode = concat("<?php ", code);

  auto evalFilename = std::string();
  auto vm = &*g_context;
  string_printf(
    evalFilename,
    "%s(%d)(%s" EVAL_FILENAME_SUFFIX,
    vm->getContainingFileName()->data(),
    vm->getLine(),
    string_md5(code.data(), code.size()).c_str()
  );
  Unit* unit = vm->compileEvalString(prefixedCode.get(), evalFilename.c_str());
  if (!RuntimeOption::EvalJitEvaledCode) {
    unit->setInterpretOnly();
  }
  const StringData* msg;
  int line = 0;

  vmStack().popC();
  if (unit->parseFatal(msg, line)) {
    auto const errnum = static_cast<int>(ErrorMode::WARNING);
    if (vm->errorNeedsLogging(errnum)) {
      // manual call to Logger instead of logError as we need to use
      // evalFileName and line as the exception doesn't track the eval()
      Logger::Error(
        "\nFatal error: %s in %s on line %d",
        msg->data(),
        evalFilename.c_str(),
        line
      );
    }

    vmStack().pushBool(false);
    return;
  }
  vm->evalUnit(unit, pc, EventHook::Eval);
}

OPTBLD_INLINE void iopDefFunc(IOP_ARGS) {
  auto fid = decode_iva(pc);
  Func* f = vmfp()->m_func->unit()->lookupFuncId(fid);
  setCachedFunc(f, isDebuggerAttached());
}

OPTBLD_INLINE void iopDefCls(IOP_ARGS) {
  auto cid = decode_iva(pc);
  PreClass* c = vmfp()->m_func->unit()->lookupPreClassId(cid);
  Unit::defClass(c);
}

OPTBLD_INLINE void iopDefClsNop(IOP_ARGS) {
  decode_iva(pc); // cid
}

OPTBLD_INLINE void iopDefTypeAlias(IOP_ARGS) {
  auto tid = decode_iva(pc);
  vmfp()->m_func->unit()->defTypeAlias(tid);
}

static inline void checkThis(ActRec* fp) {
  if (!fp->hasThis()) {
    raise_error(Strings::FATAL_NULL_THIS);
  }
}

OPTBLD_INLINE void iopThis(IOP_ARGS) {
  checkThis(vmfp());
  ObjectData* this_ = vmfp()->getThis();
  vmStack().pushObject(this_);
}

OPTBLD_INLINE void iopBareThis(IOP_ARGS) {
  auto bto = decode_oa<BareThisOp>(pc);
  if (vmfp()->hasThis()) {
    ObjectData* this_ = vmfp()->getThis();
    vmStack().pushObject(this_);
  } else {
    vmStack().pushNull();
    switch (bto) {
    case BareThisOp::Notice:   raise_notice(Strings::WARN_NULL_THIS); break;
    case BareThisOp::NoNotice: break;
    case BareThisOp::NeverNull:
      assert(!"$this cannot be null in BareThis with NeverNull option");
      break;
    }
  }
}

OPTBLD_INLINE void iopCheckThis(IOP_ARGS) {
  checkThis(vmfp());
}

OPTBLD_INLINE void iopInitThisLoc(IOP_ARGS) {
  auto id = decode_la(pc);
  TypedValue* thisLoc = frame_local(vmfp(), id);
  tvRefcountedDecRef(thisLoc);
  if (vmfp()->hasThis()) {
    thisLoc->m_data.pobj = vmfp()->getThis();
    thisLoc->m_type = KindOfObject;
    tvIncRef(thisLoc);
  } else {
    tvWriteUninit(thisLoc);
  }
}

static inline RefData* lookupStatic(const StringData* name,
                                    const ActRec* fp,
                                    bool& inited) {
  auto const func = fp->m_func;

  if (UNLIKELY(func->isClosureBody())) {
    assert(!func->hasVariadicCaptureParam());
    return lookupStaticFromClosure(
      frame_local(fp, func->numParams())->m_data.pobj, name, inited);
  }

  auto const refData = rds::bindStaticLocal(func, name);
  inited = !refData->isUninitializedInRDS();
  if (!inited) refData->initInRDS();
  return refData.get();
}

OPTBLD_INLINE void iopStaticLoc(IOP_ARGS) {
  auto localId = decode_la(pc);
  auto var = decode_litstr(pc);

  bool inited;
  auto const refData = lookupStatic(var, vmfp(), inited);
  if (!inited) {
    refData->tv()->m_type = KindOfNull;
  }

  auto const tvLocal = frame_local(vmfp(), localId);
  auto const tmpTV = make_tv<KindOfRef>(refData);
  tvBind(&tmpTV, tvLocal);
  vmStack().pushBool(inited);
}

OPTBLD_INLINE void iopStaticLocInit(IOP_ARGS) {
  auto localId = decode_la(pc);
  auto var = decode_litstr(pc);

  bool inited;
  auto const refData = lookupStatic(var, vmfp(), inited);

  if (!inited) {
    auto const initVal = vmStack().topC();
    cellDup(*initVal, *refData->tv());
  }

  auto const tvLocal = frame_local(vmfp(), localId);
  auto const tmpTV = make_tv<KindOfRef>(refData);
  tvBind(&tmpTV, tvLocal);
  vmStack().discard();
}

OPTBLD_INLINE void iopCatch(IOP_ARGS) {
  auto vm = &*g_context;
  assert(vm->m_faults.size() > 0);
  Fault fault = vm->m_faults.back();
  vm->m_faults.pop_back();
  assert(fault.m_raiseFrame == vmfp());
  assert(fault.m_userException);
  vmStack().pushObjectNoRc(fault.m_userException);
}

OPTBLD_INLINE void iopLateBoundCls(IOP_ARGS) {
  Class* cls = frameStaticClass(vmfp());
  if (!cls) {
    raise_error(HPHP::Strings::CANT_ACCESS_STATIC);
  }
  vmStack().pushClass(cls);
}

OPTBLD_INLINE void iopVerifyParamType(IOP_ARGS) {
  auto paramId = decode_la(pc);
  const Func *func = vmfp()->m_func;
  assert(paramId < func->numParams());
  assert(func->numParams() == int(func->params().size()));
  const TypeConstraint& tc = func->params()[paramId].typeConstraint;
  assert(tc.hasConstraint());
  bool useStrictTypes =
    func->unit()->isHHFile() || RuntimeOption::EnableHipHopSyntax ||
    !vmfp()->useWeakTypes();
  if (!tc.isTypeVar() && !tc.isTypeConstant()) {
    tc.verifyParam(frame_local(vmfp(), paramId), func, paramId, useStrictTypes);
  }
}

OPTBLD_INLINE void implVerifyRetType() {
  if (LIKELY(!RuntimeOption::EvalCheckReturnTypeHints)) {
    return;
  }

  const auto func = vmfp()->m_func;
  const auto tc = func->returnTypeConstraint();
  bool useStrictTypes = func->unit()->useStrictTypes();
  if (!tc.isTypeVar() && !tc.isTypeConstant()) {
    tc.verifyReturn(vmStack().topTV(), func, useStrictTypes);
  }
}

OPTBLD_INLINE void iopVerifyRetTypeC(IOP_ARGS) {
  implVerifyRetType();
}

OPTBLD_INLINE void iopVerifyRetTypeV(IOP_ARGS) {
  implVerifyRetType();
}

OPTBLD_INLINE TCA iopNativeImpl(IOP_ARGS) {
  auto const jitReturn = jitReturnPre(vmfp());

  BuiltinFunction func = vmfp()->func()->builtinFuncPtr();
  assert(func);
  // Actually call the native implementation. This will handle freeing the
  // locals in the normal case. In the case of an exception, the VM unwinder
  // will take care of it.
  func(vmfp());

  // Grab caller info from ActRec.
  ActRec* sfp = vmfp()->sfp();
  Offset soff = vmfp()->m_soff;

  // Adjust the stack; the native implementation put the return value in the
  // right place for us already
  vmStack().ndiscard(vmfp()->func()->numSlotsInFrame());
  vmStack().ret();

  // Return control to the caller.
  vmfp() = sfp;
  pc = LIKELY(vmfp() != nullptr) ? vmfp()->func()->getEntry() + soff : nullptr;

  return jitReturnPost(jitReturn);
}

OPTBLD_INLINE void iopSelf(IOP_ARGS) {
  Class* clss = arGetContextClass(vmfp());
  if (!clss) {
    raise_error(HPHP::Strings::CANT_ACCESS_SELF);
  }
  vmStack().pushClass(clss);
}

OPTBLD_INLINE void iopParent(IOP_ARGS) {
  Class* clss = arGetContextClass(vmfp());
  if (!clss) {
    raise_error(HPHP::Strings::CANT_ACCESS_PARENT_WHEN_NO_CLASS);
  }
  Class* parent = clss->parent();
  if (!parent) {
    raise_error(HPHP::Strings::CANT_ACCESS_PARENT_WHEN_NO_PARENT);
  }
  vmStack().pushClass(parent);
}

OPTBLD_INLINE void iopCreateCl(IOP_ARGS) {
  auto numArgs = decode_iva(pc);
  auto clsName = decode_litstr(pc);
  auto const cls = Unit::loadClass(clsName)->rescope(
    const_cast<Class*>(vmfp()->m_func->cls())
  );
  auto obj = newInstance(cls);
  c_Closure::fromObject(obj)->init(numArgs, vmfp(), vmStack().top());
  vmStack().ndiscard(numArgs);
  vmStack().pushObjectNoRc(obj);
}

static inline BaseGenerator* this_base_generator(const ActRec* fp) {
  auto const obj = fp->getThis();
  assert(obj->getVMClass() == AsyncGenerator::getClass() ||
         obj->getVMClass() == Generator::getClass());
  return obj->getVMClass() == Generator::getClass()
    ? static_cast<BaseGenerator*>(Generator::fromObject(obj))
    : static_cast<BaseGenerator*>(AsyncGenerator::fromObject(obj));
}

static inline Generator* this_generator(const ActRec* fp) {
  auto const obj = fp->getThis();
  return Generator::fromObject(obj);
}

const StaticString s_this("this");

OPTBLD_INLINE TCA iopCreateCont(IOP_ARGS) {
  auto const jitReturn = jitReturnPre(vmfp());

  auto const fp = vmfp();
  auto const func = fp->func();
  auto const numSlots = func->numSlotsInFrame();
  auto const resumeOffset = func->unit()->offsetOf(pc);
  assert(!fp->resumed());
  assert(func->isGenerator());

  // Create the {Async,}Generator object. Create takes care of copying local
  // variables and iterators.
  auto const obj = func->isAsync()
    ? AsyncGenerator::Create(fp, numSlots, nullptr, resumeOffset)
    : Generator::Create<false>(fp, numSlots, nullptr, resumeOffset);

  auto const genData = func->isAsync() ?
    static_cast<BaseGenerator*>(AsyncGenerator::fromObject(obj)) :
    static_cast<BaseGenerator*>(Generator::fromObject(obj));

  EventHook::FunctionSuspendE(fp, genData->actRec());

  // Grab caller info from ActRec.
  ActRec* sfp = fp->sfp();
  Offset soff = fp->m_soff;

  // Free ActRec and store the return value.
  vmStack().ndiscard(numSlots);
  vmStack().ret();
  tvCopy(make_tv<KindOfObject>(obj), *vmStack().topTV());
  assert(vmStack().topTV() == &fp->m_r);

  // Return control to the caller.
  vmfp() = sfp;
  pc = LIKELY(sfp != nullptr) ? sfp->func()->getEntry() + soff : nullptr;

  return jitReturnPost(jitReturn);
}

OPTBLD_INLINE void moveProgramCounterIntoGenerator(PC &pc, BaseGenerator* gen) {
  assert(gen->isRunning());
  ActRec* genAR = gen->actRec();
  genAR->setReturn(vmfp(), pc, genAR->func()->isAsync() ?
    mcg->ustubs().asyncGenRetHelper :
    mcg->ustubs().genRetHelper);

  vmfp() = genAR;

  assert(genAR->func()->contains(gen->resumable()->resumeOffset()));
  pc = genAR->func()->unit()->at(gen->resumable()->resumeOffset());
  vmpc() = pc;
}

OPTBLD_INLINE bool tvIsGenerator(TypedValue tv) {
  return tv.m_type == KindOfObject &&
         tv.m_data.pobj->instanceof(Generator::getClass());
}

template<bool recursive>
OPTBLD_INLINE void contEnterImpl(PC& pc) {

  // The stack must have one cell! Or else resumableStackBase() won't work!
  assert(vmStack().top() + 1 ==
         (TypedValue*)vmfp() - vmfp()->m_func->numSlotsInFrame());

  // Do linkage of the generator's AR.
  assert(vmfp()->hasThis());
  // `recursive` determines whether we enter just the top generator or whether
  // we drop down to the lowest running delegate generator. This is useful for
  // ContRaise, which should throw from the context of the lowest generator.
  if(!recursive || vmfp()->getThis()->getVMClass() != Generator::getClass()) {
    moveProgramCounterIntoGenerator(pc, this_base_generator(vmfp()));
  } else {
    // TODO(https://github.com/facebook/hhvm/issues/6040)
    // Implement throwing from delegate generators.
    assert(vmfp()->getThis()->getVMClass() == Generator::getClass());
    auto gen = this_generator(vmfp());
    if (gen->m_delegate.m_type != KindOfNull) {
      SystemLib::throwExceptionObject("Throwing from a delegate generator is "
          "not currently supported in HHVM");
    }
    moveProgramCounterIntoGenerator(pc, gen);
  }

  EventHook::FunctionResumeYield(vmfp());
}

OPTBLD_INLINE void iopContEnter(IOP_ARGS) {
  contEnterImpl<false>(pc);
}

OPTBLD_INLINE void iopContRaise(IOP_ARGS) {
  contEnterImpl<true>(pc);
  iopThrow(pc);
}

OPTBLD_INLINE void moveProgramCounterToCaller(PC& pc) {
  auto fp = vmfp();
  // Grab caller info from ActRec.
  ActRec* sfp = fp->sfp();
  Offset soff = fp->m_soff;

  // Return control to the next()/send()/raise() caller.
  vmfp() = sfp;
  pc = sfp != nullptr ? sfp->func()->getEntry() + soff : nullptr;
}

OPTBLD_INLINE TCA yield(PC& pc, const Cell* key, const Cell value) {
  auto const jitReturn = jitReturnPre(vmfp());

  auto const fp = vmfp();
  auto const func = fp->func();
  auto const resumeOffset = func->unit()->offsetOf(pc);
  assert(fp->resumed());
  assert(func->isGenerator());

  EventHook::FunctionSuspendR(fp, nullptr);

  if (!func->isAsync()) {
    // Non-async generator.
    assert(fp->sfp());
    frame_generator(fp)->yield(resumeOffset, key, value);

    // Push return value of next()/send()/raise().
    vmStack().pushNull();
  } else {
    // Async generator.
    auto const gen = frame_async_generator(fp);
    auto const eagerResult = gen->yield(resumeOffset, key, value);
    if (eagerResult) {
      // Eager execution => return StaticWaitHandle.
      assert(fp->sfp());
      vmStack().pushObjectNoRc(eagerResult);
    } else {
      // Resumed execution => return control to the scheduler.
      assert(!fp->sfp());
    }
  }

  moveProgramCounterToCaller(pc);

  return jitReturnPost(jitReturn);
}

OPTBLD_INLINE TCA iopYield(IOP_ARGS) {
  auto const value = *vmStack().topC();
  vmStack().discard();
  return yield(pc, nullptr, value);
}

OPTBLD_INLINE TCA iopYieldK(IOP_ARGS) {
  auto const key = *vmStack().indC(1);
  auto const value = *vmStack().topC();
  vmStack().ndiscard(2);
  return yield(pc, &key, value);
}

OPTBLD_INLINE bool typeIsValidGeneratorDelegate(DataType type) {
  return type == KindOfArray           ||
         type == KindOfPersistentArray ||
         type == KindOfObject;
}

OPTBLD_INLINE void iopContAssignDelegate(IOP_ARGS) {
  auto param = *vmStack().topC();
  vmStack().discard();

  auto itId = decode_ia(pc);

  auto gen = frame_generator(vmfp());
  auto iter = frame_iter(vmfp(), itId);

  if (UNLIKELY(!typeIsValidGeneratorDelegate(param.m_type))) {
    tvRefcountedDecRef(param);
    SystemLib::throwErrorObject(
      "Can use \"yield from\" only with arrays and Traversables"
    );
  }

  // We don't use the iterator if we have a delegate generator (as iterators
  // mess with the internal state of the generator), so short circuit and dont
  // init our iterator in that case. Otherwise, if we init our iterator and it
  // returns false then we know that we have an empty iterator (like `[]`) in
  // which case just set our delegate to Null so that ContEnterDelegate and
  // YieldFromDelegate know something is up.
  if (tvIsGenerator(param) || iter->init(&param)) {
    cellSet(param, gen->m_delegate);
  } else {
    cellSetNull(gen->m_delegate);
  }
  // When using a subgenerator we don't actually read the values of the m_key
  // and m_value of our frame generator (the delegating generator). The
  // generator itself is still holding a reference to them though, so null
  // out the key/value to free the memory.
  cellSetNull(gen->m_key);
  cellSetNull(gen->m_value);
}

OPTBLD_INLINE void iopContEnterDelegate(IOP_ARGS) {
  // Make sure we have a delegate
  auto gen = frame_generator(vmfp());

  // Ignore the VM Stack, we want to pass that down from ContEnter

  // ContEnterDelegate doesn't do anything for iterators.
  if (!tvIsGenerator(gen->m_delegate)) {
    return;
  }

  auto delegate = Generator::fromObject(gen->m_delegate.m_data.pobj);

  if (delegate->getState() == BaseGenerator::State::Done) {
    // If our generator finished earlier (or if there was nothing to do) just
    // continue on and let YieldFromDelegate handle cleaning up.
    return;
  }

  // A pretty odd if statement, but consider the following situation.
  // Generators A and B both do `yield from` on a shared delegate generator,
  // C. When A is first used we autoprime it, and therefore also autoprime C as
  // well. Then we also autoprime B when it gets used, which advances C past
  // some perfectly valid data.
  // Basically this check is to make sure that we autoprime delegate generators
  // when needed, and not if they're shared.
  if (gen->getState() == BaseGenerator::State::Priming &&
      delegate->getState() != BaseGenerator::State::Created) {
    return;
  }

  // We're about to resume executing our generator, so make sure we're in the
  // right state.
  delegate->preNext(false);

  moveProgramCounterIntoGenerator(pc, delegate);
  EventHook::FunctionResumeYield(vmfp());
}

OPTBLD_INLINE
TCA yieldFromGenerator(PC& pc, Generator* gen, Offset resumeOffset) {
  auto fp = vmfp();

  assert(tvIsGenerator(gen->m_delegate));
  auto delegate = Generator::fromObject(gen->m_delegate.m_data.pobj);

  if (delegate->getState() == BaseGenerator::State::Done) {
    // If the generator is done, just copy the return value onto the stack.
    cellDup(delegate->m_value, *vmStack().topTV());
    return nullptr;
  }

  auto jitReturn = jitReturnPre(fp);

  EventHook::FunctionSuspendR(fp, nullptr);
  // We don't actually want to "yield" anything here. The implementation of
  // key/current are smart enough to dive into our delegate generator, so
  // really what we want to do is clean up all of the generator metadata
  // (state, ressume address, etc) and continue on.
  assert(gen->isRunning());
  gen->resumable()->setResumeAddr(nullptr, resumeOffset);
  gen->setState(BaseGenerator::State::Started);

  moveProgramCounterToCaller(pc);

  return jitReturnPost(jitReturn);
}

OPTBLD_INLINE
TCA yieldFromIterator(PC& pc, Generator* gen, Iter* it, Offset resumeOffset) {
  auto fp = vmfp();

  // For the most part this should never happen, the emitter assigns our
  // delegate to a non-null value in ContAssignDelegate. The one exception to
  // this is if we are given an empty iterator, in which case
  // ContAssignDelegate will remove our delegate and just send us to
  // YieldFromDelegate to return our null.
  if (UNLIKELY(gen->m_delegate.m_type == KindOfNull)) {
    tvWriteNull(vmStack().topTV());
    return nullptr;
  }

  // Otherwise, if iteration is finished we just return null.
  auto arr = it->arr();
  if (arr.end()) {
    // Push our null return value onto the stack
    tvWriteNull(vmStack().topTV());
    return nullptr;
  }

  auto jitReturn = jitReturnPre(fp);

  EventHook::FunctionSuspendR(fp, nullptr);
  auto key = *(arr.first().asTypedValue());
  auto value = *(arr.second().asTypedValue());
  gen->yield(resumeOffset, &key, value);

  moveProgramCounterToCaller(pc);

  it->next();

  return jitReturnPost(jitReturn);
}

OPTBLD_INLINE TCA iopYieldFromDelegate(IOP_ARGS) {
  auto gen = frame_generator(vmfp());

  auto func = vmfp()->func();
  PC origPc = pc - encoded_op_size(Op::YieldFromDelegate);

  auto itId = decode_ia(pc);
  Iter* it = frame_iter(vmfp(), itId);

  auto offset = decode_ba(pc);
  auto resumePc = origPc + offset;
  auto resumeOffset = func->unit()->offsetOf(resumePc);

  if (tvIsGenerator(gen->m_delegate)) {
    return yieldFromGenerator(pc, gen, resumeOffset);
  } else {
    return yieldFromIterator(pc, gen, it, resumeOffset);
  }
}

OPTBLD_INLINE void iopContUnsetDelegate(IOP_ARGS) {
  auto itId = decode_ia(pc);
  auto shouldFreeIter = (bool)decode_iva(pc);

  auto gen = frame_generator(vmfp());

  // The `shouldFreeIter` immediate determines whether we need to call free
  // on our iterator or not. Normally if we finish executing our yield from
  // successfully then the implementation of `next` will automatically do it
  // for us when there aren't any elements left, but if an exception is thrown
  // then we need to do it manually. We don't use the iterator when the
  // delegate is a generator though, so even if the param tells us to free it
  // we should just ignore it.
  if (UNLIKELY(shouldFreeIter && !tvIsGenerator(gen->m_delegate))) {
    auto iter = frame_iter(vmfp(), itId);
    iter->free();
  }

  cellSetNull(gen->m_delegate);
}

OPTBLD_INLINE void iopContCheck(IOP_ARGS) {
  auto checkStarted = decode_iva(pc);
  this_base_generator(vmfp())->preNext(checkStarted);
}

OPTBLD_INLINE void iopContValid(IOP_ARGS) {
  vmStack().pushBool(
    this_generator(vmfp())->getState() != BaseGenerator::State::Done);
}

OPTBLD_INLINE void iopContStarted(IOP_ARGS) {
  vmStack().pushBool(
    this_generator(vmfp())->getState() != BaseGenerator::State::Created);
}

OPTBLD_INLINE Generator *currentlyDelegatedGenerator(Generator *gen) {
  while(tvIsGenerator(gen->m_delegate)) {
    gen = Generator::fromObject(gen->m_delegate.m_data.pobj);
  }
  return gen;
}

OPTBLD_INLINE void iopContKey(IOP_ARGS) {
  Generator* cont = this_generator(vmfp());
  if (!RuntimeOption::AutoprimeGenerators) cont->startedCheck();

  // If we are currently delegating to a generator, return its key instead
  cont = currentlyDelegatedGenerator(cont);

  cellDup(cont->m_key, *vmStack().allocC());
}

OPTBLD_INLINE void iopContCurrent(IOP_ARGS) {
  Generator* cont = this_generator(vmfp());
  if (!RuntimeOption::AutoprimeGenerators) cont->startedCheck();

  // If we are currently delegating to a generator, return its value instead
  cont = currentlyDelegatedGenerator(cont);

  if(cont->getState() == BaseGenerator::State::Done) {
    vmStack().pushNull();
  } else {
    cellDup(cont->m_value, *vmStack().allocC());
  }
}

OPTBLD_INLINE void iopContGetReturn(IOP_ARGS) {
  Generator* cont = this_generator(vmfp());
  if (!RuntimeOption::AutoprimeGenerators) cont->startedCheck();

  if(!cont->successfullyFinishedExecuting()) {
    SystemLib::throwExceptionObject("Cannot get return value of a generator "
                                    "that hasn't returned");
  }

  cellDup(cont->m_value, *vmStack().allocC());
}

OPTBLD_INLINE void asyncSuspendE(PC& pc) {
  assert(!vmfp()->resumed());
  assert(vmfp()->func()->isAsyncFunction());
  const auto func = vmfp()->m_func;
  const auto resumeOffset = func->unit()->offsetOf(pc);

  // Pop the blocked dependency.
  Cell* value = vmStack().topC();
  assert(value->m_type == KindOfObject);
  assert(value->m_data.pobj->instanceof(c_WaitableWaitHandle::classof()));

  auto child = static_cast<c_WaitableWaitHandle*>(value->m_data.pobj);
  assert(!child->isFinished());
  vmStack().discard();

  // Create the AsyncFunctionWaitHandle object. Create takes care of
  // copying local variables and itertors.
  auto waitHandle = static_cast<c_AsyncFunctionWaitHandle*>(
    c_AsyncFunctionWaitHandle::Create<true>(vmfp(),
                                            vmfp()->func()->numSlotsInFrame(),
                                            nullptr, resumeOffset, child));

  // Call the FunctionSuspend hook. FunctionSuspend will decref the newly
  // allocated waitHandle if it throws.
  EventHook::FunctionSuspendE(vmfp(), waitHandle->actRec());

  // Grab caller info from ActRec.
  ActRec* sfp = vmfp()->sfp();
  Offset soff = vmfp()->m_soff;

  // Free ActRec and store the return value.
  vmStack().ndiscard(vmfp()->m_func->numSlotsInFrame());
  vmStack().ret();
  tvCopy(make_tv<KindOfObject>(waitHandle), *vmStack().topTV());
  assert(vmStack().topTV() == &vmfp()->m_r);
  // In case we were called by a jitted FCallAwait, let it know
  // that we suspended.
  vmStack().topTV()->m_aux.u_fcallAwaitFlag = 1;
  // Return control to the caller.
  vmfp() = sfp;
  pc = LIKELY(vmfp() != nullptr) ?
    vmfp()->func()->getEntry() + soff : nullptr;
}

OPTBLD_INLINE void asyncSuspendR(PC& pc) {
  auto const fp = vmfp();
  auto const func = fp->func();
  auto const resumeOffset = func->unit()->offsetOf(pc);
  assert(fp->resumed());
  assert(func->isAsync());

  // Obtain child
  Cell& value = *vmStack().topC();
  assert(value.m_type == KindOfObject);
  assert(value.m_data.pobj->instanceof(c_WaitableWaitHandle::classof()));
  auto const child = static_cast<c_WaitableWaitHandle*>(value.m_data.pobj);

  // Before adjusting the stack or doing anything, check the suspend hook.
  // This can throw.
  EventHook::FunctionSuspendR(fp, child);

  // Await child and suspend the async function/generator. May throw.
  if (!func->isGenerator()) {
    // Async function.
    assert(!fp->sfp());
    frame_afwh(fp)->await(resumeOffset, child);
    vmStack().discard();
  } else {
    // Async generator.
    auto const gen = frame_async_generator(fp);
    auto const eagerResult = gen->await(resumeOffset, child);
    vmStack().discard();
    if (eagerResult) {
      // Eager execution => return AsyncGeneratorWaitHandle.
      assert(fp->sfp());
      vmStack().pushObjectNoRc(eagerResult);
    } else {
      // Resumed execution => return control to the scheduler.
      assert(!fp->sfp());
    }
  }

  // Grab caller info from ActRec.
  ActRec* sfp = fp->sfp();
  Offset soff = fp->m_soff;

  // Return control to the caller or scheduler.
  vmfp() = sfp;
  pc = sfp != nullptr ? sfp->func()->getEntry() + soff : nullptr;
}

OPTBLD_INLINE TCA iopAwait(IOP_ARGS) {
  auto const awaitable = vmStack().topC();
  auto wh = c_WaitHandle::fromCell(awaitable);
  if (UNLIKELY(wh == nullptr)) {
    if (LIKELY(awaitable->m_type == KindOfObject)) {
      auto const obj = awaitable->m_data.pobj;
      auto const cls = obj->getVMClass();
      auto const func = cls->lookupMethod(s_getWaitHandle.get());
      if (func && !(func->attrs() & AttrStatic)) {
        TypedValue ret;
        g_context->invokeFuncFew(&ret, func, obj, nullptr, 0, nullptr);
        cellSet(*tvToCell(&ret), *vmStack().topC());
        tvRefcountedDecRef(ret);
        wh = c_WaitHandle::fromCell(vmStack().topC());
      }
    }

    if (UNLIKELY(wh == nullptr)) {
      SystemLib::throwBadMethodCallExceptionObject("Await on a non-WaitHandle");
    }
  }
  if (LIKELY(wh->isFailed())) {
    throw req::root<Object>{wh->getException()};
  }
  if (wh->isSucceeded()) {
    cellSet(wh->getResult(), *vmStack().topC());
    return nullptr;
  }
  return suspendStack(pc);
}

TCA suspendStack(PC &pc) {
  while (true) {
    auto const jitReturn = []() {
      // FixMe: what to do here?
      try {
        return jitReturnPre(vmfp());
      } catch (...) {
        // We're halfway through a bytecode
        always_assert(false);
      }
    }();

    if (vmfp()->resumed()) {
      // suspend resumed execution
      asyncSuspendR(pc);
      return jitReturnPost(jitReturn);
    }

    auto const suspendOuter = vmfp()->isFCallAwait();
    assertx(jitReturn.sfp || !suspendOuter);

    // suspend eager execution
    asyncSuspendE(pc);

    auto retIp = jitReturnPost(jitReturn);
    if (!suspendOuter) return retIp;
    if (retIp) {
      auto const& us = mcg->ustubs();
      if (retIp == us.resumeHelper) retIp = us.fcallAwaitSuspendHelper;
      return retIp;
    }
    vmpc() = pc;
  }
}

OPTBLD_INLINE void iopWHResult(IOP_ARGS) {
  // we should never emit this bytecode for non-waithandle
  auto const wh = c_WaitHandle::fromCellAssert(vmStack().topC());
  // the failure condition is likely since we punt to this opcode
  // in the JIT when the state is failed.
  if (wh->isFailed()) {
    throw_object(Object{wh->getException()});
  }
  if (wh->isSucceeded()) {
    cellSet(wh->getResult(), *vmStack().topC());
    return;
  }
  SystemLib::throwInvalidOperationExceptionObject(
    "Request for result on pending wait handle, "
    "must await or join() before calling result()");
  not_reached();
}

OPTBLD_INLINE void iopCheckProp(IOP_ARGS) {
  auto propName = decode_litstr(pc);

  auto* cls = vmfp()->getClass();
  auto* propVec = cls->getPropData();
  always_assert(propVec);

  auto* ctx = arGetContextClass(vmfp());
  auto idx = ctx->lookupDeclProp(propName);

  auto& tv = (*propVec)[idx];
  vmStack().pushBool(tv.m_type != KindOfUninit);
}

OPTBLD_INLINE void iopInitProp(IOP_ARGS) {
  auto propName = decode_litstr(pc);
  auto propOp = decode_oa<InitPropOp>(pc);

  auto* cls = vmfp()->getClass();
  TypedValue* tv;

  auto* ctx = arGetContextClass(vmfp());
  auto* fr = vmStack().topC();

  switch (propOp) {
    case InitPropOp::Static:
      tv = cls->getSPropData(ctx->lookupSProp(propName));
      break;

    case InitPropOp::NonStatic: {
      auto* propVec = cls->getPropData();
      always_assert(propVec);
      Slot idx = ctx->lookupDeclProp(propName);
      tv = &(*propVec)[idx];
    } break;
  }

  cellDup(*fr, *tvToCell(tv));
  vmStack().popC();
}

OPTBLD_INLINE void iopIncStat(IOP_ARGS) {
  auto counter = decode_iva(pc);
  auto value = decode_iva(pc);
  Stats::inc(Stats::StatCounter(counter), value);
}

OPTBLD_INLINE void iopOODeclExists(IOP_ARGS) {
  auto subop = decode<OODeclExistsOp>(pc);

  TypedValue* aloadTV = vmStack().topTV();
  tvCastToBooleanInPlace(aloadTV);
  assert(aloadTV->m_type == KindOfBoolean);
  bool autoload = aloadTV->m_data.num;
  vmStack().popX();

  TypedValue* name = vmStack().topTV();
  tvCastToStringInPlace(name);
  assert(isStringType(name->m_type));

  ClassKind kind;
  switch (subop) {
    case OODeclExistsOp::Class : kind = ClassKind::Class; break;
    case OODeclExistsOp::Trait : kind = ClassKind::Trait; break;
    case OODeclExistsOp::Interface : kind = ClassKind::Interface; break;
  }
  tvAsVariant(name) = Unit::classExists(name->m_data.pstr, autoload, kind);
}

OPTBLD_INLINE void iopSilence(IOP_ARGS) {
  auto localId = decode_la(pc);
  auto subop = decode_oa<SilenceOp>(pc);

  switch (subop) {
    case SilenceOp::Start: {
      auto level = zero_error_level();
      TypedValue* local = frame_local(vmfp(), localId);
      local->m_type = KindOfInt64;
      local->m_data.num = level;
      break;
    }
    case SilenceOp::End: {
      TypedValue* oldTV = frame_local(vmfp(), localId);
      assert(oldTV->m_type == KindOfInt64);
      restore_error_level(oldTV->m_data.num);
      break;
    }
  }
}

std::string prettyStack(const std::string& prefix) {
  if (!vmfp()) return "__Halted";
  int offset = (vmfp()->m_func->unit() != nullptr)
               ? pcOff() : 0;
  auto begPrefix = prefix + "__";
  auto midPrefix = prefix + "|| ";
  auto endPrefix = prefix + "\\/";
  auto stack = vmStack().toString(vmfp(), offset, midPrefix);
  return begPrefix + "\n" + stack + endPrefix;
}

// callable from gdb
void DumpStack() {
  fprintf(stderr, "%s\n", prettyStack("").c_str());
}

// callable from gdb
void DumpCurUnit(int skip) {
  ActRec* fp = vmfp();
  Offset pc = fp->m_func->unit() ? pcOff() : 0;
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

// callable from gdb
void PrintTCCallerInfo() {
  VMRegAnchor _;

  auto const u = vmfp()->m_func->unit();
  auto const rip = []() -> jit::TCA {
    DECLARE_FRAME_POINTER(reg_fp);
    // NB: We can't directly mutate the register-mapped `reg_fp'.
    for (ActRec* fp = reg_fp; fp; fp = fp->m_sfp) {
      auto const rip = jit::TCA(fp->m_savedRip);
      if (mcg->code().isValidCodeAddress(rip)) return rip;
    }
    return nullptr;
  }();

  fprintf(stderr, "Called from TC address %p\n", rip);
  std::cerr << u->filepath()->data() << ':'
            << u->getLineNumber(u->offsetOf(vmpc())) << '\n';
}

// thread-local cached coverage info
static __thread Unit* s_prev_unit;
static __thread int s_prev_line;

void recordCodeCoverage(PC pc) {
  Unit* unit = vmfp()->m_func->unit();
  assert(unit != nullptr);
  if (unit == SystemLib::s_nativeFuncUnit ||
      unit == SystemLib::s_nativeClassUnit ||
      unit == SystemLib::s_hhas_unit) {
    return;
  }
  int line = unit->getLineNumber(pcOff());
  assert(line != -1);

  if (unit != s_prev_unit || line != s_prev_line) {
    s_prev_unit = unit;
    s_prev_line = line;
    const StringData* filepath = unit->filepath();
    assert(filepath->isStatic());
    TI().m_coverage->Record(filepath->data(), line, line);
  }
}

void resetCoverageCounters() {
  s_prev_line = -1;
  s_prev_unit = nullptr;
}

static inline void
condStackTraceSep(Op opcode) {
  TRACE(3, "%s "
        "========================================"
        "========================================\n",
        opcodeToName(opcode));
}

#define COND_STACKTRACE(pfx)\
  ONTRACE(3, auto stack = prettyStack(pfx);\
          Trace::trace("%s\n", stack.c_str());)

/*
 * iopRetWrapper is used to normalize the calling convention for the iop*
 * functions, since some return void and some return TCA. Any functions that
 * return void are treated as though they returned nullptr.
 */
OPTBLD_INLINE static TCA iopRetWrapper(void(*fn)(PC&), PC& pc) {
  fn(pc);
  return nullptr;
}
OPTBLD_INLINE static TCA iopRetWrapper(TCA(*fn)(PC& pc), PC& pc) {
  return fn(pc);
}

/**
 * The interpOne functions are fat wrappers around the iop* functions, mostly
 * adding a bunch of debug-only logging and stats tracking.
 */
#define O(opcode, imm, push, pop, flags)                                \
  TCA interpOne##opcode(ActRec* fp, TypedValue* sp, Offset pcOff) {     \
  interp_set_regs(fp, sp, pcOff);                                       \
  SKTRACE(5, SrcKey(liveFunc(), vmpc(), liveResumed()), "%40s %p %p\n", \
          "interpOne" #opcode " before (fp,sp)", vmfp(), vmsp());       \
  if (Stats::enableInstrCount()) {                                      \
    Stats::inc(Stats::Instr_Transl##opcode, -1);                        \
    Stats::inc(Stats::Instr_InterpOne##opcode);                         \
  }                                                                     \
  if (Trace::moduleEnabled(Trace::interpOne, 1)) {                      \
    static const StringData* cat = makeStaticString("interpOne");       \
    static const StringData* name = makeStaticString(#opcode);          \
    Stats::incStatGrouped(cat, name, 1);                                \
  }                                                                     \
  if (Trace::moduleEnabled(Trace::ringbuffer)) {                        \
    auto sk = SrcKey{vmfp()->func(), vmpc(), vmfp()->resumed()}.toAtomicInt(); \
    Trace::ringbufferEntry(Trace::RBTypeInterpOne, sk, 0);              \
  }                                                                     \
  INC_TPC(interp_one)                                                   \
  /* Correct for over-counting in TC-stats. */                          \
  Stats::inc(Stats::Instr_TC, -1);                                      \
  condStackTraceSep(Op##opcode);                                        \
  COND_STACKTRACE("op"#opcode" pre:  ");                                \
  PC pc = vmpc();                                                       \
  ONTRACE(1, auto offset = vmfp()->m_func->unit()->offsetOf(pc);        \
          Trace::trace("op"#opcode" offset: %d\n", offset));            \
  assert(peek_op(pc) == Op::opcode);                                    \
  pc += encoded_op_size(Op::opcode);                                    \
  auto const retAddr = iopRetWrapper(iop##opcode, pc);                  \
  vmpc() = pc;                                                          \
  COND_STACKTRACE("op"#opcode" post: ");                                \
  condStackTraceSep(Op##opcode);                                        \
  /*
   * Only set regstate back to dirty if an exception is not
   * propagating.  If an exception is throwing, regstate for this call
   * is actually still correct, and we don't have information in the
   * fixup map for interpOne calls anyway.
   */ \
  tl_regState = VMRegState::DIRTY;                                      \
  return retAddr;                                                       \
}
OPCODES
#undef O

InterpOneFunc interpOneEntryPoints[] = {
#define O(opcode, imm, push, pop, flags) &interpOne##opcode,
OPCODES
#undef O
};

template <bool breakOnCtlFlow>
TCA dispatchImpl() {
  // Unfortunately, MSVC doesn't support computed
  // gotos, so use a switch instead.
  bool collectCoverage = RID().getCoverage();

#ifndef _MSC_VER
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
  if (collectCoverage) {
    optab = optabCover;
  }
  DEBUGGER_ATTACHED_ONLY(optab = optabDbg);
#endif

  bool isCtlFlow = false;
  TCA retAddr = nullptr;
  Op op;

#ifdef _MSC_VER
# define DISPATCH_ACTUAL() goto DispatchSwitch
#else
# define DISPATCH_ACTUAL() goto *optab[size_t(op)]
#endif

#define DISPATCH() do {                                                 \
    if (breakOnCtlFlow && isCtlFlow) {                                  \
      ONTRACE(1,                                                        \
              Trace::trace("dispatch: Halt dispatch(%p)\n",             \
                           vmfp()));                                    \
      return retAddr;                                                   \
    }                                                                   \
    opPC = pc;                                                          \
    op = decode_op(pc);                                                 \
    COND_STACKTRACE("dispatch:                    ");                   \
    FTRACE(1, "dispatch: {}: {}\n", pcOff(),                            \
           instrToString(opPC, vmfp()->m_func->unit()));                \
    DISPATCH_ACTUAL();                                                  \
} while (0)

  ONTRACE(1, Trace::trace("dispatch: Enter dispatch(%p)\n",
          vmfp()));
  PC pc = vmpc();
  PC opPC;
  DISPATCH();

#define OPCODE_DBG_BODY(name, imm, push, pop, flags)          \
  phpDebuggerOpcodeHook(opPC)
#define OPCODE_COVER_BODY(name, imm, push, pop, flags)        \
  if (collectCoverage) {                                      \
    recordCodeCoverage(opPC);                                 \
  }
#define OPCODE_MAIN_BODY(name, imm, push, pop, flags)         \
  {                                                           \
    if (breakOnCtlFlow && Stats::enableInstrCount()) {        \
      Stats::inc(Stats::Instr_InterpBB##name);                \
    }                                                         \
    retAddr = iopRetWrapper(iop##name, pc);                   \
    vmpc() = pc;                                              \
    if (breakOnCtlFlow) {                                     \
      isCtlFlow = instrIsControlFlow(Op::name);               \
    }                                                         \
    if (instrCanHalt(Op::name) && UNLIKELY(!pc)) {            \
      vmfp() = nullptr;                                       \
      /* We returned from the top VM frame in this nesting level. This means
       * m_savedRip in our ActRec must have been callToExit, which should've
       * been returned by jitReturnPost(), whether or not we were called from
       * the TC. We only actually return callToExit to our caller if that
       * caller is dispatchBB(). */                           \
      assert(retAddr == mcg->ustubs().callToExit);    \
      return breakOnCtlFlow ? retAddr : nullptr;              \
    }                                                         \
    assert(isCtlFlow || !retAddr);                            \
    DISPATCH();                                               \
  }

#ifdef _MSC_VER
DispatchSwitch:
  switch (uint8_t(op)) {
#define O(name, imm, push, pop, flags)                        \
    case Op::name: {                                          \
      DEBUGGER_ATTACHED_ONLY(OPCODE_DBG_BODY(name, imm, push, pop, flags)); \
      OPCODE_COVER_BODY(name, imm, push, pop, flags)          \
      OPCODE_MAIN_BODY(name, imm, push, pop, flags)           \
    }
#else
#define O(name, imm, push, pop, flags)                        \
  LabelDbg##name:                                             \
    OPCODE_DBG_BODY(name, imm, push, pop, flags);             \
  LabelCover##name:                                           \
    OPCODE_COVER_BODY(name, imm, push, pop, flags)            \
  Label##name:                                                \
    OPCODE_MAIN_BODY(name, imm, push, pop, flags)
#endif

  OPCODES

#ifdef _MSC_VER
    }
#endif
#undef O
#undef DISPATCH
#undef DISPATCH_ACTUAL
#undef OPCODE_DBG_BODY
#undef OPCODE_COVER_BODY
#undef OPCODE_MAIN_BODY

  assert(retAddr == nullptr);
  return nullptr;
}

static void dispatch() {
  DEBUG_ONLY auto const retAddr = dispatchImpl<false>();
  assert(retAddr == nullptr);
}

// We are about to go back to translated code, check whether we should
// stick with the interpreter. NB: if we've just executed a return
// from pseudomain, then there's no PC and no more code to interpret.
OPTBLD_INLINE TCA switchModeForDebugger(TCA retAddr) {
  if (DEBUGGER_FORCE_INTR && (vmpc() != 0)) {
    if (retAddr) {
      // We just interpreted a bytecode that decided we need to return to an
      // address in the TC rather than interpreting up into our caller. This
      // means it might not be safe to throw an exception right now (see
      // discussion in jitReturnPost). So, resume execution in the TC at a stub
      // that will throw the execution from a safe place.
      FTRACE(1, "Want to throw VMSwitchMode but retAddr = {}, "
             "overriding with throwSwitchMode stub.\n", retAddr);
      return mcg->ustubs().throwSwitchMode;
    } else {
      throw VMSwitchMode();
    }
  }

  return retAddr;
}

TCA dispatchBB() {
  if (Trace::moduleEnabled(Trace::dispatchBB)) {
    auto cat = makeStaticString("dispatchBB");
    auto name = makeStaticString(show(SrcKey(vmfp()->func(), vmpc(),
                                             vmfp()->resumed())));
    Stats::incStatGrouped(cat, name, 1);
  }
  if (Trace::moduleEnabled(Trace::ringbuffer)) {
    auto sk = SrcKey{vmfp()->func(), vmpc(), vmfp()->resumed()}.toAtomicInt();
    Trace::ringbufferEntry(Trace::RBTypeDispatchBB, sk, 0);
  }
  auto retAddr = dispatchImpl<true>();
  return switchModeForDebugger(retAddr);
}

}
