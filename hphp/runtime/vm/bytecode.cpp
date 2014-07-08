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
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/repo-global-data.h"
#include "hphp/runtime/base/repo-auth-type-codec.h"
#include "hphp/runtime/vm/func-inline.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/translator-runtime.h"
#include "hphp/runtime/vm/srckey.h"
#include "hphp/runtime/vm/member-operations.h"
#include "hphp/runtime/base/class-info.h"
#include "hphp/runtime/base/code-coverage.h"
#include "hphp/runtime/base/unit-cache.h"
#include "hphp/runtime/base/base-includes.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/mixed-array.h"
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
#include "hphp/runtime/vm/resumable.h"
#include "hphp/runtime/ext/ext_math.h"
#include "hphp/runtime/ext/ext_string.h"
#include "hphp/runtime/ext/ext_closure.h"
#include "hphp/runtime/ext/ext_generator.h"
#include "hphp/runtime/ext/ext_function.h"
#include "hphp/runtime/ext/std/ext_std_variable.h"
#include "hphp/runtime/ext/ext_array.h"
#include "hphp/runtime/ext/ext_apc.h"
#include "hphp/runtime/ext/asio/async_function_wait_handle.h"
#include "hphp/runtime/ext/asio/async_generator.h"
#include "hphp/runtime/ext/asio/async_generator_wait_handle.h"
#include "hphp/runtime/ext/asio/static_wait_handle.h"
#include "hphp/runtime/ext/asio/wait_handle.h"
#include "hphp/runtime/ext/asio/waitable_wait_handle.h"
#include "hphp/runtime/ext/reflection/ext_reflection.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/vm/type-profile.h"
#include "hphp/runtime/server/source-root-info.h"
#include "hphp/runtime/base/extended-logger.h"
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

using JIT::mcg;

#if DEBUG
#define OPTBLD_INLINE
#else
#define OPTBLD_INLINE ALWAYS_INLINE
#endif
TRACE_SET_MOD(bcinterp);

// Identifies the set of return helpers that we may set m_savedRip to in an
// ActRec.
static bool isReturnHelper(void* address) {
  auto tcAddr = reinterpret_cast<JIT::TCA>(address);
  auto& u = mcg->tx().uniqueStubs;
  return tcAddr == u.retHelper ||
         tcAddr == u.genRetHelper ||
         tcAddr == u.retInlHelper ||
         tcAddr == u.callToExit;
}

ActRec* ActRec::sfp() const {
  // Native frame? (used by enterTCHelper)
  if (UNLIKELY(((uintptr_t)m_sfp - s_stackLimit) < s_stackSize)) {
    return nullptr;
  }

  return m_sfp;
}

void ActRec::setReturn(ActRec* fp, PC pc, void* retAddr) {
  assert(fp->func()->contains(pc));
  assert(isReturnHelper(retAddr));
  m_sfp = fp;
  m_savedRip = reinterpret_cast<uintptr_t>(retAddr);
  m_soff = Offset(pc - fp->func()->getEntry());
}

void ActRec::setReturnVMExit() {
  assert(isReturnHelper(mcg->tx().uniqueStubs.callToExit));
  m_sfp = nullptr;
  m_savedRip = reinterpret_cast<uintptr_t>(mcg->tx().uniqueStubs.callToExit);
  m_soff = 0;
}

bool
ActRec::skipFrame() const {
  return m_func && m_func->isSkipFrame();
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
    var = vmfp()->m_func->unit()->lookupLitstrId(id);       \
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

#define SYNC() vmpc() = pc

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

static Offset pcOff() {
  return vmfp()->m_func->unit()->offsetOf(vmpc());
}

//=============================================================================
// VarEnv.

VarEnv::VarEnv()
  : m_nvTable()
  , m_extraArgs(nullptr)
  , m_depth(0)
  , m_global(true)
{
  TRACE(3, "Creating VarEnv %p [global scope]\n", this);
  assert(!g_context->m_globalVarEnv);
  g_context->m_globalVarEnv = this;

  auto tableWrapper = smart_new<GlobalNameValueTableWrapper>(&m_nvTable);
  auto globalArray = make_tv<KindOfArray>(tableWrapper->asArrayData());
  m_nvTable.set(makeStaticString("GLOBALS"), &globalArray);
}

VarEnv::VarEnv(ActRec* fp, ExtraArgs* eArgs)
  : m_nvTable(fp)
  , m_extraArgs(eArgs)
  , m_depth(1)
  , m_global(false)
{
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
     * let the smart allocator clean them up).  This is because we're
     * not supposed to run destructors for objects that are live at
     * the end of a request.
     */
    m_nvTable.leak();
  }
}

VarEnv* VarEnv::createGlobal() {
  return smart_new<VarEnv>();
}

VarEnv* VarEnv::createLocal(ActRec* fp) {
  return smart_new<VarEnv>(fp, fp->getExtraArgs());
}

