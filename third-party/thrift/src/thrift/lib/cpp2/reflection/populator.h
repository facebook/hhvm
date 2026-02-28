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
#include <iostream>
#include <random>
#include <type_traits>
#include <typeindex>
#include <vector>

#include <folly/FBString.h>
#include <folly/Traits.h>
#include <folly/io/Cursor.h>
#include <thrift/lib/cpp2/FieldRefTraits.h>
#include <thrift/lib/cpp2/Thrift.h>
#include <thrift/lib/cpp2/op/Create.h>
#include <thrift/lib/cpp2/op/Get.h>
#include <thrift/lib/cpp2/type/NativeType.h>

namespace apache::thrift::populator {

struct populator_opts {
  // Container sizes are chosen randomly (geometrically distributed)
  bool random_container_size = true;
  // (Average) container sizes
  size_t list_len = 10;
  size_t set_len = 10;
  size_t map_len = 10;
  size_t bin_len = 10;
  size_t str_len = 10;
  // Probability to use for populating optional fields.
  float optional_field_prob = 0.0;
  size_t recursion_limit = 0;
  // A budget for the total size of the generated object.
  // The populator will stop generating more list/set/map elements if the budget
  // is reached. Note that the budget is not counting container overhead, so the
  // generated object may be slightly larger than the budget.
  size_t size_budget = 100 * 1024 * 1024;
};

namespace detail {
// generate a value with a Bernoulli distribution:
// p is the probability of true, (1-p) for false
template <typename Rng>
bool get_bernoulli(Rng& rng, float p) {
  std::bernoulli_distribution gen(p);
  return gen(rng);
}

// generate a value of type Int within [min, max]
template <typename Int, typename Rng>
Int rand_in_range(Rng& rng, Int min, Int max) {
  // uniform_int_distribution undefined for char,
  // use the next larger type if it's small
  using int_type = std::conditional_t<
      (sizeof(Int) > 1),
      Int,
      std::conditional_t<
          std::numeric_limits<Int>::is_signed,
          signed short,
          unsigned short>>;

  std::uniform_int_distribution<int_type> gen(min, max);
  int_type tmp = gen(rng);
  return static_cast<Int>(tmp);
}

// pointer type for cpp2.ref fields, while discrimiminating against the
// pointer corner case in Thrift (e.g., a unqiue_pointer<folly::IOBuf>)
template <typename>
struct is_smart_pointer : std::false_type {};
template <typename D>
struct is_smart_pointer<std::unique_ptr<folly::IOBuf, D>> : std::false_type {};

// supported smart pointer types for cpp2.ref_type fields
template <typename T, typename D>
struct is_smart_pointer<std::unique_ptr<T, D>> : std::true_type {};
template <typename T>
struct is_smart_pointer<std::shared_ptr<T>> : std::true_type {};

template <typename T>
using enable_if_smart_pointer = std::enable_if_t<is_smart_pointer<T>::value>;

template <typename T>
using disable_if_smart_pointer = std::enable_if_t<!is_smart_pointer<T>::value>;

struct extract_descriptor_fid {
  template <typename T>
  using apply = typename T::metadata::id;
};

template <typename T, typename Enable = void>
struct deref;

// General case: we derference field_ref and return a reference to what it
// refers to.
template <typename T>
struct deref<T, disable_if_smart_pointer<T>> {
  static decltype(auto) clear_and_get(T& in) { return *in; }
};

// Special case: We specifically *do not* dereference a unique pointer to
// an IOBuf, because this is a type that the protocol can (de)serialize
// directly
template <>
struct deref<std::unique_ptr<folly::IOBuf>> {
  using T = std::unique_ptr<folly::IOBuf>;
  static T& clear_and_get(T& in) { return in; }
};

// General case: deref returns a reference to what the
// unique pointer contains
template <typename PtrType>
struct deref<PtrType, enable_if_smart_pointer<PtrType>> {
  using T = std::remove_const_t<typename PtrType::element_type>;
  static T& clear_and_get(std::shared_ptr<const T>& in) {
    auto t = std::make_shared<T>();
    auto ret = t.get();
    in = std::move(t);
    return *ret;
  }
  static T& clear_and_get(std::shared_ptr<T>& in) {
    in = std::make_shared<T>();
    return *in;
  }
  static T& clear_and_get(std::unique_ptr<T>& in) {
    in = std::make_unique<T>();
    return *in;
  }
};

template <typename T>
using infer_tag = type::infer_tag<T, true /* GuessStringTag */>;

template <typename Rng>
struct State {
  Rng& rng;
  const populator_opts& opts;
  std::map<std::type_index, size_t> tag_counts;
  size_t size{};
};

// Using the type_info of the tags to check if we are populating recursively
template <typename Tag>
class RecursionGuard {
 private:
  size_t& cnt_;
  const bool can_recurse_;

