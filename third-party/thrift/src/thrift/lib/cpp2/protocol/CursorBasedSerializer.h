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
#include <initializer_list>
#include <folly/io/Cursor.h>
#include <folly/io/IOBufQueue.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/protocol/detail/CursorBasedSerialization.h>
#include <thrift/lib/cpp2/type/NativeType.h>
#include <thrift/lib/cpp2/type/ThriftType.h>

/**
 * Cursor serialization for Thrift objects.
 * Provides a way to read and write Thrift types without having to materialize
 * them in memory. This can result in CPU / memory savings depending on the
 * access pattern, particularly when interfacing with another data
 * representation.
 *
 * This API derives its efficiency from ordering requirements on reading and
 * writing the data. Fields can only be read / written in order of increasing
 * field id in the struct. (Skipping fields is permitted). Data serialized using
 * other sources requires fields to be sorted in field id order in the IDL
 * file, or the struct to be annotated with `@thrift.SerializeInFieldIdOrder`.
 *
 * The general pattern is paired calls to beginWrite()/endWrite() (or
 * beginRead/endRead) which return a sub-cursor that is then passed in to the
 * end function, though scalars and materialized types can be read/written
 * directly.
 *
 * test/CursorBasedSerializerTest.cpp has several complete examples of usage.
 */

namespace apache::thrift {

/**
 * Manages the lifetime of a Thrift object being used with cursor
 * de/serialization.
 * Must outlive the reader/writer objects it returns.
 */
template <typename T>
class CursorSerializationWrapper {
  using Tag = type::infer_tag<T>;
  static_assert(
      type::is_a_v<Tag, type::structured_c>, "T must be a thrift class");

 public:
  CursorSerializationWrapper() = default;

  explicit CursorSerializationWrapper(std::unique_ptr<folly::IOBuf> serialized)
      : serializedData_(std::move(serialized)) {}

  /**
   * Object write path (traditional Thrift serialization)
   * Serializes from a Thrift object.
   */
  /* implicit */ CursorSerializationWrapper(
      const T& t, ExternalBufferSharing sharing = COPY_EXTERNAL_BUFFER) {
    BinarySerializer::serialize(t, &queue_, sharing);
    serializedData_ = queue_.move();
  }

  /**
   * Object read path (traditional Thrift deserialization)
   * Deserializes into a (returned) Thrift object.
   */
  T deserialize() const {
    assert(serializedData_);
    T ret;
    BinarySerializer::deserialize(serializedData_.get(), ret);
    return ret;
  }

  /**
   * Cursor read path
   * Template parameter determines whether chained buffers are supported.
   * Setting to false allows chained buffers and disables string_view API.
   */
  template <bool Contiguous = true>
  StructuredCursorReader<Tag, Contiguous> beginRead() const {
    assert(serializedData_);
    return StructuredCursorReader<Tag, Contiguous>(*serializedData_);
  }
  template <bool Contiguous>
  void endRead(StructuredCursorReader<Tag, Contiguous>&& reader) const {
    reader.finalize();
  }

  /** Cursor write path */
  StructuredCursorWriter<Tag> beginWrite() {
    serializedData_.reset();
    return StructuredCursorWriter<Tag>(queue_);
  }
  void endWrite(StructuredCursorWriter<Tag>&& writer) {
    writer.finalize();
    serializedData_ = queue_.move();
  }

  /** Access to serialized data */
  const folly::IOBuf& serializedData() const& {
    assert(serializedData_);
    return *serializedData_;
  }
  std::unique_ptr<folly::IOBuf> serializedData() && {
    assert(serializedData_);
    return std::move(serializedData_);
  }

 private:
  std::unique_ptr<folly::IOBuf> serializedData_;
  folly::IOBufQueue queue_;
};

/**
 * Cursor deserializer for Thrift structs and unions.
 * Typically constructed from a CursorSerializationWrapper.
 */
template <typename Tag, bool Contiguous = true>
class StructuredCursorReader : detail::BaseCursorReader {
  static_assert(
      type::is_a_v<Tag, type::structured_c>, "T must be a thrift class");
  using T = type::native_type<Tag>;

