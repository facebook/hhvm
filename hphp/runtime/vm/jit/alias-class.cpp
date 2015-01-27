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
#include <bitset>

#include <folly/Hash.h>
#include <folly/Format.h>

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
    return StkPtrInfo { inst->src(0), -inst->extra<ReDefSP>()->offset };
  case RetAdjustStk:
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

std::string bit_str(AliasClass::rep bits, AliasClass::rep skip) {
  using A = AliasClass;

  switch (bits) {
  case A::BEmpty:    return "Empty";
  case A::BNonFrame: return "!Fr";
  case A::BNonStack: return "!St";
  case A::BHeap:     return "Heap";
  case A::BUnknown:  return "Unk";
  case A::BElem:     return "Elem";
  case A::BFrame:    break;
  case A::BProp:     break;
  case A::BElemI:    break;
  case A::BElemS:    break;
  case A::BStack:    break;
  }

  auto ret = std::string{};
  auto const bset = std::bitset<32>{bits};
  for (auto i = 0; i < 32; ++i) {
    if (!bset.test(i)) continue;
    if (1ul << i == skip) continue;
    switch (1ul << i) {
    case A::BEmpty:
    case A::BNonFrame:
    case A::BNonStack:
    case A::BHeap:
    case A::BUnknown:
    case A::BElem:
      always_assert(0);
    case A::BFrame:  ret += "Fr"; break;
    case A::BProp:   ret += "Pr"; break;
    case A::BElemI:  ret += "Ei"; break;
    case A::BElemS:  ret += "Es"; break;
    case A::BStack:  ret += "St"; break;
    }
  }
  return ret;
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

#define X(What, what)                                       \
  AliasClass::AliasClass(A##What x)                         \
    : m_bits(B##What)                                       \
    , m_stag(STag::What)                                    \
    , m_##what(x)                                           \
  {                                                         \
    assert(checkInvariants());                              \
  }                                                         \
                                                            \
  folly::Optional<A##What> AliasClass::what() const {       \
    if (m_stag == STag::What) return m_##what;              \
    return folly::none;                                     \
  }                                                         \
                                                            \
  folly::Optional<A##What> AliasClass::is_##what() const {  \
    if (*this <= A##What##Any) return what();               \
    return folly::none;                                     \
  }

X(Frame, frame)
X(Prop, prop)
X(ElemI, elemI)
X(ElemS, elemS)
X(Stack, stack)

#undef X

AliasClass::rep AliasClass::stagBit(STag tag) {
  switch (tag) {
  case STag::None:    return BEmpty;
  case STag::Frame:   return BFrame;
  case STag::Prop:    return BProp;
  case STag::ElemI:   return BElemI;
  case STag::ElemS:   return BElemS;
  case STag::Stack:   return BStack;
  }
  always_assert(0);
}

bool AliasClass::checkInvariants() const {
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

  assert(m_bits & stagBit(m_stag));

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

AliasClass AliasClass::unionData(rep newBits, AliasClass a, AliasClass b) {
  assert(a.m_stag == b.m_stag);
  switch (a.m_stag) {
  case STag::None:
    break;
  case STag::Frame:
  case STag::Prop:
  case STag::ElemI:
  case STag::ElemS:
    assert(!a.equivData(b));
    break;

  case STag::Stack:
    {
      auto const stkA = a.m_stack;
      auto const stkB = b.m_stack;

      // If two AStack have different bases, we can't union them any better
      // than AStackAny, since we don't know where they are relative to each
      // other.  We know two AStacks with different FramePtr bases can't alias,
      // but that doesn't help us represent a union of them.
      if (stkA.base != stkB.base) return AliasClass{newBits};

      // Make a stack range big enough to contain both of them.
      auto const highest = std::max(stkA.offset, stkB.offset);
      auto const lowest = std::min(lowest_offset(stkA), lowest_offset(stkB));
      auto const newStack = AStack {
        stkA.base,
        highest,
        highest - lowest
      };
      auto ret = AliasClass{newBits};
      new (&ret.m_stack) AStack(newStack);
      ret.m_stag = STag::Stack;
      assert(ret.checkInvariants());
      assert(a <= ret && b <= ret);
      return ret;
    }
  }

  return AliasClass{newBits};
}

AliasClass AliasClass::operator|(AliasClass o) const {
  if (o <= *this) return *this;
  if (*this <= o) return o;

  auto const unioned = static_cast<rep>(m_bits | o.m_bits);

  // Note: union operations are guaranteed to be commutative, so if there are
  // two non-None stags, we have to consistently choose between them.  For now
  // we throw both away in any case where they differ, and try to merge them
  // with unionData if they are the same.
  auto const stag1 = m_stag;
  auto const stag2 = o.m_stag;
  if (stag1 == stag2) return unionData(unioned, *this, o);
  if (stag1 != STag::None && stag2 != STag::None) {
    return AliasClass{unioned};
  }
  if (stag2 != STag::None) return o | *this;

  auto ret = AliasClass{unioned};
  switch (stag1) {
  case STag::None:
    break;
  case STag::Frame:  new (&ret.m_frame) AFrame(m_frame); break;
  case STag::Prop:   new (&ret.m_prop) AProp(m_prop); break;
  case STag::ElemI:  new (&ret.m_elemI) AElemI(m_elemI); break;
  case STag::ElemS:  new (&ret.m_elemS) AElemS(m_elemS); break;
  case STag::Stack:  new (&ret.m_stack) AStack(m_stack); break;
  }
  ret.m_stag = stag1;
  return ret;
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

bool AliasClass::operator<=(AliasClass o) const {
  if (m_bits == BEmpty) return true;

  auto const isect = static_cast<rep>(m_bits & o.m_bits);
  if (isect != m_bits) return false;

  // If they have the same specialized tag, then since isect is equal to
  // m_bits, the stagBit must be part of the intersection or be BEmpty.  This
  // means they can only be in a subclass relationship if that specialized data
  // is.
  if (m_stag == o.m_stag) return subclassData(o);

  /*
   * Disjoint stags.  The stagBit for m_stag must be part of the intersection
   * (or be BEmpty), since isect == m_bits above.  This breaks down into the
   * following cases:
   *
   * If the osbit is part of the intersection, then this can't be a subclass of
   * `o', because this has only generic information for that bit but it is set
   * in isect.  If the osbit is BEmpty, osbit & isect is zero, which avoids
   * this case.
   *
   * The remaining situations are that m_stag is STag::None, in which case it
   * is a subclass since osbit wasn't in the intersection.  Or that m_stag has
   * a bit that is in the isect (since m_bits == isect), and that bit is set in
   * o.m_bits.  In either case this is a subclass, so we can just return true.
   */
  auto const osbit = stagBit(o.m_stag);
  if (osbit & isect) return false;
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

/*
 * TODO(#2884927): we probably need to be aware of possibly-integer-like string
 * keys here before we can start using ElemS for anything.  (Or else ensure
 * that we never use ElemS with an integer-like string.)
 */
bool AliasClass::maybe(AliasClass o) const {
  auto const isect = static_cast<rep>(m_bits & o.m_bits);
  if (isect == 0) return false;
  if (*this <= o || o <= *this) return true;

  /*
   * If we have the same stag, then the cases are either the stag is in the
   * intersection or not.  If it's not (including if it was BEmpty), we already
   * know the intersection is non-empty and return true.
   *
   * If it is in the intersection, and it is not the only relevant bit, then we
   * still have a non-empty intersection.
   *
   * Finally if it's in the intersection and the only isect bit, then we need
   * to see if the data is in a maybe relationship.
   */
  if (m_stag == o.m_stag) {
    auto const bit = stagBit(m_stag);
    assert(isect != 0);
    if ((bit & isect) == isect) return maybeData(o);
    return true;
  }

  /*
   * The stags are different.  However, since isect is non-empty, there are
   * three cases:
   *
   *    o One of the stag bits is not in the intersection, and thus not
   *      relevant.  Since the other stag bit is in the intersection, but
   *      didn't have specialized information from one of the intersectees, the
   *      intersection is non-empty and we should return true.
   *
   *    o Both of the stag bits are not in the intersection, and thus not
   *      relevant.  Some other bit was set, since we already checked that
   *      isect was non-zero, so we should return true.
   *
   *    o Both of the stag bits are in the intersection.  But each intersectee
   *      therefore had only generic information for the bit of the other, so
   *      the intersection is non-empty, and we should return true.
   *
   * All of these cases are handled by the following statement:
   */
  return true;
}

//////////////////////////////////////////////////////////////////////

AliasClass canonicalize(AliasClass a) {
  using T = AliasClass::STag;
  switch (a.m_stag) {
  case T::None:  return a;
  case T::Frame: return a;
  case T::Prop:  a.m_prop.obj = canonical(a.m_prop.obj);   return a;
  case T::ElemI: a.m_elemI.arr = canonical(a.m_elemI.arr); return a;
  case T::ElemS: a.m_elemS.arr = canonical(a.m_elemS.arr); return a;
  case T::Stack: a.m_stack = canonicalize_stk(a.m_stack);  return a;
  }
  always_assert(0);
}

//////////////////////////////////////////////////////////////////////

std::string show(AliasClass acls) {
  using A  = AliasClass;

  auto ret = bit_str(acls.m_bits, A::stagBit(acls.m_stag));
  if (!ret.empty() && acls.m_stag != A::STag::None) {
    ret += ' ';
  }

  switch (acls.m_stag) {
  case A::STag::None:
    break;
  case A::STag::Frame:
    folly::format(&ret, "Fr t{}:{}", acls.m_frame.fp->id(), acls.m_frame.id);
    break;
  case A::STag::Prop:
    folly::format(&ret, "Pr t{}:{}", acls.m_prop.obj->id(), acls.m_prop.offset);
    break;
  case A::STag::ElemI:
    folly::format(&ret, "Ei t{}:{}", acls.m_elemI.arr->id(), acls.m_elemI.idx);
    break;
  case A::STag::ElemS:
    folly::format(&ret, "Es t{}:{.10}", acls.m_elemS.arr->id(),
      acls.m_elemS.key);
    break;
  case A::STag::Stack:
    folly::format(&ret, "St t{}:{}{}",
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
