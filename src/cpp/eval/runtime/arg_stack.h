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

#ifndef __EVAL_ARG_STACK_H__
#define __EVAL_ARG_STACK_H__

#include <cpp/eval/base/eval_base.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

class ArgStack {
public:
  ArgStack();
  ~ArgStack();
  void push(CVarRef v);
  void pop(uint n);
  Array pull(uint s, uint n) const;
  void clear();
  uint pos() const { return m_ptr; }
private:
  uint m_ptr;
  uint m_cap;
  Variant *m_stack;
};


///////////////////////////////////////////////////////////////////////////////
}
}

#endif /* __EVAL_ARG_STACK_H__ */
