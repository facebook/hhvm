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
#include <runtime/eval/ast/expression.h>
#include <runtime/eval/runtime/eval_frame_injection.h>
#include <runtime/eval/runtime/variable_environment.h>
#include <runtime/eval/parser/parser.h>
#include <runtime/eval/runtime/file_repository.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

String EvalFrameInjection::getFileNameEval() {
  ASSERT(m_file);
  if (!RuntimeOption::SandboxCheckMd5) return m_file;
  String file(m_file);
  if (m_flags & PseudoMain) {
    if (m_name[0] == '_') return m_name;
    file = m_name + 10;
  }
  return FileRepository::translateFileName(file.get());
}

Array EvalFrameInjection::getArgsEval() {
  return m_env.getParams();
}

EvalFrameInjection::EvalStaticClassNameHelper::EvalStaticClassNameHelper
(CStrRef name, bool sp) : m_set(false), m_prev(NULL) {
  if (!sp) {
    m_prev =
      FrameInjection::SetStaticClassName(ThreadInfo::s_threadInfo.getNoCheck(),
                                         name);
    m_set = true;
  }
}

EvalFrameInjection::EvalStaticClassNameHelper::EvalStaticClassNameHelper
(CObjRef obj) : m_set(false), m_prev(NULL) {
  m_prev =
    FrameInjection::SetStaticClassName(ThreadInfo::s_threadInfo.getNoCheck(),
                                       obj->getRoot()->o_getClassName());
  m_set = true;
}

EvalFrameInjection::EvalStaticClassNameHelper::~EvalStaticClassNameHelper() {
  if (m_set) {
    FrameInjection::SetStaticClassName(ThreadInfo::s_threadInfo.getNoCheck(),
                                       *m_prev);
  }
}

///////////////////////////////////////////////////////////////////////////////
}
}