  template <typename Ident>
  using type_tag = op::get_type_tag<T, Ident>;
  template <typename Ident>
  using native_type = op::get_native_type<T, Ident>;

  template <typename TypeClass, typename Ident>
  using enable_for =
      typename std::enable_if_t<type::is_a_v<type_tag<Ident>, TypeClass>, int>;

  template <typename Ident>
  using enable_string_view = typename std::enable_if_t<
      Contiguous && type::is_a_v<type_tag<Ident>, type::string_c>,
      int>;

  template <typename U, typename Ident>
  using maybe_optional = std::conditional_t<
      type::is_optional_or_union_field_v<T, Ident>,
      std::optional<U>,
      U>;

  template <typename Ident>
  using bool_if_optional = std::
      conditional_t<type::is_optional_or_union_field_v<T, Ident>, bool, void>;

 public:
  /**
   * Materializes a field into its native type (like in the generated struct).
   * Optional and union fields are wrapped in std::optional.
   *
   * Ex:
   *  StructuredCursorReader<OuterStruct> reader;
   *  std::string val1 = reader.read<ident::unqualified_string>();
   *  std::optional<InnerStruct> val2 = reader.read<ident::optional_struct>();
   *  Union val3 = reader.read<ident::unqualified_union>();
   *  StructuredCursorReader<Union> unionReader;
   *  std::optional<int32_t> val4 = unionReader.read<ident::int>();
   */
  template <typename Ident>
  maybe_optional<native_type<Ident>, Ident> read() {
    maybe_optional<native_type<Ident>, Ident> value;
    readField<Ident>(
        [&] {
          op::decode<type_tag<Ident>>(protocol_, detail::maybe_emplace(value));
        },
        value);
    return value;
  }

  /**
   * These methods allow reading a field in-place, which may be more efficient.
   * Optional and union fields return bool to indicate whether the field was
   * present; the value is not modified if the field is absent.
   *
   * Ex:
   *  StructuredCursorReader<Struct> reader;
   *  std::string val1;
   *  reader.read<ident::unqualified_string>(val1);
   *  int32_t val2;
   *  if (reader.read<ident::optional_int>(val2)) { use(val2); }
   *  // else val2 is still uninitialized
   */

  /** numeric types */

  template <typename Ident, enable_for<type::number_c, Ident> = 0>
  [[nodiscard]] bool_if_optional<Ident> read(native_type<Ident>& value) {
    return readField<Ident>(
        [&] { op::decode<type_tag<Ident>>(protocol_, value); }, value);
  }

  /** string/binary */

  template <typename Ident, enable_string_view<Ident> = 0>
  [[nodiscard]] bool_if_optional<Ident> read(std::string_view& value);

  template <typename Ident, enable_for<type::string_c, Ident> = 0>
  [[nodiscard]] bool_if_optional<Ident> read(std::string& value) {
    return readField<Ident>([&] { protocol_.readString(value); }, value);
  }

  template <typename Ident, enable_for<type::string_c, Ident> = 0>
  [[nodiscard]] bool_if_optional<Ident> read(folly::IOBuf& value) {
    return readField<Ident>([&] { protocol_.readBinary(value); }, value);
  }

  /** containers
   *
   * See the ContainerCursorReader docblock for example usage.
   *
   * Note: when beginRead returns a reader, that reader must be passed to
   * endRead before any other methods on this object can be called.
   */

  template <typename Ident, enable_for<type::container_c, Ident> = 0>
  maybe_optional<ContainerCursorReader<type_tag<Ident>>, Ident> beginRead() {
    if (!beforeReadField<Ident>()) {
      return {};
    }
    state_ = State::Child;
    return ContainerCursorReader<type_tag<Ident>>{std::move(protocol_)};
  }

  template <typename CTag>
  void endRead(ContainerCursorReader<CTag>&& child) {
    if (state_ != State::Child) {
      // This is a sentinel iterator for an empty container.
      DCHECK_EQ(child.remaining_, 0);
      return;
    }
    child.finalize();
    protocol_ = std::move(child.protocol_);
    afterReadField();
    state_ = State::Active;
  }

