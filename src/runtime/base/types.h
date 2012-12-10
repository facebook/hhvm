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

#ifndef __HPHP_TYPES_H__
#define __HPHP_TYPES_H__

#include <util/base.h>
#include <util/thread_local.h>
#include <util/mutex.h>
#include <util/case_insensitive.h>
#include <vector>
#include <runtime/base/macros.h>
#include <runtime/base/memory/memory_manager.h>

#include <boost/static_assert.hpp>

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
class VarNR;
class RefData;
typedef Variant Numeric;
typedef Variant Primitive;
typedef Variant PlusOperand;
typedef Variant Sequence;

/**
 * Macros related to Variant that are needed by StringData, ObjectData,
 * and ArrayData.
 */
extern const Variant null_variant;
extern const Variant init_null_variant;
extern const VarNR null_varNR;
extern const VarNR true_varNR;
extern const VarNR false_varNR;
extern const VarNR INF_varNR;
extern const VarNR NEGINF_varNR;
extern const VarNR NAN_varNR;
extern const String null_string;
extern const Array null_array;

/*
 * All TypedValue-compatible types have their reference count field at
 * the same offset in the object.
 *
 * This offset assumes there will be no padding after the initial
 * pointer member in some of these types, and that the object/array
 * vtable is implemented with a single pointer at the front of the
 * object.  All this should be true pretty much anywhere you might
 * want to use hphp (if it's not, you'll hit compile-time assertions
 * in the relevant classes and may have to fiddle with this).
 */
const size_t FAST_REFCOUNT_OFFSET = sizeof(void*);

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
  MinDataType            = -13,

  KindOfClass            = -13,

  // Values below zero are not PHP values, but runtime-internal.
  KindOfAny              = -8,
  KindOfUncounted        = -7,
  KindOfUncountedInit    = -6,

  KindOfInvalid          = -1,
  KindOfUnknown          = KindOfInvalid,

  /**
   * Beware if you change the order, as we may have a few type checks
   * in the code that depend on the order.  Also beware of adding to
   * the number of bits needed to represent this.  (Known dependency
   * in unwind-x64.h.)
   */
  KindOfUninit           = 0,

  // Note: KindOfStringBit must be set in KindOfStaticString and KindOfString,
  //       and it must be 0 in any other real DataType.
  KindOfStringBit        = 4,

  // Note: KindOfUncountedInitBit must be set for Null, Boolean, Int64, Double,
  //       and StaticString, and it must be 0 for any other real DataType.
  KindOfUncountedInitBit = 8,

  KindOfNull             = 8,     //   0001000
  KindOfBoolean          = 9,     //   0001001
  KindOfInt64            = 10,    //   0001010
  KindOfDouble           = 11,    //   0001011

  KindOfStaticString     = 12,    //   0001100
  KindOfString           = 20,    //   0010100
  KindOfArray            = 32,    //   0100000
  KindOfObject           = 64,    //   1000000
  KindOfRef              = 96,    //   1100000
  KindOfIndirect         = 97,    //   1100001

  MaxNumDataTypes        = KindOfIndirect + 1, // marker, not a valid type
  MaxNumDataTypesIndex   = 11 + 1,  // 1 + the number of valid DataTypes above

  MaxDataType            = 0x7fffffff // Allow KindOf* > 11 in HphpArray.
};
BOOST_STATIC_ASSERT((sizeof(DataType) == 4));


BOOST_STATIC_ASSERT(KindOfString       & KindOfStringBit);
BOOST_STATIC_ASSERT(KindOfStaticString & KindOfStringBit);
BOOST_STATIC_ASSERT(!(KindOfUninit     & KindOfStringBit));
BOOST_STATIC_ASSERT(!(KindOfNull       & KindOfStringBit));
BOOST_STATIC_ASSERT(!(KindOfBoolean    & KindOfStringBit));
BOOST_STATIC_ASSERT(!(KindOfInt64      & KindOfStringBit));
BOOST_STATIC_ASSERT(!(KindOfDouble     & KindOfStringBit));
BOOST_STATIC_ASSERT(!(KindOfArray      & KindOfStringBit));
BOOST_STATIC_ASSERT(!(KindOfObject     & KindOfStringBit));
BOOST_STATIC_ASSERT(!(KindOfRef        & KindOfStringBit));
BOOST_STATIC_ASSERT(!(KindOfIndirect   & KindOfStringBit));
BOOST_STATIC_ASSERT(!(KindOfClass      & KindOfStringBit));

