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

#include "hphp/util/numa.h"
#include "hphp/util/portability.h"
#include "hphp/util/ringbuffer.h"
#include "hphp/util/text-util.h"
#include "hphp/util/trace.h"

#include "hphp/system/systemlib.h"

#include "hphp/runtime/base/apc-stats.h"
#include "hphp/runtime/base/apc-typed-value.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/array-provenance.h"
#include "hphp/runtime/base/code-coverage.h"
#include "hphp/runtime/base/collections.h"
#include "hphp/runtime/base/container-functions.h"
#include "hphp/runtime/base/enum-util.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/hhprof.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/object-data.h"
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
#include "hphp/runtime/base/type-structure-helpers-defs.h"
#include "hphp/runtime/base/tv-arith.h"
#include "hphp/runtime/base/tv-comparisons.h"
#include "hphp/runtime/base/tv-conversions.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/base/tv-type.h"
#include "hphp/runtime/base/type-variant.h"
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
#include "hphp/runtime/ext/json/JSON_parser.h"

#include "hphp/runtime/server/rpc-request-handler.h"
#include "hphp/runtime/server/source-root-info.h"

#include "hphp/runtime/vm/act-rec-defs.h"
#include "hphp/runtime/vm/act-rec.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/class-meth-data-ref.h"
#include "hphp/runtime/vm/cti.h"
#include "hphp/runtime/vm/debug/debug.h"
#include "hphp/runtime/vm/debugger-hook.h"
#include "hphp/runtime/vm/event-hook.h"
#include "hphp/runtime/ext/functioncredential/ext_functioncredential.h"
#include "hphp/runtime/vm/globals-array.h"
#include "hphp/runtime/vm/hh-utils.h"
#include "hphp/runtime/vm/hhbc-codec.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/interp-helpers.h"
#include "hphp/runtime/vm/iter.h"
#include "hphp/runtime/vm/member-operations.h"
#include "hphp/runtime/vm/memo-cache.h"
#include "hphp/runtime/vm/method-lookup.h"
#include "hphp/runtime/vm/native.h"
#include "hphp/runtime/vm/reified-generics.h"
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
#include "hphp/runtime/vm/jit/enter-tc.h"
#include "hphp/runtime/vm/jit/perf-counters.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/translator-runtime.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/unwind-itanium.h"
#include "hphp/util/stacktrace-profiler.h"


