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
  case A::BMIState:        return "Mis";
  case A::BActRec:         return "ActRec";
  case A::BLocal:          break;
  case A::BIter:           break;
  case A::BProp:           break;
  case A::BElemI:          break;
  case A::BElemS:          break;
  case A::BStack:          break;
  case A::BMITempBase:     break;
  case A::BMIBase:         break;
  case A::BRds:            break;
  case A::BFContext:       break;
  case A::BFFunc:          break;
  case A::BFMeta:          break;
  case A::BFBasePtr:       break;
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
    case A::BMIState:
    case A::BActRec:
      always_assert(0);
    case A::BLocal:          ret += "Lv"; break;
    case A::BIter:           ret += "It"; break;
    case A::BProp:           ret += "Pr"; break;
    case A::BElemI:          ret += "Ei"; break;
    case A::BElemS:          ret += "Es"; break;
    case A::BStack:          ret += "St"; break;
    case A::BMITempBase:     ret += "MiTB"; break;
    case A::BMIBase:         ret += "MiB"; break;
    case A::BRds:            ret += "Rds"; break;
    case A::BFContext:       ret += "Fc"; break;
    case A::BFFunc:          ret += "Ff"; break;
    case A::BFMeta:          ret += "Fm"; break;
    case A::BFBasePtr:       ret += "FrBase"; break;
    }
  }
  return ret;
}

//////////////////////////////////////////////////////////////////////

template<typename T>
size_t framelike_hash(size_t hash, T t) {
  return folly::hash::hash_combine(hash, t.base.offset, t.ids.raw());
}

template<typename T>
void framelike_checkInvariants(T t) {
  assertx(t.base.offset <= 0);
  assertx(!t.ids.empty());
}

template<typename T>
bool framelike_equal(T a, T b) {
  return a.base == b.base && a.ids == b.ids;
}

template<typename T>
bool framelike_subclass(T a, T b) {
  return a.base == b.base && a.ids <= b.ids;
}

template<typename T>
bool framelike_maybe(T a, T b) {
  return a.base == b.base && a.ids.maybe(b.ids);
}

//////////////////////////////////////////////////////////////////////

}

template<typename T>
AliasClass AliasClass::framelike_union(rep newBits, rep frm, T a, T b) {
  auto ret = AliasClass{newBits};
  auto const frmA = a;
  auto const frmB = b;
  if (frmA.base != frmB.base) return ret;

  auto const newIds = frmA.ids | frmB.ids;
  // Even when newIds.isAny(), we still know it won't alias things in
  // other frames, so keep the specialization tag.
  ret.framelike_union_set(T { frmA.base, newIds });
  ret.m_stagBits = static_cast<rep>(ret.m_stagBits | frm);
  assertx(ret.checkInvariants());
  assertx(AliasClass{ a } <= ret && AliasClass { b } <= ret);
  return ret;
}

void AliasClass::framelike_union_set(ALocal cls) {
  m_stagBits = BLocal;
  m_local = cls;
}
void AliasClass::framelike_union_set(AIter cls) {
  m_stagBits = BIter;
  m_iter = cls;
}

namespace detail {

FPRelOffset frame_base_offset(SSATmp* fp) {
  always_assert(fp->isA(TFramePtr));

  fp = canonical(fp);
  auto fpInst = fp->inst();
  if (fpInst->is(DefFP, DefFuncEntryFP)) return FPRelOffset{0};
  always_assert(fpInst->is(BeginInlining));
  auto const spInst = fpInst->src(0)->inst();
  assertx(spInst->is(DefFrameRelSP, DefRegSP));
  auto const offsetOfSp = spInst->extra<DefStackData>()->irSPOff;

  // FP-relative offset of the inlined frame.
  return fpInst->extra<BeginInlining>()->spOffset.to<FPRelOffset>(offsetOfSp);
}
}

AStack::AStack(SSATmp* fp, FPRelOffset o, int32_t s)
  : offset(o)
  , size(s)
{
  always_assert(fp->isA(TFramePtr));

  fp = canonical(fp);
  auto beginInlining = fp->inst();
  if (!beginInlining->is(BeginInlining)) return;
  auto const sp = beginInlining->src(0)->inst();
  assertx(sp->is(DefFrameRelSP, DefRegSP));

  // FP-relative offset of the inlined frame.
  auto const innerFPRel =
    beginInlining->extra<BeginInlining>()->spOffset.to<FPRelOffset>(
      sp->extra<DefStackData>()->irSPOff);

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
  offset = spRel.to<FPRelOffset>(defSP->extra<DefStackData>()->irSPOff);
}