BOOST_STATIC_ASSERT(KindOfNull         & KindOfUncountedInitBit);
BOOST_STATIC_ASSERT(KindOfBoolean      & KindOfUncountedInitBit);
BOOST_STATIC_ASSERT(KindOfInt64        & KindOfUncountedInitBit);
BOOST_STATIC_ASSERT(KindOfDouble       & KindOfUncountedInitBit);
BOOST_STATIC_ASSERT(KindOfStaticString & KindOfUncountedInitBit);
BOOST_STATIC_ASSERT(!(KindOfUninit     & KindOfUncountedInitBit));
BOOST_STATIC_ASSERT(!(KindOfString     & KindOfUncountedInitBit));
BOOST_STATIC_ASSERT(!(KindOfArray      & KindOfUncountedInitBit));
BOOST_STATIC_ASSERT(!(KindOfObject     & KindOfUncountedInitBit));
BOOST_STATIC_ASSERT(!(KindOfRef        & KindOfUncountedInitBit));
BOOST_STATIC_ASSERT(!(KindOfIndirect   & KindOfUncountedInitBit));
BOOST_STATIC_ASSERT(!(KindOfClass      & KindOfUncountedInitBit));

const unsigned int kDataTypeMask = 0x7F;

BOOST_STATIC_ASSERT(MaxNumDataTypes - 1 <= kDataTypeMask);

// All DataTypes greater than this value are refcounted.
const DataType KindOfRefCountThreshold = KindOfStaticString;

enum DataTypeCategory {
  DataTypeGeneric,
  DataTypeCountness,
  DataTypeCountnessInit,
  DataTypeSpecific
};

std::string tname(DataType t);

inline int getDataTypeIndex(DataType type) {
  switch (type) {
    case KindOfUninit       : return 0;
    case KindOfNull         : return 1;
    case KindOfBoolean      : return 2;
    case KindOfInt64        : return 3;
    case KindOfDouble       : return 4;
    case KindOfStaticString : return 5;
    case KindOfString       : return 6;
    case KindOfArray        : return 7;
    case KindOfObject       : return 8;
    case KindOfRef          : return 9;
    case KindOfIndirect     : return 10;
    default                 : not_reached();
  }
}

inline DataType getDataTypeValue(unsigned index) {
  switch (index) {
    case 0  : return KindOfUninit;
    case 1  : return KindOfNull;
    case 2  : return KindOfBoolean;
    case 3  : return KindOfInt64;
    case 4  : return KindOfDouble;
    case 5  : return KindOfStaticString;
    case 6  : return KindOfString;
    case 7  : return KindOfArray;
    case 8  : return KindOfObject;
    case 9  : return KindOfRef;
    case 10 : return KindOfIndirect;
    default : not_reached();
  }
}

// These are used in type_variant.cpp and translator-x64.cpp
const unsigned int kShiftDataTypeToDestrIndex = 5;
const unsigned int kDestrTableSize = 4;

#define TYPE_TO_DESTR_IDX(t) ((t) >> kShiftDataTypeToDestrIndex)

static inline ALWAYS_INLINE unsigned typeToDestrIndex(DataType t) {
  ASSERT(t >= KindOfString && t <= KindOfRef);
  return TYPE_TO_DESTR_IDX(t);
}

// Helper macro for checking if a given type is refcounted
#define IS_REFCOUNTED_TYPE(t) ((t) > KindOfRefCountThreshold)
// Helper macro for checking if a type is KindOfString or KindOfStaticString.
BOOST_STATIC_ASSERT(KindOfStaticString == 0x0C);
BOOST_STATIC_ASSERT(KindOfString       == 0x14);
#define IS_STRING_TYPE(t) (((t) & ~0x18) == KindOfStringBit)
// Check if a type is KindOfUninit or KindOfNull
#define IS_NULL_TYPE(t) (unsigned(t) <= KindOfNull)
// Other type check macros
#define IS_INT_TYPE(t) ((t) == KindOfInt64)
#define IS_ARRAY_TYPE(t) ((t) == KindOfArray)
#define IS_BOOL_TYPE(t) ((t) == KindOfBoolean)
#define IS_DOUBLE_TYPE(t) ((t) == KindOfDouble)

#define IS_REAL_TYPE(t) \
  (((t) >= KindOfUninit && (t) < MaxNumDataTypes) || (t) == KindOfClass)

namespace Collection {
enum Type {
  InvalidType = 0,
  VectorType = 1,
  MapType = 2,
  StableMapType = 3,
  MaxNumTypes = 4
};
}

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

namespace VM {
  class Class;
}

///////////////////////////////////////////////////////////////////////////////
// code injection classes

class RequestInjectionData {
public:
  static const ssize_t MemExceededFlag = 1 << 0;
  static const ssize_t TimedOutFlag    = 1 << 1;
  static const ssize_t SignaledFlag    = 1 << 2;
  static const ssize_t EventHookFlag   = 1 << 3;
  static const ssize_t LastFlag        = EventHookFlag;

  RequestInjectionData()
    : conditionFlags(0), surprisePage(NULL), started(0), timeoutSeconds(-1),
      debugger(false), debuggerIdle(0), dummySandbox(false),
      debuggerIntr(false), coverage(false) {
  }

