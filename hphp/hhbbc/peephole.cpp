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
#include "hphp/hhbbc/peephole.h"

#include <vector>

#include "hphp/runtime/vm/hhbc.h"
#include "hphp/hhbbc/interp-state.h"
#include "hphp/hhbbc/type-system.h"
#include "hphp/hhbbc/representation.h"

namespace HPHP { namespace HHBBC {

TRACE_SET_MOD(hhbbc);

//////////////////////////////////////////////////////////////////////

void BasicPeephole::push_back(const Bytecode& next) {
  FTRACE(1, "BasicPeephole::push_back {}\n", show(next));
  if (next.op == Op::Nop) return;
  if (m_next.size()) {
    auto& cur = m_next.back();

    if ((cur.op == Op::RGetCNop && next.op == Op::UnboxRNop) ||
        (next.op == Op::PopC && (cur.op == Op::Dup ||
                                 cur.op == Op::Null ||
                                 cur.op == Op::False ||
                                 cur.op == Op::True ||
                                 cur.op == Op::Int ||
                                 cur.op == Op::Double ||
                                 cur.op == Op::String))) {
      m_next.pop_back();
      return;
    }

    if (!m_ctx.func->isGenerator &&
        m_next.size() > 1 &&
        cur.op == Op::UnboxRNop &&
        next.op == Op::Await) {
      auto& prev = (&cur)[-1];
      if (prev.op == Op::FCallD) {
        auto& call = prev.FCallD;
        auto async = [&]() {
          if (call.str2->empty()) {
            return m_index.is_async_func(
              m_index.resolve_func(m_ctx, call.str3));
          }
          auto cls = m_index.resolve_class(m_ctx, call.str2);
          assert(cls);
          auto func = m_index.resolve_method(m_ctx, subCls(*cls),
                                             call.str3);
          return m_index.is_async_func(func);
        }();
        if (async) {
          prev = bc::FCallAwait {
            call.arg1, call.str2, call.str3
          };
          m_next.pop_back();
          return;
        }
      }
    }

    if (cur.op == Op::SetL && next.op == Op::PopC) {
      cur = bc_with_loc(next.srcLoc, bc::PopL { cur.SetL.loc1 });
      return;
    }
  }
  m_next.push_back(next);
}

std::string BasicPeephole::show(const Bytecode& op) {
  return ::HPHP::HHBBC::show(m_ctx.func, op);
}

void ConcatPeephole::finalize() {
  while (!m_working.empty()) {
    squash();
  }
  m_next.finalize();
}

void ConcatPeephole::append(const Bytecode& op,
                            const State& state,
                            const std::vector<Op>& srcStack) {
  FTRACE(1, "ConcatPeephole::append {}\n", m_next.show(op));
  assert(state.stack.size() == srcStack.size());
  int nstack = state.stack.size();

  // Size of the stack at the previous Concat.
  int prevsz = m_working.empty() ? -1 : m_working.back().stacksz;

  // Squash the innermost concat stream if we consumed its concat result.
  if (nstack < prevsz - 1 || (nstack == prevsz - 1 &&
                              srcStack[nstack - 1] != Op::Concat)) {
    squash();
  }

  if (op.op == Op::Concat) {
    auto ind1 = nstack - 1;
    auto ind2 = nstack - 2;

    // Non-string concat; just append, squashing if this terminates a stream.
    if (!state.stack[ind1].type.subtypeOf(TStr) ||
        !state.stack[ind2].type.subtypeOf(TStr)) {
      if (nstack == prevsz) {
        squash();
      }
      return push_back(op);
    }

    // If the first concat operand is from the previous concat in the
    // stream, continue the current stream.
    if (srcStack[ind2] == Op::Concat && nstack == prevsz) {
      return push_back(op, true);
    }

    // If the second concat operand is from the previous concat in the
    // stream, continue the current stream.
    if (srcStack[ind1] == Op::Concat && nstack == prevsz - 1) {
      m_working.back().stacksz--;
      return push_back(op, true);
    }

    // Correction for cases where we might have bizarre opcode sequences like
    // [stk: 2] Concat, [stk: 1] CGetL2, [stk: 2] Concat, where it's unsafe
    // to reorder.
    if (nstack == prevsz) {
      squash();
    }

    // Start a new stream.
    m_working.push_back({{}, nstack, 0});
    return push_back(op, true);
  }

  // Just push by default.
  push_back(op);
}

/*
 * Push to the innermost stream.
 */
void ConcatPeephole::push_back(const Bytecode& op, bool is_concat) {
  if (m_working.empty()) {
    m_next.push_back(op);
  } else {
    auto& inner = m_working.back();

    if (is_concat) ++inner.concats;
    inner.stream.emplace_back(op, is_concat);
  }
}

/*
 * Reorder and rewrite the most nested concat subsequence, and append it to
 * the previous subsequence in the stack.
 */
void ConcatPeephole::squash() {
  assert(!m_working.empty());

  auto workstream = std::move(m_working.back());
  m_working.pop_back();

  // Concat counters.
  uint32_t naccum = 1;
  int ntotal = 0;

  assert(workstream.stream.front().first.op == Op::Concat);

  for (auto& item : workstream.stream) {
    auto& op = item.first;
    auto& is_concat = item.second;

    // If we passed the last concat, just append the remaining bytecode.
    if (ntotal < workstream.concats && is_concat) {
      // Bump counters.
      ++naccum;
      ++ntotal;

      // Emit a ConcatN if we hit the limit, or if we hit the final Concat.
      if (naccum == kMaxConcatN || ntotal == workstream.concats) {
        if (naccum >= 2) {
          if (naccum == 2) {
            push_back(bc::Concat {});
          } else {
            push_back(bc::ConcatN {naccum});
          }
        }
        naccum = 1;
      }
      continue;
    }

    push_back(op);
  }
}

//////////////////////////////////////////////////////////////////////

}}
