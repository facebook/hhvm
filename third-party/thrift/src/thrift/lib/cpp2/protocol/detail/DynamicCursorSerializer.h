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

#include <thrift/lib/cpp2/protocol/CursorBasedSerializer.h>
#include <thrift/lib/cpp2/protocol/Object.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>

namespace apache::thrift::detail {

template <typename ProtocolReader, typename ProtocolWriter>
class DynamicCursorSerializationWrapper;
template <typename ProtocolReader, bool Contiguous>
class ContainerDynamicCursorReader;
template <typename ProtocolWriter>
class ContainerDynamicCursorWriter;

template <typename Tag>
void ensureTypesCompatible(protocol::TType nextTType) {
  auto nextType = type::toBaseType(nextTType);
  auto requestedType = type::base_type_v<Tag>;
  if (requestedType == type::BaseType::Union) {
    // TType doesn't distinguish union and struct
    requestedType = type::BaseType::Struct;
  }
  if (requestedType == type::BaseType::String) {
    // TType doesn't distinguish binary and string
    requestedType = type::BaseType::Binary;
  }
  if (nextType != requestedType) {
    folly::throw_exception<std::runtime_error>(fmt::format(
        "Expected type {} but got {}",
        type::getBaseTypeName(requestedType),
        type::getBaseTypeName(nextType)));
  }
}

template <typename ProtocolReader, bool Contiguous>
class StructuredDynamicCursorReader : detail::BaseCursorReader<ProtocolReader> {
  using Base = detail::BaseCursorReader<ProtocolReader>;
  using State = typename Base::State;
  using Base::checkState;
  using Base::protocol_;
  using Base::state_;
  template <typename Tag>
  using view_type = detail::lift_view_t<type::native_type<Tag>, Contiguous>;
  using ContainerDynamicCursorReader =
      ContainerDynamicCursorReader<ProtocolReader, Contiguous>;

 public:
  protocol::TType fieldType() { return readState_.fieldType; }
  int16_t fieldId() { return readState_.fieldId; }

  template <typename Tag>
  view_type<Tag> read(Tag) {
    beforeReadField();
    ensureTypesCompatible<Tag>(readState_.fieldType);
    view_type<Tag> ret;
    if constexpr (std::is_same_v<view_type<Tag>, std::string_view>) {
      ret = detail::readStringView(*protocol_);
    } else {
      op::decode<Tag>(*protocol_, ret);
    }
    afterReadField();
    return ret;
  }

  protocol::Value readValue() {
    beforeReadField();
    auto ret = apache::thrift::protocol::detail::parseValue(
        *protocol_, readState_.fieldType, true);
    afterReadField();
    return ret;
  }

  std::unique_ptr<folly::IOBuf> readRaw() {
    beforeReadField();
    auto begin = protocol_->getCursor();
    protocol_->skip(readState_.fieldType);
    auto end = protocol_->getCursor();

    auto numBytes = end - begin;
    auto ret = folly::IOBuf::create(numBytes);
    begin.clone(*ret, numBytes);
    afterReadField();
    return ret;
  }

  StructuredDynamicCursorReader beginReadStructured() {
    beforeReadField();
    if (readState_.fieldType != TType::T_STRUCT) {
      folly::throw_exception<std::runtime_error>(
          "Expected T_STRUCT for structured read");
    }
    state_ = State::Child;
    return StructuredDynamicCursorReader(protocol_);
  }
  void endRead(StructuredDynamicCursorReader&& reader) {
    checkState(State::Child);
    reader.finalize();
    afterReadField();
    state_ = State::Active;
  }
  void abandonRead(StructuredDynamicCursorReader&& reader) {
    checkState(State::Child);
    reader.abandon();
    state_ = State::Abandoned;
  }

  ContainerDynamicCursorReader beginReadContainer() {
    beforeReadField();
    state_ = State::Child;
    return ContainerDynamicCursorReader(protocol_, readState_.fieldType);
  }
  void endRead(ContainerDynamicCursorReader&& reader) {
    checkState(State::Child);
    reader.finalize();
    afterReadField();
    state_ = State::Active;
  }
  void abandonRead(ContainerDynamicCursorReader&& reader) {
    checkState(State::Child);
    reader.abandon();
    state_ = State::Abandoned;
  }

