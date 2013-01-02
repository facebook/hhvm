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
#include <runtime/base/hphp_system.h>
#include <runtime/base/server/source_root_info.h>

#include <util/parser/parser.h>

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
static StaticString s_closureBrackets("{closure}");
static StaticString s_closureGenBrackets("{closureGen}");

///////////////////////////////////////////////////////////////////////////////

CStrRef FrameInjection::GetClassName(bool skip /* = false */) {
  const_assert(!hhvm); // VM should use ExecutionContext::getContextClassName()
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
  const_assert(!hhvm);
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
  const_assert(!hhvm);
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
  const_assert(!hhvm);
  FrameInjection *t = ThreadInfo::s_threadInfo->m_top;
  if (t && skip) {
    t = t->m_prev;
  }
  if (t) {
    return t->getFileName();
  }
  return "";
}

Array FrameInjection::getStackFrame(bool withSelf, bool withThis) {
  const_assert(!hhvm);
  Array frame = Array::Create();

  if (m_prev) {
    String file = m_prev->getFileName();
    if (!file.empty() && m_prev->m_line) {
      frame.set(s_file, file, true);
      frame.set(s_line, m_prev->m_line, true);
    }
  } else if (m_flags & PseudoMain) {
    // Stop at top, don't include top file
    return Array();
  }

  if (m_flags & PseudoMain) {
    frame.set(s_function, "include", true);
    frame.set(s_args, Array::Create(getFileName()), true);
  } else {
    const char *c = strstr(m_name, "::");
    const char *f = m_name;
    if (c) {
      f = c + 2;
      frame.set(s_class, getClassName()->copy(), true);
      if (ObjectData *obj = getObjectV()) {
        if (withThis) {
          frame.set(s_object, Object(obj), true);
        }
        frame.set(s_type, "->", true);
      } else {
        frame.set(s_type, "::", true);
      }
    }
    ASSERT(f);
    switch (f[0]) {
    case ParserBase::CharClosure:
      frame.set(s_function, s_closureBrackets, true);
      break;
    case ParserBase::CharContinuationFromClosure:
      frame.set(s_function, s_closureGenBrackets, true);
      break;
    default:
      if (const char *c = strstr(f, "$$")) {
        frame.set(s_function, String(f, c - f, CopyString), true);
      } else {
        frame.set(s_function, f, true);
      }
      break;
    }

    Array args = getArgs();
    if (!args.isNull()) {
      frame.set(s_args, args, true);
    } else {
      frame.set(s_args, Array::Create(), true);
    }
  }
  return frame;
}

Array FrameInjection::GetBacktrace(bool skip /* = false */,
                                   bool withSelf /* = false */,
                                   bool withThis /* = true */) {
  const_assert(!hhvm);
  Array bt = Array::Create();
  FrameInjection *t = ThreadInfo::s_threadInfo->m_top;
  if (skip && t) {
    t = t->m_prev;
  }
  // This is used by onError with extended exceptions
  if (withSelf && t) {
    String filename = t->getFileName();
    // If the top frame is not an extension function, and has actually been
    // entered (i.e. has a real line number) add it to the trace
    if (filename != "" && t->m_line != 0) {
      Array frame = Array::Create();
      frame.set(s_file, filename, true);
      frame.set(s_line, t->m_line, true);
      bt.append(frame);
    }
  }
  Array sbt = bt;
  FrameInjection *st = t;
  while (t && (RuntimeOption::InjectedStackTraceLimit < 0 ||
               bt.size() < RuntimeOption::InjectedStackTraceLimit)) {
    Array frame = t->getStackFrame(withSelf, withThis);
    if (frame.isNull()) return bt;
    bt.append(frame);
    t = t->m_prev;
  }
  if (!t) return bt;
  // The stack depth has exceeded the limit. Re-construct bt to include the
  // top and bottom frames. This is considered more useful in cases such as
  // infinite recursion.
  ASSERT(bt.size() == RuntimeOption::InjectedStackTraceLimit);
  int half = RuntimeOption::InjectedStackTraceLimit / 2;

  // start over, get the top half
  bt = sbt;
  t = st;
  while (t && bt.size() < half) {
    Array frame = t->getStackFrame(withSelf, withThis);
    if (frame.isNull()) not_reached();
    bt.append(frame);
    t = t->m_prev;
  }
  // then the bottom half
  std::vector<FrameInjection *> remainingFrames;
  while (t) {
    if (!t->m_prev && (t->m_flags & PseudoMain)) break;
    remainingFrames.push_back(t);
    t = t->m_prev;
  }
  int remaining = remainingFrames.size();
  int omitted = remaining - half;
  always_assert(omitted >= 0);
  if (omitted > 0) {
    std::string tmp("(...) ");
    tmp += boost::lexical_cast<std::string>(omitted) +
           " omitted frame" + ((omitted > 1) ? "s" : "");
    Array frame;
    frame.set(s_function, tmp);
    bt.append(frame);
  }
  for (int i = omitted; i < remaining; i++) {
    Array frame = remainingFrames[i]->getStackFrame(withSelf, withThis);
    if (frame.isNull()) not_reached();
    bt.append(frame);
  }
  return bt;
}

