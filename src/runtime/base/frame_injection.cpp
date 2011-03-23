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
#include <runtime/base/types.h>
#include <runtime/base/type_conversions.h>
#include <runtime/base/builtin_functions.h>
#include <runtime/base/source_info.h>
#include <runtime/base/class_info.h>
#include <runtime/base/frame_injection.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// static strings

static StaticString s_file("file");
static StaticString s_line("line");
static StaticString s_function("function");
static StaticString s_args("args");
static StaticString s_class("class");
static StaticString s_object("object");
static StaticString s_type("type");

///////////////////////////////////////////////////////////////////////////////


static inline void injection_check(ThreadInfo *&info) {
#ifdef INFINITE_RECURSION_DETECTION
  check_recursion(info);
#endif
  if (info->m_pendingException) {
    throw_pending_exception(info);
  }
}

// constructors with hot profiler
FrameInjection::FrameInjection(CStrRef cls, const char *name)
    : m_class(cls), m_name(name), m_object(NULL),
#ifdef ENABLE_LATE_STATIC_BINDING
      m_staticClass(NULL), m_callingObject(NULL),
#endif /* ENABLE_LATE_STATIC_BINDING */
      m_line(0), m_flags(0) {
  m_info = ThreadInfo::s_threadInfo.getNoCheck();
  injection_check(m_info);
  doCommon();
  hotProfilerInit(m_info, name);
}

FrameInjection::FrameInjection(CStrRef cls, const char *name, ObjectData *obj)
    : m_class(cls), m_name(name), m_object(obj),
#ifdef ENABLE_LATE_STATIC_BINDING
      m_staticClass(NULL), m_callingObject(NULL),
#endif
      m_line(0), m_flags(0) {
  m_info = ThreadInfo::s_threadInfo.getNoCheck();
  injection_check(m_info);
  doCommon();
  hotProfilerInit(m_info, name);
}

FrameInjection::FrameInjection(CStrRef cls, const char *name, int fs)
    : m_class(cls), m_name(name), m_object(NULL),
#ifdef ENABLE_LATE_STATIC_BINDING
      m_staticClass(NULL), m_callingObject(NULL),
#endif
      m_line(0), m_flags(fs) {
  m_info = ThreadInfo::s_threadInfo.getNoCheck();
  injection_check(m_info);
  doCommon();
  hotProfilerInit(m_info, name);
}

FrameInjection::FrameInjection(CStrRef cls,
                               const char *name, ObjectData *obj, int fs)
    : m_class(cls), m_name(name), m_object(obj),
#ifdef ENABLE_LATE_STATIC_BINDING
      m_staticClass(NULL), m_callingObject(NULL),
#endif
      m_line(0), m_flags(fs) {
  m_info = ThreadInfo::s_threadInfo.getNoCheck();
  injection_check(m_info);
  doCommon();
  hotProfilerInit(m_info, name);
}

// constructors without hot profiler
FrameInjection::FrameInjection(CStrRef cls, const char *name, bool unused)
    : m_class(cls), m_name(name), m_object(NULL),
#ifdef ENABLE_LATE_STATIC_BINDING
      m_staticClass(NULL), m_callingObject(NULL),
#endif
      m_line(0), m_flags(0) {
  m_info = ThreadInfo::s_threadInfo.getNoCheck();
  injection_check(m_info);
  doCommon();
#ifdef HOTPROFILER
  m_prof = false;
#endif
}

FrameInjection::FrameInjection(CStrRef cls,
                               const char *name, ObjectData *obj, bool unused)
    : m_class(cls), m_name(name), m_object(obj),
#ifdef ENABLE_LATE_STATIC_BINDING
      m_staticClass(NULL), m_callingObject(NULL),
#endif
      m_line(0), m_flags(0) {
  m_info = ThreadInfo::s_threadInfo.getNoCheck();
  injection_check(m_info);
  doCommon();
#ifdef HOTPROFILER
  m_prof = false;
#endif
}

FrameInjection::FrameInjection(CStrRef cls,
                               const char *name, int fs, bool unused)
    : m_class(cls), m_name(name), m_object(NULL),
#ifdef ENABLE_LATE_STATIC_BINDING
      m_staticClass(NULL), m_callingObject(NULL),