  void skip() {
    beforeReadField();
    readState_.skip(protocol_);
    afterReadField();
  }

 private:
  explicit StructuredDynamicCursorReader(ProtocolReader* p)
      : detail::BaseCursorReader<ProtocolReader>(p) {
    readState_.readStructBegin(protocol_);
    readState_.readFieldBegin(protocol_);
  }
  StructuredDynamicCursorReader() { readState_.fieldType = TType::T_STOP; }

  void finalize() {
    checkState(State::Active);
    while (readState_.fieldType != TType::T_STOP) {
      protocol_->skip(readState_.fieldType);
      readState_.readFieldEnd(protocol_);
      readState_.readFieldBegin(protocol_);
    }
    readState_.readStructEnd(protocol_);
    state_ = State::Done;
  }

  void afterReadField() {
    readState_.readFieldEnd(protocol_);
    readState_.readFieldBegin(protocol_);
  }

  void beforeReadField() {
    checkState(State::Active);
    if (readState_.fieldType == TType::T_STOP) {
      folly::throw_exception<std::runtime_error>("No more fields available");
    }
  }

  typename ProtocolReader::StructReadState readState_{};

  friend ContainerDynamicCursorReader;
  template <typename Reader, typename Writer>
  friend class DynamicCursorSerializationWrapper;
};

template <typename ProtocolReader, bool Contiguous>
class ContainerDynamicCursorReader : detail::BaseCursorReader<ProtocolReader> {
  using Base = detail::BaseCursorReader<ProtocolReader>;
  using State = typename Base::State;
  using Base::checkState;
  using Base::protocol_;
  using Base::state_;
  template <typename Tag>
  using view_type = detail::lift_view_t<type::native_type<Tag>, Contiguous>;
  using StructuredDynamicCursorReader =
      StructuredDynamicCursorReader<ProtocolReader, Contiguous>;
  template <typename Tag>
  static constexpr bool vectorize =
      ProtocolReader::kSupportsArithmeticVectors() &&
      !std::is_same_v<view_type<Tag>, bool> &&
      std::is_arithmetic_v<view_type<Tag>>;

  uint32_t remaining_;
  std::optional<protocol::TType> keyType_; // maps only
  protocol::TType valueType_; // includes sets
  protocol::TType containerType_;
  bool partialRead_ = false; // key has been read but not the value.
 public:
  // Only for maps
  protocol::TType keyType() { return *keyType_; }
  protocol::TType valueType() { return valueType_; }
  uint32_t remaining() { return remaining_; }

  // For maps, the caller is responsible for alternating between reading keys
  // and values.
  template <typename Tag>
  view_type<Tag> read(Tag) {
    checkRemaining();
    ensureTypesCompatible<Tag>(nextTType());
    view_type<Tag> ret;
    if constexpr (std::is_same_v<view_type<Tag>, std::string_view>) {
      ret = detail::readStringView(*protocol_);
    } else {
      op::decode<Tag>(*protocol_, ret);
    }
    advance();
    return ret;
  }

  // Writes to the provided span, and returns the subspan that was written to.
  template <typename Tag>
  std::span<view_type<Tag>> readChunk(Tag, std::span<view_type<Tag>> buf) {
    checkRemaining();
    ensureTypesCompatible<Tag>(nextTType());
    if (keyType_) {
      folly::throw_exception<std::runtime_error>(
          "readChunk doesn't support maps");
    }
    using T = view_type<Tag>;
    static_assert(
        std::is_arithmetic_v<T>, "readChunk only supports arithmetic types");
    auto n = std::min<size_t>(buf.size(), remaining_);
    if (vectorize<Tag>) {
      protocol_->template readArithmeticVector<T>(buf.data(), n);
    } else {
      for (size_t i = 0; i < n; i++) {
        op::decode<Tag>(*protocol_, buf[i]);
      }
    }
    remaining_ -= n;
    return buf.subspan(0, n);
  }

