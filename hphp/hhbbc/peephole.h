/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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
 * Very simple peephole rules based on 1-token of lookahead.  This runs behind
 * the ConcatPeephole.
 */
struct BasicPeephole {
  explicit BasicPeephole(std::vector<Bytecode>& stream)
    : m_next(stream)
  {}

  void finalize() { if (m_pending) flush(); }

  void push_back(const Bytecode& op) {
    if (!m_pending) {
      m_pending = op;
      return;
    }

    auto const peeped = peep(op);
    flush();
    if (!peeped) {
      m_pending = op;
    }
  }

private:
  bool peep(const Bytecode& next) {
    auto nop = [&] (Op a, Op b) -> bool {
      if (m_pending->op == a && next.op == b) {
        m_pending = bc::Nop {};
        return true;
      }
      return false;
    };

    return false
      || nop(Op::RGetCNop, Op::UnboxRNop)
      ;
  }

  void flush() {
    m_next.push_back(*m_pending);
    m_pending = folly::none;
  }

private:
  std::vector<Bytecode>& m_next;
  folly::Optional<Bytecode> m_pending;
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
inline Peephole make_peephole(std::vector<Bytecode>& sink) {
  return ConcatPeephole(BasicPeephole(sink));
}

//////////////////////////////////////////////////////////////////////

}}

#endif
