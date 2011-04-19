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

#ifndef __HPHP_CLASS_STATICS_H__
#define __HPHP_CLASS_STATICS_H__

#include <runtime/base/types.h>
#include <runtime/base/util/smart_ptr.h>
#include <runtime/base/complex_types.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

namespace Eval {
class FunctionCallExpression;
class VariableEnvironment;
}

class ClassStatics : public Countable {
public:
  ClassStatics(int redecId);
  ClassStatics(litstr name);
  virtual ~ClassStatics() {}

  int getRedeclaringId() const {
    return m_redecId;
  }

  virtual Variant os_getInit(CStrRef s);
  virtual Variant os_get(CStrRef s);
  virtual Variant &os_lval(CStrRef s);
  virtual Variant os_invoke(const char *c, const char *s, CArrRef params,
      int64 hash = -1, bool fatal = true);
  Object create(CArrRef params, bool init = true, ObjectData* root = NULL);
  virtual Object createOnly(ObjectData* root = NULL);
  virtual Variant os_constant(const char *s);
  virtual bool os_get_call_info(MethodCallPackage &info, int64 hash = -1);

  DECLARE_OBJECT_ALLOCATION(ClassStatics);
  void destruct() {} // artifact when not deriving from ObjectData

private:
  const char *m_clsname;
  int m_redecId;

  void throwUnknownClass();
};

typedef SmartPtr<ClassStatics> ClassStaticsPtr;

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_CLASS_STATICS_H__
