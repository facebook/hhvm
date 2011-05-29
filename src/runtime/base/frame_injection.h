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
#ifndef __HPHP_FRAME_INJECTION_H__
#define __HPHP_FRAME_INJECTION_H__

#include <util/base.h>
#include <util/thread_local.h>
#include <runtime/base/types.h>
#include <runtime/base/complex_types.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class FrameInjection {
public:
  enum Flag {
    PseudoMain      =  1 << 0,
    BuiltinFunction =  1 << 1,
    BreakPointHit   =  1 << 2,
    EvalFrame       =  1 << 3,
    Function        =  1 << 4,
    StaticMethod    =  1 << 5,
    ObjectMethod    =  1 << 6,
    ParserFrame     =  1 << 7
 };

  static CStrRef GetClassName(bool skip = false);
  static CStrRef GetParentClassName(bool skip = false);
  static ObjectData *GetThis(bool skip = false);
  static String GetContainingFileName(bool skip = false);
  static Array GetBacktrace(bool skip = false, bool withSelf = false,
                            bool withThis = true);
  static Array GetCallerInfo(bool skip = false);
  static int GetLine(bool skip = false);

#ifdef ENABLE_LATE_STATIC_BINDING
  // what does "static::" resolve to?
  static CStrRef GetStaticClassName(ThreadInfo *info);
  static const String *SetStaticClassName(ThreadInfo *info, CStrRef cls) {
    ASSERT(info);
    FrameInjection *t = info->m_top;
    if (t) {
      const String *old = t->m_staticClass;
      t->m_staticClass = &cls;
      return old;
    }
    return NULL;
  }
  static void ResetStaticClassName(ThreadInfo *info) {
    ASSERT(info);
    FrameInjection *t = info->m_top;
    if (t) t->m_staticClass = NULL;
  }
#endif /* ENABLE_LATE_STATIC_BINDING */

  static bool IsGlobalScope();
  static bool IsGlobalScope(FrameInjection *frame);
  static FrameInjection *GetStackFrame(int level);

private:
  inline void initCommon() {
    ASSERT(m_name);
    ASSERT(m_info);
#ifdef INFINITE_RECURSION_DETECTION
    check_recursion(m_info);
#endif
    if (m_info->m_pendingException) {
      throw_pending_exception(m_info);
    }
    m_prev = m_info->m_top;
    m_info->m_top = this;
  }

protected:
  // constructors and destructor are supposed to be inlined by subclasses
  FrameInjection(const char *name, int fs)
    : m_name(name),
#ifdef ENABLE_LATE_STATIC_BINDING
      m_staticClass(NULL),
#endif
      m_line(0), m_flags(fs) {
    m_info = ThreadInfo::s_threadInfo.getNoCheck();
    initCommon();
    // NOTE: hot profiler needs to be called by subclasses
  }

  ~FrameInjection() {
    m_info->m_top = m_prev;
    // NOTE: hot profiler needs to be called by subclasses
  }

  void hotProfilerInit(const char *name) {
#ifdef HOTPROFILER
    Profiler *prof = m_info->m_profiler;
    if (prof) begin_profiler_frame(prof, name);
#endif
  }
  void hotProfilerFini() {
#ifdef HOTPROFILER
    Profiler *prof = m_info->m_profiler;
    if (prof) end_profiler_frame(prof);
#endif
  }

public:

  /**
   * Simple accessors
   */
  CStrRef getClassName() const;
  ObjectData *getObjectV() const;
  const char *getFunction() const { return m_name;}
  FrameInjection *getPrev() const { return m_prev;}
  int getFlags() const { return m_flags;}
  int getLine() const { return m_line;}
  void setLine(int line) { m_line = line;}
  void setBreakPointHit() { m_flags |= BreakPointHit;}
  ThreadInfo* getThreadInfo() const { return m_info;}

  bool isEvalFrame() const { return m_flags & EvalFrame; }
  bool isParserFrame() const { return m_flags & ParserFrame; }
  bool isFunctionFrame() const { return m_flags & Function; }
  bool isStaticMethodFrame() const { return m_flags & StaticMethod; }
  bool isObjectMethodFrame() const { return m_flags & ObjectMethod; }

#ifdef ENABLE_LATE_STATIC_BINDING
  void setStaticClassName(CStrRef cls) { m_staticClass = &cls; }
  void resetStaticClassName() { m_staticClass = NULL; }
