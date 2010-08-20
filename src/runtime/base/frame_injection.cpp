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

const char *FrameInjection::GetParentClassName(bool skip /* = false */) {
  CStrRef cls = GetClassName(skip);
  if (cls.empty()) return cls;
  const ClassInfo *classInfo = ClassInfo::FindClass(cls);
  if (classInfo) {
    const char *parentClass = classInfo->getParentClass();
    if (parentClass && parentClass[0]) {
      return parentClass;
    }
  }
  return "";
}

Object FrameInjection::GetThis(bool skip /* = false */) {
  FrameInjection *t = ThreadInfo::s_threadInfo->m_top;
  if (t && skip) {
    t = t->m_prev;
  }
  if (t) {
    return t->m_object;
  }
  return Object();
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
      frame.set(s_file, filename, -1, true);
      frame.set(s_line, t->m_line, -1, true);
      bt.append(frame);
    }
  }
  while (t) {
    Array frame = Array::Create();

    if (t->m_prev) {
      String file = t->m_prev->getFileName();
      if (!file.empty() && t->m_prev->m_line) {
        frame.set(s_file, file, -1, true);
        frame.set(s_line, t->m_prev->m_line, -1, true);
      }
    } else if (t->m_flags & PseudoMain) {
      // Stop at top, don't include top file
      break;
    }

    if (t->m_flags & PseudoMain) {
      frame.set(s_function, "include", -1, true);
      frame.set(s_args, Array::Create(t->getFileName()), -1, true);
    } else {
      const char *c = strstr(t->m_name, "::");
      if (c) {
        frame.set(s_function, String(c + 2), -1, true);
        frame.set(s_class, t->m_class->copy(), -1, true);
        if (!t->m_object.isNull()) {
          if (withThis) {
            frame.set(s_object, t->m_object, -1, true);
          }
          frame.set(s_type, "->", -1, true);
        } else {
          frame.set(s_type, "::", -1, true);
        }
      } else {
        frame.set(s_function, t->m_name, -1, true);
      }

      Array args = t->getArgs();
      if (!args.isNull()) {
        frame.set(s_args, args, -1, true);
      } else {
        frame.set(s_args, Array::Create(), -1, true);
      }
    }

    bt.append(frame);
    t = t->m_prev;
  }
  return bt;
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

Object &FrameInjection::getThis() {
  if (m_object.get() && m_object->o_getId()) {
    return m_object;
  }

  static Object s_black_hole_object;
  ASSERT(s_black_hole_object.isNull()); // in case callers modified me
  s_black_hole_object.reset();
  return s_black_hole_object;
}

Object &FrameInjection::getThisForArrow() {
  if (m_object.get() && m_object->o_getId()) {
    return m_object;
  }
  throw FatalErrorException("Using $this when not in object context");
}

///////////////////////////////////////////////////////////////////////////////
// static late binding

String FrameInjection::GetStaticClassName(ThreadInfo *info) {
  if (!info) info = ThreadInfo::s_threadInfo.get();
  for (FrameInjection *t = info->m_top; t; t = t->m_prev) {
    if (t == info->m_top && !t->m_object.isNull() && t->m_object->o_getId()) {
      return t->m_object->o_getClassName();
    }
    if (!t->m_staticClass.empty()) {
      return t->m_staticClass;
    }
    if (t != info->m_top && !t->m_callingObject.isNull()) {
      return t->m_callingObject->o_getClassName();
    }
  }
  return "";
}

void FrameInjection::SetStaticClassName(ThreadInfo *info, CStrRef cls) {
  if (!info) info = ThreadInfo::s_threadInfo.get();
  FrameInjection *t = info->m_top;
  if (t) t->m_staticClass = cls;
}

void FrameInjection::ResetStaticClassName(ThreadInfo *info) {
  if (!info) info = ThreadInfo::s_threadInfo.get();
  FrameInjection *t = info->m_top;
  if (t) t->m_staticClass.reset();
}

void FrameInjection::SetCallingObject(ThreadInfo* info, ObjectData *obj) {
  if (!info) info = ThreadInfo::s_threadInfo.get();
  FrameInjection *t = info->m_top;
  if (t) t->m_callingObject = obj;
}

///////////////////////////////////////////////////////////////////////////////
}