  inline volatile ssize_t* getConditionFlags() {
    if (hhvm) {
      ASSERT(cflagsPtr);
      return cflagsPtr;
    } else {
      return &conditionFlags;
    }
  }

  union {
    volatile ssize_t conditionFlags; // condition flags can indicate if a
                                     // thread has exceeded the memory limit,
                                     // timed out, or received a signal
    ssize_t* cflagsPtr;              // under hhvm, this will point to the real
                                     // condition flags, somewhere in the
                                     // thread's targetcache
  };
  void *surprisePage;              // beginning address of page to
                                   // protect for error conditions
  Mutex surpriseLock;              // mutex controlling access to surprisePage

  time_t started;      // when a request was started
  int timeoutSeconds;  // how many seconds to timeout

  bool debugger;       // whether there is a DebuggerProxy attached to me
  int  debuggerIdle;   // skipping this many interrupts while proxy is idle
  bool dummySandbox;   // indicating it is from a dummy sandbox thread
  bool debuggerIntr;   // indicating we should force interrupt for debugger
  std::stack<void *> interrupts;   // CmdInterrupts this thread's handling
  bool coverage;       // is coverage being collected

  void reset();

  void setMemExceededFlag();
  void setTimedOutFlag();
  void setSignaledFlag();
  void setEventHookFlag();
  void clearEventHookFlag();
  ssize_t fetchAndClearFlags();

  void onSessionInit();
};

class FrameInjection;
class ObjectAllocatorBase;
class Profiler;
class GlobalVariables;
class CodeCoverage;

int object_alloc_size_to_index(size_t);
size_t object_alloc_index_to_size(int);

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

  std::vector<ObjectAllocatorBase *> m_allocators;
  FrameInjection *m_top;
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
  ObjectAllocatorBase* instanceSizeAllocator(size_t size) {
    const_assert(hhvm);
    int index = object_alloc_size_to_index(size);
    ASSERT_NOT_IMPLEMENTED(index != -1);
    return m_allocators[index];
  }

  ObjectAllocatorBase* instanceIdxAllocator(int index) {
    const_assert(hhvm);
    return m_allocators[index];
  }

  static bool valid(ThreadInfo* info);
};

extern void throw_infinite_recursion_exception();
extern void throw_call_non_object() ATTRIBUTE_COLD ATTRIBUTE_NORETURN;

inline void* stack_top_ptr() { // x64 specific
  register void* rsp asm("rsp");
  return rsp;
}

inline bool stack_in_bounds(ThreadInfo *&info) {
  return stack_top_ptr() >= info->m_stacklimit;
}

inline bool is_stack_ptr(void* p) {
  return p > stack_top_ptr() && ThreadInfo::t_stackbase >= p;
}

// The ThreadInfo pointer itself must be from the current stack frame.
inline void check_recursion(ThreadInfo *&info) {
  if (!stack_in_bounds(info)) {
    throw_infinite_recursion_exception();
  }
}

// implemented in runtime/base/builtin_functions.cpp
extern void pause_forever() ATTRIBUTE_COLD ATTRIBUTE_NORETURN;
extern void check_request_surprise(ThreadInfo *info) ATTRIBUTE_COLD;

extern bool SegFaulting;

inline void check_request_timeout(ThreadInfo *info) {
  const_assert(!hhvm);
  if (SegFaulting) pause_forever();
  info->m_mm->refreshStats();
  if (info->m_reqInjectionData.conditionFlags) check_request_surprise(info);
}

inline void check_request_timeout_nomemcheck(ThreadInfo *info) {
  const_assert(!hhvm);
  if (SegFaulting) pause_forever();
  if (info->m_reqInjectionData.conditionFlags) check_request_surprise(info);
}

void throw_pending_exception(ThreadInfo *info) ATTRIBUTE_COLD
  ATTRIBUTE_NORETURN;

void check_request_timeout_info(ThreadInfo *info, int lc);
void check_request_timeout_ex(int lc);

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

class CallInfo;
class MethodCallPackage;

class AccessFlags {
public:
  enum Type {
    None = 0,

    Error = 1,
    CheckExist = 2,
    Key = 4,
    NoHipHop = 8,

    Error_Key = Error | Key,
    CheckExist_Key = CheckExist | Key,
    Error_NoHipHop = Error | NoHipHop,
  };
  static Type IsKey(bool s) { return s ? Key : None; }
  static Type IsError(bool e) { return e ? Error : None; }
};

#define ACCESSPARAMS_DECL AccessFlags::Type flags = AccessFlags::None
#define ACCESSPARAMS_IMPL AccessFlags::Type flags

namespace HphpBinary {
  enum Type {
    hphpc,
    hphpi,
    hhvm,
    program,
    test
  };
}

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_TYPES_H__