VarEnv* VarEnv::clone(ActRec* fp) const {
  return smart_new<VarEnv>(this, fp);
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
    // don't free global VarEnv
    if (!isGlobalScope()) {
      smart_delete(this);
    }
  } else {
    m_nvTable.attach(g_context->getPrevVMState(fp));
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

Array VarEnv::getDefinedVariables() const {
  Array ret = Array::Create();

  NameValueTable::Iterator iter(&m_nvTable);
  for (; iter.valid(); iter.next()) {
    auto const sd = iter.curKey();
    auto const tv = iter.curVal();
    if (tvAsCVarRef(tv).isReferenced()) {
      ret.setWithRef(StrNR(sd).asString(), tvAsCVarRef(tv));
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
  assert(nargs > 0);
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
  assert(!fp->resumed());
  return (TypedValue*)fp - fp->func()->numSlotsInFrame();
}

TypedValue* Stack::resumableStackBase(const ActRec* fp) {
  assert(fp->resumed());
  auto const sfp = fp->sfp();
  if (sfp) {
    // The non-reentrant case occurs when a non-async or async generator is
    // resumed via ContEnter or ContRaise opcode. These opcodes leave a single
    // value on the stack that becomes part of the generator's stack. So we
    // find the caller's FP, compensate for its locals and iterators, and then
    // we've found the base of the generator's stack.
    assert(fp->func()->isGenerator());
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


//=============================================================================
// ExecutionContext.

using namespace HPHP;

ActRec* ExecutionContext::getOuterVMFrame(const ActRec* ar) {
  ActRec* sfp = ar->sfp();
  if (LIKELY(sfp != nullptr)) return sfp;
  return LIKELY(!m_nestedVMs.empty()) ? m_nestedVMs.back().fp : nullptr;
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
    Class* ctx = arGetContextClass(vmfp());
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
  auto const class_ = Unit::loadClass(clsName);
  if (class_ == nullptr) {
    raise_error("unknown class %s", clsName->data());
  }
  return createObject(class_, params, init);
}

ObjectData* ExecutionContext::createObject(const Class* class_,
                                           const Variant& params,
                                           bool init) {
  Object o;
  o = newInstance(const_cast<Class*>(class_));
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
  return vmfp();
}

ObjectData* ExecutionContext::getThis() {
  VMRegAnchor _;
  ActRec* fp = vmfp();
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
  ActRec* ar = vmfp();
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

StringData* ExecutionContext::getContainingFileName() {
  VMRegAnchor _;
  ActRec* ar = vmfp();
  if (ar == nullptr) return staticEmptyString();
  if (ar->skipFrame()) {
    ar = getPrevVMState(ar);
    if (ar == nullptr) return staticEmptyString();
  }
  Unit* unit = ar->m_func->unit();
  assert(unit->filepath()->isStatic());
  // XXX: const StringData* -> Variant(bool) conversion problem makes this ugly
  return const_cast<StringData*>(unit->filepath());
}

int ExecutionContext::getLine() {
  VMRegAnchor _;
  ActRec* ar = vmfp();
  Unit* unit = ar ? ar->m_func->unit() : nullptr;
  Offset pc = unit ? pcOff() : 0;
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
  ActRec* ar = vmfp();
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

  ActRec* fp = vmfp();
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

void ExecutionContext::setVar(StringData* name, const TypedValue* v) {
  VMRegAnchor _;
  ActRec *fp = vmfp();
  if (!fp) return;
  if (fp->skipFrame()) fp = getPrevVMState(fp);
  fp->getVarEnv()->set(name, v);
}

void ExecutionContext::bindVar(StringData* name, TypedValue* v) {
  VMRegAnchor _;
  ActRec *fp = vmfp();
  if (!fp) return;
  if (fp->skipFrame()) fp = getPrevVMState(fp);
  fp->getVarEnv()->bind(name, v);
}

Array ExecutionContext::getLocalDefinedVariables(int frame) {
  VMRegAnchor _;
  ActRec *fp = vmfp();
  for (; frame > 0; --frame) {
    if (!fp) break;
    fp = getPrevVMState(fp);
  }
  if (!fp) {
    return empty_array();
  }
  assert(!fp->hasInvName());
  if (fp->hasVarEnv()) {
    return fp->m_varEnv->getDefinedVariables();
  }
  const Func *func = fp->m_func;
  auto numLocals = func->numNamedLocals();
  ArrayInit ret(numLocals, ArrayInit::Map{});
  for (Id id = 0; id < numLocals; ++id) {
    TypedValue* ptv = frame_local(fp, id);
    if (ptv->m_type == KindOfUninit) {
      continue;
    }
    Variant name(func->localVarName(id), Variant::StaticStrInit{});
    ret.add(name, tvAsVariant(ptv));
  }
  return ret.toArray();
}

NEVER_INLINE
void ExecutionContext::shuffleExtraStackArgs(ActRec* ar) {
  const Func* func = ar->m_func;
  assert(func);
  assert(!ar->m_varEnv);

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
        Array::attach(MixedArray::MakePacked(numVarArgs, tvArgs));
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
      Array::attach(MixedArray::MakePacked(numVarArgs, tvArgs));
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

void ExecutionContext::shuffleMagicArgs(ActRec* ar) {
  // We need to put this where the first argument is
  StringData* invName = ar->getInvName();
  int nargs = ar->numArgs();
  ar->setVarEnv(nullptr);
  assert(!ar->hasVarEnv() && !ar->hasInvName());

  // We need to make an array containing all the arguments passed by
  // the caller and put it where the second argument is.
  auto argArray = Array::attach(
    nargs ? MixedArray::MakePacked(
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
}

// This helper only does a stack overflow check for the native stack
static inline void checkNativeStack() {
  auto const info = ThreadInfo::s_threadInfo.getNoCheck();
  // Check whether func's maximum stack usage would overflow the stack.
  // Both native and VM stack overflows are independently possible.
  if (!stack_in_bounds(info)) {
    TRACE(1, "Maximum stack depth exceeded.\n");
    raise_error("Stack overflow");
  }
}

/*
 * This helper does a stack overflow check on *both* the native stack
 * and the VM stack.
 *
 * In some cases for re-entry, we're checking for space other than
 * just the callee, and `extraCells' may need to be passed with a
 * non-zero value.  (We over-check in these situations, but it's fine.)
 */
ALWAYS_INLINE
static void checkStack(Stack& stk, const Func* f, int32_t extraCells) {
  assert(f);
  auto const info = ThreadInfo::s_threadInfo.getNoCheck();
  /*
   * Check whether func's maximum stack usage would overflow the stack.
   * Both native and VM stack overflows are independently possible.
   *
   * All stack checks are inflated by kStackCheckPadding to ensure
   * there is space both for calling leaf functions /and/ for
   * re-entry.  (See kStackCheckReenterPadding and
   * kStackCheckLeafPadding.)
   */
  auto limit = f->maxStackCells() + kStackCheckPadding + extraCells;
  if (!stack_in_bounds(info) || stk.wouldOverflow(limit)) {
    TRACE(1, "Maximum stack depth exceeded.\n");
    raise_error("Stack overflow");
  }
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

static NEVER_INLINE void shuffleMagicArrayArgs(ActRec* ar, const Cell&args,
                                               Stack& stack, int nregular) {
  assert(ar != nullptr && ar->hasInvName());
  assert(!cellIsNull(&args));
  assert(nregular >= 0);
  assert((stack.top() + nregular) == (void*) ar);
  assert(isContainer(args));
  DEBUG_ONLY const Func* f = ar->m_func;
  assert(f &&
         (f->name()->isame(s___call.get()) ||
          f->name()->isame(s___callStatic.get())));

  // We'll need to make this the first argument
  StringData* invName = ar->getInvName();
  ar->setVarEnv(nullptr);
  assert(!ar->hasVarEnv() && !ar->hasInvName());

  auto nargs = getContainerSize(args);

  if (UNLIKELY(0 == nargs)) {
    // We need to make an array containing all the arguments passed by
    // the caller and put it where the second argument is.
    auto argArray = Array::attach(
      nregular
      ? MixedArray::MakePacked(
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
        && args.m_type == KindOfArray
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
      stack.pushArray(ai.create());
    }
  }

  ar->setNumArgs(2);
}

// offset is the number of params already on the stack to which the
// contents of args are to be added; for call_user_func_array, this is
// always 0; for unpacked arguments, it may be greater if normally passed
// params precede the unpack.
static bool prepareArrayArgs(ActRec* ar, const Cell& args,
                             Stack& stack,
                             int nregular,
                             bool doCufRefParamChecks,
                             TypedValue* retval) {
  assert(ar != nullptr);
  assert(!cellIsNull(&args));
  assert(nregular >= 0);
  assert((stack.top() + nregular) == (void*) ar);
  const Func* f = ar->m_func;
  assert(f);

  assert(!ar->hasExtraArgs());

  assert(isContainer(args));
  int nargs = nregular + getContainerSize(args);
  assert(!ar->hasVarEnv() || (0 == nargs));
  if (UNLIKELY(ar->hasInvName())) {
    shuffleMagicArrayArgs(ar, args, stack, nregular);
    return true;
  }

  int nparams = f->numNonVariadicParams();
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
                        from->m_data.pref->m_count >= 2)) {
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
            cleanupParamsAndActRec(stack, ar, nullptr, &i);
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
      // prepareFuncEntry
      ar->initNumArgs(nargs);
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
    ar->initNumArgs(f->numParams());
    return true;
  }

  auto const hasVarParam = f->hasVariadicCaptureParam();
  auto const extra = nargs - nparams;
  if (f->attrs() & AttrMayUseVV) {
    ExtraArgs* extraArgs = ExtraArgs::allocateUninit(extra);
    PackedArrayInit ai(extra);
    if (UNLIKELY(nextra_regular > 0)) {
      for (int i = 0; i < nextra_regular; ++i) {
        TypedValue* to = extraArgs->getExtraArg(i);
        const TypedValue* from = stack.top();
        if (from->m_type == KindOfRef && from->m_data.pref->isReferenced()) {
          refDup(*from, *to);
        } else {
          cellDup(*tvToCell(from), *to);
        }
        if (hasVarParam) {
          // appendWithRef bumps the refcount: this accounts for the fact
          // that the extra args values went from being present on the stack
          // to being in (both) ExtraArgs and the variadic args
          ai.appendWithRef(tvAsCVarRef(from));
        }
        stack.discard();
      }
    }
    for (int i = nextra_regular; i < extra; ++i, ++iter) {
      TypedValue* to = extraArgs->getExtraArg(i);
      const TypedValue* from = iter.secondRefPlus().asTypedValue();
      if (from->m_type == KindOfRef && from->m_data.pref->isReferenced()) {
        refDup(*from, *to);
      } else {
        cellDup(*tvToCell(from), *to);
      }
      if (hasVarParam) {
        // appendWithRef bumps the refcount: this accounts for the fact
        // that the extra args values went from being present in
        // arrayArgs to being in (both) ExtraArgs and the variadic args
        ai.appendWithRef(iter.secondRefPlus());
      }
    }
    assert(!iter); // iter should now be exhausted
    if (hasVarParam) {
      auto const ad = ai.create();
      stack.pushArray(ad);
      assert(ad->hasExactlyOneRef());
    }
    ar->initNumArgs(nargs);
    ar->setExtraArgs(extraArgs);
  } else {
    assert(hasVarParam);
    if (nparams == 0
        && nextra_regular == 0
        && args.m_type == KindOfArray
        && args.m_data.parr->isVectorData()) {
      stack.pushArray(args.m_data.parr);
    } else {
      PackedArrayInit ai(extra);
      if (UNLIKELY(nextra_regular > 0)) {
        for (int i = 0; i < nextra_regular; ++i) {
          // appendWithRef bumps the refcount and splits if necessary,
          // to compensate for the upcoming pop from the stack
          ai.appendWithRef(tvAsVariant(stack.top()));
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
      stack.pushArray(ad);
      assert(ad->hasExactlyOneRef());
    }
    ar->initNumArgs(f->numParams());
  }
  return true;
}

void ExecutionContext::prepareFuncEntry(ActRec *ar, PC& pc,
                                        StackArgsState stk) {
  assert(!ar->resumed());
  const Func* func = ar->m_func;
  Offset firstDVInitializer = InvalidAbsoluteOffset;
  bool raiseMissingArgumentWarnings = false;
  const int nparams = func->numNonVariadicParams();
  auto& stack = vmStack();

  if (UNLIKELY(ar->m_varEnv != nullptr)) {
    // m_varEnv != nullptr means we have a varEnv, extraArgs, or an invName.
    if (ar->hasInvName()) {
      // shuffleMagicArgs deals with everything. no need for further
      // argument munging
      shuffleMagicArgs(ar);
    } else if (ar->hasVarEnv()) {
      assert(func->isPseudoMain());
      pushLocalsAndIterators(func);
      ar->m_varEnv->enterFP(vmfp(), ar);
      vmfp() = ar;
      pc = func->getEntry();
      // Nothing more to do; get out
      return;
    } else {
      assert(ar->hasExtraArgs());
      assert(nparams < ar->numArgs());
    }
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
    SYNC();
    const Func::ParamInfoVec& paramInfo = func->params();
    for (int i = ar->numArgs(); i < nparams; ++i) {
      Offset dvInitializer = paramInfo[i].funcletOff;
      if (dvInitializer == InvalidAbsoluteOffset) {
        const char* name = func->name()->data();
        if (nparams == 1) {
          raise_warning(
            Strings::MISSING_ARGUMENT, name,
            func->hasVariadicCaptureParam() ? "at least" : "exactly", i);
        } else {
          raise_warning(
            Strings::MISSING_ARGUMENTS, name,
            func->hasVariadicCaptureParam() ? "at least" : "exactly",
            nparams, i);
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

void ExecutionContext::enterVMAtAsyncFunc(ActRec* enterFnAr,
                                          Resumable* resumable,
                                          ObjectData* exception) {
  assert(enterFnAr);
  assert(enterFnAr->func()->isAsync());
  assert(enterFnAr->resumed());
  assert(resumable);

  vmfp() = enterFnAr;
  vmpc() = vmfp()->func()->unit()->at(resumable->resumeOffset());
  assert(vmfp()->func()->contains(vmpc()));
  EventHook::FunctionResume(enterFnAr);

  if (UNLIKELY(exception != nullptr)) {
    assert(exception->instanceof(SystemLib::s_ExceptionClass));
    Object e(exception);
    throw e;
  }

  bool useJit = ThreadInfo::s_threadInfo->m_reqInjectionData.getJit();
  if (LIKELY(useJit && resumable->resumeAddr())) {
    Stats::inc(Stats::VMEnter);
    mcg->enterTCAfterPrologue(resumable->resumeAddr());
  } else {
    enterVMAtCurPC();
  }
}

void ExecutionContext::enterVMAtFunc(ActRec* enterFnAr, StackArgsState stk) {
  assert(enterFnAr);
  assert(!enterFnAr->resumed());
  Stats::inc(Stats::VMEnter);

  bool useJit = ThreadInfo::s_threadInfo->m_reqInjectionData.getJit();
  bool useJitPrologue = useJit && vmfp()
    && !enterFnAr->m_varEnv
    && (stk != StackArgsState::Trimmed);
  // The jit prologues only know how to do limited amounts of work; cannot
  // be used for magic call/pseudo-main/extra-args already determined or
  // ... or if the stack args have been explicitly been prepared (e.g. via
  // entry as part of invoke func).

  if (LIKELY(useJitPrologue)) {
    const int np = enterFnAr->m_func->numNonVariadicParams();
    int na = enterFnAr->numArgs();
    if (na > np) na = np + 1;
    JIT::TCA start = enterFnAr->m_func->getPrologue(na);
    mcg->enterTCAtPrologue(enterFnAr, start);
    return;
  }

  prepareFuncEntry(enterFnAr, vmpc(), stk);
  if (!EventHook::FunctionCall(enterFnAr, EventHook::NormalFunc)) return;
  checkStack(vmStack(), enterFnAr->m_func, 0);
  assert(vmfp()->func()->contains(vmpc()));

  if (useJit) {
    JIT::TCA start = enterFnAr->m_func->getFuncBody();
    mcg->enterTCAfterPrologue(start);
  } else {
    dispatch();
  }
}

void ExecutionContext::enterVMAtCurPC() {
  assert(vmfp());
  assert(vmpc());
  assert(vmfp()->func()->contains(vmpc()));
  Stats::inc(Stats::VMEnter);

  if (ThreadInfo::s_threadInfo->m_reqInjectionData.getJit()) {
    SrcKey sk(vmfp()->func(), vmpc(), vmfp()->resumed());
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
void ExecutionContext::enterVM(ActRec* ar, StackArgsState stk,
                               Resumable* resumable, ObjectData* exception) {
  assert(ar);
  assert(!ar->sfp());
  assert(isReturnHelper(reinterpret_cast<void*>(ar->m_savedRip)));
  assert(ar->m_soff == 0);
  assert(!resumable || (stk == StackArgsState::Untrimmed));

  DEBUG_ONLY int faultDepth = m_faults.size();
  SCOPE_EXIT { assert(m_faults.size() == faultDepth); };

  vmFirstAR() = ar;

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
      if (!resumable) {
        enterVMAtFunc(ar, stk);
      } else {
        enterVMAtAsyncFunc(ar, resumable, exception);
      }
    } else {
      enterVMAtCurPC();
    }

    // Everything succeeded with no exception---return to the previous
    // VM nesting level.
    return;

  } catch (...) {
    always_assert(tl_regState == VMRegState::CLEAN);
    switch (exception_handler()) {
      case UnwindAction::Propagate:
        break;
      case UnwindAction::ResumeVM:
        if (vmpc()) { goto resume; }
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

  Cell* originalSP = vmRegsUnsafe().stack.top();

  VMRegAnchor _;
  DEBUG_ONLY Cell* reentrySP = vmStack().top();

  if (this_ != nullptr) {
    this_->incRefCount();
  }

  // We must do a stack overflow check for leaf functions on re-entry,
  // because we won't have checked that the stack is deep enough for a
  // leaf function /after/ re-entry, and the prologue for the leaf
  // function will not make a check.
  if (f->attrs() & AttrPhpLeafFn ||
      !(f->numParams() + kNumActRecCells <= kStackCheckReenterPadding)) {
    // Check both the native stack and VM stack for overflow.
    checkStack(vmStack(), f,
      kNumActRecCells /* numParams is included in f->maxStackCells */);
  } else {
    // invokeFunc() must always check the native stack for overflow no
    // matter what.
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

  ActRec* ar = vmStack().allocA();
  ar->setReturnVMExit();
  ar->m_func = f;
  if (this_) {
    ar->setThis(this_);
  } else if (cls) {
    ar->setClass(cls);
  } else {
    ar->setThis(nullptr);
  }
  auto numPassedArgs = cellIsNull(&args) ? 0 : getContainerSize(args);
  if (invName) {
    ar->setInvName(invName);
  } else {
    ar->setVarEnv(varEnv);
  }
  ar->initNumArgs(numPassedArgs);

#ifdef HPHP_TRACE
  if (vmfp() == nullptr) {
    TRACE(1, "Reentry: enter %s(%p) from top-level\n",
          f->name()->data(), ar);
  } else {
    TRACE(1, "Reentry: enter %s(pc %p ar %p) from %s(%p)\n",
          f->name()->data(), vmpc(), ar,
          vmfp()->m_func ? vmfp()->m_func->name()->data()
                         : "unknownBuiltin",
          vmfp());
  }
#endif

  if (!varEnv) {
    auto const& prepArgs = cellIsNull(&args)
      ? make_tv<KindOfArray>(staticEmptyArray())
      : args;
    auto prepResult = prepareArrayArgs(
      ar, prepArgs,
      vmStack(), 0,
      (bool) (flags & InvokeCuf), retval);
    if (UNLIKELY(!prepResult)) {
      assert(KindOfNull == retval->m_type);
      return;
    }
  }

  pushVMState(originalSP);
  SCOPE_EXIT {
    assert(vmStack().top() == reentrySP);
    popVMState();
  };

  enterVM(ar, varEnv ? StackArgsState::Untrimmed : StackArgsState::Trimmed);

  tvCopy(*vmStack().topTV(), *retval);
  vmStack().discard();
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

  Cell* originalSP = vmRegsUnsafe().stack.top();

  VMRegAnchor _;
  DEBUG_ONLY Cell* reentrySP = vmStack().top();

  // See similar block of code above for why this is needed on
  // AttrPhpLeafFn.
  if (f->attrs() & AttrPhpLeafFn ||
      !(argc + kNumActRecCells <= kStackCheckReenterPadding)) {
    // Check both the native stack and VM stack for overflow
    checkStack(vmStack(), f, argc + kNumActRecCells);
  } else {
    // invokeFuncFew() must always check the native stack for overflow
    // no matter what
    checkNativeStack();
  }

  if (ObjectData* thiz = ActRec::decodeThis(thisOrCls)) {
    thiz->incRefCount();
  }

  ActRec* ar = vmStack().allocA();
  ar->setReturnVMExit();
  ar->m_func = f;
  ar->m_this = (ObjectData*)thisOrCls;
  ar->initNumArgs(argc);
  if (UNLIKELY(invName != nullptr)) {
    ar->setInvName(invName);
  } else {
    ar->m_varEnv = nullptr;
  }

#ifdef HPHP_TRACE
  if (vmfp() == nullptr) {
    TRACE(1, "Reentry: enter %s(%p) from top-level\n",
          f->name()->data(), ar);
  } else {
    TRACE(1, "Reentry: enter %s(pc %p ar %p) from %s(%p)\n",
          f->name()->data(), vmpc(), ar,
          vmfp()->m_func ? vmfp()->m_func->name()->data()
                         : "unknownBuiltin",
          vmfp());
  }
#endif

  for (ssize_t i = 0; i < argc; ++i) {
    const TypedValue *from = &argv[i];
    TypedValue *to = vmStack().allocTV();
    if (LIKELY(from->m_type != KindOfRef || !f->byRef(i))) {
      cellDup(*tvToCell(from), *to);
    } else {
      refDup(*from, *to);
    }
  }

  pushVMState(originalSP);
  SCOPE_EXIT {
    assert(vmStack().top() == reentrySP);
    popVMState();
  };

  enterVM(ar, StackArgsState::Untrimmed);

  tvCopy(*vmStack().topTV(), *retval);
  vmStack().discard();
}

void ExecutionContext::resumeAsyncFunc(Resumable* resumable,
                                       ObjectData* freeObj,
                                       const Cell& awaitResult) {
  assert(tl_regState == VMRegState::CLEAN);
  SCOPE_EXIT { assert(tl_regState == VMRegState::CLEAN); };

  auto fp = resumable->actRec();
  // We don't need to check for space for the ActRec (unlike generally
  // in normal re-entry), because the ActRec isn't on the stack.
  checkStack(vmStack(), fp->func(), 0);

  Cell* savedSP = vmStack().top();
  cellDup(awaitResult, *vmStack().allocC());

  // decref after awaitResult is on the stack
  decRefObj(freeObj);

  pushVMState(savedSP);
  SCOPE_EXIT { popVMState(); };

  enterVM(fp, StackArgsState::Untrimmed, resumable);
}

void ExecutionContext::resumeAsyncFuncThrow(Resumable* resumable,
                                            ObjectData* freeObj,
                                            ObjectData* exception) {
  assert(exception);
  assert(exception->instanceof(SystemLib::s_ExceptionClass));
  assert(tl_regState == VMRegState::CLEAN);
  SCOPE_EXIT { assert(tl_regState == VMRegState::CLEAN); };

  auto fp = resumable->actRec();
  checkStack(vmStack(), fp->func(), 0);

  // decref after we hold reference to the exception
  Object e(exception);
  decRefObj(freeObj);

  pushVMState(vmStack().top());
  SCOPE_EXIT { popVMState(); };

  enterVM(fp, StackArgsState::Untrimmed, resumable, exception);
}

void ExecutionContext::invokeUnit(TypedValue* retval, const Unit* unit) {
  auto const func = unit->getMain();
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
  ActRec* prevFp = fp->sfp();
  if (LIKELY(prevFp != nullptr)) {
    if (prevSp) {
      if (UNLIKELY(fp->resumed())) {
        assert(fp->func()->isGenerator());
        *prevSp = (TypedValue*)prevFp - prevFp->func()->numSlotsInFrame();
      } else {
        *prevSp = (TypedValue*)(fp + 1);
      }
    }
    if (prevPc) *prevPc = prevFp->func()->base() + fp->m_soff;
    if (fromVMEntry) *fromVMEntry = false;
    return prevFp;
  }
  // Linear search from end of m_nestedVMs. In practice, we're probably
  // looking for something recently pushed.
  int i = m_nestedVMs.size() - 1;
  ActRec* firstAR = vmFirstAR();
  while (i >= 0 && firstAR != fp) {
    firstAR = m_nestedVMs[i--].firstAR;
  }
  if (i == -1) return nullptr;
  const VMState& vmstate = m_nestedVMs[i];
  prevFp = vmstate.fp;
  assert(prevFp);
  assert(prevFp->func()->unit());
  if (prevSp) *prevSp = vmstate.sp;
  if (prevPc) {
    *prevPc = prevFp->func()->unit()->offsetOf(vmstate.pc);
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
      make_map_array(
        s_file, parserFrame->filename,
        s_line, parserFrame->lineNumber
      )
    );
  }

  VMRegAnchor _;
  if (!vmfp()) {
    // If there are no VM frames, we're done
    return bt;
  }

  int depth = 0;
  ActRec* fp = nullptr;
  Offset pc = 0;

  // Get the fp and pc of the top frame (possibly skipping one frame)
  {
    if (skip) {
      fp = getPrevVMState(vmfp(), &pc);
      if (!fp) {
        // We skipped over the only VM frame, we're done
        return bt;
      }
    } else {
      fp = vmfp();
      Unit *unit = vmfp()->m_func->unit();
      assert(unit);
      pc = unit->offsetOf(vmpc());
    }

    // Handle the top frame
    if (withSelf) {
      // Builtins don't have a file and line number
      if (!fp->m_func->isBuiltin()) {
        Unit* unit = fp->m_func->unit();
        assert(unit);
        const char* filename = fp->m_func->filename()->data();
        Offset off = pc;

        ArrayInit frame(parserFrame ? 4 : 2, ArrayInit::Map{});
        frame.set(s_file, filename);
        frame.set(s_line, unit->getLineNumber(off));
        if (parserFrame) {
          frame.set(s_function, s_include);
          frame.set(s_args, Array::Create(parserFrame->filename));
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

    ArrayInit frame(7, ArrayInit::Map{});

    auto const curUnit = fp->m_func->unit();
    auto const curOp = *reinterpret_cast<const Op*>(curUnit->at(pc));
    auto const isReturning =
      curOp == Op::RetC || curOp == Op::RetV ||
      curOp == Op::CreateCont || curOp == Op::Await ||
      fp->localsDecRefd();

    // Builtins and generators don't have a file and line number
    if (prevFp && !prevFp->m_func->isBuiltin() && !fp->resumed()) {
      auto const prevUnit = prevFp->m_func->unit();
      auto prevFile = prevUnit->filepath();
      if (prevFp->m_func->originalFilename()) {
        prevFile = prevFp->m_func->originalFilename();
      }
      assert(prevFile);
      frame.set(s_file, const_cast<StringData*>(prevFile));

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
                prevFp->m_func->unit()->getLineNumber(prevPc - pcAdjust));
    }

    // check for include
    String funcname = const_cast<StringData*>(fp->m_func->name());
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

    frame.set(s_function, funcname);

    if (!funcname.same(s_include)) {
      // Closures have an m_this but they aren't in object context
      Class* ctx = arGetContextClass(fp);
      if (ctx != nullptr && !fp->m_func->isClosureBody()) {
        frame.set(s_class, ctx->name()->data());
        if (fp->hasThis() && !isReturning) {
          if (withThis) {
            frame.set(s_object, Object(fp->getThis()));
          }
          frame.set(s_type, "->");
        } else {
          frame.set(s_type, "::");
        }
      }
    }

    Array args = Array::Create();
    if (ignoreArgs) {
      // do nothing
    } else if (funcname.same(s_include)) {
      if (depth) {
        args.append(const_cast<StringData*>(curUnit->filepath()));
        frame.set(s_args, args);
      }
    } else if (!RuntimeOption::EnableArgsInBacktraces || isReturning) {
      // Provide an empty 'args' array to be consistent with hphpc
      frame.set(s_args, args);
    } else {
      const int nparams = fp->m_func->numNonVariadicParams();
      int nargs = fp->numArgs();
      int nformals = std::min(nparams, nargs);

      if (UNLIKELY(fp->hasVarEnv() && fp->getVarEnv()->getFP() != fp)) {
        // VarEnv is attached to eval or debugger frame, other than the current
        // frame. Access locals thru VarEnv.
        auto varEnv = fp->getVarEnv();
        auto func = fp->func();
        for (int i = 0; i < nformals; i++) {
          TypedValue *arg = varEnv->lookup(func->localVarName(i));
          args.append(tvAsVariant(arg));
        }
      } else {
        for (int i = 0; i < nformals; i++) {
          TypedValue *arg = frame_local(fp, i);
          args.append(tvAsVariant(arg));
        }
      }

      /* builtin extra args are not stored in varenv */
      if (nargs > nparams && fp->hasExtraArgs()) {
        for (int i = nparams; i < nargs; i++) {
          TypedValue *arg = fp->getExtraArg(i - nparams);
          args.append(tvAsVariant(arg));
        }
      }
      frame.set(s_args, args);
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
  auto const tv = Unit::lookupCns(name.get());
  if (tv == nullptr) {
    return nullptr;
  }
  ConstInfoMap::const_iterator it = m_constInfo.find(name.get());
  if (it != m_constInfo.end()) {
    return it->second;
  }
  StringData* key = makeStaticString(name.get());
  ClassInfo::ConstantInfo* ci = new ClassInfo::ConstantInfo();
  ci->name = StrNR(key);
  ci->valueLen = 0;
  ci->valueText = "";
  ci->setValue(tvAsCVarRef(tv));
  m_constInfo[key] = ci;
  return ci;
}

/*
  Instantiate hoistable classes and functions.
  If there is any more work left to do, setup a
  new frame ready to execute the pseudomain.

  return true iff the pseudomain needs to be executed.
*/
bool ExecutionContext::evalUnit(Unit* unit, PC& pc, int funcType) {
  vmpc() = pc;
  unit->merge();
  if (unit->isMergeOnly()) {
    Stats::inc(Stats::PseudoMain_Skipped);
    *vmStack().allocTV() = *unit->getMainReturn();
    return false;
  }
  Stats::inc(Stats::PseudoMain_Executed);

  ActRec* ar = vmStack().allocA();
  assert((uintptr_t)&ar->m_func < (uintptr_t)&ar->m_r);
  Class* cls = liveClass();
  if (vmfp()->hasThis()) {
    ObjectData *this_ = vmfp()->getThis();
    this_->incRefCount();
    ar->setThis(this_);
  } else if (vmfp()->hasClass()) {
    ar->setClass(vmfp()->getClass());
  } else {
    ar->setThis(nullptr);
  }
  Func* func = unit->getMain(cls);
  assert(!func->isCPPBuiltin());
  ar->m_func = func;
  ar->initNumArgs(0);
  assert(vmfp());
  assert(!vmfp()->hasInvName());
  ar->setReturn(vmfp(), pc, mcg->tx().uniqueStubs.retHelper);
  pushLocalsAndIterators(func);
  if (!vmfp()->hasVarEnv()) {
    vmfp()->setVarEnv(VarEnv::createLocal(vmfp()));
  }
  ar->m_varEnv = vmfp()->m_varEnv;
  ar->m_varEnv->enterFP(vmfp(), ar);

  vmfp() = ar;
  pc = func->getEntry();
  SYNC();
  bool ret = EventHook::FunctionCall(vmfp(), funcType);
  pc = vmpc();
  checkStack(vmStack(), func, 0);
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
  auto key = StrNR(val);

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
  m_lastErrorPath = String::attach(getContainingFileName());
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
    vmStack().pushUninit();
  }
  // Push iterators.
  for (int i = 0; i < func->numIterators(); i++) {
    vmStack().allocI();
  }
}

void ExecutionContext::enqueueAPCHandle(APCHandle* handle, size_t size) {
  assert(handle->getUncounted() && size > 0);
  assert(handle->getType() == KindOfString ||
         handle->getType() == KindOfArray);
  m_apcHandles.m_handles.push_back(handle);
  m_apcHandles.m_memSize += size;
}

// Treadmill solution for the SharedVariant memory management
namespace {
class FreedAPCHandle {
  size_t m_memSize;
  std::vector<APCHandle*> m_apcHandles;
public:
  explicit FreedAPCHandle(std::vector<APCHandle*>&& shandles, size_t size)
    : m_memSize(size), m_apcHandles(std::move(shandles))
  {}
  void operator()() {
    for (auto handle : m_apcHandles) {
      APCTypedValue::fromHandle(handle)->deleteUncounted();
    }
    APCStats::getAPCStats().removePendingDelete(m_memSize);
  }
};
}

void ExecutionContext::manageAPCHandle() {
  assert(apcExtension::UseUncounted || m_apcHandles.m_handles.size() == 0);
  if (m_apcHandles.m_handles.size() > 0) {
    Treadmill::enqueue(
        FreedAPCHandle(std::move(m_apcHandles.m_handles),
                       m_apcHandles.m_memSize));
    APCStats::getAPCStats().addPendingDelete(m_apcHandles.m_memSize);
    m_apcHandles.m_handles.clear();
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

StrNR ExecutionContext::createFunction(const String& args,
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
  return lambda->nameStr();
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
  ActRec *fp = vmfp();
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
    // Start with the correct parent FP so that VarEnv can properly exitFP().
    // Note that if the same VarEnv is used across multiple frames, the most
    // recent FP must be used. This can happen if we are trying to debug
    // an eval() call or a call issued by debugger itself.
    auto savedFP = vmfp();
    if (fp) {
      vmfp() = fp->m_varEnv->getFP();
    }
    SCOPE_EXIT { vmfp() = savedFP; };

    // Invoke the given PHP, possibly specialized to match the type of the
    // current function on the stack, optionally passing a this pointer or
    // class used to execute the current function.
    invokeFunc(retval, unit->getMain(functionClass), init_null_variant,
               this_, frameClass, fp ? fp->m_varEnv : nullptr, nullptr,
               InvokePseudoMain);
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
  return failed;
}

void ExecutionContext::enterDebuggerDummyEnv() {
  static Unit* s_debuggerDummy = compile_string("<?php?>", 7);
  // Ensure that the VM stack is completely empty (vmfp() should be null)
  // and that we're not in a nested VM (reentrancy)
  assert(vmfp() == nullptr);
  assert(m_nestedVMs.size() == 0);
  assert(m_nesting == 0);
  assert(vmStack().count() == 0);
  ActRec* ar = vmStack().allocA();
  ar->m_func = s_debuggerDummy->getMain();
  ar->initNumArgs(0);
  ar->setThis(nullptr);
  ar->setReturnVMExit();
  vmfp() = ar;
  vmpc() = s_debuggerDummy->entry();
  vmFirstAR() = ar;
  vmfp()->setVarEnv(m_globalVarEnv);
  m_globalVarEnv->enterFP(nullptr, vmfp());
}

void ExecutionContext::exitDebuggerDummyEnv() {
  assert(m_globalVarEnv);
  // Ensure that vmfp() is valid
  assert(vmfp() != nullptr);
  // Ensure that vmfp() points to the only frame on the call stack.
  // In other words, make sure there are no VM frames directly below
  // this one and that we are not in a nested VM (reentrancy)
  assert(!vmfp()->sfp());
  assert(m_nestedVMs.size() == 0);
  assert(m_nesting == 0);
  // Teardown the frame we erected by enterDebuggerDummyEnv()
  const Func* func = vmfp()->m_func;
  try {
    frame_free_locals_inl_no_hook<true>(vmfp(), func->numLocals());
  } catch (...) {}
  vmStack().ndiscard(func->numSlotsInFrame());
  vmStack().discardAR();
  // After tearing down this frame, the VM stack should be completely empty
  assert(vmStack().count() == 0);
  vmfp() = nullptr;
  vmpc() = nullptr;
}

// Walk the stack and find any return address to jitted code and bash it to
// the appropriate RetFromInterpreted*Frame helper. This ensures that we don't
// return into jitted code and gives the system the proper chance to interpret
// blacklisted tracelets.
void ExecutionContext::preventReturnsToTC() {
  assert(isDebuggerAttached());
  if (RuntimeOption::EvalJit) {
    ActRec *ar = vmfp();
    while (ar) {
      if (!isReturnHelper(reinterpret_cast<void*>(ar->m_savedRip)) &&
          (mcg->isValidCodeAddress((JIT::TCA)ar->m_savedRip))) {
        TRACE_RB(2, "Replace RIP in fp %p, savedRip 0x%" PRIx64 ", "
                 "func %s\n", ar, ar->m_savedRip,
                 ar->m_func->fullName()->data());
        if (ar->resumed()) {
          ar->m_savedRip =
            reinterpret_cast<uintptr_t>(mcg->tx().uniqueStubs.genRetHelper);
        } else {
          ar->m_savedRip =
            reinterpret_cast<uintptr_t>(mcg->tx().uniqueStubs.retHelper);
        }
        assert(isReturnHelper(reinterpret_cast<void*>(ar->m_savedRip)));
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
    TypedValue* tv = vmStack().indTV(depth);
    tvRefcountedDecRef(tv);
  }

  if (!ndiscard) {
    tvRet = vmStack().allocTV();
  } else {
    vmStack().ndiscard(ndiscard - 1);
    tvRet = vmStack().topTV();
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
  Class* const ctx = arGetContextClass(vmfp());

  StringData* name;
  TypedValue* fr = nullptr;
  TypedValue* cref;
  TypedValue* pname;
  tvWriteUninit(&tvScratch);

  switch (lcode) {
  case LNL:
    loc = frame_local_inner(vmfp(), decodeVariableSizeImm(&vec));
    goto lcodeName;
  case LNC:
    loc = vmStack().indTV(depth--);
    goto lcodeName;

  lcodeName:
    if (define) {
      lookupd_var(vmfp(), name, loc, fr);
    } else {
      lookup_var(vmfp(), name, loc, fr);
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
    loc = frame_local_inner(vmfp(), decodeVariableSizeImm(&vec));
    goto lcodeGlobal;
  case LGC:
    loc = vmStack().indTV(depth--);
    goto lcodeGlobal;

  lcodeGlobal:
    if (define) {
      lookupd_gbl(vmfp(), name, loc, fr);
    } else {
      lookup_gbl(vmfp(), name, loc, fr);
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
    cref = vmStack().indTV(mdepth);
    pname = vmStack().indTV(depth--);
    goto lcodeSprop;
  case LSL:
    cref = vmStack().indTV(mdepth);
    pname = frame_local_inner(vmfp(), decodeVariableSizeImm(&vec));
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
    loc = frame_local_inner(vmfp(), localInd);
    if (warn) {
      if (loc->m_type == KindOfUninit) {
        raise_notice(Strings::UNDEFINED_VARIABLE,
                     vmfp()->m_func->localVarName(localInd)->data());
      }
    }
    break;
  }
  case LC:
  case LR:
    loc = vmStack().indTV(depth--);
    break;
  case LH:
    assert(vmfp()->hasThis());
    tvScratch.m_type = KindOfObject;
    tvScratch.m_data.pobj = vmfp()->getThis();
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
          vmfp()->m_func->unit()->lookupLitstrId(memberImm);
        assert(!IS_REFCOUNTED_TYPE(tvLiteral.m_type));
        curMember = &tvLiteral;
      } else if (mcode == MEI) {
        tvAsVariant(&tvLiteral) = memberImm;
        curMember = &tvLiteral;
      } else {
        assert(memberCodeImmIsLoc(mcode));
        curMember = frame_local_inner(vmfp(), memberImm);
      }
    } else {
      curMember = (setMember && mcode == MW) ? nullptr
                                             : vmStack().indTV(depth--);
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
        result =
          // We're not really going to modify it in the non-D case, so
          // this is safe.
          const_cast<TypedValue*>(
            Elem<warn>(tvScratch, tvRef, base, *curMember)
          );
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
    TypedValue* tv = vmStack().indTV(depth);
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
      TypedValue* retSrc = vmStack().topTV();
      TypedValue* dest = vmStack().indTV(ndiscard + mdepth - 1);
      assert(dest != retSrc);
      memcpy(dest, retSrc, sizeof *dest);
    }
  }

  vmStack().ndiscard(ndiscard);
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
  vmStack().popA();
}

OPTBLD_INLINE void ExecutionContext::iopPopC(IOP_ARGS) {
  NEXT();
  vmStack().popC();
}

OPTBLD_INLINE void ExecutionContext::iopPopV(IOP_ARGS) {
  NEXT();
  vmStack().popV();
}

OPTBLD_INLINE void ExecutionContext::iopPopR(IOP_ARGS) {
  NEXT();
  if (vmStack().topTV()->m_type != KindOfRef) {
    vmStack().popC();
  } else {
    vmStack().popV();
  }
}

OPTBLD_INLINE void ExecutionContext::iopDup(IOP_ARGS) {
  NEXT();
  vmStack().dup();
}

OPTBLD_INLINE void ExecutionContext::iopBox(IOP_ARGS) {
  NEXT();
  vmStack().box();
}

OPTBLD_INLINE void ExecutionContext::iopUnbox(IOP_ARGS) {
  NEXT();
  vmStack().unbox();
}

OPTBLD_INLINE void ExecutionContext::iopBoxR(IOP_ARGS) {
  NEXT();
  TypedValue* tv = vmStack().topTV();
  if (tv->m_type != KindOfRef) {
    tvBox(tv);
  }
}

OPTBLD_INLINE void ExecutionContext::iopBoxRNop(IOP_ARGS) {
  NEXT();
  assert(refIsPlausible(*vmStack().topTV()));
}

OPTBLD_INLINE void ExecutionContext::iopUnboxR(IOP_ARGS) {
  NEXT();
  if (vmStack().topTV()->m_type == KindOfRef) {
    vmStack().unbox();
  }
}

OPTBLD_INLINE void ExecutionContext::iopUnboxRNop(IOP_ARGS) {
  NEXT();
  assert(cellIsPlausible(*vmStack().topTV()));
}

OPTBLD_INLINE void ExecutionContext::iopNull(IOP_ARGS) {
  NEXT();
  vmStack().pushNull();
}

OPTBLD_INLINE void ExecutionContext::iopNullUninit(IOP_ARGS) {
  NEXT();
  vmStack().pushNullUninit();
}

OPTBLD_INLINE void ExecutionContext::iopTrue(IOP_ARGS) {
  NEXT();
  vmStack().pushTrue();
}

OPTBLD_INLINE void ExecutionContext::iopFalse(IOP_ARGS) {
  NEXT();
  vmStack().pushFalse();
}

OPTBLD_INLINE void ExecutionContext::iopFile(IOP_ARGS) {
  NEXT();
  const StringData* s = vmfp()->m_func->unit()->filepath();
  vmStack().pushStaticString(const_cast<StringData*>(s));
}

OPTBLD_INLINE void ExecutionContext::iopDir(IOP_ARGS) {
  NEXT();
  const StringData* s = vmfp()->m_func->unit()->dirpath();
  vmStack().pushStaticString(const_cast<StringData*>(s));
}

OPTBLD_INLINE void ExecutionContext::iopNameA(IOP_ARGS) {
  NEXT();
  auto const cls  = vmStack().topA();
  auto const name = cls->name();
  vmStack().popA();
  vmStack().pushStaticString(const_cast<StringData*>(name));
}

OPTBLD_INLINE void ExecutionContext::iopInt(IOP_ARGS) {
  NEXT();
  DECODE(int64_t, i);
  vmStack().pushInt(i);
}

OPTBLD_INLINE void ExecutionContext::iopDouble(IOP_ARGS) {
  NEXT();
  DECODE(double, d);
  vmStack().pushDouble(d);
}

OPTBLD_INLINE void ExecutionContext::iopString(IOP_ARGS) {
  NEXT();
  DECODE_LITSTR(s);
  vmStack().pushStaticString(s);
}

OPTBLD_INLINE void ExecutionContext::iopArray(IOP_ARGS) {
  NEXT();
  DECODE(Id, id);
  ArrayData* a = vmfp()->m_func->unit()->lookupArrayId(id);
  vmStack().pushStaticArray(a);
}

OPTBLD_INLINE void ExecutionContext::iopNewArray(IOP_ARGS) {
  NEXT();
  DECODE_IVA(capacity);
  if (capacity == 0) {
    vmStack().pushArrayNoRc(staticEmptyArray());
  } else {
    vmStack().pushArrayNoRc(MixedArray::MakeReserve(capacity));
  }
}

OPTBLD_INLINE void ExecutionContext::iopNewMixedArray(IOP_ARGS) {
  NEXT();
  DECODE_IVA(capacity);
  if (capacity == 0) {
    vmStack().pushArrayNoRc(staticEmptyArray());
  } else {
    vmStack().pushArrayNoRc(MixedArray::MakeReserveMixed(capacity));
  }
}

OPTBLD_INLINE void ExecutionContext::iopNewLikeArrayL(IOP_ARGS) {
  NEXT();
  DECODE_LA(local);
  DECODE_IVA(capacity);

  ArrayData* arr;
  TypedValue* fr = frame_local(vmfp(), local);

  if (LIKELY(fr->m_type == KindOfArray)) {
    arr = MixedArray::MakeReserveLike(fr->m_data.parr, capacity);
  } else {
    capacity = (capacity ? capacity : MixedArray::SmallSize);
    arr = MixedArray::MakeReserve(capacity);
  }
  vmStack().pushArrayNoRc(arr);
}

OPTBLD_INLINE void ExecutionContext::iopNewPackedArray(IOP_ARGS) {
  NEXT();
  DECODE_IVA(n);
  // This constructor moves values, no inc/decref is necessary.
  auto* a = MixedArray::MakePacked(n, vmStack().topC());
  vmStack().ndiscard(n);
  vmStack().pushArrayNoRc(a);
}

OPTBLD_INLINE void ExecutionContext::iopNewStructArray(IOP_ARGS) {
  NEXT();
  DECODE(uint32_t, n); // number of keys and elements
  assert(n > 0 && n <= MixedArray::MaxMakeSize);
  StringData* names[MixedArray::MaxMakeSize];
  for (size_t i = 0; i < n; i++) {
    DECODE_LITSTR(s);
    names[i] = s;
  }
  // This constructor moves values, no inc/decref is necessary.
  auto* a = MixedArray::MakeStruct(n, names, vmStack().topC());
  vmStack().ndiscard(n);
  vmStack().pushArrayNoRc(a->asArrayData());
}

OPTBLD_INLINE void ExecutionContext::iopAddElemC(IOP_ARGS) {
  NEXT();
  Cell* c1 = vmStack().topC();
  Cell* c2 = vmStack().indC(1);
  Cell* c3 = vmStack().indC(2);
  if (c3->m_type != KindOfArray) {
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

OPTBLD_INLINE void ExecutionContext::iopAddElemV(IOP_ARGS) {
  NEXT();
  Ref* r1 = vmStack().topV();
  Cell* c2 = vmStack().indC(1);
  Cell* c3 = vmStack().indC(2);
  if (c3->m_type != KindOfArray) {
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

OPTBLD_INLINE void ExecutionContext::iopAddNewElemC(IOP_ARGS) {
  NEXT();
  Cell* c1 = vmStack().topC();
  Cell* c2 = vmStack().indC(1);
  if (c2->m_type != KindOfArray) {
    raise_error("AddNewElemC: $2 must be an array");
  }
  cellAsVariant(*c2).asArrRef().append(tvAsCVarRef(c1));
  vmStack().popC();
}

OPTBLD_INLINE void ExecutionContext::iopAddNewElemV(IOP_ARGS) {
  NEXT();
  Ref* r1 = vmStack().topV();
  Cell* c2 = vmStack().indC(1);
  if (c2->m_type != KindOfArray) {
    raise_error("AddNewElemV: $2 must be an array");
  }
  cellAsVariant(*c2).asArrRef().appendRef(tvAsVariant(r1));
  vmStack().popV();
}

OPTBLD_INLINE void ExecutionContext::iopNewCol(IOP_ARGS) {
  NEXT();
  DECODE_IVA(cType);
  DECODE_IVA(nElms);
  ObjectData* obj = newCollectionHelper(cType, nElms);
  vmStack().pushObject(obj);
}

OPTBLD_INLINE void ExecutionContext::iopColAddNewElemC(IOP_ARGS) {
  NEXT();
  Cell* c1 = vmStack().topC();
  Cell* c2 = vmStack().indC(1);
  if (c2->m_type == KindOfObject && c2->m_data.pobj->isCollection()) {
    collectionInitAppend(c2->m_data.pobj, c1);
  } else {
    raise_error("ColAddNewElemC: $2 must be a collection");
  }
  vmStack().popC();
}

OPTBLD_INLINE void ExecutionContext::iopColAddElemC(IOP_ARGS) {
  NEXT();
  Cell* c1 = vmStack().topC();
  Cell* c2 = vmStack().indC(1);
  Cell* c3 = vmStack().indC(2);
  if (c3->m_type == KindOfObject && c3->m_data.pobj->isCollection()) {
    collectionInitSet(c3->m_data.pobj, c2, c1);
  } else {
    raise_error("ColAddElemC: $3 must be a collection");
  }
  vmStack().popC();
  vmStack().popC();
}

OPTBLD_INLINE void ExecutionContext::iopCns(IOP_ARGS) {
  NEXT();
  DECODE_LITSTR(s);
  auto const cns = Unit::loadCns(s);
  if (cns == nullptr) {
    raise_notice(Strings::UNDEFINED_CONSTANT, s->data(), s->data());
    vmStack().pushStaticString(s);
    return;
  }
  auto const c1 = vmStack().allocC();
  cellDup(*cns, *c1);
}

OPTBLD_INLINE void ExecutionContext::iopCnsE(IOP_ARGS) {
  NEXT();
  DECODE_LITSTR(s);
  auto const cns = Unit::loadCns(s);
  if (cns == nullptr) {
    raise_error("Undefined constant '%s'", s->data());
  }
  auto const c1 = vmStack().allocC();
  cellDup(*cns, *c1);
}

OPTBLD_INLINE void ExecutionContext::iopCnsU(IOP_ARGS) {
  NEXT();
  DECODE_LITSTR(name);
  DECODE_LITSTR(fallback);
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

OPTBLD_INLINE void ExecutionContext::iopDefCns(IOP_ARGS) {
  NEXT();
  DECODE_LITSTR(s);
  bool result = Unit::defCns(s, vmStack().topTV());
  vmStack().replaceTV<KindOfBoolean>(result);
}

OPTBLD_INLINE void ExecutionContext::iopClsCns(IOP_ARGS) {
  NEXT();
  DECODE_LITSTR(clsCnsName);

  auto const cls    = vmStack().topA();
  auto const clsCns = cls->clsCnsGet(clsCnsName);

  if (clsCns.m_type == KindOfUninit) {
    raise_error("Couldn't find constant %s::%s",
                cls->name()->data(), clsCnsName->data());
  }

  cellDup(clsCns, *vmStack().topTV());
}

OPTBLD_INLINE void ExecutionContext::iopClsCnsD(IOP_ARGS) {
  NEXT();
  DECODE_LITSTR(clsCnsName);
  DECODE(Id, classId);
  const NamedEntityPair& classNamedEntity =
    vmfp()->m_func->unit()->lookupNamedEntityPairId(classId);

  auto const clsCns = lookupClsCns(classNamedEntity.second,
                                   classNamedEntity.first, clsCnsName);
  auto const c1 = vmStack().allocC();
  cellDup(clsCns, *c1);
}

OPTBLD_INLINE void ExecutionContext::iopConcat(IOP_ARGS) {
  NEXT();
  Cell* c1 = vmStack().topC();
  Cell* c2 = vmStack().indC(1);

  cellAsVariant(*c2) = concat(cellAsVariant(*c2).toString(),
                              cellAsCVarRef(*c1).toString());
  assert_refcount_realistic_nz(c2->m_data.pstr->getCount());
  vmStack().popC();
}

OPTBLD_INLINE void ExecutionContext::iopConcatN(IOP_ARGS) {
  NEXT();
  DECODE_IVA(n);

  Cell* c1 = vmStack().topC();
  Cell* c2 = vmStack().indC(1);

  if (n == 2) {
    cellAsVariant(*c2) = concat(cellAsVariant(*c2).toString(),
                                cellAsCVarRef(*c1).toString());
    assert_refcount_realistic_nz(c2->m_data.pstr->getCount());
  } else if (n == 3) {
    Cell* c3 = vmStack().indC(2);
    cellAsVariant(*c3) = concat3(cellAsVariant(*c3).toString(),
                                 cellAsCVarRef(*c2).toString(),
                                 cellAsCVarRef(*c1).toString());
    assert_refcount_realistic_nz(c3->m_data.pstr->getCount());
  } else /* n == 4 */ {
    Cell* c3 = vmStack().indC(2);
    Cell* c4 = vmStack().indC(3);
    cellAsVariant(*c4) = concat4(cellAsVariant(*c4).toString(),
                                 cellAsCVarRef(*c3).toString(),
                                 cellAsCVarRef(*c2).toString(),
                                 cellAsCVarRef(*c1).toString());
    assert_refcount_realistic_nz(c4->m_data.pstr->getCount());
  }

  for (int i = 1; i < n; ++i) {
    vmStack().popC();
  }
}

OPTBLD_INLINE void ExecutionContext::iopNot(IOP_ARGS) {
  NEXT();
  Cell* c1 = vmStack().topC();
  cellAsVariant(*c1) = !cellAsVariant(*c1).toBoolean();
}

OPTBLD_INLINE void ExecutionContext::iopAbs(IOP_ARGS) {
  NEXT();
  auto c1 = vmStack().topC();

  tvAsVariant(c1) = f_abs(tvAsCVarRef(c1));
}

template<class Op>
OPTBLD_INLINE void ExecutionContext::implCellBinOp(IOP_ARGS, Op op) {
  NEXT();
  auto const c1 = vmStack().topC();
  auto const c2 = vmStack().indC(1);
  auto const result = op(*c2, *c1);
  tvRefcountedDecRefCell(c2);
  *c2 = result;
  vmStack().popC();
}

template<class Op>
OPTBLD_INLINE void ExecutionContext::implCellBinOpBool(IOP_ARGS, Op op) {
  NEXT();
  auto const c1 = vmStack().topC();
  auto const c2 = vmStack().indC(1);
  bool const result = op(*c2, *c1);
  tvRefcountedDecRefCell(c2);
  *c2 = make_tv<KindOfBoolean>(result);
  vmStack().popC();
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

OPTBLD_INLINE void ExecutionContext::iopPow(IOP_ARGS) {
  implCellBinOp(IOP_PASS_ARGS, cellPow);
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
  implCellBinOp(IOP_PASS_ARGS, cellShl);
}

OPTBLD_INLINE void ExecutionContext::iopShr(IOP_ARGS) {
  implCellBinOp(IOP_PASS_ARGS, cellShr);
}

OPTBLD_INLINE void ExecutionContext::iopSqrt(IOP_ARGS) {
  NEXT();
  Cell* c1 = vmStack().topC();

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
  cellBitNot(*vmStack().topC());
}

OPTBLD_INLINE void ExecutionContext::iopCastBool(IOP_ARGS) {
  NEXT();
  Cell* c1 = vmStack().topC();
  tvCastToBooleanInPlace(c1);
}

OPTBLD_INLINE void ExecutionContext::iopCastInt(IOP_ARGS) {
  NEXT();
  Cell* c1 = vmStack().topC();
  tvCastToInt64InPlace(c1);
}

OPTBLD_INLINE void ExecutionContext::iopCastDouble(IOP_ARGS) {
  NEXT();
  Cell* c1 = vmStack().topC();
  tvCastToDoubleInPlace(c1);
}

OPTBLD_INLINE void ExecutionContext::iopCastString(IOP_ARGS) {
  NEXT();
  Cell* c1 = vmStack().topC();
  tvCastToStringInPlace(c1);
}

OPTBLD_INLINE void ExecutionContext::iopCastArray(IOP_ARGS) {
  NEXT();
  Cell* c1 = vmStack().topC();
  tvCastToArrayInPlace(c1);
}

OPTBLD_INLINE void ExecutionContext::iopCastObject(IOP_ARGS) {
  NEXT();
  Cell* c1 = vmStack().topC();
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
  Cell* c1 = vmStack().topC();   // c2 instanceof c1
  Cell* c2 = vmStack().indC(1);
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
  vmStack().popC();
  vmStack().replaceC<KindOfBoolean>(r);
}

OPTBLD_INLINE void ExecutionContext::iopInstanceOfD(IOP_ARGS) {
  NEXT();
  DECODE(Id, id);
  if (shouldProfile()) {
    InstanceBits::profile(vmfp()->m_func->unit()->lookupLitstrId(id));
  }
  const NamedEntity* ne = vmfp()->m_func->unit()->lookupNamedEntityId(id);
  Cell* c1 = vmStack().topC();
  bool r = cellInstanceOf(c1, ne);
  vmStack().replaceC<KindOfBoolean>(r);
}

OPTBLD_INLINE void ExecutionContext::iopPrint(IOP_ARGS) {
  NEXT();
  Cell* c1 = vmStack().topC();
  write(cellAsVariant(*c1).toString());
  vmStack().replaceC<KindOfInt64>(1);
}

OPTBLD_INLINE void ExecutionContext::iopClone(IOP_ARGS) {
  NEXT();
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

OPTBLD_INLINE void ExecutionContext::iopExit(IOP_ARGS) {
  NEXT();
  int exitCode = 0;
  Cell* c1 = vmStack().topC();
  if (c1->m_type == KindOfInt64) {
    exitCode = c1->m_data.num;
  } else {
    write(cellAsVariant(*c1).toString());
  }
  vmStack().popC();
  vmStack().pushNull();
  throw ExitException(exitCode);
}

OPTBLD_INLINE void ExecutionContext::iopFatal(IOP_ARGS) {
  NEXT();
  TypedValue* top = vmStack().topTV();
  std::string msg;
  DECODE_OA(FatalOp, kind_char);
  if (IS_STRING_TYPE(top->m_type)) {
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

  Cell* c1 = vmStack().topC();
  if (c1->m_type == KindOfInt64 || c1->m_type == KindOfBoolean) {
    int64_t n = c1->m_data.num;
    if (op == OpJmpZ ? n == 0 : n != 0) {
      pc += offset - 1;
      vmStack().popX();
    } else {
      pc += sizeof(Offset);
      vmStack().popX();
    }
  } else {
    auto const condition = toBoolean(cellAsCVarRef(*c1));
    if (op == OpJmpZ ? !condition : condition) {
      pc += offset - 1;
      vmStack().popC();
    } else {
      pc += sizeof(Offset);
      vmStack().popC();
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
    Iter *iter = frame_iter(vmfp(), iterId);                      \
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

  TypedValue* val = vmStack().topTV();
  if (!bounded) {
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

OPTBLD_INLINE void ExecutionContext::iopSSwitch(IOP_ARGS) {
  PC origPC = pc;
  NEXT();
  DECODE(int32_t, veclen);
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
      break;
    }
  }
  if (i == cases) {
    // default case
    pc = origPC + jmptab[veclen-1].dest;
  }
  vmStack().popC();
}

OPTBLD_INLINE void ExecutionContext::ret(IOP_ARGS) {
  // Get the return value.
  TypedValue retval = *vmStack().topTV();

  // Free $this and local variables. Calls FunctionReturn hook. The return value
  // is kept on the stack so that the unwinder would free it if the hook fails.
  frame_free_locals_inl(vmfp(), vmfp()->func()->numLocals(), &retval);
  vmStack().discard();

  // If in an eagerly executed async function, wrap the return value
  // into succeeded StaticWaitHandle.
  if (UNLIKELY(!vmfp()->resumed() && vmfp()->func()->isAsyncFunction())) {
    auto const& retvalCell = *tvAssertCell(&retval);
    auto const waitHandle = c_StaticWaitHandle::CreateSucceeded(retvalCell);
    cellCopy(make_tv<KindOfObject>(waitHandle), retval);
  }

  if (shouldProfile()) {
    auto f = const_cast<Func*>(vmfp()->func());
    f->incProfCounter();
    if (!(f->isPseudoMain() || f->isClosureBody() || f->isMagic() ||
          Func::isSpecial(f->name()))) {
      recordType(TypeProfileKey(TypeProfileKey::MethodName, f->name()),
                 retval.m_type);
    }
  }

  // Type profile return value.
  if (RuntimeOption::EvalRuntimeTypeProfile) {
    profileOneArgument(retval, -1, vmfp()->func());
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
  } else if (vmfp()->func()->isAsyncFunction()) {
    // Mark the async function as succeeded and store the return value.
    assert(!sfp);
    frame_afwh(vmfp())->ret(retval);
  } else if (vmfp()->func()->isAsyncGenerator()) {
    // Mark the async generator as finished.
    assert(IS_NULL_TYPE(retval.m_type));
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
    assert(IS_NULL_TYPE(retval.m_type));
    frame_generator(vmfp())->ret();

    // Push return value of next()/send()/raise().
    vmStack().pushNull();
  } else {
    not_reached();
  }

  // Return control to the caller.
  vmfp() = sfp;
  pc = LIKELY(vmfp() != nullptr) ? vmfp()->func()->getEntry() + soff : nullptr;
}

OPTBLD_INLINE void ExecutionContext::iopRetC(IOP_ARGS) {
  NEXT();
  ret(IOP_PASS_ARGS);
}

OPTBLD_INLINE void ExecutionContext::iopRetV(IOP_ARGS) {
  NEXT();
  assert(!vmfp()->resumed());
  assert(!vmfp()->func()->isResumable());
  ret(IOP_PASS_ARGS);
}

OPTBLD_INLINE void ExecutionContext::iopUnwind(IOP_ARGS) {
  assert(!m_faults.empty());
  assert(m_faults.back().m_raiseOffset != kInvalidOffset);
  throw VMPrepareUnwind();
}

OPTBLD_INLINE void ExecutionContext::iopThrow(IOP_ARGS) {
  Cell* c1 = vmStack().topC();
  if (c1->m_type != KindOfObject ||
      !c1->m_data.pobj->instanceof(SystemLib::s_ExceptionClass)) {
    raise_error("Exceptions must be valid objects derived from the "
                "Exception base class");
  }

  Object obj(c1->m_data.pobj);
  vmStack().popC();
  DEBUGGER_ATTACHED_ONLY(phpDebuggerExceptionThrownHook(obj.get()));
  throw obj;
}

OPTBLD_INLINE void ExecutionContext::iopAGetC(IOP_ARGS) {
  NEXT();
  TypedValue* tv = vmStack().topTV();
  lookupClsRef(tv, tv, true);
}

OPTBLD_INLINE void ExecutionContext::iopAGetL(IOP_ARGS) {
  NEXT();
  DECODE_LA(local);
  TypedValue* top = vmStack().allocTV();
  TypedValue* fr = frame_local_inner(vmfp(), local);
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
  Cell* to = vmStack().allocC();
  TypedValue* fr = frame_local(vmfp(), local);
  cgetl_body(vmfp(), fr, to, local);
}

OPTBLD_INLINE void ExecutionContext::iopCGetL2(IOP_ARGS) {
  NEXT();
  DECODE_LA(local);
  TypedValue* oldTop = vmStack().topTV();
  TypedValue* newTop = vmStack().allocTV();
  memcpy(newTop, oldTop, sizeof *newTop);
  Cell* to = oldTop;
  TypedValue* fr = frame_local(vmfp(), local);
  cgetl_body(vmfp(), fr, to, local);
}

OPTBLD_INLINE void ExecutionContext::iopCGetL3(IOP_ARGS) {
  NEXT();
  DECODE_LA(local);
  TypedValue* oldTop = vmStack().topTV();
  TypedValue* oldSubTop = vmStack().indTV(1);
  TypedValue* newTop = vmStack().allocTV();
  memmove(newTop, oldTop, sizeof *oldTop * 2);
  Cell* to = oldSubTop;
  TypedValue* fr = frame_local(vmfp(), local);
  cgetl_body(vmfp(), fr, to, local);
}

OPTBLD_INLINE void ExecutionContext::iopPushL(IOP_ARGS) {
  NEXT();
  DECODE_LA(local);
  TypedValue* locVal = frame_local(vmfp(), local);
  assert(locVal->m_type != KindOfUninit);
  assert(locVal->m_type != KindOfRef);

  TypedValue* dest = vmStack().allocTV();
  *dest = *locVal;
  locVal->m_type = KindOfUninit;
}

OPTBLD_INLINE void ExecutionContext::iopCGetN(IOP_ARGS) {
  NEXT();
  StringData* name;
  TypedValue* to = vmStack().topTV();
  TypedValue* fr = nullptr;
  lookup_var(vmfp(), name, to, fr);
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
  TypedValue* to = vmStack().topTV();
  TypedValue* fr = nullptr;
  lookup_gbl(vmfp(), name, to, fr);
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
  TypedValue* clsref = vmStack().topTV();                   \
  TypedValue* nameCell = vmStack().indTV(1);                \
  TypedValue* output = nameCell;                          \
  TypedValue* val;                                        \
  bool visible, accessible;                               \
  lookup_sprop(vmfp(), clsref, name, nameCell, val, visible, \
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
  vmStack().popA();                                         \
  SPROP_OP_POSTLUDE                                       \
} while (0)

OPTBLD_INLINE void ExecutionContext::iopCGetS(IOP_ARGS) {
  StringData* name;
  GETS(false);
  if (shouldProfile() && name && name->isStatic()) {
    recordType(TypeProfileKey(TypeProfileKey::StaticPropName, name),
               vmStack().top()->m_type);
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
  if (immVec.decodeLastMember(vmfp()->unit(), name, mc)) {
    recordType(TypeProfileKey(mc, name), vmStack().top()->m_type);
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
  Ref* to = vmStack().allocV();
  TypedValue* fr = frame_local(vmfp(), local);
  vgetl_body(fr, to);
}

OPTBLD_INLINE void ExecutionContext::iopVGetN(IOP_ARGS) {
  NEXT();
  StringData* name;
  TypedValue* to = vmStack().topTV();
  TypedValue* fr = nullptr;
  lookupd_var(vmfp(), name, to, fr);
  assert(fr != nullptr);
  tvRefcountedDecRefCell(to);
  vgetl_body(fr, to);
  decRefStr(name);
}

OPTBLD_INLINE void ExecutionContext::iopVGetG(IOP_ARGS) {
  NEXT();
  StringData* name;
  TypedValue* to = vmStack().topTV();
  TypedValue* fr = nullptr;
  lookupd_gbl(vmfp(), name, to, fr);
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
  TypedValue* tv1 = vmStack().allocTV();
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
  TypedValue* tv1 = vmStack().topTV();
  TypedValue* tv = nullptr;
  bool e;
  lookup_var(vmfp(), name, tv1, tv);
  if (tv == nullptr) {
    e = false;
  } else {
    e = !cellIsNull(tvToCell(tv));
  }
  vmStack().replaceC<KindOfBoolean>(e);
  decRefStr(name);
}

OPTBLD_INLINE void ExecutionContext::iopIssetG(IOP_ARGS) {
  NEXT();
  StringData* name;
  TypedValue* tv1 = vmStack().topTV();
  TypedValue* tv = nullptr;
  bool e;
  lookup_gbl(vmfp(), name, tv1, tv);
  if (tv == nullptr) {
    e = false;
  } else {
    e = !cellIsNull(tvToCell(tv));
  }
  vmStack().replaceC<KindOfBoolean>(e);
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
  vmStack().popA();
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
    Class* ctx = arGetContextClass(vmfp());
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

OPTBLD_INLINE void ExecutionContext::iopIsTypeL(IOP_ARGS) {
  NEXT();
  DECODE_LA(local);
  DECODE_OA(IsTypeOp, op);
  TypedValue* tv = frame_local(vmfp(), local);
  if (tv->m_type == KindOfUninit) {
    raise_undefined_local(vmfp(), local);
  }
  TypedValue* topTv = vmStack().allocTV();
  topTv->m_data.num = isTypeHelper(tv, op);
  topTv->m_type = KindOfBoolean;
}

OPTBLD_INLINE void ExecutionContext::iopIsTypeC(IOP_ARGS) {
  NEXT();
  DECODE_OA(IsTypeOp, op);
  TypedValue* topTv = vmStack().topTV();
  assert(topTv->m_type != KindOfRef);
  bool ret = isTypeHelper(topTv, op);
  tvRefcountedDecRefCell(topTv);
  topTv->m_data.num = ret;
  topTv->m_type = KindOfBoolean;
}

OPTBLD_INLINE void ExecutionContext::iopAssertRATL(IOP_ARGS) {
  NEXT();
  DECODE_LA(localId);
  if (debug) {
    auto const rat = decodeRAT(vmfp()->m_func->unit(), pc);
    auto const tv = *frame_local(vmfp(), localId);
    auto const func = vmfp()->func();
    always_assert_flog(
      tvMatchesRepoAuthType(tv, rat),
      "failed assert RATL on local {}: ${} in {}:{}, expected {}, got {}",
      localId,
      localId < func->numNamedLocals() ? func->localNames()[localId]->data()
                                       : "<unnamed>",
      getContainingFileName()->data(),
      getLine(),
      show(rat),
      tv.pretty()
    );
    return;
  }
  pc += encodedRATSize(pc);
}

OPTBLD_INLINE void ExecutionContext::iopAssertRATStk(IOP_ARGS) {
  NEXT();
  DECODE_IVA(stkSlot);
  if (debug) {
    auto const rat = decodeRAT(vmfp()->m_func->unit(), pc);
    auto const tv = *vmStack().indTV(stkSlot);
    always_assert_flog(
      tvMatchesRepoAuthType(tv, rat),
      "failed assert RATStk {} in {}:{}, expected {}, got {}",
      stkSlot,
      getContainingFileName()->data(),
      getLine(),
      show(rat),
      tv.pretty()
    );
    return;
  }
  pc += encodedRATSize(pc);
}

OPTBLD_INLINE void ExecutionContext::iopBreakTraceHint(IOP_ARGS) {
  NEXT();
}

OPTBLD_INLINE void ExecutionContext::iopEmptyL(IOP_ARGS) {
  NEXT();
  DECODE_LA(local);
  TypedValue* loc = frame_local(vmfp(), local);
  bool e = !cellToBool(*tvToCell(loc));
  vmStack().pushBool(e);
}

OPTBLD_INLINE void ExecutionContext::iopEmptyN(IOP_ARGS) {
  NEXT();
  StringData* name;
  TypedValue* tv1 = vmStack().topTV();
  TypedValue* tv = nullptr;
  bool e;
  lookup_var(vmfp(), name, tv1, tv);
  if (tv == nullptr) {
    e = true;
  } else {
    e = !cellToBool(*tvToCell(tv));
  }
  vmStack().replaceC<KindOfBoolean>(e);
  decRefStr(name);
}

OPTBLD_INLINE void ExecutionContext::iopEmptyG(IOP_ARGS) {
  NEXT();
  StringData* name;
  TypedValue* tv1 = vmStack().topTV();
  TypedValue* tv = nullptr;
  bool e;
  lookup_gbl(vmfp(), name, tv1, tv);
  if (tv == nullptr) {
    e = true;
  } else {
    e = !cellToBool(*tvToCell(tv));
  }
  vmStack().replaceC<KindOfBoolean>(e);
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
  vmStack().popA();
  output->m_data.num = e;
  output->m_type = KindOfBoolean;
  SPROP_OP_POSTLUDE
}

OPTBLD_INLINE void ExecutionContext::iopEmptyM(IOP_ARGS) {
  isSetEmptyM<true>(IOP_PASS_ARGS);
}

OPTBLD_INLINE void ExecutionContext::iopAKExists(IOP_ARGS) {
  NEXT();
  TypedValue* arr = vmStack().topTV();
  TypedValue* key = arr + 1;
  bool result = f_array_key_exists(tvAsCVarRef(key), tvAsCVarRef(arr));
  vmStack().popTV();
  vmStack().replaceTV<KindOfBoolean>(result);
}

OPTBLD_INLINE void ExecutionContext::iopIdx(IOP_ARGS) {
  NEXT();
  TypedValue* def = vmStack().topTV();
  TypedValue* key = vmStack().indTV(1);
  TypedValue* arr = vmStack().indTV(2);

  TypedValue result = JIT::genericIdx(*arr, *key, *def);
  vmStack().popTV();
  vmStack().popTV();
  tvRefcountedDecRef(arr);
  *arr = result;
}

OPTBLD_INLINE void ExecutionContext::iopArrayIdx(IOP_ARGS) {
  NEXT();
  TypedValue* def = vmStack().topTV();
  TypedValue* key = vmStack().indTV(1);
  TypedValue* arr = vmStack().indTV(2);

  Variant result = f_hphp_array_idx(tvAsCVarRef(arr),
                                    tvAsCVarRef(key),
                                    tvAsCVarRef(def));
  vmStack().popTV();
  vmStack().popTV();
  tvAsVariant(arr) = result;
}

OPTBLD_INLINE void ExecutionContext::iopSetL(IOP_ARGS) {
  NEXT();
  DECODE_LA(local);
  assert(local < vmfp()->m_func->numLocals());
  Cell* fr = vmStack().topC();
  TypedValue* to = frame_local(vmfp(), local);
  tvSet(*fr, *to);
}

OPTBLD_INLINE void ExecutionContext::iopSetN(IOP_ARGS) {
  NEXT();
  StringData* name;
  Cell* fr = vmStack().topC();
  TypedValue* tv2 = vmStack().indTV(1);
  TypedValue* to = nullptr;
  lookupd_var(vmfp(), name, tv2, to);
  assert(to != nullptr);
  tvSet(*fr, *to);
  memcpy((void*)tv2, (void*)fr, sizeof(TypedValue));
  vmStack().discard();
  decRefStr(name);
}

OPTBLD_INLINE void ExecutionContext::iopSetG(IOP_ARGS) {
  NEXT();
  StringData* name;
  Cell* fr = vmStack().topC();
  TypedValue* tv2 = vmStack().indTV(1);
  TypedValue* to = nullptr;
  lookupd_gbl(vmfp(), name, tv2, to);
  assert(to != nullptr);
  tvSet(*fr, *to);
  memcpy((void*)tv2, (void*)fr, sizeof(TypedValue));
  vmStack().discard();
  decRefStr(name);
}

OPTBLD_INLINE void ExecutionContext::iopSetS(IOP_ARGS) {
  NEXT();
  TypedValue* tv1 = vmStack().topTV();
  TypedValue* classref = vmStack().indTV(1);
  TypedValue* propn = vmStack().indTV(2);
  TypedValue* output = propn;
  StringData* name;
  TypedValue* val;
  bool visible, accessible;
  lookup_sprop(vmfp(), classref, name, propn, val, visible, accessible);
  if (!(visible && accessible)) {
    raise_error("Invalid static property access: %s::%s",
                classref->m_data.pcls->name()->data(),
                name->data());
  }
  tvSet(*tv1, *val);
  tvRefcountedDecRefCell(propn);
  memcpy(output, tv1, sizeof(TypedValue));
  vmStack().ndiscard(2);
  decRefStr(name);
}

OPTBLD_INLINE void ExecutionContext::iopSetM(IOP_ARGS) {
  NEXT();
  DECLARE_SETHELPER_ARGS
  if (!setHelperPre<false, true, false, false, 1,
      VectorLeaveCode::LeaveLast>(MEMBERHELPERPRE_ARGS)) {
    Cell* c1 = vmStack().topC();

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
        Class* ctx = arGetContextClass(vmfp());
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
    TypedValue* from = frame_local(vmfp(), local);
    tvAsVariant(base).setWithRef(tvAsVariant(from));
  }
  setHelperPost<0>(SETHELPERPOST_ARGS);
}

OPTBLD_INLINE void ExecutionContext::iopSetWithRefRM(IOP_ARGS) {
  NEXT();
  DECLARE_SETHELPER_ARGS
  bool skip = setHelperPre<false, true, false, false, 1,
                           VectorLeaveCode::ConsumeAll>(MEMBERHELPERPRE_ARGS);
  if (!skip) {
    TypedValue* from = vmStack().top();
    tvAsVariant(base).setWithRef(tvAsVariant(from));
  }
  setHelperPost<0>(SETHELPERPOST_ARGS);
  vmStack().popTV();
}

OPTBLD_INLINE void ExecutionContext::iopSetOpL(IOP_ARGS) {
  NEXT();
  DECODE_LA(local);
  DECODE_OA(SetOpOp, op);
  Cell* fr = vmStack().topC();
  Cell* to = tvToCell(frame_local(vmfp(), local));
  SETOP_BODY_CELL(to, op, fr);
  tvRefcountedDecRefCell(fr);
  cellDup(*to, *fr);
}

OPTBLD_INLINE void ExecutionContext::iopSetOpN(IOP_ARGS) {
  NEXT();
  DECODE_OA(SetOpOp, op);
  StringData* name;
  Cell* fr = vmStack().topC();
  TypedValue* tv2 = vmStack().indTV(1);
  TypedValue* to = nullptr;
  // XXX We're probably not getting warnings totally correct here
  lookupd_var(vmfp(), name, tv2, to);
  assert(to != nullptr);
  SETOP_BODY(to, op, fr);
  tvRefcountedDecRef(fr);
  tvRefcountedDecRef(tv2);
  cellDup(*tvToCell(to), *tv2);
  vmStack().discard();
  decRefStr(name);
}

OPTBLD_INLINE void ExecutionContext::iopSetOpG(IOP_ARGS) {
  NEXT();
  DECODE_OA(SetOpOp, op);
  StringData* name;
  Cell* fr = vmStack().topC();
  TypedValue* tv2 = vmStack().indTV(1);
  TypedValue* to = nullptr;
  // XXX We're probably not getting warnings totally correct here
  lookupd_gbl(vmfp(), name, tv2, to);
  assert(to != nullptr);
  SETOP_BODY(to, op, fr);
  tvRefcountedDecRef(fr);
  tvRefcountedDecRef(tv2);
  cellDup(*tvToCell(to), *tv2);
  vmStack().discard();
  decRefStr(name);
}

OPTBLD_INLINE void ExecutionContext::iopSetOpS(IOP_ARGS) {
  NEXT();
  DECODE_OA(SetOpOp, op);
  Cell* fr = vmStack().topC();
  TypedValue* classref = vmStack().indTV(1);
  TypedValue* propn = vmStack().indTV(2);
  TypedValue* output = propn;
  StringData* name;
  TypedValue* val;
  bool visible, accessible;
  lookup_sprop(vmfp(), classref, name, propn, val, visible, accessible);
  if (!(visible && accessible)) {
    raise_error("Invalid static property access: %s::%s",
                classref->m_data.pcls->name()->data(),
                name->data());
  }
  SETOP_BODY(val, op, fr);
  tvRefcountedDecRefCell(propn);
  tvRefcountedDecRef(fr);
  cellDup(*tvToCell(val), *output);
  vmStack().ndiscard(2);
  decRefStr(name);
}

OPTBLD_INLINE void ExecutionContext::iopSetOpM(IOP_ARGS) {
  NEXT();
  DECODE_OA(SetOpOp, op);
  DECLARE_SETHELPER_ARGS
  if (!setHelperPre<MoreWarnings, true, false, false, 1,
      VectorLeaveCode::LeaveLast>(MEMBERHELPERPRE_ARGS)) {
    TypedValue* result;
    Cell* rhs = vmStack().topC();

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
        Class *ctx = arGetContextClass(vmfp());
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
  TypedValue* to = vmStack().allocTV();
  tvWriteUninit(to);
  TypedValue* fr = frame_local(vmfp(), local);
  if (UNLIKELY(fr->m_type == KindOfUninit)) {
    raise_undefined_local(vmfp(), local);
    tvWriteNull(fr);
  } else {
    fr = tvToCell(fr);
  }
  IncDecBody<true>(op, fr, to);
}

OPTBLD_INLINE void ExecutionContext::iopIncDecN(IOP_ARGS) {
  NEXT();
  DECODE_OA(IncDecOp, op);
  StringData* name;
  TypedValue* nameCell = vmStack().topTV();
  TypedValue* local = nullptr;
  lookupd_var(vmfp(), name, nameCell, local);
  assert(local != nullptr);
  IncDecBody<true>(op, tvToCell(local), nameCell);
  decRefStr(name);
}

OPTBLD_INLINE void ExecutionContext::iopIncDecG(IOP_ARGS) {
  NEXT();
  DECODE_OA(IncDecOp, op);
  StringData* name;
  TypedValue* nameCell = vmStack().topTV();
  TypedValue* gbl = nullptr;
  lookupd_gbl(vmfp(), name, nameCell, gbl);
  assert(gbl != nullptr);
  IncDecBody<true>(op, tvToCell(gbl), nameCell);
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
  IncDecBody<true>(op, tvToCell(val), output);
  vmStack().discard();
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
        Class* ctx = arGetContextClass(vmfp());
        IncDecProp<true>(tvScratch, *tvRef.asTypedValue(), ctx, op, base,
                         *curMember, to);
        break;
      }
      default: assert(false);
      }
    }
  }
  setHelperPost<0>(SETHELPERPOST_ARGS);
  Cell* c1 = vmStack().allocC();
  memcpy(c1, &to, sizeof(TypedValue));
}

OPTBLD_INLINE void ExecutionContext::iopBindL(IOP_ARGS) {
  NEXT();
  DECODE_LA(local);
  Ref* fr = vmStack().topV();
  TypedValue* to = frame_local(vmfp(), local);
  tvBind(fr, to);
}

OPTBLD_INLINE void ExecutionContext::iopBindN(IOP_ARGS) {
  NEXT();
  StringData* name;
  TypedValue* fr = vmStack().topTV();
  TypedValue* nameTV = vmStack().indTV(1);
  TypedValue* to = nullptr;
  lookupd_var(vmfp(), name, nameTV, to);
  assert(to != nullptr);
  tvBind(fr, to);
  memcpy((void*)nameTV, (void*)fr, sizeof(TypedValue));
  vmStack().discard();
  decRefStr(name);
}

OPTBLD_INLINE void ExecutionContext::iopBindG(IOP_ARGS) {
  NEXT();
  StringData* name;
  TypedValue* fr = vmStack().topTV();
  TypedValue* nameTV = vmStack().indTV(1);
  TypedValue* to = nullptr;
  lookupd_gbl(vmfp(), name, nameTV, to);
  assert(to != nullptr);
  tvBind(fr, to);
  memcpy((void*)nameTV, (void*)fr, sizeof(TypedValue));
  vmStack().discard();
  decRefStr(name);
}

OPTBLD_INLINE void ExecutionContext::iopBindS(IOP_ARGS) {
  NEXT();
  TypedValue* fr = vmStack().topTV();
  TypedValue* classref = vmStack().indTV(1);
  TypedValue* propn = vmStack().indTV(2);
  TypedValue* output = propn;
  StringData* name;
  TypedValue* val;
  bool visible, accessible;
  lookup_sprop(vmfp(), classref, name, propn, val, visible, accessible);
  if (!(visible && accessible)) {
    raise_error("Invalid static property access: %s::%s",
                classref->m_data.pcls->name()->data(),
                name->data());
  }
  tvBind(fr, val);
  tvRefcountedDecRefCell(propn);
  memcpy(output, fr, sizeof(TypedValue));
  vmStack().ndiscard(2);
  decRefStr(name);
}

OPTBLD_INLINE void ExecutionContext::iopBindM(IOP_ARGS) {
  NEXT();
  DECLARE_SETHELPER_ARGS
  TypedValue* tv1 = vmStack().topTV();
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
  assert(local < vmfp()->m_func->numLocals());
  TypedValue* tv = frame_local(vmfp(), local);
  tvRefcountedDecRef(tv);
  tvWriteUninit(tv);
}

OPTBLD_INLINE void ExecutionContext::iopUnsetN(IOP_ARGS) {
  NEXT();
  StringData* name;
  TypedValue* tv1 = vmStack().topTV();
  TypedValue* tv = nullptr;
  lookup_var(vmfp(), name, tv1, tv);
  assert(!vmfp()->hasInvName());
  if (tv != nullptr) {
    tvRefcountedDecRef(tv);
    tvWriteUninit(tv);
  }
  vmStack().popC();
  decRefStr(name);
}

OPTBLD_INLINE void ExecutionContext::iopUnsetG(IOP_ARGS) {
  NEXT();
  TypedValue* tv1 = vmStack().topTV();
  StringData* name = lookup_name(tv1);
  VarEnv* varEnv = m_globalVarEnv;
  assert(varEnv != nullptr);
  varEnv->unset(name);
  vmStack().popC();
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
      Class* ctx = arGetContextClass(vmfp());
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
  ActRec* ar = vmStack().allocA();
  ar->m_func = func;
  ar->initNumArgs(numArgs);
  ar->setVarEnv(nullptr);
  return ar;
}

OPTBLD_INLINE void ExecutionContext::iopFPushFunc(IOP_ARGS) {
  NEXT();
  DECODE_IVA(numArgs);
  Cell* c1 = vmStack().topC();
  const Func* func = nullptr;

  // Throughout this function, we save obj/string/array and defer
  // refcounting them until after the stack has been discarded.

  if (IS_STRING_TYPE(c1->m_type)) {
    StringData* origSd = c1->m_data.pstr;
    func = Unit::loadFunc(origSd);
    if (func == nullptr) {
      raise_error("Call to undefined function %s()", c1->m_data.pstr->data());
    }

    vmStack().discard();
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

  if (c1->m_type == KindOfArray) {
    // support: array($instance, 'method') and array('Class', 'method')
    // which are both valid callables
    ArrayData* origArr = c1->m_data.parr;
    ObjectData* arrThis = nullptr;
    HPHP::Class* arrCls = nullptr;
    StringData* invName = nullptr;

    func = vm_decode_function(
      tvAsCVarRef(c1),
      vmfp(),
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

    vmStack().discard();
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

OPTBLD_INLINE void ExecutionContext::iopFPushFuncU(IOP_ARGS) {
  NEXT();
  DECODE_IVA(numArgs);
  DECODE(Id, nsFunc);
  DECODE(Id, globalFunc);
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

void ExecutionContext::fPushObjMethodImpl(
    Class* cls, StringData* name, ObjectData* obj, int numArgs) {
  const Func* f;
  LookupResult res = lookupObjMethod(f, cls, name,
                                     arGetContextClass(vmfp()), true);
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
    ar->setInvName(name);
  } else {
    ar->setVarEnv(NULL);
    decRefStr(name);
  }
}

static void throw_call_non_object(const char* methodName,
                                  const char* typeName = nullptr) {
  std::string msg;
  folly::format(&msg, "Call to a member function {}() on a non-object ({})",
    methodName, typeName);

  if (RuntimeOption::ThrowExceptionOnBadMethodCall) {
    Object e(SystemLib::AllocBadMethodCallExceptionObject(String(msg)));
    throw e;
  }
  throw FatalErrorException(msg.c_str());
}

OPTBLD_INLINE void ExecutionContext::iopFPushObjMethod(IOP_ARGS) {
  NEXT();
  DECODE_IVA(numArgs);
  Cell* c1 = vmStack().topC(); // Method name.
  if (!IS_STRING_TYPE(c1->m_type)) {
    raise_error(Strings::METHOD_NAME_MUST_BE_STRING);
  }
  Cell* c2 = vmStack().indC(1); // Object.
  if (c2->m_type != KindOfObject) {
    throw_call_non_object(c1->m_data.pstr->data(),
                          getDataTypeString(c2->m_type).get()->data());
  }
  ObjectData* obj = c2->m_data.pobj;
  Class* cls = obj->getVMClass();
  StringData* name = c1->m_data.pstr;
  // We handle decReffing obj and name in fPushObjMethodImpl
  vmStack().ndiscard(2);
  fPushObjMethodImpl(cls, name, obj, numArgs);
}

OPTBLD_INLINE void ExecutionContext::iopFPushObjMethodD(IOP_ARGS) {
  NEXT();
  DECODE_IVA(numArgs);
  DECODE_LITSTR(name);
  Cell* c1 = vmStack().topC();
  if (c1->m_type != KindOfObject) {
    throw_call_non_object(name->data(),
                          getDataTypeString(c1->m_type).get()->data());
  }
  ObjectData* obj = c1->m_data.pobj;
  Class* cls = obj->getVMClass();
  // We handle decReffing obj in fPushObjMethodImpl
  vmStack().discard();
  fPushObjMethodImpl(cls, name, obj, numArgs);
}

template<bool forwarding>
void ExecutionContext::pushClsMethodImpl(Class* cls,
                                           StringData* name,
                                           ObjectData* obj,
                                           int numArgs) {
  const Func* f;
  LookupResult res = lookupClsMethod(f, cls, name, obj,
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
    ar->setInvName(name);
  } else {
    ar->setVarEnv(nullptr);
    decRefStr(const_cast<StringData*>(name));
  }
}

OPTBLD_INLINE void ExecutionContext::iopFPushClsMethod(IOP_ARGS) {
  NEXT();
  DECODE_IVA(numArgs);
  Cell* c1 = vmStack().indC(1); // Method name.
  if (!IS_STRING_TYPE(c1->m_type)) {
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

OPTBLD_INLINE void ExecutionContext::iopFPushClsMethodD(IOP_ARGS) {
  NEXT();
  DECODE_IVA(numArgs);
  DECODE_LITSTR(name);
  DECODE(Id, classId);
  const NamedEntityPair &nep =
    vmfp()->m_func->unit()->lookupNamedEntityPairId(classId);
  Class* cls = Unit::loadClass(nep.second, nep.first);
  if (cls == nullptr) {
    raise_error(Strings::UNKNOWN_CLASS, nep.first->data());
  }
  ObjectData* obj = vmfp()->hasThis() ? vmfp()->getThis() : nullptr;
  pushClsMethodImpl<false>(cls, name, obj, numArgs);
}

OPTBLD_INLINE void ExecutionContext::iopFPushClsMethodF(IOP_ARGS) {
  NEXT();
  DECODE_IVA(numArgs);
  Cell* c1 = vmStack().indC(1); // Method name.
  if (!IS_STRING_TYPE(c1->m_type)) {
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

OPTBLD_INLINE void ExecutionContext::iopFPushCtor(IOP_ARGS) {
  NEXT();
  DECODE_IVA(numArgs);
  TypedValue* tv = vmStack().topTV();
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
  ActRec* ar = vmStack().allocA();
  ar->m_func = f;
  ar->setThis(this_);
  ar->initNumArgsFromFPushCtor(numArgs);
  ar->setVarEnv(nullptr);
}

OPTBLD_INLINE void ExecutionContext::iopFPushCtorD(IOP_ARGS) {
  NEXT();
  DECODE_IVA(numArgs);
  DECODE(Id, id);
  const NamedEntityPair &nep =
    vmfp()->m_func->unit()->lookupNamedEntityPairId(id);
  Class* cls = Unit::loadClass(nep.second, nep.first);
  if (cls == nullptr) {
    raise_error(Strings::UNKNOWN_CLASS,
                vmfp()->m_func->unit()->lookupLitstrId(id)->data());
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
  vmStack().pushObject(this_);
  // Push new activation record.
  ActRec* ar = vmStack().allocA();
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

  Iter* it = frame_iter(vmfp(), itId);
  CufIter &cit = it->cuf();

  ObjectData* obj = nullptr;
  HPHP::Class* cls = nullptr;
  StringData* invName = nullptr;
  TypedValue *func = vmStack().topTV();

  ActRec* ar = vmfp();
  if (vmfp()->m_func->isBuiltin()) {
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
  vmStack().popC();
}

OPTBLD_INLINE void ExecutionContext::iopFPushCufIter(IOP_ARGS) {
  NEXT();
  DECODE_IVA(numArgs);
  DECODE_IA(itId);

  Iter* it = frame_iter(vmfp(), itId);

  auto f = it->cuf().func();
  auto o = it->cuf().ctx();
  auto n = it->cuf().name();

  ActRec* ar = vmStack().allocA();
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
      vmStack().pushFalse();
    }
  } else if (safe) {
    vmStack().pushTrue();
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
  DEBUG_ONLY auto const ar = arFromInstr(vmStack().top(), (Op*)pc);
  NEXT();
  DECODE_IVA(paramId);
  assert(paramId < ar->numArgs());
}

OPTBLD_INLINE void ExecutionContext::iopFPassCW(IOP_ARGS) {
  auto const ar = arFromInstr(vmStack().top(), reinterpret_cast<const Op*>(pc));
  NEXT();
  DECODE_IVA(paramId);
  assert(paramId < ar->numArgs());
  auto const func = ar->m_func;
  if (func->mustBeRef(paramId)) {
    raise_strict_warning("Only variables should be passed by reference");
  }
}

OPTBLD_INLINE void ExecutionContext::iopFPassCE(IOP_ARGS) {
  auto const ar = arFromInstr(vmStack().top(), reinterpret_cast<const Op*>(pc));
  NEXT();
  DECODE_IVA(paramId);
  assert(paramId < ar->numArgs());
  auto const func = ar->m_func;
  if (func->mustBeRef(paramId)) {
    raise_error("Cannot pass parameter %d by reference", paramId+1);
  }
}

OPTBLD_INLINE void ExecutionContext::iopFPassV(IOP_ARGS) {
  ActRec* ar = arFromInstr(vmStack().top(), (Op*)pc);
  NEXT();
  DECODE_IVA(paramId);
  assert(paramId < ar->numArgs());
  const Func* func = ar->m_func;
  if (!func->byRef(paramId)) {
    vmStack().unbox();
  }
}

OPTBLD_INLINE void ExecutionContext::iopFPassVNop(IOP_ARGS) {
  DEBUG_ONLY auto const ar = arFromInstr(vmStack().top(), (Op*)pc);
  NEXT();
  DECODE_IVA(paramId);
  assert(paramId < ar->numArgs());
  assert(ar->m_func->byRef(paramId));
}

OPTBLD_INLINE void ExecutionContext::iopFPassR(IOP_ARGS) {
  ActRec* ar = arFromInstr(vmStack().top(), (Op*)pc);
  NEXT();
  DECODE_IVA(paramId);
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

OPTBLD_INLINE void ExecutionContext::iopFPassL(IOP_ARGS) {
  ActRec* ar = arFromInstr(vmStack().top(), (Op*)pc);
  NEXT();
  DECODE_IVA(paramId);
  DECODE_LA(local);
  assert(paramId < ar->numArgs());
  TypedValue* fr = frame_local(vmfp(), local);
  TypedValue* to = vmStack().allocTV();
  if (!ar->m_func->byRef(paramId)) {
    cgetl_body(vmfp(), fr, to, local);
  } else {
    vgetl_body(fr, to);
  }
}

OPTBLD_INLINE void ExecutionContext::iopFPassN(IOP_ARGS) {
  ActRec* ar = arFromInstr(vmStack().top(), (Op*)pc);
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
  ActRec* ar = arFromInstr(vmStack().top(), (Op*)pc);
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
  ActRec* ar = arFromInstr(vmStack().top(), (Op*)pc);
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
  ActRec* ar = arFromInstr(vmStack().top(), (Op*)pc);
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
    TypedValue* tv1 = vmStack().allocTV();
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
  TRACE(3, "FCall: pc %p func %p base %d\n", vmpc(),
        vmfp()->m_func->unit()->entry(),
        int(vmfp()->m_func->base()));
  prepareFuncEntry(ar, pc, StackArgsState::Untrimmed);
  SYNC();
  if (EventHook::FunctionCall(ar, EventHook::NormalFunc)) return true;
  pc = vmpc();
  return false;
}

OPTBLD_INLINE void ExecutionContext::iopFCall(IOP_ARGS) {
  ActRec* ar = arFromInstr(vmStack().top(), (Op*)pc);
  NEXT();
  DECODE_IVA(numArgs);
  assert(numArgs == ar->numArgs());
  checkStack(vmStack(), ar->m_func, 0);
  ar->setReturn(vmfp(), pc, mcg->tx().uniqueStubs.retHelper);
  doFCall(ar, pc);
  if (RuntimeOption::EvalRuntimeTypeProfile) {
    profileAllArguments(ar);
  }
}

OPTBLD_INLINE void ExecutionContext::iopFCallD(IOP_ARGS) {
  auto const ar = arFromInstr(vmStack().top(), reinterpret_cast<const Op*>(pc));
  NEXT();
  DECODE_IVA(numArgs);
  DECODE_LITSTR(clsName);
  DECODE_LITSTR(funcName);
  (void) clsName;
  (void) funcName;
  if (!RuntimeOption::EvalJitEnableRenameFunction &&
      !(ar->m_func->attrs() & AttrInterceptable)) {
    assert(ar->m_func->name()->isame(funcName));
  }
  assert(numArgs == ar->numArgs());
  checkStack(vmStack(), ar->m_func, 0);
  ar->setReturn(vmfp(), pc, mcg->tx().uniqueStubs.retHelper);
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
  const NamedEntity* ne = vmfp()->m_func->unit()->lookupNamedEntityId(id);
  Func* func = Unit::lookupFunc(ne);
  if (func == nullptr) {
    raise_error("Call to undefined function %s()",
                vmfp()->m_func->unit()->lookupLitstrId(id)->data());
  }
  TypedValue* args = vmStack().indTV(numArgs-1);
  TypedValue ret;
  if (Native::coerceFCallArgs(args, numArgs, numNonDefault, func)) {
    Native::callFunc(func, nullptr, args, numArgs, ret);
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

bool ExecutionContext::doFCallArray(PC& pc, int numStackValues,
                                    CallArrOnInvalidContainer onInvalid) {
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
      case CallArrOnInvalidContainer::WarnAndContinue:
        tvRefcountedDecRef(c1);
        // argument_unpacking RFC dictates "containers and Traversables"
        raise_debugging("Only containers may be unpacked");
        c1->m_type = KindOfArray;
        c1->m_data.parr = staticEmptyArray();
        break;
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
    ar->setReturn(vmfp(), pc, mcg->tx().uniqueStubs.retHelper);

    if (UNLIKELY((CallArrOnInvalidContainer::WarnAndContinue == onInvalid)
                 && func->anyByRef())) {
      raise_error("Unpacking unsupported for calls to functions that"
                  " take any arguments by reference");
      vmStack().pushNull();
      return false;
    }

    auto prepResult = prepareArrayArgs(ar, args, vmStack(), numStackValues,
                                       /* ref param checks */ true, nullptr);
    if (UNLIKELY(!prepResult)) {
      vmStack().pushNull(); // return value is null if args are invalid
      return false;
    }
  }

  prepareFuncEntry(ar, pc, StackArgsState::Trimmed);
  SYNC();
  if (UNLIKELY(!EventHook::FunctionCall(ar, EventHook::NormalFunc))) {
    pc = vmpc();
    return false;
  }
  return true;
}

bool ExecutionContext::doFCallArrayTC(PC pc) {
  assert_native_stack_aligned();
  assert(tl_regState == VMRegState::DIRTY);
  tl_regState = VMRegState::CLEAN;
  auto const ret = doFCallArray(pc, 1, CallArrOnInvalidContainer::CastToArray);
  tl_regState = VMRegState::DIRTY;
  return ret;
}

OPTBLD_INLINE void ExecutionContext::iopFCallArray(IOP_ARGS) {
  NEXT();
  (void)doFCallArray(pc, 1, CallArrOnInvalidContainer::CastToArray);
}

OPTBLD_INLINE void ExecutionContext::iopFCallUnpack(IOP_ARGS) {
  ActRec* ar = arFromInstr(vmStack().top(), (Op*)pc);
  NEXT();
  DECODE_IVA(numArgs);
  assert(numArgs == ar->numArgs());
  checkStack(vmStack(), ar->m_func, 0);
  (void) doFCallArray(pc, numArgs,
                      CallArrOnInvalidContainer::WarnAndContinue);
}

OPTBLD_INLINE void ExecutionContext::iopCufSafeArray(IOP_ARGS) {
  NEXT();
  Array ret;
  ret.append(tvAsVariant(vmStack().top() + 1));
  ret.appendWithRef(tvAsVariant(vmStack().top() + 0));
  vmStack().popTV();
  vmStack().popTV();
  tvAsVariant(vmStack().top()) = ret;
}

OPTBLD_INLINE void ExecutionContext::iopCufSafeReturn(IOP_ARGS) {
  NEXT();
  bool ok = cellToBool(*tvToCell(vmStack().top() + 1));
  tvRefcountedDecRef(vmStack().top() + 1);
  tvRefcountedDecRef(vmStack().top() + (ok ? 2 : 0));
  if (ok) vmStack().top()[2] = vmStack().top()[0];
  vmStack().ndiscard(2);
}

inline bool ExecutionContext::initIterator(PC& pc, PC& origPc, Iter* it,
                                             Offset offset, Cell* c1) {
  bool hasElems = it->init(c1);
  if (!hasElems) {
    ITER_SKIP(offset);
  }
  vmStack().popC();
  return hasElems;
}

OPTBLD_INLINE void ExecutionContext::iopIterInit(IOP_ARGS) {
  PC origPc = pc;
  NEXT();
  DECODE_IA(itId);
  DECODE(Offset, offset);
  DECODE_LA(val);
  Cell* c1 = vmStack().topC();
  Iter* it = frame_iter(vmfp(), itId);
  TypedValue* tv1 = frame_local(vmfp(), val);
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
  Cell* c1 = vmStack().topC();
  Iter* it = frame_iter(vmfp(), itId);
  TypedValue* tv1 = frame_local(vmfp(), val);
  TypedValue* tv2 = frame_local(vmfp(), key);
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
  Cell* c1 = vmStack().topC();
  Iter* it = frame_iter(vmfp(), itId);
  TypedValue* tv1 = frame_local(vmfp(), val);
  if (initIterator(pc, origPc, it, offset, c1)) {
    tvAsVariant(tv1).setWithRef(it->arr().secondRef());
  }
}

OPTBLD_INLINE void ExecutionContext::iopWIterInitK(IOP_ARGS) {
  PC origPc = pc;
  NEXT();
  DECODE_IA(itId);
  DECODE(Offset, offset);
  DECODE_LA(val);
  DECODE_LA(key);
  Cell* c1 = vmStack().topC();
  Iter* it = frame_iter(vmfp(), itId);
  TypedValue* tv1 = frame_local(vmfp(), val);
  TypedValue* tv2 = frame_local(vmfp(), key);
  if (initIterator(pc, origPc, it, offset, c1)) {
    tvAsVariant(tv1).setWithRef(it->arr().secondRef());
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
    Class* ctx = arGetContextClass(vmfp());
    hasElems = new_miter_object(it, r1->m_data.pref, ctx, val, key);
  } else {
    hasElems = new_miter_other(it, r1->m_data.pref);
  }

  if (!hasElems) {
    ITER_SKIP(offset);
  }

  vmStack().popV();
  return hasElems;
}

OPTBLD_INLINE void ExecutionContext::iopMIterInit(IOP_ARGS) {
  PC origPc = pc;
  NEXT();
  DECODE_IA(itId);
  DECODE(Offset, offset);
  DECODE_LA(val);
  Ref* r1 = vmStack().topV();
  assert(r1->m_type == KindOfRef);
  Iter* it = frame_iter(vmfp(), itId);
  TypedValue* tv1 = frame_local(vmfp(), val);
  initIteratorM(pc, origPc, it, offset, r1, tv1, nullptr);
}

OPTBLD_INLINE void ExecutionContext::iopMIterInitK(IOP_ARGS) {
  PC origPc = pc;
  NEXT();
  DECODE_IA(itId);
  DECODE(Offset, offset);
  DECODE_LA(val);
  DECODE_LA(key);
  Ref* r1 = vmStack().topV();
  assert(r1->m_type == KindOfRef);
  Iter* it = frame_iter(vmfp(), itId);
  TypedValue* tv1 = frame_local(vmfp(), val);
  TypedValue* tv2 = frame_local(vmfp(), key);
  initIteratorM(pc, origPc, it, offset, r1, tv1, tv2);
}

OPTBLD_INLINE void ExecutionContext::iopIterNext(IOP_ARGS) {
  PC origPc = pc;
  NEXT();
  DECODE_IA(itId);
  DECODE(Offset, offset);
  DECODE_LA(val);
  Iter* it = frame_iter(vmfp(), itId);
  TypedValue* tv1 = frame_local(vmfp(), val);
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
  Iter* it = frame_iter(vmfp(), itId);
  TypedValue* tv1 = frame_local(vmfp(), val);
  TypedValue* tv2 = frame_local(vmfp(), key);
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
  Iter* it = frame_iter(vmfp(), itId);
  TypedValue* tv1 = frame_local(vmfp(), val);
  if (it->next()) {
    ITER_SKIP(offset);
    tvAsVariant(tv1).setWithRef(it->arr().secondRef());
  }
}

OPTBLD_INLINE void ExecutionContext::iopWIterNextK(IOP_ARGS) {
  PC origPc = pc;
  NEXT();
  DECODE_IA(itId);
  DECODE(Offset, offset);
  DECODE_LA(val);
  DECODE_LA(key);
  Iter* it = frame_iter(vmfp(), itId);
  TypedValue* tv1 = frame_local(vmfp(), val);
  TypedValue* tv2 = frame_local(vmfp(), key);
  if (it->next()) {
    ITER_SKIP(offset);
    tvAsVariant(tv1).setWithRef(it->arr().secondRef());
    tvAsVariant(tv2) = it->arr().first();
  }
}

OPTBLD_INLINE void ExecutionContext::iopMIterNext(IOP_ARGS) {
  PC origPc = pc;
  NEXT();
  DECODE_IA(itId);
  DECODE(Offset, offset);
  DECODE_LA(val);
  Iter* it = frame_iter(vmfp(), itId);
  TypedValue* tv1 = frame_local(vmfp(), val);
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
  Iter* it = frame_iter(vmfp(), itId);
  TypedValue* tv1 = frame_local(vmfp(), val);
  TypedValue* tv2 = frame_local(vmfp(), key);
  if (miter_next_key(it, tv1, tv2)) {
    ITER_SKIP(offset);
  }
}

OPTBLD_INLINE void ExecutionContext::iopIterFree(IOP_ARGS) {
  NEXT();
  DECODE_IA(itId);
  Iter* it = frame_iter(vmfp(), itId);
  it->free();
}

OPTBLD_INLINE void ExecutionContext::iopMIterFree(IOP_ARGS) {
  NEXT();
  DECODE_IA(itId);
  Iter* it = frame_iter(vmfp(), itId);
  it->mfree();
}

OPTBLD_INLINE void ExecutionContext::iopCIterFree(IOP_ARGS) {
  NEXT();
  DECODE_IA(itId);
  Iter* it = frame_iter(vmfp(), itId);
  it->cfree();
}

OPTBLD_INLINE void inclOp(ExecutionContext *ec, IOP_ARGS, InclOpFlags flags) {
  NEXT();
  Cell* c1 = vmStack().topC();
  String path(prepareKey(*c1));
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
      raise_error("File not found: %s", path.data());
    } else {
      raise_warning("File not found: %s", path.data());
    }
    vmStack().pushFalse();
    return;
  }

  if (!(flags & InclOpFlags::Once) || initial) {
    ec->evalUnit(unit, pc, EventHook::PseudoMain);
  } else {
    Stats::inc(Stats::PseudoMain_Guarded);
    vmStack().pushTrue();
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
  Cell* c1 = vmStack().topC();

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
    "%s(%d" EVAL_FILENAME_SUFFIX,
    getContainingFileName()->data(),
    getLine()
  );
  Unit* unit = compileEvalString(prefixedCode.get(), evalFilename.c_str());

  const StringData* msg;
  int line = 0;

  vmStack().popC();
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

    vmStack().pushFalse();
    return;
  }
  evalUnit(unit, pc, EventHook::Eval);
}

OPTBLD_INLINE void ExecutionContext::iopDefFunc(IOP_ARGS) {
  NEXT();
  DECODE_IVA(fid);
  Func* f = vmfp()->m_func->unit()->lookupFuncId(fid);
  setCachedFunc(f, isDebuggerAttached());
}

OPTBLD_INLINE void ExecutionContext::iopDefCls(IOP_ARGS) {
  NEXT();
  DECODE_IVA(cid);
  PreClass* c = vmfp()->m_func->unit()->lookupPreClassId(cid);
  Unit::defClass(c);
}

OPTBLD_INLINE void ExecutionContext::iopNopDefCls(IOP_ARGS) {
  NEXT();
  DECODE_IVA(cid);
}

OPTBLD_INLINE void ExecutionContext::iopDefTypeAlias(IOP_ARGS) {
  NEXT();
  DECODE_IVA(tid);
  vmfp()->m_func->unit()->defTypeAlias(tid);
}

static inline void checkThis(ActRec* fp) {
  if (!fp->hasThis()) {
    raise_error(Strings::FATAL_NULL_THIS);
  }
}

OPTBLD_INLINE void ExecutionContext::iopThis(IOP_ARGS) {
  NEXT();
  checkThis(vmfp());
  ObjectData* this_ = vmfp()->getThis();
  vmStack().pushObject(this_);
}

OPTBLD_INLINE void ExecutionContext::iopBareThis(IOP_ARGS) {
  NEXT();
  DECODE_OA(BareThisOp, bto);
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

OPTBLD_INLINE void ExecutionContext::iopCheckThis(IOP_ARGS) {
  NEXT();
  checkThis(vmfp());
}

OPTBLD_INLINE void ExecutionContext::iopInitThisLoc(IOP_ARGS) {
  NEXT();
  DECODE_LA(id);
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

static inline RefData* lookupStatic(StringData* name,
                                    const ActRec* fp,
                                    bool& inited) {
  auto const func = fp->m_func;

  if (UNLIKELY(func->isClosureBody())) {
    assert(!func->hasVariadicCaptureParam());
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
  DECODE_LA(localId);
  DECODE_LITSTR(var);

  bool inited;
  auto const refData = lookupStatic(var, vmfp(), inited);
  if (!inited) {
    refData->tv()->m_type = KindOfNull;
  }

  auto const tvLocal = frame_local(vmfp(), localId);
  auto const tmpTV = make_tv<KindOfRef>(refData);
  tvBind(&tmpTV, tvLocal);
  if (inited) {
    vmStack().pushTrue();
  } else {
    vmStack().pushFalse();
  }
}

OPTBLD_INLINE void ExecutionContext::iopStaticLocInit(IOP_ARGS) {
  NEXT();
  DECODE_LA(localId);
  DECODE_LITSTR(var);

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

OPTBLD_INLINE void ExecutionContext::iopCatch(IOP_ARGS) {
  NEXT();
  assert(m_faults.size() > 0);
  Fault fault = m_faults.back();
  m_faults.pop_back();
  assert(fault.m_raiseFrame == vmfp());
  assert(fault.m_faultType == Fault::Type::UserException);
  vmStack().pushObjectNoRc(fault.m_userException);
}

OPTBLD_INLINE void ExecutionContext::iopLateBoundCls(IOP_ARGS) {
  NEXT();
  Class* cls = frameStaticClass(vmfp());
  if (!cls) {
    raise_error(HPHP::Strings::CANT_ACCESS_STATIC);
  }
  vmStack().pushClass(cls);
}

OPTBLD_INLINE void ExecutionContext::iopVerifyParamType(IOP_ARGS) {
  SYNC(); // We might need vmpc() to be updated to throw.
  NEXT();

  DECODE_LA(paramId);
  const Func *func = vmfp()->m_func;
  assert(paramId < func->numParams());
  assert(func->numParams() == int(func->params().size()));
  const TypeConstraint& tc = func->params()[paramId].typeConstraint;
  assert(tc.hasConstraint());
  if (!tc.isTypeVar()) {
    tc.verifyParam(frame_local(vmfp(), paramId), func, paramId);
  }
}

OPTBLD_INLINE void ExecutionContext::implVerifyRetType(IOP_ARGS) {
  if (LIKELY(!RuntimeOption::EvalCheckReturnTypeHints)) {
    NEXT();
    return;
  }
  SYNC();
  NEXT();
  const auto func = vmfp()->m_func;
  const auto tc = func->returnTypeConstraint();
  if (!tc.isTypeVar()) {
    tc.verifyReturn(vmStack().topTV(), func);
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
}

OPTBLD_INLINE void ExecutionContext::iopHighInvalid(IOP_ARGS) {
  fprintf(stderr, "invalid bytecode executed\n");
  abort();
}

OPTBLD_INLINE void ExecutionContext::iopSelf(IOP_ARGS) {
  NEXT();
  Class* clss = arGetContextClass(vmfp());
  if (!clss) {
    raise_error(HPHP::Strings::CANT_ACCESS_SELF);
  }
  vmStack().pushClass(clss);
}

OPTBLD_INLINE void ExecutionContext::iopParent(IOP_ARGS) {
  NEXT();
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

OPTBLD_INLINE void ExecutionContext::iopCreateCl(IOP_ARGS) {
  NEXT();
  DECODE_IVA(numArgs);
  DECODE_LITSTR(clsName);
  auto const cls = Unit::loadClass(clsName);
  auto const cl = static_cast<c_Closure*>(newInstance(cls));
  cl->init(numArgs, vmfp(), vmStack().top());
  vmStack().ndiscard(numArgs);
  vmStack().pushObject(cl);
}

const StaticString s_this("this");

OPTBLD_INLINE void ExecutionContext::iopCreateCont(IOP_ARGS) {
  NEXT();
  auto const fp = vmfp();
  auto const func = fp->func();
  auto const numSlots = func->numSlotsInFrame();
  auto const resumeOffset = func->unit()->offsetOf(pc);
  assert(!fp->resumed());
  assert(func->isGenerator());

  // Create the {Async,}Generator object. Create takes care of copying local
  // variables and iterators.
  auto const gen = func->isAsync()
    ? static_cast<BaseGenerator*>(
        c_AsyncGenerator::Create(fp, numSlots, nullptr, resumeOffset))
    : static_cast<BaseGenerator*>(
        c_Generator::Create<false>(fp, numSlots, nullptr, resumeOffset));

  // Call the FunctionSuspend hook. Keep the generator on the stack so that
  // the unwinder could free it if the hook fails.
  vmStack().pushObjectNoRc(gen);
  EventHook::FunctionSuspend(gen->actRec(), false);
  vmStack().discard();

  // Grab caller info from ActRec.
  ActRec* sfp = fp->sfp();
  Offset soff = fp->m_soff;

  // Free ActRec and store the return value.
  vmStack().ndiscard(numSlots);
  vmStack().ret();
  tvCopy(make_tv<KindOfObject>(gen), *vmStack().topTV());
  assert(vmStack().topTV() == &fp->m_r);

  // Return control to the caller.
  vmfp() = sfp;
  pc = LIKELY(sfp != nullptr) ? sfp->func()->getEntry() + soff : nullptr;
}

static inline BaseGenerator* this_base_generator(const ActRec* fp) {
  auto const obj = fp->getThis();
  assert(obj->instanceof(c_AsyncGenerator::classof()) ||
         obj->instanceof(c_Generator::classof()));
  return static_cast<BaseGenerator*>(obj);
}

static inline c_Generator* this_generator(const ActRec* fp) {
  auto const obj = this_base_generator(fp);
  assert(obj->getVMClass() == c_Generator::classof());
  return static_cast<c_Generator*>(obj);
}

OPTBLD_INLINE void ExecutionContext::contEnterImpl(IOP_ARGS) {
  NEXT();

  // The stack must have one cell! Or else resumableStackBase() won't work!
  assert(vmStack().top() + 1 ==
         (TypedValue*)vmfp() - vmfp()->m_func->numSlotsInFrame());

  // Do linkage of the generator's AR.
  assert(vmfp()->hasThis());
  BaseGenerator* gen = this_base_generator(vmfp());
  assert(gen->getState() == BaseGenerator::State::Running);
  ActRec* genAR = gen->actRec();
  genAR->setReturn(vmfp(), pc, mcg->tx().uniqueStubs.genRetHelper);

  vmfp() = genAR;

  assert(genAR->func()->contains(gen->resumable()->resumeOffset()));
  pc = genAR->func()->unit()->at(gen->resumable()->resumeOffset());
  SYNC();
  EventHook::FunctionResume(vmfp());
}

OPTBLD_INLINE void ExecutionContext::iopContEnter(IOP_ARGS) {
  contEnterImpl(IOP_PASS_ARGS);
}

OPTBLD_INLINE void ExecutionContext::iopContRaise(IOP_ARGS) {
  contEnterImpl(IOP_PASS_ARGS);
  iopThrow(IOP_PASS_ARGS);
}

OPTBLD_INLINE void ExecutionContext::yield(IOP_ARGS,
                                           const Cell* key,
                                           const Cell& value) {
  auto const fp = vmfp();
  auto const func = fp->func();
  auto const resumeOffset = func->unit()->offsetOf(pc);
  assert(fp->resumed());
  assert(func->isGenerator());

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

  EventHook::FunctionSuspend(fp, true);

  // Grab caller info from ActRec.
  ActRec* sfp = fp->sfp();
  Offset soff = fp->m_soff;

  // Return control to the next()/send()/raise() caller.
  vmfp() = sfp;
  pc = sfp != nullptr ? sfp->func()->getEntry() + soff : nullptr;
}

OPTBLD_INLINE void ExecutionContext::iopYield(IOP_ARGS) {
  NEXT();
  auto const value = *vmStack().topC();
  vmStack().discard();

  yield(IOP_PASS_ARGS, nullptr, value);
}

OPTBLD_INLINE void ExecutionContext::iopYieldK(IOP_ARGS) {
  NEXT();
  auto const key = *vmStack().indC(1);
  auto const value = *vmStack().topC();
  vmStack().ndiscard(2);

  yield(IOP_PASS_ARGS, &key, value);
}

OPTBLD_INLINE void ExecutionContext::iopContCheck(IOP_ARGS) {
  NEXT();
  DECODE_IVA(checkStarted);
  this_base_generator(vmfp())->preNext(checkStarted);
}

OPTBLD_INLINE void ExecutionContext::iopContValid(IOP_ARGS) {
  NEXT();
  vmStack().pushBool(
    this_generator(vmfp())->getState() != BaseGenerator::State::Done);
}

OPTBLD_INLINE void ExecutionContext::iopContKey(IOP_ARGS) {
  NEXT();
  c_Generator* cont = this_generator(vmfp());
  cont->startedCheck();
  cellDup(cont->m_key, *vmStack().allocC());
}

OPTBLD_INLINE void ExecutionContext::iopContCurrent(IOP_ARGS) {
  NEXT();
  c_Generator* cont = this_generator(vmfp());
  cont->startedCheck();
  cellDup(cont->m_value, *vmStack().allocC());
}

OPTBLD_INLINE void ExecutionContext::asyncSuspendE(IOP_ARGS, int32_t iters) {
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
    c_AsyncFunctionWaitHandle::Create(vmfp(), vmfp()->func()->numSlotsInFrame(),
                                      nullptr, resumeOffset, child));

  // Call the FunctionSuspend hook. Keep the AsyncFunctionWaitHandle
  // on the stack so that the unwinder could free it if the hook fails.
  vmStack().pushObjectNoRc(waitHandle);
  EventHook::FunctionSuspend(waitHandle->actRec(), false);
  vmStack().discard();

  // Grab caller info from ActRec.
  ActRec* sfp = vmfp()->sfp();
  Offset soff = vmfp()->m_soff;

  // Free ActRec and store the return value.
  vmStack().ndiscard(vmfp()->m_func->numSlotsInFrame());
  vmStack().ret();
  tvCopy(make_tv<KindOfObject>(waitHandle), *vmStack().topTV());
  assert(vmStack().topTV() == &vmfp()->m_r);

  // Return control to the caller.
  vmfp() = sfp;
  pc = LIKELY(vmfp() != nullptr) ? vmfp()->func()->getEntry() + soff : nullptr;
}

OPTBLD_INLINE void ExecutionContext::asyncSuspendR(IOP_ARGS) {
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

  // Call the FunctionSuspend hook.
  EventHook::FunctionSuspend(fp, true);

  // Grab caller info from ActRec.
  ActRec* sfp = fp->sfp();
  Offset soff = fp->m_soff;

  // Return control to the caller or scheduler.
  vmfp() = sfp;
  pc = sfp != nullptr ? sfp->func()->getEntry() + soff : nullptr;
}

OPTBLD_INLINE void ExecutionContext::iopAwait(IOP_ARGS) {
  NEXT();
  DECODE_IVA(iters);

  auto const wh = c_WaitHandle::fromCell(vmStack().topC());
  if (UNLIKELY(wh == nullptr)) {
    raise_error("Await on a non-WaitHandle");
    not_reached();
  } else if (wh->isSucceeded()) {
    cellSet(wh->getResult(), *vmStack().topC());
    return;
  } else if (UNLIKELY(wh->isFailed())) {
    throw Object(wh->getException());
  }

  if (vmfp()->resumed()) {
    // suspend resumed execution
    asyncSuspendR(IOP_PASS_ARGS);
  } else {
    // suspend eager execution
    asyncSuspendE(IOP_PASS_ARGS, iters);
  }
}

template<class Op>
OPTBLD_INLINE void ExecutionContext::roundOpImpl(Op op) {
  TypedValue* val = vmStack().topTV();

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

  auto* cls = vmfp()->getClass();
  auto* propVec = cls->getPropData();
  always_assert(propVec);

  auto* ctx = arGetContextClass(vmfp());
  auto idx = ctx->lookupDeclProp(propName);

  auto& tv = (*propVec)[idx];
  if (tv.m_type != KindOfUninit) {
    vmStack().pushTrue();
  } else {
    vmStack().pushFalse();
  }
}

OPTBLD_INLINE void ExecutionContext::iopInitProp(IOP_ARGS) {
  NEXT();
  DECODE_LITSTR(propName);
  DECODE_OA(InitPropOp, propOp);

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

OPTBLD_INLINE void ExecutionContext::iopStrlen(IOP_ARGS) {
  NEXT();
  TypedValue* subj = vmStack().topTV();
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

OPTBLD_INLINE void ExecutionContext::iopOODeclExists(IOP_ARGS) {
  NEXT();
  DECODE_OA(OODeclExistsOp, subop);

  TypedValue* aloadTV = vmStack().topTV();
  tvCastToBooleanInPlace(aloadTV);
  assert(aloadTV->m_type == KindOfBoolean);
  bool autoload = aloadTV->m_data.num;
  vmStack().popX();

  TypedValue* name = vmStack().topTV();
  tvCastToStringInPlace(name);
  assert(IS_STRING_TYPE(name->m_type));

  ClassKind kind;
  switch (subop) {
    case OODeclExistsOp::Class : kind = ClassKind::Class; break;
    case OODeclExistsOp::Trait : kind = ClassKind::Trait; break;
    case OODeclExistsOp::Interface : kind = ClassKind::Interface; break;
  }
  tvAsVariant(name) = Unit::classExists(name->m_data.pstr, autoload, kind);
}

OPTBLD_INLINE void ExecutionContext::iopSilence(IOP_ARGS) {
  NEXT();
  DECODE_LA(localId);
  DECODE_OA(SilenceOp, subop);

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

string
ExecutionContext::prettyStack(const string& prefix) const {
  if (!vmfp()) {
    string s("__Halted");
    return s;
  }
  int offset = (vmfp()->m_func->unit() != nullptr)
               ? pcOff() : 0;
  string begPrefix = prefix + "__";
  string midPrefix = prefix + "|| ";
  string endPrefix = prefix + "\\/";
  string stack = vmStack().toString(vmfp(), offset, midPrefix);
  return begPrefix + "\n" + stack + endPrefix;
}

void ExecutionContext::DumpStack() {
  string s = g_context->prettyStack("");
  fprintf(stderr, "%s\n", s.c_str());
}

void ExecutionContext::DumpCurUnit(int skip) {
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

void ExecutionContext::PrintTCCallerInfo() {
  VMRegAnchor _;
  ActRec* fp = vmfp();
  Unit* u = fp->m_func->unit();
  fprintf(stderr, "Called from TC address %p\n",
          mcg->getTranslatedCaller());
  std::cerr << u->filepath()->data() << ':'
            << u->getLineNumber(u->offsetOf(vmpc())) << '\n';
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

#define O(name, imm, push, pop, flags)                      \
void ExecutionContext::op##name() {                         \
  condStackTraceSep("op"#name" ");                          \
  COND_STACKTRACE("op"#name" pre:  ");                      \
  PC pc = vmpc();                                             \
  assert(*reinterpret_cast<const Op*>(pc) == Op##name);     \
  ONTRACE(1,                                                \
          auto offset = vmfp()->m_func->unit()->offsetOf(pc); \
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

template <bool breakOnCtlFlow>
inline void ExecutionContext::dispatchImpl() {
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
    if (breakOnCtlFlow && isCtlFlow) {                                  \
      ONTRACE(1,                                                        \
              Trace::trace("dispatch: Halt ExecutionContext::dispatch(%p)\n", \
                           vmfp()));                                      \
      return;                                                           \
    }                                                                   \
    Op op = *reinterpret_cast<const Op*>(pc);                           \
    COND_STACKTRACE("dispatch:                    ");                   \
    ONTRACE(1,                                                          \
            Trace::trace("dispatch: %d: %s\n", pcOff(),                 \
                         nametab[uint8_t(op)]));                        \
    goto *optab[uint8_t(op)];                                           \
} while (0)

  ONTRACE(1, Trace::trace("dispatch: Enter ExecutionContext::dispatch(%p)\n",
          vmfp()));
  PC pc = vmpc();
  DISPATCH();

#define O(name, imm, push, pop, flags)                        \
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
             op == OpAwait || op == OpCreateCont ||           \
             op == OpYield || op == OpYieldK ||               \
             op == OpNativeImpl);                             \
      vmfp() = 0;                                               \
      return;                                                 \
    }                                                         \
    DISPATCH();                                               \
  }
  OPCODES
#undef O
#undef DISPATCH
}

void ExecutionContext::dispatch() {
  dispatchImpl<false>();
}

// We are about to go back to translated code, check whether we should
// stick with the interpreter. NB: if we've just executed a return
// from pseudomain, then there's no PC and no more code to interpret.
void ExecutionContext::switchModeForDebugger() {
  if (DEBUGGER_FORCE_INTR && (vmpc() != 0)) {
    throw VMSwitchMode();
  }
}

void ExecutionContext::dispatchBB() {
  if (Trace::moduleEnabled(Trace::dispatchBB)) {
    auto cat = makeStaticString("dispatchBB");
    auto name = makeStaticString(show(SrcKey(vmfp()->func(), vmpc(),
                                             vmfp()->resumed())));
    Stats::incStatGrouped(cat, name, 1);
  }

  dispatchImpl<true>();
  switchModeForDebugger();
}

void ExecutionContext::recordCodeCoverage(PC pc) {
  Unit* unit = vmfp()->m_func->unit();
  assert(unit != nullptr);
  if (unit == SystemLib::s_nativeFuncUnit ||
      unit == SystemLib::s_nativeClassUnit ||
      unit == SystemLib::s_hhas_unit) {
    return;
  }
  int line = unit->getLineNumber(pcOff());
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
  if (UNLIKELY(!vmfp())) {
    // first entry
    assert(m_nestedVMs.size() == 0);
    return;
  }

  VMState savedVM = { vmpc(), vmfp(), vmFirstAR(), savedSP };
  TRACE(3, "savedVM: %p %p %p %p\n", vmpc(), vmfp(), vmFirstAR(), savedSP);

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
    vmfp() = nullptr;
    vmpc() = nullptr;
    vmFirstAR() = nullptr;
    return;
  }

  assert(m_nestedVMs.size() >= 1);

  VMState &savedVM = m_nestedVMs.back();
  vmpc() = savedVM.pc;
  vmfp() = savedVM.fp;
  vmFirstAR() = savedVM.firstAR;
  vmStack().top() = savedVM.sp;

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

  TRACE(1, "Reentry: exit fp %p pc %p\n", vmfp(), vmpc());
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
  vmStack().requestInit();
  ObjectData::resetMaxId();
  ResourceData::resetMaxId();
  mcg->requestInit();

  if (RuntimeOption::EvalJitEnableRenameFunction) {
    assert(SystemLib::s_anyNonPersistentBuiltins);
  }

  /*
   * The normal case for production mode is that all builtins are
   * persistent, and every systemlib unit is accordingly going to be
   * merge only.
   *
   * However, if we have rename_function generally enabled, or if any
   * builtin functions were specified as interceptable at
   * repo-generation time, we'll actually need to merge systemlib on
   * every request because some of the builtins will not be marked
   * persistent.
   */
  if (UNLIKELY(SystemLib::s_anyNonPersistentBuiltins)) {
    SystemLib::s_unit->merge();
    Extension::MergeSystemlib();
    if (SystemLib::s_hhas_unit) SystemLib::s_hhas_unit->merge();
    SystemLib::s_nativeFuncUnit->merge();
    SystemLib::s_nativeClassUnit->merge();
  } else {
    // System units are merge only, and everything is persistent.
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
  vmStack().requestExit();
  profileRequestEnd();
  EventHook::Disable();
  EnvConstants::requestExit();

  if (m_globalVarEnv) {
    smart_delete(m_globalVarEnv);
    m_globalVarEnv = 0;
  }

  if (Logger::UseRequestLog) Logger::SetThreadHook(nullptr, nullptr);
}

///////////////////////////////////////////////////////////////////////////////
}
