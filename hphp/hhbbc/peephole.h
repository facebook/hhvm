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
#ifndef incl_HPHP_HHBBC_PEEPHOLE_H_
#define incl_HPHP_HHBBC_PEEPHOLE_H_

#include <vector>
#include <utility>

#include <folly/Optional.h>

#include "hphp/runtime/vm/hhbc.h"
#include "hphp/hhbbc/interp-state.h"
#include "hphp/hhbbc/type-system.h"

namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

/*
 * Very simple peephole rules based on last few tokens.  This runs behind
 * the ConcatPeephole.
 */
struct BasicPeephole {
  explicit BasicPeephole(std::vector<Bytecode>& stream,
                         const Index& index, const Context& ctx)
      : m_index(index), m_ctx(ctx), m_next(stream)
  {}

  void finalize() {
    if (!m_next.size()) {
      m_next.emplace_back(bc::Nop {});
    }
  }

  void push_back(const Bytecode& op);
private:
  const Index& m_index;
  const Context& m_ctx;
  std::vector<Bytecode>& m_next;
};

//////////////////////////////////////////////////////////////////////

/*
 * Perform a block-local optimization that folds sequences of Concat opcodes
 * into ConcatN opcodes.
 */
struct ConcatPeephole {
  explicit ConcatPeephole(BasicPeephole next)
    : m_next(next)
  {}
  ConcatPeephole(ConcatPeephole&&) = default;
  ConcatPeephole& operator=(const ConcatPeephole&) = delete;

  /*
   * Ensure the output stream is in a finished state.
   */
  void finalize();

  /*
   * Register the next bytecode into the stream.
   *
   * The State `state' is the pre-step interp state.
   */
  void append(const Bytecode& op,
              const State& state,
              const std::vector<Op>& srcStack);

private:
  /*
   * A working stream used to reorder and accumulate Concat's as ConcatN's.
   * The stream is reorderable up through the concats-th Concat opcode.
   */
  struct ConcatStream {
    std::vector<std::pair<Bytecode,bool>> stream;
                  // The working stream; contains bytecode, plus whether
                  // or not the bytecode is a rewriteable Concat.
    int stacksz;  // Stack size at the beginning of the stream.
    int concats;  // Number of concats to accumulate.
  };

private:
  void push_back(const Bytecode& op, bool is_concat = false);
  void squash();

private:
  BasicPeephole m_next;
  // Concat streams, nested at increasing stack depths.
  std::vector<ConcatStream> m_working;
};

//////////////////////////////////////////////////////////////////////

using Peephole = ConcatPeephole;

/*
 * Create the chain of peephole objects.  The interface to all the Peepholes is
 * the same as the interface to ConcatPeephole, above.
 */
inline Peephole make_peephole(std::vector<Bytecode>& sink,
                              const Index& index, const Context& ctx) {
  return ConcatPeephole(BasicPeephole(sink, index, ctx));
}

//////////////////////////////////////////////////////////////////////

}}

#endif
