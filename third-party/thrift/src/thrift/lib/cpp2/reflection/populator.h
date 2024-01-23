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
#include <array>
#include <iostream>
#include <iterator>
#include <random>
#include <type_traits>
#include <vector>

#include <fatal/type/array.h>
#include <fatal/type/conditional.h>
#include <fatal/type/convert.h>
#include <folly/Traits.h>
#include <folly/io/Cursor.h>
#include <thrift/lib/cpp2/Thrift.h>
#include <thrift/lib/cpp2/reflection/reflection.h>

namespace apache {
namespace thrift {
namespace populator {

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
  using int_type = fatal::conditional<
      (sizeof(Int) > 1),
      Int,
      fatal::conditional<
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
using enable_if_smart_pointer =
    typename std::enable_if<is_smart_pointer<T>::value>::type;

template <typename T>
using disable_if_smart_pointer =
    typename std::enable_if<!is_smart_pointer<T>::value>::type;

// helper predicate for determining if a struct's MemberInfo is required
// to be read out of the protocol
struct is_required_field {
  template <typename MemberInfo>
  using apply = std::integral_constant<
      bool,
      MemberInfo::optional::value == optionality::required>;
};

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
  using T = typename std::remove_const<typename PtrType::element_type>::type;
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

} // namespace detail

template <typename TypeClass, typename Type, typename Enable = void>
struct populator_methods;

template <typename Int>
struct populator_methods<type_class::integral, Int> {
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
struct populator_methods<type_class::floating_point, Fp> {
  template <typename Rng>
  static void populate(Rng& rng, const populator_opts&, Fp& out) {
    std::uniform_real_distribution<Fp> gen;
    out = gen(rng);
    DVLOG(4) << "generated real: " << out;
  }
};

template <>
struct populator_methods<type_class::string, std::string> {
  template <typename Rng>
  static void populate(Rng& rng, const populator_opts& opts, std::string& str) {
    using larger_char =
        fatal::conditional<std::numeric_limits<char>::is_signed, int, unsigned>;

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
struct populator_methods<type_class::binary, std::string> {
  template <typename Rng>
  static void populate(Rng& rng, const populator_opts& opts, std::string& bin) {
    const auto length = detail::rand_in_range(rng, opts.bin_len);
    bin = std::string(length, 0);
    auto iter = bin.begin();
    generate_bytes(rng, bin, length, [&](uint8_t c) { *iter++ = c; });
  }
};

template <>
struct populator_methods<type_class::binary, folly::IOBuf> {
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
struct populator_methods<type_class::binary, std::unique_ptr<folly::IOBuf>> {
  template <typename Rng>
  static void populate(
      Rng& rng,
      const populator_opts& opts,
      std::unique_ptr<folly::IOBuf>& bin) {
    bin = std::make_unique<folly::IOBuf>();
    return populator_methods<type_class::binary, folly::IOBuf>::populate(
        rng, opts, *bin);
  }
};

// handle dereferencing smart pointers
template <typename TypeClass, typename PtrType>
struct populator_methods<
    TypeClass,
    PtrType,
    detail::enable_if_smart_pointer<PtrType>> {
  using element_type = typename PtrType::element_type;
  using type_methods = populator_methods<TypeClass, element_type>;

  template <typename Rng>
  static void populate(Rng& rng, const populator_opts& opts, PtrType& out) {
    return type_methods::populate(rng, opts, *out);
  }
};

// Enumerations
template <typename Type>
struct populator_methods<type_class::enumeration, Type> {
  using int_type = typename std::underlying_type<Type>::type;
  using int_methods = populator_methods<type_class::integral, int_type>;

  template <typename Rng>
  static void populate(Rng& rng, const populator_opts& opts, Type& out) {
    int_type tmp;
    int_methods::populate(rng, opts, tmp);
    out = static_cast<Type>(tmp);
  }
};

// Lists
template <typename ElemClass, typename Type>
struct populator_methods<type_class::list<ElemClass>, Type> {
  using elem_type = typename Type::value_type;
  using elem_tclass = ElemClass;
  static_assert(
      !std::is_same<elem_tclass, type_class::unknown>(),
      "Unable to serialize unknown list element");

  using elem_methods = populator_methods<elem_tclass, elem_type>;

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
template <typename ElemClass, typename Type>
struct populator_methods<type_class::set<ElemClass>, Type> {
  // TODO: fair amount of shared code bewteen this and specialization for
  // type_class::list
  using elem_type = typename Type::value_type;
  using elem_tclass = ElemClass;
  static_assert(
      !std::is_same<elem_tclass, type_class::unknown>(),
      "Unable to serialize unknown type");
  using elem_methods = populator_methods<elem_tclass, elem_type>;

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
template <typename KeyClass, typename MappedClass, typename Type>
struct populator_methods<type_class::map<KeyClass, MappedClass>, Type> {
  using key_type = typename Type::key_type;
  using key_tclass = KeyClass;

  using mapped_type = typename Type::mapped_type;
  using mapped_tclass = MappedClass;

  static_assert(
      !std::is_same<key_tclass, type_class::unknown>(),
      "Unable to serialize unknown key type in map");
  static_assert(
      !std::is_same<mapped_tclass, type_class::unknown>(),
      "Unable to serialize unknown mapped type in map");

  using key_methods = populator_methods<key_tclass, key_type>;
  using mapped_methods = populator_methods<mapped_tclass, mapped_type>;

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
struct populator_methods<type_class::variant, Union> {
  using traits = fatal::variant_traits<Union>;

 private:
  struct write_member_by_fid {
    template <typename Fid, std::size_t Index, typename Rng>
    void operator()(
        fatal::indexed<Fid, Index>,
        Rng& rng,
        const populator_opts& opts,
        Union& obj) {
      using descriptor = fatal::get<
          typename traits::descriptors,
          Fid,
          detail::extract_descriptor_fid>;
      using methods = populator_methods<
          typename descriptor::metadata::type_class,
          typename descriptor::type>;

      assert(Fid::value == descriptor::metadata::id::value);

      DVLOG(3) << "writing union field "
               << fatal::z_data<typename descriptor::metadata::name>()
               << ", fid: " << descriptor::metadata::id::value;

      typename descriptor::type tmp;
      typename descriptor::setter setter;

      methods::populate(rng, opts, tmp);
      setter(obj, std::move(tmp));
    }
  };

 public:
  template <typename Rng>
  static void populate(Rng& rng, const populator_opts& opts, Union& out) {
    DVLOG(3) << "begin writing union: "
             << fatal::z_data<typename traits::name>()
             << ", type: " << folly::to_underlying(out.getType());

    // array of all possible FIDs of this union
    using fids_seq = fatal::sort<fatal::as_sequence<
        fatal::transform<
            typename traits::descriptors,
            detail::extract_descriptor_fid>,
        fatal::sequence,
        field_id_t>>;

    // std::array of field_id_t
    const auto range = populator_opts::range<std::size_t>(
        0, fatal::size<fids_seq>::value - !fatal::empty<fids_seq>::value);
    const auto selected = detail::rand_in_range(rng, range);

    fatal::sorted_search<fids_seq>(
        fatal::as_array<fids_seq>::data[selected],
        write_member_by_fid(),
        rng,
        opts,
        out);
    DVLOG(3) << "end writing union";
  }
};

// specialization for structs
template <typename Struct>
struct populator_methods<type_class::structure, Struct> {
 private:
  using traits = apache::thrift::reflect_struct<Struct>;

  using all_fields =
      fatal::partition<typename traits::members, detail::is_required_field>;
  using required_fields = fatal::first<all_fields>;
  using optional_fields = fatal::second<all_fields>;

  using isset_array = std::array<bool, fatal::size<required_fields>::value>;

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
    template <typename Member, std::size_t Index, typename Rng>
    void operator()(
        fatal::indexed<Member, Index>,
        Rng& rng,
        const populator_opts& opts,
        Struct& out) {
      using methods =
          populator_methods<typename Member::type_class, typename Member::type>;

      auto&& got = typename Member::field_ref_getter{}(out);
      using member_type = folly::remove_cvref_t<decltype(*got)>;

      // Popualate optional fields with `optional_field_prob` probability.
      const auto skip = //
          Member::optional::value == optionality::optional &&
          !detail::get_bernoulli(rng, opts.optional_field_prob);
      if (skip) {
        return;
      }

      DVLOG(3) << "populating member: "
               << fatal::z_data<typename Member::name>();

      Member::mark_set(out, true);
      ensure_cpp_ref(got);
      field_populator<Member, member_type, methods>::populate(rng, opts, *got);
    }

   private:
    template <class T>
    static void ensure_cpp_ref(T&&) {}

    template <class T>
    static void ensure_cpp_ref(std::unique_ptr<T>& v) {
      v = std::make_unique<T>();
    }

    template <class T>
    static void ensure_cpp_ref(std::shared_ptr<T>& v) {
      v = std::make_shared<T>();
    }
  };

 public:
  template <typename Rng>
  static void populate(Rng& rng, const populator_opts& opts, Struct& out) {
    fatal::foreach<typename traits::members>(
        member_populator(), rng, opts, out);
  }
};

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
  using TypeClass = type_class_of_thrift_class_t<Type>;
  return populator_methods<TypeClass, Type>::populate(rng, opts, out);
}

} // namespace populator
} // namespace thrift
} // namespace apache
