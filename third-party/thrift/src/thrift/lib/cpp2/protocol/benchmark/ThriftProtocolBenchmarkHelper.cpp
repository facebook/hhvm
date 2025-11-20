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

#include "ThriftProtocolBenchmarkHelper.h"

#include <google/protobuf/io/zero_copy_stream.h>
#include <folly/io/Cursor.h>
#include <folly/io/IOBufQueue.h>

static const std::string kShortString(1 << 3, 'x'); // 8 bytes
static const std::vector<int> kIntList = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
static const std::map<std::string, int> kStringMap = {
    {"one", 1}, {"two", 2}, {"three", 3}, {"four", 4}, {"five", 5}};

// Trait to map TestData types to their field group counts
template <typename TestData>
struct FieldGroupCount;

template <>
struct FieldGroupCount<benchmark::ThriftTestDataSmall> {
  static constexpr size_t value = 1;
};

template <>
struct FieldGroupCount<benchmark::ThriftTestDataMedium> {
  static constexpr size_t value = 3;
};

template <>
struct FieldGroupCount<benchmark::ThriftTestDataLarge> {
  static constexpr size_t value = 9;
};

template <>
struct FieldGroupCount<benchmark::ProtobufTestDataSmall> {
  static constexpr size_t value = 1;
};

template <>
struct FieldGroupCount<benchmark::ProtobufTestDataMedium> {
  static constexpr size_t value = 3;
};

template <>
struct FieldGroupCount<benchmark::ProtobufTestDataLarge> {
  static constexpr size_t value = 9;
};

template <>
struct FieldGroupCount<benchmark::CarbonTestDataSmall> {
  static constexpr size_t value = 1;
};

template <>
struct FieldGroupCount<benchmark::CarbonTestDataMedium> {
  static constexpr size_t value = 3;
};

template <>
struct FieldGroupCount<benchmark::CarbonTestDataLarge> {
  static constexpr size_t value = 9;
};