  /** structured types
   *
   * Note: when beginRead returns a reader, that reader must be passed to
   * endRead before any other methods on this object can be called.
   */

  template <typename Ident, enable_for<type::structured_c, Ident> = 0>
  maybe_optional<StructuredCursorReader<type_tag<Ident>, Contiguous>, Ident>
  beginRead() {
    if (!beforeReadField<Ident>()) {
      return {};
    }
    state_ = State::Child;
    return StructuredCursorReader<type_tag<Ident>, Contiguous>{
        std::move(protocol_)};
  }

  template <typename CTag>
  void endRead(StructuredCursorReader<CTag, Contiguous>&& child) {
    checkState(State::Child);
    child.finalize();
    protocol_ = std::move(child.protocol_);
    afterReadField();
    state_ = State::Active;
  }

  /** union type accessor */

  template <
      typename...,
      typename U = T,
      typename = std::enable_if_t<is_thrift_union_v<U>>>
  auto readType() -> typename U::Type {
    return static_cast<typename T::Type>(readState_.fieldId);
  }

 private:
  explicit StructuredCursorReader(const folly::IOBuf& c)
      : StructuredCursorReader([&] {
          folly::io::Cursor cursor(&c);
          BinaryProtocolReader reader;
          reader.setInput(cursor);
          return reader;
        }()) {
    if (Contiguous && c.isChained()) {
      folly::throw_exception<std::runtime_error>(
          "Chained buffer passed to contiguous reader.");
    }
  }
  explicit StructuredCursorReader(BinaryProtocolReader&& p)
      : BaseCursorReader(std::move(p)) {
    readState_.readStructBegin(&protocol_);
    readState_.readFieldBegin(&protocol_);
  }
  StructuredCursorReader() { readState_.fieldType = TType::T_STOP; }

  void finalize() {
    checkState(State::Active);
    if (readState_.fieldType != TType::T_STOP) {
      protocol_.skip(readState_.fieldType);
      readState_.readFieldEnd(&protocol_);
      // Because there's no struct begin marker in Binary protocol this skips
      // the rest of the struct.
      protocol_.skip(TType::T_STRUCT);
    }
    readState_.readStructEnd(&protocol_);
    state_ = State::Done;
  }

  template <typename Ident>
  bool beforeReadField() {
    checkState(State::Active);

    using field_id = op::get_field_id<T, Ident>;
    static_assert(field_id::value > FieldId{0}, "FieldId must be positive");
    if (field_id::value <= fieldId_) {
      folly::throw_exception<std::runtime_error>("Reading field out of order");
    }
    fieldId_ = field_id::value;

    while (readState_.fieldType != TType::T_STOP &&
           FieldId{readState_.fieldId} < field_id::value) {
      readState_.readFieldEnd(&protocol_);
      apache::thrift::skip(protocol_, readState_.fieldType);
      int16_t lastRead = readState_.fieldId;
      readState_.readFieldBegin(&protocol_);
      if (lastRead >= readState_.fieldId) {
        folly::throw_exception<std::runtime_error>(
            "Reading fields that were serialized out of order");
      }
      lastRead = readState_.fieldId;
    }

    if (FieldId{readState_.fieldId} != field_id::value) {
      return false;
    }
    if (readState_.fieldType != op::typeTagToTType<type_tag<Ident>>) {
      folly::throw_exception<std::runtime_error>(fmt::format(
          "Field type mismatch: expected {}, got {}.",
          readState_.fieldType,
          op::typeTagToTType<type_tag<Ident>>));
    }
    return true;
  }

  void afterReadField() {
    readState_.readFieldEnd(&protocol_);
    readState_.readFieldBegin(&protocol_);
  }

