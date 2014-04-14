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
#ifndef incl_HPHP_HHBBC_PEEPHOLE_H_
#define incl_HPHP_HHBBC_PEEPHOLE_H_

#include <vector>

#include "hphp/runtime/vm/hhbc.h"
#include "hphp/hhbbc/interp-state.h"
#include "hphp/hhbbc/type-system.h"

namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

/*
 * Single-block peephole optimization manager.
 */
class BytecodeAccumulator {
public:
  explicit BytecodeAccumulator(std::vector<Bytecode>& stream)
    : m_stream(stream) {}

private:
  /*
   * A working stream used to reorder and accumulate Concat's as ConcatN's.
   * The stream is reorderable up through the concats-th Concat opcode.
   */
  struct ConcatStream {
    std::vector<std::pair<Bytecode, bool>> stream;
                  // The working stream; contains bytecode, plus whether
                  // or not the bytecode is a rewriteable Concat.
    int stacksz;  // Stack size at the beginning of the stream.
    int concats;  // Number of concats to accumulate.
  };

public:
  /*
   * Ensure the output stream is in a finished state.
   */
  void finalize();

  /*
   * Register the next bytecode into the stream.
   *
   * The State `state' is the pre-step interp state.
   */
  void append(const Bytecode& op, const State& state,
              const std::vector<Op> srcStack);

private:
  /*
   * Push to the innermost stream.
   */
  void push_back(const Bytecode& op, bool is_concat = false);

  /*
   * Reorder and rewrite the most nested concat subsequence, and append it to
   * the previous subsequence in the stack.
   */
  void squash();

private:
  // The managed output stream; only appended to.
  std::vector<Bytecode>& m_stream;

  // Concat streams, nested at increasing stack depths.
  std::vector<ConcatStream> m_working;
};

//////////////////////////////////////////////////////////////////////

}}

#endif // incl_HPHP_HHBBC_PEEPHOLE_H_