 public:
  template <typename Rng>
  explicit RecursionGuard(State<Rng>& state)
      : cnt_(state.tag_counts[typeid(Tag)]),
        can_recurse_(cnt_ <= state.opts.recursion_limit) {
    if (can_recurse_) {
      cnt_++;
    }
  }

  ~RecursionGuard() {
    if (can_recurse_) {
      cnt_--;
    }
  }

  operator bool() const { return can_recurse_; }

  RecursionGuard(const RecursionGuard&) = delete;
  RecursionGuard(RecursionGuard&&) = delete;
  RecursionGuard& operator=(const RecursionGuard&) = delete;
  RecursionGuard& operator=(RecursionGuard&&) = delete;
};

template <typename Rng>
size_t get_container_size(State<Rng>& state, size_t mean_size) {
  if (state.opts.random_container_size) {
    double p = 1.0 / static_cast<double>(mean_size + 1);
    std::geometric_distribution<size_t> gen(p);
    return gen(state.rng);
  } else {
    return mean_size;
  }
}

} // namespace detail

template <typename Tag, typename Type, typename Enable = void>
struct populator_methods;

template <typename Int>
struct populator_methods<
    detail::infer_tag<Int>,
    Int,
    std::enable_if_t<type::is_a_v<detail::infer_tag<Int>, type::integral_c>>> {
  template <typename Rng>
  static Int next_value(Rng& rng) {
    using limits = std::numeric_limits<Int>;
    Int out = detail::rand_in_range(rng, limits::min(), limits::max());
    DVLOG(4) << "generated int: " << out;
    return out;
  }

  // Special overload to work with lists of booleans.
  template <typename Rng>
  static void populate(
      detail::State<Rng>& state, std::vector<bool>::reference out) {
    out = next_value(state.rng);
    state.size += sizeof(Int);
  }

  template <typename Rng>
  static void populate(detail::State<Rng>& state, Int& out) {
    out = next_value(state.rng);
    state.size += sizeof(Int);
  }
};

template <typename Fp>
struct populator_methods<
    detail::infer_tag<Fp>,
    Fp,
    std::enable_if_t<
        type::is_a_v<detail::infer_tag<Fp>, type::floating_point_c>>> {
  template <typename Rng>
  static void populate(detail::State<Rng>& state, Fp& out) {
    std::uniform_real_distribution<Fp> gen;
    out = gen(state.rng);
    state.size += sizeof(Fp);
    DVLOG(4) << "generated real: " << out;
  }
};

template <>
struct populator_methods<type::string_t, std::string> {
  template <typename Rng>
  static void populate(detail::State<Rng>& state, std::string& str) {
    using larger_char =
        std::conditional_t<std::numeric_limits<char>::is_signed, int, unsigned>;

    // all printable chars (see `man ascii`)
    std::uniform_int_distribution<larger_char> char_gen(0x20, 0x7E);

    const std::size_t length =
        detail::get_container_size(state, state.opts.str_len);

    str = std::string(length, 0);
    std::generate_n(str.begin(), length, [&]() {
      return static_cast<char>(char_gen(state.rng));
    });
    state.size += sizeof(char) * length;

    DVLOG(4) << "generated string of len" << length;
  }
};

template <>
struct populator_methods<
    type::cpp_type<folly::fbstring, type::string_t>,
    folly::fbstring> {
  template <typename Rng>
  static void populate(detail::State<Rng>& state, folly::fbstring& bin) {
    std::string t;
    populator_methods<type::string_t, std::string>::populate(state, t);
    bin = folly::fbstring(std::move(t));
  }
};

template <typename Rng, typename Binary, typename WriteFunc>
void generate_bytes(
    Rng& rng, Binary&, const std::size_t length, const WriteFunc& write_func) {
  std::uniform_int_distribution<unsigned> byte_gen(0, 0xFF);
  for (std::size_t i = 0; i < length; i++) {
    write_func(static_cast<uint8_t>(byte_gen(rng)));
  }
  DVLOG(4) << "generated binary of length " << length;
}

template <>
struct populator_methods<type::binary_t, std::string> {
  template <typename Rng>
  static void populate(detail::State<Rng>& state, std::string& bin) {
    const auto length = detail::get_container_size(state, state.opts.bin_len);
    bin = std::string(length, 0);
    auto iter = bin.begin();
    generate_bytes(state.rng, bin, length, [&](uint8_t c) { *iter++ = c; });
    state.size += length;
  }
};

template <>
struct populator_methods<
    type::cpp_type<folly::fbstring, type::binary_t>,
    folly::fbstring> {
  template <typename Rng>
  static void populate(detail::State<Rng>& state, folly::fbstring& bin) {
    std::string t;
    populator_methods<type::binary_t, std::string>::populate(state, t);
    bin = folly::fbstring(std::move(t));
  }
};

template <>
struct populator_methods<
    type::cpp_type<folly::IOBuf, type::binary_t>,
    folly::IOBuf> {
  template <typename Rng>
  static void populate(detail::State<Rng>& state, folly::IOBuf& bin) {
    const auto length = detail::get_container_size(state, state.opts.bin_len);
    bin = folly::IOBuf(folly::IOBuf::CREATE, length);
    bin.append(length);
    folly::io::RWUnshareCursor range(&bin);
    generate_bytes(
        state.rng, range, length, [&](uint8_t c) { range.write<uint8_t>(c); });
    state.size += length;
  }
};

template <>
struct populator_methods<
    type::cpp_type<std::unique_ptr<folly::IOBuf>, type::binary_t>,
    std::unique_ptr<folly::IOBuf>> {
  template <typename Rng>
  static void populate(
      detail::State<Rng>& state, std::unique_ptr<folly::IOBuf>& bin) {
    bin = std::make_unique<folly::IOBuf>();
    return populator_methods<
        type::cpp_type<folly::IOBuf, type::binary_t>,
        folly::IOBuf>::populate(state, *bin);
  }
};

// handle dereferencing smart pointers
template <typename Tag, typename PtrType>
struct populator_methods<
    Tag,
    PtrType,
    detail::enable_if_smart_pointer<PtrType>> {
  using element_type = typename PtrType::element_type;
  using type_methods = populator_methods<Tag, element_type>;

  template <typename Rng>
  static void populate(detail::State<Rng>& state, PtrType& out) {
    return type_methods::populate(state, *out);
  }
};

// Enumerations
template <typename Type>
struct populator_methods<
    detail::infer_tag<Type>,
    Type,
    std::enable_if_t<type::is_a_v<detail::infer_tag<Type>, type::enum_c>>> {
  using int_type = std::underlying_type_t<Type>;
  using int_methods = populator_methods<detail::infer_tag<int_type>, int_type>;

  template <typename Rng>
  static void populate(detail::State<Rng>& state, Type& out) {
    int_type tmp;
    int_methods::populate(state, tmp);
    out = static_cast<Type>(tmp);
    state.size += sizeof(Type);
  }
};

// Lists
template <typename ElemTag, typename Type>
struct populator_methods<type::list<ElemTag>, Type> {
  using elem_type = typename Type::value_type;
  using elem_methods = populator_methods<ElemTag, elem_type>;