  template <typename Ident, typename F, typename U>
  bool_if_optional<Ident> readField(F&& f, U& val) {
    bool ret = [&] {
      if (!beforeReadField<Ident>()) {
        return false;
      }
      f();
      afterReadField();
      return true;
    }();
    if constexpr (type::is_optional_or_union_field_v<T, Ident>) {
      return ret;
    } else if (ret) {
    } else if (type::is_terse_field_v<T, Ident>) {
      val = *op::get<Ident>(op::getIntrinsicDefault<T>());
    } else {
      val = *op::get<Ident>(op::getDefault<T>());
    }
  }

 private:
  // Last field id the caller tried to read.
  FieldId fieldId_{0};
  // Contains last field id read from the buffer.
  BinaryProtocolReader::StructReadState readState_;

  template <typename, bool>
  friend class StructuredCursorReader;
  friend class CursorSerializationWrapper<T>;
};

/**
 * Allows iterating over containers without materializing them.
 * Behaves like a standard collection, ex:
 *  StructuredCursorReader<Struct> reader;
 *  auto listReader = reader.beginRead<ident::unqualified_list>();
 *  for (auto& val : listReader) { use(val); }
 *  reader.endRead(listReader);
 *
 *  auto mapReader = reader.beginRead<ident::unqualified_map>();
 *  std::unordered_map<K, V> map;
 *  map.reserve(mapReader.size());
 *  map.insert(mapReader.begin(), mapReader.end());
 *  reader.endRead(mapReader);
 */
template <typename Tag>
class ContainerCursorReader : detail::BaseCursorReader {
 public:
  ContainerCursorIterator<Tag> begin() {
    checkState(State::Active);
    return remaining_ ? ContainerCursorIterator<Tag>{*this}
                      : ContainerCursorIterator<Tag>{};
  }
  ContainerCursorIterator<Tag> end() const { return {}; }

  // Returns remaining size if reading has begun.
  int32_t remaining() const {
    checkState(State::Active);
    return remaining_;
  }
  [[deprecated("Call remaining() instead")]] int32_t size() const {
    return remaining();
  }

 private:
  explicit ContainerCursorReader(BinaryProtocolReader&& p);
  ContainerCursorReader() { remaining_ = 0; }

  void finalize();

  bool advance() {
    checkState(State::Active);
    DCHECK_GT(remaining_, 0);
    if (--remaining_ == 0) {
      return false;
    }
    read();
    return true;
  }

  typename detail::ContainerTraits<Tag>::ElementType& currentValue() {
    checkState(State::Active);
    return lastRead_;
  }

  void read();

  // remaining_ includes the element cached in the reader as lastRead_.
  uint32_t remaining_;
  typename detail::ContainerTraits<Tag>::ElementType lastRead_;

  template <typename, bool>
  friend class StructuredCursorReader;
  friend class ContainerCursorIterator<Tag>;
};

/**
 * Allows iterating over containers without materializing them.
 * This is an InputIterator, which means it must be iterated in a single pass.
 * Operator++ invalidates all other copies of the iterator.
 */
template <typename Tag>
class ContainerCursorIterator {
 public:
  using iterator_category = std::input_iterator_tag;
  using difference_type = ssize_t;
  using value_type = typename detail::ContainerTraits<Tag>::ElementType;
  // These are non-const to allow moving the deserialzed value out.
  // Modifying them does not affect the underlying buffer.
  using pointer = value_type*;
  using reference = value_type&;

  explicit ContainerCursorIterator(ContainerCursorReader<Tag>& in)
      : reader_(&in) {}
  ContainerCursorIterator() = default;

  // Prefix increment
  ContainerCursorIterator& operator++() {
    if (!reader_ || !reader_->advance()) {
      reader_ = nullptr;
    }
    return *this;
  }

  // Postfix increment
  // This iterator only targets weakly_incrementable, which allows void return.
  void operator++(int) { ++(*this); }

  // Dereference
  // These return non-const to allow moving the deserialzed value out.
  // Modifying them does not affect the underlying buffer.
  reference operator*() { return reader_->currentValue(); }
  pointer operator->() { return &reader_->currentValue(); }

