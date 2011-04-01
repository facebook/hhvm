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
    PseudoMain      = 1,
    BuiltinFunction = 2,
    BreakPointHit   = 4
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
  static void SetCallingObject(ThreadInfo* info, ObjectData *obj) {
    ASSERT(info);
    FrameInjection *t = info->m_top;
    if (t) t->m_callingObject = obj;
  }

  void setStaticClassName(CStrRef cls) { m_staticClass = &cls; }
  void resetStaticClassName() { m_staticClass = NULL; }
  void setCallingObject(ObjectData *obj) { m_callingObject = obj; }
#endif /* ENABLE_LATE_STATIC_BINDING */

  static bool IsGlobalScope();
  static bool IsGlobalScope(FrameInjection *frame);
  static FrameInjection *GetStackFrame(int level);

public:
  // NOTE: obj has to be the root object
  // constructors with hot profiler
  FrameInjection(ThreadInfo *&info, CStrRef cls, const char *name);
  FrameInjection(ThreadInfo *&info, CStrRef cls, const char *name,
                 ObjectData *obj);
  FrameInjection(ThreadInfo *&info, CStrRef cls, const char *name, int fs);
  FrameInjection(ThreadInfo *&info, CStrRef cls, const char *name,
                 ObjectData *obj, int fs);

  // constructors without hot profiler
  FrameInjection(ThreadInfo *&info, CStrRef cls, const char *name,
                 bool unused);
  FrameInjection(ThreadInfo *&info, CStrRef cls, const char *name,
                 ObjectData *obj, bool unused);
  FrameInjection(ThreadInfo *&info, CStrRef cls, const char *name, int fs,
                 bool unused);
  FrameInjection(ThreadInfo *&info, CStrRef cls, const char *name,
                 ObjectData *obj, int fs, bool unused);

  virtual ~FrameInjection();

  /**
   * Simple accessors
   */
  const char *getClass() const { return m_class;}
  const char *getFunction() const { return m_name;}
  FrameInjection *getPrev() const { return m_prev;}
  int getFlags() const { return m_flags;}
  int getLine() const { return m_line;}
  void setLine(int line) { m_line = line;}
  void setBreakPointHit() { m_flags |= BreakPointHit;}

  /**
   * Complex accessors. EvalFrameInjection overwrites these.
   */
  virtual String getFileName();
  virtual Array getArgs();

  /**
   * This function checks object ID to make sure it's not 0. If it's 0, it
   * returns a null object. Otherwise, it returns "this";
   */
  ObjectData *getThis();

  /**
   * This function checks object ID to make sure it's not 0. If it's 0, it
   * was a fake object created just for calling an instance method with a form
   * of ClassName::MethodName(). Then it will throw a "using this in non-
   * object context" fatal.
   */
  ObjectData *getThisForArrow();

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
private:
  FrameInjection *m_prev;
  CStrRef         m_class;
  const char     *m_name;
  Object          m_object;
#ifdef HOTPROFILER
  bool            m_prof;
#endif

#ifdef ENABLE_LATE_STATIC_BINDING
  // for static late binding
  const String   *m_staticClass;
  ObjectData     *m_callingObject;
#endif /* ENABLE_LATE_STATIC_BINDING */

  int             m_line;
  int             m_flags;

  inline void doCommon() {
    ASSERT(m_class.get());
    ASSERT(m_name);
    m_prev = m_info->m_top;
    m_info->m_top = this;
  }
  inline void hotProfilerInit(ThreadInfo *info, const char *name) {
#ifdef HOTPROFILER
    m_prof = true;
    Profiler *prof = m_info->m_profiler;
    if (prof) begin_profiler_frame(prof, name);
#endif
  }

};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_FRAME_INJECTION_H__
