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

#ifndef incl_HPHP_TYPES_H_
#define incl_HPHP_TYPES_H_

#include "hphp/util/base.h"
#include "hphp/runtime/base/datatype.h"
#include "hphp/util/thread-local.h"
#include "hphp/util/mutex.h"
#include "hphp/util/case-insensitive.h"
#include "hphp/runtime/base/macros.h"
#include "hphp/runtime/base/memory-manager.h"

#include <boost/intrusive_ptr.hpp>

#include <stdint.h>
#include <atomic>
#include <limits>
#include <type_traits>
#include <vector>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// forward declarations of all data types

/**
 * Complex data types. Note that Numeric, Primitive, PlusOperand and Sequence
 * are actually all Variant type in implementation, but we'd keep them using
 * their own names in different places, so not to lose their inferred types.
 * Should we need to take advantage of this extra type inference information
 * in the future, we will be able to.
 */
class String;
class StaticString;
class Array;
class Object;
template<typename T> class SmartObject;
class Resource;
template<typename T> class SmartResource;
class Variant;
class VarNR;
class RefData;

/**
 * Macros related to Variant that are needed by StringData, ObjectData,
 * and ArrayData.
 */
extern const Variant null_variant;      // uninitialized variant
extern const Variant init_null_variant; // php null
extern const VarNR null_varNR;
extern const VarNR true_varNR;
extern const VarNR false_varNR;
extern const VarNR INF_varNR;
extern const VarNR NEGINF_varNR;
extern const VarNR NAN_varNR;
extern const String null_string;
extern const Array null_array;
extern const Array empty_array;

/*
 * All Refcounted types have their m_count field at the same offset
 * in the object. This offset is chosen to allow a RefData's count
 * field to pack after a TypedValue.
 *
 * Other refcounted types (ArrayData, StringData, and ObjectData)
 * have small fields that are packed into the same space.
 */
const size_t FAST_REFCOUNT_OFFSET = 12;

/*
 * All native collection class have their m_size field at the same
 * offset in the object.
 */
const size_t FAST_COLLECTION_SIZE_OFFSET = 20;

/**
 * These are underlying data structures for the above complex data types. Since
 * we use reference counting to achieve copy-on-write and automatic object
 * lifetime, we need these data objects to store real data that's shared by
 * multiple wrapper objects.
 */
class StringData;
class ArrayData;
class ObjectData;
class ResourceData;

/**
 * Miscellaneous objects to help arrays to construct or to iterate.
 */
class ArrayIter;
class MutableArrayIter;

class FullPos;

class VariableSerializer;
class VariableUnserializer;

///////////////////////////////////////////////////////////////////////////////

/**
 * Many functions may elect to take "litstr" separately from "String" class.
 * This code specialization helps speed a lot by not instantiating a String
 * object to box an otherwise literal value. This also means, though not
 * obviously thus dangerous not to know, whenever a function takes a parameter
 * with type of "litstr", one can only pass in a literal string that has
 * a "permanent" memory address to be stored. To make this really clear, I
 * invented "litstr" as a typedef-ed name for "const char *" that expects a
 * literal string only. Therefore, throughout this entire runtime library,
 *
 *   litstr == literal string
 *   const char * == any C-string pointer
 *
 */

namespace Uns {
enum class Mode {
  Value = 0,
  Key = 1,
  ColValue = 2,
  ColKey = 3,
};
}

namespace Collection {
enum Type {
  InvalidType = 0,
  VectorType = 1,
  MapType = 2,
  StableMapType = 3,
  SetType = 4,
  PairType = 5,
  FrozenVectorType = 6,
  FrozenSetType = 7,
  MaxNumTypes = 8
};
inline Type stringToType(const char* str, size_t len) {
  switch (len) {
    case 3:
      if (!strcasecmp(str, "map")) return MapType;
      break;
    case 4:
      if (!strcasecmp(str, "pair")) return PairType;
      break;
    case 6:
      if (!strcasecmp(str, "hh\\set")) return SetType;
      break;
    case 9:
      if (!strcasecmp(str, "stablemap")) return StableMapType;
      if (!strcasecmp(str, "hh\\vector")) return VectorType;
      break;
    case 12:
      if (!strcasecmp(str, "hh\\frozenset")) return FrozenSetType;
      break;
    case 15:
      if (!strcasecmp(str, "hh\\frozenvector")) return FrozenVectorType;
      break;
    default:
      break;
  }
  return InvalidType;
}
inline Type stringToType(const std::string& s) {
  return stringToType(s.c_str(), s.size());
}
}

/**
 * Some of these typedefs are for platform independency, including "int64".
 * Some of them are for clarity, for example, "litstr". Some of them are purely
 * for being able to vertically align type-specialized functions so they look
 * cleaner.
 */
