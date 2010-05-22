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

#ifndef __EVAL_EXT_EXT_H__
#define __EVAL_EXT_EXT_H__

#include <runtime/eval/base/eval_base.h>
#include <util/case_insensitive.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////
class Function;

class EvalOverrides {
public:
  EvalOverrides();
  ~EvalOverrides();
  const Function *findFunction(const char *name) const;
private:
  hphp_const_char_imap<const Function*> m_functions;
};

extern EvalOverrides evalOverrides;

///////////////////////////////////////////////////////////////////////////////
}
}

#endif /* __EVAL_EXT_EXT_H__ */
