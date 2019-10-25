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
#include "hphp/runtime/vm/jit/alias-class.h"

#include <limits>
#include <algorithm>
#include <bitset>

#include <folly/Hash.h>
#include <folly/Format.h>

#include "hphp/util/safe-cast.h"

#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/vm/jit/dce.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/analysis.h"

namespace HPHP { namespace jit {

namespace {

//////////////////////////////////////////////////////////////////////

// Helper for returning the lowest index of an AStack range, non inclusive.
// I.e. a AStack class affects stack slots in [sp+offset,lowest_offset).
int32_t lowest_offset(AStack stk) {
  auto const off    = int64_t{stk.offset.offset};
  auto const sz     = int64_t{stk.size};
  auto const low    = off - sz;
  auto const i32min = int64_t{std::numeric_limits<int32_t>::min()};
  return safe_cast<int32_t>(std::max(low, i32min));
}

std::string bit_str(AliasClass::rep bits, AliasClass::rep skip) {
  using A = AliasClass;

  switch (bits) {
  case A::BEmpty:          return "Empty";
  case A::BHeap:           return "Heap";
  case A::BUnknownTV:      return "UnkTV";
  case A::BUnknown:        return "Unk";
  case A::BElem:           return "Elem";
  case A::BMIStateTV:      return "MisTV";
  case A::BMIState:        return "Mis";
  case A::BIter:           return "Iter";
  case A::BFrame:          break;
  case A::BIterBase:       break;
  case A::BIterType:       break;
  case A::BIterPos:        break;
  case A::BIterEnd:        break;
  case A::BProp:           break;
  case A::BElemI:          break;
  case A::BElemS:          break;
  case A::BStack:          break;
  case A::BMITempBase:     break;
  case A::BMITvRef:        break;
  case A::BMITvRef2:       break;
  case A::BMIBase:         break;
  case A::BMIPropS:        break;
  case A::BRds:            break;
  }

  auto ret = std::string{};
  auto const bset = std::bitset<32>{bits};
  for (auto i = 0; i < 32; ++i) {
    if (!bset.test(i)) continue;
    if ((1ul << i) & skip) continue;
    switch (1ul << i) {
    case A::BEmpty:
    case A::BHeap:
    case A::BUnknown:
    case A::BUnknownTV:
    case A::BElem:
    case A::BMIStateTV:
    case A::BMIState:
    case A::BIter:
      always_assert(0);
    case A::BFrame:          ret += "Fr"; break;
    case A::BIterBase:       ret += "ItB"; break;
    case A::BIterType:       ret += "ItT"; break;
    case A::BIterPos:        ret += "ItP"; break;
    case A::BIterEnd:        ret += "ItE"; break;
    case A::BProp:           ret += "Pr"; break;
    case A::BElemI:          ret += "Ei"; break;
    case A::BElemS:          ret += "Es"; break;
    case A::BStack:          ret += "St"; break;
    case A::BMITempBase:     ret += "MiTB"; break;
    case A::BMITvRef:        ret += "MiT1"; break;
    case A::BMITvRef2:       ret += "MiT2"; break;
    case A::BMIBase:         ret += "MiB"; break;
    case A::BMIPropS:        ret += "MiP"; break;
    case A::BRds:            ret += "Rds"; break;
    }
  }
  return ret;
}

//////////////////////////////////////////////////////////////////////

template<class T>
size_t framelike_hash(size_t hash, T t) {
  return folly::hash::hash_combine(hash, t.base.offset, t.id);
}

template<class T>
void framelike_checkInvariants(T t) {
  assertx(t.base.offset <= 0);
}

template<class T, class U>
bool framelike_equal(T a, U b) {
  return a.base == b.base && a.id == b.id;
}

//////////////////////////////////////////////////////////////////////

}

namespace detail {
FPRelOffset frame_base_offset(SSATmp* fp) {
  always_assert(fp->isA(TFramePtr));

  fp = canonical(fp);
  auto fpInst = fp->inst();
  if (UNLIKELY(fpInst->is(DefLabel))) fpInst = resolveFpDefLabel(fp);
  if (fpInst->is(DefFP, DefFuncEntryFP)) return FPRelOffset{0};
  always_assert(fpInst->is(DefInlineFP));
  auto const spInst = fpInst->src(0)->inst();
  assertx(spInst->is(DefFrameRelSP, DefRegSP));
  auto const offsetOfSp = spInst->extra<FPInvOffsetData>()->offset;

  // FP-relative offset of the inlined frame.
  return fpInst->extra<DefInlineFP>()->spOffset.to<FPRelOffset>(offsetOfSp);
}
}

AStack::AStack(SSATmp* fp, FPRelOffset o, int32_t s)
  : offset(o)
  , size(s)
{
  always_assert(fp->isA(TFramePtr));

  fp = canonical(fp);
  auto defInlineFP = fp->inst();
  if (UNLIKELY(defInlineFP->is(DefLabel))) defInlineFP = resolveFpDefLabel(fp);
  if (!defInlineFP->is(DefInlineFP)) return;
  auto const sp = defInlineFP->src(0)->inst();
  assertx(sp->is(DefFrameRelSP, DefRegSP));

  // FP-relative offset of the inlined frame.
  auto const innerFPRel =
    defInlineFP->extra<DefInlineFP>()->spOffset.to<FPRelOffset>(
      sp->extra<FPInvOffsetData>()->offset);

  // Offset from the outermost FP is simply the sum of (inner frame relative to
  // outer) and (offset relative to inner frame).
  offset += innerFPRel.offset;
}

AStack::AStack(SSATmp* sp, IRSPRelOffset spRel, int32_t s)
  : offset()
  , size(s)
{
  always_assert(sp->isA(TStkPtr));

  auto const defSP = sp->inst();
  always_assert_flog(defSP->is(DefFrameRelSP, DefRegSP),
                     "unexpected StkPtr: {}\n", sp->toString());
  offset = spRel.to<FPRelOffset>(defSP->extra<FPInvOffsetData>()->offset);
}

//////////////////////////////////////////////////////////////////////

size_t AliasClass::Hash::operator()(AliasClass acls) const {
  auto const hash = folly::hash::twang_mix64(
    acls.m_bits | static_cast<uint32_t>(acls.m_stag)
  );
  switch (acls.m_stag) {
  case STag::None:
    return hash;

  case STag::IterBase: return framelike_hash(hash, acls.m_iterBase);
  case STag::IterType: return framelike_hash(hash, acls.m_iterType);
  case STag::IterPos:  return framelike_hash(hash, acls.m_iterPos);
  case STag::IterEnd:  return framelike_hash(hash, acls.m_iterEnd);
  case STag::IterAll:  return framelike_hash(hash, acls.m_iterAll);

  case STag::Frame:
    return folly::hash::hash_combine(hash,
                                     acls.m_frame.base.offset,
                                     acls.m_frame.ids.raw());
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
                                     acls.m_stack.offset.offset,
                                     acls.m_stack.size);

  case STag::Rds:
    return folly::hash::hash_combine(hash, acls.m_rds.handle);
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
    assertx(checkInvariants());                             \
  }                                                         \
                                                            \
  folly::Optional<A##What> AliasClass::is_##what() const {  \
    if (*this <= A##What##Any) return what();               \
    return folly::none;                                     \
  }

X(Frame, frame)
X(IterBase, iterBase)
X(IterType, iterType)
X(IterPos, iterPos)
X(IterEnd, iterEnd)
X(Prop, prop)
X(ElemI, elemI)
X(ElemS, elemS)
X(Stack, stack)
X(Rds, rds)

#undef X

#define X(What, what)                                       \
  folly::Optional<A##What> AliasClass::what() const {       \
    if (m_stag == STag::What) return m_##what;              \
    return folly::none;                                     \
  }

X(Frame, frame)
X(Prop, prop)
X(ElemI, elemI)
X(ElemS, elemS)
X(Stack, stack)
X(Rds, rds)

#undef X

#define X(What, what)                                       \
  folly::Optional<A##What> AliasClass::what() const {       \
    if (m_stag == STag::What) return m_##what;              \
    if (m_stag == STag::IterAll) {                          \
      auto const ui = asUIter();                            \
      assertx(ui.hasValue());                               \
      return A##What { ui->base, ui->id };                  \
    }                                                       \
    return folly::none;                                     \
  }