#endif /* ENABLE_LATE_STATIC_BINDING */

  /**
   * Complex accessors. EvalFrameInjection overwrites these.
   */
  String getFileName();
  Array getArgs();

public:
#ifdef ENABLE_LATE_STATIC_BINDING
  class StaticClassNameHelper {
  public:
    StaticClassNameHelper(ThreadInfo *info, CStrRef cls) : m_info(info) {
      FrameInjection::SetStaticClassName(info, cls);
    }
    ~StaticClassNameHelper() {
      FrameInjection::ResetStaticClassName(m_info);
    }
  private:
    ThreadInfo *m_info;
  };
#endif /* ENABLE_LATE_STATIC_BINDING */

protected:
  ThreadInfo     *m_info;
  FrameInjection *m_prev;
  const char     *m_name;

#ifdef ENABLE_LATE_STATIC_BINDING
  const String   *m_staticClass;
#endif /* ENABLE_LATE_STATIC_BINDING */

  int m_line;
  int m_flags;

  inline bool checkClassName(CStrRef cls) {
    int len = cls.length();
    if (strncmp(cls.data(), m_name, len) != 0) return false;
    if (m_name[len] != ':' || m_name[len+1] != ':') return false;
    return true;
  }
};

class FrameInjectionFunction : public FrameInjection {
public:
  FrameInjectionFunction(const char *name, int fs)
    : FrameInjection(name, fs | Function){
    hotProfilerInit(name);
  }
  ~FrameInjectionFunction() {
    hotProfilerFini();
  }
  ObjectData *getThis() const;
  ObjectData *getThisForArrow();
};

class FIFunctionMem : public FrameInjectionFunction {
public:
  FIFunctionMem(const char *name);
  ~FIFunctionMem();
};

class FIFunctionNoMem : public FrameInjectionFunction {
public:
  FIFunctionNoMem(const char *name);
  ~FIFunctionNoMem();
};

class FIFunctionFS : public FrameInjectionFunction {
public:
  FIFunctionFS(const char *name, int fs);
  ~FIFunctionFS();
};

class FrameInjectionStaticMethod : public FrameInjection {
public:
  FrameInjectionStaticMethod(const char *name)
    : FrameInjection(name, StaticMethod) {
    hotProfilerInit(name);
  }
  ~FrameInjectionStaticMethod() {
    hotProfilerFini();
  }

  ObjectData *getThis() const { return NULL; }
  ObjectData *getThisForArrow() {
    throw FatalErrorException("Using $this when not in object context");
  }
};

class FIStaticMethodMem : public FrameInjectionStaticMethod {
public:
  FIStaticMethodMem(const char *name);
  ~FIStaticMethodMem();
};

class FIStaticMethodNoMem : public FrameInjectionStaticMethod {
public:
  FIStaticMethodNoMem(const char *name);
  ~FIStaticMethodNoMem();
};

class FrameInjectionObjectMethod : public FrameInjection {
public:
  FrameInjectionObjectMethod(const char *name, ObjectData *obj)
    : FrameInjection(name, ObjectMethod)   {
    ASSERT(obj);
    m_object = obj;
    obj->incRefCount();
    hotProfilerInit(name);
  }
  ~FrameInjectionObjectMethod() {
    if (m_object->decRefCount() == 0) {
      m_object->release();
    }
    hotProfilerFini();
  }

  /**
   * This function checks object ID to make sure it's not 0. If it's 0, it
   * returns a null object. Otherwise, it returns "this";
   */
  ObjectData *getThis() const;

  /**
   * This function checks object ID to make sure it's not 0. If it's 0, it
   * was a fake object created just for calling an instance method with a form
   * of ClassName::MethodName(). Then it will throw a "using this in non-
   * object context" fatal.
   */
  ObjectData *getThisForArrow();

private:
  ObjectData *m_object;
};

class FIObjectMethodMem : public FrameInjectionObjectMethod {
public:
  FIObjectMethodMem(const char *name, ObjectData *obj);
  ~FIObjectMethodMem();
};

class FIObjectMethodNoMem : public FrameInjectionObjectMethod {
public:
  FIObjectMethodNoMem(const char *name, ObjectData *obj);
  ~FIObjectMethodNoMem();
};

/* No profile version of Function, for call_user_func_array, etc. */
class FIFunctionNP : public FrameInjection {
public:
  FIFunctionNP(const char *name);
  ~FIFunctionNP();
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_FRAME_INJECTION_H__
