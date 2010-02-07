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

/**
 * Zend implementation uses HashTable for arrays ubiquitously. That just means
 * it will use a lot more memory than needed sometimes, esp. with vectors. Here
 * we use 7 specialized array types to store differently shaped arrays. Right
 * now we rely on an "escalation" process at run-time to adjust itself to use
 * a type without losing semantics. In the future, it's not that crazy to
 * provide type hints on what kind of array a code write intends to build.
 */
class EmptyArray;
class Vector;
class VectorLong;
class VectorString;
class VectorVariant;
class Map;
class MapLong;
class MapString;
class MapVariant;

class VariableSerializer;
class VariableUnserializer;

///////////////////////////////////////////////////////////////////////////////

/**
 * LiteralString is separated from non-literal ones (KindOfString), because
 * they deserve separate treatment for better optimizations. This means a lot
 * of functions may elect to take "litstr " separately from "String"
 * class. This code specialization helps speed a lot by not instantiating a
 * String object to box an otherwise literal value. This also means, though not
 * obviously thus dangerous not to know, whenever a function takes a parameter
 * with type of "litstr ", one can only pass in a literal string that has
 * a "permanent" memory address to be stored. To make this really clear, I
 * invented "litstr" as a typedef-ed name for "litstr " that expects a
 * literal string only. Therefore, throughout this entire runtime library,
 *
 *   litstr == literal string
 *   litstr  == any C-string pointer
 *
 */
enum DataType {
  // do not rearrange the order, we have check such as m_type <= LiteralString
  KindOfNull,
  KindOfBoolean,
  KindOfByte,
  KindOfInt16,
  KindOfInt32,
  KindOfInt64,
  KindOfDouble,
  LiteralString,
  KindOfString,
  KindOfArray,
  KindOfObject,
  KindOfVariant,

  KindOfLast, // marker, not a valid type
};

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
    : started(0), timeoutSeconds(-1), timedout(false), signaled(false) {}

  time_t started;     // when a request was started
  int timeoutSeconds; // how many seconds to timeout
  bool timedout;      // flag to set when timeout is detected
  bool signaled;      // flag to set when a signal was raised
};

class FrameInjection;
class ObjectAllocatorBase;
class Profiler;

// implemented in cpp/base/thread_info
class ThreadInfo {
public:
  static DECLARE_THREAD_LOCAL(ThreadInfo, s_threadInfo);

  std::vector<ObjectAllocatorBase *> m_allocators;
  FrameInjection *m_top;
  RequestInjectionData m_reqInjectionData;

  // This integer is reset during every hphp_session_init()
  int m_stackdepth;

  // This pointer is set by ProfilerFactory
  Profiler *m_profiler;

  ThreadInfo();
};

extern void throw_infinite_recursion_exception();
class RecursionInjection {
public:
  RecursionInjection(ThreadInfo *info) : m_info(info) {
    if (++m_info->m_stackdepth > 1000) throw_infinite_recursion_exception();
  }
  ~RecursionInjection() {
    --m_info->m_stackdepth;
  }

private:
  ThreadInfo *m_info;
};

// implemented in cpp/base/timeout_thread
extern void throw_request_timeout_exception(ThreadInfo *info);
extern bool f_pcntl_signal_dispatch();

class RequestInjection {
public:
  RequestInjection(ThreadInfo *info) {
    RequestInjectionData &p = info->m_reqInjectionData;
    if (p.timedout) throw_request_timeout_exception(info);
    if (p.signaled) f_pcntl_signal_dispatch();
  }
};

// implemented in cpp/ext/ext_hotprofiler.cpp
class ProfilerInjection {
public:
  ProfilerInjection(ThreadInfo *info, const char *symbol);
  ~ProfilerInjection();
private:
  ThreadInfo *m_info;
};

// definitions for various numeric limits
// implemented in cpp/base/builtin_functions.cpp
class Limits {
public:
  static double inf_double;
  static double nan_double;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_TYPES_H__
