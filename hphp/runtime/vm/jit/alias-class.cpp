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
#include "hphp/runtime/vm/jit/alias-class.h"

#include <limits>
#include <algorithm>

#include <folly/Hash.h>
#include <folly/Format.h>
#include <folly/Bits.h>

#include "hphp/util/safe-cast.h"

#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/analysis.h"

namespace HPHP { namespace jit {

namespace {

//////////////////////////////////////////////////////////////////////

struct StkPtrInfo {
  SSATmp* fp;
  int32_t offset;
};

/*
 * For a StkPtr, determine the frame pointer it is part of, and the logical
 * offset (in cells) from that frame.  ("logical" because in a generator this
 * number is not the actual difference between the two pointers.)
 */
StkPtrInfo canonicalize_stkptr(SSATmp* sp) {
  assert(sp->type() <= Type::StkPtr);

  auto const inst = sp->inst();
  switch (inst->op()) {
  case DefSP:
    return StkPtrInfo { inst->src(0), -inst->extra<DefSP>()->offset };
  case ReDefSP:
    return StkPtrInfo { inst->src(1), -inst->extra<ReDefSP>()->spOffset };
  case RetAdjustStack:
    return StkPtrInfo { inst->src(0), 2 };

  case AdjustSP:
    {
      auto const prev = canonicalize_stkptr(inst->src(0));
      return StkPtrInfo {
        prev.fp,
        prev.offset + inst->extra<AdjustSP>()->offset
      };
    }

  case Call:
    {
      auto const prev = canonicalize_stkptr(inst->src(0));
      auto const extra = inst->extra<Call>();
      return StkPtrInfo {
        prev.fp,
        prev.offset + extra->spOffset +
          safe_cast<int32_t>(extra->numParams) +
          int32_t{kNumActRecCells} - 1
      };
    }

  case CallArray:
    {
      auto const prev = canonicalize_stkptr(inst->src(0));
      auto const extra = inst->extra<CallArray>();
      return StkPtrInfo {
        prev.fp,
        prev.offset + extra->spOffset + int32_t{kNumActRecCells} + 1 - 1
      };
    }

  case ContEnter:
    {
      auto const prev = canonicalize_stkptr(inst->src(0));
      auto const extra = inst->extra<ContEnter>();
      return StkPtrInfo {
        prev.fp,
        prev.offset + extra->spOffset + 1 - 1
      };
    }

  default:
    always_assert_flog(false, "unexpected StkPtr: {}\n", sp->toString());
  }
}

AStack canonicalize_stk(AStack stk) {
  if (stk.base->type() <= Type::FramePtr) return stk;
  auto const info = canonicalize_stkptr(stk.base);
  return AStack { info.fp, stk.offset + info.offset, stk.size };
}

//////////////////////////////////////////////////////////////////////

// Helper for returning the lowest index of an AStack range, non inclusive.
// I.e. a AStack class affects stack slots in [sp+offset,lowest_offset).
int32_t lowest_offset(AStack stk) {
  auto const off    = int64_t{stk.offset};
  auto const sz     = int64_t{stk.size};
  auto const low    = off - sz;
  auto const i32min = int64_t{std::numeric_limits<int32_t>::min()};
  return safe_cast<int32_t>(std::max(low, i32min));
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

size_t AliasClass::Hash::operator()(AliasClass acls) const {
  auto const hash = folly::hash::twang_mix64(
    acls.m_bits | static_cast<uint32_t>(acls.m_stag)
  );
  switch (acls.m_stag) {
  case STag::None:
    return hash;
  case STag::Frame:
    return folly::hash::hash_combine(hash,
                                     acls.m_frame.fp,
                                     acls.m_frame.id);
  case STag::Prop:
    return folly::hash::hash_combine(hash,
                                     acls.m_prop.obj,
                                     acls.m_prop.offset);
  case STag::ElemI:
    return folly::hash::hash_combine(hash,
                                     acls.m_elemI.arr,
                                     acls.m_elemI.idx);
  case STag::ElemS:
    return folly::hash::hash_combine(hash,
                                     acls.m_elemS.arr,
                                     acls.m_elemS.key->hash());
  case STag::Stack:
    return folly::hash::hash_combine(hash,
                                     acls.m_stack.base,
                                     acls.m_stack.offset,
                                     acls.m_stack.size);
  }
  not_reached();
}

//////////////////////////////////////////////////////////////////////

#define X(What, what)                                 \
  AliasClass::AliasClass(A##What x)                   \
    : m_bits(B##What)                                 \
    , m_stag(STag::What)                              \
    , m_##what(x)                                     \
  {                                                   \
    assert(checkInvariants());                        \
  }                                                   \
                                                      \
  folly::Optional<A##What> AliasClass::what() const { \
    if (m_stag == STag::What) return m_##what;        \
    return folly::none;                               \
  }

X(Frame, frame)
X(Prop, prop)
X(ElemI, elemI)
X(ElemS, elemS)
X(Stack, stack)

#undef X

bool AliasClass::checkInvariants() const {
  // For now we only allow stags when there is only a single bit set in the
  // class.  Note: operator<= is making use of this invariant.
  if (m_stag != STag::None) {
    assert(folly::popcount(uint32_t{m_bits}) == 1);
  }

  switch (m_stag) {
  case STag::None:    break;
  case STag::Frame:   break;
  case STag::Prop:    break;
  case STag::ElemI:   break;
  case STag::Stack:
    assert(m_stack.base->type() <= Type::StkPtr ||
           m_stack.base->type() <= Type::FramePtr);
    assert(m_stack.size != 0);  // use AEmpty if you want that
    assert(m_stack.size > 0);
    break;
  case STag::ElemS:
    assert(m_elemS.key->isStatic());
    break;
  }
  return true;
}

bool AliasClass::equivData(AliasClass o) const {
  assert(m_stag == o.m_stag);
  switch (m_stag) {
  case STag::None:    return true;
  case STag::Frame:   return m_frame.fp == o.m_frame.fp &&
                             m_frame.id == o.m_frame.id;
  case STag::Prop:    return m_prop.obj == o.m_prop.obj &&
                             m_prop.offset == o.m_prop.offset;
  case STag::ElemI:   return m_elemI.arr == o.m_elemI.arr &&
                             m_elemI.idx == o.m_elemI.idx;
  case STag::ElemS:   return m_elemS.arr == o.m_elemS.arr &&
                             m_elemS.key == o.m_elemS.key;
  case STag::Stack:   return m_stack.base == o.m_stack.base &&
                             m_stack.offset == o.m_stack.offset &&
                             m_stack.size == o.m_stack.size;
  }
  not_reached();
}

bool AliasClass::operator==(AliasClass o) const {
  return m_bits == o.m_bits &&
         m_stag == o.m_stag &&
         equivData(o);
}

AliasClass AliasClass::operator|(AliasClass o) const {
  if (o <= *this) return *this;
  if (*this <= o) return o;
#define X(x) if (o <= x && *this <= x) return x;
  X(AStackAny)
  X(AElemIAny)
  X(AElemAny)
  X(APropAny)
  X(AHeapAny)
  X(ANonFrame)
  X(ANonStack)
#undef X
  return AUnknown;
}

bool AliasClass::subclassData(AliasClass o) const {
  switch (m_stag) {
  case STag::None:
  case STag::Frame:
  case STag::Prop:
  case STag::ElemI:
  case STag::ElemS:
    return equivData(o);
  case STag::Stack:
    if (m_stack.base != o.m_stack.base) return false;
    return m_stack.offset <= o.m_stack.offset &&
           lowest_offset(m_stack) >= lowest_offset(o.m_stack);
  }
  not_reached();
}

bool AliasClass::subclassDataDisjoint(AliasClass o) const {
  if (o.m_stag == STag::None) return true;
  return false;
}

bool AliasClass::operator<=(AliasClass o) const {
  if (m_bits == BEmpty) return true;

  auto const isect = static_cast<rep>(m_bits & o.m_bits);
  if (isect != m_bits) return false;
  if (folly::popcount(uint32_t{o.m_bits}) == 1) {
    if (m_stag != o.m_stag) return subclassDataDisjoint(o);
    return subclassData(o);
  }
  return true;
}

bool AliasClass::maybeData(AliasClass o) const {
  assert(m_stag == o.m_stag);
  switch (m_stag) {
  case STag::None:
    not_reached();  // handled above
  case STag::Frame:
    return m_frame.fp == o.m_frame.fp && m_frame.id == o.m_frame.id;
  case STag::Prop:
    /*
     * We can't tell if two objects could be the same from here in general, but
     * we can rule out simple cases based on type.  The props can't be the same
     * if they are at different offsets, though.
     *
     * For now we're ignoring the type information, and only using offset.
     * TODO(#2939547) TODO(#2884927)
     */
    if (m_prop.offset != o.m_prop.offset) return false;
    return true;

  /*
   * Two arrays can generally be the same even if they aren't the same SSATmp,
   * because we might have loaded it from more than one place, and we have
   * linear chains in array modification instructions.
   */
  case STag::ElemI:
    if (m_elemI.idx != o.m_elemI.idx) return false;
    return true;
  case STag::ElemS:
    if (m_elemS.key != o.m_elemS.key) return false;
    return true;

  /*
   * Stack offsets that have different StkPtr bases are always presumed to
   * possibly alias.  If both are FramePtrs but different, they can't alias.
   * Stack offsets on the same base pointer may only alias if they have
   * overlapping ranges.
   */
  case STag::Stack:
    if (m_stack.base != o.m_stack.base) {
      return !(m_stack.base->type() <= Type::FramePtr &&
               o.m_stack.base->type() <= Type::FramePtr);
    }
    {
      // True if there's a non-empty intersection of the two stack slot
      // intervals.
      auto const lowest_upper = std::min(m_stack.offset, o.m_stack.offset);
      auto const highest_lower = std::max(
        lowest_offset(m_stack),
        lowest_offset(o.m_stack)
      );
      return lowest_upper > highest_lower;
    }
  }
  not_reached();
}

bool AliasClass::maybeDataDisjoint(AliasClass o) const {
  assert(m_stag != o.m_stag);
  if (m_stag == STag::None || o.m_stag == STag::None) return true;

  switch (m_stag) {
  case STag::None:
    not_reached();
  case STag::Frame:
  case STag::Prop:
  case STag::Stack:
    return false;
  case STag::ElemI:
  case STag::ElemS:
    /*
     * TODO(#2884927): we probably need to be aware of possibly-integer-like
     * string keys here before we can start using ElemS for anything.  (Or else
     * ensure that we never use ElemS with an integer-like string.)
     */
    return false;
  }

  not_reached();
}

bool AliasClass::maybe(AliasClass o) const {
  auto const isect = static_cast<rep>(m_bits & o.m_bits);
  if (isect == 0) return false;
  if (*this <= o || o <= *this) return true;
  if (folly::popcount(uint32_t{o.m_bits}) == 1) {
    if (m_stag != o.m_stag) return maybeDataDisjoint(o);
    return maybeData(o);
  }
  return true;
}

//////////////////////////////////////////////////////////////////////

AliasClass canonicalize(AliasClass a) {
  if (auto const x = a.prop())  return AProp { canonical(x->obj), x->offset };
  if (auto const x = a.elemI()) return AElemI { canonical(x->arr), x->idx };
  if (auto const x = a.elemS()) return AElemS { canonical(x->arr), x->key };
  if (auto const x = a.stack()) return canonicalize_stk(*x);
  return a;
}

//////////////////////////////////////////////////////////////////////

std::string show(AliasClass acls) {
  auto ret = std::string{};
  using A  = AliasClass;

  switch (acls.m_bits) {
  case A::BEmpty:     ret = "empty";    break;
  case A::BNonFrame:  ret = "nonframe"; break;
  case A::BNonStack:  ret = "nonstack"; break;
  case A::BHeap:      ret = "heap";     break;
  case A::BUnknown:   ret = "unk";      break;
  case A::BFrame:     ret = "frame";    break;
  case A::BProp:      ret = "prop";     break;
  case A::BElemI:     ret = "elemI";    break;
  case A::BElemS:     ret = "elemS";    break;
  case A::BElem:      ret = "elem";     break;
  case A::BStack:     ret = "stk";      break;
  }

  switch (acls.m_stag) {
  case A::STag::None:
    break;
  case A::STag::Frame:
    folly::format(&ret, " t{}:{}", acls.m_frame.fp->id(), acls.m_frame.id);
    break;
  case A::STag::Prop:
    folly::format(&ret, " t{}:{}", acls.m_prop.obj->id(), acls.m_prop.offset);
    break;
  case A::STag::ElemI:
    folly::format(&ret, " t{}:{}", acls.m_elemI.arr->id(), acls.m_elemI.idx);
    break;
  case A::STag::ElemS:
    folly::format(&ret, " t{}:{.10}", acls.m_elemS.arr->id(),
      acls.m_elemS.key);
    break;
  case A::STag::Stack:
    folly::format(&ret, " t{}:{}{}",
      acls.m_stack.base->id(),
      acls.m_stack.offset,
      acls.m_stack.size == std::numeric_limits<int32_t>::max()
        ? "<"
        : folly::sformat(";{}", acls.m_stack.size)
    );
    break;
  }

  return ret;
}

//////////////////////////////////////////////////////////////////////

}}
