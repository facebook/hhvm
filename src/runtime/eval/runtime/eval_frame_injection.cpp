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
#include <runtime/eval/runtime/eval_frame_injection.h>
#include <runtime/eval/runtime/variable_environment.h>
#include <runtime/eval/parser/parser.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

String EvalFrameInjection::getFileNameEval() {
  return m_file;
}

Array EvalFrameInjection::getArgsEval() {
  return m_env.getParams();
}

EvalFrameInjection::EvalStaticClassNameHelper::EvalStaticClassNameHelper
(CStrRef name, bool sp) : m_set(false), m_prev(NULL) {
#ifdef ENABLE_LATE_STATIC_BINDING
  if (!sp) {
    m_prev =
      FrameInjection::SetStaticClassName(ThreadInfo::s_threadInfo.getNoCheck(),
                                         name);
    m_set = true;
  }
#endif
}

EvalFrameInjection::EvalStaticClassNameHelper::EvalStaticClassNameHelper
(CObjRef obj) : m_set(false), m_prev(NULL) {
#ifdef ENABLE_LATE_STATIC_BINDING
  FrameInjection::SetStaticClassName(ThreadInfo::s_threadInfo.getNoCheck(),
                                     obj->getRoot()->o_getClassName());
#endif
}

EvalFrameInjection::EvalStaticClassNameHelper::~EvalStaticClassNameHelper() {
#ifdef ENABLE_LATE_STATIC_BINDING
  if (m_set) {
    FrameInjection::SetStaticClassName(ThreadInfo::s_threadInfo.getNoCheck(),
                                       *m_prev);
  }
#endif
}

///////////////////////////////////////////////////////////////////////////////
}
}
