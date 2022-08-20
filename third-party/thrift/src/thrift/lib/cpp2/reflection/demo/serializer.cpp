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

#include <algorithm>
#include <cassert>
#include <cstring>
#include <iostream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include <fatal/type/search.h>
#include <thrift/lib/cpp2/reflection/demo/gen-cpp2/data_constants.h>
#include <thrift/lib/cpp2/reflection/demo/gen-cpp2/data_fatal_types.h>
#include <thrift/lib/cpp2/reflection/reflection.h>

struct memory_buffer {
  void write(const char* begin, const char* end) {
    buffer_.insert(buffer_.end(), begin, end);
  }

  const char* read(std::size_t size) {
    assert(index_ <= buffer_.size());

    if (buffer_.size() - index_ < size) {
      throw std::underflow_error("not enough data");
    }

    const auto data = std::next(buffer_.data(), index_);
    index_ += size;

    return data;
  }

  bool empty() const { return index_ == buffer_.size(); }

 private:
  using buffer_type = std::vector<char>;

  buffer_type buffer_;
  buffer_type::size_type index_ = 0;
};

struct data_writer {
  explicit data_writer(memory_buffer& buffer) : buffer_(buffer) {}

  template <typename T>
  void write_opaque(T const* data, std::size_t size) {
    buffer_.write(
        reinterpret_cast<const char*>(data),
        reinterpret_cast<const char*>(std::next(data, size)));
  }

  template <typename T>
  void write_raw(T const& value) {
    write_opaque(std::addressof(value), 1);
  }

  template <typename T>
  void write_string(T const* data, std::size_t size) {
    write_raw(size);
    write_opaque(data, size);
  }

 private:
  memory_buffer& buffer_;
};

struct data_reader {
  explicit data_reader(memory_buffer& buffer) : buffer_(buffer) {}

  template <typename T = char>
  std::pair<const T*, T const*> read_opaque(std::size_t size) {
    const auto data = buffer_.read(size * sizeof(T));

    return std::make_pair(
        reinterpret_cast<const T*>(data),
        reinterpret_cast<const T*>(std::next(data, size)));
  }

  template <typename T>
  T read_raw() {
    const auto data = read_opaque(sizeof(T));
    T out;
    std::copy(
        data.first, data.second, reinterpret_cast<char*>(std::addressof(out)));
    return out;
  }

  template <typename T>
  void read_string(std::basic_string<T>& out) {
    const auto data = read_opaque<T>(read_raw<std::size_t>());
    out.append(data.first, data.second);
  }

 private:
  memory_buffer& buffer_;
};

using namespace apache::thrift;

template <typename TypeClass>
struct serializer {
  static_assert(
      !std::is_same<type_class::unknown, TypeClass>::value,
      "no static reflection support for the given type"
      " - did you forget to include the reflection metadata?"
      " see thrift/lib/cpp2/reflection/reflection.h");

  template <typename T>
  static void serialize(T const& what, data_writer& writer) {
    writer.write_raw(what);
  }

  template <typename T>
  static void deserialize(T& out, data_reader& reader) {
    out = reader.read_raw<T>();
  }
};

template <>
struct serializer<type_class::string> {
  template <typename T>
  static void serialize(T const& what, data_writer& writer) {
    writer.write_string(what.data(), what.size());
  }

  template <typename T>
  static void deserialize(T& out, data_reader& reader) {
    reader.read_string(out);
  }
};

template <>
struct serializer<type_class::enumeration> {
  template <typename T>
  static void serialize(T const& what, data_writer& writer) {
    const auto name = fatal::enum_to_string(what, nullptr);
    writer.write_string(name, std::strlen(name));
  }

  template <typename T>
  static void deserialize(T& out, data_reader& reader) {
    std::string name;
    reader.read_string(name);

    out = fatal::enum_traits<T>::parse(name);
  }
};

template <typename ValueTypeClass>
struct serializer<type_class::list<ValueTypeClass>> {
  template <typename T>
  static void serialize(T const& what, data_writer& writer) {
    writer.write_raw(what.size());

    for (const auto& i : what) {
      serializer<ValueTypeClass>::serialize(i, writer);
    }
  }

  template <typename T>
  static void deserialize(T& out, data_reader& reader) {
    auto count = reader.read_raw<typename T::size_type>();

    while (count--) {
      out.emplace_back();
      serializer<ValueTypeClass>::deserialize(out.back(), reader);
    }
  }
};

template <typename ValueTypeClass>
struct serializer<type_class::set<ValueTypeClass>> {
  template <typename T>
  static void serialize(T const& what, data_writer& writer) {
    writer.write_raw(what.size());

    for (const auto& i : what) {
      serializer<ValueTypeClass>::serialize(i, writer);
    }
  }

  template <typename T>
  static void deserialize(T& out, data_reader& reader) {
    auto count = reader.read_raw<typename T::size_type>();

    while (count--) {
      typename T::value_type value;
      serializer<ValueTypeClass>::deserialize(value, reader);
      out.emplace(std::move(value));
    }
  }
};

