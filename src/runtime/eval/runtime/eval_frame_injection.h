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
#ifndef __HPHP_EVAL_RUNTIME_EVAL_FRAME_INJECTION_H__
#define __HPHP_EVAL_RUNTIME_EVAL_FRAME_INJECTION_H__

#include <runtime/base/frame_injection.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

class VariableEnvironment;
class Construct;

class EvalFrameInjection : public FrameInjection {
public:
  EvalFrameInjection(const char *cls, const char *name,
                     VariableEnvironment &env, const char *file,
                     ObjectData *obj = NULL, int fs = 0)
    : FrameInjection(ThreadInfo::s_threadInfo.get(), cls, name, obj, fs),
      m_env(env), m_file(file) { }

  virtual String getFileName();
  virtual Array getArgs();
  static void SetLine(const Construct *c);
private:
  VariableEnvironment &m_env;
  const char *m_file;
};

#define SET_LINE \
  EvalFrameInjection::SetLine(this)

///////////////////////////////////////////////////////////////////////////////
}
}

#endif // __HPHP_EVAL_RUNTIME_EVAL_FRAME_INJECTION_H__

