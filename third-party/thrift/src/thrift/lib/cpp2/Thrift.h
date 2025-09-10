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

#include <utility>
#include <folly/Traits.h>
#include <folly/Utility.h>
#include <folly/functional/Invoke.h>

#include <cstdint>
#include <memory>
#include <type_traits>

static_assert(
    FOLLY_CPLUSPLUS >= 202002L, "Thrift must be built with C++20 or later.");

namespace apache::thrift {

namespace detail {

template <typename Tag>
struct invoke_reffer;

template <class List, FieldOrdinal>
struct at_impl {
  static_assert(folly::always_false<List>);
};

template <class... Args, FieldOrdinal Ord>
struct at_impl<folly::tag_t<Args...>, Ord> {
  using type =
      folly::type_pack_element_t<folly::to_underlying(Ord), void, Args...>;
};

// Similar to mp_at in boost mp11, but Ordinal based
template <class List, FieldOrdinal Ord>
using at = typename at_impl<List, Ord>::type;

template <typename T, typename TagList>
inline constexpr bool contains_v = false;

template <typename T, typename... Types>
inline constexpr bool contains_v<T, folly::tag_t<Types...>> =
    folly::is_one_of_v<T, Types...>;

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

namespace detail::st {

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
  static std::bool_constant<T::__fbthrift_cpp2_gen_json> //
  __fbthrift_cpp2_gen_json();

  template <typename T>
  static std::bool_constant<T::__fbthrift_cpp2_uses_op_encode> //
  __fbthrift_cpp2_uses_op_encode();

  template <typename T>
  static std::bool_constant<T::__fbthrift_cpp2_is_runtime_annotation> //
  __fbthrift_cpp2_is_runtime_annotation();

  template <typename T>
  static std::string_view __fbthrift_thrift_uri() {
    return T::__fbthrift_thrift_uri();
  }
  template <typename T, typename = void>
  struct detect_uri : std::false_type {};
  template <typename T>
  struct detect_uri<T, folly::void_t<decltype(T::__fbthrift_thrift_uri())>>
      : std::true_type {};

  template <typename T, typename Ord>
  static const folly::StringPiece __fbthrift_get_field_name() {
    static_assert(
        1 <= size_t(Ord::value) && size_t(Ord::value) <= num_fields<T>,
        "Field not found");
    return T::__fbthrift_get_field_name(Ord::value);
  }

  template <typename T>
  static const folly::StringPiece __fbthrift_get_class_name() {
    return T::__fbthrift_get_class_name();
  }

  template <typename T, typename Annotation, typename = void>
  struct __fbthrift_has_struct_annotation_impl : std::false_type {};

  template <typename T, typename Annotation>
  struct __fbthrift_has_struct_annotation_impl<
      T,
      Annotation,
      folly::void_t<typename T::__fbthrift_struct_annotations>>
      : std::bool_constant<
            contains_v<Annotation, typename T::__fbthrift_struct_annotations>> {
  };

  template <typename T, typename Annotation>
  static constexpr bool __fbthrift_has_struct_annotation() {
    return __fbthrift_has_struct_annotation_impl<T, Annotation>::value;
  }

  template <typename T>
  static constexpr const FieldId* __fbthrift_list_of_field_with_annotation() {
    return T::__fbthrift_list_of_field_with_annotation;
  }

  template <typename T, typename Annotation, std::size_t Index, typename = void>
  struct __fbthrift_check_field_annotation : std::false_type {};

  template <typename T, typename Annotation, std::size_t Index>
      struct __fbthrift_check_field_annotation < T,
      Annotation, Index,
      std::enable_if_t<Index<
          folly::type_list_size_v<typename T::__fbthrift_field_annotations>>> {
    static constexpr bool value = contains_v<
        Annotation,
        folly::type_list_element_t<
            Index,
            typename T::__fbthrift_field_annotations>>;
  };

  template <typename T, typename FieldId, typename Annotation, typename = void>
  struct __fbthrift_has_field_annotation_impl : std::false_type {};

  template <typename T, typename FieldId, typename Annotation>
  struct __fbthrift_has_field_annotation_impl<
      T,
      FieldId,
      Annotation,
      folly::void_t<
          typename T::__fbthrift_list_of_field_with_annotation,
          typename T::__fbthrift_field_annotations>> {
    static_assert(type::is_field_id_v<FieldId>, "FieldId must be a field_id");

    static constexpr std::size_t field_index = folly::type_list_find_v<
        FieldId,
        typename T::__fbthrift_list_of_field_with_annotation>;

    static constexpr bool value =
        __fbthrift_check_field_annotation<T, Annotation, field_index>::value;
  };