  // Equality is only defined against itself and the end iterator
  bool operator==(ContainerCursorIterator other) const {
    return reader_ == other.reader_;
  }

 private:
  ContainerCursorReader<Tag>* reader_ = nullptr;
};

/**
 * Allows writing strings whose size is not known exactly until after
 * serialization is complete (e.g. for compression).
 * An upper bound on the size must be known. Accessing more than `maxSize` bytes
 * into `writeableData()` is undefined behavior (ASAN crash / data corruption).
 * Gives access to a contiguous buffer for writing into.
 * Ex:
 *  StructuredCursorWriter writer;
 *  std::string uncompressedData = ...;
 *  auto maxSize = uncompressedData.size();
 *  auto child = writer.beginWrite<ident::binary_field>(maxSize);
 *  int32_t cSize = ZSTD_compress(child.writeableData(), maxSize,
 *                                uncompressedData.c_str(), maxSize, 1);
 *  writer.endWrite(std::move(child), cSize);
 */
class StringCursorWriter : detail::DelayedSizeCursorWriter {
 public:
  uint8_t* writeableData() {
    checkState(State::Active);
    return data_;
  }

 private:
  StringCursorWriter(BinaryProtocolWriter&& p, int32_t maxSize)
      : DelayedSizeCursorWriter(std::move(p)) {
    writeSize();
    data_ = protocol_.ensure(maxSize);
  }

  void finalize(int32_t actualSize) {
    DelayedSizeCursorWriter::finalize(actualSize);
    protocol_.advance(actualSize);
  }

  uint8_t* data_;

  template <typename T>
  friend class StructuredCursorWriter;
};

/**
 * Cursor serializer for Thrift structs and unions.
 * Typically constructed from a CursorSerializationWrapper.
 */
template <typename Tag>
class StructuredCursorWriter : detail::BaseCursorWriter {
  static_assert(
      type::is_a_v<Tag, type::structured_c>, "T must be a thrift class");
  using T = type::native_type<Tag>;

  template <typename Ident>
  using type_tag = op::get_type_tag<T, Ident>;
  template <typename Ident>
  using native_type = op::get_native_type<T, Ident>;

  template <typename TypeClass, typename Ident>
  using enable_for =
      typename std::enable_if_t<type::is_a_v<type_tag<Ident>, TypeClass>, int>;

 public:
  /** numeric types */

  template <typename Ident, enable_for<type::number_c, Ident> = 0>
  void write(native_type<Ident> value) {
    writeField<Ident>(
        [&] { op::encode<type_tag<Ident>>(protocol_, value); }, value);
  }

  /** string/binary */

  template <typename Ident, enable_for<type::string_c, Ident> = 0>
  void write(const folly::IOBuf& value) {
    writeField<Ident>([&] { protocol_.writeBinary(value); }, value);
  }

  template <typename Ident, enable_for<type::string_c, Ident> = 0>
  void write(std::string_view value) {
    writeField<Ident>([&] { protocol_.writeBinary(value); }, value);
  }

  /** Allows writing strings whose size isn't known until afterwards.
   * See the StringCursorWriter docblock for example usage.
   *
   * Note: none of this writer's other methods may be called between
   * beginWrite() and the corresponding endWrite().
   */

  template <typename Ident, enable_for<type::string_c, Ident> = 0>
  StringCursorWriter beginWrite(int32_t maxSize) {
    beforeWriteField<Ident>();
    state_ = State::Child;
    StringCursorWriter child{std::move(protocol_), maxSize};
    return child;
  }

  void endWrite(StringCursorWriter&& child, int32_t actualSize) {
    checkState(State::Child);
    child.finalize(actualSize);
    protocol_ = std::move(child.protocol_);
    afterWriteField();
    state_ = State::Active;
  }

  /** containers */

  template <
      typename Ident,
      typename Container,
      enable_for<type::container_c, Ident> = 0>
  void write(const Container& value) {
    writeField<Ident>(
        [&] { op::encode<type_tag<Ident>>(protocol_, value); }, value);
  }