X(IterBase, iterBase)
X(IterType, iterType)
X(IterPos, iterPos)
X(IterEnd, iterEnd)

#undef X

folly::Optional<AliasClass> AliasClass::mis() const {
  auto const bits = static_cast<rep>(m_bits & BMIState);

  if (bits != BEmpty) return AliasClass{bits};
  return folly::none;
}

folly::Optional<AliasClass> AliasClass::is_mis() const {
  if (*this <= AMIStateAny) return mis();
  return folly::none;
}

AliasClass::rep AliasClass::stagBits(STag tag) {
  switch (tag) {
  case STag::None:           return BEmpty;
  case STag::Frame:          return BFrame;
  case STag::IterBase:       return BIterBase;
  case STag::IterType:       return BIterType;
  case STag::IterPos:        return BIterPos;
  case STag::IterEnd:        return BIterEnd;
  case STag::IterAll:        return BIter;
  case STag::Prop:           return BProp;
  case STag::ElemI:          return BElemI;
  case STag::ElemS:          return BElemS;
  case STag::Stack:          return BStack;
  case STag::Rds:            return BRds;
  }
  always_assert(0);
}

bool AliasClass::checkInvariants() const {
  switch (m_stag) {
  case STag::None:           break;
  case STag::IterBase:       framelike_checkInvariants(m_iterBase); break;
  case STag::IterType:       framelike_checkInvariants(m_iterType); break;
  case STag::IterPos:        framelike_checkInvariants(m_iterPos);  break;
  case STag::IterEnd:        framelike_checkInvariants(m_iterEnd);  break;
  case STag::IterAll:        framelike_checkInvariants(m_iterAll);  break;
  case STag::Prop:           break;
  case STag::ElemI:          break;
  case STag::Frame:
    assertx(m_frame.base.offset <= 0);
    assertx(!m_frame.ids.empty());
    break;
  case STag::Stack:
    assertx(m_stack.size > 0);          // use AEmpty if you want that
    break;
  case STag::ElemS:
    assertx(m_elemS.key->isStatic());
    break;
  case STag::Rds:
    assertx(rds::isValidHandle(m_rds.handle));
    break;
  }

  assertx((m_bits & stagBits(m_stag)) == stagBits(m_stag));

  return true;
}