//////////////////////////////////////////////////////////////////////

size_t AliasClass::Hash::operator()(AliasClass acls) const {
  auto const hash = folly::hash::twang_mix64(acls.m_bits | acls.m_stagBits);
  switch (stagFor(acls.m_stagBits)) {
  case STag::None:
    return hash;

  case STag::Iter:  return framelike_hash(hash, acls.m_iter);
  case STag::Local: return framelike_hash(hash, acls.m_local);

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
  case STag::FrameAll:
    return folly::hash::hash_combine(hash, acls.m_frameAll.base.offset);

  case STag::Rds:
    return folly::hash::hash_combine(hash, acls.m_rds.handle);
  }
  not_reached();
}

//////////////////////////////////////////////////////////////////////

#define X(What, what)                                       \
  AliasClass::AliasClass(A##What x)                         \
    : m_bits(B##What)                                       \
    , m_stagBits(B##What)                                   \
    , m_##what(x)                                           \
  {                                                         \
    assertx(checkInvariants());                             \
  }                                                         \
                                                            \
  folly::Optional<A##What> AliasClass::is_##what() const {  \
    if (*this <= A##What##Any) return what();               \
    return folly::none;                                     \
  }

X(Local, local)
X(Iter, iter)
X(Prop, prop)
X(ElemI, elemI)
X(ElemS, elemS)
X(Stack, stack)
X(Rds, rds)

#undef X

#define X(What, what)                                       \
  folly::Optional<A##What> AliasClass::what() const {       \
    if (m_stagBits & B##What) return m_##what;              \
    return folly::none;                                     \
  }

X(Local, local)
X(Iter, iter)
X(Prop, prop)
X(ElemI, elemI)
X(ElemS, elemS)
X(Stack, stack)
X(Rds, rds)

#undef X

#define X(What, what)                                       \
  AliasClass::AliasClass(A##What x)                         \
    : m_bits(B##What)                                       \
    , m_stagBits(B##What)                                   \
    , m_frameAll(x)                                         \
  {                                                         \
    assertx(checkInvariants());                             \
  }                                                         \
                                                            \
  folly::Optional<A##What> AliasClass::is_##what() const {  \
    if (*this <= A##What##Any) return what();               \
    return folly::none;                                     \
  }                                                         \
  folly::Optional<A##What> AliasClass::what() const {       \
    if (m_stagBits & B##What) {                             \
      return A##What { m_frameAll };                        \
    }                                                       \
    return folly::none;                                     \
  }

X(FContext, fcontext)
X(FFunc, ffunc)
X(FMeta, fmeta)
X(ActRec, actrec)

#undef X
#undef Y

folly::Optional<AliasClass> AliasClass::mis() const {
  auto const bits = static_cast<rep>(m_bits & BMIState);

  if (bits != BEmpty) return AliasClass{bits};
  return folly::none;
}

folly::Optional<AliasClass> AliasClass::is_mis() const {
  if (*this <= AMIStateAny) return mis();
  return folly::none;
}

folly::Optional<AliasClass> AliasClass::frame_base() const {
  auto const bits = static_cast<rep>(m_bits & BFBasePtr);

  if (bits != BEmpty) return AliasClass{bits};
  return folly::none;
}

folly::Optional<AliasClass> AliasClass::is_frame_base() const {
  if (*this <= AFBasePtr) return frame_base();
  return folly::none;
}

AliasClass::STag AliasClass::stagFor(rep bits) {
  if (bits == BEmpty) return STag::None;
  STag ret{STag::None};
#define O(tag, mask) if ((bits & mask) == bits) ret = STag::tag;
  ALIAS_CLASS_SPEC
#undef O
  //assertx(ret != STag::None);
  always_assert_flog(ret != STag::None, "Bad bits: {}", (uint32_t)bits);
  return ret;
}

bool AliasClass::checkInvariants() const {
  switch (stagFor(m_stagBits)) {
  case STag::None:           break;
  case STag::Iter:           framelike_checkInvariants(m_iter);  break;
  case STag::Local:          framelike_checkInvariants(m_local); break;
  case STag::FrameAll:       assertx(m_frameAll.base.offset <= 0); break;
  case STag::Prop:           break;
  case STag::ElemI:          break;
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

  assertx((m_bits & m_stagBits) == m_stagBits);

  return true;
}

bool AliasClass::equivData(AliasClass o) const {
  assertx(stagFor(m_stagBits) == stagFor(o.m_stagBits));
  switch (stagFor(m_stagBits)) {
  case STag::None:     return true;
  case STag::Local:    return framelike_equal(m_local, o.m_local);
  case STag::Iter:     return framelike_equal(m_iter, o.m_iter);
  case STag::FrameAll: return m_frameAll.base == o.m_frameAll.base;
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
         m_stagBits == o.m_stagBits &&
         equivData(o);
}

AliasClass AliasClass::unionData(rep newBits, AliasClass a, AliasClass b) {
  assertx(stagFor(a.m_stagBits) == stagFor(b.m_stagBits));
  auto const frm = static_cast<rep>((a.m_stagBits | b.m_stagBits) & BActRec);
  switch (stagFor(a.m_stagBits)) {
  case STag::None:
    break;
  case STag::Prop:
  case STag::ElemI:
  case STag::ElemS:
  case STag::Rds:
  case STag::FrameAll:
    assertx(!a.equivData(b));
    break;
  case STag::Iter:  return framelike_union(newBits, frm, a.m_iter, b.m_iter);
  case STag::Local: return framelike_union(newBits, frm, a.m_local, b.m_local);

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
      ret.m_stagBits = BStack;
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
  // adjacent AStack ranges, multiple ALocal locals.)
  auto const stag1 = stagFor(m_stagBits);
  auto const stag2 = stagFor(o.m_stagBits);
  if (stag1 == STag::None && stag2 == STag::None) {
    return AliasClass{unioned};
  }
  if (stag1 == STag::None && stag2 != STag::None) {
    return o.precise_union(*this); // flip args
  }
  if (stag1 == STag::FrameAll || stag2 == STag::FrameAll) {
    auto const uf1 = asUFrameBase();
    auto const uf2 = o.asUFrameBase();
    if (uf1 && uf2 && uf1->base == uf2->base) {
      auto ret = stag1 == STag::FrameAll ? o : *this;
      ret.m_bits = unioned;
      ret.m_stagBits = static_cast<rep>(o.m_stagBits | m_stagBits);
      return ret;
    }
  }
  assertx(stag1 != STag::None);
  if (stag2 != STag::None) return folly::none;
  if (o.m_bits & m_stagBits) return folly::none;

  // Keep the data and stag from this, but change its bits.
  auto ret = *this;
  ret.m_bits = unioned;
  assertx(stagFor(ret.m_stagBits) == stag1);
  return ret;
}

AliasClass AliasClass::operator|(AliasClass o) const {
  if (auto const c = precise_union(o)) return *c;

  auto const unioned = static_cast<rep>(m_bits | o.m_bits);

  // If they have the same stag, try to merge them with unionData.
  auto stag1 = stagFor(m_stagBits);
  auto stag2 = stagFor(o.m_stagBits);
  if (stag1 == stag2) return unionData(unioned, *this, o);

  auto const frame1 = asUFrameBase();
  auto const frame2 = o.asUFrameBase();
  if (frame1 && frame2 && frame1->base == frame2->base) {
    if (stag1 == STag::FrameAll || stag2 == STag::FrameAll) {
      auto ret = stag1 == STag::FrameAll ? o : *this;
      ret.m_bits = unioned;
      ret.m_stagBits = static_cast<rep>(m_stagBits | o.m_stagBits);
      return ret;
    }
  }

  // If one of the alias classes have a non-None stag, we can only keep it if
  // the other doesn't have any of the corresponding bits set.
  if (stag1 != STag::None && (o.m_bits & m_stagBits)) stag1 = STag::None;
  if (stag2 != STag::None && (m_bits & o.m_stagBits)) stag2 = STag::None;

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
   * Note also that we might be in a situation where one of the STags
   * is representing a union of more primitive STags. But for this
   * case, we've already thrown away the overlap by setting stags to
   * None above.
   */
  const AliasClass* best = &o;
  auto const stag = [&] () -> STag {
    if (stag1 != STag::None) {
      if (stag2 == STag::None || m_stagBits < o.m_stagBits) {
        best = this;
        ret.m_stagBits = m_stagBits;
        return stag1;
      }
    }
    ret.m_stagBits = o.m_stagBits;
    return stag2;
  }();

  switch (stag) {
  case STag::None:
    break;
  case STag::Iter:     new (&ret.m_iter) AIter(best->m_iter); break;
  case STag::FrameAll: new (&ret.m_frameAll) UFrameBase(best->m_frameAll);break;
  case STag::Local:    new (&ret.m_local) ALocal(best->m_local); break;
  case STag::Prop:     new (&ret.m_prop) AProp(best->m_prop); break;
  case STag::ElemI:    new (&ret.m_elemI) AElemI(best->m_elemI); break;
  case STag::ElemS:    new (&ret.m_elemS) AElemS(best->m_elemS); break;
  case STag::Stack:    new (&ret.m_stack) AStack(best->m_stack); break;
  case STag::Rds:      new (&ret.m_rds) ARds(best->m_rds); break;
  }

  // If both alias classes have compatible frames we can merge their frame
  // specific specializations.
  if (frame1 && frame2 && frame1->base == frame2->base) {
    auto const frameBits = (m_stagBits | o.m_stagBits) & BActRec;
    ret.m_stagBits = static_cast<rep>(ret.m_stagBits | frameBits);
  }

  return ret;
}

bool AliasClass::subclassData(AliasClass o) const {
  assertx(stagFor(m_stagBits) == stagFor(o.m_stagBits));
  switch (stagFor(m_stagBits)) {
  case STag::None:
  case STag::Prop:
  case STag::ElemI:
  case STag::ElemS:
  case STag::Rds:
    return equivData(o);
  case STag::Iter:
    return framelike_subclass(m_iter, o.m_iter);
  case STag::Local:
    return framelike_subclass(m_local, o.m_local);
  case STag::Stack:
    return m_stack.offset <= o.m_stack.offset &&
           lowest_offset(m_stack) >= lowest_offset(o.m_stack);
  case STag::FrameAll: return m_frameAll.base == o.m_frameAll.base;
  }
  not_reached();
}

folly::Optional<UFrameBase> AliasClass::asUFrameBase() const {
  switch (stagFor(m_stagBits)) {
  case STag::None:
  case STag::Prop:
  case STag::ElemI:
  case STag::ElemS:
  case STag::Rds:
  case STag::Stack:
    return folly::none;
  case STag::Local:     return m_local;
  case STag::Iter:      return m_iter;
  case STag::FrameAll:  return m_frameAll;
  }
  not_reached();
}

/*
 * Should return true if this's specialized data is entirely contained in o's
 * specialized data, for only the portion of the data relevant for
 * "relevant_bits", with the precondition that m_stagBits != o.m_stagBits.
 *
 * We know the only case this can happen is if at least one of them is an
 * iterator, and they are only going to have a subclass relationship inside of
 * relevant_bits only if they refer to the same iterator.
 */
bool AliasClass::diffSTagSubclassData(rep relevant_bits, AliasClass o) const {
  if ((relevant_bits & BActRec) == relevant_bits) {
    if ((o.m_stagBits & m_bits) == relevant_bits) {
      return asUFrameBase()->base == o.asUFrameBase()->base;
    }
    return false;
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
bool AliasClass::diffSTagMaybeData(rep relevant_bits, AliasClass o) const {
  if (relevant_bits & BActRec) {
    return asUFrameBase()->base == o.asUFrameBase()->base;
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
  if (stagFor(m_stagBits) == stagFor(o.m_stagBits)) {
    auto const frame = m_stagBits & BActRec;
    auto const oframe = o.m_stagBits & BActRec;
    // If they have any frame specializations we should have those bits as well.
    if ((frame & oframe) == (oframe & isect)) {
      return subclassData(o);
    }
    return false;
  }

  auto const sbits  = m_stagBits;
  auto const osbits = o.m_stagBits;

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
   * The remaining situations are that stag is STag::None, in which case it
   * is a subclass since osbits wasn't in the intersection.  Or that stag has
   * a bit that is in the isect (since m_bits == isect), and that bit is set in
   * o.m_bits.  In either case this is a subclass, so we can just return true.
   */
  if (osbits & isect) return false;
  return true;
}

bool AliasClass::maybeData(AliasClass o) const {
  assertx(stagFor(m_stagBits) == stagFor(o.m_stagBits));
  if (m_stagBits & o.m_stagBits & BActRec) {
    auto const uframe1 = asUFrameBase();
    auto const uframe2 = o.asUFrameBase();
    if (uframe1 && uframe2 && uframe1->base == uframe2->base) {
      return true;
    }
  }
  switch (stagFor(m_stagBits)) {
  case STag::None:
    not_reached();  // handled outside
  case STag::Iter:   return framelike_maybe(m_iter, o.m_iter);
  case STag::Local:  return framelike_maybe(m_local, o.m_local);
  case STag::FrameAll: return m_frameAll.base == o.m_frameAll.base;
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

  auto const sbits = m_stagBits;
  auto const osbits = o.m_stagBits;

  /*
   * If a shared portion of the specialized information is in the intersection,
   * and the intersection is otherwise empty, we can just check if the data
   * associated with the intersecting bits is in a maybe relationship.
   */
  if (auto const inner_bits = (sbits & osbits & isect)) {
    if ((inner_bits & isect) == isect) {
      if (stagFor(sbits) == stagFor(osbits)) return maybeData(o);
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

  // Only MIState and FBasePtr are allowed to have no specialization and be a
  // valid single location.
  if (m_stagBits == BEmpty) return *this <= (AMIStateAny | AFBasePtr);

  // ALocal, AStack, and AIter can contain multiple locations.
  if (auto const frame = is_local()) {
    return frame->ids.hasSingleValue();
  }
  if (auto const iter = is_iter()) {
    return iter->ids.hasSingleValue();
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
  if (offset == offsetof(MInstrState, base)) {
    return AliasClass{AliasClass::BMIBase};
  }
  always_assert(false);
}

//////////////////////////////////////////////////////////////////////

AliasClass canonicalize(AliasClass a) {
  using T = AliasClass::STag;
  switch (AliasClass::stagFor(a.m_stagBits)) {
  case T::None:           return a;
  case T::Local:          return a;
  case T::Iter:           return a;
  case T::Stack:          return a;
  case T::Rds:            return a;
  case T::FrameAll:       return a;
  case T::Prop:     a.m_prop.obj = canonical(a.m_prop.obj);   return a;
  case T::ElemI:    a.m_elemI.arr = canonical(a.m_elemI.arr); return a;
  case T::ElemS:    a.m_elemS.arr = canonical(a.m_elemS.arr); return a;
  }
  not_reached();
}

//////////////////////////////////////////////////////////////////////

std::string show(AliasClass acls) {
  using A  = AliasClass;

  auto ret = bit_str(acls.m_bits, acls.m_stagBits);
  if (!ret.empty() && acls.m_stagBits != A::BEmpty) {
    ret += ' ';
  }

  switch (A::stagFor(acls.m_stagBits)) {
  case A::STag::None:
    break;
  case A::STag::Local:
    folly::format(&ret, "Lv {}:{}", acls.m_local.base.offset,
                  show(acls.m_local.ids));
    break;
  case A::STag::Iter:
    folly::format(&ret, "It {}:{}", acls.m_iter.base.offset,
                  show(acls.m_iter.ids));
  case A::STag::FrameAll:
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

  if (acls.m_stagBits & A::BActRec) {
    auto const off = acls.asUFrameBase()->base.offset;
    if (A::stagFor(acls.m_stagBits) != A::STag::FrameAll) ret += "+";
    if (acls.m_stagBits & A::BFMeta)    ret += "Fm";
    if (acls.m_stagBits & A::BFContext) ret += "Fc";
    if (acls.m_stagBits & A::BFFunc)    ret += "Ff";
    if (A::stagFor(acls.m_stagBits) == A::STag::FrameAll) {
      folly::format(&ret, "@{}", off);
    }
  }

  return ret;
}

//////////////////////////////////////////////////////////////////////

}}