static bool IsCallUserFunc(const char* name) {
  return strcasecmp(name, "call_user_func") == 0 ||
    strcasecmp(name, "call_user_func_array") == 0;
}

Array FrameInjection::GetCallerInfo(bool skip /* = false */) {
  const_assert(!hhvm);
  FrameInjection *t = ThreadInfo::s_threadInfo->m_top;
  if (skip && t) {
    t = t->m_prev;
  }
  while (t) {
    if (IsCallUserFunc(t->m_name)) {
      t = t->m_prev;
    } else {
      break;
    }
  }
  while (t && t->m_prev) {
    String file = t->m_prev->getFileName();
    if (!file.empty() && t->m_prev->m_line) {
      Array result = Array::Create();
      if (getHphpBinaryType() == HphpBinary::program) {
        // GetCallerInfo() must always return the absolute file path.
        // getFileName() returns a relative path under hphpc, so we
        // we convert it to an absolute path here.
        const string& srcRoot = SourceRootInfo::GetCurrentSourceRoot();
        ASSERT(!file.size() || file[0] != '/');
        ASSERT(srcRoot.size() && srcRoot[srcRoot.size()-1] == '/');
        file = srcRoot + file.data();
      } else {
        // If we are not under hphpc, then getFileName() should return
        // an absolute path.
        ASSERT(!file.size() || file[0] == '/');
      }
      result.set(s_file, file, true);
      result.set(s_line, t->m_prev->m_line, true);
      return result;
    }
    t = t->m_prev;
    if (t) {
      if (IsCallUserFunc(t->m_name)) {
        continue;
      }
    }
    break;
  }
  return Array::Create();
}

int FrameInjection::GetLine(bool skip /* = false */) {
  const_assert(!hhvm);
  FrameInjection *t = ThreadInfo::s_threadInfo->m_top;
  if (t && skip) {
    t = t->m_prev;
  }
  if (t) {
    return t->m_line;
  }
  return -1;
}

CStrRef FrameInjection::GetStaticClassName(ThreadInfo *info) {
  const_assert(!hhvm);
  ASSERT(info);
  for (FrameInjection *t = info->m_top; t; t = t->m_prev) {
    if (t != info->m_top) {
      if (t->m_staticClass) return *t->m_staticClass;
    }
    ObjectData *obj = t->getObjectV();
    if (obj) {
      return obj->o_getClassName();
    }
    if (!(t->m_flags &
          (PseudoMain|BuiltinFunction|StaticMethod|ObjectMethod))) {
      break;
    }
  }
  return empty_string;
}

bool FrameInjection::IsGlobalScope() {
  const_assert(!hhvm);
  return IsGlobalScope(ThreadInfo::s_threadInfo->m_top);
}

bool FrameInjection::IsGlobalScope(FrameInjection *frame) {
  const_assert(!hhvm);
  while (frame) {
    if ((frame->m_flags & PseudoMain) == 0) {
      return false;
    }
    frame = frame->getPrev();
  }
  return true;
}