  template <typename Rng>
  static void populate(detail::State<Rng>& state, Type& out) {
    auto recursion_guard = detail::RecursionGuard<type::list<ElemTag>>(state);
    if (!recursion_guard) {
      return;
    }

    std::uint32_t list_size =
        detail::get_container_size(state, state.opts.list_len);
    out = Type();

    DVLOG(3) << "populating list size " << list_size;

    for (decltype(list_size) i = 0;
         i < list_size && state.size < state.opts.size_budget;
         i++) {
      elem_methods::populate(state, out.emplace_back());
    }
  }
};

// Sets
template <typename ElemTag, typename Type>
struct populator_methods<type::set<ElemTag>, Type> {
  // TODO: fair amount of shared code bewteen this and specialization for
  // type_class::list
  using elem_type = typename Type::value_type;
  using elem_methods = populator_methods<ElemTag, elem_type>;

  template <typename Rng>
  static void populate(detail::State<Rng>& state, Type& out) {
    auto recursion_guard = detail::RecursionGuard<type::set<ElemTag>>(state);
    if (!recursion_guard) {
      return;
    }

    std::uint32_t set_size =
        detail::get_container_size(state, state.opts.set_len);

    DVLOG(3) << "populating set size " << set_size;
    out = Type();

    for (decltype(set_size) i = 0;
         i < set_size && state.size < state.opts.size_budget;
         i++) {
      elem_type tmp;
      elem_methods::populate(state, tmp);
      out.insert(std::move(tmp));
    }
  }
};

// Maps
template <typename KeyTag, typename MappedTag, typename Type>
struct populator_methods<type::map<KeyTag, MappedTag>, Type> {
  using key_type = typename Type::key_type;
  using mapped_type = typename Type::mapped_type;

  using key_methods = populator_methods<KeyTag, key_type>;
  using mapped_methods = populator_methods<MappedTag, mapped_type>;

