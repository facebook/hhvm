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
#ifndef incl_HPHP_HHBBC_PEEPHOLE_H_
#define incl_HPHP_HHBBC_PEEPHOLE_H_

#include <vector>
#include <utility>

#include <folly/Optional.h>

#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/hhbbc/interp-state.h"
#include "hphp/hhbbc/type-system.h"

namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

/*
 * Very simple peephole rules based on last few tokens.  This runs behind
 * the AppendPeephole.
 */
struct BasicPeephole {
  explicit BasicPeephole(BytecodeVec& stream,
                         const Index& index, const Context& ctx)
      : m_index(index), m_ctx(ctx), m_next(stream)
  {}

  void finalize() {
    if (!m_next.size()) {
      m_next.emplace_back(bc::Nop {});
    }
  }

  void push_back(const Bytecode& op);
  std::string show(const Bytecode& op);
private:
  const Index& m_index;
  const Context& m_ctx;
  BytecodeVec& m_next;
};

//////////////////////////////////////////////////////////////////////

struct PeepholeStackElem {
  PeepholeStackElem() = default;
  PeepholeStackElem(Op op, bool is_str) : op{op}, is_str{is_str} {}

  Op    op{Op::Nop};
  bool  is_str{false};
};

/*
 * Perform a block-local optimization that folds sequences of Concat,
 * Add*Elem opcodes.
 */
struct AppendPeephole {
  explicit AppendPeephole(BasicPeephole next)
    : m_next(next)
  {}
  AppendPeephole(AppendPeephole&&) = default;
  AppendPeephole& operator=(const AppendPeephole&) = delete;
  AppendPeephole& operator=(AppendPeephole&&) = delete;

  /*
   * Ensure the output stream is in a finished state.
   */
  void finalize();

  /*
   * Called before we interpret op
   *
   * srcStack and stack reflect the pre-interp state, and stack may be
   * modified eg to reflect the staticness of types.
   */
  void prestep(const Bytecode& op,
               const std::vector<PeepholeStackElem>& srcStack,
               InterpStack& stack);

  /*
   * Register the next bytecode into the stream.
   *
   * The srcStack reflects the pre-interp state.
   *
   * stack is the post-interp state.
   */
  void append(const Bytecode& op,
              bool squashAddElem,
              const std::vector<PeepholeStackElem>& srcStack,
              const InterpStack& stack);

private:
  enum class ASKind {
    Normal,
    Concat,
    AddElem
  };

  /*
   * A working stream used to reorder and accumulate Concat's as ConcatN's.
   * The stream is reorderable up through the concats-th Concat opcode.
   */
  struct AppendStream {
    AppendStream(int stackix, Op generator) :
        stackix{stackix},
        concats{0},
        generator{generator} {}

    // The working stream; contains bytecode, plus whether or not the
    // bytecode is a rewriteable Concat.
    std::vector<std::pair<Bytecode,ASKind>> stream;
    // Stack index of the concat result
    int stackix;
    // Number of ASKind::Concat's in the stream
    int concats;
    // The opcode generating this stream
    Op generator;
    // When dealing with an Add*ElemC stream, the current value,
    // possibly refcounted.
    Variant addElemResult;
  };

private:
  void push_back(const Bytecode& op, ASKind = ASKind::Normal);
  void squash(InterpStack* stack);
  int squashAbove(const std::vector<PeepholeStackElem>& srcStack,
                  int depth,
                  InterpStack* stack);

private:
  BasicPeephole m_next;
  // Concat streams, nested at increasing stack depths.
  std::vector<AppendStream> m_working;
};

//////////////////////////////////////////////////////////////////////

using Peephole = AppendPeephole;

/*
 * Create the chain of peephole objects.  The interface to all the Peepholes is
 * the same as the interface to AppendPeephole, above.
 */
inline Peephole make_peephole(BytecodeVec& sink,
                              const Index& index, const Context& ctx) {
  return AppendPeephole(BasicPeephole(sink, index, ctx));
}

bool poppable(Op op);

//////////////////////////////////////////////////////////////////////

}}

#endif