namespace {
// Thrift helpers
template <typename TestData, size_t N>
void populateThriftFieldGroup(TestData& testData) {
#define POPULATE_FIELDS(index)                    \
  if constexpr (N == index) {                     \
    testData.intField##index() = 42;              \
    testData.longField##index() = 123456789L;     \
    testData.shortString##index() = kShortString; \
    testData.intList##index() = kIntList;         \
    testData.stringMap##index() = kStringMap;     \
    testData.boolField##index() = true;           \
    testData.doubleField##index() = 3.14159;      \
  }

  POPULATE_FIELDS(1)
  POPULATE_FIELDS(2)
  POPULATE_FIELDS(3)
  POPULATE_FIELDS(4)
  POPULATE_FIELDS(5)
  POPULATE_FIELDS(6)
  POPULATE_FIELDS(7)
  POPULATE_FIELDS(8)
  POPULATE_FIELDS(9)
#undef POPULATE_FIELDS
}

template <typename TestData, size_t... Is>
void populateThriftFieldGroupsImpl(
    TestData& testData, std::index_sequence<Is...>) {
  (populateThriftFieldGroup<TestData, Is + 1>(testData), ...);
}

template <typename TestData, size_t N>
void populateThriftFieldGroups(TestData& testData) {
  populateThriftFieldGroupsImpl(testData, std::make_index_sequence<N>{});
}

// Protobuf helpers
template <typename TestData, size_t N>
void populateProtobufFieldGroup(TestData& testData) {
#define POPULATE_PROTOBUF_FIELDS(index)                    \
  if constexpr (N == index) {                              \
    testData.set_int_field_##index(42);                    \
    testData.set_long_field_##index(123456789L);           \
    testData.set_short_string_##index(kShortString);       \
    for (auto val : kIntList) {                            \
      testData.add_int_list_##index(val);                  \
    }                                                      \
    for (const auto& [key, val] : kStringMap) {            \
      (*testData.mutable_string_map_##index())[key] = val; \
    }                                                      \
    testData.set_bool_field_##index(true);                 \
    testData.set_double_field_##index(3.14159);            \
  }

  POPULATE_PROTOBUF_FIELDS(1)
  POPULATE_PROTOBUF_FIELDS(2)
  POPULATE_PROTOBUF_FIELDS(3)
  POPULATE_PROTOBUF_FIELDS(4)
  POPULATE_PROTOBUF_FIELDS(5)
  POPULATE_PROTOBUF_FIELDS(6)
  POPULATE_PROTOBUF_FIELDS(7)
  POPULATE_PROTOBUF_FIELDS(8)
  POPULATE_PROTOBUF_FIELDS(9)
#undef POPULATE_PROTOBUF_FIELDS
}

template <typename TestData, size_t... Is>
void populateProtobufFieldGroupsImpl(
    TestData& testData, std::index_sequence<Is...>) {
  (populateProtobufFieldGroup<TestData, Is + 1>(testData), ...);
}

template <typename TestData, size_t N>
void populateProtobufFieldGroups(TestData& testData) {
  populateProtobufFieldGroupsImpl(testData, std::make_index_sequence<N>{});
}

// Carbon helpers
template <typename TestData, size_t N>
void populateCarbonFieldGroup(TestData& testData) {
#define POPULATE_CARBON_FIELDS(index)                   \
  if constexpr (N == index) {                           \
    testData.intField##index##_ref() = 42;              \
    testData.longField##index##_ref() = 123456789L;     \
    testData.shortString##index##_ref() = kShortString; \
    testData.intList##index##_ref() = kIntList;         \
    testData.stringMap##index##_ref() = kStringMap;     \
    testData.boolField##index##_ref() = true;           \
    testData.doubleField##index##_ref() = 3.14159;      \
  }

  POPULATE_CARBON_FIELDS(1)
  POPULATE_CARBON_FIELDS(2)
  POPULATE_CARBON_FIELDS(3)
  POPULATE_CARBON_FIELDS(4)
  POPULATE_CARBON_FIELDS(5)
  POPULATE_CARBON_FIELDS(6)
  POPULATE_CARBON_FIELDS(7)
  POPULATE_CARBON_FIELDS(8)
  POPULATE_CARBON_FIELDS(9)
#undef POPULATE_CARBON_FIELDS
}

template <typename TestData, size_t... Is>
void populateCarbonFieldGroupsImpl(
    TestData& testData, std::index_sequence<Is...>) {
  (populateCarbonFieldGroup<TestData, Is + 1>(testData), ...);
}

template <typename TestData, size_t N>
void populateCarbonFieldGroups(TestData& testData) {
  populateCarbonFieldGroupsImpl(testData, std::make_index_sequence<N>{});
}
} // namespace

// Thrift test data generation
template <typename TestData>
TestData createThriftTestData() {
  constexpr size_t N = FieldGroupCount<TestData>::value;
  TestData testData;
  populateThriftFieldGroups<TestData, N>(testData);
  return testData;
}

// Explicit template instantiations for Thrift test data
template benchmark::ThriftTestDataSmall
createThriftTestData<benchmark::ThriftTestDataSmall>();
template benchmark::ThriftTestDataMedium
createThriftTestData<benchmark::ThriftTestDataMedium>();
template benchmark::ThriftTestDataLarge
createThriftTestData<benchmark::ThriftTestDataLarge>();

// Protobuf test data generation
template <typename TestData>
TestData createProtobufTestData() {
  constexpr size_t N = FieldGroupCount<TestData>::value;
  TestData testData;
  populateProtobufFieldGroups<TestData, N>(testData);
  return testData;
}

// Explicit template instantiations for Protobuf test data
template benchmark::ProtobufTestDataSmall
createProtobufTestData<benchmark::ProtobufTestDataSmall>();
template benchmark::ProtobufTestDataMedium
createProtobufTestData<benchmark::ProtobufTestDataMedium>();
template benchmark::ProtobufTestDataLarge
createProtobufTestData<benchmark::ProtobufTestDataLarge>();

// Carbon test data generation
template <typename TestData>
TestData createCarbonTestData() {
  constexpr size_t N = FieldGroupCount<TestData>::value;
  TestData testData;
  populateCarbonFieldGroups<TestData, N>(testData);
  return testData;
}

// Explicit template instantiations for Carbon test data
template benchmark::CarbonTestDataSmall
createCarbonTestData<benchmark::CarbonTestDataSmall>();
template benchmark::CarbonTestDataMedium
createCarbonTestData<benchmark::CarbonTestDataMedium>();
template benchmark::CarbonTestDataLarge
createCarbonTestData<benchmark::CarbonTestDataLarge>();

// Cursor-based serialization helpers
template <typename TestData, size_t N, typename Writer>
void writeCurSeFieldGroup(Writer& writer) {
#define WRITE_CURSE_FIELDS(index)                                              \
  if constexpr (N == index) {                                                  \
    writer.template write<apache::thrift::ident::intField##index>(42);         \
    writer.template write<apache::thrift::ident::longField##index>(            \
        123456789L);                                                           \
    writer.template write<apache::thrift::ident::shortString##index>(          \
        kShortString);                                                         \
    writer.template write<apache::thrift::ident::intList##index>(kIntList);    \
    writer.template write<apache::thrift::ident::stringMap##index>(            \
        kStringMap);                                                           \
    writer.template write<apache::thrift::ident::boolField##index>(true);      \
    writer.template write<apache::thrift::ident::doubleField##index>(3.14159); \
  }

  WRITE_CURSE_FIELDS(1)
  WRITE_CURSE_FIELDS(2)
  WRITE_CURSE_FIELDS(3)
  WRITE_CURSE_FIELDS(4)
  WRITE_CURSE_FIELDS(5)
  WRITE_CURSE_FIELDS(6)
  WRITE_CURSE_FIELDS(7)
  WRITE_CURSE_FIELDS(8)
  WRITE_CURSE_FIELDS(9)
