/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <boost/format.hpp>
#include <boost/utility/typed_in_place_factory.hpp>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <libgen.h>
#include <sys/mman.h>

#include <compiler/builtin_symbols.h>
#include <runtime/vm/bytecode.h>
#include <runtime/vm/event_hook.h>
#include <runtime/vm/translator/translator-deps.h>
#include <runtime/vm/translator/translator-x64.h>
#include <runtime/vm/member_operations.h>
#include <runtime/base/code_coverage.h>
#include <runtime/eval/runtime/file_repository.h>
#include <runtime/base/base_includes.h>
#include <runtime/base/execution_context.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/array/hphp_array.h>
#include <runtime/base/strings.h>
#include <util/util.h>
#include <util/trace.h>
#include <util/debug.h>
#include <util/stat_cache.h>

#include <runtime/base/tv_macros.h>
#include <runtime/vm/instrumentation_hook.h>
#include <runtime/vm/php_debug.h>
#include <runtime/vm/debugger_hook.h>
#include <runtime/vm/runtime.h>
#include <runtime/vm/translator/targetcache.h>
#include <runtime/vm/type_constraint.h>
#include <runtime/vm/translator/translator-inline.h>
#include <runtime/ext/profile/extprofile_string.h>
#include <runtime/ext/ext_error.h>
#include <runtime/ext/ext_continuation.h>
#include <runtime/ext/ext_function.h>
#include <runtime/ext/ext_variable.h>
#include <runtime/ext/ext_array.h>
#include <runtime/vm/stats.h>
#include <runtime/vm/type-profile.h>
#include <runtime/base/server/source_root_info.h>
#include <runtime/base/util/extended_logger.h>

#include <system/lib/systemlib.h>
#include <runtime/ext/ext_collection.h>

#include "runtime/vm/name_value_table_wrapper.h"
#include "runtime/vm/request_arena.h"
#include "util/arena.h"

using std::string;