  protocol::Value readValue() {
    checkRemaining();
    auto ret = apache::thrift::protocol::detail::parseValue(
        *protocol_, nextTType(), true);
    advance();
    return ret;
  }

  std::unique_ptr<folly::IOBuf> readRaw() {
    checkRemaining();
    auto begin = protocol_->getCursor();
    protocol_->skip(nextTType());
    auto end = protocol_->getCursor();

    auto numBytes = end - begin;
    auto ret = folly::IOBuf::create(numBytes);
    begin.clone(*ret, numBytes);
    advance();
    return ret;
  }

  StructuredDynamicCursorReader beginReadStructured() {
    checkRemaining();
    if (nextTType() != protocol::TType::T_STRUCT) {
      folly::throw_exception<std::runtime_error>(
          "Expected T_STRUCT for structured read");
    }
    state_ = State::Child;
    return StructuredDynamicCursorReader(protocol_);
  }
  void endRead(StructuredDynamicCursorReader&& reader) {
    checkState(State::Child);
    reader.finalize();
    advance();
    state_ = State::Active;
  }
  void abandonRead(StructuredDynamicCursorReader&& reader) {
    checkState(State::Child);
    reader.abandon();
    state_ = State::Abandoned;
  }

  ContainerDynamicCursorReader beginReadContainer() {
    checkRemaining();
    state_ = State::Child;
    return ContainerDynamicCursorReader(protocol_, nextTType());
  }
  void endRead(ContainerDynamicCursorReader&& reader) {
    checkState(State::Child);
    reader.finalize();
    advance();
    state_ = State::Active;
  }
  void abandonRead(ContainerDynamicCursorReader&& reader) {
    checkState(State::Child);
    reader.abandon();
    state_ = State::Abandoned;
  }

  // Skips either key or value for map, for consistency with read().
  void skip() {
    checkRemaining();
    if (keyType_ && !partialRead_) {
      protocol_->skip(*keyType_);
    } else {
      protocol_->skip(valueType_);
    }
    advance();
  }

 private:
  void checkRemaining() {
    checkState(State::Active);
    if (remaining_ == 0 && !partialRead_) {
      folly::throw_exception<std::out_of_range>("No more elements available");
    }
  }
  void advance() {
    if (keyType_) {
      if (partialRead_) {
        remaining_--;
        partialRead_ = false;
      } else {
        partialRead_ = true;
      }
    } else {
      remaining_--;
    }
  }
  protocol::TType nextTType() {
    if (keyType_ && !partialRead_) {
      return *keyType_;
    }
    return valueType_;
  }

  ContainerDynamicCursorReader(ProtocolReader* p, protocol::TType containerType)
      : detail::BaseCursorReader<ProtocolReader>(p),
        containerType_(containerType) {
    if (containerType == protocol::TType::T_MAP) {
      protocol_->readMapBegin(keyType_.emplace(), valueType_, remaining_);
    } else if (containerType == protocol::TType::T_SET) {
      protocol_->readSetBegin(valueType_, remaining_);
    } else if (containerType == protocol::TType::T_LIST) {
      protocol_->readListBegin(valueType_, remaining_);
    } else {
      folly::throw_exception<std::runtime_error>("Unexpected container type");
    }
  }

  void finalize() {
    checkState(State::Active);
    while (remaining_ || partialRead_) {
      skip();
    }
    if (containerType_ == protocol::TType::T_MAP) {
      protocol_->readMapEnd();
    } else if (containerType_ == protocol::TType::T_SET) {
      protocol_->readSetEnd();
    } else if (containerType_ == protocol::TType::T_LIST) {
      protocol_->readListEnd();
    }
    state_ = State::Done;
  }

  friend StructuredDynamicCursorReader;
};

template <typename ProtocolWriter>
class StructuredDynamicCursorWriter : detail::BaseCursorWriter<ProtocolWriter> {
  using Base = detail::BaseCursorWriter<ProtocolWriter>;
  using State = typename Base::State;
  using Base::checkState;
  using Base::protocol_;
  using Base::state_;
  template <typename Tag>
  using view_type = detail::lift_view_t<type::native_type<Tag>, true>;
  using ContainerDynamicCursorWriter =
      ContainerDynamicCursorWriter<ProtocolWriter>;