#undef WRITE_CURSE_FIELDS
}

template <typename TestData, typename Writer, size_t... Is>
void writeCurSeFieldGroupsImpl(Writer& writer, std::index_sequence<Is...>) {
  (writeCurSeFieldGroup<TestData, Is + 1>(writer), ...);
}

template <typename TestData>
std::unique_ptr<folly::IOBuf> serializeThriftCurSe() {
  constexpr size_t N = FieldGroupCount<TestData>::value;
  apache::thrift::CursorSerializationWrapper<TestData> wrapper;
  auto writer = wrapper.beginWrite();

  writeCurSeFieldGroupsImpl<TestData>(writer, std::make_index_sequence<N>{});

  wrapper.endWrite(std::move(writer));
  return std::move(wrapper).serializedData();
}

template std::unique_ptr<folly::IOBuf>
serializeThriftCurSe<benchmark::ThriftTestDataSmall>();
template std::unique_ptr<folly::IOBuf>
serializeThriftCurSe<benchmark::ThriftTestDataMedium>();
template std::unique_ptr<folly::IOBuf>
serializeThriftCurSe<benchmark::ThriftTestDataLarge>();

// Cursor-based deserialization helpers
template <typename TestData, size_t N, typename Reader>
void readCurSeFieldGroup(Reader& reader) {
#define READ_CURSE_FIELDS(index)                                       \
  if constexpr (N == index) {                                          \
    reader.template read<apache::thrift::ident::intField##index>();    \
    reader.template read<apache::thrift::ident::longField##index>();   \
    reader.template read<apache::thrift::ident::shortString##index>(); \
    reader.template read<apache::thrift::ident::intList##index>();     \
    reader.template read<apache::thrift::ident::stringMap##index>();   \
    reader.template read<apache::thrift::ident::boolField##index>();   \
    reader.template read<apache::thrift::ident::doubleField##index>(); \
  }

  READ_CURSE_FIELDS(1)
  READ_CURSE_FIELDS(2)
  READ_CURSE_FIELDS(3)
  READ_CURSE_FIELDS(4)
  READ_CURSE_FIELDS(5)
  READ_CURSE_FIELDS(6)
  READ_CURSE_FIELDS(7)
  READ_CURSE_FIELDS(8)
  READ_CURSE_FIELDS(9)
#undef READ_CURSE_FIELDS
}