namespace HPHP {

// RepoAuthoritative has been raptured out of runtime_option.cpp. It needs
// to be closer to other bytecode.cpp data.
bool RuntimeOption::RepoAuthoritative = false;

namespace VM {

using Transl::tx64;

#if DEBUG
#define OPTBLD_INLINE
#else
#define OPTBLD_INLINE ALWAYS_INLINE
#endif
static const Trace::Module TRACEMOD = Trace::bcinterp;

namespace {

struct HaltVM : std::exception {
  const char* what() const throw() { return "HaltVM"; }
};

struct VMPrepareThrow : std::exception {
  const char* what() const throw() { return "VMPrepareThrow"; }
};

struct VMPrepareUnwind : std::exception {
  const char* what() const throw() { return "VMPrepareUnwind"; }
};

}

template <>
Class* arGetContextClassImpl<false>(const ActRec* ar) {
  if (ar == NULL) {
    return NULL;
  }
  return ar->m_func->cls();
}

template <>
Class* arGetContextClassImpl<true>(const ActRec* ar) {
  if (ar == NULL) {
    return NULL;
  }
  if (ar->m_func->isPseudoMain() || ar->m_func->isBuiltin()) {
    // Pseudomains inherit the context of their caller
    VMExecutionContext* context = g_vmContext;
    ar = context->getPrevVMState(ar);
    while (ar != NULL &&
             (ar->m_func->isPseudoMain() || ar->m_func->isBuiltin())) {
      ar = context->getPrevVMState(ar);
    }
    if (ar == NULL) {
      return NULL;
    }
  }
  return ar->m_func->cls();
}

static StaticString s_call_user_func(LITSTR_INIT("call_user_func"));
static StaticString s_call_user_func_array(LITSTR_INIT("call_user_func_array"));
static StaticString s_hphpd_break(LITSTR_INIT("hphpd_break"));
static StaticString s_fb_enable_code_coverage(
  LITSTR_INIT("fb_enable_code_coverage"));
static StaticString s_file(LITSTR_INIT("file"));
static StaticString s_line(LITSTR_INIT("line"));
static StaticString s_stdclass(LITSTR_INIT("stdclass"));
static StaticString s___call(LITSTR_INIT("__call"));
static StaticString s___callStatic(LITSTR_INIT("__callStatic"));

///////////////////////////////////////////////////////////////////////////////

//=============================================================================
// Miscellaneous macros.

#define NEXT() pc++
#define DECODE_JMP(type, var)                                                 \
  type var __attribute__((unused)) = *(type*)pc;                              \
  ONTRACE(1,                                                                  \
          Trace::trace("decode:     Immediate %s %" PRIi64"\n", #type,         \
                       (int64_t)var));
#define ITER_SKIP(offset)  pc = origPc + (offset);

#define DECODE(type, var)                                                     \
  DECODE_JMP(type, var);                                                      \
  pc += sizeof(type)
#define DECODE_IVA(var)                                                       \
  int32 var UNUSED = decodeVariableSizeImm(&pc);                              \
  ONTRACE(1,                                                                  \
          Trace::trace("decode:     Immediate int32 %" PRIi64"\n",             \
                       (int64_t)var));
#define DECODE_LITSTR(var)                                \
  StringData* var;                                        \
  do {                                                    \
    DECODE(Id, id);                                       \
    var = m_fp->m_func->unit()->lookupLitstrId(id);       \
  } while (false)

#define DECODE_HA(var) DECODE_IVA(var)
#define DECODE_IA(var) DECODE_IVA(var)

#define SYNC() m_pc = pc

//=============================================================================
// Miscellaneous helpers.

static inline Class* frameStaticClass(ActRec* fp) {
  if (fp->hasThis()) {
    return fp->getThis()->getVMClass();
  } else if (fp->hasClass()) {
    return fp->getClass();
  } else {
    return NULL;
  }
}

//=============================================================================
// VarEnv.

VarEnv::VarEnv()
  : m_depth(0)
  , m_malloced(false)
  , m_cfp(0)
  , m_previous(0)
  , m_nvTable(boost::in_place<NameValueTable>(
      RuntimeOption::EvalVMInitialGlobalTableSize))
{
  // The global scope always contains certain special names.
  {
    TypedValue globalArray;
    globalArray.m_type = KindOfArray;
    globalArray.m_data.parr =
      new (request_arena()) NameValueTableWrapper(&*m_nvTable);
    globalArray.m_data.parr->incRefCount();
    m_nvTable->set(StringData::GetStaticString("GLOBALS"), &globalArray);
    tvRefcountedDecRef(&globalArray);
  }

  SystemGlobals* g = reinterpret_cast<SystemGlobals*>(
    get_global_variables());
#define X(s) \
  m_nvTable->migrateSet(StringData::GetStaticString(#s), \
                       g->gvm_##s.asTypedValue());
  X(argc);
  X(argv);
  X(_SERVER);
  X(_GET);
  X(_POST);
  X(_COOKIE);
  X(_FILES);
  X(_ENV);
  X(_REQUEST);
  X(_SESSION);
  X(HTTP_RAW_POST_DATA);
  X(http_response_header);
#undef X
}

VarEnv::VarEnv(ActRec* fp, ExtraArgs* eArgs)
  : m_extraArgs(eArgs)
  , m_depth(1)
  , m_malloced(false)
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
    ASSERT(func->lookupVarId(func->localVarName(i)) == (int)i);
    origLocs[i] = m_nvTable->migrateSet(func->localVarName(i), loc);
  }
}

VarEnv::~VarEnv() {
  TRACE(3, "Destroying VarEnv %p [%s]\n",
           this,
           isGlobalScope() ? "global scope" : "local scope");
  ASSERT(m_restoreLocations.empty());
  if (g_vmContext->m_topVarEnv == this) {
    g_vmContext->m_topVarEnv = m_previous;
  }

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

VarEnv* VarEnv::createLazyAttach(ActRec* fp,
                                 bool skipInsert /* = false */) {
  const Func* func = fp->m_func;
  const size_t numNames = func->numNamedLocals();
  ExtraArgs* eArgs = fp->getExtraArgs();
  const size_t neededSz = sizeof(VarEnv) +
                          sizeof(TypedValue*) * numNames;

  TRACE(3, "Creating lazily attached VarEnv\n");

  if (LIKELY(!skipInsert)) {
    varenv_arena().beginFrame();
    void* mem = varenv_arena().alloc(neededSz);
    VarEnv* ret = new (mem) VarEnv(fp, eArgs);
    TRACE(3, "Creating lazily attached VarEnv %p\n", mem);
    ret->setPrevious(g_vmContext->m_topVarEnv);
    g_vmContext->m_topVarEnv = ret;
    return ret;
  }

  /*
   * For skipInsert == true, we're adding a VarEnv in the middle of
   * the chain, which means we can't use the stack allocation.
   *
   * The caller must immediately setPrevious, so don't bother setting
   * it to an invalid pointer except in a debug build.
   */
  void* mem = malloc(neededSz);
  VarEnv* ret = new (mem) VarEnv(fp, eArgs);
  ret->m_malloced = true;
  if (debug) {
    ret->setPrevious((VarEnv*)-1);
  }
  return ret;
}

VarEnv* VarEnv::createGlobal() {
  ASSERT(!g_vmContext->m_globalVarEnv);
  ASSERT(!g_vmContext->m_topVarEnv);

  VarEnv* ret = new (request_arena()) VarEnv();
  TRACE(3, "Creating VarEnv %p [global scope]\n", ret);
  g_vmContext->m_globalVarEnv = g_vmContext->m_topVarEnv = ret;
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
  ASSERT(m_depth == 0 || g_vmContext->arGetSfp(fp) == m_cfp ||
         (g_vmContext->arGetSfp(fp) == fp && g_vmContext->isNested()));
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
    ASSERT(func->lookupVarId(func->localVarName(i)) == (int)i);
    origLocs[i] = m_nvTable->migrate(func->localVarName(i), loc);
  }
  m_restoreLocations.push_back(origLocs);
}

void VarEnv::detach(ActRec* fp) {
  TRACE(3, "Detaching VarEnv %p [%s] @%p\n",
           this,
           isGlobalScope() ? "global scope" : "local scope",
           fp);
  ASSERT(fp == m_cfp);
  ASSERT(m_depth > 0);

  // Merge/remove fp's overlaid locals, if it had any.
  const Func* func = fp->m_func;
  if (Id const numLocals = func->numNamedLocals()) {
    /*
     * In the case of a lazily attached VarEnv, we have our locations
     * for the first (lazy) attach stored immediately following the
     * VarEnv in memory.  In this case m_restoreLocations will be empty.
     */
    ASSERT((!isGlobalScope() && m_depth == 1) == m_restoreLocations.empty());
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
    m_cfp = NULL;
    // don't free global varEnv
    if (context->m_globalVarEnv != this) {
      ASSERT(!isGlobalScope());
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

TypedValue* VarEnv::lookupRawPointer(const StringData* name) {
  ensureNvt();
  return m_nvTable->lookupRawPointer(name);
}

TypedValue* VarEnv::lookupAddRawPointer(const StringData* name) {
  ensureNvt();
  return m_nvTable->lookupAddRawPointer(name);
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
  ASSERT(nargs > 0);

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
  StackElms() : m_elms(NULL) {}
  ~StackElms() {
    flush();
  }
  TypedValue* elms() {
    if (m_elms == NULL) {
      // RuntimeOption::EvalVMStackElms-sized and -aligned.
      size_t algnSz = RuntimeOption::EvalVMStackElms * sizeof(TypedValue);
      if (posix_memalign((void**)&m_elms, algnSz, algnSz) != 0) {
        throw std::runtime_error(
          std::string("VM stack initialization failed: ") + strerror(errno));
      }
    }
    return m_elms;
  }
  void flush() {
    if (m_elms != NULL) {
      free(m_elms);
      m_elms = NULL;
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
  : m_elms(NULL), m_top(NULL), m_base(NULL) {
}

Stack::~Stack() {
  requestExit();
}

void
Stack::protect() {
  if (trustSigSegv) {
    mprotect(m_elms, sizeof(void*), PROT_NONE);
  }
}

void
Stack::unprotect() {
  if (trustSigSegv) {
    mprotect(m_elms, sizeof(void*), PROT_READ | PROT_WRITE);
  }
}

void
Stack::requestInit() {
  m_elms = t_se->elms();
  if (trustSigSegv) {
    RequestInjectionData& data = ThreadInfo::s_threadInfo->m_reqInjectionData;
    Lock l(data.surpriseLock);
    ASSERT(data.surprisePage == NULL);
    data.surprisePage = m_elms;
  }
  // Burn one element of the stack, to satisfy the constraint that
  // valid m_top values always have the same high-order (>
  // log(RuntimeOption::EvalVMStackElms)) bits.
  m_top = m_base = m_elms + RuntimeOption::EvalVMStackElms - 1;

  // Because of the surprise page at the bottom of the stack we lose an
  // additional 256 elements which must be taken into account when checking for
  // overflow.
  UNUSED size_t maxelms =
    RuntimeOption::EvalVMStackElms - sSurprisePageSize / sizeof(TypedValue);
  ASSERT(!wouldOverflow(maxelms - 1));
  ASSERT(wouldOverflow(maxelms));

  // Reset permissions on our stack's surprise page
  unprotect();
}

void
Stack::requestExit() {
  if (m_elms != NULL) {
    if (trustSigSegv) {
      RequestInjectionData& data = ThreadInfo::s_threadInfo->m_reqInjectionData;
      Lock l(data.surpriseLock);
      ASSERT(data.surprisePage == m_elms);
      unprotect();
      data.surprisePage = NULL;
    }
    m_elms = NULL;
  }
}

void
Stack::flush() {
  if (!t_se.isNull()) {
    t_se->flush();
  }
  TargetCache::flush();
}

void Stack::toStringElm(std::ostream& os, TypedValue* tv, const ActRec* fp)
  const {
  if (tv->m_type < MinDataType || tv->m_type > MaxNumDataTypes) {
    os << " ??? type " << tv->m_type << "\n";
    return;
  }
  ASSERT(tv->m_type >= MinDataType && tv->m_type < MaxNumDataTypes);
  if (IS_REFCOUNTED_TYPE(tv->m_type) && tv->m_data.pref->_count <= 0) {
    // OK in the invoking frame when running a destructor.
    os << " ??? inner_count " << tv->m_data.pref->_count << " ";
    return;
  }
  switch (tv->m_type) {
  case KindOfRef:
    os << "V:(";
    os << "@" << tv->m_data.pref;
    tv = tv->m_data.pref->tv();  // Unbox so contents get printed below
    ASSERT(tv->m_type != KindOfRef);
    toStringElm(os, tv, fp);
    os << ")";
    return;
  case KindOfClass:
    os << "A:";
    break;
  default:
    os << "C:";
    break;
  }
  switch (tv->m_type) {
  case KindOfUninit: {
    os << "Undefined";
    break;
  }
  case KindOfNull: {
    os << "Null";
    break;
  }
  case KindOfBoolean: {
    os << (tv->m_data.num ? "True" : "False");
    break;
  }
  case KindOfInt64: {
    os << "0x" << std::hex << tv->m_data.num << std::dec;
    break;
  }
  case KindOfDouble: {
    os << tv->m_data.dbl;
    break;
  }
  case KindOfStaticString:
  case KindOfString: {
    ASSERT(tv->m_data.pstr->getCount() > 0);
    int len = tv->m_data.pstr->size();
    bool truncated = false;
    if (len > 128) {
      len = 128;
      truncated = true;
    }
    os << tv->m_data.pstr << ":\""
       << Util::escapeStringForCPP(tv->m_data.pstr->data(), len)
       << "\"" << (truncated ? "..." : "");
    break;
  }
  case KindOfArray: {
    ASSERT(tv->m_data.parr->getCount() > 0);
    os << tv->m_data.parr << ":Array";
    break;
  }
  case KindOfObject: {
    ASSERT(tv->m_data.pobj->getCount() > 0);
    os << tv->m_data.pobj << ":Object("
       << tvAsVariant(tv).asObjRef().get()->o_getClassName().get()->data()
       << ")";
    break;
  }
  case KindOfRef: {
    not_reached();
  }
  case KindOfClass: {
    os << tv->m_data.pcls
       << ":" << tv->m_data.pcls->name()->data();
    break;
  }
  default: {
    os << "?";
    break;
  }
  }
}

void Stack::toStringIter(std::ostream& os, Iter* it) const {
  switch (it->m_itype) {
  case Iter::TypeUndefined: {
    os << "I:Undefined";
    break;
  }
  case Iter::TypeArray: {
    os << "I:Array";
    break;
  }
  case Iter::TypeMutableArray: {
    os << "I:MutableArray";
    break;
  }
  case Iter::TypeIterator: {
    os << "I:Iterator";
    break;
  }
  default: {
    ASSERT(false);
    os << "I:?";
    break;
  }
  }
}

void Stack::toStringFrag(std::ostream& os, const ActRec* fp,
                         const TypedValue* top) const {
  TypedValue* tv;

  // The only way to figure out which stack elements are activation records is
  // to follow the frame chain. However, the goal for each stack frame is to
  // print stack fragments from deepest to shallowest -- a then b in the
  // following example:
  //
  //   {func:foo,soff:51}<C:8> {func:bar} C:8 C:1 {func:biz} C:0
  //                           aaaaaaaaaaaaaaaaaa bbbbbbbbbbbbbb
  //
  // Use depth-first recursion to get the output order correct.

  if (LIKELY(!fp->m_func->isGenerator())) {
    tv = frameStackBase(fp);
  } else {
    tv = generatorStackBase(fp);
  }

  for (tv--; (uintptr_t)tv >= (uintptr_t)top; tv--) {
    os << " ";
    toStringElm(os, tv, fp);
  }
}

void Stack::toStringAR(std::ostream& os, const ActRec* fp,
                       const FPIEnt *fe, const TypedValue* top) const {
  ActRec *ar;
  if (LIKELY(!fp->m_func->isGenerator())) {
    ar = arAtOffset(fp, -fe->m_fpOff);
  } else {
    // Deal with generators' split stacks. See unwindAR for reasoning.
    TypedValue* genStackBase = generatorStackBase(fp);
    ActRec* fakePrevFP =
      (ActRec*)(genStackBase + fp->m_func->numSlotsInFrame());
    ar = arAtOffset(fakePrevFP, -fe->m_fpOff);
  }

  if (fe->m_parentIndex != -1) {
    toStringAR(os, fp, &fp->m_func->fpitab()[fe->m_parentIndex],
      (TypedValue*)&ar[1]);
  } else {
    toStringFrag(os, fp, (TypedValue*)&ar[1]);
  }

  os << " {func:" << ar->m_func->fullName()->data() << "}";
  TypedValue* tv = (TypedValue*)ar;
  for (tv--; (uintptr_t)tv >= (uintptr_t)top; tv--) {
    os << " ";
    toStringElm(os, tv, fp);
  }
}

void Stack::toStringFragAR(std::ostream& os, const ActRec* fp,
                           int offset, const TypedValue* top) const {
  const FPIEnt *fe = fp->m_func->findFPI(offset);
  if (fe != NULL) {
    toStringAR(os, fp, fe, top);
  } else {
    toStringFrag(os, fp, top);
  }
}

void Stack::toStringFrame(std::ostream& os, const ActRec* fp,
                          int offset, const TypedValue* ftop,
                          const string& prefix) const {
  ASSERT(fp);

  // Use depth-first recursion to output the most deeply nested stack frame
  // first.
  {
    Offset prevPc;
    TypedValue* prevStackTop;
    ActRec* prevFp = g_vmContext->getPrevVMState(fp, &prevPc, &prevStackTop);
    if (prevFp != NULL) {
      toStringFrame(os, prevFp, prevPc, prevStackTop, prefix);
    }
  }

  os << prefix;
  const Func* func = fp->m_func;
  ASSERT(func);
  func->validate();
  string funcName;
  if (func->isMethod()) {
    funcName = string(func->preClass()->name()->data()) + "::" +
      string(func->name()->data());
  } else {
    funcName = string(func->name()->data());
  }
  os << "{func:" << funcName
     << ",soff:" << fp->m_soff
     << ",this:0x" << std::hex << (fp->hasThis() ? fp->getThis() : NULL)
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
      toStringElm(os, tv, fp);
    }
    os << ">";
  }

  ASSERT(!func->isBuiltin() || func->numIterators() == 0);
  if (func->numIterators() > 0) {
    os << "|";
    Iter* it = &((Iter*)&tv[1])[-1];
    for (int i = 0; i < func->numIterators(); i++, it--) {
      if (i > 0) {
        os << " ";
      }
      if (func->checkIterScope(offset, i)) {
        toStringIter(os, it);
      } else {
        os << "I:Undefined";
      }
    }
    os << "|";
  }

  toStringFragAR(os, fp, offset, ftop);

  os << std::endl;
}

string Stack::toString(const ActRec* fp, int offset,
                       const string prefix/* = "" */) const {
  std::ostringstream os;
  os << prefix << "=== Stack at " << curUnit()->filepath()->data() << ":" <<
    curUnit()->getLineNumber(curUnit()->offsetOf(Transl::vmpc())) << " func " <<
    curFunc()->fullName()->data() << " ===\n";

  toStringFrame(os, fp, offset, m_top, prefix);

  return os.str();
}

void Stack::clearEvalStack(ActRec *fp, int32 numLocals) {
}

UnwindStatus Stack::unwindFrag(ActRec* fp, int offset,
                               PC& pc, Fault& f) {
  const Func* func = fp->m_func;
  TRACE(1, "unwindFrag: func %s\n", func->fullName()->data());

  bool unwindingGeneratorFrame = func->isGenerator();
  TypedValue* evalTop;
  if (UNLIKELY(unwindingGeneratorFrame)) {
    ASSERT(!isValidAddress((uintptr_t)fp));
    evalTop = generatorStackBase(fp);
  } else {
    ASSERT(isValidAddress((uintptr_t)fp));
    evalTop = frameStackBase(fp);
  }
  ASSERT(isValidAddress((uintptr_t)evalTop));
  ASSERT(evalTop >= m_top);

  while (m_top < evalTop) {
    popTV();
  }

  const EHEnt *eh = func->findEH(offset);
  while (eh != NULL) {
    if (eh->m_ehtype == EHEnt::EHType_Fault) {
      pc = (uchar*)(func->unit()->entry() + eh->m_fault);
      return UnwindResumeVM;
    } else if (f.m_faultType == Fault::KindOfUserException) {
      ObjectData* obj = f.m_userException;
      std::vector<std::pair<Id, Offset> >::const_iterator it;
      for (it = eh->m_catches.begin(); it != eh->m_catches.end(); it++) {
        Class* cls = func->unit()->lookupClass(it->first);
        if (cls && obj->instanceof(cls)) {
          pc = func->unit()->at(it->second);
          return UnwindResumeVM;
        }
      }
    }
    if (eh->m_parentIndex != -1) {
      eh = &func->ehtab()[eh->m_parentIndex];
    } else {
      break;
    }
  }

  if (fp->isFromFPushCtor()) {
    ASSERT(fp->hasThis());
    fp->getThis()->setNoDestruct();
  }

  if (LIKELY(!unwindingGeneratorFrame)) {
    // A generator's locals don't live on this stack.
    // onFunctionExit might throw
    try {
      frame_free_locals_inl(fp, func->numLocals());
    } catch (...) {}
    ndiscard(func->numSlotsInFrame());
  }
  return UnwindPropagate;
}

void Stack::unwindARFrag(ActRec* ar) {
  while (m_top < (TypedValue*)ar) {
    popTV();
  }
}

void Stack::unwindAR(ActRec* fp, const FPIEnt* fe) {
  while (true) {
    TRACE(1, "unwindAR: function %s, pIdx %d\n",
          fp->m_func->name()->data(), fe->m_parentIndex);
    ActRec* ar;
    if (LIKELY(!fp->m_func->isGenerator())) {
      ar = arAtOffset(fp, -fe->m_fpOff);
    } else {
      // fp is pointing into the continuation object. Since fpOff is given as an
      // offset from the frame pointer as if it were in the normal place on the
      // main stack, we have to reconstruct that "normal place".
      TypedValue* genStackBase = generatorStackBase(fp);
      ActRec* fakePrevFP =
        (ActRec*)(genStackBase + fp->m_func->numSlotsInFrame());
      ar = arAtOffset(fakePrevFP, -fe->m_fpOff);
    }
    ASSERT((TypedValue*)ar >= m_top);
    unwindARFrag(ar);

    if (ar->isFromFPushCtor()) {
      ASSERT(ar->hasThis());
      ar->getThis()->setNoDestruct();
    }

    popAR();
    if (fe->m_parentIndex != -1) {
      fe = &fp->m_func->fpitab()[fe->m_parentIndex];
    } else {
      return;
    }
  }
}

UnwindStatus Stack::unwindFrame(ActRec*& fp, int offset, PC& pc, Fault& f) {
  VMExecutionContext* context = g_vmContext;

  while (true) {
    SrcKey sk(fp->m_func, offset);
    SKTRACE(1, sk, "unwindFrame: func %s, offset %d fp %p\n",
            fp->m_func->name()->data(),
            offset, fp);
    const FPIEnt *fe = fp->m_func->findFPI(offset);
    if (fe != NULL) {
      unwindAR(fp, fe);
    }
    if (unwindFrag(fp, offset, pc, f) == UnwindResumeVM) {
      return UnwindResumeVM;
    }
    ActRec *prevFp = context->arGetSfp(fp);
    SKTRACE(1, sk, "unwindFrame: fp %p prevFp %p\n",
            fp, prevFp);
    Offset prevOff = fp->m_soff + prevFp->m_func->base();
    if (LIKELY(!fp->m_func->isGenerator())) {
      // We don't need to refcount the AR's refcounted members; that was
      // taken care of in frame_free_locals, called from unwindFrag().
      // If it's a generator, the AR doesn't live on this stack.
      discardAR();
    }

    if (prevFp == fp) {
      TRACE(1, "unwindFrame: reached the end of this nesting's ActRec "
               "chain\n");
      break;
    }
    // Keep the pc up to date while unwinding.
    const Func *prevF = prevFp->m_func;
    ASSERT(isValidAddress((uintptr_t)prevFp) || prevF->isGenerator());
    pc = prevF->unit()->at(prevF->base() + fp->m_soff);
    fp = prevFp;
    offset = prevOff;
  }

  return UnwindPropagate;
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
  ASSERT(!func->isGenerator());
  return (TypedValue*)((uintptr_t)fp
                       - (uintptr_t)(func->numLocals()) * sizeof(TypedValue)
                       - (uintptr_t)(func->numIterators() * sizeof(Iter)));
}

TypedValue* Stack::generatorStackBase(const ActRec* fp) {
  // We know generators are always called from a function with an empty stack.
  // So we find the caller's FP, compensate for its locals, and then we've found
  // the base of the generator's stack.
  ASSERT(fp->m_func->isGenerator());
  ActRec* callerFP = (ActRec*)fp->m_savedRbp;
  return (TypedValue*)callerFP - callerFP->m_func->numSlotsInFrame();
}


__thread RequestArenaStorage s_requestArenaStorage;
__thread VarEnvArenaStorage s_varEnvArenaStorage;

///////////////////////////////////////////////////////////////////////////////
} // namespace VM

//=============================================================================
// ExecutionContext.

using namespace HPHP::VM;
using namespace HPHP::MethodLookup;

ActRec* VMExecutionContext::arGetSfp(const ActRec* ar) {
  ActRec* prevFrame = (ActRec*)ar->m_savedRbp;
  if (LIKELY(prevFrame >= m_stack.getStackLowAddress() &&
             prevFrame < m_stack.getStackHighAddress())) {
    return prevFrame;
  }

  // Generators have their frames outside the main stack, so prevFrame might
  // point there. We need to be careful to distinguish this from a prevFrame
  // that points into the C++ stack.
  if (!prevFrame) {
    return const_cast<ActRec*>(ar);
  }
  int64* magicPtr = (int64*)(prevFrame + 1);
  if (*magicPtr == c_Continuation::kMagic) {
    ASSERT(prevFrame->m_func->isGenerator());
    return prevFrame;
  }
  return const_cast<ActRec*>(ar);
}

TypedValue* VMExecutionContext::lookupClsCns(const NamedEntity* ne,
                                             const StringData* cls,
                                             const StringData* cns) {
  Class* class_ = Unit::loadClass(ne, cls);
  if (class_ == NULL) {
    raise_error(Strings::UNKNOWN_CLASS, cls->data());
  }
  TypedValue* clsCns = class_->clsCnsGet(cns);
  if (clsCns == NULL) {
    raise_error("Couldn't find constant %s::%s",
                cls->data(), cns->data());
  }
  return clsCns;
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
                                                Class* ctx,
                                                CallType callType,
                                                bool raise /* = false */) {
  const Func* method;
  if (callType == CtorMethod) {
    ASSERT(methodName == NULL);
    method = cls->getCtor();
  } else {
    ASSERT(callType == ObjMethod || callType == ClsMethod);
    ASSERT(methodName != NULL);
    method = cls->lookupMethod(methodName);
    while (!method) {
      static StringData* sd__construct
        = StringData::GetStaticString("__construct");
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
      return NULL;
    }
  }
  ASSERT(method);
  bool accessible = true;
  // If we found a protected or private method, we need to do some
  // accessibility checks.
  if ((method->attrs() & (AttrProtected|AttrPrivate)) &&
      !g_vmContext->getDebuggerBypassCheck()) {
    Class* baseClass = method->baseCls();
    ASSERT(baseClass);
    // If the context class is the same as the class that first
    // declared this method, then we know we have the right method
    // and we can stop here.
    if (ctx == baseClass) {
      return method;
    }
    // The anonymous context cannot access protected or private methods,
    // so we can fail fast here.
    if (ctx == NULL) {
      if (raise) {
        raise_error("Call to %s method %s::%s from anonymous context",
                    (method->attrs() & AttrPrivate) ? "private" : "protected",
                    cls->name()->data(),
                    method->name()->data());
      }
      return NULL;
    }
    ASSERT(ctx);
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
        return NULL;
      }
      // We now know this protected method is accessible, but we need to
      // keep going because the context class may define a private method
      // with this name.
      ASSERT(accessible && baseClass->classof(ctx));
    }
  }
  // If this is an ObjMethod call ("$obj->foo()") AND there is an ancestor
  // of cls that declares a private method with this name AND the context
  // class is an ancestor of cls, check if the context class declares a
  // private method with this name.
  if (method->hasPrivateAncestor() && callType == ObjMethod &&
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
  return NULL;
}

LookupResult VMExecutionContext::lookupObjMethod(const Func*& f,
                                                 const Class* cls,
                                                 const StringData* methodName,
                                                 bool raise /* = false */) {
  Class* ctx = arGetContextClass(getFP());
  f = lookupMethodCtx(cls, methodName, ctx, ObjMethod, false);
  if (!f) {
    f = cls->lookupMethod(s___call.get());
    if (!f) {
      if (raise) {
        // Throw a fatal error
        lookupMethodCtx(cls, methodName, ctx, ObjMethod, true);
      }
      return MethodNotFound;
    }
    return MagicCallFound;
  }
  if (f->attrs() & AttrStatic) {
    return MethodFoundNoThis;
  }
  return MethodFoundWithThis;
}

LookupResult
VMExecutionContext::lookupClsMethod(const Func*& f,
                                    const Class* cls,
                                    const StringData* methodName,
                                    ObjectData* obj,
                                    bool raise /* = false */) {
  Class* ctx = arGetContextClass(getFP());
  f = lookupMethodCtx(cls, methodName, ctx, ClsMethod, false);
  if (!f) {
    if (obj && obj->instanceof(cls)) {
      f = obj->getVMClass()->lookupMethod(s___call.get());
    }
    if (!f) {
      f = cls->lookupMethod(s___callStatic.get());
      if (!f) {
        if (raise) {
          // Throw a fatal errpr
          lookupMethodCtx(cls, methodName, ctx, ClsMethod, true);
        }
        return MethodNotFound;
      }
      f->validate();
      ASSERT(f);
      ASSERT(f->attrs() & AttrStatic);
      return MagicCallStaticFound;
    }
    ASSERT(f);
    ASSERT(obj);
    // __call cannot be static, this should be enforced by semantic
    // checks defClass time or earlier
    ASSERT(!(f->attrs() & AttrStatic));
    return MagicCallFound;
  }
  if (obj && !(f->attrs() & AttrStatic) && obj->instanceof(cls)) {
    return MethodFoundWithThis;
  }
  return MethodFoundNoThis;
}

LookupResult VMExecutionContext::lookupCtorMethod(const Func*& f,
                                                  const Class* cls,
                                                  bool raise /* = false */) {
  f = cls->getCtor();
  if (!(f->attrs() & AttrPublic)) {
    Class* ctx = arGetContextClass(getFP());
    f = lookupMethodCtx(cls, NULL, ctx, CtorMethod, raise);
    if (!f) {
      // If raise was true than lookupMethodCtx should have thrown,
      // so we should only be able to get here if raise was false
      ASSERT(!raise);
      return MethodNotFound;
    }
  }
  return MethodFoundWithThis;
}

ObjectData* VMExecutionContext::createObject(StringData* clsName,
                                             CArrRef params,
                                             bool init /* = true */) {
  Class* class_ = Unit::loadClass(clsName);
  if (class_ == NULL) {
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

ObjectData* VMExecutionContext::getThis(bool skipFrame /* = false */) {
  VMRegAnchor _;
  ActRec* fp = getFP();
  if (skipFrame) {
    fp = getPrevVMState(fp);
    if (!fp) return NULL;
  }
  if (fp->hasThis()) {
    return fp->getThis();
  }
  return NULL;
}

CStrRef VMExecutionContext::getContextClassName(bool skipFrame /* = false */) {
  VMRegAnchor _;
  ActRec* ar = getFP();
  ASSERT(ar != NULL);
  if (skipFrame) {
    ar = getPrevVMState(ar);
    if (!ar) return empty_string;
  }
  if (ar->hasThis()) {
    return ar->getThis()->o_getClassName();
  } else if (ar->hasClass()) {
    return ar->getClass()->nameRef();
  } else {
    return empty_string;
  }
}

CStrRef VMExecutionContext::getParentContextClassName(bool skip /* = false */) {
  VMRegAnchor _;
  ActRec* ar = getFP();
  ASSERT(ar != NULL);
  if (skip) {
    ar = getPrevVMState(ar);
    if (!ar) return empty_string;
  }
  if (ar->hasThis()) {
    const Class* cls = ar->getThis()->getVMClass();
    if (cls->parent() == NULL) {
      return empty_string;
    }
    return cls->parent()->nameRef();
  } else if (ar->hasClass()) {
    const Class* cls = ar->getClass();
    if (cls->parent() == NULL) {
      return empty_string;
    }
    return cls->parent()->nameRef();
  } else {
    return empty_string;
  }
}

CStrRef VMExecutionContext::getContainingFileName(
  bool skipFrame /* = false */) {
  VMRegAnchor _;
  ActRec* ar = getFP();
  if (ar == NULL) return empty_string;
  if (skipFrame) {
    ar = getPrevVMState(ar);
    if (ar == NULL) return empty_string;
  }
  Unit* unit = ar->m_func->unit();
  return unit->filepathRef();
}

int VMExecutionContext::getLine(bool skipFrame /* = false */) {
  VMRegAnchor _;
  ActRec* ar = getFP();
  Unit* unit = ar ? ar->m_func->unit() : NULL;
  Offset pc = unit ? pcOff() : 0;
  if (ar == NULL) return -1;
  if (skipFrame) {
    ar = getPrevVMState(ar, &pc);
  }
  if (ar == NULL || (unit = ar->m_func->unit()) == NULL) return -1;
  return unit->getLineNumber(pc);
}

Array VMExecutionContext::getCallerInfo(bool skipFrame /* = false */) {
  VMRegAnchor _;
  Array result = Array::Create();
  ActRec* ar = getFP();
  if (skipFrame) {
    ar = getPrevVMState(ar);
  }
  while (ar->m_func->name()->isame(s_call_user_func.get())
         || ar->m_func->name()->isame(s_call_user_func_array.get())) {
    ar = getPrevVMState(ar);
    if (ar == NULL) {
      return result;
    }
  }

  Offset pc;
  ar = getPrevVMState(ar, &pc);
  while (ar != NULL) {
    if (!ar->m_func->name()->isame(s_call_user_func.get())
        && !ar->m_func->name()->isame(s_call_user_func_array.get())) {
      Unit* unit = ar->m_func->unit();
      int lineNumber;
      if ((lineNumber = unit->getLineNumber(pc)) != -1) {
        ASSERT(!unit->filepath()->size() ||
               unit->filepath()->data()[0] == '/');
        result.set(s_file, unit->filepath()->data(), true);
        result.set(s_line, lineNumber);
        return result;
      }
    }
    ar = getPrevVMState(ar, &pc);
  }
  return result;
}

bool VMExecutionContext::defined(CStrRef name) {
  return m_constants.nvGet(name.get()) != NULL;
}

TypedValue* VMExecutionContext::getCns(StringData* cns,
                                       bool system /* = true */,
                                       bool dynamic /* = true */) {
  if (dynamic) {
    TypedValue* tv = m_constants.nvGet(cns);
    if (tv != NULL) {
      return tv;
    }
  }
  if (system) {
    const ClassInfo::ConstantInfo* ci = ClassInfo::FindConstant(cns->data());
    if (ci != NULL) {
      if (!dynamic) {
        ConstInfoMap::const_iterator it = m_constInfo.find(cns);
        if (it != m_constInfo.end()) {
          // This is a dynamic constant, so don't report it.
          ASSERT(ci == it->second);
          return NULL;
        }
      }
      TypedValue tv;
      TV_WRITE_UNINIT(&tv);
      tvAsVariant(&tv) = ci->getValue();
      m_constants.nvSet(cns, &tv, false);
      tvRefcountedDecRef(&tv);
      return m_constants.nvGet(cns);
    }
  }
  return NULL;
}

bool VMExecutionContext::setCns(StringData* cns, CVarRef val, bool dynamic) {
  if (m_constants.nvGet(cns) != NULL ||
      ClassInfo::FindConstant(cns->data()) != NULL) {
    raise_warning(Strings::CONSTANT_ALREADY_DEFINED, cns->data());
    return false;
  }
  if (!val.isAllowedAsConstantValue()) {
    raise_warning(Strings::CONSTANTS_MUST_BE_SCALAR);
    return false;
  }
  val.setEvalScalar();
  TypedValue* tv = val.getTypedAccessor();
  m_constants.nvSet(cns, tv, false);
  ASSERT(m_constants.nvGet(cns) != NULL);
  if (RuntimeOption::EvalJit) {
    if (dynamic) {
      newPreConst(cns, *tv);
    }
    tx64->defineCns(cns);
  }
  return true;
}

void VMExecutionContext::newPreConst(StringData* name,
                                     const TypedValue& val) {
  name->incRefCount();
  PreConst pc = { val, this, name };
  m_preConsts.push_back(pc);
  VM::Transl::mergePreConst(m_preConsts.back());
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

VarEnv* VMExecutionContext::getVarEnv(bool skipBuiltin) {
  Transl::VMRegAnchor _;

  HPHP::VM::VarEnv* builtinVarEnv = NULL;
  HPHP::VM::ActRec* fp = getFP();
  if (skipBuiltin) {
    if (fp->hasVarEnv()) {
      builtinVarEnv = fp->getVarEnv();
    }
    fp = getPrevVMState(fp);
  }
  if (!fp) return NULL;
  ASSERT(!fp->hasInvName());
  if (!fp->hasVarEnv()) {
    if (builtinVarEnv) {
      // If the builtin function has its own VarEnv, we temporarily
      // remove it from the list before making a VarEnv for the calling
      // function to satisfy various ASSERTs
      ASSERT(builtinVarEnv == m_topVarEnv);
      m_topVarEnv = m_topVarEnv->previous();
    }
    fp->m_varEnv = VarEnv::createLazyAttach(fp);
    if (builtinVarEnv) {
      // Put the builtin function's VarEnv back in the list
      builtinVarEnv->setPrevious(fp->m_varEnv);
      m_topVarEnv = builtinVarEnv;
    }
  }
  return fp->m_varEnv;
}

void VMExecutionContext::setVar(StringData* name, TypedValue* v, bool ref) {
  Transl::VMRegAnchor _;
  // setVar() should only be called after getVarEnv() has been called
  // to create a varEnv
  ActRec *fp = getPrevVMState(getFP());
  if (!fp) return;
  ASSERT(!fp->hasInvName());
  ASSERT(!fp->hasExtraArgs());
  ASSERT(fp->m_varEnv != NULL);
  if (ref) {
    fp->m_varEnv->bind(name, v);
  } else {
    fp->m_varEnv->set(name, v);
  }
}

Array VMExecutionContext::getLocalDefinedVariables(int frame) {
  Transl::VMRegAnchor _;
  ActRec *fp = getFP();
  for (; frame > 0; --frame) {
    if (!fp) break;
    fp = getPrevVMState(fp);
  }
  if (!fp) {
    return Array::Create();
  }
  ASSERT(!fp->hasInvName());
  if (fp->hasVarEnv()) {
    return fp->m_varEnv->getDefinedVariables();
  }
  Array ret = Array::Create();
  const Func *func = fp->m_func;
  for (Id id = 0; id < func->numNamedLocals(); ++id) {
    TypedValue* ptv = frame_local(fp, id);
    if (ptv->m_type == KindOfUninit) {
      continue;
    }
    Variant name(func->localVarName(id)->data());
    ret.add(name, tvAsVariant(ptv));
  }
  return ret;
}

void VMExecutionContext::shuffleMagicArgs(ActRec* ar) {
  // We need to put this where the first argument is
  StringData* invName = ar->getInvName();
  int nargs = ar->numArgs();
  ar->setVarEnv(NULL);
  ASSERT(!ar->hasVarEnv() && !ar->hasInvName());
  // We need to make an array containing all the arguments passed by the
  // caller and put it where the second argument is
  HphpArray* argArray = pack_args_into_array(ar, nargs);
  argArray->incRefCount();
  // Remove the arguments from the stack
  for (int i = 0; i < nargs; ++i) {
    m_stack.popC();
  }
  // Move invName to where the first argument belongs, no need
  // to incRef/decRef since we are transferring ownership
  m_stack.pushStringNoRc(invName);
  // Move argArray to where the second argument belongs. We've already
  // incReffed the array above so we don't need to do it here.
  m_stack.pushArrayNoRc(argArray);

  ar->setNumArgs(2);
}

static inline void checkStack(Stack& stk, const Func* f) {
  ThreadInfo* info = ThreadInfo::s_threadInfo.getNoCheck();
  // Check whether func's maximum stack usage would overflow the stack.
  // Both native and VM stack overflows are independently possible.
  if (!stack_in_bounds(info) ||
      stk.wouldOverflow(f->maxStackCells())) {
    TRACE(1, "Maximum VM stack depth exceeded.\n");
    raise_error("Stack overflow");
  }
}

template <bool reenter, bool handle_throw>
bool VMExecutionContext::prepareFuncEntry(ActRec *ar,
                                          PC& pc,
                                          ExtraArgs* extraArgs) {
  const Func* func = ar->m_func;
  if (!reenter) {
    // For the reenter case, intercept and magic shuffling are handled
    // in invokeFunc() before calling prepareFuncEntry(), so we only
    // need to perform these checks for the non-reenter case.
    if (UNLIKELY(func->maybeIntercepted())) {
      Variant *h = get_intercept_handler(func->fullNameRef(),
                                         &func->maybeIntercepted());
      if (h && !run_intercept_handler<handle_throw>(ar, h)) {
        return false;
      }
    }
    if (UNLIKELY(ar->hasInvName())) {
      shuffleMagicArgs(ar);
    }
  }
  // It is now safe to access m_varEnv directly
  ASSERT(!ar->hasInvName());
  int nargs = ar->numArgs();
  // Set pc below, once we know that DV dispatch is unnecessary.
  m_fp = ar;
  bool raiseMissingArgumentWarnings = false;
  int nparams = func->numParams();
  Offset firstDVInitializer = InvalidAbsoluteOffset;
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
      ASSERT(m_fp->m_func == func);
    } else {
      // For the reenter case, extra arguments are handled below (with
      // the extraArgs vector passed to this function).  The below
      // handles pulling extra args from the execution stack in a
      // non-reentry case.
      if (!reenter) {
        if (func->attrs() & AttrMayUseVV) {
          // If there are extra parameters then we cannot be a pseudomain
          // inheriting a VarEnv
          ASSERT(!m_fp->m_varEnv);
          // Extra parameters must be moved off the stack.
          const int numExtras = nargs - nparams;
          m_fp->setExtraArgs(ExtraArgs::allocateCopy(
            (TypedValue*)(uintptr_t(m_fp) - nargs * sizeof(TypedValue)),
            numExtras));
          for (int i = 0; i < numExtras; i++) {
            m_stack.discard();
          }
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
  pushLocalsAndIterators(func, nparams);

  /*
   * If we're reentering, make sure to finalize the ActRec before
   * possibly raising any exceptions, so unwinding won't get confused.
   */
  if (reenter) {
    if (ar->hasVarEnv()) {
      // If this is a pseudomain inheriting a VarEnv from our caller,
      // there cannot be extra arguments
      ASSERT(!extraArgs);
      // Now that locals have been initialized, it is safe to attach
      // the VarEnv inherited from our caller to the current frame
      ar->m_varEnv->attach(ar);
    } else if (extraArgs) {
      // Create an ExtraArgs structure and stash the extra args in
      // there.
      ar->setExtraArgs(extraArgs);
    }
  }

  // cppext functions/methods have their own logic for raising
  // warnings for missing arguments, so we only need to do this work
  // for non-cppext functions/methods
  if (raiseMissingArgumentWarnings && !func->isBuiltin()) {
    pc = func->getEntry();
    // m_pc is not set to callee. if the callee is in a different unit,
    // debugBacktrace() can barf in unit->offsetOf(m_pc) where it
    // asserts that m_pc >= m_bc && m_pc < m_bc + m_bclen. Sync m_fp
    // to function entry point in called unit.
    SYNC();
    const Func::ParamInfoVec& paramInfo = func->params();
    for (int i = nargs; i < nparams; ++i) {
      Offset dvInitializer = paramInfo[i].funcletOff();
      if (dvInitializer == InvalidAbsoluteOffset) {
        raise_warning("Missing argument %d to %s()",
                      i + 1, func->name()->data());
      }
    }
  }

  if (firstDVInitializer != InvalidAbsoluteOffset) {
    pc = func->unit()->entry() + firstDVInitializer;
  } else {
    pc = func->getEntry();
  }
  return true;
}

void VMExecutionContext::syncGdbState() {
  if (RuntimeOption::EvalJit && !RuntimeOption::EvalJitNoGdb) {
    tx64->m_debugInfo.debugSync();
  }
}

void VMExecutionContext::enterVMWork(ActRec* enterFnAr) {
  if (enterFnAr) {
    EventHook::FunctionEnter(enterFnAr, EventHook::NormalFunc);
    INST_HOOK_FENTRY(enterFnAr->m_func->fullName());
  }
  Stats::inc(Stats::VMEnter);
  if (RuntimeOption::EvalJit &&
      !shouldProfile() &&
      !ThreadInfo::s_threadInfo->m_reqInjectionData.coverage &&
      !(RuntimeOption::EvalJitDisabledByHphpd && isDebuggerAttached()) &&
      LIKELY(!DEBUGGER_FORCE_INTR)) {
    Transl::SrcKey sk(Transl::curFunc(), m_pc);
    (void) curUnit()->offsetOf(m_pc); /* assert */
    tx64->resume(sk);
  } else {
    dispatch();
  }
}

// Enumeration codes for the handling of VM exceptions.
enum {
  EXCEPTION_START = 0,
  EXCEPTION_PROPAGATE,
  EXCEPTION_RESUMEVM,
  EXCEPTION_DEBUGGER
};

static void pushFault(Fault::FaultType t, Exception* e, Object* o = NULL) {
  VMExecutionContext* ec = g_vmContext;
  Fault fault;
  fault.m_faultType = t;
  if (t == Fault::KindOfUserException) {
    // User object.
    ASSERT(o);
    fault.m_userException = o->get();
    fault.m_userException->incRefCount();
  } else {
    fault.m_cppException = e;
  }
  ec->m_faults.push_back(fault);
}

static int exception_handler() {
  int longJmpType;
  try {
    throw;
  } catch (Object& e) {
    pushFault(HPHP::VM::Fault::KindOfUserException, NULL, &e);
    longJmpType = g_vmContext->hhvmPrepareThrow();
  } catch (VMSwitchModeException &e) {
    longJmpType = g_vmContext->switchMode(e.unwindBuiltin());
  } catch (Exception &e) {
    pushFault(HPHP::VM::Fault::KindOfCPPException, e.clone());
    longJmpType = g_vmContext->hhvmPrepareThrow();
  } catch (...) {
    pushFault(HPHP::VM::Fault::KindOfCPPException,
              new Exception("unknown exception"));
    longJmpType = g_vmContext->hhvmPrepareThrow();
  }
  return longJmpType;
}

void VMExecutionContext::enterVM(TypedValue* retval,
                                 ActRec* ar,
                                 ExtraArgs* extraArgs) {
  m_firstAR = ar;
  ar->m_savedRip = (uintptr_t)tx64->getCallToExit();

  /*
   * TODO(#1343044): some of the structure of this code dates back to
   * when it used to be setjmp/longjmp based.  It is probable we could
   * simplify it a little more, and maybe combine some of the logic
   * with exception_handler().
   *
   * When an exception is propagating, each nesting of the VM is
   * responsible for unwinding its portion of the execution stack, and
   * finding user handlers if it is a catchable exception.
   *
   * This try/catch is where all this logic is centered.  The actual
   * unwinding happens under hhvmPrepareThrow, which returns a new
   * "jumpCode" here to indicate what to do next.  Either we'll enter
   * the VM loop again at a user error/fault handler, or propagate the
   * exception to a less-nested VM.
   */
  int jumpCode = EXCEPTION_START;
short_jump:
  try {
    switch (jumpCode) {
    case EXCEPTION_START:
      if (prepareFuncEntry<true,true>(ar, m_pc, extraArgs)) {
        enterVMWork(ar);
      }
      break;
    case EXCEPTION_PROPAGATE:
      // Jump out of this try/catch before throwing.
      goto propagate;
    case EXCEPTION_DEBUGGER:
      // Triggered by switchMode() to switch VM mode
      // do nothing but reenter the VM with same VM stack
      /* Fallthrough */
    case EXCEPTION_RESUMEVM:
      enterVMWork(0);
      break;
    default:
      NOT_REACHED();
    }
  } catch (const VMPrepareThrow&) {
    jumpCode = hhvmPrepareThrow();
    goto short_jump;
  } catch (const VMPrepareUnwind&) {
    /*
     * This is slightly different from VMPrepareThrow, because we need
     * to get the offset for the EHEnt that raised the exception using
     * findFaultPCFromEH.
     */
    Offset faultPC = m_fp->m_func->findFaultPCFromEH(pcOff());
    Fault fault = m_faults.back();
    UnwindStatus unwindType = m_stack.unwindFrame(m_fp, faultPC, m_pc, fault);
    jumpCode = handleUnwind(unwindType);
    goto short_jump;
  } catch (const HaltVM&) {
  } catch (...) {
    ASSERT(tl_regState == REGSTATE_CLEAN);
    jumpCode = exception_handler();
    ASSERT(jumpCode != EXCEPTION_START);
    goto short_jump;
  }

  *retval = *m_stack.topTV();
  m_stack.discard();
  return;

propagate:
  ASSERT(m_faults.size() > 0);
  Fault fault = m_faults.back();
  m_faults.pop_back();
  switch (fault.m_faultType) {
  case Fault::KindOfUserException: {
    Object obj = fault.m_userException;
    fault.m_userException->decRefCount();
    throw obj;
  }
  case Fault::KindOfCPPException:
    // throwException() will take care of deleting heap-allocated
    // exception object for us
    fault.m_cppException->throwException();
    NOT_REACHED();
  default:
    not_implemented();
  }
  NOT_REACHED();
}

void VMExecutionContext::reenterVM(TypedValue* retval,
                                   ActRec* ar,
                                   ExtraArgs* extraArgs,
                                   TypedValue* savedSP) {
  ar->m_soff = 0;
  ar->m_savedRbp = 0;
  VMState savedVM = { getPC(), getFP(), m_firstAR, savedSP };
  TRACE(3, "savedVM: %p %p %p %p\n", m_pc, m_fp, m_firstAR, savedSP);
  pushVMState(savedVM, ar);
  ASSERT(m_nestedVMs.size() >= 1);
  try {
    enterVM(retval, ar, extraArgs);
    popVMState();
  } catch (...) {
    popVMState();
    throw;
  }
  TRACE(1, "Reentry: exit fp %p pc %p\n", m_fp, m_pc);
}

int VMExecutionContext::switchMode(bool unwindBuiltin) {
  if (unwindBuiltin) {
    // from Jit calling a builtin, should unwind a frame, and push a
    // return value on stack
    tx64->sync(); // just to set tl_regState
    unwindBuiltinFrame();
    m_stack.pushNull();
  }
  return EXCEPTION_DEBUGGER;
}

void VMExecutionContext::invokeFunc(TypedValue* retval,
                                    const Func* f,
                                    CArrRef params,
                                    ObjectData* this_ /* = NULL */,
                                    Class* cls /* = NULL */,
                                    VarEnv* varEnv /* = NULL */,
                                    StringData* invName /* = NULL */,
                                    Unit* toMerge /* = NULL */) {
  ASSERT(retval);
  ASSERT(f);
  // If this is a regular function, this_ and cls must be NULL
  ASSERT(f->preClass() || f->isPseudoMain() || (!this_ && !cls));
  // If this is a method, either this_ or cls must be non-NULL
  ASSERT(!f->preClass() || (this_ || cls));
  // If this is a static method, this_ must be NULL
  ASSERT(!(f->attrs() & HPHP::VM::AttrStatic) || (!this_));
  // invName should only be non-NULL if we are calling __call or
  // __callStatic
  ASSERT(!invName || f->name()->isame(s___call.get()) ||
         f->name()->isame(s___callStatic.get()));
  // If a variable environment is being inherited then params must be empty
  ASSERT(!varEnv || params.empty());

  VMRegAnchor _;

  // Check if we need to run an intercept handler
  if (UNLIKELY(f->maybeIntercepted())) {
    Variant *h = get_intercept_handler(f->fullNameRef(),
                                       &f->maybeIntercepted());
    if (h) {
      if (!run_intercept_handler_for_invokefunc(retval, f, params, this_,
                                                invName, h)) {
        return;
      }
      // Discard the handler's return value
      tvRefcountedDecRef(retval);
    }
  }

  bool isMagicCall = (invName != NULL);

  if (this_ != NULL) {
    this_->incRefCount();
  }
  Cell* savedSP = m_stack.top();

  checkStack(m_stack, f);

  if (toMerge != NULL) {
    ASSERT(f->unit() == toMerge && f->isPseudoMain());
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
    ar->setThis(NULL);
  }
  if (isMagicCall) {
    ar->initNumArgs(2);
  } else {
    ar->initNumArgs(params.size());
  }
  ar->setVarEnv(varEnv);

#ifdef HPHP_TRACE
  if (m_fp == NULL) {
    TRACE(1, "Reentry: enter %s(%p) from top-level\n",
          f->name()->data(), ar);
  } else {
    TRACE(1, "Reentry: enter %s(pc %p ar %p) from %s(%p)\n",
          f->name()->data(), m_pc, ar,
          m_fp->m_func ? m_fp->m_func->name()->data() : "unknownBuiltin", m_fp);
  }
#endif

  HphpArray *arr = dynamic_cast<HphpArray*>(params.get());
  ExtraArgs* extraArgs = nullptr;
  if (isMagicCall) {
    // Put the method name into the location of the first parameter. We
    // are transferring ownership, so no need to incRef/decRef here.
    m_stack.pushStringNoRc(invName);
    // Put array of arguments into the location of the second parameter
    m_stack.pushArray(arr);
  } else {
    Array hphpArrCopy(HphpArray::GetStaticEmptyArray());
    if (UNLIKELY(!arr) && !params.empty()) {
      // empty() check needed because we sometimes represent empty arrays
      // as smart pointers with m_px == NULL, which freaks out
      // ArrayData::merge.
      hphpArrCopy.merge(params);
      arr = dynamic_cast<HphpArray*>(hphpArrCopy.get());
      ASSERT(arr && IsHphpArray(arr));
    }
    if (arr) {
      const int numParams = f->numParams();
      const int numExtraArgs = arr->size() - numParams;
      if (numExtraArgs > 0 && (f->attrs() & AttrMayUseVV)) {
        extraArgs = ExtraArgs::allocateUninit(numExtraArgs);
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
            ASSERT(extraArgs == NULL);
            ar->setNumArgs(numParams);
            break;
          }
          ASSERT(extraArgs != NULL && numExtraArgs > 0);
          // VarEnv expects the extra args to be in "reverse" order
          // (i.e. the last extra arg has the lowest address)
          to = extraArgs->getExtraArg(paramId - numParams);
        }
        if (LIKELY(!f->byRef(paramId))) {
          tvDup(from, to);
          if (to->m_type == KindOfRef) {
            tvUnbox(to);
          }
        } else {
          if (from->m_type != KindOfRef) {
            tvBox(from);
          }
          tvDup(from, to);
        }
      }
    }
  }

  if (m_fp) {
    reenterVM(retval, ar, extraArgs, savedSP);
  } else {
    ASSERT(m_nestedVMs.size() == 0);
    enterVM(retval, ar, extraArgs);
  }
}

void VMExecutionContext::invokeUnit(TypedValue* retval, Unit* unit) {
  Func* func = unit->getMain();
  if (!m_globalVarEnv) {
    VarEnv::createGlobal();
  }
  invokeFunc(retval, func, Array::Create(), NULL, NULL,
             m_globalVarEnv, NULL, unit);
}

void VMExecutionContext::unwindBuiltinFrame() {
  // Unwind the frame for a builtin. Currently only used for
  // hphpd_break and fb_enable_code_coverage
  ASSERT(m_fp->m_func->isBuiltin());
  ASSERT(m_fp->m_func->name()->isame(s_hphpd_break.get()) ||
         m_fp->m_func->name()->isame(s_fb_enable_code_coverage.get()));
  // Free any values that may be on the eval stack
  TypedValue *evalTop = (TypedValue*)getFP();
  while (m_stack.topTV() < evalTop) {
    m_stack.popTV();
  }
  // Free the locals and VarEnv if there is one
  frame_free_locals_inl(m_fp, m_fp->m_func->numLocals());
  // Tear down the frame
  Offset pc = -1;
  ActRec* sfp = getPrevVMState(m_fp, &pc);
  ASSERT(pc != -1);
  m_fp = sfp;
  m_pc = m_fp->m_func->unit()->at(pc);
  m_stack.discardAR();
}

int VMExecutionContext::hhvmPrepareThrow() {
  Fault fault = m_faults.back();
  tx64->sync();
  TRACE(2, "hhvmPrepareThrow: %p(\"%s\") {\n", m_fp,
           m_fp->m_func->name()->data());
  UnwindStatus unwindType;
  unwindType = m_stack.unwindFrame(m_fp, pcOff(),
                                   m_pc, fault);
  return handleUnwind(unwindType);
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
                                           Offset*       prevPc /* = NULL */,
                                           TypedValue**  prevSp /* = NULL */) {
  if (fp == NULL) {
    return NULL;
  }
  ActRec* prevFp = arGetSfp(fp);
  if (prevFp != fp) {
    if (prevSp) {
      if (UNLIKELY(fp->m_func->isGenerator())) {
        *prevSp = (TypedValue*)prevFp - prevFp->m_func->numSlotsInFrame();
      } else {
        *prevSp = (TypedValue*)&fp[1];
      }
    }
    if (prevPc) *prevPc = prevFp->m_func->base() + fp->m_soff;
    return prevFp;
  }
  // Linear search from end of m_nestedVMs. In practice, we're probably
  // looking for something recently pushed.
  int i = m_nestedVMs.size() - 1;
  for (; i >= 0; --i) {
    if (m_nestedVMs[i].m_entryFP == fp) break;
  }
  if (i == -1) return NULL;
  const VMState& vmstate = m_nestedVMs[i].m_savedState;
  prevFp = vmstate.fp;
  ASSERT(prevFp);
  ASSERT(prevFp->m_func->unit());
  if (prevSp) *prevSp = vmstate.sp;
  if (prevPc) {
    *prevPc = prevFp->m_func->unit()->offsetOf(vmstate.pc);
  }
  return prevFp;
}

Array VMExecutionContext::debugBacktrace(bool skip /* = false */,
                                         bool withSelf /* = false */,
                                         bool withThis /* = false */,
                                         VMParserFrame*
                                         parserFrame /* = NULL */) {
  static StringData* s_file = StringData::GetStaticString("file");
  static StringData* s_line = StringData::GetStaticString("line");
  static StringData* s_function = StringData::GetStaticString("function");
  static StringData* s_args = StringData::GetStaticString("args");
  static StringData* s_class = StringData::GetStaticString("class");
  static StringData* s_object = StringData::GetStaticString("object");
  static StringData* s_type = StringData::GetStaticString("type");
  static StringData* s_include = StringData::GetStaticString("include");

  Array bt = Array::Create();

  // If there is a parser frame, put it at the beginning of
  // the backtrace
  if (parserFrame) {
    Array frame = Array::Create();
    frame.set(String(s_file), parserFrame->filename, true);
    frame.set(String(s_line), parserFrame->lineNumber, true);
    bt.append(frame);
  }
  Transl::VMRegAnchor _;
  if (!getFP()) {
    // If there are no VM frames, we're done
    return bt;
  }
  // Get the fp and pc of the top frame (possibly skipping one frame)
  ActRec* fp;
  Offset pc;
  if (skip) {
    fp = getPrevVMState(getFP(), &pc);
    if (!fp) {
      // We skipped over the only VM frame, we're done
      return bt;
    }
  } else {
    fp = getFP();
    Unit *unit = getFP()->m_func->unit();
    ASSERT(unit);
    pc = unit->offsetOf(m_pc);
  }
  int depth = 0;
  // Handle the top frame
  if (withSelf) {
    // Builtins don't have a file and line number
    if (!fp->m_func->isBuiltin()) {
      Unit *unit = fp->m_func->unit();
      ASSERT(unit);
      const char* filename = unit->filepath()->data();
      ASSERT(filename);
      Offset off = pc;
      Array frame = Array::Create();
      frame.set(String(s_file), filename, true);
      frame.set(String(s_line), unit->getLineNumber(off), true);
      if (parserFrame) {
        frame.set(String(s_function), String(s_include), true);
        frame.set(String(s_args), Array::Create(parserFrame->filename), true);
      }
      bt.append(frame);
      depth++;
    }
  }
  // Handle the subsequent VM frames
  for (ActRec* prevFp = getPrevVMState(fp, &pc); fp != NULL;
       fp = prevFp, prevFp = getPrevVMState(fp, &pc)) {
    Array frame = Array::Create();
    // do not capture frame for HPHP only functions
    if (fp->m_func->isNoInjection()) {
      continue;
    }
    // Builtins don't have a file and line number
    if (prevFp && !prevFp->m_func->isBuiltin()) {
      Unit* unit = prevFp->m_func->unit();
      ASSERT(unit);
      const char *filename = unit->filepath()->data();
      ASSERT(filename);
      frame.set(String(s_file), filename, true);
      frame.set(String(s_line),
                prevFp->m_func->unit()->getLineNumber(pc), true);
    }
    // check for include
    StringData *funcname = const_cast<StringData*>(fp->m_func->name());
    if (fp->m_func->isClosureBody()) {
      static StringData* s_closure_label =
          StringData::GetStaticString("{closure}");
      funcname = s_closure_label;
    }
    // check for pseudomain
    if (funcname->empty()) {
      if (!prevFp) continue;
      funcname = s_include;
    }
    frame.set(String(s_function), String(funcname), true);
    if (funcname != s_include) {
      // Closures have an m_this but they aren't in object context
      Class* ctx = arGetContextClass(fp);
      if (ctx != NULL && !fp->m_func->isClosureBody()) {
        frame.set(String(s_class), ctx->name()->data(), true);
        if (fp->hasThis()) {
          if (withThis) {
            frame.set(String(s_object), Object(fp->getThis()), true);
          }
          frame.set(String(s_type), "->", true);
        } else {
          frame.set(String(s_type), "::", true);
        }
      }
    }
    Array args = Array::Create();
    if (funcname == s_include) {
      if (depth) {
        args.append(String(const_cast<StringData*>(
                             fp->m_func->unit()->filepath())));
        frame.set(String(s_args), args, true);
      }
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
      frame.set(String(s_args), args, true);
    }
    bt.append(frame);
    depth++;
  }
  return bt;
}

MethodInfoVM::~MethodInfoVM() {
  for (std::vector<const ClassInfo::ParameterInfo*>::iterator it =
       parameters.begin(); it != parameters.end(); ++it) {
    if ((*it)->value != NULL) {
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

Array VMExecutionContext::getConstantsInfo() {
  // Return an array of all defined constant:value pairs.  This method is used
  // to support get_defined_constants().
  return Array(m_constants.copy());
}

const ClassInfo::MethodInfo* VMExecutionContext::findFunctionInfo(
  CStrRef name) {
  StringIMap<AtomicSmartPtr<MethodInfoVM> >::iterator it =
    m_functionInfos.find(name);
  if (it == m_functionInfos.end()) {
    Func* func = Unit::lookupFunc(name.get());
    if (func == NULL || func->builtinFuncPtr()) {
      // Fall back to the logic in ClassInfo::FindFunction() logic to deal
      // with builtin functions
      return NULL;
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
  if (name->empty()) return NULL;
  StringIMap<AtomicSmartPtr<ClassInfoVM> >::iterator it =
    m_classInfos.find(name);
  if (it == m_classInfos.end()) {
    Class* cls = Unit::lookupClass(name.get());
    if (cls == NULL) return NULL;
    if (cls->clsInfo()) return cls->clsInfo();
    if (cls->attrs() & (AttrInterface | AttrTrait)) {
      // If the specified name matches with something that is not formally
      // a class, return NULL
      return NULL;
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
    if (cls == NULL)  return NULL;
    if (cls->clsInfo()) return cls->clsInfo();
    if (!(cls->attrs() & AttrInterface)) {
      // If the specified name matches with something that is not formally
      // an interface, return NULL
      return NULL;
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
  if (cls == NULL) return NULL;
  if (cls->clsInfo()) return cls->clsInfo();
  if (!(cls->attrs() & AttrTrait)) {
    return NULL;
  }
  AtomicSmartPtr<ClassInfoVM> &classInfo = m_traitInfos[name];
  classInfo = new ClassInfoVM();
  cls->getClassInfo(classInfo.get());
  return classInfo.get();
}

const ClassInfo::ConstantInfo* VMExecutionContext::findConstantInfo(
    CStrRef name) {
  TypedValue* tv = m_constants.nvGet(name.get());
  if (tv == NULL) {
    return NULL;
  }
  ConstInfoMap::const_iterator it = m_constInfo.find(name.get());
  if (it != m_constInfo.end()) {
    return it->second;
  }
  StringData* key = StringData::GetStaticString(name.get());
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
  if (spath.isNull()) return NULL;

  // Check if this file has already been included.
  EvaledFilesMap::const_iterator it = m_evaledFiles.find(spath.get());
  HPHP::Eval::PhpFile* efile = NULL;
  if (it != m_evaledFiles.end()) {
    // We found it! Return the unit.
    efile = it->second;
    initial = false;
    if (!initial_opt) efile->incRef();
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
      rpath = NEW(StringData)(rp.data(), rp.size(), CopyString);
      if (!rpath.same(spath)) {
        hasRealpath = true;
        it = m_evaledFiles.find(rpath.get());
        if (it != m_evaledFiles.end()) {
          // We found it! Update the mapping for spath and
          // return the unit.
          efile = it->second;
          m_evaledFiles[spath.get()] = efile;
          spath.get()->incRefCount();
          efile->incRef();
          initial = false;
          if (!initial_opt) efile->incRef();
          return efile;
        }
      }
    }
  }
  // This file hasn't been included yet, so we need to parse the file
  efile = HPHP::Eval::FileRepository::checkoutFile(
    hasRealpath ? rpath.get() : spath.get(), s);
  ASSERT(!efile || efile->getRef() > 0);
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
      efile->incRef();
    }
    DEBUGGER_ATTACHED_ONLY(phpFileLoadHook(efile));
  }
  return efile;
}

Unit* VMExecutionContext::evalInclude(StringData* path,
                                      const StringData* curUnitFilePath,
                                      bool* initial) {
  namespace fs = boost::filesystem;
  fs::path currentUnit(curUnitFilePath->data());
  fs::path currentDir(currentUnit.branch_path());
  HPHP::Eval::PhpFile* efile =
    lookupPhpFile(path, currentDir.string().c_str(), initial);
  if (efile) {
    return efile->unit();
  }
  return NULL;
}

HPHP::VM::Unit* VMExecutionContext::evalIncludeRoot(
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
    ASSERT(flags & InclOpDocRoot);
    absPath = SourceRootInfo::GetCurrentPhpRoot();
    TRACE(2, "lookupIncludeRoot(%s): docRoot -> %s\n",
          path->data(),
          absPath->data());
  }

  absPath += StrNR(path);

  EvaledFilesMap::const_iterator it = m_evaledFiles.find(absPath.get());
  if (it != m_evaledFiles.end()) {
    if (initial) *initial = false;
    if (!initial) it->second->incRef();
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
bool VMExecutionContext::evalUnit(Unit* unit, bool local,
                                  PC& pc, int funcType) {
  m_pc = pc;
  unit->merge();
  if (unit->isMergeOnly()) {
    Stats::inc(Stats::PseudoMain_Skipped);
    *m_stack.allocTV() = *unit->getMainReturn();
    return false;
  }
  Stats::inc(Stats::PseudoMain_Executed);


  ActRec* ar = m_stack.allocA();
  ASSERT((uintptr_t)&ar->m_func < (uintptr_t)&ar->m_r);
  VM::Class* cls = curClass();
  if (local) {
    cls = NULL;
    ar->setThis(NULL);
  } else if (m_fp->hasThis()) {
    ObjectData *this_ = m_fp->getThis();
    this_->incRefCount();
    ar->setThis(this_);
  } else if (m_fp->hasClass()) {
    ar->setClass(m_fp->getClass());
  } else {
    ar->setThis(NULL);
  }
  Func* func = unit->getMain(cls);
  ASSERT(!func->isBuiltin());
  ASSERT(!func->isGenerator());
  ar->m_func = func;
  ar->initNumArgs(0);
  ASSERT(getFP());
  ASSERT(!m_fp->hasInvName());
  arSetSfp(ar, m_fp);
  ar->m_soff = uintptr_t(m_fp->m_func->unit()->offsetOf(pc) -
                         m_fp->m_func->base());
  ar->m_savedRip = (uintptr_t)tx64->getRetFromInterpretedFrame();
  pushLocalsAndIterators(func);
  if (local) {
    ar->m_varEnv = 0;
  } else {
    if (!m_fp->hasVarEnv()) {
      m_fp->m_varEnv = VarEnv::createLazyAttach(m_fp);
    }
    ar->m_varEnv = m_fp->m_varEnv;
    ar->m_varEnv->attach(ar);
  }
  m_fp = ar;
  pc = func->getEntry();
  SYNC();
  EventHook::FunctionEnter(m_fp, funcType);
  return true;
}

CVarRef VMExecutionContext::getEvaledArg(const StringData* val) {
  CStrRef key = *(String*)&val;

  if (m_evaledArgs.get()) {
    CVarRef arg = m_evaledArgs.get()->get(key);
    if (&arg != &null_variant) return arg;
  }
  String code = HPHP::concat3("<?php return ", key, ";");
  VM::Unit* unit = compileEvalString(code.get());
  ASSERT(unit != NULL);
  Variant v;
  // Default arg values are not currently allowed to depend on class context.
  g_vmContext->invokeFunc((TypedValue*)&v, unit->getMain(),
                          Array::Create());
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
      ObjectData* o = *m_liveBCObjs.begin();
      Instance* instance = static_cast<Instance*>(o);
      instance->destruct(); // Let the instance remove the node.
    }
    m_liveBCObjs.clear();
  }
}

// Evaled units have a footprint in the TC and translation metadata. The
// applications we care about tend to have few, short, stereotyped evals,
// where the same code keeps getting eval'ed over and over again; so we
// keep around units for each eval'ed string, so that the TC space isn't
// wasted on each eval.
typedef RankedCHM<StringData*, HPHP::VM::Unit*,
        StringDataHashCompare,
        RankEvaledUnits> EvaledUnitsMap;
static EvaledUnitsMap s_evaledUnits;
Unit* VMExecutionContext::compileEvalString(StringData* code) {
  EvaledUnitsMap::accessor acc;
  // Promote this to a static string; otherwise it may get swept
  // across requests.
  code = StringData::GetStaticString(code);
  if (s_evaledUnits.insert(acc, code)) {
    acc->second = compile_string(code->data(), code->size());
  }
  return acc->second;
}

CStrRef VMExecutionContext::createFunction(CStrRef args, CStrRef code) {
  // It doesn't matter if there's a user function named __lambda_func; we only
  // use this name during parsing, and then change it to an impossible name
  // with a NUL byte before we merge it into the request's func map.  This also
  // has the bonus feature that the value of __FUNCTION__ inside the created
  // function will match Zend. (Note: Zend will actually fatal if there's a
  // user function named __lambda_func when you call create_function. Huzzah!)
  static StringData* oldName = StringData::GetStaticString("__lambda_func");
  std::ostringstream codeStr;
  codeStr << "<?php function " << oldName->data()
          << "(" << args.data() << ") {"
          << code.data() << "}\n";
  StringData* evalCode = StringData::GetStaticString(codeStr.str());
  Unit* unit = VM::compile_string(evalCode->data(), evalCode->size());
  // Move the function to a different name.
  std::ostringstream newNameStr;
  newNameStr << '\0' << "lambda_" << ++m_lambdaCounter;
  StringData* newName = StringData::GetStaticString(newNameStr.str());
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
  invokeFunc(&retval, unit->getMain(), Array::Create());

  return unit->getLambda()->nameRef();
}

void VMExecutionContext::evalPHPDebugger(TypedValue* retval, StringData *code,
                                         int frame) {
  ASSERT(retval);
  // The code has "<?php" prepended already
  Unit* unit = compileEvalString(code);
  if (unit == NULL) {
    raise_error("Syntax error");
    tvWriteNull(retval);
    return;
  }

  VarEnv *varEnv = NULL;
  ActRec *fp = getFP();
  ActRec *cfpSave = NULL;
  if (fp) {
    VM::VarEnv* vit = 0;
    for (; frame > 0; --frame) {
      if (fp->hasVarEnv()) {
        if (!vit) {
          vit = m_topVarEnv;
        } else if (vit != fp->m_varEnv) {
          vit = vit->previous();
        }
        ASSERT(vit == fp->m_varEnv);
      }
      ActRec* prevFp = getPrevVMState(fp);
      if (!prevFp) {
        // To be safe in case we failed to get prevFp
        // XXX: it's unclear why this is possible, but it was
        // causing some crashes.
        break;
      }
      fp = prevFp;
    }
    if (!fp->hasVarEnv()) {
      if (!vit) {
        fp->m_varEnv = VarEnv::createLazyAttach(fp);
      } else {
        const bool skipInsert = true;
        fp->m_varEnv = VarEnv::createLazyAttach(fp, skipInsert);
        // Slide it in front of the VarEnv most recently above it.
        fp->m_varEnv->setPrevious(vit->previous());
        vit->setPrevious(fp->m_varEnv);
      }
    }
    varEnv = fp->m_varEnv;
    cfpSave = varEnv->getCfp();
  }
  ObjectData *this_ = NULL;
  Class *cls = NULL;
  if (fp) {
    if (fp->hasThis()) {
      this_ = fp->getThis();
    } else if (fp->hasClass()) {
      cls = fp->getClass();
    }
    phpDebuggerEvalHook(fp->m_func);
  }

  const static StaticString s_cppException("Hit an exception");
  const static StaticString s_phpException("Hit a php exception");
  const static StaticString s_exit("Hit exit");
  const static StaticString s_fatal("Hit fatal");
  try {
    invokeFunc(retval, unit->getMain(fp->m_func->cls()), Array::Create(),
               this_, cls, varEnv, NULL, unit);
  } catch (FatalErrorException &e) {
    g_vmContext->write(s_fatal);
    g_vmContext->write(" : ");
    g_vmContext->write(e.getMessage().c_str());
    g_vmContext->write("\n");
    g_vmContext->write(ExtendedLogger::StringOfStackTrace(*e.getBackTrace()));
  } catch (ExitException &e) {
    g_vmContext->write(s_exit.data());
    g_vmContext->write(" : ");
    std::ostringstream os;
    os << ExitException::ExitCode;
    g_vmContext->write(os.str());
  } catch (Eval::DebuggerException &e) {
    if (varEnv) {
      varEnv->setCfp(cfpSave);
    }
    throw;
  } catch (Exception &e) {
    g_vmContext->write(s_cppException.data());
    g_vmContext->write(" : ");
    g_vmContext->write(e.getMessage().c_str());
    ExtendedException* ee = dynamic_cast<ExtendedException*>(&e);
    if (ee) {
      g_vmContext->write("\n");
      g_vmContext->write(
        ExtendedLogger::StringOfStackTrace(*ee->getBackTrace()));
    }
  } catch (Object &e) {
    g_vmContext->write(s_phpException.data());
    g_vmContext->write(" : ");
    g_vmContext->write(e->t___tostring().data());
  } catch (...) {
    g_vmContext->write(s_cppException.data());
  }

  if (varEnv) {
    // The debugger eval frame may have attached to the VarEnv from a
    // frame that was not the top frame, so we need to manually set
    // cfp back to what it was before
    varEnv->setCfp(cfpSave);
  }
}

void VMExecutionContext::enterDebuggerDummyEnv() {
  static Unit* s_debuggerDummy = NULL;
  if (!s_debuggerDummy) {
    s_debuggerDummy = compile_string("<?php?>", 7);
  }
  if (!m_globalVarEnv) {
    VarEnv::createGlobal();
  }
  VarEnv* varEnv = m_topVarEnv;
  if (!getFP()) {
    ASSERT(m_stack.count() == 0);
    ActRec* ar = m_stack.allocA();
    ar->m_func = s_debuggerDummy->getMain();
    ar->setThis(NULL);
    ar->m_soff = 0;
    ar->m_savedRbp = 0;
    ar->m_savedRip = (uintptr_t)tx64->getCallToExit();
    m_fp = ar;
    m_pc = s_debuggerDummy->entry();
    m_firstAR = ar;
  }
  m_fp->setVarEnv(varEnv);
  varEnv->attach(m_fp);
}

void VMExecutionContext::exitDebuggerDummyEnv() {
  ASSERT(m_topVarEnv);
  ASSERT(m_globalVarEnv == m_topVarEnv);
  m_globalVarEnv->detach(getFP());
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
    ASSERT(!fp->hasInvName());
    if (fp->hasVarEnv()) {
      val = fp->m_varEnv->lookup(name);
    } else {
      val = NULL;
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
    ASSERT(!fp->hasInvName());
    if (!fp->hasVarEnv()) {
      fp->m_varEnv = VarEnv::createLazyAttach(fp);
    }
    val = fp->m_varEnv->lookup(name);
    if (val == NULL) {
      TypedValue tv;
      TV_WRITE_NULL(&tv);
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
  ASSERT(g_vmContext->m_globalVarEnv);
  val = g_vmContext->m_globalVarEnv->lookup(name);
}

static inline void lookupd_gbl(ActRec* fp,
                               StringData*& name,
                               TypedValue* key,
                               TypedValue*& val) {
  name = lookup_name(key);
  ASSERT(g_vmContext->m_globalVarEnv);
  VarEnv* varEnv = g_vmContext->m_globalVarEnv;
  val = varEnv->lookup(name);
  if (val == NULL) {
    TypedValue tv;
    TV_WRITE_NULL(&tv);
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
  ASSERT(clsRef->m_type == KindOfClass);
  name = lookup_name(key);
  Class* ctx = arGetContextClass(fp);
  val = clsRef->m_data.pcls->getSProp(ctx, name, visible, accessible);
}

static inline void lookupClsRef(TypedValue* input,
                                TypedValue* output,
                                bool decRef = false) {
  const Class* class_ = NULL;
  if (IS_STRING_TYPE(input->m_type)) {
    class_ = Unit::loadClass(input->m_data.pstr);
    if (class_ == NULL) {
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
    // We're using pref here arbitrarily; any refcounted union member works.
    return tv->m_data.pref->_count;
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
    ASSERT(result == &tvRef);
    result = &tvRef2;
  }
}

#define DECLARE_GETHELPER_ARGS                  \
  unsigned ndiscard;                            \
  TypedValue* tvRet;                            \
  TypedValue* base;                             \
  bool baseStrOff = false;                      \
  TypedValue tvScratch;                         \
  TypedValue tvLiteral;                         \
  TypedValue tvRef;                             \
  TypedValue tvRef2;                            \
  MemberCode mcode = MEL;                       \
  TypedValue* curMember = 0;
#define GETHELPERPRE_ARGS                                              \
  pc, ndiscard, base, baseStrOff, tvScratch, tvLiteral,                \
    tvRef, tvRef2, mcode, curMember
// The following arguments are outputs:
// pc:         bytecode instruction after the vector instruction
// ndiscard:   number of stack elements to discard
// base:       ultimate result of the vector-get
// baseStrOff: StrOff flag associated with base
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
inline void OPTBLD_INLINE VMExecutionContext::getHelperPre(
    PC& pc,
    unsigned& ndiscard,
    TypedValue*& base,
    bool& baseStrOff,
    TypedValue& tvScratch,
    TypedValue& tvLiteral,
    TypedValue& tvRef,
    TypedValue& tvRef2,
    MemberCode& mcode,
    TypedValue*& curMember) {
  // The caller is responsible for moving pc to point to the vector immediate
  // before calling getHelperPre().
  const ImmVector immVec = ImmVector::createFromStream(pc);
  const uint8_t* vec = immVec.vec();
  ASSERT(immVec.size() > 0);

  // PC needs to be advanced before we do anything, otherwise if we
  // raise a notice in the middle of this we could resume at the wrong
  // instruction.
  pc += immVec.size() + sizeof(int32_t) + sizeof(int32_t);

  ndiscard = immVec.numStackValues();
  int depth = ndiscard - 1;
  const LocationCode lcode = LocationCode(*vec++);

  TypedValue* loc = NULL;
  TypedValue dummy;
  Class* const ctx = arGetContextClass(getFP());

  StringData* name;
  TypedValue* fr = NULL;
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
    lookup_var(m_fp, name, loc, fr);
    if (fr == NULL) {
      if (warn) {
        raise_notice(Strings::UNDEFINED_VARIABLE, name->data());
      }
      TV_WRITE_NULL(&dummy);
      loc = &dummy;
    } else {
      loc = fr;
    }
    LITSTR_DECREF(name);
    break;

  case LGL:
    loc = frame_local_inner(m_fp, decodeVariableSizeImm(&vec));
    goto lcodeGlobal;
  case LGC:
    loc = m_stack.indTV(depth--);
    goto lcodeGlobal;

  lcodeGlobal:
    lookup_gbl(m_fp, name, loc, fr);
    if (fr == NULL) {
      if (warn) {
        raise_notice(Strings::UNDEFINED_VARIABLE, name->data());
      }
      TV_WRITE_NULL(&dummy);
      loc = &dummy;
    } else {
      loc = fr;
    }
    LITSTR_DECREF(name);
    break;

  case LSC:
    cref = m_stack.topTV();
    pname = m_stack.indTV(depth--);
    goto lcodeSprop;
  case LSL:
    cref = m_stack.topTV();
    pname = frame_local_inner(m_fp, decodeVariableSizeImm(&vec));
    goto lcodeSprop;

  lcodeSprop: {
    bool visible, accessible;
    ASSERT(cref->m_type == KindOfClass);
    const Class* class_ = cref->m_data.pcls;
    StringData* name = lookup_name(pname);
    loc = class_->getSProp(ctx, name, visible, accessible);
    if (!(visible && accessible)) {
      raise_error("Invalid static property access: %s::%s",
                  class_->name()->data(),
                  name->data());
    }
    LITSTR_DECREF(name);
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
    ASSERT(m_fp->hasThis());
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
      int64 memberImm = decodeMemberCodeImm(&vec, mcode);
      if (memberCodeImmIsString(mcode)) {
        tvAsVariant(&tvLiteral) =
          m_fp->m_func->unit()->lookupLitstrId(memberImm);
        ASSERT(!IS_REFCOUNTED_TYPE(tvLiteral.m_type));
        curMember = &tvLiteral;
      } else if (mcode == MEI) {
        tvAsVariant(&tvLiteral) = memberImm;
        curMember = &tvLiteral;
      } else {
        ASSERT(memberCodeImmIsLoc(mcode));
        curMember = frame_local_inner(m_fp, memberImm);
      }
    } else {
      curMember = m_stack.indTV(depth--);
    }

    if (mleave == LeaveLast) {
      if (vec >= pc) {
        ASSERT(vec == pc);
        break;
      }
    }

    TypedValue* result;
    switch (mcode) {
    case MEL:
    case MEC:
    case MET:
    case MEI:
      result = Elem<warn>(tvScratch, tvRef, base, baseStrOff, curMember);
      break;
    case MPL:
    case MPC:
    case MPT:
      result = Prop<warn, false, false>(tvScratch, tvRef, ctx, base,
                                        curMember);
      break;
    case MW:
      raise_error("Cannot use [] for reading");
      result = NULL;
      break;
    default:
      ASSERT(false);
      result = NULL; // Silence compiler warning.
    }
    ASSERT(result != NULL);
    ratchetRefs(result, tvRef, tvRef2);
    base = result;
  }

  if (mleave == ConsumeAll) {
    ASSERT(vec == pc);
    if (debug) {
      if (lcode == LSC || lcode == LSL) {
        ASSERT(depth == 0);
      } else {
        ASSERT(depth == -1);
      }
    }
  }

  if (saveResult) {
    // If requested, save a copy of the result.  If base already points to
    // tvScratch, no reference counting is necessary, because (with the
    // exception of the following block), tvScratch is never populated such
    // that it owns a reference that must be accounted for.
    if (base != &tvScratch) {
      // Acquire a reference to the result via tvDup(); base points to the
      // result but does not own a reference.
      tvDup(base, &tvScratch);
    }
  }
}

#define GETHELPERPOST_ARGS ndiscard, tvRet, tvScratch, tvRef, tvRef2
template <bool saveResult>
inline void OPTBLD_INLINE VMExecutionContext::getHelperPost(
    unsigned ndiscard, TypedValue*& tvRet, TypedValue& tvScratch,
    TypedValue& tvRef, TypedValue& tvRef2) {
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
  tvRefcountedDecRef(&tvRef);
  tvRefcountedDecRef(&tvRef2);

  if (saveResult) {
    // If tvRef wasn't just allocated, we've already decref'd it in
    // the loop above.
    memcpy(tvRet, &tvScratch, sizeof(TypedValue));
  }
}

#define GETHELPER_ARGS \
  pc, ndiscard, tvRet, base, baseStrOff, tvScratch, tvLiteral, \
    tvRef, tvRef2, mcode, curMember
inline void OPTBLD_INLINE
VMExecutionContext::getHelper(PC& pc,
                              unsigned& ndiscard,
                              TypedValue*& tvRet,
                              TypedValue*& base,
                              bool& baseStrOff,
                              TypedValue& tvScratch,
                              TypedValue& tvLiteral,
                              TypedValue& tvRef,
                              TypedValue& tvRef2,
                              MemberCode& mcode,
                              TypedValue*& curMember) {
  getHelperPre<true, true, ConsumeAll>(GETHELPERPRE_ARGS);
  getHelperPost<true>(GETHELPERPOST_ARGS);
}

void
VMExecutionContext::getElem(TypedValue* base, TypedValue* key,
                            TypedValue* dest) {
  ASSERT(base->m_type != KindOfArray);
  bool baseStrOff = false;
  VMRegAnchor _;
  TV_WRITE_UNINIT(dest);
  TypedValue* result = Elem<true>(*dest, *dest, base, baseStrOff, key);
  if (result != dest) {
    tvDup(result, dest);
  }
}

#define DECLARE_SETHELPER_ARGS                                                \
  unsigned ndiscard;                                                          \
  TypedValue* base;                                                           \
  bool baseStrOff = false;                                                    \
  TypedValue tvScratch;                                                       \
  TypedValue tvLiteral;                                                      \
  TypedValue tvRef;                                                           \
  TypedValue tvRef2;                                                          \
  MemberCode mcode = MEL;                                                     \
  TypedValue* curMember = 0;
#define SETHELPERPRE_ARGS                                                     \
  pc, ndiscard, base, baseStrOff, tvScratch, tvLiteral, tvRef, tvRef2, \
    mcode, curMember
// The following arguments are outputs:  (TODO put them in struct)
// pc:         bytecode instruction after the vector instruction
// ndiscard:   number of stack elements to discard
// base:       ultimate result of the vector-get
// baseStrOff: StrOff flag associated with base
// tvScratch:  temporary result storage
// tvRef:      temporary result storage
// tvRef2:     temporary result storage
// mcode:      output MemberCode for the last member if LeaveLast
// curMember:  output last member value one if LeaveLast; but undefined
//             if the last mcode == MW
//
// TODO(#1068709) XXX this function should be merged with getHelperPre.
template <bool warn,
          bool define,
          bool unset,
          bool reffy,
          unsigned mdepth, // extra args on stack for set (e.g. rhs)
          VMExecutionContext::VectorLeaveCode mleave>
inline bool OPTBLD_INLINE VMExecutionContext::setHelperPre(
    PC& pc, unsigned& ndiscard, TypedValue*& base,
    bool& baseStrOff, TypedValue& tvScratch, TypedValue& tvLiteral,
    TypedValue& tvRef, TypedValue& tvRef2,
    MemberCode& mcode, TypedValue*& curMember) {
  // The caller must move pc to the vector immediate before calling
  // setHelperPre.
  const ImmVector immVec = ImmVector::createFromStream(pc);
  const uint8_t* vec = immVec.vec();
  ASSERT(immVec.size() > 0);

  // PC needs to be advanced before we do anything, otherwise if we
  // raise a notice in the middle of this we could resume at the wrong
  // instruction.
  pc += immVec.size() + sizeof(int32_t) + sizeof(int32_t);

  ndiscard = immVec.numStackValues();
  int depth = mdepth + ndiscard - 1;
  const LocationCode lcode = LocationCode(*vec++);

  TypedValue* loc = NULL;
  TypedValue dummy;
  Class* const ctx = arGetContextClass(getFP());

  StringData* name;
  TypedValue* fr = NULL;
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
    if (fr == NULL) {
      if (warn) {
        raise_notice(Strings::UNDEFINED_VARIABLE, name->data());
      }
      TV_WRITE_NULL(&dummy);
      loc = &dummy;
    } else {
      loc = fr;
    }
    LITSTR_DECREF(name);
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
    if (fr == NULL) {
      if (warn) {
        raise_notice(Strings::UNDEFINED_VARIABLE, name->data());
      }
      TV_WRITE_NULL(&dummy);
      loc = &dummy;
    } else {
      loc = fr;
    }
    LITSTR_DECREF(name);
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
    ASSERT(cref->m_type == KindOfClass);
    const Class* class_ = cref->m_data.pcls;
    StringData* name = lookup_name(pname);
    loc = class_->getSProp(ctx, name, visible, accessible);
    if (!(visible && accessible)) {
      raise_error("Invalid static property access: %s::%s",
                  class_->name()->data(),
                  name->data());
    }
    LITSTR_DECREF(name);
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
    ASSERT(m_fp->hasThis());
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
      int64 memberImm = decodeMemberCodeImm(&vec, mcode);
      if (memberCodeImmIsString(mcode)) {
        tvAsVariant(&tvLiteral) =
          m_fp->m_func->unit()->lookupLitstrId(memberImm);
        ASSERT(!IS_REFCOUNTED_TYPE(tvLiteral.m_type));
        curMember = &tvLiteral;
      } else if (mcode == MEI) {
        tvAsVariant(&tvLiteral) = memberImm;
        curMember = &tvLiteral;
      } else {
        ASSERT(memberCodeImmIsLoc(mcode));
        curMember = frame_local_inner(m_fp, memberImm);
      }
    } else {
      curMember = mcode == MW ? NULL : m_stack.indTV(depth--);
    }

    if (mleave == LeaveLast) {
      if (vec >= pc) {
        ASSERT(vec == pc);
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
        result = Elem<warn>(tvScratch, tvRef, base, baseStrOff, curMember);
      }
      break;
    case MPL:
    case MPC:
    case MPT:
      result = Prop<warn, define, unset>(tvScratch, tvRef, ctx, base,
                                         curMember);
      break;
    case MW:
      ASSERT(define);
      result = NewElem(tvScratch, tvRef, base);
      break;
    default:
      ASSERT(false);
      result = NULL; // Silence compiler warning.
    }
    ASSERT(result != NULL);
    ratchetRefs(result, tvRef, tvRef2);
    // Check whether an error occurred (i.e. no result was set).
    if (result == &tvScratch && result->m_type == KindOfUninit) {
      return true;
    }
    base = result;
  }

  if (mleave == ConsumeAll) {
    ASSERT(vec == pc);
    if (debug) {
      if (lcode == LSC || lcode == LSL) {
        ASSERT(depth == int(mdepth));
      } else {
        ASSERT(depth == int(mdepth) - 1);
      }
    }
  }

  return false;
}

#define SETHELPERPOST_ARGS ndiscard, tvRef, tvRef2
template <unsigned mdepth>
inline void OPTBLD_INLINE VMExecutionContext::setHelperPost(
    unsigned ndiscard, TypedValue& tvRef, TypedValue& tvRef2) {
  // Clean up the stack.  Decref all the elements for the vector, but
  // leave the first mdepth (they are not part of the vector data).
  for (unsigned depth = mdepth; depth-mdepth < ndiscard; ++depth) {
    TypedValue* tv = m_stack.indTV(depth);
    tvRefcountedDecRef(tv);
  }

  // NOTE: currently the only instructions using this that have return
  // values on the stack also have more inputs than the K-vector, so
  // mdepth > 0.  They also always return the original top value of
  // the stack.
  if (mdepth > 0) {
    ASSERT(mdepth == 1 &&
      "We don't really support mdepth > 1 in setHelperPost");

    TypedValue* retSrc = m_stack.topTV();
    if (ndiscard > 0) {
      TypedValue* dest = m_stack.indTV(ndiscard + mdepth - 1);
      memcpy(dest, retSrc, sizeof *dest);
    }
  }

  m_stack.ndiscard(ndiscard);
  tvRefcountedDecRef(&tvRef);
  tvRefcountedDecRef(&tvRef2);
}

inline void OPTBLD_INLINE VMExecutionContext::iopLowInvalid(PC& pc) {
  fprintf(stderr, "invalid bytecode executed\n");
  abort();
}

inline void OPTBLD_INLINE VMExecutionContext::iopNop(PC& pc) {
  NEXT();
}

inline void OPTBLD_INLINE VMExecutionContext::iopPopC(PC& pc) {
  NEXT();
  m_stack.popC();
}

inline void OPTBLD_INLINE VMExecutionContext::iopPopV(PC& pc) {
  NEXT();
  m_stack.popV();
}

inline void OPTBLD_INLINE VMExecutionContext::iopPopR(PC& pc) {
  NEXT();
  if (m_stack.topTV()->m_type != KindOfRef) {
    m_stack.popC();
  } else {
    m_stack.popV();
  }
}

inline void OPTBLD_INLINE VMExecutionContext::iopDup(PC& pc) {
  NEXT();
  m_stack.dup();
}

inline void OPTBLD_INLINE VMExecutionContext::iopBox(PC& pc) {
  NEXT();
  m_stack.box();
}

inline void OPTBLD_INLINE VMExecutionContext::iopUnbox(PC& pc) {
  NEXT();
  m_stack.unbox();
}

inline void OPTBLD_INLINE VMExecutionContext::iopBoxR(PC& pc) {
  NEXT();
  TypedValue* tv = m_stack.topTV();
  if (tv->m_type != KindOfRef) {
    tvBox(tv);
  }
}

inline void OPTBLD_INLINE VMExecutionContext::iopUnboxR(PC& pc) {
  NEXT();
  if (m_stack.topTV()->m_type == KindOfRef) {
    m_stack.unbox();
  }
}

inline void OPTBLD_INLINE VMExecutionContext::iopNull(PC& pc) {
  NEXT();
  m_stack.pushNull();
}

inline void OPTBLD_INLINE VMExecutionContext::iopTrue(PC& pc) {
  NEXT();
  m_stack.pushTrue();
}

inline void OPTBLD_INLINE VMExecutionContext::iopFalse(PC& pc) {
  NEXT();
  m_stack.pushFalse();
}

inline void OPTBLD_INLINE VMExecutionContext::iopFile(PC& pc) {
  NEXT();
  const StringData* s = m_fp->m_func->unit()->filepath();
  m_stack.pushStaticString(const_cast<StringData*>(s));
}

inline void OPTBLD_INLINE VMExecutionContext::iopDir(PC& pc) {
  NEXT();
  const StringData* s = m_fp->m_func->unit()->dirpath();
  m_stack.pushStaticString(const_cast<StringData*>(s));
}

inline void OPTBLD_INLINE VMExecutionContext::iopInt(PC& pc) {
  NEXT();
  DECODE(int64, i);
  m_stack.pushInt(i);
}

inline void OPTBLD_INLINE VMExecutionContext::iopDouble(PC& pc) {
  NEXT();
  DECODE(double, d);
  m_stack.pushDouble(d);
}

inline void OPTBLD_INLINE VMExecutionContext::iopString(PC& pc) {
  NEXT();
  DECODE_LITSTR(s);
  m_stack.pushStaticString(s);
}

inline void OPTBLD_INLINE VMExecutionContext::iopArray(PC& pc) {
  NEXT();
  DECODE(Id, id);
  ArrayData* a = m_fp->m_func->unit()->lookupArrayId(id);
  m_stack.pushStaticArray(a);
}

inline void OPTBLD_INLINE VMExecutionContext::iopNewArray(PC& pc) {
  NEXT();
  // Clever sizing avoids extra work in HphpArray construction.
  ArrayData* arr = NEW(HphpArray)(size_t(3U) << (HphpArray::MinLgTableSize-2));
  m_stack.pushArray(arr);
}

inline void OPTBLD_INLINE VMExecutionContext::iopNewTuple(PC& pc) {
  NEXT();
  DECODE_IVA(n);
  // This constructor moves values, no inc/decref is necessary.
  HphpArray* arr = NEW(HphpArray)(n, m_stack.topC());
  m_stack.ndiscard(n);
  m_stack.pushArray(arr);
}

inline void OPTBLD_INLINE VMExecutionContext::iopAddElemC(PC& pc) {
  NEXT();
  Cell* c1 = m_stack.topC();
  Cell* c2 = m_stack.indC(1);
  Cell* c3 = m_stack.indC(2);
  if (c3->m_type != KindOfArray) {
    raise_error("AddElemC: $3 must be an array");
  }
  if (c2->m_type == KindOfInt64) {
    tvCellAsVariant(c3).asArrRef().set(c2->m_data.num, tvAsCVarRef(c1));
  } else {
    tvCellAsVariant(c3).asArrRef().set(tvAsCVarRef(c2), tvAsCVarRef(c1));
  }
  m_stack.popC();
  m_stack.popC();
}

inline void OPTBLD_INLINE VMExecutionContext::iopAddElemV(PC& pc) {
  NEXT();
  Var* v1 = m_stack.topV();
  Cell* c2 = m_stack.indC(1);
  Cell* c3 = m_stack.indC(2);
  if (c3->m_type != KindOfArray) {
    raise_error("AddElemC: $3 must be an array");
  }
  if (c2->m_type == KindOfInt64) {
    tvCellAsVariant(c3).asArrRef().set(c2->m_data.num, ref(tvAsCVarRef(v1)));
  } else {
    tvCellAsVariant(c3).asArrRef().set(tvAsCVarRef(c2), ref(tvAsCVarRef(v1)));
  }
  m_stack.popV();
  m_stack.popC();
}

inline void OPTBLD_INLINE VMExecutionContext::iopAddNewElemC(PC& pc) {
  NEXT();
  Cell* c1 = m_stack.topC();
  Cell* c2 = m_stack.indC(1);
  if (c2->m_type != KindOfArray) {
    raise_error("AddNewElemC: $2 must be an array");
  }
  tvCellAsVariant(c2).asArrRef().append(tvAsCVarRef(c1));
  m_stack.popC();
}

inline void OPTBLD_INLINE VMExecutionContext::iopAddNewElemV(PC& pc) {
  NEXT();
  Var* v1 = m_stack.topV();
  Cell* c2 = m_stack.indC(1);
  if (c2->m_type != KindOfArray) {
    raise_error("AddNewElemC: $2 must be an array");
  }
  tvCellAsVariant(c2).asArrRef().append(ref(tvAsCVarRef(v1)));
  m_stack.popV();
}

inline void OPTBLD_INLINE VMExecutionContext::iopCns(PC& pc) {
  NEXT();
  DECODE_LITSTR(s);
  TypedValue* cns = getCns(s);
  if (cns != NULL) {
    Cell* c1 = m_stack.allocC();
    TV_READ_CELL(cns, c1);
  } else {
    raise_notice(Strings::UNDEFINED_CONSTANT,
                 s->data(), s->data());
    m_stack.pushStaticString(s);
  }
}

inline void OPTBLD_INLINE VMExecutionContext::iopDefCns(PC& pc) {
  NEXT();
  DECODE_LITSTR(s);
  TypedValue* tv = m_stack.topTV();

  tvAsVariant(tv) = setCns(s, tvAsCVarRef(tv));
}

inline void OPTBLD_INLINE VMExecutionContext::iopClsCns(PC& pc) {
  NEXT();
  DECODE_LITSTR(clsCnsName);
  TypedValue* tv = m_stack.topTV();
  ASSERT(tv->m_type == KindOfClass);
  Class* class_ = tv->m_data.pcls;
  ASSERT(class_ != NULL);
  TypedValue* clsCns = class_->clsCnsGet(clsCnsName);
  if (clsCns == NULL) {
    raise_error("Couldn't find constant %s::%s",
                class_->name()->data(), clsCnsName->data());
  }
  TV_READ_CELL(clsCns, tv);
}

inline void OPTBLD_INLINE VMExecutionContext::iopClsCnsD(PC& pc) {
  NEXT();
  DECODE_LITSTR(clsCnsName);
  DECODE(Id, classId);
  const NamedEntityPair& classNamedEntity =
    m_fp->m_func->unit()->lookupNamedEntityPairId(classId);

  TypedValue* clsCns = lookupClsCns(classNamedEntity.second,
                                    classNamedEntity.first, clsCnsName);
  ASSERT(clsCns != NULL);
  Cell* c1 = m_stack.allocC();
  TV_READ_CELL(clsCns, c1);
}

inline void OPTBLD_INLINE VMExecutionContext::iopConcat(PC& pc) {
  NEXT();
  Cell* c1 = m_stack.topC();
  Cell* c2 = m_stack.indC(1);
  if (IS_STRING_TYPE(c1->m_type) && IS_STRING_TYPE(c2->m_type)) {
    tvCellAsVariant(c2) = concat(tvCellAsVariant(c2), tvCellAsCVarRef(c1));
  } else {
    tvCellAsVariant(c2) = concat(tvCellAsVariant(c2).toString(),
                                 tvCellAsCVarRef(c1).toString());
  }
  ASSERT(c2->m_data.pstr->getCount() > 0);
  m_stack.popC();
}

#define MATHOP(OP, VOP) do {                                                  \
  NEXT();                                                                     \
  Cell* c1 = m_stack.topC();                                                  \
  Cell* c2 = m_stack.indC(1);                                                 \
  if (c2->m_type == KindOfInt64 && c1->m_type == KindOfInt64) {               \
    int64 a = c2->m_data.num;                                                 \
    int64 b = c1->m_data.num;                                                 \
    MATHOP_DIVCHECK(0)                                                        \
    c2->m_data.num = a OP b;                                                  \
    m_stack.popX();                                                           \
  }                                                                           \
  MATHOP_DOUBLE(OP)                                                           \
  else {                                                                      \
    tvCellAsVariant(c2) = VOP(tvCellAsVariant(c2), tvCellAsCVarRef(c1));      \
    m_stack.popC();                                                           \
  }                                                                           \
} while (0)
#define MATHOP_DOUBLE(OP)                                                     \
  else if (c2->m_type == KindOfDouble                                         \
             && c1->m_type == KindOfDouble) {                                 \
    double a = c2->m_data.dbl;                                                \
    double b = c1->m_data.dbl;                                                \
    MATHOP_DIVCHECK(0.0)                                                      \
    c2->m_data.dbl = a OP b;                                                  \
    m_stack.popX();                                                           \
  }
#define MATHOP_DIVCHECK(x)
inline void OPTBLD_INLINE VMExecutionContext::iopAdd(PC& pc) {
  MATHOP(+, plus);
}

inline void OPTBLD_INLINE VMExecutionContext::iopSub(PC& pc) {
  MATHOP(-, minus);
}

inline void OPTBLD_INLINE VMExecutionContext::iopMul(PC& pc) {
  MATHOP(*, multiply);
}
#undef MATHOP_DIVCHECK

#define MATHOP_DIVCHECK(x)                                                    \
    if (b == x) {                                                             \
      raise_warning("Division by zero");                                      \
      c2->m_data.num = 0;                                                     \
      c2->m_type = KindOfBoolean;                                             \
    } else
inline void OPTBLD_INLINE VMExecutionContext::iopDiv(PC& pc) {
  NEXT();
  Cell* c1 = m_stack.topC();  // denominator
  Cell* c2 = m_stack.indC(1); // numerator
  // Special handling for evenly divisible ints
  if (c2->m_type == KindOfInt64 && c1->m_type == KindOfInt64
      && c1->m_data.num != 0 && c2->m_data.num % c1->m_data.num == 0) {
    int64 b = c1->m_data.num;
    MATHOP_DIVCHECK(0)
    c2->m_data.num /= b;
    m_stack.popX();
  }
  MATHOP_DOUBLE(/)
  else {
    tvCellAsVariant(c2) = divide(tvCellAsVariant(c2), tvCellAsCVarRef(c1));
    m_stack.popC();
  }
}
#undef MATHOP_DOUBLE

#define MATHOP_DOUBLE(OP)
inline void OPTBLD_INLINE VMExecutionContext::iopMod(PC& pc) {
  MATHOP(%, modulo);
}
#undef MATHOP_DOUBLE
#undef MATHOP_DIVCHECK

#define LOGICOP(OP) do {                                                      \
  NEXT();                                                                     \
  Cell* c1 = m_stack.topC();                                                  \
  Cell* c2 = m_stack.indC(1);                                                 \
  {                                                                           \
    tvCellAsVariant(c2) =                                                     \
      (bool)(bool(tvCellAsVariant(c2)) OP bool(tvCellAsVariant(c1)));         \
  }                                                                           \
  m_stack.popC();                                                             \
} while (0)

inline void OPTBLD_INLINE VMExecutionContext::iopXor(PC& pc) {
  LOGICOP(^);
}
#undef LOGICOP

inline void OPTBLD_INLINE VMExecutionContext::iopNot(PC& pc) {
  NEXT();
  Cell* c1 = m_stack.topC();
  tvCellAsVariant(c1) = !bool(tvCellAsVariant(c1));
}

#define CMPOP(OP, VOP) do {                                                   \
  NEXT();                                                                     \
  Cell* c1 = m_stack.topC();                                                  \
  Cell* c2 = m_stack.indC(1);                                                 \
  if (c2->m_type == KindOfInt64 && c1->m_type == KindOfInt64) {               \
    int64 a = c2->m_data.num;                                                 \
    int64 b = c1->m_data.num;                                                 \
    c2->m_data.num = (a OP b);                                                \
    c2->m_type = KindOfBoolean;                                               \
    m_stack.popX();                                                           \
  } else {                                                                    \
    int64 result = VOP(tvCellAsVariant(c2), tvCellAsCVarRef(c1));             \
    tvRefcountedDecRefCell(c2);                                               \
    c2->m_data.num = result;                                                  \
    c2->m_type = KindOfBoolean;                                               \
    m_stack.popC();                                                           \
  }                                                                           \
} while (0)
inline void OPTBLD_INLINE VMExecutionContext::iopSame(PC& pc) {
  CMPOP(==, same);
}

inline void OPTBLD_INLINE VMExecutionContext::iopNSame(PC& pc) {
  CMPOP(!=, !same);
}

inline void OPTBLD_INLINE VMExecutionContext::iopEq(PC& pc) {
  CMPOP(==, equal);
}

inline void OPTBLD_INLINE VMExecutionContext::iopNeq(PC& pc) {
  CMPOP(!=, !equal);
}

inline void OPTBLD_INLINE VMExecutionContext::iopLt(PC& pc) {
  CMPOP(<, less);
}

inline void OPTBLD_INLINE VMExecutionContext::iopLte(PC& pc) {
  CMPOP(<=, not_more);
}

inline void OPTBLD_INLINE VMExecutionContext::iopGt(PC& pc) {
  CMPOP(>, more);
}

inline void OPTBLD_INLINE VMExecutionContext::iopGte(PC& pc) {
  CMPOP(>=, not_less);
}
#undef CMPOP

#define MATHOP_DOUBLE(OP)
#define MATHOP_DIVCHECK(x)
inline void OPTBLD_INLINE VMExecutionContext::iopBitAnd(PC& pc) {
  MATHOP(&, bitwise_and);
}

inline void OPTBLD_INLINE VMExecutionContext::iopBitOr(PC& pc) {
  MATHOP(|, bitwise_or);
}

inline void OPTBLD_INLINE VMExecutionContext::iopBitXor(PC& pc) {
  MATHOP(^, bitwise_xor);
}
#undef MATHOP
#undef MATHOP_DOUBLE
#undef MATHOP_DIVCHECK

inline void OPTBLD_INLINE VMExecutionContext::iopBitNot(PC& pc) {
  NEXT();
  Cell* c1 = m_stack.topC();
  if (LIKELY(c1->m_type == KindOfInt64)) {
    c1->m_data.num = ~c1->m_data.num;
  } else if (c1->m_type == KindOfDouble) {
    c1->m_type = KindOfInt64;
    c1->m_data.num = ~int64(c1->m_data.dbl);
  } else if (IS_STRING_TYPE(c1->m_type)) {
    tvCellAsVariant(c1) = ~tvCellAsVariant(c1);
  } else {
    raise_error("Unsupported operand type for ~");
  }
}

#define SHIFTOP(OP) do {                                                      \
  NEXT();                                                                     \
  Cell* c1 = m_stack.topC();                                                  \
  Cell* c2 = m_stack.indC(1);                                                 \
  if (c2->m_type == KindOfInt64 && c1->m_type == KindOfInt64) {               \
    int64 a = c2->m_data.num;                                                 \
    int64 b = c1->m_data.num;                                                 \
    c2->m_data.num = a OP b;                                                  \
    m_stack.popX();                                                           \
  } else {                                                                    \
    tvCellAsVariant(c2) = tvCellAsVariant(c2).toInt64() OP                    \
                          tvCellAsCVarRef(c1).toInt64();                      \
    m_stack.popC();                                                           \
  }                                                                           \
} while (0)
inline void OPTBLD_INLINE VMExecutionContext::iopShl(PC& pc) {
  SHIFTOP(<<);
}

inline void OPTBLD_INLINE VMExecutionContext::iopShr(PC& pc) {
  SHIFTOP(>>);
}
#undef SHIFTOP

inline void OPTBLD_INLINE VMExecutionContext::iopCastBool(PC& pc) {
  NEXT();
  Cell* c1 = m_stack.topC();
  tvCastToBooleanInPlace(c1);
}

inline void OPTBLD_INLINE VMExecutionContext::iopCastInt(PC& pc) {
  NEXT();
  Cell* c1 = m_stack.topC();
  tvCastToInt64InPlace(c1);
}

inline void OPTBLD_INLINE VMExecutionContext::iopCastDouble(PC& pc) {
  NEXT();
  Cell* c1 = m_stack.topC();
  tvCastToDoubleInPlace(c1);
}

inline void OPTBLD_INLINE VMExecutionContext::iopCastString(PC& pc) {
  NEXT();
  Cell* c1 = m_stack.topC();
  tvCastToStringInPlace(c1);
}

inline void OPTBLD_INLINE VMExecutionContext::iopCastArray(PC& pc) {
  NEXT();
  Cell* c1 = m_stack.topC();
  tvCastToArrayInPlace(c1);
}

inline void OPTBLD_INLINE VMExecutionContext::iopCastObject(PC& pc) {
  NEXT();
  Cell* c1 = m_stack.topC();
  tvCastToObjectInPlace(c1);
}

inline bool OPTBLD_INLINE VMExecutionContext::cellInstanceOf(
  TypedValue* tv, const NamedEntity* ne) {
  ASSERT(tv->m_type != KindOfRef);
  if (tv->m_type == KindOfObject) {
    Class* cls = Unit::lookupClass(ne);
    if (cls) return tv->m_data.pobj->instanceof(cls);
  }
  return false;
}

inline void OPTBLD_INLINE VMExecutionContext::iopInstanceOf(PC& pc) {
  NEXT();
  Cell* c1 = m_stack.topC();   // c2 instanceof c1
  Cell* c2 = m_stack.indC(1);
  bool r = false;
  if (IS_STRING_TYPE(c1->m_type)) {
    const NamedEntity* rhs = Unit::GetNamedEntity(c1->m_data.pstr);
    r = cellInstanceOf(c2, rhs);
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

inline void OPTBLD_INLINE VMExecutionContext::iopInstanceOfD(PC& pc) {
  NEXT();
  DECODE(Id, id);
  const NamedEntity* ne = m_fp->m_func->unit()->lookupNamedEntityId(id);
  Cell* c1 = m_stack.topC();
  bool r = cellInstanceOf(c1, ne);
  tvRefcountedDecRefCell(c1);
  c1->m_data.num = r;
  c1->m_type = KindOfBoolean;
}

inline void OPTBLD_INLINE VMExecutionContext::iopPrint(PC& pc) {
  NEXT();
  Cell* c1 = m_stack.topC();
  print(tvCellAsVariant(c1).toString());
  tvRefcountedDecRefCell(c1);
  c1->m_type = KindOfInt64;
  c1->m_data.num = 1;
}

inline void OPTBLD_INLINE VMExecutionContext::iopClone(PC& pc) {
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

inline int OPTBLD_INLINE
VMExecutionContext::handleUnwind(UnwindStatus unwindType) {
  int longJumpType;
  if (unwindType == UnwindPropagate) {
    longJumpType = EXCEPTION_PROPAGATE;
    if (m_nestedVMs.empty()) {
      m_fp = NULL;
      m_pc = NULL;
    }
  } else {
    ASSERT(unwindType == UnwindResumeVM);
    longJumpType = EXCEPTION_RESUMEVM;
  }
  return longJumpType;
}

inline void OPTBLD_INLINE VMExecutionContext::iopExit(PC& pc) {
  NEXT();
  int exitCode = 0;
  Cell* c1 = m_stack.topC();
  if (c1->m_type == KindOfInt64) {
    exitCode = c1->m_data.num;
  } else {
    print(tvCellAsVariant(c1).toString());
  }
  m_stack.popC();
  throw ExitException(exitCode);
}

inline void OPTBLD_INLINE VMExecutionContext::iopRaise(PC& pc) {
  not_implemented();
}

inline void OPTBLD_INLINE VMExecutionContext::iopFatal(PC& pc) {
  TypedValue* top = m_stack.topTV();
  std::string msg;
  if (IS_STRING_TYPE(top->m_type)) {
    msg = top->m_data.pstr->data();
  } else {
    msg = "Fatal error message not a string";
  }
  m_stack.popTV();
  raise_error(msg);
}

#define JMP_SURPRISE_CHECK()                                               \
  if (offset < 0 && UNLIKELY(Transl::TargetCache::loadConditionFlags())) { \
    SYNC();                                                                \
    EventHook::CheckSurprise();                                            \
  }

inline void OPTBLD_INLINE VMExecutionContext::iopJmp(PC& pc) {
  NEXT();
  DECODE_JMP(Offset, offset);
  JMP_SURPRISE_CHECK();
  pc += offset - 1;
}

#define JMPOP(OP, VOP) do {                                                   \
  Cell* c1 = m_stack.topC();                                                  \
  if (c1->m_type == KindOfInt64 || c1->m_type == KindOfBoolean) {             \
    int64 n = c1->m_data.num;                                                 \
    if (n OP 0) {                                                             \
      NEXT();                                                                 \
      DECODE_JMP(Offset, offset);                                             \
      JMP_SURPRISE_CHECK();                                                   \
      pc += offset - 1;                                                       \
      m_stack.popX();                                                         \
    } else {                                                                  \
      pc += 1 + sizeof(Offset);                                               \
      m_stack.popX();                                                         \
    }                                                                         \
  } else {                                                                    \
    if (VOP(tvCellAsCVarRef(c1))) {                                           \
      NEXT();                                                                 \
      DECODE_JMP(Offset, offset);                                             \
      JMP_SURPRISE_CHECK();                                                   \
      pc += offset - 1;                                                       \
      m_stack.popC();                                                         \
    } else {                                                                  \
      pc += 1 + sizeof(Offset);                                               \
      m_stack.popC();                                                         \
    }                                                                         \
  }                                                                           \
} while (0)
inline void OPTBLD_INLINE VMExecutionContext::iopJmpZ(PC& pc) {
  JMPOP(==, !bool);
}

inline void OPTBLD_INLINE VMExecutionContext::iopJmpNZ(PC& pc) {
  JMPOP(!=, bool);
}
#undef JMPOP
#undef JMP_SURPRISE_CHECK

enum SwitchMatch {
  MATCH_NORMAL,  // value was converted to an int: match normally
  MATCH_NONZERO, // can't be converted to an int: match first nonzero case
  MATCH_DEFAULT, // can't be converted to an int: match default case
};

static SwitchMatch doubleCheck(double d, int64& out) {
  if (int64(d) == d) {
    out = d;
    return MATCH_NORMAL;
  } else {
    return MATCH_DEFAULT;
  }
}

inline void OPTBLD_INLINE VMExecutionContext::iopSwitch(PC& pc) {
  PC origPC = pc;
  NEXT();
  DECODE(int32_t, veclen);
  ASSERT(veclen > 0);
  Offset* jmptab = (Offset*)pc;
  pc += veclen * sizeof(int32_t);
  DECODE(int64, base);
  DECODE_IVA(bounded);

  TypedValue* val = m_stack.topTV();
  if (!bounded) {
    ASSERT(val->m_type == KindOfInt64);
    // Continuation switch: no bounds checking needed
    int64 label = val->m_data.num;
    m_stack.popX();
    ASSERT(label >= 0 && label < veclen);
    pc = origPC + jmptab[label];
  } else {
    // Generic integer switch
    int64 intval;
    SwitchMatch match = MATCH_NORMAL;

    switch (val->m_type) {
      case KindOfUninit:
      case KindOfNull:
        intval = 0;
        break;

      case KindOfBoolean:
        // bool(true) is equal to any non-zero int, bool(false) == 0
        if (val->m_data.num) {
          match = MATCH_NONZERO;
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
        match = MATCH_DEFAULT;
        tvDecRef(val);
        break;

      case KindOfObject:
        intval = val->m_data.pobj->o_toInt64();
        tvDecRef(val);
        break;

      default:
        not_reached();
    }
    m_stack.discard();

    if (match != MATCH_NORMAL ||
        intval < base || intval >= (base + veclen - 2)) {
      switch (match) {
        case MATCH_NORMAL:
        case MATCH_DEFAULT:
          pc = origPC + jmptab[veclen - 1];
          break;

        case MATCH_NONZERO:
          pc = origPC + jmptab[veclen - 2];
          break;
      }
    } else {
      pc = origPC + jmptab[intval - base];
    }
  }
}

inline void OPTBLD_INLINE VMExecutionContext::iopRetC(PC& pc) {
  NEXT();
  uint soff = m_fp->m_soff;
  ASSERT(!m_fp->m_func->isGenerator());

  // Call the runtime helpers to free the local variables and iterators
  frame_free_locals_inl(m_fp, m_fp->m_func->numLocals());
  ActRec* sfp = arGetSfp(m_fp);
  // Memcpy the the return value on top of the activation record. This works
  // the same regardless of whether the return value is boxed or not.
  memcpy(&(m_fp->m_r), m_stack.topTV(), sizeof(TypedValue));
  // Adjust the stack
  m_stack.ndiscard(m_fp->m_func->numSlotsInFrame() + 1);

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
      m_stack.toStringElm(os, m_stack.topTV(), m_fp);
      ONTRACE(1,
              Trace::trace("Return %s from VMExecutionContext::dispatch("
                           "%p)\n", os.str().c_str(), m_fp));
    }
#endif
    m_fp = NULL;
    m_pc = NULL;
    throw HaltVM();
  }
}

inline void OPTBLD_INLINE VMExecutionContext::iopRetV(PC& pc) {
  iopRetC(pc);
}

inline void OPTBLD_INLINE VMExecutionContext::iopUnwind(PC& pc) {
  throw VMPrepareUnwind();
}

inline void OPTBLD_INLINE VMExecutionContext::iopThrow(PC& pc) {
  Cell* c1 = m_stack.topC();
  if (c1->m_type != KindOfObject ||
      !static_cast<Instance*>(c1->m_data.pobj)->
        instanceof(SystemLib::s_ExceptionClass)) {
    raise_error("Exceptions must be valid objects derived from the "
                "Exception base class");
  }
  ObjectData* e = c1->m_data.pobj;
  Fault fault;
  fault.m_faultType = Fault::KindOfUserException;
  fault.m_userException = e;
  m_faults.push_back(fault);
  m_stack.discard();
  DEBUGGER_ATTACHED_ONLY(phpExceptionHook(e));
  throw VMPrepareThrow();
}

inline void OPTBLD_INLINE VMExecutionContext::iopAGetC(PC& pc) {
  NEXT();
  TypedValue* tv = m_stack.topTV();
  lookupClsRef(tv, tv, true);
}

inline void OPTBLD_INLINE VMExecutionContext::iopAGetL(PC& pc) {
  NEXT();
  DECODE_HA(local);
  TypedValue* top = m_stack.allocTV();
  TypedValue* fr = frame_local_inner(m_fp, local);
  lookupClsRef(fr, top);
}

static void raise_undefined_local(ActRec* fp, Id pind) {
  ASSERT(pind < fp->m_func->numNamedLocals());
  raise_notice(Strings::UNDEFINED_VARIABLE,
               fp->m_func->localVarName(pind)->data());
}

static inline void cgetl_inner_body(TypedValue* fr, TypedValue* to) {
  ASSERT(fr->m_type != KindOfUninit);
  tvDup(fr, to);
  if (to->m_type == KindOfRef) {
    tvUnbox(to);
  }
}

static inline void cgetl_body(ActRec* fp,
                              TypedValue* fr,
                              TypedValue* to,
                              Id pind) {
  if (fr->m_type == KindOfUninit) {
    // `to' is uninitialized here, so we need to TV_WRITE_NULL before
    // possibly causing stack unwinding.
    TV_WRITE_NULL(to);
    raise_undefined_local(fp, pind);
  } else {
    cgetl_inner_body(fr, to);
  }
}

inline void OPTBLD_INLINE VMExecutionContext::iopCGetL(PC& pc) {
  NEXT();
  DECODE_HA(local);
  Cell* to = m_stack.allocC();
  TypedValue* fr = frame_local(m_fp, local);
  cgetl_body(m_fp, fr, to, local);
}

inline void OPTBLD_INLINE VMExecutionContext::iopCGetL2(PC& pc) {
  NEXT();
  DECODE_HA(local);
  TypedValue* oldTop = m_stack.topTV();
  TypedValue* newTop = m_stack.allocTV();
  memcpy(newTop, oldTop, sizeof *newTop);
  Cell* to = oldTop;
  TypedValue* fr = frame_local(m_fp, local);
  cgetl_body(m_fp, fr, to, local);
}

inline void OPTBLD_INLINE VMExecutionContext::iopCGetL3(PC& pc) {
  NEXT();
  DECODE_HA(local);
  TypedValue* oldTop = m_stack.topTV();
  TypedValue* oldSubTop = m_stack.indTV(1);
  TypedValue* newTop = m_stack.allocTV();
  memmove(newTop, oldTop, sizeof *oldTop * 2);
  Cell* to = oldSubTop;
  TypedValue* fr = frame_local(m_fp, local);
  cgetl_body(m_fp, fr, to, local);
}

inline void OPTBLD_INLINE VMExecutionContext::iopCGetN(PC& pc) {
  NEXT();
  StringData* name;
  TypedValue* to = m_stack.topTV();
  TypedValue* fr = NULL;
  lookup_var(m_fp, name, to, fr);
  if (fr == NULL || fr->m_type == KindOfUninit) {
    raise_notice(Strings::UNDEFINED_VARIABLE, name->data());
    tvRefcountedDecRefCell(to);
    TV_WRITE_NULL(to);
  } else {
    tvRefcountedDecRefCell(to);
    cgetl_inner_body(fr, to);
  }
  LITSTR_DECREF(name); // TODO(#1146727): leaks during exceptions
}

inline void OPTBLD_INLINE VMExecutionContext::iopCGetG(PC& pc) {
  NEXT();
  StringData* name;
  TypedValue* to = m_stack.topTV();
  TypedValue* fr = NULL;
  lookup_gbl(m_fp, name, to, fr);
  if (fr == NULL) {
    if (MoreWarnings) {
      raise_notice(Strings::UNDEFINED_VARIABLE, name->data());
    }
    tvRefcountedDecRefCell(to);
    TV_WRITE_NULL(to);
  } else if (fr->m_type == KindOfUninit) {
    raise_notice(Strings::UNDEFINED_VARIABLE, name->data());
    tvRefcountedDecRefCell(to);
    TV_WRITE_NULL(to);
  } else {
    tvRefcountedDecRefCell(to);
    cgetl_inner_body(fr, to);
  }
  LITSTR_DECREF(name); // TODO(#1146727): leaks during exceptions
}

#define SPROP_OP_PRELUDE                                  \
  NEXT();                                                 \
  TypedValue* clsref = m_stack.topTV();                   \
  TypedValue* nameCell = m_stack.indTV(1);                \
  TypedValue* output = nameCell;                          \
  StringData* name;                                       \
  TypedValue* val;                                        \
  bool visible, accessible;                               \
  lookup_sprop(m_fp, clsref, name, nameCell, val, visible, \
               accessible);

#define SPROP_OP_POSTLUDE                     \
  LITSTR_DECREF(name);

#define GETS(box) do {                                    \
  SPROP_OP_PRELUDE                                        \
  if (!(visible && accessible)) {                         \
    raise_error("Invalid static property access: %s::%s", \
                clsref->m_data.pcls->name()->data(),      \
                name->data());                            \
  }                                                       \
  if (box) {                                              \
    if (val->m_type != KindOfRef) {                   \
      tvBox(val);                                         \
    }                                                     \
    tvDupVar(val, output);                                \
  } else {                                                \
    tvReadCell(val, output);                              \
  }                                                       \
  m_stack.popC();                                         \
  SPROP_OP_POSTLUDE                                       \
} while (0)

inline void OPTBLD_INLINE VMExecutionContext::iopCGetS(PC& pc) {
  GETS(false);
}

inline void OPTBLD_INLINE VMExecutionContext::iopCGetM(PC& pc) {
  PC oldPC = pc;
  NEXT();
  DECLARE_GETHELPER_ARGS
  getHelper(GETHELPER_ARGS);
  if (tvRet->m_type == KindOfRef) {
    tvUnbox(tvRet);
  }
  if (typeProfileCGetM) {
    ASSERT(hasImmVector(*oldPC));
    const ImmVector& immVec = ImmVector::createFromStream(oldPC + 1);
    StringData* name;
    MemberCode mc;
    if (immVec.decodeLastMember(curUnit(), name, mc)) {
      recordType(TypeProfileKey(mc, name), m_stack.top()->m_type);
    }
  }
}

static inline void vgetl_body(TypedValue* fr, TypedValue* to) {
  if (fr->m_type != KindOfRef) {
    tvBox(fr);
  }
  tvDup(fr, to);
}

inline void OPTBLD_INLINE VMExecutionContext::iopVGetL(PC& pc) {
  NEXT();
  DECODE_HA(local);
  Var* to = m_stack.allocV();
  TypedValue* fr = frame_local(m_fp, local);
  vgetl_body(fr, to);
}

inline void OPTBLD_INLINE VMExecutionContext::iopVGetN(PC& pc) {
  NEXT();
  StringData* name;
  TypedValue* to = m_stack.topTV();
  TypedValue* fr = NULL;
  lookupd_var(m_fp, name, to, fr);
  ASSERT(fr != NULL);
  tvRefcountedDecRefCell(to);
  vgetl_body(fr, to);
  LITSTR_DECREF(name);
}

inline void OPTBLD_INLINE VMExecutionContext::iopVGetG(PC& pc) {
  NEXT();
  StringData* name;
  TypedValue* to = m_stack.topTV();
  TypedValue* fr = NULL;
  lookupd_gbl(m_fp, name, to, fr);
  ASSERT(fr != NULL);
  tvRefcountedDecRefCell(to);
  vgetl_body(fr, to);
  LITSTR_DECREF(name);
}

inline void OPTBLD_INLINE VMExecutionContext::iopVGetS(PC& pc) {
  GETS(true);
}
#undef GETS

inline void OPTBLD_INLINE VMExecutionContext::iopVGetM(PC& pc) {
  NEXT();
  DECLARE_SETHELPER_ARGS
  TypedValue* tv1 = m_stack.allocTV();
  tvWriteUninit(tv1);
  if (!setHelperPre<false, true, false, true, 1,
      ConsumeAll>(SETHELPERPRE_ARGS)) {
    if (base->m_type != KindOfRef) {
      tvBox(base);
    }
    tvDupVar(base, tv1);
  } else {
    tvWriteNull(tv1);
    tvBox(tv1);
  }
  setHelperPost<1>(SETHELPERPOST_ARGS);
}

inline void OPTBLD_INLINE VMExecutionContext::iopIssetN(PC& pc) {
  NEXT();
  StringData* name;
  TypedValue* tv1 = m_stack.topTV();
  TypedValue* tv = NULL;
  bool e;
  lookup_var(m_fp, name, tv1, tv);
  if (tv == NULL) {
    e = false;
  } else {
    e = isset(tvAsCVarRef(tv));
  }
  tvRefcountedDecRefCell(tv1);
  tv1->m_data.num = e;
  tv1->m_type = KindOfBoolean;
  LITSTR_DECREF(name);
}

inline void OPTBLD_INLINE VMExecutionContext::iopIssetG(PC& pc) {
  NEXT();
  StringData* name;
  TypedValue* tv1 = m_stack.topTV();
  TypedValue* tv = NULL;
  bool e;
  lookup_gbl(m_fp, name, tv1, tv);
  if (tv == NULL) {
    e = false;
  } else {
    e = isset(tvAsCVarRef(tv));
  }
  tvRefcountedDecRefCell(tv1);
  tv1->m_data.num = e;
  tv1->m_type = KindOfBoolean;
  LITSTR_DECREF(name);
}

inline void OPTBLD_INLINE VMExecutionContext::iopIssetS(PC& pc) {
  SPROP_OP_PRELUDE
  bool e;
  if (!(visible && accessible)) {
    e = false;
  } else {
    e = isset(tvAsCVarRef(val));
  }
  m_stack.popC();
  output->m_data.num = e;
  output->m_type = KindOfBoolean;
  SPROP_OP_POSTLUDE
}

inline void OPTBLD_INLINE VMExecutionContext::iopIssetM(PC& pc) {
  NEXT();
  DECLARE_GETHELPER_ARGS
  getHelperPre<false, false, LeaveLast>(GETHELPERPRE_ARGS);
  // Process last member specially, in order to employ the IssetElem/IssetProp
  // operations.  (TODO combine with EmptyM.)
  bool issetResult = false;
  switch (mcode) {
  case MEL:
  case MEC:
  case MET:
  case MEI: {
    issetResult = IssetEmptyElem<false>(tvScratch, tvRef, base, baseStrOff,
                                        curMember);
    break;
  }
  case MPL:
  case MPC:
  case MPT: {
    Class* ctx = arGetContextClass(m_fp);
    issetResult = IssetEmptyProp<false>(ctx, base, curMember);
    break;
  }
  default: ASSERT(false);
  }
  getHelperPost<false>(GETHELPERPOST_ARGS);
  tvRet->m_data.num = issetResult;
  tvRet->m_type = KindOfBoolean;
}

#define IOP_TYPE_CHECK_INSTR_L(checkInit, what, predicate)          \
inline void OPTBLD_INLINE VMExecutionContext::iopIs ## what ## L(PC& pc) { \
  NEXT();                                                           \
  DECODE_HA(local);                                                 \
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
inline void OPTBLD_INLINE VMExecutionContext::iopIs ## what ## C(PC& pc) { \
  NEXT();                                                           \
  TypedValue* topTv = m_stack.topTV();                              \
  ASSERT(topTv->m_type != KindOfRef);                           \
  bool ret = predicate(tvAsCVarRef(topTv));                         \
  tvRefcountedDecRefCell(topTv);                                    \
  topTv->m_data.num = ret;                                          \
  topTv->m_type = KindOfBoolean;                                    \
}

#define IOP_TYPE_CHECK_INSTR(checkInit, what, predicate)          \
  IOP_TYPE_CHECK_INSTR_L(checkInit, what, predicate)              \
  IOP_TYPE_CHECK_INSTR_C(checkInit, what, predicate)              \

IOP_TYPE_CHECK_INSTR_L(false,   set, isset)
IOP_TYPE_CHECK_INSTR(true,   Null, f_is_null)
IOP_TYPE_CHECK_INSTR(true,  Array, f_is_array)
IOP_TYPE_CHECK_INSTR(true, String, f_is_string)
IOP_TYPE_CHECK_INSTR(true, Object, f_is_object)
IOP_TYPE_CHECK_INSTR(true,    Int, f_is_int)
IOP_TYPE_CHECK_INSTR(true, Double, f_is_double)
IOP_TYPE_CHECK_INSTR(true,   Bool, f_is_bool)
#undef IOP_TYPE_CHECK_INSTR

inline void OPTBLD_INLINE VMExecutionContext::iopEmptyL(PC& pc) {
  NEXT();
  DECODE_HA(local);
  TypedValue* loc = frame_local(m_fp, local);
  bool e = empty(tvAsCVarRef(loc));
  TypedValue* tv1 = m_stack.allocTV();
  tv1->m_data.num = e;
  tv1->m_type = KindOfBoolean;
}

inline void OPTBLD_INLINE VMExecutionContext::iopEmptyN(PC& pc) {
  NEXT();
  StringData* name;
  TypedValue* tv1 = m_stack.topTV();
  TypedValue* tv = NULL;
  bool e;
  lookup_var(m_fp, name, tv1, tv);
  if (tv == NULL) {
    e = true;
  } else {
    e = empty(tvAsCVarRef(tv));
  }
  tvRefcountedDecRefCell(tv1);
  tv1->m_data.num = e;
  tv1->m_type = KindOfBoolean;
  LITSTR_DECREF(name);
}

inline void OPTBLD_INLINE VMExecutionContext::iopEmptyG(PC& pc) {
  NEXT();
  StringData* name;
  TypedValue* tv1 = m_stack.topTV();
  TypedValue* tv = NULL;
  bool e;
  lookup_gbl(m_fp, name, tv1, tv);
  if (tv == NULL) {
    e = true;
  } else {
    e = empty(tvAsCVarRef(tv));
  }
  tvRefcountedDecRefCell(tv1);
  tv1->m_data.num = e;
  tv1->m_type = KindOfBoolean;
  LITSTR_DECREF(name);
}

inline void OPTBLD_INLINE VMExecutionContext::iopEmptyS(PC& pc) {
  SPROP_OP_PRELUDE
  bool e;
  if (!(visible && accessible)) {
    e = true;
  } else {
    e = empty(tvAsCVarRef(val));
  }
  m_stack.popC();
  output->m_data.num = e;
  output->m_type = KindOfBoolean;
  SPROP_OP_POSTLUDE
}

inline void OPTBLD_INLINE VMExecutionContext::iopEmptyM(PC& pc) {
  NEXT();
  DECLARE_GETHELPER_ARGS
  getHelperPre<false, false, LeaveLast>(GETHELPERPRE_ARGS);
  // Process last member specially, in order to employ the EmptyElem/EmptyProp
  // operations.  (TODO combine with IssetM)
  bool emptyResult = false;
  switch (mcode) {
  case MEL:
  case MEC:
  case MET:
  case MEI: {
    emptyResult = IssetEmptyElem<true>(tvScratch, tvRef, base, baseStrOff,
                                       curMember);
    break;
  }
  case MPL:
  case MPC:
  case MPT: {
    Class* ctx = arGetContextClass(m_fp);
    emptyResult = IssetEmptyProp<true>(ctx, base, curMember);
    break;
  }
  default: ASSERT(false);
  }
  getHelperPost<false>(GETHELPERPOST_ARGS);
  tvRet->m_data.num = emptyResult;
  tvRet->m_type = KindOfBoolean;
}

inline void OPTBLD_INLINE VMExecutionContext::iopAKExists(PC& pc) {
  NEXT();
  TypedValue* arr = m_stack.topTV();
  TypedValue* key = arr + 1;
  bool result = f_array_key_exists(tvAsCVarRef(key), tvAsCVarRef(arr));
  m_stack.popTV();
  tvRefcountedDecRef(key);
  key->m_data.num = result;
  key->m_type = KindOfBoolean;
}

inline void OPTBLD_INLINE VMExecutionContext::iopSetL(PC& pc) {
  NEXT();
  DECODE_HA(local);
  ASSERT(local < m_fp->m_func->numLocals());
  Cell* fr = m_stack.topC();
  TypedValue* to = frame_local(m_fp, local);
  tvSet(fr, to);
}

inline void OPTBLD_INLINE VMExecutionContext::iopSetN(PC& pc) {
  NEXT();
  StringData* name;
  Cell* fr = m_stack.topC();
  TypedValue* tv2 = m_stack.indTV(1);
  TypedValue* to = NULL;
  lookupd_var(m_fp, name, tv2, to);
  ASSERT(to != NULL);
  tvSet(fr, to);
  memcpy((void*)tv2, (void*)fr, sizeof(TypedValue));
  m_stack.discard();
  LITSTR_DECREF(name);
}

inline void OPTBLD_INLINE VMExecutionContext::iopSetG(PC& pc) {
  NEXT();
  StringData* name;
  Cell* fr = m_stack.topC();
  TypedValue* tv2 = m_stack.indTV(1);
  TypedValue* to = NULL;
  lookupd_gbl(m_fp, name, tv2, to);
  ASSERT(to != NULL);
  tvSet(fr, to);
  memcpy((void*)tv2, (void*)fr, sizeof(TypedValue));
  m_stack.discard();
  LITSTR_DECREF(name);
}

inline void OPTBLD_INLINE VMExecutionContext::iopSetS(PC& pc) {
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
  tvSet(tv1, val);
  tvRefcountedDecRefCell(propn);
  memcpy(output, tv1, sizeof(TypedValue));
  m_stack.ndiscard(2);
  LITSTR_DECREF(name);
}

inline void OPTBLD_INLINE VMExecutionContext::iopSetM(PC& pc) {
  NEXT();
  DECLARE_SETHELPER_ARGS
  if (!setHelperPre<false, true, false, false, 1,
      LeaveLast>(SETHELPERPRE_ARGS)) {
    Cell* c1 = m_stack.topC();

    if (mcode == MW) {
      SetNewElem<true>(base, c1);
    } else {
      switch (mcode) {
      case MEL:
      case MEC:
      case MET:
      case MEI:
        SetElem<true>(base, curMember, c1);
        break;
      case MPL:
      case MPC:
      case MPT: {
        Class* ctx = arGetContextClass(m_fp);
        SetProp<true>(ctx, base, curMember, c1);
        break;
      }
      default: ASSERT(false);
      }
    }
  }
  setHelperPost<1>(SETHELPERPOST_ARGS);
}

inline void OPTBLD_INLINE VMExecutionContext::iopSetOpL(PC& pc) {
  NEXT();
  DECODE_HA(local);
  DECODE(unsigned char, op);
  Cell* fr = m_stack.topC();
  TypedValue* to = frame_local(m_fp, local);
  SETOP_BODY(to, op, fr);
  tvRefcountedDecRefCell(fr);
  tvReadCell(to, fr);
}

inline void OPTBLD_INLINE VMExecutionContext::iopSetOpN(PC& pc) {
  NEXT();
  DECODE(unsigned char, op);
  StringData* name;
  Cell* fr = m_stack.topC();
  TypedValue* tv2 = m_stack.indTV(1);
  TypedValue* to = NULL;
  // XXX We're probably not getting warnings totally correct here
  lookupd_var(m_fp, name, tv2, to);
  ASSERT(to != NULL);
  SETOP_BODY(to, op, fr);
  tvRefcountedDecRef(fr);
  tvRefcountedDecRef(tv2);
  tvReadCell(to, tv2);
  m_stack.discard();
  LITSTR_DECREF(name);
}

inline void OPTBLD_INLINE VMExecutionContext::iopSetOpG(PC& pc) {
  NEXT();
  DECODE(unsigned char, op);
  StringData* name;
  Cell* fr = m_stack.topC();
  TypedValue* tv2 = m_stack.indTV(1);
  TypedValue* to = NULL;
  // XXX We're probably not getting warnings totally correct here
  lookupd_gbl(m_fp, name, tv2, to);
  ASSERT(to != NULL);
  SETOP_BODY(to, op, fr);
  tvRefcountedDecRef(fr);
  tvRefcountedDecRef(tv2);
  tvReadCell(to, tv2);
  m_stack.discard();
  LITSTR_DECREF(name);
}

inline void OPTBLD_INLINE VMExecutionContext::iopSetOpS(PC& pc) {
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
  tvReadCell(val, output);
  m_stack.ndiscard(2);
  LITSTR_DECREF(name);
}

inline void OPTBLD_INLINE VMExecutionContext::iopSetOpM(PC& pc) {
  NEXT();
  DECODE(unsigned char, op);
  DECLARE_SETHELPER_ARGS
  if (!setHelperPre<MoreWarnings, true, false, false, 1,
      LeaveLast>(SETHELPERPRE_ARGS)) {
    TypedValue* result;
    Cell* rhs = m_stack.topC();

    if (mcode == MW) {
      result = SetOpNewElem(tvScratch, tvRef, op, base, rhs);
    } else {
      switch (mcode) {
      case MEL:
      case MEC:
      case MET:
      case MEI:
        result = SetOpElem(tvScratch, tvRef, op, base, curMember, rhs);
        break;
      case MPL:
      case MPC:
      case MPT: {
        Class *ctx = arGetContextClass(m_fp);
        result = SetOpProp(tvScratch, tvRef, ctx, op, base, curMember, rhs);
        break;
      }
      default:
        ASSERT(false);
        result = NULL; // Silence compiler warning.
      }
    }

    if (result->m_type == KindOfRef) {
      tvUnbox(result);
    }
    tvRefcountedDecRef(rhs);
    tvDup(result, rhs);
  }
  setHelperPost<1>(SETHELPERPOST_ARGS);
}

inline void OPTBLD_INLINE VMExecutionContext::iopIncDecL(PC& pc) {
  NEXT();
  DECODE_HA(local);
  DECODE(unsigned char, op);
  TypedValue* to = m_stack.allocTV();
  tvWriteUninit(to);
  TypedValue* fr = frame_local(m_fp, local);
  IncDecBody<true>(op, fr, to);
}

inline void OPTBLD_INLINE VMExecutionContext::iopIncDecN(PC& pc) {
  NEXT();
  DECODE(unsigned char, op);
  StringData* name;
  TypedValue* nameCell = m_stack.topTV();
  TypedValue* local = NULL;
  // XXX We're probably not getting warnings totally correct here
  lookupd_var(m_fp, name, nameCell, local);
  ASSERT(local != NULL);
  IncDecBody<true>(op, local, nameCell);
  LITSTR_DECREF(name);
}

inline void OPTBLD_INLINE VMExecutionContext::iopIncDecG(PC& pc) {
  NEXT();
  DECODE(unsigned char, op);
  StringData* name;
  TypedValue* nameCell = m_stack.topTV();
  TypedValue* gbl = NULL;
  // XXX We're probably not getting warnings totally correct here
  lookupd_gbl(m_fp, name, nameCell, gbl);
  ASSERT(gbl != NULL);
  IncDecBody<true>(op, gbl, nameCell);
  LITSTR_DECREF(name);
}

inline void OPTBLD_INLINE VMExecutionContext::iopIncDecS(PC& pc) {
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

inline void OPTBLD_INLINE VMExecutionContext::iopIncDecM(PC& pc) {
  NEXT();
  DECODE(unsigned char, op);
  DECLARE_SETHELPER_ARGS
  TypedValue to;
  tvWriteUninit(&to);
  if (!setHelperPre<MoreWarnings, true, false, false, 0,
      LeaveLast>(SETHELPERPRE_ARGS)) {
    if (mcode == MW) {
      IncDecNewElem<true>(tvScratch, tvRef, op, base, to);
    } else {
      switch (mcode) {
      case MEL:
      case MEC:
      case MET:
      case MEI:
        IncDecElem<true>(tvScratch, tvRef, op, base, curMember, to);
        break;
      case MPL:
      case MPC:
      case MPT: {
        Class* ctx = arGetContextClass(m_fp);
        IncDecProp<true>(tvScratch, tvRef, ctx, op, base, curMember, to);
        break;
      }
      default: ASSERT(false);
      }
    }
  }
  setHelperPost<0>(SETHELPERPOST_ARGS);
  Cell* c1 = m_stack.allocC();
  memcpy(c1, &to, sizeof(TypedValue));
}

inline void OPTBLD_INLINE VMExecutionContext::iopBindL(PC& pc) {
  NEXT();
  DECODE_HA(local);
  Var* fr = m_stack.topV();
  TypedValue* to = frame_local(m_fp, local);
  tvBind(fr, to);
}

inline void OPTBLD_INLINE VMExecutionContext::iopBindN(PC& pc) {
  NEXT();
  StringData* name;
  TypedValue* fr = m_stack.topTV();
  TypedValue* nameTV = m_stack.indTV(1);
  TypedValue* to = NULL;
  lookupd_var(m_fp, name, nameTV, to);
  ASSERT(to != NULL);
  tvBind(fr, to);
  memcpy((void*)nameTV, (void*)fr, sizeof(TypedValue));
  m_stack.discard();
  LITSTR_DECREF(name);
}

inline void OPTBLD_INLINE VMExecutionContext::iopBindG(PC& pc) {
  NEXT();
  StringData* name;
  TypedValue* fr = m_stack.topTV();
  TypedValue* nameTV = m_stack.indTV(1);
  TypedValue* to = NULL;
  lookupd_gbl(m_fp, name, nameTV, to);
  ASSERT(to != NULL);
  tvBind(fr, to);
  memcpy((void*)nameTV, (void*)fr, sizeof(TypedValue));
  m_stack.discard();
  LITSTR_DECREF(name);
}

inline void OPTBLD_INLINE VMExecutionContext::iopBindS(PC& pc) {
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
  LITSTR_DECREF(name);
}

inline void OPTBLD_INLINE VMExecutionContext::iopBindM(PC& pc) {
  NEXT();
  DECLARE_SETHELPER_ARGS
  TypedValue* tv1 = m_stack.topTV();
  if (!setHelperPre<false, true, false, true, 1,
      ConsumeAll>(SETHELPERPRE_ARGS)) {
    // Bind the element/property with the var on the top of the stack
    tvBind(tv1, base);
  }
  setHelperPost<1>(SETHELPERPOST_ARGS);
}

inline void OPTBLD_INLINE VMExecutionContext::iopUnsetL(PC& pc) {
  NEXT();
  DECODE_HA(local);
  ASSERT(local < m_fp->m_func->numLocals());
  TypedValue* tv = frame_local(m_fp, local);
  tvRefcountedDecRef(tv);
  TV_WRITE_UNINIT(tv);
}

inline void OPTBLD_INLINE VMExecutionContext::iopUnsetN(PC& pc) {
  NEXT();
  StringData* name;
  TypedValue* tv1 = m_stack.topTV();
  TypedValue* tv = NULL;
  lookup_var(m_fp, name, tv1, tv);
  ASSERT(!m_fp->hasInvName());
  if (tv != NULL) {
    tvRefcountedDecRef(tv);
    TV_WRITE_UNINIT(tv);
  }
  m_stack.popC();
  LITSTR_DECREF(name);
}

inline void OPTBLD_INLINE VMExecutionContext::iopUnsetG(PC& pc) {
  NEXT();
  TypedValue* tv1 = m_stack.topTV();
  StringData* name = lookup_name(tv1);
  VarEnv* varEnv = m_globalVarEnv;
  ASSERT(varEnv != NULL);
  varEnv->unset(name);
  m_stack.popC();
  LITSTR_DECREF(name);
}

inline void OPTBLD_INLINE VMExecutionContext::iopUnsetM(PC& pc) {
  NEXT();
  DECLARE_SETHELPER_ARGS
  if (!setHelperPre<false, false, true, false, 0,
      LeaveLast>(SETHELPERPRE_ARGS)) {
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
    default: ASSERT(false);
    }
  }
  setHelperPost<0>(SETHELPERPOST_ARGS);
}

inline void OPTBLD_INLINE VMExecutionContext::iopFPushFunc(PC& pc) {
  NEXT();
  DECODE_IVA(numArgs);
  Cell* c1 = m_stack.topC();
  const Func* func = NULL;
  ObjectData* origObj = NULL;
  StringData* origSd = NULL;
  if (IS_STRING_TYPE(c1->m_type)) {
    origSd = c1->m_data.pstr;
    func = Unit::lookupFunc(origSd);
  } else if (c1->m_type == KindOfObject) {
    static StringData* invokeName = StringData::GetStaticString("__invoke");
    origObj = c1->m_data.pobj;
    const Class* cls = origObj->getVMClass();
    func = cls->lookupMethod(invokeName);
    if (func == NULL) {
      raise_error(Strings::FUNCTION_NAME_MUST_BE_STRING);
    }
  } else {
    raise_error(Strings::FUNCTION_NAME_MUST_BE_STRING);
  }
  if (func == NULL) {
    raise_error("Undefined function: %s", c1->m_data.pstr->data());
  }
  ASSERT(!origObj || !origSd);
  ASSERT(origObj || origSd);
  // We've already saved origObj or origSd; we'll use them after
  // overwriting the pointer on the stack.  Don't refcount it now; defer
  // till after we're done with it.
  m_stack.discard();
  ActRec* ar = m_stack.allocA();
  ar->m_func = func;
  arSetSfp(ar, m_fp);
  if (origObj) {
    if (func->attrs() & AttrStatic) {
      ar->setClass(origObj->getVMClass());
      if (origObj->decRefCount() == 0) {
        origObj->release();
      }
    } else {
      ar->setThis(origObj);
      // Teleport the reference from the destroyed stack cell to the
      // ActRec. Don't try this at home.
    }
  } else {
    ar->setThis(NULL);
    if (origSd->decRefCount() == 0) {
      origSd->release();
    }
  }
  ar->initNumArgs(numArgs);
  ar->setVarEnv(NULL);
}

inline void OPTBLD_INLINE VMExecutionContext::iopFPushFuncD(PC& pc) {
  NEXT();
  DECODE_IVA(numArgs);
  DECODE(Id, id);
  const NamedEntityPair nep = m_fp->m_func->unit()->lookupNamedEntityPairId(id);
  Func* func = Unit::lookupFunc(nep.second, nep.first);
  if (func == NULL) {
    raise_error("Undefined function: %s",
                m_fp->m_func->unit()->lookupLitstrId(id)->data());
  }
  DEBUGGER_IF(phpBreakpointEnabled(func->name()->data()));
  ActRec* ar = m_stack.allocA();
  arSetSfp(ar, m_fp);
  ar->m_func = func;
  ar->setThis(NULL);
  ar->initNumArgs(numArgs);
  ar->setVarEnv(NULL);
}

#define OBJMETHOD_BODY(cls, name, obj) do { \
  const Func* f; \
  LookupResult res = lookupObjMethod(f, cls, name, true); \
  ASSERT(f); \
  ActRec* ar = m_stack.allocA(); \
  arSetSfp(ar, m_fp); \
  ar->m_func = f; \
  if (res == MethodFoundNoThis) { \
    if (obj->decRefCount() == 0) obj->release(); \
    ar->setClass(cls); \
  } else { \
    ASSERT(res == MethodFoundWithThis || res == MagicCallFound); \
    /* Transfer ownership of obj to the ActRec*/ \
    ar->setThis(obj); \
  } \
  ar->initNumArgs(numArgs); \
  if (res == MagicCallFound) { \
    ar->setInvName(name); \
  } else { \
    ar->setVarEnv(NULL); \
    LITSTR_DECREF(name); \
  } \
} while (0)

inline void OPTBLD_INLINE VMExecutionContext::iopFPushObjMethod(PC& pc) {
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
  // We handle decReffing obj and name below
  m_stack.ndiscard(2);
  OBJMETHOD_BODY(cls, name, obj);
}

inline void OPTBLD_INLINE VMExecutionContext::iopFPushObjMethodD(PC& pc) {
  NEXT();
  DECODE_IVA(numArgs);
  DECODE_LITSTR(name);
  Cell* c1 = m_stack.topC();
  if (c1->m_type != KindOfObject) {
    throw_call_non_object(name->data());
  }
  ObjectData* obj = c1->m_data.pobj;
  Class* cls = obj->getVMClass();
  // We handle decReffing obj below
  m_stack.discard();
  OBJMETHOD_BODY(cls, name, obj);
}

#define CLSMETHOD_BODY(cls, name, obj, forwarding) do { \
  const Func* f; \
  LookupResult res = lookupClsMethod(f, cls, name, obj, true); \
  if (f->isAbstract()) { \
    raise_error("Cannot call abstract method %s()", \
                f->fullName()->data()); \
  } \
  if (res == MethodFoundNoThis || res == MagicCallStaticFound) { \
    obj = NULL; \
  } else { \
    ASSERT(obj); \
    ASSERT(res == MethodFoundWithThis || res == MagicCallFound); \
    obj->incRefCount(); \
  } \
  ASSERT(f); \
  ActRec* ar = m_stack.allocA(); \
  arSetSfp(ar, m_fp); \
  ar->m_func = f; \
  if (obj) { \
    ar->setThis(obj); \
  } else { \
    if (!forwarding) { \
      ar->setClass(cls); \
    } else { \
      /* Propogate the current late bound class if there is one, */ \
      /* otherwise use the class given by this instruction's input */ \
      if (m_fp->hasThis()) { \
        cls = m_fp->getThis()->getVMClass(); \
      } else if (m_fp->hasClass()) { \
        cls = m_fp->getClass(); \
      } \
      ar->setClass(cls); \
    } \
  } \
  ar->initNumArgs(numArgs); \
  if (res == MagicCallFound || res == MagicCallStaticFound) { \
    ar->setInvName(name); \
  } else { \
    ar->setVarEnv(NULL); \
    LITSTR_DECREF(const_cast<StringData*>(name)); \
  } \
} while (0)

inline void OPTBLD_INLINE VMExecutionContext::iopFPushClsMethod(PC& pc) {
  NEXT();
  DECODE_IVA(numArgs);
  Cell* c1 = m_stack.indC(1); // Method name.
  if (!IS_STRING_TYPE(c1->m_type)) {
    raise_error(Strings::FUNCTION_NAME_MUST_BE_STRING);
  }
  TypedValue* tv = m_stack.top();
  ASSERT(tv->m_type == KindOfClass);
  Class* cls = tv->m_data.pcls;
  StringData* name = c1->m_data.pstr;
  // CLSMETHOD_BODY will take care of decReffing name
  m_stack.ndiscard(2);
  ASSERT(cls && name);
  ObjectData* obj = m_fp->hasThis() ? m_fp->getThis() : NULL;
  CLSMETHOD_BODY(cls, name, obj, false);
}

inline void OPTBLD_INLINE VMExecutionContext::iopFPushClsMethodD(PC& pc) {
  NEXT();
  DECODE_IVA(numArgs);
  DECODE_LITSTR(name);
  DECODE(Id, classId);
  const NamedEntityPair &nep =
    m_fp->m_func->unit()->lookupNamedEntityPairId(classId);
  Class* cls = Unit::loadClass(nep.second, nep.first);
  if (cls == NULL) {
    raise_error(Strings::UNKNOWN_CLASS, nep.first->data());
  }
  ObjectData* obj = m_fp->hasThis() ? m_fp->getThis() : NULL;
  CLSMETHOD_BODY(cls, name, obj, false);
}

inline void OPTBLD_INLINE VMExecutionContext::iopFPushClsMethodF(PC& pc) {
  NEXT();
  DECODE_IVA(numArgs);
  Cell* c1 = m_stack.indC(1); // Method name.
  if (!IS_STRING_TYPE(c1->m_type)) {
    raise_error(Strings::FUNCTION_NAME_MUST_BE_STRING);
  }
  TypedValue* tv = m_stack.top();
  ASSERT(tv->m_type == KindOfClass);
  Class* cls = tv->m_data.pcls;
  ASSERT(cls);
  StringData* name = c1->m_data.pstr;
  // CLSMETHOD_BODY will take care of decReffing name
  m_stack.ndiscard(2);
  ObjectData* obj = m_fp->hasThis() ? m_fp->getThis() : NULL;
  CLSMETHOD_BODY(cls, name, obj, true);
}

#undef CLSMETHOD_BODY

inline void OPTBLD_INLINE VMExecutionContext::iopFPushCtor(PC& pc) {
  NEXT();
  DECODE_IVA(numArgs);
  TypedValue* tv = m_stack.topTV();
  ASSERT(tv->m_type == KindOfClass);
  Class* cls = tv->m_data.pcls;
  ASSERT(cls != NULL);
  // Lookup the ctor
  const Func* f;
  LookupResult res UNUSED = lookupCtorMethod(f, cls, true);
  ASSERT(res == MethodFoundWithThis);
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
  ar->setVarEnv(NULL);
}

inline void OPTBLD_INLINE VMExecutionContext::iopFPushCtorD(PC& pc) {
  NEXT();
  DECODE_IVA(numArgs);
  DECODE(Id, id);
  const NamedEntityPair &nep =
    m_fp->m_func->unit()->lookupNamedEntityPairId(id);
  Class* cls = Unit::loadClass(nep.second, nep.first);
  if (cls == NULL) {
    raise_error("Undefined class: %s",
                m_fp->m_func->unit()->lookupLitstrId(id)->data());
  }
  // Lookup the ctor
  const Func* f;
  LookupResult res UNUSED = lookupCtorMethod(f, cls, true);
  ASSERT(res == MethodFoundWithThis);
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
  ar->setVarEnv(NULL);
}

inline void OPTBLD_INLINE VMExecutionContext::doFPushCuf(PC& pc,
                                                         bool forward,
                                                         bool safe) {
  NEXT();
  DECODE_IVA(numArgs);

  TypedValue func = m_stack.topTV()[safe];

  ObjectData* obj = NULL;
  HPHP::VM::Class* cls = NULL;
  StringData* invName = NULL;

  const HPHP::VM::Func* f = vm_decode_function(tvAsVariant(&func), getFP(),
                                               forward,
                                               obj, cls, invName,
                                               !safe);

  if (safe) m_stack.topTV()[1] = m_stack.topTV()[0];
  m_stack.ndiscard(1);
  if (f == NULL) {
    f = SystemLib::GetNullFunction();
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
    ar->setThis(NULL);
  }
  ar->initNumArgs(numArgs, false /* isFPushCtor */);
  if (invName) {
    ar->setInvName(invName);
  } else {
    ar->setVarEnv(NULL);
  }
  tvRefcountedDecRef(&func);
}

inline void OPTBLD_INLINE VMExecutionContext::iopFPushCuf(PC& pc) {
  doFPushCuf(pc, false, false);
}

inline void OPTBLD_INLINE VMExecutionContext::iopFPushCufF(PC& pc) {
  doFPushCuf(pc, true, false);
}

inline void OPTBLD_INLINE VMExecutionContext::iopFPushCufSafe(PC& pc) {
  doFPushCuf(pc, false, true);
}

static inline ActRec* arFromInstr(TypedValue* sp, const Opcode* pc) {
  return arFromSpOffset((ActRec*)sp, instrSpToArDelta(pc));
}

inline void OPTBLD_INLINE VMExecutionContext::iopFPassC(PC& pc) {
#ifdef DEBUG
  ActRec* ar = arFromInstr(m_stack.top(), (Opcode*)pc);
#endif
  NEXT();
  DECODE_IVA(paramId);
#ifdef DEBUG
  ASSERT(paramId < ar->numArgs());
#endif
}

#define FPASSC_CHECKED_PRELUDE                                                \
  ActRec* ar = arFromInstr(m_stack.top(), (Opcode*)pc);                       \
  NEXT();                                                                     \
  DECODE_IVA(paramId);                                                        \
  ASSERT(paramId < ar->numArgs());                                            \
  const Func* func = ar->m_func;

inline void OPTBLD_INLINE VMExecutionContext::iopFPassCW(PC& pc) {
  FPASSC_CHECKED_PRELUDE
  if (func->mustBeRef(paramId)) {
    TRACE(1, "FPassCW: function %s(%d) param %d is by reference, "
          "raising a strict warning (attr:0x%x)\n",
          func->name()->data(), func->numParams(), paramId,
          func->isBuiltin() ? func->info()->attribute : 0);
    raise_strict_warning("Only variables should be passed by reference");
  }
}

inline void OPTBLD_INLINE VMExecutionContext::iopFPassCE(PC& pc) {
  FPASSC_CHECKED_PRELUDE
  if (func->mustBeRef(paramId)) {
    TRACE(1, "FPassCE: function %s(%d) param %d is by reference, "
          "throwing a fatal error (attr:0x%x)\n",
          func->name()->data(), func->numParams(), paramId,
          func->isBuiltin() ? func->info()->attribute : 0);
    raise_error("Cannot pass parameter %d by reference", paramId+1);
  }
}

#undef FPASSC_CHECKED_PRELUDE

inline void OPTBLD_INLINE VMExecutionContext::iopFPassV(PC& pc) {
  ActRec* ar = arFromInstr(m_stack.top(), (Opcode*)pc);
  NEXT();
  DECODE_IVA(paramId);
  ASSERT(paramId < ar->numArgs());
  const Func* func = ar->m_func;
  if (!func->byRef(paramId)) {
    m_stack.unbox();
  }
}

inline void OPTBLD_INLINE VMExecutionContext::iopFPassR(PC& pc) {
  ActRec* ar = arFromInstr(m_stack.top(), (Opcode*)pc);
  NEXT();
  DECODE_IVA(paramId);
  ASSERT(paramId < ar->numArgs());
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

inline void OPTBLD_INLINE VMExecutionContext::iopFPassL(PC& pc) {
  ActRec* ar = arFromInstr(m_stack.top(), (Opcode*)pc);
  NEXT();
  DECODE_IVA(paramId);
  DECODE_HA(local);
  ASSERT(paramId < ar->numArgs());
  TypedValue* fr = frame_local(m_fp, local);
  TypedValue* to = m_stack.allocTV();
  if (!ar->m_func->byRef(paramId)) {
    cgetl_body(m_fp, fr, to, local);
  } else {
    vgetl_body(fr, to);
  }
}

inline void OPTBLD_INLINE VMExecutionContext::iopFPassN(PC& pc) {
  ActRec* ar = arFromInstr(m_stack.top(), (Opcode*)pc);
  PC origPc = pc;
  NEXT();
  DECODE_IVA(paramId);
  ASSERT(paramId < ar->numArgs());
  if (!ar->m_func->byRef(paramId)) {
    iopCGetN(origPc);
  } else {
    iopVGetN(origPc);
  }
}

inline void OPTBLD_INLINE VMExecutionContext::iopFPassG(PC& pc) {
  ActRec* ar = arFromInstr(m_stack.top(), (Opcode*)pc);
  PC origPc = pc;
  NEXT();
  DECODE_IVA(paramId);
  ASSERT(paramId < ar->numArgs());
  if (!ar->m_func->byRef(paramId)) {
    iopCGetG(origPc);
  } else {
    iopVGetG(origPc);
  }
}

inline void OPTBLD_INLINE VMExecutionContext::iopFPassS(PC& pc) {
  ActRec* ar = arFromInstr(m_stack.top(), (Opcode*)pc);
  PC origPc = pc;
  NEXT();
  DECODE_IVA(paramId);
  ASSERT(paramId < ar->numArgs());
  if (!ar->m_func->byRef(paramId)) {
    iopCGetS(origPc);
  } else {
    iopVGetS(origPc);
  }
}

void VMExecutionContext::iopFPassM(PC& pc) {
  ActRec* ar = arFromInstr(m_stack.top(), (Opcode*)pc);
  NEXT();
  DECODE_IVA(paramId);
  ASSERT(paramId < ar->numArgs());
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
        ConsumeAll>(SETHELPERPRE_ARGS)) {
      if (base->m_type != KindOfRef) {
        tvBox(base);
      }
      tvDupVar(base, tv1);
    } else {
      tvWriteNull(tv1);
      tvBox(tv1);
    }
    setHelperPost<1>(SETHELPERPOST_ARGS);
  }
}

template <bool handle_throw>
void VMExecutionContext::doFCall(ActRec* ar, PC& pc) {
  ASSERT(ar->m_savedRbp == (uint64_t)m_fp);
  ar->m_savedRip = (uintptr_t)tx64->getRetFromInterpretedFrame();
  TRACE(3, "FCall: pc %p func %p base %d\n", m_pc,
        m_fp->m_func->unit()->entry(),
        int(m_fp->m_func->base()));
  ar->m_soff = m_fp->m_func->unit()->offsetOf(pc)
    - (uintptr_t)m_fp->m_func->base();
  ASSERT(pcOff() > m_fp->m_func->base());
  prepareFuncEntry<false, handle_throw>(ar, pc, 0);
  SYNC();
  EventHook::FunctionEnter(ar, EventHook::NormalFunc);
  INST_HOOK_FENTRY(ar->m_func->fullName());
}

template void VMExecutionContext::doFCall<true>(ActRec *ar, PC& pc);

inline void OPTBLD_INLINE VMExecutionContext::iopFCall(PC& pc) {
  ActRec* ar = arFromInstr(m_stack.top(), (Opcode*)pc);
  NEXT();
  DECODE_IVA(numArgs);
  ASSERT(numArgs == ar->numArgs());
  checkStack(m_stack, ar->m_func);
  doFCall<false>(ar, pc);
}

bool VMExecutionContext::prepareArrayArgs(ActRec* ar,
                                          ArrayData* args,
                                          ExtraArgs*& extraArgs) {
  extraArgs = NULL;
  if (UNLIKELY(ar->hasInvName())) {
    m_stack.pushStringNoRc(ar->getInvName());
    m_stack.pushArray(args);
    ar->setVarEnv(0);
    ar->initNumArgs(2);
  } else {
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
      if (UNLIKELY(f->byRef(i))) {
        if (UNLIKELY(!tvAsVariant(from).isReferenced())) {
          // TODO: #1746957
          // we should raise a warning and bail out here. But there are
          // lots of tests dependent on actually making the call.
          // Hopefully the warnings will get the code base cleaned up
          // and we'll be able to fix this painlessly
          const bool skipCallOnInvalidParams = false;
          int param = i + 1;
          raise_warning("Parameter %d to %s() expected to be a reference, "
                        "value given", param, f->name()->data());
          if (skipCallOnInvalidParams) {
            while (i--) m_stack.popTV();
            m_stack.popAR();
            m_stack.pushNull();
            return false;
          }
        }
        tvDup(from, m_stack.allocTV());
      } else {
        TypedValue* to = m_stack.allocTV();
        tvDup(from, to);
        if (UNLIKELY(to->m_type == KindOfRef)) {
          tvUnbox(to);
        }
      }
      pos = args->iter_advance(pos);
    }
    if (extra && (ar->m_func->attrs() & AttrMayUseVV)) {
      extraArgs = ExtraArgs::allocateUninit(extra);
      for (int i = 0; i < extra; ++i) {
        TypedValue* to = extraArgs->getExtraArg(i);
        tvDup(args->getValueRef(pos).asTypedValue(), to);
        if (to->m_type == KindOfRef && to->m_data.pref->_count == 2) {
          tvUnbox(to);
        }
        pos = args->iter_advance(pos);
      }
      ar->initNumArgs(nargs);
    } else {
      ar->initNumArgs(nparams);
    }
  }
  return true;
}

static void cleanupParamsAndActRec(VM::Stack& stack,
                                   ActRec* ar,
                                   ExtraArgs* extraArgs) {
  for (int i = ar->numArgs(); i--; ) {
    stack.popTV();
  }
  ASSERT(stack.top() == (void*)ar);
  stack.popAR();
  if (extraArgs) {
    const int numExtra = ar->numArgs() - ar->m_func->numParams();
    ExtraArgs::deallocate(extraArgs, numExtra);
  }
}

bool VMExecutionContext::doFCallArray(PC& pc) {
  ActRec* ar = (ActRec*)(m_stack.top() + 1);
  ASSERT(ar->numArgs() == 1);

  Cell* c1 = m_stack.topC();
  if (false && UNLIKELY(c1->m_type != KindOfArray)) {
    // task #1756122
    // this is what we /should/ do, but our code base depends
    // on the broken behavior of casting the second arg to an
    // array.
    cleanupParamsAndActRec(m_stack, ar, NULL);
    m_stack.pushNull();
    raise_warning("call_user_func_array() expects parameter 2 to be array");
    return false;
  }

  const Func* func = ar->m_func;
  ExtraArgs* extraArgs = NULL;
  {
    Array args(LIKELY(c1->m_type == KindOfArray) ? c1->m_data.parr :
               tvAsVariant(c1).toArray().get());
    m_stack.popTV();
    checkStack(m_stack, func);

    ASSERT(ar->m_savedRbp == (uint64_t)m_fp);
    ASSERT(!ar->m_func->isGenerator());
    ar->m_savedRip = (uintptr_t)tx64->getRetFromInterpretedFrame();
    TRACE(3, "FCallArray: pc %p func %p base %d\n", m_pc,
          m_fp->m_func->unit()->entry(),
          int(m_fp->m_func->base()));
    ar->m_soff = m_fp->m_func->unit()->offsetOf(pc)
      - (uintptr_t)m_fp->m_func->base();
    ASSERT(pcOff() > m_fp->m_func->base());

    StringData* invName = ar->hasInvName() ? ar->getInvName() : NULL;
    if (UNLIKELY(!prepareArrayArgs(ar, args.get(), extraArgs))) return false;
    if (UNLIKELY(func->maybeIntercepted())) {
      Variant *h = get_intercept_handler(func->fullNameRef(),
                                         &func->maybeIntercepted());
      if (h) {
        try {
          TypedValue retval;
          if (!run_intercept_handler_for_invokefunc(
                &retval, func, args,
                ar->hasThis() ? ar->getThis() : NULL,
                invName, h)) {
            cleanupParamsAndActRec(m_stack, ar, extraArgs);
            *m_stack.allocTV() = retval;
            return false;
          }
        } catch (...) {
          cleanupParamsAndActRec(m_stack, ar, extraArgs);
          m_stack.pushNull();
          SYNC();
          throw;
        }
      }
    }
  }

  prepareFuncEntry<true, false>(ar, pc, extraArgs);
  SYNC();
  EventHook::FunctionEnter(ar, EventHook::NormalFunc);
  INST_HOOK_FENTRY(func->fullName());
  return true;
}

inline void OPTBLD_INLINE VMExecutionContext::iopFCallArray(PC& pc) {
  NEXT();
  (void)doFCallArray(pc);
}

inline void OPTBLD_INLINE VMExecutionContext::iopCufSafeArray(PC& pc) {
  NEXT();
  Array ret;
  ret.append(tvAsVariant(m_stack.top() + 1));
  ret.appendWithRef(tvAsVariant(m_stack.top() + 0));
  m_stack.popTV();
  m_stack.popTV();
  tvAsVariant(m_stack.top()) = ret;
}

inline void OPTBLD_INLINE VMExecutionContext::iopCufSafeReturn(PC& pc) {
  NEXT();
  bool ok = tvAsVariant(m_stack.top() + 1).toBoolean();
  tvRefcountedDecRef(m_stack.top() + 1);
  tvRefcountedDecRef(m_stack.top() + (ok ? 2 : 0));
  if (ok) m_stack.top()[2] = m_stack.top()[0];
  m_stack.ndiscard(2);
}

inline void OPTBLD_INLINE VMExecutionContext::iopIterInit(PC& pc) {
  PC origPc = pc;
  NEXT();
  DECODE_IA(itId);
  DECODE(Offset, offset);
  Cell* c1 = m_stack.topC();
  if (c1->m_type == KindOfArray) {
    if (!c1->m_data.parr->empty()) {
      Iter* it = frame_iter(m_fp, itId);
      (void) new (&it->arr()) ArrayIter(c1->m_data.parr); // call CTor
      it->m_itype = Iter::TypeArray;
    } else {
      ITER_SKIP(offset);
    }
  } else if (c1->m_type == KindOfObject) {
    Iter* it = frame_iter(m_fp, itId);
    bool isIterator;
    if (c1->m_data.pobj->isCollection()) {
      isIterator = true;
      (void) new (&it->arr()) ArrayIter(c1->m_data.pobj);
    } else {
      Object obj = c1->m_data.pobj->iterableObject(isIterator);
      if (isIterator) {
        (void) new (&it->arr()) ArrayIter(obj.get());
      } else {
        Class* ctx = arGetContextClass(m_fp);
        CStrRef ctxStr = ctx ? ctx->nameRef() : null_string;
        Array iterArray(obj->o_toIterArray(ctxStr));
        ArrayData* ad = iterArray.getArrayData();
        (void) new (&it->arr()) ArrayIter(ad);
      }
    }
    if (it->arr().end()) {
      // Iterator was empty; call the destructor on the iterator we
      // just constructed and branch to done case
      it->arr().~ArrayIter();
      ITER_SKIP(offset);
    } else {
      it->m_itype = (isIterator ? Iter::TypeIterator : Iter::TypeArray);
    }
  } else {
    raise_warning("Invalid argument supplied for foreach()");
    ITER_SKIP(offset);
  }
  m_stack.popC();
}

inline void OPTBLD_INLINE VMExecutionContext::iopIterInitM(PC& pc) {
  PC origPc = pc;
  NEXT();
  DECODE_IA(itId);
  DECODE(Offset, offset);
  Var* v1 = m_stack.topV();
  ASSERT(v1->m_type == KindOfRef);
  TypedValue* rtv = v1->m_data.pref->tv();
  if (rtv->m_type == KindOfArray) {
    ArrayData* ad = rtv->m_data.parr;
    if (!ad->empty()) {
      Iter* it = frame_iter(m_fp, itId);
      MIterCtx& mi = it->marr();
      (void) new (&mi) MIterCtx(v1->m_data.pref);
      it->m_itype = Iter::TypeMutableArray;
      mi.mArray().advance();
    } else {
      ITER_SKIP(offset);
    }
  } else if (rtv->m_type == KindOfObject)  {
    if (rtv->m_data.pobj->getCollectionType() != 0) {
      raise_error("Collection elements cannot be taken by reference");
    }
    bool isIterator;
    Object obj = rtv->m_data.pobj->iterableObject(isIterator);
    if (isIterator) {
      raise_error("An iterator cannot be used with foreach by reference");
    }
    Class* ctx = arGetContextClass(m_fp);
    CStrRef ctxStr = ctx ? ctx->nameRef() : null_string;
    Array iterArray = obj->o_toIterArray(ctxStr, true);
    if (iterArray->empty()) {
      ITER_SKIP(offset);
    } else {
      ArrayData* ad = iterArray.detach();
      Iter* it = frame_iter(m_fp, itId);
      MIterCtx& mi = it->marr();
      (void) new (&mi) MIterCtx(ad);
      mi.mArray().advance();
      it->m_itype = Iter::TypeMutableArray;
    }
  } else {
    if (!hphpiCompat) {
      raise_warning("Invalid argument supplied for foreach()");
    }
    ITER_SKIP(offset);
  }
  m_stack.popV();
}

inline void OPTBLD_INLINE VMExecutionContext::iopIterValueC(PC& pc) {
  NEXT();
  DECODE_IA(itId);
  Iter* it = frame_iter(m_fp, itId);
  switch (it->m_itype) {
  case Iter::TypeUndefined: {
    ASSERT(false);
    break;
  }
  case Iter::TypeArray:
  case Iter::TypeIterator: {
    // The emitter should never generate bytecode where the iterator
    // is at the end before IterValueC is executed. However, even if
    // the iterator is at the end, it is safe to call second().
    Cell* c1 = m_stack.allocC();
    TV_WRITE_NULL(c1);
    tvCellAsVariant(c1) = it->arr().second();
    break;
  }
  case Iter::TypeMutableArray: {
    // Dup value.
    TypedValue* tv1 = m_stack.allocTV();
    tvDup(&it->marr().val(), tv1);
    ASSERT(tv1->m_type == KindOfRef);
    tvUnbox(tv1);
    break;
  }
  default: {
    not_reached();
  }
  }
}

inline void OPTBLD_INLINE VMExecutionContext::iopIterValueV(PC& pc) {
  NEXT();
  DECODE_IA(itId);
  Iter* it = frame_iter(m_fp, itId);
  switch (it->m_itype) {
  case Iter::TypeUndefined: {
    ASSERT(false);
    break;
  }
  case Iter::TypeArray:
  case Iter::TypeIterator: {
    // The emitter should never generate bytecode where the iterator
    // is at the end before IterValueV is executed. However, even if
    // the iterator is at the end, it is safe to call secondRef().
    TypedValue* tv = m_stack.allocTV();
    TV_WRITE_NULL(tv);
    tvAsVariant(tv) = ref(it->arr().secondRef());
    break;
  }
  case Iter::TypeMutableArray: {
    // Dup value.
    TypedValue* tv1 = m_stack.allocTV();
    tvDup(&it->marr().val(), tv1);
    ASSERT(tv1->m_type == KindOfRef);
    break;
  }
  default: {
    not_reached();
  }
  }
}

inline void OPTBLD_INLINE VMExecutionContext::iopIterKey(PC& pc) {
  NEXT();
  DECODE_IVA(itId);
  Iter* it = frame_iter(m_fp, itId);
  switch (it->m_itype) {
  case Iter::TypeUndefined: {
    ASSERT(false);
    break;
  }
  case Iter::TypeArray:
  case Iter::TypeIterator: {
    // The iterator should never be at the end here. We can't check for it,
    // because that may call into user code, which has PHP-visible effects and
    // is incorrect.
    Cell* c1 = m_stack.allocC();
    TV_WRITE_NULL(c1);
    tvCellAsVariant(c1) = it->arr().first();
    break;
  }
  case Iter::TypeMutableArray: {
    // Dup key.
    TypedValue* tv1 = m_stack.allocTV();
    tvDup(&it->marr().key(), tv1);
    if (tv1->m_type == KindOfRef) {
      tvUnbox(tv1);
    }
    break;
  }
  default: {
    not_reached();
  }
  }
}

inline void OPTBLD_INLINE VMExecutionContext::iopIterNext(PC& pc) {
  PC origPc = pc;
  NEXT();
  DECODE_IA(itId);
  DECODE(Offset, offset);
  Iter* it = frame_iter(m_fp, itId);
  switch (it->m_itype) {
  case Iter::TypeUndefined: {
    ASSERT(false);
    break;
  }
  case Iter::TypeArray:
  case Iter::TypeIterator: {
    // The emitter should never generate bytecode where the iterator
    // is at the end before IterNext is executed. However, even if
    // the iterator is at the end, it is safe to call next().
    if (iter_next_array(it)) {
      // If after advancing the iterator we have not reached the end,
      // jump to the location specified by the second immediate argument.
      ITER_SKIP(offset);
    } else {
      // If after advancing the iterator we have reached the end, free
      // the iterator and fall through to the next instruction.
      ASSERT(it->m_itype == Iter::TypeUndefined);
    }
    break;
  }
  case Iter::TypeMutableArray: {
    MIterCtx &mi = it->marr();
    if (!mi.mArray().advance()) {
      // If after advancing the iterator we have reached the end, free
      // the iterator and fall through to the next instruction.
      mi.~MIterCtx();
      it->m_itype = Iter::TypeUndefined;
    } else {
      // If after advancing the iterator we have not reached the end,
      // jump to the location specified by the second immediate argument.
      ITER_SKIP(offset);
    }
    break;
  }
  default: {
    not_reached();
  }
  }
}

inline void OPTBLD_INLINE VMExecutionContext::iopIterFree(PC& pc) {
  NEXT();
  DECODE_IA(itId);
  Iter* it = frame_iter(m_fp, itId);
  switch (it->m_itype) {
  case Iter::TypeUndefined: {
    ASSERT(false);
    break;
  }
  case Iter::TypeArray:
  case Iter::TypeIterator: {
    it->arr().~ArrayIter();
    break;
  }
  case Iter::TypeMutableArray: {
    MIterCtx &mi = it->marr();
    mi.~MIterCtx();
    break;
  }
  default: {
    not_reached();
  }
  }
  it->m_itype = Iter::TypeUndefined;
}

inline void OPTBLD_INLINE inclOp(VMExecutionContext *ec, PC &pc,
                                 InclOpFlags flags) {
  NEXT();
  Cell* c1 = ec->m_stack.topC();
  String path(prepareKey(c1));
  bool initial;
  TRACE(2, "inclOp %s %s %s %s %s \"%s\"\n",
        flags & InclOpOnce ? "Once" : "",
        flags & InclOpDocRoot ? "DocRoot" : "",
        flags & InclOpRelative ? "Relative" : "",
        flags & InclOpLocal ? "Local" : "",
        flags & InclOpFatal ? "Fatal" : "",
        path->data());

  Unit* u = flags & (InclOpDocRoot|InclOpRelative) ?
    ec->evalIncludeRoot(path.get(), flags, &initial) :
    ec->evalInclude(path.get(), ec->m_fp->m_func->unit()->filepath(), &initial);
  ec->m_stack.popC();
  if (u == NULL) {
    ((flags & InclOpFatal) ?
     (void (*)(const char *, ...))raise_error :
     (void (*)(const char *, ...))raise_warning)("File not found: %s",
                                                 path->data());
    ec->m_stack.pushFalse();
  } else {
    if (!(flags & InclOpOnce) || initial) {
      ec->evalUnit(u, (flags & InclOpLocal), pc, EventHook::PseudoMain);
    } else {
      Stats::inc(Stats::PseudoMain_Guarded);
      ec->m_stack.pushTrue();
    }
  }
}

inline void OPTBLD_INLINE VMExecutionContext::iopIncl(PC& pc) {
  inclOp(this, pc, InclOpDefault);
}

inline void OPTBLD_INLINE VMExecutionContext::iopInclOnce(PC& pc) {
  inclOp(this, pc, InclOpOnce);
}

inline void OPTBLD_INLINE VMExecutionContext::iopReq(PC& pc) {
  inclOp(this, pc, InclOpFatal);
}

inline void OPTBLD_INLINE VMExecutionContext::iopReqOnce(PC& pc) {
  inclOp(this, pc, InclOpFatal | InclOpOnce);
}

inline void OPTBLD_INLINE VMExecutionContext::iopReqDoc(PC& pc) {
  inclOp(this, pc, InclOpFatal | InclOpOnce | InclOpDocRoot);
}

inline void OPTBLD_INLINE VMExecutionContext::iopReqMod(PC& pc) {
  inclOp(this, pc, InclOpFatal | InclOpOnce | InclOpDocRoot | InclOpLocal);
}

inline void OPTBLD_INLINE VMExecutionContext::iopReqSrc(PC& pc) {
  inclOp(this, pc, InclOpFatal | InclOpOnce | InclOpRelative | InclOpLocal);
}

inline void OPTBLD_INLINE VMExecutionContext::iopEval(PC& pc) {
  NEXT();
  Cell* c1 = m_stack.topC();
  String code(prepareKey(c1));
  String prefixedCode = concat("<?php ", code);
  Unit* unit = compileEvalString(prefixedCode.get());
  if (unit == NULL) {
    raise_error("Syntax error in eval()");
  }
  m_stack.popC();
  evalUnit(unit, false, pc, EventHook::Eval);
}

inline void OPTBLD_INLINE VMExecutionContext::iopDefFunc(PC& pc) {
  NEXT();
  DECODE_IVA(fid);
  Func* f = m_fp->m_func->unit()->lookupFuncId(fid);
  f->setCached();
}

inline void OPTBLD_INLINE VMExecutionContext::iopDefCls(PC& pc) {
  NEXT();
  DECODE_IVA(cid);
  PreClass* c = m_fp->m_func->unit()->lookupPreClassId(cid);
  Unit::defClass(c);
}

static inline void checkThis(ActRec* fp) {
  if (!fp->hasThis()) {
    raise_error(Strings::FATAL_NULL_THIS);
  }
}

inline void OPTBLD_INLINE VMExecutionContext::iopThis(PC& pc) {
  NEXT();
  checkThis(m_fp);
  ObjectData* this_ = m_fp->getThis();
  m_stack.pushObject(this_);
}

inline void OPTBLD_INLINE VMExecutionContext::iopBareThis(PC& pc) {
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

inline void OPTBLD_INLINE VMExecutionContext::iopCheckThis(PC& pc) {
  NEXT();
  checkThis(m_fp);
}

inline void OPTBLD_INLINE VMExecutionContext::iopInitThisLoc(PC& pc) {
  NEXT();
  DECODE_IVA(id);
  TypedValue* thisLoc = frame_local(m_fp, id);
  tvRefcountedDecRef(thisLoc);
  if (m_fp->hasThis()) {
    thisLoc->m_data.pobj = m_fp->getThis();
    thisLoc->m_type = KindOfObject;
    tvIncRef(thisLoc);
  } else {
    TV_WRITE_UNINIT(thisLoc);
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
  ASSERT(map != NULL);
  val = map->nvGet(name);
  if (val == NULL) {
    TypedValue tv;
    TV_WRITE_UNINIT(&tv);
    map->nvSet(name, &tv, false);
    val = map->nvGet(name);
    inited = false;
  } else {
    inited = true;
  }
}

inline void OPTBLD_INLINE VMExecutionContext::iopStaticLoc(PC& pc) {
  NEXT();
  DECODE_IVA(localId);
  DECODE_LITSTR(var);
  TypedValue* fr = NULL;
  bool inited;
  lookupStatic(var, m_fp, fr, inited);
  ASSERT(fr != NULL);
  if (fr->m_type != KindOfRef) {
    ASSERT(!inited);
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

inline void OPTBLD_INLINE VMExecutionContext::iopStaticLocInit(PC& pc) {
  NEXT();
  DECODE_IVA(localId);
  DECODE_LITSTR(var);
  TypedValue* fr = NULL;
  bool inited;
  lookupStatic(var, m_fp, fr, inited);
  ASSERT(fr != NULL);
  if (!inited) {
    Cell* initVal = m_stack.topC();
    // don't set _count field as the static var resides in an HphpArray
    TV_DUP_NC(initVal, fr);
  }
  if (fr->m_type != KindOfRef) {
    ASSERT(!inited);
    tvBox(fr);
  }
  TypedValue* tvLocal = frame_local(m_fp, localId);
  tvBind(fr, tvLocal);
  m_stack.discard();
}

inline void OPTBLD_INLINE VMExecutionContext::iopCatch(PC& pc) {
  NEXT();
  ASSERT(m_faults.size() > 0);
  Fault& fault = m_faults.back();
  ASSERT(fault.m_faultType == Fault::KindOfUserException);
  m_stack.pushObjectNoRc(fault.m_userException);
  m_faults.pop_back();
}

inline void OPTBLD_INLINE VMExecutionContext::iopLateBoundCls(PC& pc) {
  NEXT();
  Class* cls = frameStaticClass(m_fp);
  if (!cls) {
    raise_error(HPHP::Strings::CANT_ACCESS_STATIC);
  }
  m_stack.pushClass(cls);
}

inline void OPTBLD_INLINE VMExecutionContext::iopVerifyParamType(PC& pc) {
  SYNC(); // We might need m_pc to be updated to throw.
  NEXT();

  DECODE_IVA(param);
  const Func *func = m_fp->m_func;
  ASSERT(param < func->numParams());
  ASSERT(func->numParams() == int(func->params().size()));
  const TypeConstraint& tc = func->params()[param].typeConstraint();
  ASSERT(tc.exists());
  const TypedValue *tv = frame_local(m_fp, param);
  tc.verify(tv, func, param);
}

inline void OPTBLD_INLINE VMExecutionContext::iopNativeImpl(PC& pc) {
  NEXT();
  uint soff = m_fp->m_soff;
  BuiltinFunction func = m_fp->m_func->builtinFuncPtr();
  ASSERT(func);
  // Actually call the native implementation. This will handle freeing the
  // locals in the normal case. In the case of an exception, the VM unwinder
  // will take care of it.
  func(m_fp);
  // Adjust the stack; the native implementation put the return value in the
  // right place for us already
  m_stack.ndiscard(m_fp->m_func->numSlotsInFrame());
  ActRec* sfp = arGetSfp(m_fp);
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
      m_stack.toStringElm(os, m_stack.topTV(), m_fp);
      ONTRACE(1,
              Trace::trace("Return %s from VMExecutionContext::dispatch("
                           "%p)\n", os.str().c_str(), m_fp));
    }
#endif
    m_fp = NULL;
    m_pc = NULL;
    throw HaltVM();
  }
}

inline void OPTBLD_INLINE VMExecutionContext::iopHighInvalid(PC& pc) {
  fprintf(stderr, "invalid bytecode executed\n");
  abort();
}

inline void OPTBLD_INLINE VMExecutionContext::iopSelf(PC& pc) {
  NEXT();
  Class* clss = arGetContextClass(m_fp);
  if (!clss) {
    raise_error(HPHP::Strings::CANT_ACCESS_SELF);
  }
  m_stack.pushClass(clss);
}

inline void OPTBLD_INLINE VMExecutionContext::iopParent(PC& pc) {
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

template<bool isMethod>
c_Continuation*
VMExecutionContext::createContinuation(ActRec* fp,
                                       bool getArgs,
                                       const Func* origFunc,
                                       const Func* genFunc) {
  Object obj;
  Array args;
  if (fp->hasThis()) {
    obj = fp->getThis();
  }
  if (getArgs) {
    args = hhvm_get_frame_args(fp);
  }
  static const StringData* closure = StringData::GetStaticString("{closure}");
  const StringData* origName =
    origFunc->isClosureBody() ? closure : origFunc->fullName();
  int nLocals = genFunc->numLocals();
  int nIters = genFunc->numIterators();
  Class* genClass = SystemLib::s_ContinuationClass;
  c_Continuation* cont = c_Continuation::alloc(genClass, nLocals, nIters);
  cont->create((int64)0, (int64)genFunc, isMethod,
               StrNR(const_cast<StringData*>(origName)), obj, args);
  cont->incRefCount();
  cont->setNoDestruct();

  // The ActRec corresponding to the generator body lives as long as the object
  // does. We set it up once, here, and then just change FP to point to it when
  // we enter the generator body.
  ActRec* ar = cont->actRec();
  ar->m_func = genFunc;
  if (isMethod) {
    if (obj.get()) {
      ObjectData* objData = obj.get();
      ar->setThis(objData);
      objData->incRefCount();
    } else {
      ar->setClass(frameStaticClass(fp));
    }
  } else {
    ar->setThis(NULL);
  }
  ar->initNumArgs(1);
  ar->setVarEnv(NULL);

  TypedValue* contLocal = frame_local(ar, 0);
  contLocal->m_type = KindOfObject;
  contLocal->m_data.pobj = cont;
  // Do not incref the continuation here! Doing so will create a reference
  // cycle, since this reference is a local in the continuation frame and thus
  // will be decreffed when the continuation is destroyed. The corresponding
  // non-decref is in ~c_Continuation.

  return cont;
}

static inline void setContVar(const Func* genFunc,
                              const StringData* name,
                              TypedValue* src,
                              c_Continuation* cont) {
  Id destId = genFunc->lookupVarId(name);
  if (destId != kInvalidId) {
    tvDup(src, frame_local(cont->actRec(), destId));
  } else {
    ActRec *contFP = cont->actRec();
    if (!contFP->hasVarEnv()) {
      // We pass skipInsert to this VarEnv because it's going to exist
      // independent of the chain; i.e. we can't stack-allocate it. We link it
      // into the chain in UnpackCont, and take it out in PackCont.
      contFP->setVarEnv(VarEnv::createLazyAttach(contFP, true));
    }
    contFP->getVarEnv()->setWithRef(name, src);
  }
}

c_Continuation*
VMExecutionContext::fillContinuationVars(ActRec* fp,
                                         const Func* origFunc,
                                         const Func* genFunc,
                                         c_Continuation* cont) {
  // For functions that contain only named locals, the variable
  // environment is saved and restored by teleporting the values (and
  // their references) between the evaluation stack and the local
  // space at the end of the object using memcpy. Any variables in a
  // VarEnv are saved and restored from m_vars as usual.
  static const StringData* thisStr = StringData::GetStaticString("this");
  int nLocals = genFunc->numLocals();
  bool skipThis;
  if (fp->hasVarEnv()) {
    Stats::inc(Stats::Cont_CreateVerySlow);
    Array definedVariables = fp->getVarEnv()->getDefinedVariables();
    skipThis = definedVariables.exists("this", true);

    for (ArrayIter iter(definedVariables); !iter.end(); iter.next()) {
      setContVar(genFunc, iter.first().getStringData(),
                 const_cast<TypedValue*>(iter.secondRef().asTypedValue()), cont);
    }
  } else {
    skipThis = origFunc->lookupVarId(thisStr) != kInvalidId;
    for (Id i = 0; i < origFunc->numNamedLocals(); ++i) {
      setContVar(genFunc, origFunc->localVarName(i),
                 frame_local(fp, i), cont);
    }
  }

  // If $this is used as a local inside the body and is not provided
  // by our containing environment, just prefill it here instead of
  // using InitThisLoc inside the body
  if (!skipThis && cont->m_obj.get()) {
    Id id = genFunc->lookupVarId(thisStr);
    if (id != kInvalidId) {
      tvAsVariant(&cont->locals()[nLocals - id - 1]) = cont->m_obj;
    }
  }
  return cont;
}

inline void OPTBLD_INLINE VMExecutionContext::iopCreateCont(PC& pc) {
  NEXT();
  DECODE_IVA(getArgs);
  DECODE_LITSTR(genName);

  const Func* origFunc = m_fp->m_func;
  const Func* genFunc = origFunc->getGeneratorBody(genName);
  ASSERT(genFunc != NULL);

  bool isMethod = origFunc->isNonClosureMethod();
  c_Continuation* cont = isMethod ?
    createContinuation<true>(m_fp, getArgs, origFunc, genFunc) :
    createContinuation<false>(m_fp, getArgs, origFunc, genFunc);

  fillContinuationVars(m_fp, origFunc, genFunc, cont);

  TypedValue* ret = m_stack.allocTV();
  ret->m_type = KindOfObject;
  ret->m_data.pobj = cont;
}

static inline c_Continuation* frame_continuation(ActRec* fp) {
  ObjectData* obj = frame_local(fp, 0)->m_data.pobj;
  ASSERT(dynamic_cast<c_Continuation*>(obj));
  return static_cast<c_Continuation*>(obj);
}

static inline c_Continuation* this_continuation(ActRec* fp) {
  ObjectData* obj = fp->getThis();
  ASSERT(dynamic_cast<c_Continuation*>(obj));
  return static_cast<c_Continuation*>(obj);
}

void VMExecutionContext::iopContEnter(PC& pc) {
  NEXT();

  // The stack must be empty! Or else generatorStackBase() won't work!
  ASSERT(m_stack.top() == (TypedValue*)m_fp - m_fp->m_func->numSlotsInFrame());

  // Do linkage of the continuation's AR.
  ASSERT(m_fp->hasThis());
  c_Continuation* cont = this_continuation(m_fp);
  ActRec* contAR = cont->actRec();
  arSetSfp(contAR, m_fp);

  contAR->m_soff = m_fp->m_func->unit()->offsetOf(pc)
    - (uintptr_t)m_fp->m_func->base();
  contAR->m_savedRip = (uintptr_t)tx64->getRetFromInterpretedGeneratorFrame();

  m_fp = contAR;
  pc = contAR->m_func->getEntry();
}

void VMExecutionContext::iopContExit(PC& pc) {
  NEXT();

  ActRec* prevFp = arGetSfp(m_fp);
  pc = prevFp->m_func->getEntry() + m_fp->m_soff;
  m_fp = prevFp;
}

void VMExecutionContext::unpackContVarEnvLinkage(ActRec* fp) {
  // This is called from the TC, and is assumed not to reenter.
  if (fp->hasVarEnv()) {
    VarEnv*& topVE = g_vmContext->m_topVarEnv;
    fp->getVarEnv()->setPrevious(topVE);
    topVE = fp->getVarEnv();
  }
}

inline void OPTBLD_INLINE VMExecutionContext::iopUnpackCont(PC& pc) {
  NEXT();
  c_Continuation* cont = frame_continuation(m_fp);

  unpackContVarEnvLinkage(m_fp);

  // Return the label in a stack cell
  TypedValue* ret = m_stack.allocTV();
  ret->m_type = KindOfInt64;
  ret->m_data.num = cont->m_label;
}

void VMExecutionContext::packContVarEnvLinkage(ActRec* fp) {
  if (fp->hasVarEnv()) {
    g_vmContext->m_topVarEnv = fp->getVarEnv()->previous();
  }
}

inline void OPTBLD_INLINE VMExecutionContext::iopPackCont(PC& pc) {
  NEXT();
  DECODE_IVA(label);
  c_Continuation* cont = frame_continuation(m_fp);

  packContVarEnvLinkage(m_fp);
  cont->c_Continuation::t_update(label, tvAsCVarRef(m_stack.topTV()));
  m_stack.popTV();
}

inline void OPTBLD_INLINE VMExecutionContext::iopContReceive(PC& pc) {
  NEXT();
  c_Continuation* cont = frame_continuation(m_fp);
  Variant val = cont->t_receive();

  TypedValue* tv = m_stack.allocTV();
  TV_WRITE_UNINIT(tv);
  tvAsVariant(tv) = val;
}

inline void OPTBLD_INLINE VMExecutionContext::iopContRaised(PC& pc) {
  NEXT();
  c_Continuation* cont = frame_continuation(m_fp);
  cont->t_raised();
}

inline void OPTBLD_INLINE VMExecutionContext::iopContDone(PC& pc) {
  NEXT();
  c_Continuation* cont = frame_continuation(m_fp);
  cont->t_done();
}

inline void OPTBLD_INLINE VMExecutionContext::iopContNext(PC& pc) {
  NEXT();
  c_Continuation* cont = this_continuation(m_fp);
  cont->preNext();
  cont->m_received.setNull();
}

template<bool raise>
inline void VMExecutionContext::contSendImpl() {
  c_Continuation* cont = this_continuation(m_fp);
  cont->startedCheck();
  cont->preNext();
  cont->m_received.assignVal(tvAsVariant(frame_local(m_fp, 0)));
  if (raise) {
    cont->m_should_throw = true;
  }
}

inline void OPTBLD_INLINE VMExecutionContext::iopContSend(PC& pc) {
  NEXT();
  contSendImpl<false>();
}

inline void OPTBLD_INLINE VMExecutionContext::iopContRaise(PC& pc) {
  NEXT();
  contSendImpl<true>();
}

inline void OPTBLD_INLINE VMExecutionContext::iopContValid(PC& pc) {
  NEXT();
  TypedValue* tv = m_stack.allocTV();
  TV_WRITE_UNINIT(tv);
  tvAsVariant(tv) = !this_continuation(m_fp)->m_done;
}

inline void OPTBLD_INLINE VMExecutionContext::iopContCurrent(PC& pc) {
  NEXT();
  c_Continuation* cont = this_continuation(m_fp);
  cont->startedCheck();

  TypedValue* tv = m_stack.allocTV();
  TV_WRITE_UNINIT(tv);
  tvAsVariant(tv) = cont->m_value;
}

inline void OPTBLD_INLINE VMExecutionContext::iopContStopped(PC& pc) {
  NEXT();
  this_continuation(m_fp)->m_running = false;
}

inline void OPTBLD_INLINE VMExecutionContext::iopContHandle(PC& pc) {
  NEXT();
  c_Continuation* cont = this_continuation(m_fp);
  cont->m_running = false;
  cont->m_done = true;

  Variant exn = tvAsVariant(m_stack.topTV());
  m_stack.popC();
  ASSERT(exn.asObjRef().instanceof("exception"));
  throw exn.asObjRef();
}

inline void OPTBLD_INLINE VMExecutionContext::iopStrlen(PC& pc) {
  NEXT();
  TypedValue* subj = m_stack.topTV();
  int64 ans = 0;
  if (LIKELY(IS_STRING_TYPE(subj->m_type))) {
    ans = subj->m_data.pstr->size();
  } else {
    ans = f_strlen(tvAsVariant(subj));
  }

  tvRefcountedDecRef(subj);
  subj->m_type = KindOfInt64;
  subj->m_data.num = ans;
}

void VMExecutionContext::classExistsImpl(PC& pc, Attr typeAttr) {
  NEXT();
  TypedValue* aloadTV = m_stack.topTV();
  tvCastToBooleanInPlace(aloadTV);
  ASSERT(aloadTV->m_type == KindOfBoolean);
  bool autoload = aloadTV->m_data.num;
  m_stack.popX();

  TypedValue* name = m_stack.topTV();
  tvCastToStringInPlace(name);
  ASSERT(IS_STRING_TYPE(name->m_type));

  tvAsVariant(name) = Unit::classExists(name->m_data.pstr, autoload, typeAttr);
}

inline void OPTBLD_INLINE VMExecutionContext::iopClassExists(PC& pc) {
  classExistsImpl(pc, AttrNone);
}

inline void OPTBLD_INLINE VMExecutionContext::iopInterfaceExists(PC& pc) {
  classExistsImpl(pc, AttrInterface);
}

inline void OPTBLD_INLINE VMExecutionContext::iopTraitExists(PC& pc) {
  classExistsImpl(pc, AttrTrait);
}

string
VMExecutionContext::prettyStack(const string& prefix) const {
  if (!getFP()) {
    string s("__Halted");
    return s;
  }
  int offset = (m_fp->m_func->unit() != NULL)
               ? pcOff()
               : 0;
  string begPrefix = prefix + "__";
  string midPrefix = prefix + "|| ";
  string endPrefix = prefix + "\\/";
  string stack = m_stack.toString(m_fp, offset, midPrefix);
  return begPrefix + "\n" + stack + endPrefix;
}

void VMExecutionContext::checkRegStateWork() const {
  ASSERT(tl_regState == REGSTATE_CLEAN);
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
  if (fp == NULL) {
    std::cout << "Don't have a valid fp\n";
    return;
  }

  printf("Offset = %d, in function %s\n", pc, fp->m_func->name()->data());
  Unit* u = fp->m_func->unit();
  if (u == NULL) {
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
  printf("Called from TC address %p\n",
         TranslatorX64::Get()->getTranslatedCaller());
  std::cout << u->filepath()->data() << ':'
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
  ASSERT(*pc == Op##name);                                                    \
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
  const Func* f = curFunc();
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
  static const void *optabInst[] __attribute__((unused)) = {
#define O(name, imm, push, pop, flags) \
    &&LabelInst##name,
    OPCODES
#undef O
  };
  static const void *optabCover[] = {
#define O(name, imm, push, pop, flags) \
    &&LabelCover##name,
    OPCODES
#undef O
  };
  ASSERT(sizeof(optabDirect) / sizeof(const void *) == Op_count);
  ASSERT(sizeof(optabDbg) / sizeof(const void *) == Op_count);
  const void **optab = optabDirect;
  InjectionTableInt64* injTable = g_vmContext->m_injTables ?
    g_vmContext->m_injTables->getInt64Table(InstHookTypeBCPC) : NULL;
  bool collectCoverage = ThreadInfo::s_threadInfo->m_reqInjectionData.coverage;
  if (injTable) {
    optab = optabInst;
  } else if (collectCoverage) {
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

#define DISPATCH() do {                                                       \
    if ((breakOnCtlFlow && isCtlFlow) ||                                      \
        (limInstrs && UNLIKELY(numInstrs-- == 0))) {                          \
      ONTRACE(1,                                                              \
              Trace::trace("dispatch: Halt ExecutionContext::dispatch(%p)\n", \
                           m_fp));                                            \
      delete g_vmContext->m_lastLocFilter;                                    \
      g_vmContext->m_lastLocFilter = NULL;                                    \
      return;                                                                 \
    }                                                                         \
    Op op = (Op)*pc;                                                          \
    COND_STACKTRACE("dispatch:                    ");                         \
    ONTRACE(1,                                                                \
            Trace::trace("dispatch: %d: %s\n", pcOff(), nametab[op]));        \
    ASSERT(op < Op_count);                                                    \
    if (profile && (op == OpRetC || op == OpRetV)) { \
      profileReturnValue(m_stack.top()->m_type); \
    } \
    goto *optab[op];                                                          \
} while (0)

  ONTRACE(1, Trace::trace("dispatch: Enter ExecutionContext::dispatch(%p)\n",
          m_fp));
  PC pc = m_pc;
  DISPATCH();

#define O(name, imm, pusph, pop, flags)                                       \
  LabelDbg##name:                                                             \
    phpDebuggerHook(pc);                                                      \
  LabelInst##name:                                                            \
    INST_HOOK_PC(injTable, pc);                                               \
  LabelCover##name:                                                           \
    if (collectCoverage) {                                                    \
      recordCodeCoverage(pc);                                                 \
    }                                                                         \
  Label##name: {                                                              \
    iop##name(pc);                                                            \
    SYNC();                                                                   \
    if (breakOnCtlFlow) {                                                     \
      isCtlFlow = instrIsControlFlow(Op##name);                               \
      Stats::incOp(Op##name);                                                 \
    }                                                                         \
    DISPATCH();                                                               \
  }
  OPCODES
#undef O
#undef DISPATCH
}

class InterpretingFlagGuard {
private:
  bool m_oldFlag;
public:
  InterpretingFlagGuard() {
    m_oldFlag = g_vmContext->m_interpreting;
    g_vmContext->m_interpreting = true;
  }
  ~InterpretingFlagGuard() {
    g_vmContext->m_interpreting = m_oldFlag;
  }
};

void VMExecutionContext::dispatch() {
  InterpretingFlagGuard ifg;
  if (shouldProfile()) {
    dispatchImpl<Profile>(0);
  } else {
    dispatchImpl<0>(0);
  }
}

void VMExecutionContext::dispatchN(int numInstrs) {
  InterpretingFlagGuard ifg;
  dispatchImpl<LimitInstrs>(numInstrs);
  // We are about to go back to Jit, check whether we should
  // stick with interpreter
  if (DEBUGGER_FORCE_INTR) {
    throw VMSwitchModeException(false);
  }
}

void VMExecutionContext::dispatchBB() {
  InterpretingFlagGuard ifg;
  dispatchImpl<BreakOnCtlFlow>(0);
  // We are about to go back to Jit, check whether we should
  // stick with interpreter
  if (DEBUGGER_FORCE_INTR) {
    throw VMSwitchModeException(false);
  }
}

void VMExecutionContext::recordCodeCoverage(PC pc) {
  Unit* unit = getFP()->m_func->unit();
  ASSERT(unit != NULL);
  if (unit == SystemLib::s_nativeFuncUnit ||
      unit == SystemLib::s_nativeClassUnit) {
    return;
  }
  int line = unit->getLineNumber(pcOff());
  ASSERT(line != -1);

  if (unit != m_coverPrevUnit || line != m_coverPrevLine) {
    ThreadInfo* info = ThreadInfo::s_threadInfo.getNoCheck();
    m_coverPrevUnit = unit;
    m_coverPrevLine = line;
    const StringData* filepath = unit->filepath();
    ASSERT(filepath->isStatic());
    info->m_coverage->Record(filepath->data(), line, line);
  }
}

void VMExecutionContext::resetCoverageCounters() {
  m_coverPrevLine = -1;
  m_coverPrevUnit = NULL;
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
  ASSERT(m_nestedVMs.size() >= 1);

  VMState savedVM;
  memcpy(&savedVM, &m_nestedVMs.back(), sizeof(savedVM));
  m_pc = savedVM.pc;
  m_fp = savedVM.fp;
  m_firstAR = savedVM.firstAR;
  ASSERT(m_stack.top() == savedVM.sp);

  if (debug) {
    const ReentryRecord& rr = m_nestedVMs.back();
    const VMState& savedVM = rr.m_savedState;
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
  const_assert(hhvm);

  ASSERT(SystemLib::s_unit);
  ASSERT(SystemLib::s_nativeFuncUnit);
  ASSERT(SystemLib::s_nativeClassUnit);

  new (&s_requestArenaStorage) RequestArena();
  new (&s_varEnvArenaStorage) VarEnvArena();
  m_stack.requestInit();
  tx64 = nextTx64;
  tx64->requestInit();

  // Merge the systemlib unit into the ExecutionContext
  SystemLib::s_unit->merge();
  SystemLib::s_nativeFuncUnit->merge();
  SystemLib::s_nativeClassUnit->merge();
  profileRequestStart();

#ifdef DEBUG
  Class *cls = *Unit::GetNamedEntity(s_stdclass.get())->clsList();
  ASSERT(cls);
  ASSERT(cls == SystemLib::s_stdclassClass);
#endif
}

void VMExecutionContext::requestExit() {
  const_assert(hhvm);

  destructObjects();
  syncGdbState();
  tx64->requestExit();
  tx64 = NULL;
  m_stack.requestExit();
  profileRequestEnd();
  EventHook::Disable();

  if (m_globalVarEnv) {
    ASSERT(m_topVarEnv = m_globalVarEnv);
    VM::VarEnv::destroy(m_globalVarEnv);
    m_globalVarEnv = m_topVarEnv = 0;
  }

  varenv_arena().~VarEnvArena();
  request_arena().~RequestArena();
}

///////////////////////////////////////////////////////////////////////////////
}
