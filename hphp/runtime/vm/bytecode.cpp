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

#include "hphp/runtime/vm/bytecode.h"

#include <algorithm>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cinttypes>

#include <boost/filesystem.hpp>

#include <folly/String.h>
#include <folly/portability/SysMman.h>

#include "hphp/util/debug.h"
#include "hphp/util/numa.h"
#include "hphp/util/portability.h"
#include "hphp/util/ringbuffer.h"
#include "hphp/util/text-util.h"
#include "hphp/util/trace.h"

#include "hphp/compiler/builtin_symbols.h"

#include "hphp/system/systemlib.h"

#include "hphp/runtime/base/apc-stats.h"
#include "hphp/runtime/base/apc-typed-value.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/code-coverage.h"
#include "hphp/runtime/base/collections.h"
#include "hphp/runtime/base/container-functions.h"
#include "hphp/runtime/base/enum-util.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/externals.h"
#include "hphp/runtime/base/hhprof.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/set-array.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/repo-auth-type-codec.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/stat-cache.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/base/strings.h"
#include "hphp/runtime/base/type-structure.h"
#include "hphp/runtime/base/type-structure-helpers.h"
#include "hphp/runtime/base/tv-arith.h"
#include "hphp/runtime/base/tv-comparisons.h"
#include "hphp/runtime/base/tv-conversions.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/base/tv-type.h"
#include "hphp/runtime/base/unit-cache.h"

#include "hphp/runtime/ext/array/ext_array.h"
#include "hphp/runtime/ext/asio/ext_await-all-wait-handle.h"
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
#include "hphp/runtime/ext/std/ext_std_variable.h"
#include "hphp/runtime/ext/string/ext_string.h"
#include "hphp/runtime/ext/hash/hash_murmur.h"
#include "hphp/runtime/ext/json/JSON_parser.h"

#include "hphp/runtime/server/rpc-request-handler.h"
#include "hphp/runtime/server/source-root-info.h"

#include "hphp/runtime/vm/act-rec-defs.h"
#include "hphp/runtime/vm/act-rec.h"
#include "hphp/runtime/vm/debug/debug.h"
#include "hphp/runtime/vm/debugger-hook.h"
#include "hphp/runtime/vm/event-hook.h"
#include "hphp/runtime/vm/globals-array.h"
#include "hphp/runtime/vm/hh-utils.h"
#include "hphp/runtime/vm/hhbc-codec.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/interp-helpers.h"
#include "hphp/runtime/vm/member-operations.h"
#include "hphp/runtime/vm/method-lookup.h"
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
#include "hphp/runtime/vm/workload-stats.h"

#include "hphp/runtime/vm/jit/code-cache.h"
#include "hphp/runtime/vm/jit/debugger.h"
#include "hphp/runtime/vm/jit/enter-tc.h"
#include "hphp/runtime/vm/jit/perf-counters.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/translator-runtime.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/unwind-itanium.h"


namespace HPHP {

TRACE_SET_MOD(bcinterp);

// RepoAuthoritative has been raptured out of runtime_option.cpp. It needs
// to be closer to other bytecode.cpp data.
bool RuntimeOption::RepoAuthoritative = false;

using jit::TCA;

// GCC 4.8 has some real problems with all the inlining in this file, so don't
// go overboard with that version.
#if DEBUG || ((__GNUC__ == 4) && (__GNUC_MINOR__ == 8))
#define OPTBLD_INLINE
#define OPTBLD_FLT_INLINE
#else
#define OPTBLD_INLINE       ALWAYS_INLINE
#define OPTBLD_FLT_INLINE   INLINE_FLATTEN
#endif

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
  frame_free_locals_inl_no_hook(fp, fp->func()->numLocals());
}

const StaticString s___call("__call");
const StaticString s___callStatic("__callStatic");
const StaticString s_allows_unknown_fields("allows_unknown_fields");
const StaticString s_classname("classname");
const StaticString s_elem_types("elem_types");
const StaticString s_fields("fields");
const StaticString s_file("file");
const StaticString s_kind("kind");
const StaticString s_line("line");
const StaticString s_nullable("nullable");
const StaticString s_optional_shape_field("optional_shape_field");
const StaticString s_construct("__construct");

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
inline const char* prettytype(MOpMode) { return "MOpMode"; }
inline const char* prettytype(QueryMOp) { return "QueryMOp"; }
inline const char* prettytype(FPassHint) { return "FPassHint"; }
inline const char* prettytype(CudOp) { return "CudOp"; }
inline const char* prettytype(ContCheckOp) { return "ContCheckOp"; }
inline const char* prettytype(SpecialClsRef) { return "SpecialClsRef"; }
inline const char* prettytype(CollectionType) { return "CollectionType"; }

// load a T value from *pc without incrementing
template<class T> T peek(PC pc) {
  T v;
  std::memcpy(&v, pc, sizeof v);
  TRACE(2, "decode:     Immediate %s %" PRIi64"\n", prettytype(v), int64_t(v));
  return v;
}

template<class T> T decode(PC& pc) {
  auto v = peek<T>(pc);
  pc += sizeof(T);
  return v;
}

inline const StringData* decode_litstr(PC& pc) {
  auto id = decode<Id>(pc);
  return liveUnit()->lookupLitstrId(id);
}

inline const ArrayData* decode_litarr(PC& pc) {
  return liveUnit()->lookupArrayId(decode<Id>(pc));
}

ALWAYS_INLINE Offset decode_ba(PC& pc) {
  return decode<Offset>(pc);
}

// find the AR for the current FPI region using func metadata
static inline ActRec* arFromInstr(PC pc) {
  const ActRec* fp = vmfp();
  auto const func = fp->m_func;
  if (fp->resumed()) {
    fp = reinterpret_cast<const ActRec*>(Stack::resumableStackBase(fp) +
                                         func->numSlotsInFrame());
  }

  return arAtOffset(fp, -instrFpToArDelta(func, pc));
}

// Find the AR for the current FPI region by indexing from sp
static inline ActRec* arFromSp(int32_t n) {
  auto ar = reinterpret_cast<ActRec*>(vmStack().top() + n);
  assertx(ar == arFromInstr(vmpc()));
  return ar;
}

ALWAYS_INLINE MOpMode fpass_mode(ActRec* ar, int paramId) {
  assertx(paramId < ar->numArgs());
  return ar->m_func->byRef(paramId) ? MOpMode::Define : MOpMode::Warn;
}

namespace {

// wrapper for local variable LA operand
struct local_var {
  TypedValue* ptr;
  int32_t index;
  TypedValue* operator->() const { return ptr; }
  TypedValue& operator*() const { return *ptr; }
};

// wrapper for class-ref slot CA(R|W) operand
struct clsref_slot {
  LowPtr<Class>* ptr;
  uint32_t index;

  Class* take() const {
    auto ret = *ptr;
    if (debug) {
      ret->validate();
      memset(ptr, kTrashClsRef, sizeof(*ptr));
    }
    return ret.get();
  }

  void put(Class* cls) { *ptr = cls; }
};

// wrapper to handle unaligned access to variadic immediates
template<class T> struct imm_array {
  uint32_t const size;
  PC const ptr;

  explicit imm_array(uint32_t size, PC pc)
    : size{size}
    , ptr{pc}
  {}

  T operator[](uint32_t i) const {
    T e;
    memcpy(&e, ptr + i * sizeof(T), sizeof(T));
    return e;
  }
};

template<> struct imm_array<bool> {
  uint32_t const size;
  PC const ptr;

  explicit imm_array(uint32_t size, PC pc)
    : size{size}
    , ptr{pc}
  {}

  bool operator[](uint32_t i) const {
    return ptr[i / 8] & (1 << (i % 8));
  }
};

}

ALWAYS_INLINE local_var decode_local(PC& pc) {
  auto la = decode_iva(pc);
  assertx(la < vmfp()->m_func->numLocals());
  return local_var{frame_local(vmfp(), la), safe_cast<int32_t>(la)};
}

ALWAYS_INLINE Iter* decode_iter(PC& pc) {
  auto ia = decode_iva(pc);
  return frame_iter(vmfp(), ia);
}

ALWAYS_INLINE clsref_slot decode_clsref_slot(PC& pc) {
  uint32_t ca = decode_iva(pc);
  assertx(ca < vmfp()->m_func->numClsRefSlots());
  return clsref_slot{frame_clsref_slot(vmfp(), ca), ca};
}

template<typename T>
OPTBLD_INLINE imm_array<T> decode_imm_array(PC& pc) {
  auto const size = decode_iva(pc);
  auto const arr_pc = pc;
  pc += size * sizeof(T);
  return imm_array<T>{size, arr_pc};
}

template<>
OPTBLD_INLINE imm_array<bool> decode_imm_array(PC& pc) {
  auto const size = decode_iva(pc);
  auto const arr_pc = pc;
  pc += (size + 7) / 8;
  return imm_array<bool>{size, arr_pc};
}

OPTBLD_INLINE IterTable decode_iter_table(PC& pc) {
  return iterTableFromStream(pc);
}

OPTBLD_INLINE RepoAuthType decode_rat(PC& pc) {
  if (debug) return decodeRAT(liveUnit(), pc);

  pc += encodedRATSize(pc);
  return RepoAuthType{};
}

//=============================================================================
// Miscellaneous helpers.

static inline Class* frameStaticClass(ActRec* fp) {
  if (!fp->func()->cls()) return nullptr;
  if (fp->hasThis()) {
    return fp->getThis()->getVMClass();
  }
  return fp->getClass();
}

//=============================================================================
// VarEnv.

const StaticString s_GLOBALS("GLOBALS");

void VarEnv::createGlobal() {
  assertx(!g_context->m_globalVarEnv);
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
    new (tl_heap->objMalloc(sizeof(GlobalsArray))) GlobalsArray(&m_nvTable)
  );
  m_nvTable.set(s_GLOBALS.get(), globals_var.asTypedValue());
}

VarEnv::VarEnv(ActRec* fp, ExtraArgs* eArgs)
  : m_nvTable(fp)
  , m_extraArgs(eArgs)
  , m_depth(1)
  , m_global(false)
{
  assertx(fp->func()->attrs() & AttrMayUseVV);
  TRACE(3, "Creating lazily attached VarEnv %p on stack\n", this);
}

VarEnv::VarEnv(const VarEnv* varEnv, ActRec* fp)
  : m_nvTable(varEnv->m_nvTable, fp)
  , m_extraArgs(varEnv->m_extraArgs ? varEnv->m_extraArgs->clone(fp) : nullptr)
  , m_depth(1)
  , m_global(false)
{
  assertx(varEnv->m_depth == 1);
  assertx(!varEnv->m_global);
  assertx(fp->func()->attrs() & AttrMayUseVV);

  TRACE(3, "Cloning VarEnv %p to %p\n", varEnv, this);
}

VarEnv::~VarEnv() {
  TRACE(3, "Destroying VarEnv %p [%s]\n",
           this,
           isGlobalScope() ? "global scope" : "local scope");
  assertx(isGlobalScope() == (g_context->m_globalVarEnv == this));

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
  assertx(newFP);
  if (oldFP == nullptr) {
    assertx(isGlobalScope() && m_depth == 0);
  } else {
    assertx(m_depth >= 1);
    assertx(g_context->getPrevVMStateSkipFrame(newFP) == oldFP);
    if (debug) {
      auto prev = newFP;
      while (true) {
        prev = g_context->getPrevVMState(prev);
        if (prev == oldFP) break;
        assertx(!(prev->m_func->attrs() & AttrMayUseVV) || !prev->hasVarEnv());
      }
    }
    m_nvTable.detach(oldFP);
  }

  assertx(newFP->func()->attrs() & AttrMayUseVV);
  m_nvTable.attach(newFP);
  m_depth++;
}

void VarEnv::exitFP(ActRec* fp) {
  TRACE(3, "Detaching VarEnv %p [%s] @%p\n",
           this,
           isGlobalScope() ? "global scope" : "local scope",
           fp);
  assertx(fp);
  assertx(m_depth > 0);

  m_depth--;
  m_nvTable.detach(fp);

  if (m_depth == 0) {
    if (m_extraArgs) {
      assertx(!isGlobalScope());
      const auto numExtra = fp->numArgs() - fp->m_func->numNonVariadicParams();
      ExtraArgs::deallocate(m_extraArgs, numExtra);
    }

    // don't free global VarEnv
    if (!isGlobalScope()) {
      req::destroy_raw(this);
    }
  } else {
    while (true) {
      auto const prevFP = g_context->getPrevVMState(fp);
      if (prevFP->func()->attrs() & AttrMayUseVV &&
          prevFP->m_varEnv == this) {
        m_nvTable.attach(prevFP);
        break;
      }
      fp = prevFP;
    }
  }
}

void VarEnv::set(const StringData* name, const TypedValue* tv) {
  m_nvTable.set(name, tv);
}

void VarEnv::bind(const StringData* name, TypedValue* tv) {
  m_nvTable.bind(name, tv);
}

void VarEnv::setWithRef(const StringData* name, TypedValue* tv) {
  if (isRefType(tv->m_type)) {
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
    assertx(sorted == ret.get() || sorted->hasExactlyOneRef());
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
  assertx(nargs > 0);
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
  assertx(nargs > 0);
  for (unsigned i = 0; i < nargs; ++i) {
    tvDecRefGen(ea->m_extraArgs + i);
  }
  deallocateRaw(ea);
}

void ExtraArgs::deallocate(ActRec* ar) {
  const int numExtra = ar->numArgs() - ar->m_func->numNonVariadicParams();
  deallocate(ar->getExtraArgs(), numExtra);
}

void ExtraArgs::deallocateRaw(ExtraArgs* ea) {
  ea->~ExtraArgs();
  req::free(ea);
}

ExtraArgs* ExtraArgs::clone(ActRec* ar) const {
  const int numExtra = ar->numArgs() - ar->m_func->numParams();
  auto ret = allocateUninit(numExtra);
  for (int i = 0; i < numExtra; ++i) {
    tvDupWithRef(m_extraArgs[i], ret->m_extraArgs[i]);
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
struct StackElms {
  ~StackElms() { free(m_elms); }
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
      size_t algnSz = RuntimeOption::EvalVMStackElms * sizeof(TypedValue);
      madvise(m_elms, algnSz, MADV_DONTNEED);
    }
  }
private:
  TypedValue* m_elms{nullptr};
};
THREAD_LOCAL(StackElms, t_se);

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
  assertx(!(rds::header()->stackLimitAndSurprise.load() & kSurpriseFlagMask));

  // Because of the surprise page at the bottom of the stack we lose an
  // additional 256 elements which must be taken into account when checking for
  // overflow.
  UNUSED size_t maxelms =
    RuntimeOption::EvalVMStackElms - sSurprisePageSize / sizeof(TypedValue);
  assertx(!wouldOverflow(maxelms - 1));
  assertx(wouldOverflow(maxelms));
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

  tl_heap->flush();

  if (!t_se.isNull()) {
    t_se->flush();
  }
  rds::flush();
  json_parser_flush_caches();

  always_assert(tl_heap->empty());
}