  /** Allows writing containers whose size isn't known until afterwards.
   * Less efficient than using write().
   * See the ContainerCursorWriter docblock for example usage.
   *
   * Note: none of this writer's other methods may be called between
   * beginWrite() and the corresponding endWrite().
   */

  template <typename Ident, enable_for<type::container_c, Ident> = 0>
  ContainerCursorWriter<type_tag<Ident>> beginWrite() {
    beforeWriteField<Ident>();
    state_ = State::Child;
    return ContainerCursorWriter<type_tag<Ident>>{std::move(protocol_)};
  }

  template <typename CTag>
  void endWrite(ContainerCursorWriter<CTag>&& child) {
    checkState(State::Child);
    child.finalize();
    protocol_ = std::move(child.protocol_);
    afterWriteField();
    state_ = State::Active;
  }

 private:
  explicit StructuredCursorWriter(folly::IOBufQueue& q)
      : StructuredCursorWriter([&] {
          BinaryProtocolWriter writer(SHARE_EXTERNAL_BUFFER);
          writer.setOutput(&q);
          return writer;
        }()) {}
  explicit StructuredCursorWriter(BinaryProtocolWriter&& p)
      : BaseCursorWriter(std::move(p)) {
    protocol_.writeStructBegin(nullptr);
  }

  void finalize() {
    checkState(State::Active);
    visitSkippedFields<FieldId{std::numeric_limits<int16_t>::max()}>();
    protocol_.writeFieldStop();
    protocol_.writeStructEnd();
    state_ = State::Done;
  }

  template <typename Ident, bool skipWrite = false>
  void beforeWriteField() {
    checkState(State::Active);

    using field_id = op::get_field_id<T, Ident>;
    static_assert(field_id::value > FieldId{0}, "FieldId must be positive");
    static_assert(
        field_id::value < FieldId{std::numeric_limits<int16_t>::max()},
        "Preventing overflow");
    if (field_id::value <= fieldId_) {
      folly::throw_exception<std::runtime_error>("Writing field out of order");
    }
    if (is_thrift_union_v<T> && fieldId_ != FieldId{0}) {
      folly::throw_exception<std::runtime_error>(
          "Writing multiple fields to union");
    }

    visitSkippedFields<field_id::value>();

    fieldId_ = field_id::value;
    if (skipWrite) {
      return;
    }
    protocol_.writeFieldBegin(
        nullptr,
        op::typeTagToTType<type_tag<Ident>>,
        folly::to_underlying(field_id::value));
  }

  void afterWriteField() { protocol_.writeFieldEnd(); }

  template <typename Ident, typename F, typename U>
  void writeField(F&& f, U& value) {
    // Apply terse write optimization.
    if constexpr (type::is_terse_field_v<T, Ident>) {
      if (op::isEmpty<type_tag<Ident>>(value)) {
        beforeWriteField<Ident, /* skipWrite */ true>();
        return;
      }
    }

    beforeWriteField<Ident>();
    f();
    afterWriteField();
  }

  template <FieldId MaxFieldId>
  void visitSkippedFields() {
    if constexpr (is_thrift_union_v<T>) {
      return;
    }

    // Fast path: consecutive fields
    if (detail::increment(fieldId_) == MaxFieldId) {
      return;
    }

    // Slow path: iterate fields between fieldId_ and MaxFieldId.
    const auto& fieldWriters = detail::DefaultValueWriter<Tag>::fields;
    for (auto itr = std::lower_bound(
             fieldWriters.begin(),
             fieldWriters.end(),
             decltype(*fieldWriters.begin()){detail::increment(fieldId_)});
         itr != fieldWriters.end() && itr->id < MaxFieldId;
         ++itr) {
      (itr->write)(*this);
    }
  }

  FieldId fieldId_{0};