bool AliasClass::equivData(AliasClass o) const {
  assertx(m_stag == o.m_stag);
  switch (m_stag) {
  case STag::None:     return true;
  case STag::Frame:    return m_frame.base == o.m_frame.base &&
                              m_frame.ids == o.m_frame.ids;
  case STag::IterBase: return framelike_equal(m_iterBase, o.m_iterBase);
  case STag::IterType: return framelike_equal(m_iterType, o.m_iterType);
  case STag::IterPos:  return framelike_equal(m_iterPos, o.m_iterPos);
  case STag::IterEnd:  return framelike_equal(m_iterEnd, o.m_iterEnd);
  case STag::IterAll:  return framelike_equal(m_iterAll, o.m_iterAll);
  case STag::Prop:     return m_prop.obj == o.m_prop.obj &&
                              m_prop.offset == o.m_prop.offset;
  case STag::ElemI:    return m_elemI.arr == o.m_elemI.arr &&
                              m_elemI.idx == o.m_elemI.idx;
  case STag::ElemS:    return m_elemS.arr == o.m_elemS.arr &&
                              m_elemS.key == o.m_elemS.key;
  case STag::Stack:    return m_stack.offset == o.m_stack.offset &&
                              m_stack.size == o.m_stack.size;
  case STag::Rds:      return m_rds.handle == o.m_rds.handle;
  }
  not_reached();
}

bool AliasClass::operator==(AliasClass o) const {
  return m_bits == o.m_bits &&
         m_stag == o.m_stag &&
         equivData(o);
}

