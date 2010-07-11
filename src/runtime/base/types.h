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
#if defined(__GNUC__)
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
typedef char int8;
typedef int int32;
typedef long long int64;
typedef unsigned long long uint64;
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
    : started(0), timeoutSeconds(-1), timedout(false), signaled(false),
      memExceeded(false) {}

  time_t started;     // when a request was started
  int timeoutSeconds; // how many seconds to timeout
  bool timedout;      // flag to set when timeout is detected
  bool signaled;      // flag to set when a signal was raised
  bool memExceeded;   // memory limit was exceeded
};

class FrameInjection;
class ObjectAllocatorBase;
class Profiler;

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

  ThreadInfo();
  void reset();
};

extern void throw_infinite_recursion_exception();
class RecursionInjection {
public:
  // This is the amount of "slack" in stack usage checks - if the
  // stack pointer gets within this distance from the end (minus
  // overhead), throw an infinite recursion exception.
  static const int StackSlack = 32768;

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
extern void throw_request_timeout_exception() __attribute__((cold));
extern void throw_memory_exceeded_exception() __attribute__((cold, noreturn));
extern bool f_pcntl_signal_dispatch() __attribute__((cold));

extern bool SegFaulting;
class RequestInjection {
public:
  RequestInjection(ThreadInfo *info) {
    RequestInjectionData &p = info->m_reqInjectionData;
    if (SegFaulting) pauseAndExit();
    if (p.timedout) throw_request_timeout_exception();
    if (p.memExceeded) throw_memory_exceeded_exception();
    if (p.signaled) f_pcntl_signal_dispatch();
  }
private:
  void pauseAndExit() __attribute__((cold, noreturn));
};

// implemented in runtime/ext/ext_hotprofiler.cpp
class ProfilerInjection {
public:
  ProfilerInjection(ThreadInfo *info, const char *symbol);
  ~ProfilerInjection();
private:
  ThreadInfo *m_info;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_TYPES_H__