  template <typename U>
  friend class StructuredCursorWriter;
  friend class CursorSerializationWrapper<T>;
  friend struct detail::DefaultValueWriter<Tag>;
};

/**
 * Allows writing containers whose size is not known in advance.
 * Ex:
 *  StructuredCursorWriter writer;
 *  auto child = writer.beginWrite<ident::list_field>();
 *  folly::coro::AsyncGenerator<int32_t> gen = ...;
 *  while (auto val = co_await gen.next()) {
 *    child.write(*val);
 *  }
 *  writer.endWrite(std::move(child));
 */
template <typename Tag>
class ContainerCursorWriter : detail::DelayedSizeCursorWriter {
 public:
  void write(const typename detail::ContainerTraits<Tag>::ElementType& val) {
    ++n;
    detail::ContainerTraits<Tag>::write(protocol_, val);
  }

 private:
  explicit ContainerCursorWriter(BinaryProtocolWriter&& p);

  template <typename T>
  friend class StructuredCursorWriter;

  void finalize() { DelayedSizeCursorWriter::finalize(n); }

  int32_t n = 0;
};

/**
 * Adapter (for use with `@cpp.Adapter` annotation) that permits using this
 * serialization with Thrift RPC.
 */
class CursorSerializationAdapter {
 public:
  template <typename T>
  static CursorSerializationWrapper<T> fromThrift(const T& t);
  template <typename T>
  static T toThrift(const CursorSerializationWrapper<T>& w);

  template <typename Tag, typename Protocol, typename T>
  static uint32_t encode(
      Protocol& prot_, const CursorSerializationWrapper<T>& wrapper) {
    if constexpr (std::is_same_v<Protocol, BinaryProtocolWriter>) {
      return prot_.writeRaw(wrapper.serializedData());
    } else {
      folly::throw_exception<std::runtime_error>(
          "Single pass serialization only supports binary protocol.");
    }
  }

  template <typename Tag, typename Protocol, typename T>
  static void decode(Protocol& prot_, CursorSerializationWrapper<T>& wrapper) {
    if constexpr (std::is_same_v<Protocol, BinaryProtocolReader>) {
      std::unique_ptr<folly::IOBuf> buf;
      folly::copy(prot_.getCursor())
          .cloneAtMost(buf, std::numeric_limits<size_t>::max());
      // -1 to leave the stop marker for the presult struct when used in an RPC.
      prot_.skipBytes(buf->computeChainDataLength() - 1);

      if constexpr (folly::kIsDebug) {
        // Check that the buffer only contains one struct in it (without
        // advancing the real cursor).
        Protocol protForDebug;
        protForDebug.setInput(buf.get());
        protForDebug.skip(protocol::T_STRUCT);
        DCHECK_LE(
            buf->computeChainDataLength(), protForDebug.getCursorPosition() + 1)
            << "Cursor serialization only supports messages containing a single struct or union.";
      }

      wrapper = CursorSerializationWrapper<T>{std::move(buf)};
    } else {
      folly::throw_exception<std::runtime_error>(
          "Single pass serialization only supports binary protocol.");
    }
  }