AliasClass AliasClass::unionData(rep newBits, AliasClass a, AliasClass b) {
  assertx(a.m_stag == b.m_stag);
  switch (a.m_stag) {
  case STag::None:
    break;
  case STag::IterBase:
  case STag::IterType:
  case STag::IterPos:
  case STag::IterEnd:
  case STag::Prop:
  case STag::ElemI:
  case STag::ElemS:
  case STag::Rds:
  case STag::IterAll:
    assertx(!a.equivData(b));
    break;
  case STag::Frame:
    {
      auto ret = AliasClass{newBits};
      auto const frmA = a.m_frame;
      auto const frmB = b.m_frame;
      if (frmA.base != frmB.base) return ret;

      auto const newIds = frmA.ids | frmB.ids;
      // Even when newIds.isAny(), we still know it won't alias locals in other
      // frames, so keep the specialization tag.
      ret.m_stag = STag::Frame;
      ret.m_frame = AFrame { frmA.base, newIds };
      assertx(ret.checkInvariants());
      assertx(a <= ret && b <= ret);
      return ret;
    }

  case STag::Stack:
    {
      auto const stkA = a.m_stack;
      auto const stkB = b.m_stack;

      // Make a stack range big enough to contain both of them.
      auto const highest = std::max(stkA.offset, stkB.offset);
      auto const lowest = std::min(lowest_offset(stkA), lowest_offset(stkB));
      auto const newStack = AStack { highest, highest.offset - lowest };
      auto ret = AliasClass{newBits};
      new (&ret.m_stack) AStack(newStack);
      ret.m_stag = STag::Stack;
      assertx(ret.checkInvariants());
      assertx(a <= ret && b <= ret);
      return ret;
    }
  }

  return AliasClass{newBits};
}

folly::Optional<AliasClass> AliasClass::precise_union(AliasClass o) const {
  if (o <= *this) return *this;
  if (*this <= o) return o;

  auto const unioned = static_cast<rep>(m_bits | o.m_bits);

  // For a precise union, we need to make sure the returned class is not any
  // bigger than it should be.  This means we can't deal with situations where
  // we have different stags, and right now we also don't try to deal with
  // situations that have the same stag in a combinable way.  (E.g. two
  // adjacent AStack ranges, multiple AFrame locals.)
  auto const stag1 = m_stag;
  auto const stag2 = o.m_stag;
  if (stag1 == STag::None && stag2 == STag::None) {
    return AliasClass{unioned};
  }
  if (stag1 == STag::None && stag2 != STag::None) {
    return o.precise_union(*this); // flip args
  }
  assertx(stag1 != STag::None);
  if (stag2 != STag::None) return folly::none;
  if (o.m_bits & stagBits(stag1)) return folly::none;

  // Keep the data and stag from this, but change its bits.
  auto ret = *this;
  ret.m_bits = unioned;
  assertx(ret.m_stag == stag1);
  return ret;
}

AliasClass AliasClass::operator|(AliasClass o) const {
  if (auto const c = precise_union(o)) return *c;

  auto const unioned = static_cast<rep>(m_bits | o.m_bits);

  // If they have the same stag, try to merge them with unionData.
  auto stag1 = m_stag;
  auto stag2 = o.m_stag;
  if (stag1 == stag2) return unionData(unioned, *this, o);

  // If the two alias classes are fields of the same iterator, return the
  // alias class of all fields on that iterator. This check catches disjoint
  // cases like IterBase | IterType as well as subsets like IterBase | IterAll.
  //
  // Since we're expanding the tag here pessimistically, we also have to expand
  // m_bits to include all tag bits of the IterAll alias class.
  auto const iter1 = asUIter();
  auto const iter2 = o.asUIter();
  if (iter1 && iter2 && framelike_equal(*iter1, *iter2)) {
    auto ret = AliasClass{static_cast<rep>(unioned | BIter)};
    new (&ret.m_iterAll) UIterAll(*iter1);
    ret.m_stag = STag::IterAll;
    return ret;
  }

  // If one of the alias classes have a non-None stag, we can only keep it if
  // the other doesn't have any of the corresponding bits set.
  if (stag1 != STag::None && (o.m_bits & stagBits(stag1))) stag1 = STag::None;
  if (stag2 != STag::None && (m_bits & stagBits(stag2))) stag2 = STag::None;

  auto ret = AliasClass{unioned};
  if (stag1 == stag2) return ret;       // both None.

  /*
   * Union operations are guaranteed to be commutative, so if there are two
   * non-None stags, we have to consistently choose between them if we're going
   * to keep one.  For now we keep the one with a smaller `rep' value, instead
   * of discarding both.
   *
   * We can also assume we're not in any of the situations that
   * precise_diffSTag_unionData supported, and that neither *this nor `o' are
   * subtypes of each other, because we already tried both of those things.
   *
   * Note also that we might be in a situation where one of the STags is
   * representing a union of more primitive STags.  For example we could have
   * an IterPos and an IterBoth.  But for this case, we've already thrown away
   * the overlap by setting stags to None above.
   */
  const AliasClass* best = &o;
  auto const stag = [&] () -> STag {
    if (stag1 != STag::None) {
      if (stag2 == STag::None || stagBits(stag1) < stagBits(stag2)) {
        best = this;
        return stag1;
      }
    }
    return stag2;
  }();

  switch (stag) {
  case STag::None:
    break;
  case STag::IterBase: new (&ret.m_iterBase) AIterBase(best->m_iterBase); break;
  case STag::IterType: new (&ret.m_iterType) AIterType(best->m_iterType); break;
  case STag::IterPos:  new (&ret.m_iterPos) AIterPos(best->m_iterPos); break;
  case STag::IterEnd:  new (&ret.m_iterEnd) AIterEnd(best->m_iterEnd); break;
  case STag::IterAll:  new (&ret.m_iterAll) UIterAll(best->m_iterAll); break;
  case STag::Frame:    new (&ret.m_frame) AFrame(best->m_frame); break;
  case STag::Prop:     new (&ret.m_prop) AProp(best->m_prop); break;
  case STag::ElemI:    new (&ret.m_elemI) AElemI(best->m_elemI); break;
  case STag::ElemS:    new (&ret.m_elemS) AElemS(best->m_elemS); break;
  case STag::Stack:    new (&ret.m_stack) AStack(best->m_stack); break;
  case STag::Rds:      new (&ret.m_rds) ARds(best->m_rds); break;
  }
  ret.m_stag = stag;
  return ret;
}

