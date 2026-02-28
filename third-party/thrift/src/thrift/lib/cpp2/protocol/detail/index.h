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

#include <cmath>

#include <folly/MapUtil.h>
#include <folly/Optional.h>
#include <folly/Random.h>
#include <folly/Range.h>
#include <folly/Traits.h>
#include <folly/container/F14Map.h>
#include <folly/io/Cursor.h>
#include <thrift/lib/cpp2/protocol/Cpp2Ops.h>
#include <thrift/lib/cpp2/protocol/LazyDeserializationFlags.h>
#include <thrift/lib/cpp2/protocol/Protocol.h>
#include <thrift/lib/cpp2/protocol/ProtocolReaderStructReadState.h>
#include <thrift/lib/cpp2/protocol/Traits.h>
#include <thrift/lib/cpp2/protocol/detail/ReservedId.h>

namespace apache::thrift {

class VirtualReaderBase;

namespace detail {
/*
 * When index is enabled, we will inject following fields to thrift struct
 *
 * __fbthrift_random_number:
 *   Randomly generated 64 bits integer to validate index field
 *
 * __fbthrift_index_offset:
 *   The size of all thrift fields except index
 *
 * __fbthrift_field_id_to_size:
 *   map<i16, i64> representing field id to field size
 *
 * During deserialization, we will use __fbthrift_index_offset to
 * extract __fbthrift_field_id_to_size before deserializing other fields.
 * Then use __fbthrift_field_id_to_size to skip fields efficiently.
 */

struct InternalField {
  const char* name;
  int16_t id;
  TType type;
};

// This is randomly generated 64 bit integer for each thrift struct
// serialization. The same random number will be added to index field to
// validate whether we are using the correct index field
constexpr auto kExpectedRandomNumberField = InternalField{
    "__fbthrift_random_number",
    static_cast<int16_t>(ReservedId::kExpectedRandomNumber),
    TType::T_I64};

// This is the field id (key) of random number in index field.
// The id is different from random number field id to avoid confusion
// since the value is not size of field, but random number in map
constexpr int16_t kActualRandomNumberFieldId =
    static_cast<int16_t>(ReservedId::kActualRandomNumber);

// The reason this field is T_DOUBLE is because we will backfill it after
// serializing the whole struct, though we will only use the integer part.
constexpr auto kSizeField = InternalField{
    "__fbthrift_index_offset",
    static_cast<int16_t>(ReservedId::kOffset),
    TType::T_DOUBLE};

constexpr auto kIndexField = InternalField{
    "__fbthrift_field_id_to_size",
    static_cast<int16_t>(ReservedId::kIndex),
    TType::T_MAP};

// If user modified lazy fields in serialized data directly without
// deserialization, we should not use index, otherwise we might read the data
// incorrectly.
//
// To validate data integrity, we added checksum of lazy fields in serialized
// data, and stored it as value in index map with kXxh3Checksum as key.
//
// Note this field is not an actual field in serialized data.
// It's only a sentinel to check whether checksum exists.
//
// The reason we don't use actual field to store checksum is because
// user might modify the serialized data and delete checksum field,
// in which case we might still use index even data is tampered
constexpr int16_t kXxh3ChecksumFieldId =
    static_cast<int16_t>(ReservedId::kXxh3Checksum);

class Xxh3Hasher {
 public:
  ~Xxh3Hasher();
  void update(folly::io::Cursor);
  void init();
  bool is_initialized() const { return state != nullptr; }
  operator int64_t();

 private:
  void* state = nullptr;
};

[[noreturn]] void throwChecksumMismatch(int64_t expected, int64_t actual);

struct DummyIndexWriter {
  DummyIndexWriter(void*, uint32_t&, bool) {}

  void recordFieldStart() {}

  template <typename Type, typename TypeClass>
  void recordFieldEnd(TypeClass, int16_t) {}

  void finalize() {}
};

// Utility class to inject index fields to serialized data
template <class Protocol>
class IndexWriterImpl {
 public:
  IndexWriterImpl(
      Protocol* prot, uint32_t& writtenBytes, bool writeValidationFields)
      : prot_(prot), writtenBytes_(writtenBytes) {
    if (writeValidationFields) {
      hasher_.init();
      writeRandomNumberField();
    }
    writeIndexOffsetField();
  }

  void recordFieldStart() { fieldStart_ = writtenBytes_; }

