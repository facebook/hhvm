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
#include <cpp/base/types.h>
#include <cpp/base/builtin_functions.h>
#include <cpp/base/source_info.h>
#include <cpp/base/class_info.h>
#include <cpp/base/frame_injection.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
IMPLEMENT_THREAD_LOCAL(FrameInjection *, FrameInjection::s_top);

String FrameInjection::getClassName(bool skip /* = false */) {
  FrameInjection *t = *s_top;
  if (t && skip) {
    t = t->m_prev;
  }
  if (t && t->m_class) {
    return t->m_class;
  }
  return "";
}

String FrameInjection::getParentClassName(bool skip /* = false */) {
  String cls = getClassName(skip);
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

Array FrameInjection::getBacktrace(bool skip /* = false */,
                                   bool withSelf /* = false */) {
  Array bt = Array::Create();
  FrameInjection *t = *s_top;
  if (skip && t) {
    t = t->m_prev;
  }
  if (withSelf && t) {
    // This is used by onError with extended exceptions
    Array frame = Array::Create();
    frame.set("file", t->getFileName());
    frame.set("line", t->line);
    bt.append(frame);
  }
  while (t) {
    if (strncmp(t->m_name, "run_init::", 10) == 0) {
      // TODO should we generate require_once for pseudo mains?
      t = t->m_prev;
      continue;
    }
    Array frame = Array::Create();
    const char *c = strstr(t->m_name, "::");
    if (c) {
      frame.set("function", String(c + 2));
      frame.set("class", String(t->m_class));
      if (t->m_object) {
        frame.set("object", t->m_object);
        frame.set("type", "->");
      } else {
        frame.set("type", "::");
      }
    } else {
      frame.set("function", t->m_name);
    }

    if (t->m_prev) {
      String file = t->m_prev->getFileName();
      if (!file.empty()) {
        frame.set("file", file);
      }
      frame.set("line", t->m_prev->line);
    }

    Array args = t->getArgs();
    if (!args.isNull()) {
      frame.set("args", args);
    }

    bt.append(frame);
    t = t->m_prev;
  }
  return bt;
}

String FrameInjection::getFileName() {
  if (strncmp(m_name, "run_init::", 10) == 0) {
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
  return String();
}

Array FrameInjection::getArgs() {
  return Array();
}

///////////////////////////////////////////////////////////////////////////////
}
