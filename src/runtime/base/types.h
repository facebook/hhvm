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
class ObjectOffset;

/**
 * Miscellaneous objects to help arrays to construct or to iterate.
 */
class ArrayIter;
class MutableArrayIter;
class ArrayElement;

struct FullPos;

class VariableSerializer;
class VariableUnserializer;
class FiberReferenceMap;

///////////////////////////////////////////////////////////////////////////////

/**
 * LiteralString is separated from non-literal ones (KindOfString), because
 * they deserve separate treatment for better optimizations. This means a lot
 * of functions may elect to take "litstr" separately from "String" class.
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
   * Do not rearrange the order, we have check such as m_type <= LiteralString.
   * The integer values for KindOfNull, KindOfBoolean, etc. were chosen
   * deliberately to make the GET_VARIANT_METHOD macro as fast as possible.
   */
  KindOfNull    = 0,
  KindOfBoolean = 1,
  KindOfByte    = 2,
  KindOfInt16   = 3,
  KindOfInt32   = 4,
  KindOfInt64   = 5,
  KindOfDouble  = 6,
  KindOfStaticString  = 7,
  LiteralString = 8,
  KindOfString  = 9,
  KindOfArray   = 10,
  KindOfObject  = 11,
  KindOfVariant = 12,

  MaxNumDataTypes = 13, // marker, not a valid type
};

inline int getDataTypeIndex(DataType t) {
  return t;
}

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
      signaled(false), surprised(false), debugger(false), interrupt(NULL) {
  }

  time_t started;      // when a request was started
  int timeoutSeconds;  // how many seconds to timeout

  bool memExceeded;    // memory limit was exceeded
  bool timedout;       // flag to set when timeout is detected
  bool signaled;       // flag to set when a signal was raised

  bool surprised;      // any surprise happened
  Mutex surpriseMutex; // mutex protecting per-request data

  bool debugger;       // whether there is a DebuggerProxy attached to me
  void *interrupt;     // current CmdInterrupt this thread's handling

  void onSessionInit();
};

class FrameInjection;
class ObjectAllocatorBase;
class Profiler;
class GlobalVariables;

// implemented in runtime/base/thread_info
class ThreadInfo {
public:
  static DECLARE_THREAD_LOCAL(ThreadInfo, s_threadInfo);

  std::vector<ObjectAllocatorBase *> m_allocators;
  FrameInjection *m_top;
  RequestInjectionData m_reqInjectionData;

  size_t m_stacksize;
  char *m_stacklimit;

  // This pointer is set by ProfilerFactory
  Profiler *m_profiler;

  GlobalVariables *m_globals;

  ThreadInfo();

  void onSessionInit();
};

extern void throw_infinite_recursion_exception();
class RecursionInjection {
public:
  // This is the amount of "slack" in stack usage checks - if the
  // stack pointer gets within this distance from the end (minus
  // overhead), throw an infinite recursion exception.
  static const int StackSlack = 1024 * 1024;

  RecursionInjection(ThreadInfo *info) : m_info(info) {
    char marker;
    if (&marker < m_info->m_stacklimit) {
      throw_infinite_recursion_exception();
    }
  }
  ~RecursionInjection() {
  }

private:
  ThreadInfo *m_info;
};

// implemented in runtime/base/timeout_thread
extern void throw_request_timeout_exception() ATTRIBUTE_COLD;
extern void throw_memory_exceeded_exception() ATTRIBUTE_COLD
                                              __attribute__((noreturn));
extern bool f_pcntl_signal_dispatch() ATTRIBUTE_COLD;

extern bool SegFaulting;
class RequestInjection {
public:
  RequestInjection(ThreadInfo *info) {
    if (SegFaulting) pauseAndExit();
    if (info->m_reqInjectionData.surprised) checkSurprise(info);
  }
private:
  void checkSurprise(ThreadInfo *info) ATTRIBUTE_COLD;
  void pauseAndExit() ATTRIBUTE_COLD __attribute__((noreturn));
};

// implemented in runtime/ext/ext_hotprofiler.cpp
class ProfilerInjection {
public:
  ProfilerInjection(ThreadInfo *info, const char *symbol);
  ~ProfilerInjection();
private:
  ThreadInfo *m_info;
};

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

struct MethodIndexHash {
  size_t operator()(MethodIndex mi) const { return (size_t) mi.val(); }
};

class MethodIndexMap : public hphp_const_char_imap<MethodIndex> {
  public:
  void initialize();
  typedef hphp_hash_map<const MethodIndex, const char *, MethodIndexHash >
    MethodIndexReverseMap;
  MethodIndexReverseMap methodIndexReverseMap;
  private:
  void addEntry(const char * methodName, MethodIndex mi) ;
};
extern MethodIndexMap methodIndexMap;

inline MethodIndex methodIndexExists(const char * methodName) {
  MethodIndexMap::const_iterator i = methodIndexMap.find(methodName);
  if (i == methodIndexMap.end()) return MethodIndex::fail();
  return (*i).second;
}

inline MethodIndex methodIndexLookup(const char * methodName) {
  MethodIndex ret = methodIndexExists(methodName);
  MethodIndexMap::const_iterator i = methodIndexMap.find(methodName);
  ASSERT(i != methodIndexMap.end()); // only for testing
  return (*i).second;
}

inline const char * methodIndexLookupReverse(MethodIndex methodIndex) {
  MethodIndexMap::MethodIndexReverseMap::const_iterator i =
     methodIndexMap.methodIndexReverseMap.find(methodIndex);
  ASSERT(i != methodIndexMap.methodIndexReverseMap.end());
  return (*i).second;
}

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_TYPES_H__
