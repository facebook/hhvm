/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef THRIFT_CPP2_H_
#define THRIFT_CPP2_H_

#include <thrift/lib/cpp/Field.h>
#include <thrift/lib/cpp/Thrift.h>
#include <thrift/lib/cpp2/TypeClass.h>

#include <initializer_list>
#include <utility>
#include <folly/Traits.h>
#include <folly/Utility.h>
#include <folly/functional/Invoke.h>

#include <cstdint>
#include <memory>
#include <type_traits>

static_assert(FOLLY_CPLUSPLUS >= 201703L, "__cplusplus >= 201703L");

namespace apache {
namespace thrift {

namespace detail {
template <typename Tag>
struct invoke_reffer;
} // namespace detail

template <typename Tag>
using access_field_fn = detail::invoke_reffer<Tag>;
template <typename Tag>
inline constexpr access_field_fn<Tag> access_field{};

enum FragileConstructor {
  FRAGILE,
};

// re-definition of the same enums from
// thrift/compiler/ast/t_exception.h
enum class ExceptionKind {
  UNSPECIFIED = 0,
  TRANSIENT = 1, // The associated RPC may succeed if retried.
  STATEFUL = 2, // Server state must be change for the associated RPC to have
                // any chance of succeeding.
  PERMANENT =
      3, // The associated RPC can never succeed, and should not be retried.
};

enum class ExceptionBlame {
  UNSPECIFIED = 0,
  SERVER = 1, // The error was the fault of the server.
  CLIENT = 2, // The error was the fault of the client's request.
};

enum class ExceptionSafety {
  UNSPECIFIED = 0,
  SAFE = 1, // It is guaranteed the associated RPC failed completely, and no
            // significant server state changed while trying to process the
            // RPC.
};

namespace detail {
namespace st {

//  (struct_)private_access
//
//  Thrift generated types have private members but it may be necessary for the
//  Thrift support library to access those private members.
//
struct struct_private_access {
  //  These should be alias templates but Clang has a bug where it does not
  //  permit member alias templates of a friend struct to access private
  //  members of the type to which it is a friend. Making these function
  //  templates is a workaround.
  template <typename T>
  static folly::bool_constant<T::__fbthrift_cpp2_gen_json> //
  __fbthrift_cpp2_gen_json();

  template <typename T>
  static folly::bool_constant<T::__fbthrift_cpp2_is_runtime_annotation> //
  __fbthrift_cpp2_is_runtime_annotation();

  template <typename T>
  static const char* __fbthrift_thrift_uri() {
    return T::__fbthrift_thrift_uri();
  }

  template <typename T, typename Ord>
  static const folly::StringPiece __fbthrift_get_field_name() {
    return T::__fbthrift_get_field_name(Ord::value);
  }

  template <typename T>
  static const folly::StringPiece __fbthrift_get_class_name() {
    return T::__fbthrift_get_class_name();
  }

  template <typename T>
  static constexpr ExceptionSafety __fbthrift_cpp2_gen_exception_safety() {
    return T::__fbthrift_cpp2_gen_exception_safety;
  }

  template <typename T>
  static constexpr ExceptionKind __fbthrift_cpp2_gen_exception_kind() {
    return T::__fbthrift_cpp2_gen_exception_kind;
  }

  template <typename T>
  static constexpr ExceptionBlame __fbthrift_cpp2_gen_exception_blame() {
    return T::__fbthrift_cpp2_gen_exception_blame;
  }

  FOLLY_CREATE_MEMBER_INVOKER(clear_fn, __fbthrift_clear);
  FOLLY_CREATE_MEMBER_INVOKER(
      clear_terse_fields_fn, __fbthrift_clear_terse_fields);
  FOLLY_CREATE_MEMBER_INVOKER(empty_fn, __fbthrift_is_empty);

  template <typename T, typename Ord>
  static typename T::template __fbthrift_ident<Ord> __fbthrift_ident();

  template <typename T, typename Ord>
  using ident = decltype(__fbthrift_ident<T, Ord>());

  template <typename T, typename Ord>
  static typename T::template __fbthrift_id<Ord> __fbthrift_field_id();

  template <typename T, typename Ord>
  using field_id = decltype(__fbthrift_field_id<T, Ord>());

  template <typename T, typename Ord>
  static typename T::template __fbthrift_type_tag<Ord> __fbthrift_type_tag();

  template <typename T, typename Ord>
  using type_tag = decltype(__fbthrift_type_tag<T, Ord>());

  template <typename T, typename U>
  static typename T::template __fbthrift_ordinal<U> __fbthrift_ordinal();

  template <typename T, typename U>
  using ordinal = decltype(__fbthrift_ordinal<T, U>());

  template <typename T>
  static constexpr auto __fbthrift_field_size_v = T::__fbthrift_field_size_v;

  template <typename T>
  static typename T::__fbthrift_patch_struct __fbthrift_patch_struct();

