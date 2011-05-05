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
#include <runtime/base/types.h>
#include <runtime/base/type_conversions.h>
#include <runtime/base/builtin_functions.h>
#include <runtime/base/source_info.h>
#include <runtime/base/class_info.h>
#include <runtime/base/frame_injection.h>

#include <runtime/eval/runtime/eval_frame_injection.h>

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
    while (t->m_prev && t->getClassName().empty() &&
           t->m_flags & (PseudoMain | BuiltinFunction)) {
      t = t->m_prev;
    }
    CStrRef name = t->getClassName();
    if (!name.empty()) {
      return name;
    }
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
    return t->getObjectV();
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
        frame.set(s_class, t->getClassName()->copy(), true);
        if (ObjectData *obj = t->getObjectV()) {
          if (withThis) {
            frame.set(s_object, Object(obj), true);
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

#ifdef ENABLE_LATE_STATIC_BINDING
CStrRef FrameInjection::GetStaticClassName(ThreadInfo *info) {
  ASSERT(info);
  for (FrameInjection *t = info->m_top; t; t = t->m_prev) {
    if (t != info->m_top) {
      if (t->m_staticClass) return *t->m_staticClass;
    }
    ObjectData *obj = t->getObjectV();
    if (obj) {
      return obj->o_getClassName();
    }
  }
  return empty_string;
}
#endif /* ENABLE_LATE_STATIC_BINDING */

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

///////////////////////////////////////////////////////////////////////////////

CStrRef FrameInjection::getClassName() const {
  if (isEvalFrame()) {
    const Eval::EvalFrameInjection *efi =
      static_cast<const Eval::EvalFrameInjection*>(this);
    return efi->getClass();
  }
  // Otherwise, parse the name and lookup from ClassInfo
  const char *c = strstr(m_name, "::");
  if (!c) return empty_string;
  String cls(m_name, c - m_name, CopyString);
  const ClassInfo *classInfo = ClassInfo::FindClass(cls);
  if (classInfo) {
    CStrRef clsRef = classInfo->getName();
    if (!clsRef.isNull()) {
      return clsRef;
    }
  }
  return empty_string;
}

ObjectData *FrameInjection::getObjectV() const {
  if (isObjectMethodFrame()) {
    const FrameInjectionObjectMethod* ofi =
      static_cast<const FrameInjectionObjectMethod*>(this);
    return ofi->getThis();
  } else if (isEvalFrame()) {
    const Eval::EvalFrameInjection* efi =
      static_cast<const Eval::EvalFrameInjection*>(this);
    return efi->getThis();
  } else if (m_flags & PseudoMain) {
    const FrameInjectionFunction *ffi =
      static_cast<const FrameInjectionFunction*>(this);
    return ffi->getThis();
  }
  return NULL;
}

String FrameInjection::getFileName() {
  if (m_flags & PseudoMain) {
    return m_name[0] == '_' ? m_name : m_name + 10;
  }
  if (isEvalFrame()) {
    Eval::EvalFrameInjection *efi =
      static_cast<Eval::EvalFrameInjection*>(this);
    return efi->getFileNameEval();
  }
  const char *c = strstr(m_name, "::");
  const char *f = NULL;
  if (c) {
    f = SourceInfo::TheSourceInfo.getClassDeclaringFile(getClassName());
  } else {
    f = SourceInfo::TheSourceInfo.getFunctionDeclaringFile(m_name);
  }
  if (f != NULL) {
    return f;
  }
  return null_string;
}

Array FrameInjection::getArgs() {
  if (m_flags & EvalFrame) {
    Eval::EvalFrameInjection *efi =
      static_cast<Eval::EvalFrameInjection*>(this);
    return efi->getArgsEval();
  }
  return Array();
}

///////////////////////////////////////////////////////////////////////////////

ObjectData *FrameInjectionFunction::getThis() const {
  if ((m_flags & PseudoMain) && m_prev) {
    return m_prev->getObjectV();
  }
  return NULL;
}

ObjectData *FrameInjectionFunction::getThisForArrow() {
  if (ObjectData *obj = getThis()) {
    return obj;
  }
  throw FatalErrorException("Using $this when not in object context");
}

__attribute__ ((section (".text.hot")))
FIFunctionMem::FIFunctionMem(const char *name)
  : FrameInjectionFunction(name, 0) {
  // Do nothing
}

__attribute__ ((section (".text.hot")))
FIFunctionMem::~FIFunctionMem() {
#ifdef REQUEST_TIMEOUT_DETECTION
  check_request_timeout(m_info);
#endif
}

__attribute__ ((section (".text.hot")))
FIFunctionNoMem::FIFunctionNoMem(const char *name)
  : FrameInjectionFunction(name, 0) {
  // Do nothing
}

__attribute__ ((section (".text.hot")))
FIFunctionNoMem::~FIFunctionNoMem() {
#ifdef REQUEST_TIMEOUT_DETECTION
  check_request_timeout_nomemcheck(m_info);
#endif
}

__attribute__ ((section (".text.hot")))
FIFunctionFS::FIFunctionFS(const char *name, int fs)
  : FrameInjectionFunction(name, fs) {
  // Do nothing
}

__attribute__ ((section (".text.hot")))
FIFunctionFS::~FIFunctionFS() {
#ifdef REQUEST_TIMEOUT_DETECTION
  check_request_timeout(m_info);
#endif
}

///////////////////////////////////////////////////////////////////////////////

__attribute__ ((section (".text.hot")))
FIStaticMethodMem::FIStaticMethodMem(const char *name)
  : FrameInjectionStaticMethod(name) {
  // Do nothing
}

__attribute__ ((section (".text.hot")))
FIStaticMethodMem::~FIStaticMethodMem() {
#ifdef REQUEST_TIMEOUT_DETECTION
  check_request_timeout(m_info);
#endif
}

__attribute__ ((section (".text.hot")))
FIStaticMethodNoMem::FIStaticMethodNoMem(const char *name)
  : FrameInjectionStaticMethod(name) {
  // Do nothing
}

__attribute__ ((section (".text.hot")))
FIStaticMethodNoMem::~FIStaticMethodNoMem() {
#ifdef REQUEST_TIMEOUT_DETECTION
  check_request_timeout_nomemcheck(m_info);
#endif
}

///////////////////////////////////////////////////////////////////////////////

ObjectData *FrameInjectionObjectMethod::getThis() const {
  if (!m_object->o_getId()) {
    return NULL;
  }
  return m_object;
}

ObjectData *FrameInjectionObjectMethod::getThisForArrow() {
  if (m_object->o_getId()) {
    return m_object;
  }
  throw FatalErrorException("Using $this when not in object context");
}

__attribute__ ((section (".text.hot")))
FIObjectMethodMem::FIObjectMethodMem(const char *name, ObjectData *obj)
 : FrameInjectionObjectMethod(name, obj) {
  // Do nothing
}

__attribute__ ((section (".text.hot")))
FIObjectMethodMem::~FIObjectMethodMem() {
#ifdef REQUEST_TIMEOUT_DETECTION
  check_request_timeout(m_info);
#endif
}

__attribute__ ((section (".text.hot")))
FIObjectMethodNoMem::FIObjectMethodNoMem(const char *name, ObjectData *obj)
 : FrameInjectionObjectMethod(name, obj) {
  // Do nothing
}

__attribute__ ((section (".text.hot")))
FIObjectMethodNoMem::~FIObjectMethodNoMem() {
#ifdef REQUEST_TIMEOUT_DETECTION
  check_request_timeout_nomemcheck(m_info);
#endif
}

///////////////////////////////////////////////////////////////////////////////

FIFunctionNP::FIFunctionNP(const char *name)
  : FrameInjection(name, BuiltinFunction | Function) {
}

FIFunctionNP::~FIFunctionNP() {
#ifdef REQUEST_TIMEOUT_DETECTION
  check_request_timeout(m_info);
#endif
}

///////////////////////////////////////////////////////////////////////////////
}