#endif
      m_line(0), m_flags(fs) {
  m_info = ThreadInfo::s_threadInfo.getNoCheck();
  injection_check(m_info);
  doCommon();
#ifdef HOTPROFILER
  m_prof = false;
#endif
}

FrameInjection::FrameInjection(CStrRef cls, const char *name, ObjectData *obj,
                               int fs, bool unused)
    : m_class(cls), m_name(name), m_object(obj),
#ifdef ENABLE_LATE_STATIC_BINDING
      m_staticClass(NULL), m_callingObject(NULL),
#endif
      m_line(0), m_flags(fs) {
  m_info = ThreadInfo::s_threadInfo.getNoCheck();
  injection_check(m_info);
  doCommon();
#ifdef HOTPROFILER
  m_prof = false;
#endif
}

FrameInjection::~FrameInjection() {
#ifdef REQUEST_TIMEOUT_DETECTION
  check_request_timeout(m_info);
#endif

  m_info->m_top = m_prev;
#ifdef HOTPROFILER
  if (m_prof) {
    Profiler *prof = m_info->m_profiler;
    if (prof) end_profiler_frame(prof);
  }
#endif
}

CStrRef FrameInjection::GetClassName(bool skip /* = false */) {
  FrameInjection *t = ThreadInfo::s_threadInfo->m_top;
  if (t && skip) {
    t = t->m_prev;
  }
  // If we have included a file inside a class method or called a builtin
  // function, we should walk up to find that class
  if (t) {
    while (t->m_prev && t->m_class.empty() &&
           t->m_flags & (PseudoMain | BuiltinFunction)) {
      t = t->m_prev;
    }
  }
  if (t && !t->m_class.empty()) {
    return t->m_class;
  }
  return empty_string;
}

CStrRef FrameInjection::GetParentClassName(bool skip /* = false */) {
  CStrRef cls = GetClassName(skip);
  if (cls.empty()) return cls;
  const ClassInfo *classInfo = ClassInfo::FindClass(cls);
  if (classInfo) {
    CStrRef parentClass = classInfo->getParentClass();
    if (!parentClass.isNull()) {
      return parentClass;
    }
  }
  return empty_string;
}

ObjectData *FrameInjection::GetThis(bool skip /* = false */) {
  FrameInjection *t = ThreadInfo::s_threadInfo->m_top;
  if (t && skip) {
    t = t->m_prev;
  }
  if (t) {
    return t->m_object.get();
  }
  return NULL;
}

String FrameInjection::GetContainingFileName(bool skip /* = false */) {
  FrameInjection *t = ThreadInfo::s_threadInfo->m_top;
  if (t && skip) {
    t = t->m_prev;
  }
  if (t) {
    return t->getFileName();
  }
  return "";
}

Array FrameInjection::GetBacktrace(bool skip /* = false */,
                                   bool withSelf /* = false */,
                                   bool withThis /* = true */) {
  Array bt = Array::Create();
  FrameInjection *t = ThreadInfo::s_threadInfo->m_top;
  if (skip && t) {
    t = t->m_prev;
  }
  // This is used by onError with extended exceptions
  if (withSelf && t) {
    String filename = t->getFileName();
    // If the top frame is not an extension function,
    // add it to the trace
    if (filename != "") {
      Array frame = Array::Create();
      frame.set(s_file, filename, true);
      frame.set(s_line, t->m_line, true);
      bt.append(frame);
    }
  }
  while (t && (RuntimeOption::InjectedStackTraceLimit < 0
            || bt.size() < RuntimeOption::InjectedStackTraceLimit)) {
    Array frame = Array::Create();

    if (t->m_prev) {
      String file = t->m_prev->getFileName();
      if (!file.empty() && t->m_prev->m_line) {
        frame.set(s_file, file, true);
        frame.set(s_line, t->m_prev->m_line, true);
      }
    } else if (t->m_flags & PseudoMain) {
      // Stop at top, don't include top file
      break;
    }

    if (t->m_flags & PseudoMain) {
      frame.set(s_function, "include", true);
      frame.set(s_args, Array::Create(t->getFileName()), true);
    } else {
      const char *c = strstr(t->m_name, "::");
      if (c) {
        frame.set(s_function, String(c + 2), true);
        frame.set(s_class, t->m_class->copy(), true);
        if (!t->m_object.isNull()) {
          if (withThis) {
            frame.set(s_object, t->m_object, true);
          }
          frame.set(s_type, "->", true);
        } else {
          frame.set(s_type, "::", true);
        }
      } else {
        frame.set(s_function, t->m_name, true);
      }

      Array args = t->getArgs();
      if (!args.isNull()) {
        frame.set(s_args, args, true);
      } else {
        frame.set(s_args, Array::Create(), true);
      }
    }

    bt.append(frame);
    t = t->m_prev;
  }
  return bt;
}