  template <typename T>
  using patch_struct = decltype(__fbthrift_patch_struct<T>());
};
//  TODO(dokwon): Remove all usage of struct_private_access and standardize on
//  private_access.
using private_access = struct_private_access;

template <typename T, typename = void>
struct IsThriftClass : std::false_type {};

template <typename T>
struct IsThriftClass<T, folly::void_t<typename T::__fbthrift_cpp2_type>>
    : std::true_type {};

template <typename T, typename = void>
struct IsThriftUnion : std::false_type {};

template <typename T>
struct IsThriftUnion<T, folly::void_t<typename T::__fbthrift_cpp2_type>>
    : folly::bool_constant<T::__fbthrift_cpp2_is_union> {};

// __fbthrift_clear_terse_fields should be called for a terse struct field
// before deserialization so that it only clears out terse fields in a terse
// struct.
using clear_terse_fields_fn = private_access::clear_terse_fields_fn;
inline static constexpr clear_terse_fields_fn clear_terse_fields{};

} // namespace st
} // namespace detail

using clear_fn = detail::st::private_access::clear_fn;
inline constexpr clear_fn clear{};

using empty_fn = detail::st::private_access::empty_fn;
inline static constexpr empty_fn empty{};

// TODO(dokwon): Add apache::thrift::uri support for generated enum types.
template <typename T>
FOLLY_EXPORT const std::string& uri() {
  static const auto& kUri =
      *new std::string(detail::st::private_access::__fbthrift_thrift_uri<T>());
  return kUri;
}

template <typename T>
constexpr bool is_thrift_class_v =
    apache::thrift::detail::st::IsThriftClass<T>::value;

template <typename T>
constexpr bool is_thrift_union_v =
    apache::thrift::detail::st::IsThriftUnion<T>::value;

template <typename T>
constexpr bool is_thrift_exception_v = is_thrift_class_v<T>&&
    std::is_base_of<apache::thrift::TException, T>::value;

template <typename T>
constexpr bool is_thrift_struct_v =
    is_thrift_class_v<T> && !is_thrift_union_v<T> && !is_thrift_exception_v<T>;

template <typename T, typename Fallback>
using type_class_of_thrift_class_or_t = //
    folly::conditional_t<
        is_thrift_union_v<T>,
        type_class::variant,
        folly::conditional_t<
            is_thrift_class_v<T>, // struct or exception
            type_class::structure,
            Fallback>>;

template <typename T, typename Fallback>
using type_class_of_thrift_class_enum_or_t = //
    folly::conditional_t<
        std::is_enum<T>::value,
        type_class::enumeration,
        type_class_of_thrift_class_or_t<T, Fallback>>;

template <typename T>
using type_class_of_thrift_class_t = type_class_of_thrift_class_or_t<T, void>;

template <typename T>
using type_class_of_thrift_class_enum_t =
    type_class_of_thrift_class_enum_or_t<T, void>;

namespace detail {

template <typename T>
struct enum_hash {
  size_t operator()(T t) const {
    using underlying_t = typename std::underlying_type<T>::type;
    return std::hash<underlying_t>()(underlying_t(t));
  }
};

} // namespace detail

namespace detail {

// Adapted from Fatal (https://github.com/facebook/fatal/)
// Inlined here to keep the amount of mandatory dependencies at bay
// For more context, see http://ericniebler.com/2013/08/07/
// - Universal References and the Copy Constructor
template <typename, typename...>
struct is_safe_overload {
  using type = std::true_type;
};
template <typename Class, typename T>
struct is_safe_overload<Class, T> {
  using type = std::integral_constant<
      bool,
      !std::is_same<
          Class,
          typename std::remove_cv<
              typename std::remove_reference<T>::type>::type>::value>;
};

} // namespace detail

template <typename Class, typename... Args>
using safe_overload_t = typename std::enable_if<
    apache::thrift::detail::is_safe_overload<Class, Args...>::type::value>::
    type;

} // namespace thrift
} // namespace apache

#define FBTHRIFT_CPP_DEFINE_MEMBER_INDIRECTION_FN(...)                       \
  struct __fbthrift_cpp2_indirection_fn {                                    \
    template <typename __fbthrift_t>                                         \
    FOLLY_ERASE constexpr auto operator()(__fbthrift_t&& __fbthrift_v) const \
        noexcept(                                                            \
            noexcept(static_cast<__fbthrift_t&&>(__fbthrift_v).__VA_ARGS__)) \
            -> decltype((                                                    \
                static_cast<__fbthrift_t&&>(__fbthrift_v).__VA_ARGS__)) {    \
      return static_cast<__fbthrift_t&&>(__fbthrift_v).__VA_ARGS__;          \
    }                                                                        \
  }