 public:
  template <typename Tag>
  void write(int16_t fieldId, Tag, const view_type<Tag>& value) {
    beforeWriteField(fieldId, op::typeTagToTType<Tag>);
    op::encode<Tag>(*protocol_, value);
    afterWriteField();
  }

  // Allows writing std::array/initializer_list directly instead of copying to
  // std::vector first.
  template <typename Tag>
  void write(
      int16_t fieldId,
      Tag,
      std::span<typename view_type<Tag>::value_type> value) {
    beforeWriteField(fieldId, op::typeTagToTType<Tag>);
    op::encode<Tag>(*protocol_, value);
    afterWriteField();
  }

  void writeValue(int16_t fieldId, const protocol::Value& value) {
    beforeWriteField(fieldId, protocol::detail::getTType(value));
    protocol::serializeValue(*protocol_, value);
    afterWriteField();
  }

  template <typename Tag>
  void writeRaw(int16_t fieldId, Tag, const folly::IOBuf& value) {
    return writeRaw(fieldId, op::typeTagToTType<Tag>, value);
  }
  void writeRaw(
      int16_t fieldId, protocol::TType type, const folly::IOBuf& value) {
    beforeWriteField(fieldId, type);
    protocol_->writeRaw(value);
    afterWriteField();
  }

  StructuredDynamicCursorWriter beginWriteStructured(int16_t fieldId) {
    beforeWriteField(fieldId, protocol::TType::T_STRUCT);
    state_ = State::Child;
    return StructuredDynamicCursorWriter(protocol_);
  }
  void endWrite(StructuredDynamicCursorWriter&& writer) {
    checkState(State::Child);
    writer.finalize();
    afterWriteField();
    state_ = State::Active;
  }
  void abandonWrite(StructuredDynamicCursorWriter&& writer) {
    checkState(State::Child);
    writer.abandon();
    state_ = State::Abandoned;
  }

  template <typename Tag>
  ContainerDynamicCursorWriter beginWriteContainer(
      int16_t fieldId, Tag, uint32_t size) {
    std::optional<protocol::TType> keyTypeForMap;
    if constexpr (type::is_a_v<Tag, type::map_c>) {
      keyTypeForMap = op::typeTagToTType<typename Tag::key_tag>;
    }
    return beginWriteContainer(
        fieldId,
        op::typeTagToTType<Tag>,
        size,
        op::typeTagToTType<typename Tag::value_tag>,
        keyTypeForMap);
  }
  ContainerDynamicCursorWriter beginWriteContainer(
      int16_t fieldId,
      protocol::TType containerType,
      uint32_t size,
      protocol::TType valueType,
      std::optional<protocol::TType> keyTypeForMap = {}) {
    beforeWriteField(fieldId, containerType);
    state_ = State::Child;
    return ContainerDynamicCursorWriter(
        protocol_, containerType, size, valueType, keyTypeForMap);
  }
  void endWrite(ContainerDynamicCursorWriter&& writer) {
    checkState(State::Child);
    writer.finalize();
    afterWriteField();
    state_ = State::Active;
  }
  void abandonWrite(ContainerDynamicCursorWriter&& writer) {
    checkState(State::Child);
    writer.abandon();
    state_ = State::Abandoned;
  }

 private:
  explicit StructuredDynamicCursorWriter(ProtocolWriter* p) : Base(p) {
    protocol_->writeStructBegin(nullptr);
  }

  void beforeWriteField(int16_t fieldId, protocol::TType type) {
    checkState(State::Active);
    protocol_->writeFieldBegin(nullptr, type, fieldId);
  }
  void afterWriteField() { protocol_->writeFieldEnd(); }

  void finalize() {
    checkState(State::Active);
    protocol_->writeFieldStop();
    protocol_->writeStructEnd();
    state_ = State::Done;
  }

