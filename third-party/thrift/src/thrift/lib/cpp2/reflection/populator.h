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
  template <typename Int = std::size_t>
  struct range {
    Int min;
    Int max;
    range(Int min, Int max) : min(min), max(max) { assert(min <= max); }
  };

  range<> list_len = range<>(0, 0xFF);
  range<> set_len = range<>(0, 0xFF);
  range<> map_len = range<>(0, 0xFF);
  range<> bin_len = range<>(0, 0xFF);
  range<> str_len = range<>(0, 0xFF);
  // Probability to use for populating optional fields.
  float optional_field_prob = 0.0;
};

namespace detail {
// generate a value with a Bernoulli distribution:
// p is the probability of true, (1-p) for false
template <typename Rng>
bool get_bernoulli(Rng& rng, float p) {
  std::bernoulli_distribution gen(p);
  return gen(rng);
}

// generate a value of type Int within [range.min, range.max]
template <typename Int, typename Rng>
Int rand_in_range(Rng& rng, const populator_opts::range<Int>& range) {
  // uniform_int_distribution undefined for char,
  // use the next larger type if it's small
  using int_type = std::conditional_t<
      (sizeof(Int) > 1),
      Int,
      std::conditional_t<
          std::numeric_limits<Int>::is_signed,
          signed short,
          unsigned short>>;

  std::uniform_int_distribution<int_type> gen(range.min, range.max);
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

// General case: methods on deref are no-op, returning their input
template <typename T>
struct deref<T, disable_if_smart_pointer<T>> {
  static T& clear_and_get(T& in) { return in; }
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
    Int out = detail::rand_in_range(
        rng, populator_opts::range<Int>(limits::min(), limits::max()));
    DVLOG(4) << "generated int: " << out;
    return out;
  }

  // Special overload to work with lists of booleans.
  template <typename Rng>
  static void populate(
      Rng& rng, const populator_opts&, std::vector<bool>::reference out) {
    out = next_value(rng);
  }

  template <typename Rng>
  static void populate(Rng& rng, const populator_opts&, Int& out) {
    out = next_value(rng);
  }
};

template <typename Fp>
struct populator_methods<
    detail::infer_tag<Fp>,
    Fp,
    std::enable_if_t<
        type::is_a_v<detail::infer_tag<Fp>, type::floating_point_c>>> {
  template <typename Rng>
  static void populate(Rng& rng, const populator_opts&, Fp& out) {
    std::uniform_real_distribution<Fp> gen;
    out = gen(rng);
    DVLOG(4) << "generated real: " << out;
  }
};

template <>
struct populator_methods<type::string_t, std::string> {
  template <typename Rng>
  static void populate(Rng& rng, const populator_opts& opts, std::string& str) {
    using larger_char =
        std::conditional_t<std::numeric_limits<char>::is_signed, int, unsigned>;

    // all printable chars (see `man ascii`)
    std::uniform_int_distribution<larger_char> char_gen(0x20, 0x7E);

    const std::size_t length = detail::rand_in_range(rng, opts.str_len);

    str = std::string(length, 0);
    std::generate_n(str.begin(), length, [&]() {
      return static_cast<char>(char_gen(rng));
    });

    DVLOG(4) << "generated string of len" << length;
  }
};

template <>
struct populator_methods<
    type::cpp_type<folly::fbstring, type::string_t>,
    folly::fbstring> {
  template <typename Rng>
  static void populate(
      Rng& rng, const populator_opts& opts, folly::fbstring& bin) {
    std::string t;
    populator_methods<type::string_t, std::string>::populate(rng, opts, t);
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
  static void populate(Rng& rng, const populator_opts& opts, std::string& bin) {
    const auto length = detail::rand_in_range(rng, opts.bin_len);
    bin = std::string(length, 0);
    auto iter = bin.begin();
    generate_bytes(rng, bin, length, [&](uint8_t c) { *iter++ = c; });
  }
};

template <>
struct populator_methods<
    type::cpp_type<folly::fbstring, type::binary_t>,
    folly::fbstring> {
  template <typename Rng>
  static void populate(
      Rng& rng, const populator_opts& opts, folly::fbstring& bin) {
    std::string t;
    populator_methods<type::binary_t, std::string>::populate(rng, opts, t);
    bin = folly::fbstring(std::move(t));
  }
};

template <>
struct populator_methods<
    type::cpp_type<folly::IOBuf, type::binary_t>,
    folly::IOBuf> {
  template <typename Rng>
  static void populate(
      Rng& rng, const populator_opts& opts, folly::IOBuf& bin) {
    const auto length = detail::rand_in_range(rng, opts.bin_len);
    bin = folly::IOBuf(folly::IOBuf::CREATE, length);
    bin.append(length);
    folly::io::RWUnshareCursor range(&bin);
    generate_bytes(
        rng, range, length, [&](uint8_t c) { range.write<uint8_t>(c); });
  }
};

template <>
struct populator_methods<
    type::cpp_type<std::unique_ptr<folly::IOBuf>, type::binary_t>,
    std::unique_ptr<folly::IOBuf>> {
  template <typename Rng>
  static void populate(
      Rng& rng,
      const populator_opts& opts,
      std::unique_ptr<folly::IOBuf>& bin) {
    bin = std::make_unique<folly::IOBuf>();
    return populator_methods<
        type::cpp_type<folly::IOBuf, type::binary_t>,
        folly::IOBuf>::populate(rng, opts, *bin);
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
  static void populate(Rng& rng, const populator_opts& opts, PtrType& out) {
    return type_methods::populate(rng, opts, *out);
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
  static void populate(Rng& rng, const populator_opts& opts, Type& out) {
    int_type tmp;
    int_methods::populate(rng, opts, tmp);
    out = static_cast<Type>(tmp);
  }
};

// Lists
template <typename ElemTag, typename Type>
struct populator_methods<type::list<ElemTag>, Type> {
  using elem_type = typename Type::value_type;
  using elem_methods = populator_methods<ElemTag, elem_type>;

  template <typename Rng>
  static void populate(Rng& rng, const populator_opts& opts, Type& out) {
    std::uint32_t list_size = detail::rand_in_range(rng, opts.list_len);
    out = Type();

    DVLOG(3) << "populating list size " << list_size;

    out.resize(list_size);
    for (decltype(list_size) i = 0; i < list_size; i++) {
      elem_methods::populate(rng, opts, out[i]);
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
  static void populate(Rng& rng, const populator_opts& opts, Type& out) {
    std::uint32_t set_size = detail::rand_in_range(rng, opts.set_len);

    DVLOG(3) << "populating set size " << set_size;
    out = Type();

    for (decltype(set_size) i = 0; i < set_size; i++) {
      elem_type tmp;
      elem_methods::populate(rng, opts, tmp);
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
  static void populate(Rng& rng, const populator_opts& opts, Type& out) {
    std::uint32_t map_size = detail::rand_in_range(rng, opts.map_len);

    DVLOG(3) << "populating map size " << map_size;
    out = Type();

    for (decltype(map_size) i = 0; i < map_size; i++) {
      key_type key_tmp;
      key_methods::populate(rng, opts, key_tmp);
      mapped_methods::populate(rng, opts, out[std::move(key_tmp)]);
    }
  }
};

// specialization for variants (Thrift unions)
template <typename Union>
struct populator_methods<type::union_t<Union>, Union> {
  template <typename Rng>
  static void populate(Rng& rng, const populator_opts& opts, Union& out) {
    DVLOG(3) << "begin writing union: "
             << op::get_class_name_v<Union> << ", type: "
             << folly::to_underlying(out.getType());

    const auto selected = static_cast<type::Ordinal>(detail::rand_in_range(
        rng, populator_opts::range<size_t>{0, op::size_v<Union> - 1}));

    op::for_each_ordinal<Union>([&](auto ord) {
      using Ord = decltype(ord);
      if (ord != selected) {
        return;
      }
      using methods = populator_methods<
          op::get_type_tag<Union, Ord>,
          op::get_native_type<Union, Ord>>;

      DVLOG(3) << "writing union field "
               << op::get_name_v<Union, Ord> << ", fid: "
               << folly::to_underlying(op::get_field_id_v<Union, Ord>);

      methods::populate(rng, opts, op::get<Ord>(out).ensure());
    });

    DVLOG(3) << "end writing union";
  }
};

// specialization for structs
template <typename Struct>
struct populator_methods<type::struct_t<Struct>, Struct> {
 private:
  template <
      typename Member,
      typename MemberType,
      typename Methods,
      typename Enable = void>
  struct field_populator;

  // generic field writer
  template <typename Member, typename MemberType, typename Methods>
  struct field_populator<
      Member,
      MemberType,
      Methods,
      detail::disable_if_smart_pointer<MemberType>> {
    template <typename Rng>
    static void populate(
        Rng& rng, const populator_opts& opts, MemberType& out) {
      Methods::populate(rng, opts, out);
    }
  };

  // writer for default/required ref structs
  template <typename Member, typename PtrType, typename Methods>
  struct field_populator<
      Member,
      PtrType,
      Methods,
      detail::enable_if_smart_pointer<PtrType>> {
    using element_type = typename PtrType::element_type;

    template <typename Rng>
    static void populate(Rng& rng, const populator_opts& opts, PtrType& out) {
      field_populator<Member, element_type, Methods>::populate(
          rng, opts, detail::deref<PtrType>::clear_and_get(out));
    }
  };

  class member_populator {
   public:
    template <typename Ord, typename Rng>
    void operator()(Ord, Rng& rng, const populator_opts& opts, Struct& out) {
      using methods = populator_methods<
          op::get_type_tag<Struct, Ord>,
          op::get_native_type<Struct, Ord>>;

      auto&& got = op::get<Ord>(out);

      // Popualate optional fields with `optional_field_prob` probability.
      const auto skip = //
          ::apache::thrift::detail::is_optional_field_ref_v<
              std::decay_t<decltype(got)>> &&
          !detail::get_bernoulli(rng, opts.optional_field_prob);
      if (skip) {
        return;
      }

      DVLOG(3) << "populating member: " << op::get_name_v<Struct, Ord>;

      op::ensure<Ord>(out);
      field_populator<
          op::get_type_tag<Struct, Ord>,
          op::get_native_type<Struct, Ord>,
          methods>::populate(rng, opts, *got);
    }
  };

 public:
  template <typename Rng>
  static void populate(Rng& rng, const populator_opts& opts, Struct& out) {
    op::for_each_ordinal<Struct>(
        [&](auto ord) { member_populator()(ord, rng, opts, out); });
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
  static void populate(Rng& rng, const populator_opts& opts, T& out) {
    inner_type tmp;
    inner_methods::populate(rng, opts, tmp);
    out = Adapter::fromThrift(std::move(tmp));
  }
};

// cpp.type is handled transparently by each implementation
template <typename T, typename Tag>
struct populator_methods<type::cpp_type<T, Tag>, T>
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

template <typename Type, typename Rng>
void populate(Type& out, const populator_opts& opts, Rng& rng) {
  return populator_methods<detail::infer_tag<Type>, Type>::populate(
      rng, opts, out);
}

} // namespace apache::thrift::populator
