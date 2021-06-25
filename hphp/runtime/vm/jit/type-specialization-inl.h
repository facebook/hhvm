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
                          ClassSpec clsSpec,
                          RecordSpec recSpec)
  : m_kind(SpecKind::None)
  , m_arrSpec(arrSpec)
  , m_clsSpec(clsSpec)
  , m_recSpec(recSpec)
{
  if (arrSpec != ArraySpec::Bottom()) m_kind |= SpecKind::Array;
  if (clsSpec != ClassSpec::Bottom()) m_kind |= SpecKind::Class;
  if (recSpec != RecordSpec::Bottom()) m_kind |= SpecKind::Record;
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

inline RecordSpec TypeSpec::recSpec() const {
  return m_recSpec;
}

inline bool TypeSpec::operator==(const TypeSpec& rhs) const {
  auto const& lhs = *this;
  return lhs.arrSpec() == rhs.arrSpec() &&
         lhs.clsSpec() == rhs.clsSpec() &&
         lhs.recSpec() == rhs.recSpec();
}

inline bool TypeSpec::operator!=(const TypeSpec& rhs) const {
  return !(*this == rhs);
}

inline bool TypeSpec::operator<=(const TypeSpec& rhs) const {
  auto const& lhs = *this;
  return lhs.arrSpec() <= rhs.arrSpec() &&
         lhs.clsSpec() <= rhs.clsSpec() &&
         lhs.recSpec() <= rhs.recSpec();
}

inline bool TypeSpec::operator>=(const TypeSpec& rhs) const {
  return rhs <= *this;
}

inline TypeSpec TypeSpec::operator|(const TypeSpec& rhs) const {
  auto const& lhs = *this;
  return TypeSpec(lhs.arrSpec() | rhs.arrSpec(),
                  lhs.clsSpec() | rhs.clsSpec(),
                  lhs.recSpec() | rhs.recSpec());
}

inline TypeSpec TypeSpec::operator&(const TypeSpec& rhs) const {
  auto const& lhs = *this;
  return TypeSpec(lhs.arrSpec() & rhs.arrSpec(),
                  lhs.clsSpec() & rhs.clsSpec(),
                  lhs.recSpec() & rhs.recSpec());
}

inline TypeSpec TypeSpec::operator-(const TypeSpec& rhs) const {
  auto const& lhs = *this;
  return TypeSpec(lhs.arrSpec() - rhs.arrSpec(),
                  lhs.clsSpec() - rhs.clsSpec(),
                  lhs.recSpec() - rhs.recSpec());
}

///////////////////////////////////////////////////////////////////////////////

#define EMIT_TEMPLATE(b, T) EMIT_TEMPLATE_##b(T)
#define EMIT_TEMPLATE_true(T) template<typename T>
#define EMIT_TEMPLATE_false(T)

#define IMPLEMENT_SPEC_OPERS(Spec, is_template, ...)      \
  EMIT_TEMPLATE(is_template, __VA_ARGS__)                 \
  inline uintptr_t Spec::bits() const {                   \
    return m_bits;                                        \
  }                                                       \
  EMIT_TEMPLATE(is_template, __VA_ARGS__)                 \
  constexpr Spec Spec::Top() {                            \
    return Spec{};                                        \
  }                                                       \
  EMIT_TEMPLATE(is_template, __VA_ARGS__)                 \
  constexpr Spec Spec::Bottom() {                         \
    return Spec{BottomTag{}};                             \
  }                                                       \
  EMIT_TEMPLATE(is_template, __VA_ARGS__)                 \
  inline Spec::operator bool() const {                    \
    return *this != Top() && *this != Bottom();           \
  }                                                       \
  EMIT_TEMPLATE(is_template, __VA_ARGS__)                 \
  inline bool Spec::operator==(const Spec& rhs) const {   \
    return m_bits == rhs.m_bits;                          \
  }                                                       \
  EMIT_TEMPLATE(is_template, __VA_ARGS__)                 \
  inline bool Spec::operator!=(const Spec& rhs) const {   \
    return !(*this == rhs);                               \
  }                                                       \
  EMIT_TEMPLATE(is_template, __VA_ARGS__)                 \
  inline bool Spec::operator>=(const Spec& rhs) const {   \
    return rhs <= *this;                                  \
  }                                                       \
  EMIT_TEMPLATE(is_template, __VA_ARGS__)                 \
  inline bool Spec::operator<(const Spec& rhs) const {    \
    return *this <= rhs && *this != rhs;                  \
  }                                                       \
  EMIT_TEMPLATE(is_template, __VA_ARGS__)                 \
  inline bool Spec::operator>(const Spec& rhs) const {    \
    return *this >= rhs && *this != rhs;                  \
  }                                                       \
  EMIT_TEMPLATE(is_template, __VA_ARGS__)                 \
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

IMPLEMENT_SPEC_OPERS(ArraySpec, false)

///////////////////////////////////////////////////////////////////////////////
// ClsRecSpec.
template<typename T>
constexpr inline ClsRecSpec<T>::ClsRecSpec()
  : m_sort(IsTop)
  , m_ptr(0)
{}

template<typename T>
constexpr inline ClsRecSpec<T>::ClsRecSpec(ClsRecSpec<T>::BottomTag)
  : m_sort(IsBottom)
  , m_ptr(0)
{}

template<typename T>
inline ClsRecSpec<T>::ClsRecSpec(const T* t, ClsRecSpec<T>::SubTag)
  : m_sort(IsSub)
  , m_ptr(reinterpret_cast<uintptr_t>(t))
{}

template<typename T>
inline ClsRecSpec<T>::ClsRecSpec(const T* t, ClsRecSpec<T>::ExactTag)
  : m_sort(IsExact)
  , m_ptr(reinterpret_cast<uintptr_t>(t))
{}

template<typename T>
inline bool ClsRecSpec<T>::exact() const {
  return m_sort == IsExact;
}

template<typename T>
inline const T* ClsRecSpec<T>::typeCns() const {
  return (m_sort == IsSub || m_sort == IsExact)
    ? reinterpret_cast<const T*>(m_ptr)
    : nullptr;
}

template<typename T>
inline const T* ClsRecSpec<T>::exactTypeCns() const {
  return (m_sort == IsExact)
    ? reinterpret_cast<const T*>(m_ptr)
    : nullptr;
}

template<typename T>
std::string ClsRecSpec<T>::toString() const {
  auto const type = exact() ? "=" : "<=";
  auto const name = typeCns()->name()->data();
  return folly::to<std::string>(type, name);
}

IMPLEMENT_SPEC_OPERS(ClsRecSpec<T>, true, T)

template<typename T>
bool ClsRecSpec<T>::operator<=(const ClsRecSpec<T>& rhs) const {
  auto const& lhs = *this;

  if (lhs == rhs) return true;
  if (lhs == Bottom() || rhs == Top()) return true;
  if (lhs == Top() || rhs == Bottom()) return false;

  return !rhs.exact() && lhs.typeCns()->subtypeOf(rhs.typeCns());
}

namespace {
bool isNormalType(const Class* cls) {
  return isNormalClass(cls);
}
bool isNormalType(const RecordDesc*) {
  return true;
}
}

template<typename T>
ClsRecSpec<T> ClsRecSpec<T>::operator|(const ClsRecSpec<T>& rhs) const {
  auto const& lhs = *this;

  if (lhs <= rhs) return rhs;
  if (rhs <= lhs) return lhs;

  assertx(lhs.typeCns() && rhs.typeCns());

  // We're unwilling to unify with interfaces, so just return Top.
  if (!isNormalType(lhs.typeCns()) || !isNormalType(rhs.typeCns())) {
    return Top();
  }

  // Unify to a common ancestor if possible.
  if (auto t = lhs.typeCns()->commonAncestor(rhs.typeCns())) {
    return ClsRecSpec<T>(t, ClsRecSpec<T>::SubTag{});
  }

  return Top();
}

///////////////////////////////////////////////////////////////////////////////

#undef IMPLEMENT_SPEC_OPERS

}}
