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

#include <folly/Hash.h>
#include <folly/Format.h>

#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/analysis.h"

namespace HPHP { namespace jit {

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
  }
  not_reached();
}

//////////////////////////////////////////////////////////////////////

#define X(What, what)                           \
  AliasClass::AliasClass(A##What x)             \
    : m_bits(B##What)                           \
    , m_stag(STag::What)                        \
    , m_##what(x)                               \
  {                                             \
    assert(checkInvariants());                  \
  }

X(Frame, frame)
X(Prop, prop)
X(ElemI, elemI)
X(ElemS, elemS)

#undef X

bool AliasClass::checkInvariants() const {
  switch (m_stag) {
  case STag::None:  break;
  case STag::Frame: break;
  case STag::Prop:  break;
  case STag::ElemI: break;
  case STag::ElemS:
    assert(m_elemS.key->isStatic());
    break;
  }
  return true;
}

bool AliasClass::equivData(AliasClass o) const {
  assert(m_stag == o.m_stag);
  switch (m_stag) {
  case STag::None:   return true;
  case STag::Frame:  return m_frame.fp == o.m_frame.fp &&
                            m_frame.id == o.m_frame.id;
  case STag::Prop:   return m_prop.obj == o.m_prop.obj &&
                            m_prop.offset == o.m_prop.offset;
  case STag::ElemI:  return m_elemI.arr == o.m_elemI.arr &&
                            m_elemI.idx == o.m_elemI.idx;
  case STag::ElemS:  return m_elemS.arr == o.m_elemS.arr &&
                            m_elemS.key == o.m_elemS.key;
  }
  not_reached();
}

bool AliasClass::operator==(AliasClass o) const {
  return m_bits == o.m_bits &&
         m_stag == o.m_stag &&
         equivData(o);
}

AliasClass AliasClass::operator|(AliasClass o) const {
  // We don't have a purpose for doing better than this yet:
  if (o <= *this) return *this;
  if (*this <= o) return o;
  return AUnknown;
}

bool AliasClass::operator<=(AliasClass o) const {
  auto const isect = static_cast<rep>(m_bits & o.m_bits);
  if (isect != m_bits)    return false;
  if (m_stag != o.m_stag) return m_bits == BEmpty;
  return equivData(o);
}

bool AliasClass::maybe(AliasClass o) const {
  auto const isect = static_cast<rep>(m_bits & o.m_bits);
  if (isect == 0) return false;
  if (*this <= o || o <= *this) return true;

  // Right now, stags map one to one on bits, so we must have the same stag,
  // unless one of them is none.  If either are none, we have a non-empty
  // intersection.
  if (m_stag == STag::None || o.m_stag == STag::None) return true;
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
  }
  not_reached();
}

folly::Optional<AFrame> AliasClass::frame() const {
  if (m_stag == STag::Frame) return m_frame;
  return folly::none;
}

folly::Optional<AProp> AliasClass::prop() const {
  if (m_stag == STag::Prop) return m_prop;
  return folly::none;
}

folly::Optional<AElemI> AliasClass::elemI() const {
  if (m_stag == STag::ElemI) return m_elemI;
  return folly::none;
}

folly::Optional<AElemS> AliasClass::elemS() const {
  if (m_stag == STag::ElemS) return m_elemS;
  return folly::none;
}

//////////////////////////////////////////////////////////////////////

AliasClass canonicalize(AliasClass a) {
  if (auto const x = a.prop())  return AProp { canonical(x->obj), x->offset };
  if (auto const x = a.elemI()) return AElemI { canonical(x->arr), x->idx };
  if (auto const x = a.elemS()) return AElemS { canonical(x->arr), x->key };
  return a;
}

//////////////////////////////////////////////////////////////////////

std::string show(AliasClass acls) {
  auto ret = std::string{};
  using A  = AliasClass;

  switch (acls.m_bits) {
  case A::BEmpty:     ret = "empty";    break;
  case A::BNonFrame:  ret = "nonframe"; break;
  case A::BUnknown:   ret = "unk";      break;
  case A::BFrame:     ret = "frame";    break;
  case A::BProp:      ret = "prop";     break;
  case A::BElemI:     ret = "elemI";    break;
  case A::BElemS:     ret = "elemS";    break;
  case A::BElem:      ret = "elem";     break;
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
  }

  return ret;
}

//////////////////////////////////////////////////////////////////////

}}
