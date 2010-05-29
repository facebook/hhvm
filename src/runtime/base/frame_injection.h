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
    PseudoMain = 1,
    BuiltinFunction = 2
  };
  FrameInjection(ThreadInfo *info,
                 const char *cls, const char *name, ObjectData *obj = NULL,
                 int fs = 0) : line(0), flags(fs), m_info(info),
                 m_class(cls), m_name(name),
                 m_object(obj ? obj->getRoot() : NULL) {
    ASSERT(m_class);
    ASSERT(m_name);
    m_prev = m_info->m_top;
    m_info->m_top = this;
  }
  virtual ~FrameInjection() {
    m_info->m_top = m_prev;
  }

  static const char *GetClassName(bool skip = false);
  static const char *GetParentClassName(bool skip = false);
  static Object GetThis(bool skip = false);
  static String GetContainingFileName(bool skip = false);
  static Array GetBacktrace(bool skip = false, bool withSelf = false);

  virtual String getFileName();

  int line;
  int flags;

  virtual Array getArgs();
  CObjRef getObjectRef() { return m_object; }

public:
  // what does "static::" resolve to?
  static String GetStaticClassName(ThreadInfo *info);
  static void SetStaticClassName(ThreadInfo *info, CStrRef cls);
  static void ResetStaticClassName(ThreadInfo *info);
  static void SetCallingObject(ThreadInfo* info, ObjectData *obj);

  class StaticClassNameHelper {
  public:
    StaticClassNameHelper(ThreadInfo *info) : m_info(info) {}
    ~StaticClassNameHelper() {
      FrameInjection::ResetStaticClassName(m_info);
    }

    int64   call(int64   ret) { return ret;}
    String  call(CStrRef ret) { return ret;}
    Array   call(CArrRef ret) { return ret;}
    Object  call(CObjRef ret) { return ret;}
    Variant call(CVarRef ret) { return ref(ret);}

  private:
    ThreadInfo *m_info;
  };


private:
  ThreadInfo *m_info;
  FrameInjection *m_prev;
  const char *m_class;
  const char *m_name;
  Object      m_object;

  // for static late binding
  String      m_staticClass;
  Object      m_callingObject;
};

/**
 * For setting line numbers, so to make gcc happy.
 */
inline void set_ln(int &ln, int v) { ln = v; }

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_FRAME_INJECTION_H__