typedef const char * litstr; /* literal string */
//typedef const String & CStrRef;
typedef const Array & CArrRef;
typedef const Object & CObjRef;
typedef const Resource & CResRef;
typedef const Variant & CVarRef;

typedef const class VRefParamValue    &VRefParam;
typedef const class RefResultValue    &RefResult;
typedef const class VariantStrongBind &CVarStrongBind;
typedef const class VariantWithRefBind&CVarWithRefBind;

inline CVarStrongBind
strongBind(CVarRef v)     { return *(VariantStrongBind*)&v; }
inline CVarStrongBind
strongBind(RefResult v)   { return *(VariantStrongBind*)&v; }
inline CVarWithRefBind
withRefBind(CVarRef v)    { return *(VariantWithRefBind*)&v; }

inline CVarRef
variant(CVarStrongBind v) { return *(Variant*)&v; }
inline CVarRef
variant(CVarWithRefBind v){ return *(Variant*)&v; }
inline CVarRef
variant(RefResult v)      { return *(Variant*)&v; }
inline CVarRef
variant(CVarRef v)        { return v; }

/**
 * ref() can be used to cause strong binding
 *
 *   a = ref(b); // strong binding: now both a and b point to the same data
 *   a = b;      // weak binding: a will copy or copy-on-write
 *
 */
inline RefResult ref(CVarRef v) {
  return *(RefResultValue*)&v;
}

inline RefResult ref(Variant& v) {
  return *(RefResultValue*)&v;
}

class Class;

///////////////////////////////////////////////////////////////////////////////
// code injection classes

class RequestInjectionData {
public:
  static const ssize_t MemExceededFlag      = 1 << 0;
  static const ssize_t TimedOutFlag         = 1 << 1;
  static const ssize_t SignaledFlag         = 1 << 2;
  static const ssize_t EventHookFlag        = 1 << 3;
  static const ssize_t PendingExceptionFlag = 1 << 4;
  static const ssize_t InterceptFlag        = 1 << 5;
  // Set by the debugger to break out of loops in translated code.
  static const ssize_t DebuggerSignalFlag   = 1 << 6;
  static const ssize_t LastFlag             = DebuggerSignalFlag;

  RequestInjectionData()
    : cflagsPtr(nullptr),
      m_timeoutSeconds(-1), m_hasTimer(false), m_timerActive(false),
      m_debugger(false), m_debuggerIntr(false), m_coverage(false),
      m_jit(false) {
  }

  ~RequestInjectionData();

  inline std::atomic<ssize_t>* getConditionFlags() {
    assert(cflagsPtr);
    return cflagsPtr;
  }

  std::atomic<ssize_t>* cflagsPtr;  // this points to the real condition flags,
                                    // somewhere in the thread's targetcache

 private:
#ifndef __APPLE__
  timer_t m_timer_id;    // id of our timer
#endif
  int m_timeoutSeconds;  // how many seconds to timeout
  bool m_hasTimer;       // Whether we've created our timer yet
  std::atomic<bool> m_timerActive;
                         // Set true when we activate a timer,
                         // cleared when the signal handler runs
  bool m_debugger;       // whether there is a DebuggerProxy attached to me
  bool m_debuggerIntr;   // indicating we should force interrupt for debugger
  bool m_coverage;       // is coverage being collected
  bool m_jit;            // is the jit enabled
 public:
  int getTimeout() const { return m_timeoutSeconds; }
  void setTimeout(int seconds);
  int getRemainingTime() const;
  void resetTimer(int seconds = 0);
  void onTimeout();
  bool getJit() const { return m_jit; }
  bool getDebugger() const { return m_debugger; }
  void setDebugger(bool d) {
    m_debugger = d;
    updateJit();
  }
  static constexpr uint32_t debuggerReadOnlyOffset() {
    return offsetof(RequestInjectionData, m_debugger);
  }
  bool getDebuggerIntr() const { return m_debuggerIntr; }
  void setDebuggerIntr(bool d) {
    m_debuggerIntr = d;
    updateJit();
  }
  bool getCoverage() const { return m_coverage; }
  void setCoverage(bool flag) {
    m_coverage = flag;
    updateJit();
  }
  void updateJit();

  std::stack<void *> interrupts;   // CmdInterrupts this thread's handling

  void reset();

  void setMemExceededFlag();
  void setTimedOutFlag();
  void clearTimedOutFlag();
  void setSignaledFlag();
  void setEventHookFlag();
  void clearEventHookFlag();
  void setPendingExceptionFlag();
  void clearPendingExceptionFlag();
  void setInterceptFlag();
  void clearInterceptFlag();
  void setDebuggerSignalFlag();
  ssize_t fetchAndClearFlags();