  template <typename Type, typename TypeClass>
  void recordFieldEnd(TypeClass, int16_t id) {
    if (!std::is_same<TypeClass, type_class::integral>{} &&
        !fixed_cost_skip_v<
            typename Protocol::ProtocolReader,
            TypeClass,
            folly::remove_cvref_t<Type>>) {
      const auto fieldSize = writtenBytes_ - fieldStart_;
      fieldIdAndSize_.push_back({id, fieldSize});
      if (hasher_.is_initialized()) {
        hasher_.update(prot_->tail(fieldSize));
      }
    }
  }

  void finalize() {
    if (hasher_.is_initialized()) {
      fieldIdAndSize_.push_back(
          {kXxh3ChecksumFieldId, static_cast<int64_t>(hasher_)});
    }
    prot_->rewriteDouble(
        writtenBytes_ - sizeFieldEnd_, writtenBytes_ - indexOffsetLocation_);
    writeIndexField();
  }

 private:
  struct FieldIndex {
    int16_t fieldId;
    int64_t fieldSize;
  };

  void writeIndexOffsetField() {
    writtenBytes_ +=
        prot_->writeFieldBegin(kSizeField.name, kSizeField.type, kSizeField.id);
    indexOffsetLocation_ = writtenBytes_;
    writtenBytes_ += prot_->writeDouble(0);
    writtenBytes_ += prot_->writeFieldEnd();
    sizeFieldEnd_ = writtenBytes_;
  }

  void writeRandomNumberField() {
    const int64_t randomNumber = static_cast<int64_t>(folly::Random::rand64());
    writtenBytes_ += prot_->writeFieldBegin(
        kExpectedRandomNumberField.name,
        kExpectedRandomNumberField.type,
        kExpectedRandomNumberField.id);
    writtenBytes_ += prot_->writeI64(randomNumber);
    writtenBytes_ += prot_->writeFieldEnd();
    fieldIdAndSize_.push_back({kActualRandomNumberFieldId, randomNumber});
  }

  void writeIndexField() {
    writtenBytes_ += prot_->writeFieldBegin(
        kIndexField.name, kIndexField.type, kIndexField.id);
    writtenBytes_ += prot_->writeMapBegin(
        TType::T_I16, TType::T_I64, fieldIdAndSize_.size());
    for (auto index : fieldIdAndSize_) {
      writtenBytes_ += prot_->writeI16(index.fieldId);
      writtenBytes_ += prot_->writeI64(index.fieldSize);
    }
    writtenBytes_ += prot_->writeMapEnd();
    writtenBytes_ += prot_->writeFieldEnd();
  }