  template <typename T, typename FieldId, typename Annotation>
  static constexpr bool __fbthrift_has_field_annotation() {
    return __fbthrift_has_field_annotation_impl<T, FieldId, Annotation>::value;
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

  template <typename T>
  static constexpr auto num_fields = T::__fbthrift_num_fields;

  template <typename T>
  static constexpr const int16_t* field_ids() {
    return T::__fbthrift_reflection_field_ids;
  }

  // This is a function and not an alias to workaround a bug in clang 18 and
  // older: https://github.com/llvm/llvm-project/issues/66604.
  // See https://www.godbolt.org/z/n3fbbEd9v.
  template <typename T>
  static typename T::__fbthrift_reflection_idents idents();

  template <typename T>
  static typename T::__fbthrift_reflection_type_tags type_tags();

  template <typename T, typename... Args>
  static constexpr std::string_view __fbthrift_get_module_name() {
    return T::template __fbthrift_get_module_name<Args...>();
  }
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
    : std::bool_constant<T::__fbthrift_cpp2_is_union> {};

template <typename T>
using detect_complete = decltype(sizeof(T));

// __fbthrift_clear_terse_fields should be called for a terse struct field
// before deserialization so that it only clears out terse fields in a terse
// struct.
using clear_terse_fields_fn = private_access::clear_terse_fields_fn;
inline static constexpr clear_terse_fields_fn clear_terse_fields{};

} // namespace detail::st

using clear_fn = detail::st::private_access::clear_fn;
inline constexpr clear_fn clear{};

using empty_fn = detail::st::private_access::empty_fn;
inline static constexpr empty_fn empty{};

template <typename T>
FOLLY_EXPORT const std::string& uri() {
  static_assert(
      folly::is_detected_v<detail::st::detect_complete, T> ||
          folly::is_detected_v<detail::st::detect_complete, Client<T>>,
      "T must be a complete type or service tag.");
  std::string_view uri;
  if constexpr (detail::st::private_access::detect_uri<T>::value) {
    uri = detail::st::private_access::__fbthrift_thrift_uri<T>();
  } else if constexpr (detail::st::private_access::detect_uri<
                           TEnumTraits<T>>::value) {
    uri = detail::st::private_access::__fbthrift_thrift_uri<TEnumTraits<T>>();
  } else if constexpr (detail::st::private_access::detect_uri<
                           Client<T>>::value) {
    uri = detail::st::private_access::__fbthrift_thrift_uri<Client<T>>();
  } else {
    // MSVC and GCC fire this assert even when we took an earlier branch...
    if constexpr (!folly::kIsWindows && !folly::kGnuc) {
      static_assert(folly::always_false<T>, "No URI defined for type");
    } else {
      // This will fail to build because we checked earlier that no URI is
      // defined for this type.
      uri = detail::st::private_access::__fbthrift_thrift_uri<T>();
    }
  }
  static const auto& kUri = *new std::string(uri);
  return kUri;
}

template <typename T>
constexpr bool is_thrift_class_v =
    apache::thrift::detail::st::IsThriftClass<T>::value;

template <typename T>
constexpr bool is_thrift_union_v =
    apache::thrift::detail::st::IsThriftUnion<T>::value;

template <typename T>
constexpr bool is_thrift_exception_v =
    is_thrift_class_v<T> && std::is_base_of_v<apache::thrift::TException, T>;

template <typename T>
constexpr bool is_thrift_struct_v =
    is_thrift_class_v<T> && !is_thrift_union_v<T> && !is_thrift_exception_v<T>;

template <typename T>
constexpr bool is_thrift_service_tag_v = //
    folly::is_detected_v< //
        detail::st::detect_complete,
        apache::thrift::Client<T>> ||
    folly::is_detected_v<
        detail::st::detect_complete,
        apache::thrift::ServiceHandler<T>>;

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
        std::is_enum_v<T>,
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
    using underlying_t = std::underlying_type_t<T>;
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
      !std::is_same_v<Class, std::remove_cv_t<std::remove_reference_t<T>>>>;
};

} // namespace detail

template <typename Class, typename... Args>
using safe_overload_t = std::enable_if_t<
    apache::thrift::detail::is_safe_overload<Class, Args...>::type::value>;

} // namespace apache::thrift

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

namespace apache::thrift {

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

// We identify field quailfier using different types of C++ field_ref. For
// cpp.ref fields, we can not deduce the field qualifier information.
namespace qualifier {
template <class Struct, class Id>
struct is_cpp_ref_field_optional : std::false_type {
  static_assert(sizeof(Struct), "Struct must be a complete type.");
};
template <class Struct, class Id>
struct is_cpp_ref_field_terse : std::false_type {
  static_assert(sizeof(Struct), "Struct must be a complete type.");
};
// Identify a field has deprecated terse write optimization with custom default
// value specified in IDL which differs from the intrinsic default value.
template <class Struct, class Id>
struct is_deprecated_terse_writes_with_custom_default_field : std::false_type {
  static_assert(sizeof(Struct), "Struct must be a complete type.");
};
} // namespace qualifier

template <class T, class F>
constexpr std::bool_constant<std::is_invocable_v<F&&, T&>> callable(F&&) {
  return {};
}

// __FBTHRIFT_IS_VALID(VARIABLE, EXPRESSION)
//
// Uses SFINAE to check whether expression is valid. The caller must specifies
// one variable so that the expression depends on its type.
//
// e.g.,
//
//   if constexpr (__FBTHRIFT_IS_VALID(list, list.reserve(n))) {
//     list.reserve(n);
//   }
#define __FBTHRIFT_IS_VALID(VARIABLE, ...)                            \
  (::apache::thrift::detail::callable<decltype(VARIABLE)>(            \
       [&](auto&& VARIABLE) -> std::void_t<decltype(__VA_ARGS__)> {}) \
       .value)

/**
 * Specialization defn in _types.h / service.h
 */
template <typename T, bool IsEnum = std::is_enum_v<T>>
struct TSchemaAssociation {
  static_assert(folly::always_false<T>, "invalid use of base template");
};

} // namespace detail

} // namespace apache::thrift

#endif // #ifndef THRIFT_CPP2_H_
