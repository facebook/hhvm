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
#include <variant>
#include <folly/io/Cursor.h>
#include <folly/io/IOBufQueue.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
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
 * endRead() may be replaced with abandonRead() if the caller doesn't intend to
 * read any more data from the cursor. This is a faster operation than calling
 * endRead() as it does not skip to the position of the next valid read.
 * Once a child reader has been abandoned, abandonRead is the only method that
 * can be called on its parents.
 *
 * test/CursorBasedSerializerTest.cpp has several complete examples of usage.
 */

namespace apache::thrift {

struct CursorWriteOpts {
  size_t minGrowth = (1 << 14) - 16;
  size_t maxGrowth = (1 << 14) - 16;
  bool padBuffer = false;
  size_t padSize = 128;
  ExternalBufferSharing sharing = ExternalBufferSharing::SHARE_EXTERNAL_BUFFER;
};

/**
 * Manages the lifetime of a Thrift object being used with cursor
 * de/serialization.
 * Must outlive the reader/writer objects it returns.
 */
template <
    typename T,
    typename ProtocolReader = BinaryProtocolReader,
    typename ProtocolWriter = BinaryProtocolWriter>
class CursorSerializationWrapper {
  using Tag = type::infer_tag<T>;
  static_assert(
      type::is_a_v<Tag, type::structured_c>, "T must be a thrift class");
  // NOTE: We will expand Reader and Writer so Compact can be used in following
  // diffs.
  static_assert(
      std::is_same_v<ProtocolReader, BinaryProtocolReader>,
      "ProtocolReader must be BinaryProtocolReader");
  static_assert(
      std::is_same_v<ProtocolWriter, BinaryProtocolWriter>,
      "ProtocolWriter must be BinaryProtocolWriter");
  using Serializer = Serializer<ProtocolReader, ProtocolWriter>;

 public:
  CursorSerializationWrapper() = default;

  explicit CursorSerializationWrapper(std::unique_ptr<folly::IOBuf> serialized)
      : serializedData_(std::move(serialized)) {}

  ~CursorSerializationWrapper() {
    DCHECK(!isActive()) << "Destroying wrapper with active read or write";
  }
  // Moving wrapper during reads/writes will throw.
  CursorSerializationWrapper(CursorSerializationWrapper&& other) noexcept(
      false) {
    other.checkInactive("Moving wrapper during reads/writes not supported");
    serializedData_ = std::move(other.serializedData_);
  }
  CursorSerializationWrapper& operator=(
      CursorSerializationWrapper&& other) noexcept(false) {
    checkInactive("Moving wrapper during reads/writes not supported");
    other.checkInactive("Moving wrapper during reads/writes not supported");
    serializedData_ = std::move(other.serializedData_);
    return *this;
  }