FrameInjection *FrameInjection::GetStackFrame(int level) {
  const_assert(!hhvm);
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
  const_assert(!hhvm);
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

ObjectData *FrameInjection::GetObjectV(
  const FrameInjection *fi) {
  const_assert(!hhvm);
  do {
    if (LIKELY(fi->isObjectMethodFrame())) {
      const FrameInjectionObjectMethod* ofi =
        static_cast<const FrameInjectionObjectMethod*>(fi);
      return ofi->getThisForBacktrace();
    }

    if (!(fi->m_flags & (PseudoMain | BuiltinFunction)) || !fi->m_prev) {
      return NULL;
    }

    fi = fi->m_prev;
  } while (true);
  return NULL;
}

String FrameInjection::getFileName() {
  const_assert(!hhvm);
  if (m_flags & PseudoMain) {
    return m_name[0] == '_' ? m_name : m_name + 10;
  }
  const char *f = SourceInfo::TheSourceInfo.getFunctionDeclaringFile(m_name);
  if (f != NULL) {
    return f;
  }
  const char *c = strstr(m_name, "::");
  if (c) {
    f = SourceInfo::TheSourceInfo.getClassDeclaringFile(getClassName());
    if (f != NULL) {
      return f;
    }
  }
  return null_string;
}

Array FrameInjection::getArgs() {
  const_assert(!hhvm);
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

HOT_FUNC_HPHP
FIFunctionMem::FIFunctionMem(const char *name)
  : FrameInjectionFunction(name, 0) {
  // Do nothing
}

HOT_FUNC_HPHP
FIFunctionMem::~FIFunctionMem() {
#ifdef REQUEST_TIMEOUT_DETECTION
  check_request_timeout(m_info);
#endif
}

HOT_FUNC_HPHP
FIFunctionNoMem::FIFunctionNoMem(const char *name)
  : FrameInjectionFunction(name, 0) {
  // Do nothing
}

HOT_FUNC_HPHP
FIFunctionNoMem::~FIFunctionNoMem() {
#ifdef REQUEST_TIMEOUT_DETECTION
  check_request_timeout_nomemcheck(m_info);
#endif
}

HOT_FUNC_HPHP
FIFunctionFS::FIFunctionFS(const char *name, int fs)
  : FrameInjectionFunction(name, fs) {
  // Do nothing
}

HOT_FUNC_HPHP
FIFunctionFS::~FIFunctionFS() {
#ifdef REQUEST_TIMEOUT_DETECTION
  check_request_timeout(m_info);
#endif
}

///////////////////////////////////////////////////////////////////////////////

HOT_FUNC_HPHP
FIStaticMethodMem::FIStaticMethodMem(const char *name)
  : FrameInjectionStaticMethod(name) {
  // Do nothing
}

HOT_FUNC_HPHP
FIStaticMethodMem::~FIStaticMethodMem() {
#ifdef REQUEST_TIMEOUT_DETECTION
  check_request_timeout(m_info);
#endif
}

HOT_FUNC_HPHP
FIStaticMethodNoMem::FIStaticMethodNoMem(const char *name)
  : FrameInjectionStaticMethod(name) {
  // Do nothing
}

HOT_FUNC_HPHP
FIStaticMethodNoMem::~FIStaticMethodNoMem() {
#ifdef REQUEST_TIMEOUT_DETECTION
  check_request_timeout_nomemcheck(m_info);
#endif
}

///////////////////////////////////////////////////////////////////////////////

HOT_FUNC_HPHP
ObjectData *FrameInjectionObjectMethod::getThis() const {
  if (!m_object->o_getId()) {
    return NULL;
  }
  return m_object;
}

ObjectData *FrameInjectionObjectMethod::getThisForBacktrace() const {
  if (m_object && !m_object->o_getId()) {
    return NULL;
  }
  return m_object;
};

ObjectData *FrameInjectionObjectMethod::getThisForArrow() {
  if (m_object->o_getId()) {
    return m_object;
  }
  throw FatalErrorException("Using $this when not in object context");
}

HOT_FUNC_HPHP
FIObjectMethodMem::FIObjectMethodMem(const char *name, ObjectData *obj)
 : FrameInjectionObjectMethod(name, obj) {
  // Do nothing
}

HOT_FUNC_HPHP
FIObjectMethodMem::~FIObjectMethodMem() {
#ifdef REQUEST_TIMEOUT_DETECTION
  check_request_timeout(m_info);
#endif
}

HOT_FUNC_HPHP
FIObjectMethodNoMem::FIObjectMethodNoMem(const char *name, ObjectData *obj)
 : FrameInjectionObjectMethod(name, obj) {
  // Do nothing
}

HOT_FUNC_HPHP
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
