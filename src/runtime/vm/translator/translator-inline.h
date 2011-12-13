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

#ifndef incl_TRANSLATOR_INLINE_H_
#define incl_TRANSLATOR_INLINE_H_

#include "translator.h"
#include <runtime/base/execution_context.h>

namespace HPHP   {
namespace VM     {
namespace Transl {

/*
 * Accessors for the virtual machine registers, both rvalues and
 * lvalues.
 */
static inline Cell*&  vmsp() { return (Cell*&)g_context->m_stack.top(); }
static inline Cell*&  vmfp() { return (Cell*&)g_context->m_fp; }
static inline const uchar*& vmpc() { return g_context->m_pc; }
static inline ActRec*& vmFirstAR() { return g_context->m_firstAR; }

static inline ActRec* curFrame()    { return (ActRec*)vmfp(); }
static inline const Func* curFunc() { return curFrame()->m_func; }
static inline const Unit* curUnit() { return curFunc()->m_unit; }


static inline uintptr_t tlsBase() {
  uintptr_t retval;
  asm ("movq %%fs:0, %0" : "=r" (retval));
  return retval;
}

} } } // HPHP::VM::Transl

#endif
