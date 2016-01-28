/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/type-specialization.h"

#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/vm/class.h"

namespace HPHP { namespace jit {
///////////////////////////////////////////////////////////////////////////////
// ArraySpec.

const ArraySpec ArraySpec::Top;
const ArraySpec ArraySpec::Bottom(ArraySpec::BottomTag{});

bool ArraySpec::operator<=(const ArraySpec& rhs) const {
  auto const& lhs = *this;

  if (lhs == Bottom || rhs == Top) return true;
  if (lhs == Top || rhs == Bottom) return false;

  // It's possible to subtype RAT::Array types, but it's potentially O(n), so
  // we just don't do it.
  return (!rhs.kind()  || lhs.kind()  == rhs.kind()) &&
         (!rhs.type()  || lhs.type()  == rhs.type()) &&
         (!rhs.shape() || lhs.shape() == rhs.shape());
}

ArraySpec ArraySpec::operator|(const ArraySpec& rhs) const {
  auto const& lhs = *this;

  if (lhs <= rhs) return rhs;
  if (rhs <= lhs) return lhs;

  // Take the union componentwise.  Components union trivially (i.e., to
  // "unspecialized") unless they are equal.
  auto new_kind = lhs.kind() == rhs.kind() ? lhs.kind() : folly::none;
  auto new_type = lhs.type() == rhs.type() ? lhs.type() : nullptr;

  // If the shapes were nontrivial and equal, the specs would be equal.
  assertx(!lhs.shape() || lhs.shape() != rhs.shape());

  // Nontrivial kind /and/ type unions would imply equal kinds and types.
  assertx(!new_kind || !new_type);

  if (new_kind) return ArraySpec(*new_kind);
  if (new_type) return ArraySpec(new_type);
  return Top;
}

ArraySpec ArraySpec::operator&(const ArraySpec& rhs) const {
  auto const& lhs = *this;

  if (lhs <= rhs) return lhs;
  if (rhs <= lhs) return rhs;

  // Take the intersection componentwise.  If one component is unspecialized,
  // it behaves like Top, and the intersection is the other component (the
  // Bottom case is handled by the subtype checks above).  Otherwise, they
  // intersect to either component if they are equal, else to Bottom.
#define NEW_COMPONENT(name, defval)                   \
  (lhs.name() && !rhs.name() ? lhs.name() :           \
   rhs.name() && !lhs.name() ? rhs.name() :           \
   lhs.name() == rhs.name()  ? lhs.name() : (defval))

  // If the default value is returned, it might mean either Bottom or Top, so
  // we need more checks.
  auto new_kind  = NEW_COMPONENT(kind,  folly::none);
  auto new_type  = NEW_COMPONENT(type,  nullptr);
  auto new_shape = NEW_COMPONENT(shape, nullptr);

#undef NEW_COMPONENT

  if ((!new_kind  && lhs.kind()) ||
      (!new_shape && lhs.shape())) {
    // If there's no new_x, but lhs.x() is nontrivial, then rhs.x() is not
    // equal to it and also nontrivial.  The intersection is thus Bottom.
    //
    // Note that we ignore this check for type(), because we don't subtype RAT
    // types precisely.
    return Bottom;
  }

  if (new_shape && new_kind == ArrayData::kStructKind) {
    // We could potentially be dropping a type, but that's okay.
    return ArraySpec(new_shape);
  }

  if (new_kind && new_type) {
    return ArraySpec(*new_kind, new_type);
  } else if (new_kind) {
    return ArraySpec(*new_kind);
  } else if (new_type) {
    return ArraySpec(new_type);
  }

  return Top;
}

///////////////////////////////////////////////////////////////////////////////
// ClassSpec.

const ClassSpec ClassSpec::Top;
const ClassSpec ClassSpec::Bottom(ClassSpec::BottomTag{});

bool ClassSpec::operator<=(const ClassSpec& rhs) const {
  auto const& lhs = *this;

  if (lhs == rhs) return true;
  if (lhs == Bottom || rhs == Top) return true;
  if (lhs == Top || rhs == Bottom) return false;

  return !rhs.exact() && lhs.cls()->classof(rhs.cls());
}

ClassSpec ClassSpec::operator|(const ClassSpec& rhs) const {
  auto const& lhs = *this;

  if (lhs <= rhs) return rhs;
  if (rhs <= lhs) return lhs;

  assertx(lhs.cls() && rhs.cls());

  // We're unwilling to unify with interfaces, so just return Top.
  if (!isNormalClass(lhs.cls()) || !isNormalClass(rhs.cls())) {
    return Top;
  }

  // Unify to a common ancestor if possible.
  if (auto cls = lhs.cls()->commonAncestor(rhs.cls())) {
    return ClassSpec(cls, ClassSpec::SubTag{});
  }

  return Top;
}

ClassSpec ClassSpec::operator&(const ClassSpec& rhs) const {
  auto const& lhs = *this;

  if (lhs <= rhs) return lhs;
  if (rhs <= lhs) return rhs;

  assertx(lhs.cls() && rhs.cls());

  // If neither class is an interface, their intersection is trivial.
  if (isNormalClass(lhs.cls()) && isNormalClass(rhs.cls())) {
    return Bottom;
  }

  // If either is an interface, we'd need to explore all implementing classes
  // in the program to know if they have a non-empty intersection.  Instead,
  // we'll just try to take the "better" of the two.  We consider a normal
  // class better than an interface, because it might influence important
  // things like method dispatch or property accesses better than an interface
  // type could.
  if (isNormalClass(lhs.cls())) return lhs;
  if (isNormalClass(rhs.cls())) return rhs;

  // If they are both interfaces, we have to pick one arbitrarily, but we must
  // do so in a way that is stable regardless of which one was passed as lhs or
  // rhs (to guarantee that operator& is commutative).  We use the class name
  // in this case to ensure that the ordering is dependent only on the source
  // program (Class* or something like that seems less desirable).
  return lhs.cls()->name()->compare(rhs.cls()->name()) < 0 ? lhs : rhs;
}

///////////////////////////////////////////////////////////////////////////////
}}