  void onSessionInit();
};

class GlobalNameValueTableWrapper;
class ObjectAllocatorBase;
class Profiler;
class CodeCoverage;
typedef GlobalNameValueTableWrapper GlobalVariables;

int object_alloc_size_to_index(size_t);
size_t object_alloc_index_to_size(int);

// implemented in runtime/base/thread-info
typedef boost::intrusive_ptr<ArrayData> ArrayHolder;
void intrusive_ptr_add_ref(ArrayData* a);
void intrusive_ptr_release(ArrayData* a);

class ThreadInfo {
public:
  enum Executing {
    Idling,
    RuntimeFunctions,
    ExtensionFunctions,
    UserFunctions,
    NetworkIO,
  };

  static void GetExecutionSamples(std::map<Executing, int> &counts);

public:
  static DECLARE_THREAD_LOCAL_NO_CHECK(ThreadInfo, s_threadInfo);

  RequestInjectionData m_reqInjectionData;

  // For infinite recursion detection.  m_stacklimit is the lowest
  // address the stack can grow to.
  char *m_stacklimit;

  // Either null, or populated by initialization of ThreadInfo as an
  // approximation of the highest address of the current thread's
  // stack.
  static __thread char* t_stackbase;

  // This is the amount of "slack" in stack usage checks - if the
  // stack pointer gets within this distance from the end (minus
  // overhead), throw an infinite recursion exception.
  static const int StackSlack = 1024 * 1024;

  MemoryManager* m_mm;

  // This pointer is set by ProfilerFactory
  Profiler *m_profiler;
  CodeCoverage *m_coverage;

  Executing m_executing;

  // A C++ exception which will be thrown by the next surprise check.
  Exception* m_pendingException;

  ThreadInfo();
  ~ThreadInfo();

  void onSessionInit();
  void onSessionExit();
  void setPendingException(Exception* e);
  void clearPendingException();

  static bool valid(ThreadInfo* info);
};

extern void throw_infinite_recursion_exception();
extern void throw_call_non_object() ATTRIBUTE_NORETURN;

inline void* stack_top_ptr() {
  DECLARE_STACK_POINTER(sp);
  return sp;
}

inline bool stack_in_bounds(ThreadInfo *&info) {
  return stack_top_ptr() >= info->m_stacklimit;
}

// The ThreadInfo pointer itself must be from the current stack frame.
inline void check_recursion(ThreadInfo *&info) {
  if (!stack_in_bounds(info)) {
    throw_infinite_recursion_exception();
  }
}

// implemented in runtime/base/builtin-functions.cpp
extern ssize_t check_request_surprise(ThreadInfo *info);

// implemented in runtime/ext/ext_hotprofiler.cpp
extern void begin_profiler_frame(Profiler *p, const char *symbol);
extern void end_profiler_frame(Profiler *p, const char *symbol);

///////////////////////////////////////////////////////////////////////////////

class ExecutionProfiler {
public:
  ExecutionProfiler(ThreadInfo *info, bool builtin) : m_info(info) {
    m_executing = m_info->m_executing;
    m_info->m_executing =
      builtin ? ThreadInfo::ExtensionFunctions : ThreadInfo::UserFunctions;
  }
  explicit ExecutionProfiler(ThreadInfo::Executing executing) {
    m_info = ThreadInfo::s_threadInfo.getNoCheck();
    m_executing = m_info->m_executing;
    m_info->m_executing = executing;
  }
  ~ExecutionProfiler() {
    m_info->m_executing = m_executing;
  }
private:
  ThreadInfo *m_info;
  ThreadInfo::Executing m_executing;
};

class AccessFlags {
public:
  enum Type {
    None = 0,
    Error = 1,
    CheckExist = 2,
    Key = 4,
    Error_Key = Error | Key,
  };
  static Type IsKey(bool s) { return s ? Key : None; }
  static Type IsError(bool e) { return e ? Error : None; }
};

#define ACCESSPARAMS_DECL AccessFlags::Type flags = AccessFlags::None
#define ACCESSPARAMS_IMPL AccessFlags::Type flags

/*
 * Non-enumerated version of type for referring to opcodes or the
 * bytecode stream.  (Use the enum Op in hhbc.h for an enumerated
 * version.)
 */
typedef uint8_t Opcode;

/*
 * Program counters in the bytecode interpreter.
 *
 * Normally points to an Opcode, but has type const uchar* because
 * during a given instruction it is incremented while decoding
 * immediates and may point to arbitrary bytes.
 */
typedef const uchar* PC;

/*
 * Id type for various components of a unit that have to have unique
 * identifiers.  For example, function ids, class ids, string literal
 * ids.
 */
