/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_HHBC_TRANSLATOR_INTERNAL_H_
#define incl_HPHP_HHBC_TRANSLATOR_INTERNAL_H_

namespace HPHP { namespace jit {

//////////////////////////////////////////////////////////////////////

/*
 * Create a catch block with a user-defined body (usually empty or a
 * SpillStack). Regardless of what body() does, it must return the current
 * stack pointer. This is a block to be invoked by the unwinder while unwinding
 * through a call to C++ from translated code. When attached to an instruction
 * as its taken field, code will be generated and the block will be registered
 * with the unwinder automatically.
 */
template<typename Body>
Block* HhbcTranslator::makeCatchImpl(Body body) {
  auto exit = m_irb->makeExit(Block::Hint::Unused);

  BlockPusher bp(*m_irb, makeMarker(bcOff()), exit);
  gen(BeginCatch);
  auto sp = body();
  gen(EndCatch, m_irb->fp(), sp);

  return exit;
}

//////////////////////////////////////////////////////////////////////

}}

#endif