  friend ContainerDynamicCursorWriter;
  template <typename Reader, typename Writer>
  friend class DynamicCursorSerializationWrapper;
};

template <typename ProtocolWriter>
class ContainerDynamicCursorWriter : detail::BaseCursorWriter<ProtocolWriter> {
  using Base = detail::BaseCursorWriter<ProtocolWriter>;
  using State = typename Base::State;
  using Base::checkState;
  using Base::protocol_;
  using Base::state_;
  template <typename Tag>
  using view_type = detail::lift_view_t<type::native_type<Tag>, true>;
  using StructuredDynamicCursorWriter =
      StructuredDynamicCursorWriter<ProtocolWriter>;
  template <typename Tag>
  static constexpr bool vectorize =
      ProtocolWriter::kSupportsArithmeticVectors() &&
      !std::is_same_v<view_type<Tag>, bool> &&
      std::is_arithmetic_v<view_type<Tag>>;

  uint32_t remaining_;
  std::optional<protocol::TType> keyType_; // maps only
  protocol::TType valueType_; // includes sets
  protocol::TType containerType_;
  bool partialWrite_ = false; // key has been written but not the value.

 public:
  // For maps, the caller is responsible for alternating between reading keys
  // and values.
  template <typename Tag>
  void write(Tag, const view_type<Tag>& value) {
    checkRemaining();
    ensureTypesCompatible<Tag>(nextTType());

    op::encode<Tag>(*protocol_, value);
    advance();
  }

  template <typename Tag>
  void writeChunk(Tag, std::span<const view_type<Tag>> value) {
    checkRemaining(value.size());
    ensureTypesCompatible<Tag>(nextTType());
    if (keyType_) {
      folly::throw_exception<std::runtime_error>(
          "writeChunk doesn't support maps");
    }
    if (vectorize<Tag>) {
      using T = view_type<Tag>;
      protocol_->template writeArithmeticVector<T>(value.data(), value.size());
    } else {
      for (const auto& v : value) {
        op::encode<Tag>(*protocol_, v);
      }
    }
    remaining_ -= value.size();
  }

  void writeValue(const protocol::Value& value) {
    checkRemaining();
    apache::thrift::protocol::detail::serializeValue(*protocol_, value);
    advance();
  }

  void writeRaw(const folly::IOBuf& value) {
    checkRemaining();
    protocol_->writeRaw(value);
    advance();
  }

  StructuredDynamicCursorWriter beginWriteStructured() {
    checkRemaining();
    state_ = State::Child;
    return StructuredDynamicCursorWriter(protocol_);
  }
  void endWrite(StructuredDynamicCursorWriter&& writer) {
    checkState(State::Child);
    writer.finalize();
    advance();
    state_ = State::Active;
  }
  void abandonWrite(StructuredDynamicCursorWriter&& writer) {
    checkState(State::Child);
    writer.abandon();
    state_ = State::Abandoned;
  }

  template <typename Tag>
  ContainerDynamicCursorWriter beginWriteContainer(Tag, uint32_t size) {
    std::optional<protocol::TType> keyTypeForMap;
    if constexpr (type::is_a_v<Tag, type::map_c>) {
      keyTypeForMap = op::typeTagToTType<typename Tag::key_tag>;
    }
    return beginWriteContainer(
        op::typeTagToTType<Tag>,
        size,
        op::typeTagToTType<typename Tag::value_tag>,
        keyTypeForMap);
  }
  ContainerDynamicCursorWriter beginWriteContainer(
      protocol::TType containerType,
      uint32_t size,
      protocol::TType valueType,
      std::optional<protocol::TType> keyTypeForMap = {}) {
    checkRemaining();
    state_ = State::Child;
    return ContainerDynamicCursorWriter(
        protocol_, containerType, size, valueType, keyTypeForMap);
  }
  void endWrite(ContainerDynamicCursorWriter&& writer) {
    checkState(State::Child);
    writer.finalize();
    advance();
    state_ = State::Active;
  }
  void abandonWrite(ContainerDynamicCursorWriter&& writer) {
    checkState(State::Child);
    writer.abandon();
    state_ = State::Abandoned;
  }