template <typename TestData, typename Reader, size_t... Is>
void readCurSeFieldGroupsImpl(Reader& reader, std::index_sequence<Is...>) {
  (readCurSeFieldGroup<TestData, Is + 1>(reader), ...);
}

// Cursor-based deserialization
template <typename TestData>
void deserializeThriftCurSe(std::unique_ptr<folly::IOBuf> serialized) {
  apache::thrift::CursorSerializationWrapper<TestData> wrapper(
      std::move(serialized));
  auto reader = wrapper.beginRead();

  constexpr size_t N = FieldGroupCount<TestData>::value;
  readCurSeFieldGroupsImpl<TestData>(reader, std::make_index_sequence<N>{});

  wrapper.endRead(std::move(reader));
}

// Explicit template instantiations for deserialization
template void deserializeThriftCurSe<benchmark::ThriftTestDataSmall>(
    std::unique_ptr<folly::IOBuf>);
template void deserializeThriftCurSe<benchmark::ThriftTestDataMedium>(
    std::unique_ptr<folly::IOBuf>);
template void deserializeThriftCurSe<benchmark::ThriftTestDataLarge>(
    std::unique_ptr<folly::IOBuf>);

// IOBuf ZeroCopyOutputStream adaptor for protobuf
namespace {
class IOBufZeroCopyOutputStream
    : public google::protobuf::io::ZeroCopyOutputStream {
 public:
  explicit IOBufZeroCopyOutputStream(folly::IOBuf* buf, size_t size)
      : buf_(buf), size_(size), position_(0) {}

  bool Next(void** data, int* size) override {
    if (position_ >= size_) {
      return false;
    }

    *data = buf_->writableData() + position_;
    *size = size_ - position_;
    position_ = size_;
    return true;
  }

  void BackUp(int count) override { position_ -= count; }

  int64_t ByteCount() const override { return position_; }

 private:
  folly::IOBuf* buf_;
  size_t size_;
  size_t position_;
};

class IOBufZeroCopyInputStream
    : public google::protobuf::io::ZeroCopyInputStream {
 public:
  explicit IOBufZeroCopyInputStream(const folly::IOBuf* buf)
      : buf_(buf), position_(0) {}

  bool Next(const void** data, int* size) override {
    if (position_ >= buf_->length()) {
      return false;
    }

    *data = buf_->data() + position_;
    *size = buf_->length() - position_;
    position_ = buf_->length();
    return true;
  }

  void BackUp(int count) override { position_ -= count; }

  bool Skip(int count) override {
    if (position_ + count > buf_->length()) {
      return false;
    }
    position_ += count;
    return true;
  }

  int64_t ByteCount() const override { return position_; }

 private:
  const folly::IOBuf* buf_;
  size_t position_;
};
} // namespace

// Protobuf serialization using SerializeToArray with IOBuf
template <typename TestData>
std::unique_ptr<folly::IOBuf> serializeProtobufArray() {
  TestData testData = createProtobufTestData<TestData>();
  size_t size = testData.ByteSizeLong();
  auto buf = folly::IOBuf::create(size);
  testData.SerializeToArray(buf->writableData(), size);
  buf->append(size);
  return buf;
}

// Protobuf serialization using SerializeAsString
template <typename TestData>
std::unique_ptr<folly::IOBuf> serializeProtobufString() {
  TestData testData = createProtobufTestData<TestData>();
  std::string serialized = testData.SerializeAsString();
  return folly::IOBuf::copyBuffer(serialized);
}

// Protobuf serialization using ZeroCopyOutputStream
template <typename TestData>
std::unique_ptr<folly::IOBuf> serializeProtobufZeroCopy() {
  TestData testData = createProtobufTestData<TestData>();
  size_t size = testData.ByteSizeLong();
  auto buf = folly::IOBuf::create(size);
  IOBufZeroCopyOutputStream stream(buf.get(), size);
  testData.SerializeToZeroCopyStream(&stream);
  buf->append(stream.ByteCount());
  return buf;
}

