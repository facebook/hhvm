/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#pragma once

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

  bool operator==(const Varea& area) const { return start == area.start; }
  bool operator!=(const Varea& area) const { return start != area.start; }

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
  explicit Vtext(CodeBlock& main, DataBlock& data)
    : m_areas{main}
    , m_data(data)
  {}
  Vtext(CodeBlock& main, CodeBlock& cold, DataBlock& data)
    : m_areas{main, cold}
    , m_data(data)
  {}
  Vtext(CodeBlock& main, CodeBlock& cold, CodeBlock& frozen, DataBlock& data)
    : m_areas{main, cold, frozen}
    , m_data(data)
  {
    // Main and frozen aren't allowed to alias each other unless cold is /also/
    // the same code region.
    assertx(this->main() != this->frozen() ||
            this->main() == this->cold());
  }

  /*
   * Get an existing area.
   */
  const Varea& area(AreaIndex i) const;
        Varea& area(AreaIndex i);
  Varea& main() { return area(AreaIndex::Main); }
  Varea& cold() { return area(AreaIndex::Cold); }
  Varea& frozen() { return area(AreaIndex::Frozen); }

  DataBlock& data() { return m_data; }

  /*
   * The vector of areas.
   */
  const jit::vector<Varea>& areas() const { return m_areas; }

  // Get the underlying address backing the code at `a'.
  CodeAddress toDestAddress(CodeAddress a);

private:
  jit::vector<Varea> m_areas; // indexed by AreaIndex
  DataBlock& m_data;
};

///////////////////////////////////////////////////////////////////////////////

}}

#include "hphp/runtime/vm/jit/vasm-text-inl.h"

