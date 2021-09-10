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

namespace HPHP { namespace jit {
///////////////////////////////////////////////////////////////////////////////

inline SpecKind operator|(SpecKind l, SpecKind r) {
  return static_cast<SpecKind>(
      static_cast<uint8_t>(l) | static_cast<uint8_t>(r));
}

inline SpecKind operator&(SpecKind l, SpecKind r) {
  return static_cast<SpecKind>(
      static_cast<uint8_t>(l) & static_cast<uint8_t>(r));
}

inline SpecKind& operator|=(SpecKind& l, SpecKind r) {
  return l = l | r;
}

///////////////////////////////////////////////////////////////////////////////

inline TypeSpec::TypeSpec()
  : m_kind(SpecKind::None)
{}

inline TypeSpec::TypeSpec(ArraySpec arrSpec,
                          ClassSpec clsSpec)
  : m_kind(SpecKind::None)
  , m_arrSpec(arrSpec)
  , m_clsSpec(clsSpec)
{
  if (arrSpec != ArraySpec::Bottom()) m_kind |= SpecKind::Array;
  if (clsSpec != ClassSpec::Bottom()) m_kind |= SpecKind::Class;
}

inline SpecKind TypeSpec::kind() const {
  return m_kind;
}

inline ArraySpec TypeSpec::arrSpec() const {
  return m_arrSpec;
}

inline ClassSpec TypeSpec::clsSpec() const {
  return m_clsSpec;
}

inline bool TypeSpec::operator==(const TypeSpec& rhs) const {
  auto const& lhs = *this;
  return lhs.arrSpec() == rhs.arrSpec() &&
         lhs.clsSpec() == rhs.clsSpec();
}

inline bool TypeSpec::operator!=(const TypeSpec& rhs) const {
  return !(*this == rhs);
}

inline bool TypeSpec::operator<=(const TypeSpec& rhs) const {
  auto const& lhs = *this;
  return lhs.arrSpec() <= rhs.arrSpec() &&
         lhs.clsSpec() <= rhs.clsSpec();
}

inline bool TypeSpec::operator>=(const TypeSpec& rhs) const {
  return rhs <= *this;
}

inline TypeSpec TypeSpec::operator|(const TypeSpec& rhs) const {
  auto const& lhs = *this;
  return TypeSpec(lhs.arrSpec() | rhs.arrSpec(),
                  lhs.clsSpec() | rhs.clsSpec());

}

inline TypeSpec TypeSpec::operator&(const TypeSpec& rhs) const {
  auto const& lhs = *this;
  return TypeSpec(lhs.arrSpec() & rhs.arrSpec(),
                  lhs.clsSpec() & rhs.clsSpec());

}

inline TypeSpec TypeSpec::operator-(const TypeSpec& rhs) const {
  auto const& lhs = *this;
  return TypeSpec(lhs.arrSpec() - rhs.arrSpec(),
                  lhs.clsSpec() - rhs.clsSpec());
}

///////////////////////////////////////////////////////////////////////////////

#define IMPLEMENT_SPEC_OPERS(Spec)                        \
  inline uintptr_t Spec::bits() const {                   \
    return m_bits;                                        \
  }                                                       \
  constexpr Spec Spec::Top() {                            \
    return Spec{};                                        \
  }                                                       \
  constexpr Spec Spec::Bottom() {                         \
    return Spec{BottomTag{}};                             \
  }                                                       \
  inline Spec::operator bool() const {                    \
    return *this != Top() && *this != Bottom();           \
  }                                                       \
  inline bool Spec::operator==(const Spec& rhs) const {   \
    return m_bits == rhs.m_bits;                          \
  }                                                       \
  inline bool Spec::operator!=(const Spec& rhs) const {   \
    return !(*this == rhs);                               \
  }                                                       \
  inline bool Spec::operator>=(const Spec& rhs) const {   \
    return rhs <= *this;                                  \
  }                                                       \
  inline bool Spec::operator<(const Spec& rhs) const {    \
    return *this <= rhs && *this != rhs;                  \
  }                                                       \
  inline bool Spec::operator>(const Spec& rhs) const {    \
    return *this >= rhs && *this != rhs;                  \
  }                                                       \
  inline Spec Spec::operator-(const Spec& rhs) const {    \
    return *this <= rhs ? Bottom() : *this;               \
  }

///////////////////////////////////////////////////////////////////////////////
// ArraySpec.

static_assert(sizeof(ArraySpec) == 8);

constexpr inline ArraySpec::ArraySpec()
  : m_layout(ArrayLayout::Top().toUint16())
  , m_type(0)
{}

constexpr inline ArraySpec::ArraySpec(ArraySpec::BottomTag)
  : m_layout(ArrayLayout::Bottom().toUint16())
  , m_type(0)
{}

inline ArraySpec::ArraySpec(ArrayLayout layout)
  : m_layout(layout.toUint16())
  , m_type(0)
{
  assertx(checkInvariants());
}

inline ArraySpec::ArraySpec(const RepoAuthType::Array* type)
  : m_layout(ArrayLayout::Top().toUint16())
  , m_type(reinterpret_cast<uintptr_t>(type))
{
  assertx(checkInvariants());
}

inline ArraySpec::ArraySpec(ArrayLayout layout,
                            const RepoAuthType::Array* type)
  : m_layout(layout.toUint16())
  , m_type(reinterpret_cast<uintptr_t>(type))
{
  assertx(checkInvariants());
}

inline ArraySpec ArraySpec::narrowToLayout(ArrayLayout layout) const {
  return *this & ArraySpec(layout);
}

inline void ArraySpec::setType(const RepoAuthType::Array* adjusted) {
  assertx(type() && adjusted);
  m_type = reinterpret_cast<uintptr_t>(adjusted);
}

inline ArrayLayout ArraySpec::layout() const {
  return ArrayLayout::FromUint16(m_layout);
}

inline const RepoAuthType::Array* ArraySpec::type() const {
  return reinterpret_cast<const RepoAuthType::Array*>(m_type);
}

IMPLEMENT_SPEC_OPERS(ArraySpec)

///////////////////////////////////////////////////////////////////////////////
// ClasssSpec.

constexpr inline ClassSpec::ClassSpec()
  : m_sort(IsTop)
  , m_ptr(0)
{}

constexpr inline ClassSpec::ClassSpec(ClassSpec::BottomTag)
  : m_sort(IsBottom)
  , m_ptr(0)
{}

inline ClassSpec::ClassSpec(const Class* c, ClassSpec::SubTag)
  : m_sort(IsSub)
  , m_ptr(reinterpret_cast<uintptr_t>(c))
{}

inline ClassSpec::ClassSpec(const Class* c, ClassSpec::ExactTag)
  : m_sort(IsExact)
  , m_ptr(reinterpret_cast<uintptr_t>(c))
{}

inline bool ClassSpec::exact() const {
  return m_sort == IsExact;
}

inline const Class* ClassSpec::cls() const {
  return (m_sort == IsSub || m_sort == IsExact)
    ? reinterpret_cast<const Class*>(m_ptr)
    : nullptr;
}

inline const Class* ClassSpec::exactCls() const {
  return (m_sort == IsExact)
    ? reinterpret_cast<const Class*>(m_ptr)
    : nullptr;
}

inline std::string ClassSpec::toString() const {
  auto const type = exact() ? "=" : "<=";
  auto const name = cls()->name()->data();
  return folly::to<std::string>(type, name);
}

IMPLEMENT_SPEC_OPERS(ClassSpec)

inline bool ClassSpec::operator<=(const ClassSpec& rhs) const {
  auto const& lhs = *this;

  if (lhs == rhs) return true;
  if (lhs == Bottom() || rhs == Top()) return true;
  if (lhs == Top() || rhs == Bottom()) return false;

  return !rhs.exact() && lhs.cls()->subtypeOf(rhs.cls());
}

inline ClassSpec ClassSpec::operator|(const ClassSpec& rhs) const {
  auto const& lhs = *this;

  if (lhs <= rhs) return rhs;
  if (rhs <= lhs) return lhs;

  assertx(lhs.cls() && rhs.cls());

  // We're unwilling to unify with interfaces, so just return Top.
  if (!isNormalClass(lhs.cls()) || !isNormalClass(rhs.cls())) {
    return Top();
  }

  // Unify to a common ancestor if possible.
  if (auto t = lhs.cls()->commonAncestor(rhs.cls())) {
    return ClassSpec(t, ClassSpec::SubTag{});
  }

  return Top();
}

///////////////////////////////////////////////////////////////////////////////

#undef IMPLEMENT_SPEC_OPERS

}}