 private:
  void checkRemaining(uint32_t n = 1) {
    checkState(State::Active);
    if (remaining_ < n && !partialWrite_) {
      folly::throw_exception<std::out_of_range>("Already wrote all elements");
    }
  }
  void advance() {
    if (keyType_) {
      if (partialWrite_) {
        remaining_--;
        partialWrite_ = false;
      } else {
        partialWrite_ = true;
      }
    } else {
      remaining_--;
    }
  }
  protocol::TType nextTType() {
    if (keyType_ && !partialWrite_) {
      return *keyType_;
    }
    return valueType_;
  }

  ContainerDynamicCursorWriter(
      ProtocolWriter* p,
      protocol::TType containerType,
      uint32_t size,
      protocol::TType valueType,
      std::optional<protocol::TType> keyType)
      : Base(p),
        remaining_(size),
        keyType_(keyType),
        valueType_(valueType),
        containerType_(containerType) {
    if (containerType == protocol::TType::T_MAP && keyType_) {
      protocol_->writeMapBegin(*keyType_, valueType_, remaining_);
    } else if (containerType == protocol::TType::T_SET && !keyType_) {
      protocol_->writeSetBegin(valueType_, remaining_);
    } else if (containerType == protocol::TType::T_LIST && !keyType_) {
      protocol_->writeListBegin(valueType_, remaining_);
    } else {
      folly::throw_exception<std::runtime_error>("Unexpected container type");
    }
  }

  void finalize() {
    checkState(State::Active);
    if (remaining_ || partialWrite_) {
      folly::throw_exception<std::runtime_error>("Not all elements written");
    }
    if (containerType_ == protocol::TType::T_MAP) {
      protocol_->writeMapEnd();
    } else if (containerType_ == protocol::TType::T_SET) {
      protocol_->writeSetEnd();
    } else if (containerType_ == protocol::TType::T_LIST) {
      protocol_->writeListEnd();
    }
    state_ = State::Done;
  }

  friend StructuredDynamicCursorWriter;
};

template <typename ProtocolReader, typename ProtocolWriter>
class DynamicCursorSerializationWrapper {
  using Serializer = Serializer<ProtocolReader, ProtocolWriter>;

  static_assert(
      !ProtocolReader::kUsesFieldNames() &&
          !ProtocolWriter::ProtocolReader::kUsesFieldNames(),
      "Only field id-based protocols are supported");

 public:
  DynamicCursorSerializationWrapper() = default;

  explicit DynamicCursorSerializationWrapper(
      std::unique_ptr<folly::IOBuf> serialized)
      : serializedData_(std::move(serialized)) {}

  ~DynamicCursorSerializationWrapper() {
    DCHECK(!isActive()) << "Destroying wrapper with active read or write";
  }

  // Moving wrapper during reads/writes will throw.
  DynamicCursorSerializationWrapper(
      DynamicCursorSerializationWrapper&& other) noexcept(false) {
    other.checkInactive("Moving wrapper during reads/writes not supported");
    serializedData_ = std::move(other.serializedData_);
  }
  DynamicCursorSerializationWrapper& operator=(
      DynamicCursorSerializationWrapper&& other) noexcept(false) {
    checkInactive("Moving wrapper during reads/writes not supported");
    other.checkInactive("Moving wrapper during reads/writes not supported");
    serializedData_ = std::move(other.serializedData_);
    return *this;
  }

  DynamicCursorSerializationWrapper(const DynamicCursorSerializationWrapper&) =
      delete;
  DynamicCursorSerializationWrapper& operator=(
      const DynamicCursorSerializationWrapper&) = delete;

  /**
   * Object write path (traditional Thrift serialization)
   * Serializes from a Thrift object.
   */
  template <typename T>
  /* implicit */ DynamicCursorSerializationWrapper(
      const T& t, ExternalBufferSharing sharing = COPY_EXTERNAL_BUFFER) {
    CursorWriteOpts opts{.sharing = sharing};
    t.write(writer(opts));
    serializedData_ = queue_.move();
    done();
  }

  /**
   * Object read path (traditional Thrift deserialization)
   * Deserializes into a (returned) Thrift object.
   */
  template <typename T>
  T deserialize() const {
    if (std::holds_alternative<ProtocolWriter>(protocol_)) {
      folly::throw_exception<std::runtime_error>(
          "Concurrent reads/writes not supported");
    }
    checkHasData();
    return Serializer::template deserialize<T>(serializedData_.get());
  }