typedef int Id;
const Id kInvalidId = Id(-1);

// Bytecode offsets.  Used for both absolute offsets and relative
// offsets.
typedef int32_t Offset;
constexpr Offset kInvalidOffset = std::numeric_limits<Offset>::max();
typedef hphp_hash_set<Offset> OffsetSet;

/*
 * Various fields in the VM's runtime have indexes that are addressed
 * using this "slot" type.  For example: methods, properties, class
 * constants.
 *
 * No slot value greater than or equal to kInvalidSlot will actually
 * be used for one of these.
 */
typedef uint32_t Slot;
const Slot kInvalidSlot = Slot(-1);

/*
 * Handles into Request Data Segment.  These are offsets from
 * RDS::tl_base.  See rds.h.
 */
namespace RDS {
  typedef uint32_t Handle;
  constexpr Handle kInvalidHandle = 0;
}

/*
 * Unique identifier for a Func*.
 */
typedef uint32_t FuncId;
constexpr FuncId InvalidFuncId = FuncId(-1LL);
typedef hphp_hash_set<FuncId> FuncIdSet;

/*
 * Special types that are not relevant to the runtime as a whole.
 * The order for public/protected/private matters in numerous places.
 *
 * Attr unions are directly stored as integers in .hhbc repositories, so
 * incompatible changes here require a schema version bump.
 *
 * AttrTrait on a method means that the method is NOT a constructor,
 * even though it may look like one
 *
 * AttrNoOverride (WholeProgram only) on a class means its not extended
 * and on a method means that no extending class defines the method.
 *
 * AttrVariadicByRef indicates a function is a builtin that takes
 * variadic arguments, where the arguments are either by ref or
 * optionally by ref.  (It is equivalent to having ClassInfo's
 * (RefVariableArguments | MixedVariableArguments).)
 *
 * AttrMayUseVV indicates that a function may need to use a VarEnv or
 * varargs (aka extraArgs) at run time.
 *
 * AttrPhpLeafFn indicates a function does not make any explicit calls
 * to other php functions.  It may still call other user-level
 * functions via re-entry (e.g. for destructors or autoload), and it
 * may make calls to builtins using FCallBuiltin.
 *
 * AttrBuiltin is set on builtin functions - whether c++ or php
 *
 * AttrAllowOverride is set on builtin functions that can be replaced
 *   by user implementations
 *
 * AttrSkipFrame is set to indicate that the frame should be ignored
 *   when searching for the context (eg array_map evaluates its
 *   callback in the context of its caller).
 *
 * AttrVMEntry is set on functions that are generally VM entry points.
 *   This is not necessary for correctness; it simply allows better
 *   code generation in the JIT.
 */
enum Attr {
  AttrNone          = 0,         // class  property  method  //
  AttrReference     = (1 <<  0), //                     X    //
  AttrPublic        = (1 <<  1), //            X        X    //
  AttrProtected     = (1 <<  2), //            X        X    //
  AttrPrivate       = (1 <<  3), //            X        X    //
  AttrStatic        = (1 <<  4), //            X        X    //
  AttrAbstract      = (1 <<  5), //    X                X    //
  AttrFinal         = (1 <<  6), //    X                X    //
  AttrInterface     = (1 <<  7), //    X                     //
  AttrPhpLeafFn     = (1 <<  7), //                     X    //
  AttrTrait         = (1 <<  8), //    X                X    //
  AttrNoInjection   = (1 <<  9), //                     X    //
  AttrUnique        = (1 << 10), //    X                X    //
  AttrDynamicInvoke = (1 << 11), //                     X    //
  AttrNoExpandTrait = (1 << 12), //    X                     //
  AttrNoOverride    = (1 << 13), //    X                X    //
  AttrClone         = (1 << 14), //                     X    //
  AttrVariadicByRef = (1 << 15), //                     X    //
  AttrMayUseVV      = (1 << 16), //                     X    //
  AttrPersistent    = (1 << 17), //    X                X    //
  AttrDeepInit      = (1 << 18), //            X             //
  AttrHot           = (1 << 19), //                     X    //
  AttrBuiltin       = (1 << 20), //    X                X    //
  AttrAllowOverride = (1 << 21), //                     X    //
  AttrSkipFrame     = (1 << 22), //                     X    //
  AttrNative        = (1 << 23), //                     X    //
  AttrVMEntry       = (1 << 24), //                     X    //
};

inline Attr operator|(Attr a, Attr b) { return Attr((int)a | (int)b); }

inline const char* attrToVisibilityStr(Attr attr) {
  return (attr & AttrPrivate)   ? "private"   :
         (attr & AttrProtected) ? "protected" : "public";
}

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_TYPES_H_
