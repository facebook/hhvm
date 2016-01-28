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

inline TypeSpec::TypeSpec(ArraySpec arrSpec, ClassSpec clsSpec)
  : m_kind(SpecKind::None)
  , m_arrSpec(arrSpec)
  , m_clsSpec(clsSpec)
{
  if (arrSpec != ArraySpec::Bottom) m_kind |= SpecKind::Array;
  if (clsSpec != ClassSpec::Bottom) m_kind |= SpecKind::Class;
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
  inline Spec::operator bool() const {                    \
    return *this != Top && *this != Spec::Bottom;         \
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
    return *this <= rhs ? Bottom : *this;                 \
  }

///////////////////////////////////////////////////////////////////////////////
// ArraySpec.

constexpr inline ArraySpec::ArraySpec()
  : m_sort(IsTop)
  , m_kind(ArrayData::ArrayKind{})
  , m_ptr(0)
{}

inline ArraySpec::ArraySpec(ArraySpec::BottomTag)
  : m_sort(IsBottom)
  , m_kind(ArrayData::ArrayKind{})
  , m_ptr(0)
{}

inline ArraySpec::ArraySpec(ArrayData::ArrayKind kind)
  : m_sort(HasKind)
  , m_kind(kind)
  , m_ptr(0)
{}

inline ArraySpec::ArraySpec(const RepoAuthType::Array* arrTy)
  : m_sort(HasType)
  , m_kind(ArrayData::ArrayKind{})
  , m_ptr(reinterpret_cast<uintptr_t>(arrTy))
{}

inline ArraySpec::ArraySpec(ArrayData::ArrayKind kind,
                            const RepoAuthType::Array* arrTy)
  : m_sort(HasKind | HasType)
  , m_kind(kind)
  , m_ptr(reinterpret_cast<uintptr_t>(arrTy))
{}

inline ArraySpec::ArraySpec(const Shape* shape)
  : m_sort(HasKind | HasShape)
  , m_kind(ArrayData::kStructKind)
  , m_ptr(reinterpret_cast<uintptr_t>(shape))
{}

inline folly::Optional<ArrayData::ArrayKind> ArraySpec::kind() const {
  auto kind = m_kind;
  return (m_sort & HasKind) ? folly::make_optional(kind) : folly::none;
}

inline const RepoAuthType::Array* ArraySpec::type() const {
  return (m_sort & HasType)
    ? reinterpret_cast<const RepoAuthType::Array*>(m_ptr)
    : nullptr;
}

inline const Shape* ArraySpec::shape() const {
  return (m_sort & HasShape)
    ? reinterpret_cast<const Shape*>(m_ptr)
    : nullptr;
}

IMPLEMENT_SPEC_OPERS(ArraySpec)

///////////////////////////////////////////////////////////////////////////////

inline ArraySpec::SortOf operator|(ArraySpec::SortOf l, ArraySpec::SortOf r) {
  return static_cast<ArraySpec::SortOf>(
      static_cast<uint8_t>(l) | static_cast<uint8_t>(r));
}

inline ArraySpec::SortOf operator&(ArraySpec::SortOf l, ArraySpec::SortOf r) {
  return static_cast<ArraySpec::SortOf>(
      static_cast<uint8_t>(l) & static_cast<uint8_t>(r));
}

///////////////////////////////////////////////////////////////////////////////
// ClassSpec.

constexpr inline ClassSpec::ClassSpec()
  : m_sort(IsTop)
  , m_ptr(0)
{}

inline ClassSpec::ClassSpec(ClassSpec::BottomTag)
  : m_sort(IsBottom)
  , m_ptr(0)
{}

inline ClassSpec::ClassSpec(const Class* cls, ClassSpec::SubTag)
  : m_sort(IsSub)
  , m_ptr(reinterpret_cast<uintptr_t>(cls))
{}

inline ClassSpec::ClassSpec(const Class* cls, ClassSpec::ExactTag)
  : m_sort(IsExact)
  , m_ptr(reinterpret_cast<uintptr_t>(cls))
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

IMPLEMENT_SPEC_OPERS(ClassSpec)

///////////////////////////////////////////////////////////////////////////////

#undef IMPLEMENT_SPEC_OPERS

}}