template <typename KeyTypeClass, typename MappedTypeClass>
struct serializer<type_class::map<KeyTypeClass, MappedTypeClass>> {
  template <typename T>
  static void serialize(T const& what, data_writer& writer) {
    writer.write_raw(what.size());

    for (const auto& i : what) {
      serializer<KeyTypeClass>::serialize(i.first, writer);
      serializer<MappedTypeClass>::serialize(i.second, writer);
    }
  }

  template <typename T>
  static void deserialize(T& out, data_reader& reader) {
    auto count = reader.read_raw<typename T::size_type>();

    while (count--) {
      typename T::key_type key;
      serializer<KeyTypeClass>::deserialize(key, reader);

      auto& value = out[std::move(key)];
      serializer<MappedTypeClass>::deserialize(value, reader);
    }
  }
};

struct struct_member_serializer {
  template <typename Member, std::size_t Index, typename T>
  void operator()(
      fatal::indexed<Member, Index>, T const& what, data_writer& writer) const {
    const auto& value = typename Member::getter{}(what);
    serializer<typename Member::type_class>::serialize(value, writer);
  }
};

struct struct_member_deserializer {
  template <typename Member, std::size_t Index, typename T>
  void operator()(
      fatal::indexed<Member, Index>, T& out, data_reader& reader) const {
    auto& member_ref = typename Member::getter{}(out);
    serializer<typename Member::type_class>::deserialize(member_ref, reader);
  }
};

template <>
struct serializer<type_class::structure> {
  template <typename T>
  static void serialize(T const& what, data_writer& writer) {
    fatal::foreach<typename reflect_struct<T>::members>(
        struct_member_serializer(), what, writer);
  }

  template <typename T>
  static void deserialize(T& out, data_reader& reader) {
    fatal::foreach<typename reflect_struct<T>::members>(
        struct_member_deserializer(), out, reader);
  }
};

struct variant_member_serializer {
  template <typename Member, std::size_t Index, typename T>
  void operator()(
      fatal::indexed<Member, Index>,
      T const& variant,
      data_writer& writer) const {
    using name = typename Member::metadata::name;
    writer.write_string(fatal::z_data<name>(), fatal::size<name>::value);
    const auto& value = Member::get(variant);
    using type_class = typename Member::metadata::type_class;
    serializer<type_class>::serialize(value, writer);
  }
};

struct variant_member_deserializer {
  template <typename Member, typename T>
  void operator()(fatal::tag<Member>, T& out, data_reader& reader) const {
    Member::set(out);
    auto& value = Member::get(out);
    using type_class = typename Member::metadata::type_class;
    serializer<type_class>::deserialize(value, reader);
  }
};

struct get_variant_member_name {
  template <typename Member>
  using apply = typename Member::metadata::name;
};

template <>
struct serializer<type_class::variant> {
  template <typename T>
  static void serialize(T const& what, data_writer& writer) {
    bool found = fatal::scalar_search<
        typename fatal::variant_traits<T>::descriptors,
        fatal::get_type::id>(
        what.getType(), variant_member_serializer(), what, writer);

    if (!found) {
      writer.write_string("", 0);
    }
  }

  template <typename T>
  static void deserialize(T& out, data_reader& reader) {
    std::string which;
    reader.read_string(which);

    bool found = fatal::trie_find<
        typename fatal::variant_traits<T>::descriptors,
        get_variant_member_name>(
        which.begin(), which.end(), variant_member_deserializer(), out, reader);

    if (!found) {
      fatal::variant_traits<T>::clear(out);
    }
  }
};

template <typename T>
void serialize(T const& what, data_writer& writer) {
  serializer<reflect_type_class_of_thrift_class<T>>::serialize(what, writer);
}

template <typename T>
void deserialize(T& out, data_reader& reader) {
  serializer<reflect_type_class_of_thrift_class<T>>::deserialize(out, reader);
}

template <typename T>
void test(T const& what) {
  try {
    memory_buffer buffer;

    data_writer writer(buffer);
    serialize(what, writer);

    data_reader reader(buffer);
    T deserialized;
    deserialize(deserialized, reader);

    if (!buffer.empty()) {
      throw std::runtime_error("found unused bytes after deserializing");
    }

    if (what != deserialized) {
      throw std::runtime_error("mismatch between input and deserialized data");
    }

    std::cout << "success\n";
  } catch (const std::exception& e) {
    std::cout << "FAILURE: " << e.what() << '\n';
  }
}

int main() {
  std::cerr << "example 1: ";
  test(static_reflection::demo::data_constants::example_1());

  std::cerr << "example 2: ";
  test(static_reflection::demo::data_constants::example_2());

  std::cerr << "example 3: ";
  test(static_reflection::demo::data_constants::example_3());

  return 0;
}