  template <bool ZC, typename Tag, typename Protocol, typename T>
  static uint32_t serializedSize(
      Protocol&, const CursorSerializationWrapper<T>& wrapper) {
    return ZC ? 0 : wrapper.serializedData().computeChainDataLength();
  }
};

// End public API

template <typename Tag>
ContainerCursorWriter<Tag>::ContainerCursorWriter(BinaryProtocolWriter&& p)
    : DelayedSizeCursorWriter(std::move(p)) {
  if constexpr (type::is_a_v<Tag, type::list_c>) {
    protocol_.writeByte(
        op::typeTagToTType<typename detail::ContainerTraits<Tag>::ValueTag>);
  } else if constexpr (type::is_a_v<Tag, type::set_c>) {
    protocol_.writeByte(
        op::typeTagToTType<typename detail::ContainerTraits<Tag>::KeyTag>);
  } else if constexpr (type::is_a_v<Tag, type::map_c>) {
    protocol_.writeByte(
        op::typeTagToTType<typename detail::ContainerTraits<Tag>::KeyTag>);
    protocol_.writeByte(
        op::typeTagToTType<typename detail::ContainerTraits<Tag>::ValueTag>);
  } else {
    static_assert(!sizeof(Tag), "unexpected tag");
  }
  writeSize();
}

template <typename Tag, bool Contiguous>
template <
    typename Ident,
    typename StructuredCursorReader<Tag, Contiguous>::
        template enable_string_view<Ident>>
typename StructuredCursorReader<Tag, Contiguous>::template bool_if_optional<
    Ident>
StructuredCursorReader<Tag, Contiguous>::read(std::string_view& value) {
  return readField<Ident>(
      [&] {
        int32_t size;
        protocol_.readI32(size);
        if (size < 0) {
          TProtocolException::throwNegativeSize();
        }
        folly::io::Cursor c = protocol_.getCursor();
        if (static_cast<size_t>(size) >= c.length()) {
          TProtocolException::throwTruncatedData();
        }
        value = std::string_view(reinterpret_cast<const char*>(c.data()), size);
        protocol_.skipBytes(size);
      },
      value);
}

template <typename Tag>
ContainerCursorReader<Tag>::ContainerCursorReader(BinaryProtocolReader&& p)
    : BaseCursorReader(std::move(p)) {
  // Check element types
  if constexpr (type::is_a_v<Tag, type::list_c>) {
    TType type;
    protocol_.readListBegin(type, remaining_);
    if (type !=
        op::typeTagToTType<typename detail::ContainerTraits<Tag>::ValueTag>) {
      folly::throw_exception<std::runtime_error>(
          "Unexpected element type in list");
    }
  } else if constexpr (type::is_a_v<Tag, type::set_c>) {
    TType type;
    protocol_.readSetBegin(type, remaining_);
    if (type !=
        op::typeTagToTType<typename detail::ContainerTraits<Tag>::KeyTag>) {
      folly::throw_exception<std::runtime_error>(
          "Unexpected element type in set");
    }
  } else if constexpr (type::is_a_v<Tag, type::map_c>) {
    TType key, value;
    protocol_.readMapBegin(key, value, remaining_);
    if (key !=
            op::typeTagToTType<typename detail::ContainerTraits<Tag>::KeyTag> ||
        value !=
            op::typeTagToTType<
                typename detail::ContainerTraits<Tag>::ValueTag>) {
      folly::throw_exception<std::runtime_error>("Unexpected key type in map");
    }
  } else {
    static_assert(!sizeof(Tag), "unexpected tag");
  }

  if (remaining_) {
    read();
  }
}

template <typename Tag>
void ContainerCursorReader<Tag>::finalize() {
  checkState(State::Active);
  state_ = State::Done;

  if (remaining_ > 1) {
    // Skip remaining unread elements
    skip_n(protocol_, remaining_ - 1, detail::ContainerTraits<Tag>::wireTypes);
  }

  if constexpr (type::is_a_v<Tag, type::list_c>) {
    protocol_.readListEnd();
  } else if constexpr (type::is_a_v<Tag, type::set_c>) {
    protocol_.readSetEnd();
  } else if constexpr (type::is_a_v<Tag, type::map_c>) {
    protocol_.readMapEnd();
  } else {
    static_assert(!sizeof(Tag), "unexpected tag");
  }
}

template <typename Tag>
void ContainerCursorReader<Tag>::read() {
  DCHECK_GT(remaining_, 0);

  if constexpr (type::is_a_v<Tag, type::list_c>) {
    op::decode<typename detail::ContainerTraits<Tag>::ValueTag>(
        protocol_, lastRead_);
  } else if constexpr (type::is_a_v<Tag, type::set_c>) {
    op::decode<typename detail::ContainerTraits<Tag>::KeyTag>(
        protocol_, lastRead_);
  } else if constexpr (type::is_a_v<Tag, type::map_c>) {
    op::decode<typename detail::ContainerTraits<Tag>::KeyTag>(
        protocol_, lastRead_.first);
    op::decode<typename detail::ContainerTraits<Tag>::ValueTag>(
        protocol_, lastRead_.second);
  } else {
    static_assert(!sizeof(Tag), "unexpected tag");
  }
}

} // namespace apache::thrift
