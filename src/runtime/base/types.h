/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef __HPHP_TYPES_H__
#define __HPHP_TYPES_H__

#include <util/base.h>
#include <util/thread_local.h>
#include <util/mutex.h>
#include <util/case_insensitive.h>
#include <vector>
#include <runtime/base/macros.h>
#include <runtime/base/memory/memory_manager.h>

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
class Variant;
typedef Variant Numeric;
typedef Variant Primitive;
typedef Variant PlusOperand;
typedef Variant Sequence;

/**
 * Macros related to Variant that are needed by StringData, ObjectData,
 * and ArrayData.
 */
extern const Variant null_variant;
extern const String null_string;
extern const Array null_array;
#if defined(__GNUC__) && defined(WORDSIZE_IS_64)
#define FAST_REFCOUNT_FOR_VARIANT
#define FAST_REFCOUNT_OFFSET (sizeof(void*))
#endif

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
 * Arrays, strings and objects can take offsets or array elements. These offset
 * objects will help to store temporary information to make the task easier.
 */
class StringOffset;

/**
 * Miscellaneous objects to help arrays to construct or to iterate.
 */
class ArrayIter;
class MutableArrayIter;

struct FullPos;

class VariableSerializer;
class VariableUnserializer;
class FiberReferenceMap;

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

enum DataType {
  /**
   * Beware if you change the order, as we may have a few type checks in the
   * code that depend on the order.
   */
  KindOfUninit  = 0,
  KindOfNull    = 1,
  KindOfBoolean = 2,
  KindOfInt32   = 3,
  KindOfInt64   = 4,
  KindOfDouble  = 5,
  KindOfStaticString  = 6,
  KindOfString  = 7,
  KindOfArray   = 8,
  KindOfObject  = 9,
  KindOfVariant = 10,

  MaxNumDataTypes = 11, // marker, not a valid type

  MaxDataType   = 0x7fffffff // Allow KindOf* > 11 in HphpArray.
};

inline int getDataTypeIndex(DataType t) {
  return t;
}

// Helper macro for checking if a given type is refcounted
#define IS_REFCOUNTED_TYPE(t) ((t) > KindOfStaticString)

enum StringDataMode {
  AttachLiteral, // const char * points to a literal string
  AttachString,  // const char * points to a malloc-ed string
  CopyString,    // make a real copy of the string

  StringDataModeCount
};

/**
 * Some of these typedefs are for platform independency, including "int64".
 * Some of them are for clarity, for example, "litstr". Some of them are purely
 * for being able to vertically align type-specialized functions so they look
 * cleaner.
 */
typedef const char * litstr; /* literal string */
typedef const String & CStrRef;
typedef const Array & CArrRef;
typedef const Object & CObjRef;
typedef const Variant & CVarRef;

///////////////////////////////////////////////////////////////////////////////
// code injection classes

class RequestInjectionData {
public:
  RequestInjectionData()
    : started(0), timeoutSeconds(-1), memExceeded(false), timedout(false),
      signaled(false), surprised(false), debugger(false), debuggerIdle(0) {
  }

  time_t started;      // when a request was started
  int timeoutSeconds;  // how many seconds to timeout

  bool memExceeded;    // memory limit was exceeded
  bool timedout;       // flag to set when timeout is detected
  bool signaled;       // flag to set when a signal was raised

  bool surprised;      // any surprise happened
  Mutex surpriseMutex; // mutex protecting per-request data

  bool debugger;       // whether there is a DebuggerProxy attached to me
  int  debuggerIdle;   // skipping this many interrupts while proxy is idle
  std::stack<void *> interrupts;   // CmdInterrupts this thread's handling

  void reset();
  void onSessionInit();
};

class FrameInjection;
class ObjectAllocatorBase;
class Profiler;
class GlobalVariables;

// implemented in runtime/base/thread_info
DECLARE_BOOST_TYPES(Array);
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

  FrameInjection *m_top;
  RequestInjectionData m_reqInjectionData;

  // For infinite recursion detection
  size_t m_stacksize;
  char *m_stacklimit;

  // This is the amount of "slack" in stack usage checks - if the
  // stack pointer gets within this distance from the end (minus
  // overhead), throw an infinite recursion exception.
  static const int StackSlack = 1024 * 1024;

  MemoryManager* m_mm;

  // This pointer is set by ProfilerFactory
  Profiler *m_profiler;

  GlobalVariables *m_globals;
  Executing m_executing;
  bool m_pendingException;
  ArrayPtr m_exceptionStack;
  std::string m_exceptionMsg;

  ThreadInfo();
  ~ThreadInfo();

  void onSessionInit();
  void onSessionExit();
  void clearPendingException();
};