  /**
   * Object write path (traditional Thrift serialization)
   * Serializes from a Thrift object.
   */
  /* implicit */ CursorSerializationWrapper(
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
  StructuredCursorReader<Tag, ProtocolReader, Contiguous> beginRead() {
    checkHasData();
    if (Contiguous && serializedData_->isChained()) {
      folly::throw_exception<std::runtime_error>(
          "Chained buffer passed to contiguous reader.");
    }
    return StructuredCursorReader<Tag, ProtocolReader, Contiguous>(reader());
  }
  template <bool Contiguous>
  void endRead(
      StructuredCursorReader<Tag, ProtocolReader, Contiguous>&& reader) {
    reader.finalize();
    done();
  }

  /**
   * A faster version of endRead for when the caller doesn't intend to read any
   * more. Once a child reader has been abandoned, abandonRead is the only
   * method that can be called on any of its parents.
   */
  template <bool Contiguous>
  void abandonRead(
      StructuredCursorReader<Tag, ProtocolReader, Contiguous>&& reader) {
    reader.abandon();
    done();
  }

  /** Cursor write path */
  StructuredCursorWriter<Tag, BinaryProtocolWriter> beginWriteWithOpts(
      const CursorWriteOpts& opts) {
    serializedData_.reset(); // Prevent concurrent read from seeing wrong data.
    return StructuredCursorWriter<Tag, BinaryProtocolWriter>(writer(opts));
  }

  StructuredCursorWriter<Tag, BinaryProtocolWriter> beginWrite() {
    return beginWriteWithOpts(CursorWriteOpts{});
  }

  void endWrite(StructuredCursorWriter<Tag, BinaryProtocolWriter>&& writer) {
    writer.finalize();
    serializedData_ = queue_.move();
    done();
  }

  /**
   * Allows writing to be aborted. This is useful when you have an error
   * condition and you want to avoid writing to the buffer. Prevents
   * CursorSerializationWrapper dtor from throwing if the write was not
   * completed.
   */
  void abandonWrite(
      StructuredCursorWriter<Tag, BinaryProtocolWriter>&& writer) {
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

/**
 * Cursor deserializer for Thrift structs and unions.
 * Typically constructed from a CursorSerializationWrapper.
 */
template <
    typename Tag,
    typename ProtocolReader = BinaryProtocolReader,
    bool Contiguous = false>
class StructuredCursorReader : detail::BaseCursorReader<ProtocolReader> {
  using Base = detail::BaseCursorReader<ProtocolReader>;
  using State = typename Base::State;
  using Base::checkState;
  using Base::protocol_;
  using Base::state_;

  static_assert(
      type::is_a_v<Tag, type::structured_c>, "T must be a thrift class");
  using T = type::native_type<Tag>;
  static_assert(detail::validateCppTypes<T>());

  template <typename Ident>
  using type_tag = op::get_type_tag<T, Ident>;
  template <typename Ident>
  using native_type = op::get_native_type<T, Ident>;
  template <typename Ident>
  using view_type = detail::lift_view_t<native_type<Ident>, Contiguous>;

  template <typename TypeClass, typename Ident>
  using enable_for =
      typename std::enable_if_t<type::is_a_v<type_tag<Ident>, TypeClass>, int>;

  template <typename Ident>
  using enable_string_view = typename std::enable_if_t<
      type::is_a_v<type_tag<Ident>, type::string_c> && Contiguous,
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
   * If Contiguous = true, std::string is turned into std::string_view.
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
  maybe_optional<view_type<Ident>, Ident> read() {
    maybe_optional<view_type<Ident>, Ident> value;
    readField<Ident>(
        [&] {
          detail::decodeTo<type_tag<Ident>, ProtocolReader>(
              *protocol_, detail::maybe_emplace(value));
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
        [&] { op::decode<type_tag<Ident>>(*protocol_, value); }, value);
  }

  /** string/binary */

  template <typename Ident, enable_string_view<Ident> = 0>
  [[nodiscard]] bool_if_optional<Ident> read(std::string_view& value) {
    return readField<Ident>(
        [&] { value = detail::readStringView<ProtocolReader>(*protocol_); },
        value);
  }

  template <typename Ident, enable_for<type::string_c, Ident> = 0>
  [[nodiscard]] bool_if_optional<Ident> read(std::string& value) {
    return readField<Ident>([&] { protocol_->readString(value); }, value);
  }

  template <typename Ident, enable_for<type::string_c, Ident> = 0>
  [[nodiscard]] bool_if_optional<Ident> read(folly::IOBuf& value) {
    return readField<Ident>([&] { protocol_->readBinary(value); }, value);
  }

  /** containers
   *
   * See the ContainerCursorReader docblock for example usage.
   *
   * Note: when beginRead returns a reader, that reader must be passed to
   * endRead before any other methods on this object can be called.
   */

  template <typename Ident, enable_for<type::container_c, Ident> = 0>
  maybe_optional<
      ContainerCursorReader<type_tag<Ident>, ProtocolReader, Contiguous>,
      Ident>
  beginRead() {
    if (!beforeReadField<Ident>()) {
      return {};
    }
    state_ = State::Child;
    return ContainerCursorReader<type_tag<Ident>, ProtocolReader, Contiguous>{
        protocol_};
  }

  template <typename CTag>
  void endRead(
      ContainerCursorReader<CTag, ProtocolReader, Contiguous>&& child) {
    if (state_ != State::Child) {
      // This is a sentinel iterator for an empty container.
      DCHECK_EQ(child.remaining_, 0);
      return;
    }
    child.finalize();
    afterReadField();
    state_ = State::Active;
  }

  /**
   * A faster version of endRead for when the caller doesn't intend to read any
   * more. Once a child reader has been abandoned, abandonRead is the only
   * method that can be called on any of its parents.
   */
  template <typename CTag>
  void abandonRead(
      ContainerCursorReader<CTag, ProtocolReader, Contiguous>&& child) {
    child.abandon();
    state_ = State::Abandoned;
  }

  /** structured types
   *
   * Note: when beginRead returns a reader, that reader must be passed to
   * endRead before any other methods on this object can be called.
   */

  template <typename Ident, enable_for<type::structured_c, Ident> = 0>
  maybe_optional<
      StructuredCursorReader<type_tag<Ident>, ProtocolReader, Contiguous>,
      Ident>
  beginRead() {
    if (!beforeReadField<Ident>()) {
      return {};
    }
    state_ = State::Child;
    return StructuredCursorReader<type_tag<Ident>, ProtocolReader, Contiguous>{
        protocol_};
  }

  template <typename CTag>
  void endRead(
      StructuredCursorReader<CTag, ProtocolReader, Contiguous>&& child) {
    checkState(State::Child);
    child.finalize();
    afterReadField();
    state_ = State::Active;
  }

  /**
   * A faster version of endRead for when the caller doesn't intend to read any
   * more. Once a child reader has been abandoned, abandonRead is the only
   * method that can be called on any of its parents.
   */
  template <typename CTag>
  void abandonRead(
      StructuredCursorReader<CTag, ProtocolReader, Contiguous>&& child) {
    checkState(State::Child);
    child.abandon();
    state_ = State::Abandoned;
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
  explicit StructuredCursorReader(ProtocolReader* p)
      : detail::BaseCursorReader<ProtocolReader>(p) {
    readState_.readStructBegin(protocol_);
    readState_.readFieldBegin(protocol_);
  }
  StructuredCursorReader() { readState_.fieldType = TType::T_STOP; }

  void finalize() {
    checkState(State::Active);
    if (readState_.fieldType != TType::T_STOP) {
      protocol_->skip(readState_.fieldType);
      readState_.readFieldEnd(protocol_);
      // Because there's no struct begin marker in Binary protocol this skips
      // the rest of the struct.
      protocol_->skip(TType::T_STRUCT);
    }
    readState_.readStructEnd(protocol_);
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
      readState_.readFieldEnd(protocol_);
      apache::thrift::skip(*protocol_, readState_.fieldType);
      int16_t lastRead = readState_.fieldId;
      readState_.readFieldBegin(protocol_);
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
    readState_.readFieldEnd(protocol_);
    readState_.readFieldBegin(protocol_);
  }

  template <typename Ident, typename F, typename U>
  bool_if_optional<Ident> readField(F&& f, U& val) {
    bool read = beforeReadField<Ident>();

    if (read) {
      f();
      afterReadField();
    } else if constexpr (type::is_terse_field_v<T, Ident>) {
      val = copy(*op::get<Ident>(op::getIntrinsicDefault<T>()));
    } else if constexpr (!type::is_optional_or_union_field_v<T, Ident>) {
      val = copy(*op::get<Ident>(op::getDefault<T>()));
    }

    if constexpr (type::is_optional_or_union_field_v<T, Ident>) {
      return read;
    }
  }

 private:
  // Last field id the caller tried to read.
  FieldId fieldId_{0};
  // Contains last field id read from the buffer.
  typename ProtocolReader::StructReadState readState_{};

  // For copyable types, defer the copy until the assignment/return
  template <std::copy_constructible T>
  const T& copy(const T& in) {
    return in;
  }
  std::unique_ptr<folly::IOBuf> copy(const std::unique_ptr<folly::IOBuf>& in) {
    return in->clone();
  }

  template <typename, typename, bool>
  friend class StructuredCursorReader;
  template <typename, typename, bool>
  friend class ContainerCursorReader;
  friend class CursorSerializationWrapper<T>;
};

/**
 * Allows iterating over containers (set, list) without materializing them.
 * Behaves like a standard collection, ex:
 *  StructuredCursorReader<Struct> reader;
 *  auto listReader = reader.beginRead<ident::unqualified_list>();
 *  for (auto& val : listReader) { use(val); }
 *  reader.endRead(std::move(listReader));
 *
 *  Note: ContainerCursorReader does not support map types.
 */
template <typename Tag, typename ProtocolReader, bool Contiguous>
class ContainerCursorReader : detail::BaseCursorReader<ProtocolReader> {
  using Base = detail::BaseCursorReader<ProtocolReader>;
  using State = typename Base::State;
  using Base::checkState;
  using Base::protocol_;
  using Base::state_;

  using ElementType = detail::lift_view_t<
      typename detail::ContainerTraits<Tag>::ElementType,
      Contiguous>;
  using ElementTag = typename detail::ContainerTraits<Tag>::ElementTag;
  template <typename CTag, typename OwnTag>
  using enable_cursor_for = std::enable_if_t<
      (type::is_a_v<OwnTag, type::list_c> ||
       type::is_a_v<OwnTag, type::set_c>) &&
          type::is_a_v<ElementTag, CTag>,
      int>;

 public:
  /**
   * Iterator read path
   *
   * When deserializing container elements into their normal C++ types is
   * acceptable, this provides the most conventient way to consume a container.
   * It permits use of range-based for loops and STL methods / constructors
   * taking two iterators, as in the examples in this class' docblock.
   */
  ContainerCursorIterator<Tag, ProtocolReader, Contiguous> begin() {
    checkState(State::Active);
    if (remaining_ == 0) {
      return {};
    }
    if (!lastRead_) {
      lastRead_.emplace();
      readItem();
    }
    return ContainerCursorIterator<Tag, ProtocolReader, Contiguous>{*this};
  }
  ContainerCursorIterator<Tag, ProtocolReader, Contiguous> end() const {
    return {};
  }

  // Returns remaining size if reading has begun.
  int32_t remaining() const {
    checkState(State::Active);
    return remaining_;
  }
  [[deprecated("Call remaining() instead")]] int32_t size() const {
    return remaining();
  }

  /**
   * Manual read path for nested containers.
   *
   * Allows iterating nested containers without materializing them.
   * Caller is responsible for checking `remaining() > 0` before calling
   * beginRead, which throws if no elements remain.
   * Only lists and sets are supported.
   *
   * Note: the reader returned from beginRead must be passed to
   * endRead before any other methods on this object can be called.
   *
   * Ex:
   *  StructuredCursorReader<Struct> reader;
   *  auto outerReader = reader.beginRead<ident::list_of_list>();
   *  for (size_t n = outerReader.remaining(); n > 0; --n) {
   *    auto innerReader = outerReader.beginRead();
   *    for (auto& element : innerReader) {
   *      use(element);
   *    }
   *    outerReader.endRead(std::move(innerReader));
   *  }
   *  reader.endRead(std::move(outerReader));
   */

  template <
      typename...,
      typename U = Tag,
      enable_cursor_for<type::container_c, U> = 0>
  ContainerCursorReader<ElementTag, ProtocolReader, Contiguous> beginRead() {
    checkState(State::Active);
    if (!remaining_) {
      folly::throw_exception<std::out_of_range>("No elements remaining");
    }
    DCHECK(!lastRead_)
        << "Can't mix manual and iterator APIs on same container.";
    state_ = State::Child;
    return ContainerCursorReader<ElementTag, ProtocolReader, Contiguous>{
        protocol_};
  }

  template <typename CTag>
  void endRead(
      ContainerCursorReader<CTag, ProtocolReader, Contiguous>&& child) {
    checkState(State::Child);
    child.finalize();
    state_ = State::Active;
    --remaining_;
  }

  /**
   * A faster version of endRead for when the caller doesn't intend to read any
   * more. Once a child reader has been abandoned, abandonRead is the only
   * method that can be called on any of its parents.
   */
  template <typename CTag>
  void abandonRead(
      ContainerCursorReader<CTag, ProtocolReader, Contiguous>&& child) {
    checkState(State::Child);
    child.abandon();
    state_ = State::Abandoned;
  }

  /**
   * Manual read path for containers of structured types.
   *
   * Allows using cursor serialization on contained elements.
   * Caller is responsible for checking `remaining() > 0` before calling
   * beginRead, which throws if no elements remain.
   * Only lists and sets are supported.
   *
   * Note: the reader returned from beginRead must be passed to
   * endRead before any other methods on this object can be called.
   *
   * Ex:
   *  StructuredCursorReader<Struct> reader;
   *  auto outerReader = reader.beginRead<ident::list_of_list>();
   *  for (size_t n = outerReader.remaining(); n > 0; --n) {
   *    auto innerReader = outerReader.beginRead();
   *    use(innerReader.read<ident::unqualified_int>());
   *    outerReader.endRead(std::move(innerReader));
   *  }
   *  reader.endRead(std::move(outerReader));
   */

  template <
      typename...,
      typename U = Tag,
      enable_cursor_for<type::structured_c, U> = 0>
  StructuredCursorReader<ElementTag, ProtocolReader, Contiguous> beginRead() {
    checkState(State::Active);
    if (!remaining_) {
      folly::throw_exception<std::out_of_range>("No elements remaining");
    }
    DCHECK(!lastRead_)
        << "Can't mix manual and iterator APIs on same container.";
    state_ = State::Child;
    return StructuredCursorReader<ElementTag, ProtocolReader, Contiguous>{
        protocol_};
  }

  template <typename CTag>
  void endRead(
      StructuredCursorReader<CTag, ProtocolReader, Contiguous>&& child) {
    checkState(State::Child);
    child.finalize();
    state_ = State::Active;
    --remaining_;
  }

  /**
   * A faster version of endRead for when the caller doesn't intend to read any
   * more. Once a child reader has been abandoned, abandonRead is the only
   * method that can be called on any of its parents.
   */
  template <typename CTag>
  void abandonRead(
      StructuredCursorReader<CTag, ProtocolReader, Contiguous>&& child) {
    checkState(State::Child);
    child.abandon();
    state_ = State::Abandoned;
  }

 private:
  explicit ContainerCursorReader(ProtocolReader* p);
  ContainerCursorReader() { remaining_ = 0; }

  void finalize();

  bool advance() {
    checkState(State::Active);
    DCHECK_GT(remaining_, 0);
    if (--remaining_ == 0) {
      return false;
    }
    readItem();
    return true;
  }

  ElementType& currentValue() {
    checkState(State::Active);
    DCHECK(lastRead_);
    return *lastRead_;
  }

  void readItem();

  // remaining_ includes the element cached in the reader as lastRead_.
  uint32_t remaining_;
  std::optional<ElementType> lastRead_;

  template <typename, typename, bool>
  friend class StructuredCursorReader;
  template <typename, typename, bool>
  friend class ContainerCursorReader;
  friend class ContainerCursorIterator<Tag, ProtocolReader, Contiguous>;
};

/**
 * Allows iterating over containers without materializing them.
 * This is an InputIterator, which means it must be iterated in a single pass.
 * Operator++ invalidates all other copies of the iterator.
 */
template <typename Tag, typename ProtocolReader, bool Contiguous>
class ContainerCursorIterator {
 public:
  using iterator_category = std::input_iterator_tag;
  using difference_type = ssize_t;
  // Same as the element type in regular generated code, or std::string_view if
  // that would be std::string and Contiguous = true.
  using value_type = detail::lift_view_t<
      typename detail::ContainerTraits<Tag>::ElementType,
      Contiguous>;
  // These are non-const to allow moving the deserialized value out.
  // Modifying them does not affect the underlying buffer.
  using pointer = value_type*;
  using reference = value_type&;

  explicit ContainerCursorIterator(
      ContainerCursorReader<Tag, ProtocolReader, Contiguous>& in)
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
  // These return non-const to allow moving the deserialized value out.
  // Modifying them does not affect the underlying buffer.
  reference operator*() {
    FOLLY_SAFE_DCHECK(reader_);
    return reader_->currentValue();
  }
  pointer operator->() {
    FOLLY_SAFE_DCHECK(reader_);
    return &reader_->currentValue();
  }

  // Equality is only defined against itself and the end iterator
  bool operator==(ContainerCursorIterator other) const {
    return reader_ == other.reader_;
  }

 private:
  ContainerCursorReader<Tag, ProtocolReader, Contiguous>* reader_ = nullptr;
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
class StringCursorWriter
    : detail::DelayedSizeCursorWriter<BinaryProtocolWriter> {
 public:
  uint8_t* writeableData() {
    checkState(State::Active);
    return data_;
  }

 private:
  StringCursorWriter(BinaryProtocolWriter* p, int32_t maxSize)
      : DelayedSizeCursorWriter(p) {
    writeSize();
    data_ = protocol_->ensure(maxSize);
  }

  void finalize(int32_t actualSize) {
    DelayedSizeCursorWriter::finalize(actualSize);
    protocol_->advance(actualSize);
  }

  uint8_t* data_;

  template <typename T, typename PW>
  friend class StructuredCursorWriter;
};

/**
 * Cursor serializer for Thrift structs and unions.
 * Typically constructed from a CursorSerializationWrapper.
 */
template <typename Tag, typename ProtocolWriter = BinaryProtocolWriter>
class StructuredCursorWriter : detail::BaseCursorWriter<ProtocolWriter> {
  using Base = detail::BaseCursorWriter<ProtocolWriter>;
  using Base::protocol_;
  using Base::state_;
  using State = typename Base::State;
  using Base::checkState;

  static_assert(
      type::is_a_v<Tag, type::structured_c>, "T must be a thrift class");
  using T = type::native_type<Tag>;
  static_assert(detail::validateCppTypes<T>());

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
        [&] { op::encode<type_tag<Ident>>(*protocol_, value); }, value);
  }

  /** string/binary */

  template <typename Ident, enable_for<type::string_c, Ident> = 0>
  void write(const folly::IOBuf& value) {
    writeField<Ident>([&] { protocol_->writeBinary(value); }, value);
  }

  template <typename Ident, enable_for<type::string_c, Ident> = 0>
  void write(std::string_view value) {
    writeField<Ident>([&] { protocol_->writeBinary(value); }, value);
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
    StringCursorWriter child{protocol_, maxSize};
    return child;
  }

  void endWrite(StringCursorWriter&& child, int32_t actualSize) {
    checkState(State::Child);
    child.finalize(actualSize);
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
        [&] { op::encode<type_tag<Ident>>(*protocol_, value); }, value);
  }

  /**
   * Allows writing containers whose size isn't known until afterwards.
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
    return ContainerCursorWriter<type_tag<Ident>>{protocol_};
  }

  template <typename CTag>
  void endWrite(ContainerCursorWriter<CTag>&& child) {
    checkState(State::Child);
    child.finalize();
    afterWriteField();
    state_ = State::Active;
  }

  template <typename CTag>
  void abandonWrite(ContainerCursorWriter<CTag>&& child) {
    checkState(State::Child);
    child.abandon();
    state_ = State::Abandoned;
  }

  /** structured types
   *
   * Note: none of this writer's other methods may be called between
   * beginWrite() and the corresponding endWrite().
   */

  template <typename Ident, enable_for<type::structured_c, Ident> = 0>
  StructuredCursorWriter<type_tag<Ident>> beginWrite() {
    beforeWriteField<Ident>();
    state_ = State::Child;
    return StructuredCursorWriter<type_tag<Ident>>{protocol_};
  }

  template <typename CTag>
  void endWrite(StructuredCursorWriter<CTag>&& child) {
    checkState(State::Child);
    child.finalize();
    afterWriteField();
    state_ = State::Active;
  }

  template <typename CTag>
  void abandonWrite(StructuredCursorWriter<CTag>&& child) {
    checkState(State::Child);
    child.abandon();
    state_ = State::Abandoned;
  }

  template <typename Ident, enable_for<type::structured_c, Ident> = 0>
  void write(const native_type<Ident>& value) {
    writeField<Ident>(
        [&] { op::encode<type_tag<Ident>>(*protocol_, value); }, value);
  }

  /** This is a niche API to support writing the fields of a struct temporally
   * out of order in cases where both fields need to be written using CurSe, the
   * computation of the second field that produces its serialized value must
   * happen before the computation of the first field can proceed, and
   * reordering the fields in the struct is not feasible. In other cases,
   * migrating one of the fields to change the order in the struct or performing
   * the computation in field order is preferable due to the added cost and
   * complexity of using this API. */
  template <typename Ident, enable_for<type::structured_c, Ident> = 0>
  void writeSerialized(
      CursorSerializationWrapper<native_type<Ident>>&& cursorValue) {
    beforeWriteField<Ident>();
    // Will fail if the cursor value is not finalized
    protocol_->writeRaw(cursorValue.serializedData());
    afterWriteField();
  }

 private:
  explicit StructuredCursorWriter(ProtocolWriter* p) : Base(p) {
    protocol_->writeStructBegin(nullptr);
  }

  void finalize() {
    checkState(State::Active);
    visitSkippedFields<FieldAfterLast<T>>();
    protocol_->writeFieldStop();
    protocol_->writeStructEnd();
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
    if (!is_thrift_union_v<T> && field_id::value <= fieldId_) {
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
    protocol_->writeFieldBegin(
        nullptr,
        op::typeTagToTType<type_tag<Ident>>,
        folly::to_underlying(field_id::value));
  }

  void afterWriteField() { protocol_->writeFieldEnd(); }

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
  FOLLY_ALWAYS_INLINE void visitSkippedFields() {
    if constexpr (is_thrift_union_v<T>) {
      return;
    }

    // Fast path: consecutive fields
    if (detail::increment(fieldId_) == MaxFieldId) {
      return;
    }

    visitSkippedFieldsSlowPath<MaxFieldId>();
  }

  template <FieldId MaxFieldId>
  void visitSkippedFieldsSlowPath() {
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

  template <typename, typename>
  friend class StructuredCursorWriter;
  template <typename>
  friend class ContainerCursorWriter;
  friend class CursorSerializationWrapper<T>;
  friend struct detail::DefaultValueWriter<Tag>;
};

/**
 * Allows writing containers (list, set) whose size is not known in advance.
 * Ex:
 *  StructuredCursorWriter writer;
 *  auto child = writer.beginWrite<ident::list_field>();
 *  folly::coro::AsyncGenerator<int32_t> gen = ...;
 *  while (auto val = co_await gen.next()) {
 *    child.write(*val);
 *  }
 *  writer.endWrite(std::move(child));
 * Note: ContainerCursorWriter does not support map type.
 */
template <typename Tag>
class ContainerCursorWriter
    : detail::DelayedSizeCursorWriter<BinaryProtocolWriter> {
  using ElementType = typename detail::ContainerTraits<Tag>::ElementType;
  using ElementTag = typename detail::ContainerTraits<Tag>::ElementTag;
  template <typename CTag, typename OwnTag>
  using enable_cursor_for = std::enable_if_t<
      (type::is_a_v<OwnTag, type::list_c> ||
       type::is_a_v<OwnTag, type::set_c>) &&
          type::is_a_v<ElementTag, CTag>,
      int>;

 public:
  void write(const ElementType& val) {
    checkState(State::Active);
    ++n;
    detail::ContainerTraits<Tag>::write(*protocol_, val);
  }

  /**
   * Allows writing containers whose size isn't known until afterwards.
   * Less efficient than using write().
   * See the ContainerCursorWriter docblock for example usage.
   *
   * Note: none of this writer's other methods may be called between
   * beginWrite() and the corresponding endWrite().
   */
  template <
      typename...,
      typename U = Tag,
      enable_cursor_for<type::container_c, U> = 0>
  ContainerCursorWriter<ElementTag> beginWrite() {
    checkState(State::Active);
    state_ = State::Child;
    return ContainerCursorWriter<ElementTag>{protocol_};
  }

  template <typename CTag>
  void endWrite(ContainerCursorWriter<CTag>&& child) {
    checkState(State::Child);
    child.finalize();
    ++n;
    state_ = State::Active;
  }

  /**
   * structured types
   *
   * Note: none of this writer's other methods may be called between
   * beginWrite() and the corresponding endWrite().
   */
  template <
      typename...,
      typename U = Tag,
      enable_cursor_for<type::structured_c, U> = 0>
  StructuredCursorWriter<ElementTag> beginWrite() {
    checkState(State::Active);
    state_ = State::Child;
    return StructuredCursorWriter<ElementTag>{protocol_};
  }

  template <typename CTag>
  void endWrite(StructuredCursorWriter<CTag>&& child) {
    checkState(State::Child);
    child.finalize();
    ++n;
    state_ = State::Active;
  }

 private:
  explicit ContainerCursorWriter(BinaryProtocolWriter* p);

  template <typename, typename>
  friend class StructuredCursorWriter;
  template <typename>
  friend class ContainerCursorWriter;

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
        // The check below should be using protForDebug.getCursorPosition() + 1,
        // but due to a bug somewhere with streaming, an extra byte gets added
        // to the buffer, causing this check to fail.
        DCHECK_LE(
            buf->computeChainDataLength(), protForDebug.getCursorPosition() + 2)
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
ContainerCursorWriter<Tag>::ContainerCursorWriter(BinaryProtocolWriter* p)
    : DelayedSizeCursorWriter(p) {
  if constexpr (type::is_a_v<Tag, type::list_c>) {
    protocol_->writeByte(
        op::typeTagToTType<typename detail::ContainerTraits<Tag>::ElementTag>);
  } else if constexpr (type::is_a_v<Tag, type::set_c>) {
    protocol_->writeByte(
        op::typeTagToTType<typename detail::ContainerTraits<Tag>::ElementTag>);
  } else if constexpr (type::is_a_v<Tag, type::map_c>) {
    protocol_->writeByte(
        op::typeTagToTType<typename detail::ContainerTraits<Tag>::KeyTag>);
    protocol_->writeByte(
        op::typeTagToTType<typename detail::ContainerTraits<Tag>::ValueTag>);
  } else {
    static_assert(!sizeof(Tag), "unexpected tag");
  }
  writeSize();
}

template <typename Tag, typename ProtocolReader, bool Contiguous>
ContainerCursorReader<Tag, ProtocolReader, Contiguous>::ContainerCursorReader(
    ProtocolReader* p)
    : detail::BaseCursorReader<ProtocolReader>(p) {
  // Check element types
  if constexpr (type::is_a_v<Tag, type::list_c>) {
    TType type;
    protocol_->readListBegin(type, remaining_);
    if (type !=
        op::typeTagToTType<typename detail::ContainerTraits<Tag>::ElementTag>) {
      folly::throw_exception<std::runtime_error>(
          "Unexpected element type in list");
    }
  } else if constexpr (type::is_a_v<Tag, type::set_c>) {
    TType type;
    protocol_->readSetBegin(type, remaining_);
    if (type !=
        op::typeTagToTType<typename detail::ContainerTraits<Tag>::ElementTag>) {
      folly::throw_exception<std::runtime_error>(
          "Unexpected element type in set");
    }
  } else if constexpr (type::is_a_v<Tag, type::map_c>) {
    TType key, value;
    protocol_->readMapBegin(key, value, remaining_);
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
}

template <typename Tag, typename ProtocolReader, bool Contiguous>
void ContainerCursorReader<Tag, ProtocolReader, Contiguous>::finalize() {
  checkState(State::Active);
  state_ = State::Done;

  if (remaining_ > 0) {
    if (!lastRead_) {
      lastRead_.emplace();
      readItem();
    }
    // Skip remaining unread elements
    skip_n(*protocol_, remaining_ - 1, detail::ContainerTraits<Tag>::wireTypes);
  }

  if constexpr (type::is_a_v<Tag, type::list_c>) {
    protocol_->readListEnd();
  } else if constexpr (type::is_a_v<Tag, type::set_c>) {
    protocol_->readSetEnd();
  } else if constexpr (type::is_a_v<Tag, type::map_c>) {
    protocol_->readMapEnd();
  } else {
    static_assert(!sizeof(Tag), "unexpected tag");
  }
}

template <typename Tag, typename ProtocolReader, bool Contiguous>
void ContainerCursorReader<Tag, ProtocolReader, Contiguous>::readItem() {
  DCHECK_GT(remaining_, 0);
  DCHECK(lastRead_);

  if constexpr (type::is_a_v<Tag, type::list_c>) {
    detail::decodeTo<
        typename detail::ContainerTraits<Tag>::ElementTag,
        ProtocolReader>(*protocol_, *lastRead_);
  } else if constexpr (type::is_a_v<Tag, type::set_c>) {
    detail::decodeTo<
        typename detail::ContainerTraits<Tag>::ElementTag,
        ProtocolReader>(*protocol_, *lastRead_);
  } else if constexpr (type::is_a_v<Tag, type::map_c>) {
    detail::
        decodeTo<typename detail::ContainerTraits<Tag>::KeyTag, ProtocolReader>(
            *protocol_, lastRead_->first);
    detail::decodeTo<
        typename detail::ContainerTraits<Tag>::ValueTag,
        ProtocolReader>(*protocol_, lastRead_->second);
  } else {
    static_assert(!sizeof(Tag), "unexpected tag");
  }
}

} // namespace apache::thrift