  template <typename Rng>
  static void populate(detail::State<Rng>& state, Type& out) {
    auto recursion_guard =
        detail::RecursionGuard<type::map<KeyTag, MappedTag>>(state);
    if (!recursion_guard) {
      return;
    }

    std::uint32_t map_size =
        detail::get_container_size(state, state.opts.map_len);

    DVLOG(3) << "populating map size " << map_size;
    out = Type();

    for (decltype(map_size) i = 0;
         i < map_size && state.size < state.opts.size_budget;
         i++) {
      key_type key_tmp;
      key_methods::populate(state, key_tmp);
      mapped_methods::populate(state, out[std::move(key_tmp)]);
    }
  }
};

// specialization for variants (Thrift unions)
template <typename Union>
struct populator_methods<type::union_t<Union>, Union> {
  template <typename Rng>
  static void populate(detail::State<Rng>& state, Union& out) {
    DVLOG(0) << "begin writing union: "
             << op::get_class_name_v<Union> << ", type: "
             << folly::to_underlying(out.getType());

    const auto selected = static_cast<type::Ordinal>(
        detail::rand_in_range<size_t>(state.rng, 0, op::num_fields<Union> - 1));

    op::for_each_ordinal<Union>([&](auto ord) {
      using Ord = decltype(ord);
      if (ord != selected) {
        return;
      }
      using methods = populator_methods<
          op::get_type_tag<Union, Ord>,
          op::get_native_type<Union, Ord>>;

      DVLOG(0) << "writing union field "
               << op::get_name_v<Union, Ord> << ", fid: "
               << folly::to_underlying(op::get_field_id_v<Union, Ord>);

      methods::populate(state, op::get<Ord>(out).ensure());
    });

    DVLOG(0) << "end writing union";
  }
};

// specialization for structs
template <typename Struct>
struct populator_methods<type::struct_t<Struct>, Struct> {
 private:
  class member_populator {
   public:
    template <typename Ord, typename Rng>
    void operator()(Ord, detail::State<Rng>& state, Struct& out) {
      DVLOG(0) << "begin writing union: " << op::get_class_name_v<Struct>;
      using methods = populator_methods<
          op::get_type_tag<Struct, Ord>,
          op::get_native_type<Struct, Ord>>;

      auto&& got = op::get<Ord>(out);
      using field_ref_type = std::decay_t<decltype(got)>;

      // Popualate optional fields with `optional_field_prob` probability.
      const auto skip = //
          ::apache::thrift::detail::is_optional_field_ref_v<field_ref_type> &&
          (state.size >= state.opts.size_budget ||
           !detail::get_bernoulli(state.rng, state.opts.optional_field_prob));
      if (skip) {
        return;
      }

      DVLOG(3) << "populating member: " << op::get_name_v<Struct, Ord>;

      op::ensure<Ord>(out);
      methods::populate(
          state, detail::deref<field_ref_type>::clear_and_get(got));
      DVLOG(0) << "end writing union";
    }
  };

 public:
  template <typename Rng>
  static void populate(detail::State<Rng>& state, Struct& out) {
    auto recursion_guard =
        detail::RecursionGuard<type::struct_t<Struct>>(state);
    if (!recursion_guard) {
      return;
    }
    op::for_each_ordinal<Struct>(
        [&](auto ord) { member_populator()(ord, state, out); });
  }
};
template <typename Exn>
struct populator_methods<type::exception_t<Exn>, Exn>
    : populator_methods<type::struct_t<Exn>, Exn> {};

// TODO: support field adapters too.
template <typename Adapter, typename InnerTag, typename T>
struct populator_methods<type::adapted<Adapter, InnerTag>, T> {
  using inner_type = std::decay_t<adapt_detail::thrift_t<Adapter, T>>;
  using inner_methods = populator_methods<InnerTag, inner_type>;

  template <typename Rng>
  static void populate(detail::State<Rng>& state, T& out) {
    inner_type tmp;
    inner_methods::populate(state, tmp);
    out = Adapter::fromThrift(std::move(tmp));
  }
};

// non-integral cpp.type is handled transparently by each implementation
template <typename T, typename Tag>
struct populator_methods<
    type::cpp_type<T, Tag>,
    T,
    std::enable_if_t<!type::is_a_v<Tag, type::integral_c>>>
    : populator_methods<Tag, T> {};

/**
 * Entrypoints for using populator
 * Populates Thrift datatype with random data
 *
 * // C++
 * MyStruct a;
 * populator_opts opts;
 *
 * populate(a, opts);
 *
 * @author: Dylan Knutson <dymk@fb.com>
 */

template <typename Type, typename Rng, typename Tag = detail::infer_tag<Type>>
void populate(Type& out, const populator_opts& opts, Rng& rng) {
  detail::State<Rng> state{
      .rng = rng, .opts = opts, .tag_counts = {}, .size = 0};
  return populator_methods<Tag, Type>::populate(state, out);
}

} // namespace apache::thrift::populator