extern void throw_infinite_recursion_exception();
extern void throw_call_non_object() ATTRIBUTE_COLD __attribute__((noreturn));

// The ThreadInfo pointer itself must be from the current stack frame.
inline void check_recursion(ThreadInfo *&info) {
  if ((char *)&info < info->m_stacklimit) {
    throw_infinite_recursion_exception();
  }
}

// implemented in runtime/base/builtin_functions.cpp
extern void pause_and_exit() ATTRIBUTE_COLD __attribute__((noreturn));
extern void check_request_surprise(ThreadInfo *info) ATTRIBUTE_COLD;

extern bool SegFaulting;

inline void check_request_timeout(ThreadInfo *info) {
  if (SegFaulting) pause_and_exit();
  info->m_mm->refreshStats();
  if (info->m_reqInjectionData.surprised) check_request_surprise(info);
}

inline void check_request_timeout_NA(ThreadInfo *info) {
  if (SegFaulting) pause_and_exit();
  ASSERT(info->m_mm->DEBUG_checkStats());
  if (info->m_reqInjectionData.surprised) check_request_surprise(info);
}

void throw_pending_exception(ThreadInfo *info) ATTRIBUTE_COLD
                                               __attribute__((noreturn));

void check_request_timeout_info(ThreadInfo *info, int lc);
void check_request_timeout_ex(const FrameInjection &fi, int lc);

// implemented in runtime/ext/ext_hotprofiler.cpp
extern void begin_profiler_frame(Profiler *p, const char *symbol);
extern void end_profiler_frame(Profiler *p);

///////////////////////////////////////////////////////////////////////////////

class ExecutionProfiler {
public:
  ExecutionProfiler(ThreadInfo *info, bool builtin) : m_info(info) {
    m_executing = m_info->m_executing;
    m_info->m_executing =
      builtin ? ThreadInfo::ExtensionFunctions : ThreadInfo::UserFunctions;
  }
  ExecutionProfiler(ThreadInfo::Executing executing) {
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

///////////////////////////////////////////////////////////////////////////////
// Fast Method Call
struct MethodIndex {
  unsigned int m_callIndex:32;
  unsigned int m_overloadIndex:32;
  MethodIndex (unsigned int callIndex, unsigned int overloadIndex) :
    m_callIndex(callIndex), m_overloadIndex(overloadIndex) {}
  uint64 val() const { return ((uint64)m_callIndex)<<32|m_overloadIndex; }
  bool operator== (const MethodIndex& mi) const { return val()==mi.val(); }

  static MethodIndex fail() { return MethodIndex(0,0); }
  bool isFail() { return val()==0; }
  private:
  MethodIndex() {}
};

struct MethodIndexHMap {
  MethodIndexHMap() : name(NULL), methodIndex(0,0) {}
  MethodIndexHMap(const char *name, MethodIndex methodIndex)
    : name(name), methodIndex(methodIndex) {}
  const char* name;
  MethodIndex methodIndex;
  static MethodIndex methodIndexExists(CStrRef methodName);
  static void initialize(bool useSystem);
};

extern const unsigned g_methodIndexHMapSize;
extern const MethodIndexHMap g_methodIndexHMap[];
extern const unsigned g_methodIndexReverseCallIndex[];
extern const char * g_methodIndexReverseIndex[];

extern const unsigned g_methodIndexHMapSizeSys;
extern const MethodIndexHMap g_methodIndexHMapSys[];
extern const unsigned g_methodIndexReverseCallIndexSys[];
extern const char * g_methodIndexReverseIndexSys[];

inline MethodIndex methodIndexExists(CStrRef methodName) {
  return MethodIndexHMap::methodIndexExists(methodName);
}

inline MethodIndex methodIndexLookup(CStrRef methodName) {
  MethodIndex methodIndex = MethodIndexHMap::methodIndexExists(methodName);
  ASSERT(!methodIndex.isFail());
  return methodIndex;
}

const bool g_bypassMILR = true;
const char * methodIndexLookupReverse(MethodIndex methodIndex) ;

class CallInfo;
class MethodCallPackage;

class AccessFlags {
public:
  enum Type {
    None = 0,

    Error = 1,
    CheckExist = 2,
    Key = 4,

    Error_Key = Error | Key,
    CheckExist_Key = CheckExist | Key
  };
  static Type IsKey(bool s) { return s ? Key : None; }
  static Type IsError(bool e) { return e ? Error : None; }
};

#define ACCESSPARAMS_DECL AccessFlags::Type flags = AccessFlags::None
#define ACCESSPARAMS_IMPL AccessFlags::Type flags
///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_TYPES_H__