// Explicit template instantiations for protobuf serialization
template std::unique_ptr<folly::IOBuf>
serializeProtobufArray<benchmark::ProtobufTestDataSmall>();
template std::unique_ptr<folly::IOBuf>
serializeProtobufArray<benchmark::ProtobufTestDataMedium>();
template std::unique_ptr<folly::IOBuf>
serializeProtobufArray<benchmark::ProtobufTestDataLarge>();

// Explicit template instantiations for protobuf String serialization
template std::unique_ptr<folly::IOBuf>
serializeProtobufString<benchmark::ProtobufTestDataSmall>();
template std::unique_ptr<folly::IOBuf>
serializeProtobufString<benchmark::ProtobufTestDataMedium>();
template std::unique_ptr<folly::IOBuf>
serializeProtobufString<benchmark::ProtobufTestDataLarge>();

// Explicit template instantiations for protobuf ZeroCopy serialization
template std::unique_ptr<folly::IOBuf>
serializeProtobufZeroCopy<benchmark::ProtobufTestDataSmall>();
template std::unique_ptr<folly::IOBuf>
serializeProtobufZeroCopy<benchmark::ProtobufTestDataMedium>();
template std::unique_ptr<folly::IOBuf>
serializeProtobufZeroCopy<benchmark::ProtobufTestDataLarge>();

// Protobuf deserialization using ParseFromArray
template <typename TestData>
TestData deserializeProtobufArray(std::unique_ptr<folly::IOBuf> serialized) {
  TestData testData;
  testData.ParseFromArray(serialized->data(), serialized->length());
  return testData;
}

// Protobuf deserialization using ParseFromString
template <typename TestData>
TestData deserializeProtobufString(std::unique_ptr<folly::IOBuf> serialized) {
  TestData testData;
  std::string str(
      reinterpret_cast<const char*>(serialized->data()), serialized->length());
  testData.ParseFromString(str);
  return testData;
}

// Protobuf deserialization using ZeroCopyInputStream
template <typename TestData>
TestData deserializeProtobufZeroCopy(std::unique_ptr<folly::IOBuf> serialized) {
  TestData testData;
  IOBufZeroCopyInputStream stream(serialized.get());
  testData.ParseFromZeroCopyStream(&stream);
  return testData;
}

// Explicit template instantiations for protobuf deserialization
template benchmark::ProtobufTestDataSmall deserializeProtobufArray<
    benchmark::ProtobufTestDataSmall>(std::unique_ptr<folly::IOBuf>);
template benchmark::ProtobufTestDataMedium deserializeProtobufArray<
    benchmark::ProtobufTestDataMedium>(std::unique_ptr<folly::IOBuf>);
template benchmark::ProtobufTestDataLarge deserializeProtobufArray<
    benchmark::ProtobufTestDataLarge>(std::unique_ptr<folly::IOBuf>);

template benchmark::ProtobufTestDataSmall deserializeProtobufString<
    benchmark::ProtobufTestDataSmall>(std::unique_ptr<folly::IOBuf>);
template benchmark::ProtobufTestDataMedium deserializeProtobufString<
    benchmark::ProtobufTestDataMedium>(std::unique_ptr<folly::IOBuf>);
template benchmark::ProtobufTestDataLarge deserializeProtobufString<
    benchmark::ProtobufTestDataLarge>(std::unique_ptr<folly::IOBuf>);

template benchmark::ProtobufTestDataSmall deserializeProtobufZeroCopy<
    benchmark::ProtobufTestDataSmall>(std::unique_ptr<folly::IOBuf>);
template benchmark::ProtobufTestDataMedium deserializeProtobufZeroCopy<
    benchmark::ProtobufTestDataMedium>(std::unique_ptr<folly::IOBuf>);
template benchmark::ProtobufTestDataLarge deserializeProtobufZeroCopy<
    benchmark::ProtobufTestDataLarge>(std::unique_ptr<folly::IOBuf>);
