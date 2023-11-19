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

#pragma once

#include <algorithm>
#include <atomic>
#include <memory>
#include <type_traits>

#include <folly/Indestructible.h>
#include <folly/Memory.h>
#include <folly/Portability.h>
#include <folly/Range.h>
#include <folly/Traits.h>
#include <folly/container/F14Map.h>
#include <folly/lang/Align.h>
#include <folly/lang/Exception.h>
#include <folly/synchronization/AtomicUtil.h>

#include <thrift/lib/cpp/protocol/TType.h>
#include <thrift/lib/cpp2/Thrift.h>
#include <thrift/lib/cpp2/op/Compare.h>

namespace apache {
namespace thrift {
namespace detail {

namespace st {

template <typename Int>
struct alignas(folly::cacheline_align_v) enum_find {
  struct find_name_result {
    folly::StringPiece result{nullptr, nullptr};
    find_name_result() = default;
    explicit constexpr find_name_result(folly::StringPiece r) noexcept
        : result{r} {}
    explicit constexpr operator bool() const noexcept {
      return result.data() != nullptr;
    }
  };
  struct find_value_result {
    bool found{false};
    Int result{0};
    find_value_result() = default;
    explicit constexpr find_value_result(Int r) noexcept
        : found{true}, result{r} {}
    explicit constexpr operator bool() const noexcept { return found; }
  };

  // metadata for the slow path to fill the caches for the fast path
  struct metadata {
    std::size_t const size{};
    const Int* const values{};
    const folly::StringPiece* const names{};
  };

  // the fast path cache types
  using find_name_map_t = folly::F14FastMap<Int, folly::StringPiece>;
  using find_value_map_t = folly::F14FastMap<folly::StringPiece, Int>;

  // an approximate state of the fast-path caches; approximately mutex-like
  struct cache_state {
    std::atomic<unsigned> cell{0}; // 0 init, +1 locked, -1 ready
    FOLLY_ERASE bool ready() noexcept {
      return folly::to_signed(cell.load(std::memory_order_acquire)) < 0;
    }
    FOLLY_ERASE bool try_lock() noexcept {
      return !folly::atomic_fetch_set(cell, 0, std::memory_order_relaxed);
    }
    FOLLY_ERASE bool unlock(bool ready) noexcept {
      return cell.store(ready ? -1 : 0, std::memory_order_release), ready;
    }
  };

  // the fast-path caches, guarded by the cache-state
  struct bidi_cache {
    find_name_map_t find_name_index;
    find_value_map_t find_value_index;

    FOLLY_NOINLINE explicit bidi_cache(const metadata& meta_) {
      find_name_index.reserve(meta_.size);
      find_value_index.reserve(meta_.size);
      for (std::size_t i = 0; i < meta_.size; ++i) {
        find_name_index.emplace(meta_.values[i], meta_.names[i].data());
        find_value_index.emplace(meta_.names[i].data(), meta_.values[i]);
      }
    }
  };

  // these fields all fit within a single cache line for fast path performance
  // the metadata is stored separately since it is used only in the slow path
  cache_state state; // protects the fast-path caches
  folly::aligned_storage_for_t<bidi_cache> cache{}; // the fast-path caches
  const metadata& meta; // source for the fast-path caches

  FOLLY_ERASE explicit constexpr enum_find(const metadata& meta_) noexcept
      : meta{meta_} {}

  FOLLY_NOINLINE bool prep_and_unlock() noexcept {
    const auto try_ = [&] { return ::new (&cache) bidi_cache(meta), true; };
    const auto catch_ = []() noexcept { return false; };
    return state.unlock(folly::catch_exception(try_, +catch_));
  }
  FOLLY_ERASE bool try_prepare() noexcept {
    return state.try_lock() && prep_and_unlock();
  }

  FOLLY_ERASE find_name_result find_name_fast(const Int value) noexcept {
    using result = find_name_result;
    const auto& map = reinterpret_cast<bidi_cache&>(cache).find_name_index;
    const auto found = map.find(value);
    return found == map.end() ? result() : result(found->second);
  }
  FOLLY_NOINLINE find_name_result find_name_scan(const Int value) noexcept {
    using result = find_name_result;
    // reverse order to simulate loop-map-insert then map-find
    const auto range = folly::range(meta.values, meta.values + meta.size);
    const auto found = range.rfind(value);
    return found == range.npos ? result() : result(meta.names[found]);
  }
  // param order optimizes outline findName by minimizing native instructions
  FOLLY_NOINLINE static find_name_result find_name(
      const Int value, enum_find& self) noexcept {
    // with two likelinesses v.s. one, gets the right code layout
    return FOLLY_LIKELY(self.state.ready()) || FOLLY_LIKELY(self.try_prepare())
        ? self.find_name_fast(value)
        : self.find_name_scan(value);
  }

