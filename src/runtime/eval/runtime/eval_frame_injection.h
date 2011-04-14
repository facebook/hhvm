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
  class EvalStaticClassNameHelper {
  public:
    EvalStaticClassNameHelper(CStrRef name, bool sp);
    EvalStaticClassNameHelper(CObjRef obj);
    ~EvalStaticClassNameHelper();
    bool m_set;
    const String *m_prev;
  };

  EvalFrameInjection(CStrRef cls, const char *name,
                     VariableEnvironment &env, const char *file,
                     ObjectData *obj = NULL, int fs = 0)
    : FrameInjection(name, fs | FrameInjection::EvalFrame),
      m_class(cls), m_env(env), m_file(file) {
        m_object = obj ? obj->getRoot() : NULL;
        if (m_object) {
          m_object->incRefCount();
        }
      }

  ~EvalFrameInjection() {
    if (m_object && m_object->decRefCount() == 0)  {
      m_object->release();
    }
  }

  String getFileNameEval();
  Array getArgsEval();

  VariableEnvironment &getEnv() { return m_env; }
  CStrRef getClass() const { return m_class; }
  ObjectData *getObject() const { return m_object; }

private:
  CStrRef m_class;
  ObjectData *m_object;
  VariableEnvironment &m_env;
  const char *m_file;
};

#define SET_LINE_EXPR \
  set_line(m_loc.line0, m_loc.char0, m_loc.line1, m_loc.char1)
#define SET_LINE      if (!SET_LINE_EXPR) return Variant::lvalBlackHole();
#define SET_LINE_VOID if (!SET_LINE_EXPR) return;

///////////////////////////////////////////////////////////////////////////////
}
}

#endif // __HPHP_EVAL_RUNTIME_EVAL_FRAME_INJECTION_H__

