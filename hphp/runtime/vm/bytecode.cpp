/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "folly/String.h"

#include "hphp/runtime/base/tv-comparisons.h"
#include "hphp/runtime/base/tv-conversions.h"
#include "hphp/runtime/base/tv-arith.h"
#include "hphp/compiler/builtin_symbols.h"
#include "hphp/runtime/vm/event-hook.h"
#include "hphp/runtime/vm/jit/translator.h"
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
#include "hphp/util/util.h"
#include "hphp/util/trace.h"
#include "hphp/util/debug.h"
#include "hphp/runtime/base/stat-cache.h"
#include "hphp/runtime/vm/debug/debug.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/php-debug.h"
#include "hphp/runtime/vm/debugger-hook.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/jit/target-cache.h"
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
#include "hphp/runtime/ext/ext_variable.h"
#include "hphp/runtime/ext/ext_array.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/vm/type-profile.h"
#include "hphp/runtime/server/source-root-info.h"
#include "hphp/runtime/base/extended-logger.h"
#include "hphp/runtime/base/tracer.h"
#include "hphp/runtime/base/memory-profile.h"

#include "hphp/system/systemlib.h"
#include "hphp/runtime/ext/ext_collections.h"

#include "hphp/runtime/vm/name-value-table-wrapper.h"
#include "hphp/runtime/vm/request-arena.h"
#include "hphp/util/arena.h"

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <boost/format.hpp>
#include <boost/utility/typed_in_place_factory.hpp>

#include <cinttypes>

#include <libgen.h>
#include <sys/mman.h>

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

using Transl::VMRegAnchor;
using Transl::EagerVMRegAnchor;

#if DEBUG
#define OPTBLD_INLINE
#else
#define OPTBLD_INLINE ALWAYS_INLINE
#endif
TRACE_SET_MOD(bcinterp);