  FOLLY_ERASE find_value_result
  find_value_fast(folly::StringPiece const name) noexcept {
    using result = find_value_result;
    const auto& map = reinterpret_cast<bidi_cache&>(cache).find_value_index;
    const auto found = map.find(name);
    return found == map.end() ? result() : result(found->second);
  }
  FOLLY_NOINLINE find_value_result
  find_value_scan(folly::StringPiece const name) noexcept {
    using result = find_value_result;
    // reverse order to simulate loop-map-insert then map-find
    const auto range = folly::range(meta.names, meta.names + meta.size);
    const auto found = range.rfind(name);
    return found == range.npos ? result() : result(meta.values[found]);
  }
  // param order optimizes outline findValue by minimizing native instructions
  FOLLY_NOINLINE static find_value_result find_value(
      folly::StringPiece const name, enum_find& self) noexcept {
    // with two likelinesses v.s. one, gets the right code layout
    return FOLLY_LIKELY(self.state.ready()) || FOLLY_LIKELY(self.try_prepare())
        ? self.find_value_fast(name)
        : self.find_value_scan(name);
  }
};
extern template struct enum_find<int>; // default

template <typename E, typename U = std::underlying_type_t<E>>
FOLLY_EXPORT FOLLY_ALWAYS_INLINE enum_find<U>& enum_find_instance() {
  using traits = TEnumTraits<E>;
  using metadata = typename enum_find<U>::metadata;
  const auto values = reinterpret_cast<const U*>(traits::values.data());
  static const metadata meta{traits::size, values, traits::names.data()};
  static enum_find<U> impl{meta};
  return impl;
}

template <typename E, typename U = std::underlying_type_t<E>>
FOLLY_ERASE bool enum_find_name(
    E const value, folly::StringPiece* const out) noexcept {
  const auto r = enum_find<U>::find_name(U(value), enum_find_instance<E>());
  return r && ((*out = r.result), true);
}

template <typename E, typename U = std::underlying_type_t<E>>
FOLLY_ERASE bool enum_find_value(
    folly::StringPiece const name, E* const out) noexcept {
  const auto uout = reinterpret_cast<U*>(out);
  const auto r = enum_find<U>::find_value(name, enum_find_instance<E>());
  return r && ((*uout = r.result), true);
}

//  copy_field_fn
//  copy_field
//
//  Returns a copy of a field. Used by structure copy-cosntructors.
//
//  Transitively copies through unique-ptr's, which are not copy-constructible.
template <typename TypeClass>
struct copy_field_fn;
template <typename TypeClass>
inline constexpr copy_field_fn<TypeClass> copy_field{};

template <typename>
struct copy_field_rec {
  template <typename T>
  T operator()(T const& t) const {
    return t;
  }
};

template <typename ValueTypeClass>
struct copy_field_rec<type_class::list<ValueTypeClass>> {
  template <typename T>
  T operator()(T const& t) const {
    T result;
    for (const auto& e : t) {
      result.push_back(copy_field<ValueTypeClass>(e));
    }
    return result;
  }
};

template <typename ValueTypeClass>
struct copy_field_rec<type_class::set<ValueTypeClass>> {
  template <typename T>
  T operator()(T const& t) const {
    T result;
    for (const auto& e : t) {
      result.emplace_hint(result.end(), copy_field<ValueTypeClass>(e));
    }
    return result;
  }
};

template <typename KeyTypeClass, typename MappedTypeClass>
struct copy_field_rec<type_class::map<KeyTypeClass, MappedTypeClass>> {
  template <typename T>
  T operator()(T const& t) const {
    T result;
    for (const auto& pair : t) {
      result.emplace_hint(
          result.end(),
          copy_field<KeyTypeClass>(pair.first),
          copy_field<MappedTypeClass>(pair.second));
    }
    return result;
  }
};

template <typename TypeClass>
struct copy_field_fn : copy_field_rec<TypeClass> {
  using rec = copy_field_rec<TypeClass>;

  using rec::operator();
  template <typename T>
  std::unique_ptr<T> operator()(const std::unique_ptr<T>& t) const {
    return !t ? nullptr : std::make_unique<T>((*this)(*t));
  }