bool AliasClass::subclassData(AliasClass o) const {
  assertx(m_stag == o.m_stag);
  switch (m_stag) {
  case STag::None:
  case STag::IterBase:
  case STag::IterType:
  case STag::IterPos:
  case STag::IterEnd:
  case STag::IterAll:
  case STag::Prop:
  case STag::ElemI:
  case STag::ElemS:
  case STag::Rds:
    return equivData(o);
  case STag::Frame:
    return m_frame.base == o.m_frame.base && m_frame.ids <= o.m_frame.ids;
  case STag::Stack:
    return m_stack.offset <= o.m_stack.offset &&
           lowest_offset(m_stack) >= lowest_offset(o.m_stack);
  }
  not_reached();
}

folly::Optional<AliasClass::UIterAll> AliasClass::asUIter() const {
  switch (m_stag) {
  case STag::None:
  case STag::Frame:
  case STag::Prop:
  case STag::ElemI:
  case STag::ElemS:
  case STag::Rds:
  case STag::Stack:
    return folly::none;
  case STag::IterBase:  return UIterAll { m_iterBase.base, m_iterBase.id };
  case STag::IterType:  return UIterAll { m_iterType.base, m_iterType.id };
  case STag::IterPos:   return UIterAll { m_iterPos.base, m_iterPos.id };
  case STag::IterEnd:   return UIterAll { m_iterEnd.base, m_iterEnd.id };
  case STag::IterAll:   return m_iterAll;
  }
  not_reached();
}

bool AliasClass::refersToSameIterHelper(AliasClass o) const {
  assertx(stagBits(m_stag) & stagBits(o.m_stag));
  assertx(m_stag == STag::IterAll || o.m_stag == STag::IterAll);
  return framelike_equal(*asUIter(), *o.asUIter());
}

/*
 * Should return true if this's specialized data is entirely contained in o's
 * specialized data, for only the portion of the data relevant for
 * "relevant_bits", with the precondition that m_stag != o.m_stag.
 *
 * We know the only case this can happen is if at least one of them is an
 * iterator, and they are only going to have a subclass relationship inside of
 * relevant_bits only if they refer to the same iterator.
 */
bool AliasClass::diffSTagSubclassData(rep /*relevant_bits*/,
                                      AliasClass o) const {
  if (m_stag == STag::IterAll || o.m_stag == STag::IterAll) {
    return refersToSameIterHelper(o);
  } else {
    return false;
  }
}

/*
 * This function conceptually should check that the portions of the specialized
 * information on the intersection of the stagBits for the two classes may
 * overlap.
 *
 * Again since we only deal with iterators, the intersection must be iterator
 * information, and we know it only intersects if they are the same iterator
 * id.
 */