static std::string toStringElm(const TypedValue* tv) {
  std::ostringstream os;

  if (!isRealType(tv->m_type)) {
    os << " ??? type " << static_cast<data_type_t>(tv->m_type) << "\n";
    return os.str();
  }
  if (isRefcountedType(tv->m_type) &&
      !tv->m_data.pcnt->checkCount()) {
    // OK in the invoking frame when running a destructor.
    os << " ??? inner_count " << tvGetCount(*tv) << " ";
    return os.str();
  }

  auto print_count = [&] {
    if (tv->m_data.pcnt->isStatic()) {
      os << ":c(static)";
    } else if (tv->m_data.pcnt->isUncounted()) {
      os << ":c(uncounted)";
    } else {
      os << ":c(" << tvGetCount(*tv) << ")";
    }
  };

  switch (tv->m_type) {
  case KindOfRef:
    os << "V:(";
    os << "@" << tv->m_data.pref;
    os << toStringElm(tv->m_data.pref->tv());
    os << ")";
    return os.str();
  case KindOfUninit:
  case KindOfNull:
  case KindOfBoolean:
  case KindOfInt64:
  case KindOfDouble:
  case KindOfPersistentString:
  case KindOfString:
  case KindOfPersistentVec:
  case KindOfVec:
  case KindOfPersistentDict:
  case KindOfDict:
  case KindOfPersistentKeyset:
  case KindOfKeyset:
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
    case KindOfPersistentVec:
    case KindOfVec:
      assertx(tv->m_data.parr->isVecArray());
      assertx(tv->m_data.parr->checkCount());
      os << tv->m_data.parr;
      print_count();
      os << ":Vec";
      continue;
    case KindOfPersistentDict:
    case KindOfDict:
      assertx(tv->m_data.parr->isDict());
      assertx(tv->m_data.parr->checkCount());
      os << tv->m_data.parr;
      print_count();
      os << ":Dict";
      continue;
    case KindOfPersistentKeyset:
    case KindOfKeyset:
      assertx(tv->m_data.parr->isKeyset());
      assertx(tv->m_data.parr->checkCount());
      os << tv->m_data.parr;
      print_count();
      os << ":Keyset";
      continue;
    case KindOfPersistentArray:
    case KindOfArray:
      assertx(tv->m_data.parr->isPHPArray());
      assertx(tv->m_data.parr->checkCount());
      os << tv->m_data.parr;
      print_count();
      os << ":Array";
      continue;
    case KindOfObject:
      assertx(tv->m_data.pobj->checkCount());
      os << tv->m_data.pobj;
      print_count();
      os << ":Object("
         << tv->m_data.pobj->getClassName().get()->data()
         << ")";
      continue;
    case KindOfResource:
      assertx(tv->m_data.pres->checkCount());
      os << tv->m_data.pres;
      print_count();
      os << ":Resource("
         << tv->m_data.pres->data()->o_getClassName().get()->data()
         << ")";
      continue;
    case KindOfRef:
      break;
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
  assertx(false);
  return "I:?";
}

/*
 * Return true if Offset o is inside the protected region of a fault
 * funclet for iterId, otherwise false. itRef will be set to true if
 * the iterator was initialized with MIterInit*, false if the iterator
 * was initialized with IterInit*.
 */
static bool checkIterScope(const Func* f, Offset o, Id iterId, bool& itRef) {
  assertx(o >= f->base() && o < f->past());
  for (auto const& eh : f->ehtab()) {
    if (eh.m_base <= o && o < eh.m_past &&
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
  assertx(fp);

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
  assertx(func);
  func->validate();
  std::string funcName(func->fullName()->data());
  os << "{func:" << funcName
     << ",soff:" << fp->m_soff
     << ",this:0x"
     << std::hex << (func->cls() && fp->hasThis() ? fp->getThis() : nullptr)
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

  // Ideally we'd like to display the contents of the class-ref slots here, but
  // we have no metadata to tell us which ones are currently occupied and valid.

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
  assertx(!fp->resumed());
  return (TypedValue*)fp - fp->func()->numSlotsInFrame();
}

TypedValue* Stack::resumableStackBase(const ActRec* fp) {
  assertx(fp->resumed());
  auto sfp = fp->sfp();
  if (sfp) {
    // The non-reentrant case occurs when a non-async or async generator is
    // resumed via ContEnter or ContRaise opcode. These opcodes leave a single
    // value on the stack that becomes part of the generator's stack. So we
    // find the caller's FP, compensate for its locals and iterators, and then
    // we've found the base of the generator's stack.
    assertx(fp->func()->isGenerator());

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
    assertx(fp->func()->isAsync());
    return g_context.getNoCheck()->m_nestedVMs.back().sp;
  }
}

Array getDefinedVariables(const ActRec* fp) {
  if (UNLIKELY(fp == nullptr)) return empty_array();

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
  assertx(func);

  // the last (variadic) param is included in numParams (since it has a
  // name), but the arg in that slot should be included as the first
  // element of the variadic array
  const auto numArgs = ar->numArgs();
  const auto numVarArgs = numArgs - func->numNonVariadicParams();
  assertx(numVarArgs > 0);

  const auto takesVariadicParam = func->hasVariadicCaptureParam();
  auto& stack = vmStack();
  if (func->attrs() & AttrMayUseVV) {
    auto const tvArgs = reinterpret_cast<TypedValue*>(ar) - numArgs;
    ar->setExtraArgs(ExtraArgs::allocateCopy(tvArgs, numVarArgs));
    if (takesVariadicParam) {
      try {
        VArrayInit ai{numVarArgs};
        for (uint32_t i = 0; i < numVarArgs; ++i) {
          ai.appendWithRef(*(tvArgs + numVarArgs - 1 - i));
        }
        // Remove them from the stack
        stack.ndiscard(numVarArgs);
        if (RuntimeOption::EvalHackArrDVArrs) {
          stack.pushVecNoRc(ai.create());
        } else {
          stack.pushArrayNoRc(ai.create());
        }
        // Before, for each arg: refcount = n + 1 (stack)
        // After, for each arg: refcount = n + 2 (ExtraArgs, varArgsArray)
      } catch (...) {
        ExtraArgs::deallocateRaw(ar->getExtraArgs());
        ar->resetExtraArgs();
        throw;
      }
    } else {
      // Discard the arguments from the stack; they were all moved
      // into the extra args so we don't decref.
      stack.ndiscard(numVarArgs);
    }
    // leave ar->numArgs reflecting the actual number of args passed
  } else {
    assertx(takesVariadicParam); // called only if extra args are used
    auto tvArgs = reinterpret_cast<TypedValue*>(ar) - numArgs + numVarArgs - 1;
    VArrayInit ai{numVarArgs};
    for (uint32_t i = 0; i < numVarArgs; ++i) {
      ai.appendWithRef(*(tvArgs--));
    }
    // Discard the arguments from the stack
    for (uint32_t i = 0; i < numVarArgs; ++i) stack.popTV();
    if (RuntimeOption::EvalHackArrDVArrs) {
      stack.pushVecNoRc(ai.create());
    } else {
      stack.pushArrayNoRc(ai.create());
    }
    assertx(func->numParams() == (numArgs - numVarArgs + 1));
    ar->setNumArgs(func->numParams());
  }
}

static void shuffleMagicArgs(ActRec* ar) {
  assertx(ar->magicDispatch());

  // We need to put this where the first argument is
  auto const invName = ar->clearMagicDispatch();
  int const nargs = ar->numArgs();

  // We need to make an array containing all the arguments passed by
  // the caller and put it where the second argument is.
  auto argArray = Array::attach(
    [&]{
      auto const args = reinterpret_cast<TypedValue*>(ar) - nargs;
      if (RuntimeOption::EvalHackArrDVArrs) {
        return nargs
          ? PackedArray::MakeVec(nargs, args)
          : staticEmptyVecArray();
      }
      return nargs ? PackedArray::MakeVArray(nargs, args) : staticEmptyVArray();
    }()
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
  if (RuntimeOption::EvalHackArrDVArrs) {
    stack.pushVecNoRc(argArray.detach());
  } else {
    stack.pushArrayNoRc(argArray.detach());
  }

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
  assertx(stack.top() + (numParams != nullptr ? (*numParams) :
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
  assertx(ar != nullptr && ar->magicDispatch());
  assertx(!cellIsNull(&args));
  assertx(nregular >= 0);
  assertx((stack.top() + nregular) == (void*) ar);
  assertx(isContainer(args));
  DEBUG_ONLY const Func* f = ar->m_func;
  assertx(f &&
         (f->name()->isame(s___call.get()) ||
          f->name()->isame(s___callStatic.get())));

  // We'll need to make this the first argument
  auto const invName = ar->clearMagicDispatch();

  auto nargs = getContainerSize(args);

  if (UNLIKELY(0 == nargs)) {
    // We need to make an array containing all the arguments passed by
    // the caller and put it where the second argument is.
    auto argArray = Array::attach(
      [&]{
        auto const args = reinterpret_cast<TypedValue*>(ar) - nregular;
        if (RuntimeOption::EvalHackArrDVArrs) {
          return nregular
            ? PackedArray::MakeVec(nregular, args)
            : staticEmptyVecArray();
        }
        return nregular
          ? PackedArray::MakeVArray(nregular, args)
          : staticEmptyVArray();
      }()
    );

    // Remove the arguments from the stack; they were moved into the
    // array so we don't need to decref.
    stack.ndiscard(nregular);

    // Move invName to where the first argument belongs, no need
    // to incRef/decRef since we are transferring ownership
    assertx(stack.top() == (void*) ar);
    stack.pushStringNoRc(invName);

    // Move argArray to where the second argument belongs. We've already
    // incReffed the array above so we don't need to do it here.
    if (RuntimeOption::EvalHackArrDVArrs) {
      stack.pushVecNoRc(argArray.detach());
    } else {
      stack.pushArrayNoRc(argArray.detach());
    }
  } else {
    if (nregular == 0 &&
        !RuntimeOption::EvalHackArrDVArrs &&
        isArrayType(args.m_type) &&
        args.m_data.parr->isVArray()) {
      assertx(stack.top() == (void*) ar);
      stack.pushStringNoRc(invName);
      stack.pushArray(args.m_data.parr);
    } else if (nregular == 0 &&
               RuntimeOption::EvalHackArrDVArrs &&
               isVecType(args.m_type)) {
      assertx(stack.top() == (void*) ar);
      stack.pushStringNoRc(invName);
      stack.pushVec(args.m_data.parr);
    } else {
      VArrayInit ai(nargs + nregular);
      // The arguments are pushed in order, so we should refer them by
      // index instead of taking the top, that would lead to reverse order.
      for (int i = nregular - 1; i >= 0; --i) {
        // appendWithRef bumps the refcount and splits if necessary,
        // to compensate for the upcoming pop from the stack
        ai.appendWithRef(tvAsVariant(stack.indTV(i)));
      }
      for (int i = 0; i < nregular; ++i) {
        stack.popTV();
      }
      assertx(stack.top() == (void*) ar);
      stack.pushStringNoRc(invName);
      for (ArrayIter iter(args); iter; ++iter) {
        ai.appendWithRef(iter.secondValPlus());
      }
      if (RuntimeOption::EvalHackArrDVArrs) {
        stack.pushVecNoRc(ai.create());
      } else {
        stack.pushArrayNoRc(ai.create());
      }
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
                      int nregular, TypedValue* retval, bool checkRefAnnot) {
  assertx(!cellIsNull(&args));
  assertx(nregular >= 0);
  assertx((stack.top() + nregular) == (void*) ar);
  const Func* const f = ar->m_func;
  assertx(f);

  assertx(isContainer(args));
  int const nargs = nregular + getContainerSize(args);
  if (UNLIKELY(ar->magicDispatch())) {
    shuffleMagicArrayArgs(ar, args, stack, nregular);
    return true;
  }

#define WRAP(e)                                                        \
  try {                                                                \
    e;                                                                 \
  } catch (...) {                                                      \
    /* If the user error handler throws an exception, discard the
     * uninitialized value(s) at the top of the eval stack so that the
     * unwinder doesn't choke */                                       \
    stack.discard();                                                   \
    if (retval) { tvWriteNull(*retval); }                              \
    throw;                                                             \
  }

  auto const calledFromHH = [] {
    if (RuntimeOption::EnableHipHopSyntax) return true;

    auto fp = vmfp();
    while (fp && fp->func()->isBuiltin()) {
      fp = g_context->getPrevVMState(fp);
    }

    return fp && fp->func()->unit()->isHHFile();
  };

  int const nparams = f->numNonVariadicParams();
  int nextra_regular = std::max(nregular - nparams, 0);
  ArrayIter iter(args);
  if (LIKELY(nextra_regular == 0)) {
    for (int i = nregular; iter && (i < nparams); ++i, ++iter) {
      auto const from = iter.secondValPlus();
      TypedValue* to = stack.allocTV();
      if (LIKELY(!f->byRef(i))) {
        cellDup(tvToCell(from), *to);
      } else if (LIKELY(isRefType(from.m_type) &&
                        from.m_data.pref->hasMultipleRefs())) {
        if (checkRefAnnot && calledFromHH()) {
          WRAP(raiseParamRefMismatchForFunc(f, i));
        }
        refDup(from, *to);
      } else {
        if (checkRefAnnot && calledFromHH()) {
          WRAP(raiseParamRefMismatchForFunc(f, i));
        }
        cellDup(tvToCell(from), *to);
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

#undef WRAP

  // there are "extra" arguments; passed as standard arguments prior to the
  // ... unpack operator and/or still remaining in argArray
  assertx(nargs > nparams);
  assertx(nextra_regular > 0 || !!iter);
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
    VArrayInit ai(extra);
    if (UNLIKELY(nextra_regular > 0)) {
      // The arguments are pushed in order, so we should refer them by
      // index instead of taking the top, that would lead to reverse order.
      for (int i = nextra_regular - 1; i >= 0; --i) {
        TypedValue* to = extraArgs->getExtraArg(nextra_regular - i - 1);
        const TypedValue* from = stack.indTV(i);
        if (isRefType(from->m_type) && from->m_data.pref->isReferenced()) {
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
      auto const from = iter.secondValPlus();
      tvDupWithRef(from, *to);
      if (hasVarParam) {
        ai.appendWithRef(from);
      }
    }
    assertx(!iter); // iter should now be exhausted
    if (hasVarParam) {
      auto const ad = ai.create();
      assertx(ad->hasExactlyOneRef());
      if (RuntimeOption::EvalHackArrDVArrs) {
        stack.pushVecNoRc(ad);
      } else {
        stack.pushArrayNoRc(ad);
      }
    }
    ar->setNumArgs(nargs);
    ar->setExtraArgs(extraArgs);
  } else {
    assertx(hasVarParam);
    if (nparams == nregular &&
        !RuntimeOption::EvalHackArrDVArrs &&
        isArrayType(args.m_type) &&
        args.m_data.parr->isVArray()) {
      stack.pushArray(args.m_data.parr);
    } else if (nparams == nregular &&
               RuntimeOption::EvalHackArrDVArrs &&
               isVecType(args.m_type)) {
      stack.pushVec(args.m_data.parr);
    } else {
      VArrayInit ai(extra);
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
        ai.appendWithRef(iter.secondValPlus());
      }
      assertx(!iter); // iter should now be exhausted
      auto const ad = ai.create();
      assertx(ad->hasExactlyOneRef());
      if (RuntimeOption::EvalHackArrDVArrs) {
        stack.pushVecNoRc(ad);
      } else {
        stack.pushArrayNoRc(ad);
      }
    }
    ar->setNumArgs(f->numParams());
  }
  return true;
}

static void prepareFuncEntry(ActRec *ar, PC& pc, StackArgsState stk) {
  assertx(!ar->resumed());
  const Func* func = ar->m_func;
  Offset firstDVInitializer = InvalidAbsoluteOffset;
  bool raiseMissingArgumentWarnings = false;
  const int nparams = func->numNonVariadicParams();
  auto& stack = vmStack();

  if (stk == StackArgsState::Trimmed &&
      (ar->func()->attrs() & AttrMayUseVV) &&
      ar->hasExtraArgs()) {
    assertx(nparams < ar->numArgs());
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
        assertx(nargs == func->numParams());
        assertx(((TypedValue*)ar - stack.top()) == func->numParams());
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
        if (RuntimeOption::EvalHackArrDVArrs) {
          stack.pushVecNoRc(staticEmptyVecArray());
        } else {
          stack.pushArrayNoRc(staticEmptyVArray());
        }
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

  pushFrameSlots(func, nlocals);

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
  assertx(enterFnAr);
  assertx(!enterFnAr->resumed());
  Stats::inc(Stats::VMEnter);

  const bool useJit = RID().getJit() && !RID().getJitFolding();
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
    jit::enterTCAtPrologue(enterFnAr, start);
    return;
  }

  if (UNLIKELY(varEnv != nullptr)) {
    enterFnAr->setVarEnv(varEnv);
    assertx(enterFnAr->func()->isPseudoMain());
    pushFrameSlots(enterFnAr->func());
    auto oldFp = vmfp();
    if (UNLIKELY(oldFp && oldFp->skipFrame())) {
      oldFp = g_context->getPrevVMStateSkipFrame(oldFp);
    }
    varEnv->enterFP(oldFp, enterFnAr);
    vmfp() = enterFnAr;
    vmpc() = enterFnAr->func()->getEntry();
  } else {
    prepareFuncEntry(enterFnAr, vmpc(), stk);
  }

  if (!EventHook::FunctionCall(enterFnAr, EventHook::NormalFunc)) return;
  checkStack(vmStack(), enterFnAr->m_func, 0);
  calleeDynamicCallChecks(enterFnAr);
  checkForRequiredCallM(enterFnAr);
  assertx(vmfp()->func()->contains(vmpc()));

  if (useJit) {
    jit::TCA start = enterFnAr->m_func->getFuncBody();
    jit::enterTCAfterPrologue(start);
  } else {
    dispatch();
  }
}

void enterVMAtCurPC() {
  assertx(vmfp());
  assertx(vmpc());
  assertx(vmfp()->func()->contains(vmpc()));
  Stats::inc(Stats::VMEnter);
  if (RID().getJit()) {
    jit::enterTC();
  } else {
    dispatch();
  }
}

/*
 * Helper for function entry, including pseudo-main entry.
 */
void pushFrameSlots(const Func* func, int nparams /*= 0*/) {
  // Push locals.
  for (int i = nparams; i < func->numLocals(); i++) {
    vmStack().pushUninit();
  }
  // Push iterators.
  for (int i = 0; i < func->numIterators(); i++) {
    vmStack().allocI();
  }
  vmStack().allocClsRefSlots(func->numClsRefSlots());
}

void unwindPreventReturnToTC(ActRec* ar) {
  auto const savedRip = reinterpret_cast<jit::TCA>(ar->m_savedRip);
  always_assert_flog(jit::tc::isValidCodeAddress(savedRip),
                     "preventReturnToTC({}): {} isn't in TC",
                     ar, savedRip);

  if (isReturnHelper(savedRip)) return;

  auto& ustubs = jit::tc::ustubs();
  if (ar->resumed()) {
    // async functions use callToExit stub
    assertx(ar->func()->isGenerator());
    ar->setJitReturn(ar->func()->isAsync()
      ? ustubs.asyncGenRetHelper : ustubs.genRetHelper);
  } else {
    ar->setJitReturn(ustubs.retHelper);
  }
}

void debuggerPreventReturnToTC(ActRec* ar) {
  auto const savedRip = reinterpret_cast<jit::TCA>(ar->m_savedRip);
  always_assert_flog(jit::tc::isValidCodeAddress(savedRip),
                     "preventReturnToTC({}): {} isn't in TC",
                     ar, savedRip);

  if (isReturnHelper(savedRip) || isDebuggerReturnHelper(savedRip)) return;

  // We're going to smash the return address. Before we do, save the catch
  // block attached to the call in a side table so the return helpers and
  // unwinder can find it when needed.
  jit::stashDebuggerCatch(ar);

  auto& ustubs = jit::tc::ustubs();
  if (ar->resumed()) {
    // async functions use callToExit stub
    assertx(ar->func()->isGenerator());
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
  assertx(isDebuggerAttached());
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
    assertx(fp->func()->attrs() & AttrMayUseVV);
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
    assertx(func->attrs() & AttrMayUseVV);
    if (!fp->hasVarEnv()) {
      fp->setVarEnv(VarEnv::createLocal(fp));
    }
    val = fp->m_varEnv->lookup(name);
    if (val == nullptr) {
      TypedValue tv;
      tvWriteNull(tv);
      fp->m_varEnv->set(name, &tv);
      val = fp->m_varEnv->lookup(name);
    }
  }
}

static inline void lookup_gbl(ActRec* /*fp*/, StringData*& name,
                              TypedValue* key, TypedValue*& val) {
  name = lookup_name(key);
  assertx(g_context->m_globalVarEnv);
  val = g_context->m_globalVarEnv->lookup(name);
}

static inline void lookupd_gbl(ActRec* /*fp*/, StringData*& name,
                               TypedValue* key, TypedValue*& val) {
  name = lookup_name(key);
  assertx(g_context->m_globalVarEnv);
  VarEnv* varEnv = g_context->m_globalVarEnv;
  val = varEnv->lookup(name);
  if (val == nullptr) {
    TypedValue tv;
    tvWriteNull(tv);
    varEnv->set(name, &tv);
    val = varEnv->lookup(name);
  }
}

static inline void lookup_sprop(ActRec* fp,
                                Class* cls,
                                StringData*& name,
                                TypedValue* key,
                                TypedValue*& val,
                                bool& visible,
                                bool& accessible) {
  name = lookup_name(key);
  auto const ctx = arGetContextClass(fp);

  auto const lookup = cls->getSProp(ctx, name);

  val = lookup.prop;
  visible = lookup.prop != nullptr;
  accessible = lookup.accessible;
}

static inline Class* lookupClsRef(Cell* input) {
  Class* class_ = nullptr;
  if (isStringType(input->m_type)) {
    class_ = Unit::loadClass(input->m_data.pstr);
    if (class_ == nullptr) {
      raise_error(Strings::UNKNOWN_CLASS, input->m_data.pstr->data());
    }
  } else if (input->m_type == KindOfObject) {
    class_ = input->m_data.pobj->getVMClass();
  } else {
    raise_error("Cls: Expected string or object");
  }
  return class_;
}

static UNUSED int innerCount(TypedValue tv) {
  return isRefcountedType(tv.m_type) ? tvGetCount(tv) : -1;
}

static inline tv_lval ratchetRefs(tv_lval result,
                                  TypedValue& tvRef,
                                  TypedValue& tvRef2) {
  TRACE(5, "Ratchet: result %p(k%d c%d), ref %p(k%d c%d) ref2 %p(k%d c%d)\n",
        result.tv_ptr(), static_cast<data_type_t>(result.type()),
        innerCount(*result),
        &tvRef, static_cast<data_type_t>(tvRef.m_type), innerCount(tvRef),
        &tvRef2, static_cast<data_type_t>(tvRef2.m_type), innerCount(tvRef2));
  // Due to complications associated with ArrayAccess, it is possible to acquire
  // a reference as a side effect of vector operation processing. Such a
  // reference must be retained until after the next iteration is complete.
  // Therefore, move the reference from tvRef to tvRef2, so that the reference
  // will be released one iteration later. But only do this if tvRef was used in
  // this iteration, otherwise we may wipe out the last reference to something
  // that we need to stay alive until the next iteration.
  if (tvRef.m_type != KindOfUninit) {
    if (isRefcountedType(tvRef2.m_type)) {
      tvDecRefCountable(&tvRef2);
      TRACE(5, "Ratchet: decref tvref2\n");
      tvWriteUninit(tvRef2);
    }

    memcpy(&tvRef2, &tvRef, sizeof(TypedValue));
    tvWriteUninit(tvRef);
    // Update result to point to relocated reference. This can be done
    // unconditionally here because we maintain the invariant throughout that
    // either tvRef is KindOfUninit, or tvRef contains a valid object that
    // result points to.
    assertx(result.tv_ptr() == &tvRef);
    return tv_lval(&tvRef2);
  }

  assertx(result.tv_ptr() != &tvRef);
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

OPTBLD_INLINE void iopNop() {
}

OPTBLD_INLINE void iopEntryNop() {
}

OPTBLD_INLINE void iopDiscardClsRef(clsref_slot slot) {
  slot.take();
}

OPTBLD_INLINE void iopPopC() {
  vmStack().popC();
}

OPTBLD_INLINE void iopPopV() {
  vmStack().popV();
}

OPTBLD_INLINE void iopPopR() {
  if (!isRefType(vmStack().topTV()->m_type)) {
    vmStack().popC();
  } else {
    vmStack().popV();
  }
}

OPTBLD_INLINE void iopPopU() {
  vmStack().popU();
}

OPTBLD_INLINE void iopPopL(local_var to) {
  assertx(to.index < vmfp()->m_func->numLocals());
  Cell* fr = vmStack().topC();
  if (isRefType(to->m_type) || vmfp()->m_func->isPseudoMain()) {
    // Manipulate the ref-counts as if this was a SetL, PopC pair to preserve
    // destructor ordering.
    tvSet(*fr, *to);
    vmStack().popC();
  } else {
    cellMove(*fr, *to);
    vmStack().discard();
  }
}

OPTBLD_INLINE void iopDup() {
  vmStack().dup();
}

OPTBLD_INLINE void iopBox() {
  vmStack().box();
}

OPTBLD_INLINE void iopUnbox() {
  vmStack().unbox();
}

OPTBLD_INLINE void iopBoxR() {
  TypedValue* tv = vmStack().topTV();
  if (!isRefType(tv->m_type)) {
    tvBox(*tv);
  }
}

OPTBLD_INLINE void iopBoxRNop() {
  assertx(refIsPlausible(*vmStack().topTV()));
}

OPTBLD_INLINE void iopUnboxR() {
  if (isRefType(vmStack().topTV()->m_type)) {
    vmStack().unbox();
  }
}

OPTBLD_INLINE void iopUnboxRNop() {
  assertx(cellIsPlausible(*vmStack().topTV()));
}

OPTBLD_INLINE void iopRGetCNop() {
}

OPTBLD_INLINE void iopCGetCUNop() {
}

OPTBLD_INLINE void iopUGetCUNop() {
}

OPTBLD_INLINE void iopNull() {
  vmStack().pushNull();
}

OPTBLD_INLINE void iopNullUninit() {
  vmStack().pushNullUninit();
}

OPTBLD_INLINE void iopTrue() {
  vmStack().pushBool(true);
}

OPTBLD_INLINE void iopFalse() {
  vmStack().pushBool(false);
}

OPTBLD_INLINE void iopFile() {
  auto s = vmfp()->m_func->unit()->filepath();
  vmStack().pushStaticString(s);
}

OPTBLD_INLINE void iopDir() {
  auto s = vmfp()->m_func->unit()->dirpath();
  vmStack().pushStaticString(s);
}

OPTBLD_INLINE void iopMethod() {
  auto s = vmfp()->m_func->fullName();
  vmStack().pushStaticString(s);
}

OPTBLD_INLINE void iopClsRefName(clsref_slot slot) {
  auto const cls  = slot.take();
  auto const name = cls->name();
  vmStack().pushStaticString(name);
}

OPTBLD_INLINE void iopInt(int64_t imm) {
  vmStack().pushInt(imm);
}

OPTBLD_INLINE void iopDouble(double imm) {
  vmStack().pushDouble(imm);
}

OPTBLD_INLINE void iopString(const StringData* s) {
  vmStack().pushStaticString(s);
}

OPTBLD_INLINE void iopArray(const ArrayData* a) {
  assertx(a->isPHPArray());
  assertx(!RuntimeOption::EvalHackArrDVArrs || a->isNotDVArray());
  vmStack().pushStaticArray(a);
}

OPTBLD_INLINE void iopDict(const ArrayData* a) {
  assertx(a->isDict());
  vmStack().pushStaticDict(a);
}

OPTBLD_INLINE void iopKeyset(const ArrayData* a) {
  assertx(a->isKeyset());
  vmStack().pushStaticKeyset(a);
}

OPTBLD_INLINE void iopVec(const ArrayData* a) {
  assertx(a->isVecArray());
  vmStack().pushStaticVec(a);
}

OPTBLD_INLINE void iopNewArray(uint32_t capacity) {
  if (capacity == 0) {
    vmStack().pushArrayNoRc(staticEmptyArray());
  } else {
    vmStack().pushArrayNoRc(PackedArray::MakeReserve(capacity));
  }
}

OPTBLD_INLINE void iopNewMixedArray(uint32_t capacity) {
  if (capacity == 0) {
    vmStack().pushArrayNoRc(staticEmptyArray());
  } else {
    vmStack().pushArrayNoRc(MixedArray::MakeReserveMixed(capacity));
  }
}

OPTBLD_INLINE void iopNewDictArray(uint32_t capacity) {
  if (capacity == 0) {
    vmStack().pushDictNoRc(staticEmptyDictArray());
  } else {
    vmStack().pushDictNoRc(MixedArray::MakeReserveDict(capacity));
  }
}

OPTBLD_INLINE
void iopNewLikeArrayL(local_var fr, uint32_t capacity) {
  ArrayData* arr;
  if (LIKELY(isArrayType(fr->m_type))) {
    arr = MixedArray::MakeReserveLike(fr->m_data.parr, capacity);
  } else {
    if (capacity == 0) capacity = PackedArray::SmallSize;
    arr = PackedArray::MakeReserve(capacity);
  }
  vmStack().pushArrayNoRc(arr);
}

OPTBLD_INLINE void iopNewPackedArray(uint32_t n) {
  // This constructor moves values, no inc/decref is necessary.
  auto* a = PackedArray::MakePacked(n, vmStack().topC());
  vmStack().ndiscard(n);
  vmStack().pushArrayNoRc(a);
}

namespace {

template <typename F>
ArrayData* newStructArrayImpl(imm_array<int32_t> ids, F f) {
  auto const n = ids.size;
  assertx(n > 0 && n <= ArrayData::MaxElemsOnStack);
  req::vector<const StringData*> names;
  names.reserve(n);
  auto unit = vmfp()->m_func->unit();
  for (size_t i = 0; i < n; ++i) {
    auto name = unit->lookupLitstrId(ids[i]);
    names.push_back(name);
  }

  // This constructor moves values, no inc/decref is necessary.
  auto const a = f(n, names.data(), vmStack().topC())->asArrayData();
  vmStack().ndiscard(n);
  return a;
}

}

OPTBLD_INLINE void iopNewStructArray(imm_array<int32_t> ids) {
  auto const a = newStructArrayImpl(ids, MixedArray::MakeStruct);
  vmStack().pushArrayNoRc(a);
}

OPTBLD_INLINE void iopNewStructDArray(imm_array<int32_t> ids) {
  assertx(!RuntimeOption::EvalHackArrDVArrs);
  auto const a = newStructArrayImpl(ids, MixedArray::MakeStructDArray);
  vmStack().pushArrayNoRc(a);
}

OPTBLD_INLINE void iopNewStructDict(imm_array<int32_t> ids) {
  auto const a = newStructArrayImpl(ids, MixedArray::MakeStructDict);
  vmStack().pushDictNoRc(a);
}

OPTBLD_INLINE void iopNewVecArray(uint32_t n) {
  // This constructor moves values, no inc/decref is necessary.
  auto* a = PackedArray::MakeVec(n, vmStack().topC());
  vmStack().ndiscard(n);
  vmStack().pushVecNoRc(a);
}

OPTBLD_INLINE void iopNewKeysetArray(uint32_t n) {
  // This constructor moves values, no inc/decref is necessary.
  auto* a = SetArray::MakeSet(n, vmStack().topC());
  vmStack().ndiscard(n);
  vmStack().pushKeysetNoRc(a);
}

OPTBLD_INLINE void iopNewVArray(uint32_t n) {
  assertx(!RuntimeOption::EvalHackArrDVArrs);
  // This constructor moves values, no inc/decref is necessary.
  auto a = PackedArray::MakeVArray(n, vmStack().topC());
  vmStack().ndiscard(n);
  vmStack().pushArrayNoRc(a);
}

OPTBLD_INLINE void iopNewDArray(uint32_t capacity) {
  assertx(!RuntimeOption::EvalHackArrDVArrs);
  if (capacity == 0) {
    vmStack().pushArrayNoRc(staticEmptyDArray());
  } else {
    vmStack().pushArrayNoRc(MixedArray::MakeReserveDArray(capacity));
  }
}

OPTBLD_INLINE void iopAddElemC() {
  Cell* c1 = vmStack().topC();
  Cell* c2 = vmStack().indC(1);
  Cell* c3 = vmStack().indC(2);
  if (!isArrayType(c3->m_type) && !isDictType(c3->m_type)) {
    raise_error("AddElemC: $3 must be an array or dict");
  }
  if (c2->m_type == KindOfInt64) {
    cellAsVariant(*c3).asArrRef().set(c2->m_data.num, tvAsCVarRef(c1));
  } else {
    cellAsVariant(*c3).asArrRef().set(tvAsCVarRef(c2), tvAsCVarRef(c1));
  }
  vmStack().popC();
  vmStack().popC();
}

OPTBLD_INLINE void iopAddElemV() {
  Ref* r1 = vmStack().topV();
  Cell* c2 = vmStack().indC(1);
  Cell* c3 = vmStack().indC(2);
  if (!isArrayType(c3->m_type) && !isDictType(c3->m_type)) {
    raise_error("AddElemV: $3 must be an array or dict");
  }
  if (c2->m_type == KindOfInt64) {
    cellAsVariant(*c3).asArrRef().setRef(c2->m_data.num, tvAsVariant(r1));
  } else {
    cellAsVariant(*c3).asArrRef().setRef(tvAsCVarRef(c2), tvAsVariant(r1));
  }
  vmStack().popV();
  vmStack().popC();
}

OPTBLD_INLINE void iopAddNewElemC() {
  Cell* c1 = vmStack().topC();
  Cell* c2 = vmStack().indC(1);
  if (isArrayType(c2->m_type)) {
    cellAsVariant(*c2).asArrRef().append(tvAsCVarRef(c1));
  } else if (isVecType(c2->m_type)) {
    auto in = c2->m_data.parr;
    auto out = PackedArray::AppendVec(in, *c1, in->cowCheck());
    if (in != out) decRefArr(in);
    c2->m_type = KindOfVec;
    c2->m_data.parr = out;
  } else if (isKeysetType(c2->m_type)) {
    auto in = c2->m_data.parr;
    auto out = SetArray::Append(in, *c1, in->cowCheck());
    if (in != out) decRefArr(in);
    c2->m_type = KindOfKeyset;
    c2->m_data.parr = out;
  } else {
    raise_error("AddNewElemC: $2 must be an array, vec, or keyset");
  }
  assertx(cellIsPlausible(*c2));
  vmStack().popC();
}

OPTBLD_INLINE void iopAddNewElemV() {
  Ref* r1 = vmStack().topV();
  Cell* c2 = vmStack().indC(1);
  if (!isArrayType(c2->m_type)) {
    raise_error("AddNewElemV: $2 must be an array");
  }
  cellAsVariant(*c2).asArrRef().appendRef(tvAsVariant(r1));
  vmStack().popV();
}

OPTBLD_INLINE void iopNewCol(CollectionType cType) {
  assertx(cType != CollectionType::Pair);
  // Incref the collection object during construction.
  auto obj = collections::alloc(cType);
  vmStack().pushObjectNoRc(obj);
}

OPTBLD_INLINE void iopNewPair() {
  Cell* c1 = vmStack().topC();
  Cell* c2 = vmStack().indC(1);
  // elements were pushed onto the stack in the order they should appear
  // in the pair, so the top of the stack should become the second element
  auto pair = collections::allocPair(*c2, *c1);
  // This constructor moves values, no inc/decref is necessary.
  vmStack().ndiscard(2);
  vmStack().pushObjectNoRc(pair);
}

OPTBLD_INLINE void iopColFromArray(CollectionType cType) {
  assertx(cType != CollectionType::Pair);
  auto const c1 = vmStack().topC();
  if (cType == CollectionType::Vector || cType == CollectionType::ImmVector) {
     if (UNLIKELY(!isVecType(c1->m_type))) {
       raise_error("ColFromArray: $1 must be a Vec when creating an "
                   "(Imm)Vector");
     }
   } else if (UNLIKELY(!isDictType(c1->m_type))) {
       raise_error("ColFromArray: $1 must be a Dict when creating an (Imm)Set "
                   "or an (Imm)Map");
   }
  // This constructor reassociates the ArrayData with the collection, so no
  // inc/decref is needed for the array. The collection object itself is
  // increfed.
  auto obj = collections::alloc(cType, c1->m_data.parr);
  vmStack().discard();
  vmStack().pushObjectNoRc(obj);
}

OPTBLD_INLINE void iopCns(const StringData* s) {
  auto const cns = Unit::loadCns(s);
  if (cns == nullptr) {
    raise_notice(Strings::UNDEFINED_CONSTANT, s->data(), s->data());
    vmStack().pushStaticString(s);
    return;
  }
  auto const c1 = vmStack().allocC();
  cellDup(*cns, *c1);
}

OPTBLD_INLINE void iopCnsE(const StringData* s) {
  auto const cns = Unit::loadCns(s);
  if (cns == nullptr) {
    raise_error("Undefined constant '%s'", s->data());
  }
  auto const c1 = vmStack().allocC();
  cellDup(*cns, *c1);
}

OPTBLD_INLINE void iopCnsU(const StringData* name, const StringData* fallback) {
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

OPTBLD_INLINE void iopDefCns(const StringData* s) {
  bool result = Unit::defCns(s, vmStack().topTV());
  vmStack().replaceTV<KindOfBoolean>(result);
}

OPTBLD_INLINE void iopClsCns(const StringData* clsCnsName, clsref_slot slot) {
  auto const cls    = slot.take();
  auto const clsCns = cls->clsCnsGet(clsCnsName);

  if (clsCns.m_type == KindOfUninit) {
    raise_error("Couldn't find constant %s::%s",
                cls->name()->data(), clsCnsName->data());
  }

  cellDup(clsCns, *vmStack().allocTV());
}

OPTBLD_INLINE void iopClsCnsD(const StringData* clsCnsName, Id classId) {
  const NamedEntityPair& classNamedEntity =
    vmfp()->m_func->unit()->lookupNamedEntityPairId(classId);
  auto const clsCns = g_context->lookupClsCns(classNamedEntity.second,
                                       classNamedEntity.first, clsCnsName);
  auto const c1 = vmStack().allocC();
  cellDup(clsCns, *c1);
}

OPTBLD_FLT_INLINE void iopConcat() {
  auto const c1 = vmStack().topC();
  auto const c2 = vmStack().indC(1);
  auto const s2 = cellAsVariant(*c2).toString();
  auto const s1 = cellAsCVarRef(*c1).toString();
  cellAsVariant(*c2) = concat(s2, s1);
  assertx(c2->m_data.pstr->checkCount());
  vmStack().popC();
}

OPTBLD_INLINE void iopConcatN(uint32_t n) {
  auto const c1 = vmStack().topC();
  auto const c2 = vmStack().indC(1);

  if (n == 2) {
    auto const s2 = cellAsVariant(*c2).toString();
    auto const s1 = cellAsCVarRef(*c1).toString();
    cellAsVariant(*c2) = concat(s2, s1);
    assertx(c2->m_data.pstr->checkCount());
  } else if (n == 3) {
    auto const c3 = vmStack().indC(2);
    auto const s3 = cellAsVariant(*c3).toString();
    auto const s2 = cellAsCVarRef(*c2).toString();
    auto const s1 = cellAsCVarRef(*c1).toString();
    cellAsVariant(*c3) = concat3(s3, s2, s1);
    assertx(c3->m_data.pstr->checkCount());
  } else {
    assertx(n == 4);
    auto const c3 = vmStack().indC(2);
    auto const c4 = vmStack().indC(3);
    auto const s4 = cellAsVariant(*c4).toString();
    auto const s3 = cellAsCVarRef(*c3).toString();
    auto const s2 = cellAsCVarRef(*c2).toString();
    auto const s1 = cellAsCVarRef(*c1).toString();
    cellAsVariant(*c4) = concat4(s4, s3, s2, s1);
    assertx(c4->m_data.pstr->checkCount());
  }

  for (int i = 1; i < n; ++i) {
    vmStack().popC();
  }
}

OPTBLD_INLINE void iopNot() {
  Cell* c1 = vmStack().topC();
  cellAsVariant(*c1) = !cellAsVariant(*c1).toBoolean();
}

template<class Fn>
OPTBLD_INLINE void implCellBinOp(Fn fn) {
  auto const c1 = vmStack().topC();
  auto const c2 = vmStack().indC(1);
  auto const result = fn(*c2, *c1);
  tvDecRefGen(c2);
  *c2 = result;
  vmStack().popC();
}

template<class Fn>
OPTBLD_INLINE void implCellBinOpBool(Fn fn) {
  auto const c1 = vmStack().topC();
  auto const c2 = vmStack().indC(1);
  bool const result = fn(*c2, *c1);
  tvDecRefGen(c2);
  *c2 = make_tv<KindOfBoolean>(result);
  vmStack().popC();
}

template<class Fn>
OPTBLD_INLINE void implCellBinOpInt64(Fn fn) {
  auto const c1 = vmStack().topC();
  auto const c2 = vmStack().indC(1);
  auto const result = fn(*c2, *c1);
  tvDecRefGen(c2);
  *c2 = make_tv<KindOfInt64>(result);
  vmStack().popC();
}

OPTBLD_INLINE void iopAdd() {
  implCellBinOp(cellAdd);
}

OPTBLD_INLINE void iopSub() {
  implCellBinOp(cellSub);
}

OPTBLD_INLINE void iopMul() {
  implCellBinOp(cellMul);
}

OPTBLD_INLINE void iopAddO() {
  implCellBinOp(cellAddO);
}

OPTBLD_INLINE void iopSubO() {
  implCellBinOp(cellSubO);
}

OPTBLD_INLINE void iopMulO() {
  implCellBinOp(cellMulO);
}

OPTBLD_INLINE void iopDiv() {
  implCellBinOp(cellDiv);
}

OPTBLD_INLINE void iopPow() {
  implCellBinOp(cellPow);
}

OPTBLD_INLINE void iopMod() {
  implCellBinOp(cellMod);
}

OPTBLD_INLINE void iopBitAnd() {
  implCellBinOp(cellBitAnd);
}

OPTBLD_INLINE void iopBitOr() {
  implCellBinOp(cellBitOr);
}

OPTBLD_INLINE void iopBitXor() {
  implCellBinOp(cellBitXor);
}

OPTBLD_INLINE void iopXor() {
  implCellBinOpBool([&] (Cell c1, Cell c2) -> bool {
    return cellToBool(c1) ^ cellToBool(c2);
  });
}

OPTBLD_INLINE void iopSame() {
  implCellBinOpBool(cellSame);
}

OPTBLD_INLINE void iopNSame() {
  implCellBinOpBool([&] (Cell c1, Cell c2) {
    return !cellSame(c1, c2);
  });
}

OPTBLD_INLINE void iopEq() {
  implCellBinOpBool([&] (Cell c1, Cell c2) {
    return cellEqual(c1, c2);
  });
}

OPTBLD_INLINE void iopNeq() {
  implCellBinOpBool([&] (Cell c1, Cell c2) {
    return !cellEqual(c1, c2);
  });
}

OPTBLD_INLINE void iopLt() {
  implCellBinOpBool([&] (Cell c1, Cell c2) {
    return cellLess(c1, c2);
  });
}

OPTBLD_INLINE void iopLte() {
  implCellBinOpBool(cellLessOrEqual);
}

OPTBLD_INLINE void iopGt() {
  implCellBinOpBool([&] (Cell c1, Cell c2) {
    return cellGreater(c1, c2);
  });
}

OPTBLD_INLINE void iopGte() {
  implCellBinOpBool(cellGreaterOrEqual);
}

OPTBLD_INLINE void iopCmp() {
  implCellBinOpInt64([&] (Cell c1, Cell c2) {
    return cellCompare(c1, c2);
  });
}

OPTBLD_INLINE void iopShl() {
  implCellBinOp(cellShl);
}

OPTBLD_INLINE void iopShr() {
  implCellBinOp(cellShr);
}

OPTBLD_INLINE void iopBitNot() {
  cellBitNot(*vmStack().topC());
}

OPTBLD_INLINE void iopCastBool() {
  Cell* c1 = vmStack().topC();
  tvCastToBooleanInPlace(c1);
}

OPTBLD_INLINE void iopCastInt() {
  Cell* c1 = vmStack().topC();
  tvCastToInt64InPlace(c1);
}

OPTBLD_INLINE void iopCastDouble() {
  Cell* c1 = vmStack().topC();
  tvCastToDoubleInPlace(c1);
}

OPTBLD_INLINE void iopCastString() {
  Cell* c1 = vmStack().topC();
  tvCastToStringInPlace(c1);
}

OPTBLD_INLINE void iopCastArray() {
  Cell* c1 = vmStack().topC();
  tvCastToArrayInPlace(c1);
}

OPTBLD_INLINE void iopCastObject() {
  Cell* c1 = vmStack().topC();
  tvCastToObjectInPlace(c1);
}

OPTBLD_INLINE void iopCastDict() {
  Cell* c1 = vmStack().topC();
  tvCastToDictInPlace(c1);
}

OPTBLD_INLINE void iopCastKeyset() {
  Cell* c1 = vmStack().topC();
  tvCastToKeysetInPlace(c1);
}

OPTBLD_INLINE void iopCastVec() {
  Cell* c1 = vmStack().topC();
  tvCastToVecInPlace(c1);
}

OPTBLD_INLINE void iopCastVArray() {
  assertx(!RuntimeOption::EvalHackArrDVArrs);
  Cell* c1 = vmStack().topC();
  tvCastToVArrayInPlace(c1);
}

OPTBLD_INLINE void iopCastDArray() {
  assertx(!RuntimeOption::EvalHackArrDVArrs);
  Cell* c1 = vmStack().topC();
  tvCastToDArrayInPlace(c1);
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

OPTBLD_INLINE void iopInstanceOf() {
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

OPTBLD_INLINE void iopInstanceOfD(Id id) {
  const NamedEntity* ne = vmfp()->m_func->unit()->lookupNamedEntityId(id);
  Cell* c1 = vmStack().topC();
  bool r = cellInstanceOf(c1, ne);
  vmStack().replaceC<KindOfBoolean>(r);
}

namespace {

Array resolveAndVerifyTypeStructureHelper(const Array& ts, bool suppress) {
  Class* declaringCls = nullptr;
  Class* calledCls = nullptr;
  if (typeStructureCouldBeNonStatic(ts)) {
    auto const frame = vmfp();
    if (frame && frame->func() && frame->func()->isMethod()) {
      declaringCls = frame->func()->cls();
      calledCls = frame->hasClass()
        ? frame->getClass()
        : frame->getThis()->getVMClass();
    }
  }
  return resolveAndVerifyTypeStructure(ts, declaringCls, calledCls, suppress);
}

} // namespace

OPTBLD_INLINE void iopIsTypeStruct(const ArrayData* a) {
  auto c1 = vmStack().topC();
  assertx(c1 != nullptr);
  auto resolved = resolveAndVerifyTypeStructureHelper(ArrNR(a), true);
  auto b = checkTypeStructureMatchesCell(resolved, *c1);
  vmStack().replaceC<KindOfBoolean>(b);
}

OPTBLD_INLINE void iopAsTypeStruct(const ArrayData* a) {
  auto c1 = vmStack().topC();
  assertx(c1 != nullptr);
  std::string givenType, expectedType, errorKey;
  auto resolved = resolveAndVerifyTypeStructureHelper(ArrNR(a), false);
  if (!checkTypeStructureMatchesCell(
        resolved, *c1, givenType, expectedType, errorKey)) {
    throwTypeStructureDoesNotMatchCellException(
      givenType, expectedType, errorKey);
  }
}

OPTBLD_INLINE void iopPrint() {
  Cell* c1 = vmStack().topC();
  g_context->write(cellAsVariant(*c1).toString());
  vmStack().replaceC<KindOfInt64>(1);
}

OPTBLD_INLINE void iopClone() {
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

OPTBLD_INLINE void iopExit() {
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

OPTBLD_INLINE void iopFatal(FatalOp kind_char) {
  TypedValue* top = vmStack().topTV();
  std::string msg;
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
    auto const flags = handle_request_surprise();

    // Memory Threhsold callback should also be fired here
    if (flags & MemThresholdFlag) {
      EventHook::DoMemoryThresholdCallback();
    }
  }
}

OPTBLD_INLINE void iopJmp(PC& pc, PC targetpc) {
  jmpSurpriseCheck(targetpc - pc);
  pc = targetpc;
}

OPTBLD_INLINE void iopJmpNS(PC& pc, PC targetpc) {
  pc = targetpc;
}

template<Op op>
OPTBLD_INLINE void jmpOpImpl(PC& pc, PC targetpc) {
  static_assert(op == OpJmpZ || op == OpJmpNZ,
                "jmpOpImpl should only be used by JmpZ and JmpNZ");
  jmpSurpriseCheck(targetpc - pc);

  Cell* c1 = vmStack().topC();
  if (c1->m_type == KindOfInt64 || c1->m_type == KindOfBoolean) {
    int64_t n = c1->m_data.num;
    vmStack().popX();
    if (op == OpJmpZ ? n == 0 : n != 0) pc = targetpc;
  } else {
    auto const cond = cellAsCVarRef(*c1).toBoolean();
    vmStack().popC();
    if (op == OpJmpZ ? !cond : cond) pc = targetpc;
  }
}

OPTBLD_INLINE void iopJmpZ(PC& pc, PC targetpc) {
  jmpOpImpl<OpJmpZ>(pc, targetpc);
}

OPTBLD_INLINE void iopJmpNZ(PC& pc, PC targetpc) {
  jmpOpImpl<OpJmpNZ>(pc, targetpc);
}

OPTBLD_INLINE
void iopIterBreak(PC& pc, PC targetpc, const IterTable& iterTab) {
  for (auto const& ent : iterTab) {
    auto iter = frame_iter(vmfp(), ent.id);
    switch (ent.kind) {
      case KindOfIter:  iter->free();  break;
      case KindOfMIter: iter->mfree(); break;
      case KindOfCIter: iter->cfree(); break;
      case KindOfLIter: iter->free();  break;
    }
  }
  pc = targetpc;
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

OPTBLD_INLINE
void iopSwitch(PC origpc, PC& pc, SwitchKind kind, int64_t base,
               imm_array<Offset> jmptab) {
  auto const veclen = jmptab.size;
  assertx(veclen > 0);
  TypedValue* val = vmStack().topTV();
  if (kind == SwitchKind::Unbounded) {
    assertx(val->m_type == KindOfInt64);
    // Continuation switch: no bounds checking needed
    int64_t label = val->m_data.num;
    vmStack().popX();
    assertx(label >= 0 && label < veclen);
    pc = origpc + jmptab[label];
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
            case KindOfPersistentVec:
            case KindOfVec:
            case KindOfPersistentDict:
            case KindOfDict:
            case KindOfPersistentKeyset:
            case KindOfKeyset:
            case KindOfPersistentArray:
            case KindOfArray:
            case KindOfObject:
            case KindOfResource:
            case KindOfRef:
              not_reached();
          }
          if (val->m_type == KindOfString) tvDecRefStr(val);
          return;
        }

        case KindOfVec:
          tvDecRefArr(val);
        case KindOfPersistentVec:
          match = SwitchMatch::DEFAULT;
          return;

        case KindOfDict:
          tvDecRefArr(val);
        case KindOfPersistentDict:
          match = SwitchMatch::DEFAULT;
          return;

        case KindOfKeyset:
          tvDecRefArr(val);
        case KindOfPersistentKeyset:
          match = SwitchMatch::DEFAULT;
          return;

        case KindOfArray:
          tvDecRefArr(val);
        case KindOfPersistentArray:
          match = SwitchMatch::DEFAULT;
          return;

        case KindOfObject:
          intval = val->m_data.pobj->toInt64();
          tvDecRefObj(val);
          return;

        case KindOfResource:
          intval = val->m_data.pres->data()->o_toInt64();
          tvDecRefRes(val);
          return;

        case KindOfRef:
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
          pc = origpc + jmptab[veclen - 1];
          break;

        case SwitchMatch::NONZERO:
          pc = origpc + jmptab[veclen - 2];
          break;
      }
    } else {
      pc = origpc + jmptab[intval - base];
    }
  }
}

OPTBLD_INLINE
void iopSSwitch(PC origpc, PC& pc, imm_array<StrVecItem> jmptab) {
  auto const veclen = jmptab.size;
  assertx(veclen > 1);
  unsigned cases = veclen - 1; // the last vector item is the default case
  Cell* val = tvToCell(vmStack().topTV());
  Unit* u = vmfp()->m_func->unit();
  unsigned i;
  for (i = 0; i < cases; ++i) {
    auto item = jmptab[i];
    const StringData* str = u->lookupLitstrId(item.str);
    if (cellEqual(*val, str)) {
      pc = origpc + item.dest;
      vmStack().popC();
      return;
    }
  }
  // default case
  pc = origpc + jmptab[veclen - 1].dest;
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
    if (reinterpret_cast<TCA>(savedRip) != jit::tc::ustubs().callToExit) {
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
      assertx(jit::g_unwind_rds.isInit());
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
    assertx(vmfp() == nullptr);
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
    return jit::tc::ustubs().resumeHelper;
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
    assertx(vmStack().topTV() == vmfp()->retSlot());
    // In case we were called by a jitted FCallAwait, let it know
    // that we finished eagerly.
    vmStack().topTV()->m_aux.u_fcallAwaitFlag = 0;
  } else if (vmfp()->func()->isAsyncFunction()) {
    // Mark the async function as succeeded and store the return value.
    assertx(!sfp);
    auto wh = frame_afwh(vmfp());
    wh->ret(retval);
    decRefObj(wh);
  } else if (vmfp()->func()->isAsyncGenerator()) {
    // Mark the async generator as finished.
    assertx(isNullType(retval.m_type));
    auto const gen = frame_async_generator(vmfp());
    auto const eagerResult = gen->ret();
    if (eagerResult) {
      // Eager execution => return StaticWaitHandle.
      assertx(sfp);
      vmStack().pushObjectNoRc(eagerResult);
    } else {
      // Resumed execution => return control to the scheduler.
      assertx(!sfp);
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

OPTBLD_INLINE TCA iopRetC(PC& pc) {
  return ret(pc);
}

OPTBLD_INLINE TCA iopRetV(PC& pc) {
  assertx(!vmfp()->resumed());
  assertx(!vmfp()->func()->isResumable());
  return ret(pc);
}

OPTBLD_INLINE TCA iopRetM(PC& pc, uint32_t numRet) {
  auto const jitReturn = jitReturnPre(vmfp());

  req::vector<TypedValue> retvals;
  retvals.reserve(numRet);

  for (int i = numRet - 1; i >= 0; i--) {
    retvals.push_back(*vmStack().indC(i));
  }

  vmStack().ndiscard(numRet);

  // Free $this and local variables. Calls FunctionReturn hook. The return
  // value must be removed from the stack, or the unwinder would try to free it
  // if the hook throws---but the event hook routine decrefs the return value
  // in that case if necessary.
  frame_free_locals_inl(vmfp(), vmfp()->func()->numLocals(), &retvals[0]);

  assertx(!vmfp()->func()->isGenerator() && !vmfp()->func()->isAsync());

  if (isProfileRequest()) {
    profileIncrementFuncCounter(vmfp()->func());
  }

  // Grab caller info from ActRec.
  ActRec* sfp = vmfp()->sfp();
  Offset soff = vmfp()->m_soff;

  // Free ActRec and store the return value.
  vmStack().ndiscard(vmfp()->func()->numSlotsInFrame());
  vmStack().ret();

  // Discard scratch space for return values allocated for FCallM
  vmStack().ndiscard(numRet - 1);
  *vmStack().topTV() = retvals[1];

  for (int i = 2; i < numRet; i++) {
    *vmStack().allocTV() = retvals[i];
  }

  // Store the actual return value at the top of the stack
  *vmStack().allocTV() = retvals[0];

  // Return control to the caller.
  vmfp() = sfp;
  pc = LIKELY(vmfp() != nullptr) ? vmfp()->func()->getEntry() + soff : nullptr;

  return jitReturnPost(jitReturn);
}

OPTBLD_INLINE void iopUnwind() {
  assertx(!g_context->m_faults.empty());
  assertx(g_context->m_faults.back().m_raiseOffset != kInvalidOffset);
  throw VMPrepareUnwind();
}

OPTBLD_INLINE void iopThrow() {
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

OPTBLD_INLINE void iopClsRefGetC(clsref_slot slot) {
  auto const cell = vmStack().topC();
  auto const cls  = lookupClsRef(cell);
  slot.put(cls);
  vmStack().popC();
}

OPTBLD_INLINE void iopClsRefGetL(local_var fr, clsref_slot slot) {
  slot.put(lookupClsRef(tvToCell(fr.ptr)));
}

static void raise_undefined_local(ActRec* fp, Id pind) {
  assertx(pind < fp->m_func->numNamedLocals());
  raise_notice(Strings::UNDEFINED_VARIABLE,
               fp->m_func->localVarName(pind)->data());
}

static inline void cgetl_inner_body(TypedValue* fr, TypedValue* to) {
  assertx(fr->m_type != KindOfUninit);
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
    tvWriteNull(*to);
    if (warn) raise_undefined_local(fp, pind);
  } else {
    cgetl_inner_body(fr, to);
  }
}

OPTBLD_FLT_INLINE void iopCGetL(local_var fr) {
  Cell* to = vmStack().allocC();
  cgetl_body(vmfp(), fr.ptr, to, fr.index, true);
}

OPTBLD_INLINE void iopCGetQuietL(local_var fr) {
  Cell* to = vmStack().allocC();
  cgetl_body(vmfp(), fr.ptr, to, fr.index, false);
}

OPTBLD_INLINE void iopCUGetL(local_var fr) {
  auto to = vmStack().allocTV();
  tvDup(*tvToCell(fr.ptr), *to);
}

OPTBLD_INLINE void iopCGetL2(local_var fr) {
  TypedValue* oldTop = vmStack().topTV();
  TypedValue* newTop = vmStack().allocTV();
  memcpy(newTop, oldTop, sizeof *newTop);
  Cell* to = oldTop;
  cgetl_body(vmfp(), fr.ptr, to, fr.index, true);
}

OPTBLD_INLINE void iopPushL(local_var locVal) {
  assertx(locVal->m_type != KindOfUninit);
  assertx(!isRefType(locVal->m_type));
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
    tvDecRefGen(to);
    tvWriteNull(*to);
  } else {
    tvDecRefGen(to);
    cgetl_inner_body(fr, to);
  }
}

OPTBLD_INLINE void iopCGetN() { cgetn_body(true); }
OPTBLD_INLINE void iopCGetQuietN() { cgetn_body(false); }

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
    tvDecRefGen(to);
    tvWriteNull(*to);
  } else if (fr->m_type == KindOfUninit) {
    if (warn) raise_notice(Strings::UNDEFINED_VARIABLE, name->data());
    tvDecRefGen(to);
    tvWriteNull(*to);
  } else {
    tvDecRefGen(to);
    cgetl_inner_body(fr, to);
  }
}

OPTBLD_INLINE void iopCGetG() { cgetg_body(true); }
OPTBLD_INLINE void iopCGetQuietG() { cgetg_body(false); }

struct SpropState {
  SpropState(Stack&, clsref_slot slot);
  ~SpropState();
  StringData* name;
  Class* cls;
  TypedValue* output;
  TypedValue* val;
  TypedValue oldNameCell;
  bool visible;
  bool accessible;
};

SpropState::SpropState(Stack& vmstack, clsref_slot slot) {
  cls = slot.take();
  auto nameCell = output = vmstack.topTV();
  lookup_sprop(vmfp(), cls, name, nameCell, val, visible, accessible);
  oldNameCell = *nameCell;
}

SpropState::~SpropState() {
  decRefStr(name);
  tvDecRefGen(oldNameCell);
}

template<bool box> void getS(clsref_slot slot) {
  SpropState ss(vmStack(), slot);
  if (!(ss.visible && ss.accessible)) {
    raise_error("Invalid static property access: %s::%s",
                ss.cls->name()->data(),
                ss.name->data());
  }
  if (box) {
    if (!isRefType(ss.val->m_type)) {
      tvBox(*ss.val);
    }
    refDup(*ss.val, *ss.output);
  } else {
    cellDup(*tvToCell(ss.val), *ss.output);
  }
}

OPTBLD_INLINE void iopCGetS(clsref_slot slot) {
  getS<false>(slot);
}

static inline MInstrState& initMState() {
  auto& mstate = vmMInstrState();
  tvWriteUninit(mstate.tvRef);
  tvWriteUninit(mstate.tvRef2);
  return mstate;
}

using LookupNameFn = void (*)(ActRec*, StringData*&, TypedValue*, TypedValue*&);

static inline void baseNGImpl(TypedValue* key, MOpMode mode,
                              LookupNameFn lookupd, LookupNameFn lookup) {
  auto& mstate = initMState();
  StringData* name;
  TypedValue* baseVal;

  if (mode == MOpMode::Define) lookupd(vmfp(), name, key, baseVal);
  else                         lookup(vmfp(), name, key, baseVal);
  SCOPE_EXIT { decRefStr(name); };

  if (baseVal == nullptr) {
    assertx(mode != MOpMode::Define);
    if (mode == MOpMode::Warn) {
      raise_notice(Strings::UNDEFINED_VARIABLE, name->data());
    }
    tvWriteNull(mstate.tvTempBase);
    mstate.base = &mstate.tvTempBase;
    return;
  }

  mstate.base = baseVal;
}

static inline void baseNImpl(TypedValue* key, MOpMode mode) {
  baseNGImpl(key, mode, lookupd_var, lookup_var);
}

OPTBLD_INLINE void iopBaseNC(uint32_t idx, MOpMode mode) {
  baseNImpl(vmStack().indTV(idx), mode);
}

OPTBLD_INLINE void iopBaseNL(local_var loc, MOpMode mode) {
  baseNImpl(tvToCell(loc.ptr), mode);
}

OPTBLD_INLINE void iopFPassBaseNC(ActRec* ar, uint32_t paramId, uint32_t idx) {
  auto const mode = fpass_mode(ar, paramId);
  baseNImpl(vmStack().indTV(idx), mode);
}

OPTBLD_INLINE void iopFPassBaseNL(ActRec* ar, uint32_t paramId, local_var loc) {
  auto const mode = fpass_mode(ar, paramId);
  baseNImpl(tvToCell(loc.ptr), mode);
}

static inline void baseGImpl(TypedValue* key, MOpMode mode) {
  baseNGImpl(key, mode, lookupd_gbl, lookup_gbl);
}

OPTBLD_INLINE void iopBaseGC(uint32_t idx, MOpMode mode) {
  baseGImpl(vmStack().indTV(idx), mode);
}

OPTBLD_INLINE void iopBaseGL(local_var loc, MOpMode mode) {
  baseGImpl(tvToCell(loc.ptr), mode);
}

OPTBLD_INLINE void iopFPassBaseGC(ActRec* ar, uint32_t paramId, uint32_t idx) {
  auto const mode = fpass_mode(ar, paramId);
  baseGImpl(vmStack().indTV(idx), mode);
}

OPTBLD_INLINE void iopFPassBaseGL(ActRec* ar, uint32_t paramId, local_var loc) {
  auto const mode = fpass_mode(ar, paramId);
  baseGImpl(tvToCell(loc.ptr), mode);
}

static inline tv_lval baseSImpl(TypedValue* key,
                                    clsref_slot slot) {
  auto const class_ = slot.take();

  auto const name = lookup_name(key);
  SCOPE_EXIT { decRefStr(name); };
  auto const lookup = class_->getSProp(arGetContextClass(vmfp()), name);
  if (!lookup.prop || !lookup.accessible) {
    raise_error("Invalid static property access: %s::%s",
                class_->name()->data(),
                name->data());
  }

  return tv_lval(lookup.prop);
}

OPTBLD_INLINE void iopBaseSC(uint32_t keyIdx, clsref_slot slot) {
  auto& mstate = initMState();
  mstate.base = baseSImpl(vmStack().indTV(keyIdx), slot);
}

OPTBLD_INLINE void iopBaseSL(local_var keyLoc, clsref_slot slot) {
  auto& mstate = initMState();
  mstate.base = baseSImpl(tvToCell(keyLoc.ptr), slot);
}

OPTBLD_INLINE void baseLImpl(local_var loc, MOpMode mode) {
  auto& mstate = initMState();
  auto local = tvToCell(loc.ptr);
  if (mode == MOpMode::Warn && local->m_type == KindOfUninit) {
    raise_notice(Strings::UNDEFINED_VARIABLE,
                 vmfp()->m_func->localVarName(loc.index)->data());
  }
  mstate.base = local;
}

OPTBLD_INLINE void iopBaseL(local_var loc, MOpMode mode) {
  baseLImpl(loc, mode);
}

OPTBLD_INLINE void iopFPassBaseL(ActRec* ar, uint32_t paramId, local_var loc) {
  auto mode = fpass_mode(ar, paramId);
  baseLImpl(loc, mode);
}

OPTBLD_INLINE void iopBaseC(uint32_t idx) {
  auto& mstate = initMState();
  mstate.base = vmStack().indTV(idx);
}

OPTBLD_INLINE void iopBaseR(uint32_t idx) {
  iopBaseC(idx);
}

OPTBLD_INLINE void iopBaseH() {
  auto& mstate = initMState();
  mstate.tvTempBase = make_tv<KindOfObject>(vmfp()->getThis());
  mstate.base = &mstate.tvTempBase;
}

static OPTBLD_INLINE void propDispatch(MOpMode mode, TypedValue key,
                                       bool reffy) {
  auto& mstate = vmMInstrState();
  auto ctx = arGetContextClass(vmfp());

  auto const result = [&]{
    switch (mode) {
      case MOpMode::None:
        assertx(!reffy);
        return Prop<MOpMode::None>(mstate.tvRef, ctx, mstate.base, key);
      case MOpMode::Warn:
        assertx(!reffy);
        return Prop<MOpMode::Warn>(mstate.tvRef, ctx, mstate.base, key);
      case MOpMode::Define:
        if (reffy) {
          return Prop<MOpMode::Define,KeyType::Any,true>(
            mstate.tvRef, ctx, mstate.base, key);
        } else {
          return Prop<MOpMode::Define,KeyType::Any,false>(
            mstate.tvRef, ctx, mstate.base, key);
        }
      case MOpMode::Unset:
        assertx(!reffy);
        return Prop<MOpMode::Unset>(mstate.tvRef, ctx, mstate.base, key);
      case MOpMode::InOut:
        always_assert_flog(false, "MOpMode::InOut can only occur on Elem");
    }
    always_assert(false);
  }();

  mstate.base = ratchetRefs(result, mstate.tvRef, mstate.tvRef2);
}

static OPTBLD_INLINE void propQDispatch(MOpMode mode, TypedValue key,
                                        bool reffy) {
  auto& mstate = vmMInstrState();
  auto ctx = arGetContextClass(vmfp());

  auto const result = [&] {
    switch (mode) {
      case MOpMode::None:
      case MOpMode::Warn:
        assertx(key.m_type == KindOfPersistentString);
        return nullSafeProp(mstate.tvRef, ctx,
                            mstate.base, key.m_data.pstr);
      case MOpMode::Define:
        if (reffy) raise_error(Strings::NULLSAFE_PROP_WRITE_ERROR);
      case MOpMode::InOut:
        always_assert_flog(false, "MOpMode::InOut can only occur on Elem");
      case MOpMode::Unset:
        always_assert(false);
    }
    not_reached();
  }();

  mstate.base = ratchetRefs(result, mstate.tvRef, mstate.tvRef2);
}

static OPTBLD_INLINE
void elemDispatch(MOpMode mode, TypedValue key, bool reffy) {
  auto& mstate = vmMInstrState();
  auto const b = mstate.base;

  auto const result = [&]() -> tv_rval {
    switch (mode) {
      case MOpMode::None:
        return
          UNLIKELY(checkHACIntishCast())
            ? Elem<MOpMode::None, true>(mstate.tvRef, b, key)
            : Elem<MOpMode::None, false>(mstate.tvRef, b, key);
      case MOpMode::Warn:
        return
          UNLIKELY(checkHACIntishCast())
            ? Elem<MOpMode::Warn, true>(mstate.tvRef, b, key)
            : Elem<MOpMode::Warn, false>(mstate.tvRef, b, key);
      case MOpMode::InOut:
        return
          UNLIKELY(checkHACIntishCast())
            ? Elem<MOpMode::InOut, true>(mstate.tvRef, b, key)
            : Elem<MOpMode::InOut, false>(mstate.tvRef, b, key);
      case MOpMode::Define:
        if (UNLIKELY(checkHACIntishCast())) {
          return reffy
            ? ElemD<MOpMode::Define, true, true>(mstate.tvRef, b, key)
            : ElemD<MOpMode::Define, false, true>(mstate.tvRef, b, key);
        } else {
          return reffy
            ? ElemD<MOpMode::Define, true, false>(mstate.tvRef, b, key)
            : ElemD<MOpMode::Define, false, false>(mstate.tvRef, b, key);
        }
      case MOpMode::Unset:
        return UNLIKELY(checkHACIntishCast())
          ? ElemU<true>(mstate.tvRef, b, key)
          : ElemU<false>(mstate.tvRef, b, key);
    }
    always_assert(false);
  }().as_lval();

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

static OPTBLD_INLINE void dimDispatch(MOpMode mode, MemberKey mk,
                                      bool reffy) {
  auto const key = key_tv(mk);
  if (mk.mcode == MQT) {
    propQDispatch(mode, key, reffy);
  } else if (mcodeIsProp(mk.mcode)) {
    propDispatch(mode, key, reffy);
  } else if (mcodeIsElem(mk.mcode)) {
    elemDispatch(mode, key, reffy);
  } else {
    if (mode == MOpMode::Warn) raise_error("Cannot use [] for reading");

    auto& mstate = vmMInstrState();
    auto const base = mstate.base;
    auto const result = [&] {
      if (reffy) {
        if (UNLIKELY(isHackArrayType(type(base)))) {
          throwRefInvalidArrayValueException(val(base).parr);
        }
        return NewElem<true>(mstate.tvRef, base);
      } else {
        return NewElem<false>(mstate.tvRef, base);
      }
    }();
    mstate.base = ratchetRefs(result, mstate.tvRef, mstate.tvRef2);
  }
}

OPTBLD_INLINE void iopDim(MOpMode mode, MemberKey mk) {
  dimDispatch(mode, mk, false);
}

OPTBLD_INLINE void iopFPassDim(ActRec* ar, uint32_t paramId, MemberKey mk) {
  auto const mode = fpass_mode(ar, paramId);
  dimDispatch(mode, mk, false);
}

static OPTBLD_INLINE void mFinal(MInstrState& mstate,
                                 int32_t nDiscard,
                                 folly::Optional<TypedValue> result) {
  auto& stack = vmStack();
  for (auto i = 0; i < nDiscard; ++i) stack.popTV();
  if (result) tvCopy(*result, *stack.allocTV());

  tvDecRefGenUnlikely(mstate.tvRef);
  tvDecRefGenUnlikely(mstate.tvRef2);
}

static OPTBLD_INLINE
void queryMImpl(MemberKey mk, int32_t nDiscard, QueryMOp op) {
  auto const key = key_tv(mk);
  auto& mstate = vmMInstrState();
  TypedValue result;
  switch (op) {
    case QueryMOp::InOut:
      always_assert_flog(
        mcodeIsElem(mk.mcode), "QueryM InOut is only compatible with Elem"
      );
      // fallthrough
    case QueryMOp::CGet:
    case QueryMOp::CGetQuiet:
      dimDispatch(getQueryMOpMode(op), mk, false);
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
        assertx(mcodeIsElem(mk.mcode));
        if (UNLIKELY(checkHACIntishCast())) {
          result.m_data.num = op == QueryMOp::Empty
            ? IssetEmptyElem<true, true>(mstate.base, key)
            : IssetEmptyElem<false, true>(mstate.base, key);
        } else {
          result.m_data.num = op == QueryMOp::Empty
            ? IssetEmptyElem<true, false>(mstate.base, key)
            : IssetEmptyElem<false, false>(mstate.base, key);
        }
      }
      break;
  }
  mFinal(mstate, nDiscard, result);
}

OPTBLD_INLINE void iopQueryM(uint32_t nDiscard, QueryMOp subop, MemberKey mk) {
  queryMImpl(mk, nDiscard, subop);
}

static OPTBLD_INLINE void vGetMImpl(MemberKey mk, int32_t nDiscard) {
  auto& mstate = vmMInstrState();
  TypedValue result;
  dimDispatch(MOpMode::Define, mk, true);
  tvBoxIfNeeded(mstate.base);
  refDup(*mstate.base, result);
  mFinal(mstate, nDiscard, result);
}

OPTBLD_INLINE void iopVGetM(uint32_t nDiscard, MemberKey mk) {
  vGetMImpl(mk, nDiscard);
}

static void checkFPassHint(ActRec* ar, uint32_t paramId, FPassHint hint) {
  assertx(paramId < ar->numArgs());

  if (!RuntimeOption::EvalThrowOnCallByRefAnnotationMismatch &&
      !RuntimeOption::EvalWarnOnCallByRefAnnotationMismatch) {
    return;
  }

  switch (hint) {
  case FPassHint::Any: return;
  case FPassHint::Cell:
    if (!ar->m_func->byRef(paramId)) return;
    break;
  case FPassHint::Ref:
    if (ar->m_func->byRef(paramId)) return;
    break;
  }
  raiseParamRefMismatchForFunc(ar->m_func, paramId);
}

OPTBLD_INLINE
void iopFPassM(ActRec* ar, uint32_t paramId, uint32_t nDiscard, MemberKey mk,
               FPassHint hint) {
  checkFPassHint(ar, paramId, hint);
  auto const mode = fpass_mode(ar, paramId);
  if (mode == MOpMode::Warn) {
    return queryMImpl(mk, nDiscard, QueryMOp::CGet);
  }
  vGetMImpl(mk, nDiscard);
}

OPTBLD_FLT_INLINE void iopSetM(uint32_t nDiscard, MemberKey mk) {
  auto& mstate = vmMInstrState();
  auto const topC = vmStack().topC();

  if (mk.mcode == MW) {
    SetNewElem<true>(mstate.base, topC);
  } else {
    auto const key = key_tv(mk);
    if (mcodeIsElem(mk.mcode)) {
      auto const result = UNLIKELY(checkHACIntishCast())
        ? SetElem<true, true>(mstate.base, key, topC)
        : SetElem<true, false>(mstate.base, key, topC);
      if (result) {
        tvDecRefGen(topC);
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

OPTBLD_INLINE void iopIncDecM(uint32_t nDiscard, IncDecOp subop, MemberKey mk) {
  auto const key = key_tv(mk);

  auto& mstate = vmMInstrState();
  Cell result;
  if (mcodeIsProp(mk.mcode)) {
    result = IncDecProp(arGetContextClass(vmfp()), subop, mstate.base, key);
  } else if (mcodeIsElem(mk.mcode)) {
    result = UNLIKELY(checkHACIntishCast())
      ? IncDecElem<true>(subop, mstate.base, key)
      : IncDecElem<false>(subop, mstate.base, key);
  } else {
    result = IncDecNewElem(mstate.tvRef, subop, mstate.base);
  }

  mFinal(mstate, nDiscard, result);
}

OPTBLD_INLINE void iopSetOpM(uint32_t nDiscard, SetOpOp subop, MemberKey mk) {
  auto const key = key_tv(mk);
  auto const rhs = vmStack().topC();

  auto& mstate = vmMInstrState();
  tv_lval result;
  if (mcodeIsProp(mk.mcode)) {
    result = SetOpProp(mstate.tvRef, arGetContextClass(vmfp()), subop,
                       mstate.base, key, rhs);
  } else if (mcodeIsElem(mk.mcode)) {
    result = UNLIKELY(checkHACIntishCast())
      ? SetOpElem<true>(mstate.tvRef, subop, mstate.base, key, rhs)
      : SetOpElem<false>(mstate.tvRef, subop, mstate.base, key, rhs);
  } else {
    result = SetOpNewElem(mstate.tvRef, subop, mstate.base, rhs);
  }

  vmStack().popC();
  result = tvToCell(result);
  tvIncRefGen(*result);
  mFinal(mstate, nDiscard, *result);
}

OPTBLD_INLINE void iopBindM(uint32_t nDiscard, MemberKey mk) {
  auto& mstate = vmMInstrState();
  auto const rhs = *vmStack().topV();

  dimDispatch(MOpMode::Define, mk, true);
  tvBind(rhs, mstate.base);

  vmStack().discard();
  mFinal(mstate, nDiscard, rhs);
}

OPTBLD_INLINE void iopUnsetM(uint32_t nDiscard, MemberKey mk) {
  auto const key = key_tv(mk);

  auto& mstate = vmMInstrState();
  if (mcodeIsProp(mk.mcode)) {
    UnsetProp(arGetContextClass(vmfp()), mstate.base, key);
  } else {
    assertx(mcodeIsElem(mk.mcode));
    if (UNLIKELY(checkHACIntishCast())) {
      UnsetElem<true>(mstate.base, key);
    } else {
      UnsetElem<false>(mstate.base, key);
    }
  }

  mFinal(mstate, nDiscard, folly::none);
}

static OPTBLD_INLINE void setWithRefImpl(TypedValue key, TypedValue* value) {
  auto& mstate = vmMInstrState();
  if (UNLIKELY(RuntimeOption::EvalHackArrCompatNotices)) {
    mstate.base = [&] {
      if (LIKELY(!isRefType(value->m_type))) {
        SuppressHackArrCompatNotices shacn;
        return ElemD<MOpMode::Define, false, false>(
          mstate.tvRef, mstate.base, key
        );
      }

      // See the comment in SetWithRefMLElem() for an explanation of this check
      // and the notice suppressor below.
      if (RuntimeOption::EvalHackArrCompatCheckRefBind &&
          !val(mstate.base).parr->isGlobalsArray()) {
        raiseHackArrCompatRefBind(key);
      }

      SuppressHackArrCompatNotices shacn;
      return ElemD<MOpMode::Define, true, false>(
        mstate.tvRef, mstate.base, key
      );
    }();
  } else {
    mstate.base = UNLIKELY(isRefType(value->m_type))
      ? ElemD<MOpMode::Define, true, false>(mstate.tvRef, mstate.base, key)
      : ElemD<MOpMode::Define, false, false>(mstate.tvRef, mstate.base, key);
  }
  tvSetWithRef(*value, mstate.base);

  mFinal(mstate, 0, folly::none);
}

OPTBLD_INLINE void iopSetWithRefLML(local_var kloc, local_var vloc) {
  auto const key = *tvToCell(kloc.ptr);
  setWithRefImpl(key, vloc.ptr);
}

OPTBLD_INLINE void iopSetWithRefRML(local_var local) {
  auto const key = *tvToCell(local.ptr);
  setWithRefImpl(key, vmStack().topTV());
  vmStack().popTV();
}

OPTBLD_INLINE void iopMemoGet(uint32_t nDiscard,
                              LocalRange locals) {
  assertx(vmfp()->m_func->isMemoizeWrapper());
  assertx(locals.first + locals.restCount < vmfp()->m_func->numLocals());
  auto mstate = vmMInstrState();
  auto const res = MixedArray::MemoGet(
    mstate.base,
    frame_local(vmfp(), locals.first),
    locals.restCount + 1
  );
  mFinal(mstate, nDiscard, res);
}

OPTBLD_INLINE void iopMemoSet(uint32_t nDiscard,
                              LocalRange locals) {
  assertx(vmfp()->m_func->isMemoizeWrapper());
  assertx(locals.first + locals.restCount < vmfp()->m_func->numLocals());
  auto const value = *vmStack().topC();
  auto mstate = vmMInstrState();
  MixedArray::MemoSet(
    mstate.base,
    frame_local(vmfp(), locals.first),
    locals.restCount + 1,
    value
  );
  vmStack().discard();
  mFinal(mstate, nDiscard, value);
}

static inline void vgetl_body(TypedValue* fr, TypedValue* to) {
  if (!isRefType(fr->m_type)) {
    tvBox(*fr);
  }
  refDup(*fr, *to);
}

OPTBLD_INLINE void iopVGetL(local_var fr) {
  Ref* to = vmStack().allocV();
  vgetl_body(fr.ptr, to);
}

OPTBLD_INLINE void iopVGetN() {
  StringData* name;
  TypedValue* to = vmStack().topTV();
  TypedValue* fr = nullptr;
  lookupd_var(vmfp(), name, to, fr);
  SCOPE_EXIT { decRefStr(name); };
  assertx(fr != nullptr);
  tvDecRefGen(to);
  vgetl_body(fr, to);
}

OPTBLD_INLINE void iopVGetG() {
  StringData* name;
  TypedValue* to = vmStack().topTV();
  TypedValue* fr = nullptr;
  lookupd_gbl(vmfp(), name, to, fr);
  SCOPE_EXIT { decRefStr(name); };
  assertx(fr != nullptr);
  tvDecRefGen(to);
  vgetl_body(fr, to);
}

OPTBLD_INLINE void iopVGetS(clsref_slot slot) {
  getS<true>(slot);
}

OPTBLD_INLINE void iopIssetN() {
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

OPTBLD_INLINE void iopIssetG() {
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

OPTBLD_INLINE void iopIssetS(clsref_slot slot) {
  SpropState ss(vmStack(), slot);
  bool e;
  if (!(ss.visible && ss.accessible)) {
    e = false;
  } else {
    e = !cellIsNull(tvToCell(ss.val));
  }
  ss.output->m_data.num = e;
  ss.output->m_type = KindOfBoolean;
}

OPTBLD_FLT_INLINE void iopIssetL(local_var tv) {
  bool ret = is_not_null(tvAsCVarRef(tv.ptr));
  TypedValue* topTv = vmStack().allocTV();
  topTv->m_data.num = ret;
  topTv->m_type = KindOfBoolean;
}

OPTBLD_INLINE static bool isTypeHelper(Cell* val, IsTypeOp op) {
  assertx(cellIsPlausible(*val));

  switch (op) {
  case IsTypeOp::Null:   return is_null(tvAsCVarRef(val));
  case IsTypeOp::Bool:   return is_bool(tvAsCVarRef(val));
  case IsTypeOp::Int:    return is_int(tvAsCVarRef(val));
  case IsTypeOp::Dbl:    return is_double(tvAsCVarRef(val));
  case IsTypeOp::Arr:
    if (UNLIKELY(RuntimeOption::EvalHackArrCompatIsArrayNotices)) {
      if (isArrayType(val->m_type)) {
        if (val->m_data.parr->isVArray()) {
          raise_hackarr_compat_notice(Strings::HACKARR_COMPAT_VARR_IS_ARR);
        } else if (val->m_data.parr->isDArray()) {
          raise_hackarr_compat_notice(Strings::HACKARR_COMPAT_DARR_IS_ARR);
        }
        return true;
      } else if (isVecType(val->m_type)) {
        raise_hackarr_compat_notice(Strings::HACKARR_COMPAT_VEC_IS_ARR);
      } else if (isDictType(val->m_type)) {
        raise_hackarr_compat_notice(Strings::HACKARR_COMPAT_DICT_IS_ARR);
      } else if (isKeysetType(val->m_type)) {
        raise_hackarr_compat_notice(Strings::HACKARR_COMPAT_KEYSET_IS_ARR);
      }
      return false;
    }
    return is_array(tvAsCVarRef(val));
  case IsTypeOp::Vec:
    if (UNLIKELY(RuntimeOption::EvalHackArrCompatIsArrayNotices)) {
      if (isArrayType(val->m_type)) {
        if (val->m_data.parr->isVArray()) {
          raise_hackarr_compat_notice(Strings::HACKARR_COMPAT_VARR_IS_VEC);
        }
        return false;
      }
    }
    return is_vec(tvAsCVarRef(val));
  case IsTypeOp::Dict:
    if (UNLIKELY(RuntimeOption::EvalHackArrCompatIsArrayNotices)) {
      if (isArrayType(val->m_type)) {
        if (val->m_data.parr->isDArray()) {
          raise_hackarr_compat_notice(Strings::HACKARR_COMPAT_DARR_IS_DICT);
        }
        return false;
      }
    }
    return is_dict(tvAsCVarRef(val));
  case IsTypeOp::Keyset: return is_keyset(tvAsCVarRef(val));
  case IsTypeOp::Obj:    return is_object(tvAsCVarRef(val));
  case IsTypeOp::Str:    return is_string(tvAsCVarRef(val));
  case IsTypeOp::Res:    return val->m_type == KindOfResource;
  case IsTypeOp::Scalar: return HHVM_FN(is_scalar)(tvAsCVarRef(val));
  case IsTypeOp::ArrLike: return isArrayLikeType(val->m_type);
  case IsTypeOp::VArray:
    assertx(!RuntimeOption::EvalHackArrDVArrs);
    if (UNLIKELY(RuntimeOption::EvalHackArrCompatIsArrayNotices)) {
      if (isVecType(val->m_type)) {
        raise_hackarr_compat_notice(Strings::HACKARR_COMPAT_VEC_IS_VARR);
        return false;
      }
    }
    return is_varray(tvAsCVarRef(val));
  case IsTypeOp::DArray:
    assertx(!RuntimeOption::EvalHackArrDVArrs);
    if (UNLIKELY(RuntimeOption::EvalHackArrCompatIsArrayNotices)) {
      if (isDictType(val->m_type)) {
        raise_hackarr_compat_notice(Strings::HACKARR_COMPAT_DICT_IS_DARR);
        return false;
      }
    }
    return is_darray(tvAsCVarRef(val));
  }
  not_reached();
}

OPTBLD_INLINE void iopIsTypeL(local_var loc, IsTypeOp op) {
  if (loc.ptr->m_type == KindOfUninit) {
    raise_undefined_local(vmfp(), loc.index);
  }
  vmStack().pushBool(isTypeHelper(tvToCell(loc.ptr), op));
}

OPTBLD_INLINE void iopIsTypeC(IsTypeOp op) {
  auto val = vmStack().topC();
  vmStack().replaceC(make_tv<KindOfBoolean>(isTypeHelper(val, op)));
}

OPTBLD_INLINE void iopIsUninit() {
  auto const* cell = vmStack().topC();
  assertx(cellIsPlausible(*cell));
  vmStack().pushBool(cell->m_type == KindOfUninit);
}

OPTBLD_INLINE void iopMaybeMemoType() {
  assertx(vmfp()->m_func->isMemoizeWrapper());
  vmStack().replaceTV(make_tv<KindOfBoolean>(true));
}

OPTBLD_INLINE void iopIsMemoType() {
  assertx(vmfp()->m_func->isMemoizeWrapper());
  vmStack().replaceTV(make_tv<KindOfBoolean>(false));
}

OPTBLD_FLT_INLINE void iopAssertRATL(local_var loc, RepoAuthType rat) {
  if (debug) {
    auto const tv = *loc.ptr;
    auto const func = vmfp()->func();
    auto vm = &*g_context;
    always_assert_flog(
      tvMatchesRepoAuthType(tv, rat),
      "failed assert RATL on local {}: ${} in {}:{}, expected {}, got {}",
      loc.index,
      loc.index < func->numNamedLocals() ?
        func->localNames()[loc.index]->data() : "<unnamed>",
      vm->getContainingFileName()->data(),
      vm->getLine(),
      show(rat),
      toStringElm(&tv)
    );
  }
}

OPTBLD_INLINE void iopAssertRATStk(uint32_t stkSlot, RepoAuthType rat) {
  if (debug) {
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
  }
}

OPTBLD_INLINE void iopBreakTraceHint() {
}

OPTBLD_INLINE void iopEmptyL(local_var loc) {
  bool e = !cellToBool(*tvToCell(loc.ptr));
  vmStack().pushBool(e);
}

OPTBLD_INLINE void iopEmptyN() {
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

OPTBLD_INLINE void iopEmptyG() {
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

OPTBLD_INLINE void iopEmptyS(clsref_slot slot) {
  SpropState ss(vmStack(), slot);
  bool e;
  if (!(ss.visible && ss.accessible)) {
    e = true;
  } else {
    e = !cellToBool(*tvToCell(ss.val));
  }
  ss.output->m_data.num = e;
  ss.output->m_type = KindOfBoolean;
}

OPTBLD_INLINE void iopAKExists() {
  TypedValue* arr = vmStack().topTV();
  TypedValue* key = arr + 1;
  bool result = HHVM_FN(array_key_exists)(tvAsCVarRef(key), tvAsCVarRef(arr));
  vmStack().popTV();
  vmStack().replaceTV<KindOfBoolean>(result);
}

OPTBLD_INLINE void iopGetMemoKeyL(local_var loc) {
  auto const func = vmfp()->m_func;
  assertx(func->isMemoizeWrapper());
  assertx(!func->anyByRef());

  // If this local corresponds to one of the function's parameters, and there's
  // a useful type-hint (which is being enforced), we can use a more efficient
  // memoization scheme based on the range of types we know this local can
  // have. This scheme needs to agree with HHBBC and the JIT.
  using MK = MemoKeyConstraint;
  auto const mkc = [&]{
    if (!RuntimeOption::EvalHardTypeHints) {
      return MK::None;
    }
    if (loc.index >= func->numParams()) return MK::None;
    return memoKeyConstraintFromTC(func->params()[loc.index].typeConstraint);
  }();

  if (UNLIKELY(loc.ptr->m_type == KindOfUninit)) {
    tvWriteNull(*loc.ptr);
    raise_undefined_local(vmfp(), loc.index);
  }

  auto const key = [&](){
    switch (mkc) {
      case MK::Null:
        assertx(loc.ptr->m_type == KindOfNull);
        return make_tv<KindOfInt64>(0);
      case MK::Int:
      case MK::IntOrNull:
        if (loc.ptr->m_type == KindOfInt64) {
          return *loc.ptr;
        } else {
          assertx(loc.ptr->m_type == KindOfNull);
          return make_tv<KindOfPersistentString>(s_nullMemoKey.get());
        }
      case MK::Bool:
      case MK::BoolOrNull:
        if (loc.ptr->m_type == KindOfBoolean) {
          return make_tv<KindOfInt64>(loc.ptr->m_data.num);
        } else {
          assertx(loc.ptr->m_type == KindOfNull);
          return make_tv<KindOfInt64>(2);
        }
      case MK::Str:
      case MK::StrOrNull:
        if (tvIsString(loc.ptr)) {
          tvIncRefGen(*loc.ptr);
          return *loc.ptr;
        } else {
          assertx(loc.ptr->m_type == KindOfNull);
          return make_tv<KindOfInt64>(0);
        }
      case MK::IntOrStr:
        assertx(tvIsString(loc.ptr) || loc.ptr->m_type == KindOfInt64);
        tvIncRefGen(*loc.ptr);
        return *loc.ptr;
      case MK::None:
        // Use the generic scheme, which is performed by
        // serialize_memoize_param.
        return HHVM_FN(serialize_memoize_param)(*loc.ptr);
    }
    not_reached();
  }();

  cellCopy(key, *vmStack().allocC());
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
  return g_context->invokeFuncFew(func, nullptr, nullptr, 3, &args[0]);
}
}

OPTBLD_INLINE void iopIdx() {
  TypedValue* def = vmStack().topTV();
  TypedValue* key = vmStack().indTV(1);
  TypedValue* arr = vmStack().indTV(2);

  TypedValue result;
  if (isArrayLikeType(arr->m_type)) {
    result = HHVM_FN(hphp_array_idx)(tvAsCVarRef(arr),
                                     tvAsCVarRef(key),
                                     tvAsCVarRef(def));
    vmStack().popTV();
  } else if (isNullType(key->m_type)) {
    tvDecRefGen(arr);
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
  tvDecRefGen(arr);
  *arr = result;
}

OPTBLD_INLINE void iopArrayIdx() {
  TypedValue* def = vmStack().topTV();
  TypedValue* key = vmStack().indTV(1);
  TypedValue* arr = vmStack().indTV(2);

  auto const result = HHVM_FN(hphp_array_idx)(tvAsCVarRef(arr),
                                              tvAsCVarRef(key),
                                              tvAsCVarRef(def));
  vmStack().popTV();
  vmStack().popTV();
  tvDecRefGen(arr);
  *arr = result;
}

OPTBLD_INLINE void iopSetL(local_var to) {
  assertx(to.index < vmfp()->m_func->numLocals());
  Cell* fr = vmStack().topC();
  tvSet(*fr, *to);
}

OPTBLD_INLINE void iopSetN() {
  StringData* name;
  Cell* fr = vmStack().topC();
  TypedValue* tv2 = vmStack().indTV(1);
  TypedValue* to = nullptr;
  lookupd_var(vmfp(), name, tv2, to);
  SCOPE_EXIT { decRefStr(name); };
  assertx(to != nullptr);
  tvSet(*fr, *to);
  memcpy((void*)tv2, (void*)fr, sizeof(TypedValue));
  vmStack().discard();
}

OPTBLD_INLINE void iopSetG() {
  StringData* name;
  Cell* fr = vmStack().topC();
  TypedValue* tv2 = vmStack().indTV(1);
  TypedValue* to = nullptr;
  lookupd_gbl(vmfp(), name, tv2, to);
  SCOPE_EXIT { decRefStr(name); };
  assertx(to != nullptr);
  tvSet(*fr, *to);
  memcpy((void*)tv2, (void*)fr, sizeof(TypedValue));
  vmStack().discard();
}

OPTBLD_INLINE void iopSetS(clsref_slot slot) {
  TypedValue* tv1 = vmStack().topTV();
  Class* cls = slot.take();
  TypedValue* propn = vmStack().indTV(1);
  TypedValue* output = propn;
  StringData* name;
  TypedValue* val;
  bool visible, accessible;
  lookup_sprop(vmfp(), cls, name, propn, val, visible, accessible);
  SCOPE_EXIT { decRefStr(name); };
  if (!(visible && accessible)) {
    raise_error("Invalid static property access: %s::%s",
                cls->name()->data(),
                name->data());
  }
  tvSet(*tv1, *val);
  tvDecRefGen(propn);
  memcpy(output, tv1, sizeof(TypedValue));
  vmStack().ndiscard(1);
}

OPTBLD_INLINE void iopSetOpL(local_var loc, SetOpOp op) {
  Cell* fr = vmStack().topC();
  Cell* to = tvToCell(loc.ptr);
  setopBody(to, op, fr);
  tvDecRefGen(fr);
  cellDup(*to, *fr);
}

OPTBLD_INLINE void iopSetOpN(SetOpOp op) {
  Cell* fr = vmStack().topC();
  TypedValue* tv2 = vmStack().indTV(1);
  TypedValue* to = nullptr;
  // XXX We're probably not getting warnings totally correct here
  StringData* name;
  lookupd_var(vmfp(), name, tv2, to);
  SCOPE_EXIT { decRefStr(name); };
  assertx(to != nullptr);
  setopBody(tvToCell(to), op, fr);
  tvDecRefGen(fr);
  tvDecRefGen(tv2);
  cellDup(*tvToCell(to), *tv2);
  vmStack().discard();
}

OPTBLD_INLINE void iopSetOpG(SetOpOp op) {
  StringData* name;
  Cell* fr = vmStack().topC();
  TypedValue* tv2 = vmStack().indTV(1);
  TypedValue* to = nullptr;
  // XXX We're probably not getting warnings totally correct here
  lookupd_gbl(vmfp(), name, tv2, to);
  SCOPE_EXIT { decRefStr(name); };
  assertx(to != nullptr);
  setopBody(tvToCell(to), op, fr);
  tvDecRefGen(fr);
  tvDecRefGen(tv2);
  cellDup(*tvToCell(to), *tv2);
  vmStack().discard();
}

OPTBLD_INLINE void iopSetOpS(SetOpOp op, clsref_slot slot) {
  Cell* fr = vmStack().topC();
  Class* cls = slot.take();
  TypedValue* propn = vmStack().indTV(1);
  TypedValue* output = propn;
  StringData* name;
  TypedValue* val;
  bool visible, accessible;
  lookup_sprop(vmfp(), cls, name, propn, val, visible, accessible);
  SCOPE_EXIT { decRefStr(name); };
  if (!(visible && accessible)) {
    raise_error("Invalid static property access: %s::%s",
                cls->name()->data(),
                name->data());
  }
  setopBody(tvToCell(val), op, fr);
  tvDecRefGen(propn);
  tvDecRefGen(fr);
  cellDup(*tvToCell(val), *output);
  vmStack().ndiscard(1);
}

OPTBLD_INLINE void iopIncDecL(local_var fr, IncDecOp op) {
  TypedValue* to = vmStack().allocTV();
  tvWriteUninit(*to);
  if (UNLIKELY(fr.ptr->m_type == KindOfUninit)) {
    raise_undefined_local(vmfp(), fr.index);
    tvWriteNull(*fr.ptr);
  } else {
    fr.ptr = tvToCell(fr.ptr);
  }
  cellCopy(IncDecBody(op, fr.ptr), *to);
}

OPTBLD_INLINE void iopIncDecN(IncDecOp op) {
  StringData* name;
  TypedValue* nameCell = vmStack().topTV();
  TypedValue* local = nullptr;
  lookupd_var(vmfp(), name, nameCell, local);
  auto oldNameCell = *nameCell;
  SCOPE_EXIT {
    decRefStr(name);
    tvDecRefGen(oldNameCell);
  };
  assertx(local != nullptr);
  cellCopy(IncDecBody(op, tvToCell(local)), *nameCell);
}

OPTBLD_INLINE void iopIncDecG(IncDecOp op) {
  StringData* name;
  TypedValue* nameCell = vmStack().topTV();
  TypedValue* gbl = nullptr;
  lookupd_gbl(vmfp(), name, nameCell, gbl);
  auto oldNameCell = *nameCell;
  SCOPE_EXIT {
    decRefStr(name);
    tvDecRefGen(oldNameCell);
  };
  assertx(gbl != nullptr);
  cellCopy(IncDecBody(op, tvToCell(gbl)), *nameCell);
}

OPTBLD_INLINE void iopIncDecS(IncDecOp op, clsref_slot slot) {
  SpropState ss(vmStack(), slot);
  if (!(ss.visible && ss.accessible)) {
    raise_error("Invalid static property access: %s::%s",
                ss.cls->name()->data(),
                ss.name->data());
  }
  cellCopy(IncDecBody(op, tvToCell(ss.val)), *ss.output);
}

OPTBLD_INLINE void iopBindL(local_var to) {
  Ref* fr = vmStack().topV();
  tvBind(*fr, *to.ptr);
}

OPTBLD_INLINE void iopBindN() {
  StringData* name;
  TypedValue* fr = vmStack().topTV();
  TypedValue* nameTV = vmStack().indTV(1);
  TypedValue* to = nullptr;
  lookupd_var(vmfp(), name, nameTV, to);
  SCOPE_EXIT { decRefStr(name); };
  assertx(to != nullptr);
  tvBind(*fr, *to);
  memcpy((void*)nameTV, (void*)fr, sizeof(TypedValue));
  vmStack().discard();
}

OPTBLD_INLINE void iopBindG() {
  StringData* name;
  TypedValue* fr = vmStack().topTV();
  TypedValue* nameTV = vmStack().indTV(1);
  TypedValue* to = nullptr;
  lookupd_gbl(vmfp(), name, nameTV, to);
  SCOPE_EXIT { decRefStr(name); };
  assertx(to != nullptr);
  tvBind(*fr, *to);
  memcpy((void*)nameTV, (void*)fr, sizeof(TypedValue));
  vmStack().discard();
}

OPTBLD_INLINE void iopBindS(clsref_slot slot) {
  TypedValue* fr = vmStack().topTV();
  Class* cls = slot.take();
  TypedValue* propn = vmStack().indTV(1);
  TypedValue* output = propn;
  StringData* name;
  TypedValue* val;
  bool visible, accessible;
  lookup_sprop(vmfp(), cls, name, propn, val, visible, accessible);
  SCOPE_EXIT { decRefStr(name); };
  if (!(visible && accessible)) {
    raise_error("Invalid static property access: %s::%s",
                cls->name()->data(),
                name->data());
  }
  tvBind(*fr, *val);
  tvDecRefGen(propn);
  memcpy(output, fr, sizeof(TypedValue));
  vmStack().ndiscard(1);
}

OPTBLD_INLINE void iopUnsetL(local_var loc) {
  tvUnset(*loc.ptr);
}

OPTBLD_INLINE void iopUnsetN() {
  StringData* name;
  TypedValue* tv1 = vmStack().topTV();
  TypedValue* tv = nullptr;
  lookup_var(vmfp(), name, tv1, tv);
  SCOPE_EXIT { decRefStr(name); };
  if (tv != nullptr) {
    tvUnset(*tv);
  }
  vmStack().popC();
}

OPTBLD_INLINE void iopUnsetG() {
  TypedValue* tv1 = vmStack().topTV();
  StringData* name = lookup_name(tv1);
  SCOPE_EXIT { decRefStr(name); };
  VarEnv* varEnv = g_context->m_globalVarEnv;
  assertx(varEnv != nullptr);
  varEnv->unset(name);
  vmStack().popC();
}

OPTBLD_INLINE ActRec* fPushFuncImpl(const Func* func, int numArgs) {
  DEBUGGER_IF(phpBreakpointEnabled(func->name()->data()));
  ActRec* ar = vmStack().allocA();
  ar->m_func = func;
  ar->initNumArgs(numArgs);
  ar->trashVarEnv();
  return ar;
}

ALWAYS_INLINE std::string concat_arg_list(imm_array<uint32_t> args) {
  auto const n = args.size;
  assertx(n != 0);
  std::string ret;
  folly::toAppend(args[0], &ret);
  for (int i = 1; i != n; ++i) folly::toAppend(";", args[i], &ret);
  return ret;
}

OPTBLD_INLINE void iopFPushFunc(uint32_t numArgs, imm_array<uint32_t> args) {
  auto const n = args.size;
  std::string arglist;
  if (UNLIKELY(n)) {
    arglist = concat_arg_list(args);
  }

  Cell* c1 = vmStack().topC();
  if (c1->m_type == KindOfObject) {
    // this covers both closures and functors
    static StringData* invokeName = makeStaticString("__invoke");
    ObjectData* origObj = c1->m_data.pobj;
    const Class* cls = origObj->getVMClass();
    auto const func = LIKELY(!n)
      ? cls->lookupMethod(invokeName)
      : cls->lookupMethod(
        makeStaticString(folly::sformat("__invoke${}$inout", arglist))
      );
    if (func == nullptr) {
      raise_error(Strings::FUNCTION_NAME_MUST_BE_STRING);
    }

    vmStack().discard();
    ActRec* ar = fPushFuncImpl(func, numArgs);
    if (func->isStaticInPrologue()) {
      ar->setClass(origObj->getVMClass());
      decRefObj(origObj);
    } else {
      ar->setThis(origObj);
      // Teleport the reference from the destroyed stack cell to the
      // ActRec. Don't try this at home.
    }
    return;
  }

  auto appendSuffix = [&] (const StringData* s) {
    return StringData::Make(s, folly::sformat("${}$inout", arglist));
  };

  if (isArrayLikeType(c1->m_type) || isStringType(c1->m_type)) {
    Variant v = Variant::wrap(*c1);

    // Handle inout name mangling
    if (UNLIKELY(n)) {
      if (isStringType(c1->m_type)) {
        v = Variant::attach(appendSuffix(c1->m_data.pstr));
      } else if (c1->m_data.parr->size() == 2){
        auto s = c1->m_data.parr->at(1);
        if (isStringType(s.m_type)) {
          VArrayInit ai{2};
          ai.append(c1->m_data.parr->at(int64_t(0)));
          ai.append(Variant::attach(appendSuffix(s.m_data.pstr)));
          v = ai.toVariant();
        }
      }
    }

    // support:
    //   array($instance, 'method')
    //   array('Class', 'method'),
    //   vec[$instance, 'method'],
    //   vec['Class', 'method'],
    //   'func_name'
    //   'class::method'
    // which are all valid callables
    auto origCell = *c1;
    ObjectData* thiz = nullptr;
    HPHP::Class* cls = nullptr;
    StringData* invName = nullptr;
    bool dynamic = false;

    auto const func = vm_decode_function(
      v,
      vmfp(),
      /* forwarding */ false,
      thiz,
      cls,
      invName,
      dynamic,
      DecodeFlags::NoWarn
    );
    assertx(dynamic);
    if (func == nullptr) {
      if (isArrayLikeType(origCell.m_type)) {
        raise_error("Invalid callable (array)");
      } else {
        assertx(isStringType(origCell.m_type));
        raise_call_to_undefined(origCell.m_data.pstr);
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
      ar->trashThis();
    }

    ar->setDynamicCall();

    if (UNLIKELY(invName != nullptr)) {
      ar->setMagicDispatch(invName);
    }
    if (isArrayLikeType(origCell.m_type)) {
      decRefArr(origCell.m_data.parr);
    } else if (origCell.m_type == KindOfString) {
      decRefStr(origCell.m_data.pstr);
    }
    return;
  }

  raise_error(Strings::FUNCTION_NAME_MUST_BE_STRING);
}

OPTBLD_FLT_INLINE void iopFPushFuncD(uint32_t numArgs, Id id) {
  const NamedEntityPair nep =
    vmfp()->m_func->unit()->lookupNamedEntityPairId(id);
  Func* func = Unit::loadFunc(nep.second, nep.first);
  if (func == nullptr) {
    raise_call_to_undefined(vmfp()->m_func->unit()->lookupLitstrId(id));
  }
  ActRec* ar = fPushFuncImpl(func, numArgs);
  ar->trashThis();
}

OPTBLD_INLINE void iopFPushFuncU(uint32_t numArgs, Id nsFunc, Id globalFunc) {
  Unit* unit = vmfp()->m_func->unit();
  const NamedEntityPair nep = unit->lookupNamedEntityPairId(nsFunc);
  Func* func = Unit::loadFunc(nep.second, nep.first);
  if (func == nullptr) {
    const NamedEntityPair nep2 = unit->lookupNamedEntityPairId(globalFunc);
    func = Unit::loadFunc(nep2.second, nep2.first);
    if (func == nullptr) {
      raise_call_to_undefined(unit->lookupLitstrId(nsFunc));
    }
  }
  ActRec* ar = fPushFuncImpl(func, numArgs);
  ar->trashThis();
}

void fPushObjMethodImpl(StringData* name,
                        ObjectData* obj,
                        int numArgs,
                        bool dynamic) {
  const Func* f;
  LookupResult res;
  auto cls = obj->getVMClass();
  try {
    res = lookupObjMethod(
      f, cls, name, arGetContextClass(vmfp()), true);
  } catch (...) {
    decRefObj(obj);
    throw;
  }
  assertx(f);
  ActRec* ar = vmStack().allocA();
  ar->m_func = f;
  if (res == LookupResult::MethodFoundNoThis) {
    decRefObj(obj);
    ar->setClass(cls);
  } else {
    assertx(res == LookupResult::MethodFoundWithThis ||
           res == LookupResult::MagicCallFound);
    /* Transfer ownership of obj to the ActRec*/
    ar->setThis(obj);
  }
  ar->initNumArgs(numArgs);
  if (dynamic) ar->setDynamicCall();
  if (res == LookupResult::MagicCallFound) {
    ar->setMagicDispatch(name);
  } else {
    ar->trashVarEnv();
    decRefStr(name);
  }
}

void fPushNullObjMethod(int numArgs) {
  assertx(SystemLib::s_nullFunc);
  ActRec* ar = vmStack().allocA();
  ar->m_func = SystemLib::s_nullFunc;
  ar->trashThis();
  ar->initNumArgs(numArgs);
  ar->trashVarEnv();
  ar->setDynamicCall();
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

ALWAYS_INLINE StringData* mangleInOutName(
  const StringData* name,
  imm_array<uint32_t> args
) {
  return
    StringData::Make(
      name, folly::sformat("${}$inout", concat_arg_list(args))
    );
}

OPTBLD_INLINE void iopFPushObjMethod(uint32_t numArgs, ObjMethodOp op,
                                     imm_array<uint32_t> args) {
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
  StringData* name = c1->m_data.pstr;

  if (UNLIKELY(args.size)) {
    String s = String::attach(name);
    name = mangleInOutName(name, args);
  }

  // We handle decReffing obj and name in fPushObjMethodImpl
  vmStack().ndiscard(2);
  fPushObjMethodImpl(name, obj, numArgs, true);
}

OPTBLD_INLINE void
iopFPushObjMethodD(uint32_t numArgs, const StringData* name, ObjMethodOp op) {
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
  // We handle decReffing obj in fPushObjMethodImpl
  vmStack().discard();
  fPushObjMethodImpl(const_cast<StringData*>(name), obj, numArgs, false);
}

namespace {

void pushClsMethodImpl(Class* cls,
                       StringData* name,
                       int numArgs,
                       bool forwarding,
                       bool dynamic) {
  auto const ctx = liveClass();
  auto obj = ctx && vmfp()->hasThis() ? vmfp()->getThis() : nullptr;
  const Func* f;
  auto const res = lookupClsMethod(f, cls, name, obj, ctx, true);
  if (res == LookupResult::MethodFoundNoThis ||
      res == LookupResult::MagicCallStaticFound) {
    if (!f->isStaticInPrologue()) {
      raise_missing_this(f);
    }
    obj = nullptr;
  } else {
    assertx(obj);
    assertx(res == LookupResult::MethodFoundWithThis ||
           res == LookupResult::MagicCallFound);
    obj->incRefCount();
  }
  assertx(f);
  ActRec* ar = vmStack().allocA();
  ar->m_func = f;
  if (obj) {
    ar->setThis(obj);
  } else {
    if (forwarding && ctx) {
      /* Propagate the current late bound class if there is one, */
      /* otherwise use the class given by this instruction's input */
      if (vmfp()->hasThis()) {
        cls = vmfp()->getThis()->getVMClass();
      } else {
        cls = vmfp()->getClass();
      }
    }
    ar->setClass(cls);
  }
  ar->initNumArgs(numArgs);
  if (dynamic) ar->setDynamicCall();
  if (res == LookupResult::MagicCallFound ||
      res == LookupResult::MagicCallStaticFound) {
    ar->setMagicDispatch(name);
  } else {
    ar->trashVarEnv();
    decRefStr(const_cast<StringData*>(name));
  }
}

Class* specialClsRefToCls(SpecialClsRef ref) {
  switch (ref) {
    case SpecialClsRef::Static:
      if (auto const cls = frameStaticClass(vmfp())) return cls;
      raise_error(HPHP::Strings::CANT_ACCESS_STATIC);
    case SpecialClsRef::Self:
      if (auto const cls = arGetContextClass(vmfp())) return cls;
      raise_error(HPHP::Strings::CANT_ACCESS_SELF);
    case SpecialClsRef::Parent:
      if (auto const cls = arGetContextClass(vmfp())) {
        if (auto const parent = cls->parent()) return parent;
        raise_error(HPHP::Strings::CANT_ACCESS_PARENT_WHEN_NO_PARENT);
      }
      raise_error(HPHP::Strings::CANT_ACCESS_PARENT_WHEN_NO_CLASS);
  }
  always_assert(false);
}

}

OPTBLD_INLINE void iopFPushClsMethod(uint32_t numArgs, clsref_slot slot,
                                     imm_array<uint32_t> args) {
  auto const c1 = vmStack().topC(); // Method name.
  if (!isStringType(c1->m_type)) {
    raise_error(Strings::FUNCTION_NAME_MUST_BE_STRING);
  }
  auto const cls = slot.take();
  auto name = c1->m_data.pstr;

  if (UNLIKELY(args.size)) {
    String s = String::attach(name);
    name = mangleInOutName(name, args);
  }

  // pushClsMethodImpl will take care of decReffing name
  vmStack().ndiscard(1);
  assertx(cls && name);
  pushClsMethodImpl(cls, name, numArgs, false, true);
}

OPTBLD_INLINE
void iopFPushClsMethodD(uint32_t numArgs, const StringData* name, Id classId) {
  const NamedEntityPair &nep =
    vmfp()->m_func->unit()->lookupNamedEntityPairId(classId);
  Class* cls = Unit::loadClass(nep.second, nep.first);
  if (cls == nullptr) {
    raise_error(Strings::UNKNOWN_CLASS, nep.first->data());
  }
  pushClsMethodImpl(cls, const_cast<StringData*>(name), numArgs, false, false);
}

OPTBLD_INLINE void iopFPushClsMethodS(uint32_t numArgs, SpecialClsRef ref,
                                      imm_array<uint32_t> args) {
  auto const c1 = vmStack().topC(); // Method name.
  if (!isStringType(c1->m_type)) {
    raise_error(Strings::FUNCTION_NAME_MUST_BE_STRING);
  }
  auto const cls = specialClsRefToCls(ref);
  auto name = c1->m_data.pstr;

  if (UNLIKELY(args.size)) {
    String s = String::attach(name);
    name = mangleInOutName(name, args);
  }

  // pushClsMethodImpl will take care of decReffing name
  vmStack().ndiscard(1);
  pushClsMethodImpl(
    cls,
    name,
    numArgs,
    ref == SpecialClsRef::Self || ref == SpecialClsRef::Parent,
    true
  );
}

OPTBLD_INLINE void iopFPushClsMethodSD(uint32_t numArgs,
                                       SpecialClsRef ref,
                                       const StringData* name) {
  pushClsMethodImpl(
    specialClsRefToCls(ref),
    const_cast<StringData*>(name),
    numArgs,
    ref == SpecialClsRef::Self || ref == SpecialClsRef::Parent,
    false
  );
}

namespace {

void fpushCtorImpl(uint32_t numArgs, Class* cls, bool dynamic) {
  const Func* f;
  auto const res UNUSED =
    lookupCtorMethod(f, cls, arGetContextClass(vmfp()), true);
  assertx(res == LookupResult::MethodFoundWithThis);
  // Replace input with uninitialized instance.
  auto this_ = newInstance(cls);
  TRACE(2, "FPushCtor: just new'ed an instance of class %s: %p\n",
        cls->name()->data(), this_);
  vmStack().pushObject(this_);
  // Push new activation record.
  auto ar = vmStack().allocA();
  ar->m_func = f;
  ar->setThis(this_);
  ar->initNumArgs(numArgs);
  if (dynamic) ar->setDynamicCall();
  ar->trashVarEnv();
}

}

OPTBLD_INLINE void iopFPushCtor(uint32_t numArgs, clsref_slot slot) {
  fpushCtorImpl(numArgs, slot.take(), true);
}

OPTBLD_INLINE void iopFPushCtorD(uint32_t numArgs, Id id) {
  const NamedEntityPair &nep =
    vmfp()->m_func->unit()->lookupNamedEntityPairId(id);
  auto cls = Unit::loadClass(nep.second, nep.first);
  if (cls == nullptr) {
    raise_error(Strings::UNKNOWN_CLASS,
                vmfp()->m_func->unit()->lookupLitstrId(id)->data());
  }
  fpushCtorImpl(numArgs, cls, false);
}

OPTBLD_INLINE void iopFPushCtorI(uint32_t numArgs, uint32_t clsIx) {
  auto const func = vmfp()->m_func;
  auto const preCls = func->unit()->lookupPreClassId(clsIx);
  auto const cls = Unit::defClass(preCls, true);
  fpushCtorImpl(numArgs, cls, false);
}

OPTBLD_INLINE void iopFPushCtorS(uint32_t numArgs, SpecialClsRef ref) {
  fpushCtorImpl(numArgs, specialClsRefToCls(ref), false);
}

OPTBLD_INLINE
void iopDecodeCufIter(PC& pc, Iter* it, PC takenpc) {
  CufIter &cit = it->cuf();

  ObjectData* obj = nullptr;
  HPHP::Class* cls = nullptr;
  StringData* invName = nullptr;
  bool dynamic = false;
  TypedValue *func = vmStack().topTV();

  ActRec* ar = vmfp();
  if (vmfp()->m_func->isBuiltin()) {
    ar = g_context->getOuterVMFrame(ar);
  }
  const Func* f = vm_decode_function(tvAsVariant(func),
                                     ar, false,
                                     obj, cls, invName,
                                     dynamic,
                                     DecodeFlags::NoWarn);

  if (f == nullptr) {
    pc = takenpc;
  } else {
    cit.setFunc(f);
    if (obj) {
      cit.setCtx(obj);
      obj->incRefCount();
    } else {
      cit.setCtx(cls);
    }
    cit.setName(invName);
    cit.setDynamic(dynamic);
  }
  vmStack().popC();
}

OPTBLD_INLINE void iopFPushCufIter(uint32_t numArgs, Iter* it) {
  auto f = it->cuf().func();
  auto o = it->cuf().ctx();
  auto n = it->cuf().name();
  auto d = it->cuf().dynamic();

  ActRec* ar = vmStack().allocA();
  ar->m_func = f;
  assertx((f->implCls() != nullptr) == (o != nullptr));
  if (o) {
    ar->setThisOrClass(o);
    if (ActRec::checkThis(o)) ar->getThis()->incRefCount();
  } else {
    ar->trashThis();
  }
  ar->initNumArgs(numArgs);
  if (d) ar->setDynamicCall();
  if (n) {
    ar->setMagicDispatch(n);
    n->incRefCount();
  } else {
    ar->trashVarEnv();
  }
}

OPTBLD_INLINE void iopFThrowOnRefMismatch(ActRec* ar, imm_array<bool> byRefs) {
  assertx(byRefs.size <= ar->numArgs());
  auto const func = ar->func();
  for (auto i = 0; i < byRefs.size; ++i) {
    auto const byRef = func->byRef(i);
    if (byRef != byRefs[i]) {
      SystemLib::throwInvalidArgumentExceptionObject(
        formatParamRefMismatch(func->fullDisplayName()->data(), i, byRef));
    }
  }
}

OPTBLD_INLINE void iopFHandleRefMismatch(uint32_t paramId, FPassHint hint,
                                         const StringData* funcName) {
  assertx(hint != FPassHint::Any);
  raiseParamRefMismatchForFuncName(funcName, paramId, hint == FPassHint::Cell);
}

OPTBLD_INLINE void iopFPassC(ActRec* ar, uint32_t paramId, FPassHint hint) {
  checkFPassHint(ar, paramId, hint);
}

OPTBLD_INLINE void iopFPassV(ActRec* ar, uint32_t paramId, FPassHint hint) {
  checkFPassHint(ar, paramId, hint);
  const Func* func = ar->m_func;
  if (!func->byRef(paramId)) {
    vmStack().unbox();
  }
}

OPTBLD_INLINE void iopFPassVNop(ActRec* ar, uint32_t paramId, FPassHint hint) {
  checkFPassHint(ar, paramId, hint);
}

OPTBLD_INLINE void iopFPassR(ActRec* ar, uint32_t paramId, FPassHint hint) {
  checkFPassHint(ar, paramId, hint);
  const Func* func = ar->m_func;
  if (func->byRef(paramId)) {
    TypedValue* tv = vmStack().topTV();
    if (!isRefType(tv->m_type)) {
      tvBox(*tv);
    }
  } else {
    if (isRefType(vmStack().topTV()->m_type)) {
      vmStack().unbox();
    }
  }
}

OPTBLD_INLINE void iopFPassL(ActRec* ar, uint32_t paramId, local_var loc,
                             FPassHint hint) {
  checkFPassHint(ar, paramId, hint);
  TypedValue* fr = loc.ptr;
  TypedValue* to = vmStack().allocTV();
  if (!ar->m_func->byRef(paramId)) {
    cgetl_body(vmfp(), fr, to, loc.index, true);
  } else {
    vgetl_body(fr, to);
  }
}

OPTBLD_INLINE void iopFPassN(ActRec* ar, uint32_t paramId, FPassHint hint) {
  checkFPassHint(ar, paramId, hint);
  if (!ar->m_func->byRef(paramId)) {
    iopCGetN();
  } else {
    iopVGetN();
  }
}

OPTBLD_INLINE void iopFPassG(ActRec* ar, uint32_t paramId, FPassHint hint) {
  checkFPassHint(ar, paramId, hint);
  if (!ar->m_func->byRef(paramId)) {
    iopCGetG();
  } else {
    iopVGetG();
  }
}

OPTBLD_INLINE void iopFPassS(ActRec* ar, uint32_t paramId, clsref_slot slot,
                             FPassHint hint) {
  checkFPassHint(ar, paramId, hint);
  if (!ar->m_func->byRef(paramId)) {
    iopCGetS(slot);
  } else {
    iopVGetS(slot);
  }
}

bool doFCall(ActRec* ar, PC& pc) {
  TRACE(3, "FCall: pc %p func %p base %d\n", vmpc(),
        vmfp()->m_func->unit()->entry(),
        int(vmfp()->m_func->base()));
  prepareFuncEntry(ar, pc, StackArgsState::Untrimmed);
  vmpc() = pc;
  if (!EventHook::FunctionCall(ar, EventHook::NormalFunc)) {
    pc = vmpc();
    return false;
  }
  calleeDynamicCallChecks(ar);
  checkForRequiredCallM(ar);
  return true;
}

OPTBLD_INLINE void iopFCall(PC& pc, ActRec* ar, uint32_t numArgs) {
  assertx(numArgs == ar->numArgs());
  if (ar->isDynamicCall()) callerDynamicCallChecks(ar->func());
  checkStack(vmStack(), ar->m_func, 0);
  ar->setReturn(vmfp(), pc, jit::tc::ustubs().retHelper);
  doFCall(ar, pc);
}

OPTBLD_FLT_INLINE
void iopFCallD(PC& pc, ActRec* ar, uint32_t numArgs,
               const StringData* /*clsName*/, const StringData* funcName) {
  if (!RuntimeOption::EvalJitEnableRenameFunction &&
      !(ar->m_func->attrs() & AttrInterceptable)) {
    assertx(ar->m_func->name()->isame(funcName) ||
            (funcName == s_construct.get() &&
             ar->m_func == ar->m_func->cls()->getCtor()));
  }
  assertx(numArgs == ar->numArgs());
  if (ar->isDynamicCall()) callerDynamicCallChecks(ar->func());
  checkStack(vmStack(), ar->m_func, 0);
  ar->setReturn(vmfp(), pc, jit::tc::ustubs().retHelper);
  doFCall(ar, pc);
}

OPTBLD_INLINE
void iopFCallAwait(PC& pc, ActRec* ar, uint32_t numArgs,
                   const StringData* /*clsName*/, const StringData* funcName) {
  if (!RuntimeOption::EvalJitEnableRenameFunction &&
      !(ar->m_func->attrs() & AttrInterceptable)) {
    assertx(ar->m_func->name()->isame(funcName));
  }
  assertx(numArgs == ar->numArgs());
  if (ar->isDynamicCall()) callerDynamicCallChecks(ar->func());
  checkStack(vmStack(), ar->m_func, 0);
  ar->setReturn(vmfp(), pc, jit::tc::ustubs().retHelper);
  ar->setFCallAwait();
  doFCall(ar, pc);
}

OPTBLD_FLT_INLINE
void iopFCallBuiltin(uint32_t numArgs, uint32_t numNonDefault, Id id) {
  const NamedEntity* ne = vmfp()->m_func->unit()->lookupNamedEntityId(id);
  Func* func = Unit::lookupFunc(ne);
  if (func == nullptr) {
    raise_error("Call to undefined function %s()",
                vmfp()->m_func->unit()->lookupLitstrId(id)->data());
  }

  TypedValue* args = vmStack().indTV(numArgs-1);
  TypedValue ret;
  if (Native::coerceFCallArgs(args, numArgs, numNonDefault, func)) {
    if (func->hasVariadicCaptureParam()) {
      assertx(numArgs > 0);
      assertx(
        RuntimeOption::EvalHackArrDVArrs
          ? isVecType(args[1 - safe_cast<int32_t>(numArgs)].m_type)
          : isArrayType(args[1 - safe_cast<int32_t>(numArgs)].m_type)
      );
    }
    Native::callFunc<true>(func, nullptr, args, numNonDefault, ret);
  } else {
    if (func->attrs() & AttrParamCoerceModeNull) {
      ret.m_type = KindOfNull;
    } else {
      assertx(func->attrs() & AttrParamCoerceModeFalse);
      ret.m_type = KindOfBoolean;
      ret.m_data.num = 0;
    }
  }

  frame_free_args(args, numNonDefault);
  vmStack().ndiscard(numArgs);
  tvCopy(ret, *vmStack().allocTV());
}

static bool doFCallUnpack(PC& pc, ActRec* ar, int numStackValues,
                          void* ret = nullptr) {
  assertx(numStackValues >= 1);
  assertx(ar->numArgs() == numStackValues);

  Cell* c1 = vmStack().topC();
  if (UNLIKELY(!isContainer(*c1))) {
    Cell tmp = *c1;
    // argument_unpacking RFC dictates "containers and Traversables"
    raise_warning_unsampled("Only containers may be unpacked");
    *c1 = make_persistent_array_like_tv(staticEmptyVArray());
    tvDecRefGen(&tmp);
  }

  const Func* func = ar->m_func;
  {
    Cell args = *c1;
    vmStack().discard(); // prepareArrayArgs will push arguments onto the stack
    numStackValues--;
    SCOPE_EXIT { tvDecRefGen(&args); };
    checkStack(vmStack(), func, 0);

    assertx(!ar->resumed());
    TRACE(3, "FCallUnpack: pc %p func %p base %d\n", vmpc(),
          vmfp()->unit()->entry(),
          int(vmfp()->m_func->base()));
    ar->setReturn(vmfp(), pc, jit::tc::ustubs().retHelper);

    // When called from the jit, populate the correct return address
    if (ret) {
      ar->setJitReturn(ret);
    }

    auto prepResult = prepareArrayArgs(ar, args, vmStack(), numStackValues,
                                       nullptr, /* check ref annot */ true);
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
  calleeDynamicCallChecks(ar);
  checkForRequiredCallM(ar);
  return true;
}

bool doFCallUnpackTC(PC pc, int32_t numArgs, void* retAddr) {
  assert_native_stack_aligned();
  assertx(tl_regState == VMRegState::DIRTY);
  tl_regState = VMRegState::CLEAN;
  auto const ret = doFCallUnpack(pc, arFromSp(numArgs), numArgs, retAddr);
  tl_regState = VMRegState::DIRTY;
  return ret;
}

OPTBLD_INLINE void iopFCallUnpack(PC& pc, ActRec* ar, uint32_t numArgs) {
  assertx(numArgs == ar->numArgs());
  if (ar->isDynamicCall()) callerDynamicCallChecks(ar->func());
  checkStack(vmStack(), ar->m_func, 0);
  doFCallUnpack(pc, ar, numArgs);
}

OPTBLD_FLT_INLINE
void iopFCallM(PC& pc, ActRec* ar, uint32_t numArgs, uint32_t /* numRet */) {
  ar->setFCallM();
  iopFCall(pc, ar, numArgs);
}

OPTBLD_FLT_INLINE
void iopFCallDM(PC& pc, ActRec* ar, uint32_t numArgs, uint32_t /* numRet */,
                const StringData* clsName, const StringData* funcName) {
  ar->setFCallM();
  iopFCallD(pc, ar, numArgs, clsName, funcName);
}

OPTBLD_INLINE void iopFCallUnpackM(PC& pc, ActRec* ar, uint32_t numArgs,
                                   uint32_t /* numRet */) {
  ar->setFCallM();
  iopFCallUnpack(pc, ar, numArgs);
}

namespace {

template <bool Local, bool Pop>
bool initIterator(PC& pc, PC targetpc, Iter* it, Cell* c1) {
  auto const hasElems = it->init<Local>(c1);
  if (!hasElems) pc = targetpc;
  if (Pop) vmStack().popC();
  return hasElems;
}

}

OPTBLD_INLINE void iopIterInit(PC& pc, Iter* it, PC targetpc, local_var val) {
  Cell* c1 = vmStack().topC();
  if (initIterator<false, true>(pc, targetpc, it, c1)) {
    tvAsVariant(val.ptr) = it->arr().second();
  }
}

OPTBLD_INLINE
void iopIterInitK(PC& pc, Iter* it, PC targetpc, local_var val, local_var key) {
  Cell* c1 = vmStack().topC();
  if (initIterator<false, true>(pc, targetpc, it, c1)) {
    tvAsVariant(val.ptr) = it->arr().second();
    tvAsVariant(key.ptr) = it->arr().first();
  }
}

OPTBLD_INLINE void iopLIterInit(PC& pc, Iter* it, local_var local,
                                PC targetpc, local_var val) {
  if (isArrayLikeType(local.ptr->m_type)) {
    if (initIterator<true, false>(pc, targetpc, it, tvAssertCell(local.ptr))) {
      tvAsVariant(val.ptr) = it->arr().secondLocal(local.ptr->m_data.parr);
    }
    return;
  }

  if (initIterator<false, false>(pc, targetpc, it, tvToCell(local.ptr))) {
    tvAsVariant(val.ptr) = it->arr().second();
  }
}

OPTBLD_INLINE void iopLIterInitK(PC& pc, Iter* it, local_var local,
                                 PC targetpc, local_var val, local_var key) {
  if (isArrayLikeType(local.ptr->m_type)) {
    if (initIterator<true, false>(pc, targetpc, it, tvAssertCell(local.ptr))) {
      tvAsVariant(val.ptr) = it->arr().secondLocal(local.ptr->m_data.parr);
      tvAsVariant(key.ptr) = it->arr().firstLocal(local.ptr->m_data.parr);
    }
    return;
  }

  if (initIterator<false, false>(pc, targetpc, it, tvToCell(local.ptr))) {
    tvAsVariant(val.ptr) = it->arr().second();
    tvAsVariant(key.ptr) = it->arr().first();
  }
}

OPTBLD_INLINE void iopWIterInit(PC& pc, Iter* it, PC targetpc, local_var val) {
  Cell* c1 = vmStack().topC();
  if (initIterator<false, true>(pc, targetpc, it, c1)) {
    tvAsVariant(val.ptr).setWithRef(it->arr().secondValPlus());
  }
}

OPTBLD_INLINE void
iopWIterInitK(PC& pc, Iter* it, PC targetpc, local_var val, local_var key) {
  Cell* c1 = vmStack().topC();
  if (initIterator<false, true>(pc, targetpc, it, c1)) {
    tvAsVariant(val.ptr).setWithRef(it->arr().secondValPlus());
    tvAsVariant(key.ptr) = it->arr().first();
  }
}

inline bool initIteratorM(Iter* it, Ref* r1, TypedValue *val, TypedValue *key) {
  TypedValue* rtv = r1->m_data.pref->tv();
  if (isArrayLikeType(rtv->m_type)) {
    return new_miter_array_key(it, r1->m_data.pref, val, key);
  }
  if (rtv->m_type == KindOfObject)  {
    Class* ctx = arGetContextClass(vmfp());
    return new_miter_object(it, r1->m_data.pref, ctx, val, key);
  }
  return new_miter_other(it, r1->m_data.pref);
}

OPTBLD_INLINE void iopMIterInit(PC& pc, Iter* it, PC targetpc, local_var val) {
  Ref* r1 = vmStack().topV();
  assertx(isRefType(r1->m_type));
  if (!initIteratorM(it, r1, val.ptr, nullptr)) {
    pc = targetpc; // nothing to iterate; exit foreach loop.
  }
  vmStack().popV();
}

OPTBLD_INLINE void
iopMIterInitK(PC& pc, Iter* it, PC targetpc, local_var val, local_var key) {
  Ref* r1 = vmStack().topV();
  assertx(isRefType(r1->m_type));
  if (!initIteratorM(it, r1, val.ptr, key.ptr)) {
    pc = targetpc; // nothing to iterate; exit foreach loop.
  }
  vmStack().popV();
}

OPTBLD_INLINE void iopIterNext(PC& pc, Iter* it, PC targetpc, local_var val) {
  jmpSurpriseCheck(targetpc - pc);
  if (it->next()) {
    pc = targetpc;
    tvAsVariant(val.ptr) = it->arr().second();
  }
}

OPTBLD_INLINE
void iopIterNextK(PC& pc, Iter* it, PC targetpc, local_var val, local_var key) {
  jmpSurpriseCheck(targetpc - pc);
  if (it->next()) {
    pc = targetpc;
    tvAsVariant(val.ptr) = it->arr().second();
    tvAsVariant(key.ptr) = it->arr().first();
  }
}

OPTBLD_INLINE void iopLIterNext(PC& pc,
                                Iter* it,
                                local_var base,
                                PC targetpc,
                                local_var val) {
  jmpSurpriseCheck(targetpc - pc);
  if (isArrayLikeType(base.ptr->m_type)) {
    if (it->nextLocal(base.ptr->m_data.parr)) {
      pc = targetpc;
      tvAsVariant(val.ptr) = it->arr().secondLocal(base.ptr->m_data.parr);
    }
  } else if (it->next()) {
    pc = targetpc;
    tvAsVariant(val.ptr) = it->arr().second();
  }
}

OPTBLD_INLINE void iopLIterNextK(PC& pc,
                                 Iter* it,
                                 local_var base,
                                 PC targetpc,
                                 local_var val,
                                 local_var key) {
  jmpSurpriseCheck(targetpc - pc);
  if (isArrayLikeType(base.ptr->m_type)) {
    if (it->nextLocal(base.ptr->m_data.parr)) {
      pc = targetpc;
      tvAsVariant(val.ptr) = it->arr().secondLocal(base.ptr->m_data.parr);
      tvAsVariant(key.ptr) = it->arr().firstLocal(base.ptr->m_data.parr);
    }
  } else if (it->next()) {
    pc = targetpc;
    tvAsVariant(val.ptr) = it->arr().second();
    tvAsVariant(key.ptr) = it->arr().first();
  }
}

OPTBLD_INLINE void iopWIterNext(PC& pc, Iter* it, PC targetpc, local_var val) {
  jmpSurpriseCheck(targetpc - pc);
  if (it->next()) {
    pc = targetpc;
    tvAsVariant(val.ptr).setWithRef(it->arr().secondValPlus());
  }
}

OPTBLD_INLINE void
iopWIterNextK(PC& pc, Iter* it, PC targetpc, local_var val, local_var key) {
  jmpSurpriseCheck(targetpc - pc);
  if (it->next()) {
    pc = targetpc;
    tvAsVariant(val.ptr).setWithRef(it->arr().secondValPlus());
    tvAsVariant(key.ptr) = it->arr().first();
  }
}

OPTBLD_INLINE void iopMIterNext(PC& pc, Iter* it, PC targetpc, local_var val) {
  jmpSurpriseCheck(targetpc - pc);
  if (miter_next_key(it, val.ptr, nullptr)) {
    pc = targetpc;
  }
}

OPTBLD_INLINE void
iopMIterNextK(PC& pc, Iter* it, PC targetpc, local_var val, local_var key) {
  jmpSurpriseCheck(targetpc - pc);
  if (miter_next_key(it, val.ptr, key.ptr)) {
    pc = targetpc;
  }
}

OPTBLD_INLINE void iopIterFree(Iter* it) {
  it->free();
}

OPTBLD_INLINE void iopLIterFree(Iter* it, local_var) {
  it->free();
}

OPTBLD_INLINE void iopMIterFree(Iter* it) {
  it->mfree();
}

OPTBLD_INLINE void iopCIterFree(Iter* it) {
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

OPTBLD_INLINE void iopIncl(PC& pc) {
  inclOp(pc, InclOpFlags::Default, "include");
}

OPTBLD_INLINE void iopInclOnce(PC& pc) {
  inclOp(pc, InclOpFlags::Once, "include_once");
}

OPTBLD_INLINE void iopReq(PC& pc) {
  inclOp(pc, InclOpFlags::Fatal, "require");
}

OPTBLD_INLINE void iopReqOnce(PC& pc) {
  inclOp(pc, InclOpFlags::Fatal | InclOpFlags::Once, "require_once");
}

OPTBLD_INLINE void iopReqDoc(PC& pc) {
  inclOp(
    pc,
    InclOpFlags::Fatal | InclOpFlags::Once | InclOpFlags::DocRoot,
    "require_once"
  );
}

OPTBLD_INLINE void iopEval(PC& pc) {
  Cell* c1 = vmStack().topC();

  if (UNLIKELY(RuntimeOption::EvalAuthoritativeMode)) {
    // Ahead of time whole program optimizations need to assume it can
    // see all the code, or it really can't do much.
    raise_error("You can't use eval in RepoAuthoritative mode");
  }

  auto code = String::attach(prepareKey(*c1));
  String prefixedCode = concat(
    vmfp()->unit()->isHHFile() ? "<?hh " : "<?php ",
    code
  );

  auto evalFilename = std::string();
  auto vm = &*g_context;
  string_printf(
    evalFilename,
    "%s(%d)(%s" EVAL_FILENAME_SUFFIX,
    vm->getContainingFileName()->data(),
    vm->getLine(),
    string_md5(code.slice()).c_str()
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

OPTBLD_INLINE void iopDefFunc(uint32_t fid) {
  Func* f = vmfp()->m_func->unit()->lookupFuncId(fid);
  Unit::defFunc(f, isDebuggerAttached());
}

OPTBLD_INLINE void iopDefCls(uint32_t cid) {
  PreClass* c = vmfp()->m_func->unit()->lookupPreClassId(cid);
  Unit::defClass(c);
}

OPTBLD_INLINE void iopAliasCls(const StringData* original,
                               const StringData* alias) {
  TypedValue* aloadTV = vmStack().topTV();
  tvCastToBooleanInPlace(aloadTV);
  assertx(aloadTV->m_type == KindOfBoolean);
  bool autoload = aloadTV->m_data.num;
  vmStack().popX();

  vmStack().pushBool(Unit::aliasClass(original, alias, autoload));
}

OPTBLD_INLINE void iopDefClsNop(uint32_t /*cid*/) {}

OPTBLD_INLINE void iopDefTypeAlias(uint32_t tid) {
  vmfp()->func()->unit()->defTypeAlias(tid);
}

static inline void checkThis(ActRec* fp) {
  if (!fp->func()->cls() || !fp->hasThis()) {
    raise_error(Strings::FATAL_NULL_THIS);
  }
}

OPTBLD_INLINE void iopThis() {
  checkThis(vmfp());
  ObjectData* this_ = vmfp()->getThis();
  vmStack().pushObject(this_);
}

OPTBLD_INLINE void iopBareThis(BareThisOp bto) {
  if (vmfp()->func()->cls() && vmfp()->hasThis()) {
    ObjectData* this_ = vmfp()->getThis();
    vmStack().pushObject(this_);
  } else {
    vmStack().pushNull();
    switch (bto) {
    case BareThisOp::Notice:   raise_notice(Strings::WARN_NULL_THIS); break;
    case BareThisOp::NoNotice: break;
    case BareThisOp::NeverNull:
      assertx(!"$this cannot be null in BareThis with NeverNull option");
      break;
    }
  }
}

OPTBLD_INLINE void iopCheckThis() {
  checkThis(vmfp());
}

OPTBLD_INLINE void iopInitThisLoc(local_var thisLoc) {
  tvDecRefGen(thisLoc.ptr);
  if (vmfp()->func()->cls() && vmfp()->hasThis()) {
    thisLoc->m_data.pobj = vmfp()->getThis();
    thisLoc->m_type = KindOfObject;
    tvIncRefCountable(*thisLoc.ptr);
  } else {
    tvWriteUninit(*thisLoc.ptr);
  }
}

static inline TypedValue* lookupClosureStatic(const StringData* name,
                                              const ActRec* fp) {
  auto const func = fp->m_func;

  assertx(func->isClosureBody());
  assertx(!func->hasVariadicCaptureParam());
  auto const obj = frame_local(fp, func->numParams())->m_data.pobj;

  return lookupStaticTvFromClosure(obj, name);
}

OPTBLD_INLINE void iopStaticLocCheck(local_var loc, const StringData* var) {
  auto const func = vmfp()->m_func;

  auto ref = [&] () -> RefData* {
    if (UNLIKELY(func->isClosureBody())) {
      auto const val = lookupClosureStatic(var, vmfp());
      if (val->m_type == KindOfUninit) {
        return nullptr;
      }
      assertx(isRefType(val->m_type));
      return val->m_data.pref;
    }

    auto const staticLocalData = rds::bindStaticLocal(func, var);
    if (!staticLocalData.isInit()) {
      return nullptr;
    }

    return &staticLocalData->ref;
  }();

  if (!ref) return vmStack().pushBool(false);

  auto const tmpTV = make_tv<KindOfRef>(ref);
  tvBind(tmpTV, *loc.ptr);
  vmStack().pushBool(true);
}

OPTBLD_INLINE void iopStaticLocDef(local_var loc, const StringData* var) {
  auto const func = vmfp()->m_func;
  auto const initVal = vmStack().topC();

  auto ref = [&] () -> RefData* {
    if (UNLIKELY(func->isClosureBody())) {
      auto const val = lookupClosureStatic(var, vmfp());
      assertx(val->m_type == KindOfUninit);
      cellCopy(*initVal, *val);
      tvBox(*val);
      return val->m_data.pref;
    }

    auto const staticLocalData = rds::bindStaticLocal(func, var);
    if (LIKELY(!staticLocalData.isInit())) {
      staticLocalData->ref.initInRDS();
      staticLocalData.markInit();
      cellCopy(*initVal, *staticLocalData->ref.tv());
    } else {
      cellMove(*initVal, *staticLocalData->ref.tv());
    }
    return &staticLocalData->ref;
  }();

  auto const tmpTV = make_tv<KindOfRef>(ref);
  tvBind(tmpTV, *loc.ptr);
  vmStack().discard();
}

OPTBLD_INLINE void iopStaticLocInit(local_var loc, const StringData* var) {
  auto const func = vmfp()->m_func;
  auto const initVal = vmStack().topC();

  auto ref = [&] () -> RefData* {
    if (UNLIKELY(func->isClosureBody())) {
      auto const val = lookupClosureStatic(var, vmfp());
      if (val->m_type == KindOfUninit) {
        cellCopy(*initVal, *val);
        tvBox(*val);
      }
      return val->m_data.pref;
    }

    auto const staticLocalData = rds::bindStaticLocal(func, var);
    if (!staticLocalData.isInit()) {
      staticLocalData->ref.initInRDS();
      staticLocalData.markInit();
      cellCopy(*initVal, *staticLocalData->ref.tv());
    }
    return &staticLocalData->ref;
  }();

  auto const tmpTV = make_tv<KindOfRef>(ref);
  tvBind(tmpTV, *loc.ptr);
  vmStack().discard();
}

OPTBLD_INLINE void iopCatch() {
  auto vm = &*g_context;
  assertx(vm->m_faults.size() > 0);
  Fault fault = vm->m_faults.back();
  vm->m_faults.pop_back();
  assertx(fault.m_raiseFrame == vmfp());
  assertx(fault.m_userException);
  vmStack().pushObjectNoRc(fault.m_userException);
}

OPTBLD_INLINE void iopChainFaults() {
  auto const current = *vmStack().indC(1);
  auto const prev = *vmStack().indC(0);
  if (!isObjectType(current.m_type) ||
      !current.m_data.pobj->instanceof(SystemLib::s_ThrowableClass) ||
      !isObjectType(prev.m_type) ||
      !prev.m_data.pobj->instanceof(SystemLib::s_ThrowableClass)) {
    raise_error(
      "Inputs to ChainFault must be objects that implement Throwable"
    );
  }

  // chainFaultObjects takes ownership of a reference to prev.
  vmStack().discard();
  chainFaultObjects(current.m_data.pobj, prev.m_data.pobj);
}

OPTBLD_INLINE void iopLateBoundCls(clsref_slot slot) {
  Class* cls = frameStaticClass(vmfp());
  if (!cls) {
    raise_error(HPHP::Strings::CANT_ACCESS_STATIC);
  }
  slot.put(cls);
}

OPTBLD_INLINE void iopVerifyParamType(local_var param) {
  const Func *func = vmfp()->m_func;
  assertx(param.index < func->numParams());
  assertx(func->numParams() == int(func->params().size()));
  const TypeConstraint& tc = func->params()[param.index].typeConstraint;
  assertx(tc.hasConstraint());
  if (!tc.isTypeVar() && !tc.isTypeConstant()) {
    tc.verifyParam(param.ptr, func, param.index);
  }
}

OPTBLD_INLINE void iopVerifyOutType(uint32_t paramId) {
  auto const func = vmfp()->m_func;
  assertx(paramId < func->numParams());
  assertx(func->numParams() == int(func->params().size()));
  auto const& tc = func->params()[paramId].typeConstraint;
  assertx(tc.hasConstraint());
  if (!tc.isTypeVar() && !tc.isTypeConstant()) {
    tc.verifyOutParam(vmStack().topTV(), func, paramId);
  }
}

OPTBLD_INLINE void implVerifyRetType() {
  if (UNLIKELY(!RuntimeOption::EvalCheckReturnTypeHints)) {
    return;
  }

  const auto func = vmfp()->m_func;
  const auto tc = func->returnTypeConstraint();
  if (!tc.isTypeVar() && !tc.isTypeConstant()) {
    tc.verifyReturn(vmStack().topTV(), func);
  }
}

OPTBLD_INLINE void iopVerifyRetTypeC() {
  implVerifyRetType();
}

OPTBLD_INLINE void iopVerifyRetTypeV() {
  implVerifyRetType();
}

OPTBLD_INLINE void iopVerifyRetNonNullC() {
  if (UNLIKELY(!RuntimeOption::EvalCheckReturnTypeHints)) {
    return;
  }
  const auto func = vmfp()->m_func;
  const auto tc = func->returnTypeConstraint();
  tc.verifyReturnNonNull(vmStack().topTV(), func);
}

OPTBLD_INLINE TCA iopNativeImpl(PC& pc) {
  auto const jitReturn = jitReturnPre(vmfp());

  BuiltinFunction func = vmfp()->func()->builtinFuncPtr();
  assertx(func);
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

OPTBLD_INLINE void iopSelf(clsref_slot slot) {
  Class* clss = arGetContextClass(vmfp());
  if (!clss) {
    raise_error(HPHP::Strings::CANT_ACCESS_SELF);
  }
  slot.put(clss);
}

OPTBLD_INLINE void iopParent(clsref_slot slot) {
  Class* clss = arGetContextClass(vmfp());
  if (!clss) {
    raise_error(HPHP::Strings::CANT_ACCESS_PARENT_WHEN_NO_CLASS);
  }
  Class* parent = clss->parent();
  if (!parent) {
    raise_error(HPHP::Strings::CANT_ACCESS_PARENT_WHEN_NO_PARENT);
  }
  slot.put(parent);
}

OPTBLD_INLINE void iopCreateCl(uint32_t numArgs, uint32_t clsIx) {
  auto const func = vmfp()->m_func;
  auto const preCls = func->unit()->lookupPreClassId(clsIx);
  auto const c = Unit::defClosure(preCls);

  auto const cls = c->rescope(const_cast<Class*>(func->cls()));
  auto obj = newInstance(cls);
  c_Closure::fromObject(obj)->init(numArgs, vmfp(), vmStack().top());
  vmStack().ndiscard(numArgs);
  vmStack().pushObjectNoRc(obj);
}

static inline BaseGenerator* this_base_generator(const ActRec* fp) {
  auto const obj = fp->getThis();
  assertx(obj->getVMClass() == AsyncGenerator::getClass() ||
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

OPTBLD_INLINE TCA iopCreateCont(PC& pc) {
  auto const jitReturn = jitReturnPre(vmfp());

  auto const fp = vmfp();
  auto const func = fp->func();
  auto const numSlots = func->numSlotsInFrame();
  auto const resumeOffset = func->unit()->offsetOf(pc);
  assertx(!fp->resumed());
  assertx(func->isGenerator());

  // Create the {Async,}Generator object. Create takes care of copying local
  // variables and iterators.
  auto const obj = func->isAsync()
    ? AsyncGenerator::Create(fp, numSlots, nullptr, resumeOffset)
    : Generator::Create(fp, numSlots, nullptr, resumeOffset);

  auto const genData = func->isAsync() ?
    static_cast<BaseGenerator*>(AsyncGenerator::fromObject(obj)) :
    static_cast<BaseGenerator*>(Generator::fromObject(obj));

  EventHook::FunctionSuspendCreateCont(fp, genData->actRec());

  // Grab caller info from ActRec.
  ActRec* sfp = fp->sfp();
  Offset soff = fp->m_soff;

  // Free ActRec and store the return value.
  vmStack().ndiscard(numSlots);
  vmStack().ret();
  tvCopy(make_tv<KindOfObject>(obj), *vmStack().topTV());
  assertx(vmStack().topTV() == fp->retSlot());

  // Return control to the caller.
  vmfp() = sfp;
  pc = LIKELY(sfp != nullptr) ? sfp->func()->getEntry() + soff : nullptr;

  return jitReturnPost(jitReturn);
}

OPTBLD_INLINE void moveProgramCounterIntoGenerator(PC &pc, BaseGenerator* gen) {
  assertx(gen->isRunning());
  ActRec* genAR = gen->actRec();
  genAR->setReturn(vmfp(), pc, genAR->func()->isAsync() ?
    jit::tc::ustubs().asyncGenRetHelper :
    jit::tc::ustubs().genRetHelper);

  vmfp() = genAR;

  assertx(genAR->func()->contains(gen->resumable()->resumeOffset()));
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
  assertx(vmStack().top() + 1 ==
         (TypedValue*)vmfp() - vmfp()->m_func->numSlotsInFrame());

  // Do linkage of the generator's AR.
  assertx(vmfp()->hasThis());
  // `recursive` determines whether we enter just the top generator or whether
  // we drop down to the lowest running delegate generator. This is useful for
  // ContRaise, which should throw from the context of the lowest generator.
  if(!recursive || vmfp()->getThis()->getVMClass() != Generator::getClass()) {
    moveProgramCounterIntoGenerator(pc, this_base_generator(vmfp()));
  } else {
    // TODO(https://github.com/facebook/hhvm/issues/6040)
    // Implement throwing from delegate generators.
    assertx(vmfp()->getThis()->getVMClass() == Generator::getClass());
    auto gen = this_generator(vmfp());
    if (gen->m_delegate.m_type != KindOfNull) {
      SystemLib::throwExceptionObject("Throwing from a delegate generator is "
          "not currently supported in HHVM");
    }
    moveProgramCounterIntoGenerator(pc, gen);
  }

  EventHook::FunctionResumeYield(vmfp());
}

OPTBLD_INLINE void iopContEnter(PC& pc) {
  contEnterImpl<false>(pc);
}

OPTBLD_INLINE void iopContRaise(PC& pc) {
  contEnterImpl<true>(pc);
  iopThrow();
}

OPTBLD_INLINE void moveProgramCounterToCaller(PC& pc,
                                              ActRec* sfp, Offset soff) {
  // Return control to the next()/send()/raise() caller.
  vmfp() = sfp;
  pc = sfp != nullptr ? sfp->func()->getEntry() + soff : nullptr;
}

OPTBLD_INLINE TCA yield(PC& pc, const Cell* key, const Cell value) {
  auto const jitReturn = jitReturnPre(vmfp());

  auto const fp = vmfp();
  auto const func = fp->func();
  auto const resumeOffset = func->unit()->offsetOf(pc);
  assertx(fp->resumed());
  assertx(func->isGenerator());

  EventHook::FunctionSuspendYield(fp);

  auto const sfp = fp->sfp();
  auto const soff = fp->m_soff;

  if (!func->isAsync()) {
    // Non-async generator.
    assertx(fp->sfp());
    frame_generator(fp)->yield(resumeOffset, key, value);

    // Push return value of next()/send()/raise().
    vmStack().pushNull();
  } else {
    // Async generator.
    auto const gen = frame_async_generator(fp);
    auto const eagerResult = gen->yield(resumeOffset, key, value);
    if (eagerResult) {
      // Eager execution => return StaticWaitHandle.
      assertx(sfp);
      vmStack().pushObjectNoRc(eagerResult);
    } else {
      // Resumed execution => return control to the scheduler.
      assertx(!sfp);
    }
  }

  moveProgramCounterToCaller(pc, sfp, soff);

  return jitReturnPost(jitReturn);
}

OPTBLD_INLINE TCA iopYield(PC& pc) {
  auto const value = *vmStack().topC();
  vmStack().discard();
  return yield(pc, nullptr, value);
}

OPTBLD_INLINE TCA iopYieldK(PC& pc) {
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

OPTBLD_INLINE void iopContAssignDelegate(Iter* iter) {
  auto param = *vmStack().topC();
  vmStack().discard();
  auto gen = frame_generator(vmfp());
  if (UNLIKELY(!typeIsValidGeneratorDelegate(param.m_type))) {
    tvDecRefGen(param);
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
  if (tvIsGenerator(param) || iter->init<false>(&param)) {
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

OPTBLD_INLINE void iopContEnterDelegate(PC& pc) {
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

  assertx(tvIsGenerator(gen->m_delegate));
  auto delegate = Generator::fromObject(gen->m_delegate.m_data.pobj);

  if (delegate->getState() == BaseGenerator::State::Done) {
    // If the generator is done, just copy the return value onto the stack.
    cellDup(delegate->m_value, *vmStack().topTV());
    return nullptr;
  }

  auto jitReturn = jitReturnPre(fp);

  EventHook::FunctionSuspendYield(fp);
  auto const sfp = fp->sfp();
  auto const soff = fp->m_soff;

  // We don't actually want to "yield" anything here. The implementation of
  // key/current are smart enough to dive into our delegate generator, so
  // really what we want to do is clean up all of the generator metadata
  // (state, ressume address, etc) and continue on.
  assertx(gen->isRunning());
  gen->resumable()->setResumeAddr(nullptr, resumeOffset);
  gen->setState(BaseGenerator::State::Started);

  moveProgramCounterToCaller(pc, sfp, soff);

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
    tvWriteNull(*vmStack().topTV());
    return nullptr;
  }

  // Otherwise, if iteration is finished we just return null.
  auto arr = it->arr();
  if (arr.end()) {
    // Push our null return value onto the stack
    tvWriteNull(*vmStack().topTV());
    return nullptr;
  }

  auto jitReturn = jitReturnPre(fp);

  EventHook::FunctionSuspendYield(fp);
  auto const sfp = fp->sfp();
  auto const soff = fp->m_soff;

  auto key = *(arr.first().asTypedValue());
  auto value = *(arr.second().asTypedValue());
  gen->yield(resumeOffset, &key, value);

  moveProgramCounterToCaller(pc, sfp, soff);

  it->next();

  return jitReturnPost(jitReturn);
}

OPTBLD_INLINE TCA iopYieldFromDelegate(PC& pc, Iter* it, PC resumePc) {
  auto gen = frame_generator(vmfp());
  auto func = vmfp()->func();
  auto resumeOffset = func->unit()->offsetOf(resumePc);
  if (tvIsGenerator(gen->m_delegate)) {
    return yieldFromGenerator(pc, gen, resumeOffset);
  }
  return yieldFromIterator(pc, gen, it, resumeOffset);
}

OPTBLD_INLINE void iopContUnsetDelegate(CudOp subop, Iter* iter) {
  auto gen = frame_generator(vmfp());
  // The `shouldFreeIter` immediate determines whether we need to call free
  // on our iterator or not. Normally if we finish executing our yield from
  // successfully then the implementation of `next` will automatically do it
  // for us when there aren't any elements left, but if an exception is thrown
  // then we need to do it manually. We don't use the iterator when the
  // delegate is a generator though, so even if the param tells us to free it
  // we should just ignore it.
  if (UNLIKELY(subop == CudOp::FreeIter && !tvIsGenerator(gen->m_delegate))) {
    iter->free();
  }
  cellSetNull(gen->m_delegate);
}

OPTBLD_INLINE void iopContCheck(ContCheckOp subop) {
  this_base_generator(vmfp())->preNext(subop == ContCheckOp::CheckStarted);
}

OPTBLD_INLINE void iopContValid() {
  vmStack().pushBool(
    this_generator(vmfp())->getState() != BaseGenerator::State::Done);
}

OPTBLD_INLINE void iopContStarted() {
  vmStack().pushBool(
    this_generator(vmfp())->getState() != BaseGenerator::State::Created);
}

OPTBLD_INLINE Generator *currentlyDelegatedGenerator(Generator *gen) {
  while(tvIsGenerator(gen->m_delegate)) {
    gen = Generator::fromObject(gen->m_delegate.m_data.pobj);
  }
  return gen;
}

OPTBLD_INLINE void iopContKey() {
  Generator* cont = this_generator(vmfp());
  if (!RuntimeOption::AutoprimeGenerators) cont->startedCheck();

  // If we are currently delegating to a generator, return its key instead
  cont = currentlyDelegatedGenerator(cont);

  cellDup(cont->m_key, *vmStack().allocC());
}

OPTBLD_INLINE void iopContCurrent() {
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

OPTBLD_INLINE void iopContGetReturn() {
  Generator* cont = this_generator(vmfp());
  if (!RuntimeOption::AutoprimeGenerators) cont->startedCheck();

  if(!cont->successfullyFinishedExecuting()) {
    SystemLib::throwExceptionObject("Cannot get return value of a generator "
                                    "that hasn't returned");
  }

  cellDup(cont->m_value, *vmStack().allocC());
}

OPTBLD_INLINE void asyncSuspendE(PC& pc) {
  auto const fp = vmfp();
  auto const func = fp->func();
  auto const resumeOffset = func->unit()->offsetOf(pc);
  assertx(func->isAsync());
  assertx(resumeModeFromActRec(fp) != ResumeMode::Async);

  // Pop the dependency we are blocked on.
  auto child = wait_handle<c_WaitableWaitHandle>(*vmStack().topC());
  assertx(!child->isFinished());
  vmStack().discard();

  if (!func->isGenerator()) {  // Async function.
    // Create the AsyncFunctionWaitHandle object. Create takes care of
    // copying local variables and itertors.
    auto waitHandle = c_AsyncFunctionWaitHandle::Create<true>(
      fp, func->numSlotsInFrame(), nullptr, resumeOffset, child);

    // Call the suspend hook. It will decref the newly allocated waitHandle
    // if it throws.
    EventHook::FunctionSuspendAwaitEF(fp, waitHandle->actRec());

    // Grab caller info from ActRec.
    ActRec* sfp = fp->sfp();
    Offset soff = fp->m_soff;

    // Free ActRec and store the return value. In case we were called by
    // a jitted FCallAwait, let it know that we suspended.
    vmStack().ndiscard(func->numSlotsInFrame());
    vmStack().ret();
    tvCopy(make_tv<KindOfObject>(waitHandle), *vmStack().topTV());
    vmStack().topTV()->m_aux.u_fcallAwaitFlag = 1;
    assertx(vmStack().topTV() == fp->retSlot());

    // Return control to the caller.
    pc = LIKELY(sfp != nullptr) ? sfp->func()->getEntry() + soff : nullptr;
    vmfp() = sfp;
  } else {  // Async generator.
    // Create new AsyncGeneratorWaitHandle.
    auto waitHandle = c_AsyncGeneratorWaitHandle::Create(
      fp, nullptr, resumeOffset, child);

    // Call the suspend hook. It will decref the newly allocated waitHandle
    // if it throws.
    EventHook::FunctionSuspendAwaitEG(fp);

    // Store the return value.
    vmStack().pushObjectNoRc(waitHandle);

    // Return control to the caller (AG::next()).
    assertx(fp->sfp());
    pc = fp->sfp()->func()->getEntry() + fp->m_soff;
    vmfp() = fp->sfp();
  }
}

OPTBLD_INLINE void asyncSuspendR(PC& pc) {
  auto const fp = vmfp();
  auto const func = fp->func();
  auto const resumeOffset = func->unit()->offsetOf(pc);
  assertx(!fp->sfp());
  assertx(func->isAsync());
  assertx(resumeModeFromActRec(fp) == ResumeMode::Async);

  // Pop the dependency we are blocked on.
  auto child = req::ptr<c_WaitableWaitHandle>::attach(
    wait_handle<c_WaitableWaitHandle>(*vmStack().topC()));
  assertx(!child->isFinished());
  vmStack().discard();

  // Before adjusting the stack or doing anything, check the suspend hook.
  // This can throw.
  EventHook::FunctionSuspendAwaitR(fp, child.get());

  // Await child and suspend the async function/generator. May throw.
  if (!func->isGenerator()) {  // Async function.
    frame_afwh(fp)->await(resumeOffset, std::move(child));
  } else {  // Async generator.
    auto const gen = frame_async_generator(fp);
    gen->resumable()->setResumeAddr(nullptr, resumeOffset);
    gen->getWaitHandle()->await(std::move(child));
  }

  // Return control to the scheduler.
  pc = nullptr;
  vmfp() = nullptr;
}

OPTBLD_INLINE TCA iopAwait(PC& pc) {
  auto const awaitable = vmStack().topC();
  auto wh = c_Awaitable::fromCell(*awaitable);
  if (UNLIKELY(wh == nullptr)) {
    SystemLib::throwBadMethodCallExceptionObject("Await on a non-Awaitable");
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

OPTBLD_INLINE TCA iopAwaitAll(PC& pc, LocalRange locals) {
  uint32_t cnt = 0;
  for (auto i = locals.first; i < locals.first + locals.restCount + 1; ++i) {
    auto const local = *frame_local(vmfp(), i);
    if (cellIsNull(local)) continue;
    auto const awaitable = c_Awaitable::fromCell(local);
    if (UNLIKELY(awaitable == nullptr)) {
      SystemLib::throwBadMethodCallExceptionObject("Await on a non-Awaitable");
    }
    if (!awaitable->isFinished()) {
      ++cnt;
    }
  }

  if (!cnt) {
    vmStack().pushNull();
    return nullptr;
  }

  auto obj = Object::attach(c_AwaitAllWaitHandle::fromFrameNoCheck(
    locals.restCount + 1, cnt, frame_local(vmfp(), locals.first)
  ));
  assertx(obj->isWaitHandle());
  assertx(!static_cast<c_Awaitable*>(obj.get())->isFinished());

  vmStack().pushObjectNoRc(obj.detach());
  return suspendStack(pc);
}

TCA suspendStack(PC &pc) {
  while (true) {
    auto const jitReturn = [&] {
      try {
        return jitReturnPre(vmfp());
      } catch (VMSwitchMode&) {
        vmpc() = pc;
        throw VMSuspendStack();
      } catch (...) {
        // We're halfway through a bytecode; we can't recover
        always_assert(false);
      }
    }();

    if (resumeModeFromActRec(vmfp()) == ResumeMode::Async) {
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
      auto const& us = jit::tc::ustubs();
      if (retIp == us.resumeHelper) retIp = us.fcallAwaitSuspendHelper;
      return retIp;
    }
    vmpc() = pc;
  }
}

OPTBLD_INLINE void iopWHResult() {
  // we should never emit this bytecode for non-waithandle
  auto const wh = c_Awaitable::fromCell(*vmStack().topC());
  if (UNLIKELY(!wh)) {
    raise_error("WHResult input was not a subclass of Awaitable");
  }

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

OPTBLD_INLINE void iopCheckProp(const StringData* propName) {
  auto* cls = vmfp()->getClass();
  auto* propVec = cls->getPropData();
  always_assert(propVec);

  auto* ctx = arGetContextClass(vmfp());
  auto idx = ctx->lookupDeclProp(propName);

  auto& tv = (*propVec)[idx];
  vmStack().pushBool(tv.m_type != KindOfUninit);
}

OPTBLD_INLINE void iopInitProp(const StringData* propName, InitPropOp propOp) {
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

OPTBLD_INLINE void iopIncStat(uint32_t counter, uint32_t value) {
  Stats::inc(Stats::StatCounter(counter), value);
}

OPTBLD_INLINE void iopOODeclExists(OODeclExistsOp subop) {
  TypedValue* aloadTV = vmStack().topTV();
  if (aloadTV->m_type != KindOfBoolean) {
    raise_error("OODeclExists: Expected Bool on top of stack, got %s",
          tname(aloadTV->m_type).c_str());
  }

  bool autoload = aloadTV->m_data.num;
  vmStack().popX();

  TypedValue* name = vmStack().topTV();
  if (!isStringType(name->m_type)) {
    raise_error("OODeclExists: Expected String on stack, got %s",
          tname(aloadTV->m_type).c_str());
  }

  ClassKind kind;
  switch (subop) {
    case OODeclExistsOp::Class : kind = ClassKind::Class; break;
    case OODeclExistsOp::Trait : kind = ClassKind::Trait; break;
    case OODeclExistsOp::Interface : kind = ClassKind::Interface; break;
  }
  tvAsVariant(name) = Unit::classExists(name->m_data.pstr, autoload, kind);
}

OPTBLD_INLINE void iopSilence(local_var loc, SilenceOp subop) {
  switch (subop) {
    case SilenceOp::Start:
      loc.ptr->m_type = KindOfInt64;
      loc.ptr->m_data.num = zero_error_level();
      break;
    case SilenceOp::End:
      assertx(loc.ptr->m_type == KindOfInt64);
      restore_error_level(loc.ptr->m_data.num);
      break;
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
      if (jit::tc::isValidCodeAddress(rip)) return rip;
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

void recordCodeCoverage(PC /*pc*/) {
  Unit* unit = vmfp()->m_func->unit();
  assertx(unit != nullptr);
  if (unit == SystemLib::s_nativeFuncUnit ||
      unit == SystemLib::s_nativeClassUnit ||
      unit == SystemLib::s_hhas_unit) {
    return;
  }
  int line = unit->getLineNumber(pcOff());
  assertx(line != -1);

  if (unit != s_prev_unit || line != s_prev_line) {
    s_prev_unit = unit;
    s_prev_line = line;
    const StringData* filepath = unit->filepath();
    assertx(filepath->isStatic());
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

namespace {

/*
 * iopWrapReturn() calls a function pointer and forwards its return value if it
 * returns TCA, or nullptr if returns void.
 */
template<typename... Params, typename... Args>
OPTBLD_INLINE TCA iopWrapReturn(void(fn)(Params...), PC, Args&&... args) {
  fn(std::forward<Args>(args)...);
  return nullptr;
}

template<typename... Params, typename... Args>
OPTBLD_INLINE TCA iopWrapReturn(TCA(fn)(Params...), PC, Args&&... args) {
  return fn(std::forward<Args>(args)...);
}

/*
 * iopSwitch and iopSSwitch take vectors containing Offset and need origpc to
 * translate those to PC. Special-case that here rather than creating a new
 * flag in hhbc.h just for this one case.
 */
template<typename... Params, typename... Args>
OPTBLD_INLINE TCA iopWrapReturn(void(fn)(PC, Params...), PC origpc,
                                Args&&... args) {
  fn(origpc, std::forward<Args>(args)...);
  return nullptr;
}

/*
 * arFromInstr() is always correct for FPI-using instructions, but many opcodes
 * can use the much faster arFromSp().
 */
ALWAYS_INLINE ActRec* ar_for_inst(Op op, PC origpc,
                                  int iva_count, uint32_t* ivas) {
  switch (op) {
    case Op::FPassC:
    case Op::FPassV:
    case Op::FPassVNop:
    case Op::FPassR:
    case Op::FPassN:
    case Op::FPassG:
    case Op::FPassS:
      assertx(iva_count >= 1);
      return arFromSp(ivas[0] + 1);
    case Op::FPassL:
    case Op::FCall:
    case Op::FCallD:
    case Op::FCallAwait:
    case Op::FCallUnpack:
    case Op::FCallM:
    case Op::FCallDM:
    case Op::FCallUnpackM:
      assertx(iva_count >= 1);
      return arFromSp(ivas[0]);
    case Op::FPassM:
      assertx(iva_count >= 2);
      return arFromSp(ivas[0] + ivas[1]);
    default:
      return arFromInstr(origpc);
  }
}

/*
 * Some bytecodes with SA immediates want the raw Id to look up a NamedEntity
 * quickly, and some want the const StringData*. Support both by decoding to
 * this struct and implicitly converting to what the callee wants.
 */
struct litstr_id {
  /* implicit */ ALWAYS_INLINE operator const StringData*() const {
    return liveUnit()->lookupLitstrId(id);
  }
  /* implicit */ ALWAYS_INLINE operator Id() const {
    return id;
  }

  Id id{kInvalidId};
};

/*
 * These macros are used to generate wrapper functions for the iop*() functions
 * defined earlier in this file. iopWrapFoo() decodes immediates from the
 * bytecode stream according to the signature of Foo (in hhbc.h), then calls
 * iopFoo() with those decoded arguments.
 */
#define FLAG_NF
#define FLAG_TF
#define FLAG_CF , pc
#define FLAG_FF , ar_for_inst(op, origpc, iva_count, ivas)
#define FLAG_PF
#define FLAG_CF_TF FLAG_CF
#define FLAG_CF_FF FLAG_CF FLAG_FF

#define DECODE_IVA ivas[iva_count++] = decode_iva(pc)
#define DECODE_I64A decode<int64_t>(pc)
#define DECODE_LA decode_local(pc)
#define DECODE_IA decode_iter(pc)
#define DECODE_CAR decode_clsref_slot(pc)
#define DECODE_CAW DECODE_CAR
#define DECODE_DA decode<double>(pc)
#define DECODE_SA decode<litstr_id>(pc)
#define DECODE_AA decode_litarr(pc)
#define DECODE_RATA decode_rat(pc)
#define DECODE_BA origpc + decode_ba(pc)
#define DECODE_OA(ty) decode<ty>(pc)
#define DECODE_KA decode_member_key(pc, liveUnit())
#define DECODE_LAR decodeLocalRange(pc)
#define DECODE_BLA decode_imm_array<Offset>(pc)
#define DECODE_SLA decode_imm_array<StrVecItem>(pc)
#define DECODE_ILA decode_iter_table(pc)
#define DECODE_I32LA decode_imm_array<uint32_t>(pc)
#define DECODE_BLLA decode_imm_array<bool>(pc)
#define DECODE_VSA decode_imm_array<Id>(pc)

#define DECODE_NA
#define DECODE_ONE(a) auto const imm1 = DECODE_##a;
#define DECODE_TWO(a, b) DECODE_ONE(a) auto const imm2 = DECODE_##b;
#define DECODE_THREE(a, b, c) DECODE_TWO(a, b) auto const imm3 = DECODE_##c;
#define DECODE_FOUR(a, b, c, d) \
  DECODE_THREE(a, b, c) auto const imm4 = DECODE_##d;
#define DECODE_FIVE(a, b, c, d, e) \
  DECODE_FOUR(a, b, c, d) auto const imm5 = DECODE_##e;

#define PASS_NA
#define PASS_ONE(...) , imm1
#define PASS_TWO(...) , imm1, imm2
#define PASS_THREE(...) , imm1, imm2, imm3
#define PASS_FOUR(...) , imm1, imm2, imm3, imm4
#define PASS_FIVE(...) , imm1, imm2, imm3, imm4, imm5

#define O(name, imm, in, out, flags)                                 \
  OPTBLD_INLINE TCA iopWrap##name(PC& pc) {                          \
    UNUSED int iva_count = 0;                                        \
    UNUSED uint32_t ivas[kMaxHhbcImms];                              \
    UNUSED auto const op = Op::name;                                 \
    UNUSED auto const origpc = pc - encoded_op_size(op);             \
    DECODE_##imm                                                     \
    return iopWrapReturn(iop##name, origpc FLAG_##flags PASS_##imm); \
  }
OPCODES

#undef FLAG_NF
#undef FLAG_TF
#undef FLAG_CF
#undef FLAG_FF
#undef FLAG_PF
#undef FLAG_CF_TF
#undef FLAG_CF_FF

#undef DECODE_IVA
#undef DECODE_I64A
#undef DECODE_LA
#undef DECODE_IA
#undef DECODE_CAR
#undef DECODE_CAW
#undef DECODE_DA
#undef DECODE_SA
#undef DECODE_AA
#undef DECODE_RATA
#undef DECODE_BA
#undef DECODE_OA
#undef DECODE_KA
#undef DECODE_LAR
#undef DECODE_BLA
#undef DECODE_SLA
#undef DECODE_ILA
#undef DECODE_I32LA
#undef DECODE_BLLA
#undef DECODE_VSA

#undef DECODE_NA
#undef DECODE_ONE
#undef DECODE_TWO
#undef DECODE_THREE
#undef DECODE_FOUR
#undef DECODE_FIVE

#undef PASS_NA
#undef PASS_ONE
#undef PASS_TWO
#undef PASS_THREE
#undef PASS_FOUR
#undef PASS_FIVE

#undef O

}

/**
 * The interpOne functions are fat wrappers around the iop* functions, mostly
 * adding a bunch of debug-only logging and stats tracking.
 */
#define O(opcode, imm, push, pop, flags)                                \
  TCA interpOne##opcode(ActRec* fp, TypedValue* sp, Offset pcOff) {     \
  interp_set_regs(fp, sp, pcOff);                                       \
  SKTRACE(5, liveSK(),                                                  \
          "%40s %p %p\n",                                               \
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
    auto sk = liveSK().toAtomicInt();                                   \
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
  assertx(peek_op(pc) == Op::opcode);                                    \
  pc += encoded_op_size(Op::opcode);                                    \
  auto const retAddr = iopWrap##opcode(pc);                             \
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
  assertx(sizeof(optabDirect) / sizeof(const void *) == Op_count);
  assertx(sizeof(optabDbg) / sizeof(const void *) == Op_count);
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
    retAddr = iopWrap##name(pc);                              \
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
      assertx(retAddr == jit::tc::ustubs().callToExit);    \
      return breakOnCtlFlow ? retAddr : nullptr;              \
    }                                                         \
    assertx(isCtlFlow || !retAddr);                            \
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

  assertx(retAddr == nullptr);
  return nullptr;
}

static void dispatch() {
  WorkloadStats guard(WorkloadStats::InInterp);

  DEBUG_ONLY auto const retAddr = dispatchImpl<false>();
  assertx(retAddr == nullptr);
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
      return jit::tc::ustubs().throwSwitchMode;
    } else {
      throw VMSwitchMode();
    }
  }

  return retAddr;
}

TCA dispatchBB() {
  auto sk = [] {
    return SrcKey(vmfp()->func(), vmpc(), resumeModeFromActRec(vmfp()),
                  vmfp()->func()->cls() && vmfp()->hasThis());
  };

  if (Trace::moduleEnabled(Trace::dispatchBB)) {
    static auto cat = makeStaticString("dispatchBB");
    auto name = makeStaticString(show(sk()));
    Stats::incStatGrouped(cat, name, 1);
  }
  if (Trace::moduleEnabled(Trace::ringbuffer)) {
    Trace::ringbufferEntry(Trace::RBTypeDispatchBB, sk().toAtomicInt(), 0);
  }
  auto retAddr = dispatchImpl<true>();
  return switchModeForDebugger(retAddr);
}

}
