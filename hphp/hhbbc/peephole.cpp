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

#include "hphp/hhbbc/interp-state.h"
#include "hphp/hhbbc/optimize.h"
#include "hphp/hhbbc/representation.h"
#include "hphp/hhbbc/type-system.h"

#include "hphp/runtime/vm/hhbc.h"

#include <vector>

namespace HPHP { namespace HHBBC {

TRACE_SET_MOD(hhbbc);

//////////////////////////////////////////////////////////////////////

bool poppable(Op op) {
  switch (op) {
    case Op::Dup:
    case Op::Null:
    case Op::False:
    case Op::True:
    case Op::Int:
    case Op::Double:
    case Op::String:
    case Op::Array:
    case Op::Vec:
    case Op::Dict:
    case Op::Keyset:
    case Op::NewArray:
    case Op::NewDArray:
    case Op::NewMixedArray:
    case Op::NewDictArray:
    case Op::NewLikeArrayL:
    case Op::NewCol:
      return true;
    default:
      return false;
  }
}

void BasicPeephole::push_back(const Bytecode& next) {
  FTRACE(1, "BasicPeephole::push_back {}\n", show(next));
  if (next.op == Op::Nop) return;
  auto drop = [&] {
    FTRACE(1, " BasicPeephole::drop {}\n", show(m_next.back()));
    m_next.pop_back();
  };
  if (m_next.size()) {
    auto& cur = m_next.back();
    auto update_cur = [&] (Bytecode bc, uint32_t srcLoc = -1) {
      bc.srcLoc = srcLoc == uint32_t(-1) ? cur.srcLoc : srcLoc;
      drop();
      push_back(bc);
    };

    // Kill <side-effect-free-expr>; PopX
    if ((next.op == Op::PopC && poppable(cur.op)) ||
        (next.op == Op::PopU && cur.op == Op::NullUninit)) {
      drop();
      return;
    }

    if (cur.op == Op::This && next.op == Op::PopC) {
      return update_cur(bc::CheckThis {});
    }

    // transform <expr> === null or <expr> !== null to IsTypeC Null [Not]
    if (cur.op == Op::Null &&
        (next.op == Op::Same || next.op == Op::NSame)) {
      update_cur(bc::IsTypeC { IsTypeOp::Null });
      if (next.op == Op::NSame) {
        push_back(bc_with_loc(next.srcLoc, bc::Not {}));
      }
      return;
    }

    // transform $x === null or $x !== null to IsTypeL Null [Not]
    if (m_next.size() > 1 &&
        (cur.op == Op::CGetL || cur.op == Op::CGetL2) &&
        (next.op == Op::Same || next.op == Op::NSame)) {
      if ((&cur)[-1].op == Op::Null) {
        auto const loc = cur.op == Op::CGetL ? cur.CGetL.loc1 : cur.CGetL2.loc1;
        auto const srcLoc = cur.srcLoc;
        drop();
        drop();
        push_back(bc_with_loc(srcLoc, bc::IsTypeL { loc, IsTypeOp::Null }));
        if (next.op == Op::NSame) {
          push_back(bc_with_loc(next.srcLoc, bc::Not {}));
        }
        return;
      }
    }

    // transform $x = $x . <expr> into $x .= <expr>
    if (m_next.size() > 1 &&
        cur.op == Op::Concat &&
        (next.op == Op::SetL || next.op == Op::PopL)) {
      auto& prev = (&cur)[-1];
      auto const setLoc = next.op == Op::SetL ? next.SetL.loc1 : next.PopL.loc1;
      if (prev.op == Op::CGetL2 && prev.CGetL2.loc1 == setLoc) {
        drop();
        drop();
        push_back(bc_with_loc(next.srcLoc,
                              bc::SetOpL {
                                setLoc,
                                SetOpOp::ConcatEqual
                              }));
        if (next.op == Op::PopL) {
          push_back(bc_with_loc(next.srcLoc, bc::PopC {}));
        }
        return;
      }
    }

    // transform Not; JmpZ/JmpNZ to JmpNZ/JmpZ
    if (cur.op == Op::Not &&
        (next.op == Op::JmpZ || next.op == Op::JmpNZ)) {
      if (next.op == Op::JmpZ) {
        update_cur(bc::JmpNZ { next.JmpZ.target1 });
      } else {
        update_cur(bc::JmpZ { next.JmpNZ.target1 });
      }
      return;
    }

    // transform PopL; PushL to UnsetL
    if (cur.op == Op::PopL && next.op == Op::PushL &&
        cur.PopL.loc1 == next.PushL.loc1) {
      update_cur(bc::UnsetL { cur.PopL.loc1 });
      return;
    }

    // transform SetL; PopC to PopL
    if (cur.op == Op::SetL && next.op == Op::PopC) {
      update_cur(bc::PopL { cur.SetL.loc1 }, next.srcLoc);
      return;
    }
  }
  m_next.push_back(next);
}

std::string BasicPeephole::show(const Bytecode& op) {
  return php::show(m_ctx.func, op);
}

void AppendPeephole::finalize() {
  while (!m_working.empty()) {
    squash(nullptr);
  }
  m_next.finalize();
}

void AppendPeephole::prestep(
    const Bytecode& op,
    const std::vector<PeepholeStackElem>& srcStack,
    InterpStack& stack) {

  switch (op.op) {
    case Op::Concat:
      return;
    case Op::AddElemC:
    case Op::AddNewElemC:
      squashAbove(srcStack, srcStack.size() - op.numPop() + 1, &stack);
      return;
    default:
      squashAbove(srcStack, srcStack.size() - op.numPop(), &stack);
      return;
  }
}

void AppendPeephole::append(
    const Bytecode& op,
    bool squashAddElem,
    const std::vector<PeepholeStackElem>& srcStack,
    const InterpStack& stack) {
  FTRACE(1,
         "AppendPeephole::append {} (working-size {})\n",
         m_next.show(op), m_working.size());

  auto const squashTo = [&] (int depth) {
    return squashAbove(srcStack, depth, nullptr);
  };

  const int nstack = srcStack.size();

  switch (op.op) {
    case Op::Concat: {
      auto const ind1 = nstack - 1;
      auto const ind2 = nstack - 2;

      // Non-string concat; treat like any other opcode
      if (!srcStack[ind1].is_str || !srcStack[ind2].is_str) {
        break;
      }

      auto const stackix = squashTo(nstack);
      if (stackix >= 0 && m_working.back().generator == Op::Concat) {
        // If the first concat operand is from the previous concat in the
        // stream, continue the current stream.
        if (ind2 == stackix) {
          assertx(srcStack[ind2].op == Op::Concat);
          return push_back(op, ASKind::Concat);
        }

        // If the second concat operand is from the previous concat in the
        // stream, continue the current stream, merging into the previous
        // stream if necessary.
        if (ind1 == stackix) {
          assertx(srcStack[ind1].op == Op::Concat);
          auto const back = &m_working.back();
          back->stackix--;
          if (m_working.size() >= 2) {
            auto const prev = back - 1;
            if (back->stackix == prev->stackix) {
              prev->stream.insert(prev->stream.end(),
                                  back->stream.begin(), back->stream.end());
              prev->concats += back->concats;
              m_working.pop_back();
            }
          }
          return push_back(op, ASKind::Concat);
        }
      }

      squashTo(nstack - 2);
      // Start a new stream.
      m_working.emplace_back(nstack - 2, Op::Concat);
      FTRACE(2,
             "New stream: working-size={}, stackix={}",
             m_working.size(), nstack - 2);
      return push_back(op, ASKind::Concat);
    }
    case Op::AddElemC:
    case Op::AddNewElemC: {
      if (!squashAddElem) break;
      auto const& type = stack.back().type;

      // finish any streams in progress for the key or value
      auto const stackix = squashTo(stack.size());

      // Attempt to avoid O(n^2) time complexity of tvNonStatic. If
      // the array prior to the AddElem is one element smaller than
      // the array after the addelem, we can just append the last
      // element of the new array (which will almost certainly be
      // represented as a MapElems) to the old one, which is amortized
      // constant time.
      auto value = [&] () -> folly::Optional<Cell> {
        if (stackix != stack.size() - 1) return folly::none;
        auto& cur = m_working.back().addElemResult;
        if (cur.isNull()) return folly::none;
        auto sz = array_size(type);
        if (!sz) return folly::none;
        auto const arr = cur.asTypedValue()->m_data.parr;
        if (arr->size() + 1 != *sz) return folly::none;
        auto const last = array_get_by_index(type, -1);
        if (!last || !last->first.subtypeOf(BArrKey)) return folly::none;
        auto const key = tv(last->first);
        auto const val = tv(last->second);
        if (!key || !val) return folly::none;
        if (op.op == Op::AddElemC) {
          cur.asTypedValue()->m_data.parr = arr->set(*key, *val);
        } else {
          cur.asTypedValue()->m_data.parr = arr->append(*val);
        }
        cur.asTypedValue()->m_data.parr->incRefCount();
        return *cur.asTypedValue();
      }();
      if (!value) value = tvNonStatic(type);

      if (!value || !isArrayLikeType(value->m_type)) {
        break;
      }

      // start a new stream if necessary
      if (stackix != stack.size() - 1 ||
          m_working.back().addElemResult.isNull()) {
        squashTo(stack.size() - 1);
        // Start a new stream.
        m_working.emplace_back(stack.size() - 1, op.op);
      }

      auto& working = m_working.back();
      if (!working.addElemResult.isNull()) {
        for (auto i = working.stream.size(); i--; ) {
          if (working.stream[i].second == ASKind::AddElem) {
            auto numToPop = working.stream[i].first.numPop() - 1;
            assertx(numToPop == 1 || numToPop == 2);
            auto pop = std::make_pair(
              bc_with_loc(working.stream[i].first.srcLoc, bc::PopC{}),
              ASKind::Normal
            );
            working.stream[i] = pop;
            if (numToPop == 2) {
              working.stream.emplace(working.stream.begin() + i + 1, pop);
            }
            break;
          }
        }
      }
      working.addElemResult = std::move(tvAsVariant(&*value));
      working.generator = op.op;
      return push_back(op, ASKind::AddElem);
    }
    default:
      break;
  }

  squashTo(nstack - op.numPop());
  push_back(op);
}

/*
 * Push to the innermost stream.
 */
void AppendPeephole::push_back(const Bytecode& op, ASKind kind) {
  if (m_working.empty()) {
    m_next.push_back(op);
  } else {
    auto& inner = m_working.back();

    if (kind == ASKind::Concat) ++inner.concats;
    inner.stream.emplace_back(op, kind);
  }
}

int AppendPeephole::squashAbove(const std::vector<PeepholeStackElem>& srcStack,
                                int depth,
                                InterpStack* stack) {
  while (!m_working.empty()) {
    auto const &b = m_working.back();
    if (depth > b.stackix && srcStack[b.stackix].op == b.generator) {
      return b.stackix;
    }
    // something overwrote or popped the result of the Concat, so we're done.
    FTRACE(5, "AppendPeephole - squash: stackix={} op={}\n",
           b.stackix, opcodeToName(srcStack[b.stackix].op));
    squash(stack);
  }
  return -1;
}

/*
 * Reorder and rewrite the most nested concat subsequence, and append it to
 * the previous subsequence in the stack.
 */
void AppendPeephole::squash(InterpStack* stack) {
  assert(!m_working.empty());

  auto workstream = std::move(m_working.back());
  m_working.pop_back();

  // Concat counters.
  uint32_t naccum = 1;
  int ntotal = 0;

  if (workstream.generator == Op::Concat) {
    assert(workstream.stream.front().first.op == Op::Concat);

    for (auto& item : workstream.stream) {
      auto const& op = item.first;
      assertx(item.second != ASKind::AddElem);

      // If we passed the last concat, just append the remaining bytecode.
      if (ntotal < workstream.concats && item.second == ASKind::Concat) {
        // Bump counters.
        ++naccum;
        ++ntotal;

        // Emit a ConcatN if we hit the limit, or if we hit the final Concat.
        if (naccum == kMaxConcatN || ntotal == workstream.concats) {
          if (naccum >= 2) {
            if (naccum == 2) {
              push_back(bc_with_loc(op.srcLoc, bc::Concat {}));
            } else {
              push_back(bc_with_loc(op.srcLoc, bc::ConcatN {naccum}));
            }
          }
          naccum = 1;
        }
        continue;
      }

      push_back(op);
    }
    return;
  }

  assertx(workstream.generator == Op::AddElemC ||
          workstream.generator == Op::AddNewElemC);

  for (auto& item : workstream.stream) {
    if (item.second != ASKind::AddElem) {
      assertx(item.second == ASKind::Normal);
      push_back(item.first);
      continue;
    }
    auto numPop = item.first.numPop();
    assertx(numPop == 2 || numPop == 3);
    auto pop = bc_with_loc(item.first.srcLoc, bc::PopC{});
    while (numPop--) {
      push_back(pop);
    }
    assertx(isArrayLikeType(workstream.addElemResult.getRawType()));
    workstream.addElemResult.setEvalScalar();
    if (stack && stack->size() > workstream.stackix) {
      (*stack)[workstream.stackix].type =
        from_cell(*workstream.addElemResult.toCell());
    }
    push_back(
      bc_with_loc(item.first.srcLoc,
                  gen_constant(*workstream.addElemResult.toCell()))
    );
  }
}

//////////////////////////////////////////////////////////////////////

}}