  Protocol* prot_;
  uint32_t& writtenBytes_;
  uint32_t indexOffsetLocation_ = 0;
  uint32_t sizeFieldEnd_ = 0;
  uint32_t fieldStart_ = 0;
  std::vector<FieldIndex> fieldIdAndSize_;
  Xxh3Hasher hasher_;
};

template <class Protocol>
constexpr bool hasIndexSupport = [] {
  bool has = false;
  if constexpr (requires { Protocol::ProtocolWriter::kHasIndexSupport(); }) {
    has = Protocol::ProtocolWriter::kHasIndexSupport();
  } else if constexpr (requires { Protocol::kHasIndexSupport(); }) {
    has = Protocol::kHasIndexSupport();
  }
  return has && !std::is_base_of_v<VirtualReaderBase, Protocol>;
}();

template <class Protocol>
using IndexWriter = std::conditional_t<
    hasIndexSupport<Protocol>,
    IndexWriterImpl<Protocol>,
    DummyIndexWriter>;

template <class Protocol>
class ProtocolReaderStructReadStateWithIndexImpl
    : public ProtocolReaderStructReadState<Protocol> {
 private:
  using Base = ProtocolReaderStructReadState<Protocol>;

 public:
  void readStructBegin(Protocol* iprot) {
    readStructBeginWithIndex(iprot->getCursor());
    Base::readStructBegin(iprot);
  }

  template <class TypeClass, class Type>
  FOLLY_ALWAYS_INLINE folly::Optional<folly::IOBuf> tryFastSkip(
      Protocol* iprot, int16_t id, TType type) {
    if (isLazyDeserializationDisabled()) {
      return {};
    }

    if (fixed_cost_skip_v<Protocol, TypeClass, Type>) {
      return tryFastSkipImpl(iprot, [&] { iprot->skip(type); });
    }

    if (auto p = folly::get_ptr(fieldIdToSize_, id)) {
      return tryFastSkipImpl(iprot, [&] { iprot->skipBytes(*p); });
    }

    return {};
  }

  FOLLY_ALWAYS_INLINE bool advanceToNextField(
      Protocol* iprot,
      int32_t currFieldId,
      int32_t nextFieldId,
      TType nextFieldType) {
    bool success = Base::advanceToNextField(
        iprot, currFieldId, nextFieldId, nextFieldType);
    tryUpdateChecksum(iprot, success ? nextFieldId : Base::fieldId);
    return success;
  }

  void readFieldBeginNoInline(Protocol* iprot) {
    Base::readFieldBeginNoInline(iprot);
    tryUpdateChecksum(iprot, Base::fieldId);
  }

  void readStructEnd(Protocol* iprot) {
    Base::readStructEnd(iprot);
    if (!hasher_.is_initialized()) {
      return;
    }

    auto actual = static_cast<int64_t>(hasher_);
    if (checksum_ != actual) {
      throwChecksumMismatch(checksum_, actual);
    }
  }

 private:
  template <class Skip>
  folly::IOBuf tryFastSkipImpl(Protocol* iprot, Skip skip) {
    auto cursor = iprot->getCursor();
    skip();
    folly::IOBuf buf;
    cursor.clone(buf, iprot->getCursor() - cursor);
    if (!buf.isManaged()) {
      // Don't unshare the cloned buffer directly, as headroom and
      // tailroom might be non-trivial in size and would be carried
      // over into the allocation.
      buf = buf.cloneCoalescedAsValueWithHeadroomTailroom(0, 0);
      buf.makeManaged();
    }
    return buf;
  }

  void readStructBeginWithIndex(folly::io::Cursor structBegin) {
    if (isLazyDeserializationDisabled()) {
      return;
    }

    Protocol indexReader;
    indexReader.setInput(std::move(structBegin));
    if (!readHeadField(indexReader)) {
      return;
    }

    indexReader.skipBytes(*indexOffset_);
    readIndexField(indexReader);
    if (auto p = folly::get_ptr(fieldIdToSize_, kXxh3ChecksumFieldId)) {
      hasher_.init();
      checksum_ = *p;
    }
  }

  bool readHeadField(Protocol& p) {
    std::string name;
    TType fieldType;
    int16_t fieldId;

    p.readFieldBegin(name, fieldType, fieldId);

    if (fieldId == kExpectedRandomNumberField.id) {
      if (fieldType != kExpectedRandomNumberField.type) {
        return false;
      }
      p.readI64(randomNumber_.emplace());
      p.readFieldEnd();
      p.readFieldBegin(name, fieldType, fieldId);
    }

    if (fieldType != kSizeField.type || fieldId != kSizeField.id) {
      return false;
    }

    double indexOffset;
    p.readDouble(indexOffset);
    p.readFieldEnd();
    if (std::isnan(indexOffset)) {
      return false;
    }

    indexOffset_ = folly::to_integral(indexOffset);
    return true;
  }

  using FieldIdToSize = folly::F14FastMap<int16_t, int64_t>;

  void readIndexField(Protocol& p) {
    std::string name;
    TType fieldType;
    int16_t fieldId;
    p.readFieldBegin(name, fieldType, fieldId);
    if (fieldId != kIndexField.id || fieldType != kIndexField.type) {
      return;
    }

    Cpp2Ops<FieldIdToSize>::read(&p, &fieldIdToSize_);

    if (randomNumber_) {
      auto i = fieldIdToSize_.find(kActualRandomNumberFieldId);
      if (i == fieldIdToSize_.end() || i->second != randomNumber_) {
        fieldIdToSize_.clear();
      }
    }
  }

  void tryUpdateChecksum(Protocol* iprot, int16_t id) {
    if (!hasher_.is_initialized()) {
      return;
    }

    if (auto p = folly::get_ptr(fieldIdToSize_, id)) {
      hasher_.update(folly::io::Cursor(iprot->getCursor(), *p));
    }
  }

  FieldIdToSize fieldIdToSize_;
  folly::Optional<int64_t> indexOffset_;
  folly::Optional<int64_t> randomNumber_;
  int64_t checksum_;
  Xxh3Hasher hasher_;
};

template <class Protocol>
using ProtocolReaderStructReadStateWithIndex = std::conditional_t<
    hasIndexSupport<Protocol>,
    ProtocolReaderStructReadStateWithIndexImpl<Protocol>,
    ProtocolReaderStructReadState<Protocol>>;

} // namespace detail
} // namespace apache::thrift
