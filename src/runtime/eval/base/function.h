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

#ifndef __EVAL_BASE_FUNCTION_H__
#define __EVAL_BASE_FUNCTION_H__

#include <runtime/eval/base/eval_base.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

class VariableEnvironment;
class FunctionCallExpression;

class Function {
public:
  virtual ~Function() {}
  virtual Variant invoke(CArrRef params) const = 0;
  virtual Variant directInvoke(VariableEnvironment &env,
                               const FunctionCallExpression *caller) const = 0;
};

extern Variant invoke_from_eval(const char *function,
                                VariableEnvironment &env,
                                const FunctionCallExpression *caller,
                                int64 hash = -1,
                                bool fatal = true);

extern Variant invoke_from_eval_builtin(const char *function,
                                        VariableEnvironment &env,
                                        const FunctionCallExpression *caller,
                                        int64 hash = -1,
                                        bool fatal = true);


///////////////////////////////////////////////////////////////////////////////
}
}

#endif /* __EVAL_BASE_FUNCTION_H__ */