  template <typename T, typename Alloc>
  std::unique_ptr<T, folly::allocator_delete<Alloc>> operator()(
      const std::unique_ptr<T, folly::allocator_delete<Alloc>>& t) const {
    return !t ? nullptr
              : folly::allocate_unique<T>(
                    t.get_deleter().get_allocator(), (*this)(*t));
  }
};

struct translate_field_name_table {
  size_t size;
  const folly::StringPiece* names;
  const int16_t* ids;
  const protocol::TType* types;
};

void translate_field_name(
    folly::StringPiece fname,
    int16_t& fid,
    protocol::TType& ftype,
    const translate_field_name_table& table) noexcept;

namespace {

//  gen_check_get_json
//
//  Metafunctions for getting the member types named, respectively,
//    * __fbthrift_cpp2_gen_json
struct gen_check_get_json {
  template <typename Type>
  using apply = decltype(private_access::__fbthrift_cpp2_gen_json<Type>());
};

//  gen_check_get
//
//  Metafunction for applying Get over Type and for handling the case where
//  Get fails to apply.
//
//  Get is one of the getters above:
//    * gen_check_get_json
//
//  When Get::apply<Type>:
//    * fails to apply (because cpp.type is in use), treat as true
//    * returns signed (extern template instances are generated), treat as true
//    * returns unsigned (otherwise), treat as false
//
//  The tag types signed and unsigned are used in the generated code to minimize
//  the overhead of parsing the class body, shifting all overhead to the code
//  which inspects these tags.
template <typename Void, typename Get, typename Type>
constexpr bool gen_check_get_ = true;
template <typename Get, typename Type>
constexpr bool gen_check_get_<
    folly::void_t<typename Get::template apply<Type>>,
    Get,
    Type> = Get::template apply<Type>::value;
template <typename Get, typename Type>
constexpr bool gen_check_get = gen_check_get_<void, Get, Type>;

//  gen_check_rec
//
//  Metafunction for recursing through container types to apply the metafunction
//  gen_check_get over struct/union types.
//
//  Get is one of the getters above:
//    * gen_check_get_json
template <typename TypeClass>
struct gen_check_rec {
  template <typename Get, typename Type>
  static constexpr bool apply = true;
};
template <typename ValueTypeClass>
struct gen_check_rec_list_set {
  using ValueTraits = gen_check_rec<ValueTypeClass>;
  template <typename Get, typename Type>
  static constexpr bool apply =
      ValueTraits::template apply<Get, typename Type::value_type>;
};
template <typename ValueTypeClass>
struct gen_check_rec<type_class::list<ValueTypeClass>>
    : gen_check_rec_list_set<ValueTypeClass> {};
template <typename ValueTypeClass>
struct gen_check_rec<type_class::set<ValueTypeClass>>
    : gen_check_rec_list_set<ValueTypeClass> {};
template <typename KeyTypeClass, typename MappedTypeClass>
struct gen_check_rec<type_class::map<KeyTypeClass, MappedTypeClass>> {
  using KeyTraits = gen_check_rec<KeyTypeClass>;
  using MappedTraits = gen_check_rec<MappedTypeClass>;
  template <typename Get, typename Type>
  static constexpr bool apply =
      KeyTraits::template apply<Get, typename Type::key_type>&&
          MappedTraits::template apply<Get, typename Type::mapped_type>;
};
struct gen_check_rec_structure_variant {
  template <typename Get, typename Type>
  static constexpr bool apply = gen_check_get<Get, Type>;
};
template <>
struct gen_check_rec<type_class::structure> : gen_check_rec_structure_variant {
};
template <>
struct gen_check_rec<type_class::variant> : gen_check_rec_structure_variant {};

//  gen_check
//
//  Returns whether, if the property Get holds for the outer structure Type,
//  that it also holds for each structure-typed field FieldType of the outer
//  type, peering through containers.
//
//  Get is one of the getters above:
//    * gen_check_get_json
template <
    typename Get,
    typename Type,
    typename FieldTypeClass,
    typename FieldType>
constexpr bool gen_check = !gen_check_get<Get, Type> ||
    gen_check_rec<FieldTypeClass>::template apply<Get, FieldType>;

//  gen_check_json
//
//  Aliases to gen_check partially instantiated with one of the getters above:
//    * gen_check_get_json
//
//  Used by a generated static_assert to enforce consistency over transitive
//  dependencies in the use of extern-template instantiations over json.
template <typename Type, typename FieldTypeClass, typename FieldType>
constexpr bool gen_check_json =
    gen_check<gen_check_get_json, Type, FieldTypeClass, FieldType>;

} // namespace

} // namespace st

template <class T>
bool pointer_equal(const T& lhs, const T& rhs) {
  return lhs && rhs ? *lhs == *rhs : lhs == rhs;
}

template <class T>
bool pointer_less(const T& lhs, const T& rhs) {
  return lhs && rhs ? *lhs < *rhs : lhs < rhs;
}

} // namespace detail
} // namespace thrift
} // namespace apache