namespace HPHP {

TRACE_SET_MOD(bcinterp);

// RepoAuthoritative has been raptured out of runtime_option.cpp. It needs
// to be closer to other bytecode.cpp data.
bool RuntimeOption::RepoAuthoritative = false;

using jit::TCA;

// GCC 4.8 has some real problems with all the inlining in this file, so don't
// go overboard with that version.
#if !defined(NDEBUG) || ((__GNUC__ == 4) && (__GNUC_MINOR__ == 8))
#define OPTBLD_INLINE
#else
#define OPTBLD_INLINE       ALWAYS_INLINE
#endif

Class* arGetContextClass(const ActRec* ar) {
  if (ar == nullptr) {
    return nullptr;
  }
  return ar->m_func->cls();
}

void frame_free_locals_no_hook(ActRec* fp) {
  frame_free_locals_inl_no_hook(fp, fp->func()->numLocals());
}

const StaticString s_file("file");
const StaticString s_line("line");

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
inline const char* prettytype(SetRangeOp) { return "SetRangeOp"; }
inline const char* prettytype(TypeStructResolveOp) {
  return "TypeStructResolveOp";
}
inline const char* prettytype(CudOp) { return "CudOp"; }
inline const char* prettytype(ContCheckOp) { return "ContCheckOp"; }
inline const char* prettytype(SpecialClsRef) { return "SpecialClsRef"; }
inline const char* prettytype(CollectionType) { return "CollectionType"; }
inline const char* prettytype(IsLogAsDynamicCallOp) {
  return "IsLogAsDynamicCallOp";
}

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

inline const ArrayData* decode_litarr(PC& pc) {
  return liveUnit()->lookupArrayId(decode<Id>(pc));
}

namespace {

// wrapper for local variable ILA operand
struct local_var {
  TypedValue* ptr;
  int32_t index;
  TypedValue* operator->() const { return ptr; }
  TypedValue& operator*() const { return *ptr; }
};

// wrapper for named local variable NLA operand
struct named_local_var {
  LocalName name;
  TypedValue* ptr;
  TypedValue* operator->() const { return ptr; }
  TypedValue& operator*() const { return *ptr; }
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

}

ALWAYS_INLINE TypedValue* decode_local(PC& pc) {
  auto la = decode_iva(pc);
  assertx(la < vmfp()->m_func->numLocals());
  return frame_local(vmfp(), la);
}

ALWAYS_INLINE local_var decode_indexed_local(PC& pc) {
  auto la = decode_iva(pc);
  assertx(la < vmfp()->m_func->numLocals());
  return local_var{frame_local(vmfp(), la), safe_cast<int32_t>(la)};
}

ALWAYS_INLINE named_local_var decode_named_local_var(PC& pc) {
  auto loc = decode_named_local(pc);
  assertx(0 <= loc.id);
  assertx(loc.id < vmfp()->m_func->numLocals());
  assertx(kInvalidLocalName <= loc.name);
  assertx(loc.name < vmfp()->m_func->numNamedLocals());
  return named_local_var{loc.name, frame_local(vmfp(), loc.id)};
}

ALWAYS_INLINE Iter* decode_iter(PC& pc) {
  auto ia = decode_iva(pc);
  return frame_iter(vmfp(), ia);
}

template<typename T>
OPTBLD_INLINE imm_array<T> decode_imm_array(PC& pc) {
  auto const size = decode_iva(pc);
  auto const arr_pc = pc;
  pc += size * sizeof(T);
  return imm_array<T>{size, arr_pc};
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
  , m_depth(0)
  , m_global(true)
{
  TRACE(3, "Creating VarEnv %p [global scope]\n", this);
  ARRPROV_USE_RUNTIME_LOCATION();
  auto globals_var = Variant::attach(
    new (tl_heap->objMalloc(sizeof(GlobalsArray))) GlobalsArray(&m_nvTable)
  );
  m_nvTable.set(s_GLOBALS.get(), globals_var.asTypedValue());
}

VarEnv::VarEnv(ActRec* fp)
  : m_nvTable(fp)
  , m_depth(1)
  , m_global(false)
{
  assertx(fp->func()->attrs() & AttrMayUseVV);
  TRACE(3, "Creating lazily attached VarEnv %p on stack\n", this);
}

VarEnv::VarEnv(const VarEnv* varEnv, ActRec* fp)
  : m_nvTable(varEnv->m_nvTable, fp)
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
  return req::make_raw<VarEnv>(fp);
}

VarEnv* VarEnv::clone(ActRec* fp) const {
  return req::make_raw<VarEnv>(this, fp);
}

void VarEnv::suspend(const ActRec* oldFP, ActRec* newFP) {
  m_nvTable.suspend(oldFP, newFP);
}

void VarEnv::enterFP(ActRec* oldFP, ActRec* newFP) {
  TRACE(3, "Attaching VarEnv %p [%s] fp @%p\n",
           this,
           isGlobalScope() ? "global scope" : "local scope",
           newFP);
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

void VarEnv::set(const StringData* name, tv_rval tv) {
  m_nvTable.set(name, tv);
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

const StaticString s_reified_generics_var("0ReifiedGenerics");

Array VarEnv::getDefinedVariables() const {
  Array ret = Array::CreateDArray();

  NameValueTable::Iterator iter(&m_nvTable);
  for (; iter.valid(); iter.next()) {
    auto const sd = iter.curKey();
    auto const tv = iter.curVal();
    // Reified functions have an internal 0ReifiedGenerics variable
    if (s_reified_generics_var.equal(sd)) {
      continue;
    }
    ret.set(StrNR(sd).asString(), tvAsCVarRef(tv));
  }
  {
    // Make result independent of the hashtable implementation.
    ArrayData* sorted = ret->escalateForSort(SORTFUNC_KSORT);
    assertx(sorted == ret.get() ||
            sorted->empty() ||
            sorted->hasExactlyOneRef());
    SCOPE_EXIT {
      if (sorted != ret.get()) {
        ret = Array::attach(sorted);
      }
    };
    sorted->ksort(0, true);
  }
  return ret;
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
THREAD_LOCAL_FLAT(StackElms, t_se);

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
        kStackCheckPadding * sizeof(TypedValue)
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

  os << "C:";

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
      assertx(tv->m_data.parr->isVecArrayType());
      assertx(tv->m_data.parr->checkCount());
      os << tv->m_data.parr;
      print_count();
      os << ":Vec";
      continue;
    case KindOfPersistentDict:
    case KindOfDict:
      assertx(tv->m_data.parr->isDictType());
      assertx(tv->m_data.parr->checkCount());
      os << tv->m_data.parr;
      print_count();
      os << ":Dict";
      continue;
    case KindOfPersistentKeyset:
    case KindOfKeyset:
      assertx(tv->m_data.parr->isKeysetType());
      assertx(tv->m_data.parr->checkCount());
      os << tv->m_data.parr;
      print_count();
      os << ":Keyset";
      continue;
    case KindOfPersistentDArray:
    case KindOfDArray:
    case KindOfPersistentVArray:
    case KindOfVArray:
    case KindOfPersistentArray:
    case KindOfArray:
      assertx(tv->m_data.parr->isPHPArrayType());
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
    case KindOfRecord:
      assertx(tv->m_data.prec->checkCount());
      os << tv->m_data.prec;
      print_count();
      os << ":Record("
         << tv->m_data.prec->record()->name()->data()
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
    case KindOfFunc:
      os << ":Func("
         << tv->m_data.pfunc->fullName()->data()
         << ")";
      continue;
    case KindOfClass:
      os << ":Class("
         << tv->m_data.pclass->name()->data()
         << ")";
      continue;
    case KindOfClsMeth:
      os << ":ClsMeth("
       << tv->m_data.pclsmeth->getCls()->name()->data()
       << ", "
       << tv->m_data.pclsmeth->getFunc()->fullName()->data()
       << ")";
       continue;
    }
    not_reached();
  } while (0);

  return os.str();
}

/*
 * Return true if Offset o is inside the protected region of a fault
 * funclet for iterId, otherwise false.
 */
static bool checkIterScope(const Func* f, Offset o, Id iterId) {
  assertx(o >= f->base() && o < f->past());
  for (auto const& eh : f->ehtab()) {
    if (eh.m_base <= o && o < eh.m_past &&
        eh.m_iterId == iterId) {
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
     << ",callOff:" << fp->callOffset()
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
      if (checkIterScope(func, offset, i)) {
        os << it->toString();
      } else {
        os << "I:Undefined";
      }
    }
    os << "|";
  }

  std::vector<std::string> stackElems;
  visitStackElems(
    fp, ftop,
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
  //   {func:foo,callOff:51}<C:8> {func:bar} C:8 C:1 {func:biz} C:0
  //                              aaaaaaaaaaaaaaaaaa bbbbbbbbbbbbbb
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
  return isResumed(fp) ? Stack::resumableStackBase(fp)
                       : Stack::frameStackBase(fp);
}

TypedValue* Stack::frameStackBase(const ActRec* fp) {
  assertx(!isResumed(fp));
  return (TypedValue*)fp - fp->func()->numSlotsInFrame();
}

TypedValue* Stack::resumableStackBase(const ActRec* fp) {
  assertx(isResumed(fp));
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
  if (UNLIKELY(fp == nullptr || fp->isInlined())) return empty_darray();

  if ((fp->func()->attrs() & AttrMayUseVV) && fp->hasVarEnv()) {
    return fp->m_varEnv->getDefinedVariables();
  }
  auto const func = fp->m_func;
  auto const numLocals = func->numNamedLocals();
  DArrayInit ret(numLocals);
  for (Id id = 0; id < numLocals; ++id) {
    TypedValue* ptv = frame_local(fp, id);
    if (ptv->m_type == KindOfUninit) {
      continue;
    }
    auto const localNameSd = func->localVarName(id);
    if (!localNameSd) continue;
    Variant name(localNameSd, Variant::PersistentStrInit{});
    ret.add(name, tvAsVariant(ptv));
  }
  return ret.toArray();
}

void shuffleMagicArgs(String&& invName, uint32_t numArgs, bool hasUnpack) {
  assertx(!invName.isNull());
  auto& stack = vmStack();

  // We need to make an array containing all the arguments passed by
  // the caller and put it where the second argument is.
  auto argArray = Array::attach([&] {
    if (numArgs == 0) {
      return RuntimeOption::EvalHackArrDVArrs
        ? ArrayData::CreateVec()
        : ArrayData::CreateVArray();
    }
    auto const args = stack.indC(hasUnpack ? 1 : 0);
    return RuntimeOption::EvalHackArrDVArrs
      ? PackedArray::MakeVec(numArgs, args)
      : PackedArray::MakeVArray(numArgs, args);
  }());

  // Unpack arguments to the end of the argument array.
  if (UNLIKELY(hasUnpack)) {
    auto const args = *stack.topC();
    if (!isContainer(args)) throwInvalidUnpackArgs();
    stack.discard();
    SCOPE_EXIT { tvDecRefGen(args); };
    IterateV(args, [&](TypedValue v) { argArray.append(v); });
  }

  // Remove the arguments from the stack; they were moved into the
  // array so we don't need to decref.
  stack.ndiscard(numArgs);

  // Move invName to where the first argument belongs.
  stack.pushStringNoRc(invName.detach());

  // Move argArray to where the second argument belongs.
  if (RuntimeOption::EvalHackArrDVArrs) {
    stack.pushVecNoRc(argArray.detach());
  } else {
    stack.pushArrayNoRc(argArray.detach());
  }
}

// Unpack or repack arguments as needed to match the function signature.
// The stack contains numArgs arguments plus an extra cell containing
// arguments to unpack.
uint32_t prepareUnpackArgs(const Func* func, uint32_t numArgs,
                           bool checkInOutAnnot) {
  auto& stack = vmStack();
  auto unpackArgs = *stack.topC();
  if (!isContainer(unpackArgs)) throwInvalidUnpackArgs();
  stack.discard();
  SCOPE_EXIT { tvDecRefGen(unpackArgs); };

  auto const numUnpackArgs = getContainerSize(unpackArgs);
  auto const numParams = func->numNonVariadicParams();
  if (LIKELY(numArgs == numParams)) {
    ARRPROV_USE_RUNTIME_LOCATION();
    // Nothing to unpack.
    if (numUnpackArgs == 0) return numParams;
    // Convert unpack args to the proper type.
    if (RuntimeOption::EvalHackArrDVArrs) {
      tvCastToVecInPlace(&unpackArgs);
      stack.pushVec(unpackArgs.m_data.parr);
    } else {
      tvCastToVArrayInPlace(&unpackArgs);
      stack.pushArray(unpackArgs.m_data.parr);
    }
    return numParams + 1;
  }

  ArrayIter iter(unpackArgs);
  if (LIKELY(numArgs < numParams)) {
    for (auto i = numArgs; iter && (i < numParams); ++i, ++iter) {
      if (UNLIKELY(checkInOutAnnot && func->isInOut(i))) {
        throwParamInOutMismatch(func, i);
      }
      auto const from = iter.secondValPlus();
      tvDup(from, *stack.allocTV());
    }

    if (LIKELY(!iter)) {
      // argArray was exhausted, so there are no "extra" arguments but there
      // may be a deficit of non-variadic arguments, and the need to push an
      // empty array for the variadic argument ... that work is left to
      // prepareFuncEntry.
      return numArgs + numUnpackArgs;
    }
  }

  // there are "extra" arguments; passed as standard arguments prior to the
  // ... unpack operator and/or still remaining in argArray
  assertx(numArgs + numUnpackArgs > numParams);
  assertx(numArgs > numParams || !!iter);

  auto const numNewUnpackArgs = numArgs + numUnpackArgs - numParams;
  VArrayInit ai(numNewUnpackArgs);
  if (UNLIKELY(numArgs > numParams)) {
    // The arguments are pushed in order, so we should start from the bottom.
    auto ptr = stack.indTV(numArgs - numParams);
    for (auto i = numParams; i < numArgs; ++i) {
      ai.append(*--ptr);
    }
    for (auto i = numParams; i < numArgs; ++i) {
      stack.popTV();
    }
  }
  for (; iter; ++iter) {
    ai.append(iter.secondValPlus());
  }
  auto const ad = ai.create();
  assertx(ad->hasExactlyOneRef());
  assertx(ad->size() == numNewUnpackArgs);
  if (RuntimeOption::EvalHackArrDVArrs) {
    stack.pushVecNoRc(ad);
  } else {
    stack.pushArrayNoRc(ad);
  }
  return numParams + 1;
}

static void prepareFuncEntry(ActRec *ar, Array&& generics) {
  assertx(!isResumed(ar));
  const Func* func = ar->m_func;
  Offset firstDVInitializer = kInvalidOffset;
  folly::Optional<uint32_t> raiseTooManyArgumentsWarnings;
  const int nparams = func->numNonVariadicParams();
  auto& stack = vmStack();

  ar->trashVarEnv();
  if (!debug || (ar->func()->attrs() & AttrMayUseVV)) {
    ar->setVarEnv(nullptr);
  }

  auto const nargs = ar->numArgs();
  assertx(((TypedValue*)ar - stack.top()) == nargs);

  if (UNLIKELY(nargs > nparams)) {
    // All extra arguments are expected to be packed in a varray.
    assertx(nargs == nparams + 1);
    assertx(tvIsVecOrVArray(stack.topC()));
    auto const unpackArgs = stack.topC()->m_data.parr;
    assertx(!unpackArgs->empty());
    if (!func->hasVariadicCaptureParam()) {
      // Record the number of args for the warning before dropping extra args.
      raiseTooManyArgumentsWarnings = nparams + unpackArgs->size();
      stack.popC();
      ar->setNumArgs(nparams);
    }
  } else {
    if (nargs < nparams) {
      // This is where we are going to enter, assuming we don't fail on
      // a missing argument check.
      firstDVInitializer = func->params()[nargs].funcletOff;

      // Push uninitialized nulls for missing arguments. They will end up
      // getting default-initialized, but regardless, we need to make space
      // for them on the stack.
      for (int i = nargs; i < nparams; ++i) {
        stack.pushUninit();
      }
    }
    if (UNLIKELY(func->hasVariadicCaptureParam())) {
      ARRPROV_USE_RUNTIME_LOCATION();
      if (RuntimeOption::EvalHackArrDVArrs) {
        stack.pushVecNoRc(ArrayData::CreateVec());
      } else {
        stack.pushArrayNoRc(ArrayData::CreateVArray());
      }
    }
  }

  int nlocals = func->numParams();
  if (UNLIKELY(func->isClosureBody())) {
    int nuse = c_Closure::initActRecFromClosure(ar, stack.top());
    // initActRecFromClosure doesn't move stack
    stack.nalloc(nuse);
    nlocals += nuse;
    func = ar->m_func;
  }

  if (ar->m_func->hasReifiedGenerics()) {
    ARRPROV_USE_RUNTIME_LOCATION();
    // Currently does not work with closures
    assertx(!func->isClosureBody());
    // push for first local
    if (RuntimeOption::EvalHackArrDVArrs) {
      stack.pushVecNoRc(generics.isNull() ? ArrayData::CreateVec()
                                          : generics.detach());
    } else {
      stack.pushArrayNoRc(generics.isNull() ? ArrayData::CreateVArray()
                                            : generics.detach());
    }
    nlocals++;
  } else {
    generics.reset();
  }

  pushFrameSlots(func, nlocals);

  vmfp() = ar;
  vmpc() = firstDVInitializer != kInvalidOffset
    ? func->unit()->entry() + firstDVInitializer
    : func->getEntry();
  vmJitReturnAddr() = nullptr;

  if (nargs < func->numRequiredParams()) {
    HPHP::jit::throwMissingArgument(func, nargs);
  }
  if (raiseTooManyArgumentsWarnings) {
    HPHP::jit::raiseTooManyArguments(func, *raiseTooManyArgumentsWarnings);
  }
}

namespace {
// Check whether the location of reified generics matches the one we expect
void checkForReifiedGenericsErrors(const ActRec* ar, bool hasGenerics) {
  if (!ar->m_func->hasReifiedGenerics()) return;
  if (!hasGenerics) {
    if (areAllGenericsSoft(ar->m_func->getReifiedGenericsInfo())) {
      raise_warning_for_soft_reified(0, true, ar->m_func->fullName());
      return;
    }
    throw_call_reified_func_without_generics(ar->m_func);
  }
  auto const tv = frame_local(ar, ar->m_func->numParams());
  assertx(tv && (RuntimeOption::EvalHackArrDVArrs ? tvIsVec(tv)
                                                  : tvIsArray(tv)));
  checkFunReifiedGenericMismatch(ar->m_func, tv->m_data.parr);
}
} // namespace

static void dispatch();

void enterVMAtPseudoMain(ActRec* enterFnAr, VarEnv* varEnv) {
  assertx(enterFnAr);
  assertx(enterFnAr->func()->isPseudoMain());
  assertx(!isResumed(enterFnAr));
  ARRPROV_USE_VMPC();
  Stats::inc(Stats::VMEnter);

  enterFnAr->setVarEnv(varEnv);
  pushFrameSlots(enterFnAr->func());
  if (varEnv != nullptr) {
    auto oldFp = vmfp();
    if (UNLIKELY(oldFp && oldFp->skipFrame())) {
      oldFp = g_context->getPrevVMStateSkipFrame(oldFp);
    }
    varEnv->enterFP(oldFp, enterFnAr);
  }
  vmfp() = enterFnAr;
  vmpc() = enterFnAr->func()->getEntry();

  if (!EventHook::FunctionCall(enterFnAr, EventHook::NormalFunc)) return;
  checkStack(vmStack(), enterFnAr->m_func, 0);
  assertx(vmfp()->func()->contains(vmpc()));

  if (RID().getJit() && !RID().getJitFolding()) {
    jit::TCA start = enterFnAr->m_func->getFuncBody();
    assert_flog(jit::tc::isValidCodeAddress(start),
                "start = {} ; func = {} ({})\n",
                start, enterFnAr->m_func, enterFnAr->m_func->fullName());
    jit::enterTC(start);
  } else {
    dispatch();
  }
}

void enterVMAtFunc(ActRec* enterFnAr, Array&& generics, bool hasInOut,
                   bool dynamicCall, bool allowDynCallNoPointer) {
  assertx(enterFnAr);
  assertx(!isResumed(enterFnAr));
  ARRPROV_USE_VMPC();
  Stats::inc(Stats::VMEnter);

  auto const hasGenerics = !generics.isNull();
  prepareFuncEntry(enterFnAr, std::move(generics));

  checkForReifiedGenericsErrors(enterFnAr, hasGenerics);
  calleeDynamicCallChecks(enterFnAr->func(), dynamicCall,
                          allowDynCallNoPointer);
  if (!EventHook::FunctionCall(enterFnAr, EventHook::NormalFunc)) return;
  checkStack(vmStack(), enterFnAr->m_func, 0);
  assertx(vmfp()->func()->contains(vmpc()));

  if (RID().getJit() && !RID().getJitFolding()) {
    jit::TCA start = enterFnAr->m_func->getFuncBody();
    assert_flog(jit::tc::isValidCodeAddress(start),
                "start = {} ; func = {} ({})\n",
                start, enterFnAr->m_func, enterFnAr->m_func->fullName());
    jit::enterTC(start);
  } else {
    dispatch();
  }
}

void enterVMAtCurPC() {
  assertx(vmfp());
  assertx(vmpc());
  assertx(vmfp()->func()->contains(vmpc()));
  ARRPROV_USE_VMPC();
  Stats::inc(Stats::VMEnter);
  if (RID().getJit()) {
    jit::enterTC(jit::tc::ustubs().resumeHelper);
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
}

static inline StringData* lookup_name(TypedValue* key) {
  return prepareKey(*key);
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
                                Slot& slot,
                                bool& visible,
                                bool& accessible,
                                bool& constant,
                                bool ignoreLateInit) {
  name = lookup_name(key);
  auto const ctx = arGetContextClass(fp);

  auto const lookup = ignoreLateInit
    ? cls->getSPropIgnoreLateInit(ctx, name)
    : cls->getSProp(ctx, name);

  val = lookup.val;
  slot = lookup.slot;
  visible = lookup.val != nullptr;
  constant = lookup.constant;
  accessible = lookup.accessible;
}

static inline Class* lookupClsRef(TypedValue* input) {
  Class* class_ = nullptr;
  if (isStringType(input->m_type)) {
    class_ = Unit::loadClass(input->m_data.pstr);
    if (class_ == nullptr) {
      raise_error(Strings::UNKNOWN_CLASS, input->m_data.pstr->data());
    }
  } else if (input->m_type == KindOfObject) {
    class_ = input->m_data.pobj->getVMClass();
  } else if (isClassType(input->m_type)) {
    class_ = input->m_data.pclass;
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
        &val(result), static_cast<data_type_t>(result.type()),
        innerCount(*result),
        &tvRef, static_cast<data_type_t>(tvRef.m_type), innerCount(tvRef),
        &tvRef2, static_cast<data_type_t>(tvRef2.m_type), innerCount(tvRef2));
  // TODO can remove following ArrayAccess magic removal?
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
    assertx(&val(result) == &tvRef.m_data);
    return tv_lval(&tvRef2);
  }

  assertx(&val(result) != &tvRef.m_data);
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

OPTBLD_INLINE void iopPopC() {
  vmStack().popC();
}

OPTBLD_INLINE void iopPopU() {
  vmStack().popU();
}

OPTBLD_INLINE void iopPopU2() {
  assertx(vmStack().indC(1)->m_type == KindOfUninit);
  *vmStack().indC(1) = *vmStack().topC();
  vmStack().discard();
}

OPTBLD_INLINE void iopPopFrame(uint32_t nout) {
  assertx(vmStack().indC(nout + 0)->m_type == KindOfUninit);
  assertx(vmStack().indC(nout + 1)->m_type == KindOfUninit);
  assertx(vmStack().indC(nout + 2)->m_type == KindOfUninit);
  for (int32_t i = nout - 1; i >= 0; --i) {
    *vmStack().indC(i + 3) = *vmStack().indC(i);
  }
  vmStack().ndiscard(3);
}

OPTBLD_INLINE void iopPopL(TypedValue* to) {
  TypedValue* fr = vmStack().topC();
  tvMove(*fr, *to);
  vmStack().discard();
}

OPTBLD_INLINE void iopDup() {
  vmStack().dup();
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

OPTBLD_INLINE void iopFuncCred() {
  vmStack().pushObjectNoRc(
    FunctionCredential::newInstance(vmfp()->m_func));
}

OPTBLD_INLINE void iopClassName() {
  auto const cls  = vmStack().topC();
  if (!isClassType(cls->m_type)) {
    raise_error("Attempting to get name of non-class");
  }
  vmStack().replaceC<KindOfPersistentString>(
    cls->m_data.pclass->name()
  );
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
  assertx(a->isPHPArrayType());
  assertx(!RuntimeOption::EvalHackArrDVArrs || a->isNotDVArray());
  vmStack().pushStaticArray(a);
}

OPTBLD_INLINE void iopVec(const ArrayData* a) {
  assertx(a->isVecArrayType());
  vmStack().pushStaticVec(a);
}

OPTBLD_INLINE void iopDict(const ArrayData* a) {
  assertx(a->isDictType());
  vmStack().pushStaticDict(a);
}

OPTBLD_INLINE void iopKeyset(const ArrayData* a) {
  assertx(a->isKeysetType());
  vmStack().pushStaticKeyset(a);
}

OPTBLD_INLINE void iopNewArray(uint32_t capacity) {
  if (capacity == 0) {
    vmStack().pushArrayNoRc(ArrayData::Create());
  } else {
    vmStack().pushArrayNoRc(PackedArray::MakeReserve(capacity));
  }
}

OPTBLD_INLINE void iopNewMixedArray(uint32_t capacity) {
  if (capacity == 0) {
    vmStack().pushArrayNoRc(ArrayData::Create());
  } else {
    vmStack().pushArrayNoRc(MixedArray::MakeReserveMixed(capacity));
  }
}

OPTBLD_INLINE void iopNewDictArray(uint32_t capacity) {
  auto const ad = capacity == 0
    ? ArrayData::CreateDict()
    : MixedArray::MakeReserveDict(capacity);
  vmStack().pushDictNoRc(ad);
}

OPTBLD_INLINE
void iopNewLikeArrayL(TypedValue* fr, uint32_t capacity) {
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
  auto const a = PackedArray::MakeVec(n, vmStack().topC());
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
    vmStack().pushArrayNoRc(ArrayData::CreateDArray());
  } else {
    vmStack().pushArrayNoRc(MixedArray::MakeReserveDArray(capacity));
  }
}

namespace {
template<typename CreatorFn, typename PushFn>
void newRecordImpl(const StringData* s,
                   const imm_array<int32_t>& ids,
                   CreatorFn creatorFn, PushFn pushFn) {
  auto rec = Unit::getRecordDesc(s, true);
  if (!rec) {
    raise_error(Strings::UNKNOWN_RECORD, s->data());
  }
  auto const n = ids.size;
  assertx(n > 0 && n <= ArrayData::MaxElemsOnStack);
  req::vector<const StringData*> names;
  names.reserve(n);
  auto const unit = vmfp()->m_func->unit();
  for (size_t i = 0; i < n; ++i) {
    auto name = unit->lookupLitstrId(ids[i]);
    names.push_back(name);
  }
  auto recdata = creatorFn(rec, names.size(), names.data(), vmStack().topC());
  vmStack().ndiscard(n);
  pushFn(vmStack(), recdata);
}
}

// TODO (T29595301): Use id instead of StringData
OPTBLD_INLINE void iopNewRecord(const StringData* s, imm_array<int32_t> ids) {
  newRecordImpl(s, ids, RecordData::newRecord,
                [] (Stack& st, RecordData* r) { st.pushRecordNoRc(r); });
}

OPTBLD_INLINE void iopNewRecordArray(const StringData* s,
                                     imm_array<int32_t> ids) {
  newRecordImpl(s, ids, RecordArray::newRecordArray,
                [] (Stack& st, RecordArray* r) { st.pushRecordArrayNoRc(r); });
}

OPTBLD_INLINE void iopAddElemC() {
  TypedValue* c1 = vmStack().topC();
  TypedValue* c2 = vmStack().indC(1);
  TypedValue* c3 = vmStack().indC(2);
  if (!isArrayType(c3->m_type) && !isDictType(c3->m_type)) {
    raise_error("AddElemC: $3 must be an array or dict");
  }
  if (c2->m_type == KindOfInt64) {
    tvAsVariant(*c3).asArrRef().set(c2->m_data.num, tvAsCVarRef(c1));
  } else {
    tvAsVariant(*c3).asArrRef().set(tvAsCVarRef(c2), tvAsCVarRef(c1));
  }
  vmStack().popC();
  vmStack().popC();
}

OPTBLD_INLINE void iopAddNewElemC() {
  TypedValue* c1 = vmStack().topC();
  TypedValue* c2 = vmStack().indC(1);
  if (isArrayType(c2->m_type)) {
    tvAsVariant(*c2).asArrRef().append(tvAsCVarRef(c1));
  } else if (isVecType(c2->m_type)) {
    auto in = c2->m_data.parr;
    auto out = PackedArray::AppendVec(in, *c1);
    if (in != out) decRefArr(in);
    c2->m_type = KindOfVec;
    c2->m_data.parr = out;
  } else if (isKeysetType(c2->m_type)) {
    auto in = c2->m_data.parr;
    auto out = SetArray::Append(in, *c1);
    if (in != out) decRefArr(in);
    c2->m_type = KindOfKeyset;
    c2->m_data.parr = out;
  } else {
    raise_error("AddNewElemC: $2 must be an array, vec, or keyset");
  }
  assertx(tvIsPlausible(*c2));
  vmStack().popC();
}

OPTBLD_INLINE void iopNewCol(CollectionType cType) {
  assertx(cType != CollectionType::Pair);
  // Incref the collection object during construction.
  auto obj = collections::alloc(cType);
  vmStack().pushObjectNoRc(obj);
}

OPTBLD_INLINE void iopNewPair() {
  TypedValue* c1 = vmStack().topC();
  TypedValue* c2 = vmStack().indC(1);
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

OPTBLD_INLINE void iopCnsE(const StringData* s) {
  auto const cns = Unit::loadCns(s);
  if (type(cns) == KindOfUninit) {
    raise_error("Undefined constant '%s'", s->data());
  }
  auto const c1 = vmStack().allocC();
  tvCopy(cns, *c1);
}

OPTBLD_INLINE void iopDefCns(uint32_t cid) {
  vmfp()->func()->unit()->defCns(cid);
}

OPTBLD_INLINE void iopClsCns(const StringData* clsCnsName) {
  auto const clsTV = vmStack().topC();
  if (!isClassType(clsTV->m_type)) {
    raise_error("Attempting class constant access on non-class");
  }

  auto const cls = clsTV->m_data.pclass;
  auto const clsCns = cls->clsCnsGet(clsCnsName);

  if (clsCns.m_type == KindOfUninit) {
    raise_error("Couldn't find constant %s::%s",
                cls->name()->data(), clsCnsName->data());
  }

  tvDup(clsCns, *clsTV);
}

OPTBLD_INLINE void iopClsCnsD(const StringData* clsCnsName, Id classId) {
  const NamedEntityPair& classNamedEntity =
    vmfp()->m_func->unit()->lookupNamedEntityPairId(classId);
  auto const clsCns = g_context->lookupClsCns(classNamedEntity.second,
                                       classNamedEntity.first, clsCnsName);
  auto const c1 = vmStack().allocC();
  tvDup(clsCns, *c1);
}

OPTBLD_INLINE void iopConcat() {
  auto const c1 = vmStack().topC();
  auto const c2 = vmStack().indC(1);
  auto const s2 = tvAsVariant(*c2).toString();
  auto const s1 = tvAsCVarRef(*c1).toString();
  tvAsVariant(*c2) = concat(s2, s1);
  assertx(c2->m_data.pstr->checkCount());
  vmStack().popC();
}

OPTBLD_INLINE void iopConcatN(uint32_t n) {
  auto const c1 = vmStack().topC();
  auto const c2 = vmStack().indC(1);

  if (n == 2) {
    auto const s2 = tvAsVariant(*c2).toString();
    auto const s1 = tvAsCVarRef(*c1).toString();
    tvAsVariant(*c2) = concat(s2, s1);
    assertx(c2->m_data.pstr->checkCount());
  } else if (n == 3) {
    auto const c3 = vmStack().indC(2);
    auto const s3 = tvAsVariant(*c3).toString();
    auto const s2 = tvAsCVarRef(*c2).toString();
    auto const s1 = tvAsCVarRef(*c1).toString();
    tvAsVariant(*c3) = concat3(s3, s2, s1);
    assertx(c3->m_data.pstr->checkCount());
  } else {
    assertx(n == 4);
    auto const c3 = vmStack().indC(2);
    auto const c4 = vmStack().indC(3);
    auto const s4 = tvAsVariant(*c4).toString();
    auto const s3 = tvAsCVarRef(*c3).toString();
    auto const s2 = tvAsCVarRef(*c2).toString();
    auto const s1 = tvAsCVarRef(*c1).toString();
    tvAsVariant(*c4) = concat4(s4, s3, s2, s1);
    assertx(c4->m_data.pstr->checkCount());
  }

  for (int i = 1; i < n; ++i) {
    vmStack().popC();
  }
}

OPTBLD_INLINE void iopNot() {
  TypedValue* c1 = vmStack().topC();
  tvAsVariant(*c1) = !tvAsVariant(*c1).toBoolean();
}

template<class Fn>
OPTBLD_INLINE void implTvBinOp(Fn fn) {
  auto const c1 = vmStack().topC();
  auto const c2 = vmStack().indC(1);
  auto const result = fn(*c2, *c1);
  tvDecRefGen(c2);
  *c2 = result;
  vmStack().popC();
}

template<class Fn>
OPTBLD_INLINE void implTvBinOpBool(Fn fn) {
  auto const c1 = vmStack().topC();
  auto const c2 = vmStack().indC(1);
  bool const result = fn(*c2, *c1);
  tvDecRefGen(c2);
  *c2 = make_tv<KindOfBoolean>(result);
  vmStack().popC();
}

template<class Fn>
OPTBLD_INLINE void implTvBinOpInt64(Fn fn) {
  auto const c1 = vmStack().topC();
  auto const c2 = vmStack().indC(1);
  auto const result = fn(*c2, *c1);
  tvDecRefGen(c2);
  *c2 = make_tv<KindOfInt64>(result);
  vmStack().popC();
}

OPTBLD_INLINE void iopAdd() {
  implTvBinOp(tvAdd);
}

OPTBLD_INLINE void iopSub() {
  implTvBinOp(tvSub);
}

OPTBLD_INLINE void iopMul() {
  implTvBinOp(tvMul);
}

OPTBLD_INLINE void iopAddO() {
  implTvBinOp(tvAddO);
}

OPTBLD_INLINE void iopSubO() {
  implTvBinOp(tvSubO);
}

OPTBLD_INLINE void iopMulO() {
  implTvBinOp(tvMulO);
}

OPTBLD_INLINE void iopDiv() {
  implTvBinOp(tvDiv);
}

OPTBLD_INLINE void iopPow() {
  implTvBinOp(tvPow);
}

OPTBLD_INLINE void iopMod() {
  implTvBinOp(tvMod);
}

OPTBLD_INLINE void iopBitAnd() {
  implTvBinOp(tvBitAnd);
}

OPTBLD_INLINE void iopBitOr() {
  implTvBinOp(tvBitOr);
}

OPTBLD_INLINE void iopBitXor() {
  implTvBinOp(tvBitXor);
}

OPTBLD_INLINE void iopXor() {
  implTvBinOpBool([&] (TypedValue c1, TypedValue c2) -> bool {
    return tvToBool(c1) ^ tvToBool(c2);
  });
}

OPTBLD_INLINE void iopSame() {
  implTvBinOpBool(tvSame);
}

OPTBLD_INLINE void iopNSame() {
  implTvBinOpBool([&] (TypedValue c1, TypedValue c2) {
    return !tvSame(c1, c2);
  });
}

OPTBLD_INLINE void iopEq() {
  implTvBinOpBool([&] (TypedValue c1, TypedValue c2) {
    return tvEqual(c1, c2);
  });
}

OPTBLD_INLINE void iopNeq() {
  implTvBinOpBool([&] (TypedValue c1, TypedValue c2) {
    return !tvEqual(c1, c2);
  });
}

OPTBLD_INLINE void iopLt() {
  implTvBinOpBool([&] (TypedValue c1, TypedValue c2) {
    return tvLess(c1, c2);
  });
}

OPTBLD_INLINE void iopLte() {
  implTvBinOpBool(tvLessOrEqual);
}

OPTBLD_INLINE void iopGt() {
  implTvBinOpBool([&] (TypedValue c1, TypedValue c2) {
    return tvGreater(c1, c2);
  });
}

OPTBLD_INLINE void iopGte() {
  implTvBinOpBool(tvGreaterOrEqual);
}

OPTBLD_INLINE void iopCmp() {
  implTvBinOpInt64([&] (TypedValue c1, TypedValue c2) {
    return tvCompare(c1, c2);
  });
}

OPTBLD_INLINE void iopShl() {
  implTvBinOp(tvShl);
}

OPTBLD_INLINE void iopShr() {
  implTvBinOp(tvShr);
}

OPTBLD_INLINE void iopBitNot() {
  tvBitNot(*vmStack().topC());
}

OPTBLD_INLINE void iopCastBool() {
  TypedValue* c1 = vmStack().topC();
  tvCastToBooleanInPlace(c1);
}

OPTBLD_INLINE void iopCastInt() {
  TypedValue* c1 = vmStack().topC();
  tvCastToInt64InPlace(c1);
}

OPTBLD_INLINE void iopCastDouble() {
  TypedValue* c1 = vmStack().topC();
  tvCastToDoubleInPlace(c1);
}

OPTBLD_INLINE void iopCastString() {
  TypedValue* c1 = vmStack().topC();
  tvCastToStringInPlace(c1);
}

OPTBLD_INLINE void iopCastArray() {
  TypedValue* c1 = vmStack().topC();
  tvCastToArrayInPlace(c1);
}

OPTBLD_INLINE void iopCastDict() {
  TypedValue* c1 = vmStack().topC();
  tvCastToDictInPlace(c1);
}

OPTBLD_INLINE void iopCastKeyset() {
  TypedValue* c1 = vmStack().topC();
  tvCastToKeysetInPlace(c1);
}

OPTBLD_INLINE void iopCastVec() {
  TypedValue* c1 = vmStack().topC();
  tvCastToVecInPlace(c1);
}

OPTBLD_INLINE void iopCastVArray() {
  assertx(!RuntimeOption::EvalHackArrDVArrs);
  TypedValue* c1 = vmStack().topC();
  tvCastToVArrayInPlace(c1);
}

OPTBLD_INLINE void iopCastDArray() {
  assertx(!RuntimeOption::EvalHackArrDVArrs);
  TypedValue* c1 = vmStack().topC();
  tvCastToDArrayInPlace(c1);
}

OPTBLD_INLINE void iopDblAsBits() {
  auto c = vmStack().topC();
  if (UNLIKELY(!isDoubleType(c->m_type))) {
    vmStack().replaceC<KindOfInt64>(0);
    return;
  }
  c->m_type = KindOfInt64;
}

ALWAYS_INLINE
bool implInstanceOfHelper(const StringData* str1, TypedValue* c2) {
  const NamedEntity* rhs = NamedEntity::get(str1, false);
  // Because of other codepaths, an un-normalized name might enter the
  // table without a Class* so we need to check if it's there.
  if (LIKELY(rhs && rhs->getCachedClass() != nullptr)) {
    return tvInstanceOf(c2, rhs);
  }
  return false;
}

OPTBLD_INLINE void iopInstanceOf() {
  TypedValue* c1 = vmStack().topC();   // c2 instanceof c1
  TypedValue* c2 = vmStack().indC(1);
  bool r = false;
  if (isStringType(c1->m_type)) {
    r = implInstanceOfHelper(c1->m_data.pstr, c2);
  } else if (c1->m_type == KindOfObject) {
    if (c2->m_type == KindOfObject) {
      ObjectData* lhs = c2->m_data.pobj;
      ObjectData* rhs = c1->m_data.pobj;
      r = lhs->instanceof(rhs->getVMClass());
    }
  } else if (isClassType(c1->m_type)) {
    // TODO (T29639296) Exploit class pointer further
    r = implInstanceOfHelper(c1->m_data.pclass->name(), c2);
  } else {
    raise_error("Class name must be a valid object or a string");
  }
  vmStack().popC();
  vmStack().replaceC<KindOfBoolean>(r);
}

OPTBLD_INLINE void iopInstanceOfD(Id id) {
  const NamedEntity* ne = vmfp()->m_func->unit()->lookupNamedEntityId(id);
  TypedValue* c1 = vmStack().topC();
  bool r = tvInstanceOf(c1, ne);
  vmStack().replaceC<KindOfBoolean>(r);
}

OPTBLD_INLINE void iopIsLateBoundCls() {
  auto const cls = frameStaticClass(vmfp());
  if (!cls) {
    raise_error(HPHP::Strings::THIS_OUTSIDE_CLASS);
  }
  if (isTrait(cls)) {
    raise_error("\"is\" and \"as\" operators cannot be used with a trait");
  }
  auto const c1 = vmStack().topC();
  bool r = tvInstanceOf(c1, cls);
  vmStack().replaceC<KindOfBoolean>(r);
}

namespace {

ArrayData* resolveAndVerifyTypeStructureHelper(
  uint32_t n, const TypedValue* values, bool suppress, bool isOrAsOp) {
  Class* declaringCls = nullptr;
  Class* calledCls = nullptr;
  auto const v = *values;
  isValidTSType(v, true);
  if (typeStructureCouldBeNonStatic(v.m_data.parr)) {
    auto const frame = vmfp();
    if (frame && frame->func()) {
      declaringCls = frame->func()->cls();
      if (declaringCls) {
        calledCls = frame->hasClass()
          ? frame->getClass()
          : frame->getThis()->getVMClass();
      }
    }
  }
  return jit::resolveTypeStructHelper(n, values, declaringCls,
                                      calledCls, suppress, isOrAsOp);
}

ALWAYS_INLINE ArrayData* maybeResolveAndErrorOnTypeStructure(
  TypeStructResolveOp op,
  bool suppress
) {
  auto const a = vmStack().topC();
  isValidTSType(*a, true);

  if (op == TypeStructResolveOp::Resolve) {
    return resolveAndVerifyTypeStructureHelper(1, vmStack().topC(),
                                               suppress, true);
  }
  errorOnIsAsExpressionInvalidTypes(ArrNR(a->m_data.parr), false);
  return a->m_data.parr;
}

} // namespace

OPTBLD_INLINE void iopIsTypeStructC(TypeStructResolveOp op) {
  auto const c = vmStack().indC(1);
  auto const ts = maybeResolveAndErrorOnTypeStructure(op, true);
  auto b = checkTypeStructureMatchesTV(ArrNR(ts), *c);
  vmStack().popC(); // pop ts
  vmStack().replaceC<KindOfBoolean>(b);
}

OPTBLD_INLINE void iopThrowAsTypeStructException() {
  auto const c = vmStack().indC(1);
  auto const ts =
    maybeResolveAndErrorOnTypeStructure(TypeStructResolveOp::Resolve, false);
  std::string givenType, expectedType, errorKey;
  if (!checkTypeStructureMatchesTV(ArrNR(ts), *c, givenType, expectedType,
                                     errorKey)) {
    vmStack().popC(); // pop ts
    throwTypeStructureDoesNotMatchTVException(
      givenType, expectedType, errorKey);
  }
  raise_error("Invalid bytecode sequence: Instruction must throw");
}

OPTBLD_INLINE void iopCombineAndResolveTypeStruct(uint32_t n) {
  assertx(n != 0);
  auto const resolved =
    resolveAndVerifyTypeStructureHelper(n, vmStack().topC(), false, false);
  vmStack().popC(); // pop the first TS
  vmStack().ndiscard(n-1);
  if (RuntimeOption::EvalHackArrDVArrs) {
    vmStack().pushDict(resolved);
  } else {
    vmStack().pushArray(resolved);
  }
}

OPTBLD_INLINE void iopRecordReifiedGeneric() {
  auto const tsList = vmStack().topC();
  if (RuntimeOption::EvalHackArrDVArrs ?
      !tvIsVec(tsList) : !tvIsArray(tsList)) {
    raise_error("Invalid type-structure list in RecordReifiedGeneric");
  }
  // recordReifiedGenericsAndGetTSList decrefs the tsList
  auto const result =
    jit::recordReifiedGenericsAndGetTSList(tsList->m_data.parr);
  vmStack().discard();
  if (RuntimeOption::EvalHackArrDVArrs) {
    vmStack().pushStaticVec(result);
  } else {
    vmStack().pushStaticArray(result);
  }
}

OPTBLD_INLINE void iopCheckReifiedGenericMismatch() {
  Class* cls = arGetContextClass(vmfp());
  if (!cls) raise_error("No class scope is active");
  auto const c = vmStack().topC();
  if (RuntimeOption::EvalHackArrDVArrs ?
      !tvIsVec(c) : !tvIsArray(c)) {
    raise_error("Invalid type-structure list in CheckReifiedGenericMismatch");
  }
  checkClassReifiedGenericMismatch(cls, c->m_data.parr);
  vmStack().popC();
}

OPTBLD_INLINE void iopPrint() {
  TypedValue* c1 = vmStack().topC();
  g_context->write(tvAsVariant(*c1).toString());
  vmStack().replaceC<KindOfInt64>(1);
}

OPTBLD_INLINE void iopClone() {
  TypedValue* tv = vmStack().topTV();
  if (tv->m_type != KindOfObject) {
    raise_error("clone called on non-object");
  }
  auto newobj = tv->m_data.pobj->clone();
  vmStack().popTV();
  vmStack().pushObjectNoRc(newobj);
}

OPTBLD_INLINE void iopExit() {
  int exitCode = 0;
  TypedValue* c1 = vmStack().topC();
  if (c1->m_type == KindOfInt64) {
    exitCode = c1->m_data.num;
  } else {
    g_context->write(tvAsVariant(*c1).toString());
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
    if (flags & TimedOutFlag) {
      RID().invokeUserTimeoutCallback();
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

  TypedValue* c1 = vmStack().topC();
  if (c1->m_type == KindOfInt64 || c1->m_type == KindOfBoolean) {
    int64_t n = c1->m_data.num;
    vmStack().popX();
    if (op == OpJmpZ ? n == 0 : n != 0) pc = targetpc;
  } else {
    auto const cond = tvAsCVarRef(*c1).toBoolean();
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

OPTBLD_INLINE void iopSelect() {
  auto const cond = [&]{
    auto c = vmStack().topC();
    if (c->m_type == KindOfInt64 || c->m_type == KindOfBoolean) {
      auto const val = (bool)c->m_data.num;
      vmStack().popX();
      return val;
    } else {
      auto const val = tvAsCVarRef(*c).toBoolean();
      vmStack().popC();
      return val;
    }
  }();

  if (cond) {
    auto const t = *vmStack().topC();
    vmStack().discard();
    vmStack().replaceC(t);
  } else {
    vmStack().popC();
  }
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

        case KindOfFunc:
        case KindOfClass:
        case KindOfPersistentString:
        case KindOfString: {
          double dval = 0.0;
          auto const str =
            isFuncType(val->m_type) ? funcToStringHelper(val->m_data.pfunc) :
            isClassType(val->m_type) ? classToStringHelper(val->m_data.pclass) :
            val->m_data.pstr;
          DataType t = str->isNumericWithVal(intval, dval, 1);
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
            case KindOfPersistentDArray:
            case KindOfDArray:
            case KindOfPersistentVArray:
            case KindOfVArray:
            case KindOfPersistentArray:
            case KindOfArray:
            case KindOfObject:
            case KindOfResource:
            case KindOfFunc:
            case KindOfClass:
            case KindOfClsMeth:
            case KindOfRecord:
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

        case KindOfDArray:
        case KindOfVArray:
        case KindOfArray:
          tvDecRefArr(val);
        case KindOfPersistentDArray:
        case KindOfPersistentVArray:
        case KindOfPersistentArray:
          match = SwitchMatch::DEFAULT;
          return;

        case KindOfClsMeth:
          tvDecRefClsMeth(val);
          match = SwitchMatch::DEFAULT;
          break;

        case KindOfObject:
          intval = val->m_data.pobj->toInt64();
          tvDecRefObj(val);
          return;

        case KindOfResource:
          intval = val->m_data.pres->data()->o_toInt64();
          tvDecRefRes(val);
          return;

        case KindOfRecord: // TODO (T41029094)
          raise_error(Strings::RECORD_NOT_SUPPORTED);
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
  TypedValue* val = vmStack().topTV();
  Unit* u = vmfp()->m_func->unit();
  unsigned i;
  for (i = 0; i < cases; ++i) {
    auto item = jmptab[i];
    const StringData* str = u->lookupLitstrId(item.str);
    if (tvEqual(*val, str)) {
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
  Offset callOff;
};

OPTBLD_INLINE JitReturn jitReturnPre(ActRec* fp) {
  auto savedRip = fp->m_savedRip;
  auto const isRetHelper = isReturnHelper(reinterpret_cast<void*>(savedRip));
  if (isRetHelper) {
    // This frame wasn't called from the TC, so it's ok to return using the
    // interpreter. callToExit is special: it's a return helper but we don't
    // treat it like one in here in order to simplify some things higher up in
    // the pipeline.
    if (reinterpret_cast<TCA>(savedRip) != jit::tc::ustubs().callToExit) {
      savedRip = 0;
    }
  }
  assertx(isRetHelper || RID().getJit());

  return {savedRip, fp, fp->sfp(), fp->callOffset()};
}

OPTBLD_INLINE TCA jitReturnPost(JitReturn retInfo) {
  if (retInfo.savedRip) {
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

OPTBLD_INLINE void returnToCaller(PC& pc, ActRec* sfp, Offset callOff) {
  vmfp() = sfp;
  pc = LIKELY(sfp != nullptr)
    ? skipCall(sfp->func()->getEntry() + callOff)
    : nullptr;
}

}

template <bool suspended>
OPTBLD_INLINE TCA ret(PC& pc) {
  assertx(!suspended || vmfp()->func()->isAsyncFunction());
  assertx(!suspended || !isResumed(vmfp()));

  auto const jitReturn = jitReturnPre(vmfp());

  // Get the return value.
  TypedValue retval = *vmStack().topTV();
  vmStack().discard();

  assertx(
    !suspended || (tvIsObject(retval) && retval.m_data.pobj->isWaitHandle())
  );

  // Free $this and local variables. Calls FunctionReturn hook. The return
  // value must be removed from the stack, or the unwinder would try to free it
  // if the hook throws---but the event hook routine decrefs the return value
  // in that case if necessary.
  frame_free_locals_inl(vmfp(), vmfp()->func()->numLocals(), &retval);

  // Grab caller info from ActRec.
  ActRec* sfp = vmfp()->sfp();
  Offset callOff = vmfp()->callOffset();

  if (LIKELY(!isResumed(vmfp()))) {
    // If in an eagerly executed async function, wrap the return value into
    // succeeded StaticWaitHandle. Async eager return requests are currently
    // not respected, as we don't have a way to obtain the async eager offset.
    if (UNLIKELY(vmfp()->func()->isAsyncFunction()) && !suspended) {
      auto const& retvalCell = *tvAssertPlausible(&retval);
      // Heads up that we're assuming CreateSucceeded can't throw, or we won't
      // decref the return value.  (It can't right now.)
      auto const waitHandle = c_StaticWaitHandle::CreateSucceeded(retvalCell);
      tvCopy(make_tv<KindOfObject>(waitHandle), retval);
    }

    // Free ActRec and store the return value.
    vmStack().ndiscard(vmfp()->func()->numSlotsInFrame());
    vmStack().ret();
    *vmStack().topTV() = retval;
    assertx(vmStack().topTV() == vmfp()->retSlot());
    // In case async eager return was requested by the caller, pretend that
    // we did not finish eagerly as we already boxed the value.
    vmStack().topTV()->m_aux.u_asyncEagerReturnFlag = 0;
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
  returnToCaller(pc, sfp, callOff);

  return jitReturnPost(jitReturn);
}

OPTBLD_INLINE TCA iopRetC(PC& pc) {
  return ret<false>(pc);
}

OPTBLD_INLINE TCA iopRetCSuspended(PC& pc) {
  assertx(vmfp()->func()->isAsyncFunction());
  assertx(!isResumed(vmfp()));
  return ret<true>(pc);
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

  // Grab caller info from ActRec.
  ActRec* sfp = vmfp()->sfp();
  Offset callOff = vmfp()->callOffset();

  // Free ActRec and store the return value.
  vmStack().ndiscard(vmfp()->func()->numSlotsInFrame());
  vmStack().ret();

  // Discard scratch space for return values allocated for multi return FCall
  vmStack().ndiscard(numRet - 1);
  *vmStack().topTV() = retvals[1];

  for (int i = 2; i < numRet; i++) {
    *vmStack().allocTV() = retvals[i];
  }

  // Store the actual return value at the top of the stack
  *vmStack().allocTV() = retvals[0];

  // Return control to the caller.
  returnToCaller(pc, sfp, callOff);

  return jitReturnPost(jitReturn);
}

OPTBLD_INLINE void iopThrow(PC&) {
  TypedValue* c1 = vmStack().topC();
  if (c1->m_type != KindOfObject ||
      !c1->m_data.pobj->instanceof(SystemLib::s_ThrowableClass)) {
    raise_error("Exceptions must implement the Throwable interface.");
  }
  auto obj = Object::attach(c1->m_data.pobj);
  vmStack().discard();
  DEBUGGER_ATTACHED_ONLY(phpDebuggerExceptionThrownHook(obj.get()));
  throw req::root<Object>(std::move(obj));
}

OPTBLD_INLINE void iopThrowNonExhaustiveSwitch() {
  switch (RuntimeOption::EvalThrowOnNonExhaustiveSwitch) {
    case 0:
      return;
    case 1:
      raise_warning(Strings::NONEXHAUSTIVE_SWITCH);
      return;
    default:
      SystemLib::throwRuntimeExceptionObject(
        String(Strings::NONEXHAUSTIVE_SWITCH));
  }
  not_reached();
}

OPTBLD_INLINE void iopClassGetC() {
  auto const cell = vmStack().topC();
  if (isStringType(cell->m_type)) {
    raise_str_to_class_notice(cell->m_data.pstr);
  }
  auto const cls = lookupClsRef(cell);
  vmStack().popC();
  vmStack().pushClass(cls);
}

OPTBLD_INLINE void iopClassGetTS() {
  auto const cell = vmStack().topC();
  if (!tvIsDictOrDArray(cell)) {
    raise_error("Reified type must be a type structure");
  }
  auto const ts = cell->m_data.parr;
  auto const classname_field = ts->rval(s_classname.get());
  if (!classname_field.is_set()) {
    raise_error("You cannot create a new instance of this type as "
                "it is not a class");
  }
  assertx(isStringType(classname_field.type()));
  auto const name = classname_field.val().pstr;
  auto const generics_field = ts->rval(s_generic_types.get());
  ArrayData* reified_types = nullptr;
  if (generics_field.is_set()) {
    reified_types = generics_field.val().parr;
    auto const mangledTypeName =
      makeStaticString(mangleReifiedGenericsName(reified_types));
    reified_types->incRefCount();
    reified_types = addToReifiedGenericsTable(mangledTypeName, reified_types);
  }
  auto const cls = Unit::loadClass(name);
  if (cls == nullptr) {
    raise_error(Strings::UNKNOWN_CLASS, name->data());
  }

  vmStack().popC();
  vmStack().pushClass(cls);
  if (reified_types) {
    if (RuntimeOption::EvalHackArrDVArrs) {
      vmStack().pushStaticVec(reified_types);
    } else {
      vmStack().pushStaticArray(reified_types);
    }
  } else {
    vmStack().pushNull();
  }
}

static void raise_undefined_local(ActRec* fp, LocalName pind) {
  assertx(pind < fp->m_func->numNamedLocals());
  assertx(fp->m_func->localVarName(pind));
  if (debug) {
    auto vm = &*g_context;
    always_assert_flog(
      pind != kInvalidLocalName,
      "HHBBC incorrectly removed name info for a local in {}:{}",
      vm->getContainingFileName()->data(),
      vm->getLine()
    );
  }
  raise_notice(Strings::UNDEFINED_VARIABLE,
               fp->m_func->localVarName(pind)->data());
}

static inline void cgetl_inner_body(TypedValue* fr, TypedValue* to) {
  assertx(fr->m_type != KindOfUninit);
  tvDup(*fr, *to);
}

OPTBLD_INLINE void cgetl_body(ActRec* fp,
                              TypedValue* fr,
                              TypedValue* to,
                              LocalName lname,
                              bool warn) {
  if (fr->m_type == KindOfUninit) {
    // `to' is uninitialized here, so we need to tvWriteNull before
    // possibly causing stack unwinding.
    tvWriteNull(*to);
    if (warn) raise_undefined_local(fp, lname);
  } else {
    cgetl_inner_body(fr, to);
  }
}

OPTBLD_INLINE void iopCGetL(named_local_var fr) {
  TypedValue* to = vmStack().allocC();
  cgetl_body(vmfp(), fr.ptr, to, fr.name, true);
}

OPTBLD_INLINE void iopCGetQuietL(TypedValue* fr) {
  TypedValue* to = vmStack().allocC();
  cgetl_body(vmfp(), fr, to, kInvalidLocalName, false);
}

OPTBLD_INLINE void iopCUGetL(TypedValue* fr) {
  auto to = vmStack().allocTV();
  tvDup(*fr, *to);
}

OPTBLD_INLINE void iopCGetL2(named_local_var fr) {
  TypedValue* oldTop = vmStack().topTV();
  TypedValue* newTop = vmStack().allocTV();
  memcpy(newTop, oldTop, sizeof *newTop);
  TypedValue* to = oldTop;
  cgetl_body(vmfp(), fr.ptr, to, fr.name, true);
}

OPTBLD_INLINE void iopPushL(TypedValue* locVal) {
  assertx(locVal->m_type != KindOfUninit);
  TypedValue* dest = vmStack().allocTV();
  *dest = *locVal;
  locVal->m_type = KindOfUninit;
}

OPTBLD_INLINE void iopCGetG() {
  StringData* name;
  TypedValue* to = vmStack().topTV();
  TypedValue* fr = nullptr;
  lookup_gbl(vmfp(), name, to, fr);
  SCOPE_EXIT { decRefStr(name); };
  tvDecRefGen(to);
  if (fr == nullptr || fr->m_type == KindOfUninit) {
    tvWriteNull(*to);
  } else {
    cgetl_inner_body(fr, to);
  }
}

struct SpropState {
  SpropState(Stack&, bool ignoreLateInit);
  ~SpropState();
  StringData* name;
  Class* cls;
  TypedValue* output;
  TypedValue* val;
  TypedValue oldNameCell;
  Slot slot;
  bool visible;
  bool accessible;
  bool constant;
  Stack& vmstack;
};

SpropState::SpropState(Stack& vmstack, bool ignoreLateInit) : vmstack{vmstack} {
  auto const clsCell = vmstack.topC();
  auto const nameCell = output = vmstack.indTV(1);
  if (!isClassType(clsCell->m_type)) {
    raise_error("SpropState: expected class");
  }
  cls = clsCell->m_data.pclass;
  lookup_sprop(vmfp(), cls, name, nameCell, val,
               slot, visible, accessible, constant, ignoreLateInit);
  oldNameCell = *nameCell;
}

SpropState::~SpropState() {
  vmstack.discard();
  decRefStr(name);
  tvDecRefGen(oldNameCell);
}

OPTBLD_INLINE void iopCGetS() {
  SpropState ss(vmStack(), false);
  if (!(ss.visible && ss.accessible)) {
    raise_error("Invalid static property access: %s::%s",
                ss.cls->name()->data(),
                ss.name->data());
  }
  tvDup(*ss.val, *ss.output);
}

static inline MInstrState& initMState() {
  auto& mstate = vmMInstrState();
  tvWriteUninit(mstate.tvRef);
  tvWriteUninit(mstate.tvRef2);
  mstate.propState = MInstrPropState{};
  return mstate;
}

static inline void baseGImpl(TypedValue* key, MOpMode mode) {
  auto& mstate = initMState();
  StringData* name;
  TypedValue* baseVal;

  if (mode == MOpMode::Define) lookupd_gbl(vmfp(), name, key, baseVal);
  else                         lookup_gbl(vmfp(), name, key, baseVal);
  SCOPE_EXIT { decRefStr(name); };

  if (baseVal == nullptr) {
    assertx(mode != MOpMode::Define);
    if (mode == MOpMode::Warn) throwArrayKeyException(name, false);
    tvWriteNull(mstate.tvTempBase);
    mstate.base = &mstate.tvTempBase;
    return;
  }

  mstate.base = baseVal;
}

OPTBLD_INLINE void iopBaseGC(uint32_t idx, MOpMode mode) {
  baseGImpl(vmStack().indTV(idx), mode);
}

OPTBLD_INLINE void iopBaseGL(TypedValue* loc, MOpMode mode) {
  baseGImpl(loc, mode);
}

OPTBLD_INLINE void iopBaseSC(uint32_t keyIdx, uint32_t clsIdx, MOpMode mode) {
  auto& mstate = initMState();

  auto const clsCell = vmStack().indC(clsIdx);
  auto const key = vmStack().indTV(keyIdx);

  if (!isClassType(clsCell->m_type)) {
    raise_error("Attempting to obtain static base on non-class");
  }
  auto const class_ = clsCell->m_data.pclass;

  auto const name = lookup_name(key);
  SCOPE_EXIT { decRefStr(name); };
  auto const lookup = class_->getSProp(arGetContextClass(vmfp()), name);
  if (!lookup.val || !lookup.accessible) {
    raise_error("Invalid static property access: %s::%s",
                class_->name()->data(),
                name->data());
  }

  if (lookup.constant && (mode == MOpMode::Define ||
    mode == MOpMode::Unset || mode == MOpMode::InOut)) {
      throw_cannot_modify_static_const_prop(class_->name()->data(),
        name->data());
  }

  if (RuntimeOption::EvalCheckPropTypeHints > 0 && mode == MOpMode::Define) {
    vmMInstrState().propState = MInstrPropState{class_, lookup.slot, true};
  }

  mstate.base = tv_lval(lookup.val);
}

OPTBLD_INLINE void baseLImpl(named_local_var loc, MOpMode mode) {
  auto& mstate = initMState();
  auto local = loc.ptr;
  if (mode == MOpMode::Warn && local->m_type == KindOfUninit) {
    raise_undefined_local(vmfp(), loc.name);
  }
  mstate.base = local;
}

OPTBLD_INLINE void iopBaseL(named_local_var loc, MOpMode mode) {
  baseLImpl(loc, mode);
}

OPTBLD_INLINE void iopBaseC(uint32_t idx, MOpMode) {
  auto& mstate = initMState();
  mstate.base = vmStack().indC(idx);
}

OPTBLD_INLINE void iopBaseH() {
  auto& mstate = initMState();
  mstate.tvTempBase = make_tv<KindOfObject>(vmfp()->getThis());
  mstate.base = &mstate.tvTempBase;
}

static OPTBLD_INLINE void propDispatch(MOpMode mode, TypedValue key) {
  auto& mstate = vmMInstrState();
  auto pState = &mstate.propState;
  auto ctx = arGetContextClass(vmfp());

  auto const result = [&]{
    switch (mode) {
      case MOpMode::None:
        return Prop<MOpMode::None>(mstate.tvRef, ctx, mstate.base, key, pState);
      case MOpMode::Warn:
        return Prop<MOpMode::Warn>(mstate.tvRef, ctx, mstate.base, key, pState);
      case MOpMode::Define:
        return Prop<MOpMode::Define,KeyType::Any>(
          mstate.tvRef, ctx, mstate.base, key, pState
        );
      case MOpMode::Unset:
        return Prop<MOpMode::Unset>(
          mstate.tvRef, ctx, mstate.base, key, pState
        );
      case MOpMode::InOut:
        always_assert_flog(false, "MOpMode::InOut can only occur on Elem");
    }
    always_assert(false);
  }();

  mstate.base = ratchetRefs(result, mstate.tvRef, mstate.tvRef2);
}

static OPTBLD_INLINE void propQDispatch(MOpMode mode, TypedValue key) {
  auto& mstate = vmMInstrState();
  auto ctx = arGetContextClass(vmfp());

  assertx(mode == MOpMode::None || mode == MOpMode::Warn);
  assertx(key.m_type == KindOfPersistentString);
  auto const result = nullSafeProp(mstate.tvRef, ctx, mstate.base,
                                   key.m_data.pstr);
  mstate.base = ratchetRefs(result, mstate.tvRef, mstate.tvRef2);
}

static OPTBLD_INLINE
void elemDispatch(MOpMode mode, TypedValue key) {
  auto& mstate = vmMInstrState();
  auto const b = mstate.base;

  auto const result = [&]() -> tv_rval {
    switch (mode) {
      case MOpMode::None:
        return Elem<MOpMode::None>(mstate.tvRef, b, key);
      case MOpMode::Warn:
        return Elem<MOpMode::Warn>(mstate.tvRef, b, key);
      case MOpMode::InOut:
        return Elem<MOpMode::InOut>(mstate.tvRef, b, key);
      case MOpMode::Define:
        if (RuntimeOption::EvalArrayProvenance) {
          return ElemD<MOpMode::Define, KeyType::Any, true>(
            mstate.tvRef, b, key, &mstate.propState
          );
        } else {
          return ElemD<MOpMode::Define, KeyType::Any, false>(
            mstate.tvRef, b, key, &mstate.propState
          );
        }
      case MOpMode::Unset:
        return ElemU(mstate.tvRef, b, key);
    }
    always_assert(false);
  }().as_lval();

  if (mode == MOpMode::Define) mstate.propState = MInstrPropState{};
  mstate.base = ratchetRefs(result, mstate.tvRef, mstate.tvRef2);
}

static inline TypedValue key_tv(MemberKey key) {
  switch (key.mcode) {
    case MW:
      return TypedValue{};
    case MEL: case MPL: {
      auto local = frame_local(vmfp(), key.local.id);
      if (local->m_type == KindOfUninit) {
        raise_undefined_local(vmfp(), key.local.name);
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

static OPTBLD_INLINE void dimDispatch(MOpMode mode, MemberKey mk) {
  auto const key = key_tv(mk);
  if (mk.mcode == MQT) {
    propQDispatch(mode, key);
  } else if (mcodeIsProp(mk.mcode)) {
    propDispatch(mode, key);
  } else if (mcodeIsElem(mk.mcode)) {
    elemDispatch(mode, key);
  } else {
    if (mode == MOpMode::Warn) raise_error("Cannot use [] for reading");

    auto& mstate = vmMInstrState();
    auto const base = mstate.base;
    auto const result = [&] {
      return NewElem(mstate.tvRef, base, &mstate.propState);
    }();
    if (mode == MOpMode::Define) mstate.propState = MInstrPropState{};
    mstate.base = ratchetRefs(result, mstate.tvRef, mstate.tvRef2);
  }
}

OPTBLD_INLINE void iopDim(MOpMode mode, MemberKey mk) {
  dimDispatch(mode, mk);
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
      dimDispatch(getQueryMOpMode(op), mk);
      tvDup(*mstate.base, result);
      break;

    case QueryMOp::Isset:
      result.m_type = KindOfBoolean;
      if (mcodeIsProp(mk.mcode)) {
        auto const ctx = arGetContextClass(vmfp());
        result.m_data.num = IssetProp(ctx, mstate.base, key);
      } else {
        assertx(mcodeIsElem(mk.mcode));
        result.m_data.num = IssetElem(mstate.base, key);
      }
      break;
  }
  mFinal(mstate, nDiscard, result);
}

OPTBLD_INLINE void iopQueryM(uint32_t nDiscard, QueryMOp subop, MemberKey mk) {
  queryMImpl(mk, nDiscard, subop);
}

OPTBLD_INLINE void iopSetM(uint32_t nDiscard, MemberKey mk) {
  auto& mstate = vmMInstrState();
  auto const topC = vmStack().topC();

  if (mk.mcode == MW) {
    if (RuntimeOption::EvalArrayProvenance) {
      SetNewElem<true, true>(mstate.base, topC, &mstate.propState);
    } else {
      SetNewElem<true, false>(mstate.base, topC, &mstate.propState);
    }
  } else {
    auto const key = key_tv(mk);
    if (mcodeIsElem(mk.mcode)) {
      auto const result = RuntimeOption::EvalArrayProvenance
        ? SetElem<true, true>(mstate.base, key, topC, &mstate.propState)
        : SetElem<true, false>(mstate.base, key, topC, &mstate.propState);

      if (result) {
        tvDecRefGen(topC);
        topC->m_type = KindOfString;
        topC->m_data.pstr = result;
      }
    } else {
      auto const ctx = arGetContextClass(vmfp());
      SetProp<true>(ctx, mstate.base, key, topC, &mstate.propState);
    }
  }

  auto const result = *topC;
  vmStack().discard();
  mFinal(mstate, nDiscard, result);
}

OPTBLD_INLINE void iopSetRangeM(
  uint32_t nDiscard, SetRangeOp op, uint32_t size
) {
  auto& mstate = vmMInstrState();
  auto const count = tvCastToInt64(*vmStack().indC(0));
  auto const src = *vmStack().indC(1);
  auto const offset = tvCastToInt64(*vmStack().indC(2));

  if (op == SetRangeOp::Forward) {
    SetRange<false>(mstate.base, offset, src, count, size);
  } else {
    SetRange<true>(mstate.base, offset, src, count, size);
  }

  mFinal(mstate, nDiscard + 3, folly::none);
}

OPTBLD_INLINE void iopIncDecM(uint32_t nDiscard, IncDecOp subop, MemberKey mk) {
  auto const key = key_tv(mk);

  auto& mstate = vmMInstrState();
  TypedValue result;
  if (mcodeIsProp(mk.mcode)) {
    result = IncDecProp(
      arGetContextClass(vmfp()), subop, mstate.base, key, &mstate.propState
    );
  } else if (mcodeIsElem(mk.mcode)) {
    result = IncDecElem(
      subop, mstate.base, key, &mstate.propState
    );
  } else {
    result = IncDecNewElem(mstate.tvRef, subop, mstate.base, &mstate.propState);
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
                       mstate.base, key, rhs, &mstate.propState);
  } else if (mcodeIsElem(mk.mcode)) {
    result = SetOpElem(
      mstate.tvRef, subop, mstate.base, key, rhs, &mstate.propState
    );
  } else {
    result =
      SetOpNewElem(mstate.tvRef, subop, mstate.base, rhs, &mstate.propState);
  }

  vmStack().popC();
  tvIncRefGen(*result);
  mFinal(mstate, nDiscard, *result);
}

OPTBLD_INLINE void iopUnsetM(uint32_t nDiscard, MemberKey mk) {
  auto const key = key_tv(mk);

  auto& mstate = vmMInstrState();
  if (mcodeIsProp(mk.mcode)) {
    UnsetProp(arGetContextClass(vmfp()), mstate.base, key);
  } else {
    assertx(mcodeIsElem(mk.mcode));
    UnsetElem(mstate.base, key);
  }

  mFinal(mstate, nDiscard, folly::none);
}

namespace {

inline void checkThis(ActRec* fp) {
  if (!fp->func()->cls() || !fp->hasThis()) {
    raise_error(Strings::FATAL_NULL_THIS);
  }
}

OPTBLD_INLINE const TypedValue* memoGetImpl(LocalRange keys) {
  assertx(vmfp()->m_func->isMemoizeWrapper());
  assertx(keys.first + keys.count <= vmfp()->m_func->numLocals());

  for (auto i = 0; i < keys.count; ++i) {
    auto const key = frame_local(vmfp(), keys.first + i);
    if (!isIntType(key->m_type) && !isStringType(key->m_type)) {
      raise_error("Memoization keys can only be ints or strings");
    }
  }

  auto const c = [&] () -> const TypedValue* {
    auto const func = vmfp()->m_func;
    if (!func->isMethod() || func->isStatic()) {
      auto const lsbCls =
        func->isMemoizeWrapperLSB() ? vmfp()->getClass() : nullptr;
      if (keys.count > 0) {
        auto cache =
          lsbCls ? rds::bindLSBMemoCache(lsbCls, func)
                 : rds::bindStaticMemoCache(func);
        if (!cache.isInit()) return nullptr;
        auto const keysBegin = frame_local(vmfp(), keys.first + keys.count - 1);
        if (auto getter = memoCacheGetForKeyCount(keys.count)) {
          return getter(*cache, keysBegin);
        }
        return memoCacheGetGeneric(
          *cache,
          GenericMemoId{func->getFuncId(), keys.count}.asParam(),
          keysBegin
        );
      }

      auto cache =
        lsbCls ? rds::bindLSBMemoValue(lsbCls, func)
               : rds::bindStaticMemoValue(func);
      return cache.isInit() ? cache.get() : nullptr;
    }

    checkThis(vmfp());
    auto const this_ = vmfp()->getThis();
    auto const cls = func->cls();
    assertx(this_->instanceof(cls));
    assertx(cls->hasMemoSlots());

    auto const memoInfo = cls->memoSlotForFunc(func->getFuncId());

    auto const slot = UNLIKELY(this_->hasNativeData())
      ? this_->memoSlotNativeData(memoInfo.first, cls->getNativeDataInfo()->sz)
      : this_->memoSlot(memoInfo.first);

    if (keys.count == 0 && !memoInfo.second) {
      auto const val = slot->getValue();
      return val->m_type != KindOfUninit ? val : nullptr;
    }

    auto const cache = slot->getCache();
    if (!cache) return nullptr;

    if (memoInfo.second) {
      if (keys.count == 0) {
        return memoCacheGetSharedOnly(
          cache,
          makeSharedOnlyKey(func->getFuncId())
        );
      }
      auto const keysBegin = frame_local(vmfp(), keys.first + keys.count - 1);
      if (auto const getter = sharedMemoCacheGetForKeyCount(keys.count)) {
        return getter(cache, func->getFuncId(), keysBegin);
      }
      return memoCacheGetGeneric(
        cache,
        GenericMemoId{func->getFuncId(), keys.count}.asParam(),
        keysBegin
      );
    }

    assertx(keys.count > 0);
    auto const keysBegin = frame_local(vmfp(), keys.first + keys.count - 1);
    if (auto const getter = memoCacheGetForKeyCount(keys.count)) {
      return getter(cache, keysBegin);
    }
    return memoCacheGetGeneric(
      cache,
      GenericMemoId{func->getFuncId(), keys.count}.asParam(),
      keysBegin
    );
  }();

  assertx(!c || tvIsPlausible(*c));
  assertx(!c || c->m_type != KindOfUninit);
  return c;
}

}

OPTBLD_INLINE void iopMemoGet(PC& pc, PC notfound, LocalRange keys) {
  if (auto const c = memoGetImpl(keys)) {
    tvDup(*c, *vmStack().allocC());
  } else {
    pc = notfound;
  }
}

OPTBLD_INLINE void iopMemoGetEager(PC& pc,
                                   PC notfound,
                                   PC suspended,
                                   LocalRange keys) {
  assertx(vmfp()->m_func->isAsyncFunction());
  assertx(!isResumed(vmfp()));

  if (auto const c = memoGetImpl(keys)) {
    tvDup(*c, *vmStack().allocC());
    if (!c->m_aux.u_asyncEagerReturnFlag) {
      assertx(tvIsObject(c) && c->m_data.pobj->isWaitHandle());
      pc = suspended;
    }
  } else {
    pc = notfound;
  }
}

namespace {

OPTBLD_INLINE void memoSetImpl(LocalRange keys, TypedValue val) {
  assertx(vmfp()->m_func->isMemoizeWrapper());
  assertx(keys.first + keys.count <= vmfp()->m_func->numLocals());
  assertx(tvIsPlausible(val));

  for (auto i = 0; i < keys.count; ++i) {
    auto const key = frame_local(vmfp(), keys.first + i);
    if (!isIntType(key->m_type) && !isStringType(key->m_type)) {
      raise_error("Memoization keys can only be ints or strings");
    }
  }

  auto const func = vmfp()->m_func;
  if (!func->isMethod() || func->isStatic()) {
    auto const lsbCls =
      func->isMemoizeWrapperLSB() ? vmfp()->getClass() : nullptr;
    if (keys.count > 0) {
      auto cache =
        lsbCls ? rds::bindLSBMemoCache(lsbCls, func)
               : rds::bindStaticMemoCache(func);
      if (!cache.isInit()) cache.initWith(nullptr);
      auto const keysBegin = frame_local(vmfp(), keys.first + keys.count - 1);
      if (auto setter = memoCacheSetForKeyCount(keys.count)) {
        return setter(*cache, keysBegin, val);
      }
      return memoCacheSetGeneric(
        *cache,
        GenericMemoId{func->getFuncId(), keys.count}.asParam(),
        keysBegin,
        val
      );
    }

    auto cache =
      lsbCls ? rds::bindLSBMemoValue(lsbCls, func)
             : rds::bindStaticMemoValue(func);
    if (!cache.isInit()) {
      tvWriteUninit(*cache);
      cache.markInit();
    }

    tvSetWithAux(val, *cache);
    return;
  }

  checkThis(vmfp());
  auto const this_ = vmfp()->getThis();
  auto const cls = func->cls();
  assertx(this_->instanceof(cls));
  assertx(cls->hasMemoSlots());

  this_->setAttribute(ObjectData::UsedMemoCache);

  auto const memoInfo = cls->memoSlotForFunc(func->getFuncId());

  auto slot = UNLIKELY(this_->hasNativeData())
    ? this_->memoSlotNativeData(memoInfo.first, cls->getNativeDataInfo()->sz)
    : this_->memoSlot(memoInfo.first);

  if (keys.count == 0 && !memoInfo.second) {
    tvSetWithAux(val, *slot->getValue());
    return;
  }

  auto& cache = slot->getCacheForWrite();

  if (memoInfo.second) {
    if (keys.count == 0) {
      return memoCacheSetSharedOnly(
        cache,
        makeSharedOnlyKey(func->getFuncId()),
        val
      );
    }
    auto const keysBegin = frame_local(vmfp(), keys.first + keys.count - 1);
    if (auto const setter = sharedMemoCacheSetForKeyCount(keys.count)) {
      return setter(cache, func->getFuncId(), keysBegin, val);
    }
    return memoCacheSetGeneric(
      cache,
      GenericMemoId{func->getFuncId(), keys.count}.asParam(),
      keysBegin,
      val
    );
  }

  assertx(keys.count > 0);
  auto const keysBegin = frame_local(vmfp(), keys.first + keys.count - 1);
  if (auto const setter = memoCacheSetForKeyCount(keys.count)) {
    return setter(cache, keysBegin, val);
  }
  return memoCacheSetGeneric(
    cache,
    GenericMemoId{func->getFuncId(), keys.count}.asParam(),
    keysBegin,
    val
  );
}

}

OPTBLD_INLINE void iopMemoSet(LocalRange keys) {
  auto val = *vmStack().topC();
  assertx(val.m_type != KindOfUninit);
  if (vmfp()->m_func->isAsyncFunction()) {
    assertx(tvIsObject(val) && val.m_data.pobj->isWaitHandle());
    val.m_aux.u_asyncEagerReturnFlag = 0;
  }
  memoSetImpl(keys, val);
}

OPTBLD_INLINE void iopMemoSetEager(LocalRange keys) {
  assertx(vmfp()->m_func->isAsyncFunction());
  assertx(!isResumed(vmfp()));
  auto val = *vmStack().topC();
  assertx(val.m_type != KindOfUninit);
  val.m_aux.u_asyncEagerReturnFlag = static_cast<uint32_t>(-1);
  memoSetImpl(keys, val);
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
    e = !tvIsNull(tv);
  }
  vmStack().replaceC<KindOfBoolean>(e);
}

OPTBLD_INLINE void iopIssetS() {
  SpropState ss(vmStack(), true);
  bool e;
  if (!(ss.visible && ss.accessible)) {
    e = false;
  } else {
    e = !tvIsNull(ss.val);
  }
  ss.output->m_data.num = e;
  ss.output->m_type = KindOfBoolean;
}

OPTBLD_INLINE void iopIssetL(TypedValue* tv) {
  bool ret = !is_null(tv);
  TypedValue* topTv = vmStack().allocTV();
  topTv->m_data.num = ret;
  topTv->m_type = KindOfBoolean;
}

OPTBLD_INLINE void iopIsUnsetL(TypedValue* tv) {
  bool ret = type(tv) == KindOfUninit;
  TypedValue* topTv = vmStack().allocTV();
  topTv->m_data.num = ret;
  topTv->m_type = KindOfBoolean;
}

OPTBLD_INLINE static bool isTypeHelper(TypedValue* val, IsTypeOp op) {
  assertx(tvIsPlausible(*val));

  switch (op) {
  case IsTypeOp::Null:   return is_null(val);
  case IsTypeOp::Bool:   return is_bool(val);
  case IsTypeOp::Int:    return is_int(val);
  case IsTypeOp::Dbl:    return is_double(val);
  case IsTypeOp::Arr:    return is_array(val, !vmfp()->m_func->isBuiltin());
  case IsTypeOp::PHPArr: return is_array(val, /* logOnHackArrays = */ false);
  case IsTypeOp::Vec:    return is_vec(val);
  case IsTypeOp::Dict:   return is_dict(val);
  case IsTypeOp::Keyset: return is_keyset(val);
  case IsTypeOp::VArray: return is_varray(val);
  case IsTypeOp::DArray: return is_darray(val);
  case IsTypeOp::Obj:    return is_object(val);
  case IsTypeOp::Str:    return is_string(val);
  case IsTypeOp::Res:    return val->m_type == KindOfResource;
  case IsTypeOp::Scalar: return HHVM_FN(is_scalar)(tvAsCVarRef(val));
  case IsTypeOp::ArrLike:
    if (RuntimeOption::EvalIsCompatibleClsMethType &&
        isClsMethType(val->m_type)) {
      if (RO::EvalIsVecNotices) {
        raise_notice(Strings::CLSMETH_COMPAT_IS_ANY_ARR);
      }
      return true;
    }
    return isArrayLikeType(val->m_type);
  case IsTypeOp::ClsMeth: return is_clsmeth(val);
  case IsTypeOp::Func: return is_fun(val);
  }
  not_reached();
}

OPTBLD_INLINE void iopIsTypeL(named_local_var loc, IsTypeOp op) {
  if (loc.ptr->m_type == KindOfUninit) {
    raise_undefined_local(vmfp(), loc.name);
  }
  vmStack().pushBool(isTypeHelper(loc.ptr, op));
}

OPTBLD_INLINE void iopIsTypeC(IsTypeOp op) {
  auto val = vmStack().topC();
  vmStack().replaceC(make_tv<KindOfBoolean>(isTypeHelper(val, op)));
}

OPTBLD_INLINE void iopAssertRATL(local_var loc, RepoAuthType rat) {
  if (debug) {
    auto const tv = *loc.ptr;
    auto const func = vmfp()->func();
    auto vm = &*g_context;
    always_assert_flog(
      tvMatchesRepoAuthType(tv, rat),
      "failed assert RATL on local slot {}: maybe ${} in {}:{}, expected {},"
      " got {}",
      loc.index,
      loc.index < func->numNamedLocals() && func->localNames()[loc.index]
      ? func->localNames()[loc.index]->data()
      : "<unnamed/unknown>",
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

OPTBLD_INLINE void iopAKExists() {
  TypedValue* arr = vmStack().topTV();
  TypedValue* key = arr + 1;
  bool result = HHVM_FN(array_key_exists)(tvAsCVarRef(key), tvAsCVarRef(arr));
  vmStack().popTV();
  vmStack().replaceTV<KindOfBoolean>(result);
}

OPTBLD_INLINE void iopGetMemoKeyL(named_local_var loc) {
  DEBUG_ONLY auto const func = vmfp()->m_func;
  assertx(func->isMemoizeWrapper());

  assertx(tvIsPlausible(*loc.ptr));

  if (UNLIKELY(loc.ptr->m_type == KindOfUninit)) {
    tvWriteNull(*loc.ptr);
    raise_undefined_local(vmfp(), loc.name);
  }
  auto const cell = loc.ptr;

  // Use the generic scheme, which is performed by
  // serialize_memoize_param.
  auto const key = HHVM_FN(serialize_memoize_param)(*cell);
  tvCopy(key, *vmStack().allocC());
}

OPTBLD_INLINE void iopIdx() {
  TypedValue* def = vmStack().topTV();
  TypedValue* key = vmStack().indTV(1);
  TypedValue* arr = vmStack().indTV(2);

  if (isNullType(key->m_type)) {
    tvDecRefGen(arr);
    *arr = *def;
    vmStack().ndiscard(2);
    return;
  }

  TypedValue result;
  if (isArrayLikeType(arr->m_type)) {
    result = HHVM_FN(hphp_array_idx)(tvAsCVarRef(arr),
                                     tvAsCVarRef(key),
                                     tvAsCVarRef(def));
    vmStack().popTV();
  } else if (arr->m_type == KindOfObject) {
    auto obj = arr->m_data.pobj;
    if (obj->isCollection() && collections::contains(obj, tvAsCVarRef(key))) {
      result = collections::at(obj, key).tv();
      tvIncRefGen(result);
      vmStack().popTV();
    } else {
      result = *def;
      vmStack().discard();
    }
  } else if (isStringType(arr->m_type)) {
    // This replicates the behavior of the hack implementation of idx, which
    // first checks isset($arr[$idx]), then returns $arr[(int)$idx]
    auto str = arr->m_data.pstr;
    if (IssetElemString<KeyType::Any>(str, *key)) {
      auto idx = tvCastToInt64(*key);
      assertx(idx >= 0 && idx < str->size());
      result = make_tv<KindOfPersistentString>(str->getChar(idx));
      vmStack().popTV();
    } else {
      result = *def;
      vmStack().discard();
    }
  } else {
    result = *def;
    vmStack().discard();
  }
  vmStack().popTV();
  tvDecRefGen(arr);
  *arr = result;
}

OPTBLD_INLINE void iopArrayIdx() {
  TypedValue* def = vmStack().topTV();
  TypedValue* key = vmStack().indTV(1);
  TypedValue* arr = vmStack().indTV(2);
  if (isClsMethType(type(arr))) {
    if (RuntimeOption::EvalHackArrDVArrs) {
      tvCastToVecInPlace(arr);
    } else {
      tvCastToVArrayInPlace(arr);
    }
  }
  auto const result = HHVM_FN(hphp_array_idx)(tvAsCVarRef(arr),
                                              tvAsCVarRef(key),
                                              tvAsCVarRef(def));
  vmStack().popTV();
  vmStack().popTV();
  tvDecRefGen(arr);
  *arr = result;
}

OPTBLD_INLINE void iopSetL(TypedValue* to) {
  TypedValue* fr = vmStack().topC();
  tvSet(*fr, *to);
}

OPTBLD_INLINE void iopSetG() {
  StringData* name;
  TypedValue* fr = vmStack().topC();
  TypedValue* tv2 = vmStack().indTV(1);
  TypedValue* to = nullptr;
  lookupd_gbl(vmfp(), name, tv2, to);
  SCOPE_EXIT { decRefStr(name); };
  assertx(to != nullptr);
  tvSet(*fr, *to);
  memcpy((void*)tv2, (void*)fr, sizeof(TypedValue));
  vmStack().discard();
}

OPTBLD_INLINE void iopSetS() {
  TypedValue* tv1 = vmStack().topTV();
  TypedValue* clsCell = vmStack().indC(1);
  TypedValue* propn = vmStack().indTV(2);
  TypedValue* output = propn;
  StringData* name;
  TypedValue* val;
  bool visible, accessible, constant;
  Slot slot;

  if (!isClassType(clsCell->m_type)) {
    raise_error("Attempting static property access on non class");
  }
  auto const cls = clsCell->m_data.pclass;

  lookup_sprop(vmfp(), cls, name, propn, val, slot, visible,
               accessible, constant, true);
  SCOPE_EXIT { decRefStr(name); };
  if (!(visible && accessible)) {
    raise_error("Invalid static property access: %s::%s",
                cls->name()->data(),
                name->data());
  }
  if (constant) {
    throw_cannot_modify_static_const_prop(cls->name()->data(), name->data());
  }
  if (RuntimeOption::EvalCheckPropTypeHints > 0) {
    auto const& sprop = cls->staticProperties()[slot];
    auto const& tc = sprop.typeConstraint;
    if (tc.isCheckable()) tc.verifyStaticProperty(tv1, cls, sprop.cls, name);
  }
  tvSet(*tv1, *val);
  tvDecRefGen(propn);
  memcpy(output, tv1, sizeof(TypedValue));
  vmStack().ndiscard(2);
}

OPTBLD_INLINE void iopSetOpL(TypedValue* to, SetOpOp op) {
  TypedValue* fr = vmStack().topC();
  setopBody(to, op, fr);
  tvDecRefGen(fr);
  tvDup(*to, *fr);
}

OPTBLD_INLINE void iopSetOpG(SetOpOp op) {
  StringData* name;
  TypedValue* fr = vmStack().topC();
  TypedValue* tv2 = vmStack().indTV(1);
  TypedValue* to = nullptr;
  // XXX We're probably not getting warnings totally correct here
  lookupd_gbl(vmfp(), name, tv2, to);
  SCOPE_EXIT { decRefStr(name); };
  assertx(to != nullptr);
  setopBody(to, op, fr);
  tvDecRefGen(fr);
  tvDecRefGen(tv2);
  tvDup(*to, *tv2);
  vmStack().discard();
}

OPTBLD_INLINE void iopSetOpS(SetOpOp op) {
  TypedValue* fr = vmStack().topC();
  TypedValue* clsCell = vmStack().indC(1);
  TypedValue* propn = vmStack().indTV(2);
  TypedValue* output = propn;
  StringData* name;
  TypedValue* val;
  bool visible, accessible, constant;
  Slot slot;

  if (!isClassType(clsCell->m_type)) {
    raise_error("Attempting static property access on non class");
  }
  auto const cls = clsCell->m_data.pclass;

  lookup_sprop(vmfp(), cls, name, propn, val, slot, visible,
               accessible, constant, false);
  SCOPE_EXIT { decRefStr(name); };
  if (!(visible && accessible)) {
    raise_error("Invalid static property access: %s::%s",
                cls->name()->data(),
                name->data());
  }
  if (constant) {
    throw_cannot_modify_static_const_prop(cls->name()->data(), name->data());
  }
  auto const& sprop = cls->staticProperties()[slot];
  if (setOpNeedsTypeCheck(sprop.typeConstraint, op, val)) {
    TypedValue temp;
    tvDup(*val, temp);
    SCOPE_FAIL { tvDecRefGen(&temp); };
    setopBody(&temp, op, fr);
    sprop.typeConstraint.verifyStaticProperty(
      &temp, cls, sprop.cls, name
    );
    tvMove(temp, *val);
  } else {
    setopBody(val, op, fr);
  }

  tvDecRefGen(propn);
  tvDecRefGen(fr);
  tvDup(*val, *output);
  vmStack().ndiscard(2);
}

OPTBLD_INLINE void iopIncDecL(named_local_var fr, IncDecOp op) {
  TypedValue* to = vmStack().allocTV();
  tvWriteUninit(*to);
  if (UNLIKELY(fr.ptr->m_type == KindOfUninit)) {
    raise_undefined_local(vmfp(), fr.name);
    tvWriteNull(*fr.ptr);
  }
  tvCopy(IncDecBody(op, fr.ptr), *to);
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
  tvCopy(IncDecBody(op, gbl), *nameCell);
}

OPTBLD_INLINE void iopIncDecS(IncDecOp op) {
  SpropState ss(vmStack(), false);
  if (!(ss.visible && ss.accessible)) {
    raise_error("Invalid static property access: %s::%s",
                ss.cls->name()->data(),
                ss.name->data());
  }
  if (ss.constant) {
    throw_cannot_modify_static_const_prop(ss.cls->name()->data(),
                                          ss.name->data());
  }
  auto const checkable_sprop = [&]() -> const Class::SProp* {
    if (RuntimeOption::EvalCheckPropTypeHints <= 0) return nullptr;
    auto const& sprop = ss.cls->staticProperties()[ss.slot];
    return sprop.typeConstraint.isCheckable() ? &sprop : nullptr;
  }();

  auto const val = ss.val;
  if (checkable_sprop) {
    TypedValue temp;
    tvDup(*val, temp);
    SCOPE_FAIL { tvDecRefGen(&temp); };
    auto result = IncDecBody(op, &temp);
    SCOPE_FAIL { tvDecRefGen(&result); };
    checkable_sprop->typeConstraint.verifyStaticProperty(
      &temp,
      ss.cls,
      checkable_sprop->cls,
      ss.name
    );
    tvMove(temp, *val);
    tvCopy(result, *ss.output);
  } else {
    tvCopy(IncDecBody(op, val), *ss.output);
  }
}

OPTBLD_INLINE void iopUnsetL(TypedValue* loc) {
  tvUnset(*loc);
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

bool doFCall(ActRec* ar, uint32_t numArgs, bool hasUnpack,
             CallFlags callFlags) {
  TRACE(3, "FCall: pc %p func %p base %d\n", vmpc(),
        vmfp()->unit()->entry(),
        int(vmfp()->func()->base()));

  try {
    assertx(!callFlags.hasGenerics() || tvIsVecOrVArray(vmStack().topC()));
    auto generics = callFlags.hasGenerics()
      ? Array::attach(vmStack().topC()->m_data.parr) : Array();
    if (callFlags.hasGenerics()) vmStack().discard();

    auto const func = ar->func();
    if (hasUnpack) {
      checkStack(vmStack(), func, 0);
      auto const newNumArgs = prepareUnpackArgs(func, numArgs, true);
      ar->setNumArgs(newNumArgs);
    } else if (UNLIKELY(numArgs > func->numNonVariadicParams())) {
      if (RuntimeOption::EvalHackArrDVArrs) {
        iopNewVecArray(numArgs - func->numNonVariadicParams());
      } else {
        iopNewVArray(numArgs - func->numNonVariadicParams());
      }
      ar->setNumArgs(func->numNonVariadicParams() + 1);
    }

    prepareFuncEntry(ar, std::move(generics));

    checkForReifiedGenericsErrors(ar, callFlags.hasGenerics());
    calleeDynamicCallChecks(ar->func(), callFlags.isDynamicCall());
    return EventHook::FunctionCall(ar, EventHook::NormalFunc);
  } catch (...) {
    // Manually unwind the pre-live or live frame, as we may be called from JIT
    // and expected to enter JIT unwinder with vmfp() set to the callee.
    assertx(vmfp() == ar || vmfp() == ar->m_sfp);

    auto const func = ar->func();
    auto const numInOutParams = [&] () -> uint32_t {
      if (!func->takesInOutParams()) return 0;
      uint32_t i = 0;
      for (int p = 0; p < numArgs; ++p) i += func->isInOut(p);
      return i;
    }();

    if (ar->m_sfp == vmfp()) {
      // Unwind pre-live frame.
      assertx(vmStack().top() <= (void*)ar);
      while (vmStack().top() != (void*)ar) {
        vmStack().popTV();
      }
      vmStack().popAR();
    } else {
      // Unwind live frame.
      vmfp() = ar->m_sfp;
      vmpc() = vmfp()->func()->getEntry() + ar->callOffset();
      assertx(vmStack().top() + func->numSlotsInFrame() == (void*)ar);
      frame_free_locals_inl_no_hook(ar, func->numLocals());
      vmStack().ndiscard(func->numSlotsInFrame());
      vmStack().discardAR();
    }
    vmStack().ndiscard(numInOutParams);
    throw;
  }
}

namespace {

enum class NoCtx {};

void* takeCtx(Class* cls) { return cls; }
void* takeCtx(Object& obj) = delete;
void* takeCtx(Object&& obj) { return obj.detach(); }
void* takeCtx(NoCtx) {
  if (debug) return reinterpret_cast<void*>(ActRec::kTrashedThisSlot);
  return nullptr;
}

template<bool dynamic, typename Ctx>
void fcallImpl(PC origpc, PC& pc, const FCallArgs& fca, const Func* func,
               Ctx&& ctx, bool logAsDynamicCall = true) {
  if (fca.enforceInOut()) callerInOutChecks(func, fca);
  if (dynamic && logAsDynamicCall) callerDynamicCallChecks(func);
  callerRxChecks(vmfp(), func);
  checkStack(vmStack(), func, 0);

  assertx(kNumActRecCells == 3);
  ActRec* ar = vmStack().indA(fca.numInputs());
  ar->m_func = func;
  ar->setNumArgs(fca.numArgs + (fca.hasUnpack() ? 1 : 0));
  auto const asyncEagerReturn =
    fca.asyncEagerOffset != kInvalidOffset && func->supportsAsyncEagerReturn();
  ar->setReturn(vmfp(), origpc, jit::tc::ustubs().retHelper, asyncEagerReturn);
  ar->trashVarEnv();
  ar->setThisOrClassAllowNull(takeCtx(std::forward<Ctx>(ctx)));

  auto const callFlags = CallFlags(
    fca.hasGenerics(),
    dynamic,
    asyncEagerReturn,
    0, // call offset already set on the ActRec
    0  // generics bitmap not used by interpreter
  );

  doFCall(ar, fca.numArgs, fca.hasUnpack(), callFlags);
  pc = vmpc();
}

template<bool dynamic, typename Ctx>
void fcallImpl(PC origpc, PC& pc, const FCallArgs& fca, const Func* func,
               Ctx&& ctx, String&& invName, bool logAsDynamicCall = true) {
  if (LIKELY(invName.isNull())) {
    fcallImpl<dynamic>(origpc, pc, fca, func, std::forward<Ctx>(ctx),
                       logAsDynamicCall);
    return;
  }

  // Enforce inout-ness before reshuffling.
  if (fca.enforceInOut()) callerInOutChecks(func, fca);

  // Magic methods don't support reified generics.
  assertx(!func->hasReifiedGenerics());
  if (fca.hasGenerics()) vmStack().popC();

  // Magic methods don't support inout args.
  assertx(fca.numRets == 1);

  shuffleMagicArgs(std::move(invName), fca.numArgs, fca.hasUnpack());

  auto const flags = static_cast<FCallArgs::Flags>(
    fca.flags & ~(FCallArgs::Flags::HasUnpack | FCallArgs::Flags::HasGenerics));
  auto const fca2 =
    FCallArgs(flags, 2, 1, nullptr, kInvalidOffset, false, false, fca.context);
  fcallImpl<dynamic>(origpc, pc, fca2, func, std::forward<Ctx>(ctx),
                     logAsDynamicCall);
}

const StaticString s___invoke("__invoke");

// This covers both closures and functors.
OPTBLD_INLINE void fcallFuncObj(PC origpc, PC& pc, const FCallArgs& fca) {
  assertx(tvIsObject(vmStack().topC()));
  auto obj = Object::attach(vmStack().topC()->m_data.pobj);
  vmStack().discard();

  auto const cls = obj->getVMClass();
  auto const func = cls->lookupMethod(s___invoke.get());

  if (func == nullptr) {
    raise_error(Strings::FUNCTION_NAME_MUST_BE_STRING);
  }

  if (func->isStaticInPrologue()) {
    obj.reset();
    fcallImpl<false>(origpc, pc, fca, func, cls);
  } else {
    fcallImpl<false>(origpc, pc, fca, func, std::move(obj));
  }
}

/*
 * Supports callables:
 *   array($instance, 'method')
 *   array('Class', 'method'),
 *   vec[$instance, 'method'],
 *   vec['Class', 'method'],
 *   dict[0 => $instance, 1 => 'method'],
 *   dict[0 => 'Class', 1 => 'method'],
 *   array(Class*, Func*),
 *   array(ObjectData*, Func*),
 */
OPTBLD_INLINE void fcallFuncArr(PC origpc, PC& pc, const FCallArgs& fca) {
  assertx(tvIsArrayLike(vmStack().topC()));
  auto arr = Array::attach(vmStack().topC()->m_data.parr);
  vmStack().discard();

  ObjectData* thiz = nullptr;
  HPHP::Class* cls = nullptr;
  StringData* invName = nullptr;
  bool dynamic = false;

  auto const func = vm_decode_function(const_variant_ref{arr}, vmfp(), thiz,
                                       cls, invName, dynamic,
                                       DecodeFlags::NoWarn);
  assertx(dynamic);
  if (UNLIKELY(func == nullptr)) {
    raise_error("Invalid callable (array)");
  }

  Object thisRC(thiz);
  arr.reset();

  if (thisRC) {
    fcallImpl<true>(origpc, pc, fca, func, std::move(thisRC),
                    String::attach(invName));
  } else if (cls) {
    fcallImpl<true>(origpc, pc, fca, func, cls, String::attach(invName));
  } else {
    fcallImpl<true>(origpc, pc, fca, func, NoCtx{}, String::attach(invName));
  }
}

/*
 * Supports callables:
 *   'func_name'
 *   'class::method'
 */
OPTBLD_INLINE void fcallFuncStr(PC origpc, PC& pc, const FCallArgs& fca) {
  assertx(tvIsString(vmStack().topC()));
  auto str = String::attach(vmStack().topC()->m_data.pstr);
  vmStack().discard();

  ObjectData* thiz = nullptr;
  HPHP::Class* cls = nullptr;
  StringData* invName = nullptr;
  bool dynamic = false;

  auto const func = vm_decode_function(const_variant_ref{str}, vmfp(), thiz,
                                       cls, invName, dynamic,
                                       DecodeFlags::NoWarn);
  assertx(dynamic);
  if (UNLIKELY(func == nullptr)) {
    raise_call_to_undefined(str.get());
  }

  Object thisRC(thiz);
  str.reset();

  if (thisRC) {
    fcallImpl<true>(origpc, pc, fca, func, std::move(thisRC),
                    String::attach(invName));
  } else if (cls) {
    fcallImpl<true>(origpc, pc, fca, func, cls, String::attach(invName));
  } else {
    fcallImpl<true>(origpc, pc, fca, func, NoCtx{}, String::attach(invName));
  }
}

OPTBLD_INLINE void fcallFuncFunc(PC origpc, PC& pc, const FCallArgs& fca) {
  assertx(tvIsFunc(vmStack().topC()));
  auto func = vmStack().topC()->m_data.pfunc;
  vmStack().discard();

  if (func->cls()) {
    raise_error(Strings::CALL_ILLFORMED_FUNC);
  }

  fcallImpl<false>(origpc, pc, fca, func, NoCtx{});
}

OPTBLD_INLINE void fcallFuncClsMeth(PC origpc, PC& pc, const FCallArgs& fca) {
  assertx(tvIsClsMeth(vmStack().topC()));
  auto const clsMeth = vmStack().topC()->m_data.pclsmeth;
  vmStack().discard();

  const Func* func = clsMeth->getFunc();
  auto const cls = clsMeth->getCls();
  assertx(func && cls);

  fcallImpl<false>(origpc, pc, fca, func, cls);
}

} // namespace

OPTBLD_INLINE void iopResolveFunc(Id id) {
  auto unit = vmfp()->m_func->unit();
  auto const nep = unit->lookupNamedEntityPairId(id);
  auto func = Unit::loadFunc(nep.second, nep.first);
  if (func == nullptr) raise_resolve_undefined(unit->lookupLitstrId(id));
  vmStack().pushFunc(func);
}

OPTBLD_INLINE void iopResolveMethCaller(Id id) {
  auto unit = vmfp()->m_func->unit();
  auto const nep = unit->lookupNamedEntityPairId(id);
  auto func = Unit::loadFunc(nep.second, nep.first);
  assertx(func && func->isMethCaller());
  checkMethCaller(func, arGetContextClass(vmfp()));
  vmStack().pushFunc(func);
}

OPTBLD_INLINE void iopFCallFunc(PC origpc, PC& pc, FCallArgs fca) {
  auto const type = vmStack().topC()->m_type;
  if (isObjectType(type)) return fcallFuncObj(origpc, pc, fca);
  if (isArrayLikeType(type)) return fcallFuncArr(origpc, pc, fca);
  if (isStringType(type)) return fcallFuncStr(origpc, pc, fca);
  if (isFuncType(type)) return fcallFuncFunc(origpc, pc, fca);
  if (isClsMethType(type)) return fcallFuncClsMeth(origpc, pc, fca);

  raise_error(Strings::FUNCTION_NAME_MUST_BE_STRING);
}

OPTBLD_INLINE void iopFCallFuncD(PC origpc, PC& pc, FCallArgs fca, Id id) {
  auto const nep = vmfp()->unit()->lookupNamedEntityPairId(id);
  auto const func = Unit::loadFunc(nep.second, nep.first);
  if (UNLIKELY(func == nullptr)) {
    raise_call_to_undefined(vmfp()->unit()->lookupLitstrId(id));
  }

  fcallImpl<false>(origpc, pc, fca, func, NoCtx{});
}

namespace {

template<bool dynamic>
void fcallObjMethodImpl(PC origpc, PC& pc, const FCallArgs& fca,
                        StringData* methName) {
  const Func* func;
  LookupResult res;
  assertx(tvIsObject(vmStack().indC(fca.numInputs() + 2)));
  auto const obj = vmStack().indC(fca.numInputs() + 2)->m_data.pobj;
  auto cls = obj->getVMClass();
  auto const ctx = [&] {
    if (!fca.context) return arGetContextClass(vmfp());
    return Unit::loadClass(fca.context);
  }();
  // if lookup throws, obj will be decref'd via stack
  res = lookupObjMethod(func, cls, methName, ctx, true);
  assertx(func);
  if (res == LookupResult::MethodFoundNoThis) {
    throw_has_this_need_static(func);
  }
  assertx(res == LookupResult::MethodFoundWithThis ||
          res == LookupResult::MagicCallFound);

  auto invName = res == LookupResult::MagicCallFound
    ? String::attach(methName) : String();
  if (res != LookupResult::MagicCallFound) decRefStr(methName);

  if (func->hasReifiedGenerics() && !fca.hasGenerics()) {
    throw_call_reified_func_without_generics(func);
  }

  // fcallImpl() will do further checks before spilling the ActRec. If any
  // of these checks fail, make sure it gets decref'd only via ctx.
  tvWriteNull(*vmStack().indC(fca.numInputs() + 2));
  fcallImpl<dynamic>(origpc, pc, fca, func, Object::attach(obj),
                     std::move(invName));
}

static void raise_resolve_non_object(const char* methodName,
                                     const char* typeName = nullptr) {
  auto const msg = folly::sformat(
    "Cannot resolve a member function {}() on a non-object ({})",
    methodName, typeName
  );

  raise_fatal_error(msg.c_str());
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

ALWAYS_INLINE bool
fcallObjMethodHandleInput(const FCallArgs& fca, ObjMethodOp op,
                          const StringData* methName, bool extraStk) {
  TypedValue* obj = vmStack().indC(fca.numInputs() + 2 + (extraStk ? 1 : 0));
  if (LIKELY(isObjectType(obj->m_type))) return false;

  if (UNLIKELY(op == ObjMethodOp::NullThrows || !isNullType(obj->m_type))) {
    auto const dataTypeStr = getDataTypeString(obj->m_type).get();
    throw_call_non_object(methName->data(), dataTypeStr->data());
  }

  // null?->method(...), pop extra stack input, all arguments and two uninits,
  // the null "object" and all uninits for inout returns, then push null.
  auto& stack = vmStack();
  if (extraStk) stack.popC();
  if (fca.hasGenerics()) stack.popC();
  if (fca.hasUnpack()) stack.popC();
  for (uint32_t i = 0; i < fca.numArgs; ++i) stack.popTV();
  stack.popU();
  stack.popU();
  stack.popC();
  for (uint32_t i = 0; i < fca.numRets - 1; ++i) stack.popU();
  stack.pushNull();

  // Handled.
  return true;
}

} // namespace

OPTBLD_INLINE void
iopFCallObjMethod(PC origpc, PC& pc, FCallArgs fca, const StringData*,
                  ObjMethodOp op) {
  TypedValue* c1 = vmStack().topC(); // Method name.
  if (!isStringType(c1->m_type)) {
    raise_error(Strings::METHOD_NAME_MUST_BE_STRING);
  }

  StringData* methName = c1->m_data.pstr;
  if (fcallObjMethodHandleInput(fca, op, methName, true)) return;

  // We handle decReffing method name in fcallObjMethodImpl
  vmStack().discard();
  fcallObjMethodImpl<true>(origpc, pc, fca, methName);
}

OPTBLD_INLINE void
iopFCallObjMethodD(PC origpc, PC& pc, FCallArgs fca, const StringData*,
                   ObjMethodOp op, const StringData* methName) {
  if (fcallObjMethodHandleInput(fca, op, methName, false)) return;
  auto const methNameC = const_cast<StringData*>(methName);
  fcallObjMethodImpl<false>(origpc, pc, fca, methNameC);
}

namespace {

void resolveMethodImpl(TypedValue* c1, TypedValue* c2) {
  auto name = c1->m_data.pstr;
  ObjectData* thiz = nullptr;
  HPHP::Class* cls = nullptr;
  StringData* invName = nullptr;
  bool dynamic = false;
  auto arr = make_varray(tvAsVariant(*c2), tvAsVariant(*c1));
  auto const func = vm_decode_function(
    Variant{arr},
    vmfp(),
    thiz,
    cls,
    invName,
    dynamic,
    DecodeFlags::NoWarn
  );
  assertx(dynamic);
  if (!func) raise_error("Failure to resolve method name \'%s\'", name->data());
  if (invName) {
    SystemLib::throwInvalidOperationExceptionObject(
      "Unable to resolve magic call for inst_meth()");
  }
  if (thiz) {
    assertx(isObjectType(type(c2)));
    assertx(!(func->attrs() & AttrStatic));
    assertx(val(c2).pobj == thiz);
  } else {
    assertx(cls);
    assertx(func->attrs() & AttrStatic);
    arr.set(0, Variant{cls});
  }
  arr.set(1, Variant{func});
  vmStack().popC();
  vmStack().popC();
  if (RuntimeOption::EvalHackArrDVArrs) {
    vmStack().pushVecNoRc(arr.detach());
  } else {
    vmStack().pushArrayNoRc(arr.detach());
  }
}

} // namespace

OPTBLD_INLINE void iopResolveObjMethod() {
  TypedValue* c1 = vmStack().topC();
  TypedValue* c2 = vmStack().indC(1);
  if (!isStringType(c1->m_type)) {
    raise_error(Strings::METHOD_NAME_MUST_BE_STRING);
  }
  auto name = c1->m_data.pstr;
  if (!isObjectType(c2->m_type)) {
    raise_resolve_non_object(name->data(),
                             getDataTypeString(c2->m_type).get()->data());
  }
  resolveMethodImpl(c1, c2);
}

namespace {

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

template<bool extraStk = false>
void resolveClsMethodImpl(Class* cls, const StringData* methName) {
  const Func* func;
  auto const res = lookupClsMethod(func, cls, methName, nullptr,
                                   arGetContextClass(vmfp()), false);
  if (res == LookupResult::MethodNotFound) {
    raise_error("Failure to resolve method name \'%s::%s\'",
                cls->name()->data(), methName->data());
  }
  assertx(res == LookupResult::MethodFoundNoThis);
  assertx(func);
  if (!func->isStaticInPrologue()) throw_missing_this(func);
  auto clsmeth = ClsMethDataRef::create(cls, const_cast<Func*>(func));
  if (extraStk) vmStack().popC();
  vmStack().pushClsMethNoRc(clsmeth);
}

} // namespace

OPTBLD_INLINE void iopResolveClsMethod(const StringData* methName) {
  auto const c = vmStack().topC();
  if (!isClassType(c->m_type)) {
    raise_error("Attempting ResolveClsMethod with non-class");
  }
  resolveClsMethodImpl<true>(c->m_data.pclass, methName);
}

OPTBLD_INLINE void iopResolveClsMethodD(Id classId,
                                        const StringData* methName) {
  auto const nep = vmfp()->m_func->unit()->lookupNamedEntityPairId(classId);
  auto cls = Unit::loadClass(nep.second, nep.first);
  if (UNLIKELY(cls == nullptr)) {
    raise_error("Failure to resolve class name \'%s\'", nep.first->data());
  }
  resolveClsMethodImpl(cls, methName);
}

OPTBLD_INLINE void iopResolveClsMethodS(SpecialClsRef ref,
                                        const StringData* methName) {
  resolveClsMethodImpl(specialClsRefToCls(ref), methName);
}

namespace {

template<bool dynamic>
void fcallClsMethodImpl(PC origpc, PC& pc, const FCallArgs& fca, Class* cls,
                        StringData* methName, bool forwarding,
                        bool logAsDynamicCall = true) {
  auto const ctx = [&] {
    if (!fca.context) return liveClass();
    return Unit::loadClass(fca.context);
  }();
  auto obj = liveClass() && vmfp()->hasThis() ? vmfp()->getThis() : nullptr;
  const Func* func;
  auto const res = lookupClsMethod(func, cls, methName, obj, ctx, true);
  assertx(func);

  if (res == LookupResult::MethodFoundNoThis) {
    if (!func->isStaticInPrologue()) {
      throw_missing_this(func);
    }
    obj = nullptr;
  } else {
    assertx(obj);
    assertx(res == LookupResult::MethodFoundWithThis ||
            res == LookupResult::MagicCallFound);
  }

  auto invName = res == LookupResult::MagicCallFound
    ? String::attach(methName) : String();
  if (res != LookupResult::MagicCallFound) decRefStr(methName);

  if (func->hasReifiedGenerics() && !fca.hasGenerics()) {
    throw_call_reified_func_without_generics(func);
  }

  if (obj) {
    fcallImpl<dynamic>(origpc, pc, fca, func, Object(obj), std::move(invName),
                       logAsDynamicCall);
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
    fcallImpl<dynamic>(origpc, pc, fca, func, cls, std::move(invName),
                       logAsDynamicCall);
  }
}

} // namespace

OPTBLD_INLINE void
iopFCallClsMethod(PC origpc, PC& pc, FCallArgs fca, const StringData*,
                  IsLogAsDynamicCallOp op) {
  auto const c1 = vmStack().topC();
  if (!isClassType(c1->m_type)) {
    raise_error("Attempting to use non-class in FCallClsMethod");
  }
  auto const cls = c1->m_data.pclass;

  auto const c2 = vmStack().indC(1); // Method name.
  if (!isStringType(c2->m_type)) {
    raise_error(Strings::FUNCTION_NAME_MUST_BE_STRING);
  }
  auto methName = c2->m_data.pstr;

  // fcallClsMethodImpl will take care of decReffing method name
  vmStack().ndiscard(2);
  assertx(cls && methName);
  auto const logAsDynamicCall = op == IsLogAsDynamicCallOp::LogAsDynamicCall ||
    RuntimeOption::EvalLogKnownMethodsAsDynamicCalls;
  fcallClsMethodImpl<true>(origpc, pc, fca, cls, methName, false,
                           logAsDynamicCall);
}


OPTBLD_INLINE void
iopFCallClsMethodD(PC origpc, PC& pc, FCallArgs fca, const StringData*,
                   Id classId, const StringData* methName) {
  const NamedEntityPair &nep =
    vmfp()->m_func->unit()->lookupNamedEntityPairId(classId);
  Class* cls = Unit::loadClass(nep.second, nep.first);
  if (cls == nullptr) {
    raise_error(Strings::UNKNOWN_CLASS, nep.first->data());
  }
  auto const methNameC = const_cast<StringData*>(methName);
  fcallClsMethodImpl<false>(origpc, pc, fca, cls, methNameC, false);
}

OPTBLD_INLINE void
iopFCallClsMethodS(PC origpc, PC& pc, FCallArgs fca, const StringData*,
                   SpecialClsRef ref) {
  auto const c1 = vmStack().topC(); // Method name.
  if (!isStringType(c1->m_type)) {
    raise_error(Strings::FUNCTION_NAME_MUST_BE_STRING);
  }
  auto const cls = specialClsRefToCls(ref);
  auto methName = c1->m_data.pstr;

  // fcallClsMethodImpl will take care of decReffing name
  vmStack().ndiscard(1);
  auto const fwd = ref == SpecialClsRef::Self || ref == SpecialClsRef::Parent;
  fcallClsMethodImpl<true>(origpc, pc, fca, cls, methName, fwd);
}

OPTBLD_INLINE void
iopFCallClsMethodSD(PC origpc, PC& pc, FCallArgs fca, const StringData*,
                    SpecialClsRef ref, const StringData* methName) {
  auto const cls = specialClsRefToCls(ref);
  auto const methNameC = const_cast<StringData*>(methName);
  auto const fwd = ref == SpecialClsRef::Self || ref == SpecialClsRef::Parent;
  fcallClsMethodImpl<false>(origpc, pc, fca, cls, methNameC, fwd);
}

namespace {

ObjectData* newObjImpl(Class* cls, ArrayData* reified_types) {
  // Replace input with uninitialized instance.
  auto this_ = reified_types
    ? ObjectData::newInstanceReified<true>(cls, reified_types)
    : ObjectData::newInstance<true>(cls);
  TRACE(2, "NewObj: just new'ed an instance of class %s: %p\n",
        cls->name()->data(), this_);
  return this_;
}

void newObjDImpl(Id id, ArrayData* reified_types) {
  const NamedEntityPair &nep =
    vmfp()->m_func->unit()->lookupNamedEntityPairId(id);
  auto cls = Unit::loadClass(nep.second, nep.first);
  if (cls == nullptr) {
    raise_error(Strings::UNKNOWN_CLASS,
                vmfp()->m_func->unit()->lookupLitstrId(id)->data());
  }
  auto this_ = newObjImpl(cls, reified_types);
  if (reified_types) vmStack().popC();
  vmStack().pushObjectNoRc(this_);
}

} // namespace

OPTBLD_INLINE void iopNewObj() {
  auto const clsCell = vmStack().topC();
  if (!isClassType(clsCell->m_type)) {
    raise_error("Attempting NewObj with non-class");
  }
  auto const cls = clsCell->m_data.pclass;

  callerDynamicConstructChecks(cls);
  auto this_ = newObjImpl(cls, nullptr);
  vmStack().popC();
  vmStack().pushObjectNoRc(this_);
}

OPTBLD_INLINE void iopNewObjR() {
  auto const reifiedCell = vmStack().topC();
  auto const clsCell = vmStack().indC(1);

  if (!isClassType(clsCell->m_type)) {
    raise_error("Attempting NewObjR with non-class");
  }
  auto const cls = clsCell->m_data.pclass;

  auto const reified = [&] () -> ArrayData* {
    if (reifiedCell->m_type == KindOfNull) return nullptr;
    if (!tvIsVecOrVArray(reifiedCell)) {
      raise_error("Attempting NewObjR with invalid reified generics");
    }
    return reifiedCell->m_data.parr;
  }();

  callerDynamicConstructChecks(cls);
  auto this_ = newObjImpl(cls, reified);
  vmStack().popC();
  vmStack().popC();
  vmStack().pushObjectNoRc(this_);
}

OPTBLD_INLINE void iopNewObjD(Id id) {
  newObjDImpl(id, nullptr);
}

OPTBLD_INLINE void iopNewObjRD(Id id) {
  auto const tsList = vmStack().topC();

  auto const reified = [&] () -> ArrayData* {
    if (tsList->m_type == KindOfNull) return nullptr;
    if (!tvIsVecOrVArray(tsList)) {
      raise_error("Attempting NewObjRD with invalid reified generics");
    }
    return tsList->m_data.parr;
  }();
  newObjDImpl(id, reified);
}

OPTBLD_INLINE void iopNewObjS(SpecialClsRef ref) {
  auto const cls = specialClsRefToCls(ref);
  if (ref == SpecialClsRef::Static && cls->hasReifiedGenerics()) {
    raise_error(Strings::NEW_STATIC_ON_REIFIED_CLASS, cls->name()->data());
  }
  auto const reified_generics = cls->hasReifiedGenerics()
    ? getClsReifiedGenericsProp(cls, vmfp()) : nullptr;
  auto this_ = newObjImpl(cls, reified_generics);
  vmStack().pushObjectNoRc(this_);
}

OPTBLD_INLINE void iopFCallCtor(PC origpc, PC& pc, FCallArgs fca,
                                const StringData*) {
  assertx(fca.numRets == 1);
  assertx(fca.asyncEagerOffset == kInvalidOffset);
  assertx(tvIsObject(vmStack().indC(fca.numInputs() + 2)));
  auto const obj = vmStack().indC(fca.numInputs() + 2)->m_data.pobj;

  const Func* func;
  auto const ctx = arGetContextClass(vmfp());
  auto const res UNUSED = lookupCtorMethod(func, obj->getVMClass(), ctx, true);
  assertx(res == LookupResult::MethodFoundWithThis);

  // fcallImpl() will do further checks before spilling the ActRec. If any
  // of these checks fail, make sure it gets decref'd only via ctx.
  tvWriteNull(*vmStack().indC(fca.numInputs() + 2));
  fcallImpl<false>(origpc, pc, fca, func, Object::attach(obj));
}

OPTBLD_INLINE void iopLockObj() {
  auto c1 = vmStack().topC();
  if (!tvIsObject(*c1)) raise_error("LockObj: expected an object");
  c1->m_data.pobj->lockObject();
}

bool doFCallUnpackTC(PC origpc, int32_t numInputs, CallFlags callFlags,
                     void* retAddr) {
  assert_native_stack_aligned();
  assertx(tl_regState == VMRegState::DIRTY);
  tl_regState = VMRegState::CLEAN;
  auto const ar = vmStack().indA(numInputs);
  if (callFlags.hasGenerics()) --numInputs;
  assertx(ar->numArgs() == numInputs);
  ar->setReturn(vmfp(), origpc, jit::tc::ustubs().retHelper, false);
  ar->setJitReturn(retAddr);
  auto const ret = doFCall(ar, numInputs - 1, true, callFlags);
  tl_regState = VMRegState::DIRTY;
  return ret;
}

OPTBLD_INLINE
void iopFCallBuiltin(
  uint32_t numArgs, uint32_t numNonDefault, uint32_t numOut, Id id
) {
  auto const ne = vmfp()->m_func->unit()->lookupNamedEntityId(id);
  auto const func = ne->uniqueFunc();
  if (func == nullptr || !func->isBuiltin()) {
    raise_error("Call to undefined function %s()",
                vmfp()->m_func->unit()->lookupLitstrId(id)->data());
  }

  if (func->numInOutParams() != numOut) {
    raise_error("Call to function %s() with incorrectly annotated inout "
                "parameter", func->fullName()->data());
  }

  callerRxChecks(vmfp(), func);
  assertx(!func->isMethod() || (func->isStatic() && func->cls()));
  auto const ctx = func->isStatic() ? func->cls() : nullptr;

  TypedValue* args = vmStack().indTV(numArgs-1);
  TypedValue ret;
  Native::coerceFCallArgs(args, numArgs, numNonDefault, func);

  if (func->hasVariadicCaptureParam()) {
    assertx(numArgs > 0);
    assertx(
      RuntimeOption::EvalHackArrDVArrs
        ? isVecType(args[1 - safe_cast<int32_t>(numArgs)].m_type)
        : isArrayType(args[1 - safe_cast<int32_t>(numArgs)].m_type)
    );
  }
  Native::callFunc(func, ctx, args, numNonDefault, ret, true);

  frame_free_args(args, numNonDefault);
  vmStack().ndiscard(numArgs);
  tvCopy(ret, *vmStack().allocTV());
}

namespace {

void implIterInit(PC& pc, const IterArgs& ita, TypedValue* base,
                  PC targetpc, IterTypeOp op) {
  auto const local = base != nullptr;

  if (!local) base = vmStack().topC();
  auto val = frame_local(vmfp(), ita.valId);
  auto key = ita.hasKey() ? frame_local(vmfp(), ita.keyId) : nullptr;
  auto it = frame_iter(vmfp(), ita.iterId);

  if (isArrayLikeType(type(base))) {
    auto const arr = base->m_data.parr;
    auto const res = key
      ? new_iter_array_key_helper(op)(it, arr, val, key)
      : new_iter_array_helper(op)(it, arr, val);
    if (res == 0) pc = targetpc;
    if (!local) vmStack().discard();
    return;
  }

  // NOTE: It looks like we could call new_iter_object at this point. However,
  // doing so is incorrect, since new_iter_array / new_iter_object only handle
  // array-like and object bases, respectively. We may have some other kind of
  // base which the generic Iter::init handles correctly.
  //
  // As a result, the simplest code we could have here is the generic case.
  // It's also about as fast as it can get, because at this point, we're almost
  // always going to create an object iter, which can't really be optimized.
  //

  if (it->init(base)) {
    tvAsVariant(val) = it->val();
    if (key) tvAsVariant(key) = it->key();
  } else {
    pc = targetpc;
  }
  if (!local) vmStack().popC();
}

void implIterNext(PC& pc, const IterArgs& ita, TypedValue* base, PC targetpc) {
  auto val = frame_local(vmfp(), ita.valId);
  auto key = ita.hasKey() ? frame_local(vmfp(), ita.keyId) : nullptr;
  auto it = frame_iter(vmfp(), ita.iterId);

  auto const more = [&]{
    if (base != nullptr && isArrayLikeType(base->m_type)) {
      auto const arr = base->m_data.parr;
      return key ? liter_next_key_ind(it, val, key, arr)
                 : liter_next_ind(it, val, arr);
    }
    return key ? iter_next_key_ind(it, val, key) : iter_next_ind(it, val);
  }();

  if (more) {
    vmpc() = targetpc;
    jmpSurpriseCheck(targetpc - pc);
    pc = targetpc;
  }
}

}

OPTBLD_INLINE void iopIterInit(PC& pc, const IterArgs& ita, PC targetpc) {
  auto const op = IterTypeOp::NonLocal;
  implIterInit(pc, ita, nullptr, targetpc, op);
}

OPTBLD_INLINE void iopLIterInit(PC& pc, const IterArgs& ita,
                                TypedValue* base, PC targetpc) {
  auto const op = ita.flags & IterArgs::Flags::BaseConst
    ? IterTypeOp::LocalBaseConst
    : IterTypeOp::LocalBaseMutable;
  implIterInit(pc, ita, base, targetpc, op);
}

OPTBLD_INLINE void iopIterNext(PC& pc, const IterArgs& ita, PC targetpc) {
  implIterNext(pc, ita, nullptr, targetpc);
}

OPTBLD_INLINE void iopLIterNext(PC& pc, const IterArgs& ita,
                                TypedValue* base, PC targetpc) {
  implIterNext(pc, ita, base, targetpc);
}

OPTBLD_INLINE void iopIterFree(Iter* it) {
  it->free();
}

OPTBLD_INLINE void iopLIterFree(Iter* it, TypedValue*) {
  it->free();
}

OPTBLD_INLINE void inclOp(PC origpc, PC& pc, InclOpFlags flags,
                          const char* opName) {
  TypedValue* c1 = vmStack().topC();
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
      return lookupUnit(absPath.get(), "", &initial,
                        Native::s_noNativeFuncs, false);
    }
    if (flags & InclOpFlags::DocRoot) {
      return lookupUnit(
        SourceRootInfo::RelativeToPhpRoot(path).get(), "", &initial,
        Native::s_noNativeFuncs, false);
    }
    return lookupUnit(path.get(), curUnitFilePath().c_str(), &initial,
                      Native::s_noNativeFuncs, false);
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
    g_context->evalUnit(unit, origpc, pc, EventHook::PseudoMain);
  } else {
    Stats::inc(Stats::PseudoMain_Guarded);
    vmStack().pushBool(true);
  }
}

OPTBLD_INLINE void iopIncl(PC origpc, PC& pc) {
  inclOp(origpc, pc, InclOpFlags::Default, "include");
}

OPTBLD_INLINE void iopInclOnce(PC origpc, PC& pc) {
  inclOp(origpc, pc, InclOpFlags::Once, "include_once");
}

OPTBLD_INLINE void iopReq(PC origpc, PC& pc) {
  inclOp(origpc, pc, InclOpFlags::Fatal, "require");
}

OPTBLD_INLINE void iopReqOnce(PC origpc, PC& pc) {
  inclOp(origpc, pc, InclOpFlags::Fatal | InclOpFlags::Once, "require_once");
}

OPTBLD_INLINE void iopReqDoc(PC origpc, PC& pc) {
  inclOp(
    origpc,
    pc,
    InclOpFlags::Fatal | InclOpFlags::Once | InclOpFlags::DocRoot,
    "require_once"
  );
}

OPTBLD_INLINE void iopEval(PC origpc, PC& pc) {
  TypedValue* c1 = vmStack().topC();

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
  vm->evalUnit(unit, origpc, pc, EventHook::Eval);
}

OPTBLD_INLINE void iopDefCls(uint32_t cid) {
  PreClass* c = vmfp()->m_func->unit()->lookupPreClassId(cid);
  Unit::defClass(c);
}

OPTBLD_INLINE void iopDefRecord(uint32_t cid) {
  auto const r = vmfp()->m_func->unit()->lookupPreRecordId(cid);
  Unit::defRecordDesc(r);
}

OPTBLD_INLINE void iopDefClsNop(uint32_t /*cid*/) {}

OPTBLD_INLINE void iopDefTypeAlias(uint32_t tid) {
  vmfp()->func()->unit()->defTypeAlias(tid);
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

OPTBLD_INLINE void iopInitThisLoc(TypedValue* thisLoc) {
  tvDecRefGen(thisLoc);
  if (vmfp()->func()->cls() && vmfp()->hasThis()) {
    thisLoc->m_data.pobj = vmfp()->getThis();
    thisLoc->m_type = KindOfObject;
    tvIncRefCountable(*thisLoc);
  } else {
    tvWriteUninit(*thisLoc);
  }
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

OPTBLD_INLINE void iopLateBoundCls() {
  auto const cls = frameStaticClass(vmfp());
  if (!cls) raise_error(HPHP::Strings::CANT_ACCESS_STATIC);
  vmStack().pushClass(cls);
}

OPTBLD_INLINE void iopVerifyParamType(local_var param) {
  const Func *func = vmfp()->m_func;
  assertx(param.index < func->numParams());
  assertx(func->numParams() == int(func->params().size()));
  const TypeConstraint& tc = func->params()[param.index].typeConstraint;
  if (tc.isCheckable()) tc.verifyParam(param.ptr, func, param.index);
  if (func->hasParamsWithMultiUBs()) {
    auto const& ubs = func->paramUBs();
    auto it = ubs.find(param.index);
    if (it != ubs.end()) {
      for (auto const& ub : it->second) {
        if (ub.isCheckable()) ub.verifyParam(param.ptr, func, param.index);
      }
    }
  }
}

OPTBLD_INLINE void iopVerifyParamTypeTS(local_var param) {
  iopVerifyParamType(param);
  auto const cell = vmStack().topC();
  assertx(tvIsDictOrDArray(cell));
  auto isTypeVar = tcCouldBeReified(vmfp()->m_func, param.index);
  bool warn = false;
  if ((isTypeVar || tvIsObject(param.ptr)) &&
      !verifyReifiedLocalType(cell->m_data.parr, param.ptr, isTypeVar, warn)) {
    raise_reified_typehint_error(
      folly::sformat(
        "Argument {} passed to {}() must be an instance of {}, {} given",
        param.index + 1,
        vmfp()->m_func->fullName()->data(),
        TypeStructure::toString(ArrNR(cell->m_data.parr),
          TypeStructure::TSDisplayType::TSDisplayTypeUser).c_str(),
        describe_actual_type(param.ptr)
      ), warn
    );
  }
  vmStack().popC();
}

OPTBLD_INLINE void iopVerifyOutType(uint32_t paramId) {
  auto const func = vmfp()->m_func;
  assertx(paramId < func->numParams());
  assertx(func->numParams() == int(func->params().size()));
  auto const& tc = func->params()[paramId].typeConstraint;
  if (tc.isCheckable()) tc.verifyOutParam(vmStack().topTV(), func, paramId);
  if (func->hasParamsWithMultiUBs()) {
    auto const& ubs = func->paramUBs();
    auto it = ubs.find(paramId);
    if (it != ubs.end()) {
      for (auto const& ub : it->second) {
        if (ub.isCheckable()) {
          ub.verifyOutParam(vmStack().topTV(), func, paramId);
        }
      }
    }
  }
}

namespace {

OPTBLD_INLINE void verifyRetTypeImpl(size_t ind) {
  const auto func = vmfp()->m_func;
  const auto tc = func->returnTypeConstraint();
  if (tc.isCheckable()) tc.verifyReturn(vmStack().indC(ind), func);
  if (func->hasReturnWithMultiUBs()) {
    for (auto const& ub : func->returnUBs()) {
      if (ub.isCheckable()) ub.verifyReturn(vmStack().indC(ind), func);
    }
  }
}

} // namespace

OPTBLD_INLINE void iopVerifyRetTypeC() {
  verifyRetTypeImpl(0); // TypedValue is on the top of the stack
}

OPTBLD_INLINE void iopVerifyRetTypeTS() {
  verifyRetTypeImpl(1); // TypedValue is the second element on the stack
  auto const ts = vmStack().topC();
  assertx(tvIsDictOrDArray(ts));
  auto const cell = vmStack().indC(1);
  bool isTypeVar = tcCouldBeReified(vmfp()->m_func, TypeConstraint::ReturnId);
  bool warn = false;
  if ((isTypeVar || tvIsObject(cell)) &&
      !verifyReifiedLocalType(ts->m_data.parr, cell, isTypeVar, warn)) {
    raise_reified_typehint_error(
      folly::sformat(
        "Value returned from function {}() must be of type {}, {} given",
        vmfp()->m_func->fullName()->data(),
        TypeStructure::toString(ArrNR(ts->m_data.parr),
          TypeStructure::TSDisplayType::TSDisplayTypeUser).c_str(),
        describe_actual_type(cell)
      ), warn
    );
  }
  vmStack().popC();
}

OPTBLD_INLINE void iopVerifyRetNonNullC() {
  const auto func = vmfp()->m_func;
  const auto tc = func->returnTypeConstraint();
  tc.verifyReturnNonNull(vmStack().topC(), func);
}

OPTBLD_INLINE TCA iopNativeImpl(PC& pc) {
  auto const jitReturn = jitReturnPre(vmfp());
  auto const func = vmfp()->func();
  auto const native = func->arFuncPtr();
  assertx(native != nullptr);
  // Actually call the native implementation. This will handle freeing the
  // locals in the normal case. In the case of an exception, the VM unwinder
  // will take care of it.
  native(vmfp());

  // Grab caller info from ActRec.
  ActRec* sfp = vmfp()->sfp();
  Offset callOff = vmfp()->callOffset();

  // Adjust the stack; the native implementation put the return value in the
  // right place for us already
  vmStack().ndiscard(func->numSlotsInFrame());
  vmStack().ret();

  // Return control to the caller.
  returnToCaller(pc, sfp, callOff);
  return jitReturnPost(jitReturn);
}

OPTBLD_INLINE void iopSelf() {
  auto const clss = arGetContextClass(vmfp());
  if (!clss) raise_error(HPHP::Strings::CANT_ACCESS_SELF);
  vmStack().pushClass(clss);
}

OPTBLD_INLINE void iopParent() {
  auto const clss = arGetContextClass(vmfp());
  if (!clss) raise_error(HPHP::Strings::CANT_ACCESS_PARENT_WHEN_NO_CLASS);
  auto const parent = clss->parent();
  if (!parent) raise_error(HPHP::Strings::CANT_ACCESS_PARENT_WHEN_NO_PARENT);
  vmStack().pushClass(parent);
}

OPTBLD_INLINE void iopCreateCl(uint32_t numArgs, uint32_t clsIx) {
  auto const func = vmfp()->m_func;
  auto const preCls = func->unit()->lookupPreClassId(clsIx);
  auto const c = Unit::defClosure(preCls);

  auto const cls = c->rescope(const_cast<Class*>(func->cls()));
  assertx(!cls->needInitialization());
  auto obj = RuntimeOption::RepoAuthoritative
    ? createClosureRepoAuth(cls) : createClosure(cls);
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

OPTBLD_INLINE TCA iopCreateCont(PC origpc, PC& pc) {
  auto const jitReturn = jitReturnPre(vmfp());

  auto const fp = vmfp();
  auto const func = fp->func();
  auto const numSlots = func->numSlotsInFrame();
  auto const suspendOffset = func->unit()->offsetOf(origpc);
  assertx(!isResumed(fp));
  assertx(func->isGenerator());

  // Create the {Async,}Generator object. Create takes care of copying local
  // variables and iterators.
  auto const obj = func->isAsync()
    ? AsyncGenerator::Create(fp, numSlots, nullptr, suspendOffset)
    : Generator::Create(fp, numSlots, nullptr, suspendOffset);

  auto const genData = func->isAsync() ?
    static_cast<BaseGenerator*>(AsyncGenerator::fromObject(obj)) :
    static_cast<BaseGenerator*>(Generator::fromObject(obj));

  EventHook::FunctionSuspendCreateCont(fp, genData->actRec());

  // Grab caller info from ActRec.
  ActRec* sfp = fp->sfp();
  Offset callOff = fp->callOffset();

  // Free ActRec and store the return value.
  vmStack().ndiscard(numSlots);
  vmStack().ret();
  tvCopy(make_tv<KindOfObject>(obj), *vmStack().topTV());
  assertx(vmStack().topTV() == fp->retSlot());

  // Return control to the caller.
  returnToCaller(pc, sfp, callOff);

  return jitReturnPost(jitReturn);
}

OPTBLD_INLINE void movePCIntoGenerator(PC origpc, BaseGenerator* gen) {
  assertx(gen->isRunning());
  ActRec* genAR = gen->actRec();
  auto const retHelper = genAR->func()->isAsync()
    ? jit::tc::ustubs().asyncGenRetHelper
    : jit::tc::ustubs().genRetHelper;
  genAR->setReturn(vmfp(), origpc, retHelper, false);

  vmfp() = genAR;
  vmpc() = genAR->func()->unit()->at(gen->resumable()->resumeFromYieldOffset());
}

OPTBLD_INLINE bool tvIsGenerator(TypedValue tv) {
  return tv.m_type == KindOfObject &&
         tv.m_data.pobj->instanceof(Generator::getClass());
}

template<bool recursive>
OPTBLD_INLINE void contEnterImpl(PC origpc) {

  // The stack must have one cell! Or else resumableStackBase() won't work!
  assertx(vmStack().top() + 1 ==
         (TypedValue*)vmfp() - vmfp()->m_func->numSlotsInFrame());

  // Do linkage of the generator's AR.
  assertx(vmfp()->hasThis());
  // `recursive` determines whether we enter just the top generator or whether
  // we drop down to the lowest running delegate generator. This is useful for
  // ContRaise, which should throw from the context of the lowest generator.
  if(!recursive || vmfp()->getThis()->getVMClass() != Generator::getClass()) {
    movePCIntoGenerator(origpc, this_base_generator(vmfp()));
  } else {
    // TODO(https://github.com/facebook/hhvm/issues/6040)
    // Implement throwing from delegate generators.
    assertx(vmfp()->getThis()->getVMClass() == Generator::getClass());
    auto gen = this_generator(vmfp());
    if (gen->m_delegate.m_type != KindOfNull) {
      SystemLib::throwExceptionObject("Throwing from a delegate generator is "
          "not currently supported in HHVM");
    }
    movePCIntoGenerator(origpc, gen);
  }

  EventHook::FunctionResumeYield(vmfp());
}

OPTBLD_INLINE void iopContEnter(PC origpc, PC& pc) {
  contEnterImpl<false>(origpc);
  pc = vmpc();
}

OPTBLD_INLINE void iopContRaise(PC origpc, PC& pc) {
  contEnterImpl<true>(origpc);
  pc = vmpc();
  iopThrow(pc);
}

OPTBLD_INLINE TCA yield(PC origpc, PC& pc, const TypedValue* key, const TypedValue value) {
  auto const jitReturn = jitReturnPre(vmfp());

  auto const fp = vmfp();
  auto const func = fp->func();
  auto const suspendOffset = func->unit()->offsetOf(origpc);
  assertx(isResumed(fp));
  assertx(func->isGenerator());

  EventHook::FunctionSuspendYield(fp);

  auto const sfp = fp->sfp();
  auto const callOff = fp->callOffset();

  if (!func->isAsync()) {
    // Non-async generator.
    assertx(fp->sfp());
    frame_generator(fp)->yield(suspendOffset, key, value);

    // Push return value of next()/send()/raise().
    vmStack().pushNull();
  } else {
    // Async generator.
    auto const gen = frame_async_generator(fp);
    auto const eagerResult = gen->yield(suspendOffset, key, value);
    if (eagerResult) {
      // Eager execution => return StaticWaitHandle.
      assertx(sfp);
      vmStack().pushObjectNoRc(eagerResult);
    } else {
      // Resumed execution => return control to the scheduler.
      assertx(!sfp);
    }
  }

  returnToCaller(pc, sfp, callOff);

  return jitReturnPost(jitReturn);
}

OPTBLD_INLINE TCA iopYield(PC origpc, PC& pc) {
  auto const value = *vmStack().topC();
  vmStack().discard();
  return yield(origpc, pc, nullptr, value);
}

OPTBLD_INLINE TCA iopYieldK(PC origpc, PC& pc) {
  auto const key = *vmStack().indC(1);
  auto const value = *vmStack().topC();
  vmStack().ndiscard(2);
  return yield(origpc, pc, &key, value);
}

OPTBLD_INLINE bool typeIsValidGeneratorDelegate(DataType type) {
  return type == KindOfArray           ||
         type == KindOfPersistentArray ||
         type == KindOfDArray           ||
         type == KindOfPersistentDArray ||
         type == KindOfVArray           ||
         type == KindOfPersistentVArray ||
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
  if (tvIsGenerator(param) || iter->init(&param)) {
    tvSet(param, gen->m_delegate);
  } else {
    tvSetNull(gen->m_delegate);
  }
  // When using a subgenerator we don't actually read the values of the m_key
  // and m_value of our frame generator (the delegating generator). The
  // generator itself is still holding a reference to them though, so null
  // out the key/value to free the memory.
  tvSetNull(gen->m_key);
  tvSetNull(gen->m_value);
}

OPTBLD_INLINE void iopContEnterDelegate(PC origpc, PC& pc) {
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

  movePCIntoGenerator(origpc, delegate);
  EventHook::FunctionResumeYield(vmfp());
  pc = vmpc();
}

OPTBLD_INLINE
TCA yieldFromGenerator(PC& pc, Generator* gen, Offset suspendOffset) {
  auto fp = vmfp();

  assertx(tvIsGenerator(gen->m_delegate));
  auto delegate = Generator::fromObject(gen->m_delegate.m_data.pobj);

  if (delegate->getState() == BaseGenerator::State::Done) {
    // If the generator is done, just copy the return value onto the stack.
    tvDup(delegate->m_value, *vmStack().topTV());
    return nullptr;
  }

  auto jitReturn = jitReturnPre(fp);

  EventHook::FunctionSuspendYield(fp);
  auto const sfp = fp->sfp();
  auto const callOff = fp->callOffset();

  // We don't actually want to "yield" anything here. The implementation of
  // key/current are smart enough to dive into our delegate generator, so
  // really what we want to do is clean up all of the generator metadata
  // (state, ressume address, etc) and continue on.
  assertx(gen->isRunning());
  gen->resumable()->setResumeAddr(nullptr, suspendOffset);
  gen->setState(BaseGenerator::State::Started);

  returnToCaller(pc, sfp, callOff);

  return jitReturnPost(jitReturn);
}

OPTBLD_INLINE
TCA yieldFromIterator(PC& pc, Generator* gen, Iter* it, Offset suspendOffset) {
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

  // If iteration is complete, then this bytecode pushes null on the stack.
  if (it->end()) {
    tvWriteNull(*vmStack().topTV());
    return nullptr;
  }

  auto jitReturn = jitReturnPre(fp);

  EventHook::FunctionSuspendYield(fp);
  auto const sfp = fp->sfp();
  auto const callOff = fp->callOffset();

  auto key = *it->key().asTypedValue();
  auto val = *it->val().asTypedValue();
  gen->yield(suspendOffset, &key, val);

  returnToCaller(pc, sfp, callOff);

  it->next();

  return jitReturnPost(jitReturn);
}

OPTBLD_INLINE TCA iopYieldFromDelegate(PC origpc, PC& pc, Iter* it, PC) {
  auto gen = frame_generator(vmfp());
  auto func = vmfp()->func();
  auto suspendOffset = func->unit()->offsetOf(origpc);
  if (tvIsGenerator(gen->m_delegate)) {
    return yieldFromGenerator(pc, gen, suspendOffset);
  }
  return yieldFromIterator(pc, gen, it, suspendOffset);
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
  tvSetNull(gen->m_delegate);
}

OPTBLD_INLINE void iopContCheck(ContCheckOp subop) {
  this_base_generator(vmfp())->preNext(subop == ContCheckOp::CheckStarted);
}

OPTBLD_INLINE void iopContValid() {
  vmStack().pushBool(
    this_generator(vmfp())->getState() != BaseGenerator::State::Done);
}

OPTBLD_INLINE Generator *currentlyDelegatedGenerator(Generator *gen) {
  while(tvIsGenerator(gen->m_delegate)) {
    gen = Generator::fromObject(gen->m_delegate.m_data.pobj);
  }
  return gen;
}

OPTBLD_INLINE void iopContKey() {
  Generator* cont = this_generator(vmfp());
  cont->startedCheck();

  // If we are currently delegating to a generator, return its key instead
  cont = currentlyDelegatedGenerator(cont);

  tvDup(cont->m_key, *vmStack().allocC());
}

OPTBLD_INLINE void iopContCurrent() {
  Generator* cont = this_generator(vmfp());
  cont->startedCheck();

  // If we are currently delegating to a generator, return its value instead
  cont = currentlyDelegatedGenerator(cont);

  if(cont->getState() == BaseGenerator::State::Done) {
    vmStack().pushNull();
  } else {
    tvDup(cont->m_value, *vmStack().allocC());
  }
}

OPTBLD_INLINE void iopContGetReturn() {
  Generator* cont = this_generator(vmfp());
  cont->startedCheck();

  if(!cont->successfullyFinishedExecuting()) {
    SystemLib::throwExceptionObject("Cannot get return value of a generator "
                                    "that hasn't returned");
  }

  tvDup(cont->m_value, *vmStack().allocC());
}

OPTBLD_INLINE void asyncSuspendE(PC origpc, PC& pc) {
  auto const fp = vmfp();
  auto const func = fp->func();
  auto const suspendOffset = func->unit()->offsetOf(origpc);
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
      fp, func->numSlotsInFrame(), nullptr, suspendOffset, child);

    // Call the suspend hook. It will decref the newly allocated waitHandle
    // if it throws.
    EventHook::FunctionSuspendAwaitEF(fp, waitHandle->actRec());

    // Grab caller info from ActRec.
    ActRec* sfp = fp->sfp();
    Offset callOff = fp->callOffset();

    // Free ActRec and store the return value. In case async eager return was
    // requested by the caller, let it know that we did not finish eagerly.
    vmStack().ndiscard(func->numSlotsInFrame());
    vmStack().ret();
    tvCopy(make_tv<KindOfObject>(waitHandle), *vmStack().topTV());
    vmStack().topTV()->m_aux.u_asyncEagerReturnFlag = 0;
    assertx(vmStack().topTV() == fp->retSlot());

    // Return control to the caller.
    returnToCaller(pc, sfp, callOff);
  } else {  // Async generator.
    // Create new AsyncGeneratorWaitHandle.
    auto waitHandle = c_AsyncGeneratorWaitHandle::Create(
      fp, nullptr, suspendOffset, child);

    // Call the suspend hook. It will decref the newly allocated waitHandle
    // if it throws.
    EventHook::FunctionSuspendAwaitEG(fp);

    // Store the return value.
    vmStack().pushObjectNoRc(waitHandle);

    // Return control to the caller (AG::next()).
    assertx(fp->sfp());
    returnToCaller(pc, fp->sfp(), fp->callOffset());
  }
}

OPTBLD_INLINE void asyncSuspendR(PC origpc, PC& pc) {
  auto const fp = vmfp();
  auto const func = fp->func();
  auto const suspendOffset = func->unit()->offsetOf(origpc);
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
    frame_afwh(fp)->await(suspendOffset, std::move(child));
  } else {  // Async generator.
    auto const gen = frame_async_generator(fp);
    gen->resumable()->setResumeAddr(nullptr, suspendOffset);
    gen->getWaitHandle()->await(std::move(child));
  }

  // Return control to the scheduler.
  pc = nullptr;
  vmfp() = nullptr;
}

namespace {

TCA suspendStack(PC origpc, PC &pc) {
  auto const jitReturn = jitReturnPre(vmfp());
  if (resumeModeFromActRec(vmfp()) == ResumeMode::Async) {
    // suspend resumed execution
    asyncSuspendR(origpc, pc);
  } else {
    // suspend eager execution
    asyncSuspendE(origpc, pc);
  }
  return jitReturnPost(jitReturn);
}

}

OPTBLD_INLINE TCA iopAwait(PC origpc, PC& pc) {
  auto const awaitable = vmStack().topC();
  auto wh = c_Awaitable::fromTV(*awaitable);
  if (UNLIKELY(wh == nullptr)) {
    SystemLib::throwBadMethodCallExceptionObject("Await on a non-Awaitable");
  }
  if (LIKELY(wh->isFailed())) {
    throw req::root<Object>{wh->getException()};
  }
  if (wh->isSucceeded()) {
    tvSet(wh->getResult(), *vmStack().topC());
    return nullptr;
  }
  return suspendStack(origpc, pc);
}

OPTBLD_INLINE TCA iopAwaitAll(PC origpc, PC& pc, LocalRange locals) {
  uint32_t cnt = 0;
  for (auto i = locals.first; i < locals.first + locals.count; ++i) {
    auto const local = *frame_local(vmfp(), i);
    if (tvIsNull(local)) continue;
    auto const awaitable = c_Awaitable::fromTV(local);
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
    locals.count, cnt, frame_local(vmfp(), locals.first)
  ));
  assertx(obj->isWaitHandle());
  assertx(!static_cast<c_Awaitable*>(obj.get())->isFinished());

  vmStack().pushObjectNoRc(obj.detach());
  return suspendStack(origpc, pc);
}

OPTBLD_INLINE void iopWHResult() {
  // we should never emit this bytecode for non-waithandle
  auto const wh = c_Awaitable::fromTV(*vmStack().topC());
  if (UNLIKELY(!wh)) {
    raise_error("WHResult input was not a subclass of Awaitable");
  }

  // the failure condition is likely since we punt to this opcode
  // in the JIT when the state is failed.
  if (wh->isFailed()) {
    throw_object(Object{wh->getException()});
  }
  if (wh->isSucceeded()) {
    tvSet(wh->getResult(), *vmStack().topC());
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
  auto slot = ctx->lookupDeclProp(propName);
  auto index = cls->propSlotToIndex(slot);

  auto const val = (*propVec)[index].val;
  vmStack().pushBool(type(val) != KindOfUninit);
}

OPTBLD_INLINE void iopInitProp(const StringData* propName, InitPropOp propOp) {
  auto* cls = vmfp()->getClass();

  auto* ctx = arGetContextClass(vmfp());
  auto* fr = vmStack().topC();

  auto lval = [&] () -> tv_lval {
    switch (propOp) {
      case InitPropOp::Static: {
        auto const slot = ctx->lookupSProp(propName);
        assertx(slot != kInvalidSlot);
        auto ret = cls->getSPropData(slot);
        if (RuntimeOption::EvalCheckPropTypeHints > 0) {
          auto const& sprop = cls->staticProperties()[slot];
          auto const& tc = sprop.typeConstraint;
          if (tc.isCheckable()) {
            tc.verifyStaticProperty(fr, cls, sprop.cls, sprop.name);
          }
        }
        return ret;
      }

      case InitPropOp::NonStatic: {
        auto* propVec = cls->getPropData();
        always_assert(propVec);
        auto const slot = ctx->lookupDeclProp(propName);
        auto const index = cls->propSlotToIndex(slot);
        assertx(slot != kInvalidSlot);
        auto ret = (*propVec)[index].val;
        if (RuntimeOption::EvalCheckPropTypeHints > 0) {
          auto const& prop = cls->declProperties()[slot];
          auto const& tc = prop.typeConstraint;
          if (tc.isCheckable()) tc.verifyProperty(fr, cls, prop.cls, prop.name);
        }
        return ret;
      }
    }
    always_assert(false);
  }();

  tvDup(*fr, lval);
  vmStack().popC();
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

OPTBLD_INLINE void iopSilence(TypedValue* loc, SilenceOp subop) {
  switch (subop) {
    case SilenceOp::Start:
      loc->m_type = KindOfInt64;
      loc->m_data.num = zero_error_level();
      break;
    case SilenceOp::End:
      assertx(loc->m_type == KindOfInt64);
      restore_error_level(loc->m_data.num);
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
  if (unit == SystemLib::s_hhas_unit) {
    return;
  }

  if (!RO::RepoAuthoritative && RO::EvalEnablePerFileCoverage) {
    if (unit->isCoverageEnabled()) unit->recordCoverage(pcOff());
    return;
  }

  int line = unit->getLineNumber(pcOff());
  assertx(line != -1);

  if (unit != s_prev_unit || line != s_prev_line) {
    s_prev_unit = unit;
    s_prev_line = line;
    const StringData* filepath = unit->filepath();
    assertx(filepath->isStatic());
    RI().m_coverage.Record(filepath->data(), line, line);
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
 * Some opcodes need the original PC by value, and some do not. We have wrappers
 * for both flavors.
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

template<typename... Params, typename... Args>
OPTBLD_INLINE TCA iopWrapReturn(void(fn)(PC, Params...), PC origpc,
                                Args&&... args) {
  fn(origpc, std::forward<Args>(args)...);
  return nullptr;
}

template<typename... Params, typename... Args>
OPTBLD_INLINE TCA iopWrapReturn(TCA(fn)(PC, Params...), PC origpc,
                                Args&&... args) {
  return fn(origpc, std::forward<Args>(args)...);
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
#define FLAG_CF_TF FLAG_CF

#define DECODE_IVA decode_iva(pc)
#define DECODE_I64A decode<int64_t>(pc)
#define DECODE_LA decode_local(pc)
#define DECODE_NLA decode_named_local_var(pc)
#define DECODE_ILA decode_indexed_local(pc)
#define DECODE_IA decode_iter(pc)
#define DECODE_DA decode<double>(pc)
#define DECODE_SA decode<litstr_id>(pc)
#define DECODE_AA decode_litarr(pc)
#define DECODE_RATA decode_rat(pc)
#define DECODE_BA origpc + decode_ba(pc)
#define DECODE_OA(ty) decode<ty>(pc)
#define DECODE_KA decode_member_key(pc, liveUnit())
#define DECODE_LAR decodeLocalRange(pc)
#define DECODE_ITA decodeIterArgs(pc)
#define DECODE_FCA decodeFCallArgs(op, pc, liveUnit())
#define DECODE_BLA decode_imm_array<Offset>(pc)
#define DECODE_SLA decode_imm_array<StrVecItem>(pc)
#define DECODE_VSA decode_imm_array<Id>(pc)

#define DECODE_NA
#define DECODE_ONE(a) auto const imm1 = DECODE_##a;
#define DECODE_TWO(a, b) DECODE_ONE(a) auto const imm2 = DECODE_##b;
#define DECODE_THREE(a, b, c) DECODE_TWO(a, b) auto const imm3 = DECODE_##c;
#define DECODE_FOUR(a, b, c, d) \
  DECODE_THREE(a, b, c) auto const imm4 = DECODE_##d;
#define DECODE_FIVE(a, b, c, d, e) \
  DECODE_FOUR(a, b, c, d) auto const imm5 = DECODE_##e;
#define DECODE_SIX(a, b, c, d, e, f) \
  DECODE_FIVE(a, b, c, d, e) auto const imm6 = DECODE_##f;

#define PASS_NA
#define PASS_ONE(...) , imm1
#define PASS_TWO(...) , imm1, imm2
#define PASS_THREE(...) , imm1, imm2, imm3
#define PASS_FOUR(...) , imm1, imm2, imm3, imm4
#define PASS_FIVE(...) , imm1, imm2, imm3, imm4, imm5
#define PASS_SIX(...) , imm1, imm2, imm3, imm4, imm5, imm6

#define O(name, imm, in, out, flags)                                 \
  OPTBLD_INLINE TCA iopWrap##name(PC& pc) {                          \
    UNUSED auto const op = Op::name;                                 \
    UNUSED auto const origpc = pc - encoded_op_size(op);             \
    DECODE_##imm                                                     \
    return iopWrapReturn(iop##name, origpc FLAG_##flags PASS_##imm); \
  }
OPCODES

#undef FLAG_NF
#undef FLAG_TF
#undef FLAG_CF
#undef FLAG_CF_TF

#undef DECODE_IVA
#undef DECODE_I64A
#undef DECODE_LA
#undef DECODE_NLA
#undef DECODE_ILA
#undef DECODE_IA
#undef DECODE_DA
#undef DECODE_SA
#undef DECODE_AA
#undef DECODE_RATA
#undef DECODE_BA
#undef DECODE_OA
#undef DECODE_KA
#undef DECODE_LAR
#undef DECODE_FCA
#undef DECODE_BLA
#undef DECODE_SLA
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

// fast path to look up native pc; try entry point first.
PcPair lookup_cti(const Func* func, PC pc) {
  auto unitpc = func->unit()->entry();
  auto cti_entry = func->ctiEntry();
  if (!cti_entry) {
    cti_entry = compile_cti(const_cast<Func*>(func), unitpc);
  }
  if (pc == unitpc + func->base()) {
    return {cti_code().base() + cti_entry, pc};
  }
  return {lookup_cti(func, cti_entry, unitpc, pc), pc};
}

template <bool breakOnCtlFlow>
TCA dispatchThreaded(bool coverage) {
  auto modes = breakOnCtlFlow ? ExecMode::BB : ExecMode::Normal;
  if (coverage) {
    modes = modes | ExecMode::Coverage;
  }
  DEBUGGER_ATTACHED_ONLY(modes = modes | ExecMode::Debugger);
  auto target = lookup_cti(vmfp()->func(), vmpc());
  CALLEE_SAVED_BARRIER();
  auto retAddr = g_enterCti(modes, target, rds::header());
  CALLEE_SAVED_BARRIER();
  return retAddr;
}

template <bool breakOnCtlFlow>
TCA dispatchImpl() {
  auto const checkCoverage = [&] {
    return !RO::EvalEnablePerFileCoverage
      ? RID().getCoverage()
      : vmfp() && vmfp()->unit()->isCoverageEnabled();
  };
  bool collectCoverage = checkCoverage();
  if (cti_enabled()) {
    return dispatchThreaded<breakOnCtlFlow>(collectCoverage);
  }

  // Unfortunately, MSVC doesn't support computed
  // gotos, so use a switch instead.
#ifndef _MSC_VER
  static const void* const optabDirect[] = {
#define O(name, imm, push, pop, flags) \
    &&Label##name,
    OPCODES
#undef O
  };
  static const void* const optabDbg[] = {
#define O(name, imm, push, pop, flags) \
    &&LabelDbg##name,
    OPCODES
#undef O
  };
  static const void* const optabCover[] = {
#define O(name, imm, push, pop, flags) \
    &&LabelCover##name,
    OPCODES
#undef O
  };
  assertx(sizeof(optabDirect) / sizeof(const void *) == Op_count);
  assertx(sizeof(optabDbg) / sizeof(const void *) == Op_count);
  const void* const* optab = optabDirect;
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
    if (isFCallFunc(Op::name) ||                              \
        Op::name == Op::NativeImpl ||                         \
        Op::name == Op::FCallBuiltin) {                       \
      collectCoverage = checkCoverage();                      \
      optab = !collectCoverage ? optabDirect : optabCover;    \
      DEBUGGER_ATTACHED_ONLY(optab = optabDbg);               \
    }                                                         \
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
      assertx(retAddr == jit::tc::ustubs().callToExit);       \
      return breakOnCtlFlow ? retAddr : nullptr;              \
    }                                                         \
    assertx(isCtlFlow || !retAddr);                           \
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

  tracing::BlockNoTrace _{"dispatch"};

  DEBUG_ONLY auto const retAddr = dispatchImpl<false>();
  assertx(retAddr == nullptr);
}

TCA dispatchBB() {
  auto sk = [] {
    return SrcKey(vmfp()->func(), vmpc(), resumeModeFromActRec(vmfp()));
  };

  if (Trace::moduleEnabled(Trace::dispatchBB)) {
    static auto cat = makeStaticString("dispatchBB");
    auto name = makeStaticString(show(sk()));
    Stats::incStatGrouped(cat, name, 1);
  }
  if (Trace::moduleEnabled(Trace::ringbuffer)) {
    Trace::ringbufferEntry(Trace::RBTypeDispatchBB, sk().toAtomicInt(), 0);
  }
  return dispatchImpl<true>();
}

///////////////////////////////////////////////////////////////////////////////
// Call-threaded entry points

namespace {

constexpr auto do_prof = false;

static BoolProfiler PredictProf("predict"), LookupProf("lookup");

constexpr unsigned NumPredictors = 16; // real cpus have 8-24
static __thread unsigned s_predict{0};
static __thread PcPair s_predictors[NumPredictors];
static void pushPrediction(PcPair p) {
  s_predictors[s_predict++ % NumPredictors] = p;
}
static PcPair popPrediction() {
  return s_predictors[--s_predict % NumPredictors];
}

// callsites quick reference:
//
// simple opcodes, including throw
//       call  #addr
// conditional branch
//       lea   [pc + instrLen(pc)], nextpc_saved
//       call  #addr
//       cmp   rdx, nextpc_saved
//       jne   native-target
// unconditional branch
//       call  #addr
//       jmp   native-target
// indirect branch
//       call  #addr
//       jmp   rax
// calls w/ return prediction
//       lea   [pc + instrLen(pc)], nextpc_arg
//       call  #addr
//       jmp   rax

NEVER_INLINE void execModeHelper(PC pc, ExecMode modes) {
  if (modes & ExecMode::Debugger) phpDebuggerOpcodeHook(pc);
  if (modes & ExecMode::Coverage) recordCodeCoverage(pc);
  if (modes & ExecMode::BB) {
    //Stats::inc(Stats::Instr_InterpBB##name);
  }
}

template<Op opcode, bool repo_auth, class Iop>
PcPair run(TCA* returnaddr, ExecMode modes, rds::Header* tl, PC nextpc, PC pc,
           Iop iop) {
  assert(vmpc() == pc);
  assert(peek_op(pc) == opcode);
  FTRACE(1, "dispatch: {}: {}\n", pcOff(),
         instrToString(pc, vmfp()->m_func->unit()));
  if (!repo_auth) {
    if (UNLIKELY(modes != ExecMode::Normal)) {
      execModeHelper(pc, modes);
    }
  }
  DEBUG_ONLY auto origPc = pc;
  pc += encoded_op_size(opcode); // skip the opcode
  auto retAddr = iop(pc);
  vmpc() = pc;
  assert(!isThrow(opcode));
  if (isSimple(opcode)) {
    // caller ignores rax return value, invokes next bytecode
    return {nullptr, pc};
  }
  if (isBranch(opcode) || isUnconditionalJmp(opcode)) {
    // callsites have no ability to indirect-jump out of bytecode.
    // so smash the return address to &g_exitCti
    // if we need to exit because of dispatchBB() mode.
    // TODO: t6019406 use surprise checks to eliminate BB mode
    if (modes & ExecMode::BB) {
      *returnaddr = g_exitCti;
      return {nullptr, (PC)retAddr};  // exit stub will return retAddr
    }
    return {nullptr, pc};
  }
  // call & indirect branch: caller will jump to address returned in rax
  if (instrCanHalt(opcode) && !pc) {
    vmfp() = nullptr;
    // We returned from the top VM frame in this nesting level. This means
    // m_savedRip in our ActRec must have been callToExit, which should've
    // been returned by jitReturnPost(), whether or not we were called from
    // the TC. We only actually return callToExit to our caller if that
    // caller is dispatchBB().
    assert(retAddr == jit::tc::ustubs().callToExit);
    if (!(modes & ExecMode::BB)) retAddr = nullptr;
    return {g_exitCti, (PC)retAddr};
  }
  if (instrIsControlFlow(opcode) && (modes & ExecMode::BB)) {
    return {g_exitCti, (PC)retAddr};
  }
  if (isReturnish(opcode)) {
    auto target = popPrediction();
    if (do_prof) PredictProf(pc == target.pc);
    if (pc == target.pc) return target;
  }
  if (isFCall(opcode)) {
    // call-like opcodes predict return to next bytecode
    assert(nextpc == origPc + instrLen(origPc));
    pushPrediction({*returnaddr + kCtiIndirectJmpSize, nextpc});
  }
  if (do_prof) LookupProf(pc == vmfp()->m_func->getEntry());
  // return ip to jump to, caller will do jmp(rax)
  return lookup_cti(vmfp()->m_func, pc);
}
}

// register assignments inbetween calls to cti opcodes
// rax = target of indirect branch instr (call, switch, etc)
// rdx = pc (passed as 3rd arg register, 2nd return register)
// rbx = next-pc after branch instruction, only if isBranch(op)
// r12 = rds::Header* (vmtl)
// r13 = modes
// r14 = location of return address to cti caller on native stack

#ifdef __clang__
#define DECLARE_FIXED(TL,MODES,RA)\
  rds::Header* TL; asm volatile("mov %%r12, %0" : "=r"(TL) ::);\
  ExecMode MODES;  asm volatile("mov %%r13d, %0" : "=r"(MODES) ::);\
  TCA* RA;         asm volatile("mov %%r14, %0" : "=r"(RA) ::);
#else
#define DECLARE_FIXED(TL,MODES,RA)\
  register rds::Header* TL asm("r12");\
  register ExecMode MODES  asm("r13");\
  register TCA* RA         asm("r14");
#endif

namespace cti {
// generate cti::op call-threaded function for each opcode
#define O(opcode, imm, push, pop, flags)\
PcPair opcode(PC nextpc, TCA*, PC pc) {\
  DECLARE_FIXED(tl, modes, returnaddr);\
  return run<Op::opcode,true>(returnaddr, modes, tl, nextpc, pc,\
      [](PC& pc) {\
    return iopWrap##opcode(pc);\
  });\
}
OPCODES
#undef O

// generate debug/coverage-capable opcode bodies (for non-repo-auth)
#define O(opcode, imm, push, pop, flags)\
PcPair d##opcode(PC nextpc, TCA*, PC pc) {\
  DECLARE_FIXED(tl, modes, returnaddr);\
  return run<Op::opcode,false>(returnaddr, modes, tl, nextpc, pc,\
      [](PC& pc) {\
    return iopWrap##opcode(pc);\
  });\
}
OPCODES
#undef O
}

// generate table of opcode handler addresses, used by call-threaded emitter
const CodeAddress cti_ops[] = {
  #define O(opcode, imm, push, pop, flags) (CodeAddress)&cti::opcode,
  OPCODES
  #undef O
};
const CodeAddress ctid_ops[] = {
  #define O(opcode, imm, push, pop, flags) (CodeAddress)&cti::d##opcode,
  OPCODES
  #undef O
};

}