  /**
   * Cursor read path
   * Template parameter determines whether chained buffers are supported.
   * Setting to false allows chained buffers and disables string_view API.
   * See thrift/doc/features/serialization/cursor.md for information about how
   * to get contiguous buffers cheaply from the socket.
   */
  template <bool Contiguous = false>
  StructuredDynamicCursorReader<ProtocolReader, Contiguous> beginRead() {
    checkHasData();
    if (Contiguous && serializedData_->isChained()) {
      folly::throw_exception<std::runtime_error>(
          "Chained buffer passed to contiguous reader.");
    }
    return StructuredDynamicCursorReader<ProtocolReader, Contiguous>(reader());
  }
  template <bool Contiguous>
  void endRead(
      StructuredDynamicCursorReader<ProtocolReader, Contiguous>&& reader) {
    reader.finalize();
    done();
  }

  /**
   * A faster version of endRead for when the caller doesn't intend to read
   * any more. Once a child reader has been abandoned, abandonRead is the only
   * method that can be called on any of its parents.
   */
  template <bool Contiguous>
  void abandonRead(
      StructuredDynamicCursorReader<ProtocolReader, Contiguous>&& reader) {
    reader.abandon();
    done();
  }

  /** Cursor write path */
  StructuredDynamicCursorWriter<ProtocolWriter> beginWriteWithOpts(
      const CursorWriteOpts& opts) {
    serializedData_.reset(); // Prevent concurrent read from seeing wrong data.
    return StructuredDynamicCursorWriter<ProtocolWriter>(writer(opts));
  }

  StructuredDynamicCursorWriter<ProtocolWriter> beginWrite() {
    return beginWriteWithOpts(CursorWriteOpts{});
  }

  void endWrite(StructuredDynamicCursorWriter<ProtocolWriter>&& writer) {
    writer.finalize();
    serializedData_ = queue_.move();
    done();
  }

  /**
   * Allows writing to be aborted. This is useful when you have an error
   * condition and you want to avoid writing to the buffer. Prevents
   * DynamicCursorSerializationWrapper dtor from throwing if the write was not
   * completed.
   */
  void abandonWrite(StructuredDynamicCursorWriter<ProtocolWriter>&& writer) {
    writer.abandon();
    queue_.reset();
    done();
  }

  /** Access to serialized data */
  const folly::IOBuf& serializedData() const& {
    checkHasData();
    return *serializedData_;
  }
  std::unique_ptr<folly::IOBuf> serializedData() && {
    checkHasData();
    return std::move(serializedData_);
  }

 private:
  ProtocolReader* reader() {
    checkInactive("Concurrent reads/writes not supported");
    auto& reader = protocol_.template emplace<ProtocolReader>();
    folly::io::Cursor cursor(serializedData_.get());
    reader.setInput(cursor);
    return &reader;
  }

  ProtocolWriter* writer(const CursorWriteOpts& opts) {
    checkInactive("Concurrent reads/writes not supported");
    auto& writer = protocol_.template emplace<ProtocolWriter>(opts.sharing);
    if (opts.padBuffer) {
      queue_.preallocate(opts.minGrowth, opts.minGrowth);
      queue_.trimStart(opts.padSize);
    }
    writer.setOutput(
        folly::io::QueueAppender{&queue_, opts.minGrowth, opts.maxGrowth});

    return &writer;
  }

  void done() { protocol_.template emplace<std::monostate>(); }

  bool isActive() const {
    return !std::holds_alternative<std::monostate>(protocol_);
  }
  void checkInactive(const char* message) const {
    if (isActive()) {
      folly::throw_exception<std::runtime_error>(message);
    }
  }
  void checkHasData() const {
    if (!serializedData_) {
      folly::throw_exception<std::runtime_error>("Reading from empty data");
    }
  }

  std::unique_ptr<folly::IOBuf> serializedData_;
  folly::IOBufQueue queue_;
  std::variant<std::monostate, ProtocolReader, ProtocolWriter> protocol_;
};

} // namespace apache::thrift::detail
