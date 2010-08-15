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
  static const char *GetParentClassName(bool skip = false);
  static Object GetThis(bool skip = false);
  static String GetContainingFileName(bool skip = false);
  static Array GetBacktrace(bool skip = false, bool withSelf = false,
                            bool withThis = true);
  static int GetLine(bool skip = false);

public:
  FrameInjection(ThreadInfo *info, CStrRef cls, const char *name,
                 ObjectData *obj = NULL, int fs = 0)
      : m_info(info), m_class(cls), m_name(name),
        m_object(obj ? obj->getRoot() : NULL),
        m_line(0), m_flags(fs) {
    ASSERT(m_class.get());
    ASSERT(m_name);
    m_prev = m_info->m_top;
    m_info->m_top = this;
  }

  virtual ~FrameInjection() {
    m_info->m_top = m_prev;
  }

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
  Object &getThis();

  /**
   * This function checks object ID to make sure it's not 0. If it's 0, it
   * was a fake object created just for calling an instance method with a form
   * of ClassName::MethodName(). Then it will throw a "using this in non-
   * object context" fatal.
   */
  Object &getThisForArrow();

public:
  // what does "static::" resolve to?
  static String GetStaticClassName(ThreadInfo *info);
  static void SetStaticClassName(ThreadInfo *info, CStrRef cls);
  static void ResetStaticClassName(ThreadInfo *info);
  static void SetCallingObject(ThreadInfo* info, ObjectData *obj);

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

private:
  ThreadInfo     *m_info;
  FrameInjection *m_prev;
  CStrRef         m_class;
  const char     *m_name;
  Object          m_object;

  int             m_line;
  int             m_flags;

  // for static late binding
  String          m_staticClass;
  Object          m_callingObject;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_FRAME_INJECTION_H__