bool AliasClass::diffSTagMaybeData(rep /*relevant_bits*/, AliasClass o) const {
  if (m_stag == STag::IterAll || o.m_stag == STag::IterAll) {
    return refersToSameIterHelper(o);
  } else {
    return false;
  }
}

bool AliasClass::operator<=(AliasClass o) const {
  if (m_bits == BEmpty) return true;

  auto const isect = static_cast<rep>(m_bits & o.m_bits);
  if (isect != m_bits) return false;

  // If they have the same specialized tag, then since isect is equal to
  // m_bits, the stagBits must be part of the intersection or be BEmpty.  This
  // means they can only be in a subclass relationship if that specialized data
  // is.
  if (m_stag == o.m_stag) return subclassData(o);

  auto const sbits  = stagBits(m_stag);
  auto const osbits = stagBits(o.m_stag);

  /*
   * If the stag bits for the two classes overlap, but the stags were
   * different, we're dealing with a union-style STag (like IterAll).
   */
  if (auto const inner_bits = (sbits & osbits /*& isect is redundant*/)) {
    return diffSTagSubclassData(static_cast<rep>(inner_bits), o);
  }

  /*
   * Different stags, with non-overlapping bits.  The sbits must be part of the
   * intersection (or be BEmpty), since isect == m_bits above, but osbits may
   * or may not be.  So this breaks down into the following cases:
   *
   * If the osbits are part of the intersection, then this can't be a subclass
   * of `o', because this has only generic information for that bit but it is
   * set in isect.  If the osbits was BEmpty, osbits & isect is zero, which
   * avoids this case.
   *
   * The remaining situations are that m_stag is STag::None, in which case it
   * is a subclass since osbits wasn't in the intersection.  Or that m_stag has
   * a bit that is in the isect (since m_bits == isect), and that bit is set in
   * o.m_bits.  In either case this is a subclass, so we can just return true.
   */
  if (osbits & isect) return false;
  return true;
}

