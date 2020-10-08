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

#include "hphp/runtime/vm/jit/type-specialization.h"

#include "hphp/runtime/base/repo-auth-type-array.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/record.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

namespace {
// Ideally, we would intersect RATs here, but doing so is expensive and rarely
// ends up being useful. Instead, we use a heuristic which tends to select the
// more constrained type, and fall back to id to ensure commutativity.
const RepoAuthType::Array* chooseRATArray(const RepoAuthType::Array* lhs,
                                          const RepoAuthType::Array* rhs) {
  auto const lhs_size_known = lhs->tag() == RepoAuthType::Array::Tag::Packed;
  auto const rhs_size_known = rhs->tag() == RepoAuthType::Array::Tag::Packed;
  if (lhs_size_known != rhs_size_known) return lhs_size_known ? lhs : rhs;
  return lhs->id() < rhs->id() ? lhs : rhs;
}
}

///////////////////////////////////////////////////////////////////////////////
// ArraySpec.

bool ArraySpec::operator<=(const ArraySpec& rhs) const {
  assertx(checkInvariants());
  assertx(rhs.checkInvariants());
  auto const& lhs = *this;

  if (rhs == Top()) return true;
  if (lhs == Bottom()) return true;
  if (rhs == Bottom()) return false; // since lhs is not Bottom
  if (lhs == Top()) return false; // since rhs is not Top

  if (rhs.vanilla()) {
    if (!lhs.vanilla()) {
      return false;
    }
  } else if (auto const index = rhs.bespokeIndex()) {
    if (lhs.bespokeIndex() != index) {
      return false;
    }
  }

  // It's possible to subtype RAT::Array types, but it's potentially O(n), so
  // we just don't do it. It's okay for <= to return false negative results.

  // if LHS drops RHS's type, it's bigger
  if (rhs.type() && lhs.type() != rhs.type()) {
    return false;
  }

  return true;
}

ArraySpec ArraySpec::operator|(const ArraySpec& rhs) const {
  assertx(checkInvariants());
  assertx(rhs.checkInvariants());
  auto const& lhs = *this;

  if (lhs <= rhs) return rhs;
  if (rhs <= lhs) return lhs;

  auto result = lhs;

  // lhs and rhs are both neither top nor bottom
  // but we can still have m_sort of Top and a RAT specialization
  assertx(lhs.m_sort != Sort::Bottom);
  assertx(rhs.m_sort != Sort::Bottom);

  if (lhs.m_sort == Sort::Top ||
      rhs.m_sort == Sort::Top) {
    result.m_sort = Sort::Top;
  } else if (lhs.bespokeIndex() && rhs.bespokeIndex()) {
    if (lhs.bespokeIndex() != rhs.bespokeIndex()) {
      result.m_sort = Sort::Top;
    }
  } else if (lhs.vanilla() != rhs.vanilla()) {
    result.m_sort = Sort::Top;
  }

  // now address the RAT types
  if (rhs.type() == lhs.type()) {
    result.m_ptr = rhs.m_ptr;
  } else {
    result.m_ptr = 0;
  }

  assertx(result.checkInvariants());
  return result;
}

ArraySpec ArraySpec::operator&(const ArraySpec& rhs) const {
  assertx(checkInvariants());
  assertx(rhs.checkInvariants());
  auto const& lhs = *this;

  if (lhs <= rhs) return lhs;
  if (rhs <= lhs) return rhs;

  auto result = lhs;
  // lhs and rhs are both neither top nor bottom
  // but we can still have m_sort of Top and a RAT specialization
  assertx(lhs.m_sort != Sort::Bottom);
  assertx(rhs.m_sort != Sort::Bottom);

  if (lhs.m_sort == Sort::Top) {
    result.m_sort = rhs.m_sort;
  } else if (rhs.m_sort == Sort::Top) {
    result.m_sort = lhs.m_sort;
  } else if (lhs.vanilla() != rhs.vanilla()) {
    return Bottom();
  } else if (lhs.bespokeIndex() != rhs.bespokeIndex()) {
    return Bottom();
  }

  if (!rhs.type()) {
    result.m_ptr = lhs.m_ptr;
  } else if (!lhs.type()) {
    result.m_ptr = rhs.m_ptr;
  } else if (lhs.type() != rhs.type()) {
    // If both types have an RAT and they differ, keep the "better" one.
    // (see above)
    auto const rat = chooseRATArray(lhs.type(), rhs.type());
    result.m_ptr = reinterpret_cast<uintptr_t>(rat);
  }

  result.checkInvariants();
  return result;
}

std::optional<BespokeLayout> ArraySpec::bespokeLayout() const {
  if (auto const index = bespokeIndex()) {
    return BespokeLayout::FromIndex(*index);
  } else {
    return {};
  }
}

std::string ArraySpec::toString() const {
  std::string result;

  if (vanilla()) {
    result += "=Vanilla";
  } else if (auto const layout = bespokeLayout()) {
    folly::format(&result, "=Bespoke({})", layout->describe());
  }

  if (auto const ty = type()) {
    auto const sign = result.empty() ? '=' : ':';
    result += folly::to<std::string>(sign, show(*ty));
  }
  return result;
}

bool ArraySpec::checkInvariants() const {
  // if m_sort is bottom, we should _not_ have a RAT type set
  assertx(IMPLIES(m_sort == Sort::Bottom, !type()));
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// ClsRecSpec.

template<>
ClassSpec ClassSpec::operator&(const ClassSpec& rhs) const {
  auto const& lhs = *this;

  if (lhs <= rhs) return lhs;
  if (rhs <= lhs) return rhs;

  assertx(lhs.typeCns() && rhs.typeCns());

  // If neither class is an interface, their intersection is trivial.
  if (isNormalClass(lhs.typeCns()) && isNormalClass(rhs.typeCns())) {
    return Bottom();
  }

  // If either is an interface, we'd need to explore all implementing classes
  // in the program to know if they have a non-empty intersection.  Instead,
  // we'll just try to take the "better" of the two.  We consider a normal
  // class better than an interface, because it might influence important
  // things like method dispatch or property accesses better than an interface
  // type could.
  if (isNormalClass(lhs.typeCns())) return lhs;
  if (isNormalClass(rhs.typeCns())) return rhs;

  // If they are both interfaces, we have to pick one arbitrarily, but we must
  // do so in a way that is stable regardless of which one was passed as lhs or
  // rhs (to guarantee that operator& is commutative).  We use the class name
  // in this case to ensure that the ordering is dependent only on the source
  // program (Class* or something like that seems less desirable).
  return lhs.typeCns()->name()->compare(rhs.typeCns()->name()) < 0 ? lhs : rhs;
}

template<>
RecordSpec RecordSpec::operator&(const RecordSpec& rhs) const {
  auto const& lhs = *this;

  if (lhs <= rhs) return lhs;
  if (rhs <= lhs) return rhs;

  assertx(lhs.typeCns() && rhs.typeCns());
  return Bottom();
}

///////////////////////////////////////////////////////////////////////////////
}}
