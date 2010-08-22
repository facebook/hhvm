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

String EvalFrameInjection::getFileName() {
  return m_file;
}

Array EvalFrameInjection::getArgs() {
  return m_env.getParams();
}

EvalFrameInjection::EvalStaticClassNameHelper::EvalStaticClassNameHelper
(CStrRef name, bool sp) : m_set(false) {
  if (!sp) {
    FrameInjection::SetStaticClassName(NULL, name);
    m_set = true;
  }
}

EvalFrameInjection::EvalStaticClassNameHelper::EvalStaticClassNameHelper
(CObjRef obj) : m_set(false) {
  FrameInjection::SetCallingObject(NULL, obj.get());
}

EvalFrameInjection::EvalStaticClassNameHelper::~EvalStaticClassNameHelper() {
  if (m_set) {
    FrameInjection::ResetStaticClassName(NULL);
  }
}

///////////////////////////////////////////////////////////////////////////////
}
}