Array FrameInjection::GetCallerInfo(bool skip /* = false */) {
  FrameInjection *t = ThreadInfo::s_threadInfo->m_top;
  if (skip && t) {
    t = t->m_prev;
  }
  while (t) {
    if (strcasecmp(t->m_name, "call_user_func") == 0 ||
        strcasecmp(t->m_name, "call_user_func_array") == 0) {
      t = t->m_prev;
    } else {
      break;
    }
  }
  while (t && t->m_prev) {
    String file = t->m_prev->getFileName();
    if (!file.empty() && t->m_prev->m_line) {
      Array result = Array::Create();
      result.set(s_file, file, true);
      result.set(s_line, t->m_prev->m_line, true);
      return result;
    }
    t = t->m_prev;
    if (t) {
      if (strcasecmp(t->m_name, "call_user_func") == 0 ||
          strcasecmp(t->m_name, "call_user_func_array") == 0) {
        continue;
      }
    }
    break;
  }
  return Array::Create();
}

int FrameInjection::GetLine(bool skip /* = false */) {
  FrameInjection *t = ThreadInfo::s_threadInfo->m_top;
  if (t && skip) {
    t = t->m_prev;
  }
  if (t) {
    return t->m_line;
  }
  return -1;
}

bool FrameInjection::IsGlobalScope() {
  return IsGlobalScope(ThreadInfo::s_threadInfo->m_top);
}

bool FrameInjection::IsGlobalScope(FrameInjection *frame) {
  while (frame) {
    if ((frame->m_flags & PseudoMain) == 0) {
      return false;
    }
    frame = frame->getPrev();
  }
  return true;
}

FrameInjection *FrameInjection::GetStackFrame(int level) {
  FrameInjection *frame = ThreadInfo::s_threadInfo->m_top;
  for (int i = 0; i < level && frame; i++) {
    while (frame && (frame->m_flags & PseudoMain)) {
      frame = frame->getPrev();
    }
    if (frame) {
      frame = frame->getPrev();
    }
  }
  return frame;
}

String FrameInjection::getFileName() {
  if (m_flags & PseudoMain) {
    return m_name + 10;
  }
  const char *c = strstr(m_name, "::");
  const char *f = NULL;
  if (c) {
    f = SourceInfo::TheSourceInfo.getClassDeclaringFile(m_class);
  } else {
    f = SourceInfo::TheSourceInfo.getFunctionDeclaringFile(m_name);
  }
  if (f != NULL) {
    return f;
  }
  return null_string;
}

Array FrameInjection::getArgs() {
  return Array();
}

ObjectData *FrameInjection::getThis() {
  ObjectData *ret = m_object.get();
  if (ret && !ret->o_getId()) {
    ret = NULL;
  }
  return ret;
}

ObjectData *FrameInjection::getThisForArrow() {
  ObjectData *ret = m_object.get();
  if (ret && ret->o_getId()) {
    return ret;
  }
  throw FatalErrorException("Using $this when not in object context");
}

///////////////////////////////////////////////////////////////////////////////
// static late binding

#ifdef ENABLE_LATE_STATIC_BINDING
CStrRef FrameInjection::GetStaticClassName(ThreadInfo *info) {
  ASSERT(info);
  for (FrameInjection *t = info->m_top; t; t = t->m_prev) {
    if (t != info->m_top) {
      if (t->m_staticClass) return *t->m_staticClass;
      if (t->m_callingObject) {
        return t->m_callingObject->getRoot()->o_getClassName();
      }
    }
    if (!t->m_object.isNull() && t->m_object->o_getId()) {
      return t->m_object->o_getClassName();
    }
  }
  return empty_string;
}
#endif /* ENABLE_LATE_STATIC_BINDING */

///////////////////////////////////////////////////////////////////////////////
}