namespace apache {
namespace thrift {

template <typename T>
using detect_indirection_fn_t = typename T::__fbthrift_cpp2_indirection_fn;

template <typename T>
using indirection_fn_t =
    folly::detected_or_t<folly::identity_fn, detect_indirection_fn_t, T>;

namespace detail {
struct apply_indirection_fn {
 private:
  template <typename T>
  using i = indirection_fn_t<folly::remove_cvref_t<T>>;

 public:
  template <typename T>
  FOLLY_ERASE constexpr auto operator()(T&& t) const
      noexcept(noexcept(i<T>{}(static_cast<T&&>(t))))
          -> decltype(i<T>{}(static_cast<T&&>(t))) {
    return i<T>{}(static_cast<T&&>(t));
  }
};
} // namespace detail

inline constexpr detail::apply_indirection_fn apply_indirection;

class ExceptionMetadataOverrideBase {
 public:
  virtual ~ExceptionMetadataOverrideBase() {}

  ExceptionKind errorKind() const { return errorKind_; }

  ExceptionBlame errorBlame() const { return errorBlame_; }

  ExceptionSafety errorSafety() const { return errorSafety_; }

  virtual const std::type_info* type() const = 0;

 protected:
  ExceptionKind errorKind_{ExceptionKind::UNSPECIFIED};
  ExceptionBlame errorBlame_{ExceptionBlame::UNSPECIFIED};
  ExceptionSafety errorSafety_{ExceptionSafety::UNSPECIFIED};
};

template <typename T>
class ExceptionMetadataOverride : public T,
                                  public ExceptionMetadataOverrideBase {
 public:
  explicit ExceptionMetadataOverride(const T& t) : T(t) {}
  explicit ExceptionMetadataOverride(T&& t) : T(std::move(t)) {}

  const std::type_info* type() const override {
#if FOLLY_HAS_RTTI
    return &typeid(T);
#else
    return nullptr;
#endif
  }

  // ExceptionKind
  ExceptionMetadataOverride& setTransient() {
    errorKind_ = ExceptionKind::TRANSIENT;
    return *this;
  }
  ExceptionMetadataOverride& setPermanent() {
    errorKind_ = ExceptionKind::PERMANENT;
    return *this;
  }
  ExceptionMetadataOverride& setStateful() {
    errorKind_ = ExceptionKind::STATEFUL;
    return *this;
  }

  // ExceptionBlame
  ExceptionMetadataOverride& setClient() {
    errorBlame_ = ExceptionBlame::CLIENT;
    return *this;
  }
  ExceptionMetadataOverride& setServer() {
    errorBlame_ = ExceptionBlame::SERVER;
    return *this;
  }

  // ExceptionSafety
  ExceptionMetadataOverride& setSafe() {
    errorSafety_ = ExceptionSafety::SAFE;
    return *this;
  }
};

template <typename T>
ExceptionMetadataOverride<std::decay_t<T>> overrideExceptionMetadata(T&& ex) {
  return ExceptionMetadataOverride<std::decay_t<T>>(std::forward<T>(ex));
}

namespace detail {

enum LazyDeserializationState : uint8_t { // Bitfield.
  UNTAINTED = 1 << 0,
  DESERIALIZED = 1 << 1,
};

template <typename Alloc>
void move_allocator_impl(std::false_type, Alloc&, Alloc&) {}

template <typename Alloc>
void move_allocator_impl(std::true_type, Alloc& dst, Alloc& other) {
  dst = std::move(other);
}

template <typename Alloc>
void move_allocator(Alloc& dst, Alloc& other) {
  using alloc_traits = std::allocator_traits<Alloc>;
  using pocma = typename alloc_traits::propagate_on_container_move_assignment;
  move_allocator_impl(pocma{}, dst, other);
}

template <typename Alloc>
void copy_allocator_impl(std::false_type, Alloc&, const Alloc&) {}

template <typename Alloc>
void copy_allocator_impl(std::true_type, Alloc& dst, const Alloc& other) {
  dst = other;
}

template <typename Alloc>
void copy_allocator(Alloc& dst, const Alloc& other) {
  using alloc_traits = std::allocator_traits<Alloc>;
  using pocca = typename alloc_traits::propagate_on_container_copy_assignment;
  copy_allocator_impl(pocca{}, dst, other);
}

template <typename Alloc>
void swap_allocators_impl(std::false_type, Alloc&, Alloc&) {}

template <typename Alloc>
void swap_allocators_impl(std::true_type, Alloc& a, Alloc& b) {
  using namespace std;
  swap(a, b);
}

template <typename Alloc>
void swap_allocators(Alloc& a, Alloc& b) {
  using alloc_traits = std::allocator_traits<Alloc>;
  using pocs = typename alloc_traits::propagate_on_container_swap;
  swap_allocators_impl(pocs{}, a, b);
}

} // namespace detail

} // namespace thrift
} // namespace apache

#endif // #ifndef THRIFT_CPP2_H_