ActRec* ActRec::arGetSfp() const {
  ActRec* prevFrame = (ActRec*)m_savedRbp;
  if (LIKELY(((uintptr_t)prevFrame - Util::s_stackLimit) >=
             Util::s_stackSize)) {
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
    VMExecutionContext* context = g_vmContext;
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

static inline
Transl::Translator* tx() {
  return Transl::Translator::Get();
}

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

//=============================================================================
// VarEnv.

VarEnv::VarEnv()
  : m_depth(0)
  , m_malloced(false)
  , m_global(false)
  , m_cfp(0)
  , m_nvTable(boost::in_place<NameValueTable>(
      RuntimeOption::EvalVMInitialGlobalTableSize))
{
  TypedValue globalArray;
  globalArray.m_type = KindOfArray;
  globalArray.m_data.parr =
    new (request_arena()) GlobalNameValueTableWrapper(&*m_nvTable);
  globalArray.m_data.parr->incRefCount();
  m_nvTable->set(makeStaticString("GLOBALS"), &globalArray);
  tvRefcountedDecRef(&globalArray);
}

VarEnv::VarEnv(ActRec* fp, ExtraArgs* eArgs)
  : m_extraArgs(eArgs)
  , m_depth(1)
  , m_malloced(false)
  , m_global(false)
  , m_cfp(fp)
{
  const Func* func = fp->m_func;
  const Id numNames = func->numNamedLocals();

  if (!numNames) return;

  m_nvTable = boost::in_place<NameValueTable>(numNames);

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

  if (!isGlobalScope()) {
    if (LIKELY(!m_malloced)) {
      varenv_arena().endFrame();
      return;
    }
  } else {
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

VarEnv* VarEnv::createLocalOnStack(ActRec* fp) {
  auto& va = varenv_arena();
  va.beginFrame();
  void* mem = va.alloc(getObjectSz(fp));
  VarEnv* ret = new (mem) VarEnv(fp, fp->getExtraArgs());
  TRACE(3, "Creating lazily attached VarEnv %p on stack\n", mem);
  return ret;
}

VarEnv* VarEnv::createLocalOnHeap(ActRec* fp) {
  void* mem = malloc(getObjectSz(fp));
  VarEnv* ret = new (mem) VarEnv(fp, fp->getExtraArgs());
  TRACE(3, "Creating lazily attached VarEnv %p on heap\n", mem);
  ret->m_malloced = true;
  return ret;
}

VarEnv* VarEnv::createGlobal() {
  assert(!g_vmContext->m_globalVarEnv);

  VarEnv* ret = new (request_arena()) VarEnv();
  TRACE(3, "Creating VarEnv %p [global scope]\n", ret);
  ret->m_global = true;
  g_vmContext->m_globalVarEnv = ret;
  return ret;
}

void VarEnv::destroy(VarEnv* ve) {
  bool malloced = ve->m_malloced;
  ve->~VarEnv();
  if (UNLIKELY(malloced)) free(ve);
}

void VarEnv::attach(ActRec* fp) {
  TRACE(3, "Attaching VarEnv %p [%s] %d fp @%p\n",
           this,
           isGlobalScope() ? "global scope" : "local scope",
           int(fp->m_func->numNamedLocals()), fp);
  assert(m_depth == 0 || fp->arGetSfp() == m_cfp ||
         (fp->arGetSfp() == fp && g_vmContext->isNested()));
  m_cfp = fp;
  m_depth++;

  // Overlay fp's locals, if it has any.

  const Func* func = fp->m_func;
  const Id numNames = func->numNamedLocals();
  if (!numNames) {
    return;
  }
  if (!m_nvTable) {
    m_nvTable = boost::in_place<NameValueTable>(numNames);
  }

  TypedValue** origLocs = new (varenv_arena()) TypedValue*[
    func->numNamedLocals()];
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

  VMExecutionContext* context = g_vmContext;
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
    m_nvTable = boost::in_place<NameValueTable>(kLazyNvtSize);
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
  if (!Util::isPowerOfTwo(RuntimeOption::EvalVMStackElms)) {
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
    // For RPCRequestHandler threads, the ExecutionContext can stay alive
    // across requests, and hold references to the VM stack, and
    // the TargetCache needs to keep track of which classes are live etc
    // So only flush the VM stack and the target cache if the execution
    // context is dead.

    if (!t_se.isNull()) {
      t_se->flush();
    }
    Transl::TargetCache::flush();
  }
}

static std::string toStringElm(const TypedValue* tv) {
  std::ostringstream os;

  if (tv->m_type < MinDataType || tv->m_type > MaxNumDataTypes) {
    os << " ??? type " << tv->m_type << "\n";
    return os.str();
  }

  assert(tv->m_type >= MinDataType && tv->m_type < MaxNumDataTypes);
  if (IS_REFCOUNTED_TYPE(tv->m_type) && tv->m_data.pref->m_count <= 0) {
    // OK in the invoking frame when running a destructor.
    os << " ??? inner_count " << tv->m_data.pref->m_count << " ";
    return os.str();
  }

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
      os << tv->m_data.pstr
         << "c(" << tv->m_data.pstr->getCount() << ")"
         << ":\""
         << Util::escapeStringForCPP(tv->m_data.pstr->data(), len)
         << "\"" << (truncated ? "..." : "");
    }
    break;
  case KindOfArray:
    assert(tv->m_data.parr->getCount() > 0);
    os << tv->m_data.parr
       << "c(" << tv->m_data.parr->getCount() << ")"
       << ":Array";
     break;
  case KindOfObject:
    assert(tv->m_data.pobj->getCount() > 0);
    os << tv->m_data.pobj
       << "c(" << tv->m_data.pobj->getCount() << ")"
       << ":Object("
       << tv->m_data.pobj->o_getClassName().get()->data()
       << ")";
    break;
  case KindOfResource:
    assert(tv->m_data.pres->getCount() > 0);
    os << tv->m_data.pres
       << "c(" << tv->m_data.pres->getCount() << ")"
       << ":Resource("
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
    ActRec* prevFp = g_vmContext->getPrevVMState(fp, &prevPc, &prevStackTop);
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

  assert(!func->info() || func->numIterators() == 0);
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
     << unit->getLineNumber(unit->offsetOf(vmpc())) << " func "
     << func->fullName()->data() << " ===\n";

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
  const Func* func = fp->m_func;
  assert(!func->isGenerator());
  return (TypedValue*)((uintptr_t)fp
                       - (uintptr_t)(func->numLocals()) * sizeof(TypedValue)
                       - (uintptr_t)(func->numIterators() * sizeof(Iter)));
}

TypedValue* Stack::generatorStackBase(const ActRec* fp) {
  assert(fp->m_func->isGenerator());
  VMExecutionContext* context = g_vmContext;
  ActRec* sfp = fp->arGetSfp();
  if (sfp == fp) {
    // In the reentrant case, we can consult the savedVM state. We simply
    // use the top of stack of the previous VM frame (since the ActRec,
    // locals, and iters for this frame do not reside on the VM stack).
    return context->m_nestedVMs.back().m_savedState.sp;
  }
  // In the non-reentrant case, we know generators are always called from a
  // function with an empty stack. So we find the caller's FP, compensate
  // for its locals, and then we've found the base of the generator's stack.
  return (TypedValue*)sfp - sfp->m_func->numSlotsInFrame();
}


__thread RequestArenaStorage s_requestArenaStorage;
__thread VarEnvArenaStorage s_varEnvArenaStorage;


//=============================================================================
// ExecutionContext.

using namespace HPHP;
using namespace HPHP::MethodLookup;

ActRec* VMExecutionContext::getOuterVMFrame(const ActRec* ar) {
  ActRec* prevFrame = (ActRec*)ar->m_savedRbp;
  if (LIKELY(((uintptr_t)prevFrame - Util::s_stackLimit) >=
             Util::s_stackSize)) {
    if (LIKELY(prevFrame != nullptr)) return prevFrame;
  }

  if (LIKELY(!m_nestedVMs.empty())) return m_nestedVMs.back().m_savedState.fp;
  return nullptr;
}

Cell* VMExecutionContext::lookupClsCns(const NamedEntity* ne,
                                             const StringData* cls,
                                             const StringData* cns) {
  Class* class_ = Unit::loadClass(ne, cls);
  if (class_ == nullptr) {
    raise_error(Strings::UNKNOWN_CLASS, cls->data());
  }
  Cell* clsCns = class_->clsCnsGet(cns);
  if (clsCns == nullptr) {
    raise_error("Couldn't find constant %s::%s",
                cls->data(), cns->data());
  }
  return clsCns;
}

TypedValue* VMExecutionContext::lookupClsCns(const StringData* cls,
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

const Func* VMExecutionContext::lookupMethodCtx(const Class* cls,
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
      static StringData* sd__construct
        = makeStaticString("__construct");
      if (UNLIKELY(methodName == sd__construct)) {
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
      !g_vmContext->getDebuggerBypassCheck()) {
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

LookupResult VMExecutionContext::lookupObjMethod(const Func*& f,
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
VMExecutionContext::lookupClsMethod(const Func*& f,
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
          // Throw a fatal errpr
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

LookupResult VMExecutionContext::lookupCtorMethod(const Func*& f,
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

ObjectData* VMExecutionContext::createObject(StringData* clsName,
                                             CArrRef params,
                                             bool init /* = true */) {
  Class* class_ = Unit::loadClass(clsName);
  if (class_ == nullptr) {
    throw_missing_class(clsName->data());
  }
  Object o;
  o = newInstance(class_);
  if (init) {
    // call constructor
    TypedValue ret;
    invokeFunc(&ret, class_->getCtor(), params, o.get());
    tvRefcountedDecRef(&ret);
  }

  ObjectData* ret = o.detach();
  ret->decRefCount();
  return ret;
}

ObjectData* VMExecutionContext::createObjectOnly(StringData* clsName) {
  return createObject(clsName, null_array, false);
}

ActRec* VMExecutionContext::getStackFrame() {
  VMRegAnchor _;
  return getFP();
}

ObjectData* VMExecutionContext::getThis() {
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

Class* VMExecutionContext::getContextClass() {
  VMRegAnchor _;
  ActRec* ar = getFP();
  assert(ar != nullptr);
  if (ar->skipFrame()) {
    ar = getPrevVMState(ar);
    if (!ar) return nullptr;
  }
  return ar->m_func->cls();
}

Class* VMExecutionContext::getParentContextClass() {
  if (Class* ctx = getContextClass()) {
    return ctx->parent();
  }
  return nullptr;
}

CStrRef VMExecutionContext::getContainingFileName() {
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

int VMExecutionContext::getLine() {
  VMRegAnchor _;
  ActRec* ar = getFP();
  Unit* unit = ar ? ar->m_func->unit() : nullptr;
  Offset pc = unit ? pcOff() : 0;
  if (ar == nullptr) return -1;
  if (ar->skipFrame()) {
    ar = getPrevVMState(ar, &pc);
  }
  if (ar == nullptr || (unit = ar->m_func->unit()) == nullptr) return -1;
  return unit->getLineNumber(pc);
}

Array VMExecutionContext::getCallerInfo() {
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

bool VMExecutionContext::renameFunction(const StringData* oldName,
                                        const StringData* newName) {
  return m_renamedFuncs.rename(oldName, newName);
}

bool VMExecutionContext::isFunctionRenameable(const StringData* name) {
  return m_renamedFuncs.isFunctionRenameable(name);
}

void VMExecutionContext::addRenameableFunctions(ArrayData* arr) {
  m_renamedFuncs.addRenameableFunctions(arr);
}

VarEnv* VMExecutionContext::getVarEnv() {
  VMRegAnchor _;

  ActRec* fp = getFP();
  if (UNLIKELY(!fp)) return NULL;
  if (fp->skipFrame()) {
    fp = getPrevVMState(fp);
  }
  if (!fp) return nullptr;
  assert(!fp->hasInvName());
  if (!fp->hasVarEnv()) {
    fp->setVarEnv(VarEnv::createLocalOnStack(fp));
  }
  return fp->m_varEnv;
}

void VMExecutionContext::setVar(StringData* name, TypedValue* v, bool ref) {
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

Array VMExecutionContext::getLocalDefinedVariables(int frame) {
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

void VMExecutionContext::shuffleMagicArgs(ActRec* ar) {
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

static inline void checkStack(Stack& stk, const Func* f) {
  ThreadInfo* info = ThreadInfo::s_threadInfo.getNoCheck();
  // Check whether func's maximum stack usage would overflow the stack.
  // Both native and VM stack overflows are independently possible.
  if (!stack_in_bounds(info) ||
      stk.wouldOverflow(f->maxStackCells() + kStackCheckPadding)) {
    TRACE(1, "Maximum VM stack depth exceeded.\n");
    raise_error("Stack overflow");
  }
}

bool VMExecutionContext::prepareFuncEntry(ActRec *ar, PC& pc) {
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
      if (!func->isGenerator()) {
        assert(func->isPseudoMain());
        pushLocalsAndIterators(func);
        ar->m_varEnv->attach(ar);
      }
      pc = func->getEntry();
      // Nothing more to do; get out
      return true;
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

  if (LIKELY(!func->isGenerator())) {
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
  if (raiseMissingArgumentWarnings && !func->info() &&
      !(func->attrs() & AttrNative)) {
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
  return true;
}

void VMExecutionContext::syncGdbState() {
  if (RuntimeOption::EvalJit && !RuntimeOption::EvalJitNoGdb) {
    tx()->getDebugInfo()->debugSync();
  }
}

void VMExecutionContext::enterVMPrologue(ActRec* enterFnAr) {
  assert(enterFnAr);
  Stats::inc(Stats::VMEnter);
  if (ThreadInfo::s_threadInfo->m_reqInjectionData.getJit()) {
    int np = enterFnAr->m_func->numParams();
    int na = enterFnAr->numArgs();
    if (na > np) na = np + 1;
    Transl::TCA start = enterFnAr->m_func->getPrologue(na);
    tx()->enterTCAtPrologue(enterFnAr, start);
  } else {
    if (prepareFuncEntry(enterFnAr, m_pc)) {
      enterVMWork(enterFnAr);
    }
  }
}

void VMExecutionContext::enterVMWork(ActRec* enterFnAr) {
  Transl::TCA start = nullptr;
  if (enterFnAr) {
    if (!EventHook::FunctionEnter(enterFnAr, EventHook::NormalFunc)) return;
    checkStack(m_stack, enterFnAr->m_func);
    start = enterFnAr->m_func->getFuncBody();
  }
  Stats::inc(Stats::VMEnter);
  if (ThreadInfo::s_threadInfo->m_reqInjectionData.getJit()) {
    (void) m_fp->unit()->offsetOf(m_pc); /* assert */
    if (enterFnAr) {
      assert(start);
      tx()->enterTCAfterPrologue(start);
    } else {
      SrcKey sk(m_fp->func(), m_pc);
      tx()->enterTCAtSrcKey(sk);
    }
  } else {
    dispatch();
  }
}

void VMExecutionContext::enterVM(TypedValue* retval, ActRec* ar) {
  DEBUG_ONLY int faultDepth = m_faults.size();
  SCOPE_EXIT { assert(m_faults.size() == faultDepth); };

  m_firstAR = ar;
  ar->m_savedRip = reinterpret_cast<uintptr_t>(tx()->uniqueStubs.callToExit);
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
      if (m_fp && !ar->m_varEnv) {
        enterVMPrologue(ar);
      } else if (prepareFuncEntry(ar, m_pc)) {
        enterVMWork(ar);
      }
    } else {
      enterVMWork(0);
    }

    // Everything succeeded with no exception---return to the previous
    // VM nesting level.
    *retval = *m_stack.topTV();
    m_stack.discard();
    return;

  } catch (...) {
    always_assert(Transl::tl_regState == Transl::VMRegState::CLEAN);
    auto const action = exception_handler();
    if (action == UnwindAction::ResumeVM) {
      goto resume;
    }
    always_assert(action == UnwindAction::Propagate);
  }

  /*
   * Here we have to propagate an exception out of this VM's nesting
   * level.
   */

  if (g_vmContext->m_nestedVMs.empty()) {
    m_fp = nullptr;
    m_pc = nullptr;
  }

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

void VMExecutionContext::reenterVM(TypedValue* retval,
                                   ActRec* ar,
                                   TypedValue* savedSP) {
  ar->m_soff = 0;
  ar->m_savedRbp = 0;
  VMState savedVM = { getPC(), getFP(), m_firstAR, savedSP };
  TRACE(3, "savedVM: %p %p %p %p\n", m_pc, m_fp, m_firstAR, savedSP);
  pushVMState(savedVM, ar);
  assert(m_nestedVMs.size() >= 1);
  try {
    enterVM(retval, ar);
    popVMState();
  } catch (...) {
    popVMState();
    throw;
  }
  TRACE(1, "Reentry: exit fp %p pc %p\n", m_fp, m_pc);
}

void VMExecutionContext::invokeFunc(TypedValue* retval,
                                    const Func* f,
                                    CArrRef params,
                                    ObjectData* this_ /* = NULL */,
                                    Class* cls /* = NULL */,
                                    VarEnv* varEnv /* = NULL */,
                                    StringData* invName /* = NULL */,
                                    InvokeFlags flags /* = InvokeNormal */) {
  assert(retval);
  assert(f);
  // If this is a regular function, this_ and cls must be NULL
  assert(f->preClass() || f->isPseudoMain() || (!this_ && !cls));
  // If this is a method, either this_ or cls must be non-NULL
  assert(!f->preClass() || (this_ || cls));
  // If this is a static method, this_ must be NULL
  assert(!(f->attrs() & AttrStatic && !f->isClosureBody()) ||
         (!this_));
  // invName should only be non-NULL if we are calling __call or
  // __callStatic
  assert(!invName || f->name()->isame(s___call.get()) ||
         f->name()->isame(s___callStatic.get()));
  // If a variable environment is being inherited then params must be empty
  assert(!varEnv || params.empty());

  VMRegAnchor _;

  bool isMagicCall = (invName != nullptr);

  if (this_ != nullptr) {
    this_->incRefCount();
  }
  Cell* savedSP = m_stack.top();

  if (f->attrs() & AttrPhpLeafFn ||
      f->numParams() > kStackCheckReenterPadding - kNumActRecCells) {
    checkStack(m_stack, f);
  }

  if (flags & InvokePseudoMain) {
    assert(f->isPseudoMain() && !params.get());
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
  if (isMagicCall) {
    ar->initNumArgs(2);
  } else {
    ar->initNumArgs(params.size());
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

  ArrayData *arr = params.get();
  if (isMagicCall) {
    // Put the method name into the location of the first parameter. We
    // are transferring ownership, so no need to incRef/decRef here.
    m_stack.pushStringNoRc(invName);
    // Put array of arguments into the location of the second parameter
    m_stack.pushArray(arr);
  } else if (arr) {
    const int numParams = f->numParams();
    const int numExtraArgs = arr->size() - numParams;
    ExtraArgs* extraArgs = nullptr;
    if (numExtraArgs > 0 && (f->attrs() & AttrMayUseVV)) {
      extraArgs = ExtraArgs::allocateUninit(numExtraArgs);
      ar->setExtraArgs(extraArgs);
    }
    int paramId = 0;
    for (ssize_t i = arr->iter_begin();
         i != ArrayData::invalid_index;
         i = arr->iter_advance(i), ++paramId) {
      TypedValue *from = arr->nvGetValueRef(i);
      TypedValue *to;
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

  if (m_fp) {
    reenterVM(retval, ar, savedSP);
  } else {
    assert(m_nestedVMs.size() == 0);
    enterVM(retval, ar);
  }
}

void VMExecutionContext::invokeFuncCleanupHelper(TypedValue* retval,
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

void VMExecutionContext::invokeFuncFew(TypedValue* retval,
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
    checkStack(m_stack, f);
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

  if (m_fp) {
    reenterVM(retval, ar, savedSP);
  } else {
    assert(m_nestedVMs.size() == 0);
    enterVM(retval, ar);
  }
}

void VMExecutionContext::invokeContFunc(const Func* f,
                                        ObjectData* this_,
                                        Cell* param /* = NULL */) {
  assert(f);
  assert(this_);

  EagerVMRegAnchor _;

  this_->incRefCount();

  Cell* savedSP = m_stack.top();

  // no need to check stack due to ReenterPadding
  assert(kStackCheckReenterPadding - kNumActRecCells >= 1);

  ActRec* ar = m_stack.allocA();
  ar->m_savedRbp = 0;
  ar->m_func = f;
  ar->m_soff = 0;
  ar->initNumArgs(param != nullptr ? 1 : 0);
  ar->setThis(this_);
  ar->setVarEnv(nullptr);

  if (param != nullptr) {
    cellDup(*param, *m_stack.allocC());
  }

  TypedValue retval;
  reenterVM(&retval, ar, savedSP);
  // Codegen for generator functions guarantees that they will return null
  assert(IS_NULL_TYPE(retval.m_type));
}

void VMExecutionContext::invokeUnit(TypedValue* retval, Unit* unit) {
  Func* func = unit->getMain();
  invokeFunc(retval, func, null_array, nullptr, nullptr,
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
ActRec* VMExecutionContext::getPrevVMState(const ActRec* fp,
                                           Offset* prevPc /* = NULL */,
                                           TypedValue** prevSp /* = NULL */,
                                           bool* fromVMEntry /* = NULL */) {
  if (fp == nullptr) {
    return nullptr;
  }
  ActRec* prevFp = fp->arGetSfp();
  if (prevFp != fp) {
    if (prevSp) {
      if (UNLIKELY(fp->m_func->isGenerator())) {
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
  for (; i >= 0; --i) {
    if (m_nestedVMs[i].m_entryFP == fp) break;
  }
  if (i == -1) return nullptr;
  const VMState& vmstate = m_nestedVMs[i].m_savedState;
  prevFp = vmstate.fp;
  assert(prevFp);
  assert(prevFp->m_func->unit());
  if (prevSp) *prevSp = vmstate.sp;
  if (prevPc) *prevPc = prevFp->m_func->unit()->offsetOf(vmstate.pc);
  if (fromVMEntry) *fromVMEntry = true;
  return prevFp;
}

Array VMExecutionContext::debugBacktrace(bool skip /* = false */,
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
    auto const curOp = toOp(*curUnit->at(pc));
    auto const isReturning = curOp == OpRetC || curOp == OpRetV;

    // Builtins and generators don't have a file and line number
    if (prevFp && !prevFp->m_func->isBuiltin() && !fp->m_func->isGenerator()) {
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
        toOp(*reinterpret_cast<const Opcode*>(prevUnit->at(prevPc)));
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
    if (fp->m_func->isGenerator()) {
      // retrieve the original function name from the inner continuation
      funcname = frame_continuation(fp)->t_getorigfuncname();
    }

    if (fp->m_func->isClosureBody()) {
      static StringData* s_closure_label =
          makeStaticString("{closure}");
      funcname = s_closure_label;
    }

    // check for pseudomain
    if (funcname->empty()) {
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
  destroyMembers(m_methodsVec);
  destroyMapValues(m_properties);
  destroyMapValues(m_constants);
}

Array VMExecutionContext::getUserFunctionsInfo() {
  // Return an array of all user-defined function names.  This method is used to
  // support get_defined_functions().
  return Unit::getUserFunctions();
}

const ClassInfo::MethodInfo* VMExecutionContext::findFunctionInfo(
  CStrRef name) {
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

const ClassInfo* VMExecutionContext::findClassInfo(CStrRef name) {
  if (name->empty()) return nullptr;
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

const ClassInfo* VMExecutionContext::findInterfaceInfo(CStrRef name) {
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

const ClassInfo* VMExecutionContext::findTraitInfo(CStrRef name) {
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

const ClassInfo::ConstantInfo* VMExecutionContext::findConstantInfo(
    CStrRef name) {
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

HPHP::Eval::PhpFile* VMExecutionContext::lookupPhpFile(StringData* path,
                                                       const char* currentDir,
                                                       bool* initial_opt) {
  bool init;
  bool &initial = initial_opt ? *initial_opt : init;
  initial = true;

  struct stat s;
  String spath = Eval::resolveVmInclude(path, currentDir, &s);
  if (spath.isNull()) return nullptr;

  // Check if this file has already been included.
  EvaledFilesMap::const_iterator it = m_evaledFiles.find(spath.get());
  HPHP::Eval::PhpFile* efile = nullptr;
  if (it != m_evaledFiles.end()) {
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
    if (Transl::TargetCache::testAndSetBit(efile->getId())) {
      initial = false;
    }
    // if parsing was successful, update the mappings for spath and
    // rpath (if it exists).
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

Unit* VMExecutionContext::evalInclude(StringData* path,
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

HPHP::Unit* VMExecutionContext::evalIncludeRoot(
  StringData* path, InclOpFlags flags, bool* initial) {
  HPHP::Eval::PhpFile* efile = lookupIncludeRoot(path, flags, initial);
  return efile ? efile->unit() : 0;
}

HPHP::Eval::PhpFile* VMExecutionContext::lookupIncludeRoot(StringData* path,
                                                           InclOpFlags flags,
                                                           bool* initial,
                                                           Unit* unit) {
  String absPath;
  if ((flags & InclOpRelative)) {
    namespace fs = boost::filesystem;
    if (!unit) unit = getFP()->m_func->unit();
    fs::path currentUnit(unit->filepath()->data());
    fs::path currentDir(currentUnit.branch_path());
    absPath = currentDir.string() + '/';
    TRACE(2, "lookupIncludeRoot(%s): relative -> %s\n",
          path->data(),
          absPath->data());
  } else {
    assert(flags & InclOpDocRoot);
    absPath = SourceRootInfo::GetCurrentPhpRoot();
    TRACE(2, "lookupIncludeRoot(%s): docRoot -> %s\n",
          path->data(),
          absPath->data());
  }

  absPath += StrNR(path);

  EvaledFilesMap::const_iterator it = m_evaledFiles.find(absPath.get());
  if (it != m_evaledFiles.end()) {
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
bool VMExecutionContext::evalUnit(Unit* unit, PC& pc, int funcType) {
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
  assert(!func->info());
  assert(!func->isGenerator());
  ar->m_func = func;
  ar->initNumArgs(0);
  assert(getFP());
  assert(!m_fp->hasInvName());
  arSetSfp(ar, m_fp);
  ar->m_soff = uintptr_t(m_fp->m_func->unit()->offsetOf(pc) -
                         m_fp->m_func->base());
  ar->m_savedRip = reinterpret_cast<uintptr_t>(tx()->uniqueStubs.retHelper);
  assert(isReturnHelper(ar->m_savedRip));
  pushLocalsAndIterators(func);
  if (!m_fp->hasVarEnv()) {
    m_fp->setVarEnv(VarEnv::createLocalOnStack(m_fp));
  }
  ar->m_varEnv = m_fp->m_varEnv;
  ar->m_varEnv->attach(ar);

  m_fp = ar;
  pc = func->getEntry();
  SYNC();
  bool ret = EventHook::FunctionEnter(m_fp, funcType);
  pc = m_pc;
  return ret;
}

StaticString
  s_php_namespace("<?php namespace "),
  s_curly_return(" { return "),
  s_semicolon_curly("; }"),
  s_php_return("<?php return "),
  s_semicolon(";");
CVarRef VMExecutionContext::getEvaledArg(const StringData* val,
                                         CStrRef namespacedName) {
  CStrRef key = *(String*)&val;

  if (m_evaledArgs.get()) {
    CVarRef arg = m_evaledArgs.get()->get(key);
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
  g_vmContext->invokeFunc((TypedValue*)&v, unit->getMain(),
                          null_array, nullptr, nullptr, nullptr, nullptr,
                          InvokePseudoMain);
  Variant &lv = m_evaledArgs.lvalAt(key, AccessFlags::Key);
  lv = v;
  return lv;
}

/*
 * Helper for function entry, including pseudo-main entry.
 */
void
VMExecutionContext::pushLocalsAndIterators(const Func* func,
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

void VMExecutionContext::destructObjects() {
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
Unit* VMExecutionContext::compileEvalString(StringData* code) {
  EvaledUnitsMap::accessor acc;
  // Promote this to a static string; otherwise it may get swept
  // across requests.
  code = makeStaticString(code);
  if (s_evaledUnits.insert(acc, code)) {
    acc->second = compile_string(code->data(), code->size());
  }
  return acc->second;
}

CStrRef VMExecutionContext::createFunction(CStrRef args, CStrRef code) {
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
  invokeFunc(&retval, unit->getMain(), null_array,
             nullptr, nullptr, nullptr, nullptr,
             InvokePseudoMain);

  // __lambda_func will be the only hoistable function.
  // Any functions or closures defined in it will not be hoistable.
  Func* lambda = unit->firstHoistable();
  return lambda->nameRef();
}

bool VMExecutionContext::evalPHPDebugger(TypedValue* retval, StringData *code,
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
      fp->setVarEnv(VarEnv::createLocalOnHeap(fp));
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
    invokeFunc(retval, unit->getMain(functionClass), null_array,
               this_, frameClass, varEnv, nullptr, InvokePseudoMain);
    failed = false;
  } catch (FatalErrorException &e) {
    g_vmContext->write(s_fatal);
    g_vmContext->write(" : ");
    g_vmContext->write(e.getMessage().c_str());
    g_vmContext->write("\n");
    g_vmContext->write(ExtendedLogger::StringOfStackTrace(e.getBackTrace()));
  } catch (ExitException &e) {
    g_vmContext->write(s_exit.data());
    g_vmContext->write(" : ");
    std::ostringstream os;
    os << ExitException::ExitCode;
    g_vmContext->write(os.str());
  } catch (Eval::DebuggerException &e) {
  } catch (Exception &e) {
    g_vmContext->write(s_cppException.data());
    g_vmContext->write(" : ");
    g_vmContext->write(e.getMessage().c_str());
    ExtendedException* ee = dynamic_cast<ExtendedException*>(&e);
    if (ee) {
      g_vmContext->write("\n");
      g_vmContext->write(
        ExtendedLogger::StringOfStackTrace(ee->getBackTrace()));
    }
  } catch (Object &e) {
    g_vmContext->write(s_phpException.data());
    g_vmContext->write(" : ");
    g_vmContext->write(e->invokeToString().data());
  } catch (...) {
    g_vmContext->write(s_cppException.data());
  }

  if (varEnv) {
    // The debugger eval frame may have attached to the VarEnv from a
    // frame that was not the top frame, so we need to manually set
    // cfp back to what it was before
    varEnv->setCfp(cfpSave);
  }
  return failed;
}

void VMExecutionContext::enterDebuggerDummyEnv() {
  static Unit* s_debuggerDummy = compile_string("<?php?>", 7);
  // Ensure that the VM stack is completely empty (m_fp should be null)
  // and that we're not in a nested VM (reentrancy)
  assert(getFP() == nullptr);
  assert(m_nestedVMs.size() == 0);
  assert(m_nesting == 0);
  assert(m_stack.count() == 0);
  ActRec* ar = m_stack.allocA();
  ar->m_func = s_debuggerDummy->getMain();
  ar->setThis(nullptr);
  ar->m_soff = 0;
  ar->m_savedRbp = 0;
  ar->m_savedRip = reinterpret_cast<uintptr_t>(tx()->uniqueStubs.callToExit);
  assert(isReturnHelper(ar->m_savedRip));
  m_fp = ar;
  m_pc = s_debuggerDummy->entry();
  m_firstAR = ar;
  m_fp->setVarEnv(m_globalVarEnv);
  m_globalVarEnv->attach(m_fp);
}

void VMExecutionContext::exitDebuggerDummyEnv() {
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
bool VMExecutionContext::isReturnHelper(uintptr_t address) {
  auto tcAddr = reinterpret_cast<Transl::TCA>(address);
  auto& u = tx()->uniqueStubs;
  return tcAddr == u.retHelper ||
         tcAddr == u.genRetHelper ||
         tcAddr == u.retInlHelper ||
         tcAddr == u.callToExit;
}

// Walk the stack and find any return address to jitted code and bash it to
// the appropriate RetFromInterpreted*Frame helper. This ensures that we don't
// return into jitted code and gives the system the proper chance to interpret
// blacklisted tracelets.
void VMExecutionContext::preventReturnsToTC() {
  assert(isDebuggerAttached());
  if (RuntimeOption::EvalJit) {
    ActRec *ar = getFP();
    while (ar) {
      if (!isReturnHelper(ar->m_savedRip) &&
          (tx()->isValidCodeAddress((Transl::TCA)ar->m_savedRip))) {
        TRACE_RB(2, "Replace RIP in fp %p, savedRip 0x%" PRIx64 ", "
                 "func %s\n", ar, ar->m_savedRip,
                 ar->m_func->fullName()->data());
        if (ar->m_func->isGenerator()) {
          ar->m_savedRip =
            reinterpret_cast<uintptr_t>(tx()->uniqueStubs.genRetHelper);
        } else {
          ar->m_savedRip =
            reinterpret_cast<uintptr_t>(tx()->uniqueStubs.retHelper);
        }
        assert(isReturnHelper(ar->m_savedRip));
      }
      ar = getPrevVMState(ar);
    }
  }
}

static inline StringData* lookup_name(TypedValue* key) {
  return prepareKey(key);
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
      fp->setVarEnv(VarEnv::createLocalOnStack(fp));
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
  assert(g_vmContext->m_globalVarEnv);
  val = g_vmContext->m_globalVarEnv->lookup(name);
}

static inline void lookupd_gbl(ActRec* fp,
                               StringData*& name,
                               TypedValue* key,
                               TypedValue*& val) {
  name = lookup_name(key);
  assert(g_vmContext->m_globalVarEnv);
  VarEnv* varEnv = g_vmContext->m_globalVarEnv;
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
          VMExecutionContext::VectorLeaveCode mleave>
OPTBLD_INLINE void VMExecutionContext::getHelperPre(
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
OPTBLD_INLINE void VMExecutionContext::getHelperPost(
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
VMExecutionContext::getHelper(PC& pc,
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
          VMExecutionContext::VectorLeaveCode mleave,
          bool saveResult>
OPTBLD_INLINE bool VMExecutionContext::memberHelperPre(
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
  TypedValue dummy;
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
      tvWriteNull(&dummy);
      loc = &dummy;
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
      tvWriteNull(&dummy);
      loc = &dummy;
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
        result = ElemU(tvScratch, tvRef, base, curMember);
      } else if (define) {
        result = ElemD<warn,reffy>(tvScratch, tvRef, base, curMember);
      } else {
        result = Elem<warn>(tvScratch, tvRef, base, curMember);
      }
      break;
    case MPL:
    case MPC:
    case MPT:
      result = Prop<warn, define, unset>(tvScratch, tvRef, ctx, base,
                                         curMember);
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
          VMExecutionContext::VectorLeaveCode mleave>
OPTBLD_INLINE bool VMExecutionContext::setHelperPre(
    PC& pc, unsigned& ndiscard, TypedValue*& base,
    TypedValue& tvScratch, TypedValue& tvLiteral,
    TypedValue& tvRef, TypedValue& tvRef2,
    MemberCode& mcode, TypedValue*& curMember) {
  return memberHelperPre<true, warn, define, unset,
    reffy, mdepth, mleave, false>(MEMBERHELPERPRE_OUT);
}

#define SETHELPERPOST_ARGS ndiscard, tvRef, tvRef2
template <unsigned mdepth>
OPTBLD_INLINE void VMExecutionContext::setHelperPost(
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

OPTBLD_INLINE void VMExecutionContext::iopLowInvalid(PC& pc) {
  fprintf(stderr, "invalid bytecode executed\n");
  abort();
}

OPTBLD_INLINE void VMExecutionContext::iopNop(PC& pc) {
  NEXT();
}

OPTBLD_INLINE void VMExecutionContext::iopPopC(PC& pc) {
  NEXT();
  m_stack.popC();
}

OPTBLD_INLINE void VMExecutionContext::iopPopV(PC& pc) {
  NEXT();
  m_stack.popV();
}

OPTBLD_INLINE void VMExecutionContext::iopPopR(PC& pc) {
  NEXT();
  if (m_stack.topTV()->m_type != KindOfRef) {
    m_stack.popC();
  } else {
    m_stack.popV();
  }
}

OPTBLD_INLINE void VMExecutionContext::iopDup(PC& pc) {
  NEXT();
  m_stack.dup();
}

OPTBLD_INLINE void VMExecutionContext::iopBox(PC& pc) {
  NEXT();
  m_stack.box();
}

OPTBLD_INLINE void VMExecutionContext::iopUnbox(PC& pc) {
  NEXT();
  m_stack.unbox();
}

OPTBLD_INLINE void VMExecutionContext::iopBoxR(PC& pc) {
  NEXT();
  TypedValue* tv = m_stack.topTV();
  if (tv->m_type != KindOfRef) {
    tvBox(tv);
  }
}

OPTBLD_INLINE void VMExecutionContext::iopUnboxR(PC& pc) {
  NEXT();
  if (m_stack.topTV()->m_type == KindOfRef) {
    m_stack.unbox();
  }
}

OPTBLD_INLINE void VMExecutionContext::iopNull(PC& pc) {
  NEXT();
  m_stack.pushNull();
}

OPTBLD_INLINE void VMExecutionContext::iopNullUninit(PC& pc) {
  NEXT();
  m_stack.pushNullUninit();
}

OPTBLD_INLINE void VMExecutionContext::iopTrue(PC& pc) {
  NEXT();
  m_stack.pushTrue();
}

OPTBLD_INLINE void VMExecutionContext::iopFalse(PC& pc) {
  NEXT();
  m_stack.pushFalse();
}

OPTBLD_INLINE void VMExecutionContext::iopFile(PC& pc) {
  NEXT();
  const StringData* s = m_fp->m_func->unit()->filepath();
  m_stack.pushStaticString(const_cast<StringData*>(s));
}

OPTBLD_INLINE void VMExecutionContext::iopDir(PC& pc) {
  NEXT();
  const StringData* s = m_fp->m_func->unit()->dirpath();
  m_stack.pushStaticString(const_cast<StringData*>(s));
}

OPTBLD_INLINE void VMExecutionContext::iopInt(PC& pc) {
  NEXT();
  DECODE(int64_t, i);
  m_stack.pushInt(i);
}

OPTBLD_INLINE void VMExecutionContext::iopDouble(PC& pc) {
  NEXT();
  DECODE(double, d);
  m_stack.pushDouble(d);
}

OPTBLD_INLINE void VMExecutionContext::iopString(PC& pc) {
  NEXT();
  DECODE_LITSTR(s);
  m_stack.pushStaticString(s);
}

OPTBLD_INLINE void VMExecutionContext::iopArray(PC& pc) {
  NEXT();
  DECODE(Id, id);
  ArrayData* a = m_fp->m_func->unit()->lookupArrayId(id);
  m_stack.pushStaticArray(a);
}

OPTBLD_INLINE void VMExecutionContext::iopNewArray(PC& pc) {
  NEXT();
  auto arr = HphpArray::MakeReserve(HphpArray::SmallSize);
  m_stack.pushArrayNoRc(arr);
}

OPTBLD_INLINE void VMExecutionContext::iopNewPackedArray(PC& pc) {
  NEXT();
  DECODE_IVA(n);
  // This constructor moves values, no inc/decref is necessary.
  auto* a = HphpArray::MakePacked(n, m_stack.topC());
  m_stack.ndiscard(n);
  m_stack.pushArrayNoRc(a);
}

OPTBLD_INLINE void VMExecutionContext::iopAddElemC(PC& pc) {
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

OPTBLD_INLINE void VMExecutionContext::iopAddElemV(PC& pc) {
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

OPTBLD_INLINE void VMExecutionContext::iopAddNewElemC(PC& pc) {
  NEXT();
  Cell* c1 = m_stack.topC();
  Cell* c2 = m_stack.indC(1);
  if (c2->m_type != KindOfArray) {
    raise_error("AddNewElemC: $2 must be an array");
  }
  cellAsVariant(*c2).asArrRef().append(tvAsCVarRef(c1));
  m_stack.popC();
}

OPTBLD_INLINE void VMExecutionContext::iopAddNewElemV(PC& pc) {
  NEXT();
  Ref* r1 = m_stack.topV();
  Cell* c2 = m_stack.indC(1);
  if (c2->m_type != KindOfArray) {
    raise_error("AddNewElemV: $2 must be an array");
  }
  cellAsVariant(*c2).asArrRef().append(ref(tvAsCVarRef(r1)));
  m_stack.popV();
}

OPTBLD_INLINE void VMExecutionContext::iopNewCol(PC& pc) {
  NEXT();
  DECODE_IVA(cType);
  DECODE_IVA(nElms);
  ObjectData* obj;
  switch (cType) {
    case Collection::VectorType: obj = NEWOBJ(c_Vector)(); break;
    case Collection::MapType: obj = NEWOBJ(c_Map)(); break;
    case Collection::StableMapType: obj = NEWOBJ(c_StableMap)(); break;
    case Collection::SetType: obj = NEWOBJ(c_Set)(); break;
    case Collection::PairType: obj = NEWOBJ(c_Pair)(); break;
    default:
      obj = nullptr;
      raise_error("NewCol: Invalid collection type");
      break;
  }
  // Reserve enough room for nElms elements in advance
  if (nElms) {
    collectionReserve(obj, nElms);
  }
  m_stack.pushObject(obj);
}

OPTBLD_INLINE void VMExecutionContext::iopColAddNewElemC(PC& pc) {
  NEXT();
  Cell* c1 = m_stack.topC();
  Cell* c2 = m_stack.indC(1);
  if (c2->m_type == KindOfObject && c2->m_data.pobj->isCollection()) {
    collectionAppend(c2->m_data.pobj, c1);
  } else {
    raise_error("ColAddNewElemC: $2 must be a collection");
  }
  m_stack.popC();
}

OPTBLD_INLINE void VMExecutionContext::iopColAddElemC(PC& pc) {
  NEXT();
  Cell* c1 = m_stack.topC();
  Cell* c2 = m_stack.indC(1);
  Cell* c3 = m_stack.indC(2);
  if (c3->m_type == KindOfObject && c3->m_data.pobj->isCollection()) {
    collectionSet(c3->m_data.pobj, c2, c1);
  } else {
    raise_error("ColAddElemC: $3 must be a collection");
  }
  m_stack.popC();
  m_stack.popC();
}

OPTBLD_INLINE void VMExecutionContext::iopCns(PC& pc) {
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

OPTBLD_INLINE void VMExecutionContext::iopCnsE(PC& pc) {
  NEXT();
  DECODE_LITSTR(s);
  TypedValue* cns = Unit::loadCns(s);
  if (cns == nullptr) {
    raise_error("Undefined constant '%s'", s->data());
  }
  auto const c1 = m_stack.allocC();
  cellDup(*cns, *c1);
}

OPTBLD_INLINE void VMExecutionContext::iopCnsU(PC& pc) {
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

OPTBLD_INLINE void VMExecutionContext::iopDefCns(PC& pc) {
  NEXT();
  DECODE_LITSTR(s);
  TypedValue* tv = m_stack.topTV();
  tvAsVariant(tv) = Unit::defCns(s, tv);
}

OPTBLD_INLINE void VMExecutionContext::iopClsCns(PC& pc) {
  NEXT();
  DECODE_LITSTR(clsCnsName);
  TypedValue* tv = m_stack.topTV();
  assert(tv->m_type == KindOfClass);
  Class* class_ = tv->m_data.pcls;
  assert(class_ != nullptr);
  auto const clsCns = class_->clsCnsGet(clsCnsName);
  if (clsCns == nullptr) {
    raise_error("Couldn't find constant %s::%s",
                class_->name()->data(), clsCnsName->data());
  }
  cellDup(*clsCns, *tv);
}

OPTBLD_INLINE void VMExecutionContext::iopClsCnsD(PC& pc) {
  NEXT();
  DECODE_LITSTR(clsCnsName);
  DECODE(Id, classId);
  const NamedEntityPair& classNamedEntity =
    m_fp->m_func->unit()->lookupNamedEntityPairId(classId);

  auto const clsCns = lookupClsCns(classNamedEntity.second,
                                   classNamedEntity.first, clsCnsName);
  assert(clsCns != nullptr);
  auto const c1 = m_stack.allocC();
  cellDup(*clsCns, *c1);
}

OPTBLD_INLINE void VMExecutionContext::iopConcat(PC& pc) {
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
  assert(c2->m_data.pstr->getCount() > 0);
  m_stack.popC();
}

OPTBLD_INLINE void VMExecutionContext::iopNot(PC& pc) {
  NEXT();
  Cell* c1 = m_stack.topC();
  cellAsVariant(*c1) = !cellAsVariant(*c1).toBoolean();
}


OPTBLD_INLINE void VMExecutionContext::iopAbs(PC& pc) {
  NEXT();
  auto c1 = m_stack.topC();

  tvAsVariant(c1) = f_abs(tvAsCVarRef(c1));
}

template<class Op>
OPTBLD_INLINE void VMExecutionContext::implCellBinOp(PC& pc, Op op) {
  NEXT();
  auto const c1 = m_stack.topC();
  auto const c2 = m_stack.indC(1);
  auto const result = op(*c2, *c1);
  tvRefcountedDecRefCell(c2);
  *c2 = result;
  m_stack.popC();
}

template<class Op>
OPTBLD_INLINE void VMExecutionContext::implCellBinOpBool(PC& pc, Op op) {
  NEXT();
  auto const c1 = m_stack.topC();
  auto const c2 = m_stack.indC(1);
  bool const result = op(*c2, *c1);
  tvRefcountedDecRefCell(c2);
  *c2 = make_tv<KindOfBoolean>(result);
  m_stack.popC();
}

OPTBLD_INLINE void VMExecutionContext::iopAdd(PC& pc) {
  implCellBinOp(pc, cellAdd);
}

OPTBLD_INLINE void VMExecutionContext::iopSub(PC& pc) {
  implCellBinOp(pc, cellSub);
}

OPTBLD_INLINE void VMExecutionContext::iopMul(PC& pc) {
  implCellBinOp(pc, cellMul);
}

OPTBLD_INLINE void VMExecutionContext::iopDiv(PC& pc) {
  implCellBinOp(pc, cellDiv);
}

OPTBLD_INLINE void VMExecutionContext::iopMod(PC& pc) {
  implCellBinOp(pc, cellMod);
}

OPTBLD_INLINE void VMExecutionContext::iopBitAnd(PC& pc) {
  implCellBinOp(pc, cellBitAnd);
}

OPTBLD_INLINE void VMExecutionContext::iopBitOr(PC& pc) {
  implCellBinOp(pc, cellBitOr);
}

OPTBLD_INLINE void VMExecutionContext::iopBitXor(PC& pc) {
  implCellBinOp(pc, cellBitXor);
}

OPTBLD_INLINE void VMExecutionContext::iopXor(PC& pc) {
  implCellBinOpBool(pc, [&] (Cell c1, Cell c2) -> bool {
    return cellToBool(c1) ^ cellToBool(c2);
  });
}

OPTBLD_INLINE void VMExecutionContext::iopSame(PC& pc) {
  implCellBinOpBool(pc, cellSame);
}

OPTBLD_INLINE void VMExecutionContext::iopNSame(PC& pc) {
  implCellBinOpBool(pc, [&] (Cell c1, Cell c2) {
    return !cellSame(c1, c2);
  });
}

OPTBLD_INLINE void VMExecutionContext::iopEq(PC& pc) {
  implCellBinOpBool(pc, [&] (Cell c1, Cell c2) {
    return cellEqual(c1, c2);
  });
}

OPTBLD_INLINE void VMExecutionContext::iopNeq(PC& pc) {
  implCellBinOpBool(pc, [&] (Cell c1, Cell c2) {
    return !cellEqual(c1, c2);
  });
}

OPTBLD_INLINE void VMExecutionContext::iopLt(PC& pc) {
  implCellBinOpBool(pc, [&] (Cell c1, Cell c2) {
    return cellLess(c1, c2);
  });
}

OPTBLD_INLINE void VMExecutionContext::iopLte(PC& pc) {
  implCellBinOpBool(pc, cellLessOrEqual);
}

OPTBLD_INLINE void VMExecutionContext::iopGt(PC& pc) {
  implCellBinOpBool(pc, [&] (Cell c1, Cell c2) {
    return cellGreater(c1, c2);
  });
}

OPTBLD_INLINE void VMExecutionContext::iopGte(PC& pc) {
  implCellBinOpBool(pc, cellGreaterOrEqual);
}

OPTBLD_INLINE void VMExecutionContext::iopShl(PC& pc) {
  implCellBinOp(pc, [&] (Cell c1, Cell c2) {
    return make_tv<KindOfInt64>(cellToInt(c1) << cellToInt(c2));
  });
}

OPTBLD_INLINE void VMExecutionContext::iopShr(PC& pc) {
  implCellBinOp(pc, [&] (Cell c1, Cell c2) {
    return make_tv<KindOfInt64>(cellToInt(c1) >> cellToInt(c2));
  });
}

OPTBLD_INLINE void VMExecutionContext::iopSqrt(PC& pc) {
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

OPTBLD_INLINE void VMExecutionContext::iopBitNot(PC& pc) {
  NEXT();
  cellBitNot(*m_stack.topC());
}

OPTBLD_INLINE void VMExecutionContext::iopCastBool(PC& pc) {
  NEXT();
  Cell* c1 = m_stack.topC();
  tvCastToBooleanInPlace(c1);
}

OPTBLD_INLINE void VMExecutionContext::iopCastInt(PC& pc) {
  NEXT();
  Cell* c1 = m_stack.topC();
  tvCastToInt64InPlace(c1);
}

OPTBLD_INLINE void VMExecutionContext::iopCastDouble(PC& pc) {
  NEXT();
  Cell* c1 = m_stack.topC();
  tvCastToDoubleInPlace(c1);
}

OPTBLD_INLINE void VMExecutionContext::iopCastString(PC& pc) {
  NEXT();
  Cell* c1 = m_stack.topC();
  tvCastToStringInPlace(c1);
}

OPTBLD_INLINE void VMExecutionContext::iopCastArray(PC& pc) {
  NEXT();
  Cell* c1 = m_stack.topC();
  tvCastToArrayInPlace(c1);
}

OPTBLD_INLINE void VMExecutionContext::iopCastObject(PC& pc) {
  NEXT();
  Cell* c1 = m_stack.topC();
  tvCastToObjectInPlace(c1);
}

OPTBLD_INLINE bool VMExecutionContext::cellInstanceOf(
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
bool VMExecutionContext::iopInstanceOfHelper(const StringData* str1, Cell* c2) {
  const NamedEntity* rhs = Unit::GetNamedEntity(str1, false);
  // Because of other codepaths, an un-normalized name might enter the
  // table without a Class* so we need to check if it's there.
  if (LIKELY(rhs && rhs->getCachedClass() != nullptr)) {
    return cellInstanceOf(c2, rhs);
  }
  auto normName = normalizeNS(str1);
  if (normName) {
    rhs = Unit::GetNamedEntity(normName.get(), false);
    if (LIKELY(rhs && rhs->getCachedClass() != nullptr)) {
      return cellInstanceOf(c2, rhs);
    }
  }
  return false;
}

OPTBLD_INLINE void VMExecutionContext::iopInstanceOf(PC& pc) {
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
  tvRefcountedDecRefCell(c2);
  c2->m_data.num = r;
  c2->m_type = KindOfBoolean;
}

OPTBLD_INLINE void VMExecutionContext::iopInstanceOfD(PC& pc) {
  NEXT();
  DECODE(Id, id);
  if (shouldProfile()) {
    InstanceBits::profile(m_fp->m_func->unit()->lookupLitstrId(id));
  }
  const NamedEntity* ne = m_fp->m_func->unit()->lookupNamedEntityId(id);
  Cell* c1 = m_stack.topC();
  bool r = cellInstanceOf(c1, ne);
  tvRefcountedDecRefCell(c1);
  c1->m_data.num = r;
  c1->m_type = KindOfBoolean;
}

OPTBLD_INLINE void VMExecutionContext::iopPrint(PC& pc) {
  NEXT();
  Cell* c1 = m_stack.topC();
  echo(cellAsVariant(*c1).toString());
  tvRefcountedDecRefCell(c1);
  c1->m_type = KindOfInt64;
  c1->m_data.num = 1;
}

OPTBLD_INLINE void VMExecutionContext::iopClone(PC& pc) {
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

OPTBLD_INLINE void VMExecutionContext::iopExit(PC& pc) {
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

OPTBLD_INLINE void VMExecutionContext::iopFatal(PC& pc) {
  NEXT();
  TypedValue* top = m_stack.topTV();
  std::string msg;
  DECODE_IVA(skipFrame);
  if (IS_STRING_TYPE(top->m_type)) {
    msg = top->m_data.pstr->data();
  } else {
    msg = "Fatal error message not a string";
  }
  m_stack.popTV();
  if (skipFrame) {
    raise_error_without_first_frame(msg);
  } else {
    raise_error(msg);
  }
}

OPTBLD_INLINE void VMExecutionContext::jmpSurpriseCheck(Offset offset) {
  if (offset <= 0 && UNLIKELY(Transl::TargetCache::loadConditionFlags())) {
    EventHook::CheckSurprise();
  }
}

OPTBLD_INLINE void VMExecutionContext::iopJmp(PC& pc) {
  NEXT();
  DECODE_JMP(Offset, offset);
  jmpSurpriseCheck(offset);

  pc += offset - 1;
}

template<Op op>
OPTBLD_INLINE void VMExecutionContext::jmpOpImpl(PC& pc) {
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

OPTBLD_INLINE void VMExecutionContext::iopJmpZ(PC& pc) {
  jmpOpImpl<OpJmpZ>(pc);
}

OPTBLD_INLINE void VMExecutionContext::iopJmpNZ(PC& pc) {
  jmpOpImpl<OpJmpNZ>(pc);
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

OPTBLD_INLINE void VMExecutionContext::iopIterBreak(PC& pc) {
  PC savedPc = pc;
  NEXT();
  DECODE_ITER_LIST(iterTypeList, iterIdList, veclen);
  DECODE_JMP(Offset, offset);

  jmpSurpriseCheck(offset); // we do this early so iterators are still dirty if
                            // we have an exception

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

OPTBLD_INLINE void VMExecutionContext::iopSwitch(PC& pc) {
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

OPTBLD_INLINE void VMExecutionContext::iopSSwitch(PC& pc) {
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

OPTBLD_INLINE void VMExecutionContext::iopRetC(PC& pc) {
  NEXT();
  uint soff = m_fp->m_soff;
  assert(!m_fp->m_func->isGenerator());

  // Call the runtime helpers to free the local variables and iterators
  frame_free_locals_inl(m_fp, m_fp->m_func->numLocals());
  ActRec* sfp = m_fp->arGetSfp();
  // Memcpy the the return value on top of the activation record. This works
  // the same regardless of whether the return value is boxed or not.
  TypedValue* retval_ptr = &m_fp->m_r;
  memcpy(retval_ptr, m_stack.topTV(), sizeof(TypedValue));
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
              Trace::trace("Return %s from VMExecutionContext::dispatch("
                           "%p)\n", os.str().c_str(), m_fp));
    }
#endif
    pc = 0;
  }
}

OPTBLD_INLINE void VMExecutionContext::iopRetV(PC& pc) {
  iopRetC(pc);
}

OPTBLD_INLINE void VMExecutionContext::iopUnwind(PC& pc) {
  assert(!m_faults.empty());
  assert(m_faults.back().m_savedRaiseOffset != kInvalidOffset);
  throw VMPrepareUnwind();
}

OPTBLD_INLINE void VMExecutionContext::iopThrow(PC& pc) {
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

OPTBLD_INLINE void VMExecutionContext::iopAGetC(PC& pc) {
  NEXT();
  TypedValue* tv = m_stack.topTV();
  lookupClsRef(tv, tv, true);
}

OPTBLD_INLINE void VMExecutionContext::iopAGetL(PC& pc) {
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

OPTBLD_INLINE void VMExecutionContext::iopCGetL(PC& pc) {
  NEXT();
  DECODE_LA(local);
  Cell* to = m_stack.allocC();
  TypedValue* fr = frame_local(m_fp, local);
  cgetl_body(m_fp, fr, to, local);
}

OPTBLD_INLINE void VMExecutionContext::iopCGetL2(PC& pc) {
  NEXT();
  DECODE_LA(local);
  TypedValue* oldTop = m_stack.topTV();
  TypedValue* newTop = m_stack.allocTV();
  memcpy(newTop, oldTop, sizeof *newTop);
  Cell* to = oldTop;
  TypedValue* fr = frame_local(m_fp, local);
  cgetl_body(m_fp, fr, to, local);
}

OPTBLD_INLINE void VMExecutionContext::iopCGetL3(PC& pc) {
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

OPTBLD_INLINE void VMExecutionContext::iopCGetN(PC& pc) {
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

OPTBLD_INLINE void VMExecutionContext::iopCGetG(PC& pc) {
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

OPTBLD_INLINE void VMExecutionContext::iopCGetS(PC& pc) {
  StringData* name;
  GETS(false);
  if (shouldProfile() && name && name->isStatic()) {
    recordType(TypeProfileKey(TypeProfileKey::StaticPropName, name),
               m_stack.top()->m_type);
  }
}

OPTBLD_INLINE void VMExecutionContext::iopCGetM(PC& pc) {
  PC oldPC = pc;
  NEXT();
  DECLARE_GETHELPER_ARGS
  getHelper(GETHELPER_ARGS);
  if (tvRet->m_type == KindOfRef) {
    tvUnbox(tvRet);
  }
  assert(hasImmVector(toOp(*oldPC)));
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

OPTBLD_INLINE void VMExecutionContext::iopVGetL(PC& pc) {
  NEXT();
  DECODE_LA(local);
  Ref* to = m_stack.allocV();
  TypedValue* fr = frame_local(m_fp, local);
  vgetl_body(fr, to);
}

OPTBLD_INLINE void VMExecutionContext::iopVGetN(PC& pc) {
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

OPTBLD_INLINE void VMExecutionContext::iopVGetG(PC& pc) {
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

OPTBLD_INLINE void VMExecutionContext::iopVGetS(PC& pc) {
  StringData* name;
  GETS(true);
}
#undef GETS

OPTBLD_INLINE void VMExecutionContext::iopVGetM(PC& pc) {
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

OPTBLD_INLINE void VMExecutionContext::iopIssetN(PC& pc) {
  NEXT();
  StringData* name;
  TypedValue* tv1 = m_stack.topTV();
  TypedValue* tv = nullptr;
  bool e;
  lookup_var(m_fp, name, tv1, tv);
  if (tv == nullptr) {
    e = false;
  } else {
    e = !tvIsNull(tvToCell(tv));
  }
  tvRefcountedDecRefCell(tv1);
  tv1->m_data.num = e;
  tv1->m_type = KindOfBoolean;
  decRefStr(name);
}

OPTBLD_INLINE void VMExecutionContext::iopIssetG(PC& pc) {
  NEXT();
  StringData* name;
  TypedValue* tv1 = m_stack.topTV();
  TypedValue* tv = nullptr;
  bool e;
  lookup_gbl(m_fp, name, tv1, tv);
  if (tv == nullptr) {
    e = false;
  } else {
    e = !tvIsNull(tvToCell(tv));
  }
  tvRefcountedDecRefCell(tv1);
  tv1->m_data.num = e;
  tv1->m_type = KindOfBoolean;
  decRefStr(name);
}

OPTBLD_INLINE void VMExecutionContext::iopIssetS(PC& pc) {
  StringData* name;
  SPROP_OP_PRELUDE
  bool e;
  if (!(visible && accessible)) {
    e = false;
  } else {
    e = !tvIsNull(tvToCell(val));
  }
  m_stack.popA();
  output->m_data.num = e;
  output->m_type = KindOfBoolean;
  SPROP_OP_POSTLUDE
}

template <bool isEmpty>
OPTBLD_INLINE void VMExecutionContext::isSetEmptyM(PC& pc) {
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
        base, curMember);
    break;
  }
  case MPL:
  case MPC:
  case MPT: {
    Class* ctx = arGetContextClass(m_fp);
    isSetEmptyResult = IssetEmptyProp<isEmpty>(ctx, base, curMember);
    break;
  }
  default: assert(false);
  }
  getHelperPost<false>(GETHELPERPOST_ARGS);
  tvRet->m_data.num = isSetEmptyResult;
  tvRet->m_type = KindOfBoolean;
}

OPTBLD_INLINE void VMExecutionContext::iopIssetM(PC& pc) {
  isSetEmptyM<false>(pc);
}

#define IOP_TYPE_CHECK_INSTR_L(checkInit, what, predicate)          \
OPTBLD_INLINE void VMExecutionContext::iopIs ## what ## L(PC& pc) { \
  NEXT();                                                           \
  DECODE_LA(local);                                                 \
  TypedValue* tv = frame_local(m_fp, local);                        \
  if (checkInit && tv->m_type == KindOfUninit) {                    \
    raise_undefined_local(m_fp, local);                             \
  }                                                                 \
  bool ret = predicate(tvAsCVarRef(tv));                            \
  TypedValue* topTv = m_stack.allocTV();                            \
  topTv->m_data.num = ret;                                          \
  topTv->m_type = KindOfBoolean;                                    \
}                                                                   \

#define IOP_TYPE_CHECK_INSTR_C(checkInit, what, predicate)          \
OPTBLD_INLINE void VMExecutionContext::iopIs ## what ## C(PC& pc) { \
  NEXT();                                                           \
  TypedValue* topTv = m_stack.topTV();                              \
  assert(topTv->m_type != KindOfRef);                               \
  bool ret = predicate(tvAsCVarRef(topTv));                         \
  tvRefcountedDecRefCell(topTv);                                    \
  topTv->m_data.num = ret;                                          \
  topTv->m_type = KindOfBoolean;                                    \
}

#define IOP_TYPE_CHECK_INSTR(checkInit, what, predicate)          \
  IOP_TYPE_CHECK_INSTR_L(checkInit, what, predicate)              \
  IOP_TYPE_CHECK_INSTR_C(checkInit, what, predicate)              \

IOP_TYPE_CHECK_INSTR_L(false, set, is_not_null)
IOP_TYPE_CHECK_INSTR(true,   Null, is_null)
IOP_TYPE_CHECK_INSTR(true,  Array, is_array)
IOP_TYPE_CHECK_INSTR(true, String, is_string)
IOP_TYPE_CHECK_INSTR(true, Object, is_object)
IOP_TYPE_CHECK_INSTR(true,    Int, is_int)
IOP_TYPE_CHECK_INSTR(true, Double, is_double)
IOP_TYPE_CHECK_INSTR(true,   Bool, is_bool)
#undef IOP_TYPE_CHECK_INSTR

OPTBLD_INLINE void VMExecutionContext::iopEmptyL(PC& pc) {
  NEXT();
  DECODE_LA(local);
  TypedValue* loc = frame_local(m_fp, local);
  bool e = !cellToBool(*tvToCell(loc));
  TypedValue* tv1 = m_stack.allocTV();
  tv1->m_data.num = e;
  tv1->m_type = KindOfBoolean;
}

OPTBLD_INLINE void VMExecutionContext::iopEmptyN(PC& pc) {
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
  tvRefcountedDecRefCell(tv1);
  tv1->m_data.num = e;
  tv1->m_type = KindOfBoolean;
  decRefStr(name);
}

OPTBLD_INLINE void VMExecutionContext::iopEmptyG(PC& pc) {
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
  tvRefcountedDecRefCell(tv1);
  tv1->m_data.num = e;
  tv1->m_type = KindOfBoolean;
  decRefStr(name);
}

OPTBLD_INLINE void VMExecutionContext::iopEmptyS(PC& pc) {
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

OPTBLD_INLINE void VMExecutionContext::iopEmptyM(PC& pc) {
  isSetEmptyM<true>(pc);
}

OPTBLD_INLINE void VMExecutionContext::iopAKExists(PC& pc) {
  NEXT();
  TypedValue* arr = m_stack.topTV();
  TypedValue* key = arr + 1;
  bool result = f_array_key_exists(tvAsCVarRef(key), tvAsCVarRef(arr));
  m_stack.popTV();
  tvRefcountedDecRef(key);
  key->m_data.num = result;
  key->m_type = KindOfBoolean;
}

OPTBLD_INLINE void VMExecutionContext::iopArrayIdx(PC& pc) {
  NEXT();
  TypedValue* def = m_stack.topTV();
  TypedValue* arr = m_stack.indTV(1);
  TypedValue* key = m_stack.indTV(2);

  Variant result = f_hphp_array_idx(tvAsCVarRef(key),
                                    tvAsCVarRef(arr),
                                    tvAsCVarRef(def));
  m_stack.popTV();
  m_stack.popTV();
  tvAsVariant(key) = result;
}

OPTBLD_INLINE void VMExecutionContext::iopSetL(PC& pc) {
  NEXT();
  DECODE_LA(local);
  assert(local < m_fp->m_func->numLocals());
  Cell* fr = m_stack.topC();
  TypedValue* to = frame_local(m_fp, local);
  tvSet(*fr, *to);
}

OPTBLD_INLINE void VMExecutionContext::iopSetN(PC& pc) {
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

OPTBLD_INLINE void VMExecutionContext::iopSetG(PC& pc) {
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

OPTBLD_INLINE void VMExecutionContext::iopSetS(PC& pc) {
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

OPTBLD_INLINE void VMExecutionContext::iopSetM(PC& pc) {
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
        StringData* result = SetElem<true>(base, curMember, c1);
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
        SetProp<true>(ctx, base, curMember, c1);
        break;
      }
      default: assert(false);
      }
    }
  }
  setHelperPost<1>(SETHELPERPOST_ARGS);
}

OPTBLD_INLINE void VMExecutionContext::iopSetWithRefLM(PC& pc) {
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

OPTBLD_INLINE void VMExecutionContext::iopSetWithRefRM(PC& pc) {
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

OPTBLD_INLINE void VMExecutionContext::iopSetOpL(PC& pc) {
  NEXT();
  DECODE_LA(local);
  DECODE(unsigned char, op);
  Cell* fr = m_stack.topC();
  Cell* to = tvToCell(frame_local(m_fp, local));
  SETOP_BODY_CELL(to, op, fr);
  tvRefcountedDecRefCell(fr);
  cellDup(*to, *fr);
}

OPTBLD_INLINE void VMExecutionContext::iopSetOpN(PC& pc) {
  NEXT();
  DECODE(unsigned char, op);
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

OPTBLD_INLINE void VMExecutionContext::iopSetOpG(PC& pc) {
  NEXT();
  DECODE(unsigned char, op);
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

OPTBLD_INLINE void VMExecutionContext::iopSetOpS(PC& pc) {
  NEXT();
  DECODE(unsigned char, op);
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

OPTBLD_INLINE void VMExecutionContext::iopSetOpM(PC& pc) {
  NEXT();
  DECODE(unsigned char, op);
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
            curMember, rhs);
        break;
      case MPL:
      case MPC:
      case MPT: {
        Class *ctx = arGetContextClass(m_fp);
        result = SetOpProp(tvScratch, *tvRef.asTypedValue(), ctx, op, base,
            curMember, rhs);
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

OPTBLD_INLINE void VMExecutionContext::iopIncDecL(PC& pc) {
  NEXT();
  DECODE_LA(local);
  DECODE(unsigned char, op);
  TypedValue* to = m_stack.allocTV();
  tvWriteUninit(to);
  TypedValue* fr = frame_local(m_fp, local);
  IncDecBody<true>(op, fr, to);
}

OPTBLD_INLINE void VMExecutionContext::iopIncDecN(PC& pc) {
  NEXT();
  DECODE(unsigned char, op);
  StringData* name;
  TypedValue* nameCell = m_stack.topTV();
  TypedValue* local = nullptr;
  // XXX We're probably not getting warnings totally correct here
  lookupd_var(m_fp, name, nameCell, local);
  assert(local != nullptr);
  IncDecBody<true>(op, local, nameCell);
  decRefStr(name);
}

OPTBLD_INLINE void VMExecutionContext::iopIncDecG(PC& pc) {
  NEXT();
  DECODE(unsigned char, op);
  StringData* name;
  TypedValue* nameCell = m_stack.topTV();
  TypedValue* gbl = nullptr;
  // XXX We're probably not getting warnings totally correct here
  lookupd_gbl(m_fp, name, nameCell, gbl);
  assert(gbl != nullptr);
  IncDecBody<true>(op, gbl, nameCell);
  decRefStr(name);
}

OPTBLD_INLINE void VMExecutionContext::iopIncDecS(PC& pc) {
  StringData* name;
  SPROP_OP_PRELUDE
  DECODE(unsigned char, op);
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

OPTBLD_INLINE void VMExecutionContext::iopIncDecM(PC& pc) {
  NEXT();
  DECODE(unsigned char, op);
  DECLARE_SETHELPER_ARGS
  TypedValue to;
  tvWriteUninit(&to);
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
            curMember, to);
        break;
      case MPL:
      case MPC:
      case MPT: {
        Class* ctx = arGetContextClass(m_fp);
        IncDecProp<true>(tvScratch, *tvRef.asTypedValue(), ctx, op, base,
            curMember, to);
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

OPTBLD_INLINE void VMExecutionContext::iopBindL(PC& pc) {
  NEXT();
  DECODE_LA(local);
  Ref* fr = m_stack.topV();
  TypedValue* to = frame_local(m_fp, local);
  tvBind(fr, to);
}

OPTBLD_INLINE void VMExecutionContext::iopBindN(PC& pc) {
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

OPTBLD_INLINE void VMExecutionContext::iopBindG(PC& pc) {
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

OPTBLD_INLINE void VMExecutionContext::iopBindS(PC& pc) {
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

OPTBLD_INLINE void VMExecutionContext::iopBindM(PC& pc) {
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

OPTBLD_INLINE void VMExecutionContext::iopUnsetL(PC& pc) {
  NEXT();
  DECODE_LA(local);
  assert(local < m_fp->m_func->numLocals());
  TypedValue* tv = frame_local(m_fp, local);
  tvRefcountedDecRef(tv);
  tvWriteUninit(tv);
}

OPTBLD_INLINE void VMExecutionContext::iopUnsetN(PC& pc) {
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

OPTBLD_INLINE void VMExecutionContext::iopUnsetG(PC& pc) {
  NEXT();
  TypedValue* tv1 = m_stack.topTV();
  StringData* name = lookup_name(tv1);
  VarEnv* varEnv = m_globalVarEnv;
  assert(varEnv != nullptr);
  varEnv->unset(name);
  m_stack.popC();
  decRefStr(name);
}

OPTBLD_INLINE void VMExecutionContext::iopUnsetM(PC& pc) {
  NEXT();
  DECLARE_SETHELPER_ARGS
  if (!setHelperPre<false, false, true, false, 0,
      VectorLeaveCode::LeaveLast>(MEMBERHELPERPRE_ARGS)) {
    switch (mcode) {
    case MEL:
    case MEC:
    case MET:
    case MEI:
      UnsetElem(base, curMember);
      break;
    case MPL:
    case MPC:
    case MPT: {
      Class* ctx = arGetContextClass(m_fp);
      UnsetProp(ctx, base, curMember);
      break;
    }
    default: assert(false);
    }
  }
  setHelperPost<0>(SETHELPERPOST_ARGS);
}

OPTBLD_INLINE ActRec* VMExecutionContext::fPushFuncImpl(
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

OPTBLD_INLINE void VMExecutionContext::iopFPushFunc(PC& pc) {
  NEXT();
  DECODE_IVA(numArgs);
  Cell* c1 = m_stack.topC();
  const Func* func = nullptr;
  ObjectData* origObj = nullptr;
  StringData* origSd = nullptr;
  if (IS_STRING_TYPE(c1->m_type)) {
    origSd = c1->m_data.pstr;
    func = Unit::loadFunc(origSd);
  } else if (c1->m_type == KindOfObject) {
    static StringData* invokeName = makeStaticString("__invoke");
    origObj = c1->m_data.pobj;
    const Class* cls = origObj->getVMClass();
    func = cls->lookupMethod(invokeName);
    if (func == nullptr) {
      raise_error(Strings::FUNCTION_NAME_MUST_BE_STRING);
    }
  } else {
    raise_error(Strings::FUNCTION_NAME_MUST_BE_STRING);
  }
  if (func == nullptr) {
    raise_error("Undefined function: %s", c1->m_data.pstr->data());
  }
  assert(!origObj || !origSd);
  assert(origObj || origSd);
  // We've already saved origObj or origSd; we'll use them after
  // overwriting the pointer on the stack.  Don't refcount it now; defer
  // till after we're done with it.
  m_stack.discard();
  ActRec* ar = fPushFuncImpl(func, numArgs);
  if (origObj) {
    if (func->attrs() & AttrStatic && !func->isClosureBody()) {
      ar->setClass(origObj->getVMClass());
      decRefObj(origObj);
    } else {
      ar->setThis(origObj);
      // Teleport the reference from the destroyed stack cell to the
      // ActRec. Don't try this at home.
    }
  } else {
    ar->setThis(nullptr);
    decRefStr(origSd);
  }
}

OPTBLD_INLINE void VMExecutionContext::iopFPushFuncD(PC& pc) {
  NEXT();
  DECODE_IVA(numArgs);
  DECODE(Id, id);
  const NamedEntityPair nep = m_fp->m_func->unit()->lookupNamedEntityPairId(id);
  Func* func = Unit::loadFunc(nep.second, nep.first);
  if (func == nullptr) {
    raise_error("Undefined function: %s",
                m_fp->m_func->unit()->lookupLitstrId(id)->data());
  }
  ActRec* ar = fPushFuncImpl(func, numArgs);
  ar->setThis(nullptr);
}

OPTBLD_INLINE void VMExecutionContext::iopFPushFuncU(PC& pc) {
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

void VMExecutionContext::fPushObjMethodImpl(
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

OPTBLD_INLINE void VMExecutionContext::iopFPushObjMethod(PC& pc) {
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

OPTBLD_INLINE void VMExecutionContext::iopFPushObjMethodD(PC& pc) {
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
void VMExecutionContext::pushClsMethodImpl(Class* cls,
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
      /* Propogate the current late bound class if there is one, */
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

OPTBLD_INLINE void VMExecutionContext::iopFPushClsMethod(PC& pc) {
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

OPTBLD_INLINE void VMExecutionContext::iopFPushClsMethodD(PC& pc) {
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

OPTBLD_INLINE void VMExecutionContext::iopFPushClsMethodF(PC& pc) {
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

OPTBLD_INLINE void VMExecutionContext::iopFPushCtor(PC& pc) {
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
  ar->initNumArgs(numArgs, true /* isFPushCtor */);
  arSetSfp(ar, m_fp);
  ar->setVarEnv(nullptr);
}

OPTBLD_INLINE void VMExecutionContext::iopFPushCtorD(PC& pc) {
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
  ar->initNumArgs(numArgs, true /* isFPushCtor */);
  ar->setVarEnv(nullptr);
}

OPTBLD_INLINE void VMExecutionContext::iopDecodeCufIter(PC& pc) {
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

OPTBLD_INLINE void VMExecutionContext::iopFPushCufIter(PC& pc) {
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
  ar->initNumArgs(numArgs, false /* isFPushCtor */);
}

OPTBLD_INLINE void VMExecutionContext::doFPushCuf(PC& pc,
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
  ar->initNumArgs(numArgs, false /* isFPushCtor */);
  if (invName) {
    ar->setInvName(invName);
  } else {
    ar->setVarEnv(nullptr);
  }
  tvRefcountedDecRef(&func);
}

OPTBLD_INLINE void VMExecutionContext::iopFPushCuf(PC& pc) {
  doFPushCuf(pc, false, false);
}

OPTBLD_INLINE void VMExecutionContext::iopFPushCufF(PC& pc) {
  doFPushCuf(pc, true, false);
}

OPTBLD_INLINE void VMExecutionContext::iopFPushCufSafe(PC& pc) {
  doFPushCuf(pc, false, true);
}

static inline ActRec* arFromInstr(TypedValue* sp, const Op* pc) {
  return arFromSpOffset((ActRec*)sp, instrSpToArDelta(pc));
}

OPTBLD_INLINE void VMExecutionContext::iopFPassC(PC& pc) {
#ifdef DEBUG
  ActRec* ar = arFromInstr(m_stack.top(), (Op*)pc);
#endif
  NEXT();
  DECODE_IVA(paramId);
#ifdef DEBUG
  assert(paramId < ar->numArgs());
#endif
}

#define FPASSC_CHECKED_PRELUDE                                                \
  ActRec* ar = arFromInstr(m_stack.top(), (Op*)pc);                           \
  NEXT();                                                                     \
  DECODE_IVA(paramId);                                                        \
  assert(paramId < ar->numArgs());                                            \
  const Func* func = ar->m_func;

OPTBLD_INLINE void VMExecutionContext::iopFPassCW(PC& pc) {
  FPASSC_CHECKED_PRELUDE
  if (func->mustBeRef(paramId)) {
    TRACE(1, "FPassCW: function %s(%d) param %d is by reference, "
          "raising a strict warning (attr:0x%x)\n",
          func->name()->data(), func->numParams(), paramId,
          func->info() ? func->info()->attribute : 0);
    raise_strict_warning("Only variables should be passed by reference");
  }
}

OPTBLD_INLINE void VMExecutionContext::iopFPassCE(PC& pc) {
  FPASSC_CHECKED_PRELUDE
  if (func->mustBeRef(paramId)) {
    TRACE(1, "FPassCE: function %s(%d) param %d is by reference, "
          "throwing a fatal error (attr:0x%x)\n",
          func->name()->data(), func->numParams(), paramId,
          func->info() ? func->info()->attribute : 0);
    raise_error("Cannot pass parameter %d by reference", paramId+1);
  }
}

#undef FPASSC_CHECKED_PRELUDE

OPTBLD_INLINE void VMExecutionContext::iopFPassV(PC& pc) {
  ActRec* ar = arFromInstr(m_stack.top(), (Op*)pc);
  NEXT();
  DECODE_IVA(paramId);
  assert(paramId < ar->numArgs());
  const Func* func = ar->m_func;
  if (!func->byRef(paramId)) {
    m_stack.unbox();
  }
}

OPTBLD_INLINE void VMExecutionContext::iopFPassR(PC& pc) {
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

OPTBLD_INLINE void VMExecutionContext::iopFPassL(PC& pc) {
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

OPTBLD_INLINE void VMExecutionContext::iopFPassN(PC& pc) {
  ActRec* ar = arFromInstr(m_stack.top(), (Op*)pc);
  PC origPc = pc;
  NEXT();
  DECODE_IVA(paramId);
  assert(paramId < ar->numArgs());
  if (!ar->m_func->byRef(paramId)) {
    iopCGetN(origPc);
  } else {
    iopVGetN(origPc);
  }
}

OPTBLD_INLINE void VMExecutionContext::iopFPassG(PC& pc) {
  ActRec* ar = arFromInstr(m_stack.top(), (Op*)pc);
  PC origPc = pc;
  NEXT();
  DECODE_IVA(paramId);
  assert(paramId < ar->numArgs());
  if (!ar->m_func->byRef(paramId)) {
    iopCGetG(origPc);
  } else {
    iopVGetG(origPc);
  }
}

OPTBLD_INLINE void VMExecutionContext::iopFPassS(PC& pc) {
  ActRec* ar = arFromInstr(m_stack.top(), (Op*)pc);
  PC origPc = pc;
  NEXT();
  DECODE_IVA(paramId);
  assert(paramId < ar->numArgs());
  if (!ar->m_func->byRef(paramId)) {
    iopCGetS(origPc);
  } else {
    iopVGetS(origPc);
  }
}

void VMExecutionContext::iopFPassM(PC& pc) {
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

bool VMExecutionContext::doFCall(ActRec* ar, PC& pc) {
  assert(getOuterVMFrame(ar) == m_fp);
  ar->m_savedRip =
    reinterpret_cast<uintptr_t>(tx()->uniqueStubs.retHelper);
  assert(isReturnHelper(ar->m_savedRip));
  TRACE(3, "FCall: pc %p func %p base %d\n", m_pc,
        m_fp->m_func->unit()->entry(),
        int(m_fp->m_func->base()));
  ar->m_soff = m_fp->m_func->unit()->offsetOf(pc)
    - (uintptr_t)m_fp->m_func->base();
  assert(pcOff() >= m_fp->m_func->base());
  prepareFuncEntry(ar, pc);
  SYNC();
  if (EventHook::FunctionEnter(ar, EventHook::NormalFunc)) return true;
  pc = m_pc;
  return false;
}

OPTBLD_INLINE void VMExecutionContext::iopFCall(PC& pc) {
  ActRec* ar = arFromInstr(m_stack.top(), (Op*)pc);
  NEXT();
  DECODE_IVA(numArgs);
  assert(numArgs == ar->numArgs());
  checkStack(m_stack, ar->m_func);
  doFCall(ar, pc);
}

OPTBLD_INLINE void VMExecutionContext::iopFCallBuiltin(PC& pc) {
  NEXT();
  DECODE_IVA(numArgs);
  DECODE_IVA(numNonDefault);
  DECODE(Id, id);
  const NamedEntity* ne = m_fp->m_func->unit()->lookupNamedEntityId(id);
  Func* func = Unit::lookupFunc(ne);
  if (func == nullptr) {
    raise_error("Undefined function: %s",
                m_fp->m_func->unit()->lookupLitstrId(id)->data());
  }
  TypedValue* args = m_stack.indTV(numArgs-1);
  TypedValue ret;
  if (Native::coerceFCallArgs(args, numArgs, numNonDefault, func)) {
    Native::callFunc(func, nullptr, args, numArgs, ret);
  } else {
    ret.m_type = KindOfNull;
  }

  frame_free_args(args, numNonDefault);
  m_stack.ndiscard(numArgs);
  tvCopy(ret, *m_stack.allocTV());
}

bool VMExecutionContext::prepareArrayArgs(ActRec* ar, Array& arrayArgs) {
  if (UNLIKELY(ar->hasInvName())) {
    m_stack.pushStringNoRc(ar->getInvName());
    if (UNLIKELY(!arrayArgs.get()->isVectorData())) {
      arrayArgs = arrayArgs.values();
    }
    m_stack.pushArray(arrayArgs.get());
    ar->setVarEnv(0);
    ar->initNumArgs(2);
    return true;
  }

  ArrayData* args = arrayArgs.get();

  int nargs = args->size();
  const Func* f = ar->m_func;
  int nparams = f->numParams();
  int extra = nargs - nparams;
  if (extra < 0) {
    extra = 0;
    nparams = nargs;
  }
  ssize_t pos = args->iter_begin();
  for (int i = 0; i < nparams; ++i) {
    TypedValue* from = const_cast<TypedValue*>(
      args->getValueRef(pos).asTypedValue());
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
    pos = args->iter_advance(pos);
  }
  if (extra && (ar->m_func->attrs() & AttrMayUseVV)) {
    ExtraArgs* extraArgs = ExtraArgs::allocateUninit(extra);
    for (int i = 0; i < extra; ++i) {
      TypedValue* to = extraArgs->getExtraArg(i);
      const TypedValue* from = args->getValueRef(pos).asTypedValue();
      if (from->m_type == KindOfRef && from->m_data.pref->isReferenced()) {
        refDup(*from, *to);
      } else {
        cellDup(*tvToCell(from), *to);
      }
      pos = args->iter_advance(pos);
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

bool VMExecutionContext::doFCallArray(PC& pc) {
  ActRec* ar = (ActRec*)(m_stack.top() + 1);
  assert(ar->numArgs() == 1);

  Cell* c1 = m_stack.topC();
  if (skipCufOnInvalidParams && UNLIKELY(c1->m_type != KindOfArray)) {
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
    Array args(LIKELY(c1->m_type == KindOfArray) ? c1->m_data.parr :
               tvAsVariant(c1).toArray().get());
    m_stack.popTV();
    checkStack(m_stack, func);

    assert(ar->m_savedRbp == (uint64_t)m_fp);
    assert(!ar->m_func->isGenerator());
    ar->m_savedRip =
      reinterpret_cast<uintptr_t>(tx()->uniqueStubs.retHelper);
    assert(isReturnHelper(ar->m_savedRip));
    TRACE(3, "FCallArray: pc %p func %p base %d\n", m_pc,
          m_fp->unit()->entry(),
          int(m_fp->m_func->base()));
    ar->m_soff = m_fp->unit()->offsetOf(pc)
      - (uintptr_t)m_fp->m_func->base();
    assert(pcOff() > m_fp->m_func->base());

    if (UNLIKELY(!prepareArrayArgs(ar, args))) return false;
  }

  if (UNLIKELY(!(prepareFuncEntry(ar, pc)))) {
    return false;
  }
  SYNC();
  if (UNLIKELY(!EventHook::FunctionEnter(ar, EventHook::NormalFunc))) {
    pc = m_pc;
    return false;
  }
  return true;
}

bool VMExecutionContext::doFCallArrayTC(PC pc) {
  Util::assert_native_stack_aligned();
  assert(tl_regState == VMRegState::DIRTY);
  tl_regState = VMRegState::CLEAN;
  auto const ret = doFCallArray(pc);
  tl_regState = VMRegState::DIRTY;
  return ret;
}

OPTBLD_INLINE void VMExecutionContext::iopFCallArray(PC& pc) {
  NEXT();
  (void)doFCallArray(pc);
}

OPTBLD_INLINE void VMExecutionContext::iopCufSafeArray(PC& pc) {
  NEXT();
  Array ret;
  ret.append(tvAsVariant(m_stack.top() + 1));
  ret.appendWithRef(tvAsVariant(m_stack.top() + 0));
  m_stack.popTV();
  m_stack.popTV();
  tvAsVariant(m_stack.top()) = ret;
}

OPTBLD_INLINE void VMExecutionContext::iopCufSafeReturn(PC& pc) {
  NEXT();
  bool ok = cellToBool(*tvToCell(m_stack.top() + 1));
  tvRefcountedDecRef(m_stack.top() + 1);
  tvRefcountedDecRef(m_stack.top() + (ok ? 2 : 0));
  if (ok) m_stack.top()[2] = m_stack.top()[0];
  m_stack.ndiscard(2);
}

inline bool VMExecutionContext::initIterator(PC& pc, PC& origPc, Iter* it,
                                             Offset offset, Cell* c1) {
  bool hasElems = it->init(c1);
  if (!hasElems) {
    ITER_SKIP(offset);
  }
  m_stack.popC();
  return hasElems;
}

OPTBLD_INLINE void VMExecutionContext::iopIterInit(PC& pc) {
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

OPTBLD_INLINE void VMExecutionContext::iopIterInitK(PC& pc) {
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

OPTBLD_INLINE void VMExecutionContext::iopWIterInit(PC& pc) {
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

OPTBLD_INLINE void VMExecutionContext::iopWIterInitK(PC& pc) {
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


inline bool VMExecutionContext::initIteratorM(PC& pc, PC& origPc, Iter* it,
                                              Offset offset, Ref* r1,
                                              TypedValue *val,
                                              TypedValue *key) {
  bool hasElems = false;
  TypedValue* rtv = r1->m_data.pref->tv();
  if (rtv->m_type == KindOfArray) {
    hasElems = new_miter_array_key(it, r1->m_data.pref, val, key);
  } else if (rtv->m_type == KindOfObject)  {
    Class* ctx = arGetContextClass(g_vmContext->getFP());
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

OPTBLD_INLINE void VMExecutionContext::iopMIterInit(PC& pc) {
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

OPTBLD_INLINE void VMExecutionContext::iopMIterInitK(PC& pc) {
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

OPTBLD_INLINE void VMExecutionContext::iopIterNext(PC& pc) {
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

OPTBLD_INLINE void VMExecutionContext::iopIterNextK(PC& pc) {
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

OPTBLD_INLINE void VMExecutionContext::iopWIterNext(PC& pc) {
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

OPTBLD_INLINE void VMExecutionContext::iopWIterNextK(PC& pc) {
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

OPTBLD_INLINE void VMExecutionContext::iopMIterNext(PC& pc) {
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

OPTBLD_INLINE void VMExecutionContext::iopMIterNextK(PC& pc) {
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

OPTBLD_INLINE void VMExecutionContext::iopIterFree(PC& pc) {
  NEXT();
  DECODE_IA(itId);
  Iter* it = frame_iter(m_fp, itId);
  it->free();
}

OPTBLD_INLINE void VMExecutionContext::iopMIterFree(PC& pc) {
  NEXT();
  DECODE_IA(itId);
  Iter* it = frame_iter(m_fp, itId);
  it->mfree();
}

OPTBLD_INLINE void VMExecutionContext::iopCIterFree(PC& pc) {
  NEXT();
  DECODE_IA(itId);
  Iter* it = frame_iter(m_fp, itId);
  it->cfree();
}

OPTBLD_INLINE void inclOp(VMExecutionContext *ec, PC &pc, InclOpFlags flags) {
  NEXT();
  Cell* c1 = ec->m_stack.topC();
  String path(prepareKey(c1));
  bool initial;
  TRACE(2, "inclOp %s %s %s %s \"%s\"\n",
        flags & InclOpOnce ? "Once" : "",
        flags & InclOpDocRoot ? "DocRoot" : "",
        flags & InclOpRelative ? "Relative" : "",
        flags & InclOpFatal ? "Fatal" : "",
        path->data());

  Unit* u = flags & (InclOpDocRoot|InclOpRelative) ?
    ec->evalIncludeRoot(path.get(), flags, &initial) :
    ec->evalInclude(path.get(), ec->m_fp->m_func->unit()->filepath(), &initial);
  ec->m_stack.popC();
  if (u == nullptr) {
    ((flags & InclOpFatal) ?
     (void (*)(const char *, ...))raise_error :
     (void (*)(const char *, ...))raise_warning)("File not found: %s",
                                                 path->data());
    ec->m_stack.pushFalse();
  } else {
    if (!(flags & InclOpOnce) || initial) {
      ec->evalUnit(u, pc, EventHook::PseudoMain);
    } else {
      Stats::inc(Stats::PseudoMain_Guarded);
      ec->m_stack.pushTrue();
    }
  }
}

OPTBLD_INLINE void VMExecutionContext::iopIncl(PC& pc) {
  inclOp(this, pc, InclOpDefault);
}

OPTBLD_INLINE void VMExecutionContext::iopInclOnce(PC& pc) {
  inclOp(this, pc, InclOpOnce);
}

OPTBLD_INLINE void VMExecutionContext::iopReq(PC& pc) {
  inclOp(this, pc, InclOpFatal);
}

OPTBLD_INLINE void VMExecutionContext::iopReqOnce(PC& pc) {
  inclOp(this, pc, InclOpFatal | InclOpOnce);
}

OPTBLD_INLINE void VMExecutionContext::iopReqDoc(PC& pc) {
  inclOp(this, pc, InclOpFatal | InclOpOnce | InclOpDocRoot);
}

OPTBLD_INLINE void VMExecutionContext::iopEval(PC& pc) {
  NEXT();
  Cell* c1 = m_stack.topC();
  String code(prepareKey(c1));
  String prefixedCode = concat("<?php ", code);
  Unit* unit = compileEvalString(prefixedCode.get());
  if (unit == nullptr) {
    raise_error("Syntax error in eval()");
  }
  m_stack.popC();
  evalUnit(unit, pc, EventHook::Eval);
}

OPTBLD_INLINE void VMExecutionContext::iopDefFunc(PC& pc) {
  NEXT();
  DECODE_IVA(fid);
  Func* f = m_fp->m_func->unit()->lookupFuncId(fid);
  f->setCached();
}

OPTBLD_INLINE void VMExecutionContext::iopDefCls(PC& pc) {
  NEXT();
  DECODE_IVA(cid);
  PreClass* c = m_fp->m_func->unit()->lookupPreClassId(cid);
  Unit::defClass(c);
}

OPTBLD_INLINE void VMExecutionContext::iopDefTypedef(PC& pc) {
  NEXT();
  DECODE_IVA(tid);
  m_fp->m_func->unit()->defTypedef(tid);
}

static inline void checkThis(ActRec* fp) {
  if (!fp->hasThis()) {
    raise_error(Strings::FATAL_NULL_THIS);
  }
}

OPTBLD_INLINE void VMExecutionContext::iopThis(PC& pc) {
  NEXT();
  checkThis(m_fp);
  ObjectData* this_ = m_fp->getThis();
  m_stack.pushObject(this_);
}

OPTBLD_INLINE void VMExecutionContext::iopBareThis(PC& pc) {
  NEXT();
  DECODE(unsigned char, notice);
  if (m_fp->hasThis()) {
    ObjectData* this_ = m_fp->getThis();
    m_stack.pushObject(this_);
  } else {
    m_stack.pushNull();
    if (notice) raise_notice(Strings::WARN_NULL_THIS);
  }
}

OPTBLD_INLINE void VMExecutionContext::iopCheckThis(PC& pc) {
  NEXT();
  checkThis(m_fp);
}

OPTBLD_INLINE void VMExecutionContext::iopInitThisLoc(PC& pc) {
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

/*
 * Helper for StaticLoc and StaticLocInit.
 */
static inline void
lookupStatic(StringData* name,
             const ActRec* fp,
             TypedValue*&val, bool& inited) {
  HphpArray* map = get_static_locals(fp);
  assert(map != nullptr);
  val = map->nvGet(name);
  if (val == nullptr) {
    TypedValue tv;
    tvWriteUninit(&tv);
    // TODO(#2887942): need write barrier
    map->set(name, tvAsCVarRef(&tv), false);
    val = map->nvGet(name);
    inited = false;
  } else {
    inited = true;
  }
}

OPTBLD_INLINE void VMExecutionContext::iopStaticLoc(PC& pc) {
  NEXT();
  DECODE_IVA(localId);
  DECODE_LITSTR(var);
  TypedValue* fr = nullptr;
  bool inited;
  lookupStatic(var, m_fp, fr, inited);
  assert(fr != nullptr);
  if (fr->m_type != KindOfRef) {
    assert(!inited);
    tvBox(fr);
  }
  TypedValue* tvLocal = frame_local(m_fp, localId);
  tvBind(fr, tvLocal);
  if (inited) {
    m_stack.pushTrue();
  } else {
    m_stack.pushFalse();
  }
}

OPTBLD_INLINE void VMExecutionContext::iopStaticLocInit(PC& pc) {
  NEXT();
  DECODE_IVA(localId);
  DECODE_LITSTR(var);
  TypedValue* fr = nullptr;
  bool inited;
  lookupStatic(var, m_fp, fr, inited);
  assert(fr != nullptr);
  assert(!inited || fr->m_type == KindOfRef);
  if (!inited) {
    Cell* initVal = m_stack.topC();
    cellDup(*initVal, *fr);
  }
  if (fr->m_type != KindOfRef) {
    assert(!inited);
    tvBox(fr);
  }
  TypedValue* tvLocal = frame_local(m_fp, localId);
  tvBind(fr, tvLocal);
  m_stack.discard();
}

OPTBLD_INLINE void VMExecutionContext::iopCatch(PC& pc) {
  NEXT();
  assert(m_faults.size() > 0);
  Fault fault = m_faults.back();
  m_faults.pop_back();
  assert(fault.m_faultType == Fault::Type::UserException);
  m_stack.pushObjectNoRc(fault.m_userException);
}

OPTBLD_INLINE void VMExecutionContext::iopLateBoundCls(PC& pc) {
  NEXT();
  Class* cls = frameStaticClass(m_fp);
  if (!cls) {
    raise_error(HPHP::Strings::CANT_ACCESS_STATIC);
  }
  m_stack.pushClass(cls);
}

OPTBLD_INLINE void VMExecutionContext::iopVerifyParamType(PC& pc) {
  SYNC(); // We might need m_pc to be updated to throw.
  NEXT();

  DECODE_IVA(param);
  const Func *func = m_fp->m_func;
  assert(param < func->numParams());
  assert(func->numParams() == int(func->params().size()));
  const TypeConstraint& tc = func->params()[param].typeConstraint();
  assert(tc.hasConstraint());
  if (UNLIKELY(!RuntimeOption::EvalCheckExtendedTypeHints &&
               tc.isExtended())) {
    return;
  }
  const TypedValue *tv = frame_local(m_fp, param);
  tc.verify(tv, func, param);
}

OPTBLD_INLINE void VMExecutionContext::iopNativeImpl(PC& pc) {
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
              Trace::trace("Return %s from VMExecutionContext::dispatch("
                           "%p)\n", os.str().c_str(), m_fp));
    }
#endif
    pc = 0;
  }
}

OPTBLD_INLINE void VMExecutionContext::iopHighInvalid(PC& pc) {
  fprintf(stderr, "invalid bytecode executed\n");
  abort();
}

OPTBLD_INLINE void VMExecutionContext::iopSelf(PC& pc) {
  NEXT();
  Class* clss = arGetContextClass(m_fp);
  if (!clss) {
    raise_error(HPHP::Strings::CANT_ACCESS_SELF);
  }
  m_stack.pushClass(clss);
}

OPTBLD_INLINE void VMExecutionContext::iopParent(PC& pc) {
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

OPTBLD_INLINE void VMExecutionContext::iopCreateCl(PC& pc) {
  NEXT();
  DECODE_IVA(numArgs);
  DECODE_LITSTR(clsName);
  auto const cls = Unit::loadClass(clsName);
  auto const cl = static_cast<c_Closure*>(newInstance(cls));
  cl->init(numArgs, m_fp, m_stack.top());
  m_stack.ndiscard(numArgs);
  m_stack.pushObject(cl);
}

static inline c_Continuation* createCont(const Func* origFunc,
                                         const Func* genFunc) {
  auto const cont = c_Continuation::alloc(origFunc, genFunc);
  cont->incRefCount();
  cont->setNoDestruct();

  // The ActRec corresponding to the generator body lives as long as the object
  // does. We set it up once, here, and then just change FP to point to it when
  // we enter the generator body.
  ActRec* ar = cont->actRec();
  ar->m_func = genFunc;
  ar->initNumArgs(0);
  ar->setVarEnv(nullptr);

  return cont;
}

c_Continuation*
VMExecutionContext::createContFunc(const Func* origFunc,
                                   const Func* genFunc) {
  auto cont = createCont(origFunc, genFunc);
  cont->actRec()->setThis(nullptr);
  return cont;
}

c_Continuation*
VMExecutionContext::createContMeth(const Func* origFunc,
                                   const Func* genFunc,
                                   void* objOrCls) {
  if (origFunc->isClosureBody()) {
    genFunc = genFunc->cloneAndSetClass(origFunc->cls());
  }

  auto cont = createCont(origFunc, genFunc);
  auto ar = cont->actRec();
  ar->setThisOrClass(objOrCls);
  if (ar->hasThis()) {
    ar->getThis()->incRefCount();
  }
  return cont;
}

static inline void setContVar(const Func* genFunc,
                              const StringData* name,
                              TypedValue* src,
                              ActRec* genFp) {
  Id destId = genFunc->lookupVarId(name);
  if (destId != kInvalidId) {
    // Copy the value of the local to the cont object and set the
    // local to uninit so that we don't need to change refcounts.
    tvCopy(*src, *frame_local(genFp, destId));
    tvWriteUninit(src);
  } else {
    if (!genFp->hasVarEnv()) {
      // We pass skipInsert to this VarEnv because it's going to exist
      // independent of the chain; i.e. we can't stack-allocate it. We link it
      // into the chain in UnpackCont, and take it out in ContSuspend.
      genFp->setVarEnv(VarEnv::createLocalOnHeap(genFp));
    }
    genFp->getVarEnv()->setWithRef(name, src);
  }
}

const StaticString s_this("this");

void VMExecutionContext::fillContinuationVars(ActRec* origFp,
                                              const Func* origFunc,
                                              ActRec* genFp,
                                              const Func* genFunc) {
  // For functions that contain only named locals, the variable
  // environment is saved and restored by teleporting the values (and
  // their references) between the evaluation stack and the local
  // space at the end of the object using memcpy. Any variables in a
  // VarEnv are saved and restored from m_vars as usual.
  static const StringData* thisStr = s_this.get();
  bool skipThis;
  if (origFp->hasVarEnv()) {
    // This is currently never executed but it will be needed for eager
    // execution of async functions - should be revisited later.
    assert(false);
    Stats::inc(Stats::Cont_CreateVerySlow);
    Array definedVariables = origFp->getVarEnv()->getDefinedVariables();
    skipThis = definedVariables.exists(s_this, true);

    for (ArrayIter iter(definedVariables); !iter.end(); iter.next()) {
      setContVar(genFunc, iter.first().getStringData(),
        const_cast<TypedValue*>(iter.secondRef().asTypedValue()), genFp);
    }
  } else {
    skipThis = origFunc->lookupVarId(thisStr) != kInvalidId;
    for (Id i = 0; i < origFunc->numNamedLocals(); ++i) {
      assert(i == genFunc->lookupVarId(origFunc->localVarName(i)));
      TypedValue* src = frame_local(origFp, i);
      tvCopy(*src, *frame_local(genFp, i));
      tvWriteUninit(src);
    }
  }

  // If $this is used as a local inside the body and is not provided
  // by our containing environment, just prefill it here instead of
  // using InitThisLoc inside the body
  if (!skipThis && origFp->hasThis()) {
    Id id = genFunc->lookupVarId(thisStr);
    if (id != kInvalidId) {
      tvAsVariant(frame_local(genFp, id)) = origFp->getThis();
    }
  }
}

OPTBLD_INLINE void VMExecutionContext::iopCreateCont(PC& pc) {
  NEXT();
  DECODE_LITSTR(genName);

  const Func* origFunc = m_fp->m_func;
  const Func* genFunc = origFunc->getGeneratorBody(genName);
  assert(genFunc != nullptr);

  c_Continuation* cont = origFunc->isMethod()
    ? createContMeth(origFunc, genFunc, m_fp->getThisOrClass())
    : createContFunc(origFunc, genFunc);

  fillContinuationVars(m_fp, origFunc, cont->actRec(), genFunc);

  TypedValue* ret = m_stack.allocTV();
  ret->m_type = KindOfObject;
  ret->m_data.pobj = cont;
}

OPTBLD_INLINE void VMExecutionContext::iopCreateAsync(PC& pc) {
  NEXT();
  DECODE_LITSTR(genName);
  DECODE_IVA(label);
  DECODE_IVA(iters);

  const Func* origFunc = m_fp->m_func;
  const Func* genFunc = origFunc->getGeneratorBody(genName);
  assert(genFunc != nullptr);

  c_Continuation* cont = origFunc->isMethod()
    ? createContMeth(origFunc, genFunc, m_fp->getThisOrClass())
    : createContFunc(origFunc, genFunc);

  // TODO: we should check that the value on top of the stack is indeed
  // a WaitHandle and fatal if not. Also, if it is a wait handle, assert
  // that it is not finished.
  cont->t_update(label, tvAsCVarRef(m_stack.topTV()));
  m_stack.popTV();

  fillContinuationVars(m_fp, origFunc, cont->actRec(), genFunc);

  // copy the state of all the iterators at once
  memcpy(frame_iter(cont->actRec(), iters-1),
         frame_iter(m_fp, iters-1),
         iters * sizeof(Iter));

  TypedValue* ret = m_stack.allocTV();
  ret->m_type = KindOfObject;
  ret->m_data.pobj = cont;
}

static inline c_Continuation* this_continuation(const ActRec* fp) {
  ObjectData* obj = fp->getThis();
  assert(obj->instanceof(c_Continuation::classof()));
  return static_cast<c_Continuation*>(obj);
}

void VMExecutionContext::iopContEnter(PC& pc) {
  NEXT();

  // The stack must have one cell! Or else generatorStackBase() won't work!
  assert(m_stack.top() + 1 ==
         (TypedValue*)m_fp - m_fp->m_func->numSlotsInFrame());

  // Do linkage of the continuation's AR.
  assert(m_fp->hasThis());
  c_Continuation* cont = this_continuation(m_fp);
  ActRec* contAR = cont->actRec();
  arSetSfp(contAR, m_fp);

  contAR->m_soff = m_fp->m_func->unit()->offsetOf(pc)
    - (uintptr_t)m_fp->m_func->base();
  contAR->m_savedRip =
    reinterpret_cast<uintptr_t>(tx()->uniqueStubs.genRetHelper);
  assert(isReturnHelper(contAR->m_savedRip));

  m_fp = contAR;
  pc = contAR->m_func->getEntry();
  SYNC();

  if (UNLIKELY(!EventHook::FunctionEnter(contAR, EventHook::NormalFunc))) {
    pc = m_pc;
  }
}

OPTBLD_INLINE void VMExecutionContext::iopUnpackCont(PC& pc) {
  NEXT();
  c_Continuation* cont = frame_continuation(m_fp);

  // check sanity of received value
  assert(tvIsPlausible(*m_stack.topC()));

  // Return the label in a stack cell
  TypedValue* label = m_stack.allocTV();
  label->m_type = KindOfInt64;
  label->m_data.num = cont->m_label;
}

OPTBLD_INLINE void VMExecutionContext::iopContSuspend(PC& pc) {
  NEXT();
  DECODE_IVA(label);
  c_Continuation* cont = frame_continuation(m_fp);

  cont->c_Continuation::t_update(label, tvAsCVarRef(m_stack.topTV()));
  m_stack.popTV();

  EventHook::FunctionExit(m_fp);
  ActRec* prevFp = m_fp->arGetSfp();
  pc = prevFp->m_func->getEntry() + m_fp->m_soff;
  m_fp = prevFp;
}

OPTBLD_INLINE void VMExecutionContext::iopContSuspendK(PC& pc) {
  NEXT();
  DECODE_IVA(label);
  c_Continuation* cont = frame_continuation(m_fp);

  TypedValue* val = m_stack.topTV();
  cont->c_Continuation::t_update_key(label, tvAsCVarRef(m_stack.indTV(1)),
                                     tvAsCVarRef(val));
  m_stack.popTV();
  m_stack.popTV();

  EventHook::FunctionExit(m_fp);
  ActRec* prevFp = m_fp->arGetSfp();
  pc = prevFp->m_func->getEntry() + m_fp->m_soff;
  m_fp = prevFp;
}

OPTBLD_INLINE void VMExecutionContext::iopContRetC(PC& pc) {
  NEXT();
  c_Continuation* cont = frame_continuation(m_fp);
  cont->setDone();
  tvSetIgnoreRef(*m_stack.topC(), *cont->m_value.asTypedValue());
  m_stack.popC();

  EventHook::FunctionExit(m_fp);
  ActRec* prevFp = m_fp->arGetSfp();
  pc = prevFp->m_func->getEntry() + m_fp->m_soff;
  m_fp = prevFp;
}

OPTBLD_INLINE void VMExecutionContext::iopContCheck(PC& pc) {
  NEXT();
  DECODE_IVA(check_started);
  c_Continuation* cont = this_continuation(m_fp);
  if (check_started) {
    cont->startedCheck();
  }
  cont->preNext();
}

OPTBLD_INLINE void VMExecutionContext::iopContRaise(PC& pc) {
  NEXT();
  c_Continuation* cont = this_continuation(m_fp);
  assert(cont->m_label);
  --cont->m_label;
}

OPTBLD_INLINE void VMExecutionContext::iopContValid(PC& pc) {
  NEXT();
  TypedValue* tv = m_stack.allocTV();
  tvWriteUninit(tv);
  tvAsVariant(tv) = !this_continuation(m_fp)->done();
}

OPTBLD_INLINE void VMExecutionContext::iopContKey(PC& pc) {
  NEXT();
  c_Continuation* cont = this_continuation(m_fp);
  cont->startedCheck();

  TypedValue* tv = m_stack.allocTV();
  tvWriteUninit(tv);
  tvAsVariant(tv) = cont->m_key;
}

OPTBLD_INLINE void VMExecutionContext::iopContCurrent(PC& pc) {
  NEXT();
  c_Continuation* cont = this_continuation(m_fp);
  cont->startedCheck();

  TypedValue* tv = m_stack.allocTV();
  tvWriteUninit(tv);
  tvAsVariant(tv) = cont->m_value;
}

OPTBLD_INLINE void VMExecutionContext::iopContStopped(PC& pc) {
  NEXT();
  this_continuation(m_fp)->setStopped();
}

OPTBLD_INLINE void VMExecutionContext::iopContHandle(PC& pc) {
  NEXT();
  c_Continuation* cont = this_continuation(m_fp);
  cont->setDone();
  cont->m_value.setNull();

  Variant exn = tvAsVariant(m_stack.topTV());
  m_stack.popC();
  assert(exn.asObjRef().instanceof(SystemLib::s_ExceptionClass));
  throw exn.asObjRef();
}

template<class Op>
OPTBLD_INLINE void VMExecutionContext::roundOpImpl(Op op) {
  TypedValue* val = m_stack.topTV();

  tvCastToDoubleInPlace(val);
  val->m_data.dbl = op(val->m_data.dbl);
}

OPTBLD_INLINE void VMExecutionContext::iopFloor(PC& pc) {
  NEXT();
  roundOpImpl(floor);
}

OPTBLD_INLINE void VMExecutionContext::iopCeil(PC& pc) {
  NEXT();
  roundOpImpl(ceil);
}

OPTBLD_INLINE void VMExecutionContext::iopStrlen(PC& pc) {
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

OPTBLD_INLINE void VMExecutionContext::iopIncStat(PC& pc) {
  NEXT();
  DECODE_IVA(counter);
  DECODE_IVA(value);
  Stats::inc(Stats::StatCounter(counter), value);
}

void VMExecutionContext::classExistsImpl(PC& pc, Attr typeAttr) {
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

OPTBLD_INLINE void VMExecutionContext::iopClassExists(PC& pc) {
  classExistsImpl(pc, AttrNone);
}

OPTBLD_INLINE void VMExecutionContext::iopInterfaceExists(PC& pc) {
  classExistsImpl(pc, AttrInterface);
}

OPTBLD_INLINE void VMExecutionContext::iopTraitExists(PC& pc) {
  classExistsImpl(pc, AttrTrait);
}

string
VMExecutionContext::prettyStack(const string& prefix) const {
  if (!getFP()) {
    string s("__Halted");
    return s;
  }
  int offset = (m_fp->m_func->unit() != nullptr)
               ? pcOff()
               : 0;
  string begPrefix = prefix + "__";
  string midPrefix = prefix + "|| ";
  string endPrefix = prefix + "\\/";
  string stack = m_stack.toString(m_fp, offset, midPrefix);
  return begPrefix + "\n" + stack + endPrefix;
}

void VMExecutionContext::checkRegStateWork() const {
  assert(Transl::tl_regState == Transl::VMRegState::CLEAN);
}

void VMExecutionContext::DumpStack() {
  string s = g_vmContext->prettyStack("");
  fprintf(stderr, "%s\n", s.c_str());
}

void VMExecutionContext::DumpCurUnit(int skip) {
  ActRec* fp = g_vmContext->getFP();
  Offset pc = fp->m_func->unit() ? g_vmContext->pcOff() : 0;
  while (skip--) {
    fp = g_vmContext->getPrevVMState(fp, &pc);
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

void VMExecutionContext::PrintTCCallerInfo() {
  VMRegAnchor _;
  ActRec* fp = g_vmContext->getFP();
  Unit* u = fp->m_func->unit();
  fprintf(stderr, "Called from TC address %p\n",
          tx()->getTranslatedCaller());
  std::cerr << u->filepath()->data() << ':'
            << u->getLineNumber(u->offsetOf(g_vmContext->getPC())) << std::endl;
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

#define O(name, imm, pusph, pop, flags)                                       \
void VMExecutionContext::op##name() {                                         \
  condStackTraceSep("op"#name" ");                                            \
  COND_STACKTRACE("op"#name" pre:  ");                                        \
  PC pc = m_pc;                                                               \
  assert(toOp(*pc) == Op##name);                                              \
  ONTRACE(1,                                                                  \
          int offset = m_fp->m_func->unit()->offsetOf(pc);                    \
          Trace::trace("op"#name" offset: %d\n", offset));                    \
  iop##name(pc);                                                              \
  SYNC();                                                                     \
  COND_STACKTRACE("op"#name" post: ");                                        \
  condStackTraceSep("op"#name" ");                                            \
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
inline void VMExecutionContext::dispatchImpl(int numInstrs) {
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
    Op op = toOp(*pc);                                                  \
    COND_STACKTRACE("dispatch:                    ");                   \
    ONTRACE(1,                                                          \
            Trace::trace("dispatch: %d: %s\n", pcOff(),                 \
                         nametab[uint8_t(op)]));                        \
    if (profile && (op == OpRetC || op == OpRetV)) {                    \
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
    const Op op = Op::name;                                   \
    if (op == OpRetC || op == OpRetV || op == OpNativeImpl) { \
      if (UNLIKELY(!pc)) { m_fp = 0; return; }                \
    }                                                         \
    DISPATCH();                                               \
  }
  OPCODES
#undef O
#undef DISPATCH
}

void VMExecutionContext::dispatch() {
  if (shouldProfile()) {
    dispatchImpl<Profile>(0);
  } else {
    dispatchImpl<0>(0);
  }
}

// We are about to go back to translated code, check whether we should
// stick with the interpreter. NB: if we've just executed a return
// from pseudomain, then there's no PC and no more code to interpret.
void VMExecutionContext::switchModeForDebugger() {
  if (DEBUGGER_FORCE_INTR && (getPC() != 0)) {
    throw VMSwitchMode();
  }
}

void VMExecutionContext::dispatchN(int numInstrs) {
  dispatchImpl<LimitInstrs | BreakOnCtlFlow>(numInstrs);
  switchModeForDebugger();
}

void VMExecutionContext::dispatchBB() {
  dispatchImpl<BreakOnCtlFlow>(0);
  switchModeForDebugger();
}

void VMExecutionContext::recordCodeCoverage(PC pc) {
  Unit* unit = getFP()->m_func->unit();
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

void VMExecutionContext::resetCoverageCounters() {
  m_coverPrevLine = -1;
  m_coverPrevUnit = nullptr;
}

void VMExecutionContext::pushVMState(VMState &savedVM,
                                     const ActRec* reentryAR) {
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
  m_nestedVMs.push_back(ReentryRecord(savedVM, reentryAR));
  m_nesting++;
}

void VMExecutionContext::popVMState() {
  assert(m_nestedVMs.size() >= 1);

  VMState &savedVM = m_nestedVMs.back().m_savedState;
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
}

void VMExecutionContext::requestInit() {
  assert(SystemLib::s_unit);
  assert(SystemLib::s_nativeFuncUnit);
  assert(SystemLib::s_nativeClassUnit);

  new (&s_requestArenaStorage) RequestArena();
  new (&s_varEnvArenaStorage) VarEnvArena();

  EnvConstants::requestInit(new (request_arena()) EnvConstants());
  VarEnv::createGlobal();
  m_stack.requestInit();
  Transl::Translator::advanceTranslator();
  tx()->requestInit();

  if (UNLIKELY(RuntimeOption::EvalJitEnableRenameFunction)) {
    SystemLib::s_unit->merge();
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
}

void VMExecutionContext::requestExit() {
  MemoryProfile::finishProfiling();

  destructObjects();
  syncGdbState();
  tx()->requestExit();
  Transl::Translator::clearTranslator();
  m_stack.requestExit();
  profileRequestEnd();
  EventHook::Disable();
  EnvConstants::requestExit();

  if (m_globalVarEnv) {
    VarEnv::destroy(m_globalVarEnv);
    m_globalVarEnv = 0;
  }

  varenv_arena().~VarEnvArena();
  request_arena().~RequestArena();
}

///////////////////////////////////////////////////////////////////////////////
}
