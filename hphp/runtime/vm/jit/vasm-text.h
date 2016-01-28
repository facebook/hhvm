/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_JIT_VASM_TEXT_H_
#define incl_HPHP_JIT_VASM_TEXT_H_

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/containers.h"

#include "hphp/util/data-block.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

/*
 * An area into which code is emitted.
 *
 * This is a light wrapper around CodeBlock, which additionally saves the
 * original `start' address to measure the size of code emitted.
 *
 * All Vareas are instantiated by gen-time, so `start' (or `code.base()`) can
 * be used to detect aliased CodeBlocks when we emit.
 */
struct Varea {
  /* implicit */ Varea(CodeBlock& cb)
    : code(cb)
    , start(cb.frontier())
  {}

  CodeBlock& code;
  CodeAddress start;
};

///////////////////////////////////////////////////////////////////////////////

/*
 * Represents all the code areas to emit to.
 *
 * Vtext is a lightweight container for Vareas.
 */
struct Vtext {
  explicit Vtext(CodeBlock& main)
    : m_areas{main}
  {}
  Vtext(CodeBlock& main, CodeBlock& cold)
    : m_areas{main, cold}
  {}
  Vtext(CodeBlock& main, CodeBlock& cold, CodeBlock& frozen)
    : m_areas{main, cold, frozen}
  {}

  /*
   * Get an existing area.
   */
  Varea& area(AreaIndex i);
  Varea& main() { return area(AreaIndex::Main); }
  Varea& cold() { return area(AreaIndex::Cold); }
  Varea& frozen() { return area(AreaIndex::Frozen); }

  /*
   * The vector of areas.
   */
  const jit::vector<Varea>& areas() const { return m_areas; }

private:
  jit::vector<Varea> m_areas; // indexed by AreaIndex
};

///////////////////////////////////////////////////////////////////////////////

}}

#include "hphp/runtime/vm/jit/vasm-text-inl.h"

#endif