bool AliasClass::maybeData(AliasClass o) const {
  assertx(m_stag == o.m_stag);
  switch (m_stag) {
  case STag::None:
    not_reached();  // handled outside
  case STag::IterBase: return framelike_equal(m_iterBase, o.m_iterBase);
  case STag::IterType: return framelike_equal(m_iterBase, o.m_iterType);
  case STag::IterPos:  return framelike_equal(m_iterPos, o.m_iterPos);
  case STag::IterEnd:  return framelike_equal(m_iterPos, o.m_iterEnd);
  case STag::IterAll:  return framelike_equal(m_iterAll, o.m_iterAll);
  case STag::Frame:
    return m_frame.base == o.m_frame.base && m_frame.ids.maybe(o.m_frame.ids);
  case STag::Prop:
    /*
     * We can't tell if two objects could be the same from here in general, but
     * we can rule out simple cases based on type.  The props can't be the same
     * if they are at different offsets, though.
     */
    if (m_prop.offset != o.m_prop.offset) return false;
    if (!m_prop.obj->type().maybe(o.m_prop.obj->type())) return false;
    return true;

  /*
   * Two arrays can generally be the same even if they aren't the same SSATmp,
   * because we might have loaded it from more than one place, and we have
   * linear chains in array modification instructions.
   */
  case STag::ElemI:
    if (m_elemI.idx != o.m_elemI.idx) return false;
    if (!m_elemI.arr->type().maybe(o.m_elemI.arr->type())) return false;
    return true;
  case STag::ElemS:
    if (m_elemS.key != o.m_elemS.key) return false;
    if (!m_elemS.arr->type().maybe(o.m_elemS.arr->type())) return false;
    return true;

  case STag::Stack:
    {
      // True if there's a non-empty intersection of the two stack slot
      // intervals.
      auto const lowest_upper = std::min(m_stack.offset, o.m_stack.offset);
      auto const highest_lower = std::max(
        lowest_offset(m_stack),
        lowest_offset(o.m_stack)
      );
      return lowest_upper.offset > highest_lower;
    }

  case STag::Rds:
    return m_rds.handle == o.m_rds.handle;
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

  auto const sbits = stagBits(m_stag);
  auto const osbits = stagBits(o.m_stag);

  /*
   * If a shared portion of the specialized information is in the intersection,
   * and the intersection is otherwise empty, we can just check if the data
   * associated with the intersecting bits is in a maybe relationship.
   */
  if (auto const inner_bits = (sbits & osbits & isect)) {
    if ((inner_bits & isect) == isect) {
      if (m_stag == o.m_stag) return maybeData(o);
      return diffSTagMaybeData(static_cast<rep>(inner_bits), o);
    }
  }

  // Otherwise, the intersection is non-empty and has no specialized
  // information inside the intersecting portion to consult.  So we know they
  // overlap.
  return true;
}

bool AliasClass::isSingleLocation() const {
  // Either BEmpty or more than one bit set in rep.
  if (m_bits == 0 || (m_bits & (m_bits - 1))) return false;

  // Only MIState is allowed to have no specialization and be a valid single
  // location.
  if (m_stag == STag::None) return *this <= AMIStateAny;

  // AFrame and AStack can contain multiple locations.
  if (auto const frame = is_frame()) {
    return frame->ids.hasSingleValue();
  }
  if (auto const stk = is_stack()) {
    return stk->size == 1;
  }
  // All other specializations currently have exactly one location.
  return true;
}

//////////////////////////////////////////////////////////////////////

AliasClass mis_from_offset(size_t offset) {
  if (offset == offsetof(MInstrState, tvTempBase)) {
    return AliasClass{AliasClass::BMITempBase};
  }
  if (offset == offsetof(MInstrState, tvRef)) {
    return AliasClass{AliasClass::BMITvRef};
  }
  if (offset == offsetof(MInstrState, tvRef2)) {
    return AliasClass{AliasClass::BMITvRef2};
  }
  if (offset == offsetof(MInstrState, base)) {
    return AliasClass{AliasClass::BMIBase};
  }
  always_assert(false);
}

//////////////////////////////////////////////////////////////////////

AliasClass canonicalize(AliasClass a) {
  using T = AliasClass::STag;
  switch (a.m_stag) {
  case T::None:           return a;
  case T::Frame:          return a;
  case T::IterBase:       return a;
  case T::IterType:       return a;
  case T::IterPos:        return a;
  case T::IterEnd:        return a;
  case T::IterAll:        return a;
  case T::Stack:          return a;
  case T::Rds:            return a;
  case T::Prop:     a.m_prop.obj = canonical(a.m_prop.obj);   return a;
  case T::ElemI:    a.m_elemI.arr = canonical(a.m_elemI.arr); return a;
  case T::ElemS:    a.m_elemS.arr = canonical(a.m_elemS.arr); return a;
  }
  not_reached();
}

//////////////////////////////////////////////////////////////////////

std::string show(AliasClass acls) {
  using A  = AliasClass;

  auto ret = bit_str(acls.m_bits, A::stagBits(acls.m_stag));
  if (!ret.empty() && acls.m_stag != A::STag::None) {
    ret += ' ';
  }

  switch (acls.m_stag) {
  case A::STag::None:
    break;
  case A::STag::Frame:
    folly::format(&ret, "Fr {}:{}", acls.m_frame.base.offset,
                  show(acls.m_frame.ids));
    break;
  case A::STag::IterBase:
    folly::format(&ret, "ItB {}:{}", acls.m_iterBase.base.offset,
                                     acls.m_iterBase.id);
    break;
  case A::STag::IterType:
    folly::format(&ret, "ItT {}:{}", acls.m_iterType.base.offset,
                                     acls.m_iterType.id);
    break;
  case A::STag::IterPos:
    folly::format(&ret, "ItP {}:{}", acls.m_iterPos.base.offset,
                                     acls.m_iterPos.id);
    break;
  case A::STag::IterEnd:
    folly::format(&ret, "ItE {}:{}", acls.m_iterEnd.base.offset,
                                     acls.m_iterEnd.id);
    break;
  case A::STag::IterAll:
    folly::format(&ret, "It* {}:{}", acls.m_iterAll.base.offset,
                                     acls.m_iterAll.id);
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
    folly::format(&ret, "St {}{}",
      acls.m_stack.offset.offset,
      acls.m_stack.size == std::numeric_limits<int32_t>::max()
        ? "<"
        : folly::sformat(";{}", acls.m_stack.size)
    );
    break;
  case A::STag::Rds:
    folly::format(&ret, "Rds {}", acls.m_rds.handle);
    break;
  }

  return ret;
}

//////////////////////////////////////////////////////////////////////

}}
